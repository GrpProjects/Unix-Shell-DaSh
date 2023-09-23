#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "defs.h"
#include "helpers.c"
#include "builtin.c"
#include "execute.c" //order of import is important

char **DASH_PATH;

int main(int argc, char *argv[])
{
	setbuf(stdout, NULL);
	int mode = INTERACTIVE_MODE; // defaults to interactive mode
	if(argc == 2)
		mode = BATCH_MODE;
	else if(argc > 2) {
		exitWithErr();
		return EXIT_FAILURE;
	}
	
	initDashpath();

	switch(mode)
	{
		case INTERACTIVE_MODE:
			while(true)
			{
				printf("dash> ");

				char *string;
				size_t size = 0;
				getline(&string, &size, stdin);
				string[strcspn(string, "\n")] = 0;
				execute(strdup(string));
			}
			break;

		case BATCH_MODE:
			char *batchFile = validateAndGetFile(argv[1]);
			FILE *fstream = fopen(batchFile, "r");
			if (fstream==NULL)
				exitWithErr();
			char *line;
    		ssize_t read;
			size_t size = 0;
			while ((read = getline(&line, &size, fstream)) != -1) {
				if (line[read - 1] == '\n') {
            		line[read - 1] = '\0';
       			}
				execute(line);
			}
			break;
		default:
			break;

	}
}

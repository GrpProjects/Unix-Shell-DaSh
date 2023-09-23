#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "defs.h"


//extern char* DASH_PATH;

char*
getAvailableFile(char *filename);

char **DASH_PATH;

int
main(int argc, char *argv[])
{
	setbuf(stdout, NULL);
	int mode = INTERACTIVE_MODE; // defaults to interactive mode
	if(argc == 2)
		mode = BATCH_MODE;
	else if(argc > 2)
		return EXIT_FAILURE;
		

	char *string;
	size_t size = 0;
	char *token_separator = " ";
	char *savepointer, *str1;
	DASH_PATH = malloc(sizeof(char*) * 2);
	DASH_PATH[0] = "/bin";
	DASH_PATH[1] = NULL;
	switch(mode)
	{
		case INTERACTIVE_MODE:

		while(true)
		{
			// prompt the user
			printf("dash> ");
			int argindex;

			// get user input
			getline(&string, &size, stdin);
			string[strcspn(string, "\n")] = 0;
			char *string2 = strdup(string);

			// parse the commands entered
			// handle parallel commands ( & operator)
			int commandindex;
			char *str2, *savepointer2;
			char *commandseparator = "&";

			for(commandindex = 0, str2 = string2; ; commandindex++, str2 = NULL)
			{
				char *cmd = strtok_r(str2, commandseparator, &savepointer2);
				if(cmd == NULL)
					break;

				char **myargs = malloc(sizeof(char*) * 2);
				for(argindex = 0, str1 = cmd; ; argindex++, str1 = NULL)
				{
					char *arg = strtok_r(str1, token_separator, &savepointer);
					if(arg == NULL)
						break;
					if(argindex > 1)
					{
						myargs = realloc(myargs, sizeof(char*) * (argindex+1));
					}	
					myargs[argindex] = strdup(arg);
				}

				myargs[argindex] = NULL;

				if(strcmp(myargs[0], "exit") == 0)
				{
					exit(0);
				}
				else if(strcmp(myargs[0], "cd") == 0)
				{
					if(myargs[2] != NULL) //error
						printf("Error: cd cannot have more than one argument\n");
					else	
						chdir(myargs[1]);

					continue;
				}
				else if(strcmp(myargs[0], "path") == 0)
				{
					DASH_PATH = myargs+1;
					printf("DASH_PATH = %s\n", *DASH_PATH);
					continue;

				}		

				// fork and execv() the command
				int rc = fork();
				if(rc == 0) //child
				{
					char *executablefile = getAvailableFile(myargs[0]);
					if(executablefile == NULL)
					{
						printf("Sorry, command - %s not found\n", myargs[0]);
						exit(0);
					}
					free(myargs[0]);
					myargs[0] = executablefile;
					int res = execv(myargs[0], myargs);
					if(res < 0)
						perror("exec error");
				}
				else
				{
					int cid;
					do
					{
						cid = wait(NULL);
						printf("child %d finished\n", cid);
					}while(cid > 0);
				}
			}

				

			free(string); //crashing
			string = NULL;
		}
		break;

		case BATCH_MODE:
		break;
	
		default:
		break;
	}

	return EXIT_SUCCESS;
}

char*
getFilePath(char *filename, char *path)
{
	int filepathlen = strlen(path) + strlen(filename) + 2;
	char filepath[filepathlen];
	snprintf(filepath, filepathlen, "%s/%s", path, filename);

	return strdup(filepath);

}

char*
getAvailableFile(char *filename)
{
	int index = 0;
	while(DASH_PATH[index] != NULL)
	{
		char *filepath = getFilePath(filename, DASH_PATH[index]);
		int ret = access(filepath, X_OK);

		if(ret == 0)
		{
			return filepath;
		}
		free(filepath);
		index++;
	}

	return NULL;
}

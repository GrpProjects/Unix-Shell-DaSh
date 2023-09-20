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

char*
validateAndGetFile(char *filename);

char *DASH_PATH;

int
main(int argc, char *argv[])
{
	setbuf(stdout, NULL);
	int mode = INTERACTIVE_MODE; // defaults to interactive mode
	if(argc == 2)
		mode = BATCH_MODE;
	else if(argc > 2) {
		printf("Too many arguments: Either execute dash in interactive mode or by giving a single batch file\n");
		return EXIT_FAILURE;
	}
		

	char *string;
	size_t size = 0;
	char *token_separator = " ";
	char *savepointer, *str1;

	DASH_PATH = "/bin";
	
	switch(mode)
	{
		case INTERACTIVE_MODE:

		while(true)
		{
			printf("dash> ");
			char **myargs = malloc(sizeof(char*) * 2);
			int argindex;

			getline(&string, &size, stdin);
			string[strcspn(string, "\n")] = 0;

			if(strcmp(string, "exit") == 0)
				exit(0);
	
			// parse the command entered
			for(argindex = 0, str1 = string; ; argindex++, str1 = NULL)
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
			
			// fork and execv() the command
			int rc = fork();
			if(rc == 0) //child
			{
				char *executablefile = getAvailableFile(myargs[0]);
				if(executablefile == NULL)
				{
					char errMsg[100] = "";
					snprintf(errMsg, sizeof(errMsg), "%s: command not found\n", myargs[0]);
					write(STDERR_FILENO, errMsg, strlen(errMsg));
					exit(1);
				}
				free(myargs[0]);
				myargs[0] = executablefile;
				int res = execv(myargs[0], myargs);
				if(res < 0)
					perror("exec error");
			}
			else
			{
				wait(NULL);
			}	

			free(string); //crashing
			string = NULL;
		}

		case BATCH_MODE:
			char *batchFile = validateAndGetFile(argv[1]);
			FILE *fstream = fopen(batchFile, "r");
			if (fstream==NULL){
				char *errMsg = "Unable to open the batch file\n";
				write(STDERR_FILENO, errMsg, strlen(errMsg));
				exit(1);
			}
			char *line;
    		ssize_t read;
			while ((read = getline(&line, &size, fstream)) != -1) {

			}
	
		default:

	}
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
	char *filepath = getFilePath(filename, DASH_PATH);
	int ret = access(filepath, X_OK);

	if(ret == 0)
	{
		return filepath;
	}
	free(filepath);
	return NULL;
}

char*
validateAndGetFile(char *filename)
{
	//first check the extension
	char *ext;
	char *dot = strrchr(filename, '.');
	if(!dot || dot == filename) ext = "";
	else ext = (char*) dot + 1;
	if (strcmp(ext,"txt")!=0){
		char *errMsg = "Batch file must be '.txt' file\n";
 		write(STDERR_FILENO, errMsg, strlen(errMsg));
		exit(1);
	}

	//return if file exits
	char *filepath = getFilePath(filename, ".");
	int ret = access(filepath, X_OK);
	if(ret != 0) {
		free(filepath);
		char *errMsg = "Sorry! Batch file not found\n";
		write(STDERR_FILENO, errMsg, strlen(errMsg));
		exit(1);
	}
	return filepath;
}

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

void 
freeArgs(char **args);

char **DASH_PATH;

void init_dashpath()
{
	DASH_PATH = malloc(sizeof(char*) * 2);
	DASH_PATH[0] = "/bin";
	DASH_PATH[1] = NULL;
}

bool isBuiltInCommand(char *command)
{
	if(strcmp(command, BUILTIN_EXIT) == 0 || strcmp(command, BUILTIN_CD) == 0 || strcmp(command, BUILTIN_PATH) == 0)
	{
		return true;	
	}
	return false;
}

void handleBuiltInCommand(char **myargs)
{
	if(strcmp(myargs[0], BUILTIN_EXIT) == 0)
	{
		exit(0);
	}
	else if(strcmp(myargs[0], BUILTIN_CD) == 0)
	{
		if(myargs[2] != NULL) //error
			printf("Error: cd cannot have more than one argument\n");
		else	
			chdir(myargs[1]);
		freeArgs(myargs);
	}
	else if(strcmp(myargs[0], BUILTIN_PATH) == 0)
	{
		DASH_PATH = myargs+1;
	}
}

void freeArgs(char **args)
{
	int index = 0;
	while(args[index] != NULL)
	{
		free(args[index]);
		index++;
	}
	free(args);
}

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
	

	init_dashpath();	

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

				// parse the commands entered
				// handle parallel commands ( & operator)
				int commandindex;
				char *commandstring, *savepointer2;
				char *commandseparator = "&";

				// handle individual commands
				char *token_separator = " ";
				char *savepointer, *tokenstring;

				for(commandindex = 0, commandstring = string; ; commandindex++, commandstring = NULL)
				{
					char *cmd = strtok_r(commandstring, commandseparator, &savepointer2);
					if(cmd == NULL)
						break;

					char **myargs = malloc(sizeof(char*) * 2);
					for(argindex = 0, tokenstring = cmd; ; argindex++, tokenstring = NULL)
					{
						char *arg = strtok_r(tokenstring, token_separator, &savepointer);
						if(arg == NULL)
							break;
						if(argindex > 1)
						{
							myargs = realloc(myargs, sizeof(char*) * (argindex + 2));
						}	
						myargs[argindex] = strdup(arg);
					}

					myargs[argindex] = NULL;

					if(isBuiltInCommand(myargs[0]))
					{
						handleBuiltInCommand(myargs);
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
						{
							perror("exec error");
							exit(0);	
						}
					}
					else
					{
						int cid;
						while((cid = wait(NULL)) > 0); //wait for all child processes to finish
					}
					
					freeArgs(myargs);
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

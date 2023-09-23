#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "defs.h"

char* getAvailableFile(char *filename, char *path);

char* validateAndGetFile(char *filename);

void exitWithErr(char *errMsg);

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

int main(int argc, char *argv[])
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
	char **dashPath = NULL;
	
	switch(mode)
	{
		case INTERACTIVE_MODE:

		while(true)
		{
			printf("dash> ");

			int argindex;
			bool redirection = 0;
			char *redirectionFile = NULL;

			getline(&string, &size, stdin);
			string[strcspn(string, "\n")] = 0;
			char *string2 = strdup(string);
	
			// parse the command entered
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

				for(argindex = 0, str1 = cmd; ; str1 = NULL)
				{
					char *arg = strtok_r(str1, token_separator, &savepointer);
					if(arg == NULL)
						break;
					if (strcmp(arg,">")==0) {
						redirection = 1;
						continue;
					}
					if (redirection) {
						redirection = 0;
						redirectionFile = arg;
						continue;
					} else if (argindex > 1) {
						myargs = realloc(myargs, sizeof(char*) * (argindex+1));
					}
					myargs[argindex] = strdup(arg);
					argindex++;
				}
				myargs[argindex] = NULL;

				//built in commands handling
				if(strcmp(myargs[0], "exit") == 0)
					exit(0);
				else if(strcmp(myargs[0], "cd") == 0) {
					if(myargs[2] != NULL || myargs[1] == NULL) //error
						exitWithErr("cd command must have only one argument\n");
					else	
						chdir(myargs[1]);
					freeArgs(myargs);
					continue;
				}
				else if(strcmp(myargs[0], "path") == 0) {
					if (myargs[1] == NULL)
						exitWithErr("one or more path required\n");
					free(dashPath);
					dashPath = malloc(sizeof(char*) * (argindex));
					for (int i=1; i<argindex; i++) {
						dashPath[i-1] = strdup(myargs[i]);
					}
					dashPath[argindex-1] = NULL;
					continue;
				}

				//redirection logic
				int redirectionFileNo;
				int saveOut; int saveErr;
				if (redirectionFile!=NULL) {
					redirectionFileNo = open(redirectionFile, O_RDWR|O_CREAT|O_TRUNC , 0600); //trucate if already file exits
					if (redirectionFileNo == -1) 
						exitWithErr("Unable to create or open redirection file\n");

					saveOut = dup(fileno(stdout));
					saveErr = dup(fileno(stderr));

					if (dup2(redirectionFileNo, fileno(stdout)) == -1)
						exitWithErr("Unable to redirect stdout\n");
					if (dup2(redirectionFileNo, fileno(stderr)) == -1)
						exitWithErr("Unable to redirect stderr\n");
				}
				
				// fork and execv() the command
				int rc = fork();
				if(rc == 0) //child
				{
					char *executablefile = NULL;
					for(int i=0; ;i++) {
						if (executablefile !=NULL) break;
						if (dashPath == NULL || dashPath[i]==NULL) {
							if (i=0) executablefile = getAvailableFile(myargs[0], "./");
							break;
						}
						executablefile = getAvailableFile(myargs[0], dashPath[i]);
					}
					if(executablefile == NULL) {
						char errMsg[100] = "";
						snprintf(errMsg, sizeof(errMsg), "%s: command not found\n", myargs[0]);
						exitWithErr(errMsg);
					}
					free(myargs[0]);
					myargs[0] = executablefile;
					int res = execv(myargs[0], myargs);
					if(res < 0)
						perror("exec error");
				} else {
					int cid;
					while((cid = wait(NULL)) > 0); //wait for all child processes to finish
				}	

				// free(string); //crashing
				string = NULL;

				//revert redirection logic to normal
				if (redirectionFile!=NULL){
					fflush(stdout); close(redirectionFileNo);

					dup2(saveOut, fileno(stdout));
					dup2(saveErr, fileno(stderr));

					close(saveOut);
					close(saveErr);
				}
			}
		}

		case BATCH_MODE:
			char *batchFile = validateAndGetFile(argv[1]);
			FILE *fstream = fopen(batchFile, "r");
			if (fstream==NULL)
				exitWithErr("Unable to open the batch file\n");
			char *line;
    		ssize_t read;
			while ((read = getline(&line, &size, fstream)) != -1) {
				printf("%s", line);
			}
	
		default:

	}
}

char* getFilePath(char *filename, char *path)
{
	int filepathlen = strlen(path) + strlen(filename) + 2;
	char filepath[filepathlen];
	snprintf(filepath, filepathlen, "%s/%s", path, filename);

	return strdup(filepath);

}

char* getAvailableFile(char *filename, char* path)
{
	char *filepath = getFilePath(filename, path);
	int ret = access(filepath, X_OK);

	if(ret == 0)
	{
		return filepath;
	}
	free(filepath);
	return NULL;
}

char* validateAndGetFile(char *filename)
{
	//first check the extension
	char *ext;
	char *dot = strrchr(filename, '.');
	if(!dot || dot == filename) ext = "";
	else ext = (char*) dot + 1;
	if (strcmp(ext,"txt")!=0)
		exitWithErr("Batch file must be '.txt' file\n");

	//return if file exits
	char *filepath = getAvailableFile(filename, ".");
	if(filepath == NULL)
		exitWithErr("Sorry! Batch file not found\n");

	return filepath;
}

void exitWithErr(char *errMsg) 
{
	write(STDERR_FILENO, errMsg, strlen(errMsg));
	exit(1);
}

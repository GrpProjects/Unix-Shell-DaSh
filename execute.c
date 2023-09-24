#include "redirection.c"

void execute(char *string);

void execute(char *string) {
	int argindex;
	int commandindex;

	char *token_separator = " ";
	char *str1, *savepointer;
	char *str2, *savepointer2;
	char *commandseparator = "&";

	bool redirection = 0;
	char *redirectionFile = NULL;

	// parse the command entered
	// handle parallel commands ( & operator)
	for(commandindex = 0, str2 = string; ; commandindex++, str2 = NULL)
	{
		char *cmd = strtok_r(str2, commandseparator, &savepointer2);
		if(cmd == NULL)
			break;

		char **myargs = malloc(sizeof(char*) * 2);

		for(argindex = 0, str1 = cmd; ; str1 = NULL)
		{
			char *arg = strtok_r(str1, token_separator, &savepointer);
			if(arg == NULL) break;
			if (strcmp(arg,">")==0) {
				redirection = 1;
				continue;
			}
			if (redirection) {
				redirection = 0;
				redirectionFile = refineRedirectionArgs1(arg);    // ls > out   (arg=out)
				continue;
			} else if (argindex > 1) {
				myargs = realloc(myargs, sizeof(char*) * (argindex+1));
			}

			if (arg[0] == '>') {
				redirectionFile = refineRedirectionArgs1(strdup(++arg));
				continue;
			}
			else redirectionFile = refineRedirectionArgs2(arg);  // ls>out (arg=ls>out)

			myargs[argindex] = strdup(arg); //ls
			argindex++;
		}
		myargs[argindex] = NULL;

		if(isBuiltInCommand(myargs[0]))
		{
			handleBuiltInCommand(myargs);
			continue;
		}

		if (redirectionFile!=NULL)
			redirectToFile(redirectionFile);
		
		// fork and execv() the command
		int rc = fork();
		if(rc == 0) //child
		{
			char *executablefile =  getAvailableFileInDashPath(myargs[0]);
			if(executablefile == NULL)
				exitWithErr();
			free(myargs[0]);
			myargs[0] = executablefile;
			int res = execv(myargs[0], myargs);
			if(res < 0)
				exitWithErr();
		} else {
			int cid;
			while((cid = wait(NULL)) > 0); //wait for all child processes to finish
		}	

		// free(string); //crashing
		string = NULL;

		if (redirectionFile!=NULL) 
			switchRedirectionBack();
	}
}

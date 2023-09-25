#include "redirection.c"

void execute(char *string);

void execute(char *string) 
{
	int argindex;
	int commandindex;

	char *token_separator = " \t";
	char *str1, *savepointer;
	char *str2, *savepointer2;
	char *commandseparator = "&";

	bool redirection = false;
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
			if(arg == NULL)
				break;
			if(strcmp(arg,">") == 0)
			{
				if (!redirection)
					redirection = true;
				else {
					throwErr();
					return;
				}
				continue;
			}
			if(redirection) 
			{
				if (strchr(arg, '>')!=NULL) {
					throwErr();
					return;
				}
				redirectionFile = arg;
				continue;
			}
			else if(argindex >= 1) 
			{
				myargs = realloc(myargs, sizeof(char*) * (argindex + 2));
			}
			
			int count=0;
			char *reStr = strchr(arg, '>');
			while (reStr != NULL) {
				count++;
				if (count>1){
					throwErr();
					return;
				}
				reStr = strchr(reStr + 1, '>'); // Search for the next occurrence
			}
			redirectionFile = refineRedirectionArgs(arg);  // ls>out (arg = ls>out); after changes -> arg = ls, redirection file = out;

			myargs[argindex] = strdup(arg); //ls
			argindex++;
		}
		myargs[argindex] = NULL;

		if(isBuiltInCommand(myargs[0]))
		{
			handleBuiltInCommand(myargs);
			freeArgs(myargs);
			continue;
		}

		if (redirectionFile != NULL)
			redirectToFile(redirectionFile);
		
		// fork and execv() the command
		int rc = fork();
		if(rc == 0) //child
		{
			char *executablefile =  getAvailableFileInDashPath(myargs[0]);
			if(executablefile == NULL)
				exitWithErr();

			int res = execv(executablefile, myargs);
			if(res < 0)
				exitWithErr();
		} 

		if (redirectionFile!=NULL) 
			switchRedirectionBack();

		freeArgs(myargs);
	}

	int cid;
	while((cid = wait(NULL)) > 0); //wait for all child processes to finish

}

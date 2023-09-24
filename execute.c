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
				redirectionFile = refineRedirectionArgs1(arg);
				continue;
			} else if (argindex > 1) {
				myargs = realloc(myargs, sizeof(char*) * (argindex+1));
			}

			redirectionFile = refineRedirectionArgs2(arg);

			myargs[argindex] = strdup(arg);
			argindex++;
		}
		myargs[argindex] = NULL;

		if(isBuiltInCommand(myargs[0]))
		{
			handleBuiltInCommand(myargs);
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
			char *executablefile =  getAvailableFileInDashPath(myargs[0]);
			if(executablefile == NULL) {
				char errMsg[100] = "";
				snprintf(errMsg, sizeof(errMsg), "%s: command not found\n", myargs[0]);
				exitWithErr(errMsg);
			}
			free(myargs[0]);
			myargs[0] = executablefile;
			int res = execv(myargs[0], myargs);
			if(res < 0) {
				perror("exec error");
				exit(1);
			}
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

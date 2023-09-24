bool isBuiltInCommand(char *command);
void handleBuiltInCommand(char **myargs);

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
		if(myargs[1] != NULL)
		{
			throwErr();
		}
		else
		{
			int cid;
			while((cid = wait(NULL)) > 0); //wait for all child processes to finish. To avoid Orphan processes
			exit(0);
		}
	}
	else if(strcmp(myargs[0], BUILTIN_CD) == 0)
	{
		if(myargs[2] != NULL)
			throwErr();
		else	
			chdir(myargs[1]);

	}
	else if(strcmp(myargs[0], BUILTIN_PATH) == 0)
	{
		// clear existing dash path
		clearDashPath();
		
		// copying new paths to dash path
		int argindex = 1;
		while(myargs[argindex] != NULL)
		{
			DASH_PATH = realloc(DASH_PATH, sizeof(char*) * (argindex + 1));
			DASH_PATH[argindex-1] = strdup(myargs[argindex]);
			argindex++;
		}
		DASH_PATH[argindex-1] = NULL;
		
	}
}

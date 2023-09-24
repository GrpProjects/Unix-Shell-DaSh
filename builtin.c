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
			exitWithErr();
		}
		exit(0);
	}
	else if(strcmp(myargs[0], BUILTIN_CD) == 0)
	{
		if(myargs[2] != NULL) //error
			throwErr();
		else	
			chdir(myargs[1]);
		freeArgs(myargs);
	}
	else if(strcmp(myargs[0], BUILTIN_PATH) == 0)
	{
		DASH_PATH = myargs+1;
	}
}

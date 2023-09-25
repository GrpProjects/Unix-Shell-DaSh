#include "defs.h"


void throwErr();
void exitWithErr();
void freeArgs(char **args);
void initDashpath();
char* getFilePath(char *filename, char *path);
char* getAvailableFile(char *filename, char* path);
char* getAvailableFileInDashPath(char *filename);
char* validateAndGetFile(char *file);
char* refineRedirectionArgs(char *arg);

void throwErr()
{
	char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void exitWithErr() 
{
	throwErr();	
    	exit(1);
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

void initDashpath()
{
	DASH_PATH = malloc(sizeof(char*) * 2);
	DASH_PATH[0] = strdup("/bin");
	DASH_PATH[1] = NULL;
}

void clearDashPath()
{
	int index = 0;
	while(DASH_PATH[index] != NULL)
	{
		free(DASH_PATH[index]);
		index++;
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

char* getAvailableFileInDashPath(char *filename)
{
	int index = 0;
	while(DASH_PATH[index] != NULL)
	{
		char *filepath = getAvailableFile(filename, DASH_PATH[index]);
		if (filepath!=NULL) 
		{
			return filepath;
		}
		index++;
	}
	return NULL;
}

char* validateAndGetFile(char *file)
{
	char *lastSlash = strrchr(file, '/');
	char *fileName; 
	char *filePath;
	if(lastSlash != NULL)
	{
		size_t lastSlashPos = lastSlash - file;
		filePath = (char*) malloc(lastSlashPos + 1);
		strncpy(filePath, file, lastSlashPos);
		filePath[lastSlashPos] = '\0';
		fileName = lastSlash + 1;
	} 
	else 
	{
		exitWithErr();
	}

	char *availabeFile = getAvailableFile(fileName, filePath);
	if(availabeFile == NULL)
		exitWithErr();

	return file;
}


char* refineRedirectionArgs(char *arg)
{
	//to support redirection without 
	int i; char* str; char* savepointer; char* redirectionFile = NULL;
	for(i=0, str = arg; ; str=NULL, i++)
	{
		char *refinedArg = strtok_r(str, ">", &savepointer);
		if(refinedArg == NULL) 
			break;
		if(i == 0) 
			arg = refinedArg;
		else 
			redirectionFile = refinedArg;
	}

	return redirectionFile;
}

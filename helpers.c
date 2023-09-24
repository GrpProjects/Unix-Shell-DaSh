#include "defs.h"

extern char **DASH_PATH;

void exitWithErr();
void freeArgs(char **args);
void initDashpath();
char* getFilePath(char *filename, char *path);
char* getAvailableFile(char *filename, char* path);
char* getAvailableFileInDashPath(char *filename);
char* validateAndGetFile(char *filename);
char* refineRedirectionArgs1(char *arg);
char* refineRedirectionArgs2(char *arg);

void exitWithErr() 
{
	char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
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
	DASH_PATH[0] = "/bin";
	DASH_PATH[1] = NULL;
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
		if (filepath!=NULL) {
			return filepath;
		}
		index++;
	}
}

char* validateAndGetFile(char *filename)
{
	//first check the extension
	char *ext;
	char *dot = strrchr(filename, '.');
	if(!dot || dot == filename) ext = "";
	else ext = (char*) dot + 1;
	if (strcmp(ext,"txt")!=0)
		exitWithErr();

	//return if file exits
	char *filepath = getAvailableFile(filename, ".");
	if(filepath == NULL)
		exitWithErr();

	return filepath;
}

char* refineRedirectionArgs1(char *arg)
{
	char *refinedArg = refineRedirectionArgs2(arg);
	if (refinedArg == NULL) return arg;
	return refinedArg;
}


char* refineRedirectionArgs2(char *arg)
{
	//to support redirection without 
	int i; char* str; char* savepointer; char* redirectionFile = NULL;
	for (i=0, str = arg; ; str=NULL, i++)
	{
		char *refinedArg = strtok_r(str, ">", &savepointer);
		if (refinedArg == NULL) break;
		if (i==0) arg = refinedArg;
		else redirectionFile = refinedArg;
	}
	return redirectionFile;
}

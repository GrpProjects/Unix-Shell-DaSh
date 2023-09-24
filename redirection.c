void redirectToFile(char *filename);
void switchRedirectionBack();

int redirectionFileNo;
int saveOut; int saveErr;

void redirectToFile(char *redirectionFile) 
{
    redirectionFileNo = open(redirectionFile, O_RDWR|O_CREAT|O_TRUNC , 0600); //trucate if already file exits
    if (redirectionFileNo == -1) 
        exitWithErr();

    saveOut = dup(fileno(stdout));
    saveErr = dup(fileno(stderr));

    if (dup2(redirectionFileNo, fileno(stdout)) == -1)
        exitWithErr();
    if (dup2(redirectionFileNo, fileno(stderr)) == -1)
        exitWithErr();
}

void switchRedirectionBack() 
{
    fflush(stdout); 
    close(redirectionFileNo);

    dup2(saveOut, fileno(stdout));
    dup2(saveErr, fileno(stderr));

    close(saveOut);
    close(saveErr);
}

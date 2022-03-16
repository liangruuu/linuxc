#include <stdio.h>
#include <stdlib.h>

extern char **environ;

int main()
{
    // for (int i = 0; environ[i] != NULL; i++)
    // {
    //     puts(environ[i]);
    // }

    puts(getenv("PATH"));
    // getchar();
    



    exit(0);
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>


int main(int argc, char **argv)
{
    struct passwd *pwdline;

    if(argc < 2)
    {
        fprintf(stderr, "Usage:%s <user id>\n",argv[0]);

    }
    
    pwdline = getpwuid(atoi(argv[1]));
    printf("user name:%s\n", pwdline->pw_name);
    exit(0);
}

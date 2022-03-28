#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>


int main(int argc, char **argv)
{
    struct passwd *pwdline;
    struct spwd *spwd;

    if(argc < 2)
    {
        fprintf(stderr, "Usage:%s <user name>\n",argv[0]);

    }
    
    pwdline = getpwnam(argv[1]);
    printf("user id:%d\n", pwdline->pw_uid);
    printf("user passwd:%s\n", pwdline->pw_passwd);

    spwd = getspnam(argv[1]);

    if (spwd != NULL) 
        printf("user shadow passwd:%s\n", spwd->sp_pwdp);
       

    exit(0);
}

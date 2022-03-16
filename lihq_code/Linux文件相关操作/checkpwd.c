#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>
#include <unistd.h>
#include <crypt.h>

// gcc -Wall -g -o checkpwd checkpwd.c -D_XOPEN_SOURCE -lcrypt 
// 要用root才能有权限执行

int main(int argc, char **argv)
{
    char *input_pass;
    char *crypted_pass;
    struct spwd *shadowline;

    if(argc < 2)
    {
        fprintf(stderr, "Usage:%s <user name>\n",argv[0]);
        exit(1);

    }

    input_pass = getpass("PassWord:");
    shadowline = getspnam(argv[1]);
    crypted_pass = crypt(input_pass, shadowline->sp_pwdp);

    if((strcmp(shadowline->sp_pwdp, crypted_pass) == 0))
        puts("ok!");
    else
        puts("faile!");
    
    exit(0);
}

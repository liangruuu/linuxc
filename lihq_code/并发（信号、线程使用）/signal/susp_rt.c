#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <unistd.h>
#include <unistd.h>

#define MYRTSIG (SIGRTMIN+6)

static void mysig_handler(int s)
{
    write(1, "!", 1);
}

int main()
{
    sigset_t set, oset, saveset;

    signal(MYRTSIG, mysig_handler);
    sigemptyset(&set);
    sigaddset(&set, MYRTSIG);
    sigprocmask(SIG_UNBLOCK, &set, &saveset);
    sigprocmask(SIG_BLOCK, &set, &oset);

    for(int j = 0; j < 1000; j++)
    {
         
         for(int i = 0; i < 5; i++)
         {
            write(1, "*", 1);
             sleep(1);
         }
         write(1, "\n", 1);
         sigsuspend(&oset);
    }
     sigprocmask(SIG_SETMASK, &saveset, NULL);

    exit(0);
}

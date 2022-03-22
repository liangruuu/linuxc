#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    while (1)
    {
        prompt();

        getline();

        parse();

        if (是内部命令)
        {
        }
        else if (是外部命令)
        {
            fork();
            if (< 0)
            {
            }
            if (== 0) // child
            {
                execXX();
                perror();
                exit(1);
            }
            else // parent
            {
                wait();
            }
        }
    }

    exit(0);
}
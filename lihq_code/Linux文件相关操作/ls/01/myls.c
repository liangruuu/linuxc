#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "stat.h"

int main(int argc, char *argv[])
{
    char *optstring = "-l:i:a:h:";
    int c;
    while (1)
    {
        c = getopt(argc, argv, optstring);
        if (c == -1)
            break;
        switch (c)
        {
        case 'l':
            listl(optarg);
            break;
        case 'i':
            listi(optarg);
            break;
        case 'a':
            lista(optarg);
            break;
        case 'h':
            listh(optarg);
            break;
        case 'r':
            printf("识别到可选参数的选项-e\n");
            if (optarg != NULL)
                printf("参数:%s\n", optarg);
            break;
        case '?':
            printf("不认识此选项%s\n", argv[optind - 1]);
            break;
        case 1:
            printf("非选项参数%s\n", argv[optind - 1]);
            break;
        default:
            break;
        }
    }

    exit(0);
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define FMTSIZE 1024
#define TIMESIZE 1024

/*
 *  -y: year
 *  -m: month
 *  -d: day
 *  -H: hour
 *  -M: minute
 *  -S: second
 * */


int main(int argc, char **argv)
{
    FILE *fp = stdout;
    char fmtstr[FMTSIZE];
    char timestr[TIMESIZE];
    time_t stamp;
    struct tm *tm;
    int c;

    fmtstr[0] = '\0';
    
    stamp = time(NULL);
    tm = localtime(&stamp);
    
    while(1)
    {
     c = getopt(argc, argv, "-H:MSy:md");
     if(c < 0)
         break;

     switch(c)
     {
         case 1:
            if(fp == stdout)
            {
                fp = fopen(argv[optind - 1], "w");
                if (fp == NULL)
                {
                    perror("fopen()");
                    fp = stdout;
                }
            }
            break;
         case 'H':
            if(strcmp(optarg, "12") == 0)
                strncat(fmtstr, "%I(%P) ", FMTSIZE);
            else if(strcmp(optarg, "24") == 0)
                strncat(fmtstr, "%H ", FMTSIZE);
            else
                fprintf(stderr, "Invalid argument of %s\n", optarg);
             break;
         case 'M':
             strncat(fmtstr, "%M ", FMTSIZE);
             break;
         case 'S':
            strncat(fmtstr, "%S ", FMTSIZE);
             break;
         case 'y':
            if(strcmp(optarg, "2") == 0)
                strncat(fmtstr, "%y ", FMTSIZE);
            else if(strcmp(optarg, "4") == 0)
                strncat(fmtstr, "%Y ", FMTSIZE);
            else
                fprintf(stderr, "Invalid argument of %s\n", optarg);
             break;
         case 'm':
            strncat(fmtstr, "%m ", FMTSIZE);
             break;
         case 'd':
            strncat(fmtstr, "%d ", FMTSIZE);
             break;
         default:
             break;
     }
    }

    strncat(fmtstr, "\n", FMTSIZE);
    strftime(timestr, TIMESIZE, fmtstr, tm);
    fputs(timestr, fp);
    
    if(fp != stdout)
        fclose(fp);
    
    exit(0);
}

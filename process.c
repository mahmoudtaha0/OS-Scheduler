#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int stop = 0;
int curr_time;

// char *args[] = {"./process.out", p->remainingtime, NULL};
// execve(args[0], args, NULL);
void stop_process (int signum)
{
    curr_time = getClk();
    signal(SIGTSTP, stop_process);
}

int main(int agrc, char * argv[])
{

    signal(SIGTSTP, stop_process);

    initClk();

    curr_time = getClk();

    remainingtime = atoi(argv[1]);
    
    //TODO it needs to get the remaining time from somewhere
    //remainingtime = ??;
    while (remainingtime > 0)
    {
        int new_time = getClk();

        if (new_time > curr_time)
        {
            curr_time = new_time;

            remainingtime--;
            // printf("Process: %d has remaining time: %d\n", , remainingtime);
        }

    }
    // printf("Process ended successfully...\n");
    
    destroyClk(false);
    
    return 0;
}

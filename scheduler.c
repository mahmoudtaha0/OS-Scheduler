#include "headers.h"
#include "memory.h"
#include "write_to_file.h"
#include "StandardDeviation.h"
#include "queue.h"
#include "HPF.h"
#include "RR.h"
#include "SRTN.h"
int msgq_id, sem1, sem2;

int waiting_time = 0;


void clearResources(int);

int main(int argc, char * argv[])
{

    init_memory(memory);

    union Semun semun;
    
    key_t key_id_sem = ftok("keyfile", 70);

    sem1 = semget(key_id_sem, 1, 0666 | IPC_CREAT);
    sem2 = semget(key_id_sem + 1, 1, 0666 | IPC_CREAT);

    if (sem1 == -1 || sem2 == -1)
    {
        perror("Error in create sem");
        exit(-1);
    }

    semun.val = 0; /* initial value of the semaphore, Binary semaphore */
    if (semctl(sem1, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
    if (semctl(sem2, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }

    initClk();

    if (strcmp(argv[1], "1") == 0)
    {
        printf("HPF\n");
        HPF(sem1, sem2);
    }
    if (strcmp(argv[1], "2") == 0)
    {
        printf("SRTN\n");
        SRTN(sem1, sem2);
    }
    if (argv[1][0] == '3')
    {
        printf("RR\n");
        int quantum = atoi(&argv[1][1]);
        RR(quantum, sem1, sem2);
    }

    // remove later
    sleep(5);

    clearResources(true);
    
    
}

void clearResources(int signum)
{
    // delete msgqueue
    // msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
    semctl(sem1, 0, IPC_RMID, 0);
    semctl(sem2, 0, IPC_RMID, 0);

    destroyClk(true);

    raise(SIGKILL);
}

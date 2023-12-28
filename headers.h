#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>

typedef short bool;
#define true 1
#define false 0
#define fail -1

#define SHKEY 300

bool isRunning = false;

int start_range=0, end_range=0;


typedef struct Process
{

    int pid;            // pid of forked process
    int processed_time; // time where process is running

    /**************************/
    int id;
    int starttime;
    int endtime;
    int arrival_time;
    int run_time;
    int priority;
    int stop_time;
    /**************************/

    int remaining_time; // remaining time of the process
    char *status;       // status of process (started, finished, stopped, resumed)
    int waiting_time;   // process waiting time
                        /*************************/

    int memsize;

    struct Process *next;

} Process;

typedef struct Queue
{
    Process *front, *rear;
} Queue;



/************************************SEMAPHORES************************************************/
union Semun
{
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO (Linux-specific) */
};

// if wait == false -> IPC_NOWAIT
bool down(int sem, bool wait)
{
    struct sembuf op;

    op.sem_num = 0;
    op.sem_op = -1;
    if (wait)
        op.sem_flg = !IPC_NOWAIT;
    else
        op.sem_flg = IPC_NOWAIT;

    if (semop(sem, &op, 1) == -1)
    {
        return false;
    }

    return true;
}

// if wait == false -> IPC_NOWAIT
bool up(int sem, bool wait)
{
    struct sembuf op;

    op.sem_num = 0;
    op.sem_op = 1;
    if (wait)
        op.sem_flg = !IPC_NOWAIT;
    else
        op.sem_flg = IPC_NOWAIT;

    if (semop(sem, &op, 1) == -1)
    {
        return false;
    }

    return true;
}

/*****************************************MSGQUEUE****************************************/
struct msgbuff
{
    long mtype;
    Process p;
};

///==============================
// don't mess with this variable//
int *shmaddr; //
//===============================

int getClk()
{
    return *shmaddr;
}

/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
 */
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        // Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
 */

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

#endif

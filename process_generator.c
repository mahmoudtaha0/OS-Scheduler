#include "headers.h"
#include "queue.h"

//GLOBAL_VARS
int msgq_id, sem1, sem2;
char alg_str[3];

void clearResources(int);

int readInputFile(const char* filename, Queue *q)
{

    FILE* file = fopen(filename, "r");

    if (!file)
    perror("Error opening input file");

    char line[100];

    // Counter for processes
    int count = 0;

    while(fgets(line, sizeof(line), file)) {
        if (line[0] != '#') {
            Process *p = initProcess();

            sscanf(line, "%d\t%d\t%d\t%d\t%d", &p->id, &p->arrival_time, &p->run_time, &p->priority, &p->memsize);
            p->remaining_time = p->run_time;
            enQueue(q,p);
            count++;
        }
    }
    return count;
}


int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);

    // init sem
    union Semun semun;
    key_t key_id_sem = ftok("keyfile", 70);

    // init msg queue
    key_t key_id;
    struct msgbuff message;
    int send_val;

    // create msg queue
    key_id = ftok("keyfile", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    int sem1 = semget(key_id_sem, 1, 0666 | IPC_CREAT);
    int sem2 = semget(key_id_sem + 1, 1, 0666 | IPC_CREAT);

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
    

    
    // Queue to enqueue processes in
    Queue* Q = initQueue();

    int pcount = readInputFile(argv[1], Q);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("Select the scheduling algorithm:\n");
    printf("1) Non-preemptive Highest Priority First\n");
    printf("2) Shortest Remaining Time Next\n");
    printf("3) Round Robin\n");
    char alg = '0', quantum = '0';
    scanf("%c", &alg);

    alg_str[0] = alg;
    alg_str[1] = '\0';

    // Handle RR Selection
    if (alg == '3') {
    printf("Enter time quantum for RR: ");
    scanf(" %c", &quantum);
    alg_str[0] = '3';
    alg_str[1] = quantum;
    alg_str[2] = '\0';
    }

    // 3. Initiate and create the scheduler and clock processes.
    int pid;
        
    //create Clock
    pid = fork();
    if (pid == 0)
    {
        char *args[] = {"./clk.out", NULL};
        execv(args[0], args);
    }

    // Create scheduler
    pid = fork();
    if (pid == 0)
    {
        char *args[] = {"./scheduler.out", alg_str, NULL};
        execv(args[0], args);
    }

    initClk();
    // To get time use this
    int curr_time = getClk();


    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    while(true)
    {
        int new_time = getClk();

        if(new_time > curr_time)
        {
            // get time every time cycle
            printf("current time is %d\n", curr_time);
            curr_time = new_time;

            while (!isEmpty(Q) && Q->front->arrival_time==curr_time)
            {

            // create a process to put in msgbuff and send
            Process p_send = *Q->front;
            message.p = p_send;

            // printf("%d\n", p_send.memsize);

            send_val = msgsnd(msgq_id, &message, sizeof(message.p), !IPC_NOWAIT);

            deQueue(Q);
            }
        }  

        // all processes are sent successfully
        if (isEmpty(Q))
            break;
    }

    
    //let scheduler know that no another processes will be recieved
    up(sem2, true);
    // wait until scheduler finish
    down(sem1, true);

    printf("Process Generator Finished...\n");

    // remove later
    sleep(5);

    clearResources(true);

    // 7. Clear clock resources
    

}

void clearResources(int signum)
{
    // delete msgqueue
    msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
    // delete sem

    semctl(sem1, 0, IPC_RMID, 0);
    semctl(sem2, 0, IPC_RMID, 0);

    destroyClk(true);

    raise(SIGKILL);
}
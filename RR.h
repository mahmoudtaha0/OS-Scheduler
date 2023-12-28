#include "headers.h"

int counter = 0;
int pid;
float TWT_RR = 0.0, TWTA_RR = 0.0;
float AWTA_RR, AWT_RR;
float *WTA_values_RR;
int WTA_count_RR = 0;
float SD_RR;
int total_runtime_RR = 0;
int pcount_RR = 0;



void RR(int Q, int sem1, int sem2)
{

    printf("Quantum = %d\n", Q);
    // create scheduler.log
    FILE *file = fopen("scheduler.log", "w");
    char text[] = "#At time x process y state arr w total z remain y wait k\n";

    if (file == NULL)
    {
        perror("Error opening file for append");
        return;
    }

    // Write text to the file
    fprintf(file, "%s", text);

    // Close the file
    fclose(file);

    // create memory.log
    FILE *filemem = fopen("memory.log", "w");
    char textmem[] = "#At time x allocated y bytes for process z from i to j\n";

    if (filemem == NULL)
    {
        perror("Error opening file for append");
        return;
    }

    // Write text to the file
    fprintf(filemem, "%s", textmem);

    // Close the file
    fclose(filemem);

    Queue *ready_queue = initQueue();
    Queue *waiting_queue = initQueue();
    key_t key_id;
    int rec_val, msgq_id, status, currtime = getClk();
    int quantum = Q;

    key_id = ftok("keyfile", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    // printf("schedular starts to receive msgs\n");
    struct msgbuff message;
    while (true)
    {
        rec_val = msgrcv(msgq_id, &message, sizeof(message.p), 0, IPC_NOWAIT);
        if (rec_val != -1)
        {

            Process *pro = &message.p;
            if (allocate_memory(pro->id, pro->memsize))
            {
                get_memory_ranges(pro->id, &start_range, &end_range);
                // print_memory_state();
                enQueue(ready_queue, pro);
                write_to_file("memory.log", "At time %d allocated %d bytes for process %d from %d to %d\n",
                    getClk(),
                    pro->memsize,
                    pro->id,
                    start_range,
                    end_range);
            }
            else
                enQueue(waiting_queue, pro);

            // printf("Process %d recieved\n", pro->id);

            while (msgrcv(msgq_id, &message, sizeof(message.p), 0, IPC_NOWAIT) != -1)
            {

                Process *pro = &message.p;
                if (allocate_memory(pro->id, pro->memsize))
                {
                    get_memory_ranges(pro->id, &start_range, &end_range);
                    // print_memory_state();
                    enQueue(ready_queue, pro);
                    write_to_file("memory.log", "At time %d allocated %d bytes for process %d from %d to %d\n",
                        getClk(),
                        pro->memsize,
                        pro->id,
                        start_range,
                        end_range);
                }
                else
                    enQueue(waiting_queue, pro);
                // printf("Process %d recieved\n", pro->id);
            }
        }

        if (!isEmpty(ready_queue) && !isRunning)
        {

            if (ready_queue->front->pid == 0)
            {

                counter = 0;

                // printf("process %d starting...\n", ready_queue->front->id);
                isRunning = true;

                ready_queue->front->starttime = currtime;

                if (ready_queue->front->arrival_time != ready_queue->front->starttime)
                {
                    ready_queue->front->waiting_time = ready_queue->front->waiting_time + ready_queue->front->starttime - ready_queue->front->arrival_time;
                }

                pid = fork();
                if (pid == 0)
                {

                    char rem_time[3];
                    sprintf(rem_time, "%d", ready_queue->front->remaining_time);
                    char *args[] = {"./process.out", rem_time, NULL};

                    pid = getpid(); // set pid
                    ready_queue->front->status = "started";
                    currtime = getClk();

                    printf("At time %d process %d %s arr %d total %d remain %d wait %d\n",
                           currtime,
                           ready_queue->front->id,
                           ready_queue->front->status,
                           ready_queue->front->arrival_time,
                           ready_queue->front->run_time,
                           ready_queue->front->remaining_time,
                           ready_queue->front->waiting_time);

                    write_to_file("scheduler.log", "At time %d process %d %s arr %d total %d remain %d wait %d\n",
                                  getClk(),
                                  ready_queue->front->id,
                                  ready_queue->front->status,
                                  ready_queue->front->arrival_time,
                                  ready_queue->front->run_time,
                                  ready_queue->front->remaining_time,
                                  ready_queue->front->waiting_time);
                    // printf("Process forked: %d...\n", getpid());

                    if (execve(args[0], args, NULL) == fail)
                        exit(fail);
                }

                ready_queue->front->pid = pid;
            }
            else
            {
                isRunning = true;
                // printf("Process resumed: %d %d...\n", ready_queue->front->id, ready_queue->front->pid);

                ready_queue->front->status = "resumed";

                currtime = getClk();
                // waiting = waiting + current time - stopped time
                ready_queue->front->waiting_time = ready_queue->front->waiting_time + currtime - ready_queue->front->stop_time;

                printf("At time %d process %d %s arr %d total %d remain %d wait %d\n",
                       currtime,
                       ready_queue->front->id,
                       ready_queue->front->status,
                       ready_queue->front->arrival_time,
                       ready_queue->front->run_time,
                       ready_queue->front->remaining_time,
                       ready_queue->front->waiting_time);

                write_to_file("scheduler.log", "At time %d process %d %s arr %d total %d remain %d wait %d\n",
                              getClk(),
                              ready_queue->front->id,
                              ready_queue->front->status,
                              ready_queue->front->arrival_time,
                              ready_queue->front->run_time,
                              ready_queue->front->remaining_time,
                              ready_queue->front->waiting_time);

                kill(ready_queue->front->pid, SIGCONT);
            }
        }

        if (getClk() > currtime)
        {
            // if (!isEmpty(ready_queue))
            // printf("Process %d wait %d...\n", ready_queue->front->id, ready_queue->front->waiting_time);
            // printQueue(ready_queue);
            // printf("wait_queue: ");
            // printQueue(waiting_queue);
            counter++;
            if (!isEmpty(ready_queue))
                ready_queue->front->remaining_time--;
            // printf("Process %d remaining %d...\n", ready_queue->front->id, ready_queue->front->remaining_time);
            currtime = getClk();

            if (!isEmpty(ready_queue) && ready_queue->front->remaining_time == 0)
            {
                // printf("Process %d remaining %d at time %d...\n", ready_queue->front->id, ready_queue->front->remaining_time,currtime);
                // printf("Process %d finished...\n", ready_queue->front->id);
                ready_queue->front->status = "finished";
                ready_queue->front->remaining_time = 0;
                ready_queue->front->endtime = getClk();
                total_runtime_RR += ready_queue->front->run_time;
                pcount_RR++;

                ready_queue->front->waiting_time = ready_queue->front->endtime - ready_queue->front->arrival_time - ready_queue->front->run_time;

                printf("At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
                       getClk(),
                       ready_queue->front->id,
                       ready_queue->front->status,
                       ready_queue->front->arrival_time,
                       ready_queue->front->run_time,
                       ready_queue->front->remaining_time,
                       ready_queue->front->waiting_time,
                       ready_queue->front->endtime - ready_queue->front->arrival_time,
                       ((float)(ready_queue->front->endtime - ready_queue->front->arrival_time) / (float)ready_queue->front->run_time));

                TWTA_RR += (float)(ready_queue->front->endtime - ready_queue->front->arrival_time) / (float)ready_queue->front->run_time;
                TWT_RR += ready_queue->front->waiting_time;

                WTA_values_RR = realloc(WTA_values_RR, (WTA_count_RR + 1) * sizeof(float));
                WTA_values_RR[WTA_count_RR++] = (float)(ready_queue->front->endtime - ready_queue->front->arrival_time) / (float)ready_queue->front->run_time;

                write_to_file("scheduler.log", "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
                              getClk(),
                              ready_queue->front->id,
                              ready_queue->front->status,
                              ready_queue->front->arrival_time,
                              ready_queue->front->run_time,
                              ready_queue->front->remaining_time,
                              ready_queue->front->waiting_time,
                              ready_queue->front->endtime - ready_queue->front->arrival_time,
                              ((float)(ready_queue->front->endtime - ready_queue->front->arrival_time) / (float)ready_queue->front->run_time));

                // Process *temp = ready_queue->front;
                get_memory_ranges(ready_queue->front->id, &start_range, &end_range);
                deallocate_memory(ready_queue->front->id);
                write_to_file("memory.log", "At time %d freed %d bytes for process %d from %d to %d\n",
                    getClk(),
                    ready_queue->front->memsize,
                    ready_queue->front->id,
                    start_range,
                    end_range);
                deQueue(ready_queue);
                // print_memory_state();

            if (!isEmpty(waiting_queue))
            {

                // temp = waiting_queue->front;
                if (allocate_memory(waiting_queue->front->id, waiting_queue->front->memsize))
                {
                    // print_memory_state();
                    get_memory_ranges(waiting_queue->front->id, &start_range, &end_range);
                    enQueue(ready_queue, waiting_queue->front);
                    write_to_file("memory.log", "At time %d allocated %d bytes for process %d from %d to %d\n",
                        getClk(),
                        waiting_queue->front->memsize,
                        waiting_queue->front->id,
                        start_range,
                        end_range);
                }
                deQueue(waiting_queue);
            }



                isRunning = false;
                counter = 0;
            }
            else if (!isEmpty(ready_queue) && counter == quantum)
            {

                // printf("hello %d\n", getClk());
                // printQueue(ready_queue);

                // printf("Process stopped: %d %d...\n", ready_queue->front->id, ready_queue->front->pid);
                ready_queue->front->status = "stopped";
                ready_queue->front->stop_time = getClk();

                // printf("Process %d stopped: %d...\n", ready_queue->front->id, ready_queue->front->waiting_time);
                printf("At time %d process %d %s arr %d total %d remain %d wait %d\n",
                       currtime,
                       ready_queue->front->id,
                       ready_queue->front->status,
                       ready_queue->front->arrival_time,
                       ready_queue->front->run_time,
                       ready_queue->front->remaining_time,
                       ready_queue->front->waiting_time);

                write_to_file("scheduler.log", "At time %d process %d %s arr %d total %d remain %d wait %d\n",
                              getClk(),
                              ready_queue->front->id,
                              ready_queue->front->status,
                              ready_queue->front->arrival_time,
                              ready_queue->front->run_time,
                              ready_queue->front->remaining_time,
                              ready_queue->front->waiting_time);

                // ready_queue->front->waiting_time++;
                kill(ready_queue->front->pid, SIGTSTP);

                /**********************************************/
                rec_val = msgrcv(msgq_id, &message, sizeof(message.p), 0, IPC_NOWAIT);
                if (rec_val != -1)
                {

                    Process *pro = &message.p;
                    if (allocate_memory(pro->id, pro->memsize))
                    {
                        get_memory_ranges(pro->id, &start_range, &end_range);
                        // print_memory_state();
                        enQueue(ready_queue, pro);
                        // printf("ready\n");
                        write_to_file("memory.log", "At time %d allocated %d bytes for process %d from %d to %d\n",
                                    getClk(),
                                    pro->memsize,
                                    pro->id,
                                    start_range,
                                    end_range);
                    }
                    else
                    {
                        enQueue(waiting_queue, pro);
                        // printf("%d\n", pro->memsize);
                    }

                    // printf("Process %d recieved\n", pro->id);

                    while (msgrcv(msgq_id, &message, sizeof(message.p), 0, IPC_NOWAIT) != -1)
                    {
                        Process *pro = &message.p;
                        if (allocate_memory(pro->id, pro->memsize))
                        {
                            get_memory_ranges(pro->id, &start_range, &end_range);
                            // print_memory_state();
                            enQueue(ready_queue, pro);
                            // printf("ready\n");
                            write_to_file("memory.log", "At time %d allocated %d bytes for process %d from %d to %d\n",
                                        getClk(),
                                        pro->memsize,
                                        pro->id,
                                        start_range,
                                        end_range);
                        }
                        else
                        {
                            enQueue(waiting_queue, pro);
                            // printf("%d\n", pro->memsize);
                        }
                        // printf("Process %d recieved\n", pro->id);
                    }
                }
                /**********************************************/

                Process temp = *ready_queue->front;
                deQueue(ready_queue);
                enQueue(ready_queue, &temp);
                counter = 0;
                isRunning = false;
            }
        }

        // process finished
        if (isEmpty(ready_queue) && isEmpty(waiting_queue))
        {
            // see whether the process generator finished or not
            bool is_finish = down(sem2, false);
            if (is_finish)
            {
                printf("%d\n", pcount_RR);

                // Average Waiting Time
                AWT_RR = TWT_RR / pcount_RR;

                // Average Weighted Turnaround Time
                AWTA_RR = TWTA_RR / pcount_RR;

                SD_RR = calculateStandardDeviation(WTA_values_RR, WTA_count_RR);

                int total_time_RR = getClk();

                float cpu_utilization_RR = ((float)total_runtime_RR / (float)total_time_RR) * 100;

                // create scheduler.log
                FILE *file = fopen("scheduler.perf", "w");

                if (file == NULL)
                {
                    perror("Error opening file for append");
                    return;
                }

                // Write text to the file
                fprintf(file, "CPU Utilization = %.2f%%\n", cpu_utilization_RR);
                fprintf(file, "Avg WTA = %.2f\n", AWTA_RR);
                fprintf(file, "Avg Waiting = %.2f\n", AWT_RR);
                fprintf(file, "Std WTA = %.2f\n", SD_RR);

                // Close the file
                fclose(file);
                // let the process generator terminate
                up(sem1, true);
                return;
            }
        }
    }
}
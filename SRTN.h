#include "headers.h"

int pid;
int pcount = 0;
float TWT = 0.0, TWTA = 0.0;
float AWTA, AWT;
float *WTA_values;
int WTA_count = 0;
float SD;

int total_runtime = 0;

void SRTN(int sem1, int sem2)
{
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

    // queue to recieve processes from process generator
    Queue *ready_queue = initQueue();
    // queue to make the process
    Queue *processing_queue = initQueue();
    Queue *waiting_queue = initQueue();
    key_t key_id;
    int rec_val, msgq_id, status, currtime = getClk();

    key_id = ftok("keyfile", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }

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
                enQueuePriority(ready_queue, pro, 'r');
                write_to_file("memory.log", "At time %d allocated %d bytes for process %d from %d to %d\n",
                              getClk(),
                              pro->memsize,
                              pro->id,
                              start_range,
                              end_range);
                pcount++;
            }
            else
                enQueue(waiting_queue, pro);

            // printf("Process %d recieved\n", pro->id);

            // if (ready_queue->front->id == 1)
            // wasted_time = 0;

            while (msgrcv(msgq_id, &message, sizeof(message.p), 0, IPC_NOWAIT) != -1)
            {

                Process *pro = &message.p;
                if (allocate_memory(pro->id, pro->memsize))
                {
                    get_memory_ranges(pro->id, &start_range, &end_range);
                    // printf("process %d allocated %d bytes\n", pro->id, pro->memsize);
                    enQueuePriority(ready_queue, pro, 'r');
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

        // create or resume process
        if (!isEmpty(ready_queue) && isEmpty(processing_queue))
        {

            Process temp_p = *ready_queue->front;
            deQueue(ready_queue);
            enQueue(processing_queue, &temp_p);

            if (processing_queue->front->pid == 0) // create process
            {

                pid = fork();
                if (pid == 0)
                {

                    // pass remaining time to process
                    char rem_time[3];
                    sprintf(rem_time, "%d", processing_queue->front->remaining_time);
                    char *args[] = {"./process.out", rem_time, NULL};

                    // set params
                    pid = getpid(); // set pid
                    processing_queue->front->status = "started";
                    processing_queue->front->waiting_time = getClk() - processing_queue->front->arrival_time;

                    printf("At time %d process %d %s arr %d total %d remain %d wait %d\n",
                           getClk(),
                           processing_queue->front->id,
                           processing_queue->front->status,
                           processing_queue->front->arrival_time,
                           processing_queue->front->run_time,
                           processing_queue->front->remaining_time,
                           processing_queue->front->waiting_time);

                    write_to_file("scheduler.log", "At time %d process %d %s arr %d total %d remain %d wait %d\n",
                                  getClk(),
                                  processing_queue->front->id,
                                  processing_queue->front->status,
                                  processing_queue->front->arrival_time,
                                  processing_queue->front->run_time,
                                  processing_queue->front->remaining_time,
                                  processing_queue->front->waiting_time);

                    if (execve(args[0], args, NULL) == fail)
                        exit(fail);
                }

                processing_queue->front->pid = pid;
            }
            else
            {
                processing_queue->front->status = "resumed";
                // printf("my waiting time is: %d\n", processing_queue->front->waiting_time);
                processing_queue->front->waiting_time = processing_queue->front->waiting_time + getClk() - processing_queue->front->stop_time;

                printf("At time %d process %d %s arr %d total %d remain %d wait %d\n",
                       getClk(),
                       processing_queue->front->id,
                       processing_queue->front->status,
                       processing_queue->front->arrival_time,
                       processing_queue->front->run_time,
                       processing_queue->front->remaining_time,
                       processing_queue->front->waiting_time);

                write_to_file("scheduler.log", "At time %d process %d %s arr %d total %d remain %d wait %d\n",
                              getClk(),
                              processing_queue->front->id,
                              processing_queue->front->status,
                              processing_queue->front->arrival_time,
                              processing_queue->front->run_time,
                              processing_queue->front->remaining_time,
                              processing_queue->front->waiting_time);

                kill(processing_queue->front->pid, SIGCONT);
            }
        }

        // there is a process that has less time
        if (!isEmpty(ready_queue))
        {
            if (ready_queue->front->remaining_time < processing_queue->front->remaining_time)
            {
                kill(processing_queue->front->pid, SIGTSTP);
                processing_queue->front->status = "stopped";
                processing_queue->front->stop_time = getClk();

                printf("At time %d process %d %s arr %d total %d remain %d wait %d\n",
                       getClk(),
                       processing_queue->front->id,
                       processing_queue->front->status,
                       processing_queue->front->arrival_time,
                       processing_queue->front->run_time,
                       processing_queue->front->remaining_time,
                       processing_queue->front->waiting_time);

                write_to_file("scheduler.log", "At time %d process %d %s arr %d total %d remain %d wait %d\n",
                              getClk(),
                              processing_queue->front->id,
                              processing_queue->front->status,
                              processing_queue->front->arrival_time,
                              processing_queue->front->run_time,
                              processing_queue->front->remaining_time,
                              processing_queue->front->waiting_time);

                // move process back to ready queue
                Process temp = *processing_queue->front;
                deQueue(processing_queue);
                enQueuePriority(ready_queue, &temp, 'r');
            }
        }

        if (getClk() > currtime)
        {
            currtime = getClk();

            if (!isEmpty(processing_queue))
                processing_queue->front->remaining_time--;
        }

        // process finished
        if (!isEmpty(processing_queue) && processing_queue->front->remaining_time == 0)
        {

            processing_queue->front->status = "finished";
            processing_queue->front->remaining_time = 0;
            processing_queue->front->endtime = getClk();
            total_runtime += processing_queue->front->run_time;

            printf("At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
                   getClk(),
                   processing_queue->front->id,
                   processing_queue->front->status,
                   processing_queue->front->arrival_time,
                   processing_queue->front->run_time,
                   processing_queue->front->remaining_time,
                   processing_queue->front->waiting_time,
                   processing_queue->front->endtime - processing_queue->front->arrival_time,
                   ((float)(processing_queue->front->endtime - processing_queue->front->arrival_time) / (float)processing_queue->front->run_time));

            TWTA += (float)(processing_queue->front->endtime - processing_queue->front->arrival_time) / (float)processing_queue->front->run_time;
            TWT += processing_queue->front->waiting_time;

            WTA_values = realloc(WTA_values, (WTA_count + 1) * sizeof(float));
            WTA_values[WTA_count++] = (float)(processing_queue->front->endtime - processing_queue->front->arrival_time) / (float)processing_queue->front->run_time;

            write_to_file("scheduler.log", "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
                          getClk(),
                          processing_queue->front->id,
                          processing_queue->front->status,
                          processing_queue->front->arrival_time,
                          processing_queue->front->run_time,
                          processing_queue->front->remaining_time,
                          processing_queue->front->waiting_time,
                          processing_queue->front->endtime - processing_queue->front->arrival_time,
                          ((float)(processing_queue->front->endtime - processing_queue->front->arrival_time) / (float)processing_queue->front->run_time));

            // Process *temp = processing_queue->front;
            get_memory_ranges(processing_queue->front->id, &start_range, &end_range);
            deallocate_memory(processing_queue->front->id);
            write_to_file("memory.log", "At time %d freed %d bytes for process %d from %d to %d\n",
                          getClk(),
                          processing_queue->front->memsize,
                          processing_queue->front->id,
                          start_range,
                          end_range);
            deQueue(processing_queue);

            if (!isEmpty(waiting_queue))
            {
                // processing_queue->front = waiting_queue->front;
                if (allocate_memory(waiting_queue->front->id, waiting_queue->front->memsize))
                {
                    get_memory_ranges(waiting_queue->front->id, &start_range, &end_range);
                    write_to_file("memory.log", "At time %d allocated %d bytes for process %d from %d to %d\n",
                                  getClk(),
                                  waiting_queue->front->memsize,
                                  waiting_queue->front->id,
                                  start_range,
                                  end_range);
                    // printf("process %d allocated %d bytes\n", waiting_queue->front->id, waiting_queue->front->memsize);
                    enQueuePriority(ready_queue, waiting_queue->front, 'r');
                    deQueue(waiting_queue);
                }
            }
        }

        // process finished
        if (isEmpty(ready_queue) && isEmpty(processing_queue) && isEmpty(waiting_queue))
        {
            // see whether the process generator finished or not
            bool is_finish = down(sem2, false);
            if (is_finish)
            {
                // Average Waiting Time
                AWT = TWT / pcount;

                // Average Weighted Turnaround Time
                AWTA = TWTA / pcount;

                SD = calculateStandardDeviation(WTA_values, WTA_count);

                int total_time = getClk();
                printf("Total Time: %d\n", total_time);
                printf("Total Runtime: %d\n", total_runtime);
                float cpu_utilization = ((float)total_runtime / (float)total_time) * 100;
                // printf("CPU Utilization: %.2f%%\n", cpu_utilization);

                // create scheduler.log
                FILE *file = fopen("scheduler.perf", "w");

                if (file == NULL)
                {
                    perror("Error opening file for append");
                    return;
                }

                // Write text to the file
                fprintf(file, "CPU Utilization: %.2f%%\n", cpu_utilization);
                fprintf(file, "Avg WTA = %.2f\n", AWTA);
                fprintf(file, "Avg Waiting = %.2f\n", AWT);
                fprintf(file, "Std WTA = %.2f\n", SD);

                // Close the file
                fclose(file);

                // let the process generator terminate
                up(sem1, true);
                return;
            }
        }
    }
}
#include "headers.h"

bool process_running = false;

float TWT_HPF = 0.0, TWTA_HPF = 0.0;
float AWTA_HPF, AWT_HPF;
float *WTA_values_HPF;
int WTA_count_HPF = 0;
float SD_HPF;
int total_runtime_HPF = 0;
int pcount_HPF = 0;

void HPF(int sem1, int sem2)
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

    Queue *ready_queue = initQueue();
    Queue *waiting_queue = initQueue();
    key_t key_id;
    int rec_val, msgq_id, status;

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

        // recieving any process
        rec_val = msgrcv(msgq_id, &message, sizeof(message.p), 0, IPC_NOWAIT);
        if (rec_val != -1)
        {
            Process *pro = &message.p;
            if (allocate_memory(pro->id, pro->memsize))
            {
                get_memory_ranges(pro->id, &start_range, &end_range);
                // print_memory_state();
                enQueuePriority(ready_queue, pro, 'p');
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

            while (msgrcv(msgq_id, &message, sizeof(message.p), 0, IPC_NOWAIT) != -1)
            {
                Process *pro = &message.p;
                if (allocate_memory(pro->id, pro->memsize))
                {
                    get_memory_ranges(pro->id, &start_range, &end_range);
                    // print_memory_state();
                    enQueuePriority(ready_queue, pro, 'p');
                    write_to_file("memory.log", "At time %d allocated %d bytes for process %d from %d to %d\n",
                              getClk(),
                              pro->memsize,
                              pro->id,
                              start_range,
                              end_range);
                }
                else
                    enQueue(waiting_queue, pro);
            }
        }

        // create processes
        if (!isEmpty(ready_queue) && !process_running)
        {

            // set waiting time
            ready_queue->front->waiting_time = getClk() - ready_queue->front->arrival_time;

            int pid;
            pid = fork();

            process_running = true;

            // excute process
            if (pid == 0)
            {
                // pass remaining time to process
                char rem_time[3];
                sprintf(rem_time, "%d", ready_queue->front->remaining_time);
                char *args[] = {"./process.out", rem_time, NULL};

                // set params
                ready_queue->front->pid = getpid(); // set pid
                ready_queue->front->status = "started";

                printf("At time %d process %d %s arr %d total %d remain %d wait %d\n",
                       getClk(),
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

                if (execve(args[0], args, NULL) == fail)
                    exit(fail);
            }

            // process ended successfully
            waitpid(ready_queue->front->pid, &status, 0);

            // starting to print
            ready_queue->front->status = "finished";
            ready_queue->front->remaining_time = 0;
            ready_queue->front->endtime = getClk();
            total_runtime_HPF += ready_queue->front->run_time;
            pcount_HPF++;

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

            TWTA_HPF += (float)(ready_queue->front->endtime - ready_queue->front->arrival_time) / (float)ready_queue->front->run_time;
            TWT_HPF += ready_queue->front->waiting_time;

            WTA_values_HPF = realloc(WTA_values_HPF, (WTA_count_HPF + 1) * sizeof(float));
            WTA_values_HPF[WTA_count_HPF++] = (float)(ready_queue->front->endtime - ready_queue->front->arrival_time) / (float)ready_queue->front->run_time;

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

            process_running = false;

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
                    get_memory_ranges(ready_queue->front->id, &start_range, &end_range);
                    enQueuePriority(ready_queue, waiting_queue->front, 'p');
                    deQueue(waiting_queue);
                    write_to_file("memory.log", "At time %d allocated %d bytes for process %d from %d to %d\n",
                        getClk(),
                        ready_queue->front->memsize,
                        ready_queue->front->id,
                        start_range,
                        end_range);
                }
            }

            // recieving any process
            rec_val = msgrcv(msgq_id, &message, sizeof(message.p), 0, IPC_NOWAIT);
            if (rec_val != -1)
            {
                Process *pro = &message.p;
                if (allocate_memory(pro->id, pro->memsize))
                {
                    get_memory_ranges(pro->id, &start_range, &end_range);
                    // print_memory_state();
                    enQueuePriority(ready_queue, pro, 'p');
                    write_to_file("memory.log", "At time %d allocated %d bytes for process %d from %d to %d\n",
                        getClk(),
                        pro->memsize,
                        pro->id,
                        start_range,
                        end_range);
                }
                else
                    enQueue(waiting_queue, pro);

                while (msgrcv(msgq_id, &message, sizeof(message.p), 0, IPC_NOWAIT) != -1)
                {
                    Process *pro = &message.p;
                    if (allocate_memory(pro->id, pro->memsize))
                    {
                        get_memory_ranges(pro->id, &start_range, &end_range);
                        // print_memory_state();

                        write_to_file("memory.log", "At time %d allocated %d bytes for process %d from %d to %d\n",
                            getClk(),
                            pro->memsize,
                            pro->id,
                            start_range,
                            end_range);
                        enQueuePriority(ready_queue, pro, 'p');
                    }
                    else
                        enQueue(waiting_queue, pro);
                }
            }
        }

        // process finished
        if (isEmpty(ready_queue) && isEmpty(waiting_queue))
        {
            // see whether the process generator finished or not
            bool is_finish = down(sem2, false);
            if (is_finish)
            {

                // Average Waiting Time
                AWT_HPF = TWT_HPF / pcount_HPF;

                // Average Weighted Turnaround Time
                AWTA_HPF = TWTA_HPF / pcount_HPF;

                SD_HPF = calculateStandardDeviation(WTA_values_HPF, WTA_count_HPF);

                int total_time_HPF = getClk();
                printf("Total Time: %d\n", total_time_HPF);
                printf("Total Runtime: %d\n", total_runtime_HPF);
                float cpu_utilization_HPF = ((float)total_runtime_HPF / (float)total_time_HPF) * 100;

                // create scheduler.log
                FILE *file = fopen("scheduler.perf", "w");

                if (file == NULL)
                {
                    perror("Error opening file for append");
                    return;
                }

                // Write text to the file
                fprintf(file, "CPU Utilization = %.2f%%\n", cpu_utilization_HPF);
                fprintf(file, "Avg WTA = %.2f\n", AWTA_HPF);
                fprintf(file, "Avg Waiting = %.2f\n", AWT_HPF);
                fprintf(file, "Std WTA = %.2f\n", SD_HPF);

                // Close the file
                fclose(file);
                // let the process generator terminate
                up(sem1, true);
                return;
            }
        }
    }
}
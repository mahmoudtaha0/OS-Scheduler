#include "headers.h"

Process* initProcess()
{
    Process* p = (Process*)malloc(sizeof(Process));

    p->id = -1;
    p->arrival_time = -1;
    p->run_time = -1;
    p->priority = -1;
    p->remaining_time = -1;
    p->pid = 0;
    p->starttime = -1;
    p->endtime = -1;
    p->processed_time = -1;
    p->status = "";
    p->next = NULL;
    p->waiting_time = 0;
    p->stop_time = 0;
    p->memsize = 0;
    return p;

}

Queue* initQueue()
{
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

Process* newNode(Process *pro)
{
    Process* temp = (Process*)malloc(sizeof(Process));

    temp->id = pro->id;
    temp->arrival_time = pro->arrival_time;
    temp->run_time = pro->run_time;
    temp->priority = pro->priority;
    temp->remaining_time = pro->remaining_time;
    temp->pid = pro->pid;
    temp->starttime = pro->starttime;
    temp->endtime = pro->endtime;
    temp->processed_time = pro->processed_time;
    temp->status = pro->status;
    temp->next = pro->next;
    temp->waiting_time = pro->waiting_time;
    temp->stop_time = pro->stop_time;
    temp->memsize = pro->memsize;
    /*add when needed*/

    temp->next = NULL;
    return temp;
}

void enQueue(Queue* q, Process *pro)
{
    Process* temp = newNode(pro);
 
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }

    q->rear->next = temp;
    q->rear = temp;
}

void deQueue(Queue* q)
{
    if (q->front == NULL)
        return;
 
      Process* temp = q->front;
 
    q->front = q->front->next;
 
    if (q->front == NULL)
        q->rear = NULL;
 
    free(temp);
}

int isEmpty(Queue* q)
{
    return q->front == NULL;
}

int getQueueSize(Queue* q)
{
  int size = 0;
  Process* currNode = q->front;
  while (currNode != NULL) {
    size++;
    currNode = currNode->next;
  }
  return size;
}

void enQueuePriority(Queue* q, Process* pro, char p_type)
{
    if (p_type == 'p') // priority
    {
        Process* temp = newNode(pro);

        if (q->rear == NULL) {
            q->front = q->rear = temp;
            return;
        }

        Process* prevNode = NULL;
        Process* currNode = q->front;

        while (currNode != NULL && currNode->priority <= pro->priority) {
            prevNode = currNode;
            currNode = currNode->next;
        }

        if (prevNode == NULL) {
            q->front = temp;
        } else {
            prevNode->next = temp;
        }

        temp->next = currNode;

        if (currNode == NULL) {
            q->rear = temp;
        }
    }
    else if (p_type == 'r') // remaining time
    {
        Process* temp = newNode(pro);

        if (q->rear == NULL) {
            q->front = q->rear = temp;
            return;
        }

        Process* prevNode = NULL;
        Process* currNode = q->front;

        while (currNode != NULL && currNode->remaining_time <= pro->remaining_time) {
            prevNode = currNode;
            currNode = currNode->next;
        }

        if (prevNode == NULL) {
            q->front = temp;
        } else {
            prevNode->next = temp;
        }

        temp->next = currNode;

        if (currNode == NULL) {
            q->rear = temp;
        }
    }
}

Queue *updatePriority(Queue *q, char p_type)
{
    Queue* tempQ = initQueue();
    while(!isEmpty(q))
    {
        Process *p = q->front;
        enQueuePriority(tempQ, p, p_type);
        deQueue(q);
    }

    return (tempQ);
}

void printQueue(Queue* q)
{
	if (isEmpty(q))
	{
		printf("Queue is empty\n");
		return;
	}

	Process* currNode = q->front;

	while (currNode != NULL)
	{
		printf("%d  ", currNode->id);


		currNode = currNode->next;
	}
	printf("\n");
}

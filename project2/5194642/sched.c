#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

typedef struct Process {
    int pid;
    int arrivalTime;
    int burstTime;
    int waitingTime;
    int priority;
    int workLeft;
    struct Process* nextProc;
} Process;

typedef struct Queue {
    struct Process* front;
    struct Process* rear;
} Queue;

void PPsched(FILE*, FILE*, int);
void FCFSsched(FILE*, FILE*, int);
Process* createProcess(int,int,int,int);
Queue* createQueue();
void enqueue(Queue*,Process*);
void dequeue(Queue*);
void priorityInsert(Queue*, Process*);


int main(int argc, char* argv[]) {
    int LIMIT = 0;
    if(argc < 5) {
        LIMIT = -1;
    } else {
        LIMIT = atoi(argv[4]);
    }

     FILE* outputFile = fopen(argv[2], "w");
    if(outputFile == NULL) {
        printf("File cannot be opened...\n");
        printf("Exiting program...\n");
        return 0;
    }

     FILE* inputFile = fopen(argv[1], "r");
    if(inputFile == NULL){
        printf("File cannot be opened...\n");
    } else if(strcmp(argv[3], "FCFS") == 0) {
        FCFSsched(inputFile, outputFile, LIMIT);
        fclose(inputFile);
        fclose(outputFile);
    } else if(strcmp(argv[3], "PP") == 0) {
        PPsched(inputFile, outputFile, LIMIT);
        fclose(inputFile);
        fclose(outputFile);
    }

    return 0;
}

Process* createProcess(int _pid, int _arrivalTime, int _burstTime, int _priority) {
    Process* newProc = malloc(sizeof(Process));
    newProc->pid = _pid;
    newProc->arrivalTime = _arrivalTime;
    newProc->burstTime = _burstTime;
    newProc->priority = _priority;
    newProc->waitingTime = 0;
    newProc->workLeft = _burstTime;
    newProc->nextProc = NULL;

    return newProc;
}

Queue* createQueue() {
    Queue* newQueue = malloc(sizeof(Queue));
    newQueue->front = NULL;
    newQueue->rear = NULL;

    return newQueue;
}

void enqueue(Queue* queue, Process* proc) {
    if(queue->front == NULL) {
        queue->front = proc;
        queue->rear = proc;
    } else {
        queue->rear->nextProc = proc;
        queue->rear = proc;
    }
}

void dequeue(Queue* queue) {
    Process* tempProc = queue->front;
    queue->front = queue->front->nextProc;

    if(tempProc != NULL) {
        free(tempProc);
    }
}

void deleteQueue(Queue* q){
    Process* p = q->front;
    Process* tempP;
    while(p != NULL){
        tempP = p->nextProc;
        free(p);
        p = tempP;
    }
    free(q);
}

void priorityInsert(Queue* queue, Process* proc) {
    Process* temp = createProcess(proc->pid, proc->arrivalTime, proc->burstTime, proc->priority);
    Process* start = queue->front;

    if(queue->front == NULL) {
        queue->front = temp;
        queue->rear = temp;
        return;
    }

    if(start->priority > proc->priority) {
        temp->nextProc = start;
        queue->front = temp;
    } else {
        while(start->nextProc != NULL && start->nextProc->priority <= temp->priority) {
            start = start->nextProc;
        }
        temp->nextProc = start->nextProc;
        start->nextProc = temp;
    } 
}

void PPsched(FILE* in, FILE* out, int LIMIT) {
    int i = LIMIT;
    int clock = 0;
    int pid = 0;
    int arrivalTime = 0;
    int burstTime = 0;
    int priority = 0;

    Process* tempProc;
    Queue* waitQueue = createQueue();
    Queue* readyQueue = createQueue();

    while(fscanf(in, "%d %d %d %d\n", &pid, &arrivalTime, &burstTime, &priority) != EOF && i != 0){
        tempProc = createProcess(pid, arrivalTime, burstTime, priority);
        enqueue(waitQueue, tempProc);
        i--;
    }

    while(waitQueue->front != NULL || readyQueue->front != NULL) {
        if(waitQueue->front != NULL) {
            tempProc = waitQueue->front;
            if(tempProc->arrivalTime == clock) {
                priorityInsert(readyQueue, tempProc);
                dequeue(waitQueue);
            }
        }

        tempProc = readyQueue->front;
        tempProc->workLeft--;
        clock++;
        if(tempProc->workLeft == 0) {
            tempProc->waitingTime = clock - ( tempProc->burstTime + tempProc->arrivalTime );
            fprintf(out, "%d %d %d %d\n", tempProc->pid, tempProc->arrivalTime, clock, tempProc->waitingTime);
            dequeue(readyQueue);
        }
    }
    deleteQueue(waitQueue);
    deleteQueue(readyQueue);
}

void FCFSsched(FILE* in, FILE* out, int LIMIT) {
    int i = LIMIT;
    int timeElapsed = 0;
    int pid = 0;
    int arrivalTime = 0;
    int burstTime = 0;
    int priority = 0;
    Process* tempProc;
    Queue* procQueue = createQueue();

    while(fscanf(in, "%d %d %d %d\n", &pid, &arrivalTime, &burstTime, &priority) != EOF && i != 0){
        tempProc = createProcess(pid, arrivalTime, burstTime, priority);
        enqueue(procQueue, tempProc);
        i--;
    }

    tempProc = procQueue->front;
    while(tempProc != NULL){
        int burst = tempProc->burstTime;
        int arrival = tempProc->arrivalTime;

        timeElapsed += burst;
        tempProc->waitingTime = timeElapsed - ( burst + arrival );

        fprintf(out, "%d %d %d %d\n", tempProc->pid, arrival, timeElapsed, tempProc->waitingTime);

        tempProc = tempProc->nextProc;
        dequeue(procQueue);
    }

    deleteQueue(procQueue);
}



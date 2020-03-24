#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

// Linked list node to hold words
typedef struct Node {
    char* word;
    int count;
    struct Node* nextNode;
} Node;

// Buffer for message queue
typedef struct List {
    struct Node* head;
    struct Node* tail;
} List;

// Struct to hold arguments for thread functions
typedef struct threadArgs {
    int msgid;
    char* filename;
    List* list;
} threadArgs;

void read_msq();
List* createList();
void addToList(List*, Node*);
Node* removeFromList(List*);
Node* createNode(char*);

typedef sem_t Semaphore;
Semaphore mutex;
Semaphore full;
Semaphore empty;

int main(int argc, char* argv[])
{
    FILE *file = fopen(argv[2], "w");
    List *linkedList = createList();

    read_msq(file, linkedList);

    fclose(file);

    return 0;
}

List* createList() {
    List* list = malloc(sizeof(List));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void addToList(List* l, Node* n){
    Node* tempNode = l->head;
    while(tempNode != NULL){
        if(strcmp(tempNode->word, n->word) == 0) {
            tempNode->count++;
            free(n);
            return;
        } else {
            tempNode = tempNode->nextNode;
        }
    }
    tempNode = n;
    if(l->head == NULL) {
        l->head = tempNode;
        l->tail = tempNode;
    } else {
        l->tail->nextNode = tempNode;
        l->tail = tempNode;
    }
}

Node* removeFromList(List* l) {
    Node* tempNode = l->head;
    l->head = l->head->nextNode;
    return tempNode;
}

Node* createNode(char* word) {
    Node* newNode = malloc(sizeof(Node));
    newNode->word = word;
    newNode->nextNode = NULL;
    newNode->count = 1;
    return newNode;
}

//Read message queue
void read_msq(FILE *file, List *list){
    int msqid;
    key_t key;

    //Same key as mapper
    if ((key = ftok("mapper.c", 1) == -1)) {         
        perror("ftok");
        exit(1);
    }

    //connect to queue
    if ((msqid = msgget(key, 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

    for(;;) {
        if (msgrcv(msqid, &list, sizeof list->head, 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        fprintf(file, " %s \n", list->head->word);    
    }
}

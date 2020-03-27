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

void message();
void read_msq();
List* createList();
void addToList(List*, Node*);
Node* removeFromList(List*);
Node* createNode(char*);

typedef struct my_msgbuf {
    long mtype;
    char mtext[200];
} My_msgbuf;

void message()
{
    struct my_msgbuf buf;
    int msqid;
    key_t key;

    if ((key = ftok("mapper.c", 1)) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
    
    printf("Enter lines of text, ^D to quit:\n");

    buf.mtype = 1; /* we don't really care in this case */

    while(fgets(buf.mtext, sizeof buf.mtext, stdin) != NULL) {
        int len = strlen(buf.mtext);

        /* ditch newline at end, if it exists */
        if (buf.mtext[len-1] == '\n') buf.mtext[len-1] = '\0';

        if (msgsnd(msqid, &buf, len+1, 0) == -1) /* +1 for '\0' */
            perror("msgsnd");
    }

    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    FILE *file = fopen(argv[1], "w");
    List *linkedList = createList();

    message();

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
    key = ftok("mapper.c", 1);

    //connect to queue
    msqid = msgget(key, 0666);

    while(msgrcv(msqid, &list, sizeof list->head, 0, 0) == -1) {
        addToList(list, list->head);
        fprintf(file, " %s \n", list->head->word);    
    }
}

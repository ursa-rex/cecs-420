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

// List
typedef struct List {
    struct Node* head;
    struct Node* tail;
} List;

typedef struct Message {
    long type;
    char text[256];
} Message;

List* createList();
Node* createNode(char*);
void append(List*, char*);
void printList(List*, FILE*);
void readMsg(Message*);

int main(int argc, char* argv[])
{
    FILE* file = fopen(argv[1], "w");
    if(file == NULL){
        printf("File cannot be opened....\n");
    }
    List *list = createList();

    Message message;
    int msgid;
    key_t key;

    key = ftok("mapper.c", 1);
    msgid = msgget(key, 0666);

    while(1){
        if(msgrcv(msgid, &message, sizeof(message.text), 1, 0) != -1){
            if(message.type == 1){
                append(list, message.text);
            } else {
                break;
            }
        }
    }
    msgctl(msgid, IPC_RMID, NULL);

    printList(list, file);
    printf("End of msg queue\n");
    fclose(file);

    return 0;
}

List* createList() {
    List* list = malloc(sizeof(List));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void append(List* list, char* msg) {
    char token[] = ":";
    char* word = strtok(msg, token);

    printf("%s\n", msg);

    Node* tempNode = list->head;
    while(tempNode != NULL){
        if(strcmp(tempNode->word, word) == 0) {
            tempNode->count++;
            // printf("%d\n", tempNode->count);
            return;
        } else {
            tempNode = tempNode->nextNode;
        }
    }
    tempNode = createNode(word);
    if(list->head == NULL) {                              // Check for empty list
        list->head = tempNode;
        list->tail = tempNode;
    } else {                                              // Add item to end and rename tail
        list->tail->nextNode = tempNode;
        list->tail = tempNode;
    }
}

void printList(List *list, FILE *file) //prints out list
{
    Node *node = list->head;
    while(node != NULL)
    {
        printf("%s:%d", node->word, node->count);
        fprintf(file, "%s:%d", node->word, node->count); //print list to file until EOF
        node = node->nextNode;
    }
}

Node* createNode(char *newWord) {
    Node* newNode = malloc(sizeof(Node));
    newNode->word = newWord;
    newNode->nextNode = NULL;
    newNode->count = 1;
    return newNode;
}
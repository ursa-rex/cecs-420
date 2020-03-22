#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <semaphore.h>

static int MAXLINESIZE = 1024;
static int MAXWORDSIZE = 256;

// Linked list node to hold words
typedef struct Node {
    char* word;
    int count;
    struct Node* nextNode;
} Node;

// Buffer for message queue
typedef struct Buffer {
    struct Node* head;
    struct Node* tail;
} Buffer;

// Struct to hold arguments for thread functions
typedef struct threadArgs {
    int msgid;
    char* filename;
    Buffer* buffer;
} threadArgs;

typedef sem_t Semaphore;

void runCmd(FILE*);
Buffer* createBuffer();
Node* createNode(char*);
void map(char*);
void* worker(void*);
void* sender(void*);
void addToBuffer(Buffer*, Node*);
Node* removeFromBuffer(Buffer*);

// Declaring semaphores
Semaphore mutex;
Semaphore full;
Semaphore empty;

int main(int argc, char* argv[]) {

    // Initialization
    int BUFFERSIZE = atoi(argv[2]);
    sem_init(&mutex, 0, 1);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFERSIZE);

    // Run commandFile
    FILE* file = fopen(argv[1], "r");
    if(file == NULL){
        printf("File cannot be opened...\n");
    } else {
        runCmd(file);
        fclose(file);
    }

    // Cleanup
    sem_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);
    return 0;
}

// Read commandFile and call map
void runCmd(FILE* cmdFile) {
    int x;
    char* cmd;
    char dir[MAXLINESIZE];
    while(fscanf(cmdFile, "%ms %s", &cmd, dir) != EOF){
        if(strcmp(cmd, "map") == 0) {
            x = fork();
            if( x == 0 ) {
                map(dir);
            }
        }
    }
}

Buffer* createBuffer() {
    Buffer* buff = malloc(sizeof(Buffer));
    buff->head = NULL;
    buff->tail = NULL;
    return buff;
}

Node* createNode(char* word) {
    Node* newNode = malloc(sizeof(Node));
    newNode->word = word;
    newNode->nextNode = NULL;
    newNode->count = 1;
    return newNode;
}

void addToBuffer(Buffer* b, Node* n){
    Node* tempNode = b->head;
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
    if(b->head == NULL) {                          
        b->head = tempNode;
        b->tail = tempNode;
    } else {
        b->tail->nextNode = tempNode;
        b->tail = tempNode;
    }
}

Node* removeFromBuffer(Buffer* b) {
    Node* tempNode = b->head;
    b->head = b->head->nextNode;
    return tempNode;
}

// Thread function to scan nodes from file into buffer
void* worker(void* args) {
    threadArgs* actualArgs = args;
    char word[MAXWORDSIZE];
    FILE* file = fopen(actualArgs->filename, "r");
    if(file == NULL){
        printf("File cannot be opened...\n");
    } else {
        while(fscanf(file, "%s", word) != EOF){
            //create node
            Node* tempNode = createNode(word);

            sem_wait(&empty);
            sem_wait(&mutex);

            //add to buffer
            addToBuffer(actualArgs->buffer, tempNode);

            sem_post(&mutex);
            sem_post(&full);

            free(word);
        }
    }
    fclose(file);
    free(actualArgs);
    return NULL;
}

// Thread function to scan nodes from buffer into message queue
void* sender(void* args) {
    threadArgs* actualArgs = args;
    Node* tempNode;
    do{
        sem_wait(&full);
        sem_wait(&mutex);

        //remove from buffer
        tempNode = removeFromBuffer(actualArgs->buffer);

        sem_post(&mutex);
        sem_post(&empty);

        //send
        msgsnd(actualArgs->msgid, tempNode, sizeof(tempNode), 0);

    } while(actualArgs->buffer->head != NULL);

    return NULL;
}

void map(char* directoryPath) {

    // Initializing message queue, buffer, thread arguements
    key_t key;
    int msgid;
    key = ftok("mapper.c", 1);
    msgid = msgget(key, 0666 | IPC_CREAT); // <-- This id is needed to reference message queue

    DIR* dir = opendir(".");
    struct dirent* entry;
    struct stat filestat;
    Buffer* buff = createBuffer();
    threadArgs* args = malloc(sizeof(threadArgs));
    args->msgid = msgid;
    args->buffer = buff;
    args->filename = NULL;

    // Thread creation
    if(dir == NULL) {
        printf("Could not open current directory");
        return;
    } else {
        while((entry = readdir(dir)) != NULL) {
            stat(entry->d_name, &filestat);
            if(!S_ISDIR(filestat.st_mode)) {
                args->filename = entry->d_name;
                pthread_t thread;
                pthread_create(&thread, NULL, worker, args);
            }
        }
        pthread_t senderThread;
        pthread_create(&senderThread, NULL, sender, args);
    }
    free(args);
    closedir(dir);
}

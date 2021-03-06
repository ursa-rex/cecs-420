#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
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

typedef struct Message {
    long type;
    char text[256];
} Message;

// Struct to hold arguments for thread functions
typedef struct threadArgs {
    char* filename;
} threadArgs;

typedef sem_t Semaphore;

void runCmd(FILE*);
Buffer* createBuffer();
void destroyBuffer(Buffer*);
Node* createNode(char*);
void map(char*,int);
void* worker(void*);
void sender(int);
void addToBuffer(Buffer*, char*);
void removeFromBuffer(Buffer*);
Message writeMessage(char*,int,int);

// Global initialization
Semaphore mutex;
Semaphore full;
Semaphore empty;
Buffer* buffer;
int threads = 0;
int children = 0;
int status = 0;

int main(int argc, char* argv[]) {

    // Initialization
    int BUFFERSIZE = atoi(argv[2]);
    sem_init(&mutex, 0, 1);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFERSIZE);

    // Run commandFile
    FILE* file = fopen(argv[1], "r");
    if(file == NULL){
        printf("File cannot be opened....\n");
    } else {
        runCmd(file);
    }

    // Cleanup
    sem_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);
    return 0;
}

// Read commandFile and call map
void runCmd(FILE* cmdFile) {
    int cpid;
    char* cmd;
    char dir[MAXLINESIZE];
    char flagMsg[MAXWORDSIZE];
    Message flag;

    // Initializing message queue, buffer, thread arguements
    key_t key;
    int msgid;
    key = ftok("mapper.c", 1);
    msgid = msgget(key, 0666 | IPC_CREAT); // <-- This id is needed to reference message queue

    while(fscanf(cmdFile, "%ms %s", &cmd, dir) != EOF){
        if(strcmp(cmd, "map") == 0) {
            cpid = fork();
            if( cpid == 0 ) {
                children++;
                map(dir, msgid);
            }
        }
    }
    fclose(cmdFile);
    
    while(status != children);
    strcpy(flagMsg, "/DONE/");
    flag = writeMessage(flagMsg, -1, 0);
    msgsnd(msgid, &flag, sizeof(char[128]), 0);
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

void addToBuffer(Buffer* b, char* w){
    Node* tempNode = createNode(w);
    if(b->head == NULL) {                          
        b->head = tempNode;
        b->tail = tempNode;
    } else {
        b->tail->nextNode = tempNode;
        b->tail = tempNode;
    }
}

void removeFromBuffer(Buffer* b) {
    Node* tempNode = b->head;
    b->head = b->head->nextNode;
    
    if(tempNode != NULL) {
        free(tempNode);
    }
}

void destroyBuffer(Buffer* b){
    // Node* node = b->head;
    // Node* tempNode;
    // while(node != NULL){
    //     tempNode = node->nextNode;
    //     free(node->word);
    //     free(node);
    //     node = tempNode;
    // }
    free(b);
}

Message writeMessage(char* word, int count, int type){
    char strCount[3];
    snprintf(strCount, 10, ":%d", count);
    char* text = strcat(word, strCount);

    Message message;
    strcpy(message.text, text);
    message.type = type;

    return message;
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
            sem_wait(&empty);
            sem_wait(&mutex);

            //add to buffer
            addToBuffer(buffer, word);
            printf("\t%s\n", buffer->tail->word);

            sem_post(&mutex);
            sem_post(&full);
        }
    }
    fclose(file);
    // free(actualArgs);
    threads--;
    return NULL;
}

// Thread function to scan nodes from buffer into message queue
void sender(int msgid) {
    Node* tempNode;
    Message msg;
    do{
        sem_wait(&full);
        sem_wait(&mutex);

        // remove from buffer
        tempNode = buffer->head;
        msg = writeMessage(tempNode->word, tempNode->count, 1);
        removeFromBuffer(buffer);

        sem_post(&mutex);
        sem_post(&empty);

        // send
        // printf("%s\n", msg.text);
        msgsnd(msgid, &msg, 256, 0);
    } while(buffer->head != NULL || threads > 0);
}

void map(char* directoryPath, int msgid) {
    DIR* dir = opendir(directoryPath);
    struct dirent* entry;
    buffer = createBuffer();

    // Thread creation
    if(dir == NULL) {
        printf("Could not open current directory");
        return;
    } else {
        while((entry = readdir(dir)) != NULL) {
            if(entry->d_type != DT_DIR) {
                threadArgs* args = malloc(sizeof(threadArgs));
                args->filename = NULL;
                args->filename = malloc(sizeof(char) * (strlen(directoryPath) + strlen(entry->d_name) + 2));
                args->filename = strcat(strcat(strcpy(args->filename, directoryPath), "/"), entry->d_name);
                pthread_t thread;
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                // pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                pthread_create(&thread, &attr, worker, args);
                threads++;
            }
        }
        // free(entry);
        // free(args);
        sender(msgid);
    }
    closedir(dir);
    destroyBuffer(buffer);
    status++;
    exit(0);
}

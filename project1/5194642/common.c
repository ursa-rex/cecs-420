#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node { // List node
    char* data;
    int count;
    struct Node* next;
    struct Node* prev;
} Node;

typedef struct List { // Doubly linked list
    struct Node* head;
    struct Node* tail;
    
} List;

void printList(List*, FILE*);
Node* createNode(char*);
void appendWord(List*, char*);
void deleteList(List*);
void insertionSort(List*);
List* createListFromFile(FILE*);
List* mergeSortedLists(List*, List*);

int main(int argc, char* argv[]) {
    List* list1;
    List* list2;
    List* list3;

    FILE* file = fopen(argv[1], "r");
    if(file == NULL){
        printf("File cannot be opened...\n");
    } else {
        list1 = createListFromFile(file);
        fclose(file);
    }

    file = fopen(argv[2], "r"); 
    if(file == NULL){
        printf("File cannot be opened...\n");
    } else {
        list2 = createListFromFile(file);
        fclose(file);
    }

    file = fopen(argv[3], "w");
    if(file == NULL){
        printf("File cannot be opened...\n");
    } else {
        list3 = mergeSortedLists(list1, list2);
        printList(list3, file);
        fclose(file);
    }
    deleteList(list1);
    deleteList(list2);
    deleteList(list3);
    
    return 0;
}

void printList(List* list, FILE* outputFile){
    Node* node = list->head;
    while(node != NULL ) {
        fprintf(outputFile, "%s,%d\n", node->data, node->count);
        node = node->next;
    }
}

Node* createNode(char* word) {
    Node* newNode = malloc(sizeof(Node));       // Create new node to be appended to list
    newNode->data = word;
    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->count = 1;
    return newNode;
}

void appendWord(List* list, char* word) {
    Node* tempNode = list->head;
    while(tempNode != NULL){
        if(strcmp(tempNode->data, word) == 0) {
            tempNode->count++;
            // printf("%d\n", tempNode->count);
            free(word);
            return;
        } else {
            tempNode = tempNode->next;
        }
    }
    tempNode = createNode(word);
        if(list->head == NULL) {                              // Check for empty list
            list->head = tempNode;
            list->tail = tempNode;
        } else {                                              // Add item to end and rename tail
            list->tail->next = tempNode;
            tempNode->prev = list->tail;
            list->tail = tempNode;
        }
}

void deleteList(List* list){
    Node* node = list->head;
    Node* tempNode;
    while(node != NULL){
        tempNode = node->next;
        free(node->data);
        free(node);
        node = tempNode;
    }
    free(list);
}

void insertionSort(List* list) {
    Node* node;
    Node* tempNode;
    char* tempData;
    int tempCount;
    node = list->head->next;

    while(node != NULL) {
        tempNode = node;
        while(tempNode->prev != NULL) {
            if(strcmp(tempNode->data, tempNode->prev->data) < 0) {
                tempData = tempNode->data;
                tempCount = tempNode->count;
                tempNode->data = tempNode->prev->data;
                tempNode->count = tempNode->prev->count;
                tempNode->prev->data = tempData;
                tempNode->prev->count = tempCount;
            }
            tempNode = tempNode->prev;
        }
        node = node->next;
    }
};

List* createListFromFile(FILE* file) {

    List* list = malloc(sizeof(List));
    list->head = NULL;
    list->tail = NULL;
    
    char* word = NULL;
    while(fscanf(file, "%ms", &word) != EOF){
        appendWord(list, strdup(word));
        free(word);
    }
    insertionSort(list);
    return list;
}

List* mergeSortedLists(List* a, List* b) {
    List* outputList = malloc(sizeof(List));
    outputList->head = NULL;
    outputList->tail = NULL;

    Node* node1 = a->head;
    Node* node2 = b->head;

    while(node1 != NULL && node2 != NULL){
        if(strcmp(node1->data, node2->data) > 0) {
            node2 = node2->next;
        } else if(strcmp(node1->data, node2->data) < 0) {
            node1 = node1->next;
        } else if(strcmp(node1->data, node2->data) == 0) {
            appendWord(outputList, strdup(node1->data));
            outputList->tail->count = node1->count + node2->count;
            node1 = node1->next;
            node2 = node2->next;
        }
    }
    return outputList;
}

// TODO
/*
    -free memory
    -make function to destroy lists
    -output to file
*/
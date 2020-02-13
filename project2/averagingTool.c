#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

void getAverages(FILE*);

int main(int argc, char* argv[]) {
    FILE* inputFile = fopen(argv[1], "r");
    
    getAverages(inputFile);
    fclose(inputFile);
    
    return 0;
}

void getAverages(FILE* file) {
    int pid = 0;
    int arrivalTime = 0;
    int totalArrivalTime = 0;
    int finishTime = 0;
    int totalFinishTime = 0;
    int waitTime = 0;
    int totalWaitTime = 0;
    int avgWaitTime = 0;
    int avgTurnAroundTime = 0;
    int lineCount = 0;

    while(fscanf(file, "%d %d %d %d\n", &pid, &arrivalTime, &finishTime, &waitTime) != EOF){
        lineCount++;
        totalArrivalTime += arrivalTime;
        totalFinishTime += finishTime;
        totalWaitTime += waitTime;
        printf("waitTime: %d\n", waitTime);
    }

    avgWaitTime = totalWaitTime / lineCount;
    avgTurnAroundTime = ( totalFinishTime - totalArrivalTime ) / lineCount;

    printf("Average Wait Time: %d\n", avgWaitTime);
    printf("Average Turn Around Time: %d\n", avgTurnAroundTime);
}
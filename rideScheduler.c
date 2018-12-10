#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "random437.h"
#include <semaphore.h>
#include <time.h>

static sem_t timeStepSem;
static sem_t* carSem;
static sem_t lineSem;
pthread_mutex_t mutex;

unsigned int MAXWAITPEOPLE = 800;
unsigned int MAXPERCAR;
unsigned int CARNUM;
unsigned int waitTimes[600];
unsigned int lineLengths[600];
unsigned int totalArrived, totalRiders, totalRejected;
struct timespec tim, tim2;


struct RideData 
{ 
   unsigned int timeStep, arrived, rejected, waitingInLine; 
} data;

void printMinuteStatus(struct RideData rd){
    unsigned int hours = 9 + rd.timeStep/60;
    unsigned int minutes = rd.timeStep%60; 
    fprintf(stdout,"%i arrive %i reject %i wait-line %i at %02d:%02d:00\n", rd.timeStep, rd.arrived, rd.rejected, rd.waitingInLine, hours, minutes);
    fflush(stdout);
    char string[100];
    sprintf(string,"echo \"%d, %d, %d, %d\" >> data%d%d.csv", rd.timeStep, rd.arrived, rd.rejected, rd.waitingInLine, MAXPERCAR, CARNUM);
    system(string);

}
unsigned int calculateTotalRejected(){
    return totalRejected + lineLengths[599];
}
unsigned int calculatePeopleArrived(unsigned int timeStep){
    if(timeStep < 120) return poissonRandom(25);
    else if (timeStep < 300) return poissonRandom(45);
    else if (timeStep < 420) return poissonRandom(35);
    else return poissonRandom(25);

}
unsigned int calculateLongestLine(unsigned int l[]){
    unsigned int length = 600;
    unsigned int max = 0;
    for(unsigned int i =0; i<length; i++){
        if(l[i]>max) max=l[i];
    }
    return max;
}
unsigned int calculateAverageWait(unsigned int waits[]){
    unsigned int length = 600;
    unsigned int total;
    for(unsigned int i =0; i<length; i++){
        total+=waits[i];
    }
    return total/length;
}
unsigned int calculateWaitTimes(struct RideData rd){
    waitTimes[rd.timeStep] = rd.waitingInLine/(MAXPERCAR * CARNUM);
}
void printTotalStats(struct RideData rd){
    unsigned int averageWaitTime = calculateAverageWait(waitTimes);
    unsigned int longestLine = calculateLongestLine(lineLengths);
    unsigned int totalRejected = calculateTotalRejected();
    printf("Total Arrived: %d\n", totalArrived);
    printf("Total Riders: %d\n", totalRiders);
    printf("Longest Line: %d\n", longestLine);
    printf("Total People Leaving: %d\n", totalRejected);
    printf("Average wait: %d minutes\n", averageWaitTime);
    fflush(stdout);
    
}
void * runCarThread(unsigned int i){
    while(1) {
        sem_wait(&carSem[i]);
        pthread_mutex_lock(&mutex);
        unsigned int loadedPassengers;
        if(MAXPERCAR<data.waitingInLine)loadedPassengers = MAXPERCAR;
        else loadedPassengers = data.waitingInLine;
        data.waitingInLine = data.waitingInLine - loadedPassengers;
        totalRiders += loadedPassengers;
        pthread_mutex_unlock(&mutex);
     }
    
}
void * runLineThread(){
    while(1) {
        sem_wait(&lineSem);
        pthread_mutex_lock(&mutex);
        data.arrived = calculatePeopleArrived(data.timeStep);
        totalArrived += data.arrived;
        unsigned int rejected = 0;
        if(data.waitingInLine + data.arrived > MAXWAITPEOPLE){
            rejected = data.waitingInLine + data.arrived - MAXWAITPEOPLE;
        }
        data.rejected += rejected;
        data.waitingInLine = data.waitingInLine + data.arrived - data.rejected;

        totalRejected += rejected;
        calculateWaitTimes(data);
        lineLengths[data.timeStep] = data.waitingInLine;
        pthread_mutex_unlock(&mutex);
        nanosleep(&tim, &tim2);
        sem_post(&timeStepSem);
     }
    
}

unsigned int main(unsigned int argc, char const **argv)
{
    char string[100];
    data.timeStep = 0;
    data.arrived = 0;
    data.rejected = 0;
    data.waitingInLine = 0;
    MAXPERCAR =0;
    CARNUM = 0;
    tim.tv_sec = 1;
    tim.tv_nsec = 10000;
    unsigned int index;
    unsigned int opterr = 0;
    unsigned int c;
    pthread_t lineThread;
    pthread_t* carThread;
    while ((c = getopt(argc, argv, "M:N:")) != -1)
    {
        switch (c)
        {
        case 'M':
            MAXPERCAR = atoi(optarg);
            break;
        case 'N':
            CARNUM = atoi(optarg);
            break;
        case '?':
            if (optopt == 'M' || optopt == 'N')
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
            return 1;
        default:
            abort();
        }
    }
    if( !MAXPERCAR || !CARNUM) 
    {
        fprintf(stderr, "Invalid values for MAXPERCAR and CARNUM.");
        exit(1);
    }
    sprintf(string,"echo \"timeStep, arrived, rejected, waitingInLine\" > data%d%d.csv",MAXPERCAR, CARNUM);
    system(string);
    carThread = malloc(sizeof(pthread_t)*CARNUM);
    carSem = malloc(sizeof(sem_t)*CARNUM);
    pthread_mutex_init(&mutex, NULL);
    sem_init(&timeStepSem,0,1);
    for (unsigned int i=0; i<CARNUM; i++){
        sem_init(&carSem[i],0,1);
    }
    sem_init(&lineSem,0,1);
    
    for(unsigned int i; i < CARNUM; i++){
        pthread_create(&carThread[i], NULL, &runCarThread, i);
    }
    pthread_create(&lineThread, NULL, &runLineThread, NULL);
    while(data.timeStep<600){
        for (unsigned int i=0; i<CARNUM; i++){
            sem_post(&carSem[i]);
        }
        sem_post(&lineSem);
        sem_wait(&timeStepSem);
        printMinuteStatus(data);
        data.rejected = 0;
        data.timeStep++;
    }
    pthread_kill(&lineThread, 1);
    for(unsigned int i; i < CARNUM; i++){
        pthread_kill(&carThread[i], 1);
    }
    printTotalStats(data);

}

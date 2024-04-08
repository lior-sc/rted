#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 10
#define WAIT_TIME_MS 1000 // 1 second

#define READERS_NUM 6
#define WRITERS_NUM 2

pthread_mutex_t mutex;
atomic_int counter;
int thread_id[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

bool wthread_running[WRITERS_NUM] = {false};
bool rthread_running[READERS_NUM] = {false};

static __uint8_t task1_buffer[100];

/** @brief
 * This function is used to start the read operation safely
 * @param data: pointer to a data buffer to store the current data from the shared resource
*/
int StartRead(__uint8_t *data){
    // lock the mutex
    int ret = pthread_mutex_lock(&mutex); 
    // if lock acheived, copy the data from the shared resource to the data buffer
    if(ret == 0){
        memcpy(data, task1_buffer, BUFFER_SIZE);
        atomic_fetch_add(&counter, 1);
        pthread_mutex_unlock(&mutex); // unlock the mutex
        return ret;
    }
    // unlock the mutex
    pthread_mutex_unlock(&mutex); // unlock the mutex
    return ret;


    return ret;
}

void EndRead(){
    atomic_fetch_sub(&counter, 1);
}

int StartWrite(){
    return pthread_mutex_lock(&mutex); // lock the mutex
}

void EndWrite(){
    pthread_mutex_unlock(&mutex);
}

void *writeThread(void *arg){
    __uint8_t data[BUFFER_SIZE];
    int *flag_index = (int*)arg;
    
    while(rthread_running[*flag_index] == false){
        sleep(1);
        printf("Waiting for the read thread %d to start\n", *flag_index);
    }

    while(wthread_running[*flag_index] == true){
        time_t t0 = {0};
        t0 = clock();
        for (int i = 0; i < BUFFER_SIZE; i++) {
            data[i] = rand() % 256;
        }

        // Write thread logic here
        int ret = StartWrite();
        if(ret == 0 || ret == EDEADLK){
            memcpy(task1_buffer, data, BUFFER_SIZE);
            EndWrite();
            printf("thread[%d] wrote: ", *flag_index);
            for (int i = 0; i < BUFFER_SIZE; i++) {
                printf("%d ", task1_buffer[i]);
            }
            printf("\033[65Greaders attached: %d\n", atomic_load(&counter));
            usleep(100e3);
        }

    }

    printf("Thread[%d] is exiting\n", *flag_index);

    return NULL;
}

void *readThread(void *arg){
    __uint8_t data[BUFFER_SIZE];
    int *flag_index = (int *)arg;

    while(wthread_running[*flag_index] == false){
        // do nothing. wait for the write thread to start
        sleep(1);
    }
    

    while(wthread_running[*flag_index] == true){
        __uint8_t data[BUFFER_SIZE];
        time_t t0 = {0};

        // get the current time
        t0 = clock();

        // Write thread logic heres
        int ret = StartRead(data);
        
        // Print the data written to the shared resource
        printf("thread[%d] read: ", *flag_index);
        for (int i = 0; i < BUFFER_SIZE; i++) {
            printf("%d ", data[i]);
        }
        printf("\033[65Greaders attached: %d\n", atomic_load(&counter));

        usleep(300e3);
        if(ret == 0){
            EndRead();}
        usleep(700e3);
    }
    
    printf("Read Thread[%d] is exiting.\n", *flag_index);

    return NULL;
}

int main(){

    pthread_t wthread[READERS_NUM];
    pthread_t rthread[WRITERS_NUM];
    srand(time(NULL));

    // create write threads
    for (int i = 0; i < WRITERS_NUM; i++) {
        // create the write thread
        if (pthread_create(&wthread[i], NULL, writeThread, (void*)&thread_id[i])) {
            printf("Failed to create write thread %d\n", i);
            return -1;
        }
        printf("WThread[%d] created\n", i);
    }

    // create read threads
    for (int i = 0; i < READERS_NUM; i++) {
        rthread_running[i] = true;
        if (pthread_create(&rthread[i], NULL, readThread, (void*)&thread_id[i])) {
            printf("Failed to create read thread %d\n", i);
            return -1;
        }
        printf("RThread[%d] created\n", i);
    }

    // initialize the threads
    for(int i=0; i<WRITERS_NUM; i++){
        wthread_running[i] = true;
    }

    for(int i=0; i<READERS_NUM; i++){
        rthread_running[i] = true;
    }

    // Wait for the threads to finish
    // start the threads

    for(int i=0; i<WRITERS_NUM; i++){
        pthread_join(wthread[i], NULL);
    }

    for(int i=0; i<READERS_NUM; i++){
        pthread_join(rthread[i], NULL);
    }


    counter = 0;
    return 0;
}






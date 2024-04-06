#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>

#define BUFFER_SIZE 10
#define WAIT_TIME_MS 1000 // 1 second

pthread_mutex_t mutex;
atomic_int counter;

bool wthread_running[4] = {false};
bool rthread_running[4] = {false};

static __uint8_t task1_buffer[100];

/** @brief
 * This function is used to start the read operation safely
 * @param data: pointer to a data buffer to store the current data from the shared resource
*/
int StartRead(__uint8_t *data){
    int ret_val = EBUSY; // default value is busy to avoid false positive response
    clock_t start = 0;
    const double timeout_ticks = WAIT_TIME_MS * CLOCKS_PER_SEC / 1000; // 1 second

    start = clock();
    while((double)(clock() - start) < timeout_ticks){
        ret_val = pthread_mutex_trylock(&mutex); // try to lock the mutex
        if(ret_val == 0){
            memcpy(data, task1_buffer, sizeof(task1_buffer));
            pthread_mutex_unlock(&mutex);
            break;
        }
    }
    if(ret_val == 0){
        atomic_fetch_add(&counter, 1);
    }
    else{
        printf("Error: %d - %s\n", ret_val, strerror(ret_val));
    }

    return ret_val;
}

void EndRead(){
    atomic_fetch_sub(&counter, 1);
}

int StartWrite(){
    int ret_val = EBUSY; // default value is busy to avoid false positive response
    clock_t start = 0;
    const double timeout_ticks = WAIT_TIME_MS * CLOCKS_PER_SEC / 1000; // 1 second

    start = clock();
    while((double)(clock() - start) < timeout_ticks && ret_val != 0){
         // this loop is used to wait for the mutex to be unlocked or the timeout to be reached
        ret_val = pthread_mutex_trylock(&mutex); // try to lock the mutex
    }

    // inform the user via termminal print about the status of the start write operation
    if(ret_val != 0){
        printf("Failed to unlock mutex. Error: %d - %s\n", ret_val, strerror(ret_val));
    }

    return ret_val;
}

void EndWrite(){
    pthread_mutex_unlock(&mutex);
}

void *writeThread(void *arg){
    __uint8_t data[BUFFER_SIZE];
    int flag_index = (int)arg;
    

    while(wthread_running[flag_index] == true){
        time_t t0 = {0};
        t0 = clock();
        if(StartWrite() != 0){
            printf("thread[%d] Failed to start write operation\n",flag_index);
            continue;
        }

        // Write random data to the shared resource
        for (int i = 0; i < BUFFER_SIZE; i++) {
            task1_buffer[i] = rand() % 256;
        }
        
        // Print the data written to the shared resource
        printf("thread[%d] wrote: ", flag_index);
        for (int i = 0; i < BUFFER_SIZE; i++) {
            printf("%d ", task1_buffer[i]);
        }
        printf("\033[65Greaders attached: %d\n", counter);

        EndWrite();
        
        while((double)(clock() - t0) < 10e-3 * CLOCKS_PER_SEC){
            // wait for 1ms
        }

    }

    printf("Thread[%d] is exiting\n", flag_index);

    return NULL;
}


void *readThread(void *arg){
    while(1);
    // Read thread logic here
    return NULL;
}

int main(){
    pthread_t wthread[3];
    pthread_t rthread[3];
    srand(time(NULL));

    // Initialize and create write threads
    for (int i = 0; i < 2; i++) {
        wthread_running[i] = true;
        if (pthread_create(&wthread[i], NULL, writeThread, (void*)i) != 0) {
            printf("Failed to create write thread %d\n", i);
            return -1;
        }
        printf("WThread[%d] created\n", i);
    }

    // Initialize and create read threads
    for (int i = 0; i < 3; i++) {
        rthread_running[i] = true;
        if (pthread_create(&rthread[i], NULL, readThread, (void*)i) != 0) {
            printf("Failed to create read thread %d\n", i);
            return -1;
        }
        printf("RThread[%d] created\n", i);
    }

    // Wait for the threads to finish
    

    for(int i=0; i<2; i++){
        pthread_join(wthread[i], NULL);
    }

    for(int i=0; i<3; i++){
        pthread_join(rthread[i], NULL);
    }


    counter = 0;
    return 0;
}






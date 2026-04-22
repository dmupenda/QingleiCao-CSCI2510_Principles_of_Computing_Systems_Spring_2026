#include <stdio.h>
#include <pthread.h>

int shared_counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* adder(void* arg) {
    pthread_mutex_t* mtx = (pthread_mutex_t*) arg;

    pthread_mutex_lock(mtx);//lock once

    for (int i=0; i<20000000; i++)  {
	shared_counter++;
    }
    pthread_mutex_unlock(mtx); //unlock once
   
    return NULL;
}

void* subtractor(void* arg){
    pthread_mutex_t* mtx = (pthread_mutex_t*) arg;
    
    pthread_mutex_lock(mtx);

    for (int i=0; i<20000000; i++) {
	shared_counter--;
    }
    pthread_mutex_unlock(mtx); 
    
    return NULL;
}

int main() {
    pthread_t t1, t2;

    pthread_create(&t1, NULL, adder, &mutex);
    pthread_create(&t2, NULL, subtractor, &mutex);
   
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
   
    printf("Final counter value: %d\n", shared_counter);
    return 0;
}



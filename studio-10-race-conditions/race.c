#include <stdio.h>
#include <pthread.h>

int race = 0;

void* adder(void* arg) {
    for (int i=0; i<2000000; i++){
	race++;
    }
    return NULL;
}

void* subtractor(void* arg){
    for (int i=0; i<2000000; i++){
	race--;
    }
    return NULL;
}

int main(){
    printf("===Sequential execution ===\n");
    race = 0;
    adder(NULL);
    subtractor(NULL);
    printf("After sequential adder then subtractor: race = %d\n", race);
   
    printf("\n=== Concurrent execution with threads ===\n");
    race =0;
    pthread_t t1, t2;

    //create both threads before waiting for either
    pthread_create(&t1, NULL, adder, NULL);
    pthread_create(&t2, NULL, subtractor, NULL);

    //Wait for both threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL); 

    printf("After concurrent adder/subtractor: race = %d\n", race);   
  
    return 0;
}




#include <stdio.h>
#include <pthread.h>



int shared_counter = 0;

void* adder(void* arg) {
    for (int i = 0; i < 20000000; i++) {
        shared_counter++;
    }
    return NULL;
}

void* subtractor(void* arg) {
    for (int i = 0; i < 20000000; i++) {
        shared_counter--;
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, adder, NULL);
    pthread_create(&t2, NULL, subtractor, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Final counter value: %d\n", shared_counter);
    return 0;
}

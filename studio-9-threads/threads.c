#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct thread_args {
    int arg1;
    char arg2[100];
    int ret;
};

//thread entry function
void* thread_entry(void* args){
    struct thread_args* arg_ptr = (struct thread_args*) args;
	
    printf("Thread %d received: arg1=%d, arg2=\"%s\"\n",
           arg_ptr->arg1, arg_ptr->arg1, arg_ptr->arg2);
    
    arg_ptr->ret = arg_ptr->arg1 * 10; //write a return value into the struct 
    
    return NULL;
}

int main() {
    int num_threads = 5;
    pthread_t thread_ids[num_threads];
    struct thread_args args[num_threads];
    int result; 

    for (int i = 0; i < num_threads; i++) {
	args[i].arg1 = i;
	snprintf(args[i].arg2, 100, "This is thread %d", i);
	
	result = pthread_create(&thread_ids[i], NULL, thread_entry, &args[i]);
	if (result != 0) {
	    fprintf(stderr, "Error creating thread %d (code %d)\n", i, result);
	    return 1;
	}
    }


    //joing (wait for) all N threads
    for (int i = 0; i < num_threads; i++){
	pthread_join(thread_ids[i], NULL);
	printf("Main: thread %d returned ret=%d\n", i, args[i].ret);

    } 

    return 0; 
}



#include <stdio.h> 
#include <string.h>
#include <unistd.h>   //for execvp

int main() { 
    char buffer[100];
    int max_args = 15;
    int max_argv_size = max_args + 2;
    char *cmd;
    char *my_argv[max_argv_size];

    printf("Enter command: ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL){
	
	buffer[strcspn(buffer, "\n")] = '\0'; 

	cmd = strtok(buffer, " ");
	if (cmd == NULL) {
	    printf("No command entered.\n");
	    return 1;
	}

	//Building argv
	my_argv[0] = cmd; 
	int i = 1;
	char *token = strtok(NULL, " ");
	while (token != NULL && i < max_argv_size -1){
	   my_argv[i] = token;
	   i++;
	   token = strtok(NULL, " ");
        }
        my_argv[i] = NULL; 

        execvp(cmd, my_argv);

        perror("execvp failed");
	return 1;

    }
    return 0; 
}

       


 

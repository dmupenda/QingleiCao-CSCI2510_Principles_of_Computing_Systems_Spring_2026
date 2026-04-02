
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pid_t pid1, pid2;

    //Create the pipe
    if (pipe(fd) == -1){
	perror("pipe");
	exit(EXIT_FAILURE);
    }
  
    //First Child: program1
    if ((pid1= fork()) == 0) {
	
	close(fd[0]);
	
	dup2(fd[1], STDOUT_FILENO);
	close(fd[1]); 

	char *cmd = "./program1";
	char *args[] = {"program1", NULL};
	execvp(cmd, args);
	perror("execvp program1");
	exit(EXIT_FAILURE);
    }

    //Second child: Program2
    if ((pid2 = fork()) == 0) {
	
	close(fd[1]);

	dup2(fd[0], STDIN_FILENO);
	close(fd[0]);

        char *cmd = "./program2";
        char *args[] = {"program2", NULL};
        execvp(cmd, args);
        perror("execvp program2");
        exit(EXIT_FAILURE);
    }

    //parent closes both
    close(fd[0]);
    close(fd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}






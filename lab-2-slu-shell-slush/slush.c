#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_LINE   256
#define MAX_ARGS    16
#define MAX_PROGS   16

volatile sig_atomic_t got_sigint = 0;

void handle_sigint(int sig) {
    (void)sig;
    got_sigint = 1;
}

void print_prompt(void) {
    char cwd[1024];
    char *home = getenv("HOME");

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("slush> ");
        return;
    }

    if (home != NULL && strncmp(cwd, home, strlen(home)) == 0) {
        char *rel = cwd + strlen(home);
        if (*rel == '/') rel++;
        if (*rel == '\0')
            printf("slush|~> ");
        else
            printf("slush|%s> ", rel);
    } else {
        printf("slush|%s> ", cwd);
    }
    fflush(stdout);
}

void run_pipeline(char *progs[][MAX_ARGS], int n, int in_fd) {
    if (n == 0) return;

    int cur = n - 1;

    if (cur == 0) {
        pid_t pid = fork();
        if (pid == 0) {
            if (in_fd != 0) {
                dup2(in_fd, 0);
                close(in_fd);
            }
            execvp(progs[0][0], progs[0]);
            perror(progs[0][0]);
            exit(1);
        } else if (pid > 0) {
            int status;
            while (wait(&status) == -1 && errno == EINTR);
            if (in_fd != 0) close(in_fd);
        } else {
            perror("fork");
            if (in_fd != 0) close(in_fd);
        }
    } else {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return;
        }

        pid_t pid = fork();
        if (pid == 0) {
            close(pipefd[0]);
            if (in_fd != 0) dup2(in_fd, 0);
            dup2(pipefd[1], 1);
            close(pipefd[1]);
            execvp(progs[cur][0], progs[cur]);
            perror(progs[cur][0]);
            exit(1);
        } else if (pid > 0) {
            close(pipefd[1]);
            run_pipeline(progs, cur, pipefd[0]);
            close(pipefd[0]);
            while (wait(NULL) == -1 && errno == EINTR);
        } else {
            perror("fork");
            close(pipefd[0]);
            close(pipefd[1]);
        }
    }
}

int main(void) {
    char line[MAX_LINE];
    char *tokens[MAX_ARGS * MAX_PROGS];
    char *progs[MAX_PROGS][MAX_ARGS];
    int ntokens, prog_count, arg_count;

    signal(SIGINT, handle_sigint);

    while (1) {
        if (got_sigint) {
            got_sigint = 0;
            printf("\n");
        }

        print_prompt();

        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (errno == EINTR) {
                clearerr(stdin);
                got_sigint = 0;
                continue;
            }
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0')
            continue;

        ntokens = 0;
        char *tok = strtok(line, " \t");
        while (tok != NULL && ntokens < MAX_ARGS * MAX_PROGS - 1) {
            tokens[ntokens++] = tok;
            tok = strtok(NULL, " \t");
        }

        prog_count = 0;
        arg_count = 0;
        int syntax_error = 0;

        for (int i = 0; i <= ntokens; i++) {
            if (i == ntokens || (i < ntokens && strcmp(tokens[i], "(") == 0)) {
                if (arg_count == 0 && prog_count > 0) {
                    fprintf(stderr, "Invalid null command\n");
                    syntax_error = 1;
                    break;
                }
                if (arg_count > 0) {
                    progs[prog_count][arg_count] = NULL;
                    prog_count++;
                    arg_count = 0;
                }
            } else {
                if (arg_count >= MAX_ARGS - 1) {
                    fprintf(stderr, "Too many arguments (max %d)\n", MAX_ARGS-1);
                    syntax_error = 1;
                    break;
                }
                progs[prog_count][arg_count++] = tokens[i];
            }
        }

        if (syntax_error || prog_count == 0)
            continue;

        if (prog_count == 1 && strchr(line, '(') == NULL) {
            if (strcmp(progs[0][0], "cd") == 0) {
                if (progs[0][1] == NULL)
                    fprintf(stderr, "cd: missing argument\n");
                else if (chdir(progs[0][1]) != 0)
                    perror("cd");
                continue;
            }
        }

        run_pipeline(progs, prog_count, 0);
    }

    return 0;
}

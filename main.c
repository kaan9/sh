//headers
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//macros
#define ERR "invalid: "

int pid = 0;  //global var for pid

//do nothing if ctrl + c
void sigint_handler(int signo) {
}

//kill process after alarm runs out, print message
void sigalrm_handler(int signo) {
    if (signo == SIGALRM) {
        write(STDOUT_FILENO, "Bwahaha ... tonight I dine on turtle soup\n", 43);
        kill(pid, SIGALRM);
    }
}

int main(int argc, char * argv[]) {
    //initialize variables
    int time;
    char buffer[1024];
    char * cmd;
    char * p;
    long val;
    int temp;
    int status;
    char * input1[] = {"", NULL};
    char * input2[] = {NULL};

    //read input to ./penn-shredder, assign input to time
    if ((argc > 2) || (argc == 1)) {
        time = 0;  //default
        write(STDOUT_FILENO, "invalid: too many/few inputs \n", 31);
    } else {
        val = strtol(argv[1], &p, 10);
        //check if input is valid (non-negative integer)
        if (*p == '\0') {
            time = (int)val;
            if (time < 0) {
                time = 0;
            }
        } else {
            time = 0;
        }
    }

    //reassign signal handlers
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror(ERR);
        kill(pid, SIGINT);
    }
    if (signal(SIGALRM, sigalrm_handler) == SIG_ERR) {
        perror(ERR);
        kill(pid, SIGALRM);
    }

    while (1) {
        //write prompt, read input
        write(STDOUT_FILENO, "penn-shredder# ", 15);
        temp = read(STDIN_FILENO, buffer, 1024);

        //exit on EOF, else fork and run child process
        if (temp == 0) {
            exit(0);
        } else {
            pid = fork();
            if (!pid) {
                //child process
                cmd = strtok(buffer, " \r\t\n");
                execve(cmd, input1, input2);
                perror(ERR);
                exit(0);
            } else {
                //parent process
                alarm(time);
                wait(&status);
                alarm(0);
            }
        }
        //clearing the buffer
        memset(buffer, 0, 1024);
    }
    return 0;
}

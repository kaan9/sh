/* penn-sh   ---  Kaan B Erdogmus,Belinda Liu,  CIS 380, kaanberk*/
#include <errno.h>   //for errno
#include <limits.h>  //for INT_MAX, INT_MIN (for portability)
#include <signal.h>
#include <stdio.h>   //for perror
#include <stdlib.h>  //for exit
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//functions for input/output, defines RDLEN, TOKMAX, PROCMAX
#include "ioutil.h"
#include "strutil.h"

pid_t cpid;

//do nothing if ^C is received
void siginthandler(int signum) {}

//executes processes, returns 0 on success, 1 on invalid input, -1 if fork fails
int exec_procs(PROC_LIST * proc_list) {
    if (!proc_list && proc_list->procc) return 1;
    //NOTE: Currently only executes first process

    pid_t pid = fork();
    if (pid < 0) {
        perror("Invalid fork");
        return -1;
    }
    if (!pid) {
        execvp(proc_list->procs[0][0], proc_list->procs[0]);
        //if execvp returns, it has failed and will print no such file or directory
        prints("No such file or directory\n");
        exit(EXIT_FAILURE);
    } else {
        int status = 0;
        cpid       = pid;
        wait(&status);
    }
    return 0;
}

int main(int argc, const char ** argv) {
    signal(SIGINT, siginthandler);

    //buffer for reading a line
    char buf[RDLEN];
    int tokc = 0;           //number of tokens
    char * tokens[TOKMAX];  //tokens parsed from input, only tokens contains allocated memory in main
    PROC_LIST proc_list;    //processes filtered from tokens

    for (;;) {
        prints("penn-sh# ");
        if ((!readln(buf) && (endl(), 1)) || streq(buf, "exit")) break;  //break if the readln receives an EOF or "exit"

        free_str_array(tokens, tokc);  //free previous allocations before having tokens point to new memory

        if (!(tokc = tokenize_input(buf, tokens))) continue;  // if no lines entered, skip execution

        if (tokc == TOKMAX) free(tokens[tokc--]);  // edge case, must not have last token
        tokens[tokc] = 0;                          //tokens should be null terminated

        if (!(parse_tokens(tokc, tokens, &proc_list))) {  //no valid process given, skip execution
            prints("Invalid: No such file or directory\n");
            continue;
        }

        if (exec_procs(&proc_list) == -1) {
            free_str_array(tokens, tokc);
            exit(EXIT_FAILURE);
        }
    }

    free_str_array(tokens, tokc);
    exit(EXIT_SUCCESS);
}

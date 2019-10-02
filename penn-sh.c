/* penn-sh   ---  Kaan B Erdogmus,Belinda Liu,  CIS 380, kaanberk*/
#include <errno.h>   //for errno
#include <limits.h>  //for INT_MAX, INT_MIN (for portability)
#include <signal.h>
#include <stdio.h>   //for perror
#include <stdlib.h>  //for exit
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

//functions for input/output, defines RDLEN, TOKMAX, PROCMAX
#include "ioutil.h"
#include "strutil.h"

#define FD int //to differentiate between regular integers and file descriptors

// to maintain the default input and output as STDOUT_FILENO/STDIN_FILENO get overriden with multiple processes
FD STDOUT_DFLT;
FD STDIN_DFLT;

pid_t cpid;

//do nothing if SIGINT is received
void siginthandler(int signum) {}

//executes processes, returns 0 on success, 1 on invalid input, -1 if fork fails, 2 if I/O fails
int exec_procs(PROC_LIST * proc_list) {
    if (!proc_list && proc_list->procc) return 1;

    //NOTE: Currently only executes first process with redirection

    FD out_file, in_file; //for the temporary input output

    if (proc_list->input_redirect && (in_file = open(proc_list->input_redirect, O_RDONLY, 0644)) < 0) {
        perror("Invalid: Unable to open input file");
        return 2;
    }
    if (proc_list->output_redirect && (out_file = open(proc_list->output_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
        perror("Invalid: Unable to open input file");
        close(in_file);
        return 2;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Invalid fork");
        return -1;
    }
    if (!pid) {
        //set stdio and execute
        dup2(out_file ? out_file : STDOUT_DFLT,STDOUT_FILENO);
        dup2(in_file ? in_file : STDIN_DFLT, STDIN_FILENO);
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

    //set the default I/O
    if (dup2(STDOUT_FILENO, STDOUT_DFLT) == -1 || dup2(STDIN_FILENO, STDIN_DFLT) == -1) {
        perror("Invalid: Critical I/O error");
        exit(EXIT_FAILURE);
    }

    char buf[RDLEN];        //buffer for reading a line
    int tokc = 0;           //number of tokens
    char * tokens[TOKMAX];  //tokens parsed from input, only this contains allocated memory in main
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

        int xc_res = exec_procs(&proc_list);
        if (xc_res == -1) {
            free_str_array(tokens, tokc);
            exit(EXIT_FAILURE);
        } else if (xc_res == 1) {
            prints("Invalid: No such file or directory\n");
            continue;
        }
    }

    free_str_array(tokens, tokc);
    exit(EXIT_SUCCESS);
}

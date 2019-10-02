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

int main(int argc, const char ** argv) {
    signal(SIGINT, siginthandler);

    //buffer for reading a line
    char buf[RDLEN];
    int tokc = 0;           //number of tokens
    char * tokens[TOKMAX];  //tokens parsed from input, only tokens contains allocated memory in main
    int procc = 0;          //number of processes
    PROC_LIST proc_list;    //processes filtered from tokens

    for (;;) {
        prints("penn-sh# ");
        if ((!readln(buf) && (endl(), 1)) || streq(buf, "exit")) break;  //break if the readln receives an EOF or "exit"

        free_str_array(tokens, tokc);  //free previous allocations before having tokens point to new memory

        tokc = tokenize_input(buf, tokens);
        if (!tokc) continue;  // if no lines entered, skip execution

        if (tokc == TOKMAX) free(tokens[tokc--]);  // edge case, must not have last token
        tokens[tokc] = 0;                          //tokens should be null terminated

        procc = parse_tokens(tokc, tokens, &proc_list);
        if (!procc) {  //no valid process given, skip execution
            prints("Invalid: No such file or directory\n");
            continue;
        }

        
        pid_t pid = fork();
        if (pid < 0) {
            perror("Invalid fork");
            free_str_array(tokens, tokc);
            exit(EXIT_FAILURE);
        }
        if (!pid) {
            execvp(tokens[0], tokens);
            //if execvp returns, it has failed and will print no such file or directory
            prints("No such file or directory\n");
            exit(EXIT_FAILURE);
        } else {
            int status = 0;
            cpid       = pid;
            wait(&status);
        }
    }

    free_str_array(tokens, tokc);
    exit(EXIT_SUCCESS);
}

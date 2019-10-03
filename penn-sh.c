/* penn-sh   ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/
#include <errno.h>  //for errno
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>   //for perror
#include <stdlib.h>  //for exit
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "executil.h"  //process execution utility functions
#include "ioutil.h"    //functions for input/output, defines RDLEN, TOKMAX, PROCMAX
#include "strutil.h"   //string utlity functions

//do nothing if SIGINT is received
void siginthandler(int signum) {}

int main(int argc, const char ** argv) {
    signal(SIGINT, siginthandler);

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
        switch (xc_res) {
            case -1:
                free_str_array(tokens, tokc);
                exit(EXIT_FAILURE);
            case 1:
                prints("Invalid: No such file or directory\n");
        }
    }

    free_str_array(tokens, tokc);
    exit(EXIT_SUCCESS);
}

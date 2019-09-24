/* penn-sh   ---  Kaan B Erdogmus,Belinda Liu,  CIS 380, kaanberk*/
#include <errno.h>   //for errno
#include <limits.h>  //for INT_MAX, INT_MIN (for portability)
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>   //for perror
#include <stdlib.h>  //for exit
#include <unistd.h>

//functions for input/output, defines RDLEN, TOKMAX, PROCMAX
#include "ioutil.h" 
#include "strutil.h"

pid_t cpid;

//do nothing if ^C is received
void siginthandler (int signum) {}

int main(int argc, const char ** argv) {
    signal(SIGINT, siginthandler);
   
    //buffer for reading a line
    char buf[RDLEN];
    int tokc = 0; //number of tokens
    char * tokens[TOKMAX]; //tokens parsed from input, only tokens in main contains allocated memory
    int procc = 0; //number of processes
    struct proc procs[PROCMAX]; //processes filtered from tokens

    while (1) {
        prints("penn-sh# ");
        if (!readln(buf) || streq(buf, "exit")) endl(), break; //exit if the readln receives an EOF or the input is "exit"
        
	free_str_array(tokens, tokc);
        tokc = tokenize_input(buf, tokens);
        tokens[tokc] = 0;
        if (!tokc) continue; // if no lines entered, skip execution
        
        procc = parse_tokens(tokc, tokens, procs);
        if (!procc) { //no valid process given, skip execution
             prints("No such file or directory\n");
             continue;
        }


        pid_t pid = fork();
        if (!pid) {
            execvp(tokens[0], tokens);
            //if execvp returns, it has failed and will print no such file or directory
            prints("No such file or directory\n");
            exit(EXIT_FAILURE);
        } else {
            int status = 0;
            cpid = pid;
            wait(&status);
        }
    }
    
    free_str_array(tokens, tokc);
    exit(EXIT_SUCCESS);
}

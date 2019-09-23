/* ioutil.c   --- Implementation of ioutil.h --- Kaan B Erdogmus, CIS 380, kaanberk*/

#include <errno.h>   //for errno
#include <limits.h>  //for INT_MAX, INT_MIN (for portability)
#include <stdio.h>   //for perror
#include <stdlib.h>  //for exit
#include <unistd.h>

#include "strutil.h"  //Extra Credit: string utility functions (strlen, atoi), string.h not used

#include "ioutil.h"

int check_args(int argc, const char ** argv) {
    int timeout = -1;  //default value of -1 indicates no timeout

    if (argc == ARGC) {
        timeout = atoi(argv[1]);
        if (timeout <= 0 || timeout == INT_MAX) {
            errno = EINVAL;
            perror("Invalid: optional parameter must be positive integer between 0 and INT_MAX");
            timeout = -1;
        }
    } else if (argc > ARGC) {
        errno = EINVAL;
        perror("Invalid:    USAGE: penn-shredder <timeout>");
        exit(EXIT_FAILURE);
    }
    return timeout;
}

void printc(char c) {
    if (write(STDOUT_FILENO, &c, 1) == -1) {
        perror("write to stdout failed");
        exit(EXIT_FAILURE);
    }
}

void prints(const char * s) {
    while (*s) printc(*s++);
}

void printu(unsigned long n) {
    if (n == 0) return printc('0');  //edge case

    int tp = 10;  //largest power of 10 representable as int
    while (10 * (long)tp < INT_MAX) tp *= 10;
    while (n / tp == 0 && tp != 0) tp /= 10;
    while (tp != 0) {
        printc((n / tp) + '0');
        n %= tp;
        tp /= 10;
    }
    if (n) printc(n + '0');
}

void printi(int n) {
    if (n < 0) {
        printc('-');
        printu(-((long)n));  //accounts for n = INT_MIN edge case
    } else {
        printu(n);
    }
}

void endl() {
    printc('\n');
}

int getchar() {
    char c;
    if (read(STDIN_FILENO, &c, 1) == -1) {
        perror("read from stdin failed");
        exit(EXIT_FAILURE);
    }
    return c;
}

void flush() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int readln(char * buf) {
    int len = read(STDIN_FILENO, buf, RDLEN);
    if (!len) return 0;
    if (buf[len - 1] == EOF) {
        buf[0] = 0;
        return 1;
    }
    if (buf[len - 1] != EOF && buf[len - 1] != '\n') flush();
    buf[len - 1] = 0;
    return len;
}

int tokenize_input(char * buf, char ** argv) {
    int tok_c = 0;  //count of current token
    while (tok_c < TOKMAX) {
        while (isspace(*buf)) *buf++ = 0;
        if (!*buf) return tok_c;  //no tokens
        argv[tok_c++] = buf;
        while (!isspace(*buf) && *buf) buf++;
    }
    *buf = 0;
    return tok_c;
}

/* [r]edirect, [p]ipeline, [b]ackground check
 * returns 1 if s is "<", 2 if ">", 3 if "|", 4 if "&", 0 otherwise
 */
int is_rpb(const char * s) {
    if(streq(s, "<")) return 1;
    if(streq(s, ">")) return 2;
    if(streq(s, "|")) return 3;
    if(streq(s, "&")) return 4;
    return 0;
}

/* checks if process has "&" in its arguments, modifies p->type
 * and alters args by changing "&" to NULL pointer 
 * if there are arguments after &, error
 * returns 0 if 
 */
int parse_backg_proc(struct proc * p) {
    return 0;
}

int parse_tokens(int tokc, char ** tokens, struct proc * procs) {
    if(is_rpb(tokens[0])) return 0;
    int procc = 0;

    //iterate over all tokens and separate them into processes, delimited by '|'
    //change '|' to NULL when encountered so args is NULL-terminated
    for (int i = 0; i < tokc && procc < PROCMAX; i++, procc++) {
        if (is_rpb(tokens[i])) return 0; //if the character after a '|' is an rbp, invalid syntax
        procs[procc].args = tokens + i;
        while (streq(tokens[i], "|")  && i < tokc) i++;
        if (streq(tokens[i], "|")) tokens[i] = NULL;
    } 

    //iterate over all processes and extract the '<', '>' arguments
    //replace '<' and '>' with NULL when encountered so args is NULL-terminated
    //and  only refers to the process call and arguments and not to redirections
    for (int p = 0; p < procc; p++) {
        parse_backg_proc(procs + p);

    }

    return procc;
}



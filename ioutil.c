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
    while ((c = getchar()) != '\n' && c != EOF)
        ;
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

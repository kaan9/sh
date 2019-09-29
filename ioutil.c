/* ioutil.c   --- Implementation of ioutil.h --- Kaan B Erdogmus, CIS 380, kaanberk*/

#include <errno.h>   //for errno
#include <limits.h>  //for INT_MAX, INT_MIN (for portability)
#include <stdio.h>   //for perror
#include <stdlib.h>  //for exit
#include <unistd.h>

#include "strutil.h"
#include "tokenizer.h"

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

//prints newline, retuns 0 on success
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
    int tok_c             = 0;     //count of current token
    char * tok            = NULL;  //temp token
    TOKENIZER * tokenizer = init_tokenizer(buf);
    if (!tokenizer) return 0;  //error in creating tokenizer

    while (tok_c < TOKMAX && (tok = get_next_token(tokenizer))) argv[tok_c++] = tok;

    free_tokenizer(tokenizer);

    return tok_c;
}

void free_str_array(char ** argv, size_t len) {
    for (size_t i = 0; i < len; i++) {
        free(argv[i]);
    }
}

/* [r]edirect, [p]ipeline, [b]ackground check
 * returns 1 if s is "<", 2 if ">", 3 if "|", 4 if "&", 0 otherwise
 */
int is_rpb(const char * s) {
    if (streq(s, "<")) return 1;
    if (streq(s, ">")) return 2;
    if (streq(s, "|")) return 3;
    if (streq(s, "&")) return 4;
    return 0;
}

int parse_tokens(int tokc, char ** tokens, PROC_LIST * proc_list) {
    PROC_LIST procl;  //temporary instance of PROC_LIST, to be copied into proc_list if parsing successful

    //handle background terminating background process
    if (streq(tokens[tokc - 1], "&")) {
        procl.is_background = 1;
        free(tokens[tokc - 1]);
        tokens[tokc - 1] = NULL;
    } else
        procl.is_background = 0;

    //loop over all entries and check for errors
    //effectively, any consecutive special characters or the first or last string being special chars is an error
    //everything else is (syntactically) valid
    int is_rpb_char = 1;  //is special character
    for (int i = 0; i < tokc - 1; i++) {
        if (is_rpb_char && (is_rpb_char = is_rpb(tokens[i]))) return 0;
    }
    if (tokens[tokc - 1] && is_rpb(tokens[tokc - 1])) return 0;

    int procc        = 0;
    char ** curr_tok = tokens;
    while (procc < PROCMAX && curr_tok < tokens + tokc) {
        procc++;
        break;
    }

    return procc;
}

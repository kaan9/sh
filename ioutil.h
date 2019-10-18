/* ioutil.h   --- Utility functions for I/O --- Kaan B Erdogmus, CIS 380, kaanberk*/
#ifndef IOUTIL_H
#define IOUTIL_H

#define ARGC 2       //expected maximum value of argc
#define RDLEN 1024   //length of input to be read (including terminating character which becomes '\0')
#define TOKMAX 512   //maximum number of tokens, this is currently safe with RDLEN = 1024
#define PROCMAX 256  //maximum number of processes generated directly from tokens, this is currently safe with RDLEN = 1024

//if functions of the same signature are defined in a header, their implementations are typically in the form of macros
//this ensures that the correct implementation is as is in this file
#undef getchar
#undef printc
#undef prints
#undef printu
#undef printi
#undef endl
#undef flush
#undef readln

typedef struct {
    char buf[RDLEN];

    char * tokens[TOKMAX];  // tokens parsed from input, only this contains allocated memory

    char ** procs[PROCMAX];  //pointer to the argv of each process to be executed (argv[0] is proc name), proc[i] is pipelined to proc[i+1]
    char * input_redirect;   // if NULL, read from stdin
    char * output_redirect;  // if NULL, write to stdout

    int procc;           // number of processes
    int tokc;            // number of tokens in tokens[]
    char is_background;  //1 if running the process in the background, 0 otherwise

} PROC_LIST;

typedef enum {
    JOB  = 0,  // valid job to be executed
    EXIT = 1,  // EOF or "exit"
    SKIP = 2,  // no lines entered, skip execution
    JOBS = 3,  // requesting list of jobs
    FG   = 4,  // foreground
    BG   = 5,  // background
    FAIL = -1  // parsing failed
} INPUT_T;

/* prints a character */
void printc(char c);

/*prints a 0-terminated string*/
void prints(const char * s);

/* prints an unsigned integer between 0 and INT_MAX + 1 inclusive
 * this is a utility function for printi and as such takes a long argument to account for printi(INT_MIN)
 * unspecified behavior for inputs larger than INT_MAX + 1
 */
void printu(unsigned long n);

/* prints a signed integer*/
void printi(int n);

/* prints endline*/
void endl();

/* reads a character from stdin */
int getchar();

/* flushes input stream */
void flush();

/**
 * reads the input line, tokenizes the input and parses it into proc_list if valid job
 * returns an INPUT_T based on the validity and type of input
*/
INPUT_T proc_list_from_input(PROC_LIST * proc_list, int * proc_id);

/**
 * frees the allocated memory in a proc_list instance
 */
void delete_proc_list(PROC_LIST * proc_list);

/**
 * prints the entire job in a proc_list without the &
 */
void print_job(PROC_LIST * proc_list);

#endif

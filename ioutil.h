/* ioutil.h   --- Utility functions for I/O --- Kaan B Erdogmus, CIS 380, kaanberk*/
#ifndef IOUTIL
#define IOUTIL

#define ARGC 2   //expected maximum value of argc
#define RDLEN 1024  //length of input to be read (including terminating character which becomes '\0')
#define TOKMAX 512 //maximum number of tokens, this is currently safe with RDLEN = 1024
#define PROCMAX 256 //maximum number of processes generated directly from tokens, this is currently sade with RDLEN = 1024

#undef getchar
#undef printc
#undef prints
#undef printu
#undef printi
#undef endl
#undef flush
#undef readln


//struct to represent processes after the line is parsed
//filename pointed to by out/in is where proc reads and writes from
//if null, reads and writes from stdin/stdout
struct proc {
    char ** args; //name of proc is args[0], args is 0-terminated
    union {
        char * fin; //read from file, NULL indicates read from stdin
        struct proc * pin; //read from process (pipe)
    }
    union {
        char * fout; //write to file, NULL indicates write to stdout
	    struct proc * pout; //write to process (pipe)
    }
    //determines which members of the union to use
    //00b is fin, fout; 01b is fin, pout; 10b is pin, fout; 11b is pin, pout
    char type;
};

/* checks the arguments passed to main and parses the value of timeout */
int check_args(int argc, const char ** argv);

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

/* prints endline */
void endl();

/* reads a character from stdin */
int getchar();

/* flushes input stream */
void flush();

/** reads a line of NULL-terminated input into buf 
 * reads at most RDLEN characters
 * returns length of read input
 * returns -1 if first char is EOF
 */
int readln(char * buf);

/* tokenizes a null terminated string buf of tokens delimited by whitespace
 * makes at most TOKMAX tokens and points to them with argv
 * in-place, modifies buf by overwriting spaces with 0
 * buf must be 0-terminated
 * returns number of tokens
 */
int tokenize_input(char * buf, char ** argv);

/* parses the tokens into processes
 * makes at most PROCMAX processes
 */
int parse_tokens(int tokc, char ** tokens, struct proc * procs);

#endif

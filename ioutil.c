/* ioutil.c   --- Implementation of ioutil.h --- Kaan B Erdogmus, CIS 380, kaanberk*/
#include "ioutil.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tokenizer.h"

/* checks the arguments passed to main and parses the value of timeout */
int check_args(int argc, const char** argv)
{
	int timeout = -1; //default value of -1 indicates no timeout

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

static void flush()
{
	int c;
	while ((c = getchar()) != '\n' && c != EOF)
		;
}

/** reads a line of NULL-terminated input into buf 
 * reads at most RDLEN characters
 * returns length of read input
 * returns -1 if first char is EOF
 */
int readln(char* buf)
{
	int len = read(STDIN_FILENO, buf, RDLEN);
	if (!len)
		return 0;
	if (buf[len - 1] == EOF) {
		buf[0] = 0;
		return 1;
	}
	if (buf[len - 1] != EOF && buf[len - 1] != '\n')
		flush();
	buf[len - 1] = 0;
	return len;
}

/** tokenizes a null terminated string buf of tokens delimiting
 * whitespace and separating '&', '|', '<', '>' tokens
 * makes at most TOKMAX tokens and points to them with argv
 * not in-place, each string in argv is malloc'd and must be freed
 * buf must be 0-terminated
 * returns number of tokens
 */
int tokenize_input(char* buf, char** argv)
{
	int tok_c = 0; //count of current token
	char* tok = NULL; //temp token
	TOKENIZER* tokenizer = init_tokenizer(buf);
	if (!tokenizer)
		return 0; //error in creating tokenizer

	while (tok_c < TOKMAX && (tok = get_next_token(tokenizer)))
		argv[tok_c++] = tok;

	free_tokenizer(tokenizer);

	return tok_c;
}

/**
 * frees an array of strings of length len
 */
void free_str_array(char** argv, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		free(argv[i]);
	}
}

/**
 * utility function for fg/bg parsing
 * returns true if s points to a NULL-terminated integer string, false otherwise
 */
int is_p_int(char* s)
{
	if (!*s)
		return 0;
	while (*s) {
		if (!(*s >= '0' && *s++ <= '9'))
			return 0;
	}
	return 1;
}

/* parses the tokens into an instance of proc_list
 * fills the array of processes pointed to by procs
 * fills at most PROCMAX processes
 * in-place, modifies proc_list, does not alloc
 * if an error occurs may return proc_list with corrupt data
 * returns number of processes parsed or 0 if a parsing error occurred
 */
int parse_tokens(int tokc, char** tokens, PROC_LIST* proc_list)
{
	int procc = 0; //number of processes

	//handle background terminating background process
	if (!strcmp(tokens[tokc - 1], "&")) {
		proc_list->is_background = 1;
		free(tokens[tokc - 1]);
		tokens[--tokc] = NULL;
	} else {
		proc_list->is_background = 0;
	}

	//count number of entries
	if (!strcmp(tokens[0], "|") || !strcmp(tokens[tokc - 1], "|"))
		return 0;

	//distribute the processes in the tokens into proc_list, delimited by "|"
	proc_list->procs[procc++] = tokens;
	for (int i = 0; i < tokc - 1; i++)
		if (!strcmp(tokens[i], "&"))
			return 0; //invalid char
		else if (!strcmp(tokens[i], "|")) {
			//check for invalid consecutive characters
			if (!strcmp(tokens[i + 1], "|") ||
			    !strcmp(tokens[i + 1], "<") ||
			    !strcmp(tokens[i + 1], ">"))
				return 0;

			proc_list->procs[procc++] =
				tokens + i +
				1; // get the next process into proc_list
			free(tokens[i]);
			tokens[i] =
				NULL; // free and set to NULL the "|" tokens so that each argv in proc_list->procs is NULL-terminated
		}

	proc_list->procc = procc;

	char** argv; //temporary for a process

	//check first process for invalid token
	if (!strcmp(proc_list->procs[0][0], "<") ||
	    !strcmp(proc_list->procs[0][0], ">") ||
	    !strcmp(proc_list->procs[procc - 1][0], "<") ||
	    !strcmp(proc_list->procs[procc - 1][0], ">"))
		return 0;

	//iterate over all processes except the first and the last and check for invalid "<", ">"
	for (int i = 1; i < procc - 1; i++) {
		argv = proc_list->procs[i]; // arguments of ith process
		while (*argv) {
			if (!strcmp(*argv, "<") || !strcmp(*argv, ">"))
				return 0;
			argv++;
		}
	}

	//check the first process for input redirection, also account for output redirection if procc = 1
	argv = proc_list->procs[0];
	proc_list->input_redirect = NULL;
	proc_list->output_redirect = NULL;
	if (!*argv)
		return 0;
	do {
		if (!strcmp(*argv, "<")) {
			// if input redirect and the next character is invalid, fail
			if (!*(argv + 1) || !strcmp(*(argv + 1), "<") ||
			    !strcmp(*(argv + 1), ">"))
				return 0;
			//if there is already a value input_redirect, there's a duplicate "<" so quit
			if (proc_list->input_redirect)
				return 0;
			proc_list->input_redirect = *(argv + 1);
			free(*argv);
			*argv = NULL;
		} else if (!strcmp(*argv, ">")) {
			if (procc == 1) {
				// if output redirect and the next character is invalid, fail
				if (!*(argv + 1) || !strcmp(*(argv + 1), "<") ||
				    !strcmp(*(argv + 1), ">"))
					return 0;
				//if there is already a value output_redirect, there's a duplicate ">" so quit
				if (proc_list->output_redirect)
					return 0;
				proc_list->output_redirect = *(argv + 1);
				free(*argv);
				*argv = NULL;
			} else
				return 0; //invalid input
		}
	} while (*++argv);

	if (procc == 1)
		return procc; //if there is only one process, parsing is complete

	//check the last process for (possibly duplicate) output redirection, also check for invalid input redirection
	argv = proc_list->procs[procc - 1];
	if (!*argv)
		return 0;
	do {
		if (!strcmp(*argv, "<"))
			return 0; //invalid char
		else if (!strcmp(*argv, ">")) {
			//if there is already a value output_redirect, there's a duplicate ">" so quit
			if (proc_list->output_redirect)
				return 0;
			// if output redirect and the next character is invalid, fail
			if (!*(argv + 1) || !strcmp(*(argv + 1), "<") ||
			    !strcmp(*(argv + 1), ">"))
				return 0;
			proc_list->output_redirect = *(argv + 1);
			free(*argv);
			*argv = NULL;
		}
	} while (*++argv);

	return procc;
}

INPUT_T proc_list_from_input(PROC_LIST* proc_list, int* proc_id)
{
	char buf[RDLEN];

	delete_proc_list(proc_list);
	*proc_id = -1;

	// print a newline if readline is EOF
	if ((!(proc_list->strlen = readln(buf)) && (endl(), 1)) ||
	    !strcmp(buf, "exit"))
		return EXIT;

	if (!strcmp(buf, "jobs"))
		return JOBS;

	// store the buf in proc_list for later transfer to the job's name
	// store upto NULL or & if it exists
	int i;
	for (i = 0; i < proc_list->strlen; i++) {
		proc_list->buf[i] = buf[i];
	}
	buf[i] = 0;

	if (!(proc_list->tokc = tokenize_input(buf, proc_list->tokens)))
		return SKIP; // if no lines entered, skip execution

	if (proc_list->tokc == TOKMAX)
		free(proc_list->tokens[--(
			proc_list->tokc)]); // edge case, must not have last token
	proc_list->tokens[proc_list->tokc] =
		0; //tokens should be null terminated

	//fg and bg expect an argument of the form "fg %x" or "fg x" where x is a positive integer

	if (!strcmp(proc_list->tokens[0], "fg")) {
		if (proc_list->tokc != 2)
			return FAIL;
		if (is_p_int(proc_list->tokens[1]) ||
		    (proc_list->tokens[1][0] == '%' &&
		     is_p_int(proc_list->tokens[1] + 1))) {
			*proc_id = atoi(proc_list->tokens[1][0] == '%' ?
						proc_list->tokens[1] + 1 :
						proc_list->tokens[1]);
			return FG;
		} else {
			return FAIL;
		}
	}

	if (!strcmp(proc_list->tokens[0], "bg")) {
		if (proc_list->tokc != 2)
			return FAIL;
		if (is_p_int(proc_list->tokens[1]) ||
		    (proc_list->tokens[1][0] == '%' &&
		     is_p_int(proc_list->tokens[1] + 1))) {
			*proc_id = atoi(proc_list->tokens[1][0] == '%' ?
						proc_list->tokens[1] + 1 :
						proc_list->tokens[1]);
			return BG;
		} else {
			return FAIL;
		}
	}

	if (!parse_tokens(proc_list->tokc, proc_list->tokens, proc_list))
		return FAIL; //no valid process given, skip execution

	return VJOB;
}

void delete_proc_list(PROC_LIST* proc_list)
{
	free_str_array(proc_list->tokens, proc_list->tokc);
	proc_list->procc = 0;
	proc_list->tokc = 0;
}

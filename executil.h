/* executil.h   --- Utility functions for executing processes ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/

#include "ioutil.h" //for PROC_LIST

#define FD int  //to differentiate between regular integers and file descriptors

// to maintain the default input and output as STDOUT_FILENO/STDIN_FILENO get overriden with multiple processes
FD STDOUT_DFLT;
FD STDIN_DFLT;

//save the standard I/O descriptors in the global STDOUT_DFLT, STDIN_DFLT
void save_stdio();

//restore the standard I/O descriptors from the global STDOUT_DFLT, STDIN_DFLT
void restore_stdio();

//executes processes, returns 0 on success, 1 on invalid input, -1 for a critical fork/exec failure, 2 if I/O fails
int exec_procs(PROC_LIST * proc_list);
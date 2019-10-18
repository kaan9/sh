/* executil.h   --- Utility functions for executing processes ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/

#include "ioutil.h"  //for PROC_LIST

#define FD int  //to differentiate between regular integers and file descriptors

typedef enum {
    CRITICAL = -1, OK = 0, EXEC_ERR = 1, FILE_IO_ERR = 2
} EXEC_STATUS;

//executes processes, returns 0 on success, 1 on invalid input, -1 for a critical fork/exec failure, 2 if I/O fails
int exec_procs(PROC_LIST * proc_list);

//lists currently running jobs
int jobs(/* EXEC_LIST */);

int fg(int proc_id);

int bg(int proc_id);

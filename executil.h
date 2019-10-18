/* executil.h   --- Utility functions for executing processes ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/

#include "ioutil.h"     // for PROC_LIST
#include "linkedlist.h" // for deque

#define FD int  //to differentiate between regular integers and file descriptors

typedef enum {
    CRITICAL = -1, OK = 0, EXEC_ERR = 1, FILE_IO_ERR = 2
} EXEC_STATUS;

typedef struct {
    char * name; //malloced
    int * cpids; //malloced
    int cpidc;
    int pgid;
    int job_id; 
} JOB;

struct JOB_CTRL {
   DEQUE * jobs;
   DEQUE * created;
   DEQUE * stopped;
} job_ctrl;

// initializes the job control with empty deques and a job deleter function for job_ctrl->jobs
// returns 0 on success and 1 on failure 
int init_job_ctrl();

// deletes the job control, deallocates created and stopped
// goes through all jobs, frees the name and cpids, then frees the jobs
// then frees the jobs, created, and stopped deques
int delete_job_ctrl();

// initializes a (malloced) job with name, cpids, pgid, and the job id
//
JOB * init_job(char * name, int * cpids, int cpidc, int pgid, int job_id);

//executes processes, returns 0 on success, 1 on invalid input, -1 for a critical fork/exec failure, 2 if I/O fails
int exec_procs(PROC_LIST * proc_list, FD * fg_pgid);

//lists currently running jobs
int jobs();

int fg(int proc_id);

int bg(int proc_id);

//int poll_;

/* executil.h   --- Utility functions for executing processes ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/

#ifndef EXECUTIL_H
#define EXECUTIL_H

#include "ioutil.h" // for PROC_LIST
#include "linkedlist.h" // for deque

#define FD int //to differentiate between regular integers and file descriptors

typedef enum {
	CRITICAL = -1,
	OK = 0,
	EXEC_ERR = 1,
	FILE_IO_ERR = 2
} EXEC_STATUS;

typedef struct {
	char* name; //malloced
	pid_t* cpids; //malloced
	int procc;
	int pgid;
	int job_id;
} JOB;

struct JOB_CTRL {
	DEQUE* jobs;
	DEQUE* created;
	DEQUE* stopped;
} job_ctrl;

static JOB* fg_job =
	NULL; //persistent foreground job, NULL indicates no job in the foreground

// initializes the job control with empty deques and a job deleter function for job_ctrl.jobs
// returns 0 on success and 1 on failure
int init_job_ctrl();

// deletes the job control, deallocates created and stopped
// goes through all jobs, frees the name and cpids, then frees the jobs
// then frees the jobs, created, and stopped deques
int delete_job_ctrl();

// initializes a (malloced) job with name, cpids, pgid
// copies the name, cpids into malloced memory
// job_id is set to -1 initially
// returns the created job or NULL if invalid input
JOB* init_job(char* name, int strlen, int* cpids, int cpidc, int pgid);

//executes processes, returns OK on success, 1 on invalid input, -1 for a critical fork/exec failure, 2 if I/O fails
int exec_procs(PROC_LIST* proc_list);

// stops a running job (because a process was stopped)
// prints the proper stop message
// returns 0
int stop_job(JOB* job);

// restarts a stopped (background) job int the foreground (because fg was inputted)
// prints the proper restart message
// returns 0 if job successfully removed from stopped jobs
int restart_job(JOB* job);

// (re)starts a (stopped background) job int the background (because bg was inputted, or background job is executed)
// prints the proper restart message
// returns 0 if jobs successfuly removed from stopped jobs, otherwise job_id was not in stopped jobs
int run_job(JOB* job);

// bring running bg job to fg (because fg was inputted)
// print only the name of the job
// returns 0
int bring_job_to_fg(JOB* job);

// prints that job has finished (and waited on) and remove job from jobs, created, and stopped
// returns 0 on success
int finished_job(JOB* job);

// calls WNOHANG waitpid on all processes in a job
// if any are stopped, the entire process is stopped and -1 is returned
// otherwise the number of non-terminated processes are called (WIFEXITED or WIFSIGNALED)
int wait_job(JOB* job);

//lists currently running jobs
int jobs();

int fg(int proc_id);

int bg(int proc_id);

#endif

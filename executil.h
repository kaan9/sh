#ifndef EXECUTIL_H
#define EXECUTIL_H

#include <sys/types.h>

#include "ioutil.h" /* for PROC_LIST */
#include "linkedlist.h" /* for deque */

/* to differentiate between regular integers and file descriptors */
typedef int FD;

typedef enum {
	CRITICAL = -1,
	OK = 0,
	EXEC_ERR = 1,
	FILE_IO_ERR = 2
} EXEC_STATUS;

typedef struct {
	char *name; /* malloced */
	pid_t *cpids; /* malloced */
	int procc;
	int pgid;
	int job_id;
} Job;

struct Job_CTRL {
	DEQUE *jobs;
	DEQUE *created;
	DEQUE *stopped;
} job_ctrl;

/* persistent foreground job, NULL indicates no job in the foreground */
static Job *fg_job = NULL;

/* initializes the job control with empty deques and a job deleter function for job_ctrl.jobs */
/* returns 0 on success and 1 on failure */
int init_job_ctrl();

/* deletes the job control, deallocates created and stopped */
/* goes through all jobs, frees the name and cpids, then frees the jobs */
/* then frees the jobs, created, and stopped deques */
void delete_job_ctrl();

/* initializes a (malloced) job with name, cpids, pgid */
/* copies the name, cpids into malloced memory */
/* job_id is set to -1 initially */
/* returns the created job or NULL if invalid input */
Job *init_job(char *name, int strlen, int *cpids, int cpidc, int pgid);

/* executes processes, returns OK on success, 1 on invalid input, -1 for a critical fork/exec failure, 2 if I/O fails */
int exec_procs(PROC_LIST *proc_list);

/* stops a running job (because a process was stopped) */
/* prints the proper stop message */
/* returns 0 */
void stop_job(Job *job);

/* restarts a stopped (background) job int the foreground (because fg was inputted) */
/* prints the proper restart message */
/* returns 0 if job successfully removed from stopped jobs */
int restart_job(Job *job);

/* (re)starts a (stopped background) job int the background (because bg was inputted, or background job is executed) */
/* prints the proper restart message */
/* returns 0 if jobs successfuly removed from stopped jobs, otherwise job_id was not in stopped jobs */
int run_job(Job *job);

/* bring running bg job to fg (because fg was inputted) */
/* print only the name of the job */
void bring_job_to_fg(Job *job);

/* prints that job has finished (and waited on) and remove job from jobs, created, and stopped */
/* returns 0 on success */
void finished_job(Job *job);

/* calls WNOHANG waitpid on all processes in a job */
/* if any are stopped, the entire process is stopped and -1 is returned */
/* otherwise the number of non-terminated processes are called (WIFEXITED or WIFSIGNALED) */
int wait_job(Job *job);

/* lists currently running jobs */
int jobs();

void fg(int proc_id);

void bg(int proc_id);

#endif

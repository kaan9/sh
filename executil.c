/* executil.c   --- Implementation of executil.h ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/
#include "executil.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h> /* for kill */
#include <stdio.h> /* for perror */
#include <stdlib.h> /* for exit */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* functions for input/output, defines RDLEN, TOKMAX, PROCMAX */
#include "ioutil.h"
#include "linkedlist.h"

/* list of input and output file descriptors for each file */
/* FD[i - 1][0] is read and FD[i - 1][1] is write for child #i */
/* after forking, each child should close all other FDs */
FD fds[PROCMAX][2];

/* deleter function for a JOB */
static int JOB_deleter(void *j)
{
	JOB *job = (JOB *)j;
	if (job) {
		free(job->name);
		free(job->cpids);
	}
	free(job);
	return 0;
}

int init_job_ctrl()
{
	return !((job_ctrl.jobs = make_empty_list(&JOB_deleter)) &&
		 (job_ctrl.created = make_empty_list(NULL)) &&
		 (job_ctrl.stopped = make_empty_list(NULL)));
}

/* deletes the job control, deallocates created and stopped */
/* goes through all jobs, frees the name and cpids, then frees the jobs */
/* then frees the jobs, created, and stopped deques */
void delete_job_ctrl()
{
	delete_list(job_ctrl.jobs);
	delete_list(job_ctrl.created);
	delete_list(job_ctrl.stopped);
}

/* close all file descriptor pairs in fds except for pair p */
void close_fds(int procc, int p)
{
	int i;
	for (i = 0; i < procc; i++) {
		if (i != p) {
			if (fds[i][0] != STDIN_FILENO)
				if (close(fds[i][0]))
					perror("Invalid: Couldn't close file descriptor");
			if (fds[i][1] != STDOUT_FILENO)
				if (close(fds[i][1]))
					perror("Invalid: Couldn't close file descriptor");
		}
	}
}

JOB *init_job(char *name, int strlen, int *cpids, int procc, int pgid)
{
	int i;
	JOB *job = malloc(sizeof(JOB));

	job->name = malloc(sizeof(char) * (strlen + 1));
	job->cpids = malloc(sizeof(int) * procc);
	for (i = 0; i < strlen; i++)
		job->name[i] = name[i];
	job->name[strlen] = 0;
	for (i = 0; i < procc; i++)
		job->cpids[i] = cpids[i];
	job->procc = procc;
	job->pgid = pgid;
	return job;
}

void stop_job(JOB *job)
{
	kill(-job->pgid, SIGTSTP);
	tcsetpgrp(STDIN_FILENO, getpgid(0));
	printf("Stopped: %s\n", job->name);
	push_back(job_ctrl.stopped, &job->job_id);
}

int restart_job(JOB *job)
{
	kill(-job->pgid, SIGCONT);
	tcsetpgrp(STDIN_FILENO, job->pgid);
	printf("Restarting: %s\n", job->name);
	return remove_val(job_ctrl.stopped, &job->job_id);
}

int run_job(JOB *job)
{
	kill(-job->pgid, SIGCONT);
	tcsetpgrp(STDIN_FILENO, getpgid(0));
	printf("Running: %s\n", job->name);
	return remove_val(job_ctrl.stopped, &job->job_id);
}

void bring_job_to_fg(JOB *job)
{
	tcsetpgrp(STDIN_FILENO, job->pgid);
	printf("\n%s\n", job->name);
}

void delete_job(JOB *job)
{
	remove_val(job_ctrl.created, &job->job_id);
	remove_val(job_ctrl.stopped, &job->job_id);
	replace(job_ctrl.jobs, job->job_id, NULL);
	JOB_deleter(job);
}

void finished_job(JOB *job)
{
	printf("Finished: %s\n", job->name);
	delete_job(job);
}

void killed_job(JOB *job)
{
	printf("Killed: %s\n", job->name);
	delete_job(job);
}

/* calls WNOHANG waitpid on all processes in a job */
/* if any are stopped, the entire process is stopped and -1 is returned */
/* otherwise the number of non-terminated processes are returned (WIFEXITED or WIFSIGNALED) */
/* a return of 0 implies that the job has terminated */
int wait_job(JOB *job)
{
	int i, wstatus, w, total_running;
	if (!job)
		return 0;
	for (i = 0; i < job->procc; i++) {
		if (job->cpids[i]) {
			wstatus = 0, w = 0;
			if ((w = waitpid(job->cpids[i], &wstatus,
					 WUNTRACED | WNOHANG)) == -1) {
				perror("Invalid: waitpid");
				printf("%d", job->cpids[i]);
				kill(-job->pgid,
				       SIGKILL); /* kill job if waiting fail */
				return CRITICAL;
			} else if (w) {
				if (WIFEXITED(wstatus) ||
				    WIFSIGNALED(wstatus)) {
					job->cpids[i] = 0;
				} else if (WIFSTOPPED(wstatus)) {
					/* if any process is stopped, stop all processes in job */
					stop_job(job);
					return -1;
				}
			}
		}
	}
	total_running = 0;
	for (i = 0; i < job->procc; i++)
		if (job->cpids[i])
			total_running++;
	return total_running;
}

/* execute a single process */
pid_t exec_proc(char **argv, int procc, int i)
{
	pid_t pid = fork();
	if (pid < 0) {
		perror("Invalid: fork failed");
		return -1;
	} else if (pid == 0) { /* child */
		/* close all other file descriptors */
		close_fds(procc, i);

		dup2(fds[i][0], STDIN_FILENO);
		dup2(fds[i][1], STDOUT_FILENO);
		/* execute */
		execvp(argv[0], argv);
		/* if exec returns it has failed and the parent should kill all spawned processes */
		return -1;
	} else { /* parent */
		return pid;
	}
}

int exec_procs(PROC_LIST *proc_list)
{
	/* PGID of all processes of the job, the first processes's PID is given to the PGID */
	int procc = proc_list->procc, i, pgid = 0, total;
	pid_t cpids[PROCMAX]; /* pids of the spawned processes */
	JOB *new_job;

	if (!proc_list || !proc_list->procc)
		return 1;

	/* default values for input and output redirect */
	fds[0][0] = STDIN_FILENO;
	fds[procc - 1][1] = STDOUT_FILENO;

	/* open input and output redirect files */
	if (proc_list->input_redirect &&
	    (fds[0][0] = open(proc_list->input_redirect, O_RDONLY, 0644)) < 0) {
		return FILE_IO_ERR;
	}

	if (proc_list->output_redirect &&
	    (fds[procc - 1][1] = open(proc_list->output_redirect,
				      O_WRONLY | O_CREAT | O_TRUNC, 0644)) <
		    0) {
		close(fds[0][0]);
		return FILE_IO_ERR;
	}

	/* open n - 1 pipes */
	for (i = 0; i < procc - 1; i++) {
		FD pipefd[2];
		if (pipe(pipefd)) {
			perror("Critical: couldn't open pipe");
			close_fds(i, -1);
			return CRITICAL;
		}
		fds[i][1] = pipefd[1];
		fds[i + 1][0] = pipefd[0];
	}

	/* loop through and execute all processes */
	for (i = 0; i < procc; i++) {
		if ((cpids[i] = exec_proc(proc_list->procs[i], procc, i)) ==
		    -1) {
			if (pgid)
				kill(-pgid, SIGKILL);
			return EXEC_ERR;
		}

		pgid = cpids[0];

		if (setpgid(cpids[i], pgid)) {
			perror("Invalid: setpgid failed");
			kill(-pgid, SIGKILL);
			return CRITICAL;
		}
	}

	close_fds(procc, -1);

	new_job =
		init_job(proc_list->buf, proc_list->strlen, cpids, procc, pgid);
	new_job->job_id = insert_val(job_ctrl.jobs, new_job);
	push_back(job_ctrl.created, &new_job->job_id);

	if (proc_list->is_background) {
		run_job(new_job);
	} else {
		tcsetpgrp(STDIN_FILENO, pgid);
		fg_job = new_job; /* for signal handlers in main */
		total = 0;
		while ((total = wait_job(new_job)))
			if (total == -1)
				break;
		if (!total)
			delete_job(new_job);
		fg_job = NULL;
		tcsetpgrp(STDIN_FILENO, getpgid(0));
	}
	return 0;
}

/* 1 if running 0 if stopped, also stops jobs if any process is detected to have stopped */
/* -1 if job terminated (but not yet waited) */
int bg_job_status(JOB *job)
{
	int st = wait_job(job);
	if (st == 0)
		return -1;
	if (st < 0)
		return 0;
	return 1;
}

static void print_job(JOB *job)
{
	int status;
	if (!job)
		return;
	status = wait_job(job);
	if (status == 0) {
		finished_job(job);
		return;
	}
	printf("[%d] %s %s", job->job_id, job->name,
	       status > 0 ? "(running)\n" : " (stopped)\n");
}

int jobs()
{
	struct node *np;
	if (!job_ctrl.jobs)
		return -1;
	for (np = job_ctrl.jobs->head; np; np = np->next) {
		if (np->val)
			print_job(np->val);
	}
	return 0;
}

void fg(int proc_id)
{
	printf("%d\n", proc_id);
}

void bg(int proc_id)
{
	printf("%d\n", proc_id);
}

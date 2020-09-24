/* penn-sh   ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/
#include <signal.h>
#include <stdlib.h>

#include "executil.h"
#include "ioutil.h"

void siginthandler(int signum)
{
	prints("sending int to fg1\n");
	if (fg_job) {
		prints("sending int to fg\n");
		killpg(fg_job->pgid, SIGINT);
		fg_job = NULL;
	}
}

void sigtstophandler(int signum)
{
	if (fg_job) {
		stop_job(fg_job);
		fg_job = NULL;
	}
}

void sigtermhandler(int signum)
{
}

void free_memory(PROC_LIST* proc_list)
{
	delete_proc_list(proc_list);
	delete_job_ctrl();
}

void sync_wait()
{
	if (!job_ctrl.jobs)
		return;
	for (struct node* curr = job_ctrl.jobs->head; curr; curr = curr->next) {
		if (curr->val)
			switch (wait_job(curr->val)) {
			case 0:
				finished_job(curr->val);
				curr->val = NULL;
			case -1:
				continue;
			}
	}
}

int main(int argc, const char** argv)
{
	signal(SIGINT, siginthandler);
	signal(SIGTSTP, sigtstophandler);
	signal(SIGTERM, sigtermhandler);
	signal(SIGTTOU, SIG_IGN);

	PROC_LIST proc_list; //processes filtered from tokens, contains allocated memory
	proc_list.tokc = 0; // number of processes starts at 0
	int proc_id = -1; // id of process called with fg or bg

	init_job_ctrl();

	while (1) {
		sync_wait();

		prints("penn-sh# ");

		switch (proc_list_from_input(&proc_list, &proc_id)) {
		case VJOB:
			switch (exec_procs(&proc_list)) {
			case CRITICAL: //critical
				free_memory(&proc_list);
				prints("Invalid: Critical");
				exit(EXIT_FAILURE);
			case OK:
				continue;
			case EXEC_ERR:
				prints("Invalid: No such file or directory\n");
				free_memory(&proc_list);
				exit(EXIT_FAILURE);
			case FILE_IO_ERR:
				prints("Invalid: Unable to open input/output file\n");
				continue;
			}
		case EXIT:
			free_memory(&proc_list);
			exit(EXIT_SUCCESS);
		case SKIP:
			continue;
		case JOBS:
			jobs();
			continue;
		case FG:
			fg(proc_id);
			continue;
		case BG:
			bg(proc_id);
			continue;
		case FAIL:
		default:
			prints("Invalid: No such file or directory\n");
			continue;
		}
	}
	free_memory(&proc_list);
	exit(EXIT_SUCCESS);
}

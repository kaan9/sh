/* executil.c   --- Implementation of executil.h ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/
#include "executil.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>  //for kill
#include <stdio.h>   //for perror
#include <stdlib.h>  //for exit
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//functions for input/output, defines RDLEN, TOKMAX, PROCMAX
#include "ioutil.h"
#include "strutil.h"
#include "linkedlist.h"

// list of input and output file descriptors for each file
// FD[i - 1][0] is read and FD[i - 1][1] is write for child #i
// after forking, each child should close all other FDs
FD fds[PROCMAX][2];

// deleter function for a JOB
int JOB_deleter(void * j) {
    JOB * job = (JOB *) j;
    free(job->name);
    free(job->cpids);
    free(job);
    return 0;
}

int init_job_ctrl() {
    return !((job_ctrl.jobs = make_empty_list(&JOB_deleter)) && (job_ctrl.created = make_empty_list(NULL))
        && (job_ctrl.stopped = make_empty_list(NULL)));
}

// deletes the job control, deallocates created and stopped
// goes through all jobs, frees the name and cpids, then frees the jobs
// then frees the jobs, created, and stopped deques
int delete_job_ctrl() {
    delete_list(job_ctrl.jobs);
    delete_list(job_ctrl.created);
    delete_list(job_ctrl.stopped);
    return 0;
}

// close all file descriptor pairs in fds except for pair p
void close_fds(int procc, int p) {
    for (int i = 0; i < procc; i++) {
        if (i != p) {
            if (fds[i][0] != STDIN_FILENO) if (close(fds[i][0])) perror("Invalid: Couldn't close file descriptor");
            if (fds[i][1] != STDOUT_FILENO) if (close(fds[i][1])) perror("Invalid: Couldn't close file descriptor");
        }
    }
}

JOB * init_job(char * name, int strlen, int * cpids, int procc, int pgid) {
    JOB * job = malloc(sizeof(JOB));
    
    job->name = malloc(sizeof(char) * (strlen + 1));
    job->cpids = malloc(sizeof(int) * procc);
    for (int i = 0; i < strlen; i++) job->name[i] = name[i];
    name[strlen] = 0;
    for (int i = 0; i < procc; i++) job->cpids[i] = cpids[i];
    job->pgid = pgid;
    return job;
}

int stop_job(JOB * job) {
    killpg(job->pgid, SIGTSTP);
    tcsetpgrp(STDIN_FILENO, getpgid(0));
    prints("\nStopped: ");
    prints(job->name);  
    endl();
    push_back(job_ctrl.stopped, &job->job_id);
    return 0;
}

int restart_job(JOB * job) {
    killpg(job->pgid, SIGCONT);
    tcsetpgrp(STDIN_FILENO, job->pgid);
    prints("\nRestarting: ");
    prints(job->name);  
    endl();
    return remove_val(job_ctrl.stopped, &job->job_id);
}

int run_job(JOB * job) {
    killpg(job->pgid, SIGCONT);
    tcsetpgrp(STDIN_FILENO, getpgid(0));
    prints("\nRunning: "); prints(job->name);  
    endl();
    return remove_val(job_ctrl.stopped, &job->job_id);
}

int bring_job_to_fg(JOB * job) {
    tcsetpgrp(STDIN_FILENO, job->pgid);
    endl();
    prints(job->name);  
    endl();
    return 0;
}

int delete_job(JOB * job) {
    remove_val(job_ctrl.created, &job->job_id);
    remove_val(job_ctrl.stopped, &job->job_id);
    JOB_deleter(replace(job_ctrl.jobs, job->job_id, NULL));
    return 0;
}

int finished_job(JOB * job) {
    prints("\nFinished: ");
    prints(job->name);
    endl();
    delete_job(job);
    return 0;
}

int killed_job(JOB * job) {
    prints("\nKilled: ");
    prints(job->name);
    endl();
    delete_job(job);
    return 0;
}


// calls WNOHANG waitpid on all processes in a job
// if any are stopped, the entire process is stopped and -1 is returned
// otherwise the number of non-terminated processes are called (WIFEXITED or WIFSIGNALED)
int wait_job(JOB * job) {
    int total = 0;
    for (int i = 0; i < job->procc; i++) {
         if (job->cpids[i]) {
            int wstatus = 0, w = 0;
                if ((w = waitpid(job->cpids[i], &wstatus, WUNTRACED | WNOHANG)) == -1) {
                    perror("Invalid: waitpid");
                    killpg(job->pgid, SIGKILL); // kill job if waiting fail
                    return CRITICAL;
                } else if (w) {
                    if (WIFEXITED(wstatus)) {
                        job->cpids[i] = 0;
                        total++;
                    } else if (WIFSIGNALED(wstatus)) {
                        job->cpids[i] = 0;
                        total++;
                    } else if (WIFSTOPPED(wstatus)) {
                        // if any process is stopped, stop all processes in job
                        stop_job(job);
                        return -1;
                }
            }
        }
    }
    return -1;
}

// execute a single process
pid_t exec_proc(char ** argv, int procc, int i) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Invalid: fork failed");
        return -1;
    } else if (pid == 0) { // child
        //close all other file descriptors
        close_fds(procc, i);
    
        dup2(fds[i][0], STDIN_FILENO);
        dup2(fds[i][1], STDOUT_FILENO);
        //execute
        execvp(argv[0], argv); 
        //if exec returns it has failed and the parent should kill all spawned processes
        return -1;
    } else { // parent
        return pid;
    }
}

int exec_procs(PROC_LIST * proc_list) {
    if (!proc_list || !proc_list->procc) return 1;

    int procc = proc_list->procc; //for quick access

    pid_t cpids[PROCMAX];  //pids of the spawned processes

    int pgid = 0; // PGID of all processes of the job, the first processes's PID is given to the PGID 

    //default values for input and output redirect
    fds[0][0] = STDIN_FILENO;
    fds[procc - 1][1] = STDOUT_FILENO;

    // open input and output redirect files
    if (proc_list->input_redirect && (fds[0][0] = open(proc_list->input_redirect, O_RDONLY, 0644)) < 0) {
        return FILE_IO_ERR;
    }

    if (proc_list->output_redirect && (fds[procc - 1][1] = open(proc_list->output_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
    {
        close(fds[0][0]);
        return FILE_IO_ERR;
    }

    // open n - 1 pipes
    for (int i = 0; i < procc - 1; i++) {
        FD pipefd[2];
        if (pipe(pipefd)) {
            perror("Critical: couldn't open pipe");
            close_fds(i, -1);
            return CRITICAL;
        }
        fds[i][1] = pipefd[1];
        fds[i + 1][0] = pipefd[0];
    }

    // loop through and execute all processes
    for (int i = 0; i < procc; i++) {
        if ((cpids[i] = exec_proc(proc_list->procs[i], procc, i)) == -1) {
            if (pgid) killpg(pgid, SIGKILL);
            return EXEC_ERR;
        }
        
        pgid = cpids[0];
        
        if (setpgid(cpids[i], pgid)) {
            perror("Invalid: setpgid failed");
            killpg(pgid, SIGKILL);
            return CRITICAL;
        }
    }

    close_fds(procc, -1);

    JOB * new_job = init_job(proc_list->buf, proc_list->strlen, cpids, procc, pgid);
    new_job->job_id = insert_val(job_ctrl.jobs, new_job);
    push_back(job_ctrl.created, &new_job->job_id);

    if (proc_list->is_background) {
        run_job(new_job);
    } else {
        tcsetpgrp(STDIN_FILENO, pgid);
        fg_job = new_job; // for singal handlers in main
        int total = 0;
        while ((total = wait_job(new_job)))if (total == -1) break;
        if (!total) delete_job(new_job);
        fg_job = NULL;
        tcsetpgrp(STDIN_FILENO, getpgid(0));
    }
    prints("here\n");
    return 0;
}


// 1 if running 0 if stopped, also stops jobs if any process is detected to have stopped
// -1 if job terminated (but not yet waited)
int bg_job_status(JOB * job) {
    int st = wait_job(job);
    if (st == 0) return -1;
    if (st < 0) return 0;
    return 1;
}

void * print_job(void * j) {
    if (!j) return j;
    JOB * job = (void *) j;
    int status = bg_job_status(job);
    if (status == -1) return j;
    printc('[');
    printi(job->job_id);
    prints("] ");
    prints(job->name);
    prints(status ? " (running)\n" : " (stopped)\n");
    return j;
}

int jobs() {
    map(job_ctrl.jobs, &print_job);
    return 0;
}

int fg(int proc_id) {
    printi(proc_id);
    endl();
    return 0;
}

int bg(int proc_id) {
    printi(proc_id);
    endl();
    return 0;
}

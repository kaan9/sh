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

//void kill_children(pid_t * children, int children_c, int SIG) {
//    for (int i = 0; i < children_c; i++) kill(children[i], SIG);
//}

// close all file descriptor pairs in fds except for pair p
void close_fds(int procc, int p) {
    for (int i = 0; i < procc; i++) {
        if (i != p) {
            if (fds[i][0] != STDIN_FILENO) if (close(fds[i][0])) perror("Invalid: Couldn't close file descriptor");
            if (fds[i][1] != STDOUT_FILENO) if (close(fds[i][1])) perror("Invalid: Couldn't close file descriptor");
        }
    }
}

// stops a running job (because a process was stopped)
// prints the proper stop message
// returns 0 
int stop_job(JOB * job) {
    killpg(job->pgid, SIGTSTP);
    tcsetpgrp(STDIN_FILENO, getpgid(0));
    prints("Stopped: ");
    prints(job->name);  
    endl();
    push_back(job_ctrl->stopped, (void *) job->job_id);
    return 0;
}

// restarts a stopped (background) job int the foreground (because fg was inputted)
// prints the proper restart message
// returns 0 if job successfully removed from stopped jobs
int restart_job(JOB * job) {
    killpg(job->pgid, SIGCONT);
    tcsetpgrp(STDIN_FILENO, job->pgid);
    prints("Restarting: ");
    prints(job->name);  
    endl();
    return remove_val(job_ctrl->stopped, (void *) job->job_id);
}

// (re)starts a (stopped background) job int the background (because bg was inputted, or background job is executed)
// prints the proper restart message
// returns 0 if jobs successfuly removed from stopped jobs, otherwise job_id was not in stopped jobs
int run_job(JOB * job) {
    killpg(job->pgid, SIGCONT);
    tcsetpgrp(STDIN_FILENO, getpgid(0));
    prints("Running: ");
    prints(job->name);  
    endl();
    return remove_val(job_ctrl->stopped, (void *) job->job_id);
}

// bring running bg job to fg (because fg was inputted)
// print only the name of the job
// returns 0
int bring_job_to_fg(JOB * job) {
    tcsetpgrp(STDIN_FILENO, job->pgid);
    prints(job->name);  
    endl();
    return 0;
}

// prints that job has finished and removes job from jobs, created, and stopped 
// returns 0 on success
int finished_job(JOB * job) {
    prints("Finished: ");
    prints(job->name);
    endl();
    remove_val(job_ctrl->created, (void *) job->job_id);
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

int exec_procs(PROC_LIST * proc_list, int * fg_pgid) {
    if (!proc_list || !proc_list->procc) return 1;

    int procc = proc_list->procc; //for quick access

    pid_t cpids[PROCMAX];  //pids of the spawned processes

    int pgid = 0; // PGID of all processes of the job, the first processes's PID is given to the PGID 

    //default values for input and output redirect
    fds[0][0] = STDIN_FILENO;
    fds[procc - 1][1] = STDOUT_FILENO;

    // open input and output redirect files
    if (proc_list->input_redirect && (fds[0][0] = open(proc_list->input_redirect, O_RDONLY, 0644)) < 0) {
        return FILE_IO_ERROR;
    }

    if (proc_list->output_redirect && (fds[procc - 1][1] = open(proc_list->output_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
    {
        close(fds[0][0]);
        return FILE_IO_ERROR;
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
            kill_children(cpids, i, SIGKILL);
            return EXEC_ERR;
        }
        
        *fg_pgid = pgid = cpids[0];
        
        if (setpgid(cpids[i], pgid)) {
            perror("Invalid: setpgid failed");
            kill_children(cpids, i, SIGKILL);
            return CRITICAL;
        }
    }

    close_fds(procc, -1);

    JOB * new_job = init_job(proc_list->buf, proc_list->cpids, procc, pgid);
    new_job->job_id = insert_val(job_ctrl->jobs, new_job);
    push_back(job_ctrl->created, (void *) new_job->job_id);

    if (proc_list->is_background) {
        tcsetpgrp(STDIN_FILENO, getpgid(0));
    } else {
        tcsetpgrp(STDIN_FILENO, pgid);
    }

    //wait for all children in foreground
    //if a child is stopped, stop the entire process
    int alive_c = !proc_list->is_background ? sp_procs : 0;
    while (alive_c) {
        for (int i = 0; (i < procc) && alive_c; i++) {
            if (cpids[i]) {
                int wstatus = 0, w = 0;
                if ((w = waitpid(cpids[i], &wstatus, WUNTRACED | WNOHANG)) == -1) {
                    perror("Invalid: waitpid");
                    kill_children(cpids, sp_procs, SIGKILL); // kill job if waiting failsi
                    return CRITICAL;
                } else if (w) {
                    if (WIFEXITED(wstatus)) {
                        cpids[i] = 0;
                        alive_c--;
                    } else if (WIFSIGNALED(wstatus)) {
                        cpids[i] = 0;
                        alive_c--;
                    } else if (WIFSTOPPED(wstatus)) {
                        // if any process is stopped, stop all processes in job
                        stop_job(job);
                        alive_c = 0;
                    }
                }
            }
        }
    }
    new_job = NULL; //fg back to terminal
    tcsetpgrp(STDIN_FILENO, getpgid(0));
    return 0;
}

void * print_jobs(void * j) {
    if (!j) return j;
    JOB * job = (JOB *) j;
     
    
    return j;
}

int jobs() {
    
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

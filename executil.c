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

// list of input and output file descriptors for each file
// FD[i - 1][0] is read and FD[i - 1][1] is write for child #i
// after forking, each child should close all other FDs
FD fds[PROCMAX][2];

void kill_children(pid_t * children, int children_c) {
    for (int i = 0; i < children_c; i++) kill(children[i], SIGKILL);
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
        prints("Invalid: No such file or directory\n");
        return -1;
    } else { // parent
        return pid;
    }
}

int exec_procs(PROC_LIST * proc_list, FD * fg_pgid) {
    if (!proc_list || !proc_list->procc) return 1;

    int procc = proc_list->procc; //for quick access

    int sp_procs = 0;      //count of spawned processes
    pid_t cpids[PROCMAX];  //pids of the spawned processes

    int pgid = 0; // PGID of all processes of the job, the first processes's PID is given to the PGID 

    //default values for input and output redirect
    fds[0][0] = STDIN_FILENO;
    fds[procc - 1][1] = STDOUT_FILENO;

    // open input and output redirect files
    if (proc_list->input_redirect && (fds[0][0] = open(proc_list->input_redirect, O_RDONLY, 0644)) < 0) {
        return 2;
    }

    if (proc_list->output_redirect && (fds[procc - 1][1] = open(proc_list->output_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
        close(fds[0][0]);
        return 2;
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
        if ((cpids[sp_procs++] = exec_proc(proc_list->procs[i], procc, i)) == -1) {
            kill_children(cpids, sp_procs);
            return CRITICAL;
        }
        
        *fg_pgid = pgid = cpids[0];
        
        if (setpgid(cpids[i], pgid)) {
            perror("Invalid: setpgid failed");
            kill_children(cpids, i + 1);
            return CRITICAL;
        }
    }

    close_fds(procc, -1);

    //wait for all children
    for (int i = 0; i < sp_procs; i++) {
        int status = 0;
        wait(&status);
    }
    return 0;
}

int jobs(/* EXEC_LIST */) {
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

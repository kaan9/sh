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

void save_stdio() {
    prints("saving\n");
    if (dup2(STDOUT_FILENO, STDOUT_DFLT) == -1 || dup2(STDIN_FILENO, STDIN_DFLT) == -1) {
        perror("Invalid: Critical I/O error");
        exit(EXIT_FAILURE);
    }
}

void restore_stdio() {
    prints("restoring\n");
    if (dup2(STDOUT_DFLT, STDOUT_FILENO) == -1 || dup2(STDIN_DFLT, STDIN_FILENO) == -1) {
        perror("Invalid: Critical I/O error");
        exit(EXIT_FAILURE);
    }
}

pid_t exec_proc(char ** argv, FD input, FD output, FD proc_to_close_1, FD proc_to_close_2) {
    if (input == -1) input = STDIN_FILENO;
    if (output == -1) output = STDOUT_FILENO;

    pid_t pid = fork();
    if (pid < 0) {
        perror("Invalid fork");
        return -1;
    } else if (pid == 0) {  //child process
        //set stdio from procs_io
        dup2(input, STDIN_FILENO);
        dup2(output, STDOUT_FILENO);
        close(proc_to_close_1);  //close extraneous connections
        close(proc_to_close_2);
        execvp(argv[0], argv);  //execute
        //if exec returns it has failed and the parent should kill all spawned processes
        prints("Invalid: No such file or directory\n");
        return -1;
    } else {  //parent process
        return pid;
    }
}

void kill_children(pid_t * children, int children_c) {
    for (int i = 0; i < children_c; i++) kill(children[i], SIGKILL);
}

int exec_procs(PROC_LIST * proc_list) {
    if (!proc_list || !proc_list->procc) return 1;

    // Input/Output for the previous and current spawned process
    //for process i, prev_io[0] is where the proc[i] reads and curr_io[1] is where proc[i] reads
    // proc[i] creates a new pipe and assigns it to prev_io to set its own write output and pass its read to the next processs
    // for proc[0], prev_io[0] is input_redirect || STDIN_FILENO and proc[1] is unspecified
    // the last process reads from prev_io[0] but does not create a new pipe, its output is left as output_redirect || STDOUT_FILENO
    FD prev_io[2];
    FD curr_io[2];

    int sp_procs = 0;      //count of spawned processes
    pid_t cpids[PROCMAX];  //pids of the spawned processes

    const int procc = proc_list->procc;  //shorthand

    FD rd_in_fd  = -1;  //redirect in file descriptor (STDIN_FILENO by default)
    FD rd_out_fd = -1;  //redirect out file descriptor (STDOUT_FILENO by default)

    // open input and output redirect files
    if (proc_list->input_redirect && (rd_in_fd = open(proc_list->input_redirect, O_RDONLY, 0644)) < 0)
        return 2;

    if (proc_list->output_redirect && (rd_out_fd = open(proc_list->output_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
        close(rd_in_fd);
        return 2;
    }

    // loop through and execute all processes except the last one
    prev_io[0] = rd_in_fd;
    for (int i = 0; i < procc - 1; i++) {
        pipe(curr_io);
        close(prev_io[1]);
        if ((cpids[sp_procs++] = exec_proc(proc_list->procs[0], prev_io[0], curr_io[1], curr_io[0], rd_out_fd)) == -1) {
            kill_children(cpids, sp_procs);
            return -1;
        }
        close(prev_io[0]);
        prev_io[0] = curr_io[0];
        prev_io[1] = curr_io[1];
    }

    //handle last process
    curr_io[1] = rd_out_fd;
    close(prev_io[1]);
    if ((cpids[sp_procs++] = exec_proc(proc_list->procs[procc - 1], prev_io[0], curr_io[1], curr_io[0], rd_out_fd)) == -1) {
        kill_children(cpids, sp_procs);
        return -1;
    }
    close(prev_io[0]);
    close(curr_io[0]);
    close(curr_io[1]);
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

/* executil.c   --- Implementation of executil.h ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/
#include "executil.h"

#include <fcntl.h>
#include <stdio.h>   //for perror
#include <stdlib.h>  //for exit
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//functions for input/output, defines RDLEN, TOKMAX, PROCMAX
#include "ioutil.h"
#include "strutil.h"

void save_stdio() {
    if (dup2(STDOUT_FILENO, STDOUT_DFLT) == -1 || dup2(STDIN_FILENO, STDIN_DFLT) == -1) {
        perror("Invalid: Critical I/O error");
        exit(EXIT_FAILURE);
    }
}

void restore_stdio() {
    if (dup2(STDOUT_DFLT, STDOUT_FILENO) == -1 || dup2(STDIN_DFLT, STDIN_FILENO) == -1) {
        perror("Invalid: Critical I/O error");
        exit(EXIT_FAILURE);
    }
}

int exec_procs(PROC_LIST * proc_list) {
    if (!proc_list && !proc_list->procc) return 1;

    save_stdio();  //save the default STDIN/OUT

    //FD out_file, in_file;      //for the redirection input/output

    // Input/Output for all spawned processes, procs_io[i][0] is the input and procs_io[i][1] is the output for the ith process
    // procs_io[0][0] is STDIN or input_recirect, procs_io[procc - 1][1] is STDOUT or output_recirect
    //all other values are from pipes
    FD procs_io[PROCMAX][2];

    int sp_procs = 0;  //count of spawned processes
    pid_t cpids[PROCMAX];

    const int procc = proc_list->procc;  //for quick access

    //default values for I/O redirects
    procs_io[0][0]         = STDIN_DFLT;
    procs_io[procc - 1][1] = STDOUT_DFLT;

    // open input and output redirect files
    if (proc_list->input_redirect && (procs_io[0][0] = open(proc_list->input_redirect, O_RDONLY, 0644)) < 0) {
        perror("Invalid: Unable to open input file");
        return 2;
    }
    if (proc_list->output_redirect && (procs_io[proc_list->procc - 1][1] = open(proc_list->output_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
        perror("Invalid: Unable to open output file");
        close(procs_io[0][0]);
        return 2;
    }

    //open all the pipes beforehand
    for (FD t_pipe[2], int i = 0; i < procc - 1; i++) {
        if (pipe(t_pipe) == -1) perror("pipe error"), exit(EXIT_FAILURE);
        procs_io[i][1]     = t_pipe[1];
        procs_io[i + 1][0] = t_pipe[0];
    }

    for (sp_procs = 0; sp_procs < procc; i++) {
        cpids[sp_procs] = fork();
        if (pid < 0) {
            perror("Invalid fork");
            for (int i = 0; i < sp_procs; i++) kill(cpids[i], SIGKILL); //kill all already spawned procs if cannot spawn one
            return -1;
        } else if (pid == 0) { //child process
            //set stdio from procs_io
            dup2(procs_io[sp_procs][0], STDIN_FILENO);
            dup2(procs_io[sp_procs][1], STDOUT_FILENO);
            //execute
            execvp(proc_list->procs[sp_procs][0], proc_list->procs[sp_procs]);
            //if execvp returns, it has failed and will print no such file or directory
            //additionally, kill all previously spawned processes, if any exist
            prints("Invalid: No such file or directory\n");
            for (int i = 0; i < sp_procs; i++) kill(cpids[i], SIGKILL);
            return -1;
        }
        //parent process loops back
    }

    //wait for all children
    for (int i = 0; i < sp_procs; i++) {
        wait(NULL);
    }

    restore_stdio();  //restore the default STDIN/OUT
    return 0;
}
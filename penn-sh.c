/* penn-sh   ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/
#include <signal.h>
#include <stdlib.h>  //for exit

#include "executil.h"  //process execution utility functions
#include "ioutil.h"    //functions for input/output, defines RDLEN, TOKMAX, PROCMAX
#include "strutil.h"   //string utlity functions


void siginthandler(int signum) {
    if (fg_job) {
        killpg(fg_job->pgid, SIGINT);
        fg_job = NULL;
    }
}

void sigtstophandler(int signum) {
    if (fg_job) {
        stop_job(fg_job);
        fg_job = NULL;
    }
}

void sigtermhandler(int signum) {}

void free_memory(PROC_LIST * proc_list) {
    delete_proc_list(proc_list);
    delete_job_ctrl();
}

void * handle_job(void * j) {
    if (!j) return j;
    JOB * job = (JOB *) j;
    int res = wait_job(job);
    if (!res) finished_job(job);
    return j;
}

int main(int argc, const char ** argv) {
    signal(SIGINT, siginthandler);
    signal(SIGTSTP, sigtstophandler);
    signal(SIGTERM, sigtermhandler);
    signal(SIGTTOU, SIG_IGN);


    PROC_LIST proc_list;  //processes filtered from tokens, contains allocated memory
    proc_list.tokc = 0;   // number of processes starts at 0
    int proc_id    = -1;  // id of process called with fg or bg

    init_job_ctrl();

    while (1) {
//        map(job_ctrl.jobs, &handle_job);
        prints("penn-sh# ");

        switch (proc_list_from_input(&proc_list, &proc_id)) {
            case VJOB:
                switch (exec_procs(&proc_list)) {
                    case CRITICAL:  //critical
                        free_memory(&proc_list);
                        prints("Invalid: Critical");
                        exit(EXIT_FAILURE);
                    case OK:
                        prints("continue\n");
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

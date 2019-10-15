/* penn-sh   ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/
#include <errno.h>  //for errno
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>  //for exit
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "executil.h"  //process execution utility functions
#include "ioutil.h"    //functions for input/output, defines RDLEN, TOKMAX, PROCMAX
#include "strutil.h"   //string utlity functions

//do nothing if SIGINT is received
void siginthandler(int signum) {}

int main(int argc, const char ** argv) {
    signal(SIGINT, siginthandler);

    PROC_LIST proc_list;  //processes filtered from tokens, contains allocated memory
    proc_list.tokc = 0;   // number of processes starts at 0
    int proc_id    = -1;  // id of process called with fg or bg

    while (1) {
        prints("penn-sh# ");

        switch (proc_list_from_input(&proc_list, &proc_id)) {
            case JOB:
                switch (exec_procs(&proc_list)) {
                    case -1:  //critical
                        delete_proc_list(&proc_list);
                        exit(EXIT_FAILURE);
                    case 0:
                        continue;
                    case 1:
                        prints("Invalid: No such file or directory\n");
                        continue;
                    case 2:
                    default:
                        perror("Invalid: Unable to open input/output file");
                        continue;
                }
            case EXIT:
                delete_proc_list(&proc_list);
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

    delete_proc_list(&proc_list);
    exit(EXIT_SUCCESS);
}

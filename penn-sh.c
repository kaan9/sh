/* penn-sh   ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/
#include <signal.h>
#include <stdlib.h>  //for exit

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
                    case CRITICAL:  //critical
                        delete_proc_list(&proc_list);
                        prints("Invalid: Critical");
                        exit(EXIT_FAILURE);
                    case OK:
                        continue;
                    case EXEC_ERR:
                        prints("Invalid: No such file or directory\n");
                        continue;
                    case FILE_IO_ERR:
                        prints("Invalid: Unable to open input/output file\n");
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

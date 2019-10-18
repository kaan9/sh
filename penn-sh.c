/* penn-sh   ---  Kaan B Erdogmus, Belinda Liu,  CIS 380, kaanberk*/
#include <signal.h>
#include <stdlib.h>  //for exit

#include "executil.h"  //process execution utility functions
#include "ioutil.h"    //functions for input/output, defines RDLEN, TOKMAX, PROCMAX
#include "strutil.h"   //string utlity functions

FD fg_pgid = 0;

void siginthandler(int signum) {
    if (fg_pgid) {
        kill(fg_pgid, SIGINT);
        fg_pgid = 0;
    }
}

void sigtstophandler(int signum) {
    if (fg_pgid) {
        killpg(fg_pgid, SIGTSTP);
        fg_pgid = 0;
    }
}

void sigtermhandler(int signum) {}

int main(int argc, const char ** argv) {
    signal(SIGINT, siginthandler);
    signal(SIGTSTP, sigtstophandler);
    signal(SIGTERM, sigtermhandler);
    signal(SIGTTOU, SIG_IGN);


    PROC_LIST proc_list;  //processes filtered from tokens, contains allocated memory
    proc_list.tokc = 0;   // number of processes starts at 0
    int proc_id    = -1;  // id of process called with fg or bg

    init_jobctrl();

    while (1) {
        
      /*  for (int i = 0; i < sp_procs; i++) {
            if (cpids[i]) {
                int wstatus = 0, w = 0;
                if ((w = waitpid(cpids[i], &wstatus, WUNTRACED | WNOHANG)) == -1) {
                    perror("Invalid: waitpid");
                    kill_children(cpids, sp_procs, SIGKILL); // kill job if waiting fails
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
                        kill_children(cpids, sp_procs, SIGSTOP);
                        alive_c = 0;
                    }
                }
            }
        }
*/

        prints("penn-sh# ");

        switch (proc_list_from_input(&proc_list, &proc_id)) {
            case VJOB:
                switch (exec_procs(&proc_list, &fg_pgid)) {
                    case CRITICAL:  //critical
                        delete_proc_list(&proc_list);
                        prints("Invalid: Critical");
                        exit(EXIT_FAILURE);
                    case OK:
                        continue;
                    case EXEC_ERR:
                        prints("Invalid: No such file or directory\n");
                        delete_proc_list(&proc_list);
                        exit(EXIT_FAILURE);
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

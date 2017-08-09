#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#define MAX_COMMAND_LENGTH 513
#define MAX_NUMBER_OF_PARAMS 1000

typedef struct {
    char* name;
    int pid;
    int jid;
    int background;
    int argcnt;
}Process;

void parseCmd(char* cmd, char** params);
int printjobs(Process *proc_ptr);
int myw(Process* proc_ptr, char* jid);
void loop(FILE* fp);
int isRunning(int pid);

int argcount;
int fileopen = 0;

Process proc[300];

char cmd[sizeof(char) * (MAX_COMMAND_LENGTH+1)];
char* params[MAX_NUMBER_OF_PARAMS];
char* jobname;


int jobcount = 0;
int jid = 1;

int main(int argc, char* argv[]) {
    FILE* input_fd;
    if (argc > 2) {
      fprintf(stderr, "Usage: mysh [batchFile]\n");
      fflush(stderr);
      exit(1);
    }
    if (argc == 2) {
        fclose(stdin);
        input_fd = fopen(argv[1], "r");
        if (input_fd == NULL) {
            fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
            fflush(stderr);
            exit(1);
        } else {
            fileopen = 1;
            loop(input_fd);
        }
    }

    if (argc == 1) {
       loop(stdin);
    }

    // printf("jobcount: %d\n", jobcount);

    int num;
    for (num = 0; num <= jobcount; num++) {
        free(proc[num].name);
    }

    free(input_fd);
    fclose(stdin);
    return 0;
}


// COMMAND LOOP
void loop(FILE* fp) {
    while (1) {
        if (fileopen == 0) {
            fprintf(stdout, "mysh> ");
            fflush(stdout);
        }

        int bg = 0;
        int builtInCmd = 1;

        if (fgets(cmd, sizeof(cmd), fp)  == NULL) break;

        // printf("last character is: %s\n", cmd[strlen(cmd-1)]);

        /*if (cmd[strlen(cmd-1)] != '\n') {
            // scanf("%*[^\n]%1*[\n]");
	        fprintf(stderr, "Command TOO LONG\n");
            fflush(stderr);
            continue;
         }*/

         if (fileopen == 1) {
            fprintf(stdout, "%s", cmd);
            fflush(stdout);
         }


        // remove trailing newline
        if (cmd[strlen(cmd)-1] == '\n') {
            cmd[strlen(cmd)-1] = '\0';
            // printf("newline trail removed\n");
        }
        if (cmd[strlen(cmd)-1] == '&') {
            cmd[strlen(cmd)-1] = '\0';
            bg = 1;
        }

        // jobname = malloc(sizeof(char) * sizeof(cmd));
        // jobname = strcpy(jobname, cmd);
        proc[jobcount].name =  malloc(sizeof(char) * strlen(cmd));
        proc[jobcount].name = strcpy(proc[jobcount].name, cmd);

        parseCmd(cmd, params);

        // ((params[1])[strlen(params[1]-1)]) = '\0';
        // printf("%s\n", params[1]);

        if (params[0] == NULL) {
            continue;
        }
        // handling white space and tabs
        if (strcmp(params[0], "\0") == 0) {
            continue;
        }
        if (strcmp(params[0], "\t")  == 0) {
            continue;
        }
        if (argcount >= 1 && strcmp(params[0], "&&") == 0) {
            continue;
        }
        // if (argcount >= 1 && strcmp(params[0], "&") == 0) {
        //    continue;
        // }


        if (argcount >= 2) {
            if ((strcmp(params[0], "j") == 0) &&
                           (strcmp(params[1], "&") != 0)) {
                builtInCmd = 0;
            }
            if ((strcmp(params[0], "exit") == 0) &&
                           (strcmp(params[1], "&") != 0)) {
                builtInCmd = 0;
            }
            if ((strcmp(params[0], "myw") == 0) && argcount > 2) {
                builtInCmd = 0;
            }
        }

        // handling 'j', 'exit', 'myw', & builtin cases
        if (builtInCmd == 1) {
            if (argcount >= 2) {
                if ((strcmp(params[0], "j") == 0) &&
                            (strcmp(params[1], "&") == 0)) {
                    printjobs(proc);
                    continue;
                }
                if ((strcmp(params[0], "exit") == 0) &&
                            (strcmp(params[1], "&") == 0)) {
                    break;
                }
        }

            // handling single builtin cases
            if (strcmp(params[0], "j") == 0) {
                printjobs(proc);
                continue;
            }
            if (strcmp(params[0], "j&") == 0) {
                printjobs(proc);
                continue;
            }
            if (strcmp(params[0], "exit") == 0) {
                break;
            }
            if (strcmp(params[0], "exit&") == 0) {
                break;
            }
            if ((strcmp(params[0], "myw") == 0) && argcount == 2 &&
                                                 builtInCmd == 1) {
                myw(proc, params[1]);
                continue;
            }
            if ((strcmp(params[argcount-1], "&") == 0) ||
            (proc[jobcount].name[strlen(proc[jobcount].name-1)] == '&')) {
                bg = 1;
            }
      }

        // start fork and exec
        proc[jobcount].jid = jid;
        proc[jobcount].background = bg;


        pid_t pid = fork();

       // pid_t pid = fork();
        if (pid == -1) {
           fprintf(stderr, "fork error\n");
           fflush(stderr);
           exit(1);
        } else if (pid == 0) {
            // proc[jobcount].pid = getpid();
            // printf("proc.pid in fork is %d\n", proc[jobcount].pid);
            // printf("child pid is %d\n", getpid());
            execvp(params[0], params);
            fprintf(stderr, "%s: Command not found\n", params[0]);
            fflush(stderr);
            exit(1);
        } else {
        // printf("I am the parent: %d || child pid: %d\n", getpid(), pid);
        proc[jobcount].pid = pid;
        // printf("new process pid: %d\n", proc[jobcount].pid);
        // if not a background, tell parent to wait
            if (bg == 0) {
                int childStatus;
                waitpid(pid, &childStatus, 0);
            }
        }

        jid++;
        jobcount++;
    }  // end while loop
}  // end loop function

void parseCmd(char* cmd, char** params) {
    int i = 0;
    params[i] = strtok(cmd, " \t\n");
    while (params[i] != NULL) {
        i++;
        params[i] = strtok(NULL, " \t\n");
    }

    argcount = i;
    proc[jobcount].argcnt = argcount;
}

int printjobs(Process *proc_ptr) {
    Process *proc = proc_ptr;
    char *params[MAX_NUMBER_OF_PARAMS];

    int i;
    for (i = 0; i < jobcount; i++) {
        // if ((proc[i].name[strlen(proc[i].name)-1]) == ' ') {
        //    proc[i].name[strlen(proc[i].name)-1] = '\0';
        // }
        char *namehold;
        namehold = (char*)malloc((sizeof(char)) * strlen(proc[i].name));
        namehold = strcpy(namehold, proc[i].name);
        // printf("BEFORE TOKENS: %s\n", proc[i].name);
        // printf("before Tokens pname: %s\n", proc[i].name);
        if (proc[i].background == 1) {
            int j = 0;
            params[j] = strtok(proc[i].name, " \t");
            while (params[j] != NULL) {
                j++;
                params[j] = strtok(NULL, " \t");
            }
            // strcpy(proc[i].name, namehold);
            // free(namehold);
            // printf("AFTER TOKENS: %s %s\n", params[0], params[1]);
            // printf("after Tokens pname: %s\n", proc[i].name);
            if (isRunning(proc[i].pid) == 0) {
            // printf("AFTER isRunning: %s %s\n", params[0], params[1]);
                fprintf(stdout, "%d :", proc[i].jid);
                fflush(stdout);
                int k;
                for (k = 0; k < j; k++) {
                    fprintf(stdout, " %s", params[k]);
                    fflush(stdout);
                }
                fprintf(stdout, "\n");
                fflush(stdout);
            }
            strcpy(proc[i].name, namehold);
            free(namehold);
        }
    }
    return 0;
}

int myw(Process* proc_ptr, char* jid) {
    Process *proc = proc_ptr;
    int jidnum = atoi(jid);

    if (jidnum > jobcount) {
        fprintf(stderr, "Invalid jid %d\n", jidnum);
        fflush(stderr);
        return 1;
    }

    int i = 0;
    while (proc[i].jid != jidnum) {
        i++;
        if (i > 1000) {
            fprintf(stderr, "Invalid jid %d\n", jidnum);
            fflush(stderr);
            return 1;
        }
    }
    if (proc[i].background == 0) {
        fprintf(stderr, "Invalid jid %d\n", jidnum);
        fflush(stderr);
        return 1;
    }

    int status;
    struct timeval start, end;
    long elapsedTime;

    gettimeofday(&start, NULL);
    waitpid(proc[i].pid, &status, WUNTRACED);
    gettimeofday(&end, NULL);

    elapsedTime = (end.tv_sec - start.tv_sec) * 1000000;      // sec to ms
    fprintf(stdout, "%ld : Job %d terminated\n", elapsedTime, proc[i].jid);
    fflush(stdout);
    return 0;
}

int isRunning(int pid) {
    int jpid = pid;
    int status;
    int endID;
    endID = waitpid(jpid, &status, WNOHANG|WUNTRACED);

    if (endID == 0) {  // child is still running
       return 0;
    }
    return 1;  // child has ended
}

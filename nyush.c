#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "functions.h"
#define MAX_WORDS 1000

//Shell Program Explained
//https://www.youtube.com/watch?v=ubt-UjcQUYg

struct job{
    pid_t pid;
    char *command;
};
pid_t* current_child_pid;
char* current_child_command;
int* current_status;
struct job *jobs;
int num_jobs = 0;

void add_job(pid_t pid, char *command) {
    //printf("adding command: %s\n",command);
    jobs[num_jobs].pid = pid;
    jobs[num_jobs].command = command;
    num_jobs++;
}

void remove_job(int index){
    // remove job from list
    for (int i = index; i < num_jobs - 1; i++) {
        jobs[i] = jobs[i+1];
    }
    num_jobs--;
}

void resume_job(int index) {
    *current_child_pid = jobs[index].pid;
    current_child_command = malloc(strlen(jobs[index].command) + 1);
    strcpy(current_child_command, jobs[index].command);
    kill(jobs[index].pid, SIGCONT);
    waitpid(jobs[index].pid, current_status, WUNTRACED);
}

struct job *find_job(int index) {
    if (index >= num_jobs || index < 0) {
        return NULL;
    }
    return &jobs[index-1];
}

void print_jobs() {
    for (int i = 0; i < num_jobs; i++) {
        printf("[%d] %s\n", i+1, jobs[i].command);
    }
}

void handle_terminate() {
    if (*current_child_pid > 0) {
        kill(*current_child_pid, SIGINT);
        kill(getpid(), SIGCONT);
    }
    //else do nothing
}

void handle_stop() {
    //printf("*current_child_pid: %d\n",*current_child_pid);
    if (*current_child_pid > 0) {
        //printf("stopping command: %s\n",current_child_command);
        //add_job(*current_child_pid,current_child_command);
        kill(*current_child_pid, SIGTSTP);
        kill(getpid(), SIGCONT);
    }
    //else do nothing
}

void handle_child() {
    if (WIFSTOPPED(*current_status)) {
        add_job(*current_child_pid,current_child_command);
    }
    else {
        //check to remove process from jobs
        for (int i = 0; i < num_jobs; i++){
            if(jobs[i].pid == *current_child_pid){
                remove_job(i);
                num_jobs --;
            }
        }
    }
}

int main() {
    setenv("PATH", "/bin", 1);

    jobs = malloc(100 * sizeof(struct job));
    current_child_pid = malloc(sizeof(pid_t));
    current_child_command = malloc(MAX_WORDS * sizeof(char));
    current_status = malloc(sizeof(int));
    *current_child_pid = -1;
    current_child_command = NULL;

    //get root directory
    char* root = malloc(128*sizeof(char));
    getcwd(root, 128);
    root = basename(root);

    // int* loop = malloc(sizeof(int));
    // *loop = 1;


    signal(SIGTSTP, handle_stop);
    signal(SIGCHLD, handle_child);
    signal(SIGINT, handle_terminate);
    //signal(SIGQUIT, handle_terminate);

    while(1){
        // *current_child_pid = 0;
        // current_child_command = NULL;
        //print the prompt
        print_prompt();
        fflush(stdout);

        //get input from user
        char buffer[MAX_WORDS];
        //fgets(buffer, sizeof(buffer), stdin);
        //check for built-in commands
        if (fgets(buffer, MAX_WORDS, stdin) == NULL) {
            break;
        }
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strncmp(buffer, "cd", 2) == 0){
            if (buffer[3] == '\0') {
                fprintf(stderr, "Error: invalid command\n");
                continue;
            } else if (chdir(buffer + 3) != 0) {
                fprintf(stderr, "Error: invalid directory\n");
                continue;
            }
        }
        else if (strncmp(buffer, "jobs", 4) == 0){
            //print jobs
            print_jobs();
        }
        else if (strncmp(buffer, "fg", 2) == 0){
            //recover
            if (buffer[3] == '\0') {
                fprintf(stderr, "Error: invalid command\n");
            } else {
                int index = atoi(buffer + 3) - 1;
                struct job *job = find_job(index);
                if (job == NULL) {
                    fprintf(stderr, "Error: Invalid job\n");
                } else {
                    resume_job(index);
                }
            }
        }
        else if (strncmp(buffer, "exit", 4) == 0){
            if (num_jobs > 0){
                fprintf(stderr, "Error: there are suspended jobs\n");
                continue;
            }
            else if (buffer[4]!='\0'){
                fprintf(stderr, "Error: invalid command\n");
                continue;
            }
            else{
                //break loop
                //*loop = 0;
                exit(0);
            }
        }
        else if(buffer[0]){    //look for commands if input is not empty
            //run_command(buffer,current_child_pid);
            current_child_command = malloc(strlen(buffer) + 1);
            strcpy(current_child_command, buffer);
            parse_and_run(buffer);
        }
    }
}

#ifndef NYUSH_H
#define NYUSH_H

extern pid_t* current_child_pid;
extern char* current_child_command;
extern int* current_status;
extern struct job *jobs;
extern int num_jobs;

void add_job(pid_t pid, char *command);
void remove_job(int index);
void resume_job(int index);
struct job *find_job(int index);
void print_jobs();
void handle_terminate();
void handle_stop();
void handle_child();
#endif

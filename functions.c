#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "nyush.h"

#define MAX_WORDS 1000

struct command {
    int argc;
    char* args[MAX_WORDS];
    struct command* next;
    char* file_left;
    char* file_right;
    int prev_pipe;
    int append;
};

bool is_symbol(char* str) {
    if (strcmp(str, "|") == 0 ||
        strcmp(str, "<") == 0 ||
        strcmp(str, ">") == 0 ||
        strcmp(str, ">>") == 0) {
        return true;
    }
    return false;
}

void print_prompt() {
    //print the prompt
    printf("[nyush ");

    //get current working directory
    char cwd[128];
    getcwd(cwd, sizeof(cwd));
    char* baseName = basename(cwd);

    // if (strncmp(baseName, root, strlen(root)) == 0) {
    //     // Replace the root directory with "/"
    //     char slash[strlen(baseName)+1];
    //     slash[0] = '/';
    //     strcat(slash,baseName + strlen(root));
    //     printf("%s", slash);
    // }
    // else{
        printf("%s", baseName);
    //}

    printf("]$ ");
}

//from buffer to struct command
void run_command(char* buffer, pid_t* current_child_pid) {
    char* word = strtok(buffer, " ");
    //parse commands into array of strings
        char* arguments[10];
        char* command = word;
        arguments[0] = word;
        int i = 1;
        while (command != NULL) {
            command = strtok(NULL, " ");
            arguments[i] = command;
            i++;
        }
        arguments[i] = NULL;
        
        pid_t pid = fork();
        int status;
        //printf("current process: %d\n",pid);
        if (pid!=0){
            waitpid(pid, &status, WUNTRACED);
        }
        else{
            *current_child_pid = getpid();
            //execute
            execvp(word, arguments);
            perror("execv failed");
        }
}

void print_command(struct command* to_print){
    printf("argc: %d\n",to_print->argc);
    printf("args: \n");
    for (int i = 0; i < to_print->argc; i++){
        printf("%d: %s\n",i,to_print->args[i]);
    }
    printf("next: %p\n",(void*)to_print->next);
    printf("file_left: %s\n",to_print->file_left);
    printf("file_right: %s\n",to_print->file_right);
    printf("append: %d\n",to_print->append);
    printf("\n");
}

struct command* pipe_command(struct command* command_list, pid_t* current_child_pid){
    int fd[2];

    if (pipe(fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    pid_t pid1 = fork();
    *current_child_pid = pid1;
    if (pid1 == 0) {
        //print_command(command_list);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(command_list->args[0], command_list->args);
    }

    command_list = command_list->next;

    pid_t pid2 = fork();
    *current_child_pid = pid2;
    if (pid2 == 0) {
        //print_command(command_list);
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        execvp(command_list->args[0], command_list->args);
    }
    close(fd[1]);
    close(fd[0]);
    waitpid(pid1,NULL,WUNTRACED);
    waitpid(pid2,NULL,WUNTRACED);
    return command_list->next;
}

void redirect_input(char* file){
    int in_fd = open(file, O_RDONLY);
    if (in_fd == -1) {
        fprintf(stderr, "Error: invalid file\n");
        exit(EXIT_FAILURE);
    }
    dup2(in_fd, STDIN_FILENO);
    close(in_fd);
}

void redirect_output(char* file, int append){
    int out_fd;
    if (append) {
        out_fd = open(file, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    } else {
        out_fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    }
    if (out_fd == -1) {
        fprintf(stderr, "Error: Invalid file\n");
        exit(EXIT_FAILURE);
    }
    dup2(out_fd, STDOUT_FILENO);
    close(out_fd);
}

void execute_commands(struct command* command_list) {
   // int fd[2];
    pid_t pid;
    while (command_list != NULL) {
        //print_command(command_list);
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // child process
            if (command_list != NULL && command_list->prev_pipe == 1) {
                //redirect input from pipe
                //already handled, skip
                command_list = command_list->next;
                command_list = command_list->next;
                continue;
            }
            if (command_list != NULL && command_list->next != NULL) {
                // redirect output to pipe
                command_list = pipe_command(command_list, current_child_pid);
                command_list = command_list->next;
                continue;
            }
            //printf("execvp(command_list->args[0], command_list->args);\n");
            if (execvp(command_list->args[0], command_list->args) < 0){
                fprintf(stderr, "Error: invalid program\n");
            }
            exit(EXIT_FAILURE);
        } else {
            // parent process
            *current_child_pid = pid;
            waitpid(pid, current_status, WUNTRACED);
        }
        command_list = command_list->next;
    }
}

void parse_and_run(char* buffer) {
    char* token = strtok(buffer, " ");
    if (token == NULL || is_symbol(token)){
        fprintf(stderr, "Error: invalid command\n");
    }
    struct command* current_command = (struct command*) malloc(sizeof(struct command));
    struct command* list_head = current_command;
    while (token != NULL) {
        if (strcmp(token, "|") == 0) {
            // create new command node and add to end of list
            struct command* new_command = (struct command*) malloc(sizeof(struct command));
            new_command->next = NULL;
            new_command->file_left = NULL;
            new_command->file_right = NULL;
            new_command->prev_pipe = 1;
            new_command->append = 0;
            if (current_command == NULL) {
                current_command = new_command;
            } else {
                current_command->next = new_command;
                current_command = new_command;
            }
            // set type and advance token
            token = strtok(NULL, " ");
            if (token == NULL || is_symbol(token)){
                fprintf(stderr, "Error: invalid command\n");
                break;
            }
        } else if (strcmp(token, "<") == 0) {
            // set input file for current command and advance token
            redirect_input(strtok(NULL, " "));
            token = strtok(NULL, " ");
        } else if (strcmp(token, ">") == 0) {
            // set output file for current command and advance token
            redirect_output(strtok(NULL, " "),0);
            token = strtok(NULL, " ");
            //has to be last
            break;
        } else if (strcmp(token, ">>") == 0) {
            // set output file for current command and advance token
            redirect_output(strtok(NULL, " "),1);
            token = strtok(NULL, " ");
            //has to be last
            break;
        } else {
            // add argument to current command and advance token
            current_command->args[current_command->argc] = token;
            current_command->argc++;
            token = strtok(NULL, " ");
        }
    }
    print_command(current_command);
    execute_commands(list_head);
    //pipe_command(list_head);
}




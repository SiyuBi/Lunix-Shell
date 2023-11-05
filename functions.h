#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#define MAX_WORDS 1000
//struct
struct command;
// struct Command {
//     char* type;
//     struct Command* left;
//     struct Command* right;
// };

void print_prompt();
void run_command(char* buffer, pid_t* current_child_pid);
void parse_and_run(char* buffer);
void pipe_command(struct command* command_list);
// void free_command(struct Command* cmd);
// void pipe_command(struct Command* cmd);

//void loop_command(command_t *cmds, int n_cmds);

//helper function
bool is_symbol(char* str);
//struct Job;


#endif

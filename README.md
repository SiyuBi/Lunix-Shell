# NYU Shell (nyush)

NYU Shell (nyush) is a simple Unix shell program written in C. It demonstrates basic shell functionalities including executing commands, handling signals, and managing processes.

## Features

- Execution of basic Unix commands
- Signal handling for `SIGTSTP`, `SIGCHLD`, and `SIGINT`
- Job management with basic job control
- Input/output redirection and piping between commands

## Compilation

The project can be compiled using the provided Makefile:

```sh
make
```

This will produce the `nyush` executable.

## Usage

Run the shell by executing the compiled binary:

```sh
./nyush
```

Once running, `nyush` will present a prompt where you can type commands.

## Input/Output Redirection and Piping

- Redirection and piping are supported, allowing you to direct the output of commands to files or chain multiple commands.

## Job Control

- The shell supports basic job control, allowing you to bring jobs to the foreground and background.

## Files

- `Makefile`: Contains the build instructions for the project.
- [`nyush.c`](https://github.com/SiyuBi/nyush/blob/main/nyush.c): The main shell program.
- [`functions.c`](https://github.com/SiyuBi/nyush/blob/main/functions.c): Contains the implementation of shell functionalities.
- [`functions.h`](https://github.com/SiyuBi/nyush/blob/main/functions.h): Header file for function declarations.

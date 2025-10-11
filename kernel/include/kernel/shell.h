#ifndef KERNEL_SHELL_H
#define KERNEL_SHELL_H

/**
 * Initialize and run the interactive shell
 *
 * Starts the shell's Read-Eval-Print Loop (REPL), which continuously:
 * - Displays a prompt
 * - Reads user input
 * - Parses and executes commands
 * - Repeats
 *
 */
void init_shell(void);

/* Functions for testing - not part of the public API */
#ifdef TEST
/**
 * Get the number of registered built-in commands
 *
 * @return Count of built-in commands
 */
int shell_num_builtins(void);

/**
 * Execute a shell command
 *
 * @param args Null-terminated array of command and arguments
 * @return 1 to continue shell loop, 0 to exit
 */
int shell_execute(char **args);

/**
 * Parse a command line into tokens
 *
 * @param line Input line to parse (will be modified)
 * @return Null-terminated array of tokens, or NULL on error
 */
char** parse_line(char *line);

/**
 * Built-in command: clear screen
 *
 * @param args Command arguments (unused)
 * @return 1 to continue shell loop
 */
int shell_clear(char** args);

/**
 * Built-in command: show help
 *
 * @param args Command arguments (unused)
 * @return 1 to continue shell loop
 */
int shell_help(char** args);
#endif

#endif

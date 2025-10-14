/**
 * Olympos Shell - Simple command-line interface for the kernel
 *
 * This is a basic Unix-like shell implementation that runs in kernel mode. It provides an interactive command-line
 * interface for executing built-in commands and exploring the operating system.
 *
 * Inspired by: https://brennan.io/2015/01/16/write-a-shell-in-c/
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include <kernel/kheap.h>
#include <kernel/tty.h>

#define SHELL_TOK_BUFSIZE   64          /* Max number of tokens per command */
#define SHELL_RL_BUFSIZE    1024        /* Max characters per input line */
#define SHELL_TOK_DELIM     " \t\r\n\a" /* Delimiters for tokenization */

/* Forward declarations for built-in command handlers */
int shell_clear(char** args);
int shell_help(char** args);

/* Built-in command registry
 * To add a new command:
 * 1. Add command name to builtin_str[]
 * 2. Add function pointer to builtin_func[]
 * 3. Implement the handler function with signature: int cmd(char **args)
 */
char* builtin_str[] = {"clear", "help"};
int (*builtin_func[]) (char**) = {&shell_clear, &shell_help};

/**
 * Returns the number of built-in commands
 *
 * @return Count of registered built-in commands
 */
int shell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

/**
 * Built-in command: clear
 *
 * Clears the screen by reinitializing the terminal, which resets the
 * VGA text buffer and cursor position.
 *
 * @param args Command arguments (unused)
 * @return 1 to continue shell loop
 */
int shell_clear(char **args) {
    (void) args;  // Unused parameter
    terminal_initialize();
    return 1;
}

/**
 * Built-in command: help
 *
 * Displays a list of available built-in commands.
 *
 * @param args Command arguments (unused)
 * @return 1 to continue shell loop
 */
int shell_help(char **args) {
    (void) args;  // Unused parameter
    printf("Available commands:\n");
    for (int i = 0; i < shell_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    return 1;
}

/**
 * Execute a command
 *
 * Looks up the command in the built-in command table and executes it if found.
 * If the command doesn't exist, prints an error message.
 *
 * @param args Null-terminated array of command and arguments
 * @return 1 to continue shell loop, 0 to exit
 */
int shell_execute(char **args) {
    // Empty command (user just pressed Enter)
    if (args[0] == NULL) {
        return 1;
    }
    // Search for built-in command
    for (int i = 0; i < shell_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    printf("%s: command not found\n", args[0]);
    return 1;
}

/**
 * Read a line of input from the keyboard
 *
 * @return Pointer to allocated string containing the input line,
 *         or NULL if allocation fails. Caller must free with kfree().
 */
char* input_line(void) {
    int bufsize = SHELL_RL_BUFSIZE, position = 0;
    char *buffer = (char*) kmalloc(sizeof(char) * bufsize);

    if (!buffer) {
        printf("[FAILED] input_line: buffer allocation error\n");
        return NULL;
    }

    while (1) {
        int c = getchar();
        if (c == '\n') {					// User pressed Enter - return the complete line
            putchar(c);
            buffer[position] = '\0';
            return buffer;
        }
        if (c == '\b') {
            if (position > 0) {				// Backspace: delete previous character if it exists
                putchar('\b');
                buffer[--position] = '\0';
            }
        }
        else {
            if (position >= bufsize - 1) {  // Buffer overflow protection: leave room for null terminator
                continue;                   // Buffer is full, silently ignore additional input
            }
            buffer[position++] = c;         // Add character to buffer and echo to screen
            putchar(c);
        }
    }

}

/**
 * Parse command line into arguments
 *
 * Splits the input line into tokens (command and arguments) using whitespace as delimiters.
 *
 * For example:
 *   Input:  "help   arg1  arg2"
 *   Output: ["help", "arg1", "arg2", NULL]
 *
 * @param line Input string to parse (will be modified by strtok)
 * @return Null-terminated array of string pointers. Caller must free with kfree().
 */
char** parse_line(char *line) {
    int bufsize = SHELL_TOK_BUFSIZE, position = 0;
    char** tokens = (char**) kmalloc(bufsize * sizeof(char*)), *token;

    if (!tokens) {
        printf("[FAILED] parse_line: tokens allocation error\n");
        return NULL;
    }
    token = strtok(line, SHELL_TOK_DELIM);
    while (token != NULL) {
        tokens[position++] = token;
        // TODO: Handle case where position >= bufsize (too many arguments)
        // Subsequent calls: pass NULL to continue tokenizing same string
        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
 * Main shell loop - Read-Eval-Print Loop (REPL)
 *
 * This is the main entry point for the shell. It runs an infinite loop that:
 * 1. Prints a prompt ("$ ")
 * 2. Reads a line of input from the user
 * 3. Parses the line into command and arguments
 * 4. Executes the command
 * 5. Frees allocated memory
 * 6. Repeats
 *
 * The loop continues until a command returns 0 (though currently no built-in commands do this - the shell runs forever)
 */
void init_shell(void) {
    char *line, **args;
    int status;

    do {
        // Print prompt
        printf("$ ");
        // Read command line
        line = input_line();
        // Parse into arguments
        args = parse_line(line);
        // Execute command
        status = shell_execute(args);
        // Clean up allocated memory
        kfree(line);
        kfree(args);
    } while (status);

}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define MAX_SIZE_CMD 256
#define MAX_SIZE_ARG 16
#define MAX_PATHS 16

char cmd[MAX_SIZE_CMD];          // string holder for the command
char *argv[MAX_SIZE_ARG];        // an array for command and arguments
char *paths[MAX_PATHS];          // array to store paths for the PATH command
pid_t pid;                       // global variable for the child process ID
char i;                          // global for loop counter

// Function Prototypes
void get_cmd();                  // get command string from the user
void convert_cmd();              // convert the command string to the required format by execvp()
void itush();                    // start the shell
void log_handle(int sig);        // signal handler to add log statements
void handle_exit();              // handles the "exit" command
void handle_cd(char **args);     // handles the "cd" command
void handle_path(char **args);   // handles the "path" command
void execute_command(char **args);  // executes external commands

int main() {
    // tie the handler to the SIGCHLD signal
    signal(SIGCHLD, log_handle);

    // start the shell
    itush();

    return 0;
}

void convert_cmd() {
    // split string into argv and handle '&' for background commands
    char *ptr;
    i = 0;
    ptr = strtok(cmd, " ");
    while(ptr != NULL) {
        argv[i] = ptr;
        i++;
        ptr = strtok(NULL, " ");
    }
    
    argv[i] = NULL;  // Null-terminate the argv array
}

void itush() {
    while(1) {
        // get the command from user
        get_cmd();

        // bypass empty commands
        if (!strcmp("", cmd)) continue;

        // fit the command into *argv[]
        convert_cmd();

        int background = 0;
        
        // Check if the last argument is "&"
        if (i > 0 && !strcmp(argv[i-1], "&")) {
            background = 1;
            argv[i-1] = NULL;  // Remove the "&" from the arguments
        }

        // Fork and execute the command
        pid = fork();
        if (pid == -1) {
            printf("Failed to create a child\n");
        } else if (pid == 0) {
            // Child process
            execvp(argv[0], argv);
            perror("Error executing command");
            exit(1);  // Exit if exec fails
        } else {
            // Parent process
            if (!background) {
                waitpid(pid, NULL, 0);  // Wait for the child process unless it's background
            }
        }
    }
}


// Get command from the user
void get_cmd() {
    printf("itush>\t");
    fgets(cmd, MAX_SIZE_CMD, stdin);
    // Remove trailing newline
    if ((strlen(cmd) > 0) && (cmd[strlen(cmd) - 1] == '\n')) {
        cmd[strlen(cmd) - 1] = '\0';
    }
}



// Handle the cd command (change directory)
void handle_cd(char **args) {
    if (args[1] == NULL || args[2] != NULL) {
        printf("Error: Invalid usage of cd\n");
    } else {
        printf("Changing directory to: %s\n", args[1]);  // Debug print
        if (chdir(args[1]) != 0) {
            perror("Error");
        }
    }
}

// Handle the path command (modify executable paths)
void handle_path(char **args) {
    for (int i = 0; i < MAX_PATHS; i++) {
        paths[i] = NULL;
    }

    int i = 1;
    while (args[i] != NULL && i < MAX_PATHS) {
        paths[i - 1] = strdup(args[i]);  // Store the paths
        i++;
    }
}

// Execute external commands
void execute_command(char **args) {
    pid_t pid = fork();  // Create a new process

    if (pid == 0) {  // Child process
        execvp(args[0], args);  // Execute the command
        perror("Error");        // If execvp fails
        exit(1);                // Exit child process
    } else if (pid < 0) {  // Fork failed
        perror("Error");
    } else {  // Parent process
        wait(NULL);  // Wait for the child process to finish
    }
}

// Log the termination of child processes
void log_handle(int sig) {
    FILE *pFile;
    pFile = fopen("log.txt", "a");
    if (pFile == NULL) {
        perror("Error opening file.");
    } else {
        fprintf(pFile, "[LOG] child process terminated.\n");
        fclose(pFile);
    }
}


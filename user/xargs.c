#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

// Helper function to handle argument processing
int process_argument(char *buf, char **arg_s, char **arg_e, char *x_argv[], int *x_argc, int argc) {
    if (*x_argc == MAXARG - 1) { // If full
        fprintf(2, "xargs: too many arguments\n");
        *arg_s = *arg_e = buf; // Reset the buffer
        **arg_e = '\0'; // Null-terminate the buffer
        // Skip remaining characters until the next newline
        while (**arg_e != '\n') {
            if (read(0, *arg_e, sizeof(char)) <= 0) return -1; // Handle EOF or read error
        }
        return 0; // Indicate buffer reset
    } else { // Not full
        **arg_e = '\0'; // Null-terminate the argument
        x_argv[(*x_argc)++] = *arg_s; // Add argument to x_argv
        (*arg_e)++; // Move end pointer forward
        *arg_s = *arg_e; // Move start pointer forward
        return 1; // Indicate successful argument processing
    }
}

int main(int argc, char *argv[]) {
    char *x_argv[MAXARG];
    char buf[512];
    int x_argc = argc - 1;
    char *arg_s = buf;
    char *arg_e = buf;

    // Copy the initial arguments to x_argv
    for (int i = 1; i < argc; i++) {
        x_argv[i - 1] = argv[i];
    }

    // Read input and process arguments
    while (read(0, arg_e, sizeof(char))) {
        if (*arg_e == '\n' || *arg_e == ' ') {
            if (process_argument(buf, &arg_s, &arg_e, x_argv, &x_argc, argc) == -1) break;
            
            // Fork and exec after newline
            if (*arg_e == '\n') {
                if (fork() == 0) {
                    exec(x_argv[0], x_argv);
                }
                // Reset buffer and argument count after executing the command
                arg_s = arg_e = buf;
                x_argc = argc - 1;
            }
        } else {
            arg_e++; // Accumulate characters for the current argument
        }
    }

    // Handle the remaining buffer after EOF
    if (buf != arg_e) {
        process_argument(buf, &arg_s, &arg_e, x_argv, &x_argc, argc);
        if (fork() == 0) {
            exec(x_argv[0], x_argv);
        }
    }

    wait(0); // Wait for all child processes to finish
    exit(0);
}
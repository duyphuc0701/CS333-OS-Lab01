#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

// Helper function to handle argument processing
int process_argument(char *buffer, char **arg_start, char **arg_end, char *command_args[], int *arg_count, int initial_arg_count) {
    if (*arg_count == MAXARG - 1) {
        fprintf(2, "xargs: too many arguments\n");
        *arg_start = *arg_end = buffer; 
        **arg_end = '\0';
        // Skip remaining characters until the next newline
        while (**arg_end != '\n') {
            if (read(0, *arg_end, sizeof(char)) <= 0) return -1; // Handle EOF or read error
        }
        return 0; // Indicate buffer reset
    } else { // Not full
        **arg_end = '\0'; // Null-terminate the argument
        command_args[(*arg_count)++] = *arg_start; // Add argument to command_args
        (*arg_end)++; // Move end pointer forward
        *arg_start = *arg_end; // Move start pointer forward
        return 1; // Indicate successful argument processing
    }
}

int main(int argc, char *argv[]) {
    char *command_args[MAXARG];
    char input_buffer[512];
    int current_arg_count = argc - 1;
    char *arg_start = input_buffer;
    char *arg_end = input_buffer;

    // Copy the initial arguments to command_args
    for (int i = 1; i < argc; i++) {
        command_args[i - 1] = argv[i];
    }

    // Read input and process arguments
    while (read(0, arg_end, sizeof(char))) {
        if (*arg_end == '\n' || *arg_end == ' ') {
            if (process_argument(input_buffer, &arg_start, &arg_end, command_args, &current_arg_count, argc) == -1) break;
            
            // Fork and exec after newline
            if (*arg_end == '\n') {
                if (fork() == 0) {
                    exec(command_args[0], command_args);
                }
                // Reset buffer and argument count after executing the command
                arg_start = arg_end = input_buffer;
                current_arg_count = argc - 1;
            }
        } else {
            arg_end++; // Accumulate characters for the current argument
        }
    }

    // Handle the remaining buffer after EOF
    if (input_buffer != arg_end) {
        process_argument(input_buffer, &arg_start, &arg_end, command_args, &current_arg_count, argc);
        if (fork() == 0) {
            exec(command_args[0], command_args);
        }
    }

    wait(0); // Wait for all child processes to finish
    exit(0);
}
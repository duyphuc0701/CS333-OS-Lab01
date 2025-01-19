#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

// Function to fork a child process and execute the given program with arguments
void execute_program(char* program, char** arguments) {
    if (fork() > 0) {
        wait(0);
    } else {
        if (exec(program, arguments) == -1) {
            fprintf(2, "xargs: Error executing %s\n", program);
            exit(1);
        }
    }
}

// Function to implement the xargs functionality
void process_input_and_execute(char** initial_args, int initial_arg_count, char* program_name) {
    // Buffer to store input read from stdin
    char input_buffer[1024];
    // Array to store arguments for the program 
    char *arguments[MAXARG];
    // Index to track position in the input buffer
    int buffer_index = 0;

    // Read input line by line from stdin
    while (read(0, input_buffer + buffer_index, 1) == 1) {
        if (buffer_index >= 1024) {
            fprintf(2, "xargs: Input too long.\n");
            exit(1);
        }

        // Check for end of a line
        if (input_buffer[buffer_index] == '\n') {
            input_buffer[buffer_index] = '\0';

            memmove(arguments, initial_args, sizeof(*initial_args) * initial_arg_count);

            // Add the new argument from input
            int arg_index = initial_arg_count;
            if (arg_index == 0) {
                arguments[arg_index] = program_name;
                arg_index++;
            }
            arguments[arg_index] = malloc(sizeof(char) * (buffer_index + 1));
            memmove(arguments[arg_index], input_buffer, buffer_index + 1);

            // Null-terminate the arguments array
            arguments[arg_index + 1] = 0;

            // Execute the program with the constructed arguments
            execute_program(program_name, arguments);

            free(arguments[arg_index]);

            // Reset the buffer index for the next line
            buffer_index = 0;
        } else {
            buffer_index++;
        }
    }
}

int main(int argc, char* argv[]) {
    // Default program name to "echo" if none is provided
    char *default_program = "echo";
    if (argc >= 2) {
        default_program = argv[1];
    }

    // Call the xargs functionality with the provided arguments
    process_input_and_execute(argv + 1, argc - 1, default_program);

    exit(0);
}
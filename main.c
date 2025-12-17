#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "shell.h"

char *read_cmd(void) {
    char buf[1024];      // Buffer to read one line at a time
    char *ptr = NULL;    // Final command buffer that will be returned
    int ptrlen = 0;      // Total length of command accumulated so far

    // Read lines until EOF (Ctrl+D) or error
    while (fgets(buf, 1024, stdin)) {
        int buflen = strlen(buf);  // Length of the line just read

        // First line: allocate memory for this line + null terminator
        // Subsequent lines: reallocate to make room for additional line
        if(!ptr) {
            ptr = malloc(buflen+1);
        } else {
            char *ptr2 = realloc(ptr, ptrlen+buflen+1);

            if(ptr2) {
                ptr = ptr2;
            } else {
                free(ptr);
                ptr = NULL;
            }
        }

        // If allocation failed, print error and return
        if(!ptr) {
            fprintf(stderr, "error: failed to alloc buffer: %s\n", strerror(errno));
            return NULL;
        }

        // Append the new line to our accumulated command buffer
        strcpy(ptr+ptrlen, buf);

        // Check if this line ends with a newline character
        if(buf[buflen-1] == '\n') {
            // If line is just "\n" or doesn't end with backslash, command is complete
            if(buflen == 1 || buf[buflen -2] != '\\') {
                return ptr;
            }

            // Line ends with backslash-newline: remove them and continue reading
            ptr[ptrlen+buflen-2] = '\0';  // Replace backslash with null terminator
            buflen -= 2;                   // Adjust line length (remove \ and \n)
            print_prompt2();               // Show continuation prompt
        }

        // Update total length of command accumulated
        ptrlen += buflen;
    }
    
    // EOF reached (user pressed Ctrl+D)
    // Free any allocated memory and return NULL to signal end of input
    if (ptr) {
        free(ptr);
    }
    return NULL;
}

int main(int argc, char **argv) {
    char *cmd;
    
    do {
        print_prompt1();      // Display shell prompt

        cmd = read_cmd();     // Read a command (may span multiple lines)

        // NULL means EOF (Ctrl+D) was pressed
        if(!cmd) {
            exit(EXIT_SUCCESS);
        }

        // Skip empty commands (just Enter key)
        if(cmd[0] == '\0' || strcmp(cmd, "\n") == 0) {
            free(cmd);
            continue;
        }

        // Check for exit command
        if(strcmp(cmd, "exit\n") == 0) {
            free(cmd);
            break;
        }

        // Echo the command back (for testing)
        printf("%s\n", cmd);
        free(cmd);           // Free the command buffer after processing
    } while(1);

    exit(EXIT_SUCCESS); 
}


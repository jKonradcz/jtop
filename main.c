#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    unsigned int pid;
    char* cmdline;
    // TODO: add memory field
    // TODO: add cpu field
} proc;

int main() {
    // grab the list of files in /proc
    system("ls /proc > list-new.txt");

    // open the list 
    FILE *file = fopen("list-new.txt", "r");
    if (file == NULL){
        printf("Trouble loading the /proc list.");
        return 1;
    }

    // allocate mem for file of text in file
    char* pid = NULL;
    size_t line_buffer = 0;

    // prep the process array
    size_t array_size = 10;

    // allocate mem for the array
    proc* array = malloc(array_size * sizeof(proc));

    // number of processes
    size_t proc_number = 0;

    // loop through the list
    while (getline(&pid, &line_buffer, file) != -1) {

        // check each line and each part of the line if name contains number
        for (char* point = pid; *point != '\0'; point++) {
            if (isdigit(*point)) {
                
                // remove whitespace from the pid
                pid[strcspn(pid, "\r\n")] = '\0';

                // alloc mem for the procpath
                int size = snprintf(NULL, 0, "/proc/%s/cmdline", pid);
                char* procpath = (char*)malloc(size + 1);
                snprintf(procpath, size + 1, "/proc/%s/cmdline", pid);

                // open the cmdline file (process path/name)
                FILE *cmdline_file = fopen(procpath, "r");
                if (cmdline_file == NULL) {
                    // if empty, skip this PID
                    free(procpath);
                    continue;
                }

                // read the cmdline (process name)
                char* cmdline = NULL;
                size_t cmdline_buffer = 0;
                getline(&cmdline, &cmdline_buffer, cmdline_file);
                fclose(cmdline_file);

                // check if the cmdline is not empty 
                if (cmdline[0] == '\0') {
                    free(cmdline);
                    free(procpath);
                    continue;
                }

                // find the end of the path and remove the rest (arguments etc) 
                int is_valid = 1;
                for (char* c = cmdline; *c != '\0'; c++) {
                    if (*c == ' ') {
                        *c = '\0';
                        break;
                    }

                    // and includes only ascii characters (doesnt work right now) < < < < < < <
                    if (*c < 0 || *c > 127) {
                        is_valid = 0;
                        break;
                    }
                }

                // TODO: read the /proc/[pid]/statm file to get the memory usage / possible cpu too
                // TODO: assign the values to the proc struct fields

                // check if there is space in the array
                if (proc_number == array_size) {
                    // if no space, double the size of the array
                    array_size *= 2;
                    array = realloc(array, array_size * sizeof(proc));
                }

                // add the proc pair to the array
                array[proc_number].pid = atoi(pid);
                array[proc_number].cmdline = cmdline;  
                proc_number++;

                free(procpath);
                break;
            }
        }
    }

    // TODO: sort the array by memory usage

    // open the output file
    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        printf("Could not open output file.\n");
        return 1;
    }

    // write down the array
    for (size_t i = 0; i < proc_number; i++) {
        fprintf(output_file, "%u %s\n", array[i].pid, array[i].cmdline);
    }
    fclose(output_file);

    // free the array
    for (size_t i = 0; i < proc_number; i++) {
        free(array[i].cmdline);
    }
    free(array);

    return 0;
}
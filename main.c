#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

typedef struct {
    unsigned int pid;
    char* cmdline;
    unsigned long mem;
    /// unsigned int cpu; (CPU not yet, apparently that's more difficult than I thought)
} proc;

int main() {
    // grab the list of files in /proc
    system("ls /proc > list-new.txt");

    // TODO: grab the total memory of the system (#include <sys/sysinfo.h> then ~ totalram * mem_unit)

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

    // loop through the list of processes
    while (getline(&pid, &line_buffer, file) != -1) {

        // check each line and each part of the line if name contains number
        for (char* point = pid; *point != '\0'; point++) {
            if (isdigit(*point)) {
                
                // remove whitespace from the pid
                pid[strcspn(pid, "\r\n")] = '\0';

                // alloc mem for the procpath
                int size = snprintf(NULL, 0, "/proc/%s/cmdline", pid);
                char* procpath = (char*)malloc(size + 1);
                // actually write the path to the procpath
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

                // check if the cmdline is not empty and starts with a '/'
                if (cmdline[0] == '\0' || cmdline[0] != '/') {
                    // if empty or not containing a path, skip this PID
                    free(cmdline);
                    free(procpath);
                    continue;
                }

                // find the end of the path and remove the rest (arguments etc) 
                for (char* c = cmdline; *c != '\0'; c++) {
                    if (*c == ' ') {
                        *c = '\0';
                        break;
                    }
                }

                // we are done with the cmdline, free the path
                free(procpath);

                // load the statm path (this could be simplified by providing both cmdline & statm as arguments, but will do for now)
                size = snprintf(NULL, 0, "/proc/%s/statm", pid);
                procpath = (char*)malloc(size + 1);
                snprintf(procpath, size + 1, "/proc/%s/statm", pid);    /// procpath now contains the path to the statm file
                
                // TODO: read the /proc/[pid]/statm file to get the memory usage / possible cpu too
                FILE* statm_file = fopen(procpath, "r");
                if (statm_file == NULL) {
                    // if empty, skip this PID
                    free(procpath);
                    continue;
                }
                // init the variables, the file includes many values, but we only need the second one
                unsigned long memuse, value1, value2, value3, value4, value5, value6;
                fscanf(statm_file, "%lu %lu %lu %lu %lu %lu %lu", &value1, &memuse, &value2, &value3, &value4, &value5, &value6);
                fclose(statm_file);
                free(procpath);

                // previously i used the "size" part of the statm, but that was most probably virt mem, not resident mem
                // to determine % of mem used, we need to know the total mem of the system and the "resident" from statm
                // for now, i will just store the resident mem in Mb
                
                unsigned long pagesize = sysconf(_SC_PAGESIZE);
                memuse = (memuse * pagesize) / 1048576;

                // check if there is space in the array
                if (proc_number == array_size) {
                    // if no space, double the size of the array
                    array_size *= 2;
                    array = realloc(array, array_size * sizeof(proc));
                }

                // add the proc pair to the array
                array[proc_number].pid = atoi(pid);
                array[proc_number].cmdline = cmdline;
                array[proc_number].mem = memuse;  
                proc_number++;

                break;
            }
        }
    }

    // TODO: sort the array by memory usage
    // for now, just print the total memory usage
    unsigned long total_mem = 0;
    for (size_t i = 0; i < proc_number; i++) {
        total_mem += array[i].mem;
    }
    printf("Total memory usage: %luMb\n", total_mem);

    // open the output file
    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        printf("Could not open output file.\n");
        return 1;
    }

    // write down the array
    for (size_t i = 0; i < proc_number; i++) {
        fprintf(output_file, "%u %s %luMb\n", array[i].pid, array[i].cmdline, array[i].mem);
    }
    fclose(output_file);

    // free the array
    for (size_t i = 0; i < proc_number; i++) {
        free(array[i].cmdline);
    }
    free(array);

    return 0;
}
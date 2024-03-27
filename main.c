#include <stdio.h>          // for the printf, fopen, fclose, getline, fprintf functions
#include <stdlib.h>         // for the malloc, realloc, free, atoi functions and the qsort
#include <string.h>         // for the strcspn, snprintf, strcmp functions 
#include <ctype.h>          // for the isdigit function
#include <unistd.h>         // for the sysconf function > page_size 
#include <sys/sysinfo.h>    // for the sysinfo function > totalram 

typedef struct {
    unsigned int pid;
    char* cmdline;
    unsigned long mem;
    float mempercent;
    /// unsigned int cpu; (CPU not yet, apparently that's more difficult than I thought)
} proc;

// comparison function for qsort (StackOverflow magic, apparently efficient way to sort arrays https://stackoverflow.com/questions/3893937/sorting-an-array-in-c > https://en.cppreference.com/w/c/algorithm/qsort)
// const void* as required by the qsort function
int compare_proc_by_mem(const void* a, const void* b) {
    // typecast from void* to proc*
    proc* proc_a = (proc*)a;
    proc* proc_b = (proc*)b;
    // comparison itself, with the return values -1 and 1 or 0 if equal
    // reverted the order, as I want the largest procs first
    if (proc_a->mem > proc_b->mem) {
        return -1;
    }
    if (proc_a->mem < proc_b->mem) {
        return 1;
    }
    return 0;
}

int main() {
    // grab the list of files in /proc
    // TODO: find another solution omitting the use of BASH, same for the cleanup
    system("ls /proc > proclist.txt");

    // grab the total memory of the system
    struct sysinfo info;
    sysinfo(&info); // saved in info.totalram, in bytes

    // open the list 
    FILE *input_file = fopen("proclist.txt", "r");
    if (input_file == NULL){
        printf("Trouble loading the /proc list.");
        return 1;
    }

    // allocate mem for file of text in file
    char* pid = NULL;
    size_t line_buffer = 0; // has to be in size_t, due to getline.. odd

    // prep the process array
    unsigned int array_size = 10;

    // allocate mem for the array
    proc* array = malloc(array_size * sizeof(proc));

    // number of processes
    unsigned int proc_number = 0;

    // loop through the list, grab a line until there are none(-1)
    while (getline(&pid, &line_buffer, input_file) != -1) {

        // check each line and each char in the line if name contains number
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
                size_t cmdline_buffer = 0; // same thing, size_t because of getline
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

                // TODO: remove the path, keep only the process name (after the last /)

                // we are done with the cmdline, free the path
                free(procpath);

                // load the statm path (this could be simplified by providing both cmdline & statm as arguments, but will do for now)
                size = snprintf(NULL, 0, "/proc/%s/statm", pid);
                procpath = (char*)malloc(size + 1);
                snprintf(procpath, size + 1, "/proc/%s/statm", pid);    /// procpath now contains the path to the statm file
                
                // read the /proc/[pid]/statm file to get the memory usage
                FILE* statm_file = fopen(procpath, "r");
                if (statm_file == NULL) {
                    // if empty, skip this PID
                    free(procpath);
                    continue;
                }
                // init the variables, the file includes many values, but we only need the second one (resident mem)
                float memuse;
                unsigned long value1, value2, value3, value4, value5, value6;
                fscanf(statm_file, "%lu %f %lu %lu %lu %lu %lu", &value1, &memuse, &value2, &value3, &value4, &value5, &value6);
                fclose(statm_file);
                free(procpath);

                // get the page size from the system                
                unsigned long pagesize = sysconf(_SC_PAGESIZE);

                // convert the memory usage to bytes
                memuse = (memuse * pagesize); 

                // calculate the percentage of memory used by the process
                float mempercent = (memuse * 100) / (float)(info.totalram * info.mem_unit); // mem_unit to potentialy convert to bytes

                // check if there is space in the array
                if (proc_number == array_size) {
                    // if no space, double the size of the array
                    array_size *= 2;
                    array = realloc(array, array_size * sizeof(proc));
                }

                // add the proc values to the struct and array
                array[proc_number].pid = atoi(pid);
                array[proc_number].cmdline = cmdline;
                array[proc_number].mem = memuse;
                array[proc_number].mempercent = mempercent;

                proc_number++;
                
                break;
            }
        }
    }

    // close the proclist.txt
    fclose(input_file);

    // qsort magic (fields are: start of the array, number of elements, size in memory of each element and the comparison function)
    qsort(array, proc_number, sizeof(proc), compare_proc_by_mem);

    // for now, just print the total memory usage
    unsigned long memused = 0;
    for (unsigned int i = 0; i < proc_number; i++) {
        memused += array[i].mem;
    }

    // temporary printout of values
    printf("Total memory usage: %luMb\n", memused / 1048576);
    printf("Total memory: %lu Mb\n", info.totalram / 1048576);
    int mempercent = (int)((memused * 100) / info.totalram); // TODO: use this calc in loop on each proc
    printf("Use memory percentage: %d%%\n", mempercent);

    // open the output file
    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        printf("Could not open output file.\n");
        for (unsigned int i = 0; i < proc_number; i++) {
            free(array[i].cmdline);
        }
        free(array);
        free(pid); // free the buffer from getline()
        return 1;
    }
    // temporary solution to write down the array
    for (unsigned int i = 0; i < proc_number; i++) {
        // only print if mempercent is more than 1% 
        if (array[i].mempercent > 0.5) { 
            fprintf(output_file, "%u %s %lub %.2f%%\n", array[i].pid, array[i].cmdline, array[i].mem, array[i].mempercent);
        }
    }
    fclose(output_file);

    // free the array
    for (unsigned int i = 0; i < proc_number; i++) {
        free(array[i].cmdline);
    }

    // cleanup
    
    system("rm proclist.txt"); // temporary solution to clean up after each run
    free(array);
    free(pid); // free the buffer from getline()

    return 0;
}
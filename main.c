#include "jtop.h"

int main(int argc, char **argv) {

    struct sysinfo info;
    char* pid = NULL;
    char* cmdline = NULL;
    proc* array = NULL;
    unsigned int proc_number = 0;
    unsigned int used_proc = 0;
    unsigned long memused = 0;
    int mempercent;

    // grab the total memory of the system
    sysinfo(&info); // saved in info.totalram, in bytes

    gather_proc_info(info, pid, &array, &proc_number, cmdline);

    // qsort magic (array, number of elements, size of each el, comparison fn)
    qsort(array, proc_number, sizeof(proc), compare_proc_by_mem);

    // calculate the memory usage
    calc_mem(&mempercent, &memused, info, &array, &proc_number);

    // TODO: loop to update the window

    // temporary printout of values
    printf("Total memory usage: %luMb\n", memused / 1048576);
    printf("Total memory: %lu Mb\n", info.totalram / 1048576);
    
    // memory percentage with 2 decimals
    printf("Use memory percentage: %d%%\n", mempercent);

    // open the output file
    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        printf("Could not open output file.\n");
        for (unsigned int i = 0; i < proc_number; i++) {
            free(array[i].cmdline);
        }
        free(array);
        // free(pid); // free the buffer from getline()
        return 1;
    }
    // temporary solution to write down the array
    for (unsigned int i = 0; i < proc_number; i++) {
        // only print if mempercent is more than 0.5% 
        if (array[i].mempercent > 0.5) { 
            used_proc++;
            fprintf(output_file, "%u %s %lub %.2f%%\n", array[i].pid, array[i].cmdline, array[i].mem, array[i].mempercent);
        }
    }
    fclose(output_file);

    // prep GUI size
    gui_size gui_size_var;
    gui_size_var.width = window_width;
    gui_size_var.height = line_height * used_proc;
    gui_size_var.used_proc = used_proc;
    gui_size_var.array = array;

    // create the GUI thread
    // debugging comment- printf("Pre-thread print from main, PID: %u, proc: %s, mempercent: %.2f%%\n", array[0].pid, array[0].cmdline, array[0].mempercent);
    
    pthread_t gui_thread;
    
    pthread_create(&gui_thread, NULL, make_gui_thread, &gui_size_var);

    // joining the threads so the program doesn't exit at the end of the main
    pthread_join(gui_thread, NULL);
    // TODO: function to populate the window with data

    // free the array
    for (unsigned int i = 0; i < proc_number; i++) {
        free(array[i].cmdline);
    }

    // cleanup
    system("rm proclist.txt"); // temporary solution to clean up after each run
    free(array);

    return 0;
}

// functions

int compare_proc_by_mem(const void* a, const void* b) {
    // comparison function for qsort 
    // const void* as required by the qsort function
    // typecast from void* to proc*
    if (a == NULL || b == NULL) {
        return 0;
    }
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

int gather_proc_info(struct sysinfo info, char* pid, proc** array, unsigned int* proc_number, char* cmdline) {
    // grab the list of files in /proc
    // TODO: find another solution omitting the use of BASH, same for the cleanup
    system("ls /proc > proclist.txt");

    // open the list 
    FILE *input_file = fopen("proclist.txt", "r");
    if (input_file == NULL){
        printf("Trouble loading the /proc list.");
        return 1;
    }

    // allocate mem for file of text in file
    size_t line_buffer = 0; // has to be in size_t, due to getline.. odd

    // prep the process array
    unsigned int array_size = 10;
    unsigned int actual_array_size = 0;

    // allocate mem for the array
    *array = malloc(array_size * sizeof(proc));


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
                size_t cmdline_buffer = 0; // same thing, size_t because of getline
                getline(&cmdline, &cmdline_buffer, cmdline_file);
                fclose(cmdline_file);

                // find the end of the path and remove the rest (arguments etc) 
                // changed from ' ' to '-' as the emulated paths had spaces in them
                /*
                for (char* c = cmdline; *c != '\0'; c++) {
                    if (*c == '-') {
                        *c = '\0';
                        break;
                    }
                } */
                cmdline[strcspn(cmdline, "-")] = '\0';


                // we are done with the cmdline, free the path
                free(procpath);

                // enter the cmdline info into the proc, free the cmdline buffer
                // (*array)[*proc_number].cmdline = cmdline;
                // free(cmdline);
                // cmdline = NULL;

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
                if (*proc_number == array_size) {
                    // if no space, double the size of the array
                    array_size *= 2;
                    *array = realloc(*array, array_size * sizeof(proc));
                }

                // add the proc values to the struct and array
                (*array)[*proc_number].pid = atoi(pid);
                (*array)[*proc_number].cmdline = strdup(cmdline);
                // (*array)[*proc_number].cmdline = "bruh";
                (*array)[*proc_number].mem = memuse;
                (*array)[*proc_number].mempercent = mempercent;

                (*proc_number)++;
                
                // debuging at the end of each loop
                // printf("Proc number: %u, memused: %f\n", *proc_number, memuse);

                break;
            }
        }
    }
    free(pid); // free the buffer from getline()
    // close the proclist.txt
    fclose(input_file);
    return 0;
}

int calc_mem(int* mempercent, unsigned long* memused, struct sysinfo info, proc** array, unsigned int* proc_number) {
    for (unsigned int i = 0; i < *proc_number; i++) {
        *memused += (*array)[i].mem;
    }

    // Check if totalram is not 0, if it is, set mempercent to 0 and proceed
        if (info.totalram != 0) {
        *mempercent = (int)((*memused * 100) / info.totalram); 
    } 
    else {
        *mempercent = 0;
    }

}

void* make_gui_thread(void* arg) {
    gui_size* gui_size_var = (gui_size*) arg;
    gui_main(gui_size_var);
    return NULL;
}
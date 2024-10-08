#ifndef JTOP_H
#define JTOP_H


#include <stdio.h>          // for the printf, fopen, fclose, getline, fprintf functions
#include <stdlib.h>         // for the malloc, realloc, free, atoi functions and the qsort
#include <string.h>         // for the strcspn, snprintf, strcmp functions 
#include <ctype.h>          // for the isdigit function
#include <unistd.h>         // for the sysconf function > page_size 
#include <sys/sysinfo.h>    // for the sysinfo function > totalram 
#include <gtk/gtk.h>        // for the UI   
#include <pthread.h>        // for the threads

#define window_width 500    // width of the line in the GUI
#define line_height 20      // height of the line in the GUI


    // struct to hold all the proc info
typedef struct {
    unsigned int pid;
    char* cmdline;
    unsigned long mem;
    float mempercent;
} proc;

    // struct to hold the window dimensions
typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int used_proc;
    proc* array;
} gui_size;

    // struct to hold data for the refresh fn
typedef struct {
    GtkWidget *grid;
    GtkWidget *window;
    proc *array;
} refreshd;

int compare_proc_by_mem(const void* a, const void* b);

int gather_proc_info(struct sysinfo info, char* pid, proc** array, unsigned int* proc_number, char* cmdline);

int filter_proc(proc* array, unsigned int* proc_number, unsigned int* used_proc);

int calc_mem(int* mempercent, unsigned long* memused, struct sysinfo info, proc** array, unsigned int* proc_number);

int gui_main (gui_size* gui_size_var);

void* make_gui_thread(void* arg);

void kill_proc(GtkWidget* widget, gpointer pid);

void populate_grid(GtkWidget* grid, proc* array, unsigned int used_proc);

int clear_array(proc* array, unsigned int proc_number);

void refresh(GtkWidget* grid, gpointer refresh_data);

#endif

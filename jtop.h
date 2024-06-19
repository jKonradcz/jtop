#ifndef JTOP_H
#define JTOP_H


#include <stdio.h>          // for the printf, fopen, fclose, getline, fprintf functions
#include <stdlib.h>         // for the malloc, realloc, free, atoi functions and the qsort
#include <string.h>         // for the strcspn, snprintf, strcmp functions 
#include <ctype.h>          // for the isdigit function
#include <unistd.h>         // for the sysconf function > page_size 
#include <sys/sysinfo.h>    // for the sysinfo function > totalram 
#include <gtk/gtk.h>        // for the UI   


typedef struct {
    unsigned int pid;
    char* cmdline;
    unsigned long mem;
    float mempercent;
    /// unsigned int cpu; (CPU not yet, apparently that's more difficult than I thought)
} proc;


#endif

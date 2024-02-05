#include <stdlib.h>

int main() {
    // grab the list of files in /proc
    system("ls -l /proc > list-new.txt");

    // open new output file

    // allocate mem for file of text in file

    // open the list 

    // loop through the list

        // check each line if name contains number

        // enter each folder, read the cmdline, skip if empty

        // write the pid + cmdline into the output file

    // close the list, rename it to list-old.txt

    return 0; 
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main() {
    // grab the list of files in /proc
    system("ls /proc > list-new.txt");

    // open new output file

    // open the list 
    FILE *file = fopen("list-new.txt", "r");
    if (file == NULL){
        printf("Trouble loading the /proc list.");
        return 1;
    }

    // allocate mem for file of text in file
    char* line = NULL;
    size_t line_buffer = 0;

    // loop through the list
    while (getline(&line, &line_buffer, file) != -1) {

        // check each line and each part of the line if name contains number
        for (char* point = line; *point != '\0'; point++)
            if (isdigit(*point)) {

                // placeholder, for now just print the line
                printf("%s", line);
                // break once the above is done once (i.e. printing the line)
                break;
        }

                // enter each folder, read the cmdline, skip if empty

                // write the pid + cmdline into the output file
            
    }

    // free memory, close the list, rename it to list-old.txt
    free(line);
    fclose(file);
    system("mv list-new.txt list-old.txt");

    return 0; 
}
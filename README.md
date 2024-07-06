JTOP

This Ubuntu utility scans for running processes larger than certain percent of memory, gathers its PID and allows you to KILL said process from one place, without the need to enter the terminal. Main motivation were stuck WINE processes, when trying to game. These would sometimes cumulate enough to slow down the computer, forcing me to search with TOP/HTOP for the culprit. 

It was created as my Final Project for the CS50x. 

Developed on Ubuntu 23.10, but should work on any related Debian distros.
Compiled as "gcc -o jtop main.c gui.c -g  `pkg-config --cflags --libs gtk+-3.0`" 
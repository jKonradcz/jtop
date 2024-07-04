#include "jtop.h"
// taken from docs.gtk.org/gtk3/getting_started.html
// compile with 'gcc `pkg-config --cflags gtk+-3.0` -o gui gui.c `pkg-config --libs gtk+-3.0`'

static void activate (GtkApplication* app, gpointer size) {
    // typecast the gpointer to gui_size struct
    gui_size* gui_size_var = (gui_size*) size;
    
    // create a new window
    GtkWidget *window = gtk_application_window_new (app);
    // set the window title
    gtk_window_set_title (GTK_WINDOW (window), "jTOP");
    // set the window default size, this is calculated in main.c and handed over in the gui_size struct
    gtk_window_set_default_size (GTK_WINDOW (window), gui_size_var->width, gui_size_var->height);

    // create the grid
    GtkWidget *grid = gtk_grid_new ();
    // set the grid size
    gtk_grid_set_row_spacing (GTK_GRID (grid), 20);
    gtk_grid_set_column_spacing (GTK_GRID (grid), gui_size_var->width / 5);
    // attach the grid to the window
    gtk_container_add (GTK_CONTAINER (window), grid);
    // create labels for the header
    GtkWidget *header_pid = gtk_label_new ("PID");
    GtkWidget *header_proc = gtk_label_new ("Process");
    GtkWidget *header_mem = gtk_label_new ("Memory used");
    GtkWidget *header_mempercent = gtk_label_new ("Memory %");
    GtkWidget *header_killbutton = gtk_label_new ("Kill");
    // attach the headers to their positions (numeric values are 1. collumn, 2. row, 3. width, 4. height)
    gtk_grid_attach (GTK_GRID (grid), header_pid, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), header_proc, 1, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), header_mem, 2, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), header_mempercent, 3, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), header_killbutton, 4, 0, 1, 1);
    
    // make the window visible 
    gtk_widget_show_all (window);
}

int gui_main (gui_size* gui_size_var) {
    // creates new GTK application with the ID and default flag
    GtkApplication *app = gtk_application_new ("jTOP.GUI", G_APPLICATION_DEFAULT_FLAGS);
    // connect the activate signal to the activate function and hands over the gui_size struct
    g_signal_connect (app, "activate", G_CALLBACK (activate), gui_size_var);
    // no cmdline arguments
    int status = g_application_run (G_APPLICATION (app), 0, NULL);
    // freeing the memory
    g_object_unref (app);
    return status;
}
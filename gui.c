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
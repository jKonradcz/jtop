#include "jtop.h"
// taken from docs.gtk.org/gtk3/getting_started.html
// compile with 'gcc `pkg-config --cflags gtk+-3.0` -o gui gui.c `pkg-config --libs gtk+-3.0`'

static void activate(GtkApplication* app, gpointer size) {
    // typecast the gpointer to gui_size struct
    gui_size* gui_size_var = (gui_size*) size;
    
    printf("used_proc in activate: %u\n", gui_size_var->used_proc);
    // create a new window
    GtkWidget *window = gtk_application_window_new (app);
    // set the window title
    gtk_window_set_title(GTK_WINDOW(window), "jTOP");
    // set the window default size, this is calculated in main.c and handed over in the gui_size struct
    gtk_window_set_default_size(GTK_WINDOW(window), gui_size_var->width, gui_size_var->height +1); // +1 for the footer/ refresh

    // create the grid
    GtkWidget *grid = gtk_grid_new();

    GtkWidget *footer_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    
    GtkWidget *footer_row = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), footer_box, 0, gui_size_var->used_proc + 1, 5, 1);
    
    GtkWidget *refreshbutton = gtk_button_new_with_label("Refresh");
    gtk_box_pack_end(GTK_BOX(footer_box), refreshbutton, FALSE, TRUE, 0);

    // set the grid size
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), gui_size_var->width / 5);
    // attach the grid to the window
    gtk_container_add(GTK_CONTAINER(window), grid);
    // create labels for the header
    GtkWidget *header_pid = gtk_label_new("PID");
    GtkWidget *header_proc = gtk_label_new("Process");
    GtkWidget *header_mem = gtk_label_new("Memory used");
    GtkWidget *header_mempercent = gtk_label_new("Memory %");
    GtkWidget *header_killbutton = gtk_label_new("Kill");
    // attach the headers to their positions (numeric values are 1. collumn, 2. row, 3. width, 4. height)
    gtk_grid_attach(GTK_GRID(grid), header_pid, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_proc, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_mem, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_mempercent, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_killbutton, 4, 0, 1, 1); 

    populate_grid(grid, gui_size_var->array, gui_size_var->used_proc);

    // make the window visible 
    gtk_widget_show_all(window);
}

int gui_main(gui_size* gui_size_var) {
    proc* array = gui_size_var->array;
    unsigned int used_proc = gui_size_var->used_proc;

    // creates new GTK application with the ID and default flag
    GtkApplication *app = gtk_application_new ("jTOP.GUI", G_APPLICATION_DEFAULT_FLAGS);
    // connect the activate signal to the activate function and hands over the gui_size struct
    g_signal_connect(app, "activate", G_CALLBACK(activate), gui_size_var);
    // no cmdline arguments
    int status = g_application_run(G_APPLICATION(app), 0, NULL);
    // freeing the memory
    g_object_unref(app);
    return status;
}

void kill_proc(GtkWidget* widget, gpointer pid) {
    unsigned int pid_to_kill = (unsigned int)(uintptr_t) pid; // this took me unreasonably long to figure out, there is not reason why it should be necessary, but compiler think otherwise, uintptr_r just in case the size would vary 
    char killmessage[256];
    snprintf(killmessage, sizeof(killmessage), "kill %u", pid_to_kill);
    system(killmessage);
}
// function to populate the window / grid with data
void populate_grid(GtkWidget* grid, proc* array, unsigned int used_proc) {
    char buffer[256];    
    // loop through the array of processes
        for (int i = 0; i < used_proc; i++) {
        // debug comment- printf("PID: %u, proc: %s\n", array[i].pid, array[i].cmdline);        
        snprintf(buffer, sizeof(buffer), "%u", array[i].pid);
        GtkWidget *pid = gtk_label_new(buffer);
        gtk_grid_attach(GTK_GRID(grid), pid, 0, i+1, 1, 1);

        GtkWidget *proc = gtk_label_new(array[i].cmdline);
        gtk_grid_attach(GTK_GRID(grid), proc, 1, i+1, 1, 1);

        snprintf(buffer, sizeof(buffer), "%lu", array[i].mem);
        GtkWidget *mem = gtk_label_new(buffer);
        gtk_grid_attach(GTK_GRID(grid), mem, 2, i+1, 1, 1);

        snprintf(buffer, sizeof(buffer), "%.2f%%", array[i].mempercent);
        GtkWidget *mempercent = gtk_label_new(buffer);
        gtk_grid_attach(GTK_GRID(grid), mempercent, 3, i+1, 1, 1);
        
        // create the kill button with adequate PID 
        GtkWidget *killbutton = gtk_button_new_with_label("Kill");
        g_signal_connect (killbutton, "clicked", G_CALLBACK(kill_proc), (gpointer) (uintptr_t) array[i].pid);
        gtk_grid_attach(GTK_GRID(grid), killbutton, 4, i+1, 1, 1);       
    }
}



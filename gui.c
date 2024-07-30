#include "jtop.h"
// taken from docs.gtk.org/gtk3/getting_started.html
// compile with 'gcc `pkg-config --cflags gtk+-3.0` -o gui gui.c `pkg-config --libs gtk+-3.0`'

extern pthread_mutex_t mutex;

static void activate(GtkApplication* app, gpointer size) {
    // typecast the gpointer to gui_size struct
    gui_size* gui_size_var = (gui_size*) size;
    
    // create a new window
    GtkWidget *window = gtk_application_window_new (app);
    // set the window title
    gtk_window_set_title(GTK_WINDOW(window), "jTOP");
    // set the window default size, this is calculated in main.c and handed over in the gui_size struct
    gtk_window_set_default_size(GTK_WINDOW(window), gui_size_var->width, gui_size_var->height +1); // +1 for the footer/ refresh

    // create the grid
    GtkWidget *grid = gtk_grid_new();
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

    // add the footer and refresh button
    GtkWidget *footer_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_grid_attach(GTK_GRID(grid), footer_box, 0, gui_size_var->used_proc + 1, 5, 1);
    
    // add the refresh button
    GtkWidget *refreshbutton = gtk_button_new_with_label("Refresh");
    // combine the array and grid in a new struct to hand over to the refresh
    refreshd *refresh_data = g_malloc(sizeof(refreshd));
    refresh_data->grid = grid;
    refresh_data->array = gui_size_var->array;
    
    g_signal_connect(refreshbutton, "clicked", G_CALLBACK(refresh), refresh_data);
    gtk_box_pack_end(GTK_BOX(footer_box), refreshbutton, FALSE, TRUE, 0); 

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
// fucntion for the kill button
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

void refresh(GtkWidget* widget, gpointer refresh_data) {
    refreshd *data = (refreshd *)refresh_data;
    GtkWidget *grid = data->grid;
    proc *array = data->array;

    pthread_mutex_lock(&mutex);

    struct sysinfo info;
    sysinfo(&info);

    char* pid = NULL;
    char* cmdline = NULL;
    unsigned int proc_number = 0;
    unsigned int used_proc = 0;
    unsigned long memused = 0;
    int mempercent = 0;

    gather_proc_info(info, pid, &array, &proc_number, cmdline);
    qsort(array, proc_number, sizeof(proc), compare_proc_by_mem);
    calc_mem(&mempercent, &memused, info, &array, &proc_number);
    filter_proc(array, &proc_number, &used_proc);

    // get all the elements of the grid
    GList *children = gtk_container_get_children(GTK_CONTAINER(grid));
    // iterate through the list of elements until none is left
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        // destroy all of the elements in the grid
        GtkWidget *child = GTK_WIDGET(iter->data);
        gtk_widget_destroy(child);
    }
    g_list_free(children);

    printf("In refresh- used proc: %u, proc_number: %u\n", used_proc, proc_number);
    populate_grid(grid, array, used_proc);

    pthread_mutex_unlock(&mutex);
}


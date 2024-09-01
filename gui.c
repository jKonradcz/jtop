#include "jtop.h"
// taken from docs.gtk.org/gtk3/getting_started.html
// compile with 'gcc `pkg-config --cflags gtk+-3.0` -o gui gui.c `pkg-config --libs gtk+-3.0`'

int pop_counter = 0;

extern pthread_mutex_t mutex;

static void activate(GtkApplication* app, gpointer size) {
    // typecast the gpointer to gui_size struct
    gui_size* gui_size_var = (gui_size*) size;
    
    // create a new window
    GtkWidget *window = gtk_application_window_new (app);
    // set the window title
    gtk_window_set_title(GTK_WINDOW(window), "jTOP");
    // set the window default size, this is calculated in main.c and handed over in the gui_size struct
    gtk_window_set_default_size(GTK_WINDOW(window), gui_size_var->width, gui_size_var->height); 

    // GdkGeometry to set minimum=maximum size for the horizontal size
    GdkGeometry limits;

    limits.min_width = gui_size_var->width;
    limits.max_width = gui_size_var->width;
    gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL, &limits, GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);    

    // create a box to fit other elements into
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    // create the header
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_box_pack_start(GTK_BOX(box), header, FALSE, FALSE, 0);

    // create scrollable window including the data
    GtkWidget *proc_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(box), proc_window, TRUE, TRUE, 0);

    // create the grid and add it to the scrollable window
    GtkWidget *grid = gtk_grid_new();
    // set the scroll bar (where, horizontal, vertical)
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(proc_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(proc_window), grid);
    // set the grid size
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), gui_size_var->width / 5);
    
    // create the footer
    GtkWidget *footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box), footer, FALSE, FALSE, 0);

    // create labels for the header
    GtkWidget *header_pid = gtk_label_new("PID");
    GtkWidget *header_proc = gtk_label_new("Process");
    GtkWidget *header_mem = gtk_label_new("Memory used");
    GtkWidget *header_mempercent = gtk_label_new("Memory %");
    GtkWidget *header_killbutton = gtk_label_new("Kill");

    // attach the headers to their positions (numeric values are 1. collumn, 2. row, 3. width, 4. height)
    gtk_box_pack_start(GTK_BOX(header), header_pid, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(header), header_proc, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(header), header_mem, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(header), header_mempercent, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(header), header_killbutton, FALSE, TRUE, 0);
    
    // add the refresh button
    GtkWidget *refreshbutton = gtk_button_new_with_label("Refresh");
    // combine the array and grid in a new struct to hand over to the refresh
    refreshd *refresh_data = g_malloc(sizeof(refreshd));
    refresh_data->grid = grid;
    refresh_data->window = window;
    refresh_data->array = gui_size_var->array;
    
    g_signal_connect(refreshbutton, "clicked", G_CALLBACK(refresh), refresh_data);
    gtk_box_pack_end(GTK_BOX(footer), refreshbutton, FALSE, TRUE, 0); 

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
    pop_counter++;
    // printf("In populate- used proc: %u\n", used_proc);
    char buffer[256];    
    // loop through the array of processes
    for (int i = 0; i < used_proc; i++) {
    /*
    if (pop_counter == 2) {
        printf("Populating PID n: %u\n", array[i].pid);
    }
    */

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
    GtkWidget *window = data->window;
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

        // get all the elements of the grid
    GList *children = gtk_container_get_children(GTK_CONTAINER(grid));
    // iterate through the list of elements until none is left
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        // destroy all of the elements in the grid
        GtkWidget *child = GTK_WIDGET(iter->data);
        gtk_widget_destroy(child);
    }
    g_list_free(children);

    gather_proc_info(info, pid, &array, &proc_number, cmdline);
    qsort(array, proc_number, sizeof(proc), compare_proc_by_mem);
    calc_mem(&mempercent, &memused, info, &array, &proc_number);
    filter_proc(array, &proc_number, &used_proc);

    // printf("In refresh- used proc: %u, proc_number: %u\n", used_proc, proc_number);

    populate_grid(grid, array, used_proc);

    gtk_widget_show_all(window);

    pthread_mutex_unlock(&mutex);
}


#include "jtop.h"
// taken from docs.gtk.org/gtk3/getting_started.html
// compile with 'gcc `pkg-config --cflags gtk+-3.0` -o gui gui.c `pkg-config --libs gtk+-3.0`'

static void activate (GtkApplication* app, gpointer user_data) {
    gui_size* gui_size_var = (gui_size*) user_data;
    GtkWidget *window;
    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "jTOP");
    gtk_window_set_default_size (GTK_WINDOW (window), gui_size_var->width, gui_size_var->height); // window size, TODO: Adjust width per longest cmdline
    gtk_widget_show_all (window);
}

int gui_main (gui_size* gui_size_var) {
    GtkApplication *app;
    int status;
    app = gtk_application_new ("jTOP.GUI", G_APPLICATION_DEFAULT_FLAGS); // had to be adjusted as the original was deprecated
    g_signal_connect (app, "activate", G_CALLBACK (activate), gui_size_var);
    // temp/dummy values for argc and argv, might clean up later
    char* temp_argv[] = {"jtop"};
    status = g_application_run (G_APPLICATION (app), 1, temp_argv);
    g_object_unref (app);
    return status;
}
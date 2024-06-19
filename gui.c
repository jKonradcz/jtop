#include "jtop.h"

// taken from docs.gtk.org/gtk3/getting_started.html
// compile with 'gcc `pkg-config --cflags gtk+-3.0` -o gui gui.c `pkg-config --libs gtk+-3.0`'


static void activate (GtkApplication* app, gpointer user_data) {
    GtkWidget *window;
    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "jTOP");
    gtk_window_set_default_size (GTK_WINDOW (window), 200, 200); // window size, TODO: Adjust width per longest cmdline
    gtk_widget_show_all (window);
}

int gui_main (int argc, char **argv) {
    GtkApplication *app;
    int status;
    app = gtk_application_new ("jTOP.GUI", G_APPLICATION_DEFAULT_FLAGS); // had to be adjusted as the original was deprecated
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return status;
}
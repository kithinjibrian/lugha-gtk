/*
 * The colour scheme GTK's widgets and CSS's `prefers-color-scheme` follow:
 * the property is set through g_object_set, which is variadic, so Lugha
 * can't call it directly.
 */
#include <gtk/gtk.h>

/* Sets gtk-interface-color-scheme (a GtkInterfaceColorScheme) on the default settings. */
void lgtk_set_color_scheme(int scheme)
{
    g_object_set(gtk_settings_get_default(), "gtk-interface-color-scheme", scheme, NULL);
}

/* Gives gtk-interface-color-scheme back to the platform: the system's own choice. */
void lgtk_reset_color_scheme(void)
{
    gtk_settings_reset_property(gtk_settings_get_default(), "gtk-interface-color-scheme");
}

/* The current gtk-interface-color-scheme. */
int lgtk_color_scheme(void)
{
    int scheme = 0;
    g_object_get(gtk_settings_get_default(), "gtk-interface-color-scheme", &scheme, NULL);
    return scheme;
}

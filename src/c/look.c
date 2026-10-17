/*
 * Helpers for an app's look that Lugha can't call directly: removing a set of
 * CSS classes (gtk_widget_get_css_classes returns an array the caller frees)
 * and reading a widget's text colour (gtk_widget_get_color fills a struct
 * through a pointer). Each frees what it allocates.
 */
#include <gtk/gtk.h>

/* Removes every CSS class of the widget that starts with `prefix`. */
void lgtk_remove_css_classes_with_prefix(GtkWidget *widget, const char *prefix)
{
    char **classes = gtk_widget_get_css_classes(widget);
    for (char **c = classes; *c != NULL; c++) {
        if (g_str_has_prefix(*c, prefix))
            gtk_widget_remove_css_class(widget, *c);
    }
    g_strfreev(classes);
}

/* The widget's text colour as 0xRRGGBBAA, each channel 0-255. */
guint32 lgtk_widget_rgba(GtkWidget *widget)
{
    GdkRGBA color;
    gtk_widget_get_color(widget, &color);
    guint32 r = (guint32)(color.red * 255.0f + 0.5f);
    guint32 g = (guint32)(color.green * 255.0f + 0.5f);
    guint32 b = (guint32)(color.blue * 255.0f + 0.5f);
    guint32 a = (guint32)(color.alpha * 255.0f + 0.5f);
    return (r << 24) | (g << 16) | (b << 8) | a;
}

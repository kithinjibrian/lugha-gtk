/*
 * Scrolling a widget into view: the nearest GtkViewport around it (a
 * scrolled window's, made for a child that can't scroll itself) scrolls as
 * little as it can to show it, at its next layout (GTK 4.12). Lugha can't
 * test a widget's type, so the search lives here.
 */
#include <gtk/gtk.h>

/* Scrolls the nearest viewport around `widget` to show it; FALSE when there's none. */
gboolean lgtk_scroll_to(GtkWidget *widget)
{
    GtkWidget *p = gtk_widget_get_parent(widget);
    while (p != NULL && !GTK_IS_VIEWPORT(p))
        p = gtk_widget_get_parent(p);
    if (p == NULL)
        return FALSE;
    gtk_viewport_scroll_to(GTK_VIEWPORT(p), widget, NULL);
    return TRUE;
}

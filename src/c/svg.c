/*
 * SVG pictures (GTK 4.22's GtkSvg): loading with everything outside the SVG
 * turned off before the bytes are read, and telling a syntax error apart
 * from the errors GTK reports and carries on after (an unknown element).
 * The `error` signal hands over a GError, which a Lugha handler can't take,
 * so it lives here. Also: what a picture or image shows, when it's an SVG.
 */
#include <gtk/gtk.h>

static void count_syntax_errors(GtkSvg *svg, GError *error, gpointer data)
{
    if (error->domain == GTK_SVG_ERROR && error->code == GTK_SVG_ERROR_INVALID_SYNTAX)
        (*(int *) data)++;
}

/*
 * A new SVG from `bytes`, drawing with system fonts only: no external
 * resources (other files, URLs), no animations, no GTK extensions. NULL when
 * GTK reports a syntax error. Free it with g_object_unref.
 */
GtkSvg *lgtk_svg_load(GBytes *bytes)
{
    GtkSvg *svg = gtk_svg_new();
    gtk_svg_set_features(svg, GTK_SVG_SYSTEM_RESOURCES);
    int syntax_errors = 0;
    gulong handler = g_signal_connect(svg, "error", G_CALLBACK(count_syntax_errors), &syntax_errors);
    gtk_svg_load_from_bytes(svg, bytes);
    g_signal_handler_disconnect(svg, handler);
    if (syntax_errors > 0) {
        g_object_unref(svg);
        return NULL;
    }
    return svg;
}

/* What `picture` shows if it's an SVG, else NULL; the picture keeps it. */
GtkSvg *lgtk_picture_get_svg(GtkWidget *picture)
{
    GdkPaintable *p = gtk_picture_get_paintable(GTK_PICTURE(picture));
    return GTK_IS_SVG(p) ? GTK_SVG(p) : NULL;
}

/* What `image` shows if it's an SVG, else NULL; the image keeps it. */
GtkSvg *lgtk_image_get_svg(GtkWidget *image)
{
    GdkPaintable *p = gtk_image_get_paintable(GTK_IMAGE(image));
    return GTK_IS_SVG(p) ? GTK_SVG(p) : NULL;
}

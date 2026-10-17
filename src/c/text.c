/*
 * GtkTextBuffer helpers for gtk.raw. A buffer's text and cursor are reached
 * through GtkTextIter, a struct GTK fills through a pointer, which Lugha
 * can't pass; these wrappers keep the iterators on the C side.
 */
#include <gtk/gtk.h>

/* All of the buffer's text, newly allocated: free it with g_free. */
char *lgtk_text_get(GtkTextBuffer *buffer)
{
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    return gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
}

/* The text of a C string, for Lugha to copy (it keeps no pointer). */
const char *lgtk_cstring_text(const char *text)
{
    return text;
}

/* The cursor's position, in characters from the start. */
int lgtk_text_cursor(GtkTextBuffer *buffer)
{
    GtkTextIter at;
    gtk_text_buffer_get_iter_at_mark(buffer, &at, gtk_text_buffer_get_insert(buffer));
    return gtk_text_iter_get_offset(&at);
}

/* Puts the cursor at `offset` characters, or at the end if the text is shorter. */
void lgtk_text_place_cursor(GtkTextBuffer *buffer, int offset)
{
    GtkTextIter at;
    gtk_text_buffer_get_iter_at_offset(buffer, &at, offset);
    gtk_text_buffer_place_cursor(buffer, &at);
}

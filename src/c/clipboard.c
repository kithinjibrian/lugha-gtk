/*
 * The clipboard's text, read back when it's this program's own: GTK keeps
 * what an app set in the process (a content provider), so a test can check
 * it with no other program and no waiting. Reading another program's text is
 * asynchronous in GTK 4 and isn't here.
 */
#include <gtk/gtk.h>

/* The text this program put on `clipboard`, newly allocated (g_free it);
 * NULL when the clipboard holds something else or another program's. */
char *lgtk_clipboard_local_text(GdkClipboard *clipboard)
{
    GdkContentProvider *content = gdk_clipboard_get_content(clipboard);
    if (content == NULL)
        return NULL;
    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_STRING);
    char *text = NULL;
    if (gdk_content_provider_get_value(content, &value, NULL))
        text = g_value_dup_string(&value);
    g_value_unset(&value);
    return text;
}

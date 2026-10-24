/*
 * Opening from outside (G_APPLICATION_HANDLES_OPEN): what a launch was asked
 * to open reaches the running application's `open` signal as a C array of
 * GFiles, which Lugha can't take apart. The hook here keeps each one as a
 * string (a local file's absolute path, anything else's URI) on the
 * application, then calls the handler, which reads them one by one. Also an
 * argument vector built from Lugha strings, for g_application_run.
 */
#include <gtk/gtk.h>

typedef struct {
    void (*handler)(GtkApplication *, gpointer);
    gpointer data;
    GDestroyNotify release;
} OpenHook;

static void open_hook_free(gpointer hook, GClosure *closure)
{
    OpenHook *h = hook;
    if (h->release != NULL)
        h->release(h->data);
    g_free(h);
}

static void on_open(GApplication *app, GFile **files, int n, const char *hint, gpointer hook)
{
    OpenHook *h = hook;
    GPtrArray *items = g_ptr_array_new_with_free_func(g_free);
    for (int i = 0; i < n; i++) {
        char *path = g_file_get_path(files[i]);
        g_ptr_array_add(items, path != NULL ? path : g_file_get_uri(files[i]));
    }
    g_object_set_data_full(G_OBJECT(app), "lgtk-opened", items, (GDestroyNotify)g_ptr_array_unref);
    h->handler(GTK_APPLICATION(app), h->data);
}

/* Calls `handler` on every `open`, after keeping what was asked. */
void lgtk_application_on_open(GtkApplication *app, void (*handler)(GtkApplication *, gpointer),
                              gpointer data, GDestroyNotify release)
{
    OpenHook *h = g_new(OpenHook, 1);
    h->handler = handler;
    h->data = data;
    h->release = release;
    g_signal_connect_data(app, "open", G_CALLBACK(on_open), h, open_hook_free, 0);
}

/* How many items the last `open` asked for. */
int lgtk_application_opened_count(GtkApplication *app)
{
    GPtrArray *items = g_object_get_data(G_OBJECT(app), "lgtk-opened");
    return items != NULL ? (int)items->len : 0;
}

/* The last `open`'s item `i`: a local file's absolute path, or its URI as GIO
 * normalises it (a bare host gains a trailing slash). */
const char *lgtk_application_opened(GtkApplication *app, int i)
{
    GPtrArray *items = g_object_get_data(G_OBJECT(app), "lgtk-opened");
    return g_ptr_array_index(items, i);
}

/* An argument vector of `n` empty slots, NULL-terminated; free with g_strfreev. */
char **lgtk_argv_new(int n)
{
    char **argv = g_new0(char *, n + 1);
    for (int i = 0; i < n; i++)
        argv[i] = g_strdup("");
    return argv;
}

/* Sets slot `i` to a copy of `value`. */
void lgtk_argv_set(char **argv, int i, const char *value)
{
    g_free(argv[i]);
    argv[i] = g_strdup(value);
}

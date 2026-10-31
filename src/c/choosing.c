/*
 * GTK's file dialog, answered once (PRP-001). GtkFileDialog's open, save and
 * select_folder are asynchronous: a GAsyncReadyCallback with no destroy-notify,
 * then a _finish taking a GAsyncResult and a GError **, none of which Lugha
 * can reach. The helper here starts one by mode and, in its callback, hands
 * the chosen file's local path (or NULL) to a Lugha handler exactly once, then
 * releases the handler's data. Filters go in a GListStore of GtkFileFilter,
 * whose GType Lugha can't name either.
 */
#include <gtk/gtk.h>

enum { CHOOSE_OPEN = 0, CHOOSE_SAVE = 1, CHOOSE_FOLDER = 2 };

typedef struct {
    void (*handler)(const char *, gpointer);
    gpointer data;
    GDestroyNotify release;
    int mode;
    GtkFileDialog *dialog; /* a reference, held until the answer */
} Choice;

/* Calls the handler with `path` (NULL for none), then lets everything go. */
static void choice_answer(Choice *c, const char *path)
{
    c->handler(path, c->data);
    if (c->release != NULL)
        c->release(c->data);
    g_clear_object(&c->dialog);
    g_free(c);
}

static void on_chosen(GObject *source, GAsyncResult *result, gpointer choice)
{
    Choice *c = choice;
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    GError *error = NULL;
    GFile *file = NULL;
    switch (c->mode) {
    case CHOOSE_OPEN:
        file = gtk_file_dialog_open_finish(dialog, result, &error);
        break;
    case CHOOSE_SAVE:
        file = gtk_file_dialog_save_finish(dialog, result, &error);
        break;
    default:
        file = gtk_file_dialog_select_folder_finish(dialog, result, &error);
        break;
    }
    /* A cancel, a failure, or a location with no local path: all NULL. */
    char *path = file != NULL ? g_file_get_path(file) : NULL;
    choice_answer(c, path);
    g_free(path);
    g_clear_object(&file);
    g_clear_error(&error);
}

/* Nothing to show (cancelled already, or a mode that isn't one): answered
 * NULL from the main loop, never inside the call. */
static gboolean answer_none(gpointer choice)
{
    choice_answer(choice, NULL);
    return G_SOURCE_REMOVE;
}

/*
 * Shows `dialog` in `mode` (0 open, 1 save, 2 folder), modal to `parent` when
 * the dialog is modal; `handler(path, data)` is called exactly once, with the
 * chosen file's absolute local path or NULL, then `release(data)`. An
 * already-cancelled `cancellable` shows nothing. Holds its
 * own reference to the dialog until then.
 */
void lgtk_file_dialog_choose(GtkFileDialog *dialog, GtkWidget *parent, int mode,
                             GCancellable *cancellable,
                             void (*handler)(const char *, gpointer), gpointer data,
                             GDestroyNotify release)
{
    Choice *c = g_new0(Choice, 1);
    c->handler = handler;
    c->data = data;
    c->release = release;
    c->mode = mode;
    c->dialog = g_object_ref(dialog);
    GtkWindow *window = parent != NULL ? GTK_WINDOW(parent) : NULL;
    /* GTK only listens for "cancelled", which a cancelled one never sends
     * again: it would show the chooser anyway (gtkfiledialog.c, 4.22.4). */
    if (cancellable != NULL && g_cancellable_is_cancelled(cancellable)) {
        g_idle_add(answer_none, c);
        return;
    }
    switch (mode) {
    case CHOOSE_OPEN:
        gtk_file_dialog_open(dialog, window, cancellable, on_chosen, c);
        break;
    case CHOOSE_SAVE:
        gtk_file_dialog_save(dialog, window, cancellable, on_chosen, c);
        break;
    case CHOOSE_FOLDER:
        gtk_file_dialog_select_folder(dialog, window, cancellable, on_chosen, c);
        break;
    default:
        g_idle_add(answer_none, c);
        break;
    }
}

/* Appends `filter` to the dialog's filters, making the list on first use; the
 * list holds its own reference. The first filter is the one shown first. */
void lgtk_file_dialog_add_filter(GtkFileDialog *dialog, GtkFileFilter *filter)
{
    GListModel *filters = gtk_file_dialog_get_filters(dialog);
    if (filters == NULL) {
        GListStore *store = g_list_store_new(GTK_TYPE_FILE_FILTER);
        g_list_store_append(store, filter);
        gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(store));
        g_object_unref(store);
        return;
    }
    /* The dialog's own list is the store made above. */
    g_list_store_append(G_LIST_STORE(filters), filter);
}

/* How many filters the dialog has. */
int lgtk_file_dialog_filter_count(GtkFileDialog *dialog)
{
    GListModel *filters = gtk_file_dialog_get_filters(dialog);
    return filters != NULL ? (int)g_list_model_get_n_items(filters) : 0;
}

/* The dialog's filter `i` (below the count), borrowed: the dialog's list keeps it alive. */
GtkFileFilter *lgtk_file_dialog_filter(GtkFileDialog *dialog, int i)
{
    GListModel *filters = gtk_file_dialog_get_filters(dialog);
    GtkFileFilter *filter = g_list_model_get_item(filters, (guint)i);
    g_object_unref(filter);
    return filter;
}

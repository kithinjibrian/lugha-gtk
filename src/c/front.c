/*
 * Whether a window is the one in front: its toplevel surface's
 * GDK_TOPLEVEL_STATE_FOCUSED (on Wayland, xdg_toplevel's "activated"; on X11,
 * the window manager's focus). Unlike GtkWindow's is-active, which follows
 * keyboard focus on the window's own surface, it doesn't change when one of
 * the window's own popovers (a surface of its own) takes the keyboard. GTK
 * uses the same state for the backdrop look.
 *
 * The surface exists only while the window is realized, so the hook follows
 * realize and unrealize: on realize it connects notify::state on the new
 * surface, on unrealize it lets that one go. A window counts as in front only
 * while mapped too: GDK leaves FOCUSED set on a hidden window's surface.
 */
#include <gtk/gtk.h>

typedef struct {
    void (*handler)(GtkWidget *, int, gpointer);
    gpointer data;
    GDestroyNotify release;
    GtkWidget *window;
    GdkSurface *surface; /* a reference, held while realized */
    gulong state_id;
    int front; /* the last value reported (or found when connecting) */
} FrontHook;

static int surface_is_front(GdkSurface *surface)
{
    if (surface == NULL || !GDK_IS_TOPLEVEL(surface))
        return 0;
    return (gdk_toplevel_get_state(GDK_TOPLEVEL(surface)) & GDK_TOPLEVEL_STATE_FOCUSED) != 0;
}

/* Reports `front` if it differs from the last value. */
static void front_report(FrontHook *h, int front)
{
    if (front == h->front)
        return;
    h->front = front;
    h->handler(h->window, front, h->data);
}

static int window_is_front(GtkWidget *window, GdkSurface *surface)
{
    return gtk_widget_get_mapped(window) && surface_is_front(surface);
}

static void on_state(GObject *surface, GParamSpec *pspec, gpointer hook)
{
    FrontHook *h = hook;
    front_report(h, window_is_front(h->window, GDK_SURFACE(surface)));
}

static void front_detach(FrontHook *h)
{
    if (h->surface == NULL)
        return;
    g_signal_handler_disconnect(h->surface, h->state_id);
    g_object_unref(h->surface);
    h->surface = NULL;
    h->state_id = 0;
}

static void front_attach(FrontHook *h)
{
    front_detach(h);
    GdkSurface *surface = gtk_native_get_surface(GTK_NATIVE(h->window));
    if (surface == NULL || !GDK_IS_TOPLEVEL(surface))
        return;
    h->surface = g_object_ref(surface);
    h->state_id = g_signal_connect(surface, "notify::state", G_CALLBACK(on_state), h);
}

static void on_realize(GtkWidget *window, gpointer hook)
{
    FrontHook *h = hook;
    front_attach(h);
    front_report(h, window_is_front(window, h->surface));
}

/* Map and unmap: shown or hidden, the surface's state unchanged. */
static void on_map_change(GtkWidget *window, gpointer hook)
{
    FrontHook *h = hook;
    front_report(h, window_is_front(window, h->surface));
}

static void on_unrealize(GtkWidget *window, gpointer hook)
{
    FrontHook *h = hook;
    front_detach(h);
    front_report(h, 0);
}

/* Freed with the realize handler: when the window is finalized. */
static void front_hook_free(gpointer hook, GClosure *closure)
{
    FrontHook *h = hook;
    front_detach(h);
    if (h->release != NULL)
        h->release(h->data);
    g_free(h);
}

/* Calls `handler(window, front, data)` each time `window` comes to the front
 * (1) or leaves it (0), never twice with the same value. Hiding or
 * unrealizing a window in front reports 0. Connected while realized, the
 * current value is the starting point and isn't reported. */
void lgtk_window_on_front(GtkWidget *window, void (*handler)(GtkWidget *, int, gpointer),
                          gpointer data, GDestroyNotify release)
{
    FrontHook *h = g_new0(FrontHook, 1);
    h->handler = handler;
    h->data = data;
    h->release = release;
    h->window = window;
    /* After GtkWindow's own realize has made the surface. */
    g_signal_connect_data(window, "realize", G_CALLBACK(on_realize), h, front_hook_free,
                          G_CONNECT_AFTER);
    g_signal_connect(window, "unrealize", G_CALLBACK(on_unrealize), h);
    g_signal_connect_after(window, "map", G_CALLBACK(on_map_change), h);
    g_signal_connect_after(window, "unmap", G_CALLBACK(on_map_change), h);
    if (gtk_widget_get_realized(window)) {
        front_attach(h);
        h->front = window_is_front(window, h->surface);
    }
}

/* 1 if `window` is mapped (so realized) and its toplevel is in front, else 0. */
int lgtk_window_is_front(GtkWidget *window)
{
    if (!gtk_widget_get_mapped(window))
        return 0;
    return surface_is_front(gtk_native_get_surface(GTK_NATIVE(window)));
}

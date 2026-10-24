/*
 * A clip: a widget with one child that asks for no minimum size, so the
 * child's minimum never pushes its parent (or the window) bigger. Given less
 * room than the child's minimum, the child still gets its minimum, at the top
 * start (top right in right-to-left text), and what sticks out is cut off.
 * How far it sticks out, across and down, is kept after each allocation, and
 * `overflow-changed` (no arguments) fires from an idle when it changes: never
 * during layout, where a handler that changes widgets would re-enter it.
 * This is CSS's `overflow: hidden` on a flex item: GTK has no such widget.
 */
#include <gtk/gtk.h>

#define LGTK_TYPE_CLIP (lgtk_clip_get_type())
G_DECLARE_FINAL_TYPE(LgtkClip, lgtk_clip, LGTK, CLIP, GtkWidget)

struct _LgtkClip {
    GtkWidget parent_instance;
    GtkWidget *child;
    int across, down;
    guint idle;
};

G_DEFINE_FINAL_TYPE(LgtkClip, lgtk_clip, GTK_TYPE_WIDGET)

enum { OVERFLOW_CHANGED, LAST_SIGNAL };
static guint signals[LAST_SIGNAL];

static void lgtk_clip_measure(GtkWidget *widget, GtkOrientation orientation, int for_size,
                              int *minimum, int *natural, int *minimum_baseline,
                              int *natural_baseline)
{
    LgtkClip *self = LGTK_CLIP(widget);
    int child_min = 0, child_nat = 0;
    if (self->child != NULL)
        gtk_widget_measure(self->child, orientation, for_size, &child_min, &child_nat, NULL, NULL);
    /* The point of a clip: no minimum of its own; it would like the child's natural size. */
    *minimum = 0;
    *natural = child_nat;
    *minimum_baseline = -1;
    *natural_baseline = -1;
}

static GtkSizeRequestMode lgtk_clip_get_request_mode(GtkWidget *widget)
{
    LgtkClip *self = LGTK_CLIP(widget);
    return self->child != NULL ? gtk_widget_get_request_mode(self->child)
                               : GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

static gboolean lgtk_clip_tell(gpointer data)
{
    LgtkClip *self = LGTK_CLIP(data);
    self->idle = 0;
    g_signal_emit(self, signals[OVERFLOW_CHANGED], 0);
    return G_SOURCE_REMOVE;
}

static void lgtk_clip_size_allocate(GtkWidget *widget, int width, int height, int baseline)
{
    LgtkClip *self = LGTK_CLIP(widget);
    int across = 0, down = 0;
    if (self->child != NULL) {
        int min_w = 0, nat_w = 0, min_h = 0, nat_h = 0;
        gtk_widget_measure(self->child, GTK_ORIENTATION_HORIZONTAL, -1, &min_w, &nat_w, NULL, NULL);
        int w = MAX(width, min_w);
        /* Its height for the width it gets: a wrapping label is taller when narrower. */
        gtk_widget_measure(self->child, GTK_ORIENTATION_VERTICAL, w, &min_h, &nat_h, NULL, NULL);
        int h = MAX(height, min_h);
        across = w - width;
        down = h - height;
        GskTransform *at = NULL;
        /* Right-to-left: the start is the right, so what sticks out goes left. */
        if (across > 0 && gtk_widget_get_direction(widget) == GTK_TEXT_DIR_RTL)
            at = gsk_transform_translate(NULL, &GRAPHENE_POINT_INIT(-across, 0));
        gtk_widget_allocate(self->child, w, h, -1, at);
    }
    if (across != self->across || down != self->down) {
        self->across = across;
        self->down = down;
        if (self->idle == 0)
            self->idle = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, lgtk_clip_tell, g_object_ref(self),
                                         g_object_unref);
    }
}

static void lgtk_clip_dispose(GObject *object)
{
    LgtkClip *self = LGTK_CLIP(object);
    g_clear_handle_id(&self->idle, g_source_remove);
    g_clear_pointer(&self->child, gtk_widget_unparent);
    G_OBJECT_CLASS(lgtk_clip_parent_class)->dispose(object);
}

static void lgtk_clip_class_init(LgtkClipClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    G_OBJECT_CLASS(klass)->dispose = lgtk_clip_dispose;
    widget_class->measure = lgtk_clip_measure;
    widget_class->get_request_mode = lgtk_clip_get_request_mode;
    widget_class->size_allocate = lgtk_clip_size_allocate;
    gtk_widget_class_set_css_name(widget_class, "clip");
    signals[OVERFLOW_CHANGED] = g_signal_new("overflow-changed", G_TYPE_FROM_CLASS(klass),
                                             G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
                                             G_TYPE_NONE, 0);
}

static void lgtk_clip_init(LgtkClip *self)
{
    self->child = NULL;
    self->across = self->down = 0;
    self->idle = 0;
    gtk_widget_set_overflow(GTK_WIDGET(self), GTK_OVERFLOW_HIDDEN);
}

GtkWidget *lgtk_clip_new(void)
{
    return g_object_new(LGTK_TYPE_CLIP, NULL);
}

/* Replaces the child (the old one is unparented, freed if nothing else holds
 * it); NULL leaves the clip empty. */
void lgtk_clip_set_child(GtkWidget *clip, GtkWidget *child)
{
    LgtkClip *self = LGTK_CLIP(clip);
    if (self->child == child)
        return;
    g_clear_pointer(&self->child, gtk_widget_unparent);
    if (child != NULL) {
        self->child = child;
        gtk_widget_set_parent(child, clip);
    }
    gtk_widget_queue_resize(clip);
}

GtkWidget *lgtk_clip_get_child(GtkWidget *clip)
{
    return LGTK_CLIP(clip)->child;
}

/* How far the child sticks out at the last allocation, in pixels: across
 * (`horizontal` TRUE, at the end side) or down (at the bottom). */
int lgtk_clip_overflow(GtkWidget *clip, gboolean horizontal)
{
    LgtkClip *self = LGTK_CLIP(clip);
    return horizontal ? self->across : self->down;
}

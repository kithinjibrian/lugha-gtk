/*
 * A clamp: a widget with one child, measured as the child is but kept within
 * limits (a minimum and a maximum each way; -1 is no limit), and never below
 * the child's own minimum, since GTK can't draw a widget smaller than that.
 * Given more room than its maximum, it gives its child the maximum, placed
 * by the clamp's alignment (`lgtk_clamp_set_align`; fill and start put it at
 * the start, as before), as CSS's `max-width` with `margin: auto`. It takes
 * the pointer only where its child is, so a clamp bigger than its child (a
 * layer of an overlay) doesn't hide what's under it from clicks. GTK has minimum sizes only (CSS min-width, size requests); this is
 * how kivuli gives a node an exact size or a maximum. The clamp parents its
 * child itself and unparents it in dispose.
 */
#include <gtk/gtk.h>

#define LGTK_TYPE_CLAMP (lgtk_clamp_get_type())
G_DECLARE_FINAL_TYPE(LgtkClamp, lgtk_clamp, LGTK, CLAMP, GtkWidget)

struct _LgtkClamp {
    GtkWidget parent_instance;
    GtkWidget *child;
    int min_width, max_width, min_height, max_height;
    GtkAlign halign, valign;
};

G_DEFINE_FINAL_TYPE(LgtkClamp, lgtk_clamp, GTK_TYPE_WIDGET)

/* `n` kept between `lo` and `hi` (-1: none). */
static int keep(int n, int lo, int hi)
{
    if (hi >= 0 && n > hi)
        n = hi;
    if (lo >= 0 && n < lo)
        n = lo;
    return n;
}

static void lgtk_clamp_measure(GtkWidget *widget, GtkOrientation orientation, int for_size,
                               int *minimum, int *natural, int *minimum_baseline,
                               int *natural_baseline)
{
    LgtkClamp *self = LGTK_CLAMP(widget);
    gboolean across = orientation == GTK_ORIENTATION_HORIZONTAL;
    int lo = across ? self->min_width : self->min_height;
    int hi = across ? self->max_width : self->max_height;
    int child_min = 0, child_nat = 0;
    if (self->child != NULL) {
        /* Measured for the size the child will get the other way: at most
         * the other way's maximum (a wrapping label is taller when narrower). */
        int other_hi = across ? self->max_height : self->max_width;
        int child_for = for_size;
        if (child_for >= 0 && other_hi >= 0 && child_for > other_hi)
            child_for = other_hi;
        gtk_widget_measure(self->child, orientation, child_for, &child_min, &child_nat, NULL, NULL);
    }
    /* Never below the child's minimum; the minimum limit raises it. */
    int m = child_min;
    if (lo >= 0 && m < lo)
        m = lo;
    int n = keep(child_nat, lo, hi);
    if (n < m)
        n = m;
    *minimum = m;
    *natural = n;
    *minimum_baseline = -1;
    *natural_baseline = -1;
}

static GtkSizeRequestMode lgtk_clamp_get_request_mode(GtkWidget *widget)
{
    LgtkClamp *self = LGTK_CLAMP(widget);
    return self->child != NULL ? gtk_widget_get_request_mode(self->child)
                               : GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

/* Where a child `size` long starts in `room` placed by `align`; `flip` is
 * right to left, where the start is the far side. */
static int place(int room, int size, GtkAlign align, gboolean flip)
{
    int spare = room - size;
    if (spare <= 0)
        return 0;
    switch (align) {
    case GTK_ALIGN_CENTER:
        return spare / 2;
    case GTK_ALIGN_END:
        return flip ? 0 : spare;
    default:
        return flip ? spare : 0;
    }
}

static void lgtk_clamp_size_allocate(GtkWidget *widget, int width, int height, int baseline)
{
    LgtkClamp *self = LGTK_CLAMP(widget);
    if (self->child == NULL)
        return;
    int w = width, h = height, child_min = 0, child_nat = 0;
    if (self->max_width >= 0 && w > self->max_width) {
        gtk_widget_measure(self->child, GTK_ORIENTATION_HORIZONTAL, -1, &child_min, &child_nat, NULL, NULL);
        w = MAX(self->max_width, child_min);
    }
    if (self->max_height >= 0 && h > self->max_height) {
        gtk_widget_measure(self->child, GTK_ORIENTATION_VERTICAL, w, &child_min, &child_nat, NULL, NULL);
        h = MAX(self->max_height, child_min);
    }
    gboolean rtl = gtk_widget_get_direction(widget) == GTK_TEXT_DIR_RTL;
    int x = place(width, w, self->halign, rtl);
    int y = place(height, h, self->valign, FALSE);
    GskTransform *at = NULL;
    if (x != 0 || y != 0)
        at = gsk_transform_translate(NULL, &GRAPHENE_POINT_INIT(x, y));
    gtk_widget_allocate(self->child, w, h, -1, at);
}

/* Only the child's area: picking asks this after the children, so a point
 * outside the child falls through to whatever is under the clamp. */
static gboolean lgtk_clamp_contains(GtkWidget *widget, double x, double y)
{
    LgtkClamp *self = LGTK_CLAMP(widget);
    graphene_rect_t bounds;
    if (self->child == NULL || !gtk_widget_compute_bounds(self->child, widget, &bounds))
        return FALSE;
    return graphene_rect_contains_point(&bounds, &GRAPHENE_POINT_INIT(x, y));
}

static void lgtk_clamp_dispose(GObject *object)
{
    LgtkClamp *self = LGTK_CLAMP(object);
    g_clear_pointer(&self->child, gtk_widget_unparent);
    G_OBJECT_CLASS(lgtk_clamp_parent_class)->dispose(object);
}

static void lgtk_clamp_class_init(LgtkClampClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    G_OBJECT_CLASS(klass)->dispose = lgtk_clamp_dispose;
    widget_class->measure = lgtk_clamp_measure;
    widget_class->get_request_mode = lgtk_clamp_get_request_mode;
    widget_class->size_allocate = lgtk_clamp_size_allocate;
    widget_class->contains = lgtk_clamp_contains;
    gtk_widget_class_set_css_name(widget_class, "clamp");
}

static void lgtk_clamp_init(LgtkClamp *self)
{
    self->child = NULL;
    self->min_width = self->max_width = self->min_height = self->max_height = -1;
    self->halign = self->valign = GTK_ALIGN_FILL;
}

GtkWidget *lgtk_clamp_new(void)
{
    return g_object_new(LGTK_TYPE_CLAMP, NULL);
}

/* Replaces the child (the old one is unparented, freed if nothing else holds
 * it); NULL leaves the clamp empty. */
void lgtk_clamp_set_child(GtkWidget *clamp, GtkWidget *child)
{
    LgtkClamp *self = LGTK_CLAMP(clamp);
    if (self->child == child)
        return;
    g_clear_pointer(&self->child, gtk_widget_unparent);
    if (child != NULL) {
        self->child = child;
        gtk_widget_set_parent(child, clamp);
    }
}

GtkWidget *lgtk_clamp_get_child(GtkWidget *clamp)
{
    return LGTK_CLAMP(clamp)->child;
}

/* The limits in pixels; -1 is no limit. */
void lgtk_clamp_set_limits(GtkWidget *clamp, int min_width, int max_width, int min_height, int max_height)
{
    LgtkClamp *self = LGTK_CLAMP(clamp);
    self->min_width = min_width;
    self->max_width = max_width;
    self->min_height = min_height;
    self->max_height = max_height;
    gtk_widget_queue_resize(clamp);
}

/* Where the child goes when the clamp has more room than it: GtkAlign each
 * way (fill and start: the start; right to left, start is the right). */
void lgtk_clamp_set_align(GtkWidget *clamp, GtkAlign halign, GtkAlign valign)
{
    LgtkClamp *self = LGTK_CLAMP(clamp);
    self->halign = halign;
    self->valign = valign;
    gtk_widget_queue_allocate(clamp);
}

/* A widget's own answer to whether it takes the pointer at (x, y): its class's
 * `contains`, without gtk_widget_contains's check that it's mapped, so tests
 * with windows never shown can ask. */
gboolean lgtk_widget_own_contains(GtkWidget *widget, double x, double y)
{
    return GTK_WIDGET_GET_CLASS(widget)->contains(widget, x, y);
}

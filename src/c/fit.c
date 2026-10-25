/*
 * A fit: a row (a horizontal GtkBox) and a "more" widget. It shows the row's
 * children from the start while they fit, each at its natural width with the
 * box's spacing between, and hides the rest (`gtk_widget_set_child_visible`,
 * as libadwaita's AdwSqueezer does), with "more" after the last one shown.
 * When everything fits, "more" is hidden. It asks for no more minimum width
 * than "more"'s, so a window can shrink past the row. How many children are
 * hidden is kept after each allocation, and `hidden-changed` (no arguments)
 * fires from an idle when it changes, never during layout. The row should
 * carry no padding or border of its own: its width is worked out from its
 * children's. GTK has no such widget.
 */
#include <gtk/gtk.h>

#define LGTK_TYPE_FIT (lgtk_fit_get_type())
G_DECLARE_FINAL_TYPE(LgtkFit, lgtk_fit, LGTK, FIT, GtkWidget)

struct _LgtkFit {
    GtkWidget parent_instance;
    GtkWidget *row;
    GtkWidget *more;
    int hidden;
    guint idle;
};

G_DEFINE_FINAL_TYPE(LgtkFit, lgtk_fit, GTK_TYPE_WIDGET)

enum { HIDDEN_CHANGED, LAST_SIGNAL };
static guint signals[LAST_SIGNAL];

static int natural_of(GtkWidget *w, GtkOrientation orientation, int *minimum)
{
    int min = 0, nat = 0;
    gtk_widget_measure(w, orientation, -1, &min, &nat, NULL, NULL);
    if (minimum != NULL)
        *minimum = min;
    return nat;
}

/* The row's width with its first `n` shown children, or all when n < 0. */
static int row_width(GtkWidget *row, int n)
{
    int spacing = gtk_box_get_spacing(GTK_BOX(row)), width = 0, count = 0;
    for (GtkWidget *c = gtk_widget_get_first_child(row); c != NULL; c = gtk_widget_get_next_sibling(c)) {
        if (!gtk_widget_get_visible(c))
            continue;
        if (n >= 0 && count == n)
            break;
        width += (count > 0 ? spacing : 0) + natural_of(c, GTK_ORIENTATION_HORIZONTAL, NULL);
        count++;
    }
    return width;
}

static void lgtk_fit_measure(GtkWidget *widget, GtkOrientation orientation, int for_size,
                             int *minimum, int *natural, int *minimum_baseline,
                             int *natural_baseline)
{
    LgtkFit *self = LGTK_FIT(widget);
    int min = 0, nat = 0, more_min = 0;
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        /* The least is "more" alone; the most, every child. */
        if (self->more != NULL)
            natural_of(self->more, orientation, &more_min);
        min = more_min;
        nat = MAX(self->row != NULL ? row_width(self->row, -1) : 0, min);
    } else {
        /* As tall as the tallest child or "more", whichever show. */
        if (self->row != NULL)
            for (GtkWidget *c = gtk_widget_get_first_child(self->row); c != NULL; c = gtk_widget_get_next_sibling(c)) {
                if (!gtk_widget_get_visible(c))
                    continue;
                int c_min = 0, c_nat = natural_of(c, orientation, &c_min);
                min = MAX(min, c_min);
                nat = MAX(nat, c_nat);
            }
        if (self->more != NULL) {
            int m_nat = natural_of(self->more, orientation, &more_min);
            min = MAX(min, more_min);
            nat = MAX(nat, m_nat);
        }
    }
    *minimum = min;
    *natural = nat;
    *minimum_baseline = -1;
    *natural_baseline = -1;
}

static gboolean lgtk_fit_tell(gpointer data)
{
    LgtkFit *self = LGTK_FIT(data);
    self->idle = 0;
    g_signal_emit(self, signals[HIDDEN_CHANGED], 0);
    return G_SOURCE_REMOVE;
}

/* `w` at x, `width` wide, mirrored right to left. */
static void place(GtkWidget *w, int x, int width, int height, int room, gboolean rtl)
{
    int at = rtl ? room - x - width : x;
    GskTransform *t = at != 0 ? gsk_transform_translate(NULL, &GRAPHENE_POINT_INIT(at, 0)) : NULL;
    gtk_widget_allocate(w, width, height, -1, t);
}

static void lgtk_fit_size_allocate(GtkWidget *widget, int width, int height, int baseline)
{
    LgtkFit *self = LGTK_FIT(widget);
    gboolean rtl = gtk_widget_get_direction(widget) == GTK_TEXT_DIR_RTL;
    int hidden = 0;
    if (self->row != NULL) {
        int spacing = gtk_box_get_spacing(GTK_BOX(self->row));
        int total = 0, shown = 0, used = 0;
        for (GtkWidget *c = gtk_widget_get_first_child(self->row); c != NULL; c = gtk_widget_get_next_sibling(c))
            if (gtk_widget_get_visible(c))
                total++;
        if (row_width(self->row, -1) <= width) {
            shown = total;
            used = row_width(self->row, -1);
        } else {
            /* As many as fit with "more" after them. */
            int more_w = self->more != NULL ? natural_of(self->more, GTK_ORIENTATION_HORIZONTAL, NULL) : 0;
            while (shown < total) {
                int next = row_width(self->row, shown + 1);
                if (next + spacing + more_w > width)
                    break;
                shown++;
                used = next;
            }
        }
        hidden = total - shown;
        int i = 0;
        for (GtkWidget *c = gtk_widget_get_first_child(self->row); c != NULL; c = gtk_widget_get_next_sibling(c)) {
            if (!gtk_widget_get_visible(c))
                continue;
            gtk_widget_set_child_visible(c, i < shown);
            i++;
        }
        /* The row as wide as everything when it all fits, so its children
         * can take room they expand into; just what's shown otherwise. */
        place(self->row, 0, hidden == 0 ? width : used, height, width, rtl);
        if (self->more != NULL) {
            gtk_widget_set_child_visible(self->more, hidden > 0);
            if (hidden > 0) {
                int x = shown > 0 ? used + spacing : 0;
                int more_w = MIN(natural_of(self->more, GTK_ORIENTATION_HORIZONTAL, NULL), MAX(width - x, 0));
                place(self->more, x, more_w, height, width, rtl);
            }
        }
    } else if (self->more != NULL) {
        gtk_widget_set_child_visible(self->more, FALSE);
    }
    if (hidden != self->hidden) {
        self->hidden = hidden;
        if (self->idle == 0)
            self->idle = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, lgtk_fit_tell, g_object_ref(self),
                                         g_object_unref);
    }
}

static void lgtk_fit_dispose(GObject *object)
{
    LgtkFit *self = LGTK_FIT(object);
    g_clear_handle_id(&self->idle, g_source_remove);
    g_clear_pointer(&self->row, gtk_widget_unparent);
    g_clear_pointer(&self->more, gtk_widget_unparent);
    G_OBJECT_CLASS(lgtk_fit_parent_class)->dispose(object);
}

static void lgtk_fit_class_init(LgtkFitClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    G_OBJECT_CLASS(klass)->dispose = lgtk_fit_dispose;
    widget_class->measure = lgtk_fit_measure;
    widget_class->size_allocate = lgtk_fit_size_allocate;
    gtk_widget_class_set_css_name(widget_class, "fit");
    signals[HIDDEN_CHANGED] = g_signal_new("hidden-changed", G_TYPE_FROM_CLASS(klass),
                                           G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
                                           G_TYPE_NONE, 0);
}

static void lgtk_fit_init(LgtkFit *self)
{
    self->row = NULL;
    self->more = NULL;
    self->hidden = 0;
    self->idle = 0;
}

GtkWidget *lgtk_fit_new(void)
{
    return g_object_new(LGTK_TYPE_FIT, NULL);
}

/* Replaces `*slot` with `child` (the old one unparented); NULL empties it. */
static void set_slot(GtkWidget *fit, GtkWidget **slot, GtkWidget *child)
{
    if (*slot == child)
        return;
    g_clear_pointer(slot, gtk_widget_unparent);
    if (child != NULL) {
        *slot = child;
        gtk_widget_set_parent(child, fit);
    }
    gtk_widget_queue_resize(fit);
}

/* The row: a horizontal GtkBox whose children are shown while they fit. */
void lgtk_fit_set_row(GtkWidget *fit, GtkWidget *row)
{
    g_return_if_fail(row == NULL || GTK_IS_BOX(row));
    set_slot(fit, &LGTK_FIT(fit)->row, row);
}

GtkWidget *lgtk_fit_get_row(GtkWidget *fit)
{
    return LGTK_FIT(fit)->row;
}

/* What shows after the row's shown children when some are hidden. */
void lgtk_fit_set_more(GtkWidget *fit, GtkWidget *more)
{
    set_slot(fit, &LGTK_FIT(fit)->more, more);
}

GtkWidget *lgtk_fit_get_more(GtkWidget *fit)
{
    return LGTK_FIT(fit)->more;
}

/* How many of the row's children were hidden at the last allocation, from the end. */
int lgtk_fit_hidden(GtkWidget *fit)
{
    return LGTK_FIT(fit)->hidden;
}

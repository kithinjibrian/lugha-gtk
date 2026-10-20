/*
 * Drop-downs whose choices come in groups, each under a heading: a GListStore
 * of string lists (one per group, its heading kept on it), flattened for the
 * drop-down, with a header factory showing each group's heading. The
 * factory's `setup` and `bind` signals take a GtkListHeader, which Lugha
 * can't name, so they live here.
 */
#include <gtk/gtk.h>

#define HEADING "lgtk-heading"

/* A new, empty set of groups (a `GListStore` of `GListModel`s); free it with g_object_unref. */
GListStore *lgtk_groups_new(void)
{
    return g_list_store_new(G_TYPE_LIST_MODEL);
}

/* Adds `items` as the last group, under `heading` ("" for none); the groups take their own reference. */
void lgtk_groups_add(GListStore *groups, const char *heading, GtkStringList *items)
{
    g_object_set_data_full(G_OBJECT(items), HEADING, g_strdup(heading), g_free);
    g_list_store_append(groups, items);
}

/* A header: a separator above every group but the first, then the heading. */
static void setup_header(GtkSignalListItemFactory *factory, GObject *object, gpointer data)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    GtkWidget *label = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_box_append(GTK_BOX(box), label);
    gtk_list_header_set_child(GTK_LIST_HEADER(object), box);
}

/* The group holding flat position `position` of `dropdown`'s model, or NULL. */
static GListModel *group_at(GtkWidget *dropdown, guint position)
{
    GListModel *model = gtk_drop_down_get_model(GTK_DROP_DOWN(dropdown));
    if (!GTK_IS_FLATTEN_LIST_MODEL(model) || position >= g_list_model_get_n_items(model))
        return NULL;
    return gtk_flatten_list_model_get_model_for_item(GTK_FLATTEN_LIST_MODEL(model), position);
}

static void bind_header(GtkSignalListItemFactory *factory, GObject *object, gpointer dropdown)
{
    GtkListHeader *header = GTK_LIST_HEADER(object);
    guint start = gtk_list_header_get_start(header);
    GtkWidget *box = gtk_list_header_get_child(header);
    GtkWidget *separator = gtk_widget_get_first_child(box);
    GtkWidget *label = gtk_widget_get_next_sibling(separator);
    GListModel *group = group_at(GTK_WIDGET(dropdown), start);
    const char *heading = group ? g_object_get_data(G_OBJECT(group), HEADING) : NULL;
    gtk_widget_set_visible(separator, start > 0);
    gtk_label_set_text(GTK_LABEL(label), heading ? heading : "");
    gtk_widget_set_visible(label, heading && heading[0] != '\0');
}

/*
 * Shows `groups` in `dropdown`: their choices one after another (positions
 * count through every group), each group under its heading. The drop-down
 * takes its own reference to the groups.
 */
void lgtk_drop_down_set_groups(GtkWidget *dropdown, GListStore *groups)
{
    GtkFlattenListModel *flat = gtk_flatten_list_model_new(G_LIST_MODEL(g_object_ref(groups)));
    gtk_drop_down_set_model(GTK_DROP_DOWN(dropdown), G_LIST_MODEL(flat));
    g_object_unref(flat);
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
    g_signal_connect(factory, "setup", G_CALLBACK(setup_header), NULL);
    // The drop-down outlives its factory's headers, so it isn't referenced here.
    g_signal_connect(factory, "bind", G_CALLBACK(bind_header), dropdown);
    gtk_drop_down_set_header_factory(GTK_DROP_DOWN(dropdown), factory);
    g_object_unref(factory);
}

/* Takes the headers away; set a plain model after. */
void lgtk_drop_down_clear_groups(GtkWidget *dropdown)
{
    gtk_drop_down_set_header_factory(GTK_DROP_DOWN(dropdown), NULL);
}

/* The label of the choice at flat position `position`, NULL past the end (for tests; the drop-down keeps it). */
const char *lgtk_drop_down_label(GtkWidget *dropdown, guint position)
{
    GListModel *model = gtk_drop_down_get_model(GTK_DROP_DOWN(dropdown));
    if (model == NULL || position >= g_list_model_get_n_items(model))
        return NULL;
    GObject *item = g_list_model_get_item(model, position);
    const char *label = gtk_string_object_get_string(GTK_STRING_OBJECT(item));
    g_object_unref(item);
    return label;
}

/* The heading of the group holding flat position `position`; NULL for a drop-down without groups (for tests). */
const char *lgtk_drop_down_heading(GtkWidget *dropdown, guint position)
{
    if (gtk_drop_down_get_header_factory(GTK_DROP_DOWN(dropdown)) == NULL)
        return NULL;
    GListModel *group = group_at(dropdown, position);
    return group ? g_object_get_data(G_OBJECT(group), HEADING) : NULL;
}

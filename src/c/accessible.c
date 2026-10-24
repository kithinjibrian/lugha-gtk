/*
 * Accessibility: a widget's role, its checked and selected states and its
 * label, as a screen reader hears them. The calls GTK offers for these are
 * variadic (g_object_set, gtk_accessible_update_state/_property and the
 * gtk_test_accessible_check_ functions), which Lugha can't call, so each
 * use gets its own function here.
 */
#include <gtk/gtk.h>

/* Sets the accessible role (GtkAccessibleRole). GTK refuses, with a
 * critical, once an assistive technology has read the widget: set it when
 * the widget is made. */
void lgtk_accessible_set_role(GtkWidget *widget, int role)
{
    g_object_set(widget, "accessible-role", role, NULL);
}

/* The label a screen reader says for the widget. */
void lgtk_accessible_set_label(GtkWidget *widget, const char *label)
{
    gtk_accessible_update_property(GTK_ACCESSIBLE(widget), GTK_ACCESSIBLE_PROPERTY_LABEL, label, -1);
}

/* Checked or not (a checkbox, a switch). */
void lgtk_accessible_set_checked(GtkWidget *widget, gboolean on)
{
    gtk_accessible_update_state(GTK_ACCESSIBLE(widget), GTK_ACCESSIBLE_STATE_CHECKED,
                                on ? GTK_ACCESSIBLE_TRISTATE_TRUE : GTK_ACCESSIBLE_TRISTATE_FALSE, -1);
}

/* Selected or not (a tab, a list item). */
void lgtk_accessible_set_selected(GtkWidget *widget, gboolean on)
{
    gtk_accessible_update_state(GTK_ACCESSIBLE(widget), GTK_ACCESSIBLE_STATE_SELECTED, on, -1);
}

/* Whether the widget's `state` (GtkAccessibleState) is `value`, for tests. */
gboolean lgtk_accessible_state_is(GtkWidget *widget, int state, int value)
{
    char *differs = gtk_test_accessible_check_state(GTK_ACCESSIBLE(widget), state, value);
    gboolean same = differs == NULL;
    g_free(differs);
    return same;
}

/* Whether the widget's accessible label is `label`, for tests. */
gboolean lgtk_accessible_label_is(GtkWidget *widget, const char *label)
{
    char *differs = gtk_test_accessible_check_property(GTK_ACCESSIBLE(widget), GTK_ACCESSIBLE_PROPERTY_LABEL, label);
    gboolean same = differs == NULL;
    g_free(differs);
    return same;
}

/*
 * Helpers for menus Lugha can't call directly: setting a string attribute on
 * a GMenuItem (g_menu_item_set_attribute is variadic), such as the `accel`
 * hint a popover menu shows at an item's end; reading a string variant and an
 * action's state.
 */
#include <gio/gio.h>

/* Sets the string attribute `attribute` of `item` to `value`. */
void lgtk_menu_item_set_string(GMenuItem *item, const char *attribute, const char *value)
{
    g_menu_item_set_attribute(item, attribute, "s", value);
}

/* The string inside `value`, a `s` variant (g_variant_get_string's length is not wanted). */
const char *lgtk_variant_string(GVariant *value)
{
    return g_variant_get_string(value, NULL);
}

/* The boolean state of `action` (for tests). */
gboolean lgtk_action_state_bool(GSimpleAction *action)
{
    GVariant *state = g_action_get_state(G_ACTION(action));
    gboolean b = g_variant_get_boolean(state);
    g_variant_unref(state);
    return b;
}

/* The string state of `action` (for tests); GLib keeps it, so it's copied into a string the caller owns. */
char *lgtk_action_state_string(GSimpleAction *action)
{
    GVariant *state = g_action_get_state(G_ACTION(action));
    char *s = g_strdup(g_variant_get_string(state, NULL));
    g_variant_unref(state);
    return s;
}

/*
 * Helpers for menu items Lugha can't call directly: setting a string
 * attribute on a GMenuItem (g_menu_item_set_attribute is variadic), such as
 * the `accel` hint a popover menu shows at an item's end.
 */
#include <gio/gio.h>

/* Sets the string attribute `attribute` of `item` to `value`. */
void lgtk_menu_item_set_string(GMenuItem *item, const char *attribute, const char *value)
{
    g_menu_item_set_attribute(item, attribute, "s", value);
}

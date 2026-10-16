/*
 * g_signal_emit_by_name is variadic, and Lugha can't declare a variadic C
 * function, so gtk.raw reaches it through this fixed-arity wrapper. It emits
 * a signal that takes no arguments and returns nothing, such as a button's
 * "clicked" or an entry's "activate". The signal name is used only during
 * the call, never kept.
 */
#include <glib-object.h>

void lgtk_emit(gpointer instance, const char *signal)
{
    g_signal_emit_by_name(instance, signal);
}

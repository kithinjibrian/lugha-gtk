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

/* Emits a click or long-press gesture's "pressed" (tests): g_signal_emit_by_name
 * is variadic. A long press has no press count: `n` < 0 leaves it out. */
void lgtk_emit_pressed(gpointer gesture, int n, double x, double y)
{
    if (n < 0)
        g_signal_emit_by_name(gesture, "pressed", x, y);
    else
        g_signal_emit_by_name(gesture, "pressed", n, x, y);
}

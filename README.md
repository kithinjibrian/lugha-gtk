# lugha-gtk

GTK 4 for [Lugha](https://github.com/kithinjibrian/lugha): windows and widgets as Lugha types with methods, and closures for signals.

```
import gtk { Application, Box, Button, Label, Window, run };

fun main(): i32 {
    run("org.lugha.counter", fun(a: Application) {
        let window = Window.new(a, "Counter", 320, 160);
        let label = Label.new("0");
        let button = Button.new("add one");
        let count = Ref.new(0);
        button.on_click(fun() {
            count.update(fun(n) = n + 1);
            label.set_text("{count.get()}");
        });
        let content = Box.column([label, button]);
        content.margin(24);
        window.set_child(content);
        window.present();
    })
}
```

## Requirements

- `lughac` with milestones 87–90 (C libraries, callbacks and handles, strings from C) and 99 (`import gtk;`, a package's `lib.la`), and 100 for the raw names through `gtk { … }`. `import gtk.app;` and `import gtk.widgets { … };` work on older ones too.
- GTK 4's development files and `pkg-config`. On Ubuntu:

```sh
sudo apt install libgtk-4-dev pkgconf
```

## Using it

List it in your program's `lugha.toml`:

```toml
[packages]
gtk = { git = "https://github.com/kithinjibrian/lugha-gtk", version = "0.1" }
```

The package's own `lugha.toml` links GTK through `pkg-config`, so your program needs no `[link]` table.

Inside this repository, the examples import the package by name:

```sh
lughac run examples/counter.la
lughac run examples/greeter.la
lughac run test/smoke.la      # builds widgets, prints their text, quits without a window
```

## What's covered (0.1.29)

| Module | Items |
| --- | --- |
| `gtk` | the entry (`src/lib.la`): `run`, `Application` and the widgets below, re-exported, so `import gtk { run, Window, … };` is all a program needs (0.1.3); `gtk.raw` re-exported too, so `import gtk;` reaches `gtk.raw.…` (0.1.4); every `gtk.raw` item also by its own name: `import gtk { GtkWidget, gtk_box_new };` (0.1.8, lughac milestone 100); `gtk_box_set_spacing`/`get_spacing`, `gtk_widget_set_halign`/`valign` (and `get_`), `gtk_widget_set_size_request`, `gtk_widget_get_margin_start`/`top` (0.1.9); Pango attributes (`PangoAttrList`, `pango_attr_list_new`/`insert`/`unref`/`to_string`, `pango_attr_weight_new`, `pango_attr_size_new`), `gtk_label_set_attributes`/`get_attributes`, `gtk_widget_add_css_class`/`remove_css_class`/`has_css_class` (0.1.10); `gtk_widget_set_sensitive`/`get_sensitive` (0.1.11); GLib timers `g_timeout_add_full` (a closure, kept until it returns 0) and `g_source_remove`, clocks `g_get_real_time`/`g_get_monotonic_time`, `g_application_hold`/`release`, `gtk_window_get_title` (0.1.12); `gtk_entry_set_placeholder_text`/`get_placeholder_text` (0.1.13); `gtk_editable_get_position`/`set_position`, `on_finalized` (`g_object_weak_ref`, for tests and checks) (0.1.14); `GtkTextView`/`GtkTextBuffer`: `gtk_text_view_new`, `get_buffer`, `set_wrap_mode`/`get_wrap_mode`, `gtk_text_buffer_set_text`, `connect_buffer`, and C helpers `lgtk_text_get` (an `OwnedText`: read with `lgtk_cstring_text`, free with `g_free`), `lgtk_text_cursor`, `lgtk_text_place_cursor` (0.1.15); labels' `wrap`, `wrap_mode`, `ellipsize`, `lines`, `max_width_chars`, `selectable` (set and get), `gtk_widget_set_tooltip_text`/`get_tooltip_text`, `gtk_check_button_new` and `set_child`/`get_child` (0.1.16); shortcuts: `gtk_shortcut_controller_new`/`set_scope`/`add_shortcut`/`remove_shortcut`, `gtk_widget_add_controller`, `gtk_shortcut_new`, `gtk_shortcut_trigger_parse_string`, `gtk_callback_action_new` (a closure; works off screen), `gtk_activate_action_get`, `gtk_shortcut_action_activate`; focus: `gtk_widget_grab_focus`, `gtk_root_get_focus`, `gtk_widget_is_ancestor` (0.1.17); `gtk_shortcut_get_trigger`/`get_action`, `gtk_shortcut_trigger_to_string` (0.1.18); drop-downs and string lists (`gtk_string_list_new`/`append`/`unref`, `gtk_drop_down_new`/`set_model`/`set_selected`/`get_selected`, `connect_notify` for `notify::selected`) and scales (`gtk_scale_new_with_range`, `set_digits`, `set_draw_value`, `gtk_range_set_value`/`get_value`/`set_range`/`set_increments`/`set_round_digits`) (0.1.19); reading a drop-down's list back: `gtk_drop_down_get_model`, `gtk_string_list_get_n_items`, `gtk_string_list_get_string` (0.1.20); `gtk_widget_ref`/`gtk_widget_unref`, holding a widget alive (0.1.21); theme icons (`gtk_image_new_from_icon_name`, `set_from_icon_name`/`get_icon_name`, `set_pixel_size`/`get_pixel_size`), pictures (`gtk_picture_new_for_filename`, `set_filename`, `set_content_fit`/`get_content_fit`, `get_file` with `GFile` and `g_file_get_path`), `gtk_button_set_child`/`get_child`, `gtk_widget_set_visible`/`get_visible` (0.1.22); app-wide styles (`GtkCssProvider`: `gtk_css_provider_new`, `load_from_string`, `to_string`; `gtk_style_context_add_provider_for_display`, `GdkDisplay`, `gtk_widget_get_display`), C helpers `lgtk_remove_css_classes_with_prefix` and `lgtk_widget_rgba`, `gtk_window_maximize`/`is_maximized`, `gtk_window_set_titlebar`/`get_titlebar`, header bars (`gtk_header_bar_new`, `set_title_widget`/`get_title_widget`, `pack_start`, `pack_end`, `remove`, `set_show_title_buttons`/`get_show_title_buttons`) (0.1.23); `gtk_widget_set_overflow`/`get_overflow` (0.1.24); images from bytes: `GBytes` (`g_bytes_new`, copying, `g_bytes_unref`), `GdkTexture` (`gdk_texture_new_from_bytes` with a `GError` out-parameter and `g_error_free`, `get_width`/`get_height`, `gdk_texture_unref`, `on_texture_finalized`), `gtk_picture_new`, `gtk_picture_set_paintable`/`get_paintable`, `gtk_image_set_from_paintable`/`get_paintable` (0.1.25); flow boxes (`gtk_flow_box_new`, `insert`, `remove`, `get_child_at_index`, `gtk_flow_box_child_get_child`, `set_homogeneous`, `set_row_spacing`/`set_column_spacing`, `set_max_children_per_line`, `set_selection_mode`, and their getters), `gtk_widget_set_focusable`/`get_focusable`, `gtk_widget_get_parent` (0.1.26); overlays (`gtk_overlay_new`, `set_child`/`get_child`, `add_overlay`, `remove_overlay`) (0.1.27); context menus: `GMenu` (`g_menu_new`, `append`, `append_section`), `GSimpleAction` (`new`, `set_enabled`, `g_action_activate`, `connect_action`), `GSimpleActionGroup` with `g_action_map_add_action` and `gtk_widget_insert_action_group`, `gtk_popover_menu_new_from_model`/`set_menu_model`, `gtk_popover_set_pointing_to` (`GdkRectangle`), `set_has_arrow`, `popup`/`popdown`, `gtk_widget_set_parent`/`unparent`, `get_width`/`get_height`, click and long-press gestures (`connect_click`, `connect_long_press`, `gtk_gesture_single_set_button`) and `lgtk_emit_pressed` (0.1.28); `GdkRectangle`'s fields public, so other packages can build one (0.1.29) |
| `gtk.app` | `run(id, on_activate)`, `Application` (`quit`) |
| `gtk.widgets` | the `Widget` interface; `Window` (`new`, `set_child`, `present`); `Box` (`row`, `column`, `append`, `margin`); `Button` (`new`, `label`, `set_label`, `on_click`, `margin`); `Label` (`new`, `text`, `set_text`, `margin`); `Entry` (`new`, `text`, `set_text`, `on_change`, `on_activate`, `margin`) |
| `gtk.raw` | the GTK and GLib functions above as `public` externs, typed handles (`GtkApplication`, `GtkWidget`), and `connect_app`/`connect_widget` for any signal whose handler takes its object; `gtk_box_remove` and `gtk_box_insert_child_after` (0.1.1); `emit_widget(widget, signal)`, emitting a no-argument signal such as `clicked` (0.1.2); `GtkCheckButton`: `gtk_check_button_new_with_label`, `get_active`/`set_active` (`gboolean` as `i32`), `get_label`/`set_label` (0.1.5); `gtk_widget_set_hexpand`/`vexpand` (and `get_`), `gtk_orientable_get_orientation`, `gtk_separator_new`, `gtk_scrolled_window_new`/`set_child`/`get_child` (a child that can't scroll itself, such as a label, comes back wrapped in a `GtkViewport`) (0.1.6); `gtk_box_reorder_child_after`, `gtk_widget_get_first_child`/`get_next_sibling` (0.1.7) |

Anything not wrapped yet can be declared next to your program the way `gtk.raw` does, or added here.

## License

GPL-3.0-only, like Lugha. GTK itself is LGPL-2.1-or-later.

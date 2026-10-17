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

- `lughac` with milestones 87–90 (C libraries, callbacks and handles, strings from C) and 99 (`import gtk;`, a package's `lib.la`). `import gtk.app;` and `import gtk.widgets { … };` work on older ones too.
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

## What's covered (0.1.5)

| Module | Items |
| --- | --- |
| `gtk` | the entry (`src/lib.la`): `run`, `Application` and the widgets below, re-exported, so `import gtk { run, Window, … };` is all a program needs (0.1.3); `gtk.raw` re-exported too, so `import gtk;` reaches `gtk.raw.…` (0.1.4) |
| `gtk.app` | `run(id, on_activate)`, `Application` (`quit`) |
| `gtk.widgets` | the `Widget` interface; `Window` (`new`, `set_child`, `present`); `Box` (`row`, `column`, `append`, `margin`); `Button` (`new`, `label`, `set_label`, `on_click`, `margin`); `Label` (`new`, `text`, `set_text`, `margin`); `Entry` (`new`, `text`, `set_text`, `on_change`, `on_activate`, `margin`) |
| `gtk.raw` | the GTK and GLib functions above as `public` externs, typed handles (`GtkApplication`, `GtkWidget`), and `connect_app`/`connect_widget` for any signal whose handler takes its object; `gtk_box_remove` and `gtk_box_insert_child_after` (0.1.1); `emit_widget(widget, signal)`, emitting a no-argument signal such as `clicked` (0.1.2); `GtkCheckButton`: `gtk_check_button_new_with_label`, `get_active`/`set_active` (`gboolean` as `i32`), `get_label`/`set_label` (0.1.5) |

Anything not wrapped yet can be declared next to your program the way `gtk.raw` does, or added here.

## License

GPL-3.0-only, like Lugha. GTK itself is LGPL-2.1-or-later.

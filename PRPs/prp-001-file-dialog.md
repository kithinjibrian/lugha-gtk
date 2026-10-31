## FEATURE: GTK's file dialog (open, save, folder) for Lugha, answered once with the chosen path

## OBJECTIVE

A program shows the desktop's file chooser (the xdg-desktop-portal one where there is one), modal to its window, and its closure is called exactly once with the chosen file's absolute local path, or `None`. lugha-gtk 0.1.60.

```
import gtk { gtk_file_dialog_new, gtk_file_dialog_set_title, gtk_file_filter_new,
             gtk_file_filter_set_name, gtk_file_filter_add_pattern, gtk_file_filter_unref,
             lgtk_file_dialog_add_filter, lgtk_file_dialog_choose, gtk_file_dialog_unref };

let dialog = gtk_file_dialog_new();
gtk_file_dialog_set_title(dialog, "Open a picture");
let pictures = gtk_file_filter_new();
gtk_file_filter_set_name(pictures, Some("Pictures"));
gtk_file_filter_add_pattern(pictures, "*.png");
lgtk_file_dialog_add_filter(dialog, pictures);
gtk_file_filter_unref(pictures);
lgtk_file_dialog_choose(dialog, Some(window), 0, None, fun(path: Option<string>) {
    // Some("/home/…/x.png"), or None: cancelled, failed, or not a local file
});
gtk_file_dialog_unref(dialog);   // the helper keeps its own reference while it's open
```

## CONTEXT

- **Asked for by:** kivuli PRP-071 (`../kivuli/PRPs/prp-071-file-chooser.md`, kivuli DECISION-013), for Rapidkoi's DECISION-089.
- **Headers (GTK 4.22.4, GLib installed here), each extern checked against them:**
  - `gtk/gtkfiledialog.h`: `gtk_file_dialog_new`, `set_title`/`get_title`, `set_modal`/`get_modal`, `set_initial_name`/`get_initial_name`, `get_filters`/`set_filters`, `open`/`save`/`select_folder` `(self, GtkWindow *parent, GCancellable *, GAsyncReadyCallback, gpointer)` and their `_finish` `(self, GAsyncResult *, GError **)` → `GFile *` (4.10+).
  - `gtk/gtkfilefilter.h`: `gtk_file_filter_new`, `set_name`/`get_name` (`const char *`, may be NULL), `add_pattern`.
  - `gio/gcancellable.h`: `g_cancellable_new`, `g_cancellable_cancel`. `gio/gliststore.h`, `gio/glistmodel.h`: used in C only.
- **What Lugha allows (`lughac spec`, section 8):** a callback may receive `Option<string>`, copied from C (milestone 89); `Option<string>` arguments and results; `data`/`release` closures, called on the registering thread. `GAsyncReadyCallback` has no destroy-notify and `_finish` needs a `GAsyncResult` and a `GError **`, and filters need `GTK_TYPE_FILE_FILTER` for a `GListStore`: none of that is reachable from Lugha, so a C helper does it (`src/c/choosing.c`), as `src/c/opening.c` does for `open`.
- **Creates:** `src/c/choosing.c`, `test/choosing.la`. **Changes:** `src/raw.la`, `lugha.toml` (0.1.60, the new C file), `README.md`.

## REQUIREMENTS

### Must Do

- Handles `GtkFileDialog`, `GtkFileFilter`, `GCancellable`, and the plain externs above, with `gtk_file_dialog_unref`, `gtk_file_filter_unref`, `g_cancellable_unref` as renamed `g_object_unref`.
- `lgtk_file_dialog_add_filter(dialog, filter)`: appends to the dialog's filter list, making the list (a `GListStore` of `GtkFileFilter`) on first use. The first filter is the one shown first.
- `lgtk_file_dialog_filter_count(dialog)` and `lgtk_file_dialog_filter(dialog, i)`: reading the filters back, for tests.
- `lgtk_file_dialog_choose(dialog, parent, mode, cancellable, handler)`, mode 0 open, 1 save, 2 folder: the handler is called **exactly once**, with `g_file_get_path` of the result, or `None` on an error, a cancel, or a file with no local path; then its data is released. The helper holds a reference to the dialog until then, so the caller may unref it at once. Any other mode is a programmer error, and an already-cancelled `cancellable` has nothing to show: both answer `None` from the main loop, never during the call, and show nothing. (GTK listens only for `cancelled`, which a cancelled one never sends again, so it would show the chooser anyway: `gtkfiledialog.c`, 4.22.4, lines 1033–1043.)

### Must NOT Do

- Read or open the chosen file: the program's job.
- Bind the multiple-selection and text-file variants, initial folder or file, accept label: nobody needs them yet.

## DESIGN

`src/c/choosing.c`: a `Choice { handler, data, release, mode }`; the GTK callback calls the matching `_finish`, takes the path, calls the handler, releases, and frees the path, the `GFile`, the `GError` and its reference to the dialog. A bad mode or an already-cancelled cancellable is checked before anything is shown, and answered through `g_idle_add`, so the handler never runs inside the caller.

## TESTS (`test/choosing.la`)

- [ ] Headless (a dialog and filters are plain GObjects): title, modal, initial name and filters read back after setting; a dialog starts with no filters; filter names and order.
- [ ] Display (skipped without one), one `run` per test with its own ID: a choose with an already-cancelled `GCancellable`, in each mode, is answered exactly once with `None`, after the call returns, showing nothing; a bad mode the same. (Release isn't observable from Lugha: reviewed in the C.)
- [ ] By eye (kivuli's example, PRP-071): the portal chooser over the window, filters listed, a choice gives its path.

## ACCEPTANCE

- [ ] `lughac test test/choosing.la` passes, and every other test file in `test/`
- [ ] README's table lists 0.1.60
- [ ] Tagged `v0.1.60` and pushed (the user's standing go for lugha-gtk releases)

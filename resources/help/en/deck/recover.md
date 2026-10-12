# Get back unsaved work
<!-- keywords: recover, recovery, autorecover, auto recover, autosave, crash, lost work, unsaved, power failure, restore, frozen, closed by accident, backup -->

If JeffDeck closes by surprise, or the power goes out, your latest work may still be safe. While you work, JeffDeck keeps a spare copy of any presentation that has changes you have not saved. This is called **AutoRecover**.

## How AutoRecover works

- Every 10 minutes, JeffDeck saves a spare copy of each presentation that has unsaved changes.
- JeffDeck keeps the copies in an AutoRecover folder, inside the folder where it keeps its own data. You never have to look there yourself.
- The spare copy never replaces your real file.
- The copy goes away when you save the presentation, close it, or quit JeffDeck in the normal way.

AutoRecover does not replace saving. If JeffDeck stops, you can lose the work from the last few minutes. Press Ctrl+S often.

## Recover your work

The next time you start JeffDeck after a crash, the **Recover Unsaved Work** window opens by itself. Each line shows the presentation's name, where the file was kept (or "never saved"), and when the copy was made.

1. Leave a check mark next to each presentation you want back. Clear the mark on the others.
2. Click **Open**.
3. Check the work, then press Ctrl+S.

A recovered presentation opens as unsaved work. Ctrl+S asks for a name, and starts in the folder of the original file, with the original name. Pick the original name to replace the file, or choose a new name to keep both.

The other buttons in the window are:

- **Delete** removes the checked copies for good, after it asks you to confirm.
- **Not Now** closes the window and leaves the copies. JeffDeck offers them again the next time it starts.

JeffDeck only offers copies from a run that ended by surprise. It does not offer copies from a JeffDeck window that is still open.

## Find recovered work later

If you closed the window and want your copies back, click **File > Open**, then **Recover Unsaved Work**. If there is nothing to recover, JeffDeck says so.

## Related

- [Open and save presentations](open-save)
- [The File screen](file-screen)
- [Change JeffDeck's settings](options)

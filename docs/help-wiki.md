# Help and the project wiki

JeffPub's Help is a set of Markdown files, one per topic, in
`resources/help/en/`. The top of `libs/shell/app/help.h` says how a topic file is
written. **Edit the topics there.** They are built into the program, and the
same pages are published to the project's
[wiki](https://github.com/JeffOffice/jeffoffice/wiki) automatically. Anything
edited on the wiki itself is replaced by the next update.

After adding, renaming, or removing a topic file, run
`python3 tools/help_files.py` to list it in `resources/resources.qrc`.

## How the wiki is updated

On every push to `main`, the build workflow (`.github/workflows/build.yml`)
does this:

1. The Linux build runs `JeffPub --export-help-wiki help-wiki`. The program
   writes the pages itself, so they show what Help shows, including the table
   of keyboard shortcuts, which is made from the program's commands. Every
   build does this step, pull requests too, so a link to a topic that doesn't
   exist fails the build before it is merged.
2. The `wiki` job (pushes to `main` only) runs `tools/publish_help_wiki.sh`.
   It replaces the wiki's pages with these, deletes pages that no longer
   exist, and commits "Help from `<short commit>`" as JeffOffice LLC. If
   nothing changed, it commits nothing.

The wiki needs a first page before this can work: GitHub makes the wiki's
repository only when someone saves a page on the repository's Wiki tab. Until
then the `wiki` job prints a notice and finishes without error.

## The pages

- Each topic is a page named from its title (its first `# ` line), with the
  spaces as hyphens: "Get help and report problems" becomes
  `Get-help-and-report-problems.md`. Characters that a wiki page name can't
  hold (`/ \ : * ? " < > | # %`, and brackets and parentheses) are left out.
- The contents topic (`index`) is `Home.md`. `_Sidebar.md` is the contents,
  and `_Footer.md` says that the pages are generated and where to edit them.
- Links between topics, `[Add Text Art](text-art)`, become links to pages,
  `[Add Text Art](Add-Text-Art)`. Web links stay as they are. A link to a
  topic that doesn't exist is an error.
- The title line is left out, because the wiki shows the page's name as the
  title. It stays only where the name would read differently (on Home, and on
  a title with a hyphen in it).
- Comments, such as the `keywords` line, are left out, as they are in the
  program.

## Making the pages on your own computer

```
QT_QPA_PLATFORM=offscreen build/JeffPub --export-help-wiki /tmp/help-wiki
```

The folder must be new or empty. The program makes the pages without opening a
window and exits with 0, or with 1 and a message when something is wrong.

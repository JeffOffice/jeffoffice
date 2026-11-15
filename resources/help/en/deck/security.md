# Security: what a file may reach out to
<!-- keywords: trust center, security, macros, linked files, links, linked pictures, linked media, internet, downloaded, read-only, protected, external links, programs, safe -->

A file can point to other things: files, web pages, movies, even programs. The **Security** page is where you decide what JeffDeck does about that. Click **File > Options**, then the **Security** tab. Click **OK** to keep your changes, or **Cancel** to throw them away.

These choices belong to all the JeffOffice apps: change one in one app and it changes in all of them. A row for something an app does not have says **This doesn't apply** and is grayed out.

## Macros

Macros are small programs kept inside a file. JeffDeck keeps them in the file, so they are still there when you save, and it **never runs them**. There is no setting that runs them.

## Linked files and workbook links

This does not apply in JeffDeck: it has no linked files to update. The row is grayed out.

## Linked movies and sounds

This is about a movie or sound that a slide **links** to instead of holding inside the presentation.

- **Never play it** is the usual choice. A linked movie or sound says that it is linked and does not play.
- **Ask me first** asks once for each linked file the first time you play it.
- **Play it** plays it without asking.

Even with **Play it**, only a movie or sound file whose name is on this computer plays. A web address, a file on another computer, a program, a link or special file, and a file that is not really a movie or sound never play. A linked file can be up to 256 MB. The file is read in the background for up to 20 seconds, so a slow drive can't freeze JeffDeck. A network drive that looks like a local one (a mapped drive, or a folder under /Volumes or /mnt) is read like a local file, so it can be slow. With **Ask me first**, a slide show asks one time, when it starts, for all the linked files in it.

## Files from the internet or other computers

A file you downloaded, or that sits on another computer in your network, may not be safe. This choice says how JeffDeck opens it.

- **Open as read-only first** is the usual choice. The file opens so you can read it, but you can't change it. A bar under the file says so. Click **Enable Editing** on the bar to change that one file (until you close the program). When you save it in place afterward, the download mark is taken off the file. Or use **Save As** to make a copy, or switch this setting to the next choice.
- **Open normally** opens every file the same way.

A file you open with **Open as Copy**, **Open and Repair**, or **Open Read-Only** from the **File** screen opens that way whatever this says.

## Links that start a program

A link that would start a program or a script (a file ending in .exe, .bat, .sh, and so on) is **always refused**. JeffDeck tells you, and nothing runs. A link to a file on another computer is never followed without asking you first. No setting changes this.

## Related

- [Change JeffDeck's settings](options)
- [Run the slide show](show)

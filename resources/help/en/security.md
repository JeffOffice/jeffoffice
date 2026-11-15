# Security: what a file may reach out to
<!-- keywords: trust center, security, macros, linked files, links, linked pictures, linked media, internet, downloaded, read-only, protected, external links, programs, safe -->

A file can point to other things: files, web pages, movies, even programs. The **Security** page is where you decide what JeffPub does about that. Click **File > Options**, then the **Security** tab. Click **OK** to keep your changes, or **Cancel** to throw them away.

These choices belong to all the JeffOffice apps: change one in one app and it changes in all of them. A row for something an app does not have says **This doesn't apply** and is grayed out.

## Macros

Macros are small programs kept inside a file. JeffPub keeps them in the file, so they are still there when you save, and it **never runs them**. There is no setting that runs them.

## Linked files and workbook links

This is about pictures that are **linked** to a file instead of kept inside the publication. The choice is what JeffPub does when it opens a publication that has some.

- **Ask me first** is the usual choice. If a linked picture's file has changed, JeffPub asks whether to update the picture from the file. If nothing has changed, it does not ask.
- **Update them** reads the files without asking.
- **Never update them** leaves every picture as it was saved. Each one shows **Not updated** until you use **Update Link** on it.

Whatever you choose, JeffPub only reads a file that is in the publication's own folder (or a folder you have said is all right). Any other linked file waits until you ask for it.

## Linked movies and sounds

This does not apply in JeffPub: it has no linked movies or sounds to play. The row is grayed out.

## Files from the internet or other computers

A file you downloaded, or that sits on another computer in your network, may not be safe. This choice says how JeffPub opens it.

- **Open as read-only first** is the usual choice. The file opens so you can read it, but you can't change it. The status bar says so. To change it, use **Save As** to make a copy, or switch this setting to the next choice.
- **Open normally** opens every file the same way.

A file you open with **Open as Copy**, **Open and Repair**, or **Open Read-Only** from the **File** screen opens that way whatever this says.

## Links that start a program

A link that would start a program or a script (a file ending in .exe, .bat, .sh, and so on) is **always refused**. JeffPub tells you, and nothing runs. A link to a file on another computer is never followed without asking you first. No setting changes this.

## Related

- [Change JeffPub's settings](options)
- [Pictures](pictures)

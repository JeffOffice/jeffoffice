# Security: what a file may reach out to
<!-- keywords: trust center, security, macros, linked files, links, linked pictures, linked media, internet, downloaded, read-only, protected, external links, programs, safe, trust centre -->

A file can point to other things: files, web pages, movies, even programs. The **Security** page is where you decide what JeffCrunch does about that. Click **File > Options**, then the **Security** tab. Click **OK** to keep your changes, or **Cancel** to throw them away.

These choices belong to all the JeffOffice apps: change one in one app and it changes in all of them. A row for something an app does not have says **This doesn't apply** and is grayed out.

## Macros

Macros are small programs kept inside a file. JeffCrunch keeps them in the file, so they are still there when you save, and it **never runs them**. There is no setting that runs them.

## Linked files and workbook links

This is about the other workbooks that a workbook gets numbers from, called **links**. The choice is what JeffCrunch does when it opens a workbook that has some.

- **Ask me first** is the usual choice. JeffCrunch asks whether to update the numbers from the other workbooks.
- **Update them** updates them without asking.
- **Never update them** keeps the numbers the file stored.

A choice you make for one workbook in **Data > Edit Links** (**Startup prompt**) wins for that workbook. A workbook that says "never" is always left alone. A workbook on another computer is never read when a file opens, whatever you choose. You can still use **Update Values** in **Edit Links** yourself at any time.

## Linked movies and sounds

This does not apply in JeffCrunch: it has no linked movies or sounds to play. The row is grayed out.

## Files from the internet or other computers

A file you downloaded, or that sits on another computer in your network, may not be safe. This choice says how JeffCrunch opens it.

- **Open as read-only first** is the usual choice. The file opens so you can read it, but you can't change it. A bar under the file says so. Click **Enable Editing** on the bar to change that one file (until you close the program). When you save it in place afterward, the download mark is taken off the file. Or use **Save As** to make a copy, or switch this setting to the next choice.
- **Open normally** opens every file the same way.

A file you open with **Open as Copy**, **Open and Repair**, or **Open Read-Only** from the **File** screen opens that way whatever this says.

## Links that start a program

A link that would start a program or a script (a file ending in .exe, .bat, .sh, and so on) is **always refused**. JeffCrunch tells you, and nothing runs. A link to a file on another computer is never followed without asking you first. No setting changes this.

## Related

- [Change JeffCrunch's settings](options)
- [Notes and links](notes-links)

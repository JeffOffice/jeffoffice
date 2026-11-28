# Security: what a file may reach out to
<!-- keywords: trust center, security, macros, linked files, links, linked pictures, linked media, internet, downloaded, read-only, protected, external links, programs, safe, trust centre -->

A file can point to other things: files, web pages, movies, even programs. The **Security** page is where you decide what JeffCrunch does about that. Click **File > Options**, then the **Security** tab. Click **OK** to keep your changes, or **Cancel** to throw them away.

These choices belong to all the JeffOffice apps: change one in one app and it changes in all of them. A row for something an app does not have says **This doesn't apply** and is grayed out.

## Macros

Macros are small programs kept inside a file. JeffCrunch keeps them in the file, so they are still there when you save, and it **never runs them**. There is no setting that runs them.

## Linked files and workbook links

This is about the other files that a workbook reads, such as the workbooks it gets numbers from, called **links**, the files its queries read, and the files its pivot tables are made from. The choice is what JeffCrunch does when it opens a workbook that reads some, and when you use **Refresh** or **Refresh All** on its queries and pivot tables.

- **Ask me first** is the usual choice. Before JeffCrunch reads one of those files, it shows the full path of the file and asks. Click **Read the File** only if you trust where the workbook came from. **Don't Read**, the standard button, reads nothing and leaves the workbook as it was. This choice asks each time you open a workbook, and then once for each file. After you click **Read the File** for a file, JeffCrunch does not ask about that same file again in that window, until you close the window.
- **Update them** reads a file without asking only if you have already said yes to that same workbook reading that same file. For every other file, including any file named in a workbook someone just sent you, it still asks first, the same way.
- **Never update them** reads nothing when a workbook opens, and shows no question. The numbers the file stored stay as they are. When you click **Refresh** yourself, that is a request, so JeffCrunch shows the question then. After you click **Read the File**, it does not ask about that same file again in that window.

## What is remembered

When you click **Read the File** or **Read the Files**, JeffCrunch remembers that pair: this workbook, and that file, each by its full path. Only the pair is remembered. If the workbook is moved or renamed, edited to read a different file, or the file is replaced by a shortcut to another file, JeffCrunch asks again. A workbook that you have never saved has no path, so nothing about it is remembered after you close it.

The pairs are kept only in your own settings on this computer, never in a workbook, so a workbook cannot add one, and nobody who gets a workbook from you gets them. The settings keep a scrambled form of each path, not the path itself. Up to 2,000 pairs are kept; when there are more, the pair used longest ago is forgotten first.

**Options > Security** shows how many pairs are remembered (**Remembered approvals**). Click **Forget All** to forget every one of them at once. No workbook is changed, and each one asks again the next time.

A workbook that says "never" in its own **Startup prompt** (**Data > Edit Links**) is always left alone. A workbook on another computer is never read when a file opens, whatever you choose. You can still use **Update Values** in **Edit Links** yourself at any time.

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

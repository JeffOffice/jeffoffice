# Notes and links
<!-- keywords: note, notes, comment, comments, hover, red mark, corner, new note, edit note, delete note, show note, hide note, show all notes, link, links, hyperlink, recent files, address, web page, e-mail, place in this workbook, Ctrl+K, Ctrl+click, screen tip, open link, remove link, conversation, workbook statistics, count, cells, formulas, charts, pivot tables, accessibility, alt text, merged cells -->

## Notes

A **note** is a few words kept with a cell. A cell that has one shows a small red mark in its upper-right corner. Rest the pointer on the cell, and the note appears beside it; move the pointer away, and it goes.

To put a note on a cell, select it and use **Review > New Note** (or press **Shift+F2**). Type the words and click **OK**. On a cell that has a note the same command reads **Edit Note**. Emptying the words takes the note away.

- **Delete Note** takes away the notes on the selected cells.
- **Show or Hide Note** keeps the note of the selected cells on the sheet all the time, in a box beside each cell, or puts it away again.
- **Show Every Note** does the same for every note on the sheet; the same button then reads **Hide Every Note**.
- **Previous Note** and **Next Note** go from one cell with a note to the next.

Each of these is one step you can undo. A note goes with its cell when rows or columns are inserted or deleted, and when cells are sorted. **Clear All** takes the notes off the cells it clears.

A workbook may hold a conversation of comments, with replies. JeffCrunch shows the whole conversation as a note and keeps it when you save, but it does not change it: the note is read-only.

Notes can be saved in an .xlsx workbook. A workbook opened from an older kind of file, or from a text file, keeps its notes only after you save it as an .xlsx workbook.

## Links

A **link** sends you somewhere when you follow it: to a web page, to a file, to an e-mail address, or to a place in this workbook (a cell, cells, or a name).

To put a link on a cell, select it and use **Insert > Link** (or press **Ctrl+K**). Choose where it goes, type the address or the place (or pick one of the files you opened lately from **Or a recent file**), and, if you like, the words the cell should show and a **screen tip**. On a cell that has a link the same command reads **Edit Link**. **Remove Link** takes the links off the selected cells.

Rest the pointer on a cell with a link to see where it goes. To follow it, hold **Ctrl** and click the cell, or select the cell and use **Open Link** from the right-click menu. A plain click only selects the cell.

JeffCrunch never opens anything by itself. A web page or an e-mail address opens in your usual program when you follow its link. A link to a file, or to another kind of address, asks you first. A link that would run a script is never opened.

A link goes with its cell when rows or columns are inserted or deleted, and when cells are sorted, and it is saved with the workbook.

## Workbook Statistics

**Workbook Statistics** (**Review** tab) counts what the workbook holds: how many **Sheets** it has, how many **Cells with data** (cells that hold a number, text, or a formula), how many **Formulas**, how many **Charts**, and how many **Pivot tables**. Sheets you deleted are not counted. The same numbers are on the **Info** page of the File tab.

## Check Accessibility

**Check Accessibility** (**Review** tab) opens a small window that lists what in the workbook is hard for some readers to use, or for a screen reader (a program that reads the screen aloud) to say. Each line starts with **Error**, **Warning**, or **Tip**. Click a line to go to the cell.

It looks for:

- A chart or picture with no **alt text** (the words that say what it shows). This is an error.
- A table with no header row, so its columns have no names. This is an error.
- **Merged cells**, which are hard to move through with the keyboard. This is a warning.
- Text that is hard to read against the color behind it, such as pale gray text on white. JeffCrunch measures how much the text stands out from the fill. Small text needs a ratio of at least 4.5 to 1, and large text (18 points, or bold 14 points) needs 3 to 1. It names the first cell with each combination of font and fill. This is a warning.
- A sheet that still has the name it was given, such as Sheet1. This is a tip.

Press **Check Again** after you make changes.

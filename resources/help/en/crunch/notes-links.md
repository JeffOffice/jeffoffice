# Notes, comments, and links
<!-- keywords: comments pane, new comment, reply, resolve, reopen, thread, previous comment, next comment, show comments, convert notes to comments, convert comments to notes, thesaurus, synonyms, word list, note, notes, comment, comments, hover, red mark, corner, new note, edit note, delete note, show note, hide note, show all notes, link, links, hyperlink, address, web page, e-mail, place in this workbook, Ctrl+K, Ctrl+click, screen tip, open link, remove link, conversation, workbook statistics, count, cells, formulas, charts, pivot tables, accessibility, alt text, merged cells, recent files -->

## Notes

A **note** is a few words kept with a cell. A cell that has one shows a small red mark in its upper-right corner. Rest the pointer on the cell, and the note appears beside it; move the pointer away, and it goes.

To put a note on a cell, select it and use **Review > New Note** (or press **Shift+F2**). Type the words and click **OK**. On a cell that has a note the same command reads **Edit Note**. Emptying the words takes the note away.

- **Delete Note** takes away the notes on the selected cells.
- **Show or Hide Note** keeps the note of the selected cells on the sheet all the time, in a box beside each cell, or puts it away again.
- **Show Every Note** does the same for every note on the sheet; the same button then reads **Hide Every Note**.
- **Previous Note** and **Next Note** go from one cell with a note to the next.

Each of these is one step you can undo. A note goes with its cell when rows or columns are inserted or deleted, and when cells are sorted. **Clear All** takes the notes and the comments off the cells it clears.

A cell can also hold a conversation of **comments**, with replies. See the next part.

Notes can be saved in an .xlsx workbook. A workbook opened from an older kind of file, or from a text file, keeps its notes only after you save it as an .xlsx workbook.

## Comments

A **comment** is like a note, but it starts a **thread**: people can reply to it, and one of them can mark the thread **resolved** when the question is answered. Each comment shows who wrote it and when. A cell with a thread has the same red mark as a cell with a note. Rest the pointer on the cell to read the whole thread, with each reply set in under the comment it answers. A thread that is resolved starts with the word **Resolved**.

On the **Review** tab, in the **Comments** group:

- **New Comment** (**Ctrl+Shift+F2**) starts a thread on the selected cell. Type the words and click **OK**. It works on a cell that has no note and no comments.
- **Reply** adds your words to the thread on the selected cell.
- **Resolve Thread** marks the thread as done. The same button then reads **Reopen Thread**, which opens it again.
- **Delete Comment** takes away the threads on the selected cells.
- **Previous Comment** and **Next Comment** go from one cell with a thread to the next.
- **Show Comments** opens the **Comments** pane beside the sheet. It lists every thread on the sheet as a card: the cell, each comment with its author and time, and the buttons **Reply**, **Resolve** or **Reopen**, and **Delete**. **Go To** moves to the cell. The pane follows every change, and Undo too.
- **Convert Notes to Comments** turns every plain note on the sheet into a thread of one comment. **Convert Comments to Notes** turns every thread back into a note. A note has no replies, so the comments of a thread become one note with all their words, and JeffCrunch asks before it does that to a thread that has replies.

Each of these is one step you can undo.

Programs that cannot show threads show a plain note in their place. JeffCrunch keeps that note, with the words of the thread, each time it saves. Threads, replies, and the people who wrote them stay as they were when you save and open the workbook again. A thread with more than 500 comments is shown and kept, but JeffCrunch does not add to it or change it.

Comments are saved in an .xlsx workbook. A workbook opened from an older kind of file, or from a text file, keeps its comments only after you save it as an .xlsx workbook.

## Links

A **link** sends you somewhere when you follow it: to a web page, to a file, to an e-mail address, or to a place in this workbook (a cell, cells, or a name).

To put a link on a cell, select it and use **Insert > Link** (or press **Ctrl+K**). Choose where it goes, type the address or the place (or pick one of the files you opened lately from **Or a recent file**), and, if you like, the words the cell should show and a **screen tip**. On a cell that has a link the same command reads **Edit Link**. **Remove Link** takes the links off the selected cells.

Rest the pointer on a cell with a link to see where it goes. To follow it, hold **Ctrl** and click the cell, or select the cell and use **Open Link** from the right-click menu. A plain click only selects the cell.

JeffCrunch never opens anything by itself. A web page or an e-mail address opens in your usual program when you follow its link. A link to a file, or to another kind of address, asks you first. A link that would run a script is never opened.

A link goes with its cell when rows or columns are inserted or deleted, and when cells are sorted, and it is saved with the workbook.

## Workbook Statistics

**Workbook Statistics** (**Review** tab) counts what the workbook holds: how many **Sheets** it has, how many **Cells with data** (cells that hold a number, text, or a formula), how many **Formulas**, how many **Charts**, and how many **Pivot tables**. Sheets you deleted are not counted. The same numbers are on the **Info** page of the File tab.

## Thesaurus

**Thesaurus** (**Review** tab, or **Shift+F7**) finds words that mean nearly the same as a word in a cell. It looks up the first word of the text in the cell you are on. Type another word in **Look up** and press **Enter** to look up that one instead. Pick a word in the list and click **Replace**, or double-click it. The word in the cell is replaced, and a capital first letter stays a capital. **Look Up** looks up the word you picked, to follow a chain of meanings. The word list works in English.

## Check Accessibility

**Check Accessibility** (**Review** tab) opens a small window that lists what in the workbook is hard for some readers to use, or for a screen reader (a program that reads the screen aloud) to say. Each line starts with **Error**, **Warning**, or **Tip**. Click a line to go to the cell.

It looks for:

- A chart or picture with no **alt text** (the words that say what it shows). This is an error.
- A table with no header row, so its columns have no names. This is an error.
- **Merged cells**, which are hard to move through with the keyboard. This is a warning.
- Text that is hard to read against the color behind it, such as pale gray text on white. JeffCrunch measures how much the text stands out from the fill. Small text needs a ratio of at least 4.5 to 1, and large text (18 points, or bold 14 points) needs 3 to 1. It names the first cell with each combination of font and fill. This is a warning.
- A sheet that still has the name it was given, such as Sheet1. This is a tip.

Press **Check Again** after you make changes.

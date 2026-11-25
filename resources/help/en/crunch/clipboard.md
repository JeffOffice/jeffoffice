# Cut, copy, and paste
<!-- keywords: cut, copy, paste, clipboard, move, duplicate, special, values, formulas, formats, transpose, skip blanks, add, subtract, multiply, divide, operation, merged, text, tab, html, web page, other program, Escape, marching ants, border, column widths, notes, comments, number formats, all except borders, picture, bitmap, copy as picture, printed picture, as printed, paste link, hyperlink, text import, delimited text, drop text, comma, semicolon, csv,  separator, set default paste, default paste, fixed width -->

**Copy** (**{key:Copy}**) puts a copy of the selected cells on the clipboard, a place the computer keeps for one thing at a time. **Paste** (**{key:Paste}**) puts what is on the clipboard into the cells starting at the selected cell. **Cut** (**{key:Cut}**) is like copy, but when you paste, the cells are moved and the old place is emptied.

A moving dashed border shows the cells that were copied or cut. Press **Esc** to take the border away; the cells are then no longer ready to paste. Cut cells are also let go when you make any other change before pasting them (typing, inserting, undoing): cut them again if you still want to move them.

## Paste inside JeffCrunch

Everything comes along: the values, the formulas, the formats, and the merged cells. A formula keeps pointing at the same places *relative to where it is*. Example: copy **=A1*2** from B1 and paste it into B5, and it becomes **=A5*2**. A part with a dollar sign, such as **$A$1**, stays as it is. If you select a block that is a whole number of times the size of what you copied, the copy is repeated to fill it: copy one cell, select twenty cells, paste, and all twenty get it.

A pasted number that is too wide for a column that still has its first width widens the column.

## Cut moves cells

When you cut cells and paste them, the cells really move. Every formula that pointed at them points at them in their new place. Example: B1 has **=A1+A2**. Cut A1:A2 and paste at E4, and B1 becomes **=E4+E5**. Whatever was in the cells you pasted over is gone (**Undo** brings it back), and a formula that read one of those cells shows **#REF!**, because the cell it read is no longer there. If the cells you cut are only part of a merged cell, or you paste over only part of one, nothing is moved: select the whole merged cell. When you cut cells onto another sheet, the formulas keep reading the cells they read before (they say which sheet). The clipboard is emptied after a cut and paste.

## Paste Special

**Ctrl+Alt+V**, or the arrow under **Paste** on the Home tab, lets you choose what to paste:

- **All**, **Formulas**, **Values**, or **Formats**. *Values* turns a formula into the number or text it shows.
- **All Except Borders**: everything, but the cells keep the borders they had.
- **Formulas and Number Formats**, or **Values and Number Formats**: the formulas (or the values) come with how the numbers are shown, such as percent or a date, but not the colors, fonts, or borders. The cells keep the look they had.
- **Column Widths**: the columns you paste into get the widths of the columns you copied.
- **Comments and Notes**: the notes of the copied cells are put on the cells you paste into. No cell value changes.
- An **operation**: **Add**, **Subtract**, **Multiply**, or **Divide**. The numbers you copied are combined with the numbers already in the cells. Example: the cells hold 10 and 20, you copied 5, and you chose *Multiply*: they become 50 and 100.
- **Skip blanks**: empty cells in the copy leave the cells they land on as they were.
- **Transpose**: rows become columns and columns become rows.

## The Paste Options button

Right after you paste cells you copied in JeffCrunch, a small **Paste Options** button shows up by the last cell you pasted. Click it to pick another way to put the same cells in: **Keep Source Formatting** (everything), **Values**, **Formulas**, **Formatting**, **No Borders**, **Formulas and Number Formatting**, **Values and Number Formatting**, **Keep Source Column Widths**, or **Transpose**. JeffCrunch takes the paste back and does it again that way. The change is one step: a single **Undo** takes the whole paste back, however many times you changed your mind. Example: copy A1:B2, where B1 is **=A1*2**, and paste at F5. Choose **Values** and G5 holds the number 2 instead of **=F5*2**; choose **Keep Source Formatting** and the formula is back.

From the keyboard, tap the **Ctrl** key (the **Control** key on a Mac) after a paste to open the button's menu, then use the arrow keys and **Enter** to choose. The button goes away when you change a cell or anything else in the workbook, copy something else, press **Esc**, or scroll the last pasted cell out of sight (it comes back when you scroll back). It is not offered after a cut and paste, or for text from another program.

## Paste Link, Paste as Hyperlink, and Paste Text Import

Three more choices are in the arrow under **Paste**. They work on cells you copied in the same workbook, except the last one.

- **Paste Link** puts a formula in each cell that points at the cell you copied. Example: copy A1:B2, select D4, and use **Paste Link**: D4 becomes **=Sheet1!$A$1**. When A1 changes, D4 changes with it. A copied cell that is empty shows 0.
- **Paste as Hyperlink** puts the same words in each cell that the copied cell shows, and makes each one a link to the copied cell. Hold **Ctrl** and click one to go to that cell. It works on up to 2,000 cells at a time.
- **Paste Text Import** is for text from another program. It opens the same window as opening a text file: you say what separates the pieces (a comma, a tab, a space, a semicolon, a bar, or something else), what quotes a piece, the first row to use, and how to read each column (as typed, as text, as a date, or left out). Some text lines its pieces up in columns of the same width, with spaces between. Tick **The columns have a fixed width**, and say where each column begins: each line is cut at those places, and the spaces around each piece are taken off. Choose *text* for a column of codes such as 007, so the zeros stay. The pieces go into cells, starting at the selected cell, as one step you can undo.

## Text with commas, semicolons, or columns

When the text you paste from another program is clearly a table, JeffCrunch opens the same window as opening a text file, so you can check how it is split before anything goes into the cells. This happens only with a plain **Paste** (**{key:Paste}**), never with **Paste Special** or one of its choices. It happens when the text has at least two lines and no tab, and either every line has the same number of pieces, at least three, split by commas, semicolons, or bars, with something in every column (a piece in quotes can hold a comma); or the pieces line up in columns of the same width, with the same gaps of spaces on every line (at least three lines, and at least two gaps). Numbers with thousands commas, such as 1,234, and lines of code or lists that end in a comma or a semicolon are not taken for a table. The window starts with the way the text was found to be split. If the numbers have commas in them and the pieces are split by semicolons, the comma is the decimal mark. Click **OK** to put the pieces in, or **Cancel** to put in nothing.

Example: copy these lines from an email.

    name,qty,price
    Apple,12,1.50
    Pear,3,0.25

Paste them with a third column, such as a price, and the window shows three columns, with **Comma** ticked. One sentence on one line, or lines that have different numbers of commas, or only two pieces to a line, are pasted as words, with no window. Text with tabs goes straight into the cells, as it always did. To open the window for any text at all, choose **Paste Text Import** (above).

The same happens when you drag text from another program and drop it on the grid: it goes in at the selected cell. A drop anywhere else on the window does nothing.

## Set Default Paste

**Set Default Paste** is the last choice in the arrow under **Paste**. It decides what the **Paste** button and **{key:Paste}** put in when you copied cells: everything (at first), or only the values, only the formulas, only the formatting, and so on. Example: choose *Values*, and a copied formula lands as the number it shows. The choice is kept for the next time. It does not change **Paste Special** or the choices in the menu, and text from other programs is pasted as it is. Choose *All* to go back.

The same choice is on the **Editing** page of **Options**, in two boxes: one for cells you copied in this workbook, and one for cells you copied in another workbook. Until you choose one for another workbook, it follows the first.

## The Paste menu and pictures

The arrow under **Paste** lists the usual choices, and also **No Borders**, **Formulas and Number Formatting**, **Values and Number Formatting**, **Keep Source Column Widths**, and **Comments and Notes**. They are the same as the choices in Paste Special, one click each.

The arrow next to **Copy** has **Copy as Picture** and **Copy as Bitmap**. Each puts a picture of the selected cells on the clipboard, drawn as you see them, with the gridlines. A bitmap is the size of the screen. A picture has twice as many dots, so it stays sharp when it is made larger. **Copy as Printed Picture** draws the cells as the paper would show them: the gridlines are there only if the page setup prints them. Paste it into JeffWrite or JeffDeck as an image. If the cells do not fit in one picture, select fewer.

## Between programs

When you copy, JeffCrunch also puts the cells on the clipboard as text (cells apart by tabs, one row to a line) and as a table for web pages, so other programs can take them. When you paste something that did not come from JeffCrunch, text with tabs, or a table from a web page or another spreadsheet, each piece goes into its own cell and is read as if you typed it: numbers, dates, and formulas that start with an equals sign. Copying from another workbook open in JeffCrunch brings values, formulas, and number formats, but not the other formatting. Paste them in JeffDeck or JeffPub, and they land as a table that keeps the fonts, shades, borders, and the numbers as they show (see **Paste cells as a table** in their Help).

## What can't be pasted

JeffCrunch tells you and changes nothing when the paste would reach into part of an **array formula**, over the row of column names of a **table**, into locked cells of a **protected sheet**, past the end of the sheet, or into more than 100,000 cells. A whole **array formula** can be cut and moved (select all of its cells); part of one, a data table, or the names row of a table can't.

## The Clipboard pane

The Clipboard pane remembers the last 24 things you copied or cut in this window's program, so you can paste an older one without copying it again. Click the small arrow in the corner of the **Clipboard** group on the **Home** tab (or press **Alt**, then **H**, then **FO**) to open it. A panel appears beside the page.

Each item shows a small preview and what kind of thing it is: text, formatted text, cells, a picture, shapes, or slides. The newest one is at the top. Click an item to paste it where the cursor or the selection is. It pastes like any other paste: one step of **Undo** takes it back, and the Paste Options button appears as usual.

- **Paste All** pastes every item, the oldest first, so they read in the order you copied them.
- **Delete** removes the item you picked. **Clear All** removes all of them.
- The pane collects only while it is open. To keep collecting after you close it, tick **Keep collecting when this pane is closed**. The choice is remembered, and collecting starts when the program starts, before you open the pane.
- To also collect what you copy in other programs, tick **Also collect what is copied in other programs**. Only plain text and pictures are kept, and it starts off again each time you open the program.

The items are kept in the program's memory only. Nothing is saved to a file, and the list is emptied when you close the last window of the program. An item stays in the list after the document it came from is closed. An item bigger than 8 MB is not kept, and the pane tells you when that happens; a picture counts 4 bytes for each dot (pixel), so a picture of more than about 2 million pixels is too big. All the items together are held to 64 MB, and the oldest go first. Anything a password manager marks as secret is never kept.

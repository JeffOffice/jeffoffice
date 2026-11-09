# Type in cells
<!-- keywords: autocorrect, smart quotes, dash, fraction, link, address, bold word, part of a cell, rich text, format part of text, color a word, runs, type, typing, enter, edit, change, cell, entry, formula bar, F2, double click, Tab, Escape, give up, undo, redo, delete, clear, percent, currency, money, date, time, number, text, apostrophe, formula, equals, replace, limit, characters, colour a word -->

To put something in a cell, select the cell and start typing. What you type replaces what the cell held. Press **Enter** to keep it, or **Esc** to give it up and leave the cell as it was.

## Three ways to begin

- **Type over the cell.** Select a cell and type. The first character you type starts the entry, and the cell shows what you type as you go.
- **Edit what is in the cell.** Press **F2**, or double-click the cell. The cell opens with its contents, and the cursor is at the end. Use the arrow keys to move in the text.
- **Use the formula bar.** Click in the formula bar, which is next to the Name box, and type there. The cell shows what you type too.

When you type over a cell, an arrow key finishes the entry and moves the selection. When you edit what is in the cell, the arrow keys move in the text. Press **F2** to change from one to the other.

## Finish the entry

| Keys | What they do |
| --- | --- |
| Enter | Keep the entry, and move down (or where **File > Options > Editing** says) |
| Shift+Enter | Keep the entry, and move the other way |
| Ctrl+Enter | Put the entry in every selected cell, and stay where you are |
| Tab | Keep the entry, and move right |
| Shift+Tab | Keep the entry, and move left |
| Arrow keys | Keep the entry, and move one cell (only when you typed over the cell) |
| Esc | Give the entry up |
| Alt+Enter | Start a new line inside the cell |
| Ctrl+; | Put in today's date (in the entry, when you are typing one) |
| Ctrl+Shift+; | Put in the time now |
| Alt+Down | List the words already in the column, and choose one |

**Ctrl+Enter** with several cells selected puts what you typed in all of them. A formula is written for the cell you typed in, and in each of the others it reads as a copy of it would: typing **=A1** with the block E6:F7 selected gives **=B2** in F7. It is one step of Undo.

**Alt+Down** on a cell lists, in order and once each, the words in the cells above and below it that are not separated by an empty cell. Choosing one types it. A cell with a list of its own shows that list instead.

In **File > Options > Editing**, choose whether **Enter** moves down, right, up, or left, or does not move.

If you type in several cells along a row with **Tab**, then press **Enter**, the selection goes down to the next row, in the column where you started. If you select a block of cells first, **Enter** and **Tab** move through the block.

## AutoCorrect while you type

When you type a space or a mark such as a period after a word, JeffCrunch can fix the word, as you finish it. It fixes common typing slips (such as "teh" for "the"), two capital letters at the start of a word, a small letter that starts a sentence or a day of the week, and a straight quote mark (it becomes a curly one). It also changes two hyphens between words to a long dash, and 1/2, 1/4, and 3/4 to ½, ¼, and ¾ when the entry has words in it. A number such as 1/2 typed alone stays a date or a fraction, as always.

Each fix is a step of its own. Press **{key:Undo}** right after a fix to take just that fix back and keep what you typed. Nothing is changed in a formula: an entry that starts with an equals sign, or with a plus or minus sign and then a formula, stays as you typed it. An address on the web (such as https://example.org) or an email address that is the whole entry becomes a link in its cell when you finish the entry, if **Turn web addresses into links as you type** is on.

Turn these on or off in **File > Options > Proofing**. A misspelled word is marked while you type: see [Spelling](notes-links).

## Format part of the text

Some words in a cell can look different from the rest. For example, one word can be bold or red.

1. Press F2 to edit the cell (or double-click it).
2. Select the words you want to change, with Shift and the arrow keys or with the mouse.
3. Use **Bold**, **Italic**, **Underline**, **Strikethrough**, **Font**, **Font Size**, or **Font Colour** on the **Home** tab, or Ctrl+B, Ctrl+I, and Ctrl+U.
4. Press Enter.

Only the words you selected change. The rest of the cell keeps the cell's own look. The entry box shows the words in one look while you edit, and the cell shows each part in its own look when you press Enter. If you type or delete words later, the formatting moves with the words. Choosing a font or size with no words selected changes the whole cell, as always.

Cells that have formatting inside the text from another program show it, keep it, and save it back. A cell you change keeps its formatting. Formatting inside a text applies to text, not to numbers or formulas. A copy saved as an older .xls file or as .ods keeps the words, and the save tells you that the formatting inside them is not kept.

## What you type becomes

JeffCrunch reads what you type the way your computer's settings say. The decimal mark, the thousands mark, the money sign, and the order of the day and the month in a date all come from those settings. A number you type in a cell that has no format of its own is given the format that shows it the way you typed it.

| You type | The cell holds | And shows |
| --- | --- | --- |
| 1234.5 | the number 1234.5 | 1234.5 |
| 5% | the number 0.05 | 5% |
| $12.50 | the number 12.5 | $12.50 |
| 1 1/2 | the number 1.5 | 1 1/2 |
| 3/4/2025 | the date, as a number of days | 3/4/2025 |
| 10:30 PM | the time, as a part of a day | 10:30 PM |
| TRUE | the logical value true | TRUE |
| #N/A | the error #N/A | #N/A |
| =B2*2 | a formula | the result of the formula |
| anything else | text | the text |

A cell that already has a format of its own, such as a percent or a date, keeps it. A cell whose format is **Text** keeps everything you type as text, except a formula.

To keep something that looks like a number as text, such as a code **007**, type an apostrophe first: **'007**. The apostrophe is not part of the text. When you edit the cell, you see it again.

## Formulas

A formula starts with an equals sign. For example, **=B2*2** doubles the number in B2. Type the names of functions in capitals or small letters; JeffCrunch writes them in capitals.

The names of functions are always the English ones, in every language: type **SUM**, not the name your language gives it. The marks between them follow your computer's settings. Where the decimal mark is a comma, as in German and French, you separate the arguments of a function with a semicolon and write a comma in a number: **=SUM(A1;A2)*0,5**. The formula bar and **F2** show formulas the same way. Where the decimal mark is a point, you separate arguments with a comma: **=SUM(A1,A2)*0.5**. The file always keeps the form every program reads, with commas between arguments and points in numbers, so it opens the same everywhere.

If a formula has a mistake, such as a bracket that is never closed, JeffCrunch tells you what is wrong, and the entry stays as you typed it, with the cursor near the mistake, so that you can fix it. Nothing is changed until the formula is right.

A cell holds up to 32,767 characters, and a formula up to 8,192. JeffCrunch tells you when an entry is longer.

## Clear cells

Select the cells and press **Delete**. This clears what the cells hold and keeps their formats, and it is one step you can undo. JeffCrunch clears up to 100,000 cells at a time.

On the **Home** tab, the **Clear** button has a menu. **Clear All** empties the cells, their formats, their notes, their comments (the threads with replies), and their links. A thread with more than 500 comments is kept, and JeffCrunch says so, because an Undo could not bring back the comments it never read. **Clear Formats** keeps what the cells hold and puts back the plain format. **Clear Contents** is the same as **Delete**. **Clear Comments and Notes** takes only the notes and the comments off the selected cells. **Clear Hyperlinks** takes only the links off, and the cells keep their text. A choice is dimmed when the selected cells have nothing for it to clear.

## Undo and Redo

**{key:Undo}** undoes the last change, and **{key:Redo}** does it again. They are also on the quick toolbar at the top. Each cell you type in is one step, and so is each clearing. Undo puts the cell back the way it was: its value, its formula, its format, and the formulas that depend on it. JeffCrunch remembers up to 1,000 steps, fewer when they changed very many cells, so that it never takes much memory.

## While JeffCrunch works the formulas out

When you change a cell, JeffCrunch works out the formulas that depend on it in the background. The cell you changed shows its new contents at once, and the other cells update a moment later; the status bar says **Calculating** until they have. You can go on typing meanwhile.

## What can't be changed

- A cell that is part of an **array formula** or a data table can't be changed alone.
- On a **protected sheet**, a cell that is locked can't be changed. Every cell is locked unless the workbook says it is not. JeffCrunch tells you when you try. To change the cell, choose Unprotect Sheet on the Review tab.
- The first row of a **table** holds the names of its columns. Type a new name in one of those cells to rename the column: the formulas that use the old name follow, and the table in the file stays valid. A name another column already has gets a number after it, a name can't be empty, and **Delete** doesn't clear that row.
- A workbook that could not be read whole can be looked at but not changed or saved.
- A function JeffCrunch does not have yet leaves a new formula without a value in the cell; the workbook asks the program that opens it next to work the formula out.
- Formatting, sorting, and find and replace come in a later version. Inserting and deleting, cut and paste, fill, and the sizes of rows and columns are in [Insert and delete rows, columns, and cells](insert-delete), [Cut, copy, and paste](clipboard), [Fill cells and add them up](fill-sum), and [Change the width of columns and the height of rows](sizes).

## New workbooks

In **File > Options > New Workbooks**, choose the font, the font size, and the number of sheets that every new workbook starts with. Workbooks you already have are not changed.

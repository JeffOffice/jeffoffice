# Type in cells
<!-- keywords: type, typing, enter, edit, change, cell, entry, formula bar, F2, double click, Tab, Escape, give up, undo, redo, delete, clear, percent, currency, money, date, time, number, text, apostrophe, formula, equals, replace, limit, characters -->

To put something in a cell, select the cell and start typing. What you type replaces what the cell held. Press **Enter** to keep it, or **Esc** to give it up and leave the cell as it was.

## Three ways to begin

- **Type over the cell.** Select a cell and type. The first character you type starts the entry, and the cell shows what you type as you go.
- **Edit what is in the cell.** Press **F2**, or double-click the cell. The cell opens with its contents, and the cursor is at the end. Use the arrow keys to move in the text.
- **Use the formula bar.** Click in the formula bar, which is next to the Name box, and type there. The cell shows what you type too.

When you type over a cell, an arrow key finishes the entry and moves the selection. When you edit what is in the cell, the arrow keys move in the text. Press **F2** to change from one to the other.

## Finish the entry

| Keys | What they do |
| --- | --- |
| Enter | Keep the entry, and move down |
| Shift+Enter | Keep the entry, and move up |
| Tab | Keep the entry, and move right |
| Shift+Tab | Keep the entry, and move left |
| Arrow keys | Keep the entry, and move one cell (only when you typed over the cell) |
| Esc | Give the entry up |
| Alt+Enter | Start a new line inside the cell |

If you type in several cells along a row with **Tab**, then press **Enter**, the selection goes down to the next row, in the column where you started. If you select a block of cells first, **Enter** and **Tab** move through the block.

## What you type becomes

JeffCrunch reads what you type the way your computer's settings say. The decimal mark, the thousands mark, the money sign, and the order of the day and the month in a date all come from those settings. A number you type in a cell that has no format of its own is given the format that shows it the way you typed it.

| You type | The cell holds | And shows |
| --- | --- | --- |
| 1234.5 | the number 1234.5 | 1234.5 |
| 5% | the number 0.05 | 5% |
| $12.50 | the number 12.5 | $12.50 |
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

## Undo and Redo

**{key:Undo}** undoes the last change, and **{key:Redo}** does it again. They are also on the quick access bar at the top. Each cell you type in is one step, and so is each clearing. Undo puts the cell back the way it was: its value, its formula, its format, and the formulas that depend on it. JeffCrunch remembers up to 1,000 steps, fewer when they changed very many cells, so that it never takes much memory.

## While JeffCrunch works the formulas out

When you change a cell, JeffCrunch works out the formulas that depend on it in the background. The cell you changed shows its new contents at once, and the other cells update a moment later; the status bar says **Calculating** until they have. You can go on typing meanwhile.

## What can't be changed

- A cell that is part of an **array formula** or a data table can't be changed alone.
- On a **protected sheet**, a cell that is locked can't be changed. Every cell is locked unless the workbook says it is not. JeffCrunch tells you when you try. To change the cell, take the protection off in the program that put it on.
- The first row of a **table** holds the names of its columns. Type a new name in one of those cells to rename the column: the formulas that use the old name follow, and the table in the file stays valid. A name another column already has gets a number after it, a name can't be empty, and **Delete** doesn't clear that row.
- A workbook that could not be read whole can be looked at but not changed or saved.
- A function JeffCrunch does not have yet leaves a new formula without a value in the cell; the workbook asks the program that opens it next to work the formula out.
- Formatting, inserting and deleting rows and columns, cut and paste, fill, sorting, and find and replace come in a later version.

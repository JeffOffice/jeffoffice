# Cut, copy, and paste
<!-- keywords: cut, copy, paste, clipboard, move, duplicate, special, values, formulas, formats, transpose, skip blanks, add, subtract, multiply, divide, operation, merged, text, tab, html, web page, other program, Escape, marching ants, border -->

**Copy** (**{key:Copy}**) puts a copy of the selected cells on the clipboard, a place the computer keeps for one thing at a time. **Paste** (**{key:Paste}**) puts what is on the clipboard into the cells starting at the selected cell. **Cut** (**{key:Cut}**) is like copy, but when you paste, the cells are moved and the old place is emptied.

A moving dashed border shows the cells that were copied or cut. Press **Esc** to take the border away; the cells are then no longer ready to paste.

## Paste inside JeffCrunch

Everything comes along: the values, the formulas, the formats, and the merged cells. A formula keeps pointing at the same places *relative to where it is*. Example: copy **=A1*2** from B1 and paste it into B5, and it becomes **=A5*2**. A part with a dollar sign, such as **$A$1**, stays as it is. If you select a block that is a whole number of times the size of what you copied, the copy is repeated to fill it: copy one cell, select twenty cells, paste, and all twenty get it.

A pasted number that is too wide for a column that still has its first width widens the column.

## Cut moves cells

When you cut cells and paste them, the cells really move. Every formula that pointed at them points at them in their new place. Example: B1 has **=A1+A2**. Cut A1:A2 and paste at E4, and B1 becomes **=E4+E5**. Whatever was in the cells you pasted over is gone (**Undo** brings it back), and a formula that read one of those cells shows **#REF!**, because the cell it read is no longer there. If the cells you cut are only part of a merged cell, or you paste over only part of one, nothing is moved: select the whole merged cell. When you cut cells onto another sheet, the formulas keep reading the cells they read before (they say which sheet). The clipboard is emptied after a cut and paste.

## Paste Special

**Ctrl+Alt+V**, or the arrow under **Paste** on the Home tab, lets you choose what to paste:

- **All**, **Formulas**, **Values**, or **Formats**. *Values* turns a formula into the number or text it shows.
- An **operation**: **Add**, **Subtract**, **Multiply**, or **Divide**. The numbers you copied are combined with the numbers already in the cells. Example: the cells hold 10 and 20, you copied 5, and you chose *Multiply*: they become 50 and 100.
- **Skip blanks**: empty cells in the copy leave the cells they land on as they were.
- **Transpose**: rows become columns and columns become rows.

## Between programs

When you copy, JeffCrunch also puts the cells on the clipboard as text (cells apart by tabs, one row to a line) and as a table for web pages, so other programs can take them. When you paste something that did not come from JeffCrunch, text with tabs, or a table from a web page or another spreadsheet, each piece goes into its own cell and is read as if you typed it: numbers, dates, and formulas that start with an equals sign. Copying from another workbook open in JeffCrunch brings values, formulas, and number formats, but not the other formatting.

## What can't be pasted

JeffCrunch tells you and changes nothing when the paste would reach into part of an **array formula**, over the row of column names of a **table**, into locked cells of a **protected sheet**, past the end of the sheet, or into more than 100,000 cells. Cells of an array or a table's header can't be cut and moved either.

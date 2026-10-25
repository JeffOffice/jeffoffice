# Tables
<!-- keywords: table, tables, structured reference, header row, totals row, calculated column, banded, striped, style, table looks, filter buttons, Ctrl+T, convert, plain cells, resize, column name, [@Column], Table1[Column] -->

A table is a block of cells with a row of names over its columns, treated as one thing: it has a name, a look, buttons to filter and sort its rows, and it grows when you add to it. Formulas can call its columns by name instead of by cell address.

## Make a table

Select the cells, or click a cell in the data, and choose **Table** on the **Insert** tab (**Ctrl+T**). JeffCrunch asks which cells the table covers and whether the first row holds the names of the columns. Names that are numbers, dates, or formulas become the text they show, empty names become Column1, Column2, and so on, and two columns with one name are numbered. If there are no names, the table has no row of names, and its columns are called Column1, Column2, and so on in formulas.

The **Table Looks** gallery (**Home** tab, or the **Table** tab) shows the looks: light, medium, and dark, each in black and in the six colors of the workbook's theme. Choosing one for cells that are not a table makes them one. The look is drawn under what a cell has of its own: a fill, borders, a text color, or bold that you set on a cell stays.

## The Table tab

The **Table** tab shows while the cursor is in a table.

- **Name** is the name formulas call the table by. Every formula that uses the old name follows. A name has letters, digits, underscores, and periods, begins with a letter or an underscore, and is not another table's, a name in the workbook, or a cell address such as A1.
- **Row of Names**, **Totals Row**, **Striped Rows**, **Striped Columns**, **First Column**, **Last Column**, and **Filter Buttons** are checked or unchecked for the table.
- **Change Table Size…** asks for the cells the table covers. It keeps its first cell; rows and columns can be added at the bottom and the right, or taken off.
- **Back to Plain Cells** turns the table into ordinary cells. They keep what they hold. A formula that named the table or its columns names the cells instead.

## The menu of the cells

Right-click a cell of a table and choose **Table**: **Insert Row Above**, **Insert Row Below**, **Insert Column Left**, **Insert Column Right**, **Delete Table Rows**, **Delete Table Columns**, **Totals Row**, and **Back to Plain Cells**. They put in or take out whole rows and columns of the sheet, as Insert and Delete do; one put in just past the table's end is taken in by the table.

## Totals row

Turn on **Totals Row** and the row under the table holds the word Total in its first column and a total in its last. Each cell of the row has a button: choose what the column's total shows (the average, a count, how many numbers there are, the largest, the smallest, the sum, the standard deviation, the variance, or **Your Own Formula**). A total is a SUBTOTAL of its column, so it leaves out rows a filter hides.

## Adding to a table

- Type in the row under a table, or in the column beside it, and the table grows to take the cell in. **Undo** takes the growth away and leaves what you typed.
- A formula typed in a cell of a column that is otherwise empty is put in every cell of that column: a calculated column. **Undo** right after, and the formula stays in the one cell. A formula typed over a calculated column changes the whole column when the other cells hold the old formula; if some do not, the cell stands alone.
- Rows inserted inside a table get the calculated columns' formulas. Columns inserted inside it are named in the row of names. Deleting rows or columns inside it makes it smaller; the row of names, and every column, can't all be deleted.

## Filter and sort a table

The buttons on the names filter and sort the table's rows only. They work as the buttons of a sheet's filter do (see [Sort and filter](sort-filter)). A table can't be filtered while a filter of the sheet, or of another table, covers the same rows, since each would need to know which rows are its own.

## Formulas that name a table

- `Sales[Amount]` is the cells of the Amount column, without its name and its total.
- `[@Amount]` is the cell of the formula's own row, and is written `Sales[[#This Row],[Amount]]` in the file.
- `Sales[#Headers]`, `Sales[#Data]`, `Sales[#Totals]`, and `Sales[#All]` are the names, the data, the totals, and the whole table, and `Sales[[#Headers],[Amount]]` is one column of them.
- `Sales[[Region]:[Amount]]` is the columns from Region to Amount.

Type `[` after a table's name and a list of its columns appears. While you type a formula, clicking a cell of a table writes a reference like `Sales[@Amount]` or `Sales[Amount]` (clicking a whole column of its data). Renaming a column in its header changes it in every formula; inserting and deleting columns keeps formulas pointed at the columns they named, and a column that is deleted leaves #REF! in the formulas that named it.

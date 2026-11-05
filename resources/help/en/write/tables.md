# Tables
<!-- keywords: delete cells, shift cells left, wrap text around a table, text wrapping, floating table, table, tables, insert table, rows, columns, cell, cells, merge cells, split cells, borders, shading, fill, cell size, column width, row height, fit, distribute rows, distribute columns, table properties, header row, repeat header, convert text to table, convert table to text, split table, sort, sort rows, formula, formulas, sum, view gridlines, table style, table styles, banded rows, alignment, align text in a cell, delete row, delete column, grid, gridlines, right click, table layout, table design -->

A **table** is a grid of **cells** in **rows** and **columns**. Each cell holds text, and you type in it as you do anywhere on the page. This topic shows how to add a table, change its shape and its look, and set it up for the pages it runs over.

## Add a table

- Click **Insert > Table**, then point at the grid and click the size you want. The first number is the columns across and the second is the rows down.
- Or click **Insert > Table > Insert Table** to type the number of columns and rows, and to say how the columns are fitted. **Fixed column width** keeps each column the width you give it (or, with *Auto*, shares the width of the text area among the columns). **Fit to contents** sizes the columns to the words in them. **Fit to window** makes the table as wide as the text area. Check **Remember dimensions for new tables** to start the next one with the same numbers.

The table goes where the caret is. If the caret is in the middle of a paragraph, the paragraph is cut in two and the table goes between the halves. The caret ends up in the first cell. Press **Tab** to go to the next cell, and **Shift+Tab** to go back. **Tab** in the last cell adds a row. A table can't be put inside a cell this way.

## Ready-made tables

Click **Insert > Table > Ready-Made Tables** and pick one. A calendar has the days of this month. A matrix has a top row and a left column to fill in. A list has four columns: item, description, quantity, and price. A schedule has the days of a work week and the hours of a day. The table has its words in place, so you can type over them.

## Make a table from text

Select the paragraphs, then click **Insert > Table > Convert Text to Table**. Say what marks where one cell ends and the next begins:

- **Tabs**, **Commas**, or **Other** (type the character, such as a semicolon). Each paragraph becomes a row, and the text between the marks becomes the cells of that row. The table has as many columns as the longest row, and a short row gets empty cells at its end.
- **Paragraphs**. Each paragraph becomes one cell. Type how many columns you want, and the cells fill a row at a time.

The words keep their bold, italics, and other looks. One step of **Undo** turns the table back into the paragraphs.

## Make text from a table

Put the caret in the table and click **Table Layout > Convert to Text**. Pick what goes between the words of the cells in a row: a tab, a comma, or another character. Or pick **Paragraph marks** to make each paragraph of each cell its own paragraph. A table that has another table in one of its cells has to be changed from the inside out: do the inner table first.

## The table tabs

While the caret is in a table, two tabs appear at the right of the ribbon, under the name **Table Tools**: **Table Design** and **Table Layout**. They go away when the caret leaves the table. You can also right-click in a cell to get the same commands in a menu.

## Select cells

Click in a cell, or drag from one cell to another to select a block of cells. **Table Layout > Select** picks the whole table, the row, the column, or the cell the caret is in. Most of the commands below work on the cells you selected, or on the cell the caret is in when none are selected.

## Add and take away rows and columns

On **Table Layout**, use **Insert Above**, **Insert Below**, **Insert Left**, and **Insert Right**. The new row or column takes as many rows or columns as you selected. **Delete** takes away the rows, the columns, or the whole table. **Delete > Delete Cells, Shift Left** takes away only the cells you selected, with their words, and moves the other cells of the row to the left. The row then ends with an empty space where the last cell was. It is dimmed when a cell is joined with cells below it, or when it would take every cell of a row (use **Delete Rows** for that). Take away every row and the table is gone.

## Merge and split cells

- **Merge Cells** joins the selected cells into one. The words of each cell stay, one after the other, in the merged cell. A cell with nothing in it adds nothing. Cells joined across share one wide cell; cells joined down share one tall cell.
- **Split Cells** asks how many columns and rows to make from the cell the caret is in. A cell split into more columns than it takes makes new columns in the grid, and the cells above and below get wider to go across them. A cell split into rows adds rows under it, and the cells beside it go on through the new rows. A cell that was joined down can be split into as many rows as it joins, or into columns that stay joined.

- **Split Table** cuts the table in two, above the row the caret is in. An empty paragraph goes between the two tables, so that you can type there. You can't cut above the first row, or through a cell that is joined down.

Each is one step to undo.

## Sort rows

**Table Layout > Sort** puts the rows in order. Pick a column to sort by, and say whether its cells hold **Text**, **Number**, or **Date**, and whether to go up (**Ascending**, such as A to Z or small to large) or down (**Descending**). Add up to two more columns to break ties. If you select two or more rows first, only those rows are sorted. Check **The first row is a header row** to keep it at the top.

The words of a row move together. Lines and shading that belong to a cell stay in their places. A table whose cells are joined down, or that has a table in a cell, can't be sorted.

## Add up numbers with a formula

Put the caret in a cell and click **Table Layout > Formula**. The box starts with **SUM(ABOVE)**, which adds up the numbers in the cells above, or **SUM(LEFT)** when the numbers are on the left. Press **OK** and the answer shows in the cell. It is a field, so you can click it and press **F9** to work it out again after you change a number.

In a formula you can name cells:

- **A1** is the cell in column A, row 1. Columns are lettered from the left, and rows are numbered from the top.
- **A1:B3** is a block of cells.
- **ABOVE**, **BELOW**, **LEFT**, and **RIGHT** are the cells in that direction that hold numbers. The list stops at the first empty cell.

Use them in **SUM**, **AVERAGE**, **COUNT**, **MIN**, **MAX**, **PRODUCT**, and the other functions, or in a sum such as **A1 + B2 * 2**. Numbers may have a dollar sign, commas, or a percent sign, and a number in parentheses is below zero. **Number format** shows the answer as you like, for example **0.00** for two places after the point, or **$#,##0.00** for money.

## Borders and shading

- **Table Design > Borders** puts a line on the sides of the selected cells, or takes it away: the bottom, top, left, or right, all the lines, the outside lines, the lines inside, or none. A line on the edge of the block is put on the side of the cell next to it as well, so that the two cells agree.
- **Table Design > Shading** fills the selected cells with a color, or with **No Color**.
- **Borders and Shading** (in the Borders menu, or on the right-click menu) opens a window to pick the style of the line (single, double, dotted, dashed, or triple), its width and color, which sides get it, and the fill. Choose to apply it to the selected cells or to the table.

## Make your own table style

Open the gallery on **Table Design** and click **New Table Style**. Give it a name, pick the style it starts from, and choose the lines, the look of the header row, the total row, and the first and last columns, and the fills of banded rows and columns. The new style goes on the table you are in. To change a style, click in a table that has it and click **Modify Table Style**. Every table that has the style changes with it. The header row and the other parts show when **Table Style Options** turns them on. Changing a style takes one step, and Undo brings the old one back.

## See the edges of the cells

**Table Layout > View Gridlines** draws faint dotted lines along the edges of cells, so that you can see a table that has no borders. The lines show on the screen only. They are never printed or saved in the file.

## Table styles

**Table Design > Table Styles** shows pictures of ready-made looks: plain, grid, and list tables, some in colors. Click one to give the table that look. The style gives the lines, the fills, and bold text for the parts of the table. **Clear** goes back to a table with no style.

The options at the left of the gallery say which parts the style dresses: **Header Row** and **Total Row** (the first and last rows), **First Column** and **Last Column**, and **Banded Rows** and **Banded Columns** (every other row or column gets the style's fill). Lines and shading you gave the cells yourself stay as you made them.

## Size

On **Table Layout**:

- Type in **Height** to set the height of the rows the caret is in, and in **Width** to set the width of its column. A height is a least height; the row grows with its words. For a height that does not grow, use Table Properties.
- **Fit** sizes the columns to their contents, to the window, or fixes them where they are.
- **Distribute Rows** and **Distribute Columns** make the selected rows as tall, or the selected columns as wide, as each other. With one row or column selected, they work on the whole table.

### Pull the lines with the mouse

- Point at the line on the right of a column. The pointer turns into two arrows. Press, pull sideways, and let go. The column gets that width. A dashed line shows where the edge will go.
- Point at the line under a row, and pull up or down. The row gets at least that height.
- Click in a table. A small square shows at its lower right corner. Pull it sideways to make the whole table wider or narrower. Every column grows or shrinks by the same share.

Each pull is one step. Undo takes it back.

## Where the text sits in a cell

The nine buttons on **Table Layout > Alignment** put the words at the top, the middle, or the bottom of the cell, and at its left, center, or right.

## Table Properties

Click **Table Properties** on **Table Layout**, or on the right-click menu. It has a tab for each part:

- **Table**: where the table sits (left, center, or right), how far it is from the left margin, its preferred width (automatic, in points, or as a percent of the text area), whether the columns are fixed, **Wrap text around the table** (the table sits where it is and the words of the next paragraphs flow beside it, instead of below it), the space inside every cell, and the space between cells.
- **Row**: the height and whether it is a least height or an exact one, **Do not let the row break across pages**, and **Repeat as header row at the top of each page**. A header row shows again at the top of each page a long table runs onto. The button **Table Layout > Repeat Header Rows** does the same for the selected rows.
- **Column**: the width of the column, in points or as a percent of the table. The two boxes follow each other.
- **Cell**: where the words sit up and down, and whether the words may wrap.
- **Alt Text**: a title and a description of the table for people who can't see it.

## What JeffWrite keeps

A table you make or change is saved in the file with its widths, lines, fills, merges, and options, and opens the same in JeffWrite and in other word processors. Parts of a table that the file had and JeffWrite doesn't show are kept as they were.

In an `.odt` file, a table keeps its lines, fills, and widths, and the fills that its style gives the header row and the bands are written on the cells. A formula is kept, but not its number format, and a style is not kept as a style: the table looks the same when you open it again, but it no longer has a style name.

[Back to the contents](index)

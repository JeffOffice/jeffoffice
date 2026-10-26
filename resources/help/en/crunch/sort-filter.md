# Sort and filter
<!-- keywords: sort, filter, autofilter, ascending, descending, A-Z, rules, header, color, list, months, weekdays, capitals, columns, largest, smallest, average, between, starts with, contains, date, clear, reapply, drop-down, hidden rows, table, Ctrl+Shift+L -->

## Sort

Select a cell in your data and choose **Sort A–Z** or **Sort Z–A** (**Sort and Pick Rows** on the Home tab). JeffCrunch takes the whole block of data around the cell, keeps a header row where it finds one (text over numbers), and puts the rows in order by the column you are in. With several cells selected it sorts just those; if there is data next to them it asks whether to take it along.

In the order, numbers come first, then text, then TRUE and FALSE, then errors; empty cells always go last. Capital and small letters count as the same unless you ask otherwise. Text is put in order the same way everywhere: by the letters of English, with an accented letter placed with its plain one, whatever language the computer uses.

Choose **Sort…** to open the Sort dialog:

- Add a rule for each column to sort by. A later rule breaks ties in the earlier ones. Use **Move Up** and **Move Down** to change the order of rules, and **Duplicate Rule** to start from one.
- Sort **Using** values, the **fill color** of the cells, or their **text color**. For a color, choose whether it goes **First** or **Last**; the other rows keep their order. A color is the color it comes to: one chosen from the palette and one with the same red, green, and blue are the same.
- For values, the order can be **A–Z**, **Z–A**, or a list: the days of the week and the months (in full and short, in English and in your language), or a list you type, with commas between the values. Values that are not in the list go after those that are.
- **The first row names the columns** keeps the first row out of the sort. **Capitals count** makes capital and small letters differ (small letters first). **Sort columns, not rows** puts the columns in order by the values in a row.

A formula moves with its row, and its references to cells in the same row stay in the row. A reference to a cell in another row is changed as if the formula were copied. What is tied to a cell goes with it: the cell's format, its note, and its link move to the place the cell goes. The rules that cover a range of cells (what may be entered, and conditional formats) stay where they are, as they cover the places and not the cells. Sorting is one step you can undo, and **Esc** stops a large one; a large sort is put in a little at a time.

Some cells can't be sorted, and you are told why: cells that are locked on a protected sheet, part of an array, merged cells that are not all the same size (or that span more than one row), merged cells cut by the edge of the selection, and a table's header row.

## Filter

Select a cell in your data and choose **Filter** (**Ctrl+Shift+L**). A button appears on each cell of the first row. Click one to see:

- **Sort A–Z** and **Sort Z–A**, and **Sort by a Color**.
- A list of the values in the column with check boxes, a search box, and **(Everything)**. A column of dates is listed by year, month, and day, and the values in it that are not dates are listed under them. Uncheck what you don't want, then **OK**. Searching shows the values that have all your words; **(Everything Found)** then checks or unchecks just those. The list covers the whole column. If the column has more different values than the list holds, it says so and the list is cut short; use a condition for the rest.
- **Keep a Color**, by fill color or text color.
- **Number Conditions**, **Text Conditions**, or **Date Conditions**: equals, above, at least, below, at most, between, the largest or smallest (items or percent), over or under the average, starts with, ends in, contains, dates in a period (today, current week, following month, previous quarter, so far this year, a quarter or a month of any year), and **Your Own Condition** of two comparisons joined by And or Or. In the text of a condition, ? and \* stand for any one character and any stretch of characters.
- **Show All of** the column.

A button with a funnel on it shows a column that has a filter. The rows that don't match are hidden and keep their row numbers. **Clear** (in the **Sort and Pick Rows** menu) takes every condition off and shows the rows; **Reapply** works the conditions out again after you change a value. **Filter** again turns the buttons off.

Rows hidden by a filter are kept apart from rows you hid yourself: **Unhide Rows** does not show a row the filter hides, and clearing the filter does not show a row you hid. Sorting a filtered block puts only the rows that show in order, among themselves.

A filter works on a whole sheet, up to 1,048,576 rows, a little at a time; **Clear** and turning the filter off work whatever the size. What a filter is asked to do is limited to 40 million checks (the rows times the columns that have a condition, with some for the values the conditions list); a filter that asks for more says so and is not made, and you can filter fewer rows or use fewer conditions. A filter of the sheet does not cover cells of a table: a table has its own buttons, which work the same way (see [Tables](tables)).

A list of values in a condition is kept as a file writes it, with no language in the numbers and dates (1,234.50), and a value matches a cell that shows it in your language or that way; a file filtered in one language filters the same cells in another.

The filter is saved in the file with its conditions and the rows it hides, so the file opens with the same rows showing. Reopening a file shows the funnel on every column that has a condition; rows hidden in the filter's range count as the filter's.

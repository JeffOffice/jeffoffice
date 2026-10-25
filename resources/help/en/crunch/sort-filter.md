# Sort and filter
<!-- keywords: sort, filter, autofilter, ascending, descending, A to Z, custom sort, levels, header, color, list, months, weekdays, case, left to right, top 10, average, between, begins with, contains, date, clear, reapply, drop-down, hidden rows, Ctrl+Shift+L -->

## Sort

Select a cell in your data and choose **Sort A to Z** or **Sort Z to A** (**Sort and Filter** on the Home tab). JeffCrunch takes the whole block of data around the cell, keeps a header row where it finds one (text over numbers), and puts the rows in order by the column you are in. With several cells selected it sorts just those; if there is data next to them it asks whether to take it along.

In the order, numbers come first, then text, then TRUE and FALSE, then errors; empty cells always go last. Capital and small letters count as the same unless you ask otherwise.

**Custom Sort** opens the Sort dialog:

- Add a level for each column to sort by. A later level breaks ties in the earlier ones. Use **Move Up** and **Move Down** to change the order of levels, and **Copy Level** to start from one.
- **Sort on** cell values, **cell color**, or **font color**. For a color, choose which color goes first or last; the other rows keep their order.
- For values, the order can be **A to Z**, **Z to A**, or a list: the days of the week and the months (in full and short, in English and in your language), or a list you type, with commas between the values. Values that are not in the list go after those that are.
- **My data has headers** keeps the first row out of the sort. **Case sensitive** makes capital and small letters differ (small letters first). **Sort left to right** puts the columns in order by the values in a row.

A formula moves with its row, and its references to cells in the same row stay in the row. A reference to a cell in another row is changed as if the formula were copied. Sorting is one step you can undo, and **Esc** stops a large one.

Some cells can't be sorted, and you are told why: cells that are locked on a protected sheet, part of an array, merged cells that are not all the same size (or that span more than one row), merged cells cut by the edge of the selection, and a table's header row.

## Filter

Select a cell in your data and choose **Filter** (**Ctrl+Shift+L**). A button appears on each cell of the first row. Click one to see:

- **Sort A to Z** and **Sort Z to A**, and **Sort by Color**.
- A list of the values in the column with check boxes, a search box, and **(Select All)**. A column of dates is listed by year, month, and day. Uncheck what you don't want, then **OK**. Searching shows the values that have all your words; **(Select All Search Results)** then checks or unchecks just those.
- **Filter by Color**, by cell color or font color.
- **Number Filters**, **Text Filters**, or **Date Filters**: equals, greater than, between, top 10 (items or percent), above or below average, begins with, ends with, contains, dates in a period (today, this week, next month, this quarter, year to date, a quarter or a month of any year), and a **Custom Filter** of two conditions joined by And or Or. In the text of a condition, ? and \* stand for any one character and any run.
- **Clear Filter From** the column.

A button with a funnel on it shows a column that has a filter. The rows that don't match are hidden and keep their row numbers. **Clear** (in the **Sort and Filter** menu) takes every condition off and shows the rows; **Reapply** works the conditions out again after you change a value. **Filter** again turns the buttons off.

Rows hidden by a filter are kept apart from rows you hid yourself: **Unhide Rows** does not show a row the filter hides, and clearing the filter does not show a row you hid. Sorting a filtered block puts only the rows that show in order, among themselves. A filter covers up to 100,000 rows.

The filter is saved in the file with its conditions and the rows it hides, so the file opens with the same rows showing. Reopening a file shows the funnel on every column that has a condition; rows hidden in the filter's range count as the filter's.

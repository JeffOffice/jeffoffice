# Pivot tables
<!-- keywords: list formulas, calculated field, calculated item, solve order, formula field, report filter pages, pivot table, drill down, drill up, insert pivot table, create, make, select, move pivot table, getpivotdata, generate getpivotdata, refresh, refresh all, change data source, field list, fields, rows, columns, values, filters, report filter, grand total, subtotal, layout, compact, outline, tabular, clear, value field settings, sum, count, average, show values as, percent of total, difference, running total, number format, field settings, sort, filter, top, group, ungroup, dates, months, years, expand, collapse, options, style, banded rows, blank rows, repeat labels, refresh when the file opens, pivot chart, slicer, timeline, show details, drill down, rows behind a value -->

A pivot table is a small report made from a long list. The list has one row for each sale, say; the report adds the sales up by region and month. You do not write any formulas. You say which columns of the list go down the side, which go across the top, and which one is added up, and the report fills itself in.

## An example

Here is a list in the cells A1 to C7. Each row is one sale.

| Region | Month | Amount |
|---|---|---|
| East | Jan | 10 |
| East | Jan | 5 |
| East | Feb | 20 |
| West | Jan | 7 |
| West | Feb | 3 |
| West | Feb | 4 |

A pivot table with **Region** down the side, **Month** across the top, and the sum of **Amount** in the middle looks like this.

| Sum of Amount | Month | | |
|---|---|---|---|
| Region | Jan | Feb | Grand Total |
| East | 15 | 20 | 35 |
| West | 7 | 7 | 14 |
| Grand Total | 22 | 27 | 49 |

East in January is 15 because the two East rows for January, 10 and 5, are added. The Grand Total column adds each row, the Grand Total row adds each column, and the cell in the corner adds everything.

The words used here:

- **Field** is a column of the list. Region, Month, and Amount are the fields.
- **Rows**, **Columns**, and **Values** are the three places a field can go. A field in Rows gives one line for each of its different entries; a field in Columns gives one column for each; a field in Values is the one that is added up. A fourth place, **Filters**, holds a field that leaves out the rows you do not choose.
- A **cache** (the pivot cache) is a copy of your list that the pivot table keeps inside the file, so it can show totals without looking at the cells. The report is worked out from the cache, not from the cells, so it does not change when the cells change. **Refresh** copies the cells into the cache again.

## Make a pivot table

1. Click a cell in the list, or select the cells you want to use. The list needs a row of names above its data.
2. Choose **Insert**, then **Pivot Table**.
3. The box shows the cells the data is in. Change them if they are not right. You can also type the name of a table.
4. Choose where the pivot table goes: **A new sheet** (the default), or a cell on this sheet or another. Pick a place where nothing else is, because the empty table covers about eighteen rows and three columns.
5. Choose **OK**. The table starts empty, and the **Pivot Table Fields** pane opens. Check the fields you want: a field of numbers is added up, and any other field becomes a row. Then move fields to the boxes as the next sections say.

A new sheet is its own step of Undo, and so is the table. When you save as .xlsx the table is written the way the original program writes one, so it opens there as a pivot table. A copy saved as .xls or .ods keeps the numbers the table shows in its cells, but not the table itself, and the save tells you so.

## GETPIVOTDATA: reading one number from a pivot table

`GETPIVOTDATA` gives the number a pivot table shows for the values and the rows and columns you name. For the example above:

`=GETPIVOTDATA("Sum of Amount", $E$3, "Region", "East", "Month", "Jan")` gives 15.

- The first argument is the name of the value, as the table shows it (for example Sum of Amount). The name of the field it adds up (Amount) works too.
- The second argument is any cell of the pivot table.
- After that come pairs of a field and one of its items. Name only Region, East, and you get East's total, 35. Name nothing, and you get the grand total, 49. The fields you name must be the first fields on the rows, or the first fields on the columns, in order.
- If the table does not show the number, the cell shows `#REF!`.

When you click a value of a pivot table while you type a formula, JeffCrunch writes this formula for you, so that it still reads the right number after the table is rearranged. To turn this off, choose **Options** on the **Pivot Table** tab and clear the check box about GETPIVOTDATA.

## What JeffCrunch does with a pivot table

A pivot table in a workbook is shown as it was saved. If you change nothing about it, the file is saved with the pivot table exactly as it was. When you refresh it or change it, JeffCrunch works the report out again, puts the new numbers in the cells, and writes the pivot table back so that the program that made it opens it without complaint.

JeffCrunch works out pivot tables that add, count, average, and so on, that group dates and numbers, and that show values as a share, a difference, or a running total. A pivot table that shows a value as a rank or as a share of a parent, has groups of items that someone made by hand, or uses a data model is shown and kept as saved, and the commands tell you they cannot change it.

Click any cell of a pivot table. The **Pivot Table** tab appears.

## Refresh

**Refresh** reads the cells the pivot table is made from and works the report out again. New entries come after the ones already there; entries that are gone are dropped. Column widths, and the layout you chose, stay. **Refresh All**, on the **Data** tab and on the **Pivot Table** tab, does the same for every pivot table in the workbook.

If the report would grow over cells that have something in them, JeffCrunch does not refresh. It tells you which cells are in the way. Clear them or move the pivot table, and refresh again.

**Undo** takes back a refresh in one step.

## Change Data Source

**Change Data Source** asks for the cells the list is in, including its first row of names. Type them like `Sheet1!A1:D100`, or type the name of a table or a name you made. JeffCrunch reads them and works the report out again. If a field of the list has the same name in the new cells, its place in the report stays.

## Clear

**Clear** has two choices. **Clear Filters** removes every filter and shows every entry again. **Clear All** takes every field out of the report and leaves it empty, ready to build again.

## Drill down and drill up

## Report Filter Pages

A filter sits above a pivot table and lets you look at one item at a time, such as one month. **Show Report Filter Pages** on the **Pivot Table** tab makes a page for every item at once. Each page is a new sheet, named for the item, with its own copy of the table. The filter of each copy is set to that item. If the table has more than one filter, you pick the one to use. One **Undo** takes all the pages away.

When a report has more than one field in its rows, an item of the first field can be opened or shut. Click the cell with the item's name and choose **Drill Up** to shut it: the lines of the next field under it are hidden and only its total stays. **Drill Down** opens it again. An item of the last field has nothing under it. Each is one **Undo**.

## Select and move

**Select** on the **Pivot Table** tab selects part of the report: **Entire Pivot Table**, **Values** (the numbers), or **Labels** (the names down the side; if the report has none down the side, the names across the top). **Move Pivot Table…** asks for a cell on the same sheet and draws the report again with its first cell there. The cells it leaves are emptied. Formulas that point at the report's cells follow them, as they do when you cut and paste. If the new place has something in it, JeffCrunch does not move the report and tells you. One **Undo** moves it back.

## The fields pane

**Field List** on the **Pivot Table** tab opens the **Pivot Table Fields** pane. It lists the fields of the list with a box beside each, and below it the four places: **Filters**, **Columns**, **Rows**, and **Values**.

- Check a field to add it. A field that holds only numbers goes to **Values** and is added up; any other goes to **Rows**.
- Uncheck a field to take it out everywhere.
- Click a field in one of the four places, and choose **Move To** to put it in another. The words **Move Up** and **Move Down** change its order in its place, and **Remove Field** takes it out.
- With more than one value, a line named **Values** appears in **Columns**. Move it to **Rows** to put the values down the side.
- **Value Field Settings** (a double click on a value, or the button) sets the name the report shows for the value and how it is totaled: **Sum**, **Count**, **Count Numbers**, **Average**, **Max**, **Min**, **Product**, **StdDev** and **StdDevp** (the standard deviation of a sample, or of all the values), and **Var** and **Varp** (the variance of the same). It also sets how the value is shown and its number format. See the next section.
- **Field Settings** (a double click on a field in Filters, Columns, or Rows, or the button) sets one field. See "Field Settings".

Every change is one step of **Undo**.

## Show Values As, and the number format of a value

In **Value Field Settings**, **Show values as** changes what a cell of the report shows.

- **% of Grand Total**: the cell divided by the total of the whole report. In the example, East in January is 15 of 49, which is 30.6%.
- **% of Column Total** and **% of Row Total**: the cell divided by the total of its column, or of its row.
- **Index**: how the cell compares with what its row and column would give if the data were spread evenly. A number above 1 means more than the even share.
- **Difference From**, **% Of**, and **% Difference From**: the cell compared with another item of a field. Pick the **Base field** (a field in Rows or Columns) and the **Base item** (one of its items, or the one before or after). For example, with Month as the base field and Jan as the base item, February shows how much more or less it is than January. The base item itself is left empty for a difference, and is 100% for **% Of**.
- **Running Total In**: each cell adds the cells before it along the base field.
- **Rank Smallest to Largest** and **Rank Largest to Smallest**: each cell shows its place, 1, 2, 3, and so on, among the cells of the base field in the same row or column. Rank 1 is the smallest number, or the largest.
- **% of Parent Row Total** and **% of Parent Column Total**: each cell shows what part of its parent it is. The parent of an item is the item one level up, such as the year that holds a month. An item with no parent is shown as a part of the grand total.

**Number format** takes a format code, such as `0.00`, `#,##0`, or `0.0%`. Leave it empty to keep the format the cells have. A value shown as a share gets `0.00%` unless you pick a format. The cells of the report take the format at once. **Undo** takes it back with the numbers.

## Field Settings

Select a field in the **Filters**, **Columns**, or **Rows** box and choose **Field Settings** on the **Pivot Table** tab (or double click it).

- **Custom name** changes the name the report shows for the field.
- **Subtotals**: **Automatic** shows a total for each group of the field, **None** does not.
- **Show item labels in**: compact, outline, or tabular form, as in Report Layout.
- **Insert a blank line after each item** puts an empty line after each group.
- **Repeat item labels** shows the name of an outer item on every line of its group (outline and tabular forms).
- **Sort**: keep the order the items are in (manual), or sort A to Z or Z to A by the labels. **Smallest to largest by a value** and **Largest to smallest by a value** sort the items by the numbers in a value field. The report keeps the sort when the data changes.
- **Filter** leaves items out by their labels (begins with, ends with, contains, equals) or by a value (greater than, less than, between, and so on), or keeps only the top or bottom few items by a value. The filter is worked out when you choose OK. It hides the items it leaves out; the check boxes below show which, and you can check an item to show it again.
- **Show these items** is a list of the field's items with a check each. Clear a check to hide the item, and the totals leave it out.

## Group dates and numbers

Select a field of dates or of numbers in **Rows** or **Columns** and choose **Group Field** on the **Pivot Table** tab. Groups are made from the data, so the data must have only numbers or dates in that column, with no text and no empty cells.

- **Dates**: check what to group by: **Seconds**, **Minutes**, **Hours**, **Days**, **Months**, **Quarters**, or **Years**. The finest part you check stays on the field. For each coarser part JeffCrunch adds a field, named for it (for example **Years**), and puts it before the date field in the same place. **Starting at** and **Ending at** are the first and last dates; leave them as they are to use the least and greatest dates in the data. Dates before the start or after the end go in two extra groups, which begin with `<` and `>`.
- **Numbers**: **Starting at**, **Ending at**, and **By** make groups such as 1-10, 11-20, and so on.

**Ungroup** takes the groups away and the fields that were added for them. Refresh makes the groups again from the new data. When two pivot tables are made from the same data, groups can't be made, because a group is a part of the data they share.

## Calculated fields

A calculated field is a field you make from a formula. It uses other fields of the list. For example, a bonus of one tenth of the amount is `=Amount*0.1`.

Choose **Calculated Field** on the **Pivot Table** tab. Type a name and a formula. Write a field's name in the formula, or double-click it in the **Fields** list to put it there. Put a name that has spaces in single quotes, as in `='Unit price'*Quantity`. You can use numbers, `+ - * / ^ %`, comparisons, and a few functions such as `IF`, `ROUND`, `SUM`, `MIN`, `MAX`, `AVERAGE`, and `ABS`.

Choose **Add**. The new field is put in the **Values** box. It also shows in the field list, where you can check or clear it like any other field. It can't go in the rows, columns, or filters.

A calculated field is worked out in every cell of the report. A field's name stands for the total of that field in the cell, so `=Amount*Amount` squares the cell's total. It does not square each row and add them up. A grand total uses the totals of the whole list.

**List Formulas** (next to **Calculated Field**) makes a new sheet that names each calculated field and its formula, and each calculated item with its formula, its field, and its solve order. **Undo** takes the cells away, and another **Undo** takes the sheet away.

To change a field, pick its name in the **Name** box. Its formula shows. Change it and choose **Modify**. **Delete** takes the field and its value away. Each is one step of **Undo**. A calculated field is saved with the file and is kept when the report is refreshed. Two pivot tables made from the same data can't have one, because a calculated field is a part of the data they share.

## Calculated items

A calculated item is an item you make from other items of the same field. For example, the field Region has the items East and West. A new item named Both can be `=East+West`.

Click a cell in the pivot table, and choose **Calculated Item** on the **Pivot Table** tab. Pick the **Field**. It must be in the rows or the columns. Type a **Name** and a **Formula**. Write an item's name in the formula, or double-click it in the **Items** list to put it there. Put a name that has spaces in single quotes, as in `='Big East'+West`. You can use numbers, `+ - * / ^ %`, and brackets.

The new item gets a line in the report. In each column, it shows the formula worked out from the lines of the items it names. The grand totals and the subtotals are those of the rows of the list, so the new item is not counted in them twice. If an item the formula names is hidden, the cell shows **#REF!**. If the formula names an item that is not there, the cell shows **#NAME?**.

To change an item, pick its name in the **Name** box. Its formula shows. Change it and choose **Modify**. **Delete** takes the item and its line away. Each is one step of **Undo**. A calculated item is saved with the file and is kept when the report is refreshed. Two pivot tables made from the same data can't have one, because a calculated item is a part of the data they share.

### Solve order

Sometimes one cell has two calculated items. One is in its row and the other is in its column. The order in which they are worked out can change the answer. Say the row item is `=East-West` and the column item is `=Jan+1`. Worked out in one order, the cell is (East - West) + 1. In the other order, it is (East + 1) - (West + 1), which is a different number.

**Solve Order** on the **Pivot Table** tab lists each calculated item with a number. The item with the higher number is worked out last. Pick an item in the list and change its number in the **Solve order** box. The numbers are saved with the file. **List Formulas** shows them next to each item's formula.

## Show the rows behind a value

Double click a value of a pivot table, or choose **Show Details** on the **Pivot Table** tab. JeffCrunch makes a new sheet, named Detail 1, 2, and so on, with the rows of the list that make up the number. The names of the fields are in the first row. F2 still edits the cell, if the cell can be edited. **Undo** takes the rows away, and another **Undo** takes the sheet away.

## Expand and collapse

A field whose items have others under them can show or hide the lines under its items. **Expand Entire Field** and **Collapse Entire Field** do it for every item of the field you select in the pane.

## Pivot Table Options

**Options** on the **Pivot Table** tab opens a box with the choices of the table.

- **Name** is the name of the pivot table. No other pivot table of the workbook may have it.
- **For error values show** and **For empty cells show** put a text of your own in those cells.
- **Merge and center cells with labels**, **Fit column widths when the table is updated**, and **Keep the formatting of cells when the table is updated** are saved with the table.
- **Show grand totals for rows** and **for columns** are the same as the Grand Totals choices.
- **Refresh the data when the file is opened** reads the cells again each time the workbook is opened. It is one step of Undo, like any refresh.

## Style: headers, bands, and the gallery

The **Style Options** on the **Pivot Table** tab turn the header row, the first column (row headers), and the bands of rows and of columns on or off. **Styles** has a gallery of looks, and **Clear Style** takes the look away. These looks are drawn the same way as the looks of tables. **Blank Rows** and **Repeat Labels** set the blank line and the repeated labels for every field of the rows at once.

## Design: subtotals, grand totals, and layout

- **Subtotals** shows or hides the total of each group, at the top or the bottom of the group. When there is more than one field in Rows, the total of each outer field is a subtotal.
- **Grand Totals** has four choices: off for rows and columns, on for both, on for rows only (a column of totals at the right, one for each row), and on for columns only (a row of totals at the bottom).
- **Report Layout** is the shape of the labels. In **compact** form, all the fields of Rows share one column, and a new group is shown a little to the right. In **outline** form, each field has its own column, and each group has a line of its own above its items. In **tabular** form, each field has its own column and the group's name is on its first line, with the subtotal below the group.

## The same list, a filter

Move **Month** from **Columns** to **Filters**. The report loses its columns, and a line over it shows **Month** and **(All)**. The numbers are now each region's total over every month.

## Pivot chart

Click a cell in a pivot table and choose **Insert**, then **Pivot Chart**. JeffCrunch makes a column chart of the table's values. It leaves out the grand totals. The chart is marked as the pivot table's chart in the file. It reads the cells of the table, so its numbers change when you refresh the table or change the table's layout. If the table gets more rows or columns, make the chart again, because the chart keeps the cells it was made from. A pivot chart that comes from a file is shown as saved.

## Slicers and timelines

A slicer is a set of buttons that filters a pivot table, and a timeline is a slider that does the same for dates.

Click a cell in a pivot table, then click **Slicers** on the Pivot Table tab. The Slicers pane opens and lists the slicers and timelines in the workbook. A slicer has one button for each item of its field. The buttons that are blue are the items the table shows.

Click a button to show only that item. Click it again to show all the items. Hold **Ctrl** and click to add an item to the ones shown, or to take one away. Each click is one step of Undo, and Undo puts the table back. When you save, the file keeps the slicer's choice.

**Report Connections** says which pivot tables a slicer filters. A timeline shows the span of dates it chose. JeffCrunch shows the slicers in this pane, not over the cells, and it does not change a timeline yet. Both are kept in the file as they were saved.

## Limits

A pivot table can have up to 2,000 fields, and a list of up to five million rows, and a report of up to four million cells. A file that goes past these is shown as saved and cannot be changed. Reading a hundred thousand rows of twenty columns again and working out the report took about half a second on the computer this was measured on; a slower computer takes longer.

# Pivot tables
<!-- keywords: pivot table, refresh, refresh all, change data source, field list, fields, rows, columns, values, filters, report filter, grand total, subtotal, layout, compact, outline, tabular, clear, value field settings, sum, count, average -->

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
- A **cache** is the copy of the list that the pivot table keeps. The report is worked out from the cache, not from the cells, so it does not change when the cells change. **Refresh** copies the cells into the cache again.

## What JeffCrunch does with a pivot table

A pivot table in a workbook is shown as it was saved. If you change nothing about it, the file is saved with the pivot table exactly as it was. When you refresh it or change it, JeffCrunch works the report out again, puts the new numbers in the cells, and writes the pivot table back so that the program that made it opens it without complaint.

JeffCrunch works out pivot tables that add, count, average, and so on. A pivot table that groups dates or numbers, has a calculated field or item, shows values as a percent, a difference, or a rank, or uses a data model is shown and kept as saved, and the commands tell you they cannot change it.

Click any cell of a pivot table. The **Pivot Table** tab appears.

## Refresh

**Refresh** reads the cells the pivot table is made from and works the report out again. New entries come after the ones already there; entries that are gone are dropped. Column widths, and the layout you chose, stay. **Refresh All**, on the **Data** tab and on the **Pivot Table** tab, does the same for every pivot table in the workbook.

If the report would grow over cells that have something in them, JeffCrunch does not refresh. It tells you which cells are in the way. Clear them or move the pivot table, and refresh again.

**Undo** takes back a refresh in one step.

## Change Data Source

**Change Data Source** asks for the cells the list is in, including its first row of names. Type them like `Sheet1!A1:D100`, or type the name of a table or a name you made. JeffCrunch reads them and works the report out again. If a field of the list has the same name in the new cells, its place in the report stays.

## Clear

**Clear** has two choices. **Clear Filters** removes every filter and shows every entry again. **Clear All** takes every field out of the report and leaves it empty, ready to build again.

## The fields pane

**Field List** on the **Pivot Table** tab opens the **Pivot Table Fields** pane. It lists the fields of the list with a box beside each, and below it the four places: **Filters**, **Columns**, **Rows**, and **Values**.

- Check a field to add it. A field that holds only numbers goes to **Values** and is added up; any other goes to **Rows**.
- Uncheck a field to take it out everywhere.
- Click a field in one of the four places, and choose **Move To** to put it in another. The words **Move Up** and **Move Down** change its order in its place, and **Remove Field** takes it out.
- With more than one value, a line named **Values** appears in **Columns**. Move it to **Rows** to put the values down the side.
- **Value Field Settings** (a double click on a value, or the button) sets the name the report shows for the value and how it is totaled: **Sum**, **Count**, **Count Numbers**, **Average**, **Max**, **Min**, **Product**, **StdDev** and **StdDevp** (the standard deviation of a sample, or of all the values), and **Var** and **Varp** (the variance of the same).

Every change is one step of **Undo**.

## Design: subtotals, grand totals, and layout

- **Subtotals** shows or hides the total of each group, at the top or the bottom of the group. When there is more than one field in Rows, the total of each outer field is a subtotal.
- **Grand Totals** has four choices: off for rows and columns, on for both, on for rows only (a column of totals at the right, one for each row), and on for columns only (a row of totals at the bottom).
- **Report Layout** is the shape of the labels. In **compact** form, all the fields of Rows share one column, and a new group is shown a little to the right. In **outline** form, each field has its own column, and each group has a line of its own above its items. In **tabular** form, each field has its own column and the group's name is on its first line, with the subtotal below the group.

## The same list, a filter

Move **Month** from **Columns** to **Filters**. The report loses its columns, and a line over it shows **Month** and **(All)**. The numbers are now each region's total over every month.

## Limits

A pivot table can have up to 2,000 fields, and a list of up to five million rows, and a report of up to four million cells. A file that goes past these is shown as saved and cannot be changed. A refresh of a hundred thousand rows with twenty columns takes under a second.

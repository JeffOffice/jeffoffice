# Pivot tables
<!-- keywords: pivot table, insert pivot table, create, make, getpivotdata, generate getpivotdata, refresh, refresh all, change data source, field list, fields, rows, columns, values, filters, report filter, grand total, subtotal, layout, compact, outline, tabular, clear, value field settings, sum, count, average -->

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

When you click a value of a pivot table while you type a formula, JeffCrunch writes this formula for you, so that it still reads the right number after the table is rearranged. To get the plain cell address instead, turn off **Generate GETPIVOTDATA** on the **Pivot Table** tab, under **Options**.

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

A pivot table can have up to 2,000 fields, and a list of up to five million rows, and a report of up to four million cells. A file that goes past these is shown as saved and cannot be changed. Reading a hundred thousand rows of twenty columns again and working out the report took about half a second on the computer this was measured on; a slower computer takes longer.

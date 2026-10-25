# Charts, pictures, and shapes
<!-- keywords: chart, graph, column, bar, line, pie, area, scatter, combo, legend, title, gridlines, data labels, select data, move, resize, picture, shape, object, series -->

A chart is a picture of numbers that stays tied to them: change a number and the chart changes with it. Pictures and shapes sit over the cells the same way, but they are not tied to any numbers.

## Make a chart

1. Select the numbers you want to chart. If the first row has names and the first column has labels, select those too. JeffCrunch looks at the first row and column and uses them as the names and the labels when they are words (or dates, for the labels).
2. On the **Insert** tab, in **Charts**, pick a kind: **Column**, **Bar**, **Line**, **Pie**, **Area**, **Scatter**, or **Combo**. The small arrow beside a kind has its variations, such as stacked columns or a line with markers.
3. For a look at every kind first, choose **More Charts**. The list on the left shows each kind drawn from your cells. **Swap rows and columns** turns the series the other way.

If you select only one cell, JeffCrunch charts the block of cells around it that has something in it.

Series run down the columns when the selection has at least as many rows as columns, and along the rows when it has more columns than rows. A pie holds one series. A scatter chart takes its horizontal values from the first column.

Example: you select A1:C4, where A2:A4 hold Q1, Q2, and Q3, B1 and C1 hold East and West, and the rest are numbers. The chart has two series, East and West, and three groups of columns, Q1 to Q3.

## Move and size it

Click a chart, picture, or shape to pick it. Drag it to move it, or drag one of the eight squares on its edge to size it.

- Hold **Alt** while you drag to make an edge line up with the edge of a cell.
- Hold **Shift** while you drag a corner to keep the proportions.
- The arrow keys move it a few pixels at a time. **Ctrl** with an arrow moves it one pixel, **Shift** with an arrow makes it larger or smaller, and **Alt** with an arrow goes to the next edge of a cell.
- **Esc** while you drag puts it back.

Each move or size is one step you can undo. Right-click the object and choose **Properties** to type its size, and to choose what it does when you insert, delete, or resize cells: move and size with the cells, move with them but keep its size, or stay where it is. The choice is saved in the file.

## Change what a chart shows

When a chart is picked, two buttons appear beside it and a **Chart** tab appears on the ribbon.

- The **+** button (and **Chart Parts** on the tab) turns the chart's title, axes, axis titles, gridlines, legend, and data labels on or off, and says where the legend and the labels go.
- The brush button (and **Colors** and **Looks** on the tab) gives the chart one of JeffCrunch's sets of colors and one of its looks. **Layouts** sets several elements at once.
- **Chart Data** lists the chart's series. You can add, edit, remove, and reorder them, change the cells each one reads, set the cells with the labels, switch the rows and columns, and choose how empty cells and hidden rows and columns are charted.
- **Chart Kind** changes the kind for the whole chart, or for each series: columns with a line over them, or a series against a second vertical axis.

A reference to cells is written like **Sheet1!$B$2:$B$9**, with the sheet's name in single quotes when it has spaces.

## Cut, copy, and paste an object

A picture or shape is copied and pasted the same way as a chart. A shape that uses something other than a picture from the workbook, such as a link to a file, can't be copied, and JeffCrunch tells you why.


Copy a chart and paste it into the same workbook, on any sheet: the copy reads the same cells as the first. Paste it into another workbook and it keeps the numbers it showed but no longer reads cells, because the cells it would read there are not the ones it was made from.

## Name it

With an object picked, the name box shows its name. Type a new name there and press **Enter**. Undo puts the old name back, and the new name is saved in the file.

## Delete it

Press **Delete** with the object picked. Undo brings it back.

## What is saved

Everything above is saved in the file in the form other spreadsheet programs read, so a chart you make here opens there the same way. A chart that came from another program with something JeffCrunch cannot keep when it rewrites a chart (a gradient fill, a trend line, a three-dimensional chart) can still be moved, sized, and deleted, but its elements and data are not changed here.

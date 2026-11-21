# Charts
<!-- keywords: chart, charts, graph, column chart, bar chart, line chart, pie chart, doughnut, area, scatter, insert chart, chart data, edit data, chart title, legend, data labels, chart type, chart format, axes, axis title, gridlines, chart style, chart parts, layouts, colors, looks, chart data, swap rows and columns, chart kind, format axis, error bars, trendline, chart filters, chart elements -->

A **chart** turns numbers into a picture, such as columns, a line, or slices of a pie. This topic shows how to put a chart in a document, how to type its numbers, and how to change how it looks.

## Put a chart in

1. Put the caret where the chart should go.
2. Click **Insert > Chart**, and pick a kind: column, stacked column, 100% stacked column, bar, stacked bar, line, line with markers, pie, doughnut, area, stacked area, or scatter.

A **stacked** chart piles the series on top of each other. A **100% stacked** chart makes each stack the same height, so you see shares. A **scatter** chart puts each point where its two numbers say.

The chart comes in with sample numbers, in line with the text, like a large letter. The **Chart Format** tab opens.

## Type your own numbers

Select the chart and click **Chart Format > Edit Data**. A table opens.

- The first column holds the words under the points (the **categories**), such as the months.
- The first row holds the name of each **series**, which is one set of numbers, such as one product.
- The other cells hold the numbers.

Click **Add Row** or **Add Column** for more, or **Remove Last Row** or **Remove Last Column** for fewer. Click **OK**. If a cell is not a number, the chart is not changed, and the status bar tells you which row. A chart can have up to 200 categories and 24 series.

For a scatter chart, the first column holds the numbers that place the points from left to right.

## Charts saved by another program

Some charts that other programs saved keep their numbers in a small **workbook** (a spreadsheet file) stored inside your file. Select such a chart and click **Chart Format > Edit Data**. The table shows the cells of that workbook that the chart reads. Change a number, a name under a point, or the name of a series, and click **OK**. The change goes into the workbook and into the chart together, as one step to undo, so other programs show the new numbers too.

You cannot add or remove a row or a column of such a chart, and **Chart Data** and **Swap Rows and Columns** are dimmed, because the chart is tied to those cells. The same is true when the chart reads cells in another file, in a block that is not one column or one row, or in cells that hold a formula, a date, or words where a number should be. The command is dimmed, and its tip says why.

## Change how it looks

These commands are on the **Chart Format** tab. Each one is a single step to undo, and the chart keeps its colors and the rest of its look.

- **Change Chart Type** turns the chart into another kind with the same numbers.
- **Chart Title** shows or hides the title. **Edit Chart Title** changes its words.
- **Legend** is the key that says which color is which series. Put it at the right, top, left, or bottom, or take it off.
- **Axes** shows or hides the line of words under the chart (the **categories**) and the line of numbers beside it (the **values**), and sets a title for each. A pie chart has no axes.
- **Gridlines** draws faint lines across the chart to help your eye follow the numbers. The **major** lines sit at the main numbers on the side, and the **minor** lines sit between them.
- **Chart Styles** picks one of eight overall styles.
- **Data Labels** writes each number on its column, point, or slice.
- **Wrap Text**, **Alt Text**, and **Height** and **Width** work as they do for pictures. See [Add and change pictures](pictures).

## The Chart tab

Select a chart and the **Chart** tab opens beside the **Chart Format** tab. Everything on it changes the chart in one step you can undo.

- **Chart Parts** adds, takes away, or places a part of the chart: **Axes**, **Axis Titles**, **Chart Title**, **Data Labels** (what they hold, where they go, and the mark between them), **Gridlines**, **Legend**, **Error Bars** (short bars that show how far a number may be off), **Lines** (drop lines, high-low lines, and up and down bars), **Data Table** (a grid of the numbers under the chart, with a column for each category; its list turns the table on and off, and sets the lines between rows, the lines between columns, the outline, and the legend keys, which show each series' color beside its name; it is offered for column, line, and area charts), and **Trendline** (a line that shows where the numbers are heading, with its equation if you want it).
- **Layouts** sets several parts at once, such as the title, legend, labels, and gridlines.
- **Colors** gives the series a new set of colors. **Looks** gives the whole chart a look, such as plain, clean, dark, or soft.
- **Swap Rows and Columns** turns the numbers around: the series become the words under the points, and those words become the series.
- **Chart Data** lists the series. Check the ones to draw, change their names, and move them up or down.
- **Edit Data** opens the table of numbers.
- **Chart Kind** shows a picture of each kind. Pick one kind for the whole chart, or pick a kind for each series, such as columns with a line over them.
- **Format Chart Part** sets the fill and the line of the chart's page, of the plot (the box the marks stand in), or of one series. For a line, it also sets whether it is smooth and what mark stands at each point.
- **Current Selection** lists the parts of the chart: the chart area, the plot area, and each series. Pick one, and **Format Selection** opens the format window at that part. **Reset to Match Style** takes away what was set on the picked part and gives back what the chart's style gives it. For the chart area, that is its fill, its line, and the text color a look set (the light text of the dark look goes back to the theme's text color). For the plot area, it is its fill and its line. For a series, it is its color (the one the chart's Colors choice or style gives it), its outline, its marks as the kind of chart has them, and the colors of its points; a smooth line stays when the kind of chart is a smooth one. It does not change titles, the legend, labels, axes, gridlines, or the data, and it does nothing to a part that already matches. It is one step of Undo.
- **Format Horizontal Axis** and **Format Vertical Axis** set the lowest and highest numbers, the size of each step, the order, where the other axis crosses, the look of the numbers, and the tick marks.

## The buttons beside a chart

Three small buttons stand at the right of a selected chart.

- **Chart Elements** (the plus) opens the same menu as **Chart Parts**.
- **Chart Colors and Styles** (the brush) lists the color sets and looks.
- **Chart Filters** (the funnel) lists the series. Uncheck a series to leave it out of the picture. The file still keeps it, so it comes back when you check it again. A chart always keeps at least one series in view.

## Saving a chart in an OpenDocument file

If you save the document as an **OpenDocument text** file (`.odt`), each chart goes in as an object of its own, and opens again as a chart you can change. The kind, numbers, title, legend, labels, colors, axes, gridlines, up to four trendlines for each series, and error bars all stay. A few parts do not fit that format, such as a data table, drop lines, high-low lines, up and down bars, and error bars whose amounts you typed. JeffWrite lists what was left out when you save.

## Charts in other documents

JeffWrite shows the charts that a document holds, and keeps them as they were. It can change the kind, title, legend, and labels of a chart it can write whole. If a chart has parts JeffWrite does not change, such as special effects, it says so and leaves the chart alone. If a chart reads its numbers from a worksheet kept with it, you can change everything but the numbers.

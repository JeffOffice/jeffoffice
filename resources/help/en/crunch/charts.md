# Charts, sparklines, pictures, and shapes
<!-- keywords: shape fill, shape outline, rotate, flip, crop, edit text, picture border, insert picture, insert shape, text box, shapes, line, arrow, oval, rectangle, star, chart, graph, column, bar, line, pie, area, scatter, combo, legend, title, gridlines, data labels, chart data, chart kind, looks, layouts, move, resize, picture, shape, object, series, trendline, trend line, equation, R-squared, second axis, sparkline, sparklines, date axis, win/loss, tiny chart, markers, name, alt text, alternative text, logarithmic, log scale, save as picture, template, crtx, title text, legend key -->

A chart is a picture of numbers that stays tied to them: change a number and the chart changes with it. Pictures and shapes sit over the cells the same way, but they are not tied to any numbers.

## Make a chart

1. Select the numbers you want to chart. If the first row has names and the first column has labels, select those too. JeffCrunch looks at the first row and column and uses them as the names and the labels when they are words (or dates, for the labels).
2. On the **Insert** tab, in **Charts**, pick a kind: **Column**, **Bar**, **Line**, **Pie**, **Area**, **Scatter**, or **Combo**. The small arrow beside a kind has its variations, such as stacked columns, a line with markers, or a scatter chart of points, of joined points, or of smooth lines.
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

Each move or size is one step you can undo. Right-click the object and choose **Properties** to type its **Name** (each object on a sheet needs its own) and its **Alt text** (a short sentence that tells a person who cannot see the object what it shows), to type its size, and to choose what it does when you insert, delete, or resize cells: move and size with the cells, move with them but keep its size, or stay where it is. The choice is saved in the file. **Print this object**, in the same box, turns off the object on the printed pages (it still shows on the screen); the file keeps that choice too.

## Change what a chart shows

When a chart is picked, two buttons appear beside it and a **Chart** tab appears on the ribbon.

- The **+** button (and **Chart Parts** on the tab) turns the chart's title, axes, axis titles, gridlines, legend, and data labels on or off, and says where the legend and the labels go.
- The brush button (and **Colors** and **Looks** on the tab) gives the chart one of JeffCrunch's sets of colors and one of its looks. **Layouts** sets several elements at once.
- **Chart Data** lists the chart's series. You can add, edit, remove, and reorder them, change the cells each one reads, set the cells with the labels, switch the rows and columns, and choose how empty cells and hidden rows and columns are charted.
- **Trendline** (in the **+** menu) draws a line fitted to the numbers of each series, so you can see where they are heading. Pick **Linear** for a straight line, **Exponential** for numbers that grow by a steady percent, **Logarithmic** for numbers that rise fast and then level off, **Polynomial** for a curve with bends, **Power**, or **Moving Average**, which draws the average of the last few numbers. **Show Equation** writes the line's formula on the chart, and **Show R-squared** writes how well the line fits (1 is a perfect fit; 0 is no fit at all). The trendline gets its own entry in the legend.
- When a series is drawn against a second vertical axis, the **Axes** and **Axis Titles** menus have **Second Horizontal** and **Second Vertical** entries for it. The **Legend** menu has **Over the Chart**, which lays the legend on the plot instead of beside it.
- **Data Labels** also has **Label Contains** (the value, the category's name, the series' name, the percentage, and the legend key, which is a small square in the color of the series, in any mix), **Separator** (what stands between them: a comma, semicolon, period, new line, or space), and **Number Format…** (a format code such as 0.0 for the numbers in the labels; leave it empty to show the numbers as they are). Turn off every one of the contents and the labels go away.
- **Format Horizontal Axis…** and **Format Vertical Axis…** on the **Chart** tab open a box for one axis. It sets the lowest and highest values and the major and minor units (each automatic or a number you type), whether the values run in reverse order, whether the scale is logarithmic (each mark is a base times the one before, such as 1, 10, 100, and 1000, which suits numbers that differ a lot in size and must all be more than 0), where the other axis crosses it, the number format of its labels, its tick marks, where its labels sit, and how far the labels are turned. Only a number axis has bounds and units.
- **Move Chart…** on the **Chart** tab asks for another worksheet and puts the chart there, in the same cells. It still reads the cells it read, and keeps its size and its place on the sheet, even where the rows and columns are taller or wider, and it moves and sizes with the cells there as it did. Sheets that are hidden are not offered. It is one **Undo**, even after you saved. A chart sheet (a sheet that is only a chart) can't be made yet, and a chart that is on one can't be moved.
- **Chart Title** and **Axis Titles** (in the **+** menu) also have **Title Text…**. Type the words you want. Or type an equals sign and a cell, such as **=Sheet1!A1**, and the title shows what that cell holds. When the cell changes, so does the title. The file keeps the link.
- **Save as Picture…** on the **Chart** tab (and in the right-click menu) saves the picked chart as a **PNG** or **JPEG** picture file, twice as large as it is on the sheet so it stays sharp.
- **Save as Template…** and **Apply Template…** (on the **Chart** tab, in the **Type** group) keep a chart's look in a **.crtx** file and give it to another chart. The look is the kind, the colors, the title and legend settings, the axes, and the fonts. The chart you apply it to keeps its own numbers and the cells it reads. One **Undo** takes the look away again.
- **Chart Kind** changes the kind for the whole chart, or for each series: columns with a line over them, or a series against a second vertical axis.

A reference to cells is written like **Sheet1!$B$2:$B$9**, with the sheet's name in single quotes when it has spaces.

## Sparklines

A sparkline is a tiny chart that fits inside one cell. It shows the trend of a row or a column of numbers right next to them.

1. Select the numbers. For one sparkline per row, select several rows of numbers.
2. On the **Insert** tab, in **Sparklines**, pick **Line**, **Column**, or **Win/Loss**. A **Line** joins the numbers with a line. A **Column** draws a bar for each number. A **Win/Loss** draws a bar up for each number above zero and a bar down for each number below it, all the same size, which suits results that are only good or bad.
3. In the **Create Sparklines** box, check the **Data range** (the cells the sparklines read) and type the **Location range** (the cells they go in). Choose one location cell for each row of data, or for each column of data. If you choose one cell, the sparkline reads all of the data.

The sparklines you make in one step are a group, and they share one look. Click a cell with a sparkline, and the **Sparkline** tab appears.

- **Edit Data** changes the data and the location of the whole group, or the data of the one you clicked. It also says what to do with empty cells (leave a gap, treat them as zero, or join the points across them) and whether to read cells in hidden rows and columns.
- **Type** switches between line, column, and win/loss.
- **Show** marks points: the **High Point**, the **Low Point**, **Negative Points**, the **First Point**, the **Last Point**, and, for a line, **Markers** on every point.
- **Styles** gives the group a set of colors. **Sparkline Color** sets the color of the line or bars and the **Weight** (how thick the line is, in points). **Marker Color** sets the color of each kind of marked point.
- **Axis** shows a line at zero, plots the numbers right to left, and sets the lowest and highest values of the vertical axis. **Automatic for Each Sparkline** fits each one to its own numbers. **Same for All Sparklines** gives the whole group one scale, so you can compare them. **Custom Value** lets you type a number. **Date Axis…** spaces the points by dates: type the cells that hold the date of each point, and a point with a later date sits farther along. Leave it empty to space the points evenly again.
- **Group** puts the selected sparklines in one group. **Ungroup** gives each its own look. **Clear** takes away the selected sparklines, or their whole groups.

A sparkline follows its cells: insert a row above the data, and it still reads the same numbers. Every change is one step you can undo. Sparklines are saved in the file in the form other spreadsheet programs read.

## Put in a picture, a shape, or a text box

On the **Insert** tab, in **Illustrations**, **Picture** opens a file chooser. Pick a picture file (PNG, JPEG, GIF, BMP, TIFF, or SVG) and it lands on the sheet at the selected cell. A picture wider than 480 dots is made smaller to fit, and keeps its shape. The workbook keeps the picture itself. A picture of a kind the workbook can't hold as it is (an SVG, for example) is kept as a PNG, a plain picture made of dots. A file that is not a picture, or is larger than 16 MB, is turned down with a message.

**Shapes** opens a list: rectangle, rounded rectangle, oval, triangle, diamond, arrows, a star, a heart, a cloud, a speech callout, and a line. Pick one and it lands on the sheet in light blue with a darker edge. **Text Box**, in the **Text** group, puts in an empty box with no fill and no edge.

Each one is a single step you can undo. The new object is picked, so you can drag it, size it, move it forward or back, give it a name and alt text in **Properties**, or press **Delete**.

## Change how a picture, shape, or text box looks

Pick a picture and the **Picture Format** tab appears. Pick a shape, a line, or a text box and the **Shape Format** tab appears. Each change is one step you can undo, and the object keeps its name, its alt text, its place, and its order among the other objects.

- **Shape Fill** and **Shape Outline** pick a color, or none. **Line Style** sets the weight (how thick the line is) and the dashes (solid, dashed, or dotted). A picture has **Picture Border**, which is its outline.
- **Rotate** turns the object a quarter turn to the right or the left, or flips it over from side to side or top to bottom.
- **Crop**, for a picture, cuts it to a shape of window: square, 4 wide by 3 tall, 3 wide by 4 tall, 16 by 9, or 9 by 16. The cut is made evenly from the middle of what shows, and the picture is made as large as what is left. **Show the Whole Picture** brings back what was cut. The picture file is not changed: the cut part is only hidden.
- **Edit Text** is for shapes and text boxes. Type the words, and choose how they sit across and up and down, the size, bold or regular, and whether they wrap to the shape or stay on one line.
- **Properties** sets the size, the name, the alt text, and what the object does when cells are moved or made larger.

## Cut, copy, and paste an object

A picture or shape is copied and pasted the same way as a chart. A shape that uses something other than a picture from the workbook, such as a link to a file, can't be copied, and JeffCrunch tells you why.


Copy a chart and paste it into the same workbook, on any sheet: the copy reads the same cells as the first. Paste it into another workbook and it keeps the numbers it showed but no longer reads cells, because the cells it would read there are not the ones it was made from.

## Name it

With an object picked, the name box shows its name. Type a new name there and press **Enter**. Undo puts the old name back, and the new name is saved in the file.

## Delete it

Press **Delete** with the object picked. Undo brings it back.

## What is saved

Everything above is saved in the file in the form other spreadsheet programs read, so a chart you make here opens there the same way. A chart that came from another program with something JeffCrunch cannot keep when it rewrites a chart (error bars, a three-dimensional chart) can still be moved, sized, and deleted, but its elements and data are not changed here.

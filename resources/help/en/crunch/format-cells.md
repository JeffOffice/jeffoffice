# Format cells
<!-- keywords: format, formatting, font, size, bold, italic, underline, double underline, strikethrough, color, font color, fill, fill color, background, border, borders, align, alignment, center, left, right, top, middle, bottom, wrap text, indent, rotate, rotated, orientation, angle, vertical text, merge, merge and center, unmerge, Format Cells, Ctrl+1, Format Painter, copy format, clear, clear formats, protection, locked, hidden, right-to-left, text direction, shrink to fit, justify -->

**Formatting** changes how cells look: the font, the colors, the borders, where the text sits, and how a number is written. It never changes what a cell holds. Select the cells first, then use the **Home** tab. Each change is one step you can undo.

## The Home tab

**Font group**

- The **font box** and **size box** change the typeface and the size in points. **Increase Font Size** and **Decrease Font Size** go up and down the usual sizes.
- **Bold** (**Ctrl+B**), **Italic** (**Ctrl+I**), **Underline** (**Ctrl+U**), and **Strikethrough** (**Ctrl+5**) turn on when the cell does not have them and off when it does. The arrow next to Underline also offers **Double Underline**.
- **Borders** opens a menu of lines to put on the cells: the bottom, top, left, or right edge, **No Border**, **All Borders**, the outside (also thick), a double or thick bottom line, top and bottom lines, and the lines inside. **Line Color** and **Line Style** (thin, medium, thick, dashed, dotted, double, or hairline) say how the lines look, and **More Borders** opens the Border tab of Format Cells. The last choice is on the button, so one click repeats it.
- **Fill Color** paints the background of the cells. **Font Color** colors the text. Each has a palette of the workbook's own colors, a row of standard colors, and **More Colors** for any other. **No Color** and **Automatic** take a color away.

**Alignment group**

- **Top Align**, **Middle Align**, and **Bottom Align** place the text up and down in the cell.
- **Align Left**, **Center**, and **Align Right** place it from side to side. A text that is not aligned reads from its own side: left for English, right for Arabic or Hebrew.
- **Decrease Indent** and **Increase Indent** move the text in from the edge, one step at a time.
- **Wrap Text** breaks a long text onto more lines inside the cell. The row does not grow by itself yet: use **Home > Format > AutoFit Row Height** to make it tall enough.
- **Orientation** turns the text: **Angle Counterclockwise**, **Angle Clockwise**, **Vertical Text** (the letters one under another), **Rotate Text Up**, **Rotate Text Down**, and **Horizontal Text** to put it back. The text turns inside the cell and stays where the alignment puts it.
- **Merge and Center** joins the selected cells into one and centers what is in it. Its arrow also offers **Merge Across** (one merged cell for each row), **Merge Cells** (no centering), and **Unmerge Cells**. Only the top left cell keeps its value; if other cells hold something, JeffCrunch asks first.

**Number group**: see [Number formats and your language](number-formats).

**Styles group**: see [Cell styles](cell-styles).

**Editing group**: **Clear** has **Clear All** (contents and formats), **Clear Formats** (the cells keep what they hold and show General with no fill or border), and **Clear Contents**.

## Format Painter

Select cells that look the way you want, click **Format Painter** (the brush in the Clipboard group), and then select the cells to give that look to. They take the font, fill, borders, number format, and alignment. **Double-click** the brush to paint more than once; click it again, or press **Esc**, to stop.

## The Format Cells window

Press **Ctrl+1** (on a Mac, **Command+1**) or use **Home > Format > Format Cells**. The small arrow at the corner of the Font, Alignment, and Number groups opens it on that tab. It has six tabs:

- **Number**: see [Number formats and your language](number-formats).
- **Alignment**: horizontal and vertical placement (including **Fill**, which repeats the text across the cell, **Justify**, and **Center Across Selection**, which centers over the empty cells beside it without merging them), indent, **Wrap text**, **Shrink to fit**, **Merge cells**, the **Orientation** in degrees from 90 up to 90 down or the letters one under another, and the **Text direction** (by context, left to right, or right to left).
- **Font**: typeface, style, size, underline (single, double, and the two accounting kinds), color, strikethrough, superscript, and subscript, with a preview.
- **Border**: pick a line style and color, then click the edges (or the presets **None**, **Outline**, and **Inside**) to put that line on them or take it away.
- **Fill**: the background color, a pattern and its color, with a sample.
- **Protection**: **Locked** and **Hidden**. They have no effect until the sheet is protected, and JeffCrunch does not protect sheets yet.

The window changes only what you changed. Open it on cells that differ, and a box you did not touch leaves each cell as it was.

## Whole columns and rows

Select whole columns (or whole rows) and a format is set for the column itself, so cells you type later in an empty part of it have the same look. Any other block can be up to 100,000 cells. A sheet that is protected in the file can't be formatted; take the protection off in the program that put it on.

## What is kept

The file keeps every look. Fonts, fills, borders, and cell formats you use are added to the workbook's lists once, and a look the workbook already had is found and reused, so the file does not fill with copies. Formatting a cell never touches the cells beside it, except that a border drawn on an edge is also drawn on the matching side of the cell next to it, so the line shows once.

# Cell styles
<!-- keywords: modify cell style, change a style, merge styles, import styles, cell style, styles, named style, Good, Bad, Neutral, Heading, Title, Total, Accent, Input, Output, Calculation, Check Cell, Explanatory Text, Note, Warning Text, Linked Cell, Comma, Currency, Percent, Normal, new style, name, delete style, remove style -->

A **cell style** is a named set of looks: a font, a fill, a border, a number format, and how the text sits. Give a cell the style "Heading 1" and it takes all of them at once. Styles keep a workbook consistent, because every cell with the style looks the same.

## Apply a style

1. Select the cells.
2. Click **Home > Cell Styles** and click a style. The menu shows a small sample of each one.

The menu has these groups:

- **Good, Bad and Neutral**: Normal, Good, Bad, and Neutral.
- **Data and Model**: Calculation, Check Cell, Explanatory Text, Input, Linked Cell, Note, Output, and Warning Text.
- **Titles and Headings**: Title, Heading 1 to 4, and Total.
- **Themed Cell Styles**: four tints of each of the six accent colours of the workbook's theme.
- **Number Format**: Comma, Comma [0], Currency, Currency [0], and Percent.
- **Custom**: styles the workbook already has under other names.

A style the workbook already has is used as it is, with the look it has there. A style the workbook lacks is made the first time you use it, from the workbook's own first font, and then it is part of the file with the others.

## Make a style of your own

1. Format a cell the way you want: a font, a fill, a border, a number format, and so on.
2. Leave the cursor on that cell. Select the cells that should get the style too.
3. Click **Home > Cell Styles > New Cell Style**, type a name, and click **OK**.

The style keeps the look of the cell with the cursor, and the selected cells are given it. It is now in the **Custom** group of the menu, and it is saved in the workbook. Each style needs its own name. A name that is already used (small or capital letters do not matter) is refused, and so is an empty name.

## Change a style

Click **Home > Cell Styles > Modify Cell Style**, and pick a style the workbook has (one the gallery offers is in the workbook once a cell has used it). The Format Cells dialog opens, starting from the style's own look. Change what you want and click **OK**: the style takes the change, and so does every cell that uses it, as one step you can undo. Only what a cell takes from the style changes: a cell that has formatting of its own keeps it, so a cell with its own fill keeps that fill when the style's fill changes. Whole columns and rows that are formatted with the style take the change too. A sheet that is protected against formatting cells, and has a cell that uses the style, refuses the change, and nothing is changed. You can't modify Normal this way. The style is saved in the workbook with its new look, and a cell given the style later has it.

## Merge styles from another workbook

Open the workbook that has the styles, then, in the workbook that should have them, click **Home > Cell Styles > Merge Styles** and pick the workbook. Each style it has that this one does not is added. When a style has the same name here (small or capital letters do not matter) and another look, you are asked once whether to replace those: **Replace** gives them the look from the other workbook, as Modify does (the cells that use them change only in what they take from the style), and **Keep** leaves your own. A colour that is a theme colour is brought in as the colour it shows in the other workbook, unless both workbooks have the same theme. Styles that look the same are passed over. It is one step you can undo, and Undo takes out every style it added and gives back every look it changed. The styles of the other workbook are only read, and it is not changed. At most 1,000 styles are brought in at once (more are refused, and nothing is merged); if the other workbook is busy, you are told to try again; Normal stays as it is, and a style you deleted here stays deleted.

## Delete a style

Click **Home > Cell Styles > Delete Cell Style**, and pick the style. Every cell that uses it goes back to Normal, the plain look, and the style leaves the menu. It is one step you can undo, and undoing brings back the style and the look of the cells. You can't delete Normal. The workbook file keeps the style's name, marked as hidden, so other programs do not show it either.

## Notes

- Applying a style is one step you can undo. It replaces the cell's font, fill, border, number format, alignment, and protection with the style's.
- A cell remembers its style. Formatting the cell afterward, for example making it bold, changes that cell only, and the cell keeps following the rest of the style.
- JeffCrunch does not yet have a command to change what a style is, or to bring styles in from another workbook. A workbook that has styles of its own shows them under **Custom** and in their group.

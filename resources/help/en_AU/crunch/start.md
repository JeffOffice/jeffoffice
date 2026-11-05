# Welcome to JeffCrunch
<!-- keywords: welcome, getting started, introduction, beginner, first steps, quick start, overview, what is JeffCrunch, basics, beta, spreadsheet, workbook, sheet, cells, what works, not yet, limits -->

JeffCrunch is a program for spreadsheets. It is free and open source. There is no fee, no subscription, and no account to set up. It is one of the JeffOffice apps, and it runs on Windows, macOS, and Linux.

The file you work on is called a **workbook**. A workbook holds one or more **sheets**, and each sheet is a grid of **cells**. JeffCrunch saves workbooks as .xlsx files. It opens .xlsx and .xlsm workbooks made by other programs too, and older .xls files, OpenDocument spreadsheets, and text files of values. It keeps what it can't edit yet.

## JeffCrunch is a beta

This version of JeffCrunch is a **beta**. It is ready for real work, but it does not do everything yet. Keep a copy of any workbook that matters.

**What works**

- Opening workbooks: .xlsx and .xlsm, older .xls, OpenDocument spreadsheets (.ods and .fods), and text files of values (.csv, .tsv, and .txt). Saving .xlsx, and copies as .xls or .ods. See [Open and save workbooks](open-save).
- Opening workbooks protected with a password (.xlsx, .xls, and .ods), and protecting an .xlsx workbook with one. See [Open password-protected workbooks](open-password).
- Typing numbers, text, and formulas in cells, with undo and redo. Formulas are worked out as you type, with more than 200 functions. You can click or arrow to cells while you type a formula, and the **Function Arguments** dialog fills in a function's arguments with a live result. See [Type in cells](edit-cells), [How formulas are shown and calculated](formulas), and [Enter a function with the Function Arguments dialog](function-arguments).
- Formatting cells: fonts, colours, borders, number formats, cell styles, and merged cells. See [Format cells](format-cells).
- Cutting, copying, and pasting, filling cells, adding them up, finding and replacing, sorting, filtering, tables, and pivot tables. See [Pivot tables](pivot-tables).
- Rows, columns, and sheets: inserting, deleting, sizing, hiding, and moving them.
- Charts, pictures, and shapes, and notes and links.
- Protecting a sheet or the workbook.
- Setting up the pages, seeing them before they print, printing, and making a PDF. See [Print and make a PDF](printing).

**What does not work yet**

- You can't make, change, or delete conditional format rules or data validation rules. JeffCrunch shows the ones a workbook has and keeps them.
- You can't change what a cell style is, or make a new one.
- A function JeffCrunch does not have yet leaves a new formula without a value; a formula that came with the workbook keeps the result the file stored.
- Macros are kept and saved back, but never run.
- Pivot tables have a few limits, which [Pivot tables](pivot-tables) lists.
- A workbook opened from .xls, .ods, or a text file is saved as a new .xlsx file, unless you choose an .xls or .ods copy. A copy can't hold everything, and JeffCrunch says what it will leave out before it saves.
- A password can protect an .xlsx workbook, but not an .xls or .ods copy.
- A data table (what-if table) can be worked out but not made.

## Do these five things first

1. **Open a workbook.** Press **{key:Open}**, or click **File** and then **Open**. The first rows appear at once, even in a very large workbook, while JeffCrunch reads the rest.
2. **Look around.** Click a cell to select it. Its address is in the **Name box** at the top left, and what it holds is in the **formula bar** next to it. Use the arrow keys, the mouse wheel, and the scroll bars to move. See [Select cells and move around](select-and-move).
3. **Change a cell.** Select a cell and type. Press **Enter** to keep what you typed. See [Type in cells](edit-cells).
4. **Format cells.** Select cells and use the **Home** tab to change the font, colours, borders, and number format. See [Format cells](format-cells).
5. **Change the view.** Use the **View** tab to zoom, to turn the gridlines and headings on or off, and to freeze the top rows or the first columns. See [Change the view](view).

## What you see

A workbook looks as it was saved: the widths of the columns and the heights of the rows, the fonts, colours, fills, and borders, merged cells, frozen panes, and the number formats that turn a stored number into a date, a percent, or a price. Text that is too long for its cell spills into the empty cells beside it. A number that is too wide for its cell shows as a row of # signs; widen the column, or select the cell to read the number in the formula bar. Numbers are written the way your computer's language writes them (see [Number formats and your language](number-formats)), and a workbook's conditional formats, such as colour scales, data bars, and icons, are drawn (see [Conditional formats](conditional-formats)).

Save your work with **{key:Save}**, which writes your changes into the workbook you opened, or **Save As**, which writes them into a new file. See [Open and save workbooks](open-save).

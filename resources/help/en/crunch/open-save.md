# Open and save workbooks
<!-- keywords: new, blank, book1, untitled, open, save, save as, file, workbook, xlsx, xlsm, xls, ods, csv, tsv, text file, delimited, import, export, recent, pinned, drag, drop, loading, large, big, macros, unsaved, changes, close, copy, calculation chain -->

When JeffCrunch starts with no file to open, the window holds a new, empty workbook with one sheet, named Book1. Each new workbook takes the next number (Book2, Book3, and so on), and a number is not used again after its window is closed. To start another one, press **{key:New}**, or click **File**, then **New**, and choose **Blank Workbook**; it opens in a window of its own. A new workbook that you have not changed gives way when you open a file in its window; one you have changed stays, and the file opens in another window. **Save** on a new workbook asks for a name, as **Save As** does, and saves it as a .xlsx file. Closing a new workbook you have not changed asks nothing.

To open a workbook, press **{key:Open}**, or click **File** and then **Open**, and choose **Browse**. You can also drag a workbook from your file manager onto the window. A workbook that is already open in a window is not opened a second time: that window comes to the front.

JeffCrunch opens **.xlsx** workbooks and **.xlsm** workbooks (those that hold macros). It does not run macros, and it keeps them untouched.

## Other kinds of files

JeffCrunch also opens older **.xls** workbooks, OpenDocument spreadsheets (**.ods** and the flat **.fods**), and text files of values (**.csv**, **.tsv**, and **.txt**). A workbook opened from one of them is saved as a new **.xlsx** file: when you press **{key:Save}**, JeffCrunch tells you so and asks for a name, and the file you opened is never changed.

An OpenDocument spreadsheet is read with its sheets, values, formulas (with the results the file kept), number formats, fonts, fills, borders, merged cells, column widths and row heights, hidden rows and columns, frozen panes, and names. Charts, pictures and other drawings, comments, pivot tables, macros, conditional formats, and data validation are not read yet; a message at the top tells you which of them the file has, and a copy saved from here will not have them. A spreadsheet that is protected with a password is not opened: open it in the program that made it and save a copy without the password.

A text file is one sheet. JeffCrunch finds out how its values are separated (by commas, semicolons, tabs, or vertical bars) and how its letters are coded (UTF-8 or UTF-16, or else the code page of your language), and reads each value as you would type it in your language: a number such as 1.234,5 in German, a percent, an amount of money, a date with its year (such as 3/15/2024, or 15.03.2024 in German), a time, TRUE or FALSE (in your language, such as WAHR and FALSCH in German, and in English). A value that begins with an equals sign is a formula. Digits that begin with a zero (such as 007) and numbers of more than 15 digits stay text, so that a postal code or an account number is not changed, and so do a day and a month with no year, in any order and in any language (such as 3/4, 12.5, 1-2, March 1, or Dec 25), because the year would have to be made up, and texts such as the gene name MARCH1. A sheet holds 1,048,576 rows and 16,384 columns; if the file has more, JeffCrunch opens what fits and tells you.

When you open a text file, a window first shows its first rows and asks how it is read: what separates the fields (any of a comma, semicolon, tab, space, bar, or a character you type, with several in a row counted as one if you like), what quotes a field that holds a separator, the text encoding, and the row to begin at. Above each column you choose how it is read: **As typed**, **Text** (to keep a part number like 007 or a long account number as it is), a **Date** in the order you say, or **Leave out**. The preview follows every change, and shows at most the first 200 rows and 100 columns, but the whole file is opened. **File > Open > Open a Text File** asks even when the file's name is unusual.

To keep only the values of the sheet you are looking at, click **File**, then **Save As**, and choose the **.csv** kind of file. JeffCrunch writes the values as the cells show them, in UTF-8, with a semicolon between them where your language writes a comma in its numbers and a comma elsewhere, and text in quotes when it needs them. The other sheets, the formulas, the formats, and the settings are not in a .csv file, and your workbook is not changed by writing one.

## Large workbooks

JeffCrunch reads a workbook in the background. The first rows of the sheet that was open when the workbook was saved appear almost at once, even when the workbook is very large. The status bar at the bottom left tells you what it is doing: **Opening**, **Reading** a sheet with the count of cells read so far, **Calculating** with a percent, and then **Ready**.

You can scroll, select, and copy while it reads. The other sheets' tabs turn on when they are read. A workbook that is damaged is opened as far as it can be read, and a message says what was left out.

## Recent workbooks

The **Open** page of the File screen lists the workbooks you opened lately. Right-click one to **pin** it to the top of the list or to remove it from the list.

## Saving

Press **{key:Save}**, or click **File** and then **Save**, to save your changes into the workbook you opened. To keep the original and make a new file, click **File** and then **Save As**, and choose a name. A workbook with macros is saved as a **.xlsm** file, and one without as a **.xlsx** file.

JeffCrunch saves only what you changed. Everything else in the file, such as the other sheets, the charts, the pictures, the macros, and the settings that JeffCrunch does not show, is copied as it was, byte for byte. In a sheet you changed, only the cells you changed are written again, along with the formulas that depend on them, which get their new values. If you changed no cell, the file is left alone.

If another program changed the file after you opened it, JeffCrunch does not save over it without asking, because that would lose the other program's changes. It asks whether to **Save a Copy**, to **Overwrite** the file, or to **Cancel**.

The new file is written next to the old one first, and it takes the old one's place only when it is complete. If anything goes wrong, you see a message, and the file that was there is as it was.

If the formulas are still being worked out when you save, JeffCrunch waits until they are, so the file holds their results. If you press **Esc** to stop the calculation meanwhile, the save is cancelled, and the status bar says so; save again when you want to. JeffCrunch leaves one thing out of the file: the list of the order in which a program last worked the formulas out (the calculation chain). A program that opens the workbook next makes it again. If JeffCrunch could not work out some formula, or you stopped the calculation, the file asks that program to work every formula out again when it opens the workbook.

## Closing with changes

If you close a workbook that has changes that are not saved, JeffCrunch asks whether to **Save** them, to **Don't Save** them, or to **Cancel** and go on working. An asterisk (*) in the window's title shows that there are changes not saved.

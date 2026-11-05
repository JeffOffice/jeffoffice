# Open and save workbooks
<!-- keywords: copy, older format, opendocument, compatibility, new, blank, book1, untitled, open, save, save as, file, workbook, xlsx, xlsm, xls, ods, csv, tsv, text file, delimited, import, export, recent, pinned, drag, drop, loading, large, big, macros, unsaved, changes, close, copy, calculation chain, template, xltx, xltm, cannot open, damaged, encrypted, password, save a copy, read-only, read only, open as copy, new window, backup copy, my templates, save as template, ots, repair, open and repair -->

When JeffCrunch starts with no file to open, the window holds a new, empty workbook with one sheet, named Book1. Each new workbook takes the next number (Book2, Book3, and so on), and a number is not used again after its window is closed. To start another one, press **{key:New}**, or click **File**, then **New**, and choose **Blank Workbook**; it opens in a window of its own. A new workbook that you have not changed gives way when you open a file in its window; one you have changed stays, and the file opens in another window. **Save** on a new workbook asks for a name, as **Save As** does, and saves it as a .xlsx file. Closing a new workbook you have not changed asks nothing.

To open a workbook, press **{key:Open}**, or click **File** and then **Open**, and choose **Browse**. You can also drag a workbook from your file manager onto the window. A workbook that is already open in a window is not opened a second time: that window comes to the front.

JeffCrunch opens **.xlsx** workbooks and **.xlsm** workbooks (those that hold macros), and the templates **.xltx** and **.xltm** (a template that holds macros). It does not run macros, and it keeps them untouched: the macro project, its signature, and the parts that name them are written back exactly as they were. A template is saved as a template, and a workbook with macros as a workbook with macros, because the kind is part of the file.

## Other kinds of files

JeffCrunch also opens older **.xls** workbooks, OpenDocument spreadsheets (**.ods** and the flat **.fods**), and text files of values (**.csv**, **.tsv**, and **.txt**). A workbook opened from one of them is saved as a new **.xlsx** file (or as an **.xls** or **.ods** copy: see below): when you press **{key:Save}**, JeffCrunch tells you so and asks for a name, and the file you opened is never changed.

An OpenDocument spreadsheet is read with its sheets, values, formulas (with the results the file kept), number formats, fonts, fills, borders, merged cells, column widths and row heights, hidden rows and columns, frozen panes, and names. Charts, pictures and other drawings, comments, pivot tables, macros, conditional formats, and data validation are not read yet; a message at the top tells you which of them the file has, and a copy saved from here will not have them. A spreadsheet that is protected with a password asks for it when you open it: see [Open password-protected workbooks](open-password).

A text file is one sheet. JeffCrunch finds out how its values are separated (by commas, semicolons, tabs, or vertical bars) and how its letters are coded (UTF-8 or UTF-16, or else the code page of your language), and reads each value as you would type it in your language: a number such as 1.234,5 in German, a percent, an amount of money, a date with its year (such as 3/15/2024, or 15.03.2024 in German), a time, TRUE or FALSE (in your language, such as WAHR and FALSCH in German, and in English). A value that begins with an equals sign is a formula. Digits that begin with a zero (such as 007) and numbers of more than 15 digits stay text, so that a postal code or an account number is not changed, and so do a day and a month with no year, in any order and in any language (such as 3/4, 12.5, 1-2, March 1, or Dec 25), because the year would have to be made up, and texts such as the gene name MARCH1. A sheet holds 1,048,576 rows and 16,384 columns; if the file has more, JeffCrunch opens what fits and tells you.

When you open a text file, a window first shows its first rows and asks how it is read: what separates the fields (any of a comma, semicolon, tab, space, bar, or a character you type, with several in a row counted as one if you like), what quotes a field that holds a separator, the text encoding (Unicode, the Windows code pages, the ISO 8859 pages, Mac Roman, and the East Asian ones), and the row to begin at. Above each column you choose how it is read: **As typed**, **Text** (to keep a part number like 007 or a long account number as it is), a **Date** in the order you say, or **Leave out**. The preview follows every change, and shows at most the first 200 rows and 100 columns, but the whole file is opened. **File > Open > Open a Text File** asks even when the file's name is unusual.

To keep only the values of the sheet you are looking at, click **File**, then **Save As**, and choose the **.csv** kind of file (or **.tsv** and **.txt**, tab-separated). Before it writes the file, JeffCrunch says what your workbook has that the file will not hold (the other sheets, formulas, pictures, charts, notes, and macros, for example), and asks how to write it, and whether to **Save Values Only**, to **Save as .xlsx** instead and keep everything, or to **Cancel**:

- **Separator.** A comma, semicolon, tab, space, vertical bar, or a character you type. A **.csv** begins with a semicolon where your language writes a comma in its numbers, and a comma elsewhere; a **.tsv** or **.txt** with a tab.
- **Text encoding.** UTF-8 with a mark (so that other programs take accented letters right), UTF-8 without it, UTF-16, or a code page such as Western European (1252), the ISO 8859 pages, Mac Roman, or the Japanese, Chinese, Korean, Hebrew, Arabic, Baltic, Vietnamese, and Thai pages. A letter the code page has no byte for is written as a question mark, and the status bar says how many.
- **Line ends.** Windows (carriage return and line feed), Linux and Mac (line feed), or old Mac (carriage return).
- **Cells.** **As shown in the cells** writes what you see. **As stored** writes every digit of a number (0.1 and not 10%), and a date as ISO text such as 2026-03-04.

Text goes in quotes when it holds the separator, a quote, a line break, or a space at either end. The other sheets, the formulas, the formats, and the settings are not in the file, and your workbook is not changed by writing one.

A workbook that came from a text file is written back in the form the file had: the same separator, line ends, and encoding. If you saved it as a copy without changing a cell, the copy is the same file, byte for byte, and nothing is asked.

## Saving a copy as .xls or .ods

To make a copy that older programs, or programs that read OpenDocument, can open, click **File**, then **Save As**, and choose the **.xls** or **.ods** kind of file. The copy is made from your workbook as it is now, with the changes you have not saved yet, and holds:

- the values, with their texts, numbers, truth values, and errors;
- the formulas, with the results they have now (an array formula stays an array formula);
- the number formats, fonts, fills, borders, alignment, and whether a cell is locked;
- merged cells, column widths and row heights, hidden rows and columns, row and column outlines, and frozen panes;
- the sheets, in their order, with their names and which are hidden, and the names you defined (and the print areas);
- the links, and how each sheet prints (the paper, margins, orientation, scale, headers and footers, and page breaks).

An **.xls** file also holds data tables, and keeps the workbook's calculation settings. A copy cannot hold everything. Before JeffCrunch writes the file, it says what your workbook has that the file will not: charts, pictures and other drawings, notes, pivot tables, tables, conditional formats, data validation, filters, sheet protection, form controls, macros, and links to other workbooks (the formulas that use them keep the values they had). A formula that the older form cannot say (a function it does not have, or a cell past column IV or row 65,536) is written as the value it came to, and the message counts them. The older form also stops at 65,536 rows, 256 columns, 255 sheets, and 32,767 characters in a cell, and at about 4,000 cell formats and 56 colors: what does not fit is cut, and the message says how much. You can then choose to **Save Copy**, to **Save as .xlsx** and keep everything, or to **Cancel**.

Your workbook is not changed by the copy, and it is not marked as saved. The file you opened is never written: give the copy another name. A copy of an **.xls** or **.ods** file that you opened can be saved as the other kind.

## Save a copy

Click **File > Save a Copy** and choose a name. JeffCrunch writes the workbook to that file, with the changes you have not saved, and you keep working in the workbook you had. Its name and its unsaved changes stay as they were. You can choose any kind of file that **Save As** offers. A copy is never written over the workbook's own file.

## Keep a backup copy

Click **File > Options**, and on the **General** page turn on **Always create backup copy**. Each time you save, JeffCrunch first copies the old file to one called "Backup of" plus its name, in the same folder. If the copy can't be made, the app asks whether to save without it. The answer is No unless you say Yes, and the file stays as it was.

## Save as a template

A template is a workbook you use as a starting point. It can hold formulas, styles, and sheets. Click **File > Save as Template** and type a name. The template (an **.xltx** file) goes in **My Templates**, and your workbook is not changed. In **Save As**, you can also choose **Templates (*.xltx)**, or **OpenDocument spreadsheet templates (*.ots)**, to save a template file where you like.

To start a workbook from your template, click **File > New**. Your templates are listed under **My Templates**. Click one. A new workbook opens with the template's cells and sheets. It has no file of its own yet, so **Save** asks for a name, and the template is not changed. To remove a template from the list, right-click it and choose **Remove from My Templates**.

## Other ways to open a file

On the **Open** page, click **More Ways to Open**. You can also right-click a name in the Recent list.

- **Open Read-Only** opens the workbook for reading only. You can look, scroll, search, and copy, but you can't change a cell. The title bar says "Read-Only". If you press **Save**, JeffCrunch asks for a new name, so the file you opened is not changed.
- **Open as Copy** opens the workbook with no file of its own. **Save** asks where to put it. The file you opened is never changed.
- **Open in New Window** opens the workbook in a window of its own, even when the window you are in is empty.
- **Open and Repair** is for an .xlsx file that is damaged. See below.

## Open and Repair

Sometimes an .xlsx file is damaged. A download stopped halfway, or a disk made mistakes. An .xlsx file is a bundle of smaller parts, such as the sheets, the styles, and the shared text. **Open and Repair** reads every part that it can. A part that stopped in the middle is kept up to the place where it stopped, so you may get the first rows of a sheet. A part that can't be read is left out. If the list of parts or the links between them are gone, JeffCrunch makes them again.

The repaired workbook opens with no file of its own, and a name like "Repaired budget". A message tells you what was kept up to a break and what was left out. The damaged file is not changed. Save the repaired workbook under a new name, and check the numbers before you trust it.

## Large workbooks

JeffCrunch reads a workbook in the background. The first rows of the sheet that was open when the workbook was saved appear almost at once, even when the workbook is very large. The status bar at the bottom left tells you what it is doing: **Opening**, **Reading** a sheet with the count of cells read so far, **Calculating** with a percent, and then **Ready**.

You can scroll, select, and copy while it reads. The other sheets' tabs turn on when they are read. A workbook that is damaged is opened as far as it can be read, and a message says what was left out.

## If a file does not open

When JeffCrunch cannot open a file, it says why in plain words and leaves the workbooks you have open as they are. It tells a file that is empty from one that is not a workbook at all (a PDF document, a picture, a text document, or a presentation, for example); a zip package that was cut short or damaged, which usually means a download or a copy was interrupted, so copy it again from the original; a workbook that is protected with a password, which it asks for (see [Open password-protected workbooks](open-password)); and a compound file that holds no workbook.

## Recent workbooks

The **Open** page of the File screen lists the workbooks you opened lately. Right-click one to **pin** it to the top of the list or to remove it from the list.

## Saving

Press **{key:Save}**, or click **File** and then **Save**, to save your changes into the workbook you opened. To keep the original and make a new file, click **File** and then **Save As**, and choose a name. A workbook with macros is saved as a **.xlsm** file, a template as a **.xltx** or **.xltm** file, and any other as a **.xlsx** file. A name that ends in another kind of workbook is not accepted, and the message says which ending to use; the names ending in **.xls**, **.ods**, **.csv**, **.tsv**, and **.txt** make copies, as described above.

JeffCrunch saves only what you changed. Everything else in the file, such as the other sheets, the charts, the pictures, the macros, and the settings that JeffCrunch does not show, is copied as it was, byte for byte. In a sheet you changed, only the cells you changed are written again, along with the formulas that depend on them, which get their new values. If you changed no cell, the file is left alone.

If another program changed the file after you opened it, JeffCrunch does not save over it without asking, because that would lose the other program's changes. It asks whether to **Save a Copy**, to **Overwrite** the file, or to **Cancel**.

The new file is written next to the old one first, and it takes the old one's place only when it is complete. If anything goes wrong, you see a message, and the file that was there is as it was.

If the formulas are still being worked out when you save, JeffCrunch waits until they are, so the file holds their results. If you press **Esc** to stop the calculation meanwhile, the save is cancelled, and the status bar says so; save again when you want to. JeffCrunch leaves one thing out of the file: the list of the order in which a program last worked the formulas out (the calculation chain). A program that opens the workbook next makes it again. If JeffCrunch could not work out some formula, or you stopped the calculation, the file asks that program to work every formula out again when it opens the workbook.

## Closing with changes

If you close a workbook that has changes that are not saved, JeffCrunch asks whether to **Save** them, to **Don't Save** them, or to **Cancel** and go on working. An asterisk (*) in the window's title shows that there are changes not saved.

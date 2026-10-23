# Open and save workbooks
<!-- keywords: open, save, save as, file, workbook, xlsx, xlsm, recent, pinned, drag, drop, loading, large, big, macros, unsaved, changes, close, copy, calculation chain -->

To open a workbook, press **Ctrl+O**, or click **File** and then **Open**, and choose **Browse**. You can also drag a workbook from your file manager onto the window.

JeffCrunch opens **.xlsx** workbooks and **.xlsm** workbooks (those that hold macros). It does not run macros, and it keeps them untouched.

## Large workbooks

JeffCrunch reads a workbook in the background. The first rows of the sheet that was open when the workbook was saved appear almost at once, even when the workbook is very large. The status bar at the bottom left tells you what it is doing: **Opening**, **Reading** a sheet with the count of cells read so far, **Calculating** with a percent, and then **Ready**.

You can scroll, select, and copy while it reads. The other sheets' tabs turn on when they are read. A workbook that is damaged is opened as far as it can be read, and a message says what was left out.

## Recent workbooks

The **Open** page of the File screen lists the workbooks you opened lately. Right-click one to **pin** it to the top of the list or to remove it from the list.

## Saving

Press **Ctrl+S**, or click **File** and then **Save**, to save your changes into the workbook you opened. To keep the original and make a new file, click **File** and then **Save As**, and choose a name. A workbook with macros is saved as a **.xlsm** file, and one without as a **.xlsx** file.

JeffCrunch saves only what you changed. Everything else in the file, such as the other sheets, the charts, the pictures, the macros, and the settings that JeffCrunch does not show, is copied as it was, byte for byte. In a sheet you changed, only the cells you changed are written again, along with the formulas that depend on them, which get their new values. If you changed no cell, the file is left alone.

The new file is written next to the old one first, and it takes the old one's place only when it is complete. If anything goes wrong, you see a message, and the file that was there is as it was.

If the formulas are still being worked out when you save, JeffCrunch waits until they are, so the file holds their results. JeffCrunch leaves one thing out of the file: the list of the order in which a program last worked the formulas out (the calculation chain). A program that opens the workbook next makes it again. If JeffCrunch could not work out some formula, or you stopped the calculation, the file asks that program to work every formula out again when it opens the workbook.

## Closing with changes

If you close a workbook that has changes that are not saved, JeffCrunch asks whether to **Save** them, to **Don't Save** them, or to **Cancel** and go on working. An asterisk (*) in the window's title shows that there are changes not saved.

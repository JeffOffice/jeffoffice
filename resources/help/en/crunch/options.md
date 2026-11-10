# Change JeffCrunch's settings
<!-- keywords: recovery, recover unsaved work, automatic recovery, proofing, editing, autocomplete, fill handle, drag and drop, decimal places, options, settings, preferences, usage statistics, install number, privacy, privacy notice, JP_NO_TELEMETRY, delete statistics, updates, about, edit in cell, gridline color, default zoom, standard width, privacy, personal information -->

Your settings are in the **JeffCrunch Options** window. Click **File > Options** to open it. Click **OK** to keep your changes, or **Cancel** to throw them away.

## Editing

The **Editing** page changes how typing in cells works.

- **After pressing Enter, move the selection** says where the selection goes when you finish an entry: down, right, up, left, or nowhere.
- **Offer to finish a text from the cells above and below** is called AutoComplete. When you type the first letters of a text, and only one text in that column starts with them, JeffCrunch shows the rest of it, selected. Press **Enter** to keep it. Keep typing to ignore it, or press **Backspace** to take it away. It looks only at the block of cells next to the one you are typing in, and only at texts, not numbers.
- **Use the fill handle, and drag cells to move or copy them** turns on the small square at the corner of the selection and the dragging of the selection's border. Turn it off if you keep moving cells by accident. Copy and Paste still work.
- **Show a zero in cells that have a zero value (this sheet)** turns zeros on or off for the sheet you are looking at. When it is off, a cell that holds 0 looks empty, but it still holds the 0 and adds up as 0. The choice is saved with the sheet.
- **Put a decimal point in whole numbers that are typed** is called automatic decimal places. Set **Places** to 2, and typing 1234 enters 12.34. Set it to -2, and typing 5 enters 500. A number you type with a decimal point is entered as typed.

- **Allow editing directly in cells** is on by default. When you turn it off, a typed entry or F2 goes to the formula bar, and the cell shows the result only when you finish.

## Save options

**Save automatic recovery information every** sets how often JeffCrunch keeps a copy of a workbook with changes you have not saved, from 1 to 120 minutes. The copy is a .xlsx file in a folder that JeffCrunch keeps for this purpose, made in the background so you can go on typing. It is removed when you save or close the workbook. If JeffCrunch closes by surprise, it offers the copies the next time it starts. You can also click **File > Open > Recover Unsaved Work**. A workbook that is protected with a password is not copied, because the copy would not be protected.

## Proofing options

The **Proofing** page is shared by the JeffOffice apps. It decides whether spelling is checked as you type in a cell (turn off **Check spelling as you type** to get rid of the red wavy lines in every open window and the other apps; **Spelling** on the Review tab still works), whether words in capital letters and words with numbers are ignored when spelling is checked, whether AutoCorrect fixes words as you type, whether straight quote marks become curly ones, whether dashes, fractions, and ordinals are changed, and whether a web address becomes a link. See [Spelling](notes-links) and [Type in cells](edit-cells).

## Display options

- **Gridline color** is the color of the lines between cells on your screen. It is a choice for your screen only, not saved in the workbook.
- **Zoom of a new workbook** is how large the sheets of a new workbook look, from 10% to 400%.
- **Standard column width (this sheet)** is the width of every column of this sheet that has no width of its own. It is the same as **Default Width** on the Home tab.

## Privacy options

- **Take my name and company out of a workbook's properties each time it is saved** clears the author, who saved it last, the company, and the manager from the workbook's properties when you save. The properties are the facts a file keeps about itself.

## Usage statistics

**Send anonymous usage statistics** lets you share facts about how JeffCrunch is used. Anonymous means the facts say nothing about who you are. When this is on, JeffCrunch sends these facts once a day, and on the day you update: that it is JeffCrunch, its version, your operating system and language, how many times it was started, how often each command is used, and what kind of file you opened or saved. If JeffCrunch ever closes by surprise, it also sends a crash report the next time it starts. JeffCrunch never sends your files, their names, or anything in them, and nothing you typed. If this is off, no crash report is sent either.

The choice belongs to all the JeffOffice apps: change it here and it changes in each of them. JeffCrunch asks about it the first time it starts, in a window called **Help improve JeffCrunch**.

- **Privacy notice** next to the check box opens our privacy notice at jeffoffice.com/privacy. It says what is sent, where it goes, and how long we keep it. We keep statistics for 24 months.
- **Install number** is shown under the check box. It is a random number made up for your copy of JeffOffice, the same in every JeffOffice app. It says nothing about you. Click **Copy** to copy it. If statistics are off, or no number has been made yet, it says **none**.
- To have the statistics we already hold deleted, send your install number to info@jeffoffice.com **before** you turn statistics off. Turning them off erases the number from your computer, and without it we cannot tell which records are yours.
- To turn statistics off without opening JeffCrunch, set the environment variable `JP_NO_TELEMETRY` to any value before you start it. Nothing is then counted or sent, and JeffCrunch does not ask the first-run question.

## Related

- [Welcome to JeffCrunch](start)

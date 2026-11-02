# Fields
<!-- keywords: field, fields, update field, F9, unlink, lock, field codes, page number, date, NUMPAGES, SEQ, REF, PAGEREF, formula, Insert Field -->

A **field** is a piece of text that the program works out for you, such as a page number, today's date, or the number of a figure. It has two parts: a **code**, which says what to work out, and a **result**, which is the text you see. The code for a page number is `PAGE`, and its result on the third page is `3`.

## Put a field in the text

1. Click where the field should go.
2. Click **Insert > Field**.
3. Choose a field from the list. Its code appears in the box below, and you can change it.
4. Click **OK**.

For example, choose **Date**, change the code to `DATE \@ "MMMM d, yyyy"`, and click **OK**. The text shows the date as `November 2, 2026`.

## Update fields

A field shows the result it had when it was last worked out. Press **F9** to work out the field the caret is in again. To update many fields at once, select the text that holds them, and press **F9**. Press **Ctrl+A** and then **F9** to update every field in the document.

Updating is one step: **Undo** puts every result back. Updating is not a tracked change, even when Track Changes is on, because the result is not something you typed.

A field in a header or footer that shows the page number, the page count, or a heading is always right, and needs no update.

## Show the codes

Press **Shift+F9** to show the code of the field the caret is in, in braces such as `{ PAGE }`, in place of its result. Press **Alt+F9** to show the codes of every field, and press it again to show the results. This only changes what you see: the file is saved as it was, and the document is not marked as changed.

## Unlink or lock a field

- **Ctrl+Shift+F9** turns the field into plain text. The result stays, and it is no longer a field.
- **Ctrl+F11** locks a field, so **F9** leaves it alone. **Ctrl+Shift+F11** unlocks it.

## Formats

Add switches to the code to change how a result looks:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower`, and `\* Caps` change the case or the kind of number. `{ = 21 \* CardText }` shows `twenty-one`.
- `\# "0.00"` sets how a number looks. `{ = 1234.5 \# "#,##0.00" }` shows `1,234.50`.
- `\@ "dddd, MMMM d"` sets how a date looks. `M` is the month, and `m` is the minute.

## Fields JeffWrite works out

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, QUOTE, a formula that begins with `=`, and TOC. A field of any other kind keeps the result it has, and is saved as it was.

DATE, TIME, FILENAME, and AUTHOR depend on the computer and the moment, so they can change every time you update.

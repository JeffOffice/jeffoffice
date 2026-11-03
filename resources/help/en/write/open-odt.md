# Open and save OpenDocument files
<!-- keywords: odt, fodt, ott, opendocument, open document, open, save, save as, format, file type, other programs, trade documents, share, compatibility, tracked changes, left out, template -->

JeffWrite opens and saves OpenDocument text, the kind of file whose name ends in **.odt**. Many free programs read and write it, so it is a good way to trade a document with someone who doesn't use JeffWrite.

## Open an OpenDocument file

Press **Ctrl+O**, or click **File > Open**, and choose a file:

- **.odt** is a document.
- **.ott** is a template. It opens as a new document made from it, so the template stays as it is.
- **.fodt** is the same kind of document kept in one plain XML file. JeffWrite does not write .fodt files: **Save** on one asks for a new name and saves it as an .odt file.

JeffWrite finds out what a file is from what is inside it, not from its name, so a file with the wrong ending opens too. You can also give a file's name when you start JeffWrite, or drag it onto the program from your file manager, as you do with a .docx file.

JeffWrite reads the text and its looks, headings, styles and the styles they are based on, bulleted and numbered lists, tables (with merged cells, column widths, borders, shading, and tables inside tables), sections and columns, the page's size, margins, and direction, headers and footers (first page, left, and right), footnotes and endnotes, fields (page numbers, page count, title, author, file name, cross-references, and counted numbers), bookmarks, links, a table of contents, pictures, text boxes, tab stops, comments, and tracked changes, which show as marks you can accept or reject.

Some things JeffWrite can't show yet: drawn shapes that hold no text, formulas, charts, and other objects set inside the text. If a file has any, JeffWrite tells you which when you open it. It then does not save over that file: **Save** asks for a new name, so the original keeps what the copy lost. A document protected with a password isn't opened. Open it in the program that made it, and save a copy without the password.

## Save an OpenDocument file

Click **File > Save As**, and choose **OpenDocument Text (*.odt)** in the list of file types. After that, **Ctrl+S** saves in the same kind again.

The file holds what is listed above, with its comments and its tracked changes: added and deleted text, changed looks, and paragraph breaks that were added or deleted. For a few things the format has no place, and JeffWrite says what they are after you save: they are saved as they stand now or left out. These are tracked changes to tables, columns, and page setup, and tracked changes in headers, footers, and notes (saved as they stand now); form fields (saved as the text they show); equations (saved as plain text); and drawings, shapes, charts, and embedded objects that can't be kept. If you save again after a change that leaves out something new, JeffWrite tells you about that too, and says nothing again about what it already told you.

Other programs that read OpenDocument open the file without asking to repair it.

## Related

- [Open, save, print, and export](open-save)
- [Track changes, and accept or reject them](track-changes)
- [Welcome to JeffWrite](start)

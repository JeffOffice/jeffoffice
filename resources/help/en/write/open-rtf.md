# Open RTF files
<!-- keywords: rtf, rich text, rich text format, open rtf, .rtf, older documents, import, open, convert -->

**RTF** (Rich Text Format) is a kind of document file that almost every word processor can read and write. JeffWrite opens **.rtf** files, and it keeps their text and their looks.

## Open an RTF file

Press **Ctrl+O**, or click **File > Open**, and click **Browse**. Choose **Rich Text Format (*.rtf)** in the list of file types to see only these files, or pick the file from the list of all documents. You can also drag an .rtf file onto a JeffWrite window, or give its name when you start JeffWrite from a command line.

JeffWrite tells an RTF file by what is inside it, not by its name, so a file with a different ending still opens if it is really an RTF file.

## What comes across

JeffWrite reads these parts of an RTF file:

- Text in any language, with its fonts, sizes, colors, bold, italic, underline, strikethrough, superscript, subscript, and highlight.
- Paragraph settings: alignment, indents, spacing, tab stops, borders, shading, and the keep-with-next and keep-lines-together settings.
- Styles, numbered lists, and bulleted lists.
- Tables, including tables inside tables, borders, shading, column widths, and cells that are joined.
- Page size, margins, columns, page orientation, sections, headers, and footers.
- Footnotes, endnotes, page-number fields, other fields, and bookmarks.
- Tracked changes: added text, deleted text, paragraph breaks that were added or deleted, and changed looks. A **tracked change** is an edit that a reviewer marked. See [Track changes, and accept or reject them](track-changes).
- Pictures in PNG, JPEG, EMF, and WMF form, and page breaks and column breaks.

If part of the file can't be read, JeffWrite tells you what it left out. Some RTF files hold things that a document in JeffWrite can't hold yet, such as drawings or equations. They are left out, and the text around them is kept.

## Save your changes

JeffWrite does not write .rtf files. When you save a document that came from an .rtf file, JeffWrite asks for a new name and saves it as a .docx file. The .rtf file stays as it was.

## Related

- [Open, save, print, and export](open-save)
- [Welcome to JeffWrite](start)

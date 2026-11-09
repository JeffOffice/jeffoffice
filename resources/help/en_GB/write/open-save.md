# Open, save, print, and export
<!-- keywords: web page, html, save as web page, filtered web page, email, eml, bookmarks, pdf bookmarks, shift+f5, last edit, where you stopped, built-in templates, gallery, letter, report, brief, manuscript, newsletter, open, save, save as, docx, doc, dot, odt, opendocument, rtf, older files, template, dotx, recent, pin, new, close, print, pdf, export, recover, automatic recovery, unsaved, crash, file screen, file formats, save a copy, read-only, read only, open as copy, new window, backup copy, my templates, save as template, ott, repair, open and repair, damaged, password, encrypt, protect with a password -->

Click **File** at the top left to see the File screen. It has pages for New, Open, Print, and Export, and commands for Save and Save As. Press **Esc** to go back to your document.

## Start a new document

Press **Ctrl+N**, or click **File > New** and then **Blank Document**. A new document opens in its own window. If the window you are in is empty and was never saved, the new document takes its place.

## Open a document

Press **Ctrl+O**, or click **File > Open**. Click **Browse** to pick a file. JeffWrite opens these kinds of file:

- **.docx** is a document.
- **.dotx** is a template. A template opens as a new document made from it, so the template stays as it is.
- **.docm** and **.dotm** are a document and a template that can hold **macros**, small programs that come with a file. JeffWrite never runs them. It keeps them as they were when you save as .docm or .dotm. If you save as .docx or .odt, the macros are left out, and JeffWrite tells you so.
- **.txt** is plain text, only the words. When you open one, JeffWrite reads it at once if it is plain ASCII or Unicode. If it is not, JeffWrite asks which encoding to read it in and shows the first lines, so you can see that the letters look right. An **encoding** is the way a file stores letters, such as UTF-8 or Windows 1252. When you save as .txt, you choose the encoding and the line ends, and the file keeps only the words: no fonts, pictures, or table lines.
- **.html** and **.htm** are web pages. JeffWrite reads the words, headings, bold, italic, underline, colours, lists, and tables. Bullets and numbers of lists come in as typed marks, and pictures and links are left out. It never fetches anything from the internet, and it ignores scripts. A web page is never written over: **Save** asks for a new name and saves a .docx file. To make a web page, see **Save as a web page** below.
- **.doc** and **.dot** are the older kinds of document and template. JeffWrite reads them, and it saves what you do with them as a new .docx file.
- **.odt**, **.ott**, and **.fodt** are OpenDocument files. JeffWrite reads all three. It saves .odt files, and it saves what you do with a .ott or .fodt file as a new .odt file. See [Open and save OpenDocument files](open-odt).
- **.rtf** is Rich Text Format. JeffWrite reads it and saves it. See [Open RTF files](open-rtf).

The **Open** page also lists the documents you opened or saved lately. Click one to open it. Right-click one to pin it to the top of the list, or take it off the list.

If a file has parts that JeffWrite can't edit yet, such as a table's layout or a header, it keeps them. See [Welcome to JeffWrite](start). If part of a file can't be read at all, JeffWrite tells you what it left out. It then does not save over that file: Save asks for a new name, so that the original keeps what the copy lost.

## Save a document

Press **Ctrl+S** to save. The first time, JeffWrite asks for a name. **Save As** (F12) saves a copy under a new name. Choose **Template (*.dotx)** to save a template, **Macro-Enabled Document (*.docm)** to keep macros, **Plain Text (*.txt)** for only the words, or **Rich Text Format (*.rtf)** for a file other programs can read. A file that came from an older kind of file is always saved under a new name, so the old file is never written over.

## Save a copy

Click **File > Save a Copy**. Choose a name. JeffWrite writes the document to that file, and you keep working in the document you had. Its name and its unsaved changes stay as they were. This is handy when you want to send a version to someone and go on writing.

## Keep a backup copy

Click **File > Options**, and on the **General** page turn on **Always create backup copy**. Each time you save, JeffWrite first copies the old file to one called "Backup of" plus its name, in the same folder. If the copy can't be made, JeffWrite asks whether to save without it. The answer is No unless you say Yes, and the file stays as it was. Then you can go back to the last saved version.

## Save as a template

A template is a document you use as a starting point. Click **File > Save as Template**, and type a name. The template goes in **My Templates**, and the document you are writing is not changed. In **Save As**, you can also choose **Template (*.dotx)**, or **OpenDocument Text Template (*.ott)**, to save a template file where you like.

To start a new document from your template, click **File > New**. Your templates are listed under **My Templates**. Click one. A new document opens with the template's text and styles. To remove a template from the list, right-click it and choose **Remove from My Templates**.

## Other ways to open a file

On the **Open** page, click **More Ways to Open**. You can also right-click a name in the Recent list. Each way asks for a file, or uses the one you clicked:

- **Open Read-Only** opens the file for reading only. You can look, scroll, search, and copy, but you can't change anything. The title bar says "Read-Only". If you press **Save**, JeffWrite asks for a new name, so the file you opened is not changed.
- **Open as Copy** opens a new document with the file's text. It has no file of its own, so **Save** asks where to put it. The file you opened is never changed.
- **Open in New Window** opens the file in a window of its own, even when the window you are in is empty.
- **Open and Repair** is for a .docx file that is damaged, for example one that was cut short when it was copied. See below.

## Open and Repair

Sometimes a .docx file is damaged. A copy stopped halfway, or a disk made mistakes. A .docx file is a bundle of smaller parts, such as the text, the styles, and the pictures. **Open and Repair** reads every part that it can. A part that stopped in the middle is kept up to the place where it stopped. A part that can't be read is left out. If the list of parts or the links between them are gone, JeffWrite makes them again.

The repaired document opens as a copy with a name like "Repaired letter". A message tells you what was kept up to a break and what was left out. The damaged file is not changed. Save the repaired copy under a name of its own, and check it before you trust it.

## Get back unsaved work

JeffWrite saves a spare copy of unsaved work every few minutes, as a file in its own folder. If the program closes unexpectedly, it offers those copies the next time you start it. Click **File > Open > Recover Unsaved Work** to see them later. You can choose which to open, and you can save each one where you like.

## Start from a template

**File > New** shows the **Blank Document** and a set of templates that come with JeffWrite, grouped by kind: a letter, a report, a brief, a manuscript, and a newsletter. Each picture is the first page of the document you get. A **template** is a document that is already set up, with its styles, margins, and sample words. Click one, and replace the sample words with your own. Templates you saved yourself are listed under **My Templates**.

## Pick up where you stopped

JeffWrite remembers where the cursor was when you saved or closed a file. The next time you open the file, the cursor is there. Press **Shift+F5** to jump to the places you edited last. Each press goes to the one before: the last three places, then back to the first.

## Save as .odt

An .odt file can't hold everything a .docx file can. When you save as .odt, JeffWrite lists what would be lost first, and lets you save a .docx file instead. See [Info](info).

## Print **Ctrl+P**, or click **File > Print**. Choose a printer, the copies, and the pages, look at each page in the preview, and click **Print**. See [Print](print).

## Make a PDF

Click **File > Export > Export to PDF**. Choose which pages to include: all, the current page, or a range such as 1-3, 5 (p2s3 and s3 work here too). Then choose a name. The PDF looks the same on every computer, because the fonts go inside it. The document's title, author, subject, and keywords go into the PDF. Each heading becomes a **bookmark**, an entry in the list that a PDF reader shows beside the pages. Click a bookmark to jump to its page.

## Save as a web page

Click **File > Export** and choose one of two kinds of web page. A **web page** is a file you can open in any web browser.

- **Save as Web Page** makes one .html file that holds your words, their looks, and your pictures. You can send that one file by itself. The looks it keeps are the font, the size, the colour, the highlight, bold, italic, underline, alignment, and the space around paragraphs.
- **Save as Filtered Web Page** makes a small .html file with only the structure: headings, bold and italic, lists, tables, links, and pictures. It has no fonts, sizes, or colours, so you can give it your own looks later.

Both keep headings, bulleted and numbered lists, tables with joined cells, links, and pictures. Hidden text, the words that a tracked change deleted, comments, headers, footers, and notes are left out. A link that would start a program or run a script is saved as plain words.

## Protect a PDF with a password

In the **Export to PDF** box, click **Encrypt with a password…**. Type the password twice. Anyone who opens the PDF then has to type it. The password really protects the file: the pages are scrambled with strong encryption (AES-256), and nobody can read them without it.

You can also check **Restrict printing, copying, and editing**, give a second password, and choose what is allowed. Those limits are only a request. Most PDF readers honour them, but they don't stop anyone who wants to get around them, so don't count on them to keep a file private.

A forgotten password can't be recovered, so keep it somewhere safe. The password is used for that one PDF and is never saved. The box turns off **PDF/A** while a password is set, because a PDF/A file can't have one.

## Make a PDF/A

In the **Export to PDF** box, tick **PDF/A, for keeping for a long time**. A PDF/A file holds every font it uses and follows a standard made for keeping files for many years. It is not tagged for screen readers yet, and it can't have a password.

## Make an email

Click **File > Export > Make an Email** to make a message file (.eml). Nothing is sent. You choose what goes in the message: the document, the document as a PDF, or the pages shown as pictures in the message (the first 30 pages). Open the file in your email program, add who it is for, and send it. For a PDF, you can click **Encrypt with a password…** in the box; tell the person the password some other way than in the message.

## Related

- [Welcome to JeffWrite](start)
- [Print](print)
- [Use JeffWrite with the keyboard](keyboard)

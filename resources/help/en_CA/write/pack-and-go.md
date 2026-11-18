# Pack and Go: send a document with what it needs
<!-- keywords: pack and go, package, send, share, another computer, zip, folder, fonts, linked files, linked pictures, missing pictures, copy, take with you, USB drive, license, INCLUDEPICTURE, INCLUDETEXT, field, licence -->

When you send a document to someone, the pictures and fonts it uses can get lost on the way. **Pack and Go** saves the document together with what it needs, in one ZIP file or one folder, so it looks the same on another computer.

## How to use it

1. Click **File**, then **Export**.
2. Click **Pack and Go: Save for Another Computer** to make one ZIP file, or **Pack and Go: Save to a Folder** to make a new folder.
3. Choose where to save it. For a ZIP file, type a name. For a folder, choose the folder to put the new one in.
4. Read the message that comes up. It says where the package is, how many linked files and font files went in, and anything that could not be packed.

A **ZIP file** is one file that holds other files, so it is easy to send. The other person opens it to get the files out. A folder is better when you want to copy everything to a drive.

## What goes in the package

- **The document itself**, with the same name it has now. It is a copy. Your own file does not change.
- **Linked pictures.** A linked picture shows a picture from a file instead of holding the picture itself. The package holds a copy of each one.
- **Files named by fields.** A field is a code that fills in its own result. An **INCLUDEPICTURE** field shows a picture from a file, and an **INCLUDETEXT** field shows text from another document. The package holds a copy of each file, and the field now names that copy.
- **Fonts.** A font is the look of the letters. The package holds the font files that come with the program and that the document uses, in a folder named **fonts**. Pack and Go never packs a font whose licence says it may not travel. The message names each font it left out. Fonts installed on your computer by you are not packed, so the other person may need those.

## How the links change

In the copy, every link that was packed points to the file inside the package, by a short name, such as **linked/photo.png**. The links do not use the path on your computer any more, so they work wherever the package is opened. If two files have the same name, the second gets a number added.

A link that Pack and Go can't follow stays as it was, and the message lists it. Pack and Go does not pack:

- A file that can't be found.
- A web address. Those stay as they are.
- A file on another computer on your network.
- A link that goes up and out of the document's folder, such as **../../notes.png**.
- A document, workbook, or other object that is not in the document's own folder (or a folder inside it), even when the link gives its whole path. Only pictures, movies, and sounds are packed from anywhere.
- A link that comes out of the document's folder through a shortcut to another folder (a symbolic link).
- A link to the document itself.
- A file that is not a picture, movie, sound, or document. A program is never packed.

Pack and Go does not look inside the linked files for more links, so a file that links back to this one can't cause a loop.

## Limits

To keep a package from growing too big, Pack and Go asks first when there are more than 200 linked files, or more than 256 MB of them. Choose **No** and nothing is saved. Pack and Go never packs more than 2,000 files, or more than 1 GB, whatever you answer. The files past that are left out and listed in the message.

If you use a password on the document, the copy in the package has the same password.

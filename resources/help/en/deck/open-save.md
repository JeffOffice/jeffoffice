# Open and save presentations
<!-- keywords: open, save, save as, otp, libreoffice, left out, close, new, recent files, pinned, file types, pptx, potx, ppsx, ppt, odp, template, show, rename, Ctrl+S, backup, file formats, old files, open document, kept, not shown -->

JeffDeck saves your work in .pptx files, and in .odp files, and opens several kinds of presentation files.

## Start a new presentation

Press Ctrl+N. A new window opens with a blank presentation. Or click **File > New** and then **Blank Presentation**. If the window you are in holds a new presentation that nobody has touched, the blank one takes its place. Otherwise, it opens in a window of its own.

## Save

Press Ctrl+S, or click **File > Save**. The first time, JeffDeck asks for a name and a place. After that, Save replaces the file with your latest work. A message at the bottom says "Saved" and the file's name.

A star (*) after the name in the title bar means you have changes you have not saved. Save often. If JeffDeck stops by surprise, see [Get back unsaved work](recover).

If a message says some objects "can't be saved in this kind of file yet", the file is saved, but JeffDeck had no way to write those objects. Keep the original file too. When you save an .odp file, the message lists what that kind of file can't hold. The section "Save as an .odp file" below tells what it can't hold.

## Save with a new name or kind

Click **File > Save As**, or press F12. Type a name, pick the kind, and click **Save**. The choices are:

- **Presentation (*.pptx)** is the normal choice.
- **Template (*.potx)** saves the slides as a pattern to start new presentations from.
- **Show (*.ppsx)** saves a presentation to be played.
- **OpenDocument Presentation (*.odp)** saves the slides in the file kind that LibreOffice and other free programs use.
- **OpenDocument Presentation Template (*.otp)** is the template form of that kind.

If you leave off the ending, JeffDeck adds the one that goes with the kind. After a save, the title bar shows the new name, and you keep working in the new file.

A presentation you opened from a template is saved as a new presentation: **Save** asks for a name, so the template stays as it was. The same goes for one opened from an older .ppt, .pps, or .pot file. A presentation you opened from an .odp file is saved as an .odp file again, over the same file.

## Open

Press Ctrl+O to pick a file at once. Or click **File > Open**, and then click **Browse**, or a name under **Pinned** or **Recent Presentations**.

The file opens in this window if it holds a new presentation that nobody has touched. Otherwise, it opens in a window of its own.

Right-click a name in the list to **Pin to list** (it stays at the top), **Unpin from list**, or **Remove from list**. A name in gray means JeffDeck can't find that file now.

## Kinds of files JeffDeck reads

- **.pptx, .potx, and .ppsx** are one format in three forms: a presentation, a template, and a show. JeffDeck reads and writes all three.
- **.ppt, .pps, and .pot** are the older kind. JeffDeck reads them but can't write them, so **Save** asks for a new name and saves a .pptx (a .ppsx for a .pps). It never writes over the original.
- **.odp and .otp** are open document presentations and their templates. JeffDeck reads and writes both. An .odp opened here is saved as an .odp. An .otp is a template, so **Save** asks for a new name, and suggests an .odp.

If part of a file can't be read, JeffDeck opens the rest and lists what it left out. If the whole file can't be read, it says why.

## Save as an .odp file

An .odp file opens in LibreOffice Impress with your slides, masters, pictures, notes, and comments, and with the look that JeffDeck shows. It holds animations, slide transitions, custom shows, and the settings for the slide show too. The font of each word is saved by name, so a computer that doesn't have the font shows another one.

JeffDeck itself doesn't read animations, comments, custom shows, or the show settings back from an .odp file yet. They are in the file, and LibreOffice plays them, but when you open the .odp file in JeffDeck, it lists them as left out. Keep your .pptx file if you want to go on editing them here.

An .odp file has no place for a few things. JeffDeck lists them after the save, in the message at the bottom of the window:

- **Charts, SmartArt diagrams, and equations** are saved as pictures. You can see them, but you can't edit their numbers or parts in the .odp file.
- **Sections** are not saved.
- **Reflections, bevels, and inner shadows** are left out.
- **Animation effects** that start when you click an object, and effects that change a color, are left out. A click on a movie, to pause it or play it again, is kept.
- **A movie or sound that is linked** from another file, not kept in the presentation, is saved as the picture that stands for it. Movies and sounds that are in the presentation are kept in the .odp file and play.

Keep your .pptx file too, if you need these things back.

## What is kept but not shown

Some parts of a file are things JeffDeck can't show or change yet. It keeps them with their slides and writes them back when you save, so they are not lost. They include animations, which play in the show, movies and sounds, which don't play yet, and a transition effect JeffDeck has no form for, which plays as a fade. Charts and SmartArt diagrams show as they were saved. See [Welcome to JeffDeck](start).

## Related

- [Get back unsaved work](recover)
- [The File screen](file-screen)
- [Reuse slides from another presentation](reuse)
- [Print, PDF, and slide pictures](printing)

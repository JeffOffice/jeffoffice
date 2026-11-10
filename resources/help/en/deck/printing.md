# Print, PDF, and slide pictures
<!-- keywords: print, printer, printing, copies, collate, collated, selection, last page first, reverse, zoom, slides, notes pages, handouts, outline, pdf, export, picture, png, jpeg, jpg, image, range, hidden slides, frame, grayscale, black and white, print preview, paper, letter, a4, password, encrypt, protect with a password -->

You can print your presentation, save it as a PDF, or save a slide as a picture. All three are on the File screen.

## Print

1. Press Ctrl+P, or click **File > Print**.
2. Next to the **Print** button, set **Copies**. Choose a **Printer**: the list ends with **Save as PDF**, which makes a PDF file in place of paper. With more than one copy, **Collated** prints the whole set again for each copy (1, 2, 3, then 1, 2, 3), and with it off each page's copies come out together (1, 1, 2, 2, 3, 3). A PDF file is made once, whatever the number of copies.
3. Under **Print what**, choose what comes out: **Slides**, **Notes Pages**, **Outline**, or **Handouts** with 1, 2, 3, 4, 6, or 9 slides per page.
4. For handouts of 4 or more, **Order** says whether the slides run across the page and then down (**Horizontal**) or down and then across (**Vertical**).
5. Under **Range**, choose **All slides**, **Current slide**, or **Selection** (the slides selected in the slide sorter, or the slide you are on when you are not in the sorter), or choose **Slides** and type the numbers in the box: slide numbers and ranges with commas between them, such as 1-3, 5.
6. Check what you need: **Last page first** (the pages come out in reverse order, so a stack comes out in order; it is dimmed when you print on both sides by hand), **Print hidden slides**, **Frame slides** (a thin line around each slide), and **Grayscale**.
7. Under **Sides**, choose **Print on one side**, **Print on both sides, flip on long edge** (pages that turn like a book), or **Print on both sides, flip on short edge** (pages that turn like a notepad), or **Print on both sides by hand**. The first two are dimmed when your printer can't print on both sides. Point at one to see why. **By hand** is for such a printer: JeffDeck prints the first, third, fifth, and so on of the pages, then a message asks you to turn the stack over and put it back in the printer. When you click **OK**, it prints the others, last page first. With one page there is nothing to turn over. JeffDeck remembers your choice.
8. Click **Print**. For a PDF file, JeffDeck asks for a name. A PDF file has no sides.

The preview on the right shows each page that will come out, with arrows to move from page to page. **Zoom** above it makes the page larger than the size that fits; scroll to see the rest. It is drawn the same way the page is printed. JeffDeck remembers your choices for next time.

## What each choice prints

- **Slides** prints one slide on each page, in the slide's own shape. The paper turns sideways for a wide slide.
- **Notes Pages** puts a slide at the top of a page with its speaker notes below it.
- **Handouts** put a grid of slides on each page, with a header and footer that hold the title, the date, and the page number. With 3 to a page, there are lines beside each slide to write on.
- **Outline** prints the number and title of each slide, then its body text, indented by level.

Notes pages, handouts, and the outline print on upright letter or A4 paper, whichever your region uses. Hidden slides are left out unless you check **Print hidden slides**.

## Save as a PDF

Click **File > Export > Export to PDF**. A window asks what to include: the same **Print what**, **Order**, and **Range** choices as printing, and the same three check boxes. It shows how many pages the file will have. Click **Export…**, then choose a name and a place.

The PDF has real text, with the fonts inside it, so it looks the same on every computer. Its title is the presentation's title, or the file's name.

Check **PDF/A, for keeping for a long time** to make a PDF/A-1b file. Every font is inside the file, and transparent effects are flattened, so the slides look the same in years. It is not tagged for screen readers. A PDF with a password can't be a PDF/A file, so choosing a password turns PDF/A off.

## Send a presentation by email

Click **File > Export > Make an Email**. Choose the presentation as a file, or as a PDF. JeffDeck makes a message file (.eml) with it attached and asks where to save it. Nothing is sent. Open the file in your email program, add who it is for, and send it. A presentation that has a password is attached with the password still on it. A PDF can have a password of its own.

## Protect a PDF with a password

In the **Export to PDF** window, click **Encrypt with a password…**. Type the password twice. Anyone who opens the PDF then has to type it. The password really protects the file: the pages are scrambled with strong encryption (AES-256), and nobody can read them without it.

You can also check **Restrict printing, copying, and editing**, give a second password, and choose what is allowed. Those limits are only a request. Most PDF readers honor them, but they don't stop anyone who wants to get around them, so don't count on them to keep a file private.

A forgotten password can't be recovered, so keep it somewhere safe. The password is used for that one PDF and is never saved.

When you insert a PDF that has a password as a picture, JeffDeck asks for the password. If it is wrong, it asks again; click **Cancel** to stop. The slide gets a copy of the page that needs no password. If the PDF's owner asks that it not be copied, JeffDeck places it all the same and shows a note saying so; that request is not a lock.

## Save a slide as a picture

Click **File > Export > Export Slide as Picture**. JeffDeck saves the slide you are on as a PNG or JPEG picture, 1920 pixels across, on a white background. It suggests the presentation's name and the slide's number as the file name.

## Related

- [The File screen](file-screen)
- [Speaker notes](notes)
- [Add, change, and delete slides](slides)

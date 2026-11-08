# PDF files and print shops
<!-- keywords: pdf, create pdf, save as pdf, export pdf, pdf/x, pdf/a, archive, print shop, commercial printer, cmyk, bleed, crop marks, color profile, spot color, overprint, colour profile, spot colour, password, encrypt, protect with a password -->

A **PDF** is a file that looks the same on every computer. Use one to email a publication, post it online, or send it to a print shop.

## Make a PDF

1. Click **File > Export**, then **Create PDF**.
2. Under **Optimize for**, pick a quality. **dpi** means dots per inch. More dots make sharper pictures and bigger files.
3. Under **Pages**, choose **All pages**, **Current page**, or **Pages from** one page **to** another.
4. Click **Save PDF…**, type a name, and click **Save**.

| Choice | Pictures | Best for |
|---|---|---|
| Minimum size | 96 dpi | Viewing on screen |
| Standard | 150 dpi | Sharing online |
| High quality printing | 300 dpi | Desktop printers |
| Commercial press | Full resolution | A print shop. Adds crop marks, bleed marks, registration marks, colour bars, and job information. |

Other choices:

- **Include document properties** puts the title, author, subject, and keywords in the PDF.
- **Open the PDF after saving it** shows the PDF in your viewer right away.
- **Booklet sheets** appears for booklets. It puts two pages on each side in folding order. Clear it for one page per PDF page.

## PDF/A for keeping records

**PDF/A** is a PDF made to last for years. It holds every font and uses standard colours, with no see-through effects. Check **PDF/A for long-term archiving**, or click **File > Export**, then **Create PDF/A for Archiving**.

## PDF/X for a print shop

**PDF/X** is a stricter PDF that print shops ask for. Colours are given as amounts of ink, fonts are inside, and the trim and bleed are marked. The **bleed** is extra picture or colour that runs past the page edge, so trimming leaves no white edge.

1. Click **File > Export**, then **Create PDF/X for a Commercial Printer**. Or check **PDF/X for a commercial printer** in the **Create PDF** window. You can't use it with PDF/A.
2. Under **Printing condition**, pick the one your print shop names.
3. If the shop wants crop marks and a bleed, also pick **Commercial press** under **Optimize for**.
4. Click **Save PDF…**.

The conditions are:

- **PDF/X-1a** for U.S. web offset (SWOP) or European offset (FOGRA39). It flattens see-through effects.
- **PDF/X-4** for premium coated paper (FOGRA51, PSO Coated v3), or with your printer's own colour profile (an .icc file you choose). It keeps see-through effects.

The PSO Coated v3 colour profile does not come with JeffPub. The first time you use it, JeffPub asks before it downloads the profile, once, from the European Colour Initiative.

Before it makes the file, JeffPub checks for pictures under 150 ppi (pixels per inch) at their printed size and for fonts missing from this computer. Click **Create PDF Anyway**, **Open Design Checker**, or **Cancel**.

## Protect a PDF with a password

In the **Create PDF** window, click **Encrypt with a password…**. Type the password twice. Anyone who opens the PDF then has to type it. The password really protects the file: the pages are scrambled with strong encryption (AES-256), and nobody can read them without it.

You can also check **Restrict printing, copying, and editing**, give a second password, and choose what is allowed. Those limits are only a request. Most PDF readers honor them, but they don't stop anyone who wants to get around them, so don't count on them to keep a file private.

A forgotten password can't be recovered, so keep it somewhere safe. The password is used for that one PDF and is never saved. A PDF with a password can't be PDF/A or PDF/X, so those two are turned off while a password is set. On **File > Share**, **Encrypt with a password…** above **Send as PDF** puts a password on the PDF in an email.

When you place a PDF that has a password as a picture, JeffPub asks for the password. If it is wrong, JeffPub asks again; click **Cancel** to stop. The page in your publication is a copy and needs no password. If the PDF's owner asks that it not be copied, JeffPub places it all the same and shows a note saying so; that request is not a lock.

A picture linked to a PDF with a password shows as **Not updated** when you open the publication, and JeffPub never asks for the password by itself. Click **Update Link** to be asked.

## Set up colours for a print shop

Click **File > Properties**, then the **Commercial Print Settings** card:

- **Colour model** is **RGB**, **Single colour (spot)**, **Spot colours**, **Process colours (CMYK)**, or **Process plus spot colours**. Process colours are cyan, magenta, yellow, and black ink. A spot colour is a special ink the shop mixes. Add each one under **Spot colours**.
- **Overprint black in files for a printer (PDF/X)** makes black print over the colours beneath it, so a shifted plate leaves no white edge. You choose what it covers: text below a size, lines, or fills.

## Related

- [Print a publication](printing)
- [Save as a picture, web page, or e-book](export)
- [Check your design](design-checker)

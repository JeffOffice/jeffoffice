# JeffOffice

> **JeffOffice is an independent project. It is not affiliated with, endorsed by, sponsored by, or supported by Microsoft Corporation.** See [Legal notices](#legal-notices).

JeffOffice is a free, open-source office suite for **Windows, macOS, and Linux**. It has two apps today:

- **JeffPub**, for desktop publishing: flyers, newsletters, brochures, business cards, certificates, and other print pieces. It opens and saves Microsoft Publisher files.
- **JeffDeck** (a preview), for presentations: it makes slides, plays slide shows, and opens PowerPoint files (`.pptx` and the older `.ppt`) and OpenDocument presentations (`.odp`), and saves `.pptx` and `.odp`.

More apps are on the way: JeffWrite for documents, JeffCrunch for spreadsheets, JeffMail for email and calendars, and JeffBase for databases. Each will come in an update once it's ready.

> **Status:** beta (0.7). JeffPub's features are in place and are being polished; JeffDeck is a preview, with the main features working and more still being built (see [JeffDeck](#jeffdeck)). Downloads are on the [Releases page](https://github.com/JeffOffice/jeffoffice/releases):
>
> - **Windows:** `JeffOffice-Setup.exe` installs JeffPub and JeffDeck (you can leave JeffDeck out). No administrator rights are needed.
> - **Mac:** `JeffOffice-<version>.dmg` holds both apps. It isn't signed yet, so macOS asks before opening each one the first time.
> - **Linux:** `jeffoffice_<version>_amd64.deb` installs both apps on Debian, Ubuntu, and their relatives (`sudo apt install ./jeffoffice_<version>_amd64.deb`; it replaces an earlier `jeffpub` package). For most other distributions there's an AppImage of each app: make the file executable and run it.

## JeffPub

### Why JeffPub exists

Microsoft is retiring Publisher. Microsoft 365 subscribers lost access to Publisher on October 1, 2026, and support for the last standalone version ends on October 13, 2026 ([Microsoft's announcement](https://support.microsoft.com/en-us/publisher/microsoft-publisher-will-no-longer-be-supported-after-october-2026)). Microsoft suggests converting existing publications to PDF or Word before then.

JeffPub was started in response. It gives the people, small businesses, schools, and publishers who built years of work in Publisher a free, open-source program that keeps opening, editing, and printing those `.pub` files, on Windows, macOS, and Linux, without a subscription and without depending on any one company's product plans.

### What it does

- **Page layout.** Free-form frames on a page: text boxes, pictures, shapes, tables, and Text Art headings. You can move, resize, rotate, group, align, and layer them, and they snap to margins and guides.
- **Linked text boxes.** A long story flows from one text box to the next, across columns and pages, the way newsletters need.
- **Master pages** for headers, footers, and page numbers that repeat on every page.
- **Color and font schemes.** Change one scheme and the whole publication recolors or changes fonts.
- **Templates and building blocks**, including flyers, newsletters, brochures, cards, calendars, and certificates.
- **Mail merge** from a CSV list.
- **Design Checker**, which finds text that doesn't fit its box, objects that run off the page, and low-resolution pictures.
- **Print and export** to PDF and PNG.
- Rulers, guides, a pages pane, zoom, and a two-page spread view.
- **Microsoft Publisher files.** Opens `.pub` files from every version of Publisher since Publisher 98, and saves publications back to `.pub` so they can be shared with Publisher users. JeffPub also has its own open format, `.jpub`.

Every feature and where it stands is listed in [FEATURES.md](FEATURES.md).

## JeffDeck

JeffDeck makes presentations and plays them as slide shows. It's a **preview**: the main features work, and some are still being built.

### What it does

- **Slides** with layouts and slide masters, backgrounds, slide size, headers and footers, sections, and speaker notes. Work in Normal view, the Outline, or the Slide Sorter, and reuse slides from another presentation.
- **On a slide:** text, shapes, pictures, tables, Text Art, and charts (drawn from the numbers saved with them).
- **Slide shows** full screen or with Presenter View on a second screen (your notes, the next slide, and a timer), with transitions between slides and animations that play click by click. A slide can move on by itself after the seconds you set, and Presenter View can be practiced in a window on one screen.
- **Movies and sounds** put on a slide (Insert > Media) play in place while you edit, with a control bar, and in the slide show as the file says: at a click, by themselves, or when clicked on, with their volume, trim, looping, and full screen. The Playback tab sets them.
- **Comments** on slides, with replies, which other presentation programs show too.
- **Print and export** slides, notes pages, handouts, and the outline, to paper or PDF, and slides as pictures.
- **Presentation files.** Opens `.pptx`, `.potx`, `.ppsx`, and the older `.ppt` files from Microsoft PowerPoint, and OpenDocument presentations (`.odp`, `.otp`); saves `.pptx`, `.potx`, `.ppsx`, `.odp`, and `.otp`. What it can't show yet, such as SmartArt diagrams, is kept and saved back.

### Not yet in the preview

Editing animations (the ones in a file play, but can't be changed yet), changing themes and their color variants, and SmartArt diagrams and equations (kept, but not shown; an `.odp` holds them as pictures). Help covers what JeffDeck does today, and [docs/jeffdeck.md](docs/jeffdeck.md) lists each feature and where it stands.

## Privacy

JeffPub and JeffDeck each check GitHub for new versions when they start (you can turn that off in File > Options, for both); on Windows, the update installs the whole suite, and elsewhere it opens the release page. With both open, one of them tells you about a new version each day.

Online Pictures (in JeffPub, Insert > Online Pictures) sends the words you search for to the library you choose, Openverse or Wikimedia Commons, and nothing else.

Unless you turn them off, JeffPub and JeffDeck each send anonymous usage statistics once a day while in use (and once more the day they're updated): a random number made up when JeffOffice was installed (it says nothing about you or your computer, and both apps send the same one), which app it is, its version, your operating system and its version, the language it's set to, how many times it was started, and how many times each of its commands was used. JeffDeck also says which kinds of files it opened and saved (for example, `.pptx`). They never send your files, their names, or anything in them, and the server doesn't keep IP addresses. It's one choice for all the JeffOffice apps: you make it in Windows setup or the first time one of them starts (the other doesn't ask again), and can change it anytime in File > Options in either. The collector's code is in [server/telemetry](server/telemetry).

If usage statistics are on and JeffPub or JeffDeck crashes, it also sends a crash report the next time it starts: which app it was, where in the program it stopped (as program file names and numeric positions), what kind of failure it was, the day it happened, its version, your operating system, and the same random number. It never sends your documents, their names or contents, anything you typed, or anything from your computer's memory. If usage statistics are off, no crash report is sent, and the one the crash left is deleted.

## Help

Help is built into each app (press F1). JeffPub's is also published to the project's [wiki](https://github.com/JeffOffice/jeffoffice/wiki). The topics are the Markdown files in [resources/help/en](resources/help/en) (JeffDeck's in its `deck` folder). Edit them there, not on the wiki: every push to `main` replaces the wiki's pages with JeffPub's. See [docs/help-wiki.md](docs/help-wiki.md).

## Languages

Both apps are in American English, German, Spanish, French, Italian, Dutch, and Portuguese (Brazil and Portugal). They follow your computer's language, or the one chosen in File > Options > General. The translations are in [translations](translations/README.md).

## Built with

- **C++17** and **Qt 6** (Widgets and Graphics View), built with **CMake**
- Qt provides the cross-platform UI, the rich-text layout engine, PDF output, and native printing on Windows, macOS, and Linux.

## License

Copyright © 2026 JeffOffice LLC.

JeffOffice, with JeffPub and JeffDeck, is free software under the [GNU General Public License v3.0](LICENSE). You may use, study, change, and share it. If you distribute a modified version, you must share its source under the same license.

## Legal notices

**No affiliation with Microsoft.** JeffOffice and its apps, JeffPub and JeffDeck, are independent, community-made programs. They are not Microsoft products. It is not affiliated with, authorized, sponsored, endorsed, licensed, or supported by Microsoft Corporation or any of its subsidiaries. Microsoft has not reviewed or approved this project. Do not contact Microsoft for help with JeffOffice, and do not contact this project for help with Microsoft products.

**Trademarks.** Microsoft, Microsoft Publisher, Microsoft PowerPoint, Microsoft 365, Office, Windows, Arial, Times New Roman, Calibri, Georgia, and the other Microsoft product and font names mentioned in this project are trademarks or registered trademarks of Microsoft Corporation or of their respective owners, in the United States and other countries. Qt is a trademark of The Qt Company Ltd. Linux is a registered trademark of Linus Torvalds. All other trademarks belong to their owners. This project uses these names only to describe compatibility, for example that JeffPub can open files created by Microsoft Publisher and JeffDeck files created by Microsoft PowerPoint. That use does not imply any relationship with, or endorsement by, the trademark owners. "JeffOffice", "JeffPub", and "JeffDeck" are names of this project and are not Microsoft names or products.

**No Microsoft code or artwork.** JeffOffice contains no source code, program files, icons, logos, templates, clip art, or fonts from Microsoft. Its apps use their own names, design, and the open-source Lucide icon set. Its templates and sample text are original; the sample business details in them are fictitious. Where a publication asks for a Microsoft font that isn't installed, JeffPub substitutes a freely licensed font with similar metrics, and gives each letter the missing font's own width and pair kerning (measured numbers in `resources/fontwidths.txt` and `resources/fontkerning.txt`, not font data) so lines break where they do in Microsoft Publisher; it does not ship Microsoft fonts.

**File compatibility.** JeffDeck reads and writes presentation files from the published Office Open XML (ECMA-376) and OpenDocument standards and Microsoft's published Open Specifications for the older `.ppt` format. Support for reading `.pub` files is built on the open-source [libmspub](https://wiki.documentfoundation.org/DLP/Libraries/libmspub) and [librevenge](https://sourceforge.net/p/libwpd/librevenge/) libraries, extended through independent analysis of publications the project's contributors created or own. This interoperability work is not based on any confidential Microsoft information. Converting files between programs can change how they look. Always check converted files, and keep your originals.

**No warranty.** JeffOffice is provided "as is", without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, and non-infringement. The authors and contributors are not liable for any claim, damages, data loss, or other liability arising from the use of this software. See sections 15 and 16 of the [GPL](LICENSE) for the full terms.

**Your content.** You are responsible for the text, pictures, and other material you put into your publications and presentations, and for having the rights to use them. Links to online picture libraries are provided for convenience; check each picture's license before you use it.

## Third-party software

JeffOffice includes or downloads these components, each under its own license:

| Component | Use | License |
|---|---|---|
| [Qt 6](https://www.qt.io/) | User interface, text layout, PDF, and printing | LGPL-3.0 |
| [libmspub](https://wiki.documentfoundation.org/DLP/Libraries/libmspub) (modified) | Reading `.pub` files | MPL-2.0 |
| [librevenge](https://sourceforge.net/p/libwpd/librevenge/) (modified) | Document interfaces and OLE2 streams | MPL-2.0 or LGPL-2.1+ |
| [Hunspell](https://hunspell.github.io/) | Spelling checker | MPL-1.1, GPL-2.0+, or LGPL-2.1+ |
| English dictionaries: US, Canada, and Australia ([SCOWL](http://wordlist.aspell.net/)), United Kingdom | Spelling | SCOWL license; LGPL (United Kingdom) |
| English hyphenation patterns (American and British) | Hyphenation | BSD-style |
| [WordNet](https://wordnet.princeton.edu/) thesaurus | Synonyms | WordNet license |
| Spanish dictionaries (Spain, Mexico) and hyphenation patterns | Spelling, hyphenation | GPL-3.0+, LGPL-3.0+, or MPL-1.1+ |
| French dictionary ([Grammalecte](https://grammalecte.net/)) and hyphenation patterns | Spelling, hyphenation | MPL-2.0; patterns LGPL |
| German dictionary ([igerman98](https://www.j3e.de/ispell/igerman98/)) and hyphenation patterns | Spelling, hyphenation | GPL-2.0 or GPL-3.0; patterns LGPL-2.0+ |
| Italian dictionary and hyphenation patterns | Spelling, hyphenation | GPL-3.0; patterns LGPL |
| Dutch dictionary and hyphenation patterns ([OpenTaal](https://www.opentaal.org/)) | Spelling, hyphenation | BSD (revised) or CC BY 3.0 |
| Brazilian Portuguese dictionary and hyphenation patterns ([VERO](http://pt-br.libreoffice.org/projetos/projeto-vero-verificador-ortografico/)) | Spelling, hyphenation | LGPL-3.0 or MPL |
| European Portuguese dictionary and hyphenation patterns | Spelling, hyphenation | GPL-2.0+ |
| [FFmpeg](https://ffmpeg.org/) 7.1 libraries, as built for Qt Multimedia (JeffDeck's packages only) | Playing the movies and sounds in JeffDeck | LGPL-2.1+ |
| [Lucide](https://lucide.dev/) icons and their search keywords | Interface icons and Insert > Icons | ISC |
| Open fonts | Fonts and substitutes | SIL Open Font License, GUST Font License, Bitstream Vera, GPL v2 with font exceptions (Liberation Sans Narrow); see `resources/fonts/README.md` |
| [zlib](https://zlib.net/) | Compression (when the system has none) | zlib license |
| [PDFium](https://pdfium.googlesource.com/pdfium/), built by [pdfium-binaries](https://github.com/bblanchon/pdfium-binaries) | Placing PDF pages | BSD-3-Clause, parts Apache-2.0; the build MIT |
| Built into PDFium: [FreeType](https://freetype.org/), [HarfBuzz](https://harfbuzz.github.io/), [ICU](https://icu.unicode.org/), [Little CMS](https://www.littlecms.com/), [libjpeg-turbo](https://libjpeg-turbo.org/), [OpenJPEG](https://www.openjpeg.org/), [libpng](http://www.libpng.org/pub/png/libpng.html), [zlib](https://zlib.net/), [Abseil](https://abseil.io/), Anti-Grain Geometry 2.3, [dragonbox](https://github.com/jk-jeon/dragonbox), [fast_float](https://github.com/fastfloat/fast_float), [simdutf](https://github.com/simdutf/simdutf), [LLVM libc](https://libc.llvm.org/) | Fonts, text, color, and pictures in PDF pages | FreeType License; Old MIT; Unicode License v3; MIT; IJG and BSD-3-Clause; BSD-2-Clause; PNG Reference Library License v2; zlib; Apache-2.0; AGG 2.3 license; Apache-2.0 with LLVM Exceptions or BSL-1.0; MIT; MIT; Apache-2.0 with LLVM Exceptions |
| [MinGW-w64](https://www.mingw-w64.org/) runtime (Windows) | C++ runtime DLLs | GPL-3.0 with the [GCC Runtime Library Exception](https://www.gnu.org/licenses/gcc-exception-3.1.html); winpthreads MIT |
| [NSIS](https://nsis.sourceforge.io/) (Windows) | Installer | zlib/libpng license |
| [ICU](https://icu.unicode.org/) (in PDFium; with Qt in the Linux packages) | Unicode text support | Unicode License v3 |
| [LibreTranslate](https://libretranslate.com/) | Translate opens it in the browser; not bundled | AGPL-3.0 |
| [PSO Coated v3](https://www.eci.org/) color profile, from the European Color Initiative | The printing condition in PDF/X-4 files; downloaded from ECI the first time you make one (after asking); not bundled | ECI's terms: free to use and embed in PDFs, not to pass on |
| [International ISBN Agency](https://www.isbn-international.org/range_file_generation) range table | Placing the hyphens in ISBNs from every country; downloaded from the agency when you make an ISBN barcode, monthly at most; not bundled | The agency's terms (it doesn't allow republishing the table) |

The dictionaries come unchanged from the [LibreOffice dictionaries project](https://github.com/LibreOffice/dictionaries); each language's authors, license, and license text are in [resources/dict](resources/dict/README.md). Changes made to libmspub and librevenge are listed in [third_party/README.md](third_party/README.md). PDFium's license texts, and those of the libraries built into it, are in [third_party/pdfium/licenses](third_party/pdfium/licenses) and ship with the program (in its `licenses/pdfium` folder; on Linux in `/usr/share/doc/jeffoffice/pdfium`). The same goes for FFmpeg, which JeffDeck's packages carry for its movies and sounds: its notice (what it is and where its source is) and the LGPL-2.1 text are in [third_party/ffmpeg/licenses](third_party/ffmpeg/licenses), and ship in `licenses/ffmpeg` (on Linux in `/usr/share/doc/jeffoffice/ffmpeg`).

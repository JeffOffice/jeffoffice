# JeffOffice feature lists

Each JeffOffice app has a feature list: a checklist of what the program it is modeled on can do, laid out the way that program is (File, then each ribbon tab, then what sits under the ribbon), so the work can be planned, tracked, and judged. A line starting `[x]` is a feature that works today, `[~]` is one that is partly there (the note says what is missing, or what could not be confirmed), and `[ ]` is one that is not there yet. For an app that is not built yet, `[x]` and `[~]` count what the shared parts of the suite already do for it. Every item also carries a tier: **1.0** for what must work before the app is called finished (for JeffPub, before the beta label comes off, and for JeffDeck, the preview label), and **later** for what few documents use or what needs a piece the suite doesn't have yet; the lists for apps not built yet add **first release** for the first public build. The legend, the tiers, and how a count of documents is read are explained in full at the top of [JeffDeck's list](jeffdeck.md), the closest case of an app that already exists.

| App | What it does | State | Done | Partly | Not yet | List |
|---|---|---|---:|---:|---:|---|
| JeffPub | Desktop publishing for flyers, newsletters, brochures, cards, and other print pieces; opens and saves Publisher's `.pub` files | beta (0.8.0) | 191 | 10 | 48 | [jeffpub.md](jeffpub.md) |
| JeffDeck | Presentations and slide shows; saves `.pptx` and `.odp`, and opens `.ppt` too | preview (0.8.0) | 161 | 48 | 243 | [jeffdeck.md](jeffdeck.md) |
| JeffWrite | Documents and reports; saves `.docx`, and opens `.doc`, `.rtf`, and `.odt` | not built yet | 4 | 144 | 223 | [jeffwrite.md](jeffwrite.md) |
| JeffCrunch | Spreadsheets and calculation; saves `.xlsx`, and opens `.xls`, `.ods`, and `.csv` | not built yet | 2 | 84 | 391 | [jeffcrunch.md](jeffcrunch.md) |
| JeffMail | Email, calendars, contacts, and tasks, kept where the provider keeps them | not built yet | 1 | 31 | 366 | [jeffmail.md](jeffmail.md) |
| JeffBase | Databases for lists and records; keeps its data in SQLite, and opens `.accdb` and `.mdb` | not built yet | 1 | 31 | 290 | [jeffbase.md](jeffbase.md) |

The counts are tallies of each file's checklist lines (`[x]`, `[~]`, and `[ ]`), so they move whenever a list is edited.

## How the lists were ranked, and kept current

An item is ranked by how many real documents use it, where we have the counts. They come from our own collection (for example, 2,082 documents for JeffWrite, 1,481 workbooks for JeffCrunch, and 54 presentations for JeffDeck) and from public collections (U.S. government `.doc` files, and three public spreadsheet collections: business models from the Enron case, school and lab files, and files from government sites). Where a count is missing, the list says so and ranks by what someone moving from the original needs on the first day, then what a full replacement needs. That is the case for JeffMail and JeffBase, which have no usage counts, and for JeffPub, whose 284 real publications have been used to measure fidelity but not yet to count features.

Each list says near its top when it was last reviewed. When a feature lands, change its box in that list and move the date, and update the counts in the table above, so that a list never claims more or less than the program does.

#pragma once
// The table of contents: its lines worked out from the headings (or the styles and levels a code names), and the field's result made again from them. Insert Table of Contents,
// Update Table, and Remove Table are here; F9 on a table is core/fieldops.h's, which calls the two functions at the top.

#include "core/fieldcode.h"
#include "core/fieldops.h"

#include <functional>

namespace jp::write {

// One line of a table of contents: a heading (or other paragraph a code asks for), where it is, and the page it is on.
struct TocLine {
    int level = 1;               // 1 to 9
    QString number;              // the paragraph's number as a list gives it ("2.1"), or empty
    QString text;                // the paragraph's text
    int position = 0;            // where the paragraph begins in the story
    QString bookmark;            // the bookmark that its entry links to (a _Toc name the paragraph has, or one that is to be made)
    bool newBookmark = false;
    QString page;                // the page number as it shows (empty where the pages are not laid out)
    bool noPage = false;         // an entry that a table entry field marks with \n: no page number
};

// What a page lookup gives: the page number as it shows, for the place in the story.
using PageLookup = std::function<QString(int position)>;

// The lines the table of contents with this code is made of, from the paragraphs of the story outside it (at most kMostTocEntries). `bookmarks` is the story's (the _Toc bookmarks the
// headings already have are the entries' targets).
// `truncated`, if given, says whether there were more lines than that.
QVector<TocLine> tocLines(WriteDoc &doc, const StoryScan &scan, int field, const fields::Code &code, const PageLookup &pageOf, bool *truncated = nullptr);
// Puts a hidden _Toc bookmark over the text of the paragraph that begins at each of these places (the last first, so the places stay), as part of the edit being made.
void putTocBookmarks(QTextDocument *story, const StoryScan &scan, const QVector<int> &paragraphPositions);

// The lines as text, one to a paragraph: the number, a tab, the text, a tab, and the page; for comparing with the result a table has (and for a field that is not rebuilt).
QStringList tocLineTexts(const QVector<TocLine> &lines, const fields::Code &code);

// Makes the result of the table at `field` from these lines: the paragraphs between its separator and its end (the table's style per level, its right-aligned page numbers with the leader
// the code or the settings ask for, links to _Toc bookmarks, which are made in the headings that lack them). Done inside the edit block the caller opened (the page numbers alone are those of the PAGEREF fields in it, which an update of the fields in the
// table gives). Returns false when the table could not be rebuilt (its marks are not as a table's are).
bool rebuildToc(WriteDoc &doc, QTextDocument *story, const StoryScan &scan, int field, const fields::Code &code, const QVector<TocLine> &lines, const QString &leader = QString());

// ---- other fields whose result is paragraphs (an index, a table of authorities, a bibliography)
// A piece of a line, in the look the line's style gives, bold or italic where it says.
struct GenRun {
    QString text;
    bool bold = false, italic = false;
};
// A paragraph of such a result: the style the paragraph is in (by the name a document gives it, "index 1": `styleName`, `styleId` the built-in one it is added as), its pieces (a tab
// is a tab), and whether a right-aligned tab stop with the table's leader is put at the margin for a tab.
struct GenLine {
    QString styleName, styleId;
    QVector<GenRun> runs;
    bool rightTab = false;
    QString text() const;
};
// Makes the result of the field at `field` from these paragraphs (as rebuildToc does for a table of contents): the paragraphs between its separator and its end. Where there are none, `empty`
// is the one line of text. Done inside the edit block the caller opened. Returns false when the field's marks are not as a complex field's are.
bool rebuildGenerated(WriteDoc &doc, QTextDocument *story, const StoryScan &scan, int field, const QVector<GenLine> &lines, const QString &leaderAsked, const QString &empty);
// What a page lookup gives for a place: the page number as it shows and the page's place in the document (from 1; 0 where the pages are not laid out).
struct PageRef {
    QString text;
    int index = 0;
};
using PageRefLookup = std::function<PageRef(int position)>;
// The tab leader of the result of the field at `field` (dot, hyphen, underscore, or none), as its paragraphs have it; dot where they have no right tab.
QString resultLeader(QTextDocument *story, const StoryScan &scan, int field);

// What the Table of Contents dialog asks for.
struct TocOptions {
    int levels = 3;              // the heading levels shown, 1 to 9
    bool pageNumbers = true;
    bool rightAlign = true;      // page numbers at the right margin (else after the entry)
    QString leader = QStringLiteral("dot");
    bool hyperlinks = true;
    bool entryFields = false;    // lines also for the entries marked with table entry fields (TOC \f)
};
// The code a table with these options has.
QString tocCode(const TocOptions &options);
// Puts a table of contents, built now, at the caret (in a paragraph of its own), as one step of the history. False where it can't be put (a header, a footer, or a note).
bool insertTableOfContents(WriteDoc &doc, QTextCursor &c, const TocOptions &options, bool *truncated = nullptr);
// Puts a table entry field (TC) at the caret, for a table of contents whose code names the same identifier (TOC \f): `text` is the entry (the selected text where it is empty), `level` 1 to
// 9, `identifier` the letter that says which table it is for (empty: the entry has none, as a table without one asks for), and `noPage` leaves the page number out. One step of the history.
bool markTocEntry(WriteDoc &doc, QTextCursor &c, const QString &text, int level, const QString &identifier = QString(), bool noPage = false);
// Makes the paragraphs the selection touches headings of this level (the style "Heading 1" to "Heading 9"), so that a table built from headings has them; level 0 makes them body text
// again (the document's default paragraph style). One step of the history.
bool addTextToToc(WriteDoc &doc, QTextCursor &c, int level);
// The gallery's tables: a heading (in the style "TOC Heading") and a table built now ("Automatic Table 1" is headed "Contents", "Automatic Table 2" "Table of Contents"), or "Manual Table", which
// is typed text in the table's styles that a person writes over and that no field keeps. One step of the history. False where it can't be put.
enum class TocGallery { Automatic1, Automatic2, Manual };
bool insertTocGallery(WriteDoc &doc, QTextCursor &c, TocGallery kind);
// The same for a table made by the code it is given (a table of figures: TOC \h \z \c "Figure"), at the caret, in a paragraph of its own.
bool insertTableFromCode(WriteDoc &doc, QTextCursor &c, const QString &code, const QString &leader = QString(), bool *truncated = nullptr);
// Takes the table the caret is in away (its lines, its marks, and the content control the gallery's tables have); false where the caret is in none.
bool removeTableOfContents(WriteDoc &doc, const QTextCursor &c);
// Updates the table the caret is in (or the tables a selection touches): the page numbers only, or the whole table. Returns how many tables there were.
int updateTableOfContents(WriteDoc &doc, const QTextCursor &c, bool pageNumbersOnly);

} // namespace jp::write

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

// What the Table of Contents dialog asks for.
struct TocOptions {
    int levels = 3;              // the heading levels shown, 1 to 9
    bool pageNumbers = true;
    bool rightAlign = true;      // page numbers at the right margin (else after the entry)
    QString leader = QStringLiteral("dot");
    bool hyperlinks = true;
};
// The code a table with these options has.
QString tocCode(const TocOptions &options);
// Puts a table of contents, built now, at the caret (in a paragraph of its own), as one step of the history. False where it can't be put (a header, a footer, or a note).
bool insertTableOfContents(WriteDoc &doc, QTextCursor &c, const TocOptions &options, bool *truncated = nullptr);
// Takes the table the caret is in away (its lines, its marks, and the content control the gallery's tables have); false where the caret is in none.
bool removeTableOfContents(WriteDoc &doc, const QTextCursor &c);
// Updates the table the caret is in (or the tables a selection touches): the page numbers only, or the whole table. Returns how many tables there were.
int updateTableOfContents(WriteDoc &doc, const QTextCursor &c, bool pageNumbersOnly);

} // namespace jp::write

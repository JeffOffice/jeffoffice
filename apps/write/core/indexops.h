#pragma once
// The index and the table of authorities: Mark Entry and Mark Citation put the hidden fields XE and TA at the words, and Insert Index and Insert Table of Authorities put an INDEX or a TOA
// field whose result is the paragraphs worked out from them (updated as a table of contents is, core/fieldops.h calls the two functions at the top).

#include "core/fieldcode.h"
#include "core/fieldops.h"
#include "core/tocops.h"

namespace jp::write {

constexpr int kMostIndexEntries = 50000;

// The paragraphs of the index with this code: the entries marked with XE fields outside it (of the code's type \f, inside its bookmark \b), sorted, in the styles index 1 to index 9, with the page
// numbers (and the cross-references, and the bold and italic ones the marks ask for) after each entry, and the letter headings \h asks for. Empty where nothing is marked.
QVector<GenLine> indexLines(WriteDoc &doc, const StoryScan &scan, int field, const fields::Code &code, const PageRefLookup &pageOf);
// The paragraphs of the table of authorities with this code: the citations marked with TA fields, by category, each with its pages (or "passim").
QVector<GenLine> authoritiesLines(WriteDoc &doc, const StoryScan &scan, int field, const fields::Code &code, const PageRefLookup &pageOf);
// The one line shown where nothing is marked.
QString noIndexEntries();
QString noAuthorities();

// What the Mark Entry dialog asks for.
struct IndexMark {
    QString main, sub;             // the main entry and its subentry (a colon in either goes one level deeper)
    enum Page { Current, Range, CrossReference } page = Current;
    QString bookmark;              // for Range: the bookmark whose pages the entry gives
    QString crossReference;        // for CrossReference: the text ("See ...")
    bool bold = false, italic = false;   // the look of the page number
    QString type;                  // the identifier that says which index the entry is for (empty: the entry has none)
};
// The code of the XE field a mark makes.
QString indexMarkCode(const IndexMark &mark);
// Puts the XE field at the caret (after the selection), as one step. False where it can't be put or the entry is empty.
bool markIndexEntry(WriteDoc &doc, QTextCursor &c, const IndexMark &mark);
// The same after each place in the text where the words of the entry's main text stand (those with the same upper and lower case), outside fields; returns how many places were marked.
int markAllIndexEntries(WriteDoc &doc, const IndexMark &mark);
// The entries (XE codes' entry texts) the text has marked, as the lists show them.
QStringList markedIndexEntries(WriteDoc &doc);

struct IndexOptions {
    int columns = 2;               // 1 to 4 (the index's columns: kept in the code as \c)
    bool rightAlign = true;
    QString leader = QStringLiteral("dot");
    bool runIn = false;            // the subentries run on after the entry (else one to a line, indented)
    bool headings = true;          // a letter above each group
    QString type;
};
QString indexCode(const IndexOptions &options);
// Puts an index, built now, at the caret (in a paragraph of its own), as one step. False where it can't be put (a header, a footer, or a note).
bool insertIndex(WriteDoc &doc, QTextCursor &c, const IndexOptions &options);

// What the Mark Citation dialog asks for.
struct AuthorityMark {
    QString longCitation, shortCitation;
    int category = 1;              // 1 to 16 (see authorityCategoryName)
    bool bold = false, italic = false;   // the look of the page numbers
};
QString authorityCategoryName(int category);
QString authorityMarkCode(const AuthorityMark &mark);
bool markAuthority(WriteDoc &doc, QTextCursor &c, const AuthorityMark &mark);

struct AuthoritiesOptions {
    int category = 0;              // 0: every category that has citations
    bool passim = true;            // five pages or more are "passim"
    bool headings = true;          // the category's name above its citations
    bool rightAlign = true;
    QString leader = QStringLiteral("dot");
};
QString authoritiesCode(const AuthoritiesOptions &options);
bool insertTableOfAuthorities(WriteDoc &doc, QTextCursor &c, const AuthoritiesOptions &options);
// Updates the indexes and tables of authorities the selection touches (or those the caret is in); returns how many there were. A table of authorities or index asks for no more than this.
int updateIndexOrAuthorities(WriteDoc &doc, const QTextCursor &c, bool authorities);
// Takes away the index or table of authorities the caret is in; false where the caret is in none.
bool removeIndexOrAuthorities(WriteDoc &doc, const QTextCursor &c, bool authorities);

} // namespace jp::write

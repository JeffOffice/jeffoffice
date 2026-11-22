#pragma once
// The interface fields and notes read their publication through (page numbers, business information, mail merge, story text).

#include <QDateTime>
#include <QList>
#include <QString>
#include <QStringList>
#include <algorithm>
#include <limits>

class QTextDocument;

namespace jp {
namespace fields { struct Properties; }

// What fields and notes read from the publication they're in: its page
// numbering, business information, mail merge, and the text of its stories
// (a note's number counts the notes in every story). The publication
// answers; the text engine never needs the rest of it.
class FieldSource {
public:
    virtual ~FieldSource() = default;
    virtual int firstPageNumber() const = 0;
    virtual QString pageNumberFormat() const = 0;
    // The number a page shows (1-based page of the publication) and its format: the first page number counted on, and the format of the whole, unless a publication has sections that count again.
    virtual int pageNumberShown(int page) const { return int(std::clamp<qint64>(qint64(page) + firstPageNumber() - 1, std::numeric_limits<int>::min() + 1, std::numeric_limits<int>::max())); }
    virtual QString pageNumberFormatOf(int page) const { Q_UNUSED(page) return pageNumberFormat(); }
    // For a word processor's page numbers that carry a chapter's ("2-3"): what comes before the number of a page, the chapter's number and its separator; none for a publication.
    virtual QString pageNumberPrefix() const { return QString(); }
    virtual QString businessField(const QString &key) const = 0;
    virtual QStringList mergeFields() const = 0;
    virtual QString mergeValue(int record, const QString &field) const = 0;
    virtual const QTextDocument *storyDoc(const QString &id) const = 0;
    virtual QList<const QTextDocument *> storyDocs() const = 0;   // every story, in the publication's order
    // For a word processor's page numbers: how many pages the whole document has (NUMPAGES), and how many the section of the page has
    // (SECTIONPAGES), where the pages drawn are made one at a time; 0 for a publication, whose context counts them.
    virtual int pageTotal() const { return 0; }
    virtual int sectionPageTotal() const { return 0; }
    // For a word processor's notes where a page of its document is laid out as a story of its own: the footnotes and endnotes before the page's text (the
    // numbers its own go on from), the footnotes cut at the foot of the page before (each "story|number|from"), and the height of the paragraph that
    // separates the notes from the text (0: a publication's).
    virtual int footnotesBefore() const { return 0; }
    // How a note's number is shown (a word processor numbers its notes in a format of its own: 1, i, a, symbols...), and whether the numbers begin again on each page.
    virtual QString noteMarkText(bool endnote, int number) const { Q_UNUSED(endnote); return QString::number(number); }
    virtual bool restartNotesEachPage() const { return false; }
    // Where a word processor's footnotes are set: right after the last line of the text of their column (not at the foot of the page), and in how many columns.
    virtual bool footnotesBeneathText() const { return false; }
    virtual int footnoteColumns() const { return 1; }
    virtual int endnotesBefore() const { return 0; }
    virtual QStringList carriedNotes() const { return {}; }
    virtual double noteSeparatorHeight() const { return 0; }
    // The height of the file's continuation notice, which a word processor keeps room for at the foot of a page's notes.
    virtual double noteNoticeHeight() const { return 0; }
    // The height of the file's own separator for the part of a note that goes on from the page before (0: the file has none, and the separator's height is kept).
    virtual double noteContinuationHeight() const { return 0; }
    // The stories of the file's continuation notice (under the last note of a page that goes on) and of its own separator for the part of a note that goes on (above that
    // part), for the pages to set where they are wanted; none for a publication. And where the rule is in the separator for the part that goes on: its height from the
    // story's top (-1: three fifths of the way down the paragraph that holds it) and how far from the left (after the words that are before it in its line).
    virtual const QTextDocument *noteNoticeStory() const { return nullptr; }
    virtual const QTextDocument *noteContinuationStory() const { return nullptr; }
    virtual double noteContinuationRuleAt() const { return -1; }
    virtual double noteContinuationRuleFrom() const { return 0; }
    virtual bool noteContinuationRuleFull() const { return true; }   // (the rule is as wide as the column, not two inches)
    // For a word processor's running headers: what a field that shows a paragraph style's text (STYLEREF) shows on the page whose text is the story's positions [from, to): the first
    // paragraph of the style `style` (a name) on the page, the last with the switch `l`, else the nearest before it, else the nearest after; the switches `n`, `w`, and `r` ask
    // for its number. Null: there is none (the field shows what the file had in it); a publication has none.
    virtual QString styleRefText(const QString &style, const QString &switches, int from, int to) const { Q_UNUSED(style) Q_UNUSED(switches) Q_UNUSED(from) Q_UNUSED(to) return QString(); }
    // For a word processor's page references (PAGEREF): where a bookmark begins in the story (a position of it; -1: there is none), asked of the source of the story's own fields; and,
    // asked of the source a page is drawn with, the text that a reference to the bookmark shows on it (the number of the page the bookmark is on, in its format; empty: the
    // reference shows the result the file had in it).
    virtual int bookmarkPosition(const QString &name) const { Q_UNUSED(name) return -1; }
    virtual QString pageRefText(const QString &bookmark) const { Q_UNUSED(bookmark) return QString(); }
    // The properties of the document a field of the shared engine shows (a title, an author, the file name): empty for a document that has none.
    virtual fields::Properties fieldProperties() const;
};

}  // namespace jp

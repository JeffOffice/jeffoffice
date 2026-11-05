#pragma once
// JeffWrite's document: the flowing document of libs/flow (its story, sections, and paginator) together with what a word processing
// file holds beyond the text, which is kept to be written back: styles, list definitions, the theme, and the parts the program does
// not edit. Opening a .docx fills the story from the file; saving writes the story back into the file, so what the story changed is
// what the file changes.
//
// The text is the story, a QTextDocument whose paragraphs and runs carry the formats of libs/text and the properties in
// writeprops.h; editing it is editing that document, and its own undo stack is the undo of the program (a command is what changed in
// which block). Styles are the file's: a paragraph or run's format is the whole of what it looks like (the style's with what it says
// itself on top), and what the writer calls "direct formatting" is what differs from the style's.

#include "text/textprops.h"
#include "core/tablemodel.h"
#include "flow/flowdocument.h"
#include "flow/paginator.h"
#include "office/package.h"
#include "office/packageprops.h"
#include "wml/wml.h"

#include <QHash>
#include <QSet>
#include <QObject>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextListFormat>
#include <functional>
#include <optional>
#include <memory>

namespace jp::write {

// A style as the style gallery lists it.
struct StyleEntry {
    QString id, name;
    bool character = false;
    int priority = 99;
    bool quick = false;     // shown in the gallery
};

namespace mail { struct State; }
class WriteDoc;
// The most the pictures of a document come to together.
constexpr qint64 kMostMediaBytes = qint64(512) << 20;

// What the notes of a document are to its pages (flow::FlowDocument::fields): the stories of its footnotes, made when they are asked for, and the
// height of the paragraph the pages set between the text and the notes.
class NoteFields : public FieldSource {
public:
    explicit NoteFields(WriteDoc *doc) : m_doc(doc) {}
    int firstPageNumber() const override { return 1; }
    QString pageNumberFormat() const override { return QString(); }
    QString businessField(const QString &) const override { return QString(); }
    QStringList mergeFields() const override { return {}; }
    QString mergeValue(int, const QString &) const override { return QString(); }
    const QTextDocument *storyDoc(const QString &id) const override;
    QList<const QTextDocument *> storyDocs() const override;
    double noteSeparatorHeight() const override { return separator; }
    double noteNoticeHeight() const override { return notice; }
    double noteContinuationHeight() const override { return continuation; }
    const QTextDocument *noteNoticeStory() const override;
    const QTextDocument *noteContinuationStory() const override;
    double noteContinuationRuleAt() const override { return ruleAt; }
    double noteContinuationRuleFrom() const override { return ruleFrom; }
    bool noteContinuationRuleFull() const override { return ruleFull; }
    QString styleRefText(const QString &style, const QString &switches, int from, int to) const override;
    int bookmarkPosition(const QString &name) const override;
    QString noteMarkText(bool endnote, int number) const override;
    bool restartNotesEachPage() const override { return eachPage; }
    QString footnoteFormat;    // how the footnotes' numbers are shown (w:numFmt of the settings; empty: 1, 2, 3)
    int footnoteStart = 1;     // the number the first footnote shows
    bool eachPage = false;     // the numbers begin again on each page
    double separator = 0, notice = 0, continuation = 0;   // the heights of the file's separator and continuation notice (the pages cut them: FrameSpec::noteRoomLimit)
    double ruleAt = -1, ruleFrom = 0;                     // where the rule is in the separator for the part of a note that goes on
    bool ruleFull = true;                                 // (and whether it is as wide as the column)

private:
    WriteDoc *m_doc;
    // What the fields that show a paragraph style's text keep, for the story as it is (by its revision): where the paragraphs of each style begin, the styles a name says, and the
    // texts found.
    mutable int m_styleRevision = -1, m_styleCount = -1;
    mutable bool m_styleIndexMade = false;
    mutable QHash<QString, QVector<int>> m_styleStarts;
    mutable QHash<QString, QStringList> m_styleIds;
    mutable QHash<qint64, QString> m_styleTexts;
    // Where the bookmarks of the story begin (a page reference asks), for the story as it is: found in one pass when asked, and again after an edit.
    mutable const QTextDocument *m_bookmarkStory = nullptr;
    mutable int m_bookmarkRevision = -1, m_bookmarkSize = -1;
    mutable QHash<QString, int> m_bookmarks;
};

// What an instruction of a field asks to be shown as the flowing document's own: the code of the text engine's field for the page's number, the document's page count, or the
// section's, or a paragraph style's text; empty for the rest, which show what they read.
QString liveFieldFor(const QString &instruction);

// The paragraphs of a header or footer part that a tracked change deleted whole (mark and text) are left out of what is shown: with `keepAsXml` they stay among the blocks as
// elements kept as written (what the story of the part holds, in the form it is edited in, and writes back).
void leaveOutDeletedParagraphs(QVector<office::wml::Block> *blocks, bool keepAsXml = false);

// The most distinct header and footer parts that are read; a file with more has the rest left as it is (a section's reference to one shows nothing, and nothing replaces it).
constexpr int kMostHeaderParts = 64;
// An Id for a relationship that none of `links` has.
QString nextRelationshipId(const QVector<office::Relationship> &links);

// How many times a level's run properties have been looked at for the outline effect (for a test of the cost).
int levelsScannedForOutlineForTest();
// How many expressions the attributes of tags have been looked for with since the program began: one for each name, however many tags a file has.
qint64 attributeExpressionsMadeForTest();
// How many times the expression for a frame's properties has been run on a paragraph property kept as written since the program began: a cost test (only a property that names
// a frame is looked at with it, not every property of every paragraph of a style chain).
qint64 frameExpressionRunsForTest();
// How many paragraphs the fields that show a paragraph style's text have looked at (to find where each style's paragraphs are, and to read one found), since the count was reset: a
// cost test (a page's fields cost a search, not a walk over the story).
qint64 styleRefParagraphsScannedForTest();
void resetStyleRefParagraphsScannedForTest();
// The most paragraphs of the endnotes that are set after the text (a few for each of the at most 5,000 notes), and how many the program has made copies of for them since the
// count was reset: a file whose one note of thousands of paragraphs is referred to thousands of times has no more of them made than the most.
constexpr int kMostEndnoteParagraphs = 20000;
// The most endnote references of the text that are counted for the notes set after it (the rest are left out: a reference to one shows only its number).
constexpr int kMostEndnotes = 5000;
// The most notes of a part that are made into stories (the rest are left as they are: a reference to one shows only its number), and the most footnotes the program has in a
// document (it makes no more).
constexpr int kMostNoteStories = 5000;
qint64 endnoteParagraphsMadeForTest();
void resetEndnoteParagraphsMadeForTest();
// How many walks over the whole story the endnotes' settling has made (to see whether the notes the references name are those the pages set), and how many paragraphs it has looked at in
// all, since the count was reset: a cost test (an edit that touches no reference costs neither).
qint64 endnoteSettleScansForTest();
qint64 endnoteSettleVisitedForTest();
void resetEndnoteSettleWorkForTest();
// How many places of the references that are followed as the story changes have been read or written since the count was reset (a cost test: a keystroke touches a few, whatever the
// number of the references).
qint64 endnoteSequenceWorkForTest();
void resetEndnoteSequenceWorkForTest();
// All the work the notes' bookkeeping does when the text is edited, counted in the paragraphs and places it looks at (the walks of the settling, the places of the references that are followed,
// and the walks that find the references, the places that cite notes, and the notes touched), since the count was reset: a cost test (an edit costs what it touched, with notes or without).
qint64 noteWorkForTest();
void resetNoteWorkForTest();
// How many footnote stories have been made since the count was reset: a cost test (only the notes that are shown or edited have one).
qint64 noteStoriesMadeForTest();
void resetNoteStoriesMadeForTest();

class WriteDoc : public QObject {
    Q_OBJECT
public:
    ~WriteDoc() override;

    // A new document: one empty paragraph, Normal style, paper of the region's size.
    static std::unique_ptr<WriteDoc> blank();
    // A document from a .docx package; null, with the reason, if it can't be read. `notes` get what was left out.
    static std::unique_ptr<WriteDoc> fromDocx(const QByteArray &zip, QString *error = nullptr, QStringList *notes = nullptr);
    // A document from an old binary file (.doc, .dot); null, with the reason, if it can't be read. `notes` get what was left out.
    static std::unique_ptr<WriteDoc> fromDoc(const QByteArray &bytes, QString *error = nullptr, QStringList *notes = nullptr);
    // A document from an OpenDocument text (.odt, or the flat .fodt); null, with the reason, if it can't be read. `notes` get what was left out.
    static std::unique_ptr<WriteDoc> fromOdt(const QByteArray &bytes, QString *error = nullptr, QStringList *notes = nullptr);
    // A document from a Rich Text Format file (.rtf); null, with the reason, if it can't be read. `notes` get what was left out.
    static std::unique_ptr<WriteDoc> fromRtf(const QByteArray &bytes, QString *error = nullptr, QStringList *notes = nullptr);
    // A document of plain text: a paragraph for each line, in the default style (the text of a .txt file once its characters are read).
    static std::unique_ptr<WriteDoc> fromText(const QString &text);
    // A document from the bytes of a file of any kind: an OpenDocument text, a Rich Text Format file (it begins "{\\rtf"), and an old binary file (a compound file) are read as such, anything else as a .docx.
    // `baseDir` is the folder of the file the bytes came from: the pictures of a web page that sit in files beside it are read from there (nothing else is opened, and no address of the network).
    static std::unique_ptr<WriteDoc> fromFile(const QByteArray &bytes, QString *error = nullptr, QStringList *notes = nullptr, const QString &baseDir = QString());
    // The story written back into a package (empty, with the reason, if it can't be made). `template_` saves it as a template.
    // `macros`: the macro project, its signature, and its data are kept as they were (for a .docm or .dotm); otherwise they are left out.
    QByteArray toDocx(QString *error = nullptr, bool template_ = false, bool macros = false);
    // The story written as a Rich Text Format file (empty, with the reason, if it can't be made). `notes` get what the format has no place for here.
    QByteArray toRtf(QString *error = nullptr, QStringList *notes = nullptr);
    // The story written back as an OpenDocument text (empty, with the reason, if it can't be made). `notes` get what the format has no place for.
    QByteArray toOdt(QString *error = nullptr, QStringList *notes = nullptr);
    // The document as a web page (libs/office/wml/wmlhtml.h): one file with the looks and pictures, or the filtered form with the structure alone; empty if it can't be made.
    QByteArray toHtml(bool singleFile, const QString &title, const QString &language);
private:
    // The document as the file would hold it, before it is written as a package.
    bool exportDocument(office::wml::Document *out, bool template_, bool macros);
public:

    flow::FlowDocument &flow() { return *m_flow; }
    QTextDocument *story() const { return m_flow->story(); }
    flow::Paginator &pager() { return *m_pager; }

    // ---- the endnotes the pages set
    // Whether any section puts its endnotes at its end (w:pos sectEnd): they are paragraphs of the story in the middle of it, which an edit that changes the number of the sections sets
    // again (in the edit's own undo step), as the references the text has now fall in the sections it has now.
    bool hasSectionEndEndnotes() const { return m_endnotesAtSectionEnds; }
    // Takes every run of the endnotes the pages set out of the story (a section's break goes back to the paragraph before its notes), and says so (they are set again by
    // rebuildEndnoteTails, which the edit that took them out is to end with).
    void stripEndnoteTails();
    bool endnoteTailsStripped() const { return m_tailsStripped; }
    void rebuildEndnoteTails();
    // The shown copies of an endnote that was edited are set again from its first copy (nothing is done when no note with other copies was touched).
    // (`keep`, a cursor in the words of the note, is put back at the same place of the note.)
    void refreshEndnoteCopies(QTextCursor *keep = nullptr);

    // ---- sections
    // The sections in order; the last has no break paragraph. Changing one writes it into its break paragraph, so it undoes with the text.
    int sectionCount() const { return int(m_sections.size()); }
    office::wml::SectionProps sectionProps(int index) const;
    void setSectionProps(int index, const office::wml::SectionProps &props);
    // The section the document position is in.
    int sectionAt(int position) const;
    // How many sections have been made from their records since the document was opened (to see what an edit costs).
    qint64 sectionsMadeForTest() const { return m_sectionsMade; }
    // How many elements of paragraphs the scan for the drop-down fields of forms looked at since the document was opened (it looks at each once: to see what opening costs).
    qint64 dropDownStepsForTest() const { return m_dropDownSteps; }
    // The paragraph ends the section: the break paragraphs and `flow().sections` are made again from the story. Called after edits that
    // add or remove paragraphs (the document does it itself when the story reports one).
    void syncSections();
    // Changes the properties of sections `from` to `to` (inclusive) in one pass over the story and one step of the history: `change` is given each section's properties and says
    // whether it changed them. Returns how many it changed.
    int changeSections(int from, int to, const std::function<bool(office::wml::SectionProps &)> &change);
    // Makes the paragraph at `block` the last of a new section that is like the one it is in (a break); false if it ends one already. The section the break makes
    // begins as `type` says ("nextPage", "continuous", "oddPage", or "evenPage"); the section before it begins as it did.
    bool insertSectionBreak(const QTextBlock &block, const QString &type = QStringLiteral("nextPage"));

    // ---- headers and footers
    // A section's header (or footer) has three slots: the ordinary pages' (0), the first page's (1, used by a section with a different first page), and the even pages' (2, used where
    // the document has different odd and even pages). The story of one is as the pages show it: the section's own, else the section before's; none where there is neither.
    static constexpr int kDefaultSlot = 0, kFirstSlot = 1, kEvenSlot = 2;
    QTextDocument *headerFooterStory(int section, bool footer, int slot) const;
    // Whether the section states a header (footer) of its own for the slot; else it has the section before's (it is "linked to the previous").
    bool hasOwnHeaderFooter(int section, bool footer, int slot) const;
    bool linkedToPrevious(int section, bool footer, int slot) const;
    // Makes a blank header (footer) of its own for the slot of a section that has none there (its own nor the section before's), as one step of the history. True when the slot has
    // a story now; false where it holds a part the program could not read (making one would lose it). With `lazy`, the blank one is shown in the slot to be typed in and is not made in
    // the document (no part, no reference, no step, the document not modified) until something is put in it; discardPendingHeaderFooter takes it away unmade.
    bool createHeaderFooter(int section, bool footer, int slot, bool lazy = false);
    bool isPendingHeaderFooter(const QTextDocument *story) const;
    void discardPendingHeaderFooter();
    // Link to Previous: on, the section has the section before's (its own is let go); off, it has a copy of the one it had, of its own to edit. False where that cannot be done
    // (the first section, which has none before it; or a slot that has no header to copy). One step of the history.
    bool setLinkedToPrevious(int section, bool footer, int slot, bool linked);
    // Remove Header (Remove Footer): the section's own headers (footers) of every kind are let go; a section after the first, which would then show the section before's, is given blank ones
    // of its own where that has text. One step of the history. False where the section has none to remove, or one the program can't show.
    bool removeHeaderFooter(int section, bool footer);
    // Different first page (a section's) and different odd and even pages (the document's), as one step of the history each.
    bool differentFirstPage(int section) const;
    void setDifferentFirstPage(int section, bool on);
    bool differentOddEven() const { return m_wml.evenAndOddHeaders; }
    // Facing pages with mirrored margins (the inside and outside margins swap on facing pages), and automatic hyphenation: the document's, one step of the history each.
    bool mirrorMargins() const { return m_flow->mirrorMargins; }
    void setMirrorMargins(bool on);
    // The page color (none: an invalid color).
    QColor pageColor() const { return m_flow->pageColor; }
    void setPageColor(const QColor &color);
    bool autoHyphenation() const { return m_flow->hyphenate; }
    // The hyphenation zone, in points, and the most lines in a row that may end in a hyphen (0: no limit): the settings' w:hyphenationZone and w:consecutiveHyphenLimit.
    double hyphenationZone() const { return m_flow->hyphenZone; }
    int hyphenLimit() const { return m_flow->hyphenLimit; }
    // Sets both (a setting of the document, written with it); the lines are set again.
    void setHyphenation(double zonePoints, int limit);
    // Whether words in capitals are hyphenated too (the settings' w:doNotHyphenateCaps, turned round); one step that undoes.
    bool hyphenateCaps() const { return m_flow->hyphenateCaps; }
    void setHyphenateCaps(bool on);
    void setAutoHyphenation(bool on);
    void setDifferentOddEven(bool on);
    // The slot a page of a section uses.
    int headerSlotOf(int section, bool firstOfSection, int pageNumber) const;
    // The story of the slot in the form it is edited in (made once, with what the program can't edit kept as pieces to write back where they were); null for none.
    QTextDocument *editableHeaderFooter(int section, bool footer, int slot);
    // Whether the story is a header's or a footer's, and which section and slot first has it (-1 for none).
    bool isHeaderFooterStory(const QTextDocument *story) const;
    bool headerFooterSlotOf(const QTextDocument *story, int *section, bool *footer, int *slot) const;

    // ---- footnotes
    // The story of footnote `id` (its w:id) in the form it is edited in, as the pages show it at their foot: its number mark is one character that no edit takes (an atom), what the
    // program does not edit is kept as pieces and written back where it was, and its steps are in the one history. Null for a note the document does not have.
    QTextDocument *editableFootnote(int id);
    // Whether the story is a footnote's, and which (its w:id).
    bool footnoteIdOf(const QTextDocument *story, int *id = nullptr) const;
    // The positions in the text of the characters that cite footnote `id`, in order.
    QVector<int> footnoteReferences(int id);
    // The Ids of the footnotes the text cites now (and the document has), in order of Id.
    QVector<int> citedFootnotes();
    // Whether the document has a footnote by that Id (one of the file's, or one made since).
    bool hasFootnote(int id) const { return m_noteBlocks.contains(QStringLiteral("fn%1").arg(id)); }
    // A new footnote, for a reference to be put in the text: its Id (one more than any the file or its references use), with a story that begins with its number mark and a blank;
    // 0 where none can be made (the document has the most notes, or its notes part is one the program cannot add to). A note is the document's only while the text cites it:
    // one whose last reference was taken out is not written, and one whose reference comes back (an undo, a paste of what was cut) is again.
    int makeFootnote();
    // Whether a footnote or an endnote can be made now (the document is not one with the most notes, and its part is one the program can add to).
    bool roomForFootnote();
    bool roomForEndnote();
    // How many notes of each kind the document holds (a test of what undone notes leave).
    int heldFootnotesForTest() const { return int(m_noteBlocks.size()); }
    int heldEndnotesForTest() const { return int(m_endnoteBlocks.size()); }
    // A copy of a footnote as a note of its own (what a pasted copy of a reference cites): its Id, 0 for none.
    int duplicateFootnote(int id);
    // The look and the pieces of a reference to footnote `id`: its character format (the reference character style over the look of a paragraph of this style), to put
    // where the reference is.
    QTextCharFormat footnoteReferenceFormat(int id, const QString &paragraphStyle);
    // Whether the story is one the document's header or footer, or a note, has: the stories that take no page break, section break, or note.
    bool isSideStory(const QTextDocument *story) const { return isHeaderFooterStory(story) || footnoteIdOf(story); }

    // ---- endnotes (the notes the pages set after the text, or at the end of a section: paragraphs of the story that are marked as theirs; their words are edited where they stand)
    // Whether the paragraph is one of a note's among them, and the Id of the note (a note cited again is set again: its copies have the one Id).
    bool endnoteIdOf(const QTextBlock &block, int *id = nullptr) const;
    // Whether the paragraph is one of a note whose words are edited in place that the pages set more than once (the edit of its words is one step of the undo stack with the setting again of the
    // others, which Qt joins to the edit when the edit is a block).
    bool endnoteHasCopies(const QTextBlock &block) const;
    // Whether the document has an endnote by that Id (one of the file's, or one made since).
    bool hasEndnote(int id) const { return m_endnoteBlocks.contains(QString::number(id)); }
    // The positions in the text of the characters that cite endnote `id`, in order; and the Ids of the notes the text cites now (in order of Id).
    QVector<int> endnoteReferences(int id);
    QVector<int> citedEndnotes();
    // Where the words of the note by that Id begin (after its number) and end in the story, as the pages set it (its first copy); false where no page of notes has it.
    bool endnoteWords(int id, int *from, int *to);
    // A new endnote for a reference to be put in the text: its Id (one more than any the file or its references use); 0 where none can be made (the document has the most notes, or its
    // endnotes part is one the program cannot add to). As with footnotes, a note is the document's while the text cites it.
    int makeEndnote();
    // A copy of an endnote as a note of its own (what a pasted copy of a reference cites): its Id, 0 for none.
    int duplicateEndnote(int id);
    QTextCharFormat endnoteReferenceFormat(int id, const QString &paragraphStyle);
    // A copy of a footnote as a new endnote (`toEndnote`), or of an endnote as a new footnote, with the note styles and the number mark of its new kind: its Id, 0 for none. `lastMade` is the Id of the note of that kind that the caller made just before (0 for none): the new one is the Id above it,
    // so that a run of conversions does not look for the highest Id each time.
    int convertedNote(int id, bool toEndnote, int lastMade = 0);
    // (A test of the settling: whether the references followed as the story changed are the ones a new look through it finds.)
    bool endnoteSequenceMatchesStoryForTest();
    // After an edit of the text: if the notes that the text's references name are not the notes the pages set, the notes are set again (as part of the edit's own undo step), and the
    // references and the notes' numbers are made anew.
    void settleEndnotes();

    // ---- one history
    // The steps of the text and of the headers and footers in the order they were made: an undo undoes the last, wherever it was, and a redo does it again. Returns the story the
    // step was in (null when there was none); the cursor, if given, is put where the step changed the story.
    bool canUndo() const { return !m_undoLog.isEmpty(); }
    bool canRedo() const { return !m_redoLog.isEmpty(); }
    QTextDocument *undo(QTextCursor *cursor = nullptr);
    QTextDocument *redo(QTextCursor *cursor = nullptr);

    // The body as the writer takes it from the story (what a save writes, as blocks): for a tool that reads the text a document comes to.
    QVector<office::wml::Block> bodyBlocks()
    {
        QVector<office::wml::Block> out;
        exportBody(&out);
        return out;
    }

    // The name of the part the main part has a relationship of this type to (the type's last words, such as "comments" or "commentsExtended"); empty for none.
    QString mainRelatedPart(const QString &relationshipTypeEnd) const;
    // What core/commentops.cpp keeps with the document (the comment parts of the file, read once).
    std::shared_ptr<void> commentCache;
    // The mail merge of the document (core/mergeops.h): null for a document that is not merged.
    std::shared_ptr<mail::State> mailMerge;

    // The edits made while one of these lives, in any of the document's stories, are one step of the history: an Undo takes all of them back, and a Redo does all again.
    class StepGroup {
    public:
        explicit StepGroup(WriteDoc &doc) : m_doc(doc)
        {
            if (m_doc.m_groupDepth++ == 0) m_doc.m_openGroup = m_doc.m_nextGroup++;
        }
        ~StepGroup()
        {
            if (--m_doc.m_groupDepth == 0) m_doc.m_openGroup = 0;
        }
        StepGroup(const StepGroup &) = delete;
        StepGroup &operator=(const StepGroup &) = delete;

    private:
        WriteDoc &m_doc;
    };
    // The stories besides the text that hold tracked changes: the headers and footers and the footnotes (each made editable if it was not), in a fixed order. The endnotes' words are in the text.
    // With `every`, the stories that hold no change too (the headers and footers the file names, and every footnote).
    QVector<QTextDocument *> storiesWithChanges(bool every = false);

    // ---- review (core/reviewops.h, core/commentops.h)
    // Whether edits are tracked as changes (Track Changes: the file's w:trackRevisions). A setting of the document that is written with it, not a step of the history.
    bool trackChanges() const { return m_trackChanges; }
    void setTrackChanges(bool on);
    // The document's protection (w:documentProtection): what it allows and whether it is enforced. Like Track Changes, a setting written with the document, not a step of the history.
    std::optional<office::wml::DocumentProtection> protection() const { return m_protection; }
    void setProtection(const std::optional<office::wml::DocumentProtection> &protection);
    // How a kind of note is numbered and where it is set, as the settings say now: the format of its numbers (a w:numFmt value; "decimal" where the file says none), the number the first
    // shows, whether the numbers begin again with each section or page (continuous, eachSect, eachPage), and where they are set (the footnotes' pageBottom or beneathText, the endnotes'
    // sectEnd or docEnd; empty where the file says none).
    struct NoteSettings {
        QString format = QStringLiteral("decimal");
        int start = 1;
        QString restart = QStringLiteral("continuous");
        QString position;
        bool operator==(const NoteSettings &o) const { return format == o.format && start == o.start && restart == o.restart && position == o.position; }
    };
    NoteSettings noteSettings(bool endnotes) const;
    // The separators of the notes: "separator" (between the text and the notes), "continuationSeparator" (above a note that goes on from the page before), and "continuationNotice" (the words
    // below a note that goes on to the next page). A separator is a line, text, nothing, or Kept: something the document has that the program does not edit (it is written as it was).
    struct NoteSeparator {
        enum Kind { Line, Text, None, Kept };
        Kind kind = Line;
        QString text;
        bool operator==(const NoteSeparator &o) const { return kind == o.kind && text == o.text; }
    };
    // Whether the document has notes of the kind (a document without has no separators to set).
    bool hasNotesOfKind(bool endnotes) const;
    NoteSeparator noteSeparator(bool endnotes, const QString &type) const;
    // Sets it, written with the file. The footnotes' pages show it at once; the endnotes' are made again when the document is read from what toDocx gives. False where it can't be set.
    bool setNoteSeparator(bool endnotes, const QString &type, const NoteSeparator &separator);
    // Sets it, written with the file (a setting of the document, not a step of the history). The footnotes' numbers show it at once; the endnotes' marks and notes are made again
    // when the document is read again from what toDocx gives, which a caller that changed the endnotes' settings does.
    void setNoteSettings(bool endnotes, const NoteSettings &settings);
    // False where the file's settings part can't be read, so that a change of the notes' numbering could not be written into it (the window says so and makes none).
    bool noteSettingsCanBeSaved() const;
    // The sources of the bibliography (core/citeops.h): the XML of the file's bibliography part (a custom XML part), as read or as last set; empty for none.
    QByteArray sourcesXml();
    // Sets it. A setting of the document, written with the file, not a step of the history.
    void setSourcesXml(const QByteArray &xml);
    // A number that is new whenever the sources change (and the first time they are read), the same for as long as they do not; unique across documents.
    quint64 sourcesRevision() const { return m_sourcesRevision; }
    // The person who makes the changes and comments, and their initials (the user name of the settings: the window gives them).
    QString reviewer() const { return m_reviewer; }
    QString reviewerInitials() const { return m_reviewerInitials; }
    void setReviewer(const QString &name, const QString &initials);
    // A place in the list of people who made changes (they are added as the file's marks name them): the color of their marks.
    int reviewerIndex(const QString &author);
    // An Id for the mark of a new change or comment: above every one the document has.
    int nextRevisionId();
    // How tracked changes are shown (tp::kRevisionShowDeleted and the others): the text engine's view of them, kept in the layout environment and the story. Not saved.
    int revisionView() const { return m_revisionView; }
    // Show Field Codes (Shift+F9, Alt+F9): a view of the text, kept in the layout's environment and not in the story, so it is no step of the history and no change to the document.
    // `all` shows every field's codes; `ids` are the numbers (tp::FieldId) of single fields. Not saved.
    bool allFieldCodesShown() const { return m_fieldCodesAll; }
    const QSet<int> &shownFieldCodes() const { return m_fieldCodeIds; }
    void setFieldCodes(bool all, const QSet<int> &ids);
    // Whether the text has a field (known when the file is read and as fields are put in; a text whose last field was taken out may still say yes until the fields are looked for).
    bool mayHaveFields() const { return m_mayHaveFields; }
    void setMayHaveFields(bool yes) { m_mayHaveFields = yes; }
    void setRevisionView(int view);
    // Shows the text that is hidden in the file, as the formatting marks do (a view of the text, not a change to it).
    void setShowHiddenText(bool on);
    bool showsHiddenText() const { return m_showHidden; }

    // ---- tables
    // What the last table command that did nothing said of why (empty when it did what it was asked): a limit of the file, or a shape it can't take.
    QString tableNotice;
    QSet<QString> builtInTableStylesAdded;   // built-in table styles a change put in the style sheet (their names stay)
    // The same for the picture commands (core/pictureops.h): why the last one that did nothing did nothing, when the person needs telling.
    QString pictureNotice;
    // What core/tableops.cpp keeps with the document: the tables of the story as they were last looked at, and the keys of the cell records that paragraphs held before a command
    // replaced them , to be let go when nothing can reach them.
    std::shared_ptr<void> tableViewCache;
    QVector<int> retiredCells;
    // The table style of this id for the tables the program makes: the file's own, else a built-in one, which the file written has only while a table has it. A built-in style
    // is the default of its kind only when the file has none under any id.
    bool ensureTableStyle(const QString &id);
    // The styles tables are made from: the file's, with those the tables made have.
    const office::wml::StyleSheet &tableSheet() const;
    // The cell records that the steps of the history can bring back, by the serial of the step (those a table command made, and those it took from paragraphs), and those of
    // commands that have no step yet: a record is let go only when no paragraph and none of these holds it. `tableStepsSeen` is the serial that the next step will have, as it
    // was when they were last attached.
    QHash<quint64, QVector<int>> tableStepKeys;
    QVector<int> tablePendingKeys;
    quint64 tableStepsSeen = 0;
    quint64 nextStepSerial() const { return m_nextSerial; }
    quint64 lastStepSerial() const { return m_undoLog.isEmpty() ? 0 : m_undoLog.last().serial; }
    QSet<quint64> stepSerials() const;
    // ---- styles
    const office::wml::StyleSheet &styleSheet() const { return m_wml.styles; }
    int compatMode() const { return m_wml.compatMode; }
    QString defaultParagraphStyle() const;
    QVector<StyleEntry> styleEntries() const;
    // The style the document has under `id`, adding a built-in one it lacks; false when there is none by that id.
    bool ensureStyle(const QString &id);
    // Puts a style in the style sheet (one of that id is replaced where it stands, a new one goes last) as part of the edit it is made in: the step of the history that edit becomes
    // keeps what the style was and is, so that Undo and Redo put the style sheet as they put the text. Called inside an edit block of the story (styleops.cpp does).
    void putStyle(const office::wml::Style &style);
    // Takes a style out of the style sheet as part of the edit it is made in (as putStyle: the step keeps the style, so that Undo puts it back). Nothing for an id the sheet lacks.
    void removeStyle(const QString &id);
    // Makes the edit being made a step of the story's history even if it changes nothing else (the count on the first paragraph goes up), for a style put with it.
    void markStyleStep();
    // What a run of this format says beyond the style it has (the difference from the paragraph style's look and its own character style), for a style made from a selection.
    office::wml::RunProps writtenRunProps(const QTextCharFormat &format, const QString &paragraphStyle) const { return runPropsWritten(format, paragraphStyle, nullptr); }
    // Puts `other` in the place of the style sheet and returns the one that was there (the look of a style as it was, worked out for a restyle); the caches are let go.
    void swapStyleSheet(office::wml::StyleSheet &other);
    // Lets go of what was worked out for the sheet that was swapped out (a pass that swaps begins and ends with this).
    void forgetSwappedScratch() { m_scratchHeld.clear(); }
    // Ends an edit that put styles: false (and the sheet put back as it was) when no step of the history can take what the edit changed (the story keeps no history).
    bool settleStyleEdit();
    // Counts for tests of what work costs: whole style sheets copied to look at a built-in style the document lacks, and passes over the text to find the styles in use.
    qint64 sheetCopiesForTest() const { return m_sheetCopies; }
    qint64 inUseScansForTest() const { return m_inUseScans; }
    // How many times the style sheet has been changed (a style put, added when first used, or put back by an Undo or a Redo): a list of styles made at one count is good at it.
    quint64 styleSheetRevision() const { return m_styleRev; }
    // The ids of the styles the text uses (a paragraph's, a run's character style, the default paragraph style always), for the story as it is (found again after an edit).
    const QSet<QString> &stylesInUse() const;
    // How many steps of the history hold a change to the styles (a hostile document's undo must not hold more than its steps).
    int styleStepsHeldForTest() const { return int(m_styleSteps.size()); }
    // The whole of a paragraph style's look: the paragraph's format and its run's, with the document's defaults under them.
    QTextBlockFormat styleBlockFormat(const QString &paragraphStyle, const office::wml::TableLayer *layer = nullptr);
    QTextCharFormat styleCharFormat(const QString &paragraphStyle, const QString &characterStyle = QString(), const office::wml::TableLayer *layer = nullptr);
    // What the paragraph has from its style and its list (the file's numbering gives a list's indents), for telling what it says itself.
    // `direct` says the paragraph itself states its numbering (so the list's indents come over its style's); `cache`, if given, holds
    // the answers for one pass over a document (the same style, list, and level come up again and again).
    // `table` is what the style of the table the paragraph is in gives it.
    office::wml::ParaProps baselineParagraph(const QString &paragraphStyle, int numId, int level, bool direct = false, QHash<QString, office::wml::ParaProps> *cache = nullptr,
                                             const office::wml::TableLayer *table = nullptr) const;
    // What the style of a table gives the paragraphs of the cell with this key (null: none).
    const office::wml::TableLayer *tableLayerOf(int cellKey) const;

    // ---- proofing
    // The language of text that says none: the document's default (its defaults' w:lang), empty for none (American English).
    QString proofingLanguage() const;
    // The language that a paragraph style and a character style give the text in them (the character style's first, then each style's it is based on); empty when none
    // states one.
    QString styleLanguage(const QString &paragraphStyle, const QString &characterStyle) const;

    // ---- lists
    const office::wml::Numbering &numbering() const { return m_wml.numbering; }
    // The list definition (w:numId) for a new list of plain bullets or numbers, made if need be. Bullets share one definition; each new
    // numbered list is a definition of its own, so that it starts at 1.
    int newListInstance(bool numbered);
    // The Qt list format of a level of a definition; `marker` gets the text of a marker that is not Qt's to count (set in legal style, or
    // numbered with several levels in it).
    QTextListFormat listFormat(int numId, int level, int start = -1) const;
    office::wml::ParaProps levelParagraph(int numId, int level) const;   // the indents the level gives its paragraphs
    int deepestLevel(int numId) const;   // the deepest level the definition has (0 to 8), -1 for none
    // Gives the items of the lists in a story the numbers they have: where Qt's own count of an item is the number the file's numbering gives it, the item is left to Qt's
    // count (which goes on as the person edits), and where it is not (a count shared by several numbering ids, a restart at one level, several levels in the marker, the legal style)
    // the item has its number as a marker of its own. Run after the story is filled, and after each edit that can change the numbers; its changes join the edit's undo step.
    void renumberLists(QTextDocument *story);

    // ---- the file's links and parts (what a pasted picture or hyperlink needs from the document it goes into)
    // This document's own number, which tells a copy made from it from one made from another document.
    quint64 id() const { return m_id; }
    // A relationship of the main part by its Id (empty Id if there is none).
    office::Relationship mainRelationship(const QString &id) const;
    // A kept part's bytes (and its content type); empty if the document has no such part.
    QByteArray partData(const QString &absoluteName, QString *contentType = nullptr) const;
    // ---- the file's properties (the core, extended, and custom parts of docProps)
    office::PackageProps packageProps() const;
    // Puts the properties in the file's parts (made where the file has none, with the relationship that names each) and counts the document as changed. What the parts hold besides stays.
    void setPackageProps(const office::PackageProps &props);
    // The part (and its bytes) the package's relationship of this type (the end of its type, "/core-properties") names; empty if there is none.
    QString propsPartName(const QString &typeSuffix) const;
    QByteArray propsPartData(const QString &typeSuffix) const;
    bool markedAsFinal() const;
    // Puts a part in the file (replacing the one of that name), and the relationship of the package to it of the type given (none: no relationship is made).
    void putPackagePart(const QString &absoluteName, const QString &contentType, const QByteArray &data, const QString &relationshipType = QString());
    // Takes a part out, and the relationships of the package and the main part that name it. True if there was one.
    bool removePackagePart(const QString &absoluteName);
    // The names of the parts that are there (absolute).
    QStringList packagePartNames() const;
    // The counts of the text now (the pages are the layout's, not counted here).
    struct TextCounts {
        int words = 0, characters = 0, charactersWithSpaces = 0, paragraphs = 0;
    };
    TextCounts textCounts() const;

    // The Id of a relationship of the main part to an address outside the file, made if the document has none like it.
    QString addExternalLink(const QString &type, const QString &target);
    // The Id of an image relationship to a new part with these bytes (named image1.png and on, in word/media).
    // (A picture the document already holds, byte for byte, is that one: `added`, if given, says whether a part and a relationship were made.)
    QString addMedia(const QByteArray &bytes, const QString &extension, const QString &contentType, bool *added = nullptr);
    // Takes back a relationship of the main part that addMedia made, and its part if no other relationship names it (for a picture that could not be put in the text).
    void removeMedia(const QString &id);

    // The bytes of the part that a relationship of the main part names (a chart), or none for an Id that names no part. And a new part with a relationship to it of this kind, named
    // folder/base1.ext and on (it has the relationships of `likePart`, a part it takes the place of); the Id of the relationship. Like a picture's, it is left out of the file when nothing names it (a chart taken back by Undo).
    QByteArray relatedPartData(const QString &id) const;
    QString addRelatedPart(const QString &folder, const QString &base, const QString &extension, const QByteArray &data, const QString &contentType, const QString &relationshipType, const QString &likePart = QString());
    // The relationships of the main part (for the code that edits a part and puts the new one beside it).
    const QVector<office::Relationship> &mainRelationshipsForTest() const { return m_wml.mainRelationships; }

    // The most the pictures of a document come to, together, and the limit this document is held to (a test sets a smaller one); how much they come to now.
    qint64 mediaLimit = kMostMediaBytes;
    qint64 mediaBytes() const;
    // How many places the drawn forms of the objects have (a test of what editing pictures leaves), how many hold a form, and a look for the forms nothing can reach now.
    int objectSlotsForTest() const { return int(m_flow->objects.size()); }
    int liveObjectRecordsForTest() const { return int(m_flow->objects.size() - m_freeObjects.size()); }
    void collectObjectRecordsForTest() { collectObjectRecords(); }
    // The drawn forms the program made of pictures are let go when nothing can reach them: no character of the text, and no step of the history (Undo or Redo). A form made again
    // for the same drawing is the one made before. These keep the books: `retireDrawnForm` for the form a character had before a change, and `keepObjectsForTheHistory` once the
    // change is a step.
    void retireDrawnForm(const QTextCharFormat &before);
    void keepObjectsForTheHistory();
    void collectObjectRecords();
    // The pages' form of a drawn object of the main text (a picture: its atom is the run's child kept as XML) is put in `format`: an inline object of the character, or the
    // anchor of an object set at a place on the page. False when it makes none (nothing drawn, the picture is not in the file, or the document holds the most pictures and objects it
    // can show: they are kept for as long as a step of the history can bring a character that has one back, so none is let go).
    // An id for a new drawing (its docPr): above any the text had when this was first asked, and one more each time.
    int nextDrawingId(const QTextDocument *story);
    // After text that may hold drawings of other ids was put in (a paste): the highest is looked for again.
    void forgetDrawingIds();
    bool makeDrawnFormat(const office::wml::Inline &atom, QTextCharFormat *format);

    // ---- the file
    QString filePath;
    // Whether anything changed since the file was read or saved: a step of the history other than the one it was then at, or something set by hand (setModified).
    bool isModified() const;
    void setModified(bool modified);
    // The settings of the file the program reads.
    int defaultTabStop() const { return m_wml.defaultTabStop; }
    const office::Theme *theme() const { return m_wml.hasTheme ? &m_wml.theme : nullptr; }
    bool isTemplate() const { return m_wml.mainContentType.contains(QLatin1String("template")); }
    // What the file had that its reading left out, for the person.
    QStringList readNotes;
    // The file had macros, which are not kept: written, it is the plain kind (.docx).
    bool hadMacros() const { return m_hadMacros; }
    // Something of the file is not in the document (see office::wml::ReadNotes::leftOut): saving over the original would lose it.
    bool leftSomethingOut() const { return m_leftOut; }

Q_SIGNALS:
    void sectionsChanged();
    // A header or footer story was edited (by a step, an undo, or a redo).
    void headerFooterChanged(QTextDocument *story);
    // A footnote's story was edited (by a step, an undo, or a redo).
    void noteChanged(QTextDocument *story);
    // The steps that can be undone or done again changed, or whether the document is modified.
    void historyChanged();
    void modifiedChanged();
    // Track Changes was turned on or off, the person who reviews changed, or the way changes are shown did.
    void reviewChanged();

private:
    WriteDoc();
    void build(office::wml::Document &&doc);
    void buildStory(const QVector<office::wml::Block> &body);
    // (`header`: the story of a part, not the body's; `headerFooter`: it is a header's or a footer's, whose frames set on the page are text boxes there.)
    // (`append`: the paragraphs go after the story's last, as the document's endnotes: they are marked as such, and the mark of a note's reference at the head of its text is the one its atom has.)
    // (`editable`: a header's or footer's story in the form it is edited in: what the program can't edit is kept as pieces that are written back where they were, a field is one piece
    // (a character of the story), and nothing of the part is left out.)
    void fillStory(QTextDocument *story, const QVector<office::wml::Block> &body, bool header, const QString &part = QString(), bool headerFooter = false, bool append = false, int insertAt = -1, bool editable = false);
    // Makes of a drawn object of a run (its atom) what the pages show: a picture in the line is an inline object of the character, the rest are objects set at
    // places on the page of the paragraph they are anchored to. False when it makes none (nothing drawn, or a picture the file does not hold).
    // (`lineWidth` is the width of a line of the paragraph the object is in, for a rule that is a share of it: its cell's or its section's text, less its indents; `content` is
    // the paragraph's, for whether a rule is alone in it.)
    bool drawnObjects(const office::wml::Inline &atom, const QString &part, QTextCharFormat *format, double lineWidth = 468, const QVector<office::wml::Inline> *content = nullptr);
    // The story of the header or footer part a relationship names (made once, kept); none for a link that is not a part, or past the limit of parts.
    std::shared_ptr<QTextDocument> headerStory(const QString &relationshipId);
    // Gives the flowing sections from `from` on the stories of their headers and footers, those they state and the section before's for the rest.
    void attachHeaders(int from);
    // The footnotes of the file (its footnotes part), kept as what they hold and made into stories when a page asks for one ("fn" and the note's Id).
    void loadNotes();
    // The endnotes of the file (its endnotes part), and the pages' copy of them after the last text: the paragraphs of the notes the text refers to, in the order of their references, after
    // the file's separator for them.
    void loadEndnotes();
    void appendEndnotes();
    void measureNoteSeparator();
    const QTextDocument *noteStory(const QString &id);
    QTextBlockFormat paragraphFormatOf(const office::wml::ParaProps &p) const;
    const QTextDocument *noticeStory();
    const QTextDocument *continuationStory();
    friend class NoteFields;
    void exportBody(QVector<office::wml::Block> *body);
    // (`first` to `last`, the paragraphs of an endnote that the pages set: they are written as a note's paragraphs, which end no section.)
    void exportInto(BlockSink &sink, QTextDocument *story = nullptr, QTextBlock first = QTextBlock(), QTextBlock last = QTextBlock(), bool asNote = false);
    // What writing gives a run of this format, in a paragraph of this style (and table layer): the properties it has beyond its style's.
    office::wml::RunProps runPropsWritten(const QTextCharFormat &format, const QString &styleId, const office::wml::TableLayer *layer) const;
    void settleTables();
    void pruneUnusedMedia(office::wml::Document *out) const;
    void setUpEnvironment();
    void storyChanged(int position, int removed, int added);
    // The style sheet to read a style's look from: the document's, or for a built-in style it lacks, a copy with that style added.
    const office::wml::StyleSheet &sheetFor(const QString &styleId) const;
    office::wml::SectionProps lastSectionProps() const;

    int m_drawingId = 0;
    QVector<int> m_freeObjects;                    // places of FlowDocument::objects that hold no form now (a new form takes one)
    QHash<QByteArray, int> m_objectsByDrawing;     // the first form made for a drawing's bytes, while it is held
    QHash<int, quint64> m_objectHeld;              // the forms the program made of pictures, each with the step that last made, changed, or took it from a character (0: not yet a step)
    QVector<int> m_objectPending;                  // the forms a command touched that have no step yet
    quint64 m_objectsSeen = 0;                     // the serial the next step had when the pending ones were last attached
    qsizetype m_collectAt = 2000;
    QSet<QString> m_madeLinks;                     // the Ids of the relationships to addresses outside the file that the program added (a link taken back by Undo leaves its own unused)
    QHash<QString, QString> m_madeMedia;           // the pictures' parts the program added: the Id of the relationship, and the part
    quint64 m_id = 0;
    bool m_hadMacros = false, m_leftOut = false;
    std::unique_ptr<flow::FlowDocument> m_flow;
    std::unique_ptr<flow::Paginator> m_pager;
    office::wml::Document m_wml;                   // everything of the file but the body
    std::vector<office::wml::SectionProps> m_sections;   // the sections (the last one's is in m_last)
    office::wml::SectionProps m_last;
    QVector<int> m_sectionKeys;                    // the key of each section's record (the break paragraphs', then the last's), as of the last sync
    qint64 m_sectionsMade = 0;
    qint64 m_dropDownSteps = 0;
    std::unique_ptr<NoteFields> m_noteFields;
    std::unique_ptr<NoteFields> m_styleRefSource;   // (answers STYLEREF for the pages; the notes' own is above, there when the file has notes)
    std::unique_ptr<QTextDocument> m_noticeStory, m_continuationStory;   // (the file's continuation notice and its separator for the part of a note that goes on, made when a page sets them)
    QString m_notePart;                                               // the footnotes part's name
    QHash<QString, QVector<office::wml::Block>> m_noteBlocks;         // by "fn" and Id
    QVector<office::wml::Block> m_noteNotice;                         // the paragraphs of the file's continuation notice (none: no room is kept for one)
    QVector<office::wml::Block> m_noteContinuation;                   // the paragraphs of the file's separator for the part of a note that goes on (none: the separator's)
    QVector<office::wml::Block> m_noteSeparator;                      // the paragraphs of the file's note separator (none: a line of the Normal style)
    QString m_endnotePart;                                            // the endnotes part's name
    QHash<QString, QVector<office::wml::Block>> m_endnoteBlocks;      // by Id
    QVector<office::wml::Block> m_endnoteContinuation, m_endnoteNotice;   // the file's other separator notes of the endnotes part (not set by the pages: kept, and edited with the dialog)
    QSet<QString> m_separatorsEdited;                                  // "f:separator", "e:continuationNotice"...: the separator notes the person changed
    QVector<office::wml::Block> *separatorBlocks(bool endnotes, const QString &type) const;
    void separatorsToWrite(bool endnotes, office::wml::NotesChange *change) const;
    QVector<office::wml::Block> m_endnoteSeparator;                   // the paragraphs of the file's separator for the endnotes
    struct EndnoteRef {
        QString id, mark;                                             // a note a reference of the text names, and the number the reference shows
        int section = 0;                                              // the section the reference is in (by the breaks before it)
        bool atSectionEnd = false;                                    // its section's notes are set at its end (w:pos sectEnd), not at the document's
    };
    QVector<EndnoteRef> m_endnoteRefs;                                // the references of the text, in order (a deleted one is none)
    // (The stories of the notes and of the headers and footers are made when a page needs them and are not edited: nothing in the program reaches them yet. When
    // something does, the paginator has to be told as it is of the story's edits (its measures of them are kept by the story's revision), and the pages that show
    // them made again.)
    QHash<QString, std::shared_ptr<QTextDocument>> m_noteStories;     // the stories made of them (each in the form it is edited in, in the one history)
    // Editing the notes (writedoc_noteedit.cpp)
    QHash<const QTextDocument *, int> m_noteIds;                      // the footnote each of those stories is (its w:id)
    QVector<QTextDocument *> m_dirtyNotes;                            // the stories of the notes that were edited, which another story's edit seals
    int m_footnotePartHighest = 0;                                    // the highest Id of any element of the file's footnotes part (a new note takes one above it)
    QSet<int> m_createdFootnotes;                                     // the footnotes made since the document was read (written while the text cites them)
    QSet<int> m_footnotesCitedAtOpen;                                 // those of the file the text cited when it was read (one whose last reference was taken out is not written)
    enum class NotesPart { Absent, Readable, Unusable };
    NotesPart m_footnotesPart = NotesPart::Absent;                    // the file's footnotes part: none, one the program reads, or one it cannot (not XML, or no part by its name)
    bool m_footnotesPartNew = false;                                  // the part named by m_notePart is one the program will make (there is none in the file)
    QHash<int, QVector<int>> m_footnoteRefs;                          // the references of the text, by the note's Id, found when asked and let go with any change to the text
    bool m_footnoteRefsValid = false;
    void indexFootnoteReferences();
    // The Ids of the footnotes and endnotes that places other than the text's own references cite: the headers and footers, the text boxes and other pieces of the text that are kept as XML,
    // and the notes themselves. A note stays in the file while one of them cites it, though the text no longer does.
    void notesCitedBeyondTheText(QSet<int> *footnotes, QSet<int> *endnotes);
    // What cites what: the notes that the text's kept pieces and the headers and footers cite, and, for each note, the footnotes and the endnotes that its words cite (with `notesOnly`, only the
    // second: what the notes cite).
    struct NoteCitations {
        QSet<int> beyondFootnotes, beyondEndnotes;
        QHash<int, std::pair<QSet<int>, QSet<int>>> byFootnote, byEndnote;
    };
    NoteCitations noteCitations(bool notesOnly);
    // The Ids of the notes that are alive: those the text cites (by its own references or by its kept pieces), those a header or a footer cites, and those that a note that is alive cites, and so on.
    // Notes that cite only one another are not alive.
    void liveNotes(QSet<int> *footnotes, QSet<int> *endnotes);
    // When the file is read: the notes that the text cites, and those they cite, and so on, are the ones that are let go with the text's last reference to them.
    void spreadCitationsAtOpen();
    static void spreadThroughNotes(const NoteCitations &c, QSet<int> *footnotes, QSet<int> *endnotes);
    // Notes made since the document was read, each with the history step that made it (0 until that step is added): a note whose last reference was taken out, and whose step is in neither
    // log any more (it was undone, and an edit since has made it impossible to redo), can never come back, and is let go with its slot. (An Id that was used is not used again.)
    QHash<int, quint64> m_createdFootnoteStep, m_createdEndnoteStep;
    QVector<std::pair<bool, int>> m_noteStepPending;   // (endnote?, Id) of the notes made since the last step: the next step is theirs
    int m_footnoteIdFloor = 0, m_endnoteIdFloor = 0;   // the highest Id made in this session
    void reclaimDeadNotes(bool endnotes, int keep);
    // The references to endnotes that the settling counts (those that name a note the document has, and are not what a tracked change deleted), kept as the story changes, so that an edit that
    // touches none of them does not look through the story again. Each is kept by its distance from the one before it (the first from the start of the story) and the Id it names, with the
    // sums of the distances in a tree (a Fenwick tree): a change of the story moves the places of all the references after it by changing one distance, and a place is found in a few steps,
    // whatever the number of the references. Valid once the settling has looked through the story, until a change takes a reference out, puts one in, or moves the notes they name, the notes
    // are set again, an undo or a redo is done, or the story is made anew.
    struct EndnoteSeqEntry {
        int position = 0, id = 0;
    };
    struct EndnoteSeq {
        QVector<int> gap, id, tree;   // by the order of the references; the tree has a place more than the references, counted from 1
    };
    EndnoteSeq m_endnoteSeq;
    bool m_endnoteSeqValid = false;
    void setEndnoteSeq(const QVector<EndnoteSeqEntry> &references);
    // How many references are before a place (their places are less than it), and the place of the last of them (0 for none).
    qsizetype endnoteSeqBefore(int position, int *last) const;
    void endnoteSeqMove(qsizetype rank, int by);
    void followEndnoteReferences(int position, int removed, int added);
    // An endnote cited more than once is set once for each citation: the first copy is the one whose words are edited, and the others show them. After an edit in a note that has others
    // they are set again (at the end of the edit, as one step with it).
    QHash<int, int> m_endnoteCopyCount;   // by Id: how many copies of the note the pages set
    QSet<int> m_endnoteCopiesStale;       // the Ids of the notes whose first copy was edited and that have other copies
    QHash<int, QVector<int>> m_endnoteCopiesOf;   // by Id: the numbers (EndnoteNote) of the copies the pages set, in the order of the story (the first is the one whose words are edited)
    // Whether the notes the pages set are one run of plain paragraphs at the story's end (none at the end of a section, none cut for room, no table or other element kept as XML in them): a
    // copy of a note is then found by its number, as the numbers grow with the story, and set again alone (refreshEndnoteCopiesInPlace).
    bool m_endnoteTailLocal = false;
    bool m_refreshingCopies = false;      // the shown copies of an edited note are being set again, each alone (the edits that does touch no section, no note's words, and no reference)
    bool refreshEndnoteCopiesInPlace(const QSet<int> &ids);
    QVector<EndnoteSeqEntry> endnoteReferencesIn(int from, int to);
    bool countedEndnoteReference(const QTextCharFormat &format, int *id) const;
    // The endnotes (writedoc_noteedit.cpp)
    QHash<int, int> m_endnoteCopyIds;                                 // the Id of the note each copy that the pages set is a copy of (by the number EndnoteNote gives it)
    int m_nextEndnoteSerial = 1;
    int m_endnotePartHighest = 0;                                     // the same for the endnotes part
    QSet<int> m_createdEndnotes, m_endnotesCitedAtOpen, m_endnoteEdited;   // made since the document was read; cited by its text when it was read; edited (some of its paragraphs were touched) since
    QHash<int, QByteArray> m_endnoteOriginal;                         // the file's notes, by Id: a fingerprint of the blocks as read (to tell a note that was touched and is as it was)
    NotesPart m_endnotesPart = NotesPart::Absent;
    bool m_endnotesPartNew = false;
    bool m_endnoteCapped = false;                                     // some of the notes the text cites are not set (too many, or too long)
    bool m_endnoteLast = false;                                       // the last section says it has no endnotes
    bool m_buildingEndnotes = false;                                  // the notes the pages set are being made again (the edits that does are not edits of the notes)
    QHash<int, QVector<int>> m_endnoteRefIndex;
    bool m_endnoteRefIndexValid = false;
    void indexEndnoteReferences();
    int nextEndnoteId();
    void activateEndnotes();
    void captureEndnotes(const QSet<int> *only = nullptr);
    bool tailsOf(const QTextDocument *d) const;
    void touchEndnotes(int position, int removed, int added);
    void writeEndnoteParts(office::wml::Document *out);
    int nextFootnoteId();
    void activateNotes();
    void noteEdited(QTextDocument *story);
    void writeNoteParts(office::wml::Document *out);
    // The settings part of a package being written (made, with its relationship and content type, where there is none).
    office::wml::KeptPart *settingsPartFor(office::wml::Document *out);
    // A header or footer part: its story (made when a page needs it), and what writing it back needs.
    struct HeaderPart {
        QString name;                       // "/word/header1.xml"
        bool footer = false;
        std::shared_ptr<QTextDocument> story;
        QByteArray rootStart;               // the part's root as read (empty for one the program made)
        QString relationship;               // the Id of the main part's relationship to it
        bool editForm = false;              // the story is in the form it is edited in
        bool created = false;               // made by the program: there is no such part in the file
        bool unreadable = false;            // the part is there but could not be read (not XML): it is kept as it is
        bool missing = false;               // the file has no such part: the reference to it names nothing
    };
    QHash<QString, HeaderPart> m_headerParts;                          // by part name
    // The history: a step is a story and the number that tells it from the others.
    // (`raw`: the story's count of undo commands when the step was logged; Qt says twice of a step that an edit joined to the block of the one before it, and the entry of commands
    // that an undo has taken away is none.)
    struct Step {
        QTextDocument *story = nullptr;
        quint64 serial = 0;
        int raw = 0;
        quint64 group = 0;   // the steps made while a StepGroup lived have the same: they are undone and done again together
    };
    QVector<Step> m_undoLog, m_redoLog;
    quint64 m_openGroup = 0, m_nextGroup = 1;
    int m_groupDepth = 0;
    QTextDocument *undoOne(QTextCursor *cursor);
    quint64 m_nextSerial = 1, m_cleanSerial = 0;
    bool m_forcedDirty = false;
    bool m_wasModified = false;
    QString m_pageColorAtOpen;                                         // the file's page color ("RRGGBB") when it was read
    int m_hyphenZoneAtOpen = 360, m_hyphenLimitAtOpen = 0;
    bool m_noHyphenCapsAtOpen = false;
    bool m_mirrorAtOpen = false, m_hyphenAtOpen = false;               // the file's facing pages with mirrored margins, and automatic hyphenation, when it was read
    bool m_oddEvenAtOpen = false;                                      // the file's use of different headers on odd and even pages when it was read
    QSet<QString> m_released;                                          // parts the sections let go of (a reference taken out): written no more if nothing names them
    // The empty header or footer shown to be typed in that is not made in the document yet (createHeaderFooter with `lazy`): its part, and the place it is shown in.
    struct Pending {
        QString part;
        int section = -1, slot = 0;
        bool footer = false;
    } m_pending;
    bool commitPending();
    bool m_dropCapsAsFrames = true;   // the drop caps made here are written as a paragraph of their own set as a frame (.docx); off for the .odt, which would read them as two paragraphs
    bool m_committing = false;   // the document is being changed to make the header or footer that was shown to be typed in (the story being typed in is not sealed by it)
    // Takes the styles that making a header or footer added out of the styles to write when no paragraph written has them.
    void dropHeaderStyles(office::wml::Document *out) const;
    // The styles that making a header or footer added to the document (Header and Footer, and what they are based on): they are written only for a header or footer that is kept.
    QSet<QString> m_headerStyles;
    // Changes to the style sheet by the edit now being made, and by each step of the history that made one (by its serial): freed with the step, when nothing can reach it.
    struct StyleChange {
        QString id;
        std::optional<office::wml::Style> before, after;   // none: there was no such style, or there is none now
        bool modifiedBefore = false, modifiedAfter = true; // whether the sheet counted as changed (it is written anew when it is)
    };
    quint64 m_styleRev = 0;
    QVector<StyleChange> m_pendingStyleChanges;
    QHash<quint64, QVector<StyleChange>> m_styleSteps;
    void applyStyleChanges(quint64 serial, bool undo);
    void applyChanges(const QVector<StyleChange> &changes, bool undo);
    mutable QHash<QString, std::shared_ptr<office::wml::StyleSheet>> m_scratchHeld;   // the cache of the sheet swapped out by swapStyleSheet
    mutable qint64 m_sheetCopies = 0, m_inUseScans = 0;
    mutable int m_inUseRevision = -1;
    mutable const QTextDocument *m_inUseStory = nullptr;
    mutable QSet<QString> m_inUse;
    qsizetype m_stylesCountAtOpen = 0;
    bool m_stylesModifiedAtOpen = false;
    bool partEdited(const HeaderPart &part) const;
    void renewIds(QTextDocument *copy);
    // Headers and footers (writedoc_headers.cpp)
    HeaderPart *partOfStory(const QTextDocument *story);
    const HeaderPart *partOfStory(const QTextDocument *story) const;
    // Puts a part's story in the form it is edited in; false for a part that can't be.
    bool makeEditable(HeaderPart &part);
    // Takes a story into the history and tells the page layout of its edits.
    void watchHistory(QTextDocument *story);
    void stepAdded(QTextDocument *story);
    void storyEdited(QTextDocument *story);
    void headerEdited(QTextDocument *story);
    // After a story is edited, none of the other stories' next edits joins their last step (it would not be in the order the edits were made in).
    void sealStories();
    void checkModified();
    // A new part, with its relationship and a story to show; the section's reference to it is not made.
    HeaderPart *addHeaderPart(bool footer, std::shared_ptr<QTextDocument> story, const QString &copyRelationshipsOf = QString(), bool withRelationship = true);
    QString newPartName(bool footer) const;
    void writeHeaderParts(office::wml::Document *out);
    QByteArray headerPartXml(const HeaderPart &part);
    bool partExists(const QString &name) const;
    void finishEditable(HeaderPart &part);
    QHash<int, office::wml::HeaderRefs> m_headerRefs;                 // by the key of the section's record
    QByteArray m_lastBytes;                        // the first paragraph's record of it (prop::LastSection) as last read
    int m_blockCount = 0;
    QString m_endnoteFormat = QStringLiteral("lowerRoman");   // the format the settings give the endnotes' numbers (lower case roman where they give none)
    int m_endnoteStart = 1;                                    // and the number the first begins with
    QString m_endnotePosition;                                 // where the settings put the endnotes (w:pos): empty, or docEnd, at the document's end
    int m_endnoteSections = 1;                                 // the sections the text has
    bool m_endnotesAtSectionEnds = false;                      // some section's endnotes are set at its end
    bool m_tailsStripped = false;                              // the endnotes were taken out of the story, to be set again
    const office::wml::SectionProps *m_tailSection = nullptr;  // the section whose endnotes are being set (its page's width is the separator's), or none: the last
    int m_boxTableCells = 0;   // the cells of the tables that the text boxes drawn so far hold (a document's boxes have kMostBoxTableCells of them in all: the rest are kept as they were)
    int m_letteringShapes = 0, m_letteringLetters = 0;   // the lettering on a path the document has drawn so far, and its letters (each is outlined, and may be stroked: they are bounded)
    bool m_syncing = false;
    bool m_trackChanges = false, m_trackAtOpen = false;
    std::optional<office::wml::DocumentProtection> m_protection, m_protectionAtOpen;
    QString m_reviewer = QStringLiteral("Author"), m_reviewerInitials;
    QStringList m_reviewers;
    QHash<QString, int> m_reviewerPlaces;   // (the place of each, to find one without looking through the list)
    int m_nextRevisionId = 0;   // 0: not looked for yet
    int m_revisionView = 0;
    bool m_fieldCodesAll = false;
    bool m_mayHaveFields = false;
    QSet<int> m_fieldCodeIds;
    int m_fieldCodesCount = 0;
    int m_fieldCodeView = 0;
    bool m_showHidden = false;
    int hiddenBit() const { return m_showHidden ? tp::kRevisionShowHidden : 0; }
    void writeReviewSettings(office::wml::Document *out);
    void writeSourcesPart(office::wml::Document *out);
    void writeNoteSettings(office::wml::Document *out);
    void applyFootnoteSettings();
    NoteSettings m_noteEdit[2];
    bool m_noteEdited[2] = {false, false};
    mutable NoteSettings m_noteRead[2];   // (what the settings part says, read once: the part does not change until the file is written)
    mutable bool m_noteReadDone[2] = {false, false};
    bool m_endnoteRestartSections = false;   // the endnotes' numbers begin again with each section
    QByteArray m_sourcesXml;
    QString m_sourcesPartName;
    bool m_sourcesRead = false, m_sourcesDirty = false;
    quint64 m_sourcesRevision = 0;
    void readMailSettings(const QByteArray &settings);
    void writeMailSettings(office::wml::Document *out);
    void writeCommentParts(office::wml::Document *out);
    mutable QHash<QString, std::shared_ptr<office::wml::StyleSheet>> m_scratch;
    QVector<office::wml::Style> m_addedTableStyles;                    // built-in table styles that tables made have (see ensureTableStyle)
    mutable std::shared_ptr<office::wml::StyleSheet> m_tableSheet;
    bool levelIsOutlined(const office::wml::Level &level, int key) const;
    mutable QHash<int, bool> m_levelOutlines;   // by definition and level (or, for a level a numbering id writes out itself, by the id and level): whether the level's bullets are outlined
    friend class Loader;
    friend class Saver;
};

// The section a flowing document shows for a section the file states, and back.
flow::Section flowSectionOf(const office::wml::SectionProps &props, bool evenAndOdd);
void updateSectionProps(office::wml::SectionProps *props, const flow::Section &section);

// Properties of the story's formats whose values are bytes (the XML a paragraph or run keeps as it was written): whether a property is one,
// and the bytes a format's value stands for.
bool isStoredProperty(int property);
QVariant resolveStored(const WriteDoc &doc, const QVariant &value, int property);

QByteArray packSection(const office::wml::SectionProps &props);
office::wml::SectionProps unpackSection(const QByteArray &packed);

} // namespace jp::write

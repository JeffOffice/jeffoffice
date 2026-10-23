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

#include "core/tablemodel.h"
#include "flow/flowdocument.h"
#include "flow/paginator.h"
#include "office/package.h"
#include "wml/wml.h"

#include <QHash>
#include <QSet>
#include <QObject>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextListFormat>
#include <memory>

namespace jp::write {

// A style as the style gallery lists it.
struct StyleEntry {
    QString id, name;
    bool character = false;
    int priority = 99;
    bool quick = false;     // shown in the gallery
};

class WriteDoc;

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
    // A document from the bytes of a file of either kind: an old binary file (a compound file) is read as one, anything else as a .docx.
    static std::unique_ptr<WriteDoc> fromFile(const QByteArray &bytes, QString *error = nullptr, QStringList *notes = nullptr);
    // The story written back into a package (empty, with the reason, if it can't be made). `template_` saves it as a template.
    QByteArray toDocx(QString *error = nullptr, bool template_ = false);

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
    // Different first page (a section's) and different odd and even pages (the document's), as one step of the history each.
    bool differentFirstPage(int section) const;
    void setDifferentFirstPage(int section, bool on);
    bool differentOddEven() const { return m_wml.evenAndOddHeaders; }
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
    // A copy of a footnote as a note of its own (what a pasted copy of a reference cites): its Id, 0 for none.
    int duplicateFootnote(int id);
    // The look and the pieces of a reference to footnote `id`: its character format (the reference character style over the look of a paragraph of this style), to put
    // where the reference is.
    QTextCharFormat footnoteReferenceFormat(int id, const QString &paragraphStyle);
    // Whether the story is one the document's header or footer, or a note, has: the stories that take no page break, section break, or note.
    bool isSideStory(const QTextDocument *story) const { return isHeaderFooterStory(story) || footnoteIdOf(story); }

    // ---- one history
    // The steps of the text and of the headers and footers in the order they were made: an undo undoes the last, wherever it was, and a redo does it again. Returns the story the
    // step was in (null when there was none); the cursor, if given, is put where the step changed the story.
    bool canUndo() const { return !m_undoLog.isEmpty(); }
    bool canRedo() const { return !m_redoLog.isEmpty(); }
    QTextDocument *undo(QTextCursor *cursor = nullptr);
    QTextDocument *redo(QTextCursor *cursor = nullptr);

    // ---- styles
    const office::wml::StyleSheet &styleSheet() const { return m_wml.styles; }
    QString defaultParagraphStyle() const;
    QVector<StyleEntry> styleEntries() const;
    // The style the document has under `id`, adding a built-in one it lacks; false when there is none by that id.
    bool ensureStyle(const QString &id);
    // The whole of a paragraph style's look: the paragraph's format and its run's, with the document's defaults under them.
    QTextBlockFormat styleBlockFormat(const QString &paragraphStyle);
    QTextCharFormat styleCharFormat(const QString &paragraphStyle, const QString &characterStyle = QString());
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
    // The Id of a relationship of the main part to an address outside the file, made if the document has none like it.
    QString addExternalLink(const QString &type, const QString &target);
    // The Id of an image relationship to a new part with these bytes (named image1.png and on, in word/media).
    QString addMedia(const QByteArray &bytes, const QString &extension, const QString &contentType);

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
    void exportInto(BlockSink &sink, QTextDocument *story = nullptr);
    // What writing gives a run of this format, in a paragraph of this style (and table layer): the properties it has beyond its style's.
    office::wml::RunProps runPropsWritten(const QTextCharFormat &format, const QString &styleId, const office::wml::TableLayer *layer) const;
    void settleTables();
    void setUpEnvironment();
    void storyChanged(int position, int removed, int added);
    // The style sheet to read a style's look from: the document's, or for a built-in style it lacks, a copy with that style added.
    const office::wml::StyleSheet &sheetFor(const QString &styleId) const;
    office::wml::SectionProps lastSectionProps() const;

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
    QSet<int> m_createdFootnotes;                                     // the footnotes made since the document was read (written while the text cites them)
    QSet<int> m_footnotesCitedAtOpen;                                 // those of the file the text cited when it was read (one whose last reference was taken out is not written)
    enum class NotesPart { Absent, Readable, Unusable };
    NotesPart m_footnotesPart = NotesPart::Absent;                    // the file's footnotes part: none, one the program reads, or one it cannot (not XML, or no part by its name)
    bool m_footnotesPartNew = false;                                  // the part named by m_notePart is one the program will make (there is none in the file)
    QHash<int, QVector<int>> m_footnoteRefs;                          // the references of the text, by the note's Id, found when asked and let go with any change to the text
    bool m_footnoteRefsValid = false;
    void indexFootnoteReferences();
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
    };
    QVector<Step> m_undoLog, m_redoLog;
    quint64 m_nextSerial = 1, m_cleanSerial = 0;
    bool m_forcedDirty = false;
    bool m_wasModified = false;
    bool m_oddEvenAtOpen = false;                                      // the file's use of different headers on odd and even pages when it was read
    QSet<QString> m_released;                                          // parts the sections let go of (a reference taken out): written no more if nothing names them
    // The empty header or footer shown to be typed in that is not made in the document yet (createHeaderFooter with `lazy`): its part, and the place it is shown in.
    struct Pending {
        QString part;
        int section = -1, slot = 0;
        bool footer = false;
    } m_pending;
    bool commitPending();
    bool m_committing = false;   // the document is being changed to make the header or footer that was shown to be typed in (the story being typed in is not sealed by it)
    // Takes the styles that making a header or footer added out of the styles to write when no paragraph written has them.
    void dropHeaderStyles(office::wml::Document *out) const;
    // The styles that making a header or footer added to the document (Header and Footer, and what they are based on): they are written only for a header or footer that is kept.
    QSet<QString> m_headerStyles;
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
    mutable QHash<QString, std::shared_ptr<office::wml::StyleSheet>> m_scratch;
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

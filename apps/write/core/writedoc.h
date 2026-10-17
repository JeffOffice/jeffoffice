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
    double separator = 0, notice = 0;
    double separatorMeasured = 0, noticeMeasured = 0;   // the heights as the file sets them; `separator` and `notice` are those cut to what the pages can spare (WriteDoc::limitNoteSeparator)

private:
    WriteDoc *m_doc;
};

class WriteDoc : public QObject {
    Q_OBJECT
public:
    ~WriteDoc() override;

    // A new document: one empty paragraph, Normal style, paper of the region's size.
    static std::unique_ptr<WriteDoc> blank();
    // A document from a .docx package; null, with the reason, if it can't be read. `notes` get what was left out.
    static std::unique_ptr<WriteDoc> fromDocx(const QByteArray &zip, QString *error = nullptr, QStringList *notes = nullptr);
    // The story written back into a package (empty, with the reason, if it can't be made). `template_` saves it as a template.
    QByteArray toDocx(QString *error = nullptr, bool template_ = false);

    flow::FlowDocument &flow() { return *m_flow; }
    QTextDocument *story() const { return m_flow->story(); }
    flow::Paginator &pager() { return *m_pager; }

    // ---- sections
    // The sections in order; the last has no break paragraph. Changing one writes it into its break paragraph, so it undoes with the text.
    int sectionCount() const { return int(m_sections.size()); }
    office::wml::SectionProps sectionProps(int index) const;
    void setSectionProps(int index, const office::wml::SectionProps &props);
    // The section the document position is in.
    int sectionAt(int position) const;
    // How many sections have been made from their records since the document was opened (to see what an edit costs).
    qint64 sectionsMadeForTest() const { return m_sectionsMade; }
    // The paragraph ends the section: the break paragraphs and `flow().sections` are made again from the story. Called after edits that
    // add or remove paragraphs (the document does it itself when the story reports one).
    void syncSections();
    // Makes the paragraph at `block` the last of a new section that is like the one it is in (a break); false if it ends one already.
    bool insertSectionBreak(const QTextBlock &block);

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
    bool isModified() const { return story()->isModified(); }
    void setModified(bool modified) { story()->setModified(modified); }
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

private:
    WriteDoc();
    void build(office::wml::Document &&doc);
    void buildStory(const QVector<office::wml::Block> &body);
    void fillStory(QTextDocument *story, const QVector<office::wml::Block> &body, bool header, const QString &part = QString());
    // Makes of a drawn object of a run (its atom) what the pages show: a picture in the line is an inline object of the character, the rest are objects set at
    // places on the page of the paragraph they are anchored to. False when it makes none (nothing drawn, or a picture the file does not hold).
    bool drawnObjects(const office::wml::Inline &atom, const QString &part, QTextCharFormat *format);
    // The story of the header or footer part a relationship names (made once, kept); none for a link that is not a part, or past the limit of parts.
    std::shared_ptr<QTextDocument> headerStory(const QString &relationshipId);
    // Gives the flowing sections from `from` on the stories of their headers and footers, those they state and the section before's for the rest.
    void attachHeaders(int from);
    // The footnotes of the file (its footnotes part), kept as what they hold and made into stories when a page asks for one ("fn" and the note's Id).
    void loadNotes();
    void measureNoteSeparator();
    void limitNoteSeparator();
    const QTextDocument *noteStory(const QString &id);
    friend class NoteFields;
    void exportBody(QVector<office::wml::Block> *body);
    void exportInto(BlockSink &sink);
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
    std::unique_ptr<NoteFields> m_noteFields;
    QString m_notePart;                                               // the footnotes part's name
    QHash<QString, QVector<office::wml::Block>> m_noteBlocks;         // by "fn" and Id
    QVector<office::wml::Block> m_noteNotice;                         // the paragraphs of the file's continuation notice (none: no room is kept for one)
    QVector<office::wml::Block> m_noteSeparator;                      // the paragraphs of the file's note separator (none: a line of the Normal style)
    // (The stories of the notes and of the headers and footers are made when a page needs them and are not edited: nothing in the program reaches them yet. When
    // something does, the paginator has to be told as it is of the story's edits (its measures of them are kept by the story's revision), and the pages that show
    // them made again.)
    QHash<QString, std::shared_ptr<QTextDocument>> m_noteStories;     // the stories made of them
    QHash<QString, std::shared_ptr<QTextDocument>> m_headerStories;   // by part name
    QHash<int, office::wml::HeaderRefs> m_headerRefs;                 // by the key of the section's record
    QByteArray m_lastBytes;                        // the first paragraph's record of it (prop::LastSection) as last read
    int m_blockCount = 0;
    bool m_syncing = false;
    mutable QHash<QString, std::shared_ptr<office::wml::StyleSheet>> m_scratch;
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

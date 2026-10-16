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

#include "flow/flowdocument.h"
#include "flow/paginator.h"
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
    office::wml::ParaProps baselineParagraph(const QString &paragraphStyle, int numId, int level) const;

    // ---- lists
    const office::wml::Numbering &numbering() const { return m_wml.numbering; }
    // The list definition (w:numId) for a new list of plain bullets or numbers, made if need be. Bullets share one definition; each new
    // numbered list is a definition of its own, so that it starts at 1.
    int newListInstance(bool numbered);
    // The Qt list format of a level of a definition; `marker` gets the text of a marker that is not Qt's to count (set in legal style, or
    // numbered with several levels in it).
    QTextListFormat listFormat(int numId, int level, int start = -1) const;
    office::wml::ParaProps levelParagraph(int numId, int level) const;   // the indents the level gives its paragraphs

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

Q_SIGNALS:
    void sectionsChanged();

private:
    WriteDoc();
    void build(office::wml::Document &&doc);
    void buildStory(const QVector<office::wml::Block> &body);
    void exportBody(QVector<office::wml::Block> *body);
    void setUpEnvironment();
    void storyChanged(int position, int removed, int added);
    // The style sheet to read a style's look from: the document's, or for a built-in style it lacks, a copy with that style added.
    const office::wml::StyleSheet &sheetFor(const QString &styleId) const;
    office::wml::SectionProps lastSectionProps() const;

    std::unique_ptr<flow::FlowDocument> m_flow;
    std::unique_ptr<flow::Paginator> m_pager;
    office::wml::Document m_wml;                   // everything of the file but the body
    std::vector<office::wml::SectionProps> m_sections;   // the sections (the last one's is in m_last)
    office::wml::SectionProps m_last;
    int m_blockCount = 0;
    bool m_syncing = false;
    mutable QHash<QString, std::shared_ptr<office::wml::StyleSheet>> m_scratch;
    friend class Loader;
    friend class Saver;
};

// The section a flowing document shows for a section the file states, and back.
flow::Section flowSectionOf(const office::wml::SectionProps &props, bool evenAndOdd);
void updateSectionProps(office::wml::SectionProps *props, const flow::Section &section);

QByteArray packSection(const office::wml::SectionProps &props);
office::wml::SectionProps unpackSection(const QByteArray &packed);

} // namespace jp::write

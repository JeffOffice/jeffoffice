// The footnotes of a file: read from its footnotes part, made into stories for the pages that cite them.

#include "core/writedoc.h"

#include "text/textengine.h"
#include "text/textprops.h"

#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {
// The most notes of a file that are made into stories (the rest is left out: a reference to one shows only its number).
constexpr int kMostNoteStories = 5000;
} // namespace

const QTextDocument *NoteFields::storyDoc(const QString &id) const { return m_doc->noteStory(id); }

QList<const QTextDocument *> NoteFields::storyDocs() const
{
    QList<const QTextDocument *> list;
    for (const auto &d : m_doc->m_noteStories) list << d.get();
    return list;
}

void WriteDoc::loadNotes()
{
    m_noteBlocks.clear();
    m_noteStories.clear();
    m_noteFields.reset();
    m_flow->fields = nullptr;
    QString part;
    for (const Relationship &r : m_wml.mainRelationships)
        if (!r.external && r.type.endsWith(QLatin1String("/footnotes"))) {
            part = r.target;
            m_notePart = r.target;
            break;
        }
    if (part.isEmpty()) return;
    QVector<NotePart> notes;
    if (!office::wml::readNotes(partData(part), &notes)) return;
    for (NotePart &n : notes)
        if (n.type.isEmpty() || n.type == QLatin1String("normal")) m_noteBlocks.insert(QStringLiteral("fn%1").arg(n.id), std::move(n.blocks));
    if (m_noteBlocks.isEmpty()) return;
    m_noteFields = std::make_unique<NoteFields>(this);
    m_flow->fields = m_noteFields.get();
    // The notes' numbers begin again in each section when the file's settings say so.
    const QByteArray settings = m_wml.settingsName.isEmpty() ? QByteArray() : partData(m_wml.settingsName);
    static const QRegularExpression restart(QStringLiteral("<w:footnotePr>.*?<w:numRestart [^>]*w:val=\"eachSect\""), QRegularExpression::DotMatchesEverythingOption);
    m_flow->restartNotesInSections = restart.match(QString::fromUtf8(settings)).hasMatch();
}

// The paragraph the pages set between their text and their notes (the file's own, holding the rule): a line of the Normal style and its space after.
void WriteDoc::measureNoteSeparator()
{
    if (!m_noteFields) return;
    const QString style = defaultParagraphStyle();
    QTextDocument d;
    d.setDocumentMargin(0);
    d.setDefaultFont(story()->defaultFont());
    QTextCursor c(&d);
    const QTextBlockFormat bf = styleBlockFormat(style);
    c.setBlockFormat(bf);
    c.setBlockCharFormat(styleCharFormat(style));
    FrameSpec fs;
    fs.size = QSizeF(400, 10000);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.hyphenate = false;
    StoryLayout layout;
    layout.build(&d, {fs}, m_flow->env);
    const auto lines = layout.lineInfo(0);
    m_noteFields->separator = lines.isEmpty() ? 0.0 : lines.first().rect.height() + bf.bottomMargin();
}

const QTextDocument *WriteDoc::noteStory(const QString &id)
{
    const auto made = m_noteStories.constFind(id);
    if (made != m_noteStories.constEnd()) return made.value().get();
    const auto blocks = m_noteBlocks.constFind(id);
    if (blocks == m_noteBlocks.constEnd() || m_noteStories.size() >= kMostNoteStories) return nullptr;
    auto doc = std::make_shared<QTextDocument>();
    doc->setDocumentMargin(0);
    doc->setDefaultFont(story()->defaultFont());
    doc->setUndoRedoEnabled(false);
    fillStory(doc.get(), blocks.value(), true, m_notePart);
    m_noteStories.insert(id, doc);
    return doc.get();
}

} // namespace jp::write

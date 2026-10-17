// The footnotes of a file: read from its footnotes part, made into stories for the pages that cite them.

#include "core/writedoc.h"

#include "text/textengine.h"
#include "text/textprops.h"

#include <QTextBlock>
#include <algorithm>
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
    m_noteSeparator.clear();
    m_noteNotice.clear();
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
    for (NotePart &n : notes) {
        if (n.type.isEmpty() || n.type == QLatin1String("normal")) m_noteBlocks.insert(QStringLiteral("fn%1").arg(n.id), std::move(n.blocks));
        else if (n.type == QLatin1String("separator")) m_noteSeparator = std::move(n.blocks);
        else if (n.type == QLatin1String("continuationNotice")) m_noteNotice = std::move(n.blocks);
    }
    if (m_noteBlocks.isEmpty()) return;
    m_noteFields = std::make_unique<NoteFields>(this);
    m_flow->fields = m_noteFields.get();
    // The notes' numbers begin again in each section when the file's settings say so.
    const QByteArray settings = m_wml.settingsName.isEmpty() ? QByteArray() : partData(m_wml.settingsName);
    m_flow->restartNotesInSections = office::wml::noteNumberRestart(settings, "footnotePr") == QLatin1String("eachSect");
}

// The paragraph the pages set between their text and their notes (the file's own, holding the rule): its height as the file sets it, or, for a file
// without one, a line of the Normal style and its space after.
void WriteDoc::measureNoteSeparator()
{
    if (!m_noteFields) return;
    QTextDocument d;
    d.setDocumentMargin(0);
    d.setDefaultFont(story()->defaultFont());
    double after = 0;
    if (!m_noteSeparator.isEmpty()) {
        d.setUndoRedoEnabled(false);
        fillStory(&d, m_noteSeparator, true, m_notePart);
        after = d.lastBlock().blockFormat().bottomMargin();
    } else {
        const QString style = defaultParagraphStyle();
        QTextCursor c(&d);
        const QTextBlockFormat bf = styleBlockFormat(style);
        c.setBlockFormat(bf);
        c.setBlockCharFormat(styleCharFormat(style));
        after = bf.bottomMargin();
    }
    FrameSpec fs;
    fs.size = QSizeF(400, 10000);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.hyphenate = false;
    StoryLayout layout;
    layout.build(&d, {fs}, m_flow->env);
    const auto lines = layout.lineInfo(0);
    // (The file's own paragraph, with the space before it, to the bottom of its last line; the Normal style's line is the line and the space after it.)
    m_noteFields->separatorMeasured = lines.isEmpty() ? 0.0 : (m_noteSeparator.isEmpty() ? lines.first().rect.height() : lines.last().rect.bottom()) + after;
    limitNoteSeparator();
    // The continuation notice: the height of its paragraphs, which a page keeps under its notes.
    m_noteFields->noticeMeasured = 0;
    if (!m_noteNotice.isEmpty()) {
        QTextDocument n;
        n.setDocumentMargin(0);
        n.setDefaultFont(story()->defaultFont());
        n.setUndoRedoEnabled(false);
        fillStory(&n, m_noteNotice, true, m_notePart);
        StoryLayout noticeLayout;
        noticeLayout.build(&n, {fs}, m_flow->env);
        const auto noticeLines = noticeLayout.lineInfo(0);
        m_noteFields->noticeMeasured = noticeLines.isEmpty() ? 0.0 : noticeLines.last().rect.bottom() + n.lastBlock().blockFormat().bottomMargin();
    }
}

// A separator or a continuation notice taller than a quarter of the text area of the smallest page is cut to that quarter, for it would leave a page no room for its text.
void WriteDoc::limitNoteSeparator()
{
    if (!m_noteFields) return;
    double least = 0;
    for (const flow::Section &s : m_flow->sections) {
        const double body = s.pageSize().height() - s.margins.top() - s.margins.bottom();
        if (body > 0 && (least == 0 || body < least)) least = body;
    }
    const double separator = least > 0 ? std::min(m_noteFields->separatorMeasured, least / 4) : m_noteFields->separatorMeasured;
    const double notice = least > 0 ? std::min(m_noteFields->noticeMeasured, least / 4) : m_noteFields->noticeMeasured;
    if (separator == m_noteFields->separator && notice == m_noteFields->notice) return;
    m_noteFields->separator = separator;
    m_noteFields->notice = notice;
    if (m_pager) m_pager->invalidateFrom(0);
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

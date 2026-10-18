// The footnotes of a file: read from its footnotes part, made into stories for the pages that cite them.

#include "core/writedoc.h"

#include "text/textengine.h"
#include "text/textprops.h"

#include <QFontMetricsF>
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
const QTextDocument *NoteFields::noteNoticeStory() const { return m_doc->noticeStory(); }
const QTextDocument *NoteFields::noteContinuationStory() const { return m_doc->continuationStory(); }

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
    m_noteContinuation.clear();
    m_noteNotice.clear();
    m_noteStories.clear();
    m_noticeStory.reset();
    m_continuationStory.reset();
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
        else if (n.type == QLatin1String("continuationSeparator")) m_noteContinuation = std::move(n.blocks);
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
    m_noteFields->separator = lines.isEmpty() ? 0.0 : (m_noteSeparator.isEmpty() ? lines.first().rect.height() : lines.last().rect.bottom()) + after;
    // The continuation notice: the height of its paragraphs, which a page keeps under its notes.
    auto heightOf = [&](const QVector<office::wml::Block> &blocks) {
        QTextDocument n;
        n.setDocumentMargin(0);
        n.setDefaultFont(story()->defaultFont());
        n.setUndoRedoEnabled(false);
        fillStory(&n, blocks, true, m_notePart);
        StoryLayout part;
        part.build(&n, {fs}, m_flow->env);
        const auto partLines = part.lineInfo(0);
        return partLines.isEmpty() ? 0.0 : partLines.last().rect.bottom() + n.lastBlock().blockFormat().bottomMargin();
    };
    m_noteFields->notice = m_noteNotice.isEmpty() ? 0.0 : heightOf(m_noteNotice);
    // The separator of a note that goes on from the page before: the file's own paragraph for it (its text and rule), which a page sets above that part of the note.
    m_noteFields->continuation = m_noteContinuation.isEmpty() ? 0.0 : heightOf(m_noteContinuation);
    // Where the rule is in it: at three fifths of the line that holds the separator's character, after the words that stand before it in the line.
    m_noteFields->ruleAt = -1;
    m_noteFields->ruleFrom = 0;
    m_noteFields->ruleFull = true;
    if (const QTextDocument *c = continuationStory()) {
        StoryLayout part;
        part.build(c, {fs}, m_flow->env);
        const auto partLines = part.lineInfo(0);
        QTextBlock block = c->begin();
        for (const Block &b : m_noteContinuation) {
            if (b.isRaw || !b.tag.isEmpty()) continue;
            QString before;
            bool found = false;
            for (const Inline &in : b.para.content) {
                if (in.kind == Inline::RunAtom && (in.tag == QLatin1String("separator") || in.tag == QLatin1String("continuationSeparator"))) {
                    found = true;
                    m_noteFields->ruleFull = in.tag == QLatin1String("continuationSeparator");
                    break;
                }
                if (in.kind == Inline::Text) before += in.text;
            }
            if (found && block.isValid()) {
                for (const auto &li : partLines)
                    if (li.docStart == block.position()) {
                        m_noteFields->ruleAt = li.rect.top() + li.rect.height() * 0.6;
                        break;
                    }
                if (!before.trimmed().isEmpty() && !block.begin().atEnd())
                    m_noteFields->ruleFrom = QFontMetricsF(block.begin().fragment().charFormat().font()).horizontalAdvance(before) + 3;
                break;
            }
            block = block.next();
        }
    }
}

// The file's continuation notice, or its separator for the part of a note that goes on, as a story for a page to set (none where the file has none).
const QTextDocument *WriteDoc::noticeStory()
{
    if (m_noteNotice.isEmpty()) return nullptr;
    if (!m_noticeStory) {
        m_noticeStory = std::make_unique<QTextDocument>();
        m_noticeStory->setDocumentMargin(0);
        m_noticeStory->setDefaultFont(story()->defaultFont());
        m_noticeStory->setUndoRedoEnabled(false);
        fillStory(m_noticeStory.get(), m_noteNotice, true, m_notePart);
    }
    return m_noticeStory.get();
}

const QTextDocument *WriteDoc::continuationStory()
{
    if (m_noteContinuation.isEmpty()) return nullptr;
    if (!m_continuationStory) {
        m_continuationStory = std::make_unique<QTextDocument>();
        m_continuationStory->setDocumentMargin(0);
        m_continuationStory->setDefaultFont(story()->defaultFont());
        m_continuationStory->setUndoRedoEnabled(false);
        fillStory(m_continuationStory.get(), m_noteContinuation, true, m_notePart);
    }
    return m_continuationStory.get();
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

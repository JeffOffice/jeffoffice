// The footnotes of a file: read from its footnotes part, made into stories for the pages that cite them.

#include "core/writedoc.h"

#include "text/textengine.h"
#include "text/textprops.h"

#include <QTextBlock>
#include <QTextList>
#include <algorithm>
#include <atomic>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {
// The most notes of a file that are made into stories (the rest is left out: a reference to one shows only its number).
constexpr int kMostNoteStories = 5000;
// The most of the file's continuation notice, and of its separator for the part of a note that goes on, that is read: the paragraphs, and the characters of them. (The pages set a
// quarter of a page of each at most, so the rest would only be copied to every page.)
constexpr int kMostNoticeParagraphs = 64;
constexpr int kMostNoticeCharacters = 4000;
} // namespace

const QTextDocument *NoteFields::storyDoc(const QString &id) const { return m_doc->noteStory(id); }
const QTextDocument *NoteFields::noteNoticeStory() const { return m_doc->noticeStory(); }
const QTextDocument *NoteFields::noteContinuationStory() const { return m_doc->continuationStory(); }

namespace {
std::atomic<qint64> s_styleRefParagraphs{0};
// The most paragraph texts and style names that are kept for the fields that show a paragraph style's text (the rest are made again).
constexpr int kMostStyleRefKept = 20000;
} // namespace

qint64 styleRefParagraphsScannedForTest() { return s_styleRefParagraphs.load(); }
void resetStyleRefParagraphsScannedForTest() { s_styleRefParagraphs = 0; }

QString NoteFields::styleRefText(const QString &style, const QString &switches, int from, int to) const
{
    const QTextDocument *story = m_doc->story();
    // What is kept for the story as it is: where the paragraphs of each style begin (found in one pass over the story, for every style a page's fields ask for), which
    // styles a name says, and the texts of the paragraphs found.
    const int revision = story->revision();
    const int styles = int(m_doc->m_wml.styles.styles.size());
    if (m_styleRevision != revision || m_styleCount != styles) {
        m_styleRevision = revision;
        m_styleCount = styles;
        m_styleStarts.clear();
        m_styleIndexMade = false;
        m_styleIds.clear();
        m_styleTexts.clear();
    }
    // The styles the name says: its name (as the style sheet has it, whatever the case, or without the blanks of a built-in style's id) or its id.
    auto ids = m_styleIds.find(style);
    if (ids == m_styleIds.end()) {
        const QString plain = QString(style).remove(QLatin1Char(' '));
        QStringList found;
        for (const Style &s : m_doc->m_wml.styles.styles)
            if (s.type == QLatin1String("paragraph") && (s.name.compare(style, Qt::CaseInsensitive) == 0 || s.id.compare(style, Qt::CaseInsensitive) == 0 || s.id.compare(plain, Qt::CaseInsensitive) == 0))
                found << s.id;
        if (m_styleIds.size() >= kMostStyleRefKept) m_styleIds.clear();
        ids = m_styleIds.insert(style, found);
    }
    if (ids->isEmpty()) return QString();
    if (!m_styleIndexMade) {
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            ++s_styleRefParagraphs;
            const QString id = b.blockFormat().stringProperty(tp::StyleName);
            if (!id.isEmpty()) m_styleStarts[id].push_back(b.position());
        }
        m_styleIndexMade = true;
    }
    // The paragraph on the page (its first, or its last with \l), else the last before the page, else the first after it: by where the paragraphs of the style begin. (A
    // paragraph that begins before the page and goes on into it is on it.)
    const int pageStart = story->findBlock(std::clamp(from, 0, std::max(0, story->characterCount() - 1))).position();
    int found = -1;
    const bool last = switches.contains(QLatin1Char('l'));
    for (const QString &id : *ids) {
        const auto it = m_styleStarts.constFind(id);
        if (it == m_styleStarts.cend()) continue;
        const QVector<int> &starts = it.value();
        const auto firstOnPage = std::lower_bound(starts.cbegin(), starts.cend(), pageStart), afterPage = std::lower_bound(starts.cbegin(), starts.cend(), to);
        int candidate = -1;
        if (firstOnPage < afterPage) candidate = last ? *(afterPage - 1) : *firstOnPage;
        else if (firstOnPage != starts.cbegin()) candidate = *(firstOnPage - 1);
        else if (afterPage != starts.cend()) candidate = *afterPage;
        else continue;
        // (Of several styles that the name says, the one that has a paragraph the page's own is the better, then the nearest before.)
        auto rank = [&](int at) { return at >= pageStart && at < to ? 0 : at < pageStart ? 1 : 2; };
        if (found < 0 || rank(candidate) < rank(found) || (rank(candidate) == rank(found) && ((rank(candidate) == 1 && candidate > found) || (rank(candidate) == 0 && (last ? candidate > found : candidate < found)) || (rank(candidate) == 2 && candidate < found))))
            found = candidate;
    }
    if (found < 0) return QString();
    const bool numbered = switches.contains(QLatin1Char('n')) || switches.contains(QLatin1Char('w')) || switches.contains(QLatin1Char('r'));
    const qint64 key = (qint64(found) << 1) | (numbered ? 1 : 0);
    if (const auto kept = m_styleTexts.constFind(key); kept != m_styleTexts.cend()) return kept.value();
    ++s_styleRefParagraphs;
    const QTextBlock block = story->findBlock(found);
    QString text;
    if (numbered) {
        // The paragraph's number, without the periods after it.
        QString number = block.blockFormat().stringProperty(tp::BulletChar);
        if (number.isEmpty() && block.textList() && block.textList()->format().style() != QTextListFormat::ListDisc && block.textList()->format().style() != QTextListFormat::ListCircle
            && block.textList()->format().style() != QTextListFormat::ListSquare)
            number = block.textList()->itemText(block);
        number = number.trimmed();
        while (number.endsWith(QLatin1Char('.'))) number.chop(1);
        text = number.left(1000);
    } else {
        text = block.text();
        text.remove(QChar::ObjectReplacementCharacter);
        text.replace(QChar::LineSeparator, QLatin1Char(' ')).replace(QLatin1Char('\t'), QLatin1Char(' '));
        text = text.simplified().left(1000);
    }
    if (m_styleTexts.size() >= kMostStyleRefKept) m_styleTexts.clear();
    m_styleTexts.insert(key, text);
    return text;
}

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
    // (What a header's field that shows a paragraph style's text finds on each page: this document's story, with or without notes.)
    m_styleRefSource = std::make_unique<NoteFields>(this);
    m_flow->styleRefs = m_styleRefSource.get();
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
    if (m_noteNotice.size() > kMostNoticeParagraphs) m_noteNotice.resize(kMostNoticeParagraphs);
    if (m_noteContinuation.size() > kMostNoticeParagraphs) m_noteContinuation.resize(kMostNoticeParagraphs);
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
    auto heightOf = [&](const QTextDocument *n) {
        if (!n) return 0.0;
        StoryLayout part;
        part.build(n, {fs}, m_flow->env);
        const auto partLines = part.lineInfo(0);
        return partLines.isEmpty() ? 0.0 : partLines.last().rect.bottom() + n->lastBlock().blockFormat().bottomMargin();
    };
    m_noteFields->notice = heightOf(noticeStory());
    // The separator of a note that goes on from the page before: the file's own paragraph for it (its text and rule), which a page sets above that part of the note.
    m_noteFields->continuation = heightOf(continuationStory());
    // Where the rule is in it: at three fifths of the line that holds the separator's character, after the words that stand before it in the line.
    m_noteFields->ruleAt = -1;
    m_noteFields->ruleFrom = 0;
    m_noteFields->ruleFull = true;
    if (const QTextDocument *c = continuationStory()) {
        // (In a line of any length: the words before the character are in one line, however wide the column is that the pages set them in.)
        FrameSpec wide = fs;
        wide.size.setWidth(1e5);
        StoryLayout part;
        part.build(c, {wide}, m_flow->env);
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
                // (After the words before the character in its line, where the layout has them: by the word processor's widths.)
                if (!before.trimmed().isEmpty()) {
                    int frame = -1;
                    QRectF at;
                    if (part.caretRect(std::min(block.position() + int(before.size()), block.position() + std::max(0, block.length() - 1)), &frame, &at)) m_noteFields->ruleFrom = std::max(0.0, at.x()) + 3;
                }
                break;
            }
            block = block.next();
        }
    }
}

namespace {
// A notice's story kept to its first kMostNoticeCharacters characters.
void limitNoticeStory(QTextDocument *story)
{
    if (story->characterCount() <= kMostNoticeCharacters) return;
    QTextCursor c(story);
    c.setPosition(kMostNoticeCharacters);
    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    c.removeSelectedText();
}
} // namespace

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
        limitNoticeStory(m_noticeStory.get());
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
        limitNoticeStory(m_continuationStory.get());
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

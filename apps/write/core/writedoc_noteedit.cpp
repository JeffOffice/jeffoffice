// Editing the notes of a document: the footnotes in the form they are edited in, the history they are in, making and copying them, and the notes' parts written back.
//
// A footnote is the document's only while the text cites it: its story is made when a page asks for it (or when a person edits it), and lives as long as the document does; a
// reference taken out of the text leaves the note's story where it is, and the pages set only the notes the references of the text name, so taking a reference out takes the note
// off the pages, and an undo that brings the reference back brings the note back with the words it had. Writing, the same: the part has the notes the text cites (and those it
// cited when it was read and the file keeps).

#include "core/writedoc.h"

#include "core/editops.h"
#include "core/tablemodel.h"
#include "core/writeprops.h"
#include "flow/paginator.h"
#include "text/textprops.h"

#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>
#include <atomic>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {
const QLatin1String kFootnotesRel("http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes");
const QLatin1String kFootnotesType("application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml");
const QLatin1String kSettingsRel2("http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings");
const QLatin1String kSettingsType2("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml");
} // namespace

namespace {
std::atomic<qint64> s_settleScans{0}, s_settleVisited{0}, s_sequenceWork{0}, s_noteWalk{0};
} // namespace
qint64 noteWorkForTest() { return s_settleVisited.load() + s_sequenceWork.load() + s_noteWalk.load(); }
void resetNoteWorkForTest()
{
    s_settleScans = 0;
    s_settleVisited = 0;
    s_sequenceWork = 0;
    s_noteWalk = 0;
}

// ---- the places that cite notes

namespace {
// The Ids of the footnotes and the endnotes that the references in a piece of XML name (the form a text box, a drawing, or a header that was not made over is kept in).
void collectNoteReferences(const QByteArray &xml, QSet<int> *footnotes, QSet<int> *endnotes)
{
    if (!xml.contains("oteReference")) return;
    static const QRegularExpression element(QStringLiteral("<(?:[A-Za-z0-9_.-]+:)?(foot|end)noteReference\\b([^>]*)>"));
    static const QRegularExpression idAttribute(QStringLiteral("(?:^|\\s)(?:[A-Za-z0-9_.-]+:)?id\\s*=\\s*[\"'](-?[0-9]+)[\"']"));
    const QString text = QString::fromUtf8(xml);
    for (auto m = element.globalMatch(text); m.hasNext();) {
        const QRegularExpressionMatch e = m.next();
        const QRegularExpressionMatch a = idAttribute.match(e.captured(2));
        if (a.hasMatch()) (e.captured(1) == QLatin1String("foot") ? footnotes : endnotes)->insert(a.captured(1).toInt());
    }
}

void collectNoteReferencesInBlocks(const QVector<Block> &blocks, QSet<int> *footnotes, QSet<int> *endnotes)
{
    for (const Block &b : blocks) {
        if (b.isRaw) collectNoteReferences(b.raw, footnotes, endnotes);
        for (const Inline &in : b.para.content)
            if (in.kind != Inline::Text) collectNoteReferences(in.raw, footnotes, endnotes);
    }
}

// The references among the pieces of a story that are kept as XML (and, with `references`, the story's own reference characters too: the text's are known already).
void collectNoteReferencesInStory(const QTextDocument *story, bool references, QSet<int> *footnotes, QSet<int> *endnotes)
{
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next(), ++s_noteWalk) {
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.hasProperty(prop::EndnoteBlock)) continue;   // (the endnotes the pages set: the notes are looked through themselves)
        for (const int key : {int(prop::RawBefore), int(prop::RawBlock), int(prop::RawAfter)})
            if (bf.hasProperty(key)) collectNoteReferencesInBlocks(unpackBlocks(storedBytes(bf, key)), footnotes, endnotes);
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!bytes.contains("oteReference")) continue;
            const Inline atom = unpackAtom(bytes);
            const bool own = atom.tag == QLatin1String("footnoteReference") || atom.tag == QLatin1String("endnoteReference");
            if (own && !references) continue;
            collectNoteReferences(atom.raw, footnotes, endnotes);
        }
    }
}
} // namespace

WriteDoc::NoteCitations WriteDoc::noteCitations(bool notesOnly)
{
    NoteCitations out;
    if (!notesOnly) {
        collectNoteReferencesInStory(story(), false, &out.beyondFootnotes, &out.beyondEndnotes);
        for (auto it = m_headerParts.cbegin(); it != m_headerParts.cend(); ++it) {
            const HeaderPart &part = it.value();
            if (part.editForm && part.story) collectNoteReferencesInStory(part.story.get(), true, &out.beyondFootnotes, &out.beyondEndnotes);
            else if (!part.created) collectNoteReferences(partData(part.name), &out.beyondFootnotes, &out.beyondEndnotes);
        }
    }
    // (The notes: one that cites itself is not kept alive by it.)
    for (auto it = m_noteBlocks.cbegin(); it != m_noteBlocks.cend(); ++it) {
        QSet<int> f, e;
        const auto story = m_noteStories.constFind(it.key());
        if (story != m_noteStories.cend()) collectNoteReferencesInStory(story.value().get(), true, &f, &e);
        else collectNoteReferencesInBlocks(it.value(), &f, &e);
        const int id = it.key().mid(2).toInt();
        f.remove(id);
        if (!f.isEmpty() || !e.isEmpty()) out.byFootnote.insert(id, {f, e});
    }
    for (auto it = m_endnoteBlocks.cbegin(); it != m_endnoteBlocks.cend(); ++it) {
        QSet<int> f, e;
        collectNoteReferencesInBlocks(it.value(), &f, &e);
        const int id = it.key().toInt();
        e.remove(id);
        if (!f.isEmpty() || !e.isEmpty()) out.byEndnote.insert(id, {f, e});
    }
    return out;
}

void WriteDoc::notesCitedBeyondTheText(QSet<int> *footnotes, QSet<int> *endnotes)
{
    const NoteCitations c = noteCitations(false);
    *footnotes += c.beyondFootnotes;
    *endnotes += c.beyondEndnotes;
    for (const auto &cites : c.byFootnote) {
        *footnotes += cites.first;
        *endnotes += cites.second;
    }
    for (const auto &cites : c.byEndnote) {
        *footnotes += cites.first;
        *endnotes += cites.second;
    }
}

// Notes reached from `footnotes` and `endnotes` by what the notes cite.
void WriteDoc::spreadThroughNotes(const NoteCitations &c, QSet<int> *footnotes, QSet<int> *endnotes)
{
    QVector<std::pair<bool, int>> work;
    for (const int id : std::as_const(*footnotes)) work.push_back({false, id});
    for (const int id : std::as_const(*endnotes)) work.push_back({true, id});
    while (!work.isEmpty()) {
        const auto [isEndnote, id] = work.takeLast();
        const auto it = isEndnote ? c.byEndnote.constFind(id) : c.byFootnote.constFind(id);
        if (it == (isEndnote ? c.byEndnote.cend() : c.byFootnote.cend())) continue;
        for (const int f : it->first)
            if (!footnotes->contains(f)) {
                footnotes->insert(f);
                work.push_back({false, f});
            }
        for (const int e : it->second)
            if (!endnotes->contains(e)) {
                endnotes->insert(e);
                work.push_back({true, e});
            }
    }
}

void WriteDoc::liveNotes(QSet<int> *footnotes, QSet<int> *endnotes)
{
    indexFootnoteReferences();
    indexEndnoteReferences();
    const NoteCitations c = noteCitations(false);
    *footnotes = c.beyondFootnotes;
    *endnotes = c.beyondEndnotes;
    for (auto it = m_footnoteRefs.cbegin(); it != m_footnoteRefs.cend(); ++it) footnotes->insert(it.key());
    for (auto it = m_endnoteRefIndex.cbegin(); it != m_endnoteRefIndex.cend(); ++it) endnotes->insert(it.key());
    spreadThroughNotes(c, footnotes, endnotes);
}

void WriteDoc::spreadCitationsAtOpen()
{
    const NoteCitations c = noteCitations(true);
    if (c.byFootnote.isEmpty() && c.byEndnote.isEmpty()) return;
    spreadThroughNotes(c, &m_footnotesCitedAtOpen, &m_endnotesCitedAtOpen);
}

// ---- the stories

QTextDocument *WriteDoc::editableFootnote(int id) { return const_cast<QTextDocument *>(noteStory(QStringLiteral("fn%1").arg(id))); }

bool WriteDoc::footnoteIdOf(const QTextDocument *story, int *id) const
{
    const auto found = m_noteIds.constFind(story);
    if (found == m_noteIds.cend()) return false;
    if (id) *id = found.value();
    return true;
}

// The references of the text to footnotes, by Id (a reference to a note the file lacks is one too: its Id is used).
void WriteDoc::indexFootnoteReferences()
{
    if (m_footnoteRefsValid) return;
    m_footnoteRefs.clear();
    for (QTextBlock b = story()->begin(); b.isValid(); b = b.next(), ++s_noteWalk)
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!bytes.contains("footnoteReference")) continue;
            const Inline atom = unpackAtom(bytes);
            if (atom.tag != QLatin1String("footnoteReference")) continue;
            bool ok = false;
            const int id = atom.key.toInt(&ok);
            if (!ok) continue;
            const QString text = f.text();
            for (int k = 0; k < text.size(); ++k)
                if (text[k] == QChar::ObjectReplacementCharacter) m_footnoteRefs[id].push_back(f.position() + k);
        }
    m_footnoteRefsValid = true;
}

QVector<int> WriteDoc::footnoteReferences(int id)
{
    indexFootnoteReferences();
    return m_footnoteRefs.value(id);
}

QVector<int> WriteDoc::citedFootnotes()
{
    indexFootnoteReferences();
    QVector<int> ids;
    for (auto it = m_footnoteRefs.cbegin(); it != m_footnoteRefs.cend(); ++it)
        if (hasFootnote(it.key())) ids.push_back(it.key());
    std::sort(ids.begin(), ids.end());
    return ids;
}

void WriteDoc::noteEdited(QTextDocument *story)
{
    int id = 0;
    if (!footnoteIdOf(story, &id)) return;
    if (!m_dirtyNotes.contains(story)) m_dirtyNotes.push_back(story);
    if (m_pager) m_pager->noteEdited(QStringLiteral("fn%1").arg(id), footnoteReferences(id));
    Q_EMIT noteChanged(story);
}

// ---- making notes

// The document has notes now: the pages ask for them. (A document that had none has no part yet: the part is made, with its separators, when the document is written.)
void WriteDoc::activateNotes()
{
    if (m_noteFields) return;
    if (m_notePart.isEmpty()) {
        QString name = QStringLiteral("/word/footnotes.xml");
        for (int n = 2; partExists(name) || name.compare(m_wml.mainName, Qt::CaseInsensitive) == 0; ++n) name = QStringLiteral("/word/footnotes%1.xml").arg(n);
        m_notePart = name;
        m_footnotesPartNew = true;
        m_footnotesPart = NotesPart::Readable;
        QVector<NotePart> fresh;
        office::wml::readNotes(notesPartXml(QByteArray(), false, NotesChange()), &fresh);
        for (NotePart &n : fresh) {
            if (n.type == QLatin1String("separator")) m_noteSeparator = std::move(n.blocks);
            else if (n.type == QLatin1String("continuationSeparator")) m_noteContinuation = std::move(n.blocks);
        }
    }
    m_noteFields = std::make_unique<NoteFields>(this);
    m_flow->fields = m_noteFields.get();
    const QByteArray settings = m_wml.settingsName.isEmpty() ? QByteArray() : partData(m_wml.settingsName);
    m_flow->restartNotesInSections = office::wml::noteNumberRestart(settings, "footnotePr") == QLatin1String("eachSect");
    measureNoteSeparator();
    if (m_pager) m_pager->invalidate();
}

// One more than every Id the file's notes and the text's references use (1 where there are none; a gap when the highest is out of the range of an Id).
int WriteDoc::nextFootnoteId()
{
    indexFootnoteReferences();
    QSet<int> used;
    for (auto it = m_noteBlocks.cbegin(); it != m_noteBlocks.cend(); ++it) used.insert(it.key().mid(2).toInt());
    for (auto it = m_footnoteRefs.cbegin(); it != m_footnoteRefs.cend(); ++it) used.insert(it.key());
    {
        // (And every Id that a header, a text box, or another note cites: a new note is not to be the note that they name.)
        QSet<int> beyond, unused;
        notesCitedBeyondTheText(&beyond, &unused);
        used += beyond;
    }
    int highest = std::max(m_footnotePartHighest, m_footnoteIdFloor);   // (above the Id of every element of the part: the separators have Ids as the notes have)
    for (const int id : std::as_const(used)) highest = std::max(highest, id);
    if (highest < 1'000'000'000) return highest + 1;
    for (int id = m_footnotePartHighest + 1;; ++id)
        if (!used.contains(id) && id > m_footnoteIdFloor) return id;
}

// Whether a note can be made: the document is not one with the most notes it holds (counted by the notes the text cites, and by the stories made), and its part is one the program can
// add to. Notes that were put in and taken back are no count.
bool WriteDoc::roomForFootnote()
{
    if (m_footnotesPart == NotesPart::Unusable) return false;
    if (int(m_noteBlocks.size()) < kMostNoteStories && int(m_noteStories.size()) < kMostNoteStories) return true;   // (the common case costs nothing)
    reclaimDeadNotes(false, 0);
    return int(citedFootnotes().size()) < kMostNoteStories && int(m_noteStories.size()) < kMostNoteStories;
}

bool WriteDoc::roomForEndnote()
{
    if (m_endnotesPart == NotesPart::Unusable || m_endnoteCapped) return false;
    for (const RawChild &ch : m_last.extra)
        if (ch.xml.contains("noEndnote")) return false;   // (the last section says it has none: they would not be set)
    if (int(m_endnoteBlocks.size()) < kMostNoteStories) return true;
    reclaimDeadNotes(true, 0);
    return int(citedEndnotes().size()) < kMostNoteStories;
}

// Lets go the notes that were made since the document was read, are cited by nothing, and have no step of the history that could bring a reference to them back.
void WriteDoc::reclaimDeadNotes(bool endnotes, int keep)
{
    const QSet<int> &made = endnotes ? m_createdEndnotes : m_createdFootnotes;
    const QHash<int, quint64> &steps = endnotes ? m_createdEndnoteStep : m_createdFootnoteStep;
    if (made.isEmpty()) return;
    if (endnotes) indexEndnoteReferences();
    else indexFootnoteReferences();
    QHash<int, quint64> candidates;   // (the notes nothing cites, by their steps)
    for (const int id : made) {
        ++s_noteWalk;
        const quint64 step = steps.value(id, 0);
        const bool cited = endnotes ? m_endnoteRefIndex.contains(id) : m_footnoteRefs.contains(id);
        if (id != keep && step != 0 && !cited) candidates.insert(id, step);
    }
    if (candidates.isEmpty()) return;
    QSet<quint64> wanted, present;
    for (auto it = candidates.cbegin(); it != candidates.cend(); ++it) wanted.insert(it.value());
    for (const Step &s : std::as_const(m_undoLog))
        if (wanted.contains(s.serial)) present.insert(s.serial);
    for (const Step &s : std::as_const(m_redoLog))
        if (wanted.contains(s.serial)) present.insert(s.serial);
    for (auto it = candidates.cbegin(); it != candidates.cend(); ++it) {
        if (present.contains(it.value())) continue;   // (an undo or a redo can still bring its reference back)
        const int id = it.key();
        if (endnotes) {
            m_endnoteBlocks.remove(QString::number(id));
            m_createdEndnotes.remove(id);
            m_createdEndnoteStep.remove(id);
            m_endnoteEdited.remove(id);
        } else {
            const QString key = QStringLiteral("fn%1").arg(id);
            const auto story = m_noteStories.constFind(key);
            if (story != m_noteStories.cend()) {
                m_noteIds.remove(story.value().get());
                m_dirtyNotes.removeAll(story.value().get());
                m_noteStories.erase(story);
            }
            m_noteBlocks.remove(key);
            m_createdFootnotes.remove(id);
            m_createdFootnoteStep.remove(id);
        }
    }
}

int WriteDoc::makeFootnote()
{
    reclaimDeadNotes(false, 0);   // (the edit that makes the note makes any redo impossible)
    if (!roomForFootnote()) return 0;
    activateNotes();
    // The styles a note has (a document that lacks them gets them: they are written only while a note that is written has them).
    const qsizetype had = m_wml.styles.styles.size();
    ensureStyle(QStringLiteral("FootnoteText"));
    ensureStyle(QStringLiteral("FootnoteReference"));
    for (qsizetype i = had; i < m_wml.styles.styles.size(); ++i) m_headerStyles.insert(m_wml.styles.styles[i].id);
    const int id = nextFootnoteId();
    Block note;
    note.para.props.style = QStringLiteral("FootnoteText");
    Inline mark;
    mark.kind = Inline::RunAtom;
    mark.tag = QStringLiteral("footnoteRef");
    mark.raw = "<w:footnoteRef/>";
    mark.props.style = QStringLiteral("FootnoteReference");
    note.para.content.push_back(mark);
    Inline blank;
    blank.text = QStringLiteral(" ");
    note.para.content.push_back(blank);
    const QString key = QStringLiteral("fn%1").arg(id);
    m_noteBlocks.insert(key, {note});
    m_createdFootnotes.insert(id);
    if (!noteStory(key)) {
        m_noteBlocks.remove(key);
        m_createdFootnotes.remove(id);
        return 0;
    }
    m_footnoteIdFloor = id;
    m_createdFootnoteStep[id] = 0;
    m_noteStepPending.push_back({false, id});
    return id;
}

int WriteDoc::duplicateFootnote(int id)
{
    const QString key = QStringLiteral("fn%1").arg(id);
    reclaimDeadNotes(false, id);
    if (!m_noteBlocks.contains(key) || !roomForFootnote()) return 0;
    QVector<Block> blocks;
    if (QTextDocument *s = editableFootnote(id)) {
        VectorSink sink(&blocks);
        exportInto(sink, s);
    } else {
        blocks = m_noteBlocks.value(key);
    }
    const int fresh = nextFootnoteId();
    const QString freshKey = QStringLiteral("fn%1").arg(fresh);
    m_noteBlocks.insert(freshKey, blocks);
    m_createdFootnotes.insert(fresh);
    if (!noteStory(freshKey)) {
        m_noteBlocks.remove(freshKey);
        m_createdFootnotes.remove(fresh);
        return 0;
    }
    m_footnoteIdFloor = fresh;
    m_createdFootnoteStep[fresh] = 0;
    m_noteStepPending.push_back({false, fresh});
    return fresh;
}

QTextCharFormat WriteDoc::footnoteReferenceFormat(int id, const QString &paragraphStyle)
{
    ensureStyle(QStringLiteral("FootnoteReference"));
    QTextCharFormat f = styleCharFormat(paragraphStyle.isEmpty() ? defaultParagraphStyle() : paragraphStyle, QStringLiteral("FootnoteReference"));
    Inline ref;
    ref.kind = Inline::RunAtom;
    ref.tag = QStringLiteral("footnoteReference");
    ref.key = QString::number(id);
    ref.raw = "<w:footnoteReference w:id=\"" + QByteArray::number(id) + "\"/>";
    ref.props.style = QStringLiteral("FootnoteReference");
    setStored(f, prop::Atom, packAtom(ref));
    f.setProperty(tp::Field, QStringLiteral("footnote:fn%1").arg(id));
    return f;
}

// ---- the endnotes

namespace {
const QLatin1String kEndnotesRel("http://schemas.openxmlformats.org/officeDocument/2006/relationships/endnotes");
const QLatin1String kEndnotesType("application/vnd.openxmlformats-officedocument.wordprocessingml.endnotes+xml");
} // namespace

bool WriteDoc::endnoteIdOf(const QTextBlock &block, int *id) const
{
    if (!block.isValid() || !block.blockFormat().hasProperty(prop::EndnoteBlock)) return false;
    const int serial = block.blockFormat().intProperty(prop::EndnoteNote);
    if (serial == 0) return false;
    const auto found = m_endnoteCopyIds.constFind(std::abs(serial));
    if (found == m_endnoteCopyIds.cend()) return false;
    if (id) *id = found.value();
    return true;
}

// The references of the text to endnotes, by Id (a reference to a note the file lacks is one too).
void WriteDoc::indexEndnoteReferences()
{
    if (m_endnoteRefIndexValid) return;
    m_endnoteRefIndex.clear();
    for (QTextBlock b = story()->begin(); b.isValid(); b = b.next(), ++s_noteWalk)
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!bytes.contains("endnoteReference")) continue;
            const Inline atom = unpackAtom(bytes);
            if (atom.tag != QLatin1String("endnoteReference")) continue;
            bool ok = false;
            const int id = atom.key.toInt(&ok);
            if (!ok) continue;
            const QString text = f.text();
            for (int k = 0; k < text.size(); ++k)
                if (text[k] == QChar::ObjectReplacementCharacter) m_endnoteRefIndex[id].push_back(f.position() + k);
        }
    m_endnoteRefIndexValid = true;
}

QVector<int> WriteDoc::endnoteReferences(int id)
{
    indexEndnoteReferences();
    return m_endnoteRefIndex.value(id);
}

QVector<int> WriteDoc::citedEndnotes()
{
    indexEndnoteReferences();
    QVector<int> ids;
    for (auto it = m_endnoteRefIndex.cbegin(); it != m_endnoteRefIndex.cend(); ++it)
        if (hasEndnote(it.key())) ids.push_back(it.key());
    std::sort(ids.begin(), ids.end());
    return ids;
}

bool WriteDoc::endnoteWords(int id, int *from, int *to)
{
    for (QTextBlock b = story()->lastBlock(); b.isValid(); b = b.previous()) {
        int found = 0;
        if (!endnoteIdOf(b, &found) || found != id || b.blockFormat().intProperty(prop::EndnoteNote) <= 0) continue;
        EndnoteSpan span = endnoteSpanAt(story(), b.position());
        if (!span.valid) return false;
        // A table the note holds is kept as it was, and is not among the note's own words: the words before it are the start of the note.
        const int serial = b.blockFormat().intProperty(prop::EndnoteNote);
        for (;;) {
            QTextBlock skip = story()->findBlock(span.blockFrom).previous();
            while (skip.isValid() && !skip.blockFormat().hasProperty(prop::EndnoteBlock) && skip.blockFormat().hasProperty(prop::RawBlock)) skip = skip.previous();
            if (!skip.isValid() || !skip.blockFormat().hasProperty(prop::EndnoteBlock) || skip.blockFormat().intProperty(prop::EndnoteNote) != serial) break;
            const EndnoteSpan before = endnoteSpanAt(story(), skip.position());
            span.blockFrom = before.blockFrom;
            span.textFrom = before.textFrom;
        }
        if (from) *from = span.textFrom;
        if (to) *to = span.textTo;
        return span.valid;
    }
    return false;
}

// The words of the notes that were edited, taken back from the story as the blocks the notes are made of (a note that was not edited is made again from the blocks it was read as). `only`
// names the notes to take back, else those that were edited.
void WriteDoc::captureEndnotes(const QSet<int> *only)
{
    QTextDocument *d = story();
    struct Run {
        QTextBlock first, last;
    };
    QHash<int, Run> runs;   // by the number of the copy
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
        ++s_noteWalk;
        if (!b.blockFormat().hasProperty(prop::EndnoteBlock)) continue;
        const int serial = b.blockFormat().intProperty(prop::EndnoteNote);
        if (serial <= 0) continue;   // (the separator, and the copies that are only shown)
        const auto it = runs.find(serial);
        if (it == runs.end()) runs.insert(serial, {b, b});
        else it->last = b;
    }
    for (auto it = runs.cbegin(); it != runs.cend(); ++it) {
        const int id = m_endnoteCopyIds.value(it.key());
        if (only ? !only->contains(id) : !m_endnoteEdited.contains(id)) continue;
        QVector<Block> blocks;
        VectorSink sink(&blocks);
        exportInto(sink, d, it->first, it->last, true);
        if (!blocks.isEmpty()) m_endnoteBlocks[QString::number(id)] = blocks;
    }
}

// An edit touched the paragraphs of a note: the note was edited.
void WriteDoc::touchEndnotes(int position, int removed, int added)
{
    Q_UNUSED(removed);
    QTextDocument *d = story();
    const int last = std::max(0, d->characterCount() - 1);
    const QTextBlock end = d->findBlock(std::min(position + std::max(added, 0), last));
    for (QTextBlock b = d->findBlock(std::min(position, last)); b.isValid(); b = b.next()) {
        ++s_noteWalk;
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.hasProperty(prop::EndnoteBlock) && bf.intProperty(prop::EndnoteNote) != 0) m_endnoteEdited.insert(m_endnoteCopyIds.value(std::abs(bf.intProperty(prop::EndnoteNote))));
        if (b == end) break;
    }
}

void WriteDoc::activateEndnotes()
{
    if (m_endnotePart.isEmpty()) {
        QString name = QStringLiteral("/word/endnotes.xml");
        for (int n = 2; partExists(name) || name.compare(m_wml.mainName, Qt::CaseInsensitive) == 0 || name.compare(m_notePart, Qt::CaseInsensitive) == 0; ++n) name = QStringLiteral("/word/endnotes%1.xml").arg(n);
        m_endnotePart = name;
        m_endnotesPartNew = true;
        m_endnotesPart = NotesPart::Readable;
        QVector<NotePart> fresh;
        office::wml::readNotes(notesPartXml(QByteArray(), true, NotesChange()), &fresh);
        for (NotePart &n : fresh)
            if (n.type == QLatin1String("separator")) m_endnoteSeparator = std::move(n.blocks);
    }
}

// One more than every Id the file's endnotes and the text's references use.
int WriteDoc::nextEndnoteId()
{
    indexEndnoteReferences();
    int highest = std::max(m_endnotePartHighest, m_endnoteIdFloor);   // (above the Id of every element of the part)
    QSet<int> used;
    for (auto it = m_endnoteBlocks.cbegin(); it != m_endnoteBlocks.cend(); ++it) used.insert(it.key().toInt());
    for (auto it = m_endnoteRefIndex.cbegin(); it != m_endnoteRefIndex.cend(); ++it) used.insert(it.key());
    {
        QSet<int> unused, beyond;
        notesCitedBeyondTheText(&unused, &beyond);
        used += beyond;
    }
    for (const int id : std::as_const(used)) highest = std::max(highest, id);
    if (highest < 1'000'000'000) return highest + 1;
    for (int id = m_endnotePartHighest + 1;; ++id)
        if (!used.contains(id) && id > m_endnoteIdFloor) return id;
}

int WriteDoc::makeEndnote()
{
    reclaimDeadNotes(true, 0);   // (the edit that makes the note makes any redo impossible)
    if (!roomForEndnote()) return 0;
    activateEndnotes();
    const qsizetype had = m_wml.styles.styles.size();
    ensureStyle(QStringLiteral("EndnoteText"));
    ensureStyle(QStringLiteral("EndnoteReference"));
    for (qsizetype i = had; i < m_wml.styles.styles.size(); ++i) m_headerStyles.insert(m_wml.styles.styles[i].id);
    const int id = nextEndnoteId();
    Block note;
    note.para.props.style = QStringLiteral("EndnoteText");
    Inline mark;
    mark.kind = Inline::RunAtom;
    mark.tag = QStringLiteral("endnoteRef");
    mark.raw = "<w:endnoteRef/>";
    mark.props.style = QStringLiteral("EndnoteReference");
    note.para.content.push_back(mark);
    Inline blank;
    blank.text = QStringLiteral(" ");
    note.para.content.push_back(blank);
    m_endnoteBlocks.insert(QString::number(id), {note});
    m_createdEndnotes.insert(id);
    m_endnoteIdFloor = id;
    m_createdEndnoteStep[id] = 0;
    m_noteStepPending.push_back({true, id});
    return id;
}

int WriteDoc::duplicateEndnote(int id)
{
    const QString key = QString::number(id);
    reclaimDeadNotes(true, id);
    if (!m_endnoteBlocks.contains(key) || !roomForEndnote()) return 0;
    const QSet<int> one{id};
    captureEndnotes(&one);   // (what was said in it so far)
    const QVector<Block> blocks = m_endnoteBlocks.value(key);
    const int fresh = nextEndnoteId();
    m_endnoteBlocks.insert(QString::number(fresh), blocks);
    m_createdEndnotes.insert(fresh);
    m_endnoteIdFloor = fresh;
    m_createdEndnoteStep[fresh] = 0;
    m_noteStepPending.push_back({true, fresh});
    return fresh;
}

QTextCharFormat WriteDoc::endnoteReferenceFormat(int id, const QString &paragraphStyle)
{
    ensureStyle(QStringLiteral("EndnoteReference"));
    QTextCharFormat f = styleCharFormat(paragraphStyle.isEmpty() ? defaultParagraphStyle() : paragraphStyle, QStringLiteral("EndnoteReference"));
    Inline ref;
    ref.kind = Inline::RunAtom;
    ref.tag = QStringLiteral("endnoteReference");
    ref.key = QString::number(id);
    ref.raw = "<w:endnoteReference w:id=\"" + QByteArray::number(id) + "\"/>";
    ref.props.style = QStringLiteral("EndnoteReference");
    setStored(f, prop::Atom, packAtom(ref));
    f.setProperty(tp::Field, QStringLiteral("raw:"));   // (the number it shows is given when the notes are set)
    return f;
}

// The notes that the text's references name, and those the pages set, each in order of the references: when they are not the same the notes are set again.
qint64 endnoteSequenceWorkForTest() { return s_sequenceWork.load(); }
void resetEndnoteSequenceWorkForTest() { s_sequenceWork = 0; }

// Whether the character format is that of a reference to an endnote that the settling counts, and the Id it names.
bool WriteDoc::countedEndnoteReference(const QTextCharFormat &cf, int *id) const
{
    if (!cf.hasProperty(prop::Atom) || cf.hasProperty(prop::TrackedDeleted)) return false;
    const QByteArray bytes = storedBytes(cf, prop::Atom);
    if (!bytes.contains("endnoteReference")) return false;
    const Inline atom = unpackAtom(bytes);
    bool ok = false;
    const int found = atom.tag == QLatin1String("endnoteReference") ? atom.key.toInt(&ok) : 0;
    if (!ok || !hasEndnote(found)) return false;
    *id = found;
    return true;
}

// The references among the characters from `from` to `to` (of the story as it is now).
QVector<WriteDoc::EndnoteSeqEntry> WriteDoc::endnoteReferencesIn(int from, int to)
{
    QVector<EndnoteSeqEntry> out;
    QTextDocument *d = story();
    const int end = d->characterCount();
    from = std::clamp(from, 0, end);
    to = std::clamp(to, from, end);
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() < to; b = b.next(), ++s_noteWalk)
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            if (f.position() + f.length() <= from) continue;
            if (f.position() >= to) break;
            int id = 0;
            if (!countedEndnoteReference(f.charFormat(), &id)) continue;
            const QString text = f.text();
            for (int k = 0; k < text.size(); ++k) {
                const int at = f.position() + k;
                if (text[k] == QChar::ObjectReplacementCharacter && at >= from && at < to) out.push_back({at, id});
            }
        }
    return out;
}

void WriteDoc::setEndnoteSeq(const QVector<EndnoteSeqEntry> &references)
{
    const qsizetype n = references.size();
    m_endnoteSeq.gap.resize(n);
    m_endnoteSeq.id.resize(n);
    m_endnoteSeq.tree.fill(0, n + 1);
    int before = 0;
    for (qsizetype i = 0; i < n; ++i) {
        m_endnoteSeq.gap[i] = references[i].position - before;
        m_endnoteSeq.id[i] = references[i].id;
        before = references[i].position;
    }
    for (qsizetype i = 1; i <= n; ++i) {
        m_endnoteSeq.tree[i] += m_endnoteSeq.gap[i - 1];
        const qsizetype up = i + (i & -i);
        if (up <= n) m_endnoteSeq.tree[up] += m_endnoteSeq.tree[i];
    }
}

qsizetype WriteDoc::endnoteSeqBefore(int position, int *last) const
{
    const qsizetype n = m_endnoteSeq.id.size();
    qsizetype index = 0;
    qint64 total = 0;
    qsizetype step = 1;
    while (step * 2 <= n) step *= 2;
    for (; n > 0 && step > 0; step >>= 1) {
        ++s_sequenceWork;
        if (index + step <= n && total + m_endnoteSeq.tree[index + step] < position) {
            index += step;
            total += m_endnoteSeq.tree[index];
        }
    }
    if (last) *last = int(total);
    return index;
}

void WriteDoc::endnoteSeqMove(qsizetype rank, int by)
{
    const qsizetype n = m_endnoteSeq.id.size();
    m_endnoteSeq.gap[rank] += by;
    for (qsizetype i = rank + 1; i <= n; i += i & -i) {
        ++s_sequenceWork;
        m_endnoteSeq.tree[i] += by;
    }
}

// The story changed from `position`: `removed` characters went and `added` came. A change that takes no reference out and puts none in only moves the places of those after it (one distance
// changes); one that takes any out or puts any in, or changes the Id one names, makes the settling look through the story again, which is what it does for a change in the references.
void WriteDoc::followEndnoteReferences(int position, int removed, int added)
{
    if (!m_endnoteSeqValid) return;
    const qsizetype n = m_endnoteSeq.id.size();
    int before = 0;
    const qsizetype lo = endnoteSeqBefore(position, &before);   // (the first reference at or after the change)
    const int end = position + std::max(removed, 0);
    bool inside = false;
    if (lo < n) {
        ++s_sequenceWork;
        inside = before + m_endnoteSeq.gap[lo] < end;
    }
    const QVector<EndnoteSeqEntry> now = added > 0 ? endnoteReferencesIn(position, position + added) : QVector<EndnoteSeqEntry>();
    if (!inside && now.isEmpty()) {
        if (added != removed && lo < n) endnoteSeqMove(lo, added - removed);
        return;
    }
    // (References that are the same, in the same places, after the change: its format only.)
    bool same = inside && removed == added && now.size() > 0;
    qsizetype k = lo;
    int at = before;
    for (const EndnoteSeqEntry &r : now) {
        if (!same) break;
        if (k >= n) {
            same = false;
            break;
        }
        ++s_sequenceWork;
        at += m_endnoteSeq.gap[k];
        same = at == r.position && m_endnoteSeq.id[k] == r.id;
        ++k;
    }
    if (same && k < n) {
        ++s_sequenceWork;
        same = at + m_endnoteSeq.gap[k] >= end;
    }
    if (!same) m_endnoteSeqValid = false;
}

bool WriteDoc::endnoteSequenceMatchesStoryForTest()
{
    if (!m_endnoteSeqValid) return true;
    const QVector<EndnoteSeqEntry> fresh = endnoteReferencesIn(0, story()->characterCount());
    if (fresh.size() != m_endnoteSeq.id.size()) return false;
    int at = 0;
    for (qsizetype i = 0; i < fresh.size(); ++i) {
        at += m_endnoteSeq.gap[i];
        if (fresh[i].position != at || fresh[i].id != m_endnoteSeq.id[i]) return false;
    }
    return true;
}
qint64 endnoteSettleScansForTest() { return s_settleScans.load(); }
qint64 endnoteSettleVisitedForTest() { return s_settleVisited.load(); }
void resetEndnoteSettleWorkForTest()
{
    s_settleScans = 0;
    s_settleVisited = 0;
}

void WriteDoc::settleEndnotes()
{
    if (m_endnotesPart != NotesPart::Readable || m_syncing || m_buildingEndnotes || m_endnoteCapped) return;
    for (const RawChild &ch : m_last.extra)
        if (ch.xml.contains("noEndnote")) return;
    if (m_endnoteSeqValid) return;   // (the references name the notes they named when the notes were last set, and the pages set those)
    QTextDocument *d = story();
    QVector<int> refs, tails;
    QVector<EndnoteSeqEntry> sequence;
    ++s_settleScans;
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
        ++s_settleVisited;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd() && refs.size() < kMostEndnotes; ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom) || cf.hasProperty(prop::TrackedDeleted)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!bytes.contains("endnoteReference")) continue;
            const Inline atom = unpackAtom(bytes);
            bool ok = false;
            const int id = atom.tag == QLatin1String("endnoteReference") ? atom.key.toInt(&ok) : 0;
            if (!ok || !hasEndnote(id)) continue;
            const QTextFragment fragment = it.fragment();
            const QString text = fragment.text();
            for (int k = 0; k < text.size(); ++k)
                if (text[k] == QChar::ObjectReplacementCharacter && refs.size() < kMostEndnotes) {
                    refs.push_back(id);
                    sequence.push_back({fragment.position() + k, id});
                }
        }
        const QTextBlockFormat bf = b.blockFormat();
        if (!bf.hasProperty(prop::EndnoteBlock) || bf.intProperty(prop::EndnoteNote) == 0) continue;
        const int serial = bf.intProperty(prop::EndnoteNote);
        const QTextBlock before = b.previous();
        if (before.isValid() && before.blockFormat().hasProperty(prop::EndnoteBlock) && before.blockFormat().intProperty(prop::EndnoteNote) == serial) continue;
        tails.push_back(m_endnoteCopyIds.value(std::abs(serial)));
    }
    setEndnoteSeq(sequence);
    m_endnoteSeqValid = refs.size() < kMostEndnotes;
    if (refs != tails) rebuildEndnoteTails();
}

// ---- writing them

KeptPart *WriteDoc::settingsPartFor(office::wml::Document *out)
{
    if (!out->settingsName.isEmpty())
        for (KeptPart &kept : out->parts)
            if (kept.name.compare(out->settingsName, Qt::CaseInsensitive) == 0) return &kept;
    KeptPart kept;
    kept.name = QStringLiteral("/word/settings.xml");
    for (int n = 2; partExists(kept.name) || kept.name.compare(out->mainName, Qt::CaseInsensitive) == 0; ++n) kept.name = QStringLiteral("/word/settings%1.xml").arg(n);
    kept.contentType = QString(kSettingsType2);
    out->parts.push_back(kept);
    Relationship r;
    r.id = nextRelationshipId(out->mainRelationships);
    r.type = QString(kSettingsRel2);
    r.target = kept.name;
    out->mainRelationships.push_back(r);
    out->settingsName = kept.name;
    return &out->parts.last();
}

// The notes parts as the notes are now: what was edited, made, or whose last reference was taken out; a part nothing of which changed is as it was read.
void WriteDoc::writeNoteParts(office::wml::Document *out)
{
    if (m_footnotesPart != NotesPart::Readable) return;
    indexFootnoteReferences();
    NotesChange change;
    // The notes that were edited are written from their stories (whether or not the text cites them: a note the file keeps is the file's), unless their last reference was taken
    // out of the text.
    QSet<int> live, unusedEndnotes;
    liveNotes(&live, &unusedEndnotes);   // (a note that a header, a text box, or a note that is alive cites is cited though the text does not)
    auto cited = [&](int id) { return live.contains(id); };
    for (auto it = m_noteStories.cbegin(); it != m_noteStories.cend(); ++it) {
        const int id = it.key().mid(2).toInt();
        QTextDocument *s = it.value().get();
        if (m_createdFootnotes.contains(id) || s->availableUndoSteps() == 0) continue;
        if (m_footnotesCitedAtOpen.contains(id) && !cited(id)) continue;
        VectorSink sink(&change.replaced[id]);
        exportInto(sink, s);
    }
    for (const int id : std::as_const(m_footnotesCitedAtOpen))
        if (!cited(id) && m_noteBlocks.contains(QStringLiteral("fn%1").arg(id))) change.removed.insert(id);
    QVector<int> made = m_createdFootnotes.values();
    std::sort(made.begin(), made.end());
    for (const int id : std::as_const(made)) {
        if (!cited(id)) continue;
        QVector<Block> blocks;
        if (QTextDocument *s = editableFootnote(id)) {
            VectorSink sink(&blocks);
            exportInto(sink, s);
        }
        change.added.push_back({id, blocks});
    }
    if (change.replaced.isEmpty() && change.removed.isEmpty() && change.added.isEmpty()) return;
    const QByteArray original = m_footnotesPartNew ? QByteArray() : partData(m_notePart);
    const QByteArray data = notesPartXml(original, false, change);
    if (data.isEmpty()) return;
    if (!m_footnotesPartNew) {
        for (KeptPart &kept : out->parts)
            if (kept.name.compare(m_notePart, Qt::CaseInsensitive) == 0) {
                kept.data = data;
                break;
            }
        return;
    }
    // A part the document did not have: the part, the relationship to it, and the settings that name its separators.
    KeptPart kept;
    kept.name = m_notePart;
    kept.contentType = QString(kFootnotesType);
    kept.data = data;
    out->parts.push_back(kept);
    Relationship r;
    r.id = nextRelationshipId(out->mainRelationships);
    r.type = QString(kFootnotesRel);
    r.target = m_notePart;
    out->mainRelationships.push_back(r);
    KeptPart *settings = settingsPartFor(out);
    settings->data = settingsWithNoteSeparators(settings->data, false);
}

// The endnotes part as the notes are now (as the footnotes' is): what was edited, made, or whose last reference was taken out; a part nothing of which changed is as it was read.
void WriteDoc::writeEndnoteParts(office::wml::Document *out)
{
    if (m_endnotesPart != NotesPart::Readable) return;
    captureEndnotes();
    indexEndnoteReferences();
    QSet<int> unusedFootnotes, live;
    liveNotes(&unusedFootnotes, &live);   // (a note that a header, a text box, or a note that is alive cites is cited though the text does not)
    auto cited = [&](int id) { return live.contains(id); };
    NotesChange change;
    // The notes that were touched are written again if they say something other than they did when they were read.
    for (const int id : std::as_const(m_endnoteEdited)) {
        const QString key = QString::number(id);
        if (!m_endnoteBlocks.contains(key) || m_createdEndnotes.contains(id) || (m_endnotesCitedAtOpen.contains(id) && !cited(id))) continue;
        const QVector<Block> &blocks = m_endnoteBlocks[key];
        if (blocksFingerprint(blocks, 0, blocks.size()) != m_endnoteOriginal.value(id)) change.replaced.insert(id, blocks);
    }
    for (const int id : std::as_const(m_endnotesCitedAtOpen))
        if (!cited(id) && m_endnoteBlocks.contains(QString::number(id))) change.removed.insert(id);
    QVector<int> made = m_createdEndnotes.values();
    std::sort(made.begin(), made.end());
    for (const int id : std::as_const(made))
        if (cited(id) && m_endnoteBlocks.contains(QString::number(id))) change.added.push_back({id, m_endnoteBlocks.value(QString::number(id))});
    if (change.replaced.isEmpty() && change.removed.isEmpty() && change.added.isEmpty()) return;
    const QByteArray original = m_endnotesPartNew ? QByteArray() : partData(m_endnotePart);
    const QByteArray data = notesPartXml(original, true, change);
    if (data.isEmpty()) return;
    if (!m_endnotesPartNew) {
        for (KeptPart &kept : out->parts)
            if (kept.name.compare(m_endnotePart, Qt::CaseInsensitive) == 0) {
                kept.data = data;
                break;
            }
        return;
    }
    KeptPart kept;
    kept.name = m_endnotePart;
    kept.contentType = QString(kEndnotesType);
    kept.data = data;
    out->parts.push_back(kept);
    Relationship r;
    r.id = nextRelationshipId(out->mainRelationships);
    r.type = QString(kEndnotesRel);
    r.target = m_endnotePart;
    out->mainRelationships.push_back(r);
    KeptPart *settings = settingsPartFor(out);
    settings->data = settingsWithNoteSeparators(settings->data, true);
}

} // namespace jp::write

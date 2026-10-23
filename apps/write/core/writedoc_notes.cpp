// The footnotes of a file: read from its footnotes part, made into stories for the pages that cite them.

#include "core/writedoc.h"

#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QRegularExpression>
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
// The most of the file's continuation notice, and of its separator for the part of a note that goes on, that is read: the paragraphs, and the characters of them. (The pages set a
// quarter of a page of each at most, so the rest would only be copied to every page.)
constexpr int kMostNoticeParagraphs = 64;
constexpr int kMostNoticeCharacters = 4000;
// The most bookmarks of the story that a page reference can find (the rest are not found: the reference shows what the file had).
constexpr int kMostBookmarks = 20000;
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
namespace { std::atomic<qint64> s_endnoteParagraphsMade{0}; }
qint64 endnoteParagraphsMadeForTest() { return s_endnoteParagraphsMade.load(); }
void resetEndnoteParagraphsMadeForTest() { s_endnoteParagraphsMade = 0; }
void resetStyleRefParagraphsScannedForTest() { s_styleRefParagraphs = 0; }
namespace { std::atomic<qint64> s_noteStoriesMade{0}; }
qint64 noteStoriesMadeForTest() { return s_noteStoriesMade.load(); }
void resetNoteStoriesMadeForTest() { s_noteStoriesMade = 0; }

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

int NoteFields::bookmarkPosition(const QString &name) const
{
    const QTextDocument *story = m_doc->story();
    if (m_bookmarkStory != story || m_bookmarkRevision != story->revision() || m_bookmarkSize != story->characterCount()) {
        m_bookmarkStory = story;
        m_bookmarkRevision = story->revision();
        m_bookmarkSize = story->characterCount();
        m_bookmarks.clear();
        static const QRegularExpression named(QStringLiteral("[\\s:]name\\s*=\\s*\"([^\"]*)\""));
        for (QTextBlock b = story->begin(); b.isValid() && m_bookmarks.size() < kMostBookmarks; b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QTextCharFormat cf = it.fragment().charFormat();
                if (!cf.hasProperty(prop::Atom)) continue;
                const QByteArray bytes = storedBytes(cf, prop::Atom);
                if (!bytes.contains("bookmarkStart")) continue;
                const Inline atom = unpackAtom(bytes);
                if (atom.tag != QLatin1String("bookmarkStart")) continue;
                const QString found = named.match(QString::fromUtf8(atom.raw.left(1000))).captured(1);
                if (!found.isEmpty() && !m_bookmarks.contains(found)) m_bookmarks.insert(found, it.fragment().position());
            }
    }
    return m_bookmarks.value(name, -1);
}

QList<const QTextDocument *> NoteFields::storyDocs() const
{
    QList<const QTextDocument *> list;
    for (const auto &d : m_doc->m_noteStories) list << d.get();
    return list;
}

void WriteDoc::loadEndnotes()
{
    m_endnoteBlocks.clear();
    m_endnoteSeparator.clear();
    m_endnotePart.clear();
    m_endnoteCopyIds.clear();
    m_nextEndnoteSerial = 1;
    m_createdEndnotes.clear();
    m_endnotesCitedAtOpen.clear();
    m_endnoteEdited.clear();
    m_endnoteOriginal.clear();
    m_createdEndnoteStep.clear();
    m_endnoteIdFloor = 0;
    m_endnoteSeqValid = false;
    m_endnotesPart = NotesPart::Absent;
    m_endnotesPartNew = false;
    m_endnoteCapped = false;
    m_endnoteRefIndexValid = false;
    for (const Relationship &r : m_wml.mainRelationships)
        if (!r.external && r.type.endsWith(QLatin1String("/endnotes"))) {
            m_endnotePart = r.target;
            break;
        }
    if (m_endnotePart.isEmpty()) return;
    m_endnotesPart = NotesPart::Unusable;
    QVector<NotePart> notes;
    m_endnotePartHighest = 0;
    if (!partExists(m_endnotePart) || !office::wml::readNotes(partData(m_endnotePart), &notes, &m_endnotePartHighest)) return;
    m_endnotesPart = NotesPart::Readable;
    for (NotePart &n : notes) {
        if (n.type.isEmpty() || n.type == QLatin1String("normal")) {
            // (Of two notes with one Id the first is the note: it is the one that is written back.)
            const QString key = QString::number(n.id);
            if (m_endnoteBlocks.size() < kMostNoteStories && !m_endnoteBlocks.contains(key)) {
                m_endnoteOriginal.insert(n.id, blocksFingerprint(n.blocks, 0, n.blocks.size()));
                m_endnoteBlocks.insert(key, std::move(n.blocks));
            }
        } else if (n.type == QLatin1String("separator")) {
            m_endnoteSeparator = std::move(n.blocks);
        }
    }
    if (m_endnoteSeparator.size() > kMostNoticeParagraphs) m_endnoteSeparator.resize(kMostNoticeParagraphs);
}

// The endnotes: after the document's last text, and at the end of each section whose notes are set there (w:pos sectEnd, of the section or the settings). Each is the file's separator for
// them, then the notes the references name, in the order of the references. (They are paragraphs of the story that the file does not have: they are marked, and not written. A section's break
// is after the last of its notes in the story, and is written on the paragraph before them.) A last section that says it has no endnotes has none. All of them together are at most the
// most the program sets.
void WriteDoc::appendEndnotes()
{
    if (m_endnoteRefs.isEmpty() || m_endnoteBlocks.isEmpty()) return;
    for (const RawChild &ch : m_last.extra)
        if (ch.xml.contains("noEndnote")) return;
    int room = kMostEndnoteParagraphs;
    QSet<int> placed;   // (the ids of the notes set so far: the first copy of a note is the one whose words are edited)
    m_endnoteCapped = false;
    // The paragraphs of the notes the references (by place in the list) name: the separator, then the notes.
    auto tailFor = [&](const QVector<int> &refs) {
        QVector<Block> tail = m_endnoteSeparator;
        if (tail.size() > room) tail.resize(room);
        int made = 0;
        for (int index : refs) {
            const EndnoteRef &ref = m_endnoteRefs[index];
            if (tail.size() >= room) {   // (no more are made than are set: a note of thousands of paragraphs that is referred to thousands of times is the first of them)
                m_endnoteCapped = true;
                break;
            }
            const auto found = m_endnoteBlocks.constFind(ref.id);
            if (found == m_endnoteBlocks.constEnd()) continue;
            const QVector<Block> &note = found.value();
            const int take = int(std::min<qsizetype>(note.size(), room - tail.size()));
            // (A copy whose words are edited in place is whole, and the first of its note: its paragraphs say which copy they are of; the others are shown, and say so by the sign.)
            const int id = ref.id.toInt();
            const int serial = m_nextEndnoteSerial++;
            m_endnoteCopyIds.insert(serial, id);
            const bool editable = take == note.size() && !placed.contains(id);
            if (take < note.size()) m_endnoteCapped = true;
            placed.insert(id);
            for (int i = 0; i < take; ++i) {
                Block b = note[i];
                if (!b.isRaw && b.tag.isEmpty()) b.key = QString::number(editable ? serial : -serial);
                // (The mark that begins the note's text shows the number its reference has.)
                for (Inline &in : b.para.content)
                    if (in.kind == Inline::RunAtom && in.tag == QLatin1String("endnoteRef")) in.display = ref.mark;
                tail.push_back(std::move(b));
                ++s_endnoteParagraphsMade;
            }
            ++made;
        }
        if (made == 0) return QVector<Block>();
        room -= int(tail.size());
        return tail;
    };
    // The references whose notes are at the end of their sections (all but the last section's, whose end is the document's), by section, and the rest.
    const int lastSection = m_endnoteSections - 1;
    QVector<int> atEnd;
    QMap<int, QVector<int>> bySection;
    for (int i = 0; i < m_endnoteRefs.size(); ++i) {
        const EndnoteRef &ref = m_endnoteRefs[i];
        if (ref.atSectionEnd && ref.section < lastSection) bySection[ref.section].push_back(i);
        else atEnd.push_back(i);
    }
    // (Made from the first section on, so that when there is not room for all it is the later ones that go without; set from the last back, so that the places before are still where they were.)
    struct Place {
        int section;
        QVector<Block> tail;
    };
    QVector<Place> places;
    for (auto it = bySection.cbegin(); it != bySection.cend(); ++it)
        if (QVector<Block> tail = tailFor(it.value()); !tail.isEmpty()) places.push_back({it.key(), std::move(tail)});
    const QVector<Block> closing = tailFor(atEnd);
    if (!places.isEmpty()) {
        QVector<int> ends;   // (the paragraphs that carry a section's break, in order)
        for (QTextBlock b = story()->begin(); b.isValid(); b = b.next())
            if (b.blockFormat().boolProperty(flow::fp::SectionBreakAfter)) ends.push_back(b.blockNumber());
        for (int p = int(places.size()) - 1; p >= 0; --p) {
            if (places[p].section >= ends.size()) continue;
            const int endBlock = ends[places[p].section];
            QTextBlock end = story()->findBlockByNumber(endBlock);
            const QTextBlockFormat endFormat = end.blockFormat();
            SectionProps section = endFormat.hasProperty(prop::SectionData) ? unpackSection(storedBytes(endFormat, prop::SectionData)) : m_last;
            const int before = story()->blockCount();
            m_tailSection = &section;
            fillStory(story(), places[p].tail, true, m_endnotePart, false, true, end.position() + end.length() - 1, true);
            m_tailSection = nullptr;
            const int added = story()->blockCount() - before;
            if (added <= 0) continue;
            // The section ends after its last note: its break moves there from the paragraph the text had it on.
            QTextBlockFormat text = story()->findBlockByNumber(endBlock).blockFormat();
            text.clearProperty(flow::fp::SectionBreakAfter);
            text.clearProperty(prop::SectionData);
            QTextCursor(story()->findBlockByNumber(endBlock)).setBlockFormat(text);
            QTextBlockFormat lastNote = story()->findBlockByNumber(endBlock + added).blockFormat();
            lastNote.setProperty(flow::fp::SectionBreakAfter, true);
            if (endFormat.hasProperty(prop::SectionData)) lastNote.setProperty(prop::SectionData, endFormat.property(prop::SectionData));
            QTextCursor(story()->findBlockByNumber(endBlock + added)).setBlockFormat(lastNote);
        }
    }
    if (!closing.isEmpty()) fillStory(story(), closing, true, m_endnotePart, false, true, -1, true);
}

// The endnotes the pages set are taken out of the story: each run of their paragraphs goes (with the break of a section that has them at its end, which goes back to the paragraph before
// them, as the file has it). They are set again from the references the text has then (rebuildEndnoteTails).
void WriteDoc::stripEndnoteTails()
{
    QTextDocument *d = story();
    auto noteBlock = [](const QTextBlock &b) { return b.isValid() && b.blockFormat().hasProperty(prop::EndnoteBlock); };
    QVector<std::pair<int, int>> runs;   // (the numbers of the first and the last paragraph of each run)
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
        if (!noteBlock(b)) continue;
        if (!runs.isEmpty() && runs.last().second == b.blockNumber() - 1) runs.last().second = b.blockNumber();
        else runs.push_back({b.blockNumber(), b.blockNumber()});
    }
    for (int i = int(runs.size()) - 1; i >= 0; --i) {
        const QTextBlock first = d->findBlockByNumber(runs[i].first), last = d->findBlockByNumber(runs[i].second), before = first.previous();
        if (!before.isValid()) continue;   // (notes with no paragraph before them: none are made so)
        const QTextBlockFormat lastFormat = last.blockFormat();
        if (lastFormat.boolProperty(flow::fp::SectionBreakAfter)) {
            QTextBlockFormat bf = before.blockFormat();
            bf.setProperty(flow::fp::SectionBreakAfter, true);
            if (lastFormat.hasProperty(prop::SectionData)) bf.setProperty(prop::SectionData, lastFormat.property(prop::SectionData));
            QTextCursor(before).setBlockFormat(bf);
        }
        // (A run that is the story's last: Qt keeps the last paragraph's format for what is left, not the paragraph before's, which is put back.)
        const int beforeAt = before.position();
        const QTextBlockFormat beforeFormat = before.blockFormat();
        const QTextCharFormat beforeMark = before.charFormat();
        QTextCursor gone(d);
        gone.setPosition(before.position() + before.length() - 1);
        gone.setPosition(last.position() + last.length() - 1, QTextCursor::KeepAnchor);
        gone.removeSelectedText();
        const QTextBlock left = d->findBlock(beforeAt);
        if (left.isValid() && left.blockFormat().hasProperty(prop::EndnoteBlock) && !beforeFormat.hasProperty(prop::EndnoteBlock)) {
            QTextCursor fix(left);
            fix.setBlockFormat(beforeFormat);
            fix.setBlockCharFormat(beforeMark);
        }
    }
    m_tailsStripped = m_tailsStripped || !runs.isEmpty();
}

namespace {
// Whether a reference says that its mark follows (w:customMarkFollows): the note's own mark is the text after it, and no number is shown.
bool customMarkFollows(const QByteArray &reference)
{
    static const QRegularExpression re(QStringLiteral("customMarkFollows\\s*=\\s*\"([^\"]*)\""));
    const QRegularExpressionMatch m = re.match(QString::fromUtf8(reference));
    if (!m.hasMatch()) return false;
    const QString value = m.captured(1);
    return !value.isEmpty() && value != QLatin1String("0") && value != QLatin1String("false") && value != QLatin1String("off");
}
} // namespace

// The endnotes set again after an edit that changed the sections or the references that name them (or took them out of the story): from the references the text has now, in the sections
// it has now, as they were first set, with the words of the notes that were edited, and with the references and the notes' numbers made anew. One step of the undo stack with the edit.
void WriteDoc::rebuildEndnoteTails()
{
    m_tailsStripped = false;
    if (m_endnotesPart != NotesPart::Readable || m_syncing) return;
    QTextDocument *d = story();
    const bool had = m_endnotesAtSectionEnds || !m_endnoteRefs.isEmpty() || tailsOf(d);
    if (m_endnoteBlocks.isEmpty() && !had) return;
    m_buildingEndnotes = true;
    QTextCursor edit(d);
    edit.joinPreviousEditBlock();
    // (What was edited in the notes is taken back from the story as the blocks the notes are made of: the notes are made again from those.)
    captureEndnotes();
    stripEndnoteTails();
    m_tailsStripped = false;
    // Where each section's notes go, and the format they are numbered in: its own properties say, else the settings.
    auto numberingOf = [&](const SectionProps &section) {
        office::wml::NoteNumbering own = office::wml::noteNumberingOf(section, "endnotePr");
        if (own.position.isEmpty()) own.position = m_endnotePosition;
        if (own.format.isEmpty()) own.format = m_endnoteFormat;
        return own;
    };
    QVector<bool> atEnd;
    QVector<QString> formats;
    SectionProps before = m_last;
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (!bf.boolProperty(flow::fp::SectionBreakAfter)) continue;
        if (bf.hasProperty(prop::SectionData)) before = unpackSection(storedBytes(bf, prop::SectionData));
        const office::wml::NoteNumbering n = numberingOf(before);
        atEnd.push_back(n.position == QLatin1String("sectEnd"));
        formats.push_back(n.format);
    }
    {
        const office::wml::NoteNumbering n = numberingOf(m_last);
        atEnd.push_back(n.position == QLatin1String("sectEnd"));
        formats.push_back(n.format);
    }
    m_endnoteRefs.clear();
    struct Renumber {
        int at;
        QTextCharFormat format;
    };
    QVector<Renumber> renumber;
    int section = 0, number = m_endnoteStart - 1;
    for (QTextBlock b = d->begin(); b.isValid() && m_endnoteRefs.size() < kMostEndnotes; b = b.next()) {
        for (QTextBlock::iterator it = b.begin(); !it.atEnd() && m_endnoteRefs.size() < kMostEndnotes; ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            if (!cf.hasProperty(prop::Atom) || cf.hasProperty(prop::TrackedDeleted)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!bytes.contains("endnoteReference")) continue;
            const Inline atom = unpackAtom(bytes);
            if (atom.tag != QLatin1String("endnoteReference")) continue;
            const QString text = f.text();
            for (int k = 0; k < text.size() && m_endnoteRefs.size() < kMostEndnotes; ++k) {
                if (text[k] != QChar::ObjectReplacementCharacter) continue;
                // (Each reference shows the next number, in the format of its section; one that says its mark follows shows none.)
                const bool custom = customMarkFollows(atom.raw);
                const QString shown = custom ? QString() : noteNumberText(formats.value(section, m_endnoteFormat), ++number);
                if (cf.stringProperty(tp::Field) != QStringLiteral("raw:") + shown) {
                    QTextCharFormat nf = cf;
                    nf.setProperty(tp::Field, QStringLiteral("raw:") + shown);
                    renumber.push_back({f.position() + k, nf});
                }
                m_endnoteRefs.push_back({atom.key, shown, section, atEnd.value(section)});
            }
        }
        if (b.blockFormat().boolProperty(flow::fp::SectionBreakAfter)) ++section;
    }
    for (const Renumber &r : std::as_const(renumber)) {
        QTextCursor t(d);
        t.setPosition(r.at);
        t.setPosition(r.at + 1, QTextCursor::KeepAnchor);
        t.setCharFormat(r.format);
    }
    m_endnoteSections = section + 1;
    appendEndnotes();
    edit.endEditBlock();
    m_buildingEndnotes = false;
    m_endnoteRefIndexValid = false;
    m_endnoteSeqValid = false;   // (the next settling looks through the story again, once)
    m_blockCount = d->blockCount();
    syncSections();
}

// Whether the story has paragraphs of the endnotes the pages set.
bool WriteDoc::tailsOf(const QTextDocument *d) const
{
    for (QTextBlock b = d->lastBlock(); b.isValid(); b = b.previous())
        if (b.blockFormat().hasProperty(prop::EndnoteBlock)) return true;
        else if (b.position() < d->characterCount() - 1 - 4096) break;
    return false;
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
    m_noteIds.clear();
    m_dirtyNotes.clear();
    m_createdFootnotes.clear();
    m_footnotesCitedAtOpen.clear();
    m_createdFootnoteStep.clear();
    m_noteStepPending.clear();
    m_footnoteIdFloor = 0;
    m_footnotesPart = NotesPart::Absent;
    m_footnotesPartNew = false;
    m_footnoteRefsValid = false;
    QString part;
    for (const Relationship &r : m_wml.mainRelationships)
        if (!r.external && r.type.endsWith(QLatin1String("/footnotes"))) {
            part = r.target;
            m_notePart = r.target;
            break;
        }
    if (part.isEmpty()) return;
    QVector<NotePart> notes;
    m_footnotesPart = NotesPart::Unusable;
    m_footnotePartHighest = 0;
    if (!partExists(part) || !office::wml::readNotes(partData(part), &notes, &m_footnotePartHighest)) return;
    m_footnotesPart = NotesPart::Readable;
    for (NotePart &n : notes) {
        // (Of two notes with one Id, the first is the note: it is the one that is written back.)
        if (n.type.isEmpty() || n.type == QLatin1String("normal")) {
            const QString key = QStringLiteral("fn%1").arg(n.id);
            if (!m_noteBlocks.contains(key)) m_noteBlocks.insert(key, std::move(n.blocks));
        }
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
    // A note's story is as the pages show it and as it is edited: one story, in the form the text is in (what the program does not edit is a piece kept as written, the number
    // mark is a character of its own), whose steps are in the one history.
    auto doc = std::make_shared<QTextDocument>();
    doc->setDocumentMargin(0);
    doc->setDefaultFont(story()->defaultFont());
    doc->setUndoRedoEnabled(false);
    doc->setProperty("jwNote", true);   // (editops.h's noteTextStart knows a note's story by this)
    fillStory(doc.get(), blocks.value(), true, m_notePart, false, false, -1, true);
    flow::watchEdits(doc.get());
    doc->setUndoRedoEnabled(true);
    doc->setModified(false);
    watchHistory(doc.get());
    m_noteStories.insert(id, doc);
    m_noteIds.insert(doc.get(), id.mid(2).toInt());
    ++s_noteStoriesMade;
    return doc.get();
}

} // namespace jp::write

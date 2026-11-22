#include "office/packageprops.h"
#include "core/inspectops.h"

#include "core/fieldops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>

namespace jp::write {

namespace {

bool hiddenRun(const QTextCharFormat &f) { return f.boolProperty(tp::HiddenText) && !f.hasProperty(prop::TrackedDeleted) && !f.hasProperty(prop::Atom); }

// The runs of hidden text in a story as (position, length), in order.
QVector<QPair<int, int>> hiddenRuns(const QTextDocument *story)
{
    QVector<QPair<int, int>> runs;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            if (!hiddenRun(fr.charFormat())) continue;
            if (!runs.isEmpty() && runs.last().first + runs.last().second == fr.position()) runs.last().second += fr.length();
            else runs.push_back({fr.position(), fr.length()});
        }
    return runs;
}

// The Ids of the links of the part a story is of to addresses outside the file that are not web addresses (a hyperlink is one; a linked picture or object is not); with `endnote`, those of the
// endnotes' part, whose paragraphs are in the body's story.
QSet<QString> fileLinkIds(const WriteDoc &doc, const QTextDocument *story, bool endnote = false)
{
    QSet<QString> ids;
    for (const office::Relationship &r : doc.storyRelationships(story, endnote))
        if (r.external && !r.type.endsWith(QLatin1String("/hyperlink"))) ids.insert(r.id);
    return ids;
}

// Whether the bytes of a picture or object have a quoted value that is one of the Ids (the attribute that names it, r:link or r:id, is the format's).
bool namesAnId(const QByteArray &bytes, const QSet<QString> &ids)
{
    for (qsizetype at = 0; at < bytes.size(); ++at) {
        const char quote = bytes[at];
        if (quote != '"' && quote != '\'') continue;
        const qsizetype end = bytes.indexOf(quote, at + 1);
        if (end < 0) break;
        if (end - at - 1 <= 64 && ids.contains(QString::fromLatin1(bytes.mid(at + 1, end - at - 1)))) return true;
        at = end;
    }
    return false;
}

// The values of the attributes of a picture or object that name a linked file (r:link).
QStringList linkAttributes(const QByteArray &bytes)
{
    static const QRegularExpression link(QStringLiteral(":link\\s*=\\s*(?:\"([^\"]{0,64})\"|'([^']{0,64})')"));
    QStringList out;
    QRegularExpressionMatchIterator it = link.globalMatch(QString::fromLatin1(bytes));
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        out << (m.capturedStart(1) >= 0 ? m.captured(1) : m.captured(2));
    }
    return out;
}

// The pictures and objects of the story that name a link to a file, as (position, length) runs, with the Ids they name (those of the part each is in: an endnote's are the endnotes' own). An
// atom that names a link by an Id its part has no relationship for (the part, or its list of relationships, is missing, or has no such Id) is counted in `unresolved`: where it goes can't be told.
QVector<QPair<int, int>> linkedAtoms(const WriteDoc &doc, const QTextDocument *story, QSet<QPair<bool, QString>> *named = nullptr, int *unresolved = nullptr)
{
    QVector<QPair<int, int>> runs;
    const QSet<QString> body = fileLinkIds(doc, story), notes = story == doc.story() ? fileLinkIds(doc, story, true) : QSet<QString>();
    QSet<QString> everyBody, everyNote;
    if (unresolved) {
        for (const office::Relationship &r : doc.storyRelationships(story)) everyBody.insert(r.id);
        if (story == doc.story())
            for (const office::Relationship &r : doc.storyRelationships(story, true)) everyNote.insert(r.id);
    }
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        const bool endnote = story == doc.story() && doc.endnoteIdOf(b);
        const QSet<QString> &ids = endnote ? notes : body;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            const QTextCharFormat cf = fr.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (unresolved)
                for (const QString &id : linkAttributes(bytes))
                    if (!(endnote ? everyNote : everyBody).contains(id)) {
                        ++*unresolved;
                        break;
                    }
            if (ids.isEmpty() || !namesAnId(bytes, ids)) continue;
            if (named)
                for (const QString &id : ids)
                    if (bytes.contains(id.toLatin1())) named->insert({endnote, id});
            runs.push_back({fr.position(), fr.length()});
        }
    }
    return runs;
}

// The first word of a field the text holds as a group (a field in a header, a footer, or the footnotes keeps its runs whole, so it has no name of its own): the word of its instruction.
QString groupFieldWord(const QByteArray &raw)
{
    // (the instruction may be split over runs: the pieces of the first field's instruction are joined before its first word is read)
    static const QRegularExpression attribute(QStringLiteral("w:instr=\"([^\"]*)\""));
    static const QRegularExpression piece(QStringLiteral("<w:instrText[^>]*>([^<]*)</w:instrText>"));
    static const QRegularExpression word(QStringLiteral("^\\s*(?:&quot;)?\\s*([A-Za-z]+)"));
    const QString text = QString::fromUtf8(raw.left(1 << 16));
    QString instruction;
    const QRegularExpressionMatch a = attribute.match(text), p = piece.match(text);
    if (a.hasMatch() && (!p.hasMatch() || a.capturedStart() < p.capturedStart())) {
        instruction = a.captured(1);
    } else {
        QRegularExpressionMatchIterator it = piece.globalMatch(text);
        int depth = 0;
        static const QRegularExpression fldChar(QStringLiteral("<w:fldChar[^>]*w:fldCharType=\"(begin|end)\""));
        // (only the pieces before a second field begins or the first one ends)
        QRegularExpressionMatchIterator marks = fldChar.globalMatch(text);
        qsizetype stop = text.size();
        while (marks.hasNext()) {
            const QRegularExpressionMatch m = marks.next();
            if (m.captured(1) == QLatin1String("begin") && ++depth == 2) { stop = m.capturedStart(); break; }
            if (m.captured(1) == QLatin1String("end")) { stop = m.capturedStart(); break; }
        }
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            if (m.capturedStart() >= stop) break;
            instruction += m.captured(1);
        }
    }
    const QRegularExpressionMatch m = word.match(instruction);
    return m.hasMatch() ? m.captured(1).toUpper() : QString();
}

bool isFileLinkField(const FieldPlace &f)
{
    static const QSet<QString> names = {QStringLiteral("INCLUDEPICTURE"), QStringLiteral("INCLUDETEXT"), QStringLiteral("LINK"), QStringLiteral("DDE"), QStringLiteral("DDEAUTO")};
    if (f.name.isEmpty() && f.form == FieldPlace::Group) return names.contains(groupFieldWord(f.groupRaw));
    return names.contains(f.name);
}

} // namespace

int personalInfoCount(WriteDoc &doc)
{
    const office::PackageProps p = doc.packageProps();
    int n = 0;
    for (const QString &s : {p.creator, p.lastModifiedBy, p.company, p.manager})
        if (!s.isEmpty()) ++n;
    return n;
}

int removePersonalInfo(WriteDoc &doc)
{
    office::PackageProps p = doc.packageProps();
    const int n = personalInfoCount(doc);
    if (n == 0) return 0;
    p.creator.clear();
    p.lastModifiedBy.clear();
    p.company.clear();
    p.manager.clear();
    doc.setPackageProps(p);
    return n;
}

// Every story that holds text a person reads: the body, and the headers, footers, and footnotes (made editable if they were not).
static QVector<QTextDocument *> allStories(WriteDoc &doc)
{
    QVector<QTextDocument *> stories = {doc.story()};
    for (QTextDocument *s : doc.storiesWithChanges(true))
        if (s && !stories.contains(s)) stories.push_back(s);
    return stories;
}

int hiddenTextCount(WriteDoc &doc)
{
    int n = 0;
    for (const QTextDocument *story : allStories(doc)) n += int(hiddenRuns(story).size());
    return n;
}

int removeHiddenText(WriteDoc &doc)
{
    int n = 0;
    for (QTextDocument *story : allStories(doc)) {
        const QVector<QPair<int, int>> runs = hiddenRuns(story);
        if (runs.isEmpty()) continue;
        QTextCursor c(story);
        c.beginEditBlock();
        for (qsizetype i = runs.size() - 1; i >= 0; --i) {
            c.setPosition(runs[i].first);
            c.setPosition(runs[i].first + runs[i].second, QTextCursor::KeepAnchor);
            c.removeSelectedText();
        }
        c.endEditBlock();
        n += int(runs.size());
    }
    return n;
}

int headerFooterCount(WriteDoc &doc)
{
    int n = 0;
    QSet<const QTextDocument *> seen;
    for (int s = 0; s < doc.sectionCount(); ++s)
        for (int footer = 0; footer < 2; ++footer)
            for (int slot = 0; slot < 3; ++slot) {
                if (!doc.hasOwnHeaderFooter(s, footer, slot)) continue;
                const QTextDocument *story = doc.headerFooterStory(s, footer, slot);
                if (!story || seen.contains(story)) continue;
                seen.insert(story);
                if (!story->toPlainText().trimmed().isEmpty() || story->characterCount() > 1) ++n;
            }
    return n;
}

int clearHeadersAndFooters(WriteDoc &doc)
{
    int n = 0;
    QSet<const QTextDocument *> seen;
    for (int s = 0; s < doc.sectionCount(); ++s)
        for (int footer = 0; footer < 2; ++footer)
            for (int slot = 0; slot < 3; ++slot) {
                if (!doc.hasOwnHeaderFooter(s, footer, slot)) continue;
                QTextDocument *story = doc.editableHeaderFooter(s, footer, slot);
                if (!story || seen.contains(story)) continue;
                seen.insert(story);
                if (story->characterCount() <= 1) continue;
                QTextCursor c(story);
                c.select(QTextCursor::Document);
                c.removeSelectedText();
                ++n;
            }
    return n;
}

int customXmlCount(WriteDoc &doc)
{
    static const QRegularExpression item(QStringLiteral("^/customXml/item[0-9]*\\.xml$"), QRegularExpression::CaseInsensitiveOption);
    int n = 0;
    for (const QString &name : doc.packagePartNames())
        if (item.match(name).hasMatch()) ++n;
    return n;
}

int removeCustomXml(WriteDoc &doc)
{
    const int n = customXmlCount(doc);
    for (const QString &name : doc.packagePartNames())
        if (name.startsWith(QLatin1String("/customXml/"), Qt::CaseInsensitive)) doc.removePackagePart(name);
    if (n > 0) doc.setModified(true);
    return n;
}

int externalLinkCount(WriteDoc &doc)
{
    int n = 0;
    for (QTextDocument *story : allStories(doc)) {
        for (const auto &run : linkedAtoms(doc, story)) n += run.second;
        const StoryScan scan = scanStory(story, false);
        for (const FieldPlace &f : scan.fields) n += isFileLinkField(f);
    }
    return n;
}

ExternalLinkUnread externalLinkUnread(WriteDoc &doc)
{
    ExternalLinkUnread out;
    static const QRegularExpression field(QStringLiteral("(?:w:instr=\"|<w:instrText[^>]*>)\\s*(?:&quot;)?\\s*(?:INCLUDEPICTURE|INCLUDETEXT|LINK|DDEAUTO|DDE)\\b"), QRegularExpression::CaseInsensitiveOption);
    for (QTextDocument *story : allStories(doc)) {
        linkedAtoms(doc, story, nullptr, &out.unresolved);
        for (const FieldPlace &f : scanStory(story, false).fields)
            if (f.name.isEmpty() && f.form == FieldPlace::Group && !isFileLinkField(f) && field.match(QString::fromUtf8(f.groupRaw.left(1 << 16))).hasMatch()) ++out.unreachable;   // (a link kept inside another field)
        const QSet<QString> ids = fileLinkIds(doc, story);
        const QSet<QString> noteIds = story == doc.story() ? fileLinkIds(doc, story, true) : QSet<QString>();
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextCharFormat cf = it.fragment().charFormat();
                if (!cf.hasProperty(prop::Atom)) continue;
                const QByteArray bytes = storedBytes(cf, prop::Atom);
                // (a picture or object that is a link is counted and taken out; a field in a text box, kept whole, is neither)
                const bool endnote = story == doc.story() && doc.endnoteIdOf(b);
                if (!namesAnId(bytes, endnote ? noteIds : ids) && field.match(QString::fromLatin1(bytes.left(1 << 20))).hasMatch()) ++out.unreachable;
            }
    }
    // (the comments: kept as they are in their part)
    const QString comments = doc.mainRelatedPart(QStringLiteral("comments"));
    if (!comments.isEmpty()) {
        const QString text = QString::fromUtf8(doc.partData(comments).left(1 << 22));
        QRegularExpressionMatchIterator it = field.globalMatch(text);
        while (it.hasNext()) {
            it.next();
            ++out.unreachable;
        }
    }
    return out;
}

int removeExternalLinks(WriteDoc &doc)
{
    int n = 0;
    const WriteDoc::StepGroup group(doc);
    for (QTextDocument *story : allStories(doc)) {
        // (the fields first: each becomes its result; the last in the story goes first, so the places of the rest do not move; the count of fields found is the most turns)
        const int most = int(scanStory(story, false).fields.size());
        for (int turn = 0; turn < most; ++turn) {
            const StoryScan scan = scanStory(story, false);
            int last = -1;
            for (int i = 0; i < scan.fields.size(); ++i)
                if (isFileLinkField(scan.fields[i]) && (last < 0 || scan.fields[i].begin > scan.fields[last].begin)) last = i;
            if (last < 0) break;
            QTextCursor c(story);
            c.setPosition(scan.fields[last].begin);
            c.setPosition(std::min(scan.fields[last].end + 1, story->characterCount() - 1), QTextCursor::KeepAnchor);
            if (unlinkFieldsIn(doc, story, c) == 0) break;
            ++n;
        }
        QSet<QPair<bool, QString>> named;
        const QVector<QPair<int, int>> runs = linkedAtoms(doc, story, &named);
        QTextCursor c(story);
        c.beginEditBlock();
        for (qsizetype i = runs.size() - 1; i >= 0; --i) {
            c.setPosition(runs[i].first);
            c.setPosition(runs[i].first + runs[i].second, QTextCursor::KeepAnchor);
            c.removeSelectedText();
            n += runs[i].second;
        }
        c.endEditBlock();
        // (the file's own link to the address goes when nothing names it: see WriteDoc::forgetExternalLink)
        for (const auto &[endnote, id] : std::as_const(named)) doc.forgetStoryLink(story, id, endnote);
    }
    if (n > 0) doc.setModified(true);
    return n;
}

} // namespace jp::write

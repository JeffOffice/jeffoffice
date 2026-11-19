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

// The Ids of the body's links to addresses outside the file that are not web addresses (a hyperlink is one; a linked picture or object is not).
QSet<QString> fileLinkIds(const WriteDoc &doc)
{
    QSet<QString> ids;
    for (const office::Relationship &r : doc.mainRelationships())
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

// The pictures and objects of the body that name a link to a file, as (position, length) runs, with the Ids they name.
QVector<QPair<int, int>> linkedAtoms(const WriteDoc &doc, const QSet<QString> &ids, QSet<QString> *named = nullptr)
{
    QVector<QPair<int, int>> runs;
    if (ids.isEmpty()) return runs;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            const QTextCharFormat cf = fr.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!namesAnId(bytes, ids)) continue;
            if (named)
                for (const QString &id : ids)
                    if (bytes.contains(id.toLatin1())) named->insert(id);
            runs.push_back({fr.position(), fr.length()});
        }
    return runs;
}

bool isFileLinkField(const FieldPlace &f)
{
    static const QSet<QString> names = {QStringLiteral("INCLUDEPICTURE"), QStringLiteral("INCLUDETEXT"), QStringLiteral("LINK"), QStringLiteral("DDE"), QStringLiteral("DDEAUTO")};
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
    for (const auto &run : linkedAtoms(doc, fileLinkIds(doc))) n += run.second;
    const StoryScan scan = scanStory(doc.story(), false);
    for (const FieldPlace &f : scan.fields) n += isFileLinkField(f);
    return n;
}

int removeExternalLinks(WriteDoc &doc)
{
    int n = 0;
    const WriteDoc::StepGroup group(doc);
    // (the fields first: each becomes its result; the last in the story goes first, so the places of the rest do not move; the count of fields found is the most turns)
    const int most = int(scanStory(doc.story(), false).fields.size());
    for (int turn = 0; turn < most; ++turn) {
        const StoryScan scan = scanStory(doc.story(), false);
        int last = -1;
        for (int i = 0; i < scan.fields.size(); ++i)
            if (isFileLinkField(scan.fields[i]) && (last < 0 || scan.fields[i].begin > scan.fields[last].begin)) last = i;
        if (last < 0) break;
        QTextCursor c(doc.story());
        c.setPosition(scan.fields[last].begin);
        c.setPosition(std::min(scan.fields[last].end + 1, doc.story()->characterCount() - 1), QTextCursor::KeepAnchor);
        if (unlinkFields(doc, c) == 0) break;
        ++n;
    }
    const QSet<QString> ids = fileLinkIds(doc);
    QSet<QString> named;
    const QVector<QPair<int, int>> runs = linkedAtoms(doc, ids, &named);
    QTextCursor c(doc.story());
    c.beginEditBlock();
    for (qsizetype i = runs.size() - 1; i >= 0; --i) {
        c.setPosition(runs[i].first);
        c.setPosition(runs[i].first + runs[i].second, QTextCursor::KeepAnchor);
        c.removeSelectedText();
        n += runs[i].second;
    }
    c.endEditBlock();
    // (the file's own link to the address goes when nothing names it: see WriteDoc::forgetExternalLink)
    for (const QString &id : std::as_const(named)) doc.forgetExternalLink(id);
    if (n > 0) doc.setModified(true);
    return n;
}

} // namespace jp::write

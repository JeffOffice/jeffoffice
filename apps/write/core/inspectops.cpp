#include "core/inspectops.h"

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

int hiddenTextCount(WriteDoc &doc) { return int(hiddenRuns(doc.story()).size()); }

int removeHiddenText(WriteDoc &doc)
{
    const QVector<QPair<int, int>> runs = hiddenRuns(doc.story());
    if (runs.isEmpty()) return 0;
    QTextCursor c(doc.story());
    c.beginEditBlock();
    for (qsizetype i = runs.size() - 1; i >= 0; --i) {
        c.setPosition(runs[i].first);
        c.setPosition(runs[i].first + runs[i].second, QTextCursor::KeepAnchor);
        c.removeSelectedText();
    }
    c.endEditBlock();
    return int(runs.size());
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

} // namespace jp::write

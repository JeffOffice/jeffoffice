#include "core/gotoops.h"

#include "core/editops.h"
#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>

using namespace jp::office::wml;

namespace jp::write {

namespace {

constexpr int kMostBookmarksFound = 20000;   // a bookmark can be gone to by name though the list offers fewer (the most a document's fields look through)

// A number typed for a page, section, line, or note, counted from `current` when it begins with + or -. False, with a sentence, when it is not a number or is out of reach.
bool numberOf(const QString &what, int current, int *out, QString *problem)
{
    const QString t = what.trimmed();
    bool ok = false;
    const qint64 v = t.toLongLong(&ok);
    if (t.isEmpty() || !ok || v > kMostGoToNumber || v < -kMostGoToNumber) {
        if (problem) *problem = QCoreApplication::translate("jp::write::GoTo", "Type a number.");
        return false;
    }
    const bool relative = t.startsWith(QLatin1Char('+')) || t.startsWith(QLatin1Char('-'));
    *out = int(relative ? qint64(current) + v : v);
    return true;
}

QString headingText(const QTextBlock &b)
{
    QString t = b.text();
    t.remove(QChar::ObjectReplacementCharacter);
    return t.simplified().left(120);
}

bool isHeading(const QTextBlock &b)
{
    const QTextBlockFormat bf = b.blockFormat();
    return bf.hasProperty(prop::Outline) && bf.intProperty(prop::Outline) < 9 && !bf.hasProperty(prop::EndnoteBlock) && !bf.hasProperty(prop::RawBlock);
}

QVector<GoToItem> headings(WriteDoc &doc)
{
    QVector<GoToItem> out;
    const int limit = editableEnd(doc.story());
    for (QTextBlock b = doc.story()->begin(); b.isValid() && b.position() < limit && out.size() < kMostGoToItems; b = b.next()) {
        if (!isHeading(b)) continue;
        const QString t = headingText(b);
        if (t.isEmpty()) continue;
        out.push_back({t, b.position()});
    }
    return out;
}

// The bookmarks of the text in order (the first of a name), at most `limit` of them.
QVector<GoToItem> bookmarks(WriteDoc &doc, int limit = kMostGoToItems)
{
    static const QRegularExpression named(QStringLiteral("[\\s:]name\\s*=\\s*\"([^\"]*)\""));
    QVector<GoToItem> out;
    QSet<QString> seen;
    for (QTextBlock b = doc.story()->begin(); b.isValid() && out.size() < limit; b = b.next())
        for (auto it = b.begin(); !it.atEnd() && out.size() < limit; ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!bytes.contains("bookmarkStart")) continue;
            const Inline atom = unpackAtom(bytes);
            if (atom.tag != QLatin1String("bookmarkStart")) continue;
            const QString name = named.match(QString::fromUtf8(atom.raw.left(1000))).captured(1);
            if (name.isEmpty() || name.startsWith(QLatin1Char('_')) || seen.contains(name.toLower())) continue;
            seen.insert(name.toLower());
            out.push_back({name.left(255), it.fragment().position()});
        }
    return out;
}

QVector<int> footnotePositions(WriteDoc &doc)
{
    QVector<int> out;
    for (int id : doc.citedFootnotes()) {
        out += doc.footnoteReferences(id);
        if (out.size() >= kMostGoToItems) break;
    }
    std::sort(out.begin(), out.end());
    if (out.size() > kMostGoToItems) out.resize(kMostGoToItems);
    return out;
}

int startOfSection(WriteDoc &doc, int index)
{
    if (index <= 0) return 0;
    int seen = 0;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        if (b.blockFormat().boolProperty(flow::fp::SectionBreakAfter) && ++seen == index) return b.next().isValid() ? b.next().position() : -1;
    return -1;
}

} // namespace

QVector<GoToItem> goToItems(WriteDoc &doc, GoToKind kind)
{
    QVector<GoToItem> out;
    switch (kind) {
    case GoToKind::Bookmark: return bookmarks(doc);
    case GoToKind::Heading: return headings(doc);
    case GoToKind::Footnote: {
        const QVector<int> at = footnotePositions(doc);
        for (int i = 0; i < at.size(); ++i) out.push_back({QString::number(i + 1), at[i]});
        return out;
    }
    case GoToKind::Section:
        for (int i = 0; i < doc.sectionCount() && i < kMostGoToItems; ++i) out.push_back({QString::number(i + 1), std::max(0, startOfSection(doc, i))});
        return out;
    default: return out;
    }
}

int goToPosition(WriteDoc &doc, GoToKind kind, const QString &what, int from, QString *problem)
{
    auto none = [&](const QString &why) {
        if (problem) *problem = why;
        return -1;
    };
    int n = 0;
    switch (kind) {
    case GoToKind::Page: {
        flow::Paginator &pager = doc.pager();
        if (!numberOf(what, pager.pageOfPosition(from) + 1, &n, problem)) return -1;
        if (n < 1) return none(QCoreApplication::translate("jp::write::GoTo", "There is no such page."));
        pager.paginate(n - 1);
        if (n > pager.pagesKnown()) return none(QCoreApplication::translate("jp::write::GoTo", "There is no such page."));
        return pager.page(n - 1).start.pos;
    }
    case GoToKind::Section: {
        if (!numberOf(what, doc.sectionAt(from) + 1, &n, problem)) return -1;
        if (n < 1 || n > doc.sectionCount()) return none(QCoreApplication::translate("jp::write::GoTo", "There is no such section."));
        const int at = startOfSection(doc, n - 1);
        return at < 0 ? none(QCoreApplication::translate("jp::write::GoTo", "There is no such section.")) : at;
    }
    case GoToKind::Line: {
        if (!numberOf(what, 0, &n, problem)) return -1;
        if (n < 1) return none(QCoreApplication::translate("jp::write::GoTo", "There is no such line."));
        flow::Paginator &pager = doc.pager();
        int left = n;
        for (int page = 0;; ++page) {
            pager.paginate(page);
            if (page >= pager.pagesKnown()) break;
            const auto lines = pager.lines(page);
            if (left <= lines.size()) return lines[left - 1].pos;
            left -= int(lines.size());
        }
        return none(QCoreApplication::translate("jp::write::GoTo", "There is no such line."));
    }
    case GoToKind::Bookmark: {
        const QString name = what.trimmed();
        if (name.isEmpty()) return none(QCoreApplication::translate("jp::write::GoTo", "Type the name of a bookmark."));
        for (const GoToItem &item : bookmarks(doc, kMostBookmarksFound))
            if (QString::compare(item.label, name, Qt::CaseInsensitive) == 0) return item.position;
        return none(QCoreApplication::translate("jp::write::GoTo", "There is no such bookmark."));
    }
    case GoToKind::Footnote: {
        const QVector<int> at = footnotePositions(doc);
        int current = 0;
        for (int p : at) current += p < from ? 1 : 0;
        if (!numberOf(what, current, &n, problem)) return -1;
        if (n < 1 || n > at.size()) return none(QCoreApplication::translate("jp::write::GoTo", "There is no such footnote."));
        return at[n - 1];
    }
    case GoToKind::Heading: {
        const QVector<GoToItem> all = headings(doc);
        bool number = false;
        what.trimmed().toLongLong(&number);
        if (number) {
            int current = 0;
            for (const GoToItem &h : all) current += h.position <= from ? 1 : 0;
            if (!numberOf(what, current, &n, problem)) return -1;
            if (n < 1 || n > all.size()) return none(QCoreApplication::translate("jp::write::GoTo", "There is no such heading."));
            return all[n - 1].position;
        }
        const QString part = what.trimmed();
        if (part.isEmpty()) return none(QCoreApplication::translate("jp::write::GoTo", "Type a number or part of a heading."));
        for (const GoToItem &h : all)
            if (h.label.contains(part, Qt::CaseInsensitive)) return h.position;
        return none(QCoreApplication::translate("jp::write::GoTo", "There is no such heading."));
    }
    }
    return none(QCoreApplication::translate("jp::write::GoTo", "There is no such place."));
}

} // namespace jp::write

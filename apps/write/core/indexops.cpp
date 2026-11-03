#include "core/indexops.h"

#include "core/editops.h"
#include "text/textprops.h"

#include <QCollator>
#include <QCoreApplication>
#include <QMap>
#include <QSet>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <functional>

namespace jp::write {

QString noIndexEntries() { return QCoreApplication::translate("jp::write::Fields", "No index entries found."); }
QString noAuthorities() { return QCoreApplication::translate("jp::write::Fields", "No table of authorities entries found."); }

namespace {

const QChar kDash(0x2013);

QString escapedQuotes(QString s) { return s.replace(QLatin1Char('"'), QStringLiteral("\\\"")); }

// A switch's argument as a separator: the field's text for a tab is a tab character or "\t".
QString separatorOf(const QString &argument, const QString &fallback)
{
    if (argument.isEmpty()) return fallback;
    QString s = argument;
    s.replace(QStringLiteral("\\t"), QStringLiteral("\t"));
    return s;
}

struct PageMark {
    QString text;
    int index = 0;
    bool bold = false, italic = false;
};

// The pages of a place or of a bookmark's text, "5" or "5-7"; empty where the pages are not laid out.
PageMark pagesOf(const StoryScan &scan, const PageRefLookup &pageOf, int position, const QString &bookmark, const QString &rangeSeparator)
{
    PageMark m;
    if (!pageOf) return m;
    if (!bookmark.isEmpty()) {
        const auto found = scan.bookmarkByName.constFind(bookmark.trimmed().toLower());
        if (found != scan.bookmarkByName.constEnd()) {
            const BookmarkSpan &b = scan.bookmarks[found.value()];
            const PageRef from = pageOf(b.start);
            const PageRef to = pageOf(b.end >= 0 ? std::max(b.start, b.end - 1) : b.start);
            m.index = from.index;
            m.text = to.index != from.index && !to.text.isEmpty() ? from.text + rangeSeparator + to.text : from.text;
            return m;
        }
    }
    const PageRef r = pageOf(position);
    m.index = r.index;
    m.text = r.text;
    return m;
}

bool within(const FieldPlace &f, const FieldPlace &self) { return f.begin > self.begin && f.begin < self.end; }

// ---- the index

struct Node {
    QString text;
    QVector<PageMark> pages;
    QStringList crossReferences;
    QMap<QString, Node> children;   // by the entry text in lower case
    QStringList order;              // (the keys, in the order they are sorted)
};

QString letterOf(const QString &text)
{
    for (const QChar ch : text)
        if (ch.isLetter()) return QString(ch.toUpper());
        else if (ch.isDigit()) return QStringLiteral("#");
        else if (!ch.isSpace() && !ch.isPunct()) return QStringLiteral("#");
    return QStringLiteral("#");
}

QStringList splitLevels(const QString &entry)
{
    QStringList out;
    QString cur;
    for (int i = 0; i < entry.size(); ++i) {
        if (entry[i] == QLatin1Char('\\') && i + 1 < entry.size() && entry[i + 1] == QLatin1Char(':')) {
            cur += QLatin1Char(':');
            ++i;
        } else if (entry[i] == QLatin1Char(':')) {
            out << cur.simplified();
            cur.clear();
        } else {
            cur += entry[i];
        }
    }
    out << cur.simplified();
    out.removeAll(QString());
    if (out.size() > 9) out = out.mid(0, 9);
    return out;
}

void addPage(QVector<PageMark> *pages, const PageMark &m)
{
    if (m.text.isEmpty()) return;
    for (PageMark &p : *pages)
        if (p.text == m.text) {
            p.bold = p.bold || m.bold;
            p.italic = p.italic || m.italic;
            return;
        }
    pages->push_back(m);
}

void sortKeys(Node *node, const QCollator &collator)
{
    node->order = node->children.keys();
    std::sort(node->order.begin(), node->order.end(), [&](const QString &a, const QString &b) { return collator.compare(node->children[a].text, node->children[b].text) < 0; });
    for (auto it = node->children.begin(); it != node->children.end(); ++it) sortKeys(&it.value(), collator);
}

QVector<GenRun> pagesRuns(const Node &n, const QString &entrySeparator, const QString &pageSeparator, const QString &crossSeparator)
{
    QVector<GenRun> out;
    if (!n.crossReferences.isEmpty()) {
        out.push_back(GenRun{crossSeparator + n.crossReferences.join(QStringLiteral("; ")), false, false});
        return out;
    }
    if (n.pages.isEmpty()) return out;
    QVector<PageMark> pages = n.pages;
    std::stable_sort(pages.begin(), pages.end(), [](const PageMark &a, const PageMark &b) { return a.index < b.index; });
    out.push_back(GenRun{entrySeparator, false, false});
    for (int i = 0; i < pages.size(); ++i) {
        if (i > 0) out.push_back(GenRun{pageSeparator, false, false});
        out.push_back(GenRun{pages[i].text, pages[i].bold, pages[i].italic});
    }
    return out;
}

void emit(const Node &n, int level, bool runIn, const QString &e, const QString &l, const QString &k, QVector<GenLine> *out, QVector<GenRun> *runInto)
{
    for (const QString &key : n.order) {
        const Node &c = n.children[key];
        if (runIn && level >= 2) {
            // (One line for the entry: its subentries run on after it.)
            runInto->push_back(GenRun{runInto->isEmpty() ? QStringLiteral(": ") : QStringLiteral("; "), false, false});
            runInto->push_back(GenRun{c.text, false, false});
            *runInto += pagesRuns(c, QStringLiteral(", "), l, k);
            emit(c, level + 1, runIn, e, l, k, out, runInto);
            continue;
        }
        GenLine line;
        line.styleName = QStringLiteral("index %1").arg(std::min(level, 9));
        line.styleId = QStringLiteral("Index%1").arg(std::min(level, 9));
        line.runs.push_back(GenRun{c.text, false, false});
        line.runs += pagesRuns(c, e, l, k);
        const int at = int(out->size());
        out->push_back(line);
        if (runIn) {
            QVector<GenRun> more;
            emit(c, level + 1, runIn, e, l, k, out, &more);
            (*out)[at].runs += more;
        } else {
            emit(c, level + 1, runIn, e, l, k, out, nullptr);
        }
        (*out)[at].rightTab = e == QLatin1String("\t") && (*out)[at].runs.size() > 1;
    }
}

} // namespace

QVector<GenLine> indexLines(WriteDoc &doc, const StoryScan &scan, int field, const fields::Code &code, const PageRefLookup &pageOf)
{
    const FieldPlace &self = scan.fields[field];
    Q_UNUSED(doc);
    const QString type = code.argumentOf(QLatin1Char('f')).trimmed();
    const QString rangeSeparator = separatorOf(code.argumentOf(QLatin1Char('g')), QString(kDash));
    const QString entrySeparator = separatorOf(code.argumentOf(QLatin1Char('e')), QStringLiteral(", "));
    const QString pageSeparator = separatorOf(code.argumentOf(QLatin1Char('l')), QStringLiteral(", "));
    const QString crossSeparator = separatorOf(code.argumentOf(QLatin1Char('k')), QStringLiteral(". "));
    const bool runIn = code.has(QLatin1Char('r'));
    int limitFrom = 0, limitTo = std::numeric_limits<int>::max();
    if (code.has(QLatin1Char('b'))) {
        const auto found = scan.bookmarkByName.constFind(code.argumentOf(QLatin1Char('b')).trimmed().toLower());
        if (found != scan.bookmarkByName.constEnd()) {
            limitFrom = scan.bookmarks[found.value()].start;
            if (scan.bookmarks[found.value()].end >= 0) limitTo = scan.bookmarks[found.value()].end;
        }
    }
    QChar rangeLo = QLatin1Char('A'), rangeHi = QLatin1Char('Z');
    bool ranged = false;
    if (code.has(QLatin1Char('p'))) {
        const QString r = code.argumentOf(QLatin1Char('p')).trimmed().toUpper();
        if (r.size() >= 1 && r[0].isLetter()) {
            rangeLo = rangeHi = r[0];
            if (r.size() >= 3 && r[2].isLetter()) rangeHi = r[2];
            ranged = true;
        }
    }
    Node root;
    int made = 0;
    for (int i = 0; i < scan.fields.size() && made < kMostIndexEntries; ++i) {
        const FieldPlace &f = scan.fields[i];
        if (f.name != QLatin1String("XE") || within(f, self) || f.begin < limitFrom || f.begin > limitTo) continue;
        const fields::Code xe = fields::parseCode(scan.instructionOf(i));
        if (xe.name != QLatin1String("XE") || xe.arguments.isEmpty()) continue;
        if (xe.argumentOf(QLatin1Char('f')).trimmed().compare(type, Qt::CaseInsensitive) != 0) continue;
        const QStringList levels = splitLevels(xe.arguments.first());
        if (levels.isEmpty()) continue;
        if (ranged) {
            const QChar first = letterOf(levels.first())[0];
            if (first < rangeLo || first > rangeHi) continue;
        }
        Node *node = &root;
        for (const QString &level : levels) {
            Node &child = node->children[level.toLower()];
            if (child.text.isEmpty()) child.text = level;
            node = &child;
        }
        ++made;
        if (xe.has(QLatin1Char('t'))) {
            const QString text = xe.argumentOf(QLatin1Char('t')).trimmed();
            if (!text.isEmpty() && !node->crossReferences.contains(text)) node->crossReferences << text;
            continue;
        }
        PageMark m = pagesOf(scan, pageOf, f.begin, xe.argumentOf(QLatin1Char('r')), rangeSeparator);
        m.bold = xe.has(QLatin1Char('b'));
        m.italic = xe.has(QLatin1Char('i'));
        addPage(&node->pages, m);
    }
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    sortKeys(&root, collator);
    QVector<GenLine> out;
    const bool headings = code.has(QLatin1Char('h'));
    const QString headingTemplate = code.argumentOf(QLatin1Char('h'));
    QString lastLetter;
    for (const QString &key : root.order) {
        const Node &top = root.children[key];
        if (headings) {
            const QString letter = letterOf(top.text);
            if (letter != lastLetter) {
                lastLetter = letter;
                GenLine h;
                h.styleName = QStringLiteral("index heading");
                h.styleId = QStringLiteral("IndexHeading");
                QString shown = headingTemplate.isEmpty() ? letter : headingTemplate;
                if (!headingTemplate.isEmpty()) shown.replace(QLatin1Char('A'), letter);
                h.runs.push_back(GenRun{shown, false, false});
                out.push_back(h);
            }
        }
        Node single;
        single.children.insert(key, top);
        single.order = {key};
        emit(single, 1, runIn, entrySeparator, pageSeparator, crossSeparator, &out, nullptr);
    }
    return out;
}

QString indexMarkCode(const IndexMark &m)
{
    QString entry = m.main.simplified();
    entry.replace(QLatin1Char(':'), QStringLiteral("\\:"));
    QString sub = m.sub.simplified();
    if (!sub.isEmpty()) entry += QLatin1Char(':') + sub.replace(QLatin1Char(':'), QStringLiteral("\\:"));
    QString code = QStringLiteral("XE \"%1\"").arg(escapedQuotes(entry));
    if (m.page == IndexMark::Range && !m.bookmark.trimmed().isEmpty()) code += QStringLiteral(" \\r \"%1\"").arg(escapedQuotes(m.bookmark.trimmed()));
    if (m.page == IndexMark::CrossReference && !m.crossReference.trimmed().isEmpty()) code += QStringLiteral(" \\t \"%1\"").arg(escapedQuotes(m.crossReference.trimmed()));
    if (m.bold) code += QStringLiteral(" \\b");
    if (m.italic) code += QStringLiteral(" \\i");
    if (!m.type.trimmed().isEmpty()) code += QStringLiteral(" \\f \"%1\"").arg(escapedQuotes(m.type.trimmed().left(1)));
    return code;
}

bool markIndexEntry(WriteDoc &doc, QTextCursor &c, const IndexMark &mark)
{
    if (mark.main.simplified().isEmpty()) return false;
    QTextCursor at = c;
    at.setPosition(c.selectionEnd());
    if (!insertField(doc, at, indexMarkCode(mark))) return false;
    c = at;
    return true;
}

int markAllIndexEntries(WriteDoc &doc, const IndexMark &mark)
{
    const QString word = mark.main.simplified();
    if (word.isEmpty()) return 0;
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story, false);
    // The places, from the last to the first so that the earlier stay; not inside a field, nor in a place a field would break.
    QVector<int> places;
    const QString plain = scan.plain;
    QVector<bool> covered(int(plain.size()) + 2, false);   // (the places a field's marks, code, and result take)
    QSet<int> marksAt;                                     // (where an XE field begins)
    for (const FieldPlace &f : scan.fields) {
        for (int k = std::max(0, f.begin); k <= f.end && k < covered.size(); ++k) covered[k] = true;
        if (f.name == QLatin1String("XE")) marksAt.insert(f.begin);
    }
    for (qsizetype at = plain.indexOf(word, 0, Qt::CaseSensitive); at >= 0 && places.size() < kMostIndexEntries; at = plain.indexOf(word, at + word.size(), Qt::CaseSensitive)) {
        const int begin = int(at), end = int(at) + int(word.size());
        bool inField = false;
        for (int k = begin; k < end && !inField; ++k) inField = covered[k];
        if (!inField && !marksAt.contains(end)) places << end;   // (a place that has its mark right after it is marked already)
    }
    if (places.isEmpty()) return 0;
    WriteDoc::StepGroup group(doc);
    int count = 0;
    const QString code = indexMarkCode(mark);
    for (int i = int(places.size()) - 1; i >= 0; --i) {
        QTextCursor c(story);
        c.setPosition(places[i]);
        if (insertField(doc, c, code)) ++count;
    }
    return count;
}

QStringList markedIndexEntries(WriteDoc &doc)
{
    const StoryScan scan = scanStory(doc.story(), false);
    QStringList out;
    for (int i = 0; i < scan.fields.size(); ++i)
        if (scan.fields[i].name == QLatin1String("XE")) {
            const fields::Code xe = fields::parseCode(scan.instructionOf(i));
            if (!xe.arguments.isEmpty() && !out.contains(xe.arguments.first()) && out.size() < 5000) out << xe.arguments.first();
        }
    out.sort(Qt::CaseInsensitive);
    return out;
}

QString indexCode(const IndexOptions &o)
{
    QString code = QStringLiteral(" INDEX");
    if (o.runIn) code += QStringLiteral(" \\r");
    if (o.headings) code += QStringLiteral(" \\h \"A\"");
    if (o.rightAlign) code += QStringLiteral(" \\e \"\t\"");
    code += QStringLiteral(" \\c \"%1\"").arg(std::clamp(o.columns, 1, 4));
    if (!o.type.trimmed().isEmpty()) code += QStringLiteral(" \\f \"%1\"").arg(escapedQuotes(o.type.trimmed().left(1)));
    return code + QLatin1Char(' ');
}

bool insertIndex(WriteDoc &doc, QTextCursor &c, const IndexOptions &options)
{
    return insertTableFromCode(doc, c, indexCode(options), options.leader, nullptr);
}

// ---- the table of authorities

QString authorityCategoryName(int category)
{
    switch (category) {
    case 1: return QCoreApplication::translate("jp::write::Authorities", "Cases");
    case 2: return QCoreApplication::translate("jp::write::Authorities", "Statutes");
    case 3: return QCoreApplication::translate("jp::write::Authorities", "Other Authorities");
    case 4: return QCoreApplication::translate("jp::write::Authorities", "Rules");
    case 5: return QCoreApplication::translate("jp::write::Authorities", "Treatises");
    case 6: return QCoreApplication::translate("jp::write::Authorities", "Regulations");
    case 7: return QCoreApplication::translate("jp::write::Authorities", "Constitutional Provisions");
    default: return QCoreApplication::translate("jp::write::Authorities", "Category %1").arg(category);
    }
}

namespace {

struct Authority {
    QString longText;
    QString shortText;
    QVector<PageMark> pages;
};

} // namespace

QVector<GenLine> authoritiesLines(WriteDoc &doc, const StoryScan &scan, int field, const fields::Code &code, const PageRefLookup &pageOf)
{
    const FieldPlace &self = scan.fields[field];
    Q_UNUSED(doc);
    const QString rangeSeparator = separatorOf(code.argumentOf(QLatin1Char('g')), QString(kDash));
    const QString entrySeparator = separatorOf(code.argumentOf(QLatin1Char('e')), QStringLiteral("\t"));
    const QString pageSeparator = separatorOf(code.argumentOf(QLatin1Char('l')), QStringLiteral(", "));
    const bool passim = code.has(QLatin1Char('p'));
    const bool headings = code.has(QLatin1Char('h'));
    int limitFrom = 0, limitTo = std::numeric_limits<int>::max();
    if (code.has(QLatin1Char('b'))) {
        const auto found = scan.bookmarkByName.constFind(code.argumentOf(QLatin1Char('b')).trimmed().toLower());
        if (found != scan.bookmarkByName.constEnd()) {
            limitFrom = scan.bookmarks[found.value()].start;
            if (scan.bookmarks[found.value()].end >= 0) limitTo = scan.bookmarks[found.value()].end;
        }
    }
    int wanted = 0;
    if (code.has(QLatin1Char('c'))) wanted = std::clamp(code.argumentOf(QLatin1Char('c')).trimmed().toInt(), 0, 16);
    QMap<int, QMap<QString, Authority>> byCategory;   // category -> long citation in lower case -> the authority
    int made = 0;
    for (int i = 0; i < scan.fields.size() && made < kMostIndexEntries; ++i) {
        const FieldPlace &f = scan.fields[i];
        if (f.name != QLatin1String("TA") || within(f, self) || f.begin < limitFrom || f.begin > limitTo) continue;
        const fields::Code ta = fields::parseCode(scan.instructionOf(i));
        if (ta.name != QLatin1String("TA")) continue;
        const QString longText = ta.argumentOf(QLatin1Char('l')).simplified();
        if (longText.isEmpty()) continue;
        const int category = std::clamp(ta.has(QLatin1Char('c')) ? ta.argumentOf(QLatin1Char('c')).trimmed().toInt() : 1, 1, 16);
        if (wanted > 0 && category != wanted) continue;
        Authority &a = byCategory[category][longText.toLower()];
        if (a.longText.isEmpty()) a.longText = longText;
        ++made;
        PageMark m = pagesOf(scan, pageOf, f.begin, ta.argumentOf(QLatin1Char('r')), rangeSeparator);
        m.bold = ta.has(QLatin1Char('b'));
        m.italic = ta.has(QLatin1Char('i'));
        addPage(&a.pages, m);
    }
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    QVector<GenLine> out;
    for (auto cat = byCategory.cbegin(); cat != byCategory.cend(); ++cat) {
        if (headings) {
            GenLine h;
            h.styleName = QStringLiteral("toa heading");
            h.styleId = QStringLiteral("TOAHeading");
            h.runs.push_back(GenRun{authorityCategoryName(cat.key()), false, false});
            out.push_back(h);
        }
        QVector<Authority> list;
        for (const Authority &a : cat.value()) list.push_back(a);
        std::sort(list.begin(), list.end(), [&](const Authority &a, const Authority &b) { return collator.compare(a.longText, b.longText) < 0; });
        for (Authority &a : list) {
            GenLine line;
            line.styleName = QStringLiteral("table of authorities");
            line.styleId = QStringLiteral("TableofAuthorities");
            line.runs.push_back(GenRun{a.longText, false, false});
            if (!a.pages.isEmpty()) {
                std::stable_sort(a.pages.begin(), a.pages.end(), [](const PageMark &x, const PageMark &y) { return x.index < y.index; });
                line.runs.push_back(GenRun{entrySeparator, false, false});
                if (passim && a.pages.size() >= 5) {
                    line.runs.push_back(GenRun{QCoreApplication::translate("jp::write::Authorities", "passim"), false, true});
                } else {
                    for (int i = 0; i < a.pages.size(); ++i) {
                        if (i > 0) line.runs.push_back(GenRun{pageSeparator, false, false});
                        line.runs.push_back(GenRun{a.pages[i].text, a.pages[i].bold, a.pages[i].italic});
                    }
                }
                line.rightTab = entrySeparator == QLatin1String("\t");
            }
            out.push_back(line);
        }
    }
    return out;
}

QString authorityMarkCode(const AuthorityMark &m)
{
    QString code = QStringLiteral("TA \\l \"%1\"").arg(escapedQuotes(m.longCitation.simplified()));
    if (!m.shortCitation.simplified().isEmpty()) code += QStringLiteral(" \\s \"%1\"").arg(escapedQuotes(m.shortCitation.simplified()));
    code += QStringLiteral(" \\c %1").arg(std::clamp(m.category, 1, 16));
    if (m.bold) code += QStringLiteral(" \\b");
    if (m.italic) code += QStringLiteral(" \\i");
    return code;
}

bool markAuthority(WriteDoc &doc, QTextCursor &c, const AuthorityMark &mark)
{
    if (mark.longCitation.simplified().isEmpty()) return false;
    QTextCursor at = c;
    at.setPosition(c.selectionEnd());
    if (!insertField(doc, at, authorityMarkCode(mark))) return false;
    c = at;
    return true;
}

QString authoritiesCode(const AuthoritiesOptions &o)
{
    QString code = QStringLiteral(" TOA");
    if (o.headings) code += QStringLiteral(" \\h");
    if (o.category > 0) code += QStringLiteral(" \\c \"%1\"").arg(std::clamp(o.category, 1, 16));
    if (o.passim) code += QStringLiteral(" \\p");
    if (o.rightAlign) code += QStringLiteral(" \\e \"\t\"");
    return code + QLatin1Char(' ');
}

bool insertTableOfAuthorities(WriteDoc &doc, QTextCursor &c, const AuthoritiesOptions &options)
{
    return insertTableFromCode(doc, c, authoritiesCode(options), options.leader, nullptr);
}

namespace {

// The first and the last place of the fields of this name the selection touches.
bool touched(WriteDoc &doc, const QTextCursor &c, const QString &name, int *from, int *to, int *count)
{
    const StoryScan scan = scanStory(doc.story(), false);
    *count = 0;
    *from = *to = -1;
    for (const FieldPlace &f : scan.fields)
        if (f.name == name && f.begin <= c.selectionEnd() && f.end >= c.selectionStart()) {
            ++*count;
            if (*from < 0) *from = f.begin;
            *to = f.end;
        }
    return *count > 0;
}

} // namespace

int updateIndexOrAuthorities(WriteDoc &doc, const QTextCursor &c, bool authorities)
{
    int from, to, count;
    if (!touched(doc, c, authorities ? QStringLiteral("TOA") : QStringLiteral("INDEX"), &from, &to, &count)) return 0;
    UpdateOptions o;
    o.from = from;
    o.to = to;
    updateFields(doc, o);
    return count;
}

bool removeIndexOrAuthorities(WriteDoc &doc, const QTextCursor &c, bool authorities)
{
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story, false);
    const QString name = authorities ? QStringLiteral("TOA") : QStringLiteral("INDEX");
    int found = -1;
    for (int i = 0; i < scan.fields.size(); ++i)
        if (scan.fields[i].name == name && scan.fields[i].begin <= c.selectionEnd() && scan.fields[i].end >= c.selectionStart() && scan.fields[i].form == FieldPlace::Complex) {
            found = i;
            break;
        }
    if (found < 0) return false;
    const FieldPlace f = scan.fields[found];
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor t(story);
    t.beginEditBlock();
    t.setPosition(f.begin);
    t.setPosition(f.end + 1, QTextCursor::KeepAnchor);
    t.removeSelectedText();
    QTextBlock merged = story->findBlock(f.begin);
    if (merged.isValid() && merged.text().isEmpty() && (merged.next().isValid() || merged.previous().isValid())) {
        QTextCursor d(story);
        if (merged.next().isValid()) {
            d.setPosition(merged.position());
            d.setPosition(merged.next().position(), QTextCursor::KeepAnchor);
        } else {
            d.setPosition(merged.previous().position() + merged.previous().length() - 1);
            d.setPosition(merged.position() + merged.length() - 1, QTextCursor::KeepAnchor);
        }
        d.removeSelectedText();
    }
    t.endEditBlock();
    return true;
}

} // namespace jp::write

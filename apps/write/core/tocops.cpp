#include "core/tocops.h"

#include "core/editops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "text/textprops.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextOption>
#include <algorithm>

using namespace jp::office::wml;

namespace jp::write {

namespace {

bool parseLevels(const QString &text, int *lo, int *hi)
{
    static const QRegularExpression range(QStringLiteral("^\\s*(\\d+)\\s*(?:[-\\x2013]\\s*(\\d+))?\\s*$"));
    const QRegularExpressionMatch m = range.match(text);
    if (!m.hasMatch()) return false;
    *lo = std::clamp(m.captured(1).toInt(), 1, 9);
    *hi = m.captured(2).isEmpty() ? *lo : std::clamp(m.captured(2).toInt(), 1, 9);
    if (*hi < *lo) std::swap(*lo, *hi);
    return true;
}

// The text of a paragraph as an entry shows it: no marks, no deleted text, tabs and line breaks as blanks.
QString entryText(const QTextBlock &b, int skipFrom, int skipTo)
{
    QString t;
    for (auto it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment frag = it.fragment();
        if (frag.position() >= skipFrom && frag.position() + frag.length() <= skipTo) continue;   // (a table's own result, which a paragraph can share with a heading)
        const QTextCharFormat cf = frag.charFormat();
        if (cf.hasProperty(prop::TrackedDeleted) || cf.hasProperty(prop::Atom)) {
            // (A field that is one character shows its result.)
            if (cf.hasProperty(prop::Atom) && !cf.hasProperty(prop::TrackedDeleted)) {
                const QString field = cf.stringProperty(tp::Field);
                if (field.startsWith(QLatin1String("raw:"))) t += field.mid(4).repeated(it.fragment().length());
            }
            continue;
        }
        t += frag.text();
    }
    t.remove(QChar::ObjectReplacementCharacter);
    t.replace(QChar::LineSeparator, QLatin1Char(' ')).replace(QLatin1Char('\t'), QLatin1Char(' ')).replace(QChar(0x000C), QLatin1Char(' ')).replace(QChar(0x000E), QLatin1Char(' '));
    return t.trimmed();
}

QByteArray escaped(const QString &s) { return jp::office::wml::escapeXml(s, true); }

Inline markAtom(const char *type)
{
    Inline a;
    a.kind = Inline::RunAtom;
    a.tag = QStringLiteral("fldChar");
    a.key = QString::fromLatin1(type);
    a.raw = QByteArray("<w:fldChar w:fldCharType=\"") + type + "\"/>";
    return a;
}

Inline instructionAtom(const QString &code)
{
    Inline a;
    a.kind = Inline::RunAtom;
    a.tag = QStringLiteral("instrText");
    a.raw = "<w:instrText xml:space=\"preserve\">" + jp::office::wml::escapeXml(code) + "</w:instrText>";
    return a;
}

void putAtomAt(QTextCursor &c, const Inline &atom, const QTextCharFormat &base)
{
    QTextCharFormat f = base;
    setStored(f, prop::Atom, packAtom(atom));
    f.setProperty(tp::Field, QStringLiteral("raw:") + atom.display);
    c.insertText(QString(QChar::ObjectReplacementCharacter), f);
}

// The id of the paragraph style for line `level`: a style the document has (its name "toc 3", or "contents 3"), else the built-in one, added.
QString styleForLevel(WriteDoc &doc, int level)
{
    const QString toc = QStringLiteral("toc %1").arg(level), contents = QStringLiteral("contents %1").arg(level);
    for (const Style &st : doc.styleSheet().styles)
        if (st.type == QLatin1String("paragraph") && (st.name.compare(toc, Qt::CaseInsensitive) == 0 || st.name.compare(contents, Qt::CaseInsensitive) == 0)) return st.id;
    const QString id = QStringLiteral("TOC%1").arg(level);
    const quint64 sheet = doc.styleSheetRevision();
    if (doc.ensureStyle(id) && doc.styleSheetRevision() != sheet) doc.markStyleStep();
    return id;
}

QTextCharFormat formatAtPosition(QTextDocument *story, int position)
{
    QTextCursor c(story);
    c.setPosition(position + 1);
    return c.charFormat();
}

QString leaderOf(const QTextDocument *story, int from, int to)
{
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() <= to; b = b.next())
        for (const QTextOption::Tab &t : b.blockFormat().tabPositions())
            if (t.type == QTextOption::RightTab) {
                const ParaProps pp = paraPropsOf(b.blockFormat());
                if (pp.tabs)
                    for (const TabStop &ts : *pp.tabs)
                        if (ts.val == QLatin1String("right")) return ts.leader.isEmpty() ? QStringLiteral("none") : ts.leader;
                return QStringLiteral("dot");
            }
    return QStringLiteral("dot");
}

bool omitsPage(const fields::Code &code, int level)
{
    if (!code.has(QLatin1Char('n'))) return false;
    const QString range = code.argumentOf(QLatin1Char('n'));
    int lo = 1, hi = 9;
    if (range.isEmpty() || !parseLevels(range, &lo, &hi)) return true;
    return level >= lo && level <= hi;
}

} // namespace

QVector<TocLine> tocLines(WriteDoc &doc, const StoryScan &scan, int field, const fields::Code &code, const PageLookup &pageOf, bool *truncated)
{
    if (truncated) *truncated = false;
    const FieldPlace &self = scan.fields[field];
    QTextDocument *story = doc.story();
    int lo = 1, hi = 9;
    const bool haveO = code.has(QLatin1Char('o')) && parseLevels(code.argumentOf(QLatin1Char('o')), &lo, &hi);
    const bool useU = code.has(QLatin1Char('u'));
    QHash<QString, int> custom;   // lower case style name or id -> level
    if (code.has(QLatin1Char('t'))) {
        const QStringList parts = code.argumentOf(QLatin1Char('t')).split(QRegularExpression(QStringLiteral("[,;]")));
        for (int i = 0; i + 1 < parts.size(); i += 2) {
            bool ok = false;
            const int level = parts[i + 1].trimmed().toInt(&ok);
            if (ok && level >= 1 && level <= 9 && custom.size() < 64) custom.insert(parts[i].trimmed().toLower(), level);
        }
    }
    // A table of figures: \c lists the captions of a label whole, \a lists them without the label and number.
    const bool withoutLabel = !code.has(QLatin1Char('c')) && code.has(QLatin1Char('a'));
    const QString captionId = code.argumentOf(QLatin1Char(withoutLabel ? 'a' : 'c')).trimmed().toUpper();
    const bool captions = (code.has(QLatin1Char('c')) || withoutLabel) && !captionId.isEmpty();
    const bool useO = haveO || (!useU && custom.isEmpty() && !captions && !code.has(QLatin1Char('f')));
    if (useO && !haveO) lo = 1, hi = 9;
    // The styles: id -> the level a heading style has, and id -> name.
    QHash<QString, int> headingLevel;
    QHash<QString, QString> nameOf;
    static const QRegularExpression headingName(QStringLiteral("^heading\\s+(\\d)$"), QRegularExpression::CaseInsensitiveOption);
    for (const Style &st : doc.styleSheet().styles) {
        if (st.type != QLatin1String("paragraph")) continue;
        nameOf.insert(st.id, st.name.toLower());
        if (const QRegularExpressionMatch m = headingName.match(st.name); m.hasMatch()) headingLevel.insert(st.id, m.captured(1).toInt());
    }
    for (int i = 1; i <= 9; ++i)
        if (!headingLevel.contains(QStringLiteral("Heading%1").arg(i)) && !nameOf.contains(QStringLiteral("Heading%1").arg(i))) headingLevel.insert(QStringLiteral("Heading%1").arg(i), i);
    // The captions' paragraphs, the headings' bookmarks, and the bookmark the table is limited to.
    QSet<int> captionBlocks;
    QHash<int, int> captionTextFrom;   // the block number -> where the text after the label and number begins (for \a)
    if (captions)
        for (int i = 0; i < scan.fields.size(); ++i)
            if (scan.fields[i].name == QLatin1String("SEQ") && fields::parseCode(scan.instructionOf(i)).arguments.value(0).trimmed().toUpper() == captionId) {
                const int n = story->findBlock(scan.fields[i].begin).blockNumber();
                if (!captionBlocks.contains(n)) captionTextFrom.insert(n, scan.fields[i].end + 1);
                captionBlocks.insert(n);
            }
    QHash<int, QString> tocBookmark;
    for (const BookmarkSpan &s : scan.bookmarks)
        if (s.name.startsWith(QLatin1String("_Toc")) && s.start >= 0) {
            const int n = story->findBlock(s.start).blockNumber();
            if (!tocBookmark.contains(n)) tocBookmark.insert(n, s.name);
        }
    int limitFrom = 0, limitTo = story->characterCount();
    if (code.has(QLatin1Char('b'))) {
        const auto found = scan.bookmarkByName.constFind(code.argumentOf(QLatin1Char('b')).trimmed().toLower());
        if (found != scan.bookmarkByName.constEnd()) {
            limitFrom = scan.bookmarks[found.value()].start;
            if (scan.bookmarks[found.value()].end >= 0) limitTo = scan.bookmarks[found.value()].end;   // (a bookmark that was never closed runs to the end of the text)
        }
    }
    QVector<TocLine> out;
    int serial = 0;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.hasProperty(prop::EndnoteBlock) || bf.hasProperty(prop::RawBlock)) continue;
        if (b.position() < limitFrom || b.position() > limitTo) continue;
        if (b.position() > self.separate && b.position() + b.length() <= self.end) continue;   // (the table's own lines)
        const QString style = bf.stringProperty(tp::StyleName);
        const QString styleLower = style.toLower();
        int level = 0;
        if (captions) {
            if (captionBlocks.contains(b.blockNumber())) level = 1;
        } else {
            if (custom.contains(styleLower)) level = custom.value(styleLower);
            else if (custom.contains(nameOf.value(style))) level = custom.value(nameOf.value(style));
            else if (useO && headingLevel.contains(style) && headingLevel.value(style) >= lo && headingLevel.value(style) <= hi) level = headingLevel.value(style);
            else if (useU && bf.hasProperty(prop::Outline) && bf.intProperty(prop::Outline) < 9 && bf.intProperty(prop::Outline) + 1 >= (haveO ? lo : 1) && bf.intProperty(prop::Outline) + 1 <= (haveO ? hi : 9))
                level = bf.intProperty(prop::Outline) + 1;
        }
        if (level <= 0) continue;
        TocLine line;
        line.level = level;
        line.text = entryText(b, self.separate + 1, self.end);
        if (withoutLabel && captions && captionBlocks.contains(b.blockNumber())) {
            // (What follows the number, without the punctuation that joins it to the text.)
            const int from = captionTextFrom.value(b.blockNumber()) - b.position();
            QString after = b.text().mid(std::clamp(from, 0, int(b.text().size())));
            after.remove(QChar::ObjectReplacementCharacter);
            int skip = 0;
            while (skip < after.size() && (after[skip].isSpace() || QStringLiteral(".:;-\x2013\x2014").contains(after[skip]))) ++skip;
            line.text = after.mid(skip).simplified();
        }
        if (line.text.isEmpty()) continue;
        if (out.size() >= kMostTocEntries) {
            if (truncated) *truncated = true;
            break;
        }
        if (!captions) line.number = paragraphLabelOf(b);
        line.position = b.position();
        line.bookmark = tocBookmark.value(b.blockNumber());
        if (line.bookmark.isEmpty() && code.has(QLatin1Char('h'))) line.newBookmark = true;   // (named below, once the lines are in the order of the text)
        // (The page the paragraph's text is on: where its bookmark begins, else its first character that is no page break or mark; a heading that begins with a page break is on the next page.)
        int at = b.position();
        if (const auto found = line.newBookmark ? scan.bookmarkByName.constEnd() : scan.bookmarkByName.constFind(line.bookmark.toLower()); found != scan.bookmarkByName.constEnd()) {
            at = scan.bookmarks[found.value()].start;
        } else {
            const QString plain = b.text();
            int k = 0;
            while (k < plain.size() - 1 && (plain[k] == QChar(0x000C) || plain[k] == QChar::ObjectReplacementCharacter)) ++k;
            at = b.position() + k;
        }
        line.page = pageOf ? pageOf(at) : QString();
        out.push_back(line);
    }
    // The entries that table entry fields (TC) mark, for a code with \f: those of the code's identifier (those with none, when the code names none), at the level \l gives them.
    if (code.has(QLatin1Char('f'))) {
        const QString wanted = code.argumentOf(QLatin1Char('f')).trimmed();
        int tcLo = 1, tcHi = 9;
        if (code.has(QLatin1Char('l'))) parseLevels(code.argumentOf(QLatin1Char('l')), &tcLo, &tcHi);
        bool added = false;
        for (int i = 0; i < scan.fields.size(); ++i) {
            const FieldPlace &f = scan.fields[i];
            if (f.name != QLatin1String("TC")) continue;
            if (f.begin > self.begin && f.begin < self.end) continue;
            if (f.begin < limitFrom || f.begin > limitTo) continue;
            const fields::Code tc = fields::parseCode(scan.instructionOf(i));
            if (tc.name != QLatin1String("TC") || tc.arguments.isEmpty()) continue;
            if (tc.argumentOf(QLatin1Char('f')).trimmed().compare(wanted, Qt::CaseInsensitive) != 0) continue;
            bool ok = false;
            const int level = tc.has(QLatin1Char('l')) ? tc.argumentOf(QLatin1Char('l')).trimmed().toInt(&ok) : 1;
            if (tc.has(QLatin1Char('l')) && (!ok || level < 1 || level > 9)) continue;
            if (level < tcLo || level > tcHi) continue;
            TocLine line;
            line.level = level;
            line.text = tc.arguments.first().simplified();
            if (line.text.isEmpty()) continue;
            if (out.size() >= kMostTocEntries) {
                if (truncated) *truncated = true;
                break;
            }
            line.noPage = tc.has(QLatin1Char('n'));
            const QTextBlock b = story->findBlock(f.begin);
            line.position = b.position();
            line.bookmark = tocBookmark.value(b.blockNumber());
            if (line.bookmark.isEmpty() && code.has(QLatin1Char('h'))) line.newBookmark = true;
            line.page = pageOf ? pageOf(f.begin) : QString();
            out.push_back(line);
            added = true;
        }
        if (added) std::stable_sort(out.begin(), out.end(), [](const TocLine &a, const TocLine &b) { return a.position < b.position; });
    }
    // The bookmarks that are to be made have names above those the text has, in the order of the places (see putTocBookmarks); entries of one paragraph share one.
    QHash<int, QString> madeFor;
    for (TocLine &l : out) {
        if (!l.newBookmark) continue;
        if (madeFor.contains(l.position)) {
            l.bookmark = madeFor.value(l.position);
            continue;
        }
        do l.bookmark = QStringLiteral("_Toc%1").arg(100000001 + serial++);
        while (scan.bookmarkByName.contains(l.bookmark.toLower()));
        madeFor.insert(l.position, l.bookmark);
    }
    return out;
}

void putTocBookmarks(QTextDocument *story, const StoryScan &scan, const QVector<int> &positionsIn)
{
    QVector<int> positions = positionsIn;
    std::sort(positions.begin(), positions.end(), std::greater<int>());
    positions.erase(std::unique(positions.begin(), positions.end()), positions.end());
    // Names above those the text has; ids above every id of the file.
    long id = scan.maxBookmarkId + long(positions.size());
    int serial = 0;
    QSet<QString> used;
    QHash<int, QString> names;
    for (int i = int(positions.size()) - 1; i >= 0; --i) {
        QString name;
        do name = QStringLiteral("_Toc%1").arg(100000001 + serial++);
        while (scan.bookmarkByName.contains(name.toLower()) || used.contains(name));
        used.insert(name);
        names.insert(positions[i], name);
    }
    QTextCursor c(story);
    for (int position : std::as_const(positions)) {
        const QTextBlock b = story->findBlock(position);
        if (!b.isValid()) continue;
        Inline start, end;
        start.kind = end.kind = Inline::ParaAtom;
        start.tag = QStringLiteral("bookmarkStart");
        end.tag = QStringLiteral("bookmarkEnd");
        start.key = end.key = QString::number(id);
        start.raw = "<w:bookmarkStart w:id=\"" + QByteArray::number(qlonglong(id)) + "\" w:name=\"" + escaped(names.value(position)) + "\"/>";
        end.raw = "<w:bookmarkEnd w:id=\"" + QByteArray::number(qlonglong(id)) + "\"/>";
        --id;
        QTextCharFormat look = b.charFormat();
        for (const int key : {int(prop::Atom), int(tp::Field)}) look.clearProperty(key);
        // The paragraph's text, past a page break that begins it, to its end.
        const QString plain = b.text();
        int from = 0;
        while (from < plain.size() - 1 && (plain[from] == QChar(0x000C) || plain[from] == QChar::ObjectReplacementCharacter)) ++from;
        c.setPosition(b.position() + int(plain.size()));
        putAtomAt(c, end, look);
        c.setPosition(b.position() + from);
        putAtomAt(c, start, look);
    }
}

QStringList tocLineTexts(const QVector<TocLine> &lines, const fields::Code &code)
{
    const QString separator = code.has(QLatin1Char('p')) ? code.argumentOf(QLatin1Char('p')) : QStringLiteral("\t");
    QStringList out;
    if (lines.isEmpty()) return {QCoreApplication::translate("jp::write::Fields", "No headings were found for this table of contents.")};
    for (const TocLine &l : lines) {
        QString t = l.number.isEmpty() ? l.text : l.number + QLatin1Char('\t') + l.text;
        if (!(l.noPage || omitsPage(code, l.level)) && !l.page.isEmpty()) t += separator + l.page;
        out << t;
    }
    return out;
}

bool rebuildToc(WriteDoc &doc, QTextDocument *story, const StoryScan &scan, int field, const fields::Code &code, const QVector<TocLine> &lines, const QString &leaderAsked)
{
    const FieldPlace &f = scan.fields[field];
    if (f.form != FieldPlace::Complex || f.separate < 0 || f.end <= f.separate) return false;
    const int shift = 0;   // (the headings' bookmarks are made before the pass that builds the table: see putTocBookmarks)
    const int s = f.separate + shift, e = f.end + shift;
    QTextBlock firstBlock = story->findBlock(s), lastBlock = story->findBlock(e);
    const bool sameBlock = firstBlock == lastBlock;
    const QTextBlockFormat firstFormat = firstBlock.blockFormat(), lastFormat = lastBlock.blockFormat();
    const QString leader = leaderAsked.isEmpty() ? leaderOf(story, s, e) : leaderAsked;
    const QTextCharFormat look = [&] {
        QTextCharFormat cf = formatAtPosition(story, s);
        for (const int key : {int(prop::Atom), int(tp::Field), int(tp::LinkResult)}) cf.clearProperty(key);
        return cf;
    }();
    // The page's text width, for the right-aligned page numbers.
    const SectionProps place = doc.sectionProps(doc.sectionAt(s));
    const int textWidth = std::max(1440, place.pageW - std::max(0, place.left + place.gutter) - std::max(0, place.right));

    QTextCursor c(story);
    c.setPosition(s + 1);
    c.setPosition(e, QTextCursor::KeepAnchor);
    if (c.hasSelection()) c.removeSelectedText();
    c.setPosition(s + 1);

    static const int kept[] = {int(prop::MarkFormat), int(prop::LastSection), int(prop::RawBefore), int(prop::ParaAttrs), int(prop::RawPPr), int(prop::SpacingAttrs), int(prop::IndAttrs), int(prop::NumExtra), int(prop::OddTabs),
                               int(flow::fp::Cell), int(flow::fp::Frame), int(tp::OutOfLayout)};
    const bool hyperlinks = code.has(QLatin1Char('h'));
    const QString separator = code.has(QLatin1Char('p')) ? code.argumentOf(QLatin1Char('p')) : QStringLiteral("\t");
    const bool figures = code.has(QLatin1Char('c')) || (code.has(QLatin1Char('a')) && !code.argumentOf(QLatin1Char('a')).isEmpty());
    auto blockFor = [&](const TocLine &l, QTextBlockFormat *out) {
        QString style = styleForLevel(doc, l.level);
        if (figures) {
            // (The lines of a table of figures are in the style of its own.)
            style = QStringLiteral("TableofFigures");
            for (const Style &st : doc.styleSheet().styles)
                if (st.type == QLatin1String("paragraph") && st.name.compare(QLatin1String("table of figures"), Qt::CaseInsensitive) == 0) style = st.id;
            const quint64 sheet = doc.styleSheetRevision();
            if (doc.ensureStyle(style) && doc.styleSheetRevision() != sheet) doc.markStyleStep();
        }
        QTextBlockFormat nf = doc.styleBlockFormat(style);
        ParaProps pp = paraPropsOf(nf);
        QVector<TabStop> tabs;
        const int indent = pp.indLeft.value_or(0);
        if (!l.number.isEmpty()) tabs.push_back(TabStop{indent + 440, QStringLiteral("left"), QString()});
        if (separator == QLatin1String("\t") && !(l.noPage || omitsPage(code, l.level))) tabs.push_back(TabStop{textWidth, QStringLiteral("right"), leader == QLatin1String("none") ? QString() : leader});
        pp.tabs = tabs;
        nf = blockFormatOf(pp);
        *out = nf;
        return style;
    };
    if (lines.isEmpty()) {
        c.insertText(QCoreApplication::translate("jp::write::Fields", "No headings were found for this table of contents."), look);
        return true;
    }
    for (int i = 0; i < lines.size(); ++i) {
        const TocLine &l = lines[i];
        QTextBlockFormat nf;
        const QString style = blockFor(l, &nf);
        const QTextCharFormat entryLook = doc.styleCharFormat(style);
        if (i == 0) {
            // (The paragraph the table begins in: what is its own in the file stays, and what is the style's is the first line's.)
            const QTextBlockFormat now = c.blockFormat();
            for (int key : kept)
                if (now.hasProperty(key)) nf.setProperty(key, now.property(key));
            if (sameBlock && firstFormat.hasProperty(prop::RawAfter)) nf.clearProperty(prop::RawAfter);
            c.setBlockFormat(nf);
        } else {
            c.insertBlock(nf, entryLook);
        }
        if (hyperlinks) {
            Inline open;
            open.kind = Inline::ParaAtom;
            open.role = Inline::Open;
            open.tag = QStringLiteral("hyperlink");
            open.raw = "<w:hyperlink w:anchor=\"" + escaped(l.bookmark) + "\" w:history=\"1\">";
            putAtomAt(c, open, entryLook);
        }
        if (!l.number.isEmpty()) c.insertText(l.number + QLatin1Char('\t'), entryLook);
        c.insertText(l.text, entryLook);
        if (!(l.noPage || omitsPage(code, l.level)) && !l.page.isEmpty()) {
            c.insertText(separator, entryLook);
            if (hyperlinks) {
                putAtomAt(c, markAtom("begin"), entryLook);
                putAtomAt(c, instructionAtom(QStringLiteral(" PAGEREF %1 \\h ").arg(l.bookmark)), entryLook);
                putAtomAt(c, markAtom("separate"), entryLook);
                c.insertText(l.page, entryLook);
                putAtomAt(c, markAtom("end"), entryLook);
            } else {
                c.insertText(l.page, entryLook);
            }
        }
        if (hyperlinks) {
            Inline close;
            close.kind = Inline::ParaAtom;
            close.role = Inline::Close;
            close.tag = QStringLiteral("hyperlink");
            close.raw = "</w:hyperlink>";
            putAtomAt(c, close, entryLook);
        }
    }
    // The paragraph that holds the table's end: as it was where it was one of its own, else a paragraph in the document's own style.
    QTextBlockFormat endFormat;
    if (!sameBlock) {
        endFormat = lastFormat;
    } else {
        endFormat = doc.styleBlockFormat(doc.defaultParagraphStyle());
        for (const int key : {int(prop::RawAfter), int(flow::fp::SectionBreakAfter), int(prop::SectionData)})
            if (firstFormat.hasProperty(key)) endFormat.setProperty(key, firstFormat.property(key));
    }
    c.insertBlock(endFormat, doc.styleCharFormat(endFormat.stringProperty(tp::StyleName)));
    return true;
}

} // namespace jp::write

namespace jp::write {

QString GenLine::text() const
{
    QString t;
    for (const GenRun &r : runs) t += r.text;
    return t;
}

QString resultLeader(QTextDocument *story, const StoryScan &scan, int field)
{
    const FieldPlace &f = scan.fields[field];
    if (f.separate < 0 || f.end <= f.separate) return QStringLiteral("dot");
    return leaderOf(story, f.separate, f.end);
}

bool rebuildGenerated(WriteDoc &doc, QTextDocument *story, const StoryScan &scan, int field, const QVector<GenLine> &lines, const QString &leaderAsked, const QString &empty)
{
    const FieldPlace &f = scan.fields[field];
    if (f.form != FieldPlace::Complex || f.separate < 0 || f.end <= f.separate) return false;
    const int s = f.separate, e = f.end;
    QTextBlock firstBlock = story->findBlock(s), lastBlock = story->findBlock(e);
    const bool sameBlock = firstBlock == lastBlock;
    const QTextBlockFormat firstFormat = firstBlock.blockFormat(), lastFormat = lastBlock.blockFormat();
    const QString leader = leaderAsked.isEmpty() ? leaderOf(story, s, e) : leaderAsked;
    const QTextCharFormat look = [&] {
        QTextCharFormat cf = formatAtPosition(story, s);
        for (const int key : {int(prop::Atom), int(tp::Field), int(tp::LinkResult)}) cf.clearProperty(key);
        return cf;
    }();
    const SectionProps place = doc.sectionProps(doc.sectionAt(s));
    const int textWidth = std::max(1440, place.pageW - std::max(0, place.left + place.gutter) - std::max(0, place.right));

    QTextCursor c(story);
    c.setPosition(s + 1);
    c.setPosition(e, QTextCursor::KeepAnchor);
    if (c.hasSelection()) c.removeSelectedText();
    c.setPosition(s + 1);

    static const int kept[] = {int(prop::MarkFormat), int(prop::LastSection), int(prop::RawBefore), int(prop::ParaAttrs), int(prop::RawPPr), int(prop::SpacingAttrs), int(prop::IndAttrs), int(prop::NumExtra), int(prop::OddTabs),
                               int(flow::fp::Cell), int(flow::fp::Frame), int(tp::OutOfLayout)};
    if (lines.isEmpty()) {
        c.insertText(empty, look);
        return true;
    }
    auto styleOf = [&](const GenLine &l) {
        for (const Style &st : doc.styleSheet().styles)
            if (st.type == QLatin1String("paragraph") && st.name.compare(l.styleName, Qt::CaseInsensitive) == 0) return st.id;
        const quint64 sheet = doc.styleSheetRevision();
        if (doc.ensureStyle(l.styleId) && doc.styleSheetRevision() != sheet) doc.markStyleStep();
        return l.styleId;
    };
    for (int i = 0; i < lines.size(); ++i) {
        const GenLine &l = lines[i];
        const QString style = styleOf(l);
        QTextBlockFormat nf = doc.styleBlockFormat(style);
        if (l.rightTab) {
            ParaProps pp = paraPropsOf(nf);
            QVector<TabStop> tabs = pp.tabs ? *pp.tabs : QVector<TabStop>();
            tabs.push_back(TabStop{textWidth, QStringLiteral("right"), leader == QLatin1String("none") ? QString() : leader});
            pp.tabs = tabs;
            nf = blockFormatOf(pp);
        }
        const QTextCharFormat entryLook = doc.styleCharFormat(style);
        if (i == 0) {
            const QTextBlockFormat now = c.blockFormat();
            for (int key : kept)
                if (now.hasProperty(key)) nf.setProperty(key, now.property(key));
            if (sameBlock && firstFormat.hasProperty(prop::RawAfter)) nf.clearProperty(prop::RawAfter);
            c.setBlockFormat(nf);
        } else {
            c.insertBlock(nf, entryLook);
        }
        for (const GenRun &r : l.runs) {
            QTextCharFormat cf = entryLook;
            if (r.bold) cf.setFontWeight(QFont::Bold);
            if (r.italic) cf.setFontItalic(true);
            c.insertText(r.text, cf);
        }
    }
    QTextBlockFormat endFormat;
    if (!sameBlock) {
        endFormat = lastFormat;
    } else {
        endFormat = doc.styleBlockFormat(doc.defaultParagraphStyle());
        for (const int key : {int(prop::RawAfter), int(flow::fp::SectionBreakAfter), int(prop::SectionData)})
            if (firstFormat.hasProperty(key)) endFormat.setProperty(key, firstFormat.property(key));
    }
    c.insertBlock(endFormat, doc.styleCharFormat(endFormat.stringProperty(tp::StyleName)));
    return true;
}

QString tocCode(const TocOptions &o)
{
    QString code = QStringLiteral(" TOC \\o \"1-%1\"").arg(std::clamp(o.levels, 1, 9));
    if (o.hyperlinks) code += QStringLiteral(" \\h");
    code += QStringLiteral(" \\z \\u");
    if (!o.pageNumbers) code += QStringLiteral(" \\n");
    else if (!o.rightAlign) code += QStringLiteral(" \\p \" \"");
    return code + QLatin1Char(' ');
}

bool insertTableOfContents(WriteDoc &doc, QTextCursor &c, const TocOptions &options, bool *truncated)
{
    return insertTableFromCode(doc, c, tocCode(options), options.leader, truncated);
}

bool insertTableFromCode(WriteDoc &doc, QTextCursor &c, const QString &code, const QString &leader, bool *truncated)
{
    QTextDocument *story = doc.story();
    if (c.document() != story || !intoBody(c)) return false;
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    // (No edit block round it all: the pages must see the table's marks before its lines are made, and a block holds the news of the edits back. The group makes it one step.)
    QTextCursor at(story);
    at.setPosition(c.selectionEnd());
    const QTextBlock b = at.block();
    const QString plain = b.text();
    if (!plain.isEmpty()) {
        at.movePosition(QTextCursor::EndOfBlock);
        at.insertBlock(doc.styleBlockFormat(doc.defaultParagraphStyle()), doc.styleCharFormat(doc.defaultParagraphStyle()));
    }
    const int begin = at.position();
    const QTextCharFormat look = doc.styleCharFormat(at.block().blockFormat().stringProperty(tp::StyleName));
    auto put = [&](const Inline &a) { putAtomAt(at, a, look); };
    put(markAtom("begin"));
    put(instructionAtom(code));
    put(markAtom("separate"));
    put(markAtom("end"));
    UpdateOptions u;
    u.from = begin;
    u.to = begin + 3;
    u.tocLeader = leader;
    const UpdateReport report = updateFields(doc, u, true);
    if (truncated) *truncated = report.tocTruncated;
    c.setPosition(begin);
    return true;
}

bool markTocEntry(WriteDoc &doc, QTextCursor &c, const QString &textIn, int level, const QString &identifier, bool noPage)
{
    QString text = textIn.isEmpty() ? c.selectedText() : textIn;
    text.replace(QChar::ParagraphSeparator, QLatin1Char(' ')).replace(QChar::LineSeparator, QLatin1Char(' '));
    text = text.simplified();
    if (text.isEmpty()) return false;
    QString code = QStringLiteral("TC \"%1\"").arg(QString(text).replace(QLatin1Char('"'), QStringLiteral("\\\"")));
    if (!identifier.trimmed().isEmpty()) code += QStringLiteral(" \\f %1").arg(identifier.trimmed().left(1));
    code += QStringLiteral(" \\l %1").arg(std::clamp(level, 1, 9));
    if (noPage) code += QStringLiteral(" \\n");
    QTextCursor at = c;
    at.setPosition(c.selectionEnd());
    if (!insertField(doc, at, code)) return false;
    c = at;
    return true;
}

bool addTextToToc(WriteDoc &doc, QTextCursor &c, int level)
{
    if (level < 0 || level > 9 || c.document() != doc.story()) return false;
    const QString id = level == 0 ? doc.defaultParagraphStyle() : QStringLiteral("Heading%1").arg(level);
    if (level > 0) {
        const quint64 sheet = doc.styleSheetRevision();
        if (!doc.ensureStyle(id)) return false;
        if (doc.styleSheetRevision() != sheet) doc.markStyleStep();
    }
    applyParagraphStyle(doc, c, id);
    return true;
}

bool removeTableOfContents(WriteDoc &doc, const QTextCursor &c)
{
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story, false);
    int found = -1;
    for (int i = 0; i < scan.fields.size(); ++i)
        if (scan.fields[i].name == QLatin1String("TOC") && scan.fields[i].begin <= c.selectionEnd() && scan.fields[i].end >= c.selectionStart() && scan.fields[i].form == FieldPlace::Complex) {
            found = i;
            break;
        }
    if (found < 0) return false;
    const FieldPlace f = scan.fields[found];
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    const QTextBlock first = story->findBlock(f.begin), last = story->findBlock(f.end);
    const QTextBlockFormat firstFormat = first.blockFormat(), lastFormat = last.blockFormat();
    QTextCursor t(story);
    t.setPosition(f.begin);
    t.setPosition(f.end + 1, QTextCursor::KeepAnchor);
    t.removeSelectedText();
    QTextBlock merged = story->findBlock(f.begin);
    // The content control a gallery's table is in goes with it; another's stays, closed where it was.
    if (first != last && lastFormat.hasProperty(prop::RawAfter)) {
        const QVector<Block> after = unpackBlocks(storedBytes(lastFormat, prop::RawAfter));
        const QVector<Block> before = firstFormat.hasProperty(prop::RawBefore) ? unpackBlocks(storedBytes(firstFormat, prop::RawBefore)) : QVector<Block>();
        bool gallery = !before.isEmpty() && before.last().tag == QLatin1String("sdt") && before.last().raw.contains("docPartGallery");
        QTextBlockFormat nf = merged.blockFormat();
        if (gallery && after.size() == 1) {
            QVector<Block> rest = before;
            rest.removeLast();
            if (rest.isEmpty()) nf.clearProperty(prop::RawBefore);
            else setStored(nf, prop::RawBefore, packBlocks(rest));
        } else {
            setStored(nf, prop::RawAfter, storedBytes(lastFormat, prop::RawAfter));
        }
        QTextCursor m(merged);
        m.setBlockFormat(nf);
    }
    // A paragraph that nothing is left in goes.
    merged = story->findBlock(f.begin);
    if (merged.text().isEmpty() && !merged.blockFormat().hasProperty(prop::RawBefore) && !merged.blockFormat().hasProperty(prop::RawAfter) && !merged.blockFormat().boolProperty(flow::fp::SectionBreakAfter) &&
        (merged.next().isValid() || merged.previous().isValid())) {
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
    edit.endEditBlock();
    return true;
}

int updateTableOfContents(WriteDoc &doc, const QTextCursor &c, bool pageNumbersOnly)
{
    const StoryScan scan = scanStory(doc.story(), false);
    int count = 0;
    int from = -1, to = -1;
    for (int i = 0; i < scan.fields.size(); ++i)
        if (scan.fields[i].name == QLatin1String("TOC") && scan.fields[i].begin <= c.selectionEnd() && scan.fields[i].end >= c.selectionStart()) {
            ++count;
            if (from < 0) from = scan.fields[i].begin;
            to = scan.fields[i].end;
        }
    if (count == 0) return 0;
    UpdateOptions o;
    o.from = from;
    o.to = to;
    o.pageNumbersOnly = pageNumbersOnly;
    updateFields(doc, o);
    return count;
}

} // namespace jp::write

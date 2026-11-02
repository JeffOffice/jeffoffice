#include "core/fieldops.h"

#include "core/editops.h"
#include "core/tocops.h"
#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "flow/paginator.h"
#include "text/textengine.h"
#include "text/textprops.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextList>
#include <algorithm>

using namespace jp::office::wml;

namespace jp::write {

namespace {

QByteArray escapedXml(const QString &text) { return jp::office::wml::escapeXml(text); }

// The format of the character at a position of the story.
QTextCharFormat formatAt(QTextDocument *story, int position)
{
    QTextCursor c(story);
    c.setPosition(position + 1);
    return c.charFormat();
}

// A format with what only a mark has taken off, for the text of a result.
QTextCharFormat plainOf(QTextCharFormat f)
{
    const QString field = f.stringProperty(tp::Field);
    f.clearProperty(prop::Atom);
    f.clearProperty(tp::Field);
    f.clearProperty(tp::LinkResult);
    f.clearProperty(tp::BlankFieldResult);
    if (field.startsWith(QLatin1String("pageref:"))) f.setProperty(tp::Field, field);
    return f;
}

// The look the new result of a field takes: the first plain character of the result it has, else the look of its separator (or of its first mark).
QTextCharFormat resultLook(QTextDocument *story, const FieldPlace &f)
{
    for (int p = f.separate + 1; p < f.end && p < f.separate + 400; ++p) {
        const QTextCharFormat cf = formatAt(story, p);
        if (!cf.hasProperty(prop::Atom)) return plainOf(cf);
    }
    return plainOf(formatAt(story, f.separate));
}

// The same atom with another body (`raw`, `display`): put in the format of the character at the position.
void putAtom(QTextDocument *story, int position, const Inline &atom, bool shown)
{
    QTextCharFormat f = formatAt(story, position);
    setStored(f, prop::Atom, packAtom(atom));
    if (shown) f.setProperty(tp::Field, QStringLiteral("raw:") + atom.display);
    QTextCursor c(story);
    c.setPosition(position);
    c.setPosition(position + 1, QTextCursor::KeepAnchor);
    c.setCharFormat(f);
}

// Sets or clears a flag attribute (w:fldLock, w:dirty) on the first element of an atom's XML that has the field's mark in it.
QByteArray withFlag(QByteArray raw, const char *name, bool on)
{
    static const QRegularExpression start(QStringLiteral("<((?:\\w+:)?)(fldChar|fldSimple)\\b"));
    const QString text = QString::fromUtf8(raw);
    const QRegularExpressionMatch m = start.match(text);
    if (!m.hasMatch()) return raw;
    QString out = text;
    const QRegularExpression existing(QStringLiteral("\\s(?:\\w+:)?%1=\"[^\"]*\"").arg(QLatin1String(name)));
    // (Only inside the first element's start tag.)
    const int tagEnd = int(out.indexOf(QLatin1Char('>'), m.capturedStart()));
    QString tag = out.mid(m.capturedStart(), tagEnd - m.capturedStart());
    tag.remove(existing);
    if (on) tag.insert(m.capturedLength(), QStringLiteral(" %1%2=\"%3\"").arg(m.captured(1), QLatin1String(name), QLatin1String(name) == QLatin1String("fldLock") ? QStringLiteral("1") : QStringLiteral("true")));
    out.replace(m.capturedStart(), tagEnd - m.capturedStart(), tag);
    return out.toUtf8();
}

// A group's runs with the result runs made again: the separator's properties on a run of the text. Only for a group that holds one field.
std::optional<QByteArray> groupWithResult(const QByteArray &raw, const QString &text)
{
    const qsizetype sep = raw.indexOf("fldCharType=\"separate\"");
    const qsizetype end = raw.lastIndexOf("fldCharType=\"end\"");
    if (sep < 0 || end < 0 || end < sep || raw.count("fldCharType=\"begin\"") != 1 || raw.count("fldCharType=\"end\"") != 1) return std::nullopt;
    const qsizetype afterSeparate = raw.indexOf("</w:r>", sep);
    const qsizetype endRun = raw.lastIndexOf("<w:r", end);
    const qsizetype sepRun = raw.lastIndexOf("<w:r", sep);
    if (afterSeparate < 0 || endRun < 0 || sepRun < 0 || afterSeparate + 6 > endRun) return std::nullopt;
    QByteArray rpr;
    const qsizetype pr = raw.indexOf("<w:rPr>", sepRun);
    if (pr >= 0 && pr < afterSeparate) rpr = raw.mid(pr, raw.indexOf("</w:rPr>", pr) + 8 - pr);
    QByteArray out = raw.left(afterSeparate + 6);
    if (!text.isEmpty()) out += "<w:r>" + rpr + "<w:t xml:space=\"preserve\">" + escapedXml(text) + "</w:t></w:r>";
    out += raw.mid(endRun);
    return out;
}

struct SectionIndex {
    QVector<int> ends;   // where each section's last paragraph ends (all but the last section)
    explicit SectionIndex(QTextDocument *story)
    {
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            if (b.blockFormat().boolProperty(flow::fp::SectionBreakAfter)) ends.push_back(b.position() + b.length());
    }
    int of(int position) const { return int(std::upper_bound(ends.begin(), ends.end(), position) - ends.begin()); }
};

struct PageAt {
    int index = 0;          // from 1
    int number = 0;
    QString text;
    int section = 0;        // the page's, from 0
};

class Pages {
public:
    explicit Pages(WriteDoc &doc) : m_doc(doc) {}
    void ensure()
    {
        if (m_ready) return;
        m_ready = true;
        flow::Paginator &pager = m_doc.pager();
        pager.paginate(-1);
        m_count = pager.pagesKnown();
        for (int i = 0; i < m_count; ++i) ++m_sectionPages[pager.page(i).start.section];
    }
    int count() { ensure(); return m_count; }
    int sectionPages(int section) { ensure(); return m_sectionPages.value(section); }
    PageAt at(int position)
    {
        ensure();
        PageAt out;
        if (m_count <= 0) return out;
        flow::Paginator &pager = m_doc.pager();
        int k = pager.pageOfTablePosition(position);
        if (k < 0) k = pager.pageOfPosition(position);
        k = std::clamp(k, 0, m_count - 1);
        const flow::Paginator::Page page = pager.page(k);
        out.index = k + 1;
        out.number = page.number;
        out.section = page.start.section;
        out.text = jp::pageNumberText(page.number, m_doc.flow().section(page.start.section).pageNumberFormat);
        return out;
    }

private:
    WriteDoc &m_doc;
    bool m_ready = false;
    int m_count = 0;
    QHash<int, int> m_sectionPages;
};

QString blockLabel(const QTextBlock &b)
{
    const QTextBlockFormat bf = b.blockFormat();
    QString label = bf.stringProperty(tp::BulletChar);
    if (label.isEmpty() && b.textList()) {
        const QTextListFormat lf = b.textList()->format();
        switch (lf.style()) {
        case QTextListFormat::ListDecimal:
        case QTextListFormat::ListLowerAlpha:
        case QTextListFormat::ListUpperAlpha:
        case QTextListFormat::ListLowerRoman:
        case QTextListFormat::ListUpperRoman: label = b.textList()->itemText(b); break;
        default: break;
        }
    }
    label = label.trimmed();
    return label;
}

QString withoutTrailingPeriods(QString s)
{
    while (s.endsWith(QLatin1Char('.')) || s.endsWith(QLatin1Char(' '))) s.chop(1);
    return s;
}

} // namespace

QString paragraphNumberOf(const QTextBlock &b) { return withoutTrailingPeriods(blockLabel(b)); }
QString paragraphLabelOf(const QTextBlock &b) { return blockLabel(b); }

namespace {

// Everything a pass of updating needs about the document, found when it is first asked.
class Context {
public:
    Context(WriteDoc &doc, QTextDocument *story, const StoryScan &scan, const QDateTime &now) : doc(doc), story(story), scan(scan), sections(story), pages(doc)
    {
        env.now = now.isValid() ? now : jp::fieldNow();
        env.properties = documentProperties(doc);
        env.sequences = &sequences;
        env.bookmark = [this](const QString &name) { return bookmarkInfo(name); };
        env.noteNumber = [this](const QString &name, const fields::Code &) { return noteNumber(name); };
        env.styleRef = [this](const QString &style, const fields::Code &code) { return styleReference(style, code); };
        env.number = [this](const QString &name) -> std::optional<double> {
            const std::optional<fields::BookmarkInfo> b = bookmarkInfo(name);
            if (!b) return std::nullopt;
            bool ok = false;
            const double v = b->text.trimmed().toDouble(&ok);
            return ok ? std::optional<double>(v) : std::nullopt;
        };
    }

    struct Heading {
        int position;
        int level;   // 0 to 8
    };
    const QVector<Heading> &headings()
    {
        if (!m_headingsMade) {
            m_headingsMade = true;
            for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
                const QTextBlockFormat bf = b.blockFormat();
                if (bf.hasProperty(prop::EndnoteBlock) || bf.hasProperty(prop::RawBlock)) continue;
                if (bf.hasProperty(prop::Outline) && bf.intProperty(prop::Outline) < 9) m_headings.push_back({b.position(), bf.intProperty(prop::Outline)});
            }
        }
        return m_headings;
    }

    // Where the field is, for the engine.
    void place(const FieldPlace &f)
    {
        env.position = f.begin;
        env.page = env.pages = env.sectionPages = 0;
        env.pageText.clear();
        env.section = sections.of(f.begin) + 1;
    }
    void placePages(const FieldPlace &f)
    {
        const PageAt p = pages.at(f.begin);
        env.page = p.number;
        env.pageText = p.text;
        env.pages = pages.count();
        env.sectionPages = pages.sectionPages(p.section);
    }
    void resetSequencesAt(int position)
    {
        const QVector<Heading> &hs = headings();
        while (m_nextHeading < hs.size() && hs[m_nextHeading].position < position) {
            for (auto it = m_sequenceResets.begin(); it != m_sequenceResets.end(); ++it)
                if (hs[m_nextHeading].level + 1 <= it.value()) sequences[it.key()] = 0;
            ++m_nextHeading;
        }
    }
    void noteSequenceReset(const fields::Code &code)
    {
        if (!code.has(QLatin1Char('s'))) return;
        bool ok = false;
        const int level = code.argumentOf(QLatin1Char('s')).toInt(&ok);
        if (ok && level >= 1 && level <= 9) m_sequenceResets.insert(code.arguments.value(0).trimmed().toUpper(), level);
    }
    void statistics()
    {
        if (env.properties.statistics) return;
        env.properties.statistics = true;
        QString text = scan.plain;
        text.remove(QChar::ObjectReplacementCharacter);
        int words = 0, chars = 0, withSpaces = 0, paragraphs = 0;
        bool in = false;
        for (const QChar c : text) {
            if (c == QLatin1Char('\n')) {
                ++paragraphs;
                in = false;
                continue;
            }
            ++withSpaces;
            if (c.isSpace()) {
                in = false;
            } else {
                ++chars;
                if (!in) ++words;
                in = true;
            }
        }
        env.properties.words = words;
        env.properties.characters = chars;
        env.properties.charactersWithSpaces = withSpaces;
        env.properties.paragraphs = paragraphs + 1;
        env.properties.pages = pages.count();
    }

    std::optional<fields::BookmarkInfo> bookmarkInfo(const QString &name)
    {
        const auto found = scan.bookmarkByName.constFind(name.trimmed().toLower());
        if (found == scan.bookmarkByName.constEnd()) return std::nullopt;
        const BookmarkSpan &s = scan.bookmarks[found.value()];
        fields::BookmarkInfo b;
        const int end = s.end >= s.start ? s.end : s.start;
        QString text = scan.plain.mid(s.start + 1, std::max(0, end - s.start - 1));
        text.remove(QChar::ObjectReplacementCharacter);
        text.replace(QLatin1Char('\n'), QLatin1Char(' '));
        b.text = text;
        b.position = s.start;
        const PageAt p = pages.at(s.start);
        b.page = p.text;
        b.pageIndex = p.index;
        const QString number = paragraphNumberOf(story->findBlock(s.start));
        b.number = b.fullNumber = b.relativeNumber = number;
        return b;
    }

    std::optional<QString> noteNumber(const QString &name)
    {
        const auto found = scan.bookmarkByName.constFind(name.trimmed().toLower());
        if (found == scan.bookmarkByName.constEnd()) return fields::referenceSourceNotFound();
        const BookmarkSpan &s = scan.bookmarks[found.value()];
        loadNotes();
        const int end = s.end >= s.start ? s.end : s.start;
        for (int i = 0; i < m_footnotes.size(); ++i)
            if (m_footnotes[i] > s.start && m_footnotes[i] < end) return QString::number(i + 1);
        for (int i = 0; i < m_endnotes.size(); ++i)
            if (m_endnotes[i] > s.start && m_endnotes[i] < end) return QString::number(i + 1);
        return fields::referenceSourceNotFound();
    }

    std::optional<QString> styleReference(const QString &style, const fields::Code &code)
    {
        QString id;
        for (const Style &st : doc.styleSheet().styles)
            if (st.type == QLatin1String("paragraph") && (st.id.compare(style, Qt::CaseInsensitive) == 0 || st.name.compare(style, Qt::CaseInsensitive) == 0)) {
                id = st.id;
                break;
            }
        // (A number, "1", is "Heading 1".)
        bool number = false;
        const int level = style.toInt(&number);
        if (id.isEmpty() && number && level >= 1 && level <= 9) id = QStringLiteral("Heading%1").arg(level);
        if (id.isEmpty()) return fields::referenceSourceNotFound();
        QTextBlock at = story->findBlock(env.position);
        auto matches = [&](const QTextBlock &b) { return b.blockFormat().stringProperty(tp::StyleName) == id && !b.blockFormat().hasProperty(prop::EndnoteBlock); };
        QTextBlock found;
        int steps = 0;
        for (QTextBlock b = at; b.isValid() && !found.isValid() && ++steps < 2000000; b = b.previous())
            if (matches(b)) found = b;
        for (QTextBlock b = at.next(); b.isValid() && !found.isValid() && ++steps < 4000000; b = b.next())
            if (matches(b)) found = b;
        if (!found.isValid()) return fields::referenceSourceNotFound();
        if (code.has(QLatin1Char('n')) || code.has(QLatin1Char('r')) || code.has(QLatin1Char('w'))) return paragraphNumberOf(found);
        if (code.has(QLatin1Char('p'))) return found.position() < env.position ? QCoreApplication::translate("jp::write::Fields", "above") : QCoreApplication::translate("jp::write::Fields", "below");
        QString t = found.text();
        t.remove(QChar::ObjectReplacementCharacter);
        t.replace(QChar::LineSeparator, QLatin1Char(' ')).replace(QLatin1Char('\t'), QLatin1Char(' '));
        return t.trimmed();
    }

    WriteDoc &doc;
    QTextDocument *story;
    const StoryScan &scan;
    SectionIndex sections;
    Pages pages;
    fields::Env env;
    QHash<QString, qint64> sequences;

private:
    void loadNotes()
    {
        if (m_notesLoaded) return;
        m_notesLoaded = true;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QTextCharFormat cf = it.fragment().charFormat();
                const QString field = cf.stringProperty(tp::Field);
                if (field.startsWith(QLatin1String("footnote:"))) for (int k = 0; k < it.fragment().length(); ++k) m_footnotes.push_back(it.fragment().position() + k);
                else if (cf.hasProperty(prop::Atom)) {
                    const QByteArray bytes = storedBytes(cf, prop::Atom);
                    if (bytes.contains("endnoteReference") && unpackAtom(bytes).tag == QLatin1String("endnoteReference"))
                        for (int k = 0; k < it.fragment().length(); ++k) m_endnotes.push_back(it.fragment().position() + k);
                }
            }
    }
    bool m_headingsMade = false, m_notesLoaded = false;
    QVector<Heading> m_headings;
    int m_nextHeading = 0;
    QHash<QString, int> m_sequenceResets;
    QVector<int> m_footnotes, m_endnotes;
};

struct Planned {
    int field;
    QString before, after;
    FieldChange::Outcome outcome;
    QVector<TocLine> toc;
    fields::Code code;
    bool isToc = false;
};

} // namespace

UpdateReport updateFields(WriteDoc &doc, const UpdateOptions &options, bool keepChanges)
{
    UpdateReport report;
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story);
    const int to = options.to < 0 ? int(story->characterCount()) : options.to;
    QVector<int> chosen;
    for (int i = 0; i < scan.fields.size(); ++i) {
        const FieldPlace &f = scan.fields[i];
        if (f.end >= options.from && f.begin <= to) chosen.push_back(i);
    }
    report.found = int(chosen.size());
    if (chosen.isEmpty()) return report;
    // Inner before outer, siblings in order.
    std::sort(chosen.begin(), chosen.end(), [&](int a, int b) { return scan.fields[a].end != scan.fields[b].end ? scan.fields[a].end < scan.fields[b].end : scan.fields[a].begin > scan.fields[b].begin; });

    Context ctx(doc, story, scan, options.now);
    QHash<int, QString> computed;
    QVector<Planned> plans;
    for (int index : std::as_const(chosen)) {
        const FieldPlace &f = scan.fields[index];
        FieldChange change;
        change.name = f.name;
        change.position = f.begin;
        change.before = scan.resultOf(index);
        change.instruction = scan.instructionOf(index, computed).trimmed();
        auto note = [&](FieldChange::Outcome outcome, const QString &after) {
            change.outcome = outcome;
            change.after = after;
            if (keepChanges) report.changes.push_back(change);
            if (outcome == FieldChange::Locked) ++report.locked;
            else if (outcome == FieldChange::Kept) ++report.kept;
            else if (outcome == FieldChange::Changed) ++report.changed;
            else ++report.unchanged;
        };
        if (f.locked) {
            note(FieldChange::Locked, change.before);
            continue;
        }
        if (f.live || (options.skip && options.skip(f.name))) {
            note(FieldChange::Kept, change.before);
            continue;
        }
        const fields::Code code = fields::parseCode(change.instruction);
        if (code.name == QLatin1String("TOC")) {
            if (options.pageNumbersOnly) {
                note(FieldChange::Kept, change.before);   // (its PAGEREF fields are updated on their own)
                continue;
            }
            ctx.pages.ensure();
            const PageLookup pageOf = [&](int position) { return ctx.pages.at(position).text; };
            QVector<TocLine> lines = tocLines(doc, scan, index, code, pageOf);
            const QString after = tocLineTexts(lines, code).join(QLatin1Char('\n'));
            QString before = change.before;
            before.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
            // (What the result has is the paragraphs' text, one to a line, in the same form.)
            QStringList stored;
            for (const QString &p : before.split(QLatin1Char('\n'))) stored << p;
            stored.removeAll(QString());   // (a file's table may hold empty paragraphs between its lines, and one with the field's end)
            const bool same = stored.join(QLatin1Char('\n')) == after;
            change.before = stored.join(QLatin1Char('\n'));
            if (same) {
                note(FieldChange::Unchanged, after);
            } else {
                note(FieldChange::Changed, after);
                Planned p;
                p.field = index;
                p.before = change.before;
                p.after = after;
                p.outcome = FieldChange::Changed;
                p.toc = lines;
                p.code = code;
                p.isToc = true;
                plans.push_back(p);
            }
            continue;
        }
        if (!fields::isKnown(code.name)) {
            note(FieldChange::Kept, change.before);
            continue;
        }
        ctx.place(f);
        if (code.name == QLatin1String("SEQ")) {
            ctx.noteSequenceReset(code);
            ctx.resetSequencesAt(f.begin);
        }
        if (code.name == QLatin1String("PAGE") || code.name == QLatin1String("NUMPAGES") || code.name == QLatin1String("SECTIONPAGES")) ctx.placePages(f);
        if (code.name == QLatin1String("NUMPAGES")) ctx.env.pages = ctx.pages.count();
        if (code.name == QLatin1String("NUMWORDS") || code.name == QLatin1String("NUMCHARS") || code.name == QLatin1String("DOCPROPERTY")) ctx.statistics();
        const std::optional<QString> result = fields::compute(code, ctx.env);
        if (!result) {
            note(FieldChange::Kept, change.before);
            continue;
        }
        computed.insert(index, *result);
        if (*result == change.before) {
            note(FieldChange::Unchanged, *result);
            continue;
        }
        note(FieldChange::Changed, *result);
        Planned p;
        p.field = index;
        p.before = change.before;
        p.after = *result;
        p.outcome = FieldChange::Changed;
        plans.push_back(p);
    }
    if (!options.apply || plans.isEmpty()) return report;

    // Applied from the last to the first, so that the places of the earlier stay; a field inside a result that is made again goes with it.
    std::sort(plans.begin(), plans.end(), [&](const Planned &a, const Planned &b) { return scan.fields[a.field].end > scan.fields[b.field].end; });
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    QVector<std::pair<int, int>> replaced;   // (the results made again: from, to)
    for (const Planned &p : std::as_const(plans)) {
        const FieldPlace &f = scan.fields[p.field];
        bool inside = false;
        for (const auto &r : std::as_const(replaced)) inside = inside || (f.begin >= r.first && f.end <= r.second);
        if (inside) continue;
        if (p.isToc) {
            if (rebuildToc(doc, story, scan, p.field, p.code, p.toc, options.tocLeader)) replaced.push_back({f.separate, f.end});
            continue;
        }
        if (f.form == FieldPlace::Group) {
            Inline atom = unpackAtom(storedBytes(formatAt(story, f.begin), prop::Atom));
            const std::optional<QByteArray> raw = groupWithResult(atom.raw, p.after);
            if (!raw) continue;
            atom.raw = *raw;
            atom.display = p.after;
            if (f.dirty) atom.raw = withFlag(atom.raw, "dirty", false);
            putAtom(story, f.begin, atom, true);
            continue;
        }
        if (f.separate < 0) continue;
        const QTextCharFormat look = resultLook(story, f);
        QTextCursor c(story);
        c.setPosition(f.separate + 1);
        c.setPosition(f.end, QTextCursor::KeepAnchor);
        if (c.hasSelection()) c.removeSelectedText();
        if (!p.after.isEmpty()) c.insertText(p.after, look);
        replaced.push_back({f.separate, f.end});
        if (f.dirty && f.form == FieldPlace::Complex) {
            Inline atom = unpackAtom(storedBytes(formatAt(story, f.begin), prop::Atom));
            atom.raw = withFlag(atom.raw, "dirty", false);
            putAtom(story, f.begin, atom, false);
        }
    }
    edit.endEditBlock();
    return report;
}

UpdateReport updateFieldsAt(WriteDoc &doc, const QTextCursor &c)
{
    UpdateOptions o;
    if (c.hasSelection()) {
        o.from = c.selectionStart();
        o.to = c.selectionEnd();
        return updateFields(doc, o);
    }
    const StoryScan scan = scanStory(doc.story(), false);
    const int at = fieldContaining(scan, c.position());
    if (at < 0) return UpdateReport();
    int outer = at;
    while (scan.fields[outer].parent >= 0) outer = scan.fields[outer].parent;
    o.from = scan.fields[outer].begin;
    o.to = scan.fields[outer].end;
    return updateFields(doc, o);
}

int unlinkFields(WriteDoc &doc, const QTextCursor &c)
{
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story, false);
    int from = c.selectionStart(), to = c.selectionEnd();
    QVector<int> chosen;
    if (from == to) {
        const int at = fieldContaining(scan, from);
        if (at >= 0) {
            int outer = at;
            while (scan.fields[outer].parent >= 0) outer = scan.fields[outer].parent;
            chosen << outer;
        }
    } else {
        for (int i = 0; i < scan.fields.size(); ++i)
            if (scan.fields[i].end >= from && scan.fields[i].begin <= to && scan.fields[i].parent < 0) chosen << i;
    }
    if (chosen.isEmpty()) return 0;
    std::sort(chosen.begin(), chosen.end(), [&](int a, int b) { return scan.fields[a].begin > scan.fields[b].begin; });
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    int done = 0;
    for (int index : std::as_const(chosen)) {
        const FieldPlace &f = scan.fields[index];
        if (f.locked) continue;
        if (f.form == FieldPlace::Group) {
            QTextCharFormat look = plainOf(formatAt(story, f.begin));
            QTextCursor t(story);
            t.setPosition(f.begin);
            t.setPosition(f.begin + 1, QTextCursor::KeepAnchor);
            t.removeSelectedText();
            if (!f.display.isEmpty()) t.insertText(f.display, look);
            ++done;
            continue;
        }
        if (f.separate < 0) continue;
        // The marks come out from the last to the first: the end, the separator and the code before it (every mark that belongs to the field itself, those of fields nested in the code
        // among them), the begin; the result stays.
        QTextCursor t(story);
        t.setPosition(f.end);
        t.setPosition(f.end + 1, QTextCursor::KeepAnchor);
        t.removeSelectedText();
        t.setPosition(f.begin);
        t.setPosition(f.separate + 1, QTextCursor::KeepAnchor);
        t.removeSelectedText();
        ++done;
    }
    edit.endEditBlock();
    return done;
}

int lockFields(WriteDoc &doc, const QTextCursor &c, bool lock)
{
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story, false);
    const int from = c.selectionStart(), to = c.selectionEnd();
    QVector<int> chosen;
    if (from == to) {
        const int at = fieldContaining(scan, from);
        if (at >= 0) {
            int outer = at;
            while (scan.fields[outer].parent >= 0) outer = scan.fields[outer].parent;
            chosen << outer;
        }
    } else {
        for (int i = 0; i < scan.fields.size(); ++i)
            if (scan.fields[i].end >= from && scan.fields[i].begin <= to) chosen << i;
    }
    int done = 0;
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    for (int index : std::as_const(chosen)) {
        const FieldPlace &f = scan.fields[index];
        if (f.locked == lock) continue;
        Inline atom = unpackAtom(storedBytes(formatAt(story, f.begin), prop::Atom));
        atom.raw = withFlag(atom.raw, "fldLock", lock);
        putAtom(story, f.begin, atom, f.form == FieldPlace::Group);
        ++done;
    }
    edit.endEditBlock();
    return done;
}

bool insertField(WriteDoc &doc, QTextCursor &c, const QString &codeIn, const QString &fallbackResult)
{
    const QString code = codeIn.trimmed();
    if (code.isEmpty()) return false;
    QString result = fallbackResult;
    {
        // The result the engine works out for it, where the document can say.
        const StoryScan scan = scanStory(doc.story());
        Context ctx(doc, doc.story(), scan, QDateTime());
        FieldPlace at;
        at.begin = c.selectionStart();
        ctx.place(at);
        const fields::Code parsed = fields::parseCode(code);
        if (parsed.name == QLatin1String("PAGE") || parsed.name == QLatin1String("NUMPAGES") || parsed.name == QLatin1String("SECTIONPAGES")) ctx.placePages(at);
        if (parsed.name == QLatin1String("NUMWORDS") || parsed.name == QLatin1String("NUMCHARS")) ctx.statistics();
        if (parsed.name == QLatin1String("SEQ")) {
            // (Counted from the fields before the caret.)
            for (int i = 0; i < scan.fields.size(); ++i) {
                const FieldPlace &f = scan.fields[i];
                if (f.begin >= at.begin) break;
                const fields::Code before = fields::parseCode(scan.instructionOf(i));
                if (before.name == QLatin1String("SEQ")) {
                    ctx.noteSequenceReset(before);
                    ctx.resetSequencesAt(f.begin);
                    fields::compute(before, ctx.env);
                }
            }
            ctx.noteSequenceReset(parsed);
            ctx.resetSequencesAt(at.begin);
        }
        if (const std::optional<QString> r = fields::compute(parsed, ctx.env)) result = *r;
    }
    Inline field;
    field.kind = Inline::ParaAtom;
    field.role = Inline::Single;
    field.tag = kFieldGroupTag;
    field.raw = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> " + escapedXml(code) + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>";
    if (!result.isEmpty()) field.raw += "<w:r><w:t xml:space=\"preserve\">" + escapedXml(result) + "</w:t></w:r>";
    field.raw += "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    field.key = liveFieldFor(QLatin1Char(' ') + code + QLatin1Char(' '));
    field.display = result;
    return insertFieldAtom(doc, c, field);
}

} // namespace jp::write

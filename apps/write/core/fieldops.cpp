#include "core/fieldops.h"

#include "core/editops.h"
#include "core/tableops.h"
#include "core/tocops.h"
#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "flow/paginator.h"
#include "text/textengine.h"
#include "text/textprops.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QLocale>
#include <QMap>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <algorithm>
#include <optional>

using namespace jp::office::wml;

namespace jp::write {

namespace {
qint64 g_styleVisits = 0, g_overlapChecks = 0, g_computations = 0;
}
qint64 fieldComputationsForTest() { return g_computations; }
qint64 styleRefParagraphVisitsForTest() { return g_styleVisits; }
qint64 resultOverlapChecksForTest() { return g_overlapChecks; }
void resetFieldWorkForTest() { g_styleVisits = g_overlapChecks = g_computations = 0; }

namespace {

constexpr int kMostPasses = 3;                 // an update that changes the text is worked out again (the pages moved) this many times at most

QByteArray escapedXml(const QString &text) { return jp::office::wml::escapeXml(text); }

QString unescaped(QString text)
{
    if (!text.contains(QLatin1Char('&'))) return text;
    text.replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&quot;"), QLatin1String("\"")).replace(QLatin1String("&apos;"), QLatin1String("'"));
    text.replace(QLatin1String("&amp;"), QLatin1String("&"));
    return text;
}

// The format of the character at a position of the story.
QTextCharFormat formatAt(QTextDocument *story, int position)
{
    QTextCursor c(story);
    c.setPosition(position + 1);
    return c.charFormat();
}

// Text that a tracked change deleted (its look carries the deleting change).
bool inDeletion(const QTextCharFormat &f)
{
    const int kind = f.intProperty(tp::Revision);
    return kind == 2 || kind == 3;
}

// A look without the deleting change, for text that is to be shown (an insertion's look stays: what is put in the place of a result inside an insertion is inserted by the same author).
QTextCharFormat withoutDeletion(QTextCharFormat f)
{
    if (!inDeletion(f)) return f;
    f.clearProperty(tp::Revision);
    f.clearProperty(tp::RevisionAuthor);
    f.clearProperty(tp::RevisionInserted);
    if (f.intProperty(prop::TrackedDeleted) == prop::HiddenByDeletion) f.clearProperty(tp::HiddenText);
    f.clearProperty(prop::TrackedDeleted);
    return f;
}

// A format with what only a mark has taken off, for the text of a result.
QTextCharFormat plainOf(QTextCharFormat f)
{
    const QString field = f.stringProperty(tp::Field);
    f.clearProperty(prop::Atom);
    f.clearProperty(tp::Field);
    f.clearProperty(tp::LinkResult);
    f.clearProperty(tp::BlankFieldResult);
    f.clearProperty(tp::FieldCodeText);
    if (field.startsWith(QLatin1String("pageref:"))) f.setProperty(tp::Field, field);
    return withoutDeletion(f);
}

// The look of the characters a result has (those that are no marks), at most a few thousand, in order.
QVector<QTextCharFormat> resultLooks(QTextDocument *story, const FieldPlace &f)
{
    QVector<QTextCharFormat> out;
    for (int p = f.separate + 1; p < f.end && out.size() < 4000; ++p) {
        const QTextCharFormat cf = formatAt(story, p);
        if (!cf.hasProperty(prop::Atom)) out.push_back(plainOf(cf));
    }
    return out;
}

// The look of the first character of a field's code (the run its first piece is in).
std::optional<QTextCharFormat> codeLook(QTextDocument *story, const FieldPlace &f)
{
    for (int p = f.begin + 1; p < f.separate && p < f.begin + 64; ++p) {
        const QTextCharFormat cf = formatAt(story, p);
        if (cf.hasProperty(prop::Atom) && storedBytes(cf, prop::Atom).contains("instrText")) return plainOf(cf);
    }
    return std::nullopt;
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
    const int tagEnd = int(text.indexOf(QLatin1Char('>'), m.capturedStart()));
    if (tagEnd < 0) return raw;   // (a tag that is cut off is left as it is)
    QString out = text;
    const QRegularExpression existing(QStringLiteral("\\s(?:\\w+:)?%1=\"[^\"]*\"").arg(QLatin1String(name)));
    QString tag = out.mid(m.capturedStart(), tagEnd - m.capturedStart());
    tag.remove(existing);
    if (on) tag.insert(m.capturedLength(), QStringLiteral(" %1%2=\"%3\"").arg(m.captured(1), QLatin1String(name), QLatin1String(name) == QLatin1String("fldLock") ? QStringLiteral("1") : QStringLiteral("true")));
    out.replace(m.capturedStart(), tagEnd - m.capturedStart(), tag);
    return out.toUtf8();
}

} // namespace
QByteArray fieldFlagForTest(const QByteArray &raw, const char *name, bool on);
namespace {

enum class LookMode { First, Merge, Code };

// A group's runs with the result runs made again. Only for a group that holds one field. The look of the new runs: of the first old result run (First), of each place of the old result
// (Merge), or of the run the code is in (Code); of the separator's run where there is no result run to take it from.
std::optional<QByteArray> groupWithResult(const QByteArray &raw, const QString &text, LookMode mode)
{
    const qsizetype sep = raw.indexOf("fldCharType=\"separate\"");
    const qsizetype end = raw.lastIndexOf("fldCharType=\"end\"");
    if (sep < 0 || end < 0 || end < sep || raw.count("fldCharType=\"begin\"") != 1 || raw.count("fldCharType=\"end\"") != 1) return std::nullopt;
    const qsizetype afterSeparate = raw.indexOf("</w:r>", sep);
    const qsizetype endRun = raw.lastIndexOf("<w:r", end);
    const qsizetype sepRun = raw.lastIndexOf("<w:r", sep);
    if (afterSeparate < 0 || endRun < 0 || sepRun < 0 || afterSeparate + 6 > endRun) return std::nullopt;
    auto rprOf = [&](qsizetype from, qsizetype to) -> QByteArray {
        const qsizetype pr = raw.indexOf("<w:rPr>", from);
        if (pr < 0 || pr >= to) return QByteArray();
        return raw.mid(pr, raw.indexOf("</w:rPr>", pr) + 8 - pr);
    };
    const QByteArray sepRpr = rprOf(sepRun, afterSeparate);
    // The old result's runs: their properties and how many characters each has.
    struct OldRun {
        QByteArray rpr;
        int length;
    };
    QVector<OldRun> old;
    {
        const QString body = QString::fromUtf8(raw.mid(afterSeparate + 6, endRun - afterSeparate - 6));
        static const QRegularExpression runs(QStringLiteral("<w:r>(<w:rPr>.*?</w:rPr>)?.*?<w:t[^>]*>(.*?)</w:t></w:r>"), QRegularExpression::DotMatchesEverythingOption);
        for (auto m = runs.globalMatch(body); m.hasNext() && old.size() < 4000;) {
            const QRegularExpressionMatch r = m.next();
            old.push_back({r.captured(1).toUtf8(), int(unescaped(r.captured(2)).size())});
        }
    }
    QByteArray codeRpr = sepRpr;
    if (mode == LookMode::Code) {
        const qsizetype instr = raw.indexOf("<w:instrText");
        const qsizetype run = instr < 0 ? -1 : raw.lastIndexOf("<w:r", instr);
        if (run >= 0) codeRpr = rprOf(run, instr);
    }
    QByteArray out = raw.left(afterSeparate + 6);
    if (!text.isEmpty()) {
        auto runFor = [&](int index) -> QByteArray {
            if (mode == LookMode::Code) return codeRpr;
            if (old.isEmpty()) return sepRpr;
            if (mode == LookMode::First) return old.first().rpr;
            int at = index;
            for (const OldRun &r : std::as_const(old)) {
                if (at < r.length) return r.rpr;
                at -= r.length;
            }
            return old.last().rpr;
        };
        int start = 0;
        while (start < text.size()) {
            const QByteArray rpr = runFor(start);
            int stop = start + 1;
            while (stop < text.size() && runFor(stop) == rpr) ++stop;
            out += "<w:r>" + rpr + "<w:t xml:space=\"preserve\">" + escapedXml(text.mid(start, stop - start)) + "</w:t></w:r>";
            start = stop;
        }
    }
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

// The pages, laid out when the first one is asked.
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
    bool laidOut() const { return m_ready; }
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

QByteArray fieldFlagForTest(const QByteArray &raw, const char *name, bool on) { return withFlag(raw, name, on); }

QString paragraphNumberOf(const QTextBlock &b) { return withoutTrailingPeriods(blockLabel(b)); }
QString paragraphLabelOf(const QTextBlock &b) { return blockLabel(b); }

namespace {

bool pageDependent(const QString &name)
{
    return name == QLatin1String("PAGE") || name == QLatin1String("NUMPAGES") || name == QLatin1String("SECTIONPAGES") || name == QLatin1String("PAGEREF") || name == QLatin1String("TOC") ||
           name == QLatin1String("STYLEREF");
}

// Everything a pass of updating needs about the document, found when it is first asked and kept for the pass.
class Context {
public:
    Context(WriteDoc &doc, QTextDocument *story, const StoryScan &scan, const QDateTime &now) : doc(doc), story(story), scan(scan), sections(story), pages(doc)
    {
        env.now = now.isValid() ? now : jp::fieldNow();
        env.properties = documentProperties(doc);
        env.sequences = &sequences;
        env.formulaBudget = &m_formulaBudget;
        env.bookmark = [this](const QString &name) { return bookmarkInfo(name); };
        env.bookmarkPage = [this](const QString &name, fields::BookmarkInfo *info) {
            const auto found = this->scan.bookmarkByName.constFind(name.trimmed().toLower());
            if (found == this->scan.bookmarkByName.constEnd()) return;
            const PageAt p = pages.at(this->scan.bookmarks[found.value()].start);
            info->page = p.text;
            info->pageIndex = p.index;
        };
        env.cells = [this](const QString &name) { return tableCellValues(this->doc, env.position, name); };
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

    // The language of the names a date shows: the language of the text where the field begins, else the document's, else the program's.
    QLocale localeAt(int position) const
    {
        QTextCursor c(story);
        c.setPosition(std::clamp(position + 1, 0, std::max(0, story->characterCount() - 1)));
        QString tag = c.charFormat().stringProperty(tp::Language);
        if (tag.isEmpty()) tag = doc.proofingLanguage();
        return tag.isEmpty() ? QLocale() : QLocale(tag);
    }

    // Where the field is, for the engine.
    void place(const FieldPlace &f)
    {
        env.position = f.begin;
        env.locale = localeAt(f.begin);
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

    // The results worked out so far in the pass (a bookmark's text takes them, not the results the story still has).
    void setComputed(const QHash<int, QString> *computed) { m_computed = computed; }

    std::optional<fields::BookmarkInfo> bookmarkInfo(const QString &name)
    {
        const QString key = name.trimmed().toLower();
        const auto found = scan.bookmarkByName.constFind(key);
        if (found == scan.bookmarkByName.constEnd()) return std::nullopt;
        const BookmarkSpan &s = scan.bookmarks[found.value()];
        auto cached = m_bookmarkCache.find(key);
        const int stamp = cached == m_bookmarkCache.end() ? -1 : computedInside(cached->inner);
        if (cached == m_bookmarkCache.end() || cached->stamp != stamp) {
            Cached c;
            c.info.position = s.start;
            c.info.text = textWithResults(s, &c.inner);
            c.stamp = computedInside(c.inner);
            const QString number = paragraphNumberOf(story->findBlock(s.start));
            c.info.number = c.info.fullNumber = c.info.relativeNumber = number;
            cached = m_bookmarkCache.insert(key, c);
        }
        return cached->info;
    }

    std::optional<QString> noteNumber(const QString &name)
    {
        const auto found = scan.bookmarkByName.constFind(name.trimmed().toLower());
        if (found == scan.bookmarkByName.constEnd()) return fields::referenceSourceNotFound();
        const BookmarkSpan &s = scan.bookmarks[found.value()];
        loadNotes();
        const int end = s.end >= s.start ? s.end : s.start;
        for (const QVector<int> *refs : {&m_footnotes, &m_endnotes}) {
            const auto it = std::upper_bound(refs->begin(), refs->end(), s.start);
            if (it != refs->end() && *it < end) return QString::number(int(it - refs->begin()) + 1);
        }
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
        if (id.isEmpty()) return fields::styleTextNotFound();
        indexStyles();
        const QVector<int> &where = m_styleBlocks.value(id);
        if (where.isEmpty()) return fields::styleTextNotFound();
        // The nearest paragraph of the style at or before the field, else the first after it.
        auto it = std::upper_bound(where.begin(), where.end(), env.position);
        const int position = it != where.begin() ? *(it - 1) : *it;
        const QTextBlock found = story->findBlock(position);
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
    struct Cached {
        fields::BookmarkInfo info;
        QVector<int> inner;   // the fields in the bookmark's text
        int stamp = 0;        // how many of them had a result made when the text was
    };

    int computedInside(const QVector<int> &inner) const
    {
        int n = 0;
        if (m_computed)
            for (int i : inner) n += m_computed->contains(i);
        return n;
    }

    // The bookmark's text with the results of the fields in it as they are now (made in this pass, else the story's): marks are no text.
    QString textWithResults(const BookmarkSpan &s, QVector<int> *inner) const
    {
        const int end = s.end >= s.start ? s.end : s.start;
        QString text;
        int at = s.start + 1;
        // (The fields come in the order they begin: those inside the text, the outermost of each stretch, are the ones whose results stand in place.)
        const auto first = std::lower_bound(scan.fields.begin(), scan.fields.end(), at, [](const FieldPlace &f, int position) { return f.begin < position; });
        for (auto it = first; it != scan.fields.end() && it->begin < end; ++it) {
            const int index = int(it - scan.fields.begin());
            if (it->end >= end) continue;
            inner->push_back(index);
            if (it->begin < at) continue;   // (inside one that was put in place)
            text += scan.plain.mid(at, it->begin - at);
            text += m_computed && m_computed->contains(index) ? m_computed->value(index) : scan.resultOf(index);
            at = it->end + 1;
        }
        text += scan.plain.mid(at, std::max(0, end - at));
        text.remove(QChar::ObjectReplacementCharacter);
        text.replace(QLatin1Char('\n'), QLatin1Char(' '));
        return text;
    }

    // The positions of the paragraphs of each style, in one pass over the story (the first time a style is asked).
    void indexStyles()
    {
        if (m_stylesIndexed) return;
        m_stylesIndexed = true;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            ++g_styleVisits;
            const QTextBlockFormat bf = b.blockFormat();
            if (bf.hasProperty(prop::EndnoteBlock)) continue;
            const QString id = bf.stringProperty(tp::StyleName);
            if (!id.isEmpty()) m_styleBlocks[id].push_back(b.position());
        }
    }

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
    bool m_headingsMade = false, m_notesLoaded = false, m_stylesIndexed = false;
    QVector<Heading> m_headings;
    int m_nextHeading = 0;
    QHash<QString, int> m_sequenceResets;
    QVector<int> m_footnotes, m_endnotes;
    QHash<QString, Cached> m_bookmarkCache;
    QHash<QString, QVector<int>> m_styleBlocks;
    const QHash<int, QString> *m_computed = nullptr;
    qint64 m_formulaBudget = fields::kMostFormulaSteps;
};

struct Planned {
    int field;
    QString before, after;
    QVector<TocLine> toc;
    fields::Code code;
    bool isToc = false;
};

// What one pass found and did.
struct PassResult {
    UpdateReport report;
    bool pageDependent = false;   // a field whose result depends on the pages was looked at
    bool changedText = false;
};

PassResult updatePass(WriteDoc &doc, const UpdateOptions &options, bool keepChanges, int depth, const std::function<void()> &openEdits)
{
    PassResult out;
    UpdateReport &report = out.report;
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story);
    const int to = options.to < 0 ? int(story->characterCount()) : options.to;
    QVector<int> chosen;
    QVector<bool> isChosen(scan.fields.size(), false);
    for (int i = 0; i < scan.fields.size(); ++i) {
        const FieldPlace &f = scan.fields[i];
        if (f.end >= options.from && f.begin <= to) {
            chosen.push_back(i);
            isChosen[i] = true;
        }
    }
    report.found = int(chosen.size());
    if (chosen.isEmpty()) return out;
    // Inner before outer, siblings in order.
    auto byEnd = [&](int a, int b) { return scan.fields[a].end != scan.fields[b].end ? scan.fields[a].end < scan.fields[b].end : scan.fields[a].begin > scan.fields[b].begin; };
    std::sort(chosen.begin(), chosen.end(), byEnd);

    Context ctx(doc, story, scan, options.now);
    QHash<int, QString> computed;
    ctx.setComputed(&computed);
    // A part of the text counts the sequences that come before it: the earlier fields of each sequence are worked through (and left as they are).
    {
        int firstBegin = scan.fields[chosen.first()].begin;
        for (int i : std::as_const(chosen)) firstBegin = std::min(firstBegin, scan.fields[i].begin);
        QVector<int> before;
        for (int i = 0; i < scan.fields.size(); ++i)
            if (!isChosen[i] && scan.fields[i].end < firstBegin && scan.fields[i].name == QLatin1String("SEQ")) before.push_back(i);
        std::sort(before.begin(), before.end(), byEnd);
        for (int i : std::as_const(before)) {
            const FieldPlace &f = scan.fields[i];
            const fields::Code code = fields::parseCode(scan.instructionOf(i));
            ctx.noteSequenceReset(code);
            ctx.resetSequencesAt(f.begin);
            ctx.place(f);
            fields::compute(code, ctx.env);
        }
    }
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
        // Text a tracked change deleted is left alone; a field inside an insertion is made again as part of it. A field that is one character and holds other fields is not worked out.
        if (inDeletion(formatAt(story, f.begin)) || (f.form == FieldPlace::Group && f.groupRaw.count("fldCharType=\"begin\"") != 1)) {
            note(FieldChange::Kept, change.before);
            continue;
        }
        const fields::Code code = fields::parseCode(change.instruction);
        if (pageDependent(code.name)) out.pageDependent = true;
        if (code.name == QLatin1String("TOC")) {
            if (options.pageNumbersOnly) {
                note(FieldChange::Kept, change.before);   // (its PAGEREF fields are updated on their own)
                continue;
            }
            ctx.pages.ensure();
            const PageLookup pageOf = [&](int position) { return ctx.pages.at(position).text; };
            bool cut = false;
            QVector<TocLine> lines = tocLines(doc, scan, index, code, pageOf, &cut);
            if (cut) report.tocTruncated = true;
            const QString after = tocLineTexts(lines, code).join(QLatin1Char('\n'));
            QString before = change.before;
            before.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
            // (What the result has is the paragraphs' text, one to a line, in the same form.)
            QStringList stored = before.split(QLatin1Char('\n'));
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
        if (code.name == QLatin1String("NUMWORDS") || code.name == QLatin1String("NUMCHARS") || code.name == QLatin1String("DOCPROPERTY")) ctx.statistics();
        ++g_computations;
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
        p.code = code;
        plans.push_back(p);
    }
    if (!options.apply || plans.isEmpty()) return out;

    // The headings a table of contents with links needs bookmarks in get them first, on their own: what is put in the text moves every place after it, so the pass is made again from the
    // text as it then is (the second time none is lacking).
    if (depth == 0) {
        QVector<int> lacking;
        for (const Planned &p : std::as_const(plans))
            if (p.isToc)
                for (const TocLine &l : p.toc)
                    if (l.newBookmark) lacking.push_back(l.position);
        if (!lacking.isEmpty()) {
            openEdits();
            putTocBookmarks(story, scan, lacking);
            UpdateOptions moved = options;
            for (int p : std::as_const(lacking)) {   // (each bookmark's two marks are in the text now: what comes after a heading moves by two)
                if (p < options.from) moved.from += 2;
                if (options.to >= 0 && p < options.to) moved.to += 2;
            }
            PassResult again = updatePass(doc, moved, keepChanges, depth + 1, openEdits);
            again.changedText = true;
            return again;
        }
    }

    // Applied from the last to the first, so that the places of the earlier stay; a field inside a result that is made again goes with it.
    std::sort(plans.begin(), plans.end(), [&](const Planned &a, const Planned &b) { return scan.fields[a.field].end > scan.fields[b.field].end; });
    openEdits();
    QTextCursor edit(story);
    edit.beginEditBlock();
    QMap<int, int> replaced;   // (the results made again, by where they begin: they are apart, the outer ones having been made first)
    for (const Planned &p : std::as_const(plans)) {
        const FieldPlace &f = scan.fields[p.field];
        bool inside = false;
        ++g_overlapChecks;
        if (!replaced.isEmpty()) {
            auto it = replaced.upperBound(f.begin);
            if (it != replaced.begin()) {
                --it;
                inside = f.begin >= it.key() && f.end <= it.value();
            }
        }
        if (inside) continue;
        const LookMode mode = p.code.mergesFormat() ? LookMode::Merge : p.code.usesCodeFormat() ? LookMode::Code : LookMode::First;
        if (p.isToc) {
            if (rebuildToc(doc, story, scan, p.field, p.code, p.toc, options.tocLeader)) replaced.insert(f.separate, f.end);
            out.changedText = true;
            continue;
        }
        if (f.form == FieldPlace::Group) {
            Inline atom = unpackAtom(storedBytes(formatAt(story, f.begin), prop::Atom));
            const std::optional<QByteArray> raw = groupWithResult(atom.raw, p.after, mode);
            if (!raw) continue;
            atom.raw = *raw;
            atom.display = p.after;
            if (f.dirty) atom.raw = withFlag(atom.raw, "dirty", false);
            putAtom(story, f.begin, atom, true);
            out.changedText = true;
            continue;
        }
        if (f.separate < 0) continue;
        // The look of the new text: of the first character of the old result, of each place of it (MERGEFORMAT), or of the first character of the code (CHARFORMAT).
        const QVector<QTextCharFormat> old = resultLooks(story, f);
        const QTextCharFormat sepLook = plainOf(formatAt(story, f.separate));
        QTextCharFormat first = old.isEmpty() ? sepLook : old.first();
        if (mode == LookMode::Code)
            if (const std::optional<QTextCharFormat> code = codeLook(story, f)) first = *code;
        auto looked = [](const QTextCharFormat &cf) { return cf; };
        QTextCursor c(story);
        c.setPosition(f.separate + 1);
        c.setPosition(f.end, QTextCursor::KeepAnchor);
        if (c.hasSelection()) c.removeSelectedText();
        int start = 0;
        while (start < p.after.size()) {
            const QTextCharFormat cf = mode == LookMode::Merge && !old.isEmpty() ? old[std::min<int>(start, int(old.size()) - 1)] : first;
            int stop = start + 1;
            while (mode == LookMode::Merge && !old.isEmpty() && stop < p.after.size() && old[std::min<int>(stop, int(old.size()) - 1)] == cf) ++stop;
            if (mode != LookMode::Merge || old.isEmpty()) stop = int(p.after.size());
            c.insertText(p.after.mid(start, stop - start), looked(cf));
            start = stop;
        }
        replaced.insert(f.separate, f.end);
        out.changedText = true;
        if (f.dirty) {
            Inline atom = unpackAtom(storedBytes(formatAt(story, f.begin), prop::Atom));
            atom.raw = withFlag(atom.raw, "dirty", false);
            putAtom(story, f.begin, atom, false);
        }
    }
    edit.endEditBlock();
    return out;
}

} // namespace

UpdateReport updateFields(WriteDoc &doc, const UpdateOptions &options, bool keepChanges)
{
    // The step and what an edit does round itself are made when the first change is: an update that changes nothing makes no step.
    std::optional<WriteDoc::StepGroup> group;
    std::optional<EditScope> scope;
    const std::function<void()> openEdits = [&] {
        if (group) return;
        group.emplace(doc);
        scope.emplace(doc, doc.story());
    };
    QTextDocument *story = doc.story();
    UpdateOptions o = options;
    const int before = int(story->characterCount());
    PassResult first = updatePass(doc, o, keepChanges, 0, openEdits);
    // The pages moved if the text changed: what the pages give is worked out again, until it stands (a few passes at most).
    PassResult last = first;
    int size = before;
    for (int pass = 1; pass < kMostPasses && options.apply && (first.report.changed > 0 || first.changedText) && last.changedText && first.pageDependent; ++pass) {
        const int now = int(story->characterCount());
        if (o.to >= 0) o.to += now - size;
        size = now;
        last = updatePass(doc, o, false, 1, openEdits);
        first.pageDependent = first.pageDependent || last.pageDependent;
        if (last.report.changed == 0) break;
    }
    return first.report;
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

namespace {

// The fields a selection touches (the outermost one the caret is in, with nothing selected).
QVector<int> fieldsTouched(const StoryScan &scan, const QTextCursor &c, bool nestedToo)
{
    QVector<int> chosen;
    const int from = c.selectionStart(), to = c.selectionEnd();
    if (from == to) {
        const int at = fieldContaining(scan, from);
        if (at >= 0) {
            int outer = at;
            while (scan.fields[outer].parent >= 0) outer = scan.fields[outer].parent;
            chosen << outer;
        }
    } else {
        for (int i = 0; i < scan.fields.size(); ++i)
            if (scan.fields[i].end >= from && scan.fields[i].begin <= to && (nestedToo || scan.fields[i].parent < 0)) chosen << i;
    }
    return chosen;
}

} // namespace

int unlinkFields(WriteDoc &doc, const QTextCursor &c)
{
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story, false);
    QVector<int> chosen;
    for (int i : fieldsTouched(scan, c, false))
        if (!scan.fields[i].locked && (scan.fields[i].form == FieldPlace::Group || scan.fields[i].separate >= 0)) chosen << i;
    if (chosen.isEmpty()) return 0;
    std::sort(chosen.begin(), chosen.end(), [&](int a, int b) { return scan.fields[a].begin > scan.fields[b].begin; });
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    int done = 0;
    for (int index : std::as_const(chosen)) {
        const FieldPlace &f = scan.fields[index];
        if (f.form == FieldPlace::Group) {
            QTextCharFormat look = plainOf(formatAt(story, f.begin));
            look.clearProperty(tp::FieldId);
            QTextCursor t(story);
            t.setPosition(f.begin);
            t.setPosition(f.begin + 1, QTextCursor::KeepAnchor);
            t.removeSelectedText();
            if (!f.display.isEmpty()) t.insertText(f.display, look);
            ++done;
            continue;
        }
        // The result is text of the document now: it takes nothing of the field with it.
        for (int p = f.separate + 1; p < f.end && p < f.separate + 200000; ++p) {
            QTextCharFormat cf = formatAt(story, p);
            if (!cf.hasProperty(tp::FieldId)) continue;
            cf.clearProperty(tp::FieldId);
            QTextCursor t(story);
            t.setPosition(p);
            t.setPosition(p + 1, QTextCursor::KeepAnchor);
            t.setCharFormat(cf);
        }
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
    QVector<int> chosen;
    for (int i : fieldsTouched(scan, c, true))
        if (scan.fields[i].locked != lock) chosen << i;
    if (chosen.isEmpty()) return 0;   // (nothing to change: no step)
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    for (int index : std::as_const(chosen)) {
        const FieldPlace &f = scan.fields[index];
        Inline atom = unpackAtom(storedBytes(formatAt(story, f.begin), prop::Atom));
        atom.raw = withFlag(atom.raw, "fldLock", lock);
        putAtom(story, f.begin, atom, f.form == FieldPlace::Group);
    }
    edit.endEditBlock();
    return int(chosen.size());
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

// ---- Show Field Codes

namespace {

// The number a field of the text goes by for Show Field Codes (0: it has none, a field made before the codes could be shown).
int fieldNumberOf(QTextDocument *story, const FieldPlace &f)
{
    return formatAt(story, f.form == FieldPlace::Complex ? f.separate : f.begin).intProperty(tp::FieldId);
}

} // namespace

int toggleFieldCodes(WriteDoc &doc, const QTextCursor &c, bool all)
{
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story, false);
    // The fields that have a number (the outermost ones: a field inside another is shown with it).
    QVector<int> top;
    for (int i = 0; i < scan.fields.size(); ++i)
        if (scan.fields[i].parent < 0 && fieldNumberOf(story, scan.fields[i]) > 0) top << i;
    QVector<int> chosen;
    if (all) {
        chosen = top;
    } else {
        QTextCursor one = c;
        for (int i : fieldsTouched(scan, one, false))
            if (top.contains(i)) {
                chosen << i;
                break;
            }
    }
    if (chosen.isEmpty()) {
        if (all && top.isEmpty() && scan.fields.isEmpty()) doc.setMayHaveFields(false);   // (there is none after all: the commands for the codes are dimmed)
        return 0;
    }
    auto shownNow = [&](int index) { return doc.allFieldCodesShown() || doc.shownFieldCodes().contains(fieldNumberOf(story, scan.fields[index])); };
    QSet<int> ids = doc.shownFieldCodes();
    bool allNow = doc.allFieldCodesShown();
    int changed = 0;
    if (all) {
        bool everyShown = true;
        for (int i : std::as_const(chosen)) everyShown = everyShown && shownNow(i);
        for (int i : std::as_const(chosen)) changed += shownNow(i) == everyShown;
        allNow = !everyShown;
        ids.clear();
    } else {
        const int id = fieldNumberOf(story, scan.fields[chosen.first()]);
        if (allNow) {
            // (One of all that are shown: all the others stay shown, one by one.)
            allNow = false;
            for (int i : std::as_const(top)) ids.insert(fieldNumberOf(story, scan.fields[i]));
        }
        if (ids.contains(id)) ids.remove(id);
        else ids.insert(id);
        changed = 1;
    }
    doc.setFieldCodes(allNow, ids);
    return changed;
}

} // namespace jp::write

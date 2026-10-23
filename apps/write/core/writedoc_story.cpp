// Filling the story from the body of a file: paragraphs and runs become blocks and formatted text, lists become Qt lists, and what is kept
// as XML becomes the pieces of writeprops.h.

#include "core/writedoc.h"

#include "core/drawnitems.h"
#include "core/tablemodel.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QFontDatabase>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <atomic>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {

// The Qt lists of one numbering id, by level: the items go on in the one that is open, and a deeper level's list is closed by an item of a shallower level.
struct NumState {
    QTextList *list[9];
    NumState()
    {
        for (int i = 0; i < 9; ++i) list[i] = nullptr;
    }
};

} // namespace

void WriteDoc::buildStory(const QVector<Block> &body)
{
    m_letteringShapes = m_letteringLetters = m_boxTableCells = 0;
    m_endnoteRefs.clear();
    fillStory(story(), body, false);
    appendEndnotes();
}

// The most fields that nest in one another that a header or footer shows live (deeper ones show what they last read).
constexpr int kMostFieldNesting = 8;
// The most paragraphs of a header or footer that are read (the rest is left out).
constexpr int kMostHeaderParagraphs = 400;
// The most objects set at places that a file's pages have, and the most pictures (a file with more has the rest left out).
constexpr int kMostObjects = 20000;
constexpr int kMostImages = 2000;
// The most cells of the tables of text boxes that a document has laid out in its boxes (a box whose tables would go over it keeps them as stand-ins).
constexpr int kMostBoxTableCells = 2000;
// The most endnotes that are set after the text (the rest are left out: a reference to one shows only its number), as many as the footnotes that are made into stories.

namespace {
bool drawnTag(const QString &tag)
{
    return tag == QLatin1String("pict") || tag == QLatin1String("drawing") || tag == QLatin1String("AlternateContent") || tag == QLatin1String("object");
}
} // namespace

namespace {

// What an instruction of a field asks to be shown as the flowing document's own: the code of the text engine's field for the page's number
// (in the format the instruction gives, or the section's), the document's page count, or the section's; none for the rest, which show what they read.
QString liveFieldOf(const QString &instruction)
{
    const QStringList words = instruction.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (words.isEmpty()) return QString();
    const QString kind = words.first().toUpper();
    if (kind == QLatin1String("NUMPAGES")) return QStringLiteral("pages");
    if (kind == QLatin1String("SECTIONPAGES")) return QStringLiteral("sectionpages");
    if (kind == QLatin1String("STYLEREF")) {
        // The style's name (quoted or not) and the switches that matter to what is shown; the result the file has is added after them, where the field is made.
        QString name, switches;
        int i = instruction.indexOf(QRegularExpression(QStringLiteral("STYLEREF"), QRegularExpression::CaseInsensitiveOption)) + 8;
        while (i < instruction.size() && instruction[i].isSpace()) ++i;
        if (i < instruction.size() && instruction[i] == QLatin1Char('"')) {
            const int close = instruction.indexOf(QLatin1Char('"'), i + 1);
            name = instruction.mid(i + 1, close < 0 ? -1 : close - i - 1);
            i = close < 0 ? int(instruction.size()) : close + 1;
        } else {
            const int start = i;
            while (i < instruction.size() && !instruction[i].isSpace() && instruction[i] != QLatin1Char('\\')) ++i;
            name = instruction.mid(start, i - start);
        }
        for (; i + 1 < instruction.size(); ++i)
            if (instruction[i] == QLatin1Char('\\')) {
                const QChar letter = instruction[i + 1].toLower();
                if (QStringLiteral("lnwrpt").contains(letter) && !switches.contains(letter)) switches += letter;
                ++i;
            }
        name = name.trimmed().left(256);
        return name.isEmpty() ? QString() : QStringLiteral("styleref:") + switches + QChar(0x1f) + name;
    }
    if (kind != QLatin1String("PAGE")) return QString();
    for (int i = 1; i + 1 < words.size(); ++i)
        if (words[i] == QLatin1String("\\*")) {
            const QString format = words[i + 1];
            if (format == QLatin1String("roman") || format == QLatin1String("ROMAN") || format == QLatin1String("alphabetic") || format == QLatin1String("ALPHABETIC"))
                return QStringLiteral("page:") + (format.startsWith(QLatin1String("alpha"), Qt::CaseInsensitive) ? (format == QLatin1String("alphabetic") ? QStringLiteral("alpha") : QStringLiteral("ALPHA")) : format);
            if (format.compare(QLatin1String("arabic"), Qt::CaseInsensitive) == 0) return QStringLiteral("page:arabic");
        }
    return QStringLiteral("page");
}

// The text an XML text stands for: its character references decoded, the five named (&lt; &gt; &quot; &apos; &amp;) and the numeric ones (&#233; and &#xE9;), each once (the text
// "&amp;#233;" is "&#233;"). A reference to something that is not a character the format allows is left as it was written.
QString unescapedXml(const QString &text)
{
    if (!text.contains(QLatin1Char('&'))) return text;
    static const QRegularExpression reference(QStringLiteral("&(?:#[xX]([0-9A-Fa-f]{1,8})|#([0-9]{1,10})|(lt|gt|quot|apos|amp));"));
    QString out;
    qsizetype last = 0;
    for (auto it = reference.globalMatch(text); it.hasNext();) {
        const QRegularExpressionMatch m = it.next();
        out += QStringView(text).mid(last, m.capturedStart() - last);
        last = m.capturedEnd();
        if (!m.captured(3).isEmpty()) {
            const QString name = m.captured(3);
            out += name == QLatin1String("lt") ? QLatin1Char('<') : name == QLatin1String("gt") ? QLatin1Char('>') : name == QLatin1String("quot") ? QLatin1Char('"') : name == QLatin1String("apos") ? QLatin1Char('\'') : QLatin1Char('&');
            continue;
        }
        bool ok = false;
        const uint code = m.captured(1).isEmpty() ? m.captured(2).toUInt(&ok, 10) : m.captured(1).toUInt(&ok, 16);
        const bool allowed = ok && (code == 0x9 || code == 0xA || code == 0xD || (code >= 0x20 && code <= 0x10FFFF)) && !(code >= 0xD800 && code <= 0xDFFF) && code != 0xFFFE && code != 0xFFFF;
        if (allowed) out += QString::fromUcs4(reinterpret_cast<const char32_t *>(&code), 1);
        else out += m.captured(0);
    }
    out += QStringView(text).mid(last);
    return out;
}

// The text inside an element kept as written (an instruction's words).
QString textOfXml(const QByteArray &xml)
{
    QString text = QString::fromUtf8(xml);
    text.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
    return unescapedXml(text);
}

// How many expressions the attributes of tags have been looked for with since the program began (one for each name), for a test of the cost of reading a file.
std::atomic<qint64> s_attributeExpressions{0};

// The attribute of a start tag kept as written.
QString attributeOfTag(const QByteArray &xml, const QString &name)
{
    // (The expression of each name is made once: a file can have hundreds of thousands of elements that are asked their attributes.)
    thread_local QHash<QString, QRegularExpression> expressions;
    auto it = expressions.constFind(name);
    if (it == expressions.constEnd()) {
        it = expressions.insert(name, QRegularExpression(QStringLiteral("[\\s:]%1\\s*=\\s*\"([^\"]*)\"").arg(QRegularExpression::escape(name))));
        ++s_attributeExpressions;
    }
    return it->match(QString::fromUtf8(xml.left(4000))).captured(1);
}

// Where an absolute position tab (w:ptab) takes the text after it, as tp::AlignTab says it: "alignment:relativeTo:leader" (right at the margin with no leader when the file says
// nothing: the alignment and the leader are read, as the engine knows them, and nothing else is kept).
QString alignTabOf(const QByteArray &ptab)
{
    const QString alignment = attributeOfTag(ptab, QStringLiteral("alignment")), relativeTo = attributeOfTag(ptab, QStringLiteral("relativeTo")), leader = attributeOfTag(ptab, QStringLiteral("leader"));
    return (alignment == QLatin1String("center") || alignment == QLatin1String("left") ? alignment : QStringLiteral("right")) + QLatin1Char(':') +
           (relativeTo == QLatin1String("indent") ? relativeTo : QStringLiteral("margin")) + QLatin1Char(':') +
           (leader == QLatin1String("dot") || leader == QLatin1String("hyphen") || leader == QLatin1String("underscore") || leader == QLatin1String("middleDot") ? leader : QStringLiteral("none"));
}

// Whether a font a symbol names is a symbol font (its characters are pictures of its own).
bool isSymbolFontName(const QString &font)
{
    const QString lower = font.toLower();
    return lower == QLatin1String("symbol") || lower.contains(QLatin1String("dings")) || lower.contains(QLatin1String("marlett")) || lower.contains(QLatin1String("mt extra"));
}

// Whether a note's reference says its mark follows it as text, in the run (w:customMarkFollows): it shows no number of its own and takes none.
// How an equation or a styled letter is set where its format does not say (layout only, never written): an equation's letters italic, bold, or neither as most of them are, the
// letters of the mathematical alphanumeric block as the style they stand for, and a double-struck one in the bundled face that has it.
void setMathLook(QTextCharFormat *format, const Inline &in)
{
    const bool equation = in.kind == Inline::ParaAtom && (in.tag == QLatin1String("oMath") || in.tag == QLatin1String("oMathPara"));
    const bool styled = in.kind == Inline::RunAtom && in.tag == QLatin1String("t");
    if ((!equation && !styled) || in.key.isEmpty()) return;
    if (in.key == QLatin1String("d")) format->setProperty(tp::DisplayFamily, QStringLiteral("DejaVu Sans"));
    else format->setProperty(tp::DisplayStyle, in.key);
}

// The alignment of a paragraph that holds nothing but equations that are paragraphs of their own (m:oMathPara): the group is centered unless its properties say left or right;
// -1 for any other paragraph. (Blanks, bookmarks, and the like beside them do not count.)
int displayMathAlignment(const Paragraph &p)
{
    bool found = false;
    QString jc;
    for (const Inline &in : p.content) {
        if (in.kind == Inline::Text) {
            if (!in.text.trimmed().isEmpty()) return -1;
        } else if (in.kind == Inline::RunAtom) {
            return -1;
        } else if (in.tag == QLatin1String("oMathPara")) {
            found = true;
            static const QRegularExpression align(QStringLiteral("<(?:[\\w.-]+:)?oMathParaPr>\\s*<(?:[\\w.-]+:)?jc\\s[^>]*?val=\"(\\w+)\""));
            if (jc.isEmpty()) jc = align.match(QString::fromUtf8(in.raw.left(600))).captured(1);
        } else if (in.tag == QLatin1String("oMath")) {
            return -1;
        }
    }
    if (!found) return -1;
    return jc == QLatin1String("left") ? int(Qt::AlignLeft) : jc == QLatin1String("right") ? int(Qt::AlignRight) : int(Qt::AlignHCenter);
}

// What a legacy form field shows where its result has no text of its own, from its begin mark's XML (the field's w:ffData): a check box its box (checked or not; as large as the
// text, whatever size it states: the run's own size is what is written back), a text field its default text, a drop-down the entry it has chosen (the `result` the file kept, else its `default`, else the first).
struct FormShown {
    QString text;
    bool box = false;
};

FormShown formShownOf(const QByteArray &begin)
{
    FormShown shown;
    if (!begin.contains("ffData")) return shown;
    auto unescaped = [](const QString &text) { return unescapedXml(text); };
    // (A list's entry is an element named listEntry, with or without a prefix; one whose name only begins so, listEntryNote, is not.)
    static const QRegularExpression entry(QStringLiteral("<(?:[\\w.-]+:)?listEntry(?=[\\s/>])[^>]*?[\\s:]val\\s*=\\s*\"([^\"]*)\"")), resultAt(QStringLiteral("<w:result[^>]*?[\\s:]val\\s*=\\s*\"(-?\\d+)\"")),
        defaultAt(QStringLiteral("<w:default[^>]*?[\\s:]val\\s*=\\s*\"([^\"]*)\"")), checkedAt(QStringLiteral("<w:checked\\b([^>]*)>")),
        valueOf(QStringLiteral("[\\s:]val\\s*=\\s*\"([^\"]*)\""));
    const QString raw = QString::fromUtf8(begin);
    if (begin.contains("ddList")) {
        QStringList entries;
        for (auto m = entry.globalMatch(raw); m.hasNext() && entries.size() < 4000;) entries << m.next().captured(1);
        if (entries.isEmpty()) return shown;
        const QRegularExpressionMatch result = resultAt.match(raw), def = defaultAt.match(raw);
        const int chosen = result.hasMatch() ? result.captured(1).toInt() : def.hasMatch() ? std::max(def.captured(1).toInt(), 0) : 0;
        shown.text = unescaped(entries.value(std::clamp(chosen, 0, int(entries.size()) - 1)));
    } else if (begin.contains("checkBox")) {
        // (A `checked` element is the state, whatever the default says; it is on unless it says off.)
        bool checked = false;
        if (const QRegularExpressionMatch m = checkedAt.match(raw); m.hasMatch()) {
            const QRegularExpressionMatch v = valueOf.match(m.captured(1));
            const QString value = v.captured(1).toLower();
            checked = !(v.hasMatch() && (value == QLatin1String("0") || value == QLatin1String("false") || value == QLatin1String("off")));
        } else if (const QRegularExpressionMatch d = defaultAt.match(raw); d.hasMatch()) {
            const QString value = d.captured(1).toLower();
            checked = value == QLatin1String("1") || value == QLatin1String("true") || value == QLatin1String("on");
        }
        shown.text = QString(QChar(checked ? 0x2612 : 0x2610));
        shown.box = true;
    } else if (begin.contains("textInput")) {
        if (const QRegularExpressionMatch m = defaultAt.match(raw); m.hasMatch()) shown.text = unescaped(m.captured(1)).left(4000);
    }
    return shown;
}

// How a form field's shown text is set: a box in the bundled face that has it.
void setFormLook(QTextCharFormat *format, const FormShown &shown)
{
    if (!shown.box) return;
    format->setProperty(tp::DisplayFamily, QStringLiteral("DejaVu Sans"));
}

bool markFollows(const QByteArray &reference)
{
    const QString value = attributeOfTag(reference, QStringLiteral("customMarkFollows"));
    return !value.isEmpty() && value != QLatin1String("0") && value != QLatin1String("false") && value != QLatin1String("off");
}

// Whether a tracked change deleted (or moved away) what an element is: `tag` is a revision container's name.
bool isDeletedRevision(const QString &tag) { return tag == QLatin1String("del") || tag == QLatin1String("moveFrom"); }

// A paragraph's mark that a tracked change deleted (a revision child of the mark's run properties).
bool markIsDeleted(const ParaProps &p)
{
    if (!p.markRun) return false;
    static const QRegularExpression deleted(QStringLiteral("^<[^\\s:>/]+:(del|moveFrom)[\\s/>]"));
    for (const RawChild &ch : p.markRun->extra)
        if (deleted.match(QString::fromUtf8(ch.xml.left(60))).hasMatch()) return true;
    return false;
}

// What a format is for text that a tracked change deleted: left out of the lines as hidden text is, but still the story's (the file keeps it). The format
// says whether it hid the text already (the file's own w:vanish, or its style's): that is what is written back, and the hiding that deleting adds is not.
QTextCharFormat asDeleted(QTextCharFormat format)
{
    format.setProperty(prop::TrackedDeleted, format.boolProperty(tp::HiddenText) ? prop::HiddenAlready : prop::HiddenByDeletion);
    format.setProperty(tp::HiddenText, true);
    return format;
}

// A paragraph of a header or footer in the form it is edited in: each field that begins and ends in it is one atom (kFieldGroupTag) of the runs that make it, written back as they
// were, which the story shows as one character with the result the field has (its live code where the pages answer it). A field whose pieces stand among tags that begin or end
// outside it, or one the paragraph ends inside, is left as the atoms it is.
Block withFieldsAsAtoms(const Block &block, int *styleRefs)
{
    const QVector<Inline> &c = block.para.content;
    bool any = false;
    for (const Inline &in : c)
        if ((in.kind == Inline::RunAtom && in.tag == QLatin1String("fldChar")) || (in.kind == Inline::ParaAtom && in.tag == QLatin1String("fldSimple"))) {
            any = true;
            break;
        }
    if (!any) return block;
    Block out = block;
    out.para.content.clear();
    auto isMark = [](const Inline &x, const char *key) { return x.kind == Inline::RunAtom && x.tag == QLatin1String("fldChar") && x.key == QLatin1String(key); };
    auto isSimple = [](const Inline &x, Inline::Role role) { return x.kind == Inline::ParaAtom && x.tag == QLatin1String("fldSimple") && x.role == role; };
    for (int i = 0; i < c.size();) {
        if (!isMark(c[i], "begin") && !isSimple(c[i], Inline::Open)) {
            out.para.content.push_back(c[i]);
            ++i;
            continue;
        }
        int depth = 0, end = -1;
        for (int k = i; k < c.size() && end < 0; ++k) {
            if (isMark(c[k], "begin") || isSimple(c[k], Inline::Open)) ++depth;
            else if ((isMark(c[k], "end") || isSimple(c[k], Inline::Close)) && --depth <= 0) end = k;
        }
        const bool ended = end >= 0;
        if (!ended) end = int(c.size()) - 1;
        // The tags inside it must come in pairs.
        bool paired = ended;
        QStringList open;
        for (int k = i; k <= end && paired; ++k) {
            const Inline &x = c[k];
            if (x.kind != Inline::ParaAtom || x.tag == QLatin1String("fldSimple")) continue;
            if (x.role == Inline::Open) {
                open << x.tag;
            } else if (x.role == Inline::Close) {
                if (open.isEmpty() || open.last() != x.tag) paired = false;
                else open.removeLast();
            }
        }
        if (!open.isEmpty()) paired = false;
        if (!paired) {
            for (int k = i; k <= end; ++k) out.para.content.push_back(c[k]);
            i = end + 1;
            continue;
        }
        // What the field is: its code (the instruction of the outermost), the text of its result, and the properties of the first of that text.
        QString instruction, result;
        RunProps look = c[i].props;
        bool separated = isSimple(c[i], Inline::Open), looked = false;
        int nesting = 0;
        if (separated)
            instruction = QString(attributeOfTag(c[i].raw, QStringLiteral("instr"))).replace(QLatin1String("&quot;"), QLatin1String("\"")).replace(QLatin1String("&apos;"), QLatin1String("'")).replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&amp;"), QLatin1String("&"));
        QVector<Inline> runs;
        for (int k = i; k <= end; ++k) {
            const Inline &x = c[k];
            if (isMark(x, "begin") || isSimple(x, Inline::Open)) ++nesting;
            else if (isMark(x, "end") || isSimple(x, Inline::Close)) --nesting;
            else if (isMark(x, "separate") && nesting == 1) separated = true;
            else if (x.kind == Inline::RunAtom && x.tag == QLatin1String("instrText") && nesting == 1 && !separated) instruction += textOfXml(x.raw);
            else if (x.kind == Inline::Text && separated && !x.text.isEmpty()) {
                result += x.text;
                if (!looked && !x.text.trimmed().isEmpty()) {
                    look = x.props;
                    looked = true;
                }
            }
            runs.push_back(x);   // (each run keeps the properties it has: a field's runs need not look alike)
        }
        Inline group;
        group.kind = Inline::ParaAtom;
        group.role = Inline::Single;
        group.tag = kFieldGroupTag;
        group.props = look;
        group.raw = inlinesXml(runs, 0, runs.size());
        group.display = result;
        QString live = liveFieldOf(instruction);
        if (live.startsWith(QLatin1String("styleref:"))) live += QChar(0x1f) + result.left(1000) + QChar(0x1f) + QString::number(++*styleRefs);
        group.key = live;
        if (live.isEmpty() && result.trimmed().isEmpty() && isMark(c[i], "begin"))
            if (const FormShown form = formShownOf(c[i].raw); !form.text.isEmpty()) group.display = form.text;
        out.para.content.push_back(group);
        i = end + 1;
    }
    return out;
}

// The blocks of a header or footer part that the program shows (kMostHeaderParagraphs paragraphs, those of the cells of a table counted), and the rest of the part as elements kept as
// written. A table is cut between its rows: the rows past the limit are one piece kept as XML, among the pieces kept between the rows (so they go back where they were); a table whose
// first row would pass the limit is kept whole. The paragraph that follows a cut table is shown too (what is kept after it goes after a paragraph that is not in its cells). The page
// shows this and so does the form the part is edited in: what is shown does not change as the part is opened for editing.
void boundedBlocks(const QVector<Block> &in, QVector<Block> *shown, QVector<Block> *kept)
{
    struct Row {
        qsizetype last;
        int paragraphs;
    };
    const qsizetype n = in.size();
    int paragraphs = 0;
    bool over = false, wantParagraph = false;
    auto keep = [&](const Block &b) {
        Block k = b;
        if (!b.isRaw) {
            k = Block();
            k.isRaw = true;
            k.raw = blockXml(b);
        }
        kept->push_back(k);
    };
    auto isStructure = [](const Block &b) {
        return b.isRaw && b.role != Inline::Single && (b.tag == QLatin1String("tbl") || b.tag == QLatin1String("tr") || b.tag == QLatin1String("tc"));
    };
    for (qsizetype i = 0; i < n; ++i) {
        const Block &b = in[i];
        if (over) {
            keep(b);
            continue;
        }
        if (!(isStructure(b) && b.tag == QLatin1String("tbl") && b.role == Inline::Open)) {
            if (!b.isRaw) {
                if (wantParagraph) {
                    over = true;   // (this one, and no more)
                } else if (paragraphs >= kMostHeaderParagraphs) {
                    over = true;
                    keep(b);
                    continue;
                }
                ++paragraphs;
                wantParagraph = false;
            }
            shown->push_back(b);
            continue;
        }
        // A table: where it ends, and its rows (a row has the paragraphs of its cells, and of the tables in them).
        qsizetype end = n - 1;
        QVector<Row> rows;
        int tables = 0, parts = 0;
        for (qsizetype k = i; k < n; ++k) {
            const Block &x = in[k];
            if (isStructure(x)) {
                const int step = x.role == Inline::Open ? 1 : -1;
                if (x.tag == QLatin1String("tbl")) tables += step;
                else if (x.tag == QLatin1String("tr") && tables == 1 && x.role == Inline::Open) rows.push_back({k, 0});
                else if (x.tag == QLatin1String("tr") && tables == 1 && !rows.isEmpty()) rows.last().last = k;
                parts += step;
                if (tables == 0) {
                    end = k;
                    break;
                }
            } else if (!x.isRaw && !rows.isEmpty()) {
                ++rows.last().paragraphs;
            }
        }
        const bool closed = tables == 0 && parts == 0;
        int fit = 0, sum = paragraphs;
        while (closed && fit < rows.size() && sum + rows[fit].paragraphs <= kMostHeaderParagraphs) sum += rows[fit++].paragraphs;
        if (!closed || (fit == 0 && !rows.isEmpty())) {
            for (qsizetype k = i; k <= end; ++k) keep(in[k]);
            over = true;
            continue;
        }
        if (fit == rows.size()) {
            for (qsizetype k = i; k <= end; ++k) shown->push_back(in[k]);
            paragraphs = sum;
            i = end;
            continue;
        }
        // Cut after row `fit - 1`: the rows from there to the table's end are kept, in the table.
        for (qsizetype k = i; k <= rows[fit - 1].last; ++k) shown->push_back(in[k]);
        Block rest;
        rest.isRaw = true;
        for (qsizetype k = rows[fit - 1].last + 1; k < end; ++k) rest.raw += blockXml(in[k]);
        shown->push_back(rest);
        shown->push_back(in[end]);
        paragraphs = sum;
        wantParagraph = true;
        i = end;
    }
}

// The blocks of a header or footer part for the form it is edited in: those the page shows, with the fields of their paragraphs as atoms; the rest, kept as written, goes back after the
// last of the paragraphs.
void blocksForEditing(const QVector<Block> &in, QVector<Block> *out, QVector<Block> *overflow)
{
    QVector<Block> shown;
    boundedBlocks(in, &shown, overflow);
    int styleRefs = 0;
    for (const Block &b : shown) out->push_back(b.isRaw ? b : withFieldsAsAtoms(b, &styleRefs));
}

// The properties of a frame (w:framePr, an element of a paragraph's properties kept as written), by their local names; empty for something that is not one.
QHash<QString, QString> framePropertiesOf(const QVector<RawChild> &extra, QByteArray *xml)
{
    static const QRegularExpression isFrame(QStringLiteral("^<(?:[\\w.-]+:)?framePr[\\s/>]"));
    static const QRegularExpression attribute(QStringLiteral("\\s(?!xmlns)(?:[\\w.-]+:)?([\\w.-]+)\\s*=\\s*\"([^\"]*)\""));
    QHash<QString, QString> out;
    for (const RawChild &ch : extra) {
        if (!ch.xml.contains("framePr")) continue;   // (the usual child has none: no need of the expression)
        const QString head = QString::fromUtf8(ch.xml.left(3000));
        if (!isFrame.match(head).hasMatch()) continue;
        *xml = ch.xml;
        const int close = head.indexOf(QLatin1Char('>'));
        auto it = attribute.globalMatch(head.left(close < 0 ? head.size() : close));
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            out.insert(m.captured(1), m.captured(2));
        }
        break;
    }
    return out;
}

// The frame properties a paragraph style has (its own and those of the styles it is based on, the nearer over the others, attribute by attribute): made once for a style.
struct StyleFrame {
    QHash<QString, QString> attributes;
    bool present = false;   // (some style of the chain has a frame, whatever it says)
};

StyleFrame styleFrameOf(const StyleSheet &sheet, const QString &styleId)
{
    QVector<const Style *> chain;
    for (QString at = styleId; !at.isEmpty() && chain.size() < 32;) {
        const Style *st = sheet.find(at);
        if (!st || chain.contains(st)) break;
        chain.push_back(st);
        at = st->basedOn;
    }
    StyleFrame out;
    for (int i = int(chain.size()) - 1; i >= 0; --i) {
        QByteArray here;
        const QHash<QString, QString> at = framePropertiesOf(chain[i]->pPr.extra, &here);
        if (here.isEmpty()) continue;
        out.present = true;
        for (auto it = at.cbegin(); it != at.cend(); ++it) out.attributes.insert(it.key(), it.value());
    }
    return out;
}

// The properties of the frame a paragraph is in: those of its style with its own over them, attribute by attribute. A paragraph of a style that sets a frame is in one
// whether or not it says so. `key` is what the properties come to (the paragraphs next to one another with the same are one frame); empty for a paragraph that is in none.
// `styles` has what each style gives, as they are asked for.
QHash<QString, QString> frameOfParagraph(const StyleSheet &sheet, const QString &styleId, const QVector<RawChild> &direct, QByteArray *key, QHash<QString, StyleFrame> *styles)
{
    auto found = styles->find(styleId);
    if (found == styles->end()) found = styles->insert(styleId, styleFrameOf(sheet, styleId));
    QHash<QString, QString> merged = found->attributes;
    QByteArray own;
    const QHash<QString, QString> mine = framePropertiesOf(direct, &own);
    for (auto it = mine.cbegin(); it != mine.cend(); ++it) merged.insert(it.key(), it.value());
    if (!found->present && own.isEmpty()) {
        key->clear();
        return {};
    }
    QStringList names = merged.keys();
    names.sort();
    key->clear();
    for (const QString &n : names) *key += n.toUtf8() + '=' + merged.value(n).toUtf8() + ';';
    if (key->isEmpty()) *key = "frame";
    return merged;
}

// The object a frame is, from its properties: where it is set (from the text, the margin, or the page, by an offset or at the start, middle, or end), how wide and how tall,
// the room round it, and how the text goes round it.
flow::FloatingObject frameObjectOf(const QHash<QString, QString> &at)
{
    using FO = flow::FloatingObject;
    auto points = [&](const char *name) { return std::clamp(std::isfinite(at.value(QString::fromLatin1(name)).toDouble()) ? at.value(QString::fromLatin1(name)).toDouble() / 20.0 : 0.0, -10000.0, 10000.0); };
    FO o;
    o.item = std::make_shared<TextItem>();
    o.frameWidth = std::max(0.0, points("w"));
    // (A height that is given is at least that, unless the frame says its height is the text's, or exactly that.)
    const QString rule = at.value(QStringLiteral("hRule"));
    o.frameHeight = rule == QLatin1String("auto") ? 0.0 : std::max(0.0, points("h"));
    o.frameExactHeight = rule == QLatin1String("exact");
    const QString hAnchor = at.value(QStringLiteral("hAnchor")), vAnchor = at.value(QStringLiteral("vAnchor"));
    o.hRel = hAnchor == QLatin1String("margin") ? FO::Rel::Margin : hAnchor == QLatin1String("page") ? FO::Rel::Page : FO::Rel::Column;
    o.vRel = vAnchor == QLatin1String("margin") ? FO::Rel::Margin : vAnchor == QLatin1String("page") ? FO::Rel::Page : FO::Rel::Paragraph;
    o.dx = points("x");
    o.dy = points("y");
    const QString xAlign = at.value(QStringLiteral("xAlign")), yAlign = at.value(QStringLiteral("yAlign"));
    // (Across, inside and outside are the side of the binding and the side away from it, by the page's number; down, inside is the top and outside the foot.)
    o.hAlign = xAlign == QLatin1String("center") ? FO::Align::Center : xAlign == QLatin1String("right") ? FO::Align::End : xAlign == QLatin1String("left") ? FO::Align::Start
               : xAlign == QLatin1String("outside") ? FO::Align::Outside : xAlign == QLatin1String("inside") ? FO::Align::Inside : FO::Align::None;
    o.vAlign = yAlign == QLatin1String("center") ? FO::Align::Center : yAlign == QLatin1String("bottom") || yAlign == QLatin1String("outside") ? FO::Align::End
               : yAlign == QLatin1String("top") || yAlign == QLatin1String("inside") ? FO::Align::Start : FO::Align::None;
    const double hSpace = std::max(0.0, points("hSpace")), vSpace = std::max(0.0, points("vSpace"));
    o.gap = QMarginsF(hSpace, vSpace, hSpace, vSpace);
    const QString wrap = at.value(QStringLiteral("wrap"));
    o.wrap = wrap == QLatin1String("none") ? FO::Wrap::None : wrap == QLatin1String("notBeside") ? FO::Wrap::TopBottom : FO::Wrap::Square;
    // A drop cap is a frame at the start of its paragraph, which the text goes round.
    if (!at.value(QStringLiteral("dropCap")).isEmpty() && at.value(QStringLiteral("dropCap")) != QLatin1String("none")) {
        o.hRel = FO::Rel::Column;
        o.vRel = FO::Rel::Paragraph;
        o.dx = o.dy = 0;
        o.hAlign = FO::Align::Start;
        o.vAlign = FO::Align::None;
        o.wrap = FO::Wrap::Square;
        o.frameShrinks = true;
    }
    return o;
}

} // namespace

QString liveFieldFor(const QString &instruction) { return liveFieldOf(instruction); }

qint64 attributeExpressionsMadeForTest() { return s_attributeExpressions.load(); }

// Fills `story` from the elements of a body or of a header or footer. A header's or footer's (`header`) fields for the page's number and the page
// counts are live fields of the text engine, its tables and pictures are left out, and a paragraph in a frame is set aligned as the frame is.
void WriteDoc::fillStory(QTextDocument *story, const QVector<Block> &bodyIn, bool header, const QString &part, bool headerFooter, bool append, int insertAt, bool editable)
{
    QVector<Block> prepared, overflow;
    // (A header's or footer's: the page shows, and the form it is edited in has, the same first part of it.)
    if (editable && append) {
        // (The endnotes the pages set are in the form the text is in, and no more than the pages set of them: what is shown is bounded by the most endnote paragraphs, not by a header's.)
        int styleRefs = 0;
        for (const Block &b : bodyIn) prepared.push_back(b.isRaw ? b : withFieldsAsAtoms(b, &styleRefs));
    } else if (editable) {
        blocksForEditing(bodyIn, &prepared, &overflow);
    }
    else if (headerFooter && !append) boundedBlocks(bodyIn, &prepared, &overflow);
    const QVector<Block> &body = editable || (headerFooter && !append) ? prepared : bodyIn;
    const StyleSheet &sheet = m_wml.styles;
    const Theme *thm = theme();
    const QString defaultStyle = defaultParagraphStyle();

    QTextCursor c(story);
    c.beginEditBlock();
    bool firstBlock = true;
    if (append) {
        if (insertAt >= 0) c.setPosition(std::min(insertAt, story->characterCount() - 1));   // (the end of a section's last paragraph, for its endnotes)
        else c.movePosition(QTextCursor::End);
        firstBlock = false;
    }
    auto startBlock = [&](const QTextBlockFormat &bf, const QTextCharFormat &cf) {
        if (firstBlock) {
            c.setBlockFormat(bf);
            c.setBlockCharFormat(cf);
            firstBlock = false;
        } else {
            c.insertBlock(bf, cf);
        }
    };

    QHash<QByteArray, QTextCharFormat> formats;
    const TableLayer *layer = nullptr;   // what the style of the table that the paragraphs are in gives them
    auto formatOf = [&](const QString &paraStyle, const RunProps &props) {
        const QByteArray key = paraStyle.toUtf8() + '\x01' + runPropsXml(props) + (layer ? '\x02' + layer->key.toUtf8() : QByteArray());
        auto it = formats.constFind(key);
        if (it != formats.constEnd()) return it.value();
        // (The link style a file puts on the runs of an entry of a table of contents, the paragraph style of which is toc 1 or the like, is not shown: an entry is as its style makes it.)
        QString characterStyle = props.style;
        if (!characterStyle.isEmpty()) {
            const Style *paragraph = sheet.find(paraStyle), *character = sheet.find(characterStyle);
            if (paragraph && character && paragraph->name.startsWith(QLatin1String("toc "), Qt::CaseInsensitive) && character->name.compare(QLatin1String("Hyperlink"), Qt::CaseInsensitive) == 0) characterStyle.clear();
        }
        RunProps resolved = sheet.run(paraStyle, characterStyle, layer);
        resolved.overlay(props);
        resolved.extra = props.extra;
        resolved.style = props.style;
        QTextCharFormat f = charFormatOf(resolved, thm, &props);
        // (A border the run has, w:bdr: the run's own, else its character style's, else its paragraph's style's (and the styles they are based on): style, width in eighths of a
        // point (twelve points at most), room (thirty-one at most), and color.)
        {
            QByteArray xml;
            auto of = [&](const QVector<RawChild> &extra) {
                for (const RawChild &ch : extra)
                    if (ch.xml.startsWith("<w:bdr")) return ch.xml;
                return QByteArray();
            };
            xml = of(props.extra);
            for (const QString &styleId : {characterStyle, paraStyle}) {
                QString id = styleId;
                for (int depth = 0; xml.isEmpty() && !id.isEmpty() && depth < 20; ++depth) {
                    const Style *st = sheet.find(id);
                    if (!st) break;
                    xml = of(st->rPr.extra);
                    id = st->basedOn;
                }
            }
            const QString style = attributeOfTag(xml, QStringLiteral("val")), color = attributeOfTag(xml, QStringLiteral("color"));
            if (!style.isEmpty() && style != QLatin1String("nil") && style != QLatin1String("none"))
                f.setProperty(tp::RunBorder, style + QLatin1Char(';') + QString::number(std::clamp(attributeOfTag(xml, QStringLiteral("sz")).toInt(), 1, 96)) + QLatin1Char(';') +
                                                 QString::number(std::clamp(attributeOfTag(xml, QStringLiteral("space")).toInt(), 0, 31)) + QLatin1Char(';') + (color.compare(QLatin1String("auto"), Qt::CaseInsensitive) == 0 ? QString() : color));
        }
        if (!f.hasProperty(tp::Language))
            if (const QString language = styleLanguage(paraStyle, props.style); !language.isEmpty() && language != proofingLanguage()) f.setProperty(tp::Language, language);
        formats.insert(key, f);
        return f;
    };

    QHash<int, NumState> numbering;
    QHash<QString, StyleFrame> styleFrames;   // (the frame each paragraph style gives, made when first asked)
    QHash<QString, ParaProps> baselines;
    QVector<Block> pending;   // body elements kept as XML that wait for the next paragraph
    TableBuilder tables(m_flow.get(), &sheet, m_wml.compatMode, body);

    // The paragraphs that end a section whose next one is continuous.
    QVector<bool> continuousAfter(header ? 0 : int(body.size()), false);
    if (!header) {
        bool nextContinuous = m_wml.lastSection.type == QLatin1String("continuous");
        for (int i = int(body.size()) - 1; i >= 0; --i)
            if (!body[i].isRaw && body[i].para.props.section) {
                continuousAfter[i] = nextContinuous;
                nextContinuous = body[i].para.props.section->type == QLatin1String("continuous");
            }
    }
    // The width of the text of the section each paragraph is in (the paragraph that ends a section is in it, and those before it are): what a rule that is a share of the text's is a
    // share of, less its paragraph's indents.
    QVector<double> sectionText(int(body.size()), 0.0);
    {
        double next = sectionTextWidth(m_tailSection ? *m_tailSection : m_wml.lastSection);
        for (int i = int(body.size()) - 1; i >= 0; --i) {
            if (!body[i].isRaw && body[i].para.props.section) next = sectionTextWidth(*body[i].para.props.section);
            sectionText[i] = next;
        }
    }
    int paragraphsMade = 0, endnoteNumber = m_endnoteStart - 1, blockNumber = -1;
    const int mostParagraphs = append ? kMostEndnoteParagraphs : kMostHeaderParagraphs;
    int styleRefsMade = 0;   // (each field that shows a paragraph style's text has a number in its code: two next to one another in one format would be one fragment of the text)
    // The format the endnote references of each paragraph are numbered in: its section's own (w:endnotePr of the section), else the settings'. Found as sectionText is, from
    // the sections as the file has them, for the story of the text alone.
    // (And whether its notes are set at the end of the section, which the section says (w:pos), else the settings.)
    QVector<QString> endnoteFormats;
    QVector<bool> endnotesAtSectionEnd;
    if (!header) {
        endnoteFormats.resize(int(body.size()));
        endnotesAtSectionEnd.resize(int(body.size()));
        auto numberingOf = [&](const office::wml::SectionProps &section) {
            office::wml::NoteNumbering own = office::wml::noteNumberingOf(section, "endnotePr");
            if (own.format.isEmpty()) own.format = m_endnoteFormat;
            if (own.position.isEmpty()) own.position = m_endnotePosition;
            return own;
        };
        office::wml::NoteNumbering next = numberingOf(m_wml.lastSection);
        bool anyAtEnd = next.position == QLatin1String("sectEnd");
        for (int i = int(body.size()) - 1; i >= 0; --i) {
            if (!body[i].isRaw && body[i].para.props.section) next = numberingOf(*body[i].para.props.section);
            endnoteFormats[i] = next.format;
            endnotesAtSectionEnd[i] = next.position == QLatin1String("sectEnd");
            anyAtEnd = anyAtEnd || endnotesAtSectionEnd[i];
        }
        if (!append) m_endnotesAtSectionEnds = anyAtEnd;
    }
    int sectionNo = 0;   // (the sections that have ended so far: the one the paragraph is in)
    auto lineWidth = [&]() {
        const double cell = tables.cellWidth();
        const QTextBlockFormat now = c.blockFormat();
        return std::max(1.0, (cell > 0 ? cell : sectionText.value(blockNumber, 468.0)) - now.leftMargin() - now.rightMargin());
    };
    // The frame of the paragraph before (its properties as written, and its key): the paragraphs next to one another that have the same properties are one frame.
    QByteArray lastFrameXml;
    int lastFrameKey = 0;
    for (const Block &blk : body) {
        ++blockNumber;
        // (A header's or footer's tables are laid out as the body's are: the parts of one are the blocks that make the cells; other raw elements are left out.)
        if (header && !editable && ((blk.isRaw && !tables.isStructural(blk)) || (!(headerFooter && !append) && (paragraphsMade >= mostParagraphs || (!blk.isRaw && ++paragraphsMade > mostParagraphs))))) continue;
        if (tables.isStructural(blk) || blk.isRaw) lastFrameKey = 0;
        if (tables.isStructural(blk)) {
            // The parts of a table (its start, rows, and cells) become the records of the cells that its paragraphs are given. What was kept after a cell's last
            // paragraph goes after it; for a cell that ends in a table, with the cell.
            if (blk.role == Inline::Close && blk.tag == QLatin1String("tc") && !pending.isEmpty()) {
                if (c.blockFormat().intProperty(flow::fp::Cell) == tables.key() && tables.key() != 0) {
                    QTextBlockFormat last = c.blockFormat();
                    setStored(last, prop::RawAfter, packBlocks(pending));
                    c.setBlockFormat(last);
                } else {
                    tables.addTail(pending);
                }
                pending.clear();
            }
            tables.take(blk, &pending);
            layer = tables.layer();
            continue;
        }
        if (blk.isRaw) {
            if (blk.display.isEmpty()) {
                pending.push_back(blk);
                continue;
            }
            // An element kept as XML that is shown: a stand-in paragraph that can't be edited.
            QTextBlockFormat pf;
            pf.setAlignment(Qt::AlignHCenter);
            pf.setTopMargin(6);
            pf.setBottomMargin(6);
            if (tables.key()) pf.setProperty(flow::fp::Cell, tables.key());
            setStored(pf, prop::RawBlock, packBlocks({blk}));
            if (!pending.isEmpty()) {
                setStored(pf, prop::RawBefore, packBlocks(pending));
                pending.clear();
            }
            QTextCharFormat pc;
            pc.setFontPointSize(10);
            pc.setFontItalic(true);
            pc.setForeground(QBrush(QColor(0x80, 0x80, 0x80)));
            pc.setProperty(tp::ThemeFont, QStringLiteral("minor"));
            pc.setProperty(tp::Field, QStringLiteral("raw:") + blk.display);
            startBlock(pf, pc);
            c.insertText(QString(QChar::ObjectReplacementCharacter), pc);
            continue;
        }

        const Paragraph &p = blk.para;
        const ParaProps &direct = p.props;
        const QString styleId = direct.style.isEmpty() ? defaultStyle : direct.style;
        const ParaProps base = sheet.paragraph(styleId, layer);
        int numId = direct.numId ? *direct.numId : base.numId.value_or(0);
        const int level = std::clamp(direct.ilvl ? *direct.ilvl : base.ilvl.value_or(0), 0, 8);
        if (numId > 0 && !m_wml.numbering.levelOf(numId, level)) numId = 0;
        // (The list's indents are the style's to override when the style says the paragraph is numbered, and the paragraph's own numbering
        // overrides its style's.)
        const bool numberedByItself = direct.numId.has_value();
        ParaProps resolved = numId > 0 ? baselineParagraph(styleId, numId, level, numberedByItself, &baselines, layer) : base;
        resolved.overlay(direct);
        // (What the paragraph keeps as written is its own, not its style's.)
        resolved.extra = direct.extra;
        resolved.spacingAttrs = direct.spacingAttrs;
        resolved.indAttrs = direct.indAttrs;
        resolved.style = styleId;
        resolved.numId = numId;
        resolved.ilvl = level;
        QTextBlockFormat bf = paragraphFormatOf(resolved);
        // (Line numbers: the paragraph's own element says, else its style's; an element that says off turns it off.)
        {
            auto suppresses = [](const QVector<RawChild> &extra, bool *stated) {
                bool on = false;
                for (const RawChild &child : extra) {
                    static const QRegularExpression element(QStringLiteral("^<(?:[\\w.-]+:)?suppressLineNumbers\\b([^>]*)>"));
                    const QRegularExpressionMatch m = element.match(QString::fromUtf8(child.xml.left(200)));
                    if (!m.hasMatch()) continue;
                    *stated = true;
                    on = !(m.captured(1).contains(QLatin1String("=\"0\"")) || m.captured(1).contains(QLatin1String("=\"false\"")) || m.captured(1).contains(QLatin1String("=\"off\"")));
                }
                return on;
            };
            bool stated = false;
            bool suppressed = suppresses(direct.extra, &stated);
            // (The style's, or the nearest style it is based on that says.)
            QString at = styleId;
            for (int depth = 0; !stated && !at.isEmpty() && depth < 32; ++depth) {
                const Style *style = sheet.find(at);
                if (!style) break;
                suppressed = suppresses(style->pPr.extra, &stated);
                at = style->basedOn;
            }
            if (suppressed) bf.setProperty(flow::fp::SuppressLineNumbers, true);
        }
        if (tables.key()) bf.setProperty(flow::fp::Cell, tables.key());
        // A paragraph of a frame in the body is set apart from the text: no lines and no room there, and a text box where the frame asks, at the paragraph after it.
        // (A header's or a footer's frame is a text box too where it is set at a place on the page: its logo, its page number at the page's top. One set beside the text, by its
        // alignment, is a paragraph of the story that takes no room, below.)
        QByteArray frameXml;
        QHash<QString, QString> frameAt;
        if (headerFooter && !tables.key()) frameAt = frameOfParagraph(sheet, styleId, direct.extra, &frameXml, &styleFrames);
        const bool headerFrame = headerFooter && !frameXml.isEmpty() && frameAt.value(QStringLiteral("vAnchor")) == QLatin1String("page");
        if ((!header || headerFrame) && !tables.key()) {
            if (!header) frameAt = frameOfParagraph(sheet, styleId, direct.extra, &frameXml, &styleFrames);
            if (frameXml.isEmpty()) {
                lastFrameKey = 0;
                lastFrameXml.clear();
            } else {
                if (lastFrameKey == 0 || frameXml != lastFrameXml) {
                    lastFrameKey = 0;
                    if (m_flow->objects.size() < kMostObjects) {
                        m_flow->objects.push_back(frameObjectOf(frameAt));
                        const int id = int(m_flow->objects.size());
                        m_flow->objects[id - 1].frame = int(m_flow->frames.size()) + 1;
                        m_flow->frames.push_back(id);
                        lastFrameKey = int(m_flow->frames.size());
                    }
                    lastFrameXml = frameXml;
                }
                if (lastFrameKey > 0) {
                    bf.setProperty(flow::fp::Frame, lastFrameKey);
                    bf.setProperty(tp::OutOfLayout, true);
                }
            }
        } else {
            lastFrameKey = 0;
        }
        // An empty paragraph that is a continuous section break takes no room (it is where the text before it ends: the first paragraph of the document has none, and keeps its line).
        if (!header && !firstBlock && blockNumber < continuousAfter.size() && continuousAfter[blockNumber] && p.content.isEmpty()) bf.setProperty(tp::OutOfFlow, true);
        if (header) {
            // A paragraph in a frame (the page number's, set to the side or the middle) is set that way, where the next paragraph begins, taking no room.
            for (const RawChild &ch : direct.extra) {
                if (!ch.xml.contains("framePr")) continue;
                const QString at = attributeOfTag(ch.xml, QStringLiteral("xAlign"));
                if (at == QLatin1String("center")) bf.setAlignment(Qt::AlignHCenter);
                else if (at == QLatin1String("right")) bf.setAlignment(Qt::AlignRight);
                else if (at == QLatin1String("left")) bf.setAlignment(Qt::AlignLeft);
                else if (at == QLatin1String("outside") || at == QLatin1String("inside")) bf.setProperty(tp::PageSideAlign, at == QLatin1String("outside") ? 1 : 2);   // (the page's number says which side)
                if (!at.isEmpty() && attributeOfTag(ch.xml, QStringLiteral("vAnchor")) != QLatin1String("page")) bf.setProperty(tp::OutOfFlow, true);
            }
        }
        if (!p.attrs.isEmpty()) setStored(bf, prop::ParaAttrs, p.attrs);
        // A paragraph that says it is numbered as its style says: that it said so is kept, for it changes whose indents win.
        if (numId > 0 && numberedByItself && base.numId.value_or(0) == numId) bf.setProperty(prop::NumDirect, true);
        // The tab stops of its own that the engine has no kind for (a bar) are kept beside the others.
        if (direct.tabs) {
            QVector<TabStop> odd;
            for (const TabStop &t : *direct.tabs)
                if (t.val == QLatin1String("bar") || t.val == QLatin1String("num")) odd.push_back(t);
            if (!odd.isEmpty()) setStored(bf, prop::OddTabs, packTabs(odd));
        }
        if (!pending.isEmpty()) {
            setStored(bf, prop::RawBefore, packBlocks(pending));
            pending.clear();
        }
        RunProps mark = sheet.run(styleId, direct.markRun ? direct.markRun->style : QString(), layer);   // (a character style on the mark counts)
        if (direct.markRun) {
            mark.overlay(*direct.markRun);
            mark.extra = direct.markRun->extra;
            mark.style = direct.markRun->style;
        }
        QTextCharFormat markFormat = charFormatOf(mark, thm, direct.markRun ? &*direct.markRun : nullptr);
        // A paragraph whose mark a tracked change deleted is joined to the next as the text stands (and ends no section).
        if (!direct.section && markIsDeleted(direct)) markFormat = asDeleted(markFormat);

        // A list: the items of a numbering id are in Qt lists of their own, by level (a shallower level's item ends the deeper levels' lists). The numbers they show are Qt's own
        // count, or, where that is not the file's, a marker of their own that renumberLists gives them when the story is filled.
        QTextListFormat lf;
        bool newList = false;
        QTextList *continuing = nullptr;
        if (numId > 0) {
            NumState &st = numbering[numId];
            for (int i = level + 1; i < 9; ++i) st.list[i] = nullptr;
            lf = listFormat(numId, level);
            continuing = st.list[level];
            newList = !continuing;
        }

        if (append) {
            // (The pages' copy of an endnote: it takes no part in what the document's paragraphs are, by their styles, outlines, or saved forms.)
            bf.setProperty(prop::EndnoteBlock, true);
            bf.clearProperty(prop::Outline);
            bf.clearProperty(tp::StyleName);
            if (editable) {
                // (A note's paragraph says which copy of its note it is of, and what its style is, which the copy does not carry: it is written back from those.)
                bf.setProperty(prop::EndnoteStyle, styleId);
                if (const int copy = blk.key.toInt()) bf.setProperty(prop::EndnoteNote, copy);
            }
            // The file's separator for the endnotes (w:separator, in a paragraph of nothing else) is a rule two inches long, as the footnote separator is: the paragraph's lower edge, over the
            // first 144 points of the column.
            bool separator = false, text = false;
            for (const Inline &in : p.content) {
                separator = separator || (in.kind == Inline::RunAtom && in.tag == QLatin1String("separator"));
                text = text || (in.kind == Inline::Text && !in.text.trimmed().isEmpty());
            }
            if (separator && !text) {
                const SectionProps &place = m_tailSection ? *m_tailSection : m_last;
                const int columns = std::max(1, place.cols);
                const double wide = (std::max(0, place.pageW - std::max(0, place.left + place.gutter) - std::max(0, place.right)) - (columns - 1) * std::max(0, place.colSpace)) / 20.0 / columns;
                tp::ParaBox box;
                box.bottom = tp::BoxSide{QStringLiteral("single"), 4, 0, QColor()};
                bf.setProperty(tp::Borders, box.spec());
                bf.setRightMargin(std::max(0.0, wide - 144.0));
            }
        }
        // (An equation alone in its paragraph is centered by the pages; what is written back is the paragraph as the file has it.)
        if (const int look = displayMathAlignment(p); look >= 0) bf.setProperty(tp::DisplayAlignment, look);
        startBlock(bf, markFormat);
        if (numId > 0) {
            NumState &st = numbering[numId];
            if (newList) st.list[level] = c.createList(lf);
            else continuing->add(c.block());
        }

        if (header && !editable) {
            // A field's instruction and result: the result (the text between its separator and its end) shows, or, for the page's number and the
            // page counts, one live field takes its place.
            struct Field {
                QString instruction;
                bool inResult = false;
                QTextCharFormat begin;
                QByteArray beginRaw;   // (the begin mark as written, with a form field's data)
                QVector<std::pair<QString, QTextCharFormat>> result;
            };
            QVector<Field> fields;
            auto put = [&](const QString &text, const QTextCharFormat &format) {
                if (fields.isEmpty()) c.insertText(text, format);
                else if (fields.last().inResult) fields.last().result.push_back({text, format});
            };
            auto finish = [&] {
                const Field f = fields.takeLast();
                QString live = liveFieldOf(f.instruction);
                if (live.startsWith(QLatin1String("styleref:"))) {
                    // (The result the file has is kept in the code, for a page that shows no paragraph of the style, or a layout that is of no page.)
                    QString kept;
                    for (const auto &r : f.result) kept += r.first;
                    live += QChar(0x1f) + kept.left(1000) + QChar(0x1f) + QString::number(++styleRefsMade);
                }
                if (!live.isEmpty()) {
                    QTextCharFormat ff = f.result.isEmpty() ? f.begin : f.result.first().second;
                    ff.setProperty(tp::Field, live);
                    put(QString(QChar::ObjectReplacementCharacter), ff);
                } else {
                    // (A form field whose result has no text shows what it would: its box, its default text, the entry it has chosen.)
                    bool text = false;
                    for (const auto &r : f.result) text = text || !r.first.trimmed().isEmpty();
                    const FormShown form = text ? FormShown() : formShownOf(f.beginRaw);
                    if (!form.text.isEmpty()) {
                        QTextCharFormat ff = f.begin;
                        setFormLook(&ff, form);
                        put(form.text, ff);
                    } else {
                        // (A reference whose result is nothing but blanks, to a form field that has none of its own: the word processor shows the text it refers to, and the blanks are no part of a
                        // line's height. They are shown as the file has them.)
                        const bool blank = !text && f.instruction.trimmed().startsWith(QLatin1String("REF "), Qt::CaseInsensitive);
                        for (const auto &r : f.result) {
                            QTextCharFormat shown = r.second;
                            if (blank) shown.setProperty(tp::BlankFieldResult, true);
                            put(r.first, shown);
                        }
                    }
                }
            };
            for (const Inline &in : p.content) {
                if (in.kind == Inline::Text) {
                    if (!in.text.isEmpty()) put(in.text, formatOf(styleId, in.props));
                } else if (in.kind == Inline::RunAtom && in.tag == QLatin1String("fldChar")) {
                    if (in.key == QLatin1String("begin") && fields.size() < kMostFieldNesting) {
                        fields.push_back(Field());
                        fields.last().begin = formatOf(styleId, in.props);
                        fields.last().beginRaw = in.raw;
                    } else if (in.key == QLatin1String("separate") && !fields.isEmpty()) {
                        fields.last().inResult = true;
                    } else if (in.key == QLatin1String("end") && !fields.isEmpty()) {
                        finish();
                    }
                } else if (in.kind == Inline::RunAtom && (in.tag == QLatin1String("footnoteRef") || in.tag == QLatin1String("endnoteRef"))) {
                    // (The mark that begins a note's text shows the note's number: the pages give it.)
                    QTextCharFormat mark = formatOf(styleId, in.props);
                    mark.setProperty(tp::Field, append && in.tag == QLatin1String("endnoteRef") ? QStringLiteral("raw:") + in.display : QStringLiteral("notemark"));
                    put(QString(QChar::ObjectReplacementCharacter), mark);
                } else if (in.kind == Inline::RunAtom && in.tag == QLatin1String("sym") && !in.display.isEmpty() && isSymbolFontName(attributeOfTag(in.raw, QStringLiteral("font")))) {
                    // (A symbol of a symbol font, as the text has it: shown in that font. Where it is a blank, as an older program's mark of a note begins its text, it shows
                    // nothing, but its font and its raised stand count for the line.)
                    QTextCharFormat symbol = formatOf(styleId, in.props);
                    symbol.setProperty(tp::DisplayFamily, attributeOfTag(in.raw, QStringLiteral("font")));
                    symbol.setProperty(tp::Field, QStringLiteral("raw:") + in.display);
                    put(QString(QChar::ObjectReplacementCharacter), symbol);
                } else if (in.kind == Inline::RunAtom && in.tag == QLatin1String("pgNum")) {
                    // (The page's number, as the run's own element has it.)
                    QTextCharFormat number = formatOf(styleId, in.props);
                    number.setProperty(tp::Field, liveFieldOf(QStringLiteral("PAGE")));
                    put(QString(QChar::ObjectReplacementCharacter), number);
                } else if (in.kind == Inline::RunAtom && drawnTag(in.tag)) {
                    QTextCharFormat object = formatOf(styleId, in.props);
                    if (drawnObjects(in, part, &object, lineWidth(), &p.content)) put(QString(QChar::ObjectReplacementCharacter), object);
                } else if (!in.display.isEmpty() && ((in.kind == Inline::ParaAtom && (in.tag == QLatin1String("oMath") || in.tag == QLatin1String("oMathPara"))) || (in.kind == Inline::RunAtom && in.tag == QLatin1String("t")))) {
                    // (An equation, or a styled letter of the mathematical alphanumeric block: the line of text, or the letter it is a style of.)
                    QTextCharFormat shown = formatOf(styleId, in.props);
                    setMathLook(&shown, in);
                    put(in.display, shown);
                } else if (in.kind == Inline::RunAtom && in.tag == QLatin1String("ruby") && !in.display.isEmpty()) {
                    // (Ruby text, as in the text: its base, and the ruby after it in brackets.)
                    put(in.display, formatOf(styleId, in.props));
                } else if (in.kind == Inline::RunAtom && in.tag == QLatin1String("ptab")) {
                    // (An absolute position tab: a tab that goes where it says, whatever the paragraph's stops.)
                    QTextCharFormat tab = formatOf(styleId, in.props);
                    tab.setProperty(tp::AlignTab, alignTabOf(in.raw));
                    put(QStringLiteral("\t"), tab);
                } else if (in.kind == Inline::RunAtom && in.tag == QLatin1String("instrText")) {
                    if (!fields.isEmpty() && !fields.last().inResult) fields.last().instruction += textOfXml(in.raw);
                } else if (in.kind == Inline::ParaAtom && in.tag == QLatin1String("fldSimple") && in.role == Inline::Open) {
                    if (fields.size() < kMostFieldNesting) {
                        fields.push_back(Field());
                        fields.last().instruction = QString(attributeOfTag(in.raw, QStringLiteral("instr"))).replace(QLatin1String("&quot;"), QLatin1String("\"")).replace(QLatin1String("&apos;"), QLatin1String("'")).replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&amp;"), QLatin1String("&"));
                        fields.last().inResult = true;
                    }
                } else if (in.kind == Inline::ParaAtom && in.tag == QLatin1String("fldSimple") && in.role == Inline::Close) {
                    if (!fields.isEmpty()) finish();
                }
            }
            while (!fields.isEmpty()) finish();   // (a field the paragraph ended inside of shows what it read)
            continue;
        }
        // A legacy form field whose result has no text shows what it would (formShownOf): a drop-down the entry it has chosen, a check box its box, a text field its default text. By the
        // position of its begin mark in the paragraph's content. (One pass over the elements: the fields open at a time are a stack, each remembering whether its separator has come and
        // whether its result has text.)
        QHash<int, FormShown> formShown;
        QSet<int> blankResult;   // (the pieces of text that are the result of a reference that is nothing but blanks: they are no part of a line's height)
        {
            struct Open {
                int at = 0;
                bool separated = false, hasText = false;
                QString instruction;
                QVector<int> pieces;
            };
            QVector<Open> open;
            auto close = [&](const Open &field) {
                if (field.hasText) return;
                if (field.instruction.trimmed().startsWith(QLatin1String("REF "), Qt::CaseInsensitive))
                    for (const int piece : field.pieces) blankResult.insert(piece);
                const FormShown shown = formShownOf(p.content[field.at].raw);
                if (!shown.text.isEmpty()) formShown.insert(field.at, shown);
            };
            for (int at = 0; at < p.content.size(); ++at) {
                ++m_dropDownSteps;
                const Inline &in = p.content[at];
                if (in.kind == Inline::Text) {
                    if (!open.isEmpty() && open.last().separated) {
                        if (!in.text.trimmed().isEmpty()) open.last().hasText = true;
                        else if (open.last().pieces.size() < 64 && in.text.size() >= 2) open.last().pieces << at;
                    }
                    continue;
                }
                if (in.kind == Inline::RunAtom && in.tag == QLatin1String("instrText")) {
                    if (!open.isEmpty() && !open.last().separated && open.last().instruction.size() < 64) open.last().instruction += textOfXml(in.raw);
                    continue;
                }
                if (in.kind != Inline::RunAtom || in.tag != QLatin1String("fldChar")) continue;
                if (in.key == QLatin1String("begin")) open.push_back({at, false, false});
                else if (in.key == QLatin1String("separate") && !open.isEmpty()) open.last().separated = true;
                else if (in.key == QLatin1String("end") && !open.isEmpty()) {
                    close(open.last());
                    open.pop_back();
                }
            }
            for (const Open &field : open) close(field);   // (a field the paragraph ended inside of)
        }
        // The results of the paragraph's page references (PAGEREF, without a switch that asks for a place or a paragraph's number): the text of a result is one fragment that the pages
        // show the number of the page the bookmark is on in place of (the file's own result is what is kept in the story, as it was). `refOf` says a field's first piece of text, and
        // `refRest` the later pieces of its result, which are joined to the first.
        QHash<int, QString> refOf;
        QVector<bool> refRest(p.content.size(), false);
        {
            struct Open {
                QString instruction, name;
                bool inResult = false;
                int first = -1;
            };
            QVector<Open> open;
            int deleting = 0;
            auto nameOf = [](const QString &instruction) -> QString {
                const QStringList words = instruction.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
                if (words.size() < 2 || words[0].compare(QLatin1String("PAGEREF"), Qt::CaseInsensitive) != 0) return QString();
                for (int i = 2; i < words.size(); ++i)
                    if (words[i] == QLatin1String("\\p") || words[i] == QLatin1String("\\n") || words[i] == QLatin1String("\\r") || words[i] == QLatin1String("\\w")) return QString();
                QString name = words[1];
                name.remove(QLatin1Char('"'));
                return name;
            };
            for (int i = 0; i < p.content.size(); ++i) {
                const Inline &in = p.content[i];
                if (in.kind == Inline::ParaAtom && isDeletedRevision(in.tag)) deleting = std::max(0, deleting + (in.role == Inline::Open ? 1 : in.role == Inline::Close ? -1 : 0));
                if (in.kind == Inline::RunAtom && in.tag == QLatin1String("fldChar")) {
                    if (in.key == QLatin1String("begin") && open.size() < kMostFieldNesting) open.push_back(Open());
                    else if (in.key == QLatin1String("separate") && !open.isEmpty()) {
                        open.last().inResult = true;
                        open.last().name = nameOf(open.last().instruction);
                    } else if (in.key == QLatin1String("end") && !open.isEmpty()) open.pop_back();
                } else if (in.kind == Inline::RunAtom && in.tag == QLatin1String("instrText")) {
                    if (!open.isEmpty() && !open.last().inResult) open.last().instruction += textOfXml(in.raw);
                } else if (in.kind == Inline::ParaAtom && in.tag == QLatin1String("fldSimple") && in.role == Inline::Open) {
                    if (open.size() < kMostFieldNesting) {
                        Open field;
                        field.inResult = true;
                        field.name = nameOf(QString(attributeOfTag(in.raw, QStringLiteral("instr"))).replace(QLatin1String("&quot;"), QLatin1String("\"")));
                        open.push_back(field);
                    }
                } else if (in.kind == Inline::ParaAtom && in.tag == QLatin1String("fldSimple") && in.role == Inline::Close) {
                    if (!open.isEmpty()) open.pop_back();
                } else if (in.kind == Inline::Text && !in.text.isEmpty() && deleting == 0 && !open.isEmpty() && open.last().inResult && !open.last().name.isEmpty()) {
                    Open &field = open.last();
                    if (field.first < 0) {
                        field.first = i;
                        refOf.insert(i, field.name + QChar(0x1f) + in.text);
                    } else {
                        refRest[i] = true;
                        refOf[field.first] += in.text;
                    }
                }
            }
        }
        int deletedDepth = 0;   // (inside the content a tracked change deleted)
        double advanceTop = -1;   // (a field that sets the text after it at a place from the page's top, ADVANCE with \\y: the paragraph is no higher than that)
        for (int inAt = 0; inAt < p.content.size(); ++inAt) {
            const Inline &in = p.content[inAt];
            if (in.kind == Inline::ParaAtom && isDeletedRevision(in.tag)) deletedDepth = std::max(0, deletedDepth + (in.role == Inline::Open ? 1 : in.role == Inline::Close ? -1 : 0));
            const bool deleted = deletedDepth > 0 && !(in.kind == Inline::ParaAtom && in.role != Inline::Single && isDeletedRevision(in.tag));
            if (in.kind == Inline::Text) {
                if (refRest[inAt]) continue;   // (its text is in the first piece of the result)
                if (const auto ref = refOf.constFind(inAt); ref != refOf.constEnd() && !deleted) {
                    QTextCharFormat shown = formatOf(styleId, in.props);
                    shown.setProperty(tp::Field, QStringLiteral("pageref:") + ref->section(QChar(0x1f), 0, 0));
                    c.insertText(ref->section(QChar(0x1f), 1), shown);
                    continue;
                }
                if (!in.text.isEmpty()) {
                    QTextCharFormat shown = deleted ? asDeleted(formatOf(styleId, in.props)) : formatOf(styleId, in.props);
                    if (blankResult.contains(inAt)) shown.setProperty(tp::BlankFieldResult, true);
                    c.insertText(in.text, shown);
                }
                continue;
            }
            // (The rule of the notes' separator is its paragraph's border: the mark in it is no character of the pages' copy, which is not written back.)
            if (append && in.kind == Inline::RunAtom && (in.tag == QLatin1String("separator") || in.tag == QLatin1String("continuationSeparator"))) continue;
            if (!deleted && advanceTop < 0)
                if (const std::optional<double> asked = advanceTopOf(in)) advanceTop = *asked;
            QTextCharFormat af = formatOf(styleId, in.props);
            if (deleted) af = asDeleted(af);
            Inline packed = in;
            if (in.kind == Inline::ParaAtom && in.tag == kFieldGroupTag) packed.look = runPropsXml(runPropsWritten(af, styleId, layer));   // (what writing the story gives it, to tell it formatted)
            setStored(af, prop::Atom, packAtom(packed));
            setMathLook(&af, in);
            // A reference to a footnote of the file is the note's (the pages lay it out at their foot); one to an endnote shows its number, in the format
            // the settings give (lower case roman numerals, a word processor's own).
            QString field = in.kind == Inline::RunAtom && in.tag == QLatin1String("pgNum") ? liveFieldOf(QStringLiteral("PAGE")) : QStringLiteral("raw:") + (formShown.contains(inAt) ? formShown.value(inAt).text : in.display);
            if (formShown.contains(inAt)) setFormLook(&af, formShown.value(inAt));
            // (The mark that begins a note's text shows the number the pages give it.)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("footnoteRef") && !append) field = QStringLiteral("notemark");
            if (in.kind == Inline::ParaAtom && in.tag == kFieldGroupTag && !in.key.isEmpty()) field = in.key;   // (a field the pages answer: the page's number, the count of pages...)
            // A symbol of a font that isn't a symbol font (and isn't on the machine) is its character, in that font's stand-in.
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("sym")) {
                const QString font = attributeOfTag(in.raw, QStringLiteral("font"));
                bool ok = false;
                const uint code = attributeOfTag(in.raw, QStringLiteral("char")).toUInt(&ok, 16);
                const bool symbols = isSymbolFontName(font);
                // A symbol of a symbol font is set in that font whatever font the run is in, as its character is (where the machine lacks the font the page shows the picture the
                // character stands for, as wide as the font makes it).
                if (!font.isEmpty() && symbols) af.setProperty(tp::DisplayFamily, font);
                if (ok && !font.isEmpty() && !symbols && !QFontDatabase::hasFamily(font)) {
                    const uint ch = code >= 0xF000 && code <= 0xF0FF ? code - 0xF000 : code;
                    if (ch >= 0x20 && ch <= 0xFFFF) {
                        field = QStringLiteral("raw:") + QString(QChar(char16_t(ch)));
                        af.setProperty(tp::DisplayFamily, font);   // (the page's: the run as the file has it is kept as it is)
                    }
                }
            }
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("ptab") && !deleted) {
                // (An absolute position tab shows as a tab that goes where it says; the file's element is kept as it is.)
                field = QStringLiteral("raw:\t");
                af.setProperty(tp::AlignTab, alignTabOf(in.raw));
            } else if (in.kind == Inline::RunAtom && !deleted && drawnTag(in.tag) && drawnObjects(in, part, &af, lineWidth(), &p.content)) field = QStringLiteral("raw:");
            else if (in.tag == QLatin1String("footnoteReference") && m_noteBlocks.contains(QStringLiteral("fn") + in.key)) {
                field = QStringLiteral("footnote:fn") + in.key;
                if (!header && !append) m_footnotesCitedAtOpen.insert(in.key.toInt());
                if (markFollows(in.raw)) af.setProperty(tp::NoteMarkCustom, true);
            }
            else if (in.tag == QLatin1String("endnoteReference")) {
                const QString shown = markFollows(in.raw) ? QString() : noteNumberText(endnoteFormats.value(blockNumber, m_endnoteFormat), deleted ? endnoteNumber + 1 : ++endnoteNumber);   // (a deleted one takes no number)
                field = QStringLiteral("raw:") + shown;
                if (!header && !deleted && m_endnoteRefs.size() < kMostEndnotes) m_endnoteRefs.push_back({in.key, shown, sectionNo, endnotesAtSectionEnd.value(blockNumber)});
                if (!header && !append && !deleted && m_endnoteBlocks.contains(in.key)) m_endnotesCitedAtOpen.insert(in.key.toInt());
            }
            af.setProperty(tp::Field, field);
            c.insertText(QString(QChar::ObjectReplacementCharacter), af);
        }
        if (advanceTop >= 0) {
            QTextBlockFormat asked = c.blockFormat();
            asked.setProperty(flow::fp::AdvanceTop, advanceTop);
            c.setBlockFormat(asked);
        }
        if (direct.section) {
            QTextBlockFormat last = c.blockFormat();
            last.setProperty(flow::fp::SectionBreakAfter, true);
            setStored(last, prop::SectionData, packSection(*direct.section));
            c.setBlockFormat(last);
            ++sectionNo;
        }
    }
    if (!header && !append) m_endnoteSections = sectionNo + 1;

    if (firstBlock) {
        // Nothing in the body: one empty paragraph in the default style.
        startBlock(styleBlockFormat(defaultStyle), styleCharFormat(defaultStyle));
    }
    if (headerFooter) {
        // A frame of more paragraphs than a frame has is paragraphs of the text, as the body's is (the pages make a text box of each frame, once for the story).
        for (QTextBlock b = story->begin(); b.isValid();) {
            const int key = b.blockFormat().intProperty(flow::fp::Frame);
            if (key <= 0) {
                b = b.next();
                continue;
            }
            QTextBlock e = b;
            int paragraphs = 1;
            while (e.next().isValid() && e.next().blockFormat().intProperty(flow::fp::Frame) == key) {
                e = e.next();
                ++paragraphs;
            }
            if (paragraphs > flow::kMostFrameParagraphs)
                for (QTextBlock t = b;; t = t.next()) {
                    QTextBlockFormat bf = t.blockFormat();
                    bf.clearProperty(flow::fp::Frame);
                    bf.clearProperty(tp::OutOfLayout);
                    QTextCursor(t).setBlockFormat(bf);
                    if (t == e) break;
                }
            b = e.next();
        }
    }
    if (header) {
        if (editable) {
            // What stands after the last paragraph (elements kept as XML, and the paragraphs past the limit) is kept with it.
            for (const Block &b : overflow) pending.push_back(b);
            // (What stands after a table that ends the shown part goes after a paragraph of the story itself, not in the cell that the last paragraph is in.)
            if (!pending.isEmpty() && c.blockFormat().intProperty(flow::fp::Cell) != 0) startBlock(styleBlockFormat(defaultStyle), styleCharFormat(defaultStyle));
            if (!pending.isEmpty()) {
                QTextBlockFormat bf = c.blockFormat();
                setStored(bf, prop::RawAfter, packBlocks(pending));
                c.setBlockFormat(bf);
            }
            QTextCursor first(story->begin());
            QTextBlockFormat bf = story->begin().blockFormat();
            setStored(bf, prop::MarkFormat, packFormat(story->begin().charFormat()));
            first.setBlockFormat(bf);
        }
        c.endEditBlock();
        renumberLists(story);
        return;
    }
    if (!pending.isEmpty()) {
        QTextBlockFormat bf = c.blockFormat();
        setStored(bf, prop::RawAfter, packBlocks(pending));
        c.setBlockFormat(bf);
    }
    // The last section's properties are the first paragraph's (prop::LastSection).
    {
        QTextCursor first(story->begin());
        QTextBlockFormat bf = story->begin().blockFormat();
        setStored(bf, prop::LastSection, packSection(m_last));
        setStored(bf, prop::MarkFormat, packFormat(story->begin().charFormat()));   // (and its mark's format, which the engine can't undo a change of)
        first.setBlockFormat(bf);
    }
    c.endEditBlock();
    renumberLists(story);
}

bool WriteDoc::drawnObjects(const Inline &atom, const QString &part, QTextCharFormat *format, double lineWidth, const QVector<Inline> *content)
{
    const QVector<Shape> shapes = readShapes(atom.raw);
    if (shapes.isEmpty()) return false;
    flow::FlowDocument &f = *m_flow;
    // The picture of a shape: its bytes (from the part the run is in), kept once for the document.
    // Lettering on a path is bounded in how many shapes and how many letters (each is outlined letter by letter): what a file asks for past that is left out.
    auto letteringFits = [&](const Shape &s) {
        if (m_letteringShapes >= kMostLetteringShapes || m_letteringLetters + int(s.pathText.size()) > kMostLetteringCharacters) return false;
        ++m_letteringShapes;
        m_letteringLetters += int(s.pathText.size());
        return true;
    };
    bool shownMissing = false;   // (the picture the last call made is the mark of a picture that cannot be shown)
    auto pictureOf = [&](const Shape &s) -> int {
        shownMissing = false;
        if (s.embed.isEmpty() || f.images.size() >= kMostImages) return -1;
        QString target;
        const QVector<Relationship> &links = part.isEmpty() ? m_wml.mainRelationships : m_wml.partRelationships.value(part);
        for (const Relationship &r : links)
            if (r.id == s.embed && !r.external) target = r.target;
        if (target.isEmpty()) return -1;
        QByteArray bytes = partData(target);
        // (A picture whose other form is one that is not drawn is shown as a picture that cannot be.)
        bool missing = false;
        for (const Relationship &r : links)
            if (!s.pictEmbed.isEmpty() && r.id == s.pictEmbed && !r.external && isUndrawnPictureTarget(r.target) && s.width > 0 && s.height > 0) {
                bytes = missingPictureMarkPng();
                missing = true;
            }
        shownMissing = missing;
        if (bytes.isEmpty()) return -1;
        for (int i = 0; i < f.images.size(); ++i)
            if (f.images[i].bytes == bytes) return i;
        flow::FlowImage image;
        image.bytes = bytes;
        image.format = missing ? QStringLiteral("png") : QFileInfo(target).suffix().toLower();
        f.images.push_back(image);
        return int(f.images.size()) - 1;
    };
    auto colorOf = [](const QString &rgb) { return ColorRef::rgb(QColor(QLatin1Char('#') + rgb)); };
    int firstObject = 0, lastObject = 0;
    bool inlinePicture = false;
    for (const Shape &s : shapes) {
        // Lettering on a path and a horizontal rule that are not set at a place are set in the line, as a picture is.
        const bool lettering = !s.rule && !s.pathText.isEmpty() && s.kind != Shape::Picture;
        if (!s.floating && !inlinePicture && (s.rule || lettering)) {
            const ItemPtr set = s.rule ? ruleItem(s, lineWidth, resolveCharFormat(*format, f.env).font(), content && holdsOnlyObjects(*content)) : (letteringFits(s) ? letteringItem(s) : nullptr);
            if (set) {
                if (!s.rule) set->wrap.left = set->wrap.right = set->wrap.top = set->wrap.bottom = 0;
                format->setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(set->toJson()).toJson(QJsonDocument::Compact)));
                inlinePicture = true;
                continue;
            }
            if (lettering) continue;   // (past the bounds it is left out, not drawn as the box it is)
        }
        if (s.kind == Shape::Picture && !s.floating && !inlinePicture) {
            const int image = pictureOf(s);
            if (image < 0 || s.width <= 0 || s.height <= 0) continue;
            // A picture in the line: an object set in the text, with no room round it of its own.
            auto pic = std::make_shared<PictureItem>();
            pic->rect = QRectF(0, 0, s.width, s.height);
            dressPicture(*pic, s);
            if (shownMissing) showAsMissing(*pic);
            pic->imageId = QStringLiteral("flow-img:%1").arg(image);
            pic->wrap.left = pic->wrap.right = pic->wrap.top = pic->wrap.bottom = 0;
            pic->altText = s.alt;
            format->setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(pic->toJson()).toJson(QJsonDocument::Compact)));
            inlinePicture = true;
            continue;
        }
        if (f.objects.size() >= kMostObjects) break;
        flow::FloatingObject o;
        using Rel = flow::FloatingObject::Rel;
        // (Across, a position measured from the paragraph or the line is from the column; down, one from the column is from the paragraph, and one from a character is from the line.)
        auto relOf = [](Shape::Rel r, bool across) {
            switch (r) {
            case Shape::Margin: return Rel::Margin;
            case Shape::Page: return Rel::Page;
            case Shape::Character: return across ? Rel::Character : Rel::Line;
            case Shape::LineOf: return across ? Rel::Column : Rel::Line;
            case Shape::LeftMargin: return Rel::LeftMargin;
            case Shape::RightMargin: return Rel::RightMargin;
            case Shape::TopMargin: return Rel::TopMargin;
            case Shape::BottomMargin: return Rel::BottomMargin;
            case Shape::InsideMargin: return Rel::InsideMargin;
            case Shape::OutsideMargin: return Rel::OutsideMargin;
            default: return across ? Rel::Column : Rel::Paragraph;
            }
        };
        o.hRel = relOf(s.hRel, true);
        o.vRel = relOf(s.vRel, false);
        auto align = [](Shape::Align a) { return a == Shape::Start ? flow::FloatingObject::Align::Start : a == Shape::Center ? flow::FloatingObject::Align::Center
                                                                : a == Shape::End ? flow::FloatingObject::Align::End : a == Shape::Inside ? flow::FloatingObject::Align::Inside
                                                                : a == Shape::Outside ? flow::FloatingObject::Align::Outside : flow::FloatingObject::Align::None; };
        o.hAlign = align(s.hAlign);
        o.vAlign = align(s.vAlign);
        o.dx = s.left;
        o.dy = s.top;
        o.behind = s.behind;
        o.z = s.z;
        o.wrap = s.wrap == Shape::Square ? flow::FloatingObject::Wrap::Square : s.wrap == Shape::Tight ? flow::FloatingObject::Wrap::Tight
                 : s.wrap == Shape::TopBottom ? flow::FloatingObject::Wrap::TopBottom : flow::FloatingObject::Wrap::None;
        o.side = s.side == Shape::SideLeft ? flow::FloatingObject::Side::Left : s.side == Shape::SideRight ? flow::FloatingObject::Side::Right
                 : s.side == Shape::SideLargest ? flow::FloatingObject::Side::Largest : flow::FloatingObject::Side::Both;
        o.gap = QMarginsF(s.gapLeft, s.gapTop, s.gapRight, s.gapBottom);
        if (lettering) {
            if (!letteringFits(s)) continue;
            o.item = letteringItem(s);
        } else if (s.kind == Shape::Picture) {
            const int image = pictureOf(s);
            if (image < 0) continue;
            auto pic = std::make_shared<PictureItem>();
            pic->rect = QRectF(0, 0, s.width, s.height);
            dressPicture(*pic, s);
            if (shownMissing) showAsMissing(*pic);
            pic->altText = s.alt;
            o.item = pic;
            o.image = image;
        } else if (s.kind == Shape::Line) {
            auto line = std::make_shared<LineItem>();
            line->p1 = QPointF(s.x1, s.y1);
            line->p2 = QPointF(s.x2, s.y2);
            line->stroke = Stroke::line(colorOf(s.strokeColor), std::max(0.25, s.strokeWeight));
            if (s.arrowEnd) line->stroke.endArrow = Arrow::Triangle;
            if (s.arrowStart) line->stroke.startArrow = Arrow::Triangle;
            o.item = line;
        } else {
            std::shared_ptr<Item> item;
            if (s.kind == Shape::TextBox) {
                QVector<Block> blocks;
                bool read = readBlocks(s.textBox, &blocks, true);   // (its tables are laid out in the box, within the cells a document's boxes have in all)
                if (read) {
                    int cells = 0;
                    for (const Block &blk : blocks) cells += blk.isRaw && blk.role == Inline::Open && blk.tag == QLatin1String("tc");
                    if (m_boxTableCells + cells > kMostBoxTableCells) {
                        blocks.clear();
                        read = readBlocks(s.textBox, &blocks);
                    } else {
                        m_boxTableCells += cells;
                    }
                }
                if (read) {
                    auto story = std::make_shared<QTextDocument>();
                    story->setDocumentMargin(0);
                    story->setDefaultFont(this->story()->defaultFont());
                    story->setUndoRedoEnabled(false);
                    fillStory(story.get(), blocks, true, part);
                    o.story = story;
                }
                auto box = std::make_shared<TextItem>();
                box->rect = QRectF(0, 0, s.width, s.height);
                o.insets = QMarginsF(s.inset[0], s.inset[1], s.inset[2], s.inset[3]);
                item = box;
            } else {
                auto shape = std::make_shared<ShapeItem>();
                shape->rect = QRectF(0, 0, s.width, s.height);
                shapeOfKind(*shape, s);
                item = shape;
            }
            // (A shape that says it has no fill or no line has none: a shape's own are a color and a line that it was not given.)
            item->fill = s.filled && !s.fillColor.isEmpty() ? Fill::solid(colorOf(s.fillColor)) : Fill::none();
            item->stroke = s.stroked && s.strokeWeight > 0 ? Stroke::line(colorOf(s.strokeColor), s.strokeWeight) : Stroke::none();
            // (A picture that fills it is its fill, stretched over it: it keeps its text and its outline.)
            if (!s.fillEmbed.isEmpty()) {
                Shape filling = s;
                filling.embed = s.fillEmbed;
                filling.pictEmbed.clear();
                const int image = pictureOf(filling);
                if (image >= 0 && !shownMissing) o.fillImage = image;
            }
            item->rotation = s.rotation;
            item->flipH = s.flipH;
            item->flipV = s.flipV;
            o.item = item;
        }
        if (!o.item) continue;
        f.objects.push_back(o);
        const int id = int(f.objects.size());
        if (lastObject > 0) f.objects[lastObject - 1].next = id;
        else firstObject = id;
        lastObject = id;
        // The sides of a picture's border that are lines beside it (a picture set by offsets: one that is aligned is not drawn with them).
        if (s.kind == Shape::Picture && s.hAlign == Shape::NoAlign && s.vAlign == Shape::NoAlign)
            for (const auto &line : borderLines(s)) {
                if (f.objects.size() >= kMostObjects) break;
                flow::FloatingObject lo = o;
                lo.item = line;
                lo.image = flow::FloatingObject().image;
                lo.wrap = flow::FloatingObject::Wrap::None;
                f.objects.push_back(lo);
                const int lid = int(f.objects.size());
                f.objects[lastObject - 1].next = lid;
                lastObject = lid;
            }
    }
    if (firstObject > 0) format->setProperty(flow::fp::AnchorId, firstObject);
    if (firstObject > 0 || inlinePicture) format->setProperty(tp::Field, QStringLiteral("raw:"));   // (the anchor shows nothing of its own)
    return firstObject > 0 || inlinePicture;
}

} // namespace jp::write

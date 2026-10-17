// Filling the story from the body of a file: paragraphs and runs become blocks and formatted text, lists become Qt lists, and what is kept
// as XML becomes the pieces of writeprops.h.

#include "core/writedoc.h"

#include "core/tablemodel.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
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

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {

QString letters(int n, bool upper)
{
    QString s;
    for (n = std::max(1, n); n > 0; n = (n - 1) / 26) s.prepend(QChar((upper ? 'A' : 'a') + (n - 1) % 26));
    return s;
}

QString roman(int n, bool upper)
{
    static const std::pair<int, const char *> table[] = {{1000, "m"}, {900, "cm"}, {500, "d"}, {400, "cd"}, {100, "c"}, {90, "xc"}, {50, "l"}, {40, "xl"}, {10, "x"}, {9, "ix"}, {5, "v"}, {4, "iv"}, {1, "i"}};
    QString s;
    n = std::clamp(n, 1, 3999);
    for (const auto &t : table)
        while (n >= t.first) {
            s += QLatin1String(t.second);
            n -= t.first;
        }
    return upper ? s.toUpper() : s;
}

QString counterText(const QString &format, int n)
{
    if (format == QLatin1String("lowerLetter")) return letters(n, false);
    if (format == QLatin1String("upperLetter")) return letters(n, true);
    if (format == QLatin1String("lowerRoman")) return roman(n, false);
    if (format == QLatin1String("upperRoman")) return roman(n, true);
    if (format == QLatin1String("decimalZero")) return QStringLiteral("%1").arg(n, 2, 10, QLatin1Char('0'));
    return QString::number(n);
}

struct NumState {
    int count[9];
    QTextList *list[9];
    NumState()
    {
        for (int i = 0; i < 9; ++i) {
            count[i] = -1;
            list[i] = nullptr;
        }
    }
};

} // namespace

void WriteDoc::buildStory(const QVector<Block> &body) { fillStory(story(), body, false); }

// The most fields that nest in one another that a header or footer shows live (deeper ones show what they last read).
constexpr int kMostFieldNesting = 8;
// The most paragraphs of a header or footer that are read (the rest is left out).
constexpr int kMostHeaderParagraphs = 400;
// The most objects set at places that a file's pages have, and the most pictures (a file with more has the rest left out).
constexpr int kMostObjects = 20000;
constexpr int kMostImages = 2000;

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

// The text inside an element kept as written (an instruction's words).
QString textOfXml(const QByteArray &xml)
{
    QString text = QString::fromUtf8(xml);
    text.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
    return text.replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&quot;"), QLatin1String("\""))
        .replace(QLatin1String("&apos;"), QLatin1String("'")).replace(QLatin1String("&amp;"), QLatin1String("&"));
}

// The attribute of a start tag kept as written.
QString attributeOfTag(const QByteArray &xml, const QString &name)
{
    const QRegularExpression re(QStringLiteral("[\\s:]%1\\s*=\\s*\"([^\"]*)\"").arg(QRegularExpression::escape(name)));
    return re.match(QString::fromUtf8(xml.left(4000))).captured(1);
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

// Whether a paragraph shows anything of its own once what the tracked changes deleted is left out: text, or a picture, a break, a note's mark.
bool showsContent(const QVector<Inline> &content)
{
    int deleted = 0;
    for (const Inline &in : content) {
        if (in.kind == Inline::ParaAtom) {
            if (isDeletedRevision(in.tag)) deleted += in.role == Inline::Open ? 1 : in.role == Inline::Close ? -1 : 0;
            continue;
        }
        if (deleted > 0) continue;
        if (in.kind == Inline::Text) {
            if (!in.text.isEmpty()) return true;
        } else if (in.tag == QLatin1String("drawing") || in.tag == QLatin1String("pict") || in.tag == QLatin1String("object") || in.tag == QLatin1String("footnoteReference") ||
                   in.tag == QLatin1String("endnoteReference") || in.tag == QLatin1String("sym") || in.tag == QLatin1String("br") || in.tag == QLatin1String("ptab")) {
            return true;
        }
    }
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

} // namespace

// Fills `story` from the elements of a body or of a header or footer. A header's or footer's (`header`) fields for the page's number and the page
// counts are live fields of the text engine, its tables and pictures are left out, and a paragraph in a frame is set aligned as the frame is.
void WriteDoc::fillStory(QTextDocument *story, const QVector<Block> &body, bool header, const QString &part)
{
    const StyleSheet &sheet = m_wml.styles;
    const Theme *thm = theme();
    const QString defaultStyle = defaultParagraphStyle();

    QTextCursor c(story);
    c.beginEditBlock();
    bool firstBlock = true;
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
        RunProps resolved = sheet.run(paraStyle, props.style, layer);
        resolved.overlay(props);
        resolved.extra = props.extra;
        resolved.style = props.style;
        QTextCharFormat f = charFormatOf(resolved, thm, &props);
        if (!f.hasProperty(tp::Language))
            if (const QString language = styleLanguage(paraStyle, props.style); !language.isEmpty() && language != proofingLanguage()) f.setProperty(tp::Language, language);
        formats.insert(key, f);
        return f;
    };

    QHash<int, NumState> numbering;
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
    int paragraphsMade = 0, endnoteNumber = 0, blockNumber = -1;
    for (const Block &blk : body) {
        ++blockNumber;
        if (header && (blk.isRaw || ++paragraphsMade > kMostHeaderParagraphs)) continue;
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
        const AbstractNum *definition = numId > 0 ? m_wml.numbering.abstractOf(numId) : nullptr;
        if (!definition || !definition->levels[level].present) numId = 0;
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
        QTextBlockFormat bf = blockFormatOf(resolved);
        if (tables.key()) bf.setProperty(flow::fp::Cell, tables.key());
        // An empty paragraph that is a continuous section break takes no room.
        if (!header && blockNumber < continuousAfter.size() && continuousAfter[blockNumber] && p.content.isEmpty()) bf.setProperty(tp::OutOfFlow, true);
        if (header) {
            // A paragraph in a frame (the page number's, set to the side or the middle) is set that way, where the next paragraph begins, taking no room.
            for (const RawChild &ch : direct.extra) {
                if (!ch.xml.contains("framePr")) continue;
                const QString at = attributeOfTag(ch.xml, QStringLiteral("xAlign"));
                if (at == QLatin1String("center")) bf.setAlignment(Qt::AlignHCenter);
                else if (at == QLatin1String("right") || at == QLatin1String("outside")) bf.setAlignment(Qt::AlignRight);
                else if (at == QLatin1String("left") || at == QLatin1String("inside")) bf.setAlignment(Qt::AlignLeft);
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
        // A paragraph whose mark a tracked change deleted, with nothing else that shows, is gone from the text as it stands (and ends no section).
        if (!header && !direct.section && markIsDeleted(direct) && !showsContent(p.content)) markFormat = asDeleted(markFormat);

        // A list: the item's counter, and the marker where Qt's own counting can't make it.
        QTextListFormat lf;
        bool newList = false;
        QTextList *continuing = nullptr;
        if (numId > 0) {
            NumState &st = numbering[numId];
            const Level &l = definition->levels[level];
            const Num *num = m_wml.numbering.num(numId);
            auto startOf = [&](int lvl) { return num && num->startOverrides.contains(lvl) ? num->startOverrides.value(lvl) : definition->levels[lvl].start; };
            for (int i = level + 1; i < 9; ++i) {
                st.count[i] = -1;
                st.list[i] = nullptr;
            }
            st.count[level] = st.count[level] < 0 ? startOf(level) : st.count[level] + 1;
            lf = listFormat(numId, level);
            continuing = st.list[level];
            newList = !continuing;
            // Several levels in the marker, the legal style, or no number of its own: the marker is made here.
            const bool isBullet = l.numFmt == QLatin1String("bullet") || l.numFmt == QLatin1String("none");
            const int placeholders = l.text.count(QLatin1Char('%'));
            const bool own = l.text.contains(QStringLiteral("%%1").arg(level + 1));
            if (!isBullet && (l.legal || placeholders > 1 || !own)) {
                QString marker = l.text;
                for (int k = 8; k >= 0; --k) {
                    const QString token = QStringLiteral("%%1").arg(k + 1);
                    if (!marker.contains(token)) continue;
                    const Level &lk = definition->levels[k];
                    const int n = st.count[k] < 0 ? (lk.present ? startOf(k) : 1) : st.count[k];
                    marker.replace(token, counterText(l.legal ? QStringLiteral("decimal") : lk.numFmt, n));
                }
                bf.setProperty(tp::BulletChar, marker);
                lf.setStyle(QTextListFormat::ListDisc);
            }
        }

        startBlock(bf, markFormat);
        if (numId > 0) {
            NumState &st = numbering[numId];
            if (newList) st.list[level] = c.createList(lf);
            else continuing->add(c.block());
        }

        if (header) {
            // A field's instruction and result: the result (the text between its separator and its end) shows, or, for the page's number and the
            // page counts, one live field takes its place.
            struct Field {
                QString instruction;
                bool inResult = false;
                QTextCharFormat begin;
                QVector<std::pair<QString, QTextCharFormat>> result;
            };
            QVector<Field> fields;
            auto put = [&](const QString &text, const QTextCharFormat &format) {
                if (fields.isEmpty()) c.insertText(text, format);
                else if (fields.last().inResult) fields.last().result.push_back({text, format});
            };
            auto finish = [&] {
                const Field f = fields.takeLast();
                const QString live = liveFieldOf(f.instruction);
                if (!live.isEmpty()) {
                    QTextCharFormat ff = f.result.isEmpty() ? f.begin : f.result.first().second;
                    ff.setProperty(tp::Field, live);
                    put(QString(QChar::ObjectReplacementCharacter), ff);
                } else {
                    for (const auto &r : f.result) put(r.first, r.second);
                }
            };
            for (const Inline &in : p.content) {
                if (in.kind == Inline::Text) {
                    if (!in.text.isEmpty()) put(in.text, formatOf(styleId, in.props));
                } else if (in.kind == Inline::RunAtom && in.tag == QLatin1String("fldChar")) {
                    if (in.key == QLatin1String("begin") && fields.size() < kMostFieldNesting) {
                        fields.push_back(Field());
                        fields.last().begin = formatOf(styleId, in.props);
                    } else if (in.key == QLatin1String("separate") && !fields.isEmpty()) {
                        fields.last().inResult = true;
                    } else if (in.key == QLatin1String("end") && !fields.isEmpty()) {
                        finish();
                    }
                } else if (in.kind == Inline::RunAtom && (in.tag == QLatin1String("footnoteRef") || in.tag == QLatin1String("endnoteRef"))) {
                    // (The mark that begins a note's text shows the note's number: the pages give it.)
                    QTextCharFormat mark = formatOf(styleId, in.props);
                    mark.setProperty(tp::Field, QStringLiteral("notemark"));
                    put(QString(QChar::ObjectReplacementCharacter), mark);
                } else if (in.kind == Inline::RunAtom && drawnTag(in.tag)) {
                    QTextCharFormat object = formatOf(styleId, in.props);
                    if (drawnObjects(in, part, &object)) put(QString(QChar::ObjectReplacementCharacter), object);
                } else if (in.kind == Inline::RunAtom && in.tag == QLatin1String("instrText")) {
                    if (!fields.isEmpty() && !fields.last().inResult) fields.last().instruction += textOfXml(in.raw);
                } else if (in.kind == Inline::ParaAtom && in.tag == QLatin1String("fldSimple") && in.role == Inline::Open) {
                    if (fields.size() < kMostFieldNesting) {
                        fields.push_back(Field());
                        fields.last().instruction = attributeOfTag(in.raw, QStringLiteral("instr"));
                        fields.last().inResult = true;
                    }
                } else if (in.kind == Inline::ParaAtom && in.tag == QLatin1String("fldSimple") && in.role == Inline::Close) {
                    if (!fields.isEmpty()) finish();
                }
            }
            while (!fields.isEmpty()) finish();   // (a field the paragraph ended inside of shows what it read)
            continue;
        }
        int deletedDepth = 0;   // (inside the content a tracked change deleted)
        for (const Inline &in : p.content) {
            if (in.kind == Inline::ParaAtom && isDeletedRevision(in.tag)) deletedDepth = std::max(0, deletedDepth + (in.role == Inline::Open ? 1 : in.role == Inline::Close ? -1 : 0));
            const bool deleted = deletedDepth > 0 && !(in.kind == Inline::ParaAtom && in.role != Inline::Single && isDeletedRevision(in.tag));
            if (in.kind == Inline::Text) {
                if (!in.text.isEmpty()) c.insertText(in.text, deleted ? asDeleted(formatOf(styleId, in.props)) : formatOf(styleId, in.props));
                continue;
            }
            QTextCharFormat af = formatOf(styleId, in.props);
            if (deleted) af = asDeleted(af);
            setStored(af, prop::Atom, packAtom(in));
            // A reference to a footnote of the file is the note's (the pages lay it out at their foot); one to an endnote shows its number, in the lower
            // case roman numerals a word processor's endnotes have.
            QString field = QStringLiteral("raw:") + in.display;
            // A symbol of a font that isn't a symbol font (and isn't on the machine) is its character, in that font's stand-in.
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("sym")) {
                const QString font = attributeOfTag(in.raw, QStringLiteral("font")), lower = font.toLower();
                bool ok = false;
                const uint code = attributeOfTag(in.raw, QStringLiteral("char")).toUInt(&ok, 16);
                const bool symbols = lower == QLatin1String("symbol") || lower.contains(QLatin1String("dings")) || lower.contains(QLatin1String("marlett")) || lower.contains(QLatin1String("mt extra"));
                if (ok && !font.isEmpty() && !symbols && !QFontDatabase::hasFamily(font)) {
                    const uint ch = code >= 0xF000 && code <= 0xF0FF ? code - 0xF000 : code;
                    if (ch >= 0x20 && ch <= 0xFFFF) {
                        field = QStringLiteral("raw:") + QString(QChar(char16_t(ch)));
                        af.setProperty(tp::DisplayFamily, font);   // (the page's: the run as the file has it is kept as it is)
                    }
                }
            }
            if (in.kind == Inline::RunAtom && !deleted && drawnTag(in.tag) && drawnObjects(in, part, &af)) field = QStringLiteral("raw:");
            else if (in.tag == QLatin1String("footnoteReference") && m_noteBlocks.contains(QStringLiteral("fn") + in.key)) field = QStringLiteral("footnote:fn") + in.key;
            else if (in.tag == QLatin1String("endnoteReference")) field = QStringLiteral("raw:") + roman(++endnoteNumber, false);
            af.setProperty(tp::Field, field);
            c.insertText(QString(QChar::ObjectReplacementCharacter), af);
        }
        if (direct.section) {
            QTextBlockFormat last = c.blockFormat();
            last.setProperty(flow::fp::SectionBreakAfter, true);
            setStored(last, prop::SectionData, packSection(*direct.section));
            c.setBlockFormat(last);
        }
    }

    if (firstBlock) {
        // Nothing in the body: one empty paragraph in the default style.
        startBlock(styleBlockFormat(defaultStyle), styleCharFormat(defaultStyle));
    }
    if (header) {
        c.endEditBlock();
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
}

bool WriteDoc::drawnObjects(const Inline &atom, const QString &part, QTextCharFormat *format)
{
    const QVector<Shape> shapes = readShapes(atom.raw);
    if (shapes.isEmpty()) return false;
    flow::FlowDocument &f = *m_flow;
    // The picture of a shape: its bytes (from the part the run is in), kept once for the document.
    auto pictureOf = [&](const Shape &s) -> int {
        if (s.embed.isEmpty() || f.images.size() >= kMostImages) return -1;
        QString target;
        const QVector<Relationship> &links = part.isEmpty() ? m_wml.mainRelationships : m_wml.partRelationships.value(part);
        for (const Relationship &r : links)
            if (r.id == s.embed && !r.external) target = r.target;
        if (target.isEmpty()) return -1;
        const QByteArray bytes = partData(target);
        if (bytes.isEmpty()) return -1;
        for (int i = 0; i < f.images.size(); ++i)
            if (f.images[i].bytes == bytes) return i;
        flow::FlowImage image;
        image.bytes = bytes;
        image.format = QFileInfo(target).suffix().toLower();
        f.images.push_back(image);
        return int(f.images.size()) - 1;
    };
    auto colorOf = [](const QString &rgb) { return ColorRef::rgb(QColor(QLatin1Char('#') + rgb)); };
    int firstObject = 0, lastObject = 0;
    bool inlinePicture = false;
    for (const Shape &s : shapes) {
        if (s.kind == Shape::Picture && !s.floating && !inlinePicture) {
            const int image = pictureOf(s);
            if (image < 0 || s.width <= 0 || s.height <= 0) continue;
            // A picture in the line: an object set in the text, with no room round it of its own.
            auto pic = std::make_shared<PictureItem>();
            pic->rect = QRectF(0, 0, s.width, s.height);
            pic->imgRect = QRectF(0, 0, s.width, s.height);
            pic->imageId = QStringLiteral("flow-img:%1").arg(image);
            pic->wrap.left = pic->wrap.right = pic->wrap.top = pic->wrap.bottom = 0;
            pic->altText = s.alt;
            format->setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(pic->toJson()).toJson(QJsonDocument::Compact)));
            inlinePicture = true;
            continue;
        }
        if (f.objects.size() >= kMostObjects) break;
        flow::FloatingObject o;
        o.hRel = s.hRel == Shape::Margin ? flow::FloatingObject::Rel::Margin : s.hRel == Shape::Page ? flow::FloatingObject::Rel::Page
                 : s.hRel == Shape::Character ? flow::FloatingObject::Rel::Character : flow::FloatingObject::Rel::Column;
        o.vRel = s.vRel == Shape::Margin ? flow::FloatingObject::Rel::Margin : s.vRel == Shape::Page ? flow::FloatingObject::Rel::Page
                 : s.vRel == Shape::LineOf ? flow::FloatingObject::Rel::Line : s.vRel == Shape::Character ? flow::FloatingObject::Rel::Line : flow::FloatingObject::Rel::Paragraph;
        auto align = [](Shape::Align a) { return a == Shape::Start ? flow::FloatingObject::Align::Start : a == Shape::Center ? flow::FloatingObject::Align::Center
                                                                : a == Shape::End ? flow::FloatingObject::Align::End : flow::FloatingObject::Align::None; };
        o.hAlign = align(s.hAlign);
        o.vAlign = align(s.vAlign);
        o.dx = s.left;
        o.dy = s.top;
        o.behind = s.behind;
        o.wrap = s.wrap == Shape::Square || s.wrap == Shape::Tight ? flow::FloatingObject::Wrap::Square : s.wrap == Shape::TopBottom ? flow::FloatingObject::Wrap::TopBottom : flow::FloatingObject::Wrap::None;
        o.gap = QMarginsF(s.gapLeft, s.gapTop, s.gapRight, s.gapBottom);
        if (s.kind == Shape::Picture) {
            const int image = pictureOf(s);
            if (image < 0) continue;
            auto pic = std::make_shared<PictureItem>();
            pic->rect = QRectF(0, 0, s.width, s.height);
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
                if (readBlocks(s.textBox, &blocks)) {
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
                shape->shape = s.kind == Shape::Oval ? QStringLiteral("ellipse") : QStringLiteral("rect");
                item = shape;
            }
            if (s.filled && !s.fillColor.isEmpty()) item->fill = Fill::solid(colorOf(s.fillColor));
            if (s.stroked && s.strokeWeight > 0) item->stroke = Stroke::line(colorOf(s.strokeColor), s.strokeWeight);
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
    }
    if (firstObject > 0) format->setProperty(flow::fp::AnchorId, firstObject);
    if (firstObject > 0 || inlinePicture) format->setProperty(tp::Field, QStringLiteral("raw:"));   // (the anchor shows nothing of its own)
    return firstObject > 0 || inlinePicture;
}

} // namespace jp::write

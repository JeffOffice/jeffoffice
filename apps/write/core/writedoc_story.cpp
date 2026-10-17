// Filling the story from the body of a file: paragraphs and runs become blocks and formatted text, lists become Qt lists, and what is kept
// as XML becomes the pieces of writeprops.h.

#include "core/writedoc.h"

#include "core/drawnitems.h"
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

// Whether a note's reference says its mark follows it as text, in the run (w:customMarkFollows): it shows no number of its own and takes none.
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
    o.hAlign = xAlign == QLatin1String("center") ? FO::Align::Center : xAlign == QLatin1String("right") || xAlign == QLatin1String("outside") ? FO::Align::End
               : xAlign == QLatin1String("left") || xAlign == QLatin1String("inside") ? FO::Align::Start : FO::Align::None;
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
    int paragraphsMade = 0, endnoteNumber = 0, blockNumber = -1;
    // The frame of the paragraph before (its properties as written, and its key): the paragraphs next to one another that have the same properties are one frame.
    QByteArray lastFrameXml;
    int lastFrameKey = 0;
    for (const Block &blk : body) {
        ++blockNumber;
        if (header && (blk.isRaw || ++paragraphsMade > kMostHeaderParagraphs)) continue;
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
        // A paragraph of a frame in the body is set apart from the text: no lines and no room there, and a text box where the frame asks, at the paragraph after it.
        if (!header && !tables.key()) {
            QByteArray frameXml;
            const QHash<QString, QString> frameAt = frameOfParagraph(sheet, styleId, direct.extra, &frameXml, &styleFrames);
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
        // A paragraph whose mark a tracked change deleted is joined to the next as the text stands (and ends no section).
        if (!header && !direct.section && markIsDeleted(direct)) markFormat = asDeleted(markFormat);

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
        double advanceTop = -1;   // (a field that sets the text after it at a place from the page's top, ADVANCE with \\y: the paragraph is no higher than that)
        for (const Inline &in : p.content) {
            if (in.kind == Inline::ParaAtom && isDeletedRevision(in.tag)) deletedDepth = std::max(0, deletedDepth + (in.role == Inline::Open ? 1 : in.role == Inline::Close ? -1 : 0));
            const bool deleted = deletedDepth > 0 && !(in.kind == Inline::ParaAtom && in.role != Inline::Single && isDeletedRevision(in.tag));
            if (in.kind == Inline::Text) {
                if (!in.text.isEmpty()) c.insertText(in.text, deleted ? asDeleted(formatOf(styleId, in.props)) : formatOf(styleId, in.props));
                continue;
            }
            if (!deleted && advanceTop < 0)
                if (const std::optional<double> asked = advanceTopOf(in)) advanceTop = *asked;
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
            if (in.kind == Inline::RunAtom && !deleted && drawnTag(in.tag) && drawnObjects(in, part, &af)) field = QStringLiteral("raw:");
            else if (in.tag == QLatin1String("footnoteReference") && m_noteBlocks.contains(QStringLiteral("fn") + in.key)) {
                field = QStringLiteral("footnote:fn") + in.key;
                if (markFollows(in.raw)) af.setProperty(tp::NoteMarkCustom, true);
            }
            else if (in.tag == QLatin1String("endnoteReference")) field = QStringLiteral("raw:") + (markFollows(in.raw) ? QString() : roman(deleted ? endnoteNumber + 1 : ++endnoteNumber, false));   // (a deleted one takes no number)
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
        // Lettering on a path and a horizontal rule that are not set at a place are set in the line, as a picture is.
        if (!s.floating && !inlinePicture && (s.rule || !s.pathText.isEmpty())) {
            const ItemPtr set = s.rule ? ruleItem(s, std::max(1.0, (m_last.pageW - m_last.left - m_last.right) / 20.0)) : letteringItem(s);
            if (set) {
                if (!s.rule) set->wrap.left = set->wrap.right = set->wrap.top = set->wrap.bottom = 0;
                format->setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(set->toJson()).toJson(QJsonDocument::Compact)));
                inlinePicture = true;
                continue;
            }
        }
        if (s.kind == Shape::Picture && !s.floating && !inlinePicture) {
            const int image = pictureOf(s);
            if (image < 0 || s.width <= 0 || s.height <= 0) continue;
            // A picture in the line: an object set in the text, with no room round it of its own.
            auto pic = std::make_shared<PictureItem>();
            pic->rect = QRectF(0, 0, s.width, s.height);
            dressPicture(*pic, s);
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
                                                                : a == Shape::End ? flow::FloatingObject::Align::End : flow::FloatingObject::Align::None; };
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
        if (ItemPtr lettering = letteringItem(s)) {
            o.item = lettering;
        } else if (s.kind == Shape::Picture) {
            const int image = pictureOf(s);
            if (image < 0) continue;
            auto pic = std::make_shared<PictureItem>();
            pic->rect = QRectF(0, 0, s.width, s.height);
            dressPicture(*pic, s);
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
                shape->shape = presetOf(s);
                item = shape;
            }
            // (A shape that says it has no fill or no line has none: a shape's own are a color and a line that it was not given.)
            item->fill = s.filled && !s.fillColor.isEmpty() ? Fill::solid(colorOf(s.fillColor)) : Fill::none();
            item->stroke = s.stroked && s.strokeWeight > 0 ? Stroke::line(colorOf(s.strokeColor), s.strokeWeight) : Stroke::none();
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

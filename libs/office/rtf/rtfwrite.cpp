#include "rtf/rtfwrite.h"

#include "office/theme.h"

#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>

namespace jp::office::rtf {

using namespace wml;

namespace {

constexpr int kMostDepth = 8;   // tables inside tables are written down to here; deeper ones are written as their paragraphs


// ---- what a row or a cell says, from its start tag and properties as the .docx writer keeps them (a block of a table that was edited has no parsed marks)
QString attrIn(const QString &xml, const char *element, const char *attr)
{
    const QRegularExpression e(QStringLiteral("<w:%1\\b([^>]*?)/?>").arg(QLatin1String(element)));
    const QRegularExpressionMatch m = e.match(xml);
    if (!m.hasMatch()) return QString();
    const QRegularExpression a(QStringLiteral("\\bw:%1\\s*=\\s*\"([^\"]*)\"").arg(QLatin1String(attr)));
    return a.match(m.captured(1)).captured(1);
}

bool hasElement(const QString &xml, const char *element, bool defaultOn = true)
{
    const QRegularExpression e(QStringLiteral("<w:%1\\b([^>]*?)/?>").arg(QLatin1String(element)));
    const QRegularExpressionMatch m = e.match(xml);
    if (!m.hasMatch()) return false;
    const QString v = attrIn(xml, element, "val");
    return defaultOn && (v.isEmpty() || (v != QLatin1String("0") && v != QLatin1String("false") && v != QLatin1String("off")));
}

TableBorder borderIn(const QString &xml, const char *group, const char *side)
{
    const QRegularExpression g(QStringLiteral("<w:%1\\b[^>]*>(.*?)</w:%1>").arg(QLatin1String(group)), QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch m = g.match(xml);
    TableBorder b;
    if (!m.hasMatch()) return b;
    const QString inner = m.captured(1);
    b.val = attrIn(inner, side, "val");
    b.size = attrIn(inner, side, "sz").toInt();
    b.space = attrIn(inner, side, "space").toInt();
    b.color = attrIn(inner, side, "color");
    return b;
}

TableMark rowMarkOf(const Block &b)
{
    if (b.table) return *b.table;
    TableMark m;
    m.kind = TableMark::Row;
    const QString xml = QString::fromUtf8(b.raw);
    m.height = attrIn(xml, "trHeight", "val").toInt();
    m.heightRule = attrIn(xml, "trHeight", "hRule");
    m.header = hasElement(xml, "tblHeader");
    m.cantSplit = hasElement(xml, "cantSplit");
    m.gridBefore = attrIn(xml, "gridBefore", "val").toInt();
    return m;
}

TableMark cellMarkOf(const Block &b)
{
    if (b.table) return *b.table;
    TableMark m;
    m.kind = TableMark::Cell;
    const QString xml = QString::fromUtf8(b.raw);
    m.span = std::max(1, attrIn(xml, "gridSpan", "val").toInt());
    if (xml.contains(QLatin1String("<w:vMerge"))) m.vMerge = attrIn(xml, "vMerge", "val") == QLatin1String("restart") ? QStringLiteral("restart") : QStringLiteral("continue");
    m.vAlign = attrIn(xml, "vAlign", "val");
    m.shdFill = attrIn(xml, "shd", "fill");
    m.cellWidth.type = attrIn(xml, "tcW", "type");
    m.cellWidth.w = attrIn(xml, "tcW", "w").toInt();
    m.top = borderIn(xml, "tcBorders", "top");
    m.left = borderIn(xml, "tcBorders", "left");
    m.bottom = borderIn(xml, "tcBorders", "bottom");
    m.right = borderIn(xml, "tcBorders", "right");
    return m;
}

struct Writer {
    const Document &doc;
    QStringList lost;
    QSet<QString> lostSeen;
    QVector<QString> fonts;             // the font table, by number
    QHash<QString, int> fontIndex;
    QVector<QString> colors;            // "RRGGBB", by number - 1
    QHash<QString, int> colorIndex;
    QHash<QString, int> styleNumber;    // style id -> the number it has in the file
    QHash<int, int> listNumber;         // numId -> \ls
    QVector<int> listOrder;             // the numIds, in the order of the list override table
    QByteArray pending;                 // the text of the body under construction

    explicit Writer(const Document &d) : doc(d) {}

    void note(const QString &what)
    {
        if (lostSeen.contains(what)) return;
        lostSeen.insert(what);
        lost << what;
    }

    // ---- text
    static QByteArray text(const QString &s)
    {
        QByteArray out;
        out.reserve(s.size());
        for (const QChar c : s) {
            const ushort u = c.unicode();
            if (u == '\\' || u == '{' || u == '}') {
                out += '\\';
                out += char(u);
            } else if (u == '\t') {
                out += "\\tab ";
            } else if (u == 0x2028 || u == '\n') {
                out += "\\line ";
            } else if (u == 0x0C) {
                out += "\\page ";
            } else if (u == 0x0E) {
                out += "\\column ";
            } else if (u == 0x2011) {
                out += "\\_";
            } else if (u == 0xAD) {
                out += "\\-";
            } else if (u == 0xA0) {
                out += "\\~";
            } else if (u < 0x20) {
                continue;
            } else if (u < 0x80) {
                out += char(u);
            } else {
                out += "\\u" + QByteArray::number(int(qint16(u))) + "?";
            }
        }
        return out;
    }

    // ---- tables of fonts and colors
    QString faceOf(const Fonts &f) const
    {
        auto theme = [&](const QString &name) -> QString {
            if (!doc.hasTheme) return QString();
            if (name.startsWith(QLatin1String("major"))) return doc.theme.majorLatin;
            if (name.startsWith(QLatin1String("minor"))) return doc.theme.minorLatin;
            return QString();
        };
        if (!f.ascii.isEmpty()) return f.ascii;
        if (!f.hAnsi.isEmpty()) return f.hAnsi;
        if (const QString t = theme(f.asciiTheme); !t.isEmpty()) return t;
        return theme(f.hAnsiTheme);
    }
    int font(const QString &face)
    {
        if (face.isEmpty()) return -1;
        const auto it = fontIndex.constFind(face);
        if (it != fontIndex.constEnd()) return it.value();
        if (fonts.size() >= kMaxFonts) return -1;
        fontIndex.insert(face, int(fonts.size()));
        fonts << face;
        return int(fonts.size()) - 1;
    }
    int color(const QString &rgb)
    {
        static const QRegularExpression hex(QStringLiteral("^[0-9A-Fa-f]{6}$"));
        if (!hex.match(rgb).hasMatch()) return 0;
        const QString key = rgb.toUpper();
        const auto it = colorIndex.constFind(key);
        if (it != colorIndex.constEnd()) return it.value();
        if (colors.size() >= 4000) return 0;
        colors << key;
        colorIndex.insert(key, int(colors.size()));
        return int(colors.size());
    }

    // ---- run and paragraph looks
    static QByteArray flag(const char *word, const std::optional<bool> &v)
    {
        if (!v) return QByteArray();
        return QByteArray("\\") + word + (*v ? "" : "0");
    }
    QByteArray run(const RunProps &p, bool inStyle = false)
    {
        QByteArray o;
        if (p.fonts) {
            const int f = font(faceOf(*p.fonts));
            if (f >= 0) o += "\\f" + QByteArray::number(f);
        }
        o += flag("b", p.bold) + flag("i", p.italic) + flag("caps", p.caps) + flag("scaps", p.smallCaps) + flag("strike", p.strike) + flag("v", p.vanish);
        if (p.dstrike) o += *p.dstrike ? "\\striked1" : "\\striked0";
        if (p.underline) {
            static const QHash<QString, QByteArray> kinds = {{"single", "\\ul"}, {"double", "\\uldb"}, {"thick", "\\ulth"}, {"dotted", "\\uld"}, {"dash", "\\uldash"}, {"dotDash", "\\uldashd"},
                                                             {"dotDotDash", "\\uldashdd"}, {"wave", "\\ulwave"}, {"words", "\\ulw"}, {"none", "\\ulnone"}};
            o += kinds.value(p.underline->val, QByteArray("\\ul"));
            if (!p.underline->color.isEmpty() && p.underline->color != QLatin1String("auto")) o += "\\ulc" + QByteArray::number(color(p.underline->color));
        }
        if (p.color && !p.color->val.isEmpty()) o += "\\cf" + QByteArray::number(p.color->val == QLatin1String("auto") ? 0 : color(p.color->val));
        if (p.size) o += "\\fs" + QByteArray::number(*p.size);
        if (p.spacing) o += "\\expndtw" + QByteArray::number(*p.spacing);
        if (p.position) o += (*p.position >= 0 ? "\\up" : "\\dn") + QByteArray::number(std::abs(*p.position));
        if (p.vertAlign == QLatin1String("superscript")) o += "\\super";
        else if (p.vertAlign == QLatin1String("subscript")) o += "\\sub";
        else if (p.vertAlign == QLatin1String("baseline")) o += "\\nosupersub";
        if (!p.highlight.isEmpty()) {
            static const QHash<QString, QString> names = {{"yellow", "FFFF00"},   {"green", "00FF00"},    {"cyan", "00FFFF"},      {"magenta", "FF00FF"}, {"blue", "0000FF"},     {"red", "FF0000"},
                                                          {"darkBlue", "000080"}, {"darkCyan", "008080"}, {"darkGreen", "008000"}, {"darkMagenta", "800080"}, {"darkRed", "800000"}, {"darkYellow", "808000"},
                                                          {"darkGray", "808080"}, {"lightGray", "C0C0C0"}, {"black", "000000"},    {"white", "FFFFFF"}};
            if (p.highlight == QLatin1String("none")) o += "\\highlight0";
            else if (names.contains(p.highlight)) o += "\\highlight" + QByteArray::number(color(names.value(p.highlight)));
        }
        Q_UNUSED(inStyle);
        if (!o.isEmpty()) o += ' ';
        return o;
    }

    QByteArray border(const char *side, const Border &b)
    {
        if (!b.visible()) return QByteArray();
        const QString v = b.val;
        const char *kind = v == QLatin1String("double") ? "\\brdrdb" : v == QLatin1String("dotted") ? "\\brdrdot" : v == QLatin1String("dashed") ? "\\brdrdash" : v == QLatin1String("thick") ? "\\brdrth" : "\\brdrs";
        QByteArray o = QByteArray("\\brdr") + side + kind + "\\brdrw" + QByteArray::number(std::clamp(b.size * 20 / 8, 5, 75)) + "\\brsp" + QByteArray::number(std::clamp(b.space * 20, 0, 31680));
        if (!b.color.isEmpty() && b.color != QLatin1String("auto")) o += "\\brdrcf" + QByteArray::number(color(b.color));
        return o;
    }

    QByteArray paragraph(const ParaProps &p)
    {
        QByteArray o;
        if (p.jc == QLatin1String("center")) o += "\\qc";
        else if (p.jc == QLatin1String("right") || p.jc == QLatin1String("end")) o += "\\qr";
        else if (p.jc == QLatin1String("both")) o += "\\qj";
        else if (p.jc == QLatin1String("distribute")) o += "\\qd";
        else if (p.jc == QLatin1String("left") || p.jc == QLatin1String("start")) o += "\\ql";
        if (p.indLeft) o += "\\li" + QByteArray::number(*p.indLeft);
        if (p.indRight) o += "\\ri" + QByteArray::number(*p.indRight);
        if (p.indFirst) o += "\\fi" + QByteArray::number(*p.indFirst);
        if (p.before) o += "\\sb" + QByteArray::number(*p.before);
        if (p.after) o += "\\sa" + QByteArray::number(*p.after);
        if (p.line) {
            const bool exact = p.lineRule == QLatin1String("exact");
            o += "\\sl" + QByteArray::number(exact ? -*p.line : *p.line) + (p.lineRule == QLatin1String("exact") || p.lineRule == QLatin1String("atLeast") ? "\\slmult0" : "\\slmult1");
        }
        if (p.keepNext && *p.keepNext) o += "\\keepn";
        if (p.keepLines && *p.keepLines) o += "\\keep";
        if (p.pageBreakBefore && *p.pageBreakBefore) o += "\\pagebb";
        if (p.widowControl) o += *p.widowControl ? "\\widctlpar" : "\\nowidctlpar";
        if (p.outlineLvl && *p.outlineLvl < 9) o += "\\outlinelevel" + QByteArray::number(*p.outlineLvl);
        if (p.tabs)
            for (const TabStop &t : *p.tabs) {
                if (t.val == QLatin1String("clear")) continue;
                if (t.leader == QLatin1String("dot")) o += "\\tldot";
                else if (t.leader == QLatin1String("hyphen")) o += "\\tlhyph";
                else if (t.leader == QLatin1String("underscore")) o += "\\tlul";
                o += t.val == QLatin1String("center") ? "\\tqc" : t.val == QLatin1String("right") ? "\\tqr" : t.val == QLatin1String("decimal") ? "\\tqdec" : t.val == QLatin1String("bar") ? "\\tb" : "";
                o += (t.val == QLatin1String("bar") ? "" : "\\tx") + QByteArray::number(t.pos);
            }
        if (p.borders) {
            o += border("t", p.borders->top ? *p.borders->top : Border()) + border("l", p.borders->left ? *p.borders->left : Border()) + border("b", p.borders->bottom ? *p.borders->bottom : Border()) +
                 border("r", p.borders->right ? *p.borders->right : Border());
        }
        if (p.shading && !p.shading->fill.isEmpty() && p.shading->fill != QLatin1String("auto")) o += "\\cbpat" + QByteArray::number(color(p.shading->fill));
        if (!o.isEmpty()) o += ' ';
        return o;
    }

    // ---- styles, lists
    QByteArray styleTable()
    {
        QByteArray o;
        int next = 1;
        bool haveDefault = false;
        for (const Style &s : doc.styles.styles) {
            if (s.type != QLatin1String("paragraph") && s.type != QLatin1String("character")) continue;
            if (s.type == QLatin1String("paragraph") && s.isDefault && !haveDefault) {
                styleNumber.insert(s.id, 0);
                haveDefault = true;
            } else {
                styleNumber.insert(s.id, next++);
            }
        }
        // The text the document has when nothing is said is its defaults: the first font is the file's default (\deff0), and the style numbered 0 carries the rest.
        font(faceOf(doc.styles.defaultRun.fonts.value_or(Fonts())));
        if (!haveDefault) {
            RunProps r = doc.styles.defaultRun;
            o += "{\\s0" + paragraph(doc.styles.defaultPara) + run(r, true) + "Normal;}\n";
        }
        for (const Style &s : doc.styles.styles) {
            if (!styleNumber.contains(s.id)) continue;
            const int n = styleNumber.value(s.id);
            const bool character = s.type == QLatin1String("character");
            RunProps r = s.rPr;
            ParaProps pp = s.pPr;
            if (n == 0) {
                r = doc.styles.defaultRun;
                r.overlay(s.rPr);
                pp = doc.styles.defaultPara;
                pp.overlay(s.pPr);
            }
            o += "{";
            if (character) o += "\\*\\cs" + QByteArray::number(n);
            else o += "\\s" + QByteArray::number(n);
            if (!s.basedOn.isEmpty() && styleNumber.contains(s.basedOn)) o += "\\sbasedon" + QByteArray::number(styleNumber.value(s.basedOn));
            else if (!character && n != 0) o += "\\sbasedon0";
            if (!character && !s.next.isEmpty() && styleNumber.contains(s.next)) o += "\\snext" + QByteArray::number(styleNumber.value(s.next));
            if (s.qFormat) o += "\\sqformat";
            if (s.semiHidden) o += "\\shidden";
            if (!character) o += paragraph(pp);
            o += run(r, true);
            o += text(s.name) + ";}\n";
        }
        return o;
    }

    static QByteArray numberFormat(const QString &f)
    {
        if (f == QLatin1String("bullet")) return "23";
        if (f == QLatin1String("lowerRoman")) return "2";
        if (f == QLatin1String("upperRoman")) return "1";
        if (f == QLatin1String("lowerLetter")) return "4";
        if (f == QLatin1String("upperLetter")) return "3";
        if (f == QLatin1String("ordinal")) return "5";
        if (f == QLatin1String("decimalZero")) return "22";
        if (f == QLatin1String("none")) return "255";
        return "0";
    }

    QByteArray listTables()
    {
        QSet<int> usedAbstract;
        QByteArray lists, overrides;
        for (const int numId : listOrder) {
            const Num *num = doc.numbering.num(numId);
            const AbstractNum *abs = doc.numbering.abstractOf(numId);
            if (!num || !abs) continue;
            const int def = doc.numbering.definitionOf(numId);
            const int id = 1000 + def;
            if (!usedAbstract.contains(def)) {
                usedAbstract.insert(def);
                lists += "{\\list\\listtemplateid" + QByteArray::number(id) + "\n";
                for (int l = 0; l < 9; ++l) {
                    const Level *lv = doc.numbering.levelOf(numId, l);
                    if (!lv) continue;
                    // The level's text as the format has it: a length byte, then each character; a place for a level's number is that level's number (from 0), and \levelnumbers
                    // says where in the text those places are (from 1).
                    QByteArray places, textBytes;
                    qsizetype count = 0;
                    auto add = [&](int byte) {
                        textBytes += QByteArray("\\'") + QByteArray::number(byte, 16).rightJustified(2, '0');
                        ++count;
                    };
                    for (qsizetype i = 0; i < lv->text.size(); ++i) {
                        const QChar c = lv->text[i];
                        if (c == QLatin1Char('%') && i + 1 < lv->text.size() && lv->text[i + 1].isDigit()) {
                            places += QByteArray("\\'") + QByteArray::number(int(count) + 1, 16).rightJustified(2, '0');
                            add(std::clamp(lv->text[i + 1].digitValue() - 1, 0, 8));
                            ++i;
                        } else {
                            // (a bullet in the symbol font is the character after U+F000)
                            add(c.unicode() >= 0xF000 && c.unicode() <= 0xF0FF ? int(c.unicode() & 0xFF) : c.unicode() < 256 ? int(c.unicode()) : 63);
                        }
                    }
                    lists += "{\\listlevel\\levelnfc" + numberFormat(lv->numFmt) + "\\leveljc" + (lv->jc == QLatin1String("right") ? "2" : lv->jc == QLatin1String("center") ? "1" : "0") + "\\levelstartat" +
                             QByteArray::number(lv->start) + "\\levelfollow" + (lv->suff == QLatin1String("space") ? "1" : lv->suff == QLatin1String("nothing") ? "2" : "0") + "{\\leveltext\\'" +
                             QByteArray::number(int(count), 16).rightJustified(2, '0') + textBytes + ";}{\\levelnumbers" + places + ";}";
                    if (lv->pPr.indFirst) lists += "\\fi" + QByteArray::number(*lv->pPr.indFirst);
                    if (lv->pPr.indLeft) lists += "\\li" + QByteArray::number(*lv->pPr.indLeft);
                    lists += run(lv->rPr, true);
                    lists += "}\n";
                }
                lists += "\\listid" + QByteArray::number(id) + "}\n";
            }
            overrides += "{\\listoverride\\listid" + QByteArray::number(id) + "\\listoverridecount0\\ls" + QByteArray::number(listNumber.value(numId)) + "}\n";
        }
        if (lists.isEmpty()) return QByteArray();
        return "{\\*\\listtable\n" + lists + "}\n{\\*\\listoverridetable\n" + overrides + "}\n";
    }

    // ---- a picture of a run
    bool picture(const QByteArray &raw, QByteArray *out)
    {
        static const QRegularExpression embed(QStringLiteral("r:embed\\s*=\\s*\"([^\"]+)\"")), extent(QStringLiteral("<wp:extent\\s+cx\\s*=\\s*\"(\\d+)\"\\s+cy\\s*=\\s*\"(\\d+)\""));
        const QString xml = QString::fromUtf8(raw);
        const QRegularExpressionMatch e = embed.match(xml), x = extent.match(xml);
        if (!e.hasMatch()) return false;
        QString target;
        for (const Relationship &r : doc.mainRelationships)
            if (r.id == e.captured(1) && !r.external) target = r.target;
        const KeptPart *part = nullptr;
        for (const KeptPart &p : doc.parts)
            if (p.name.compare(target, Qt::CaseInsensitive) == 0 || p.name.endsWith(QLatin1Char('/') + target, Qt::CaseInsensitive)) part = &p;
        if (!part || part->data.isEmpty()) return false;
        const QByteArray &d = part->data;
        const char *kind = d.startsWith("\x89PNG") ? "\\pngblip" : d.startsWith("\xFF\xD8") ? "\\jpegblip" : (d.size() > 44 && d.mid(40, 4) == " EMF") ? "\\emfblip" : nullptr;
        if (!kind) {
            note(QStringLiteral("pictures that are not PNG, JPEG, or EMF"));
            return true;
        }
        const qint64 cx = x.hasMatch() ? x.captured(1).toLongLong() : 0, cy = x.hasMatch() ? x.captured(2).toLongLong() : 0;
        const int w = int(std::clamp<qint64>(cx / 635, 20, 31680)), h = int(std::clamp<qint64>(cy / 635, 20, 31680));
        QByteArray o = "{\\pict" + QByteArray(kind) + "\\picw" + QByteArray::number(w / 15) + "\\pich" + QByteArray::number(h / 15) + "\\picwgoal" + QByteArray::number(w) + "\\pichgoal" +
                       QByteArray::number(h) + "\n";
        const QByteArray hex = d.toHex();
        for (qsizetype i = 0; i < hex.size(); i += 128) o += hex.mid(i, 128) + "\n";
        o += "}";
        *out = o;
        return true;
    }

    // The instruction of a field from the runs that make it (the text of each w:instrText, joined).
    static QString instructionOf(const QByteArray &raw)
    {
        static const QRegularExpression instr(QStringLiteral("<w:instrText[^>]*>([^<]*)</w:instrText>"));
        QString code;
        for (auto it = instr.globalMatch(QString::fromUtf8(raw)); it.hasNext();) code += it.next().captured(1);
        code.replace(QStringLiteral("&amp;"), QStringLiteral("&")).replace(QStringLiteral("&lt;"), QStringLiteral("<")).replace(QStringLiteral("&gt;"), QStringLiteral(">")).replace(QStringLiteral("&quot;"), QStringLiteral("\""));
        return code.trimmed();
    }

    // ---- paragraphs
    // A group of text with its looks (and the character style it has).
    QByteArray group(const RunProps &props, const QString &words)
    {
        QByteArray o = "{";
        if (!props.style.isEmpty() && styleNumber.contains(props.style)) o += "\\cs" + QByteArray::number(styleNumber.value(props.style));
        return o + run(props) + (o.size() > 1 && run(props).isEmpty() ? " " : "") + text(words) + "}";
    }

    QByteArray inlines(const Paragraph &p)
    {
        QByteArray o;
        QVector<bool> links;   // for each open link, whether it is a field (closed with one more brace)
        struct LooseField {
            QString code;
            bool inResult = false;
        };
        QVector<LooseField> fields;   // the fields whose marks are separate pieces of the paragraph
        for (const Inline &in : p.content) {
            if (in.kind == Inline::Text) {
                if (!in.text.isEmpty()) o += group(in.props, in.text);
                continue;
            }
            if (in.kind == Inline::ParaAtom) {
                if (in.tag == QLatin1String("hyperlink")) {
                    if (in.role == Inline::Open) {
                        static const QRegularExpression rid(QStringLiteral("r:id\\s*=\\s*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption), anchor(QStringLiteral("anchor\\s*=\\s*\"([^\"]+)\""));
                        const QString tag = QString::fromUtf8(in.raw);
                        QString link;
                        if (const auto m = rid.match(tag); m.hasMatch())
                            for (const Relationship &r : doc.mainRelationships)
                                if (r.id == m.captured(1)) link = r.target;
                        if (link.isEmpty())
                            if (const auto m = anchor.match(tag); m.hasMatch()) link = QStringLiteral("#") + m.captured(1);
                        if (!link.isEmpty()) o += "{\\field{\\*\\fldinst HYPERLINK \"" + text(link) + "\"}{\\fldrslt ";
                        else o += "{";
                        links << !link.isEmpty();
                    } else if (in.role == Inline::Close && !links.isEmpty()) {
                        o += links.takeLast() ? "}}" : "}";
                    }
                    continue;
                }
                if (in.tag == QLatin1String(kFieldGroupTag.data())) {
                    const QString code = instructionOf(in.raw);
                    if (!code.isEmpty()) o += "{\\field{\\*\\fldinst " + text(code) + "}{\\fldrslt " + (in.display.isEmpty() ? QByteArray() : group(in.props, in.display)) + "}}";
                    else if (!in.display.isEmpty()) o += group(in.props, in.display);
                    continue;
                }
                if (in.tag == QLatin1String("bookmarkStart") || in.tag == QLatin1String("bookmarkEnd") || in.tag == QLatin1String("proofErr")) continue;
                if (in.tag == QLatin1String("commentRangeStart") || in.tag == QLatin1String("commentRangeEnd")) {
                    note(QStringLiteral("comments"));
                    continue;
                }
                if (in.tag == QLatin1String("ins") || in.tag == QLatin1String("del") || in.tag == QLatin1String("moveFrom") || in.tag == QLatin1String("moveTo")) {
                    note(QStringLiteral("tracked changes (the text is kept as it stands)"));
                    continue;
                }
                if (!in.display.isEmpty()) o += group(in.props, in.display);
                continue;
            }
            // a RunAtom: a picture, a field mark, a note reference...
            if (in.raw.contains("w:fldChar")) {
                static const QRegularExpression kind(QStringLiteral("fldCharType\\s*=\\s*\"(\\w+)\""));
                const QString type = kind.match(QString::fromUtf8(in.raw)).captured(1);
                if (type == QLatin1String("begin")) {
                    fields.push_back({});
                } else if (type == QLatin1String("separate") && !fields.isEmpty()) {
                    o += "{\\field{\\*\\fldinst " + text(fields.last().code.trimmed()) + "}{\\fldrslt ";
                    fields.last().inResult = true;
                } else if (type == QLatin1String("end") && !fields.isEmpty()) {
                    const LooseField f = fields.takeLast();
                    o += f.inResult ? QByteArray("}}") : "{\\field{\\*\\fldinst " + text(f.code.trimmed()) + "}}";
                }
                continue;
            }
            if (in.raw.contains("instrText")) {
                if (!fields.isEmpty()) fields.last().code += instructionOf(in.raw);
                continue;
            }
            if (in.raw.contains("<w:drawing") || in.raw.contains("<w:pict")) {
                QByteArray pic;
                if (picture(in.raw, &pic)) o += "{" + pic + "}";
                else note(QStringLiteral("drawings, shapes, and text boxes"));
                continue;
            }
            if (in.raw.contains("footnoteReference") || in.raw.contains("endnoteReference")) {
                note(QStringLiteral("footnotes and endnotes"));
                continue;
            }
            if (in.raw.contains("commentReference")) {
                note(QStringLiteral("comments"));
                continue;
            }
            if (in.raw.contains("oMath")) note(QStringLiteral("equations (their text is kept)"));
            if (!in.display.isEmpty()) o += group(in.props, in.display);
        }
        while (!fields.isEmpty()) o += fields.takeLast().inResult ? "}}" : "";
        while (!links.isEmpty()) o += links.takeLast() ? "}}" : "}";
        return o;
    }

    QByteArray para(const Paragraph &p, const QByteArray &ending, bool inTable = false)
    {
        QByteArray o = inTable ? "\\pard\\intbl" : "\\pard";
        int style = 0;
        if (!p.props.style.isEmpty() && styleNumber.contains(p.props.style)) style = styleNumber.value(p.props.style);
        else if (const QString d = doc.styles.defaultId(QStringLiteral("paragraph")); styleNumber.contains(d)) style = styleNumber.value(d);
        o += "\\s" + QByteArray::number(style);
        if (p.props.numId && *p.props.numId > 0) {
            const int numId = *p.props.numId;
            if (!listNumber.contains(numId) && doc.numbering.num(numId)) {
                listNumber.insert(numId, int(listNumber.size()) + 1);
                listOrder << numId;
            }
            if (listNumber.contains(numId)) o += "\\ls" + QByteArray::number(listNumber.value(numId)) + "\\ilvl" + QByteArray::number(std::clamp(p.props.ilvl.value_or(0), 0, 8));
        }
        o += paragraph(p.props);
        // (the paragraph mark's own looks, for the paragraph's last run)
        if (!o.endsWith(' ')) o += ' ';
        return o + inlines(p) + ending;
    }

    // ---- sections
    QByteArray sectionHead(const SectionProps &s)
    {
        QByteArray o = "\\sectd";
        const QString t = s.type;
        o += t == QLatin1String("continuous") ? "\\sbkcont" : t == QLatin1String("evenPage") ? "\\sbkeven" : t == QLatin1String("oddPage") ? "\\sbkodd" : t == QLatin1String("nextColumn") ? "\\sbkcol" : "\\sbkpage";
        const bool landscape = s.orient == QLatin1String("landscape");
        o += "\\pgwsxn" + QByteArray::number(s.pageW) + "\\pghsxn" + QByteArray::number(s.pageH) + (landscape ? "\\lndscpsxn" : "") + "\\marglsxn" + QByteArray::number(s.left) + "\\margrsxn" +
             QByteArray::number(s.right) + "\\margtsxn" + QByteArray::number(s.top) + "\\margbsxn" + QByteArray::number(s.bottom) + "\\guttersxn" + QByteArray::number(s.gutter) + "\\headery" +
             QByteArray::number(s.header) + "\\footery" + QByteArray::number(s.footer);
        if (s.titlePg) o += "\\titlepg";
        if (s.cols > 1) o += "\\cols" + QByteArray::number(s.cols) + "\\colsx" + QByteArray::number(s.colSpace);
        if (s.pageStart) o += "\\pgnstarts" + QByteArray::number(*s.pageStart) + "\\pgnrestart";
        if (s.pageFormat == QLatin1String("lowerRoman")) o += "\\pgnlcrm";
        else if (s.pageFormat == QLatin1String("upperRoman")) o += "\\pgnucrm";
        else if (s.pageFormat == QLatin1String("lowerLetter")) o += "\\pgnlcltr";
        else if (s.pageFormat == QLatin1String("upperLetter")) o += "\\pgnucltr";
        else if (s.pageFormat == QLatin1String("decimal") || s.pageFormat.isEmpty()) o += "\\pgndec";
        return o + "\n";
    }

    QByteArray headerParts(const SectionProps &s, int depth)
    {
        QByteArray o;
        const HeaderRefs refs = headerRefsOf(s);
        static const char *header[3] = {"\\headerr", "\\headerf", "\\headerl"};
        static const char *footer[3] = {"\\footerr", "\\footerf", "\\footerl"};
        for (int footerKind = 0; footerKind < 2; ++footerKind)
            for (int slot = 0; slot < 3; ++slot) {
                const QString id = footerKind ? refs.footer[slot] : refs.header[slot];
                if (id.isEmpty()) continue;
                QString target;
                for (const Relationship &r : doc.mainRelationships)
                    if (r.id == id && !r.external) target = r.target;
                for (const KeptPart &p : doc.parts) {
                    if (p.name.compare(target, Qt::CaseInsensitive) != 0 && !p.name.endsWith(QLatin1Char('/') + target, Qt::CaseInsensitive)) continue;
                    QVector<Block> blocks;
                    if (!readBlocks(p.data, &blocks, true)) break;
                    QByteArray body = blocksText(blocks, depth + 1);
                    if (body.isEmpty()) body = "\\pard \\par\n";
                    o += "{" + QByteArray(footerKind ? footer[slot] : header[slot]) + " " + body + "}\n";
                    break;
                }
            }
        return o;
    }

    // ---- tables
    QByteArray cellBorders(const TableMark &c, const TableMark &table, bool firstRow, bool lastRow, bool firstCol, bool lastCol)
    {
        auto side = [&](const char *name, const TableBorder &own, const TableBorder &fallback) {
            const TableBorder &b = own.stated() ? own : fallback;
            if (!b.stated() || b.val == QLatin1String("nil") || b.val == QLatin1String("none") || b.size <= 0) return QByteArray();
            Border x;
            x.val = b.val;
            x.size = b.size;
            x.space = b.space;
            x.color = b.color;
            return QByteArray("\\clbrdr") + name + border("", x).mid(5);   // (the cell's border words are the paragraph's with "clbrdr" in front)
        };
        return side("t", c.top, firstRow ? table.top : table.insideH) + side("l", c.left, firstCol ? table.left : table.insideV) + side("b", c.bottom, lastRow ? table.bottom : table.insideH) +
               side("r", c.right, lastCol ? table.right : table.insideV);
    }

    QByteArray tableText(const QVector<Block> &blocks, qsizetype from, qsizetype to, int depth)
    {
        // The table's own marks, with its style's borders where it states none.
        TableMark table = *blocks[from].table;
        const TableStyleDef &def = doc.styles.tableStyle(table.style);
        auto fill = [](TableBorder *own, const TableBorder &style) {
            if (!own->stated()) *own = style;
        };
        fill(&table.top, def.base.tbl.top);
        fill(&table.left, def.base.tbl.left);
        fill(&table.bottom, def.base.tbl.bottom);
        fill(&table.right, def.base.tbl.right);
        fill(&table.insideH, def.base.tbl.insideH);
        fill(&table.insideV, def.base.tbl.insideV);
        // The rows: where each starts and ends.
        QByteArray o;
        QVector<qsizetype> rowFrom, rowTo;
        int level = 0;
        for (qsizetype i = from + 1; i < to; ++i) {
            const Block &b = blocks[i];
            if (b.isRaw && b.tag == QLatin1String("tbl")) level += b.role == Inline::Open ? 1 : b.role == Inline::Close ? -1 : 0;
            if (level == 0 && b.isRaw && b.tag == QLatin1String("tr")) {
                if (b.role == Inline::Open) rowFrom << i;
                else if (b.role == Inline::Close && rowTo.size() < rowFrom.size()) rowTo << i;
            }
        }
        const int rows = int(std::min(rowFrom.size(), rowTo.size()));
        for (int r = 0; r < rows; ++r) {
            const TableMark row = rowMarkOf(blocks[rowFrom[r]]);
            struct Cell {
                TableMark mark;
                qsizetype from, to;
            };
            QVector<Cell> cells;
            int nest = 0;
            for (qsizetype i = rowFrom[r] + 1; i < rowTo[r]; ++i) {
                const Block &b = blocks[i];
                if (!b.isRaw) continue;
                if (b.tag == QLatin1String("tbl")) nest += b.role == Inline::Open ? 1 : b.role == Inline::Close ? -1 : 0;
                if (nest == 0 && b.tag == QLatin1String("tc")) {
                    if (b.role == Inline::Open) cells.push_back({cellMarkOf(b), i + 1, i + 1});
                    else if (b.role == Inline::Close && !cells.isEmpty()) cells.last().to = i;
                }
            }
            QByteArray head = "\\trowd\\trgaph108";
            if (table.indent.isDxa() && table.indent.w) head += "\\trleft" + QByteArray::number(table.indent.w);
            if (table.jc == QLatin1String("center")) head += "\\trqc";
            else if (table.jc == QLatin1String("right") || table.jc == QLatin1String("end")) head += "\\trqr";
            if (row.height) head += "\\trrh" + QByteArray::number(row.heightRule == QLatin1String("exact") ? -row.height : row.height);
            if (row.header) head += "\\trhdr";
            if (row.cantSplit) head += "\\trkeep";
            int edge = table.indent.isDxa() ? table.indent.w : 0;
            for (int g = 0; g < row.gridBefore && g < table.grid.size(); ++g) edge += table.grid[g];
            int col = row.gridBefore;
            QByteArray defs;
            for (int c = 0; c < cells.size(); ++c) {
                const TableMark &m = cells[c].mark;
                const int span = std::max(1, m.span);
                // (a cell across several columns is the first of that many cells, joined: \clmgf, then \clmrg for the rest, each with its column's edge)
                for (int k = 0; k < span; ++k) {
                    int width = col + k < table.grid.size() ? table.grid[col + k] : 0;
                    if (width == 0) width = span == 1 && m.cellWidth.isDxa() && m.cellWidth.w ? m.cellWidth.w : 1440;
                    edge += width;
                    if (span > 1) defs += k == 0 ? "\\clmgf" : "\\clmrg";
                    if (m.vMerge == QLatin1String("restart")) defs += "\\clvmgf";
                    else if (m.vMerge == QLatin1String("continue")) defs += "\\clvmrg";
                    if (m.vAlign == QLatin1String("center")) defs += "\\clvertalc";
                    else if (m.vAlign == QLatin1String("bottom")) defs += "\\clvertalb";
                    defs += cellBorders(m, table, r == 0, r == rows - 1, c == 0 && k == 0, c == cells.size() - 1 && k == span - 1);
                    if (!m.shdFill.isEmpty() && m.shdFill != QLatin1String("auto")) defs += "\\clcbpat" + QByteArray::number(color(m.shdFill));
                    defs += "\\cellx" + QByteArray::number(edge);
                }
                col += span;
            }
            o += head + defs + "\n";
            for (const Cell &c : std::as_const(cells)) {
                QByteArray inner = blocksText(blocks, c.from, c.to, depth + 1, true);
                if (inner.isEmpty()) inner = "\\pard\\intbl \\par\n";
                o += inner + "\\cell\n";
                for (int k = 1; k < std::max(1, c.mark.span); ++k) o += "\\pard\\intbl \\cell\n";
            }
            o += "\\row\n";
        }
        return o;
    }

    QByteArray blocksText(const QVector<Block> &blocks, int depth) { return blocksText(blocks, 0, blocks.size(), depth, false); }

    // The blocks [from, to) of a story. A paragraph that ends a section ends it with \sect; `inCell` paragraphs are in a table cell.
    QByteArray blocksText(const QVector<Block> &blocks, qsizetype from, qsizetype to, int depth, bool inCell)
    {
        QByteArray o;
        for (qsizetype i = from; i < to; ++i) {
            const Block &b = blocks[i];
            if (b.isRaw) {
                if (b.role == Inline::Open && b.tag == QLatin1String("tbl") && b.table && depth < kMostDepth) {
                    // to the table's end
                    int level = 0;
                    qsizetype end = i;
                    for (qsizetype k = i; k < to; ++k) {
                        if (blocks[k].isRaw && blocks[k].tag == QLatin1String("tbl")) level += blocks[k].role == Inline::Open ? 1 : blocks[k].role == Inline::Close ? -1 : 0;
                        if (level == 0) {
                            end = k;
                            break;
                        }
                    }
                    if (end > i) {
                        o += tableText(blocks, i, end, depth);
                        i = end;
                        continue;
                    }
                }
                if (b.role == Inline::Single && !b.display.isEmpty()) o += "\\pard " + text(b.display) + "\\par\n";
                continue;
            }
            o += para(b.para, "\\par\n", inCell);
        }
        return o;
    }
};

} // namespace

QByteArray writeRtf(const Document &doc, QStringList *notKept, QString *error)
{
    Q_UNUSED(error);
    Writer w(doc);
    // The styles first (they number what the text refers to); then the text, which asks the tables for the fonts, colors, and lists it uses.
    const QByteArray styles = w.styleTable();
    // A section's formatting leads its text; its last paragraph carries the section's end.
    QVector<SectionProps> sections;
    for (const Block &b : doc.body)
        if (!b.isRaw && b.para.props.section) sections << *b.para.props.section;
    sections << doc.lastSection;
    QByteArray body;
    int section = 0;
    body += w.sectionHead(sections[0]) + w.headerParts(sections[0], 0);
    // (walked a block at a time so that a section's end can follow its last paragraph)
    qsizetype start = 0;
    QByteArray chunk;
    for (qsizetype i = 0; i < doc.body.size(); ++i) {
        const Block &b = doc.body[i];
        if (b.isRaw || !b.para.props.section) continue;
        // a section's blocks, its last paragraph written with \sect
        QVector<Block> part = doc.body.mid(start, i - start + 1);
        Paragraph last = part.last().para;
        part.removeLast();
        chunk = w.blocksText(part, 0);
        chunk += w.para(last, "\\sect\n");
        body += chunk;
        start = i + 1;
        ++section;
        if (section < sections.size()) body += w.sectionHead(sections[section]) + w.headerParts(sections[section], 0);
    }
    if (start < doc.body.size()) body += w.blocksText(doc.body.mid(start), 0);
    else body += "\\pard \\par\n";
    const QByteArray lists = w.listTables();
    QByteArray o = "{\\rtf1\\ansi\\ansicpg1252\\uc1\\deff0\\deftab" + QByteArray::number(doc.defaultTabStop) + "\n{\\fonttbl";
    if (w.fonts.isEmpty()) w.font(QStringLiteral("Times New Roman"));
    for (int i = 0; i < w.fonts.size(); ++i) o += "{\\f" + QByteArray::number(i) + "\\fnil " + Writer::text(w.fonts[i]) + ";}";
    o += "}\n{\\colortbl;";
    for (const QString &c : std::as_const(w.colors)) {
        bool ok = false;
        const uint v = c.toUInt(&ok, 16);
        o += "\\red" + QByteArray::number((v >> 16) & 255) + "\\green" + QByteArray::number((v >> 8) & 255) + "\\blue" + QByteArray::number(v & 255) + ";";
    }
    o += "}\n{\\stylesheet\n" + styles + "}\n" + lists + body + "}\n";
    if (notKept) *notKept = w.lost;
    return o;
}

} // namespace jp::office::rtf

#include "mathml/mathml.h"

#include <QVector>
#include <QXmlStreamReader>
#include <functional>

namespace jp::office::mathml {

namespace {

const char kMathNs[] = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";
const char kMathmlNs[] = "xmlns=\"http://www.w3.org/1998/Math/MathML\"";
constexpr uint kApplyFunction = 0x2061;

// A part read into nodes (names without their prefixes). Nothing here recurses.
struct Node {
    QString name;
    QVector<QPair<QString, QString>> attrs;
    QString text;      // the character data of the element itself
    QVector<int> kids;
};

QString localPart(QStringView qualified)
{
    const qsizetype colon = qualified.lastIndexOf(QLatin1Char(':'));
    return (colon < 0 ? qualified : qualified.mid(colon + 1)).toString();
}

// Refused: more than kMostBytes, kMostElements elements, nesting past kMostDepth, a DOCTYPE (so no entity is ever defined or loaded), and XML that is not well formed.
bool readTree(const QByteArray &xml, QVector<Node> &nodes, QString *error)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        nodes.clear();
        return false;
    };
    if (xml.size() > kMostBytes) return fail(QStringLiteral("part too large"));
    QXmlStreamReader r(xml);
    r.setNamespaceProcessing(false);
    QVector<int> open;
    while (!r.atEnd()) {
        switch (r.readNext()) {
        case QXmlStreamReader::DTD:
            return fail(QStringLiteral("a DOCTYPE is not allowed"));
        case QXmlStreamReader::EntityReference:
            return fail(QStringLiteral("an entity is not allowed"));
        case QXmlStreamReader::StartElement: {
            if (nodes.size() >= kMostElements) return fail(QStringLiteral("too many elements"));
            if (open.size() >= kMostDepth) return fail(QStringLiteral("nested too deep"));
            Node n;
            n.name = localPart(r.qualifiedName());
            for (const QXmlStreamAttribute &a : r.attributes()) n.attrs.append({localPart(a.qualifiedName()), a.value().toString()});
            const int index = int(nodes.size());
            nodes.append(n);
            if (!open.isEmpty()) nodes[open.last()].kids.append(index);
            open.append(index);
            break;
        }
        case QXmlStreamReader::EndElement:
            if (!open.isEmpty()) open.removeLast();
            break;
        case QXmlStreamReader::Characters:
            if (!open.isEmpty()) nodes[open.last()].text += r.text();
            break;
        default:
            break;
        }
    }
    if (r.hasError()) return fail(r.errorString());
    if (nodes.isEmpty()) return fail(QStringLiteral("no element"));
    return true;
}

QByteArray esc(const QString &s) { return s.toHtmlEscaped().toUtf8(); }

QVector<uint> codePoints(const QString &s) { return s.toUcs4(); }
QString fromPoint(uint c) { return QString::fromUcs4(reinterpret_cast<const char32_t *>(&c), 1); }
bool isDigit(uint c) { return c >= '0' && c <= '9'; }

bool isNaryChar(uint c) { return (c >= 0x220F && c <= 0x2211) || (c >= 0x222B && c <= 0x2233) || (c >= 0x22C0 && c <= 0x22C3) || (c >= 0x2A00 && c <= 0x2A0C); }
bool isIntegral(uint c) { return c >= 0x222B && c <= 0x2233; }
bool isBarChar(uint c) { return c == 0xAF || c == 0x203E || c == 0x2015 || c == 0x305 || c == 0x304; }
bool isUnderBar(uint c) { return c == '_' || c == 0x2581 || c == 0x332; }
bool isApplyFunction(uint c) { return c == kApplyFunction; }

// A combining mark (as the markup keeps an accent) and the spacing character MathML shows it as.
struct AccentPair {
    uint combining, spacing;
};
const AccentPair kAccents[] = {{0x300, 0x60}, {0x301, 0xB4}, {0x302, 0x5E}, {0x303, 0x7E}, {0x305, 0xAF}, {0x306, 0x2D8}, {0x307, 0x2D9}, {0x308, 0xA8}, {0x30A, 0x2DA}, {0x30C, 0x2C7}, {0x20D7, 0x2192}};

uint spacingAccent(uint c)
{
    for (const AccentPair &p : kAccents)
        if (p.combining == c) return p.spacing;
    return c;
}
// The combining mark for a mark of either kind, or 0 for a character that is no accent.
uint combiningAccent(uint c)
{
    for (const AccentPair &p : kAccents)
        if (p.combining == c || p.spacing == c || (c == 0x2C6 && p.combining == 0x302)) return p.combining;
    return 0;
}

// A MathML variant, and the style and script of the markup that stand for it.
struct Variant {
    const char *mathml, *sty, *scr;
};
const Variant kVariants[] = {{"normal", "p", ""}, {"bold", "b", ""}, {"italic", "i", ""}, {"bold-italic", "bi", ""}, {"script", "", "script"}, {"bold-script", "b", "script"},
                             {"fraktur", "", "fraktur"}, {"bold-fraktur", "b", "fraktur"}, {"double-struck", "", "double-struck"}, {"sans-serif", "", "sans-serif"},
                             {"bold-sans-serif", "b", "sans-serif"}, {"sans-serif-italic", "i", "sans-serif"}, {"sans-serif-bold-italic", "bi", "sans-serif"}, {"monospace", "", "monospace"}};

bool truthy(const QString &v)
{
    if (v.isEmpty()) return true;
    return !(v == QLatin1String("0") || v == QLatin1String("off") || v == QLatin1String("false"));
}

class Tree {
public:
    QVector<Node> nodes;
    const Node &at(int n) const { return nodes[n]; }
    const QString &name(int n) const { return nodes[n].name; }
    bool has(int n, const char *name) const
    {
        for (const auto &a : nodes[n].attrs)
            if (a.first == QLatin1String(name)) return true;
        return false;
    }
    QString attr(int n, const char *name) const
    {
        for (const auto &a : nodes[n].attrs)
            if (a.first == QLatin1String(name)) return a.second;
        return QString();
    }
    int child(int n, const char *name) const
    {
        if (n < 0) return -1;
        for (int k : nodes[n].kids)
            if (nodes[k].name == QLatin1String(name)) return k;
        return -1;
    }
    // All the text of an element and of the elements in it, in order.
    QString allText(int n) const
    {
        QString out;
        QVector<int> todo{n};
        while (!todo.isEmpty()) {
            const int cur = todo.takeLast();
            out += nodes[cur].text;
            for (int i = int(nodes[cur].kids.size()) - 1; i >= 0; --i) todo.append(nodes[cur].kids[i]);
        }
        return out;
    }
};

// ---- markup to MathML

class ToMathml {
public:
    explicit ToMathml(const Tree &t) : m_t(t) {}

    QByteArray all()
    {
        QVector<QByteArray> items;
        const bool para = m_t.name(0) == QLatin1String("oMathPara");
        if (para) {
            for (int k : m_t.at(0).kids)
                if (m_t.name(k) == QLatin1String("oMath")) items.append(wrap(children(k)));
        } else
            items = children(0);
        if (items.isEmpty()) return {};
        return QByteArray("<math ") + kMathmlNs + (para ? " display=\"block\"" : "") + ">" + wrap(items) + "</math>";
    }

private:
    const Tree &m_t;

    static QByteArray join(const QVector<QByteArray> &items)
    {
        QByteArray out;
        for (const QByteArray &i : items) out += i;
        return out;
    }
    static QByteArray wrap(const QVector<QByteArray> &items) { return items.size() == 1 ? items.first() : "<mrow>" + join(items) + "</mrow>"; }
    // The content of an argument (e, num, sub...) as one element.
    QByteArray arg(int n, const char *name) const
    {
        const int c = m_t.child(n, name);
        return c < 0 ? QByteArray("<mrow/>") : wrap(children(c));
    }
    QString prop(int n, const char *props, const char *name, bool *has = nullptr) const
    {
        const int c = m_t.child(m_t.child(n, props), name);
        if (has) *has = c >= 0 && m_t.has(c, "val");
        return c >= 0 ? m_t.attr(c, "val") : QString();
    }
    bool flag(int n, const char *props, const char *name) const
    {
        const int c = m_t.child(m_t.child(n, props), name);
        return c >= 0 && truthy(m_t.attr(c, "val"));
    }
    QVector<QByteArray> children(int n) const
    {
        QVector<QByteArray> out;
        for (int k : m_t.at(n).kids) out += one(k);
        return out;
    }
    static QByteArray mo(const QString &text, const char *attrs = "") { return QByteArray("<mo") + attrs + ">" + esc(text) + "</mo>"; }

    // Text as MathML tokens: letters, numbers, operators, and spaces.
    static QVector<QByteArray> tokens(const QString &text, const QString &sty, const QString &scr)
    {
        QString variant;
        for (const Variant &v : kVariants)
            if (sty == QLatin1String(v.sty) && scr == QLatin1String(v.scr)) variant = QLatin1String(v.mathml);
        if (variant.isEmpty())
            for (const Variant &v : kVariants)
                if (!scr.isEmpty() && scr == QLatin1String(v.scr)) variant = QLatin1String(v.mathml);
        if (variant.isEmpty() && !sty.isEmpty())
            for (const Variant &v : kVariants)
                if (sty == QLatin1String(v.sty) && !*v.scr) variant = QLatin1String(v.mathml);
        const QByteArray vattr = variant.isEmpty() ? QByteArray() : " mathvariant=\"" + variant.toUtf8() + "\"";
        const bool upright = variant == QLatin1String("normal");
        QVector<QByteArray> out;
        const QVector<uint> cp = codePoints(text);
        for (int i = 0; i < cp.size();) {
            const uint c = cp[i];
            if (QChar::isSpace(c)) {
                out.append("<mspace width=\"0.278em\"/>");
                ++i;
            } else if (isDigit(c)) {
                QString num;
                while (i < cp.size() && (isDigit(cp[i]) || (cp[i] == '.' && i + 1 < cp.size() && isDigit(cp[i + 1])))) num += fromPoint(cp[i++]);
                out.append("<mn" + vattr + ">" + esc(num) + "</mn>");
            } else if (QChar::isLetter(c)) {
                QString word = fromPoint(c);
                ++i;
                while (upright && i < cp.size() && QChar::isLetter(cp[i])) word += fromPoint(cp[i++]);
                out.append("<mi" + vattr + ">" + esc(word) + "</mi>");
            } else {
                out.append(mo(fromPoint(c)));
                ++i;
            }
        }
        return out;
    }

    QVector<QByteArray> run(int n) const
    {
        QString text, sty, scr;
        bool nor = false;
        for (int k : m_t.at(n).kids) {
            const QString &kn = m_t.name(k);
            if (kn == QLatin1String("t")) text += m_t.at(k).text;
            else if (kn == QLatin1String("rPr")) {
                const int s = m_t.child(k, "sty"), c = m_t.child(k, "scr"), o = m_t.child(k, "nor");
                if (s >= 0) sty = m_t.attr(s, "val");
                if (c >= 0) scr = m_t.attr(c, "val");
                if (o >= 0) nor = truthy(m_t.attr(o, "val"));
            }
        }
        if (text.isEmpty()) return {};
        if (nor) return {"<mtext>" + esc(text) + "</mtext>"};
        if (scr == QLatin1String("roman")) scr.clear();
        return tokens(text, sty, scr);
    }

    QVector<QByteArray> one(int n) const
    {
        const QString &name = m_t.name(n);
        if (name == QLatin1String("r")) return run(n);
        if (name == QLatin1String("f")) {
            const QString type = prop(n, "fPr", "type");
            QByteArray attrs;
            if (type == QLatin1String("noBar")) attrs = " linethickness=\"0\"";
            else if (type == QLatin1String("skw") || type == QLatin1String("lin")) attrs = " bevelled=\"true\"";
            return {"<mfrac" + attrs + ">" + arg(n, "num") + arg(n, "den") + "</mfrac>"};
        }
        if (name == QLatin1String("sSup")) return {"<msup>" + arg(n, "e") + arg(n, "sup") + "</msup>"};
        if (name == QLatin1String("sSub")) return {"<msub>" + arg(n, "e") + arg(n, "sub") + "</msub>"};
        if (name == QLatin1String("sSubSup")) return {"<msubsup>" + arg(n, "e") + arg(n, "sub") + arg(n, "sup") + "</msubsup>"};
        if (name == QLatin1String("sPre")) return {"<mmultiscripts>" + arg(n, "e") + "<mprescripts/>" + arg(n, "sub") + arg(n, "sup") + "</mmultiscripts>"};
        if (name == QLatin1String("rad")) {
            if (flag(n, "radPr", "degHide")) return {"<msqrt>" + arg(n, "e") + "</msqrt>"};
            return {"<mroot>" + arg(n, "e") + arg(n, "deg") + "</mroot>"};
        }
        if (name == QLatin1String("nary")) return nary(n);
        if (name == QLatin1String("d")) return {fenced(n)};
        if (name == QLatin1String("acc")) {
            bool has = false;
            const QString chr = prop(n, "accPr", "chr", &has);
            const uint c = has && !chr.isEmpty() ? chr.toUcs4().first() : 0x302;
            return {"<mover accent=\"true\">" + arg(n, "e") + mo(fromPoint(spacingAccent(c))) + "</mover>"};
        }
        if (name == QLatin1String("bar")) {
            if (prop(n, "barPr", "pos") == QLatin1String("bot")) return {"<munder>" + arg(n, "e") + mo(QStringLiteral("_")) + "</munder>"};
            return {"<mover>" + arg(n, "e") + mo(fromPoint(0xAF)) + "</mover>"};
        }
        if (name == QLatin1String("m")) return {matrix(n)};
        if (name == QLatin1String("eqArr")) {
            QByteArray rows;
            for (int k : m_t.at(n).kids)
                if (m_t.name(k) == QLatin1String("e")) rows += "<mtr><mtd>" + wrap(children(k)) + "</mtd></mtr>";
            return {"<mtable columnalign=\"left\">" + rows + "</mtable>"};
        }
        if (name == QLatin1String("func")) return {"<mrow>" + arg(n, "fName") + mo(fromPoint(kApplyFunction)) + arg(n, "e") + "</mrow>"};
        if (name == QLatin1String("limLow")) return {"<munder>" + arg(n, "e") + arg(n, "lim") + "</munder>"};
        if (name == QLatin1String("limUpp")) return {"<mover>" + arg(n, "e") + arg(n, "lim") + "</mover>"};
        if (name == QLatin1String("box") || name == QLatin1String("phant") || name == QLatin1String("borderBox") || name == QLatin1String("groupChr")) return {arg(n, "e")};
        if (name == QLatin1String("e") || name == QLatin1String("oMath")) return children(n);
        if (name.endsWith(QLatin1String("Pr"))) return {};
        // What has no match is kept as its text.
        const QString text = m_t.allText(n).simplified();
        if (text.isEmpty()) return {};
        return {"<mtext>" + esc(text) + "</mtext>"};
    }

    QVector<QByteArray> nary(int n) const
    {
        bool has = false;
        const QString chr = prop(n, "naryPr", "chr", &has);
        const QString symbol = has ? chr : QString(QChar(0x222B));
        const uint c = symbol.isEmpty() ? 0x222B : symbol.toUcs4().first();
        bool hasLoc = false;
        const QString loc = prop(n, "naryPr", "limLoc", &hasLoc);
        const bool under = hasLoc ? loc != QLatin1String("subSup") : !isIntegral(c);
        const bool noSub = flag(n, "naryPr", "subHide"), noSup = flag(n, "naryPr", "supHide");
        const QByteArray op = mo(symbol, " largeop=\"true\"");
        QByteArray head;
        if (noSub && noSup) head = op;
        else if (noSup) head = QByteArray(under ? "<munder>" : "<msub>") + op + arg(n, "sub") + (under ? "</munder>" : "</msub>");
        else if (noSub) head = QByteArray(under ? "<mover>" : "<msup>") + op + arg(n, "sup") + (under ? "</mover>" : "</msup>");
        else head = QByteArray(under ? "<munderover>" : "<msubsup>") + op + arg(n, "sub") + arg(n, "sup") + (under ? "</munderover>" : "</msubsup>");
        return {head, arg(n, "e")};
    }

    QByteArray fenced(int n) const
    {
        bool hasBeg = false, hasEnd = false, hasSep = false;
        const QString beg = prop(n, "dPr", "begChr", &hasBeg), end = prop(n, "dPr", "endChr", &hasEnd), sep = prop(n, "dPr", "sepChr", &hasSep);
        const QString open = hasBeg ? beg : QStringLiteral("("), close = hasEnd ? end : QStringLiteral(")"), separator = hasSep ? sep : QStringLiteral("|");
        QByteArray out = "<mrow>" + mo(open, " fence=\"true\" stretchy=\"true\"");
        bool first = true;
        for (int k : m_t.at(n).kids) {
            if (m_t.name(k) != QLatin1String("e")) continue;
            if (!first && !separator.isEmpty()) out += mo(separator, " separator=\"true\" stretchy=\"true\"");
            first = false;
            out += wrap(children(k));
        }
        return out + mo(close, " fence=\"true\" stretchy=\"true\"") + "</mrow>";
    }

    QByteArray matrix(int n) const
    {
        QString align = QStringLiteral("center");
        const int mc = m_t.child(m_t.child(m_t.child(n, "mPr"), "mcs"), "mc");
        const int jc = m_t.child(m_t.child(mc, "mcPr"), "mcJc");
        if (jc >= 0 && (m_t.attr(jc, "val") == QLatin1String("left") || m_t.attr(jc, "val") == QLatin1String("right"))) align = m_t.attr(jc, "val");
        QByteArray rows;
        for (int r : m_t.at(n).kids) {
            if (m_t.name(r) != QLatin1String("mr")) continue;
            rows += "<mtr>";
            for (int e : m_t.at(r).kids)
                if (m_t.name(e) == QLatin1String("e")) rows += "<mtd>" + wrap(children(e)) + "</mtd>";
            rows += "</mtr>";
        }
        return "<mtable columnalign=\"" + align.toUtf8() + "\">" + rows + "</mtable>";
    }
};

// ---- MathML to markup

class FromMathml {
public:
    explicit FromMathml(const Tree &t) : m_t(t) {}

    QByteArray all()
    {
        const QByteArray body = seq(m_t.at(0).kids);
        if (body.isEmpty()) return {};
        return QByteArray("<m:oMath ") + kMathNs + ">" + body + "</m:oMath>";
    }

private:
    const Tree &m_t;

    static QByteArray run(const QString &text, const QString &sty = QString(), const QString &scr = QString(), bool nor = false)
    {
        QByteArray props;
        if (nor) props = "<m:nor m:val=\"1\"/>";
        else {
            if (!scr.isEmpty()) props += "<m:scr m:val=\"" + esc(scr) + "\"/>";
            if (!sty.isEmpty()) props += "<m:sty m:val=\"" + esc(sty) + "\"/>";
        }
        const bool keep = text.startsWith(QLatin1Char(' ')) || text.endsWith(QLatin1Char(' '));
        return "<m:r>" + (props.isEmpty() ? QByteArray() : "<m:rPr>" + props + "</m:rPr>") + (keep ? "<m:t xml:space=\"preserve\">" : "<m:t>") + esc(text) + "</m:t></m:r>";
    }
    static QByteArray part(const char *tag, const QByteArray &inner) { return QByteArray("<m:") + tag + ">" + inner + "</m:" + tag + ">"; }
    static QByteArray valued(const char *tag, const QString &value) { return QByteArray("<m:") + tag + " m:val=\"" + esc(value) + "\"/>"; }

    QByteArray one(int n) { return seq(QVector<int>{n}); }
    QByteArray oneOrNone(int n) { return n >= 0 ? one(n) : QByteArray(); }
    QByteArray argAt(const QVector<int> &kids, int i, const char *tag) { return part(tag, i >= 0 && i < kids.size() ? one(kids[i]) : QByteArray()); }
    QString text(int n) const { return m_t.at(n).text.simplified(); }

    bool isOp(int n, const std::function<bool(uint)> &pred) const
    {
        if (m_t.name(n) != QLatin1String("mo")) return false;
        const QVector<uint> cp = codePoints(text(n));
        return cp.size() == 1 && pred(cp[0]);
    }
    bool isNaryBase(int n) const { return isOp(n, isNaryChar); }
    static bool isOpener(uint c) { return c == '(' || c == '[' || c == '{' || c == '|' || c == 0x2016 || c == 0x27E8 || c == 0x2308 || c == 0x230A || c == 0x27E6 || c == 0x2329 || c == '<'; }
    static bool isCloser(uint c) { return c == ')' || c == ']' || c == '}' || c == '|' || c == 0x2016 || c == 0x27E9 || c == 0x2309 || c == 0x230B || c == 0x27E7 || c == 0x232A || c == '>'; }
    bool isFence(int n, bool opening) const
    {
        if (m_t.name(n) != QLatin1String("mo") || m_t.attr(n, "fence") != QLatin1String("true")) return false;
        const QVector<uint> cp = codePoints(text(n));
        if (cp.isEmpty()) return true;
        return cp.size() == 1 && (opening ? isOpener(cp[0]) : isCloser(cp[0]));
    }
    bool isSeparator(int n) const { return m_t.name(n) == QLatin1String("mo") && m_t.attr(n, "separator") == QLatin1String("true"); }

    // A group that begins and ends with a fence is a fence of the markup; what lies between, cut at separators (when all are one character), is its elements.
    bool fence(int n, QByteArray *out)
    {
        const QVector<int> &kids = m_t.at(n).kids;
        if (kids.size() < 2 || !isFence(kids.first(), true) || !isFence(kids.last(), false)) return false;
        QString separator;
        bool mixed = false;
        for (int i = 1; i + 1 < kids.size(); ++i)
            if (isSeparator(kids[i])) {
                if (separator.isEmpty()) separator = text(kids[i]);
                else if (separator != text(kids[i])) mixed = true;
            }
        const bool cut = !mixed && !separator.isEmpty();
        QVector<QVector<int>> cells(1);
        for (int i = 1; i + 1 < kids.size(); ++i) {
            if (cut && isSeparator(kids[i])) cells.append(QVector<int>());
            else cells.last().append(kids[i]);
        }
        QByteArray props = valued("begChr", text(kids.first()));
        if (cut) props += valued("sepChr", separator);
        props += valued("endChr", text(kids.last()));
        QByteArray es;
        for (const QVector<int> &cell : cells) es += part("e", seq(cell));
        *out = part("d", part("dPr", props) + es);
        return true;
    }

    // An operator with limits (`kids` are the operator and its limits of the element called `kind`), taking `operand` (an element or -1) as what it works on.
    QByteArray nary(const QVector<int> &kids, const QString &kind, int operand)
    {
        const uint c = codePoints(text(kids[0])).first();
        const bool under = kind == QLatin1String("munder") || kind == QLatin1String("mover") || kind == QLatin1String("munderover");
        int sub = -1, sup = -1;
        if (kind == QLatin1String("munder") || kind == QLatin1String("msub")) sub = kids.size() > 1 ? kids[1] : -1;
        else if (kind == QLatin1String("mover") || kind == QLatin1String("msup")) sup = kids.size() > 1 ? kids[1] : -1;
        else {
            sub = kids.size() > 1 ? kids[1] : -1;
            sup = kids.size() > 2 ? kids[2] : -1;
        }
        QByteArray props = valued("chr", fromPoint(c)) + valued("limLoc", QLatin1String(under ? "undOvr" : "subSup"));
        if (sub < 0) props += valued("subHide", QStringLiteral("1"));
        if (sup < 0) props += valued("supHide", QStringLiteral("1"));
        return part("nary", part("naryPr", props) + part("sub", oneOrNone(sub)) + part("sup", oneOrNone(sup)) + part("e", oneOrNone(operand)));
    }

    QByteArray bareNary(int op, int operand)
    {
        const uint c = codePoints(text(op)).first();
        const QByteArray props = valued("chr", fromPoint(c)) + valued("limLoc", QLatin1String(isIntegral(c) ? "subSup" : "undOvr")) + valued("subHide", QStringLiteral("1")) + valued("supHide", QStringLiteral("1"));
        return part("nary", part("naryPr", props) + part("sub", QByteArray()) + part("sup", QByteArray()) + part("e", oneOrNone(operand)));
    }

    QByteArray multiscripts(const QVector<int> &kids)
    {
        int pre = -1;
        for (int k = 1; k < kids.size(); ++k)
            if (m_t.name(kids[k]) == QLatin1String("mprescripts")) pre = k;
        const int postEnd = pre < 0 ? int(kids.size()) : pre;
        QByteArray inner = kids.isEmpty() ? QByteArray() : one(kids[0]);
        if (pre >= 0 && pre + 2 < kids.size()) inner = part("sPre", part("sPrePr", QByteArray()) + argAt(kids, pre + 1, "sub") + argAt(kids, pre + 2, "sup") + part("e", inner));
        auto shown = [&](int i) { return i < postEnd && m_t.name(kids[i]) != QLatin1String("none"); };
        if (postEnd > 2 && (shown(1) || shown(2))) return part("sSubSup", part("e", inner) + argAt(kids, 1, "sub") + argAt(kids, 2, "sup"));
        if (postEnd == 2 && shown(1)) return part("sSub", part("e", inner) + argAt(kids, 1, "sub"));
        return inner;
    }

    bool isAccentOver(const QVector<int> &kids) const
    {
        if (kids.size() != 2 || m_t.name(kids[1]) != QLatin1String("mo")) return false;
        const QVector<uint> cp = codePoints(text(kids[1]));
        return cp.size() == 1 && combiningAccent(cp[0]) != 0;
    }

    // The sequence of elements as the markup's content. An operator with limits takes the element after it, and a function name followed by the sign for applying it too.
    QByteArray seq(const QVector<int> &nodes)
    {
        QByteArray out;
        for (int i = 0; i < nodes.size(); ++i) {
            const int n = nodes[i];
            const QString &name = m_t.name(n);
            const QVector<int> &kids = m_t.at(n).kids;
            const int after = i + 1 < nodes.size() ? nodes[i + 1] : -1;
            if (name == QLatin1String("mi") || name == QLatin1String("mn") || name == QLatin1String("mo") || name == QLatin1String("mtext") || name == QLatin1String("ms")) {
                const QString t = text(n);
                if (name == QLatin1String("mo")) {
                    const QVector<uint> cp = codePoints(t);
                    if (cp.size() == 1 && ((cp[0] >= 0x2062 && cp[0] <= 0x2064) || isApplyFunction(cp[0]))) continue;
                    if (isNaryBase(n)) {
                        out += bareNary(n, after);
                        if (after >= 0) ++i;
                        continue;
                    }
                }
                if (name == QLatin1String("mtext") || name == QLatin1String("ms")) {
                    if (!t.isEmpty()) out += run(t, QString(), QString(), true);
                } else if (name == QLatin1String("mi") || name == QLatin1String("mn")) {
                    if (t.isEmpty()) continue;
                    if (name == QLatin1String("mi") && after >= 0 && isOp(after, isApplyFunction)) {
                        const int operand = i + 2 < nodes.size() ? nodes[i + 2] : -1;
                        out += part("func", part("fName", one(n)) + part("e", oneOrNone(operand)));
                        i += operand >= 0 ? 2 : 1;
                        continue;
                    }
                    QString sty, scr;
                    const QString variant = m_t.attr(n, "mathvariant");
                    if (!variant.isEmpty()) {
                        for (const Variant &v : kVariants)
                            if (variant == QLatin1String(v.mathml)) {
                                sty = QLatin1String(v.sty);
                                scr = QLatin1String(v.scr);
                            }
                    } else if (name == QLatin1String("mi") && codePoints(t).size() > 1)
                        sty = QStringLiteral("p");
                    out += run(t, sty, scr);
                } else if (!t.isEmpty())
                    out += run(t);
            } else if (name == QLatin1String("mspace")) {
                out += run(QStringLiteral(" "));
            } else if (name == QLatin1String("mrow")) {
                QByteArray d;
                out += fence(n, &d) ? d : seq(kids);
            } else if (name == QLatin1String("mstyle") || name == QLatin1String("mpadded") || name == QLatin1String("merror") || name == QLatin1String("menclose")) {
                out += seq(kids);
            } else if (name == QLatin1String("mphantom")) {
                out += part("phant", part("phantPr", valued("show", QStringLiteral("0"))) + part("e", seq(kids)));
            } else if (name == QLatin1String("semantics") || name == QLatin1String("maction")) {
                for (int k : kids) {
                    if (m_t.name(k) == QLatin1String("annotation") || m_t.name(k) == QLatin1String("annotation-xml")) continue;
                    out += one(k);
                    break;
                }
            } else if (name == QLatin1String("annotation") || name == QLatin1String("annotation-xml") || name == QLatin1String("none") || name == QLatin1String("mprescripts")) {
                // not part of what is shown
            } else if (name == QLatin1String("mfrac")) {
                const QString thick = m_t.attr(n, "linethickness").trimmed();
                const bool noBar = !thick.isEmpty() && thick.startsWith(QLatin1Char('0')) && !thick.startsWith(QStringLiteral("0."));
                const bool skew = m_t.attr(n, "bevelled") == QLatin1String("true");
                const QByteArray props = noBar ? part("fPr", valued("type", QStringLiteral("noBar"))) : skew ? part("fPr", valued("type", QStringLiteral("skw"))) : QByteArray();
                out += part("f", props + argAt(kids, 0, "num") + argAt(kids, 1, "den"));
            } else if (name == QLatin1String("msqrt")) {
                out += part("rad", part("radPr", valued("degHide", QStringLiteral("1"))) + part("deg", QByteArray()) + part("e", seq(kids)));
            } else if (name == QLatin1String("mroot")) {
                out += part("rad", part("radPr", QByteArray()) + argAt(kids, 1, "deg") + argAt(kids, 0, "e"));
            } else if (name == QLatin1String("msup") || name == QLatin1String("msub") || name == QLatin1String("msubsup")) {
                if (!kids.isEmpty() && isNaryBase(kids[0])) {
                    out += nary(kids, name, after);
                    if (after >= 0) ++i;
                } else if (name == QLatin1String("msup"))
                    out += part("sSup", argAt(kids, 0, "e") + argAt(kids, 1, "sup"));
                else if (name == QLatin1String("msub"))
                    out += part("sSub", argAt(kids, 0, "e") + argAt(kids, 1, "sub"));
                else
                    out += part("sSubSup", argAt(kids, 0, "e") + argAt(kids, 1, "sub") + argAt(kids, 2, "sup"));
            } else if (name == QLatin1String("mmultiscripts")) {
                out += multiscripts(kids);
            } else if (name == QLatin1String("munder") || name == QLatin1String("mover") || name == QLatin1String("munderover")) {
                if (!kids.isEmpty() && isNaryBase(kids[0])) {
                    out += nary(kids, name, after);
                    if (after >= 0) ++i;
                } else if (name == QLatin1String("mover") && kids.size() == 2 && isOp(kids[1], isBarChar) && m_t.attr(n, "accent") != QLatin1String("true")) {
                    out += part("bar", part("barPr", valued("pos", QStringLiteral("top"))) + argAt(kids, 0, "e"));
                } else if (name == QLatin1String("mover") && isAccentOver(kids)) {
                    out += part("acc", part("accPr", valued("chr", fromPoint(combiningAccent(codePoints(text(kids[1]))[0])))) + argAt(kids, 0, "e"));
                } else if (name == QLatin1String("munder") && kids.size() == 2 && isOp(kids[1], isUnderBar)) {
                    out += part("bar", part("barPr", valued("pos", QStringLiteral("bot"))) + argAt(kids, 0, "e"));
                } else if (name == QLatin1String("munder")) {
                    out += part("limLow", argAt(kids, 0, "e") + argAt(kids, 1, "lim"));
                } else if (name == QLatin1String("mover")) {
                    out += part("limUpp", argAt(kids, 0, "e") + argAt(kids, 1, "lim"));
                } else {
                    out += part("limUpp", part("e", part("limLow", argAt(kids, 0, "e") + argAt(kids, 1, "lim"))) + argAt(kids, 2, "lim"));
                }
            } else if (name == QLatin1String("mtable")) {
                out += table(n);
            } else if (name == QLatin1String("mfenced")) {
                const QString open = m_t.has(n, "open") ? m_t.attr(n, "open").trimmed() : QStringLiteral("(");
                const QString close = m_t.has(n, "close") ? m_t.attr(n, "close").trimmed() : QStringLiteral(")");
                const QString seps = (m_t.has(n, "separators") ? m_t.attr(n, "separators") : QStringLiteral(",")).trimmed();
                const QByteArray props = valued("begChr", open) + valued("sepChr", seps.left(1)) + valued("endChr", close);
                QByteArray es;
                for (int k : kids) es += part("e", one(k));
                if (kids.isEmpty()) es = part("e", QByteArray());
                out += part("d", part("dPr", props) + es);
            } else {
                // No match: what it says is kept as a run.
                const QString t = m_t.allText(n).simplified();
                if (!t.isEmpty() && name != QLatin1String("script") && name != QLatin1String("style")) out += run(t, QString(), QString(), true);
            }
        }
        return out;
    }

    QByteArray table(int n)
    {
        QString align = m_t.attr(n, "columnalign").split(QLatin1Char(' '), Qt::SkipEmptyParts).value(0);
        if (align != QLatin1String("left") && align != QLatin1String("right")) align = QStringLiteral("center");
        QByteArray rows;
        int columns = 0;
        for (int r : m_t.at(n).kids) {
            const bool labeled = m_t.name(r) == QLatin1String("mlabeledtr");
            if (m_t.name(r) != QLatin1String("mtr") && !labeled) continue;
            QByteArray cells;
            int count = 0;
            bool first = true;
            for (int c : m_t.at(r).kids) {
                if (m_t.name(c) != QLatin1String("mtd")) continue;
                if (labeled && first) {
                    first = false;
                    continue;
                }
                cells += part("e", seq(m_t.at(c).kids));
                ++count;
            }
            columns = qMax(columns, count);
            rows += part("mr", cells);
        }
        if (columns == 0) return {};
        const QByteArray props = part("mcs", part("mc", part("mcPr", valued("count", QString::number(columns)) + valued("mcJc", align))));
        return part("m", part("mPr", props) + rows);
    }
};

} // namespace

QByteArray mathmlFromOmml(const QByteArray &markup, bool *ok, QString *error)
{
    if (ok) *ok = false;
    Tree tree;
    if (!readTree(markup, tree.nodes, error)) return {};
    if (tree.name(0) != QLatin1String("oMath") && tree.name(0) != QLatin1String("oMathPara")) {
        if (error) *error = QStringLiteral("not math markup");
        return {};
    }
    const QByteArray out = ToMathml(tree).all();
    if (ok) *ok = true;
    return out;
}

QByteArray ommlFromMathml(const QByteArray &xml, bool *ok, QString *error)
{
    if (ok) *ok = false;
    Tree tree;
    if (!readTree(xml, tree.nodes, error)) return {};
    if (tree.name(0) != QLatin1String("math")) {
        if (error) *error = QStringLiteral("not MathML");
        return {};
    }
    const QByteArray out = FromMathml(tree).all();
    if (ok) *ok = true;
    return out;
}

} // namespace jp::office::mathml

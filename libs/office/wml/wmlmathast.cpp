#include "wml/wmlmathast.h"

#include "wml/wmlmathedit.h"
#include "wml/xmltree.h"

#include <QHash>
#include <QStringList>

namespace jp::office::wml::mathtree {

namespace {

const char kMathNs[] = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";

// What may stand in an XML text: no control characters (but a tab, line feed, or return), no lone surrogates, and no noncharacters.
QString clean(const QString &text)
{
    QString out;
    out.reserve(text.size());
    for (qsizetype i = 0; i < text.size(); ++i) {
        const ushort c = text[i].unicode();
        if (QChar::isHighSurrogate(c) && i + 1 < text.size() && QChar::isLowSurrogate(text[i + 1].unicode())) {
            out += text[i];
            out += text[++i];
        } else if (c < 0x20 ? (c == 9 || c == 10 || c == 13) : (!QChar::isSurrogate(c) && c != 0xFFFE && c != 0xFFFF)) {
            out += text[i];
        }
    }
    return out;
}

QByteArray escaped(const QString &text) { return clean(text).toHtmlEscaped().toUtf8(); }

QByteArray runXml(const Node &n)
{
    const QString text = clean(n.text);
    if (text.isEmpty()) return {};
    const bool spaces = text.startsWith(QLatin1Char(' ')) || text.endsWith(QLatin1Char(' ')) || text.contains(QLatin1String("  ")) || n.style == 'n';
    QByteArray out = "<m:r>";
    if (n.style == 'p') out += "<m:rPr><m:sty m:val=\"p\"/></m:rPr>";
    else if (n.style == 'b') out += "<m:rPr><m:sty m:val=\"b\"/></m:rPr>";
    else if (n.style == 'n') out += "<m:rPr><m:nor/></m:rPr>";
    return out + "<m:t" + (spaces ? " xml:space=\"preserve\"" : "") + ">" + text.toHtmlEscaped().toUtf8() + "</m:t></m:r>";
}

class Writer {
public:
    bool ok = true;
    QByteArray seq(const Seq &items, int depth)
    {
        QByteArray out;
        for (const Node &n : items) {
            if (!ok) break;
            out += node(n, depth);
            if (out.size() > kMostMarkupBytes) ok = false;
        }
        return out;
    }

private:
    int m_nodes = 0;
    // An operand: its items, or the run that holds its place.
    QByteArray operand(const Seq &items, int depth)
    {
        const QByteArray x = seq(items, depth);
        return x.isEmpty() ? emptyRun() : x;
    }
    static const Seq &part(const Node &n, size_t i)
    {
        static const Seq none;
        return i < n.part.size() ? n.part[i] : none;
    }
    QByteArray node(const Node &n, int depth)
    {
        if (++m_nodes > kMostNodes) {
            ok = false;
            return {};
        }
        if (n.kind == Kind::Run) return runXml(n);
        if (n.kind == Kind::Group) return seq(part(n, 0), depth);
        if (depth + 1 > kMostEquationDepth) {
            ok = false;
            return {};
        }
        const int d = depth + 1;
        switch (n.kind) {
        case Kind::Frac: return "<m:f><m:num>" + operand(part(n, 0), d) + "</m:num><m:den>" + operand(part(n, 1), d) + "</m:den></m:f>";
        case Kind::Script: {
            const QByteArray base = operand(part(n, 0), d);
            if (n.hasA && n.hasB) return "<m:sSubSup><m:e>" + base + "</m:e><m:sub>" + operand(part(n, 1), d) + "</m:sub><m:sup>" + operand(part(n, 2), d) + "</m:sup></m:sSubSup>";
            if (n.hasB) return "<m:sSup><m:e>" + base + "</m:e><m:sup>" + operand(part(n, 2), d) + "</m:sup></m:sSup>";
            return "<m:sSub><m:e>" + base + "</m:e><m:sub>" + operand(part(n, 1), d) + "</m:sub></m:sSub>";
        }
        case Kind::Rad: {
            const QByteArray props = n.hasA ? "<m:deg>" + operand(part(n, 0), d) + "</m:deg>" : QByteArray("<m:radPr><m:degHide m:val=\"1\"/></m:radPr><m:deg/>");
            return "<m:rad>" + props + "<m:e>" + operand(part(n, 1), d) + "</m:e></m:rad>";
        }
        case Kind::Nary: {
            const bool beside = n.limits == 2 || (n.limits == 0 && naryBeside(n));
            QByteArray props = "<m:naryPr><m:chr m:val=\"" + escaped(n.text) + "\"/><m:limLoc m:val=\"" + (beside ? "subSup" : "undOvr") + "\"/>";
            if (!n.hasA) props += "<m:subHide m:val=\"1\"/>";
            if (!n.hasB) props += "<m:supHide m:val=\"1\"/>";
            props += "</m:naryPr>";
            return "<m:nary>" + props + "<m:sub>" + (n.hasA ? operand(part(n, 0), d) : QByteArray()) + "</m:sub><m:sup>" + (n.hasB ? operand(part(n, 1), d) : QByteArray()) + "</m:sup><m:e>"
                   + operand(part(n, 2), d) + "</m:e></m:nary>";
        }
        case Kind::Delim: {
            QByteArray x = "<m:d><m:dPr><m:begChr m:val=\"" + escaped(n.text) + "\"/>";
            if (n.part.size() > 1) x += "<m:sepChr m:val=\"" + escaped(n.text3.isEmpty() ? QStringLiteral("|") : n.text3) + "\"/>";
            x += "<m:endChr m:val=\"" + escaped(n.text2) + "\"/></m:dPr>";
            if (n.part.empty()) x += "<m:e>" + emptyRun() + "</m:e>";
            for (const Seq &arg : n.part) x += "<m:e>" + operand(arg, d) + "</m:e>";
            return x + "</m:d>";
        }
        case Kind::Accent: return "<m:acc><m:accPr><m:chr m:val=\"" + escaped(n.text) + "\"/></m:accPr><m:e>" + operand(part(n, 0), d) + "</m:e></m:acc>";
        case Kind::Bar: return "<m:bar><m:barPr><m:pos m:val=\"" + QByteArray(n.hasA ? "top" : "bot") + "\"/></m:barPr><m:e>" + operand(part(n, 0), d) + "</m:e></m:bar>";
        case Kind::Matrix: {
            const int cols = std::max(1, n.cols);
            QByteArray x = "<m:m><m:mPr><m:mcs><m:mc><m:mcPr><m:count m:val=\"" + QByteArray::number(cols) + "\"/><m:mcJc m:val=\"center\"/></m:mcPr></m:mc></m:mcs></m:mPr>";
            for (size_t i = 0; i < n.part.size(); ++i) {
                if (int(i) % cols == 0) x += "<m:mr>";
                x += "<m:e>" + operand(n.part[i], d) + "</m:e>";
                if (int(i) % cols == cols - 1 || i + 1 == n.part.size()) x += "</m:mr>";
            }
            return x + "</m:m>";
        }
        case Kind::Func: return "<m:func><m:fName>" + operand(part(n, 0), d) + "</m:fName><m:e>" + operand(part(n, 1), d) + "</m:e></m:func>";
        case Kind::LimLow: return "<m:limLow><m:e>" + operand(part(n, 0), d) + "</m:e><m:lim>" + operand(part(n, 1), d) + "</m:lim></m:limLow>";
        default: break;
        }
        return {};
    }
};

class Reader {
public:
    explicit Reader(const XmlTree &t) : m_t(t) {}
    bool ok = true;
    Seq seqOf(int node, int depth)
    {
        Seq out;
        if (node < 0) return out;
        for (int c = m_t.firstChild(node); c >= 0 && ok; c = m_t.next(c)) item(c, depth, &out);
        return out;
    }

private:
    const XmlTree &m_t;
    int m_nodes = 0;
    void fail() { ok = false; }
    int kid(int node, const char *name) const
    {
        if (node < 0) return -1;
        for (int c = m_t.firstChild(node); c >= 0; c = m_t.next(c))
            if (m_t.ns(c) != NsW && m_t.localName(c) == name) return c;
        return -1;
    }
    // m:val of a child (the fallback when there is none).
    QString valueOf(int node, const char *name, const QString &fallback, bool *present = nullptr) const
    {
        const int c = kid(node, name);
        if (present) *present = c >= 0;
        if (c < 0) return fallback;
        QString v = m_t.attr(c, "val", m_t.ns(c));
        if (v.isEmpty()) v = m_t.attrAny(c, "val");
        return v;
    }
    // A property that is on unless it says off (`otherwise` when it is not there).
    bool flag(int node, const char *name, bool otherwise) const
    {
        bool present = false;
        const QString v = valueOf(node, name, QString(), &present).toLower();
        if (!present) return otherwise;
        return !(v == QLatin1String("0") || v == QLatin1String("false") || v == QLatin1String("off"));
    }
    static bool oneCharacter(const QString &s) { return s.isEmpty() || s.size() == 1 || (s.size() == 2 && s[0].isHighSurrogate() && s[1].isLowSurrogate()); }
    Seq part(int node, const char *name, int depth) { return seqOf(kid(node, name), depth); }

    void item(int c, int depth, Seq *out)
    {
        if (++m_nodes > kMostNodes) return fail();
        const QByteArray name = m_t.localName(c);
        if (m_t.ns(c) == NsW) return fail();
        if (name.endsWith("Pr")) return;
        if (name == "r") return run(c, out);
        if (depth + 1 > kMostEquationDepth) return fail();
        const int d = depth + 1;
        Node n;
        if (name == "f") {
            const QString type = valueOf(kid(c, "fPr"), "type", QStringLiteral("bar"));
            if (!type.isEmpty() && type != QLatin1String("bar")) return fail();
            n = structure(Kind::Frac, {part(c, "num", d), part(c, "den", d)});
        } else if (name == "sSup" || name == "sSub" || name == "sSubSup") {
            n = structure(Kind::Script, {part(c, "e", d), part(c, "sub", d), part(c, "sup", d)});
            n.hasA = name != "sSup";
            n.hasB = name != "sSub";
        } else if (name == "rad") {
            const bool hidden = flag(kid(c, "radPr"), "degHide", false);
            n = structure(Kind::Rad, {hidden ? Seq() : part(c, "deg", d), part(c, "e", d)});
            n.hasA = !hidden;
        } else if (name == "nary") {
            const int pr = kid(c, "naryPr");
            bool present = false;
            n = structure(Kind::Nary, {Seq(), Seq(), Seq()});
            n.text = valueOf(pr, "chr", QString(QChar(0x222B)), &present);
            const NaryOp *op = naryByChar(n.text);
            if (!op) return fail();
            const QString loc = valueOf(pr, "limLoc", QString());
            if (!loc.isEmpty()) n.limits = loc == QLatin1String("subSup") ? (op->beside ? 0 : 2) : (op->beside ? 1 : 0);
            n.hasA = !flag(pr, "subHide", false);
            n.hasB = !flag(pr, "supHide", false);
            n.part[0] = n.hasA ? part(c, "sub", d) : Seq();
            n.part[1] = n.hasB ? part(c, "sup", d) : Seq();
            n.part[2] = part(c, "e", d);
        } else if (name == "d") {
            const int pr = kid(c, "dPr");
            if (!flag(pr, "grow", true) || kid(pr, "shp") >= 0) return fail();
            n.kind = Kind::Delim;
            n.operand = true;
            n.text = valueOf(pr, "begChr", QStringLiteral("("));
            n.text2 = valueOf(pr, "endChr", QStringLiteral(")"));
            n.text3 = valueOf(pr, "sepChr", QStringLiteral("|"));
            if (!oneCharacter(n.text) || !oneCharacter(n.text2) || !oneCharacter(n.text3)) return fail();
            for (int e = m_t.firstChild(c); e >= 0 && ok; e = m_t.next(e))
                if (m_t.ns(e) != NsW && m_t.localName(e) == "e") n.part.push_back(seqOf(e, d));
            if (n.part.empty()) return fail();
        } else if (name == "acc") {
            n = structure(Kind::Accent, {part(c, "e", d)});
            n.text = valueOf(kid(c, "accPr"), "chr", QString(QChar(0x302)));
            if (n.text.isEmpty() || !oneCharacter(n.text)) return fail();
        } else if (name == "bar") {
            n = structure(Kind::Bar, {part(c, "e", d)});
            n.hasA = valueOf(kid(c, "barPr"), "pos", QStringLiteral("bot")) == QLatin1String("top");
        } else if (name == "m") {
            n.kind = Kind::Matrix;
            n.operand = true;
            int rows = 0;
            std::vector<std::vector<Seq>> grid;
            for (int r = m_t.firstChild(c); r >= 0 && ok; r = m_t.next(r)) {
                if (m_t.ns(r) == NsW || m_t.localName(r) != "mr") continue;
                if (++rows > kMostMatrixSide) return fail();
                std::vector<Seq> row;
                for (int e = m_t.firstChild(r); e >= 0 && ok; e = m_t.next(e)) {
                    if (m_t.ns(e) == NsW || m_t.localName(e) != "e") continue;
                    if (int(row.size()) >= kMostMatrixSide) return fail();
                    row.push_back(seqOf(e, d));
                }
                n.cols = std::max(n.cols, int(row.size()));
                grid.push_back(std::move(row));
            }
            if (!ok || grid.empty() || n.cols == 0 || int(grid.size()) * n.cols > kMostMatrixCells) return fail();
            for (auto &row : grid) {
                row.resize(size_t(n.cols));
                for (Seq &cell : row) n.part.push_back(std::move(cell));
            }
        } else if (name == "func") {
            n = structure(Kind::Func, {part(c, "fName", d), part(c, "e", d)});
        } else if (name == "limLow") {
            n = structure(Kind::LimLow, {part(c, "e", d), part(c, "lim", d)});
        } else {
            return fail();
        }
        if (ok) out->push_back(std::move(n));
    }

    void run(int c, Seq *out)
    {
        Node n;
        bool normal = false;
        QString text;
        for (int k = m_t.firstChild(c); k >= 0; k = m_t.next(k)) {
            const QByteArray name = m_t.localName(k);
            if (name == "t") {
                QString s = m_t.text(k);
                if (m_t.attrAny(k, "space") != QLatin1String("preserve")) s = s.trimmed();
                text += s;
                if (text.size() > kMostEquationInput * 4) return fail();
            } else if (name == "rPr" && m_t.ns(k) == NsW) {
                continue;
            } else if (name == "rPr") {
                for (int p = m_t.firstChild(k); p >= 0; p = m_t.next(p)) {
                    const QByteArray pn = m_t.localName(p);
                    if (pn == "nor") {
                        normal = flag(k, "nor", true);
                    } else if (pn == "sty") {
                        const QString v = valueOf(k, "sty", QString());
                        if (v == QLatin1String("p")) n.style = 'p';
                        else if (v == QLatin1String("b")) n.style = 'b';
                        else if (!v.isEmpty() && v != QLatin1String("i")) return fail();
                    } else if (pn != "aln" && pn != "ctrlPr") {
                        return fail();
                    }
                }
            } else if (name != "ctrlPr") {
                return fail();
            }
        }
        if (text.isEmpty()) return;   // (the run that holds the place of an empty operand says nothing)
        if (normal) n.style = 'n';
        n.kind = Kind::Run;
        n.text = text;
        out->push_back(std::move(n));
    }
};

struct Sym {
    const char *name;
    char32_t cp;
};
// The names for symbols besides those equationSymbol knows (the first of each character is the one printed).
const Sym kExtraSymbols[] = {{"Rightarrow", 0x21D2}, {"Leftarrow", 0x21D0}, {"Leftrightarrow", 0x21D4}, {"mapsto", 0x21A6}, {"ll", 0x226A}, {"gg", 0x226B}, {"subseteq", 0x2286},
                             {"supset", 0x2283}, {"supseteq", 0x2287}, {"setminus", 0x2216}, {"emptyset", 0x2205}, {"hbar", 0x210F}, {"ell", 0x2113}, {"prime", 0x2032}, {"neg", 0xAC},
                             {"wedge", 0x2227}, {"vee", 0x2228}, {"oplus", 0x2295}, {"otimes", 0x2297}, {"perp", 0x22A5}, {"parallel", 0x2225}, {"cong", 0x2245}, {"simeq", 0x2243},
                             {"ni", 0x220B}, {"aleph", 0x2135}, {"vartheta", 0x3D1}, {"varphi", 0x3D5}, {"varepsilon", 0x3F5}, {"varrho", 0x3F1}, {"varsigma", 0x3C2}, {"Upsilon", 0x3A5},
                             {"star", 0x22C6}, {"circ", 0x2218}, {"bullet", 0x2219}, {"dots", 0x2026}, {"vdots", 0x22EE}, {"ddots", 0x22F1}, {"langle", 0x27E8}, {"rangle", 0x27E9},
                             {"lfloor", 0x230A}, {"rfloor", 0x230B}, {"lceil", 0x2308}, {"rceil", 0x2309}, {"uparrow", 0x2191}, {"downarrow", 0x2193}, {"Uparrow", 0x21D1}, {"Downarrow", 0x21D3},
                             {"backslash", 0x5C}, {"Vert", 0x2016}, {"therefore", 0x2234}, {"because", 0x2235}, {"square", 0x25A1}, {"triangle", 0x25B3}, {"prec", 0x227A}, {"succ", 0x227B}};
// The names of equationSymbol, canonical ones first.
const char *const kOldSymbols[] = {"alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta", "theta", "iota", "kappa", "lambda", "mu", "nu", "xi", "pi", "rho", "sigma", "tau", "upsilon", "phi", "chi",
                                   "psi", "omega", "Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma", "Phi", "Psi", "Omega", "infty", "pm", "mp", "times", "div", "cdot", "leq", "geq", "neq",
                                   "approx", "equiv", "partial", "nabla", "rightarrow", "leftarrow", "leftrightarrow", "in", "notin", "subset", "cup", "cap", "forall", "exists", "ldots", "cdots",
                                   "degree", "angle", "sim", "propto", "le", "ge", "ne", "to"};

const NaryOp kNary[] = {{"sum", 0x2211, false}, {"prod", 0x220F, false}, {"coprod", 0x2210, false}, {"int", 0x222B, true}, {"iint", 0x222C, true}, {"iiint", 0x222D, true},
                        {"oint", 0x222E, true}, {"oiint", 0x222F, true}, {"oiiint", 0x2230, true}, {"bigcup", 0x22C3, false}, {"bigcap", 0x22C2, false}, {"bigvee", 0x22C1, false},
                        {"bigwedge", 0x22C0, false}};

const struct {
    const char *name;
    ushort mark;
} kAccents[] = {{"hat", 0x302}, {"check", 0x30C}, {"tilde", 0x303}, {"acute", 0x301}, {"grave", 0x300}, {"breve", 0x306}, {"dot", 0x307}, {"ddot", 0x308}, {"bar", 0x305}, {"vec", 0x20D7}};

} // namespace

Node run(const QString &text, char style, bool operand)
{
    Node n;
    n.kind = Kind::Run;
    n.text = text;
    n.style = style;
    n.operand = operand;
    return n;
}

Node structure(Kind kind, std::vector<Seq> parts)
{
    Node n;
    n.kind = kind;
    n.part = std::move(parts);
    n.operand = true;
    return n;
}

QString trimmedText(const QString &text)
{
    qsizetype a = 0, b = text.size();
    while (a < b && text[a].isSpace()) ++a;
    while (b > a && text[b - 1].isSpace()) {
        qsizetype slashes = 0;
        for (qsizetype i = b - 2; i >= a && text[i] == QLatin1Char('\\'); --i) ++slashes;
        if (slashes % 2 == 1) break;
        --b;
    }
    return text.mid(a, b - a);
}

QByteArray emptyRun() { return "<m:r><m:t></m:t></m:r>"; }

QByteArray write(const Seq &items)
{
    Writer w;
    const QByteArray body = w.seq(items, 0);
    if (!w.ok || body.isEmpty()) return {};
    return QByteArray("<m:oMath ") + kMathNs + ">" + body + "</m:oMath>";
}

bool read(const QByteArray &markup, Seq *items)
{
    if (markup.size() > kMostMarkupBytes * 4) return false;
    XmlTree t;
    if (!t.parse("<x>" + markup + "</x>", nullptr)) return false;
    int n = t.firstChild(t.root());
    if (n < 0 || t.ns(n) == NsW) return false;
    if (t.localName(n) == "oMathPara") {
        int only = -1;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            if (t.ns(c) == NsW) return false;
            if (t.localName(c) == "oMath") {
                if (only >= 0) return false;
                only = c;
            } else if (!t.localName(c).endsWith("Pr")) {
                return false;
            }
        }
        n = only;
    }
    if (n < 0 || t.localName(n) != "oMath") return false;
    Reader r(t);
    Seq out = r.seqOf(n, 0);
    if (!r.ok) return false;
    *items = std::move(out);
    return true;
}

bool isFunctionName(const QString &name)
{
    static const QStringList names = {QStringLiteral("sin"), QStringLiteral("cos"), QStringLiteral("tan"), QStringLiteral("cot"), QStringLiteral("sec"), QStringLiteral("csc"), QStringLiteral("arcsin"),
                                      QStringLiteral("arccos"), QStringLiteral("arctan"), QStringLiteral("sinh"), QStringLiteral("cosh"), QStringLiteral("tanh"), QStringLiteral("coth"),
                                      QStringLiteral("log"), QStringLiteral("ln"), QStringLiteral("lg"), QStringLiteral("exp"), QStringLiteral("lim"), QStringLiteral("min"), QStringLiteral("max"),
                                      QStringLiteral("sup"), QStringLiteral("inf"), QStringLiteral("limsup"), QStringLiteral("liminf"), QStringLiteral("det"), QStringLiteral("dim"),
                                      QStringLiteral("gcd"), QStringLiteral("ker"), QStringLiteral("deg"), QStringLiteral("arg"), QStringLiteral("Pr"), QStringLiteral("hom")};
    return names.contains(name);
}

bool isLimitFunction(const QString &name)
{
    return name == QLatin1String("lim") || name == QLatin1String("max") || name == QLatin1String("min") || name == QLatin1String("sup") || name == QLatin1String("inf") || name == QLatin1String("limsup")
           || name == QLatin1String("liminf");
}

QString symbolByName(const QString &name)
{
    QString s = equationSymbol(name);
    if (!s.isEmpty()) return s;
    static const QHash<QString, char32_t> extra = [] {
        QHash<QString, char32_t> h;
        for (const Sym &e : kExtraSymbols) h.insert(QString::fromLatin1(e.name), e.cp);
        return h;
    }();
    const auto found = extra.constFind(name);
    return found == extra.constEnd() ? QString() : QString::fromUcs4(&*found, 1);
}

QString symbolName(uint ch)
{
    static const QHash<uint, QString> names = [] {
        QHash<uint, QString> h;
        for (const char *n : kOldSymbols) {
            const QString s = equationSymbol(QString::fromLatin1(n));
            if (!s.isEmpty() && !h.contains(s.toUcs4().first())) h.insert(s.toUcs4().first(), QString::fromLatin1(n));
        }
        for (const Sym &e : kExtraSymbols)
            if (!h.contains(e.cp)) h.insert(e.cp, QString::fromLatin1(e.name));
        return h;
    }();
    return names.value(ch);
}

QString accentName(ushort mark)
{
    for (const auto &a : kAccents)
        if (a.mark == mark) return QString::fromLatin1(a.name);
    return QString();
}

ushort accentMark(const QString &name)
{
    for (const auto &a : kAccents)
        if (name == QLatin1String(a.name)) return a.mark;
    return 0;
}

const NaryOp *naryByChar(const QString &chr)
{
    for (const NaryOp &o : kNary)
        if (chr.size() == 1 && chr[0].unicode() == o.chr) return &o;
    return nullptr;
}

const NaryOp *naryByName(const QString &name)
{
    for (const NaryOp &o : kNary)
        if (name == QLatin1String(o.latex)) return &o;
    return nullptr;
}

bool naryBeside(const Node &n)
{
    const NaryOp *o = naryByChar(n.text);
    return o && o->beside;
}

} // namespace jp::office::wml::mathtree

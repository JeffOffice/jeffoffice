// FILTERXML(xml, xpath): the values an XPath 1.0 expression picks out of a text of XML. The XML is read into a small tree (no document type, so nothing is fetched or expanded), and the
// expression is worked out over it: location paths with the usual axes, name tests, predicates, the operators, and the common functions. Everything a text can multiply is bounded.
#include "calc/machine.h"

#include <QHash>
#include <QXmlStreamReader>

#include <algorithm>
#include <cmath>
#include <memory>

namespace jp::calc {

namespace {

constexpr int kMaxXmlChars = 1000000;
constexpr int kMaxXmlNodes = 100000;
constexpr int kMaxXmlDepth = 200;
constexpr qint64 kMaxXPathWork = 3000000;
constexpr int kMaxXPathDepth = 64;

struct XNode {
    enum Kind : quint8 { Root, Element, Text, Attr } kind = Element;
    QString name, text;       // an element's or attribute's qualified name; the text of a text node or an attribute
    int parent = -1;
    std::vector<int> children;   // elements and texts
    std::vector<int> attrs;
};

struct XDoc {
    std::vector<XNode> nodes;
    bool parse(const QString &xml)
    {
        if (xml.size() > kMaxXmlChars) return false;
        nodes.reserve(256);
        XNode root;
        root.kind = XNode::Root;
        nodes.push_back(root);
        QXmlStreamReader in(xml);
        int at = 0, depth = 0;
        bool sawElement = false;
        while (!in.atEnd()) {
            const QXmlStreamReader::TokenType tt = in.readNext();
            if (nodes.size() > size_t(kMaxXmlNodes)) return false;
            switch (tt) {
            case QXmlStreamReader::DTD: return false;
            case QXmlStreamReader::EntityReference: return false;
            case QXmlStreamReader::StartElement: {
                if (++depth > kMaxXmlDepth) return false;
                if (at == 0 && sawElement) return false;
                sawElement = true;
                XNode e;
                e.name = in.qualifiedName().toString();
                e.parent = at;
                const int id = int(nodes.size());
                nodes.push_back(e);
                nodes[size_t(at)].children.push_back(id);
                for (const QXmlStreamAttribute &a : in.attributes()) {
                    XNode an;
                    an.kind = XNode::Attr;
                    an.name = a.qualifiedName().toString();
                    an.text = a.value().toString();
                    an.parent = id;
                    nodes.push_back(an);
                    nodes[size_t(id)].attrs.push_back(int(nodes.size()) - 1);
                }
                at = id;
                break;
            }
            case QXmlStreamReader::EndElement:
                --depth;
                at = nodes[size_t(at)].parent;
                break;
            default: break;
            }
            if (tt == QXmlStreamReader::Characters && at != 0 && !in.isWhitespace()) {
                XNode t;
                t.kind = XNode::Text;
                t.text = in.text().toString();
                t.parent = at;
                nodes.push_back(t);
                nodes[size_t(at)].children.push_back(int(nodes.size()) - 1);
            }
        }
        return !in.hasError() && sawElement && depth == 0;
    }
    void stringValue(int id, QString *out, int depth = 0) const
    {
        const XNode &n = nodes[size_t(id)];
        if (n.kind == XNode::Text || n.kind == XNode::Attr) {
            *out += n.text;
            return;
        }
        if (depth > kMaxXmlDepth) return;
        for (int c : n.children) stringValue(c, out, depth + 1);
    }
    QString stringValue(int id) const
    {
        QString s;
        stringValue(id, &s);
        return s;
    }
};

// An XPath value.
struct XVal {
    enum Kind : quint8 { Nodes, Str, Num, Bool } kind = Nodes;
    std::vector<int> nodes;   // in document order, once each
    QString str;
    double num = 0;
    bool b = false;
};

struct Fail {};

class XPath {
public:
    XPath(const XDoc &doc, const QString &text) : m_doc(doc), m_s(text) {}

    // The value of the expression with the root as the context.
    XVal run()
    {
        m_tok = 0;
        tokenize();
        XVal v = expr(0, 1, 1, 0);
        if (m_tok != m_toks.size()) throw Fail();
        return v;
    }

    qint64 work = 0;

private:
    enum TT : quint8 { Name, Num, Lit, Sym, End };
    struct Tok {
        TT t = End;
        QString s;
        double n = 0;
    };

    const XDoc &m_doc;
    QString m_s;
    std::vector<Tok> m_toks;
    size_t m_tok = 0;

    void charge(qint64 n = 1)
    {
        work += n;
        if (work > kMaxXPathWork) throw Fail();
    }

    static bool nameStart(QChar c) { return c.isLetter() || c == QLatin1Char('_'); }
    static bool nameChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('-') || c == QLatin1Char('.') || c == QLatin1Char(':'); }

    void tokenize()
    {
        m_toks.clear();
        int i = 0;
        const int n = int(m_s.size());
        while (i < n) {
            const QChar c = m_s[i];
            if (c.isSpace()) {
                ++i;
            } else if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
                const int end = m_s.indexOf(c, i + 1);
                if (end < 0) throw Fail();
                m_toks.push_back({Lit, m_s.mid(i + 1, end - i - 1), 0});
                i = end + 1;
            } else if (c.isDigit() || (c == QLatin1Char('.') && i + 1 < n && m_s[i + 1].isDigit())) {
                int j = i;
                while (j < n && (m_s[j].isDigit() || m_s[j] == QLatin1Char('.'))) ++j;
                bool ok = false;
                const double v = m_s.mid(i, j - i).toDouble(&ok);
                if (!ok) throw Fail();
                m_toks.push_back({Num, QString(), v});
                i = j;
            } else if (nameStart(c)) {
                int j = i + 1;
                while (j < n && nameChar(m_s[j])) {
                    // "a::b" is an axis, not a name with colons
                    if (m_s[j] == QLatin1Char(':') && j + 1 < n && m_s[j + 1] == QLatin1Char(':')) break;
                    ++j;
                }
                // a name ending in "-" or "." that is followed by a space is kept as written: only a name that is cut at an operator matters here
                m_toks.push_back({Name, m_s.mid(i, j - i), 0});
                i = j;
            } else {
                static const char *const two[] = {"//", "::", "..", "!=", "<=", ">="};
                bool matched = false;
                for (const char *t : two)
                    if (m_s.mid(i, 2) == QLatin1String(t)) {
                        m_toks.push_back({Sym, QString::fromLatin1(t), 0});
                        i += 2;
                        matched = true;
                        break;
                    }
                if (matched) continue;
                if (QStringLiteral("/.@*()[],|=<>+-").contains(c)) {
                    m_toks.push_back({Sym, QString(c), 0});
                    ++i;
                } else {
                    throw Fail();
                }
            }
            if (m_toks.size() > 4096) throw Fail();
        }
    }

    const Tok *peek(size_t ahead = 0) const { return m_tok + ahead < m_toks.size() ? &m_toks[m_tok + ahead] : nullptr; }
    bool isSym(const char *s, size_t ahead = 0) const
    {
        const Tok *t = peek(ahead);
        return t && t->t == Sym && t->s == QLatin1String(s);
    }
    bool isName(const char *s, size_t ahead = 0) const
    {
        const Tok *t = peek(ahead);
        return t && t->t == Name && t->s == QLatin1String(s);
    }

    // ---- conversions
    double toNum(const XVal &v) const
    {
        switch (v.kind) {
        case XVal::Num: return v.num;
        case XVal::Bool: return v.b ? 1 : 0;
        case XVal::Str: return strNum(v.str);
        case XVal::Nodes: return v.nodes.empty() ? std::nan("") : strNum(m_doc.stringValue(v.nodes.front()));
        }
        return 0;
    }
    static double strNum(const QString &s)
    {
        bool ok = false;
        const QString t = s.trimmed();
        if (t.isEmpty() || t.contains(QLatin1Char('e'), Qt::CaseInsensitive) || t.startsWith(QLatin1Char('+'))) return std::nan("");
        const double v = t.toDouble(&ok);
        return ok ? v : std::nan("");
    }
    QString toStr(const XVal &v) const
    {
        switch (v.kind) {
        case XVal::Str: return v.str;
        case XVal::Bool: return v.b ? QStringLiteral("true") : QStringLiteral("false");
        case XVal::Num: return numStr(v.num);
        case XVal::Nodes: return v.nodes.empty() ? QString() : m_doc.stringValue(v.nodes.front());
        }
        return {};
    }
    static QString numStr(double d)
    {
        if (std::isnan(d)) return QStringLiteral("NaN");
        if (std::isinf(d)) return d > 0 ? QStringLiteral("Infinity") : QStringLiteral("-Infinity");
        if (d == std::floor(d) && std::fabs(d) < 1e15) return QString::number(qint64(d));
        return QString::number(d, 'g', 15);
    }
    static bool toBool(const XVal &v)
    {
        switch (v.kind) {
        case XVal::Bool: return v.b;
        case XVal::Num: return v.num != 0 && !std::isnan(v.num);
        case XVal::Str: return !v.str.isEmpty();
        case XVal::Nodes: return !v.nodes.empty();
        }
        return false;
    }
    static XVal ofBool(bool b)
    {
        XVal v;
        v.kind = XVal::Bool;
        v.b = b;
        return v;
    }
    static XVal ofNum(double d)
    {
        XVal v;
        v.kind = XVal::Num;
        v.num = d;
        return v;
    }
    static XVal ofStr(const QString &s)
    {
        XVal v;
        v.kind = XVal::Str;
        v.str = s;
        return v;
    }

    bool compare(const QString &op, const XVal &a, const XVal &b)
    {
        // a node set is compared by the string value of each of its nodes
        if (a.kind == XVal::Nodes || b.kind == XVal::Nodes) {
            if (a.kind == XVal::Nodes && b.kind == XVal::Nodes) {
                for (int x : a.nodes)
                    for (int y : b.nodes) {
                        charge();
                        if (compare(op, ofStr(m_doc.stringValue(x)), ofStr(m_doc.stringValue(y)))) return true;
                    }
                return false;
            }
            const bool left = a.kind == XVal::Nodes;
            const XVal &set = left ? a : b;
            const XVal &other = left ? b : a;
            if (other.kind == XVal::Bool) return compare(op, left ? ofBool(!set.nodes.empty()) : other, left ? other : ofBool(!set.nodes.empty()));
            for (int x : set.nodes) {
                charge();
                const XVal one = other.kind == XVal::Num ? ofNum(strNum(m_doc.stringValue(x))) : ofStr(m_doc.stringValue(x));
                if (left ? compare(op, one, other) : compare(op, other, one)) return true;
            }
            return false;
        }
        if (op == QLatin1String("=") || op == QLatin1String("!=")) {
            bool eq;
            if (a.kind == XVal::Bool || b.kind == XVal::Bool) eq = toBool(a) == toBool(b);
            else if (a.kind == XVal::Num || b.kind == XVal::Num) eq = toNum(a) == toNum(b);
            else eq = toStr(a) == toStr(b);
            return op == QLatin1String("=") ? eq : !eq;
        }
        const double x = toNum(a), y = toNum(b);
        if (op == QLatin1String("<")) return x < y;
        if (op == QLatin1String("<=")) return x <= y;
        if (op == QLatin1String(">")) return x > y;
        return x >= y;
    }

    // ---- expressions: the context node, its position and the size of the set it is in
    XVal expr(int node, int pos, int size, int depth)
    {
        if (depth > kMaxXPathDepth) throw Fail();
        return orExpr(node, pos, size, depth + 1);
    }
    XVal orExpr(int node, int pos, int size, int depth)
    {
        XVal v = andExpr(node, pos, size, depth);
        while (isName("or")) {
            ++m_tok;
            XVal w = andExpr(node, pos, size, depth);
            v = ofBool(toBool(v) || toBool(w));
        }
        return v;
    }
    XVal andExpr(int node, int pos, int size, int depth)
    {
        XVal v = eqExpr(node, pos, size, depth);
        while (isName("and")) {
            ++m_tok;
            XVal w = eqExpr(node, pos, size, depth);
            v = ofBool(toBool(v) && toBool(w));
        }
        return v;
    }
    XVal eqExpr(int node, int pos, int size, int depth)
    {
        XVal v = relExpr(node, pos, size, depth);
        while (isSym("=") || isSym("!=")) {
            const QString op = m_toks[m_tok++].s;
            XVal w = relExpr(node, pos, size, depth);
            v = ofBool(compare(op, v, w));
        }
        return v;
    }
    XVal relExpr(int node, int pos, int size, int depth)
    {
        XVal v = addExpr(node, pos, size, depth);
        while (isSym("<") || isSym("<=") || isSym(">") || isSym(">=")) {
            const QString op = m_toks[m_tok++].s;
            XVal w = addExpr(node, pos, size, depth);
            v = ofBool(compare(op, v, w));
        }
        return v;
    }
    XVal addExpr(int node, int pos, int size, int depth)
    {
        XVal v = mulExpr(node, pos, size, depth);
        while (isSym("+") || isSym("-")) {
            const bool plus = m_toks[m_tok++].s == QLatin1String("+");
            XVal w = mulExpr(node, pos, size, depth);
            v = ofNum(plus ? toNum(v) + toNum(w) : toNum(v) - toNum(w));
        }
        return v;
    }
    XVal mulExpr(int node, int pos, int size, int depth)
    {
        XVal v = unaryExpr(node, pos, size, depth);
        for (;;) {
            if (isSym("*")) {
                ++m_tok;
                XVal w = unaryExpr(node, pos, size, depth);
                v = ofNum(toNum(v) * toNum(w));
            } else if (isName("div")) {
                ++m_tok;
                XVal w = unaryExpr(node, pos, size, depth);
                v = ofNum(toNum(v) / toNum(w));
            } else if (isName("mod")) {
                ++m_tok;
                XVal w = unaryExpr(node, pos, size, depth);
                v = ofNum(std::fmod(toNum(v), toNum(w)));
            } else {
                return v;
            }
        }
    }
    XVal unaryExpr(int node, int pos, int size, int depth)
    {
        if (isSym("-")) {
            ++m_tok;
            if (depth > kMaxXPathDepth) throw Fail();
            return ofNum(-toNum(unaryExpr(node, pos, size, depth + 1)));
        }
        return unionExpr(node, pos, size, depth);
    }
    XVal unionExpr(int node, int pos, int size, int depth)
    {
        XVal v = pathExpr(node, pos, size, depth);
        while (isSym("|")) {
            ++m_tok;
            XVal w = pathExpr(node, pos, size, depth);
            if (v.kind != XVal::Nodes || w.kind != XVal::Nodes) throw Fail();
            v.nodes.insert(v.nodes.end(), w.nodes.begin(), w.nodes.end());
            std::sort(v.nodes.begin(), v.nodes.end());
            v.nodes.erase(std::unique(v.nodes.begin(), v.nodes.end()), v.nodes.end());
        }
        return v;
    }

    bool startsStep() const
    {
        const Tok *t = peek();
        if (!t) return false;
        if (t->t == Name) return true;
        return t->t == Sym && (t->s == QLatin1String(".") || t->s == QLatin1String("..") || t->s == QLatin1String("@") || t->s == QLatin1String("*"));
    }

    XVal pathExpr(int node, int pos, int size, int depth)
    {
        const Tok *t = peek();
        if (!t) throw Fail();
        XVal v;
        if (t->t == Lit) {
            ++m_tok;
            return ofStr(t->s);
        }
        if (t->t == Num) {
            ++m_tok;
            return ofNum(t->n);
        }
        if (t->t == Sym && t->s == QLatin1String("(")) {
            ++m_tok;
            v = expr(node, pos, size, depth);
            if (!isSym(")")) throw Fail();
            ++m_tok;
            if (!(isSym("/") || isSym("//"))) return v;
            if (v.kind != XVal::Nodes) throw Fail();
            return relativeSteps(std::move(v), depth);
        }
        // a function call: a name that is followed by "(" and is no node test
        if (t->t == Name && isSym("(", 1) && t->s != QLatin1String("text") && t->s != QLatin1String("node")) return function(node, pos, size, depth);
        // a location path
        v.kind = XVal::Nodes;
        if (isSym("/") || isSym("//")) {
            const bool deep = isSym("//");
            ++m_tok;
            v.nodes = {0};
            if (deep) {
                v = descendantOrSelf(v);
                if (!startsStep()) throw Fail();
                return relativeSteps(step(v, depth), depth);
            }
            if (!startsStep()) return v;   // "/" is the root
            return relativeSteps(step(v, depth), depth);
        }
        if (!startsStep()) throw Fail();
        v.nodes = {node};
        return relativeSteps(step(v, depth), depth);
    }

    XVal relativeSteps(XVal v, int depth)
    {
        while (isSym("/") || isSym("//")) {
            const bool deep = isSym("//");
            ++m_tok;
            if (v.kind != XVal::Nodes) throw Fail();
            if (deep) v = descendantOrSelf(v);
            if (!startsStep()) throw Fail();
            v = step(v, depth);
        }
        return v;
    }

    XVal descendantOrSelf(const XVal &in)
    {
        XVal out;
        out.kind = XVal::Nodes;
        std::vector<int> stack;
        for (int n : in.nodes) {
            charge();
            out.nodes.push_back(n);
            stack.assign(1, n);
            while (!stack.empty()) {
                const int cur = stack.back();
                stack.pop_back();
                for (int c : m_doc.nodes[size_t(cur)].children) {
                    charge();
                    if (m_doc.nodes[size_t(c)].kind == XNode::Element || m_doc.nodes[size_t(c)].kind == XNode::Text) out.nodes.push_back(c);
                    stack.push_back(c);
                }
            }
        }
        std::sort(out.nodes.begin(), out.nodes.end());
        out.nodes.erase(std::unique(out.nodes.begin(), out.nodes.end()), out.nodes.end());
        return out;
    }

    // One step of a path from each node of `in`: the axis, the node test, and the predicates.
    XVal step(const XVal &in, int depth)
    {
        enum Axis { Child, Desc, DescSelf, Parent, Ancestor, Self, Attribute, FollowingSibling, PrecedingSibling } axis = Child;
        QString test;
        bool isText = false, isNode = false, any = false;
        if (isSym(".")) {
            ++m_tok;
            axis = Self;
            isNode = true;
        } else if (isSym("..")) {
            ++m_tok;
            axis = Parent;
            isNode = true;
        } else {
            if (isSym("@")) {
                ++m_tok;
                axis = Attribute;
            } else if (peek() && peek()->t == Name && isSym("::", 1)) {
                const QString a = m_toks[m_tok].s;
                if (a == QLatin1String("child")) axis = Child;
                else if (a == QLatin1String("descendant")) axis = Desc;
                else if (a == QLatin1String("descendant-or-self")) axis = DescSelf;
                else if (a == QLatin1String("parent")) axis = Parent;
                else if (a == QLatin1String("ancestor")) axis = Ancestor;
                else if (a == QLatin1String("self")) axis = Self;
                else if (a == QLatin1String("attribute")) axis = Attribute;
                else if (a == QLatin1String("following-sibling")) axis = FollowingSibling;
                else if (a == QLatin1String("preceding-sibling")) axis = PrecedingSibling;
                else throw Fail();
                m_tok += 2;
            }
            if (isSym("*")) {
                ++m_tok;
                any = true;
            } else if (peek() && peek()->t == Name) {
                test = m_toks[m_tok++].s;
                if ((test == QLatin1String("text") || test == QLatin1String("node")) && isSym("(")) {
                    ++m_tok;
                    if (!isSym(")")) throw Fail();
                    ++m_tok;
                    (test == QLatin1String("text") ? isText : isNode) = true;
                }
            } else {
                throw Fail();
            }
        }
        std::vector<std::vector<int>> perNode;
        XVal out;
        out.kind = XVal::Nodes;
        // the predicates are read once, as positions in the token list, and worked out for each node
        const size_t predStart = m_tok;
        size_t predEnd = m_tok;
        {
            // find the end of the predicates by reading them against a scratch context
            size_t save = m_tok;
            while (isSym("[")) {
                int open = 0;
                size_t k = m_tok;
                for (; k < m_toks.size(); ++k) {
                    if (m_toks[k].t == Sym && m_toks[k].s == QLatin1String("[")) ++open;
                    else if (m_toks[k].t == Sym && m_toks[k].s == QLatin1String("]") && --open == 0) break;
                }
                if (k >= m_toks.size()) throw Fail();
                m_tok = k + 1;
            }
            predEnd = m_tok;
            m_tok = save;
        }
        for (int n : in.nodes) {
            charge();
            std::vector<int> cand;
            const XNode &nd = m_doc.nodes[size_t(n)];
            auto matches = [&](int id) {
                const XNode &c = m_doc.nodes[size_t(id)];
                if (isNode) return true;
                if (isText) return c.kind == XNode::Text;
                if (axis == Attribute) return c.kind == XNode::Attr && (any || c.name == test);
                return c.kind == XNode::Element && (any || c.name == test);
            };
            switch (axis) {
            case Child:
                for (int c : nd.children) {
                    charge();
                    if (matches(c)) cand.push_back(c);
                }
                break;
            case Attribute:
                for (int a : nd.attrs)
                    if (matches(a)) cand.push_back(a);
                break;
            case Self:
                if (matches(n) || isNode) cand.push_back(n);
                break;
            case Parent:
                if (nd.parent >= 0 && (isNode || matches(nd.parent))) cand.push_back(nd.parent);
                break;
            case Ancestor:
                for (int p = nd.parent; p >= 0; p = m_doc.nodes[size_t(p)].parent) {
                    charge();
                    if (isNode || matches(p)) cand.push_back(p);
                }
                std::reverse(cand.begin(), cand.end());
                break;
            case Desc:
            case DescSelf: {
                XVal one;
                one.kind = XVal::Nodes;
                one.nodes = {n};
                const XVal all = descendantOrSelf(one);
                for (int id : all.nodes) {
                    if (id == n && axis == Desc) continue;
                    if (matches(id) || (isNode && id == n)) cand.push_back(id);
                }
                break;
            }
            case FollowingSibling:
            case PrecedingSibling:
                if (nd.parent >= 0 && nd.kind != XNode::Attr) {
                    const std::vector<int> &sibs = m_doc.nodes[size_t(nd.parent)].children;
                    const auto it = std::find(sibs.begin(), sibs.end(), n);
                    if (it != sibs.end()) {
                        if (axis == FollowingSibling) {
                            for (auto k = it + 1; k != sibs.end(); ++k)
                                if (matches(*k)) cand.push_back(*k);
                        } else {
                            for (auto k = sibs.begin(); k != it; ++k)
                                if (matches(*k)) cand.push_back(*k);
                        }
                    }
                }
                break;
            }
            // the predicates, each narrowing the candidates in order
            m_tok = predStart;
            while (isSym("[")) {
                ++m_tok;
                const size_t exprStart = m_tok;
                std::vector<int> kept;
                const bool reverse = axis == Ancestor || axis == PrecedingSibling;
                for (size_t i = 0; i < cand.size(); ++i) {
                    charge();
                    m_tok = exprStart;
                    const int position = reverse ? int(cand.size() - i) : int(i + 1);
                    const XVal r = expr(cand[i], position, int(cand.size()), depth + 1);
                    if (!isSym("]")) throw Fail();
                    const bool keep = r.kind == XVal::Num ? r.num == double(position) : toBool(r);
                    if (keep) kept.push_back(cand[i]);
                }
                cand = std::move(kept);
                // move past this predicate
                int open = 1;
                size_t k = exprStart;
                for (; k < m_toks.size(); ++k) {
                    if (m_toks[k].t == Sym && m_toks[k].s == QLatin1String("[")) ++open;
                    else if (m_toks[k].t == Sym && m_toks[k].s == QLatin1String("]") && --open == 0) break;
                }
                if (k >= m_toks.size()) throw Fail();
                m_tok = k + 1;
            }
            out.nodes.insert(out.nodes.end(), cand.begin(), cand.end());
            if (qint64(out.nodes.size()) > kMaxXmlNodes * 4) throw Fail();
        }
        m_tok = predEnd;
        std::sort(out.nodes.begin(), out.nodes.end());
        out.nodes.erase(std::unique(out.nodes.begin(), out.nodes.end()), out.nodes.end());
        return out;
    }

    XVal function(int node, int pos, int size, int depth)
    {
        const QString name = m_toks[m_tok].s;
        m_tok += 2;   // the name and "("
        std::vector<XVal> args;
        if (!isSym(")")) {
            for (;;) {
                args.push_back(expr(node, pos, size, depth + 1));
                if (isSym(",")) {
                    ++m_tok;
                    continue;
                }
                break;
            }
        }
        if (!isSym(")")) throw Fail();
        ++m_tok;
        auto need = [&](size_t lo, size_t hi) {
            if (args.size() < lo || args.size() > hi) throw Fail();
        };
        auto strArg = [&](size_t i) { return i < args.size() ? toStr(args[i]) : m_doc.stringValue(node); };
        if (name == QLatin1String("count")) {
            need(1, 1);
            if (args[0].kind != XVal::Nodes) throw Fail();
            return ofNum(double(args[0].nodes.size()));
        }
        if (name == QLatin1String("sum")) {
            need(1, 1);
            if (args[0].kind != XVal::Nodes) throw Fail();
            double total = 0;
            for (int n : args[0].nodes) total += strNum(m_doc.stringValue(n));
            return ofNum(total);
        }
        if (name == QLatin1String("last")) return ofNum(size);
        if (name == QLatin1String("position")) return ofNum(pos);
        if (name == QLatin1String("true")) return ofBool(true);
        if (name == QLatin1String("false")) return ofBool(false);
        if (name == QLatin1String("not")) {
            need(1, 1);
            return ofBool(!toBool(args[0]));
        }
        if (name == QLatin1String("boolean")) {
            need(1, 1);
            return ofBool(toBool(args[0]));
        }
        if (name == QLatin1String("string")) {
            need(0, 1);
            return ofStr(strArg(0));
        }
        if (name == QLatin1String("number")) {
            need(0, 1);
            return ofNum(args.empty() ? strNum(m_doc.stringValue(node)) : toNum(args[0]));
        }
        if (name == QLatin1String("string-length")) {
            need(0, 1);
            return ofNum(double(strArg(0).size()));
        }
        if (name == QLatin1String("normalize-space")) {
            need(0, 1);
            return ofStr(strArg(0).simplified());
        }
        if (name == QLatin1String("contains")) {
            need(2, 2);
            return ofBool(toStr(args[0]).contains(toStr(args[1])));
        }
        if (name == QLatin1String("starts-with")) {
            need(2, 2);
            return ofBool(toStr(args[0]).startsWith(toStr(args[1])));
        }
        if (name == QLatin1String("substring-before") || name == QLatin1String("substring-after")) {
            need(2, 2);
            const QString a = toStr(args[0]), b = toStr(args[1]);
            const int at = a.indexOf(b);
            if (at < 0) return ofStr(QString());
            return ofStr(name == QLatin1String("substring-before") ? a.left(at) : a.mid(at + b.size()));
        }
        if (name == QLatin1String("substring")) {
            need(2, 3);
            const QString a = toStr(args[0]);
            const double from = std::round(toNum(args[1]));
            const double len = args.size() > 2 ? std::round(toNum(args[2])) : 1e18;
            if (std::isnan(from) || std::isnan(len)) return ofStr(QString());
            QString out;
            for (int i = 0; i < a.size(); ++i) {
                const double p = i + 1;
                if (p >= from && p < from + len) out += a[i];
            }
            return ofStr(out);
        }
        if (name == QLatin1String("concat")) {
            if (args.size() < 2) throw Fail();
            QString out;
            for (const XVal &a : args) out += toStr(a);
            if (out.size() > kMaxValueTextChars) throw Fail();
            return ofStr(out);
        }
        if (name == QLatin1String("translate")) {
            need(3, 3);
            QString s = toStr(args[0]);
            const QString from = toStr(args[1]), to = toStr(args[2]);
            QString out;
            for (const QChar ch : s) {
                const int at = from.indexOf(ch);
                if (at < 0) out += ch;
                else if (at < to.size()) out += to[at];
            }
            return ofStr(out);
        }
        if (name == QLatin1String("name") || name == QLatin1String("local-name")) {
            need(0, 1);
            int target = node;
            if (!args.empty()) {
                if (args[0].kind != XVal::Nodes) throw Fail();
                if (args[0].nodes.empty()) return ofStr(QString());
                target = args[0].nodes.front();
            }
            QString n = m_doc.nodes[size_t(target)].name;
            if (name == QLatin1String("local-name") && n.contains(QLatin1Char(':'))) n = n.mid(n.indexOf(QLatin1Char(':')) + 1);
            return ofStr(n);
        }
        if (name == QLatin1String("floor") || name == QLatin1String("ceiling") || name == QLatin1String("round")) {
            need(1, 1);
            const double d = toNum(args[0]);
            return ofNum(name == QLatin1String("floor") ? std::floor(d) : name == QLatin1String("ceiling") ? std::ceil(d) : std::floor(d + 0.5));
        }
        throw Fail();
    }
};

Value fnFilterXml(Call &c)
{
    QString xml, path;
    if (!c.text(0, &xml) || !c.text(1, &path)) return c.failure();
    XDoc doc;
    if (!c.machine().charge(qint64(xml.size()) / 8 + 1)) return c.error(ErrorKind::Value);
    if (!doc.parse(xml)) return c.error(ErrorKind::Value);
    XVal result;
    qint64 work = 0;
    try {
        XPath xpath(doc, path);
        result = xpath.run();
        work = xpath.work;
    } catch (const Fail &) {
        return c.error(ErrorKind::Value);
    }
    if (!c.machine().charge(work / 16)) return c.error(ErrorKind::Value);
    auto valueOf = [&](const QString &s) {
        bool ok = false;
        const QString t = s.trimmed();
        const double d = t.toDouble(&ok);
        if (ok && !t.isEmpty() && std::isfinite(d) && !t.contains(QLatin1Char('e'), Qt::CaseInsensitive) && !t.startsWith(QLatin1Char('+'))) return Value::number(d);
        return Value::text(s.size() > kMaxValueTextChars ? s.left(kMaxValueTextChars) : s);
    };
    switch (result.kind) {
    case XVal::Num: return std::isfinite(result.num) ? Value::number(result.num) : c.error(ErrorKind::Value);
    case XVal::Bool: return Value::boolean(result.b);
    case XVal::Str: return valueOf(result.str);
    case XVal::Nodes: break;
    }
    if (result.nodes.empty()) return c.error(ErrorKind::NA);
    if (result.nodes.size() == 1) return valueOf(doc.stringValue(result.nodes.front()));
    if (!c.machine().charge(qint64(result.nodes.size()))) return c.error(ErrorKind::Value);
    Value out = c.machine().newArray(int(result.nodes.size()), 1);
    if (!out.isArray()) return out;
    for (size_t i = 0; i < result.nodes.size(); ++i) {
        const Value v = valueOf(doc.stringValue(result.nodes[i]));
        if (v.isText() && !c.machine().chargeText(v.text().size())) return c.error(ErrorKind::Value);
        out.set(int(i), 0, v);
    }
    return out;
}

} // namespace

void registerXmlFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"FILTERXML", 2, 2, "vv", kFnFuture, fnFilterXml});
}

} // namespace jp::calc

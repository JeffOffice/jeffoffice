#include "sheet/pivotformula.h"

#include <QHash>

#include <cmath>
#include <limits>

namespace jp::office::sheet {

struct PivotFormula::Node {
    enum Kind : quint8 { Const, Str, Name, Unary, Binary, Percent, Call } kind = Const;
    double number = 0;
    QString text;   // a text, a name, an operator, or a function's name (upper case)
    QVector<int> args;
};

PivotFormula::PivotFormula() : m_nodes(new QVector<Node>) {}
PivotFormula::~PivotFormula() { delete m_nodes; }

namespace {

using V = PivotFormulaValue;

V errV(const char *e) { return V::err(QString::fromLatin1(e)); }

}   // namespace

struct Parser {
    const QString &s;
    PivotFormula &f;
    int pos = 0;
    int depth = 0;
    QString problem;

    bool fail(const QString &m)
    {
        if (problem.isEmpty()) problem = m;
        return false;
    }
    void skip()
    {
        while (pos < s.size() && s[pos].isSpace()) ++pos;
    }
    int add(PivotFormula::Node n)
    {
        if (f.m_nodes->size() >= kPivotFormulaMaxNodes) {
            fail(QStringLiteral("the formula is too long"));
            return -1;
        }
        f.m_nodes->append(std::move(n));
        return int(f.m_nodes->size()) - 1;
    }
    bool peek(const QString &t)
    {
        skip();
        return s.mid(pos, t.size()) == t;
    }
    int cmp()
    {
        int l = concat();
        while (l >= 0) {
            skip();
            QString op;
            for (const QString &c : {QStringLiteral("<="), QStringLiteral(">="), QStringLiteral("<>"), QStringLiteral("="), QStringLiteral("<"), QStringLiteral(">")})
                if (peek(c)) {
                    op = c;
                    break;
                }
            if (op.isEmpty()) break;
            pos += op.size();
            const int r = concat();
            if (r < 0) return -1;
            PivotFormula::Node n;
            n.kind = PivotFormula::Node::Binary;
            n.text = op;
            n.args = {l, r};
            l = add(std::move(n));
        }
        return l;
    }
    int concat()
    {
        int l = sum();
        while (l >= 0 && peek(QStringLiteral("&"))) {
            ++pos;
            const int r = sum();
            if (r < 0) return -1;
            PivotFormula::Node n;
            n.kind = PivotFormula::Node::Binary;
            n.text = QStringLiteral("&");
            n.args = {l, r};
            l = add(std::move(n));
        }
        return l;
    }
    int sum()
    {
        int l = product();
        while (l >= 0) {
            skip();
            if (pos >= s.size() || (s[pos] != QLatin1Char('+') && s[pos] != QLatin1Char('-'))) break;
            const QChar op = s[pos++];
            const int r = product();
            if (r < 0) return -1;
            PivotFormula::Node n;
            n.kind = PivotFormula::Node::Binary;
            n.text = QString(op);
            n.args = {l, r};
            l = add(std::move(n));
        }
        return l;
    }
    int product()
    {
        int l = power();
        while (l >= 0) {
            skip();
            if (pos >= s.size() || (s[pos] != QLatin1Char('*') && s[pos] != QLatin1Char('/'))) break;
            const QChar op = s[pos++];
            const int r = power();
            if (r < 0) return -1;
            PivotFormula::Node n;
            n.kind = PivotFormula::Node::Binary;
            n.text = QString(op);
            n.args = {l, r};
            l = add(std::move(n));
        }
        return l;
    }
    int power()
    {
        int l = unary();
        while (l >= 0 && peek(QStringLiteral("^"))) {
            ++pos;
            const int r = unary();
            if (r < 0) return -1;
            PivotFormula::Node n;
            n.kind = PivotFormula::Node::Binary;
            n.text = QStringLiteral("^");
            n.args = {l, r};
            l = add(std::move(n));
        }
        return l;
    }
    int unary()
    {
        skip();
        if (pos < s.size() && (s[pos] == QLatin1Char('-') || s[pos] == QLatin1Char('+'))) {
            const QChar op = s[pos++];
            if (++depth > kPivotFormulaMaxDepth) {
                fail(QStringLiteral("the formula is nested too deep"));
                return -1;
            }
            const int a = unary();
            --depth;
            if (a < 0) return -1;
            PivotFormula::Node n;
            n.kind = PivotFormula::Node::Unary;
            n.text = QString(op);
            n.args = {a};
            return add(std::move(n));
        }
        int p = primary();
        while (p >= 0 && peek(QStringLiteral("%"))) {
            ++pos;
            PivotFormula::Node n;
            n.kind = PivotFormula::Node::Percent;
            n.args = {p};
            p = add(std::move(n));
        }
        return p;
    }
    int primary()
    {
        skip();
        if (pos >= s.size()) {
            fail(QStringLiteral("the formula ends too soon"));
            return -1;
        }
        const QChar c = s[pos];
        if (c == QLatin1Char('(')) {
            ++pos;
            if (++depth > kPivotFormulaMaxDepth) {
                fail(QStringLiteral("the formula is nested too deep"));
                return -1;
            }
            const int e = cmp();
            --depth;
            if (e < 0) return -1;
            if (!peek(QStringLiteral(")"))) {
                fail(QStringLiteral("a bracket is not closed"));
                return -1;
            }
            ++pos;
            return e;
        }
        if (c.isDigit() || c == QLatin1Char('.')) {
            int e = pos;
            while (e < s.size() && (s[e].isDigit() || s[e] == QLatin1Char('.'))) ++e;
            if (e < s.size() && (s[e] == QLatin1Char('e') || s[e] == QLatin1Char('E'))) {
                int k = e + 1;
                if (k < s.size() && (s[k] == QLatin1Char('+') || s[k] == QLatin1Char('-'))) ++k;
                if (k < s.size() && s[k].isDigit()) {
                    while (k < s.size() && s[k].isDigit()) ++k;
                    e = k;
                }
            }
            bool ok = false;
            PivotFormula::Node n;
            n.number = s.mid(pos, e - pos).toDouble(&ok);
            if (!ok) {
                fail(QStringLiteral("a number is not well formed"));
                return -1;
            }
            pos = e;
            return add(std::move(n));
        }
        if (c == QLatin1Char('"')) {
            ++pos;
            QString t;
            for (;;) {
                if (pos >= s.size()) {
                    fail(QStringLiteral("a text is not closed"));
                    return -1;
                }
                if (s[pos] == QLatin1Char('"')) {
                    if (pos + 1 < s.size() && s[pos + 1] == QLatin1Char('"')) {
                        t += QLatin1Char('"');
                        pos += 2;
                        continue;
                    }
                    ++pos;
                    break;
                }
                t += s[pos++];
            }
            PivotFormula::Node n;
            n.kind = PivotFormula::Node::Str;
            n.text = t;
            return add(std::move(n));
        }
        QString name;
        bool quoted = false;
        if (c == QLatin1Char('\'')) {
            quoted = true;
            ++pos;
            for (;;) {
                if (pos >= s.size()) {
                    fail(QStringLiteral("a name is not closed"));
                    return -1;
                }
                if (s[pos] == QLatin1Char('\'')) {
                    if (pos + 1 < s.size() && s[pos + 1] == QLatin1Char('\'')) {
                        name += QLatin1Char('\'');
                        pos += 2;
                        continue;
                    }
                    ++pos;
                    break;
                }
                name += s[pos++];
            }
        } else if (c.isLetter() || c == QLatin1Char('_')) {
            while (pos < s.size() && (s[pos].isLetterOrNumber() || s[pos] == QLatin1Char('_') || s[pos] == QLatin1Char('.'))) name += s[pos++];
        } else {
            fail(QStringLiteral("the formula has something it does not know"));
            return -1;
        }
        if (!quoted && peek(QStringLiteral("("))) {
            ++pos;
            PivotFormula::Node n;
            n.kind = PivotFormula::Node::Call;
            n.text = name.toUpper();
            if (++depth > kPivotFormulaMaxDepth) {
                fail(QStringLiteral("the formula is nested too deep"));
                return -1;
            }
            if (!peek(QStringLiteral(")"))) {
                for (;;) {
                    const int a = cmp();
                    if (a < 0) return -1;
                    n.args.append(a);
                    if (peek(QStringLiteral(","))) {
                        ++pos;
                        continue;
                    }
                    break;
                }
            }
            --depth;
            if (!peek(QStringLiteral(")"))) {
                fail(QStringLiteral("a bracket is not closed"));
                return -1;
            }
            ++pos;
            return add(std::move(n));
        }
        PivotFormula::Node n;
        n.kind = PivotFormula::Node::Name;
        n.text = name;
        if (!quoted) {
            const QString up = name.toUpper();
            if (up == QLatin1String("TRUE") || up == QLatin1String("FALSE")) {
                n.kind = PivotFormula::Node::Call;
                n.text = up;
            }
        }
        if (n.kind == PivotFormula::Node::Name && !f.m_names.contains(name, Qt::CaseInsensitive)) f.m_names.append(name);
        return add(std::move(n));
    }
};

std::shared_ptr<const PivotFormula> PivotFormula::parse(const QString &text, QString *why)
{
    auto f = std::make_shared<PivotFormula>();
    QString t = text.trimmed();
    if (t.startsWith(QLatin1Char('='))) t = t.mid(1);
    auto fail = [&](const QString &m) -> std::shared_ptr<const PivotFormula> {
        if (why) *why = m;
        return nullptr;
    };
    if (t.isEmpty()) return fail(QStringLiteral("the formula is empty"));
    if (t.size() > kPivotFormulaMaxChars) return fail(QStringLiteral("the formula is too long"));
    Parser p{t, *f};
    f->m_root = p.cmp();
    if (f->m_root < 0) return fail(p.problem.isEmpty() ? QStringLiteral("the formula is not well formed") : p.problem);
    p.skip();
    if (p.pos < t.size()) return fail(QStringLiteral("the formula has something it does not know"));
    return f;
}

namespace {

using Node = PivotFormula::Node;

bool toNumber(const V &v, double *d, V *error)
{
    switch (v.kind) {
    case V::Number:
    case V::Bool: *d = v.number; return true;
    case V::Error: *error = v; return false;
    case V::Text: {
        bool ok = false;
        const double x = v.text.trimmed().toDouble(&ok);
        if (ok) {
            *d = x;
            return true;
        }
        *error = errV("#VALUE!");
        return false;
    }
    }
    return false;
}

V finite(double d)
{
    if (std::isnan(d) || std::isinf(d)) return errV("#NUM!");
    return V::num(d);
}

QString showNumber(const V &v)
{
    if (v.kind == V::Text) return v.text;
    if (v.kind == V::Bool) return v.number != 0 ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    return QString::number(v.number, 'g', 15);
}

}   // namespace

PivotFormulaValue PivotFormula::evaluate(const std::function<PivotFormulaValue(const QString &name)> &field) const
{
    std::function<V(int)> eval = [&](int at) -> V {
        const Node &n = (*m_nodes)[at];
        switch (n.kind) {
        case Node::Const: return V::num(n.number);
        case Node::Str: {
            V v;
            v.kind = V::Text;
            v.text = n.text;
            return v;
        }
        case Node::Name: return field(n.text);
        case Node::Percent: {
            double d;
            V e;
            const V a = eval(n.args[0]);
            if (!toNumber(a, &d, &e)) return e;
            return V::num(d / 100);
        }
        case Node::Unary: {
            double d;
            V e;
            const V a = eval(n.args[0]);
            if (!toNumber(a, &d, &e)) return e;
            return V::num(n.text == QLatin1String("-") ? -d : d);
        }
        case Node::Binary: {
            const V a = eval(n.args[0]);
            const V b = eval(n.args[1]);
            const QString &op = n.text;
            if (op == QLatin1String("&")) {
                if (a.kind == V::Error) return a;
                if (b.kind == V::Error) return b;
                V v;
                v.kind = V::Text;
                v.text = showNumber(a) + showNumber(b);
                return v;
            }
            if (op == QLatin1String("=") || op == QLatin1String("<>") || op == QLatin1String("<") || op == QLatin1String(">") || op == QLatin1String("<=") || op == QLatin1String(">=")) {
                if (a.kind == V::Error) return a;
                if (b.kind == V::Error) return b;
                int c;
                if (a.kind == V::Text && b.kind == V::Text) c = QString::compare(a.text, b.text, Qt::CaseInsensitive);
                else if (a.kind == V::Text) c = 1;   // (text is greater than any number)
                else if (b.kind == V::Text) c = -1;
                else c = a.number < b.number ? -1 : a.number > b.number ? 1 : 0;
                bool r = false;
                if (op == QLatin1String("=")) r = c == 0;
                else if (op == QLatin1String("<>")) r = c != 0;
                else if (op == QLatin1String("<")) r = c < 0;
                else if (op == QLatin1String(">")) r = c > 0;
                else if (op == QLatin1String("<=")) r = c <= 0;
                else r = c >= 0;
                V v;
                v.kind = V::Bool;
                v.number = r ? 1 : 0;
                return v;
            }
            double x, y;
            V e;
            if (!toNumber(a, &x, &e) || !toNumber(b, &y, &e)) return e;
            if (op == QLatin1String("+")) return finite(x + y);
            if (op == QLatin1String("-")) return finite(x - y);
            if (op == QLatin1String("*")) return finite(x * y);
            if (op == QLatin1String("/")) return y == 0 ? errV("#DIV/0!") : finite(x / y);
            if (x == 0 && y == 0) return errV("#NUM!");
            return finite(std::pow(x, y));
        }
        case Node::Call: {
            const QString &fn = n.text;
            const int na = int(n.args.size());
            if (fn == QLatin1String("TRUE") || fn == QLatin1String("FALSE")) {
                V v;
                v.kind = V::Bool;
                v.number = fn == QLatin1String("TRUE");
                return v;
            }
            if (fn == QLatin1String("PI")) return V::num(M_PI);
            if (fn == QLatin1String("IF")) {
                if (na < 2 || na > 3) return errV("#VALUE!");
                const V c = eval(n.args[0]);
                double d;
                V e;
                if (!toNumber(c, &d, &e)) return e;
                if (d != 0) return eval(n.args[1]);
                if (na == 3) return eval(n.args[2]);
                V v;
                v.kind = V::Bool;
                return v;
            }
            if (fn == QLatin1String("IFERROR")) {
                if (na != 2) return errV("#VALUE!");
                const V a = eval(n.args[0]);
                return a.kind == V::Error ? eval(n.args[1]) : a;
            }
            QVector<double> xs;
            xs.reserve(na);
            for (int k = 0; k < na; ++k) {
                const V a = eval(n.args[k]);
                double d;
                V e;
                if (!toNumber(a, &d, &e)) return e;
                xs.append(d);
            }
            if (fn == QLatin1String("SUM")) {
                double t = 0;
                for (double d : xs) t += d;
                return finite(t);
            }
            if (fn == QLatin1String("MIN") || fn == QLatin1String("MAX")) {
                if (xs.isEmpty()) return V::num(0);
                double r = xs[0];
                for (double d : xs) r = fn == QLatin1String("MIN") ? std::min(r, d) : std::max(r, d);
                return V::num(r);
            }
            if (fn == QLatin1String("AVERAGE")) {
                if (xs.isEmpty()) return errV("#DIV/0!");
                double t = 0;
                for (double d : xs) t += d;
                return finite(t / double(xs.size()));
            }
            if (fn == QLatin1String("AND") || fn == QLatin1String("OR")) {
                if (xs.isEmpty()) return errV("#VALUE!");
                bool r = fn == QLatin1String("AND");
                for (double d : xs) r = fn == QLatin1String("AND") ? (r && d != 0) : (r || d != 0);
                V v;
                v.kind = V::Bool;
                v.number = r ? 1 : 0;
                return v;
            }
            if (fn == QLatin1String("NOT")) {
                if (na != 1) return errV("#VALUE!");
                V v;
                v.kind = V::Bool;
                v.number = xs[0] == 0 ? 1 : 0;
                return v;
            }
            auto digits = [&](double d) { return std::pow(10.0, std::trunc(d)); };
            if ((fn == QLatin1String("ROUND") || fn == QLatin1String("ROUNDUP") || fn == QLatin1String("ROUNDDOWN")) && (na == 1 || na == 2)) {
                const double k = digits(na == 2 ? xs[1] : 0);
                const double x = xs[0] * k;
                const double r = fn == QLatin1String("ROUND") ? std::round(x) : fn == QLatin1String("ROUNDUP") ? (x < 0 ? std::floor(x) : std::ceil(x)) : std::trunc(x);
                return finite(r / k);
            }
            if (na == 1) {
                const double x = xs[0];
                if (fn == QLatin1String("ABS")) return V::num(std::fabs(x));
                if (fn == QLatin1String("INT")) return finite(std::floor(x));
                if (fn == QLatin1String("SQRT")) return x < 0 ? errV("#NUM!") : V::num(std::sqrt(x));
                if (fn == QLatin1String("EXP")) return finite(std::exp(x));
                if (fn == QLatin1String("LN")) return x <= 0 ? errV("#NUM!") : V::num(std::log(x));
                if (fn == QLatin1String("LOG10")) return x <= 0 ? errV("#NUM!") : V::num(std::log10(x));
            }
            if (na == 2) {
                if (fn == QLatin1String("POWER")) return xs[0] == 0 && xs[1] == 0 ? errV("#NUM!") : finite(std::pow(xs[0], xs[1]));
                if (fn == QLatin1String("MOD")) return xs[1] == 0 ? errV("#DIV/0!") : finite(xs[0] - xs[1] * std::floor(xs[0] / xs[1]));
            }
            return errV("#NAME?");
        }
        }
        return errV("#VALUE!");
    };
    return eval(m_root);
}

}   // namespace jp::office::sheet

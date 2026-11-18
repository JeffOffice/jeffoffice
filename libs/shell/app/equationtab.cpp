#include "app/equationtab.h"

#include "app/icons.h"
#include "app/plaintext.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "wml/wmlmathedit.h"

#include <QCoreApplication>
#include <QGridLayout>
#include <QLineEdit>
#include <QMenu>
#include <QToolButton>
#include <QWidgetAction>

namespace jp {

namespace {

constexpr QChar kOpenMark(0x2039), kCloseMark(0x203A);

// A place to fill, with its sample text.
QString fill(const QString &sample) { return QString(kOpenMark) + sample + QString(kCloseMark); }

QVector<EquationCategory> makeCategories()
{
    const QString a = fill(QStringLiteral("a")), b = fill(QStringLiteral("b")), x = fill(QStringLiteral("x")), n = fill(QStringLiteral("n")), i = fill(QStringLiteral("i"));
    const QString y = fill(QStringLiteral("y")), c = fill(QStringLiteral("c")), d = fill(QStringLiteral("d")), m1 = fill(QStringLiteral("1")), m0 = fill(QStringLiteral("0"));
    auto v = [](const char *id, const QString &name, const QString &line, const QString &uni, const QString &latex) { return EquationVariant{QString::fromLatin1(id), name, line, uni, latex}; };
    QVector<EquationCategory> all;
    all.push_back({QStringLiteral("fraction"), QCoreApplication::translate("EquationTab", "Fraction"), QStringLiteral("divide"),
                   {v("stacked", QCoreApplication::translate("EquationTab", "Stacked Fraction"), a + QStringLiteral("/") + b, a + QStringLiteral("/") + b, QStringLiteral("\\frac{") + a + QStringLiteral("}{") + b + QStringLiteral("}")),
                    v("derivative", QCoreApplication::translate("EquationTab", "Derivative"), QStringLiteral("d") + y + QStringLiteral("/d") + x, QStringLiteral("d") + y + QStringLiteral("/d") + x,
                      QStringLiteral("\\frac{d") + y + QStringLiteral("}{d") + x + QStringLiteral("}")),
                    v("reciprocal", QCoreApplication::translate("EquationTab", "Reciprocal"), QStringLiteral("1/") + x, QStringLiteral("1/") + x, QStringLiteral("\\frac{1}{") + x + QStringLiteral("}"))}});
    all.push_back({QStringLiteral("script"), QCoreApplication::translate("EquationTab", "Script"), QStringLiteral("superscript"),
                   {v("sup", QCoreApplication::translate("EquationTab", "Superscript"), x + QStringLiteral("^") + n, x + QStringLiteral("^") + n, x + QStringLiteral("^{") + n + QStringLiteral("}")),
                    v("sub", QCoreApplication::translate("EquationTab", "Subscript"), x + QStringLiteral("_") + i, x + QStringLiteral("_") + i, x + QStringLiteral("_{") + i + QStringLiteral("}")),
                    v("supsub", QCoreApplication::translate("EquationTab", "Subscript and Superscript"), x + QStringLiteral("_") + i + QStringLiteral("^") + n, x + QStringLiteral("_") + i + QStringLiteral("^") + n,
                      x + QStringLiteral("_{") + i + QStringLiteral("}^{") + n + QStringLiteral("}"))}});
    all.push_back({QStringLiteral("radical"), QCoreApplication::translate("EquationTab", "Radical"), QStringLiteral("radical"),
                   {v("sqrt", QCoreApplication::translate("EquationTab", "Square Root"), QStringLiteral("sqrt(") + x + QStringLiteral(")"), QString(QChar(0x221A)) + QStringLiteral("(") + x + QStringLiteral(")"),
                      QStringLiteral("\\sqrt{") + x + QStringLiteral("}")),
                    v("root", QCoreApplication::translate("EquationTab", "Root with Degree"), QStringLiteral("sqrt[") + n + QStringLiteral("](") + x + QStringLiteral(")"), QString(QChar(0x221A)) + QStringLiteral("(") + n + QStringLiteral("&") + x + QStringLiteral(")"),
                      QStringLiteral("\\sqrt[") + n + QStringLiteral("]{") + x + QStringLiteral("}"))}});
    all.push_back({QStringLiteral("integral"), QCoreApplication::translate("EquationTab", "Integral"), QStringLiteral("square-function"),
                   {v("limits", QCoreApplication::translate("EquationTab", "Integral with Limits"), QStringLiteral("\\int_") + a + QStringLiteral("^") + b + QStringLiteral(" ") + x, QString(QChar(0x222B)) + QStringLiteral("_") + a + QStringLiteral("^") + b + QStringLiteral(" ") + x,
                      QStringLiteral("\\int_{") + a + QStringLiteral("}^{") + b + QStringLiteral("} ") + x),
                    v("plain", QCoreApplication::translate("EquationTab", "Integral"), QStringLiteral("\\int ") + x, QString(QChar(0x222B)) + QStringLiteral(" ") + x, QStringLiteral("\\int ") + x)}});
    all.push_back({QStringLiteral("operator"), QCoreApplication::translate("EquationTab", "Large Operator"), QStringLiteral("sigma"),
                   {v("sum", QCoreApplication::translate("EquationTab", "Sum"), QStringLiteral("\\sum_(") + i + QStringLiteral("=1)^") + n + QStringLiteral(" ") + x, QString(QChar(0x2211)) + QStringLiteral("_(") + i + QStringLiteral("=1)^") + n + QStringLiteral(" ") + x,
                      QStringLiteral("\\sum_{") + i + QStringLiteral("=1}^{") + n + QStringLiteral("} ") + x),
                    v("product", QCoreApplication::translate("EquationTab", "Product"), QStringLiteral("\\prod_(") + i + QStringLiteral("=1)^") + n + QStringLiteral(" ") + x, QString(QChar(0x220F)) + QStringLiteral("_(") + i + QStringLiteral("=1)^") + n + QStringLiteral(" ") + x,
                      QStringLiteral("\\prod_{") + i + QStringLiteral("=1}^{") + n + QStringLiteral("} ") + x),
                    v("sumPlain", QCoreApplication::translate("EquationTab", "Sum without Limits"), QStringLiteral("\\sum ") + x, QString(QChar(0x2211)) + QStringLiteral(" ") + x, QStringLiteral("\\sum ") + x)}});
    all.push_back({QStringLiteral("bracket"), QCoreApplication::translate("EquationTab", "Bracket"), QStringLiteral("parentheses"),
                   {v("round", QCoreApplication::translate("EquationTab", "Parentheses"), QStringLiteral("\\left( ") + x + QStringLiteral(" \\right)"), QStringLiteral("(") + x + QStringLiteral(")"), QStringLiteral("\\left( ") + x + QStringLiteral(" \\right)")),
                    v("square", QCoreApplication::translate("EquationTab", "Square Brackets"), QStringLiteral("\\left[ ") + x + QStringLiteral(" \\right]"), QStringLiteral("[") + x + QStringLiteral("]"), QStringLiteral("\\left[ ") + x + QStringLiteral(" \\right]")),
                    v("curly", QCoreApplication::translate("EquationTab", "Braces"), QStringLiteral("\\left\\{ ") + x + QStringLiteral(" \\right\\}"), QStringLiteral("{") + x + QStringLiteral("}"), QStringLiteral("\\left\\{ ") + x + QStringLiteral(" \\right\\}"))}});
    all.push_back({QStringLiteral("function"), QCoreApplication::translate("EquationTab", "Function"), QStringLiteral("variable"),
                   {v("sin", QCoreApplication::translate("EquationTab", "Sine"), QStringLiteral("sin(") + x + QStringLiteral(")"), QStringLiteral("sin(") + x + QStringLiteral(")"), QStringLiteral("\\sin(") + x + QStringLiteral(")")),
                    v("cos", QCoreApplication::translate("EquationTab", "Cosine"), QStringLiteral("cos(") + x + QStringLiteral(")"), QStringLiteral("cos(") + x + QStringLiteral(")"), QStringLiteral("\\cos(") + x + QStringLiteral(")")),
                    v("tan", QCoreApplication::translate("EquationTab", "Tangent"), QStringLiteral("tan(") + x + QStringLiteral(")"), QStringLiteral("tan(") + x + QStringLiteral(")"), QStringLiteral("\\tan(") + x + QStringLiteral(")")),
                    v("log", QCoreApplication::translate("EquationTab", "Logarithm"), QStringLiteral("log(") + x + QStringLiteral(")"), QStringLiteral("log(") + x + QStringLiteral(")"), QStringLiteral("\\log(") + x + QStringLiteral(")")),
                    v("ln", QCoreApplication::translate("EquationTab", "Natural Logarithm"), QStringLiteral("ln(") + x + QStringLiteral(")"), QStringLiteral("ln(") + x + QStringLiteral(")"), QStringLiteral("\\ln(") + x + QStringLiteral(")"))}});
    all.push_back({QStringLiteral("accent"), QCoreApplication::translate("EquationTab", "Accent"), QStringLiteral("chevron-up"),
                   {v("hat", QCoreApplication::translate("EquationTab", "Hat"), QStringLiteral("\\hat{") + x + QStringLiteral("}"), QStringLiteral("\\hat(") + x + QStringLiteral(")"), QStringLiteral("\\hat{") + x + QStringLiteral("}")),
                    v("bar", QCoreApplication::translate("EquationTab", "Bar"), QStringLiteral("\\bar{") + x + QStringLiteral("}"), QStringLiteral("\\bar(") + x + QStringLiteral(")"), QStringLiteral("\\bar{") + x + QStringLiteral("}")),
                    v("vec", QCoreApplication::translate("EquationTab", "Vector"), QStringLiteral("\\vec{") + x + QStringLiteral("}"), QStringLiteral("\\vec(") + x + QStringLiteral(")"), QStringLiteral("\\vec{") + x + QStringLiteral("}")),
                    v("tilde", QCoreApplication::translate("EquationTab", "Tilde"), QStringLiteral("\\tilde{") + x + QStringLiteral("}"), QStringLiteral("\\tilde(") + x + QStringLiteral(")"), QStringLiteral("\\tilde{") + x + QStringLiteral("}")),
                    v("over", QCoreApplication::translate("EquationTab", "Overbar"), QStringLiteral("\\overline{") + x + QStringLiteral("}"), QStringLiteral("\\overline(") + x + QStringLiteral(")"), QStringLiteral("\\overline{") + x + QStringLiteral("}"))}});
    all.push_back({QStringLiteral("limit"), QCoreApplication::translate("EquationTab", "Limit"), QStringLiteral("arrow-right"),
                   {v("lim", QCoreApplication::translate("EquationTab", "Limit"), QStringLiteral("lim_(") + x + QStringLiteral("\\to ") + a + QStringLiteral(") ") + y, QStringLiteral("lim_(") + x + QStringLiteral("\\to ") + a + QStringLiteral(") ") + y,
                      QStringLiteral("\\lim_{") + x + QStringLiteral("\\to ") + a + QStringLiteral("} ") + y),
                    v("max", QCoreApplication::translate("EquationTab", "Maximum"), QStringLiteral("max_(") + x + QStringLiteral(") ") + y, QStringLiteral("max_(") + x + QStringLiteral(") ") + y, QStringLiteral("\\max_{") + x + QStringLiteral("} ") + y),
                    v("min", QCoreApplication::translate("EquationTab", "Minimum"), QStringLiteral("min_(") + x + QStringLiteral(") ") + y, QStringLiteral("min_(") + x + QStringLiteral(") ") + y, QStringLiteral("\\min_{") + x + QStringLiteral("} ") + y)}});
    auto op = [&](const char *id, const QString &name, const char *symbol) {
        const QString s = QString::fromLatin1(symbol);
        return v(id, name, a + QStringLiteral(" \\") + s + QStringLiteral(" ") + b, a + QStringLiteral(" \\") + s + QStringLiteral(" ") + b, a + QStringLiteral(" \\") + s + QStringLiteral(" ") + b);
    };
    all.push_back({QStringLiteral("op"), QCoreApplication::translate("EquationTab", "Operator"), QStringLiteral("equal"),
                   {op("times", QCoreApplication::translate("EquationTab", "Times"), "times"), op("pm", QCoreApplication::translate("EquationTab", "Plus or Minus"), "pm"), op("neq", QCoreApplication::translate("EquationTab", "Not Equal"), "neq"), op("approx", QCoreApplication::translate("EquationTab", "Approximately Equal"), "approx"), op("to", QCoreApplication::translate("EquationTab", "Arrow"), "to")}});
    all.push_back({QStringLiteral("matrix"), QCoreApplication::translate("EquationTab", "Matrix"), QStringLiteral("grid-2x2"),
                   {v("two", QCoreApplication::translate("EquationTab", "2 by 2 Matrix"), QStringLiteral("\\pmatrix{") + a + QStringLiteral(" & ") + b + QStringLiteral(" \\\\ ") + c + QStringLiteral(" & ") + d + QStringLiteral("}"),
                      QStringLiteral("(\\matrix(") + a + QStringLiteral("&") + b + QStringLiteral("@") + c + QStringLiteral("&") + d + QStringLiteral("))"),
                      QStringLiteral("\\begin{pmatrix} ") + a + QStringLiteral(" & ") + b + QStringLiteral(" \\\\ ") + c + QStringLiteral(" & ") + d + QStringLiteral(" \\end{pmatrix}")),
                    v("identity", QCoreApplication::translate("EquationTab", "Identity Matrix"), QStringLiteral("\\pmatrix{") + m1 + QStringLiteral(" & ") + m0 + QStringLiteral(" \\\\ ") + m0 + QStringLiteral(" & ") + m1 + QStringLiteral("}"),
                      QStringLiteral("(\\matrix(") + m1 + QStringLiteral("&") + m0 + QStringLiteral("@") + m0 + QStringLiteral("&") + m1 + QStringLiteral("))"),
                      QStringLiteral("\\begin{pmatrix} ") + m1 + QStringLiteral(" & ") + m0 + QStringLiteral(" \\\\ ") + m0 + QStringLiteral(" & ") + m1 + QStringLiteral(" \\end{pmatrix}")),
                    v("column", QCoreApplication::translate("EquationTab", "Column of Two"), QStringLiteral("\\pmatrix{") + a + QStringLiteral(" \\\\ ") + b + QStringLiteral("}"), QStringLiteral("(\\matrix(") + a + QStringLiteral("@") + b + QStringLiteral("))"),
                      QStringLiteral("\\begin{pmatrix} ") + a + QStringLiteral(" \\\\ ") + b + QStringLiteral(" \\end{pmatrix}"))}});
    return all;
}

QString textOf(const EquationVariant &variant, EquationSyntax syntax)
{
    switch (syntax) {
    case EquationSyntax::UnicodeMath: return variant.unicode;
    case EquationSyntax::Latex: return variant.latex;
    default: return variant.line;
    }
}

QString cleaned(QString text)
{
    text.remove(kOpenMark);
    text.remove(kCloseMark);
    return text;
}

} // namespace

const QVector<EquationCategory> &equationCategories()
{
    static const QVector<EquationCategory> all = makeCategories();
    return all;
}

const QVector<EquationSymbolGroup> &equationSymbolGroups()
{
    static const QVector<EquationSymbolGroup> all = [] {
        QVector<EquationSymbolGroup> g;
        g.push_back({QStringLiteral("greek"), QCoreApplication::translate("EquationTab", "Greek Letters"),
                     QString::fromUtf8("αβγδεζηθικλμνξοπρστυφχψω" "ΓΔΘΛΞΠΣΦΨΩ" "ϵϑϖϱϕ")});
        g.push_back({QStringLiteral("operators"), QCoreApplication::translate("EquationTab", "Operators"), QString::fromUtf8("±∓×÷·∘∗⊕⊗∧∨∩∪∖⋅∙")});
        g.push_back({QStringLiteral("relations"), QCoreApplication::translate("EquationTab", "Relations"), QString::fromUtf8("≠≈≡≤≥≪≫∝∈∉⊂⊆⊃⊇∼≅∣⊥∥")});
        g.push_back({QStringLiteral("arrows"), QCoreApplication::translate("EquationTab", "Arrows"), QString::fromUtf8("←→↔⇐⇒⇔↑↓↦⟵⟶⟷")});
        g.push_back({QStringLiteral("others"), QCoreApplication::translate("EquationTab", "Sets and Logic"), QString::fromUtf8("∅∞∀∃¬∴∵∂∇ℏℕℤℚℝℂ°′″…⋯⋮⋱")});
        return g;
    }();
    return all;
}

QString equationPickText(const QString &pick, EquationSyntax syntax)
{
    if (pick == QLatin1String("text")) return syntax == EquationSyntax::Latex ? QStringLiteral("\\text{") + fill(QStringLiteral("text")) + QStringLiteral("}") : QStringLiteral("\"") + fill(QStringLiteral("text")) + QStringLiteral("\"");
    if (pick.startsWith(QLatin1String("symbol:"))) {
        const QString symbol = pick.mid(7);
        return symbol.size() == 1 || (symbol.size() == 2 && symbol[0].isHighSurrogate()) ? symbol : QString();
    }
    if (!pick.startsWith(QLatin1String("structure:"))) return QString();
    const QString which = pick.mid(10);
    const int dot = which.indexOf(QLatin1Char('.'));
    if (dot < 0) return QString();
    for (const EquationCategory &c : equationCategories()) {
        if (c.id != which.left(dot)) continue;
        for (const EquationVariant &variant : c.variants)
            if (variant.id == which.mid(dot + 1)) return textOf(variant, syntax);
    }
    return QString();
}

QString equationUnmarked(const QString &marked) { return cleaned(marked); }

void insertIntoEquationLine(QLineEdit *line, const QString &marked)
{
    if (!line || marked.isEmpty()) return;
    const QString selected = line->selectedText();
    const int from = line->hasSelectedText() ? line->selectionStart() : line->cursorPosition();
    const int to = from + selected.size();
    QString put = marked;
    // What was selected is the first place to fill.
    const int open = put.indexOf(kOpenMark), close = open < 0 ? -1 : put.indexOf(kCloseMark, open);
    if (open >= 0 && close > open && !selected.isEmpty()) put.replace(open + 1, close - open - 1, selected);
    const int firstFrom = open >= 0 && close > open ? open : -1;
    const int firstLength = firstFrom < 0 ? 0 : put.indexOf(kCloseMark, open) - open - 1;
    put = cleaned(put);
    QString text = line->text();
    text.replace(from, to - from, put);
    if (text.size() > office::wml::kMostEquationInput) return;
    line->setText(text);
    if (firstFrom >= 0) line->setSelection(from + firstFrom, firstLength);
    else line->setCursorPosition(from + put.size());
    line->setFocus();
}

EquationSyntax equationSyntaxSetting()
{
    const int v = Settings::get().value(QStringLiteral("equation/syntax"), 0).toInt();
    return v == 1 ? EquationSyntax::UnicodeMath : v == 2 ? EquationSyntax::Latex : EquationSyntax::Line;
}

void setEquationSyntaxSetting(EquationSyntax syntax) { Settings::get().setValue(QStringLiteral("equation/syntax"), int(syntax)); }

bool equationLinearSetting() { return Settings::get().value(QStringLiteral("equation/linear"), false).toBool(); }

void setEquationLinearSetting(bool linear) { Settings::get().setValue(QStringLiteral("equation/linear"), linear); }

void createEquationTabActions(const EquationActionMaker &make, const EquationTabHandlers &h)
{
    const auto edit = h.edit;
    make(QStringLiteral("equation.edit"), QCoreApplication::translate("EquationTab", "Edit Equation"), QStringLiteral("square-pi"), [edit] { if (edit) edit(QString()); }, false);
    make(QStringLiteral("equation.professional"), QCoreApplication::translate("EquationTab", "Professional"), QStringLiteral("square-pi"), [h] { if (h.setLinear) h.setLinear(false); }, true);
    make(QStringLiteral("equation.linear"), QCoreApplication::translate("EquationTab", "Linear"), QStringLiteral("text"), [h] { if (h.setLinear) h.setLinear(true); }, true);
    make(QStringLiteral("equation.normalText"), QCoreApplication::translate("EquationTab", "Normal Text"), QStringLiteral("type"), [edit] { if (edit) edit(QStringLiteral("text")); }, false);
    make(QStringLiteral("equation.syntaxLine"), QCoreApplication::translate("EquationTab", "Simple Line"), QStringLiteral("code"), [h] { if (h.setSyntax) h.setSyntax(EquationSyntax::Line); }, true);
    make(QStringLiteral("equation.syntaxUnicode"), QStringLiteral("UnicodeMath"), QStringLiteral("languages"), [h] { if (h.setSyntax) h.setSyntax(EquationSyntax::UnicodeMath); }, true);
    make(QStringLiteral("equation.syntaxLatex"), QStringLiteral("LaTeX"), QStringLiteral("code-xml"), [h] { if (h.setSyntax) h.setSyntax(EquationSyntax::Latex); }, true);
    for (const EquationCategory &c : equationCategories()) {
        const QString first = QStringLiteral("structure:") + c.id + QLatin1Char('.') + c.variants.first().id;
        make(QStringLiteral("equation.structure.") + c.id, c.name, c.icon, [edit, first] { if (edit) edit(first); }, false);
        for (const EquationVariant &v : c.variants) {
            const QString pick = QStringLiteral("structure:") + c.id + QLatin1Char('.') + v.id;
            make(QStringLiteral("equation.pick.") + c.id + QLatin1Char('.') + v.id, v.name, c.icon, [edit, pick] { if (edit) edit(pick); }, false);
        }
    }
}

QMenu *equationSymbolMenu(QWidget *owner, const EquationSymbolGroup &group, const std::function<void(const QString &pick)> &pick)
{
    auto *menu = new QMenu(owner);
    auto *host = new QWidget(menu);
    auto *grid = new QGridLayout(host);
    grid->setContentsMargins(4, 4, 4, 4);
    grid->setSpacing(2);
    int at = 0;
    for (int i = 0; i < group.symbols.size(); ++i) {
        QString symbol(group.symbols[i]);
        if (group.symbols[i].isHighSurrogate() && i + 1 < group.symbols.size()) symbol += group.symbols[++i];
        auto *b = new QToolButton(host);
        b->setText(symbol);
        b->setAutoRaise(true);
        b->setFixedSize(30, 30);
        b->setToolTip(jp::plainToolTip(symbol));
        b->setObjectName(QStringLiteral("equationSymbol"));
        QObject::connect(b, &QToolButton::clicked, menu, [menu, pick, symbol] {
            menu->close();
            if (pick) pick(QStringLiteral("symbol:") + symbol);
        });
        grid->addWidget(b, at / 8, at % 8);
        ++at;
    }
    auto *wa = new QWidgetAction(menu);
    wa->setDefaultWidget(host);
    menu->addAction(wa);
    return menu;
}

void addEquationTabMenus(RibbonParts &parts, const std::function<QAction *(const QString &id)> &act, const std::function<void(const QString &pick)> &edit)
{
    parts.colors[QString::fromLatin1(kEquationColor)] = QColor(0x3A, 0x6E, 0xA5);
    for (const EquationCategory &c : equationCategories()) {
        const QString id = c.id;
        parts.menus[QStringLiteral("equationStructure.") + id] = [act, id](QWidget *owner) {
            auto *m = new QMenu(owner);
            for (const EquationCategory &cat : equationCategories()) {
                if (cat.id != id) continue;
                for (const EquationVariant &v : cat.variants)
                    if (QAction *a = act(QStringLiteral("equation.pick.") + cat.id + QLatin1Char('.') + v.id)) m->addAction(a);
            }
            return m;
        };
    }
    for (const EquationSymbolGroup &g : equationSymbolGroups()) {
        const QString id = g.id;
        parts.menus[QStringLiteral("equationSymbols.") + id] = [id, edit](QWidget *owner) {
            for (const EquationSymbolGroup &group : equationSymbolGroups())
                if (group.id == id) return equationSymbolMenu(owner, group, edit);
            return new QMenu(owner);
        };
    }
}

void showEquationTabChoices(const std::function<QAction *(const QString &id)> &act, bool linear, EquationSyntax syntax)
{
    const struct {
        const char *id;
        bool on;
    } choices[] = {{"equation.professional", !linear}, {"equation.linear", linear}, {"equation.syntaxLine", syntax == EquationSyntax::Line},
                   {"equation.syntaxUnicode", syntax == EquationSyntax::UnicodeMath}, {"equation.syntaxLatex", syntax == EquationSyntax::Latex}};
    for (const auto &c : choices)
        if (QAction *a = act(QString::fromLatin1(c.id))) a->setChecked(c.on);
}

} // namespace jp

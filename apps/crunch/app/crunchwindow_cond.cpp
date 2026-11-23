// Conditional formatting from the Home tab: Highlight Cells Rules (greater than, less than, between, equal to, text that contains, duplicate values), Top/Bottom Rules (top and bottom items
// and percent, above and below average), and Clear Rules. A rule is asked for in a small window (a value or two, and how the cells look); the work is the session's (core/editcond.cpp).

#include "app/uistyle.h"
#include "app/condpresets.h"
#include "app/condrulesdialog.h"
#include "app/crunchwindow.h"

#include "core/editsession.h"
#include "grid/gridview.h"
#include "calc/dates.h"
#include "calc/formula.h"
#include "sheet/calculation.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMenu>
#include <QDate>
#include <QLocale>
#include <QVBoxLayout>

#include <cmath>

namespace jp {

using jp::office::sheet::CondRule;
using jp::office::sheet::CondValue;
using jp::office::sheet::DifferentialFormat;

namespace {

// Whether every sheet a parsed formula names is in the workbook (a name the formula gives to a sheet that is not there would be an error in every cell of the rule).
bool sheetsAreThere(const jp::calc::ParseResult &parsed, const jp::office::sheet::Calculation *calc)
{
    if (!calc || !parsed.ok() || parsed.program->external) return true;
    for (const QString &name : parsed.program->sheetNames)
        if (calc->sheetIndex(name) < 0) return false;
    return true;
}

// Whether the text is a formula the engine reads, with the sheets it names in the workbook.
bool readableFormula(const QString &body, const jp::office::sheet::Calculation *calc) { return !body.isEmpty() && sheetsAreThere(jp::calc::parseFormula(body), calc); }

// A value as a rule's formula, or empty with the reason in `why`: a number as it is (not infinity or "not a number"), a date as its day number, TRUE and FALSE as they are, "=..." as a formula that
// must be one (its cells are the ones typed for the cell with the cursor: `dRow` and `dCol` move them to the first cell of the rule's cells), anything else as a text in quotes.
QString valueFormula(const QString &typed, int dRow, int dCol, const jp::office::sheet::Calculation *calc, QString *why)
{
    const QString t = typed.trimmed();
    if (t.startsWith(QLatin1Char('='))) {
        const QString body = t.mid(1).trimmed();
        if (!readableFormula(body, calc)) {
            *why = CrunchWindow::tr("\"%1\" is not a formula JeffCrunch can read.").arg(t);
            return QString();
        }
        return jp::office::sheet::shiftFormula(body, dRow, dCol);
    }
    bool ok = false;
    const double n = QLocale().toDouble(t, &ok);
    if (ok) {
        if (!std::isfinite(n)) {
            *why = CrunchWindow::tr("\"%1\" is not a number the rule can use.").arg(t);
            return QString();
        }
        return QString::number(n, 'g', 15);
    }
    if (t.compare(QLatin1String("TRUE"), Qt::CaseInsensitive) == 0) return QStringLiteral("TRUE");
    if (t.compare(QLatin1String("FALSE"), Qt::CaseInsensitive) == 0) return QStringLiteral("FALSE");
    // (a date as the language writes it short, with a four-digit year too, or long, or as year-month-day)
    QString shortWithYear = QLocale().dateFormat(QLocale::ShortFormat);
    if (!shortWithYear.contains(QLatin1String("yyyy"))) shortWithYear.replace(QLatin1String("yy"), QLatin1String("yyyy"));
    QDate date;
    for (const QString &format : {QStringLiteral("yyyy-MM-dd"), shortWithYear, QLocale().dateFormat(QLocale::ShortFormat), QLocale().dateFormat(QLocale::LongFormat)}) {
        date = QLocale().toDate(t, format);
        if (date.isValid()) break;
    }
    double serial = 0;
    if (date.isValid() && jp::calc::dateToSerial(date.year(), date.month(), date.day(), jp::calc::DateSystem::Date1900, &serial)) return QString::number(serial, 'g', 15);
    QString q = typed.trimmed();
    q.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + q + QLatin1Char('"');
}

} // namespace

void CrunchWindow::createCondActions()
{
    using Kind = CondAnswer::Kind;
    auto rule = [this](const char *id, const QString &text, Kind kind, const char *icon = "") {
        mk(QString::fromLatin1(id), text, QString::fromLatin1(icon), QKeySequence(), [this, kind] { makeCondRule(kind); });
        m_opsActions << QString::fromLatin1(id);
    };
    rule("cond.greater", tr("Greater Than…"), Kind::Greater);
    rule("cond.less", tr("Less Than…"), Kind::Less);
    rule("cond.between", tr("Between…"), Kind::Between);
    rule("cond.equal", tr("Equal To…"), Kind::Equal);
    rule("cond.contains", tr("Text That Contains…"), Kind::Contains);
    rule("cond.duplicate", tr("Duplicate Values…"), Kind::Duplicate);
    rule("cond.top", tr("Top 10 Items…"), Kind::Top);
    rule("cond.bottom", tr("Bottom 10 Items…"), Kind::Bottom);
    rule("cond.topPercent", tr("Top 10 %…"), Kind::TopPercent);
    rule("cond.bottomPercent", tr("Bottom 10 %…"), Kind::BottomPercent);
    rule("cond.date", tr("A Date Occurring…"), Kind::DateOccurring);
    rule("cond.above", tr("Above Average…"), Kind::Above);
    rule("cond.below", tr("Below Average…"), Kind::Below);
    // the galleries: Data Bars, Color Scales, and Icon Sets, each choice one step that puts the rule on the selected cells
    auto gallery = [this](const QString &id, const QString &text, const QPixmap &picture, const std::function<CondSpec()> &make) {
        QAction *a = mk(id, text, QString(), QKeySequence(), [this, make] { addCondRuleFromSpec(make()); });
        a->setIcon(QIcon(picture));
        m_opsActions << id;
    };
    for (int i = 0; i < condpresets::barColors().size(); ++i)
        for (const bool gradient : {true, false}) {
            const auto &c = condpresets::barColors()[i];
            gallery(QStringLiteral("cond.bar.%1.%2").arg(i).arg(gradient ? QStringLiteral("gradient") : QStringLiteral("solid")),
                    gradient ? tr("Gradient Fill, %1").arg(tr(c.name)) : tr("Solid Fill, %1").arg(tr(c.name)), condpresets::barPixmap(c.rgb, gradient, 32), [i, gradient] {
                        CondSpec s;
                        s.kind = CondSpec::Kind::DataBar;
                        s.visual = condpresets::dataBarRule(jp::office::sheet::CellRange(), i, gradient);
                        return s;
                    });
        }
    for (int i = 0; i < condpresets::scalePresets().size(); ++i) {
        const auto &p = condpresets::scalePresets()[i];
        gallery(QStringLiteral("cond.scale.%1").arg(i), tr(p.name), condpresets::scalePixmap(p.colors, 32), [i] {
            CondSpec s;
            s.kind = CondSpec::Kind::ColorScale;
            s.visual = condpresets::colorScaleRule(jp::office::sheet::CellRange(), i);
            return s;
        });
    }
    for (const auto &group : condpresets::iconGroups())
        for (const QString &set : group.sets)
            gallery(QStringLiteral("cond.icons.%1").arg(set), condpresets::iconSetLabel(set), condpresets::iconSetStrip(set, 32), [set] {
                CondSpec s;
                s.kind = CondSpec::Kind::IconSet;
                s.visual = condpresets::iconSetRule(jp::office::sheet::CellRange(), set);
                return s;
            });
    mk(QStringLiteral("cond.new"), tr("New Rule…"), QString(), QKeySequence(), [this] { newCondRule(); });
    mk(QStringLiteral("cond.manage"), tr("Manage Rules…"), QString(), QKeySequence(), [this] { manageCondRules(); });
    m_opsActions << QStringLiteral("cond.new") << QStringLiteral("cond.manage");
    // (the two that clear are on only when there is a rule for them to clear: see updateCondActions)
    mk(QStringLiteral("cond.clearSelection"), tr("Clear Rules from Selected Cells"), QString(), QKeySequence(), [this] { clearCondRules(false); }, false, false);
    mk(QStringLiteral("cond.clearSheet"), tr("Clear Rules from This Sheet"), QString(), QKeySequence(), [this] { clearCondRules(true); }, false, false);
    connect(m_view, &grid::GridView::selectionChanged, this, [this] { updateCondActions(); });
}

// The three galleries of the Conditional Formatting menu: the data bars (gradient fills, then solid fills), the color scales, and the icon sets by group.
QMenu *CrunchWindow::buildCondGalleryMenu(int which, QWidget *owner)
{
    auto *menu = new QMenu(owner);
    if (which == 0) {
        for (const bool gradient : {true, false}) {
            menu->addSection(gradient ? tr("Gradient Fill") : tr("Solid Fill"));
            for (int i = 0; i < condpresets::barColors().size(); ++i)
                menu->addAction(act(QStringLiteral("cond.bar.%1.%2").arg(i).arg(gradient ? QStringLiteral("gradient") : QStringLiteral("solid"))));
        }
    } else if (which == 1) {
        for (int i = 0; i < condpresets::scalePresets().size(); ++i) menu->addAction(act(QStringLiteral("cond.scale.%1").arg(i)));
    } else {
        for (const auto &group : condpresets::iconGroups()) {
            menu->addSection(tr(group.name));
            for (const QString &set : group.sets) menu->addAction(act(QStringLiteral("cond.icons.%1").arg(set)));
        }
    }
    menu->addSeparator();
    menu->addAction(act(QStringLiteral("cond.new")));
    return menu;
}

// Asks how the rule is to be: the number or text it compares with, and how the cells look. False when it is given up.
bool CrunchWindow::askCondRule(CondAnswer *answer)
{
    if (m_condAsker) return m_condAsker(answer);
    using Kind = CondAnswer::Kind;
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Conditional Formatting"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout;
    layout->addLayout(form);
    QLineEdit *first = nullptr, *second = nullptr;
    const bool value = answer->kind == Kind::Greater || answer->kind == Kind::Less || answer->kind == Kind::Between || answer->kind == Kind::Equal || answer->kind == Kind::Contains;
    const bool count = answer->kind == Kind::Top || answer->kind == Kind::Bottom || answer->kind == Kind::TopPercent || answer->kind == Kind::BottomPercent;
    if (value || count) {
        first = new QLineEdit(count ? QStringLiteral("10") : QString(), &dialog);
        first->setObjectName(QStringLiteral("condValue1"));
        form->addRow(answer->kind == Kind::Between ? tr("From") : count ? tr("How many") : answer->kind == Kind::Contains ? tr("Text") : tr("Value"), first);
    }
    if (answer->kind == Kind::Between) {
        second = new QLineEdit(&dialog);
        second->setObjectName(QStringLiteral("condValue2"));
        form->addRow(tr("To"), second);
    }
    QComboBox *period = nullptr;
    if (answer->kind == Kind::DateOccurring) {
        period = new QComboBox(&dialog);
        period->setObjectName(QStringLiteral("condPeriod"));
        for (const condpresets::Period &p : condpresets::periods()) period->addItem(tr(p.text), QString::fromLatin1(p.name));
        period->setCurrentIndex(period->findData(QStringLiteral("today")));
        form->addRow(tr("Dates"), period);
    }
    auto *look = new QComboBox(&dialog);
    look->setObjectName(QStringLiteral("condPreset"));
    for (int i = 0; i < condpresets::lookCount(); ++i) look->addItem(condpresets::lookName(i));
    form->addRow(tr("Cells look like this"), look);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return false;
    if (first) answer->value1 = first->text();
    if (second) answer->value2 = second->text();
    if (period) answer->value1 = period->currentData().toString();
    answer->preset = look->currentIndex();
    return true;
}

namespace {

// The rule the answers make for `range` (cells a typed formula names are as seen from the cell with the cursor; a rule holds them as seen from the first of its cells: `dRow` and `dCol`
// move them). False, with the reason, when an answer is not usable.
bool buildCondRule(const CondSpec &spec, const jp::office::sheet::CellRange &range, int dRow, int dCol, const jp::office::sheet::Calculation *calc, CondRule *out, QString *why)
{
    using Kind = CondSpec::Kind;
    CondRule rule;
    rule.ranges.append(range);
    rule.stopIfTrue = spec.stopIfTrue;
    auto formula = [&](const QString &typed) { return valueFormula(typed, dRow, dCol, calc, why); };
    auto quoted = [](QString t) {
        t.replace(QLatin1Char('"'), QStringLiteral("\"\""));
        return t;
    };
    switch (spec.kind) {
    case Kind::CellValue: {
        const bool between = spec.op == CondRule::Operator::Between || spec.op == CondRule::Operator::NotBetween;
        if (spec.v1.trimmed().isEmpty()) {
            *why = CrunchWindow::tr("Type a value for the rule.");
            return false;
        }
        if (between && spec.v2.trimmed().isEmpty()) {
            *why = CrunchWindow::tr("Type the two values the cells are between.");
            return false;
        }
        rule.type = CondRule::Type::CellIs;
        rule.op = spec.op;
        QString from = formula(spec.v1), to = between ? formula(spec.v2) : QString();
        bool a = false, b = false;
        const double x = from.toDouble(&a), y = to.toDouble(&b);
        if (between && a && b && x > y) std::swap(from, to);   // (from the larger to the smaller: the other way round)
        rule.formula1 = from;
        rule.formula2 = to;
        break;
    }
    case Kind::TextContains:
    case Kind::TextNotContains:
    case Kind::TextBegins:
    case Kind::TextEnds: {
        const QString text = spec.text.isEmpty() ? spec.v1 : spec.text;
        if (text.trimmed().isEmpty()) {
            *why = CrunchWindow::tr("Type a value for the rule.");
            return false;
        }
        const QString first = jp::office::sheet::cellName(range.row1, range.col1);
        rule.text = text;
        if (spec.kind == Kind::TextContains) {
            rule.type = CondRule::Type::ContainsText;
            rule.formula1 = QStringLiteral("NOT(ISERROR(SEARCH(\"%1\",%2)))").arg(quoted(text), first);
        } else if (spec.kind == Kind::TextNotContains) {
            rule.type = CondRule::Type::NotContainsText;
            rule.formula1 = QStringLiteral("ISERROR(SEARCH(\"%1\",%2))").arg(quoted(text), first);
        } else if (spec.kind == Kind::TextBegins) {
            rule.type = CondRule::Type::BeginsWith;
            rule.formula1 = QStringLiteral("LEFT(%2,LEN(\"%1\"))=\"%1\"").arg(quoted(text), first);
        } else {
            rule.type = CondRule::Type::EndsWith;
            rule.formula1 = QStringLiteral("RIGHT(%2,LEN(\"%1\"))=\"%1\"").arg(quoted(text), first);
        }
        break;
    }
    case Kind::DateOccurring: rule.type = CondRule::Type::TimePeriod; rule.timePeriod = spec.period.isEmpty() ? QStringLiteral("today") : spec.period; break;
    case Kind::Blanks: rule.type = CondRule::Type::ContainsBlanks; break;
    case Kind::NoBlanks: rule.type = CondRule::Type::NotContainsBlanks; break;
    case Kind::Errors: rule.type = CondRule::Type::ContainsErrors; break;
    case Kind::NoErrors: rule.type = CondRule::Type::NotContainsErrors; break;
    case Kind::Duplicate: rule.type = CondRule::Type::DuplicateValues; break;
    case Kind::Unique: rule.type = CondRule::Type::UniqueValues; break;
    case Kind::Top:
    case Kind::Bottom:
    case Kind::TopPercent:
    case Kind::BottomPercent:
        rule.type = CondRule::Type::Top10;
        rule.percent = spec.kind == Kind::TopPercent || spec.kind == Kind::BottomPercent;
        rule.bottom = spec.kind == Kind::Bottom || spec.kind == Kind::BottomPercent;
        rule.rank = std::clamp(spec.count, 1, rule.percent ? 100 : 1000);
        break;
    case Kind::Above:
    case Kind::Below:
        rule.type = CondRule::Type::AboveAverage;
        rule.above = spec.kind == Kind::Above;
        break;
    case Kind::Formula: {
        QString body = spec.v1.trimmed();
        if (body.startsWith(QLatin1Char('='))) body = body.mid(1).trimmed();
        if (!readableFormula(body, calc)) {
            *why = CrunchWindow::tr("\"%1\" is not a formula JeffCrunch can read.").arg(spec.v1.trimmed());
            return false;
        }
        rule.type = CondRule::Type::Expression;
        rule.formula1 = jp::office::sheet::shiftFormula(body, dRow, dCol);
        break;
    }
    case Kind::ColorScale:
    case Kind::DataBar:
    case Kind::IconSet: {
        rule = spec.visual;
        rule.ranges = {range};
        rule.stopIfTrue = spec.stopIfTrue;
        rule.uid = 0;
        rule.priority = 0;
        rule.origPriority = 0;
        rule.extId.clear();
        // (the numbers and formulas the scale, bar, or icons change at must be usable)
        for (CondValue &v : rule.values)
            if (v.type == CondValue::Type::Formula) {
                QString body = v.value;
                if (body.startsWith(QLatin1Char('='))) body = body.mid(1).trimmed();
                if (!readableFormula(body, calc)) {
                    *why = CrunchWindow::tr("\"%1\" is not a formula JeffCrunch can read.").arg(v.value);
                    return false;
                }
                v.value = body;
            } else if (v.type == CondValue::Type::Number || v.type == CondValue::Type::Percent || v.type == CondValue::Type::Percentile) {
                bool ok = false;
                const double n = QLocale().toDouble(v.value.trimmed(), &ok);
                if (!ok || !std::isfinite(n)) {
                    *why = CrunchWindow::tr("\"%1\" is not a number the rule can use.").arg(v.value);
                    return false;
                }
                v.value = QString::number(n, 'g', 15);
            }
        break;
    }
    }
    if (!why->isEmpty()) return false;
    *out = rule;
    return true;
}

CondSpec specOfAnswer(const CrunchWindow::CondAnswer &a)
{
    using Kind = CrunchWindow::CondAnswer::Kind;
    using K = CondSpec::Kind;
    CondSpec s;
    s.v1 = a.value1;
    s.v2 = a.value2;
    s.look = a.preset;
    switch (a.kind) {
    case Kind::Greater: s.kind = K::CellValue; s.op = CondRule::Operator::GreaterThan; break;
    case Kind::Less: s.kind = K::CellValue; s.op = CondRule::Operator::LessThan; break;
    case Kind::Equal: s.kind = K::CellValue; s.op = CondRule::Operator::Equal; break;
    case Kind::Between: s.kind = K::CellValue; s.op = CondRule::Operator::Between; break;
    case Kind::Contains: s.kind = K::TextContains; s.text = a.value1; break;
    case Kind::Duplicate: s.kind = K::Duplicate; break;
    case Kind::Top:
    case Kind::Bottom:
    case Kind::TopPercent:
    case Kind::BottomPercent: {
        s.kind = a.kind == Kind::Top ? K::Top : a.kind == Kind::Bottom ? K::Bottom : a.kind == Kind::TopPercent ? K::TopPercent : K::BottomPercent;
        bool ok = false;
        const int n = QLocale().toInt(a.value1.trimmed(), &ok);
        s.count = ok ? n : 10;
        break;
    }
    case Kind::Above: s.kind = K::Above; break;
    case Kind::Below: s.kind = K::Below; break;
    case Kind::DateOccurring: s.kind = K::DateOccurring; s.period = a.value1; break;
    }
    return s;
}

} // namespace

void CrunchWindow::makeCondRule(CondAnswer::Kind kind)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed() || !m_session || m_sheet < 0) return;
    CondAnswer answer;
    answer.kind = kind;
    if (!askCondRule(&answer)) return;
    addCondRuleFromSpec(specOfAnswer(answer));
}

// Puts the rule the answers describe on the selected cells, in front of the others, as one step of Undo.
void CrunchWindow::addCondRuleFromSpec(const CondSpec &spec)
{
    const jp::office::sheet::CellRange range = selectedRange();
    const grid::Selection &sel = m_view->selection();
    const int dRow = int(range.row1) - int(std::max<qint64>(0, sel.activeRow)), dCol = int(range.col1) - int(std::max<qint64>(0, sel.activeCol));
    CondRule rule;
    QString why;
    if (!buildCondRule(spec, range, dRow, dCol, m_session->calculation(), &rule, &why)) {
        notice(tr("Conditional Formatting"), why);
        return;
    }
    const crunch::EditSession::Outcome o = m_session->addCondRule(m_sheet, rule, spec.hasLook() ? condpresets::lookOf(spec.look) : DifferentialFormat());
    if (!o.ok) notice(tr("Conditional Formatting"), o.problem);
}

void CrunchWindow::newCondRule()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed() || !m_session || m_sheet < 0) return;
    CondSpec spec;
    if (!askCondSpec(&spec, false)) return;
    addCondRuleFromSpec(spec);
}

bool CrunchWindow::askCondSpec(CondSpec *spec, bool editing)
{
    if (m_condSpecAsker) return m_condSpecAsker(spec, editing);
    CondRuleDialog dialog(*spec, editing, this);
    if (dialog.exec() != QDialog::Accepted) return false;
    *spec = dialog.spec();
    return true;
}

void CrunchWindow::manageCondRules()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed() || !m_session || m_sheet < 0) return;
    if (const QString no = m_session->condRulesRefusal(m_sheet); !no.isEmpty()) {
        notice(tr("Conditional Formatting"), no);
        return;
    }
    m_session->ensureCondUids(m_sheet);   // (the rules of a file just opened have no names yet: the list says which are the sheet's own by them)
    QVector<CondManageDialog::Item> items;
    for (const CondRule &r : m_session->condRules(m_sheet)) {
        CondManageDialog::Item it;
        it.rule = r;
        it.look = m_session->condLookOf(r);
        items.append(it);
    }
    const jp::office::sheet::CellRange selected = selectedRange();
    const grid::Selection &sel = m_view->selection();
    // (a formula typed for a new rule is as seen from the cell with the cursor, as in the New Rule of the menu; the formula of a rule that exists is shown as its first cell reads it)
    const int newRow = int(selected.row1) - int(std::max<qint64>(0, sel.activeRow)), newCol = int(selected.col1) - int(std::max<qint64>(0, sel.activeCol));
    auto editor = [this, selected, newRow, newCol](const CondRule *old, const DifferentialFormat &oldLook, CondManageDialog::Item *out) {
        CondSpec spec = old ? specOfRule(*old) : CondSpec();
        if (!askCondSpec(&spec, old != nullptr)) return false;
        const jp::office::sheet::CellRange range = old && !old->ranges.isEmpty() ? old->ranges.first() : selected;
        CondRule rule;
        QString why;
        if (!buildCondRule(spec, range, old ? 0 : newRow, old ? 0 : newCol, m_session->calculation(), &rule, &why)) {
            notice(tr("Conditional Formatting"), why);
            return false;
        }
        if (old && old->ranges.size() > 1) rule.ranges = old->ranges;
        out->rule = rule;
        out->look = DifferentialFormat();
        out->newLook = false;
        if (spec.hasLook()) {
            if (old && spec.keepLook) {
                out->look = oldLook;
                out->rule.dxf = old->dxf;
            } else {
                out->look = condpresets::lookOf(spec.look);
                out->newLook = true;
            }
        }
        return true;
    };
    QVector<crunch::EditSession::CondManaged> result;
    if (m_manageAsker) {
        if (!m_manageAsker(items, editor, &result)) return;
    } else {
        CondManageDialog dialog(items, editor, this);
        if (dialog.exec() != QDialog::Accepted) return;
        result = dialog.result();
    }
    const crunch::EditSession::Outcome o = m_session->setManagedCondRules(m_sheet, result);
    if (!o.ok) notice(tr("Conditional Formatting"), o.problem);
}

// Clear Rules is on when the sheet has a rule (for the selected cells, in the first case) and the sheet can be changed.
void CrunchWindow::updateCondActions()
{
    QAction *selection = act(QStringLiteral("cond.clearSelection")), *sheet = act(QStringLiteral("cond.clearSheet"));
    if (!selection || !sheet) return;
    bool any = false, onSelection = false;
    m_condScanned = 0;
    if (m_session && m_sheet >= 0 && !isReadOnly()) {
        const jp::office::sheet::CellRange selected = selectedRange();
        const QVector<CondRule> rules = m_session->condRules(m_sheet);
        const bool allowed = m_session->allows(m_sheet, crunch::EditSession::Allow::FormatCells);   // (a rule is a format: see condRulesRefusal)
        any = allowed && !rules.isEmpty();
        constexpr int kLook = 2000;   // (a sheet with more rules than that: the choice is on, and Clear looks for itself)
        if (allowed && rules.size() > kLook) onSelection = true;
        for (qsizetype at = 0; allowed && at < std::min<qsizetype>(rules.size(), kLook) && !onSelection; ++at) {
            const CondRule &r = rules[at];
            ++m_condScanned;
            for (const jp::office::sheet::CellRange &c : r.ranges)
                onSelection = onSelection || (c.valid && selected.valid && c.row1 <= selected.row2 && c.row2 >= selected.row1 && c.col1 <= selected.col2 && c.col2 >= selected.col1);
        }
    }
    selection->setEnabled(onSelection);
    sheet->setEnabled(any);
}

void CrunchWindow::clearCondRules(bool wholeSheet)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed() || !m_session || m_sheet < 0) return;
    const crunch::EditSession::Outcome o = m_session->clearCondRules(m_sheet, wholeSheet ? jp::office::sheet::CellRange() : selectedRange());
    if (!o.ok) notice(tr("Conditional Formatting"), o.problem);
}

} // namespace jp

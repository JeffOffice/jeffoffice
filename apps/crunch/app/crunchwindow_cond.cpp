// Conditional formatting from the Home tab: Highlight Cells Rules (greater than, less than, between, equal to, text that contains, duplicate values), Top/Bottom Rules (top and bottom items
// and percent, above and below average), and Clear Rules. A rule is asked for in a small window (a value or two, and how the cells look); the work is the session's (core/editcond.cpp).

#include "app/crunchwindow.h"

#include "core/editsession.h"
#include "grid/gridview.h"
#include "calc/dates.h"
#include "calc/formula.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QDate>
#include <QLocale>
#include <QVBoxLayout>

#include <cmath>

namespace jp {

using jp::office::sheet::CondRule;
using jp::office::sheet::DifferentialFormat;

namespace {

// How cells that meet a rule look: a font color and a fill, the sets the common choices offer.
struct Preset {
    const char *name;
    quint32 text, fill;   // 0 for none
};
const Preset kPresets[] = {{QT_TR_NOOP("Light Red Fill with Dark Red Text"), 0x9C0006, 0xFFC7CE}, {QT_TR_NOOP("Yellow Fill with Dark Yellow Text"), 0x9C5700, 0xFFEB9C},
                           {QT_TR_NOOP("Green Fill with Dark Green Text"), 0x006100, 0xC6EFCE}, {QT_TR_NOOP("Light Red Fill"), 0, 0xFFC7CE},
                           {QT_TR_NOOP("Red Text"), 0x9C0006, 0},                               {QT_TR_NOOP("Bold Dark Blue Text"), 0x1F3864, 0}};

DifferentialFormat formatOf(int preset)
{
    const Preset &p = kPresets[std::clamp(preset, 0, int(std::size(kPresets)) - 1)];
    DifferentialFormat d;
    if (p.text) d.fontColor = jp::office::sheet::ColorSpec::fromRgb(p.text);
    if (p.fill) {
        d.hasFill = true;
        d.fill.pattern = jp::office::sheet::PatternKind::Solid;
        d.fill.bg = jp::office::sheet::ColorSpec::fromRgb(p.fill);
    }
    if (preset == 5) d.bold = true;
    return d;
}

// A value as a rule's formula, or empty with the reason in `why`: a number as it is (not infinity or "not a number"), a date as its day number, TRUE and FALSE as they are, "=..." as a formula that
// must be one (its cells are the ones typed for the cell with the cursor: `dRow` and `dCol` move them to the first cell of the rule's cells), anything else as a text in quotes.
QString valueFormula(const QString &typed, int dRow, int dCol, QString *why)
{
    const QString t = typed.trimmed();
    if (t.startsWith(QLatin1Char('='))) {
        const QString body = t.mid(1).trimmed();
        if (body.isEmpty() || !jp::calc::parseFormula(body).ok()) {
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
    rule("cond.above", tr("Above Average…"), Kind::Above);
    rule("cond.below", tr("Below Average…"), Kind::Below);
    // (the two that clear are on only when there is a rule for them to clear: see updateCondActions)
    mk(QStringLiteral("cond.clearSelection"), tr("Clear Rules from Selected Cells"), QString(), QKeySequence(), [this] { clearCondRules(false); }, false, false);
    mk(QStringLiteral("cond.clearSheet"), tr("Clear Rules from This Sheet"), QString(), QKeySequence(), [this] { clearCondRules(true); }, false, false);
    connect(m_view, &grid::GridView::selectionChanged, this, [this] { updateCondActions(); });
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
        form->addRow(answer->kind == Kind::Between ? tr("From:") : count ? tr("How many:") : answer->kind == Kind::Contains ? tr("Text:") : tr("Value:"), first);
    }
    if (answer->kind == Kind::Between) {
        second = new QLineEdit(&dialog);
        second->setObjectName(QStringLiteral("condValue2"));
        form->addRow(tr("To:"), second);
    }
    auto *look = new QComboBox(&dialog);
    look->setObjectName(QStringLiteral("condPreset"));
    for (const Preset &p : kPresets) look->addItem(tr(p.name));
    form->addRow(tr("Cells look like this:"), look);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return false;
    if (first) answer->value1 = first->text();
    if (second) answer->value2 = second->text();
    answer->preset = look->currentIndex();
    return true;
}

void CrunchWindow::makeCondRule(CondAnswer::Kind kind)
{
    using Kind = CondAnswer::Kind;
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed() || !m_session || m_sheet < 0) return;
    CondAnswer answer;
    answer.kind = kind;
    if (!askCondRule(&answer)) return;
    const jp::office::sheet::CellRange range = selectedRange();
    CondRule rule;
    rule.ranges.append(range);
    const bool valueKind = kind == Kind::Greater || kind == Kind::Less || kind == Kind::Between || kind == Kind::Equal || kind == Kind::Contains;
    if (valueKind && answer.value1.trimmed().isEmpty()) {
        notice(tr("Conditional Formatting"), tr("Type a value for the rule."));
        return;
    }
    // (cells a typed formula names are as seen from the cell with the cursor; a rule holds them as seen from the first of its cells)
    const grid::Selection &sel = m_view->selection();
    const int dRow = int(range.row1) - int(std::max<qint64>(0, sel.activeRow)), dCol = int(range.col1) - int(std::max<qint64>(0, sel.activeCol));
    QString why;
    auto formula = [&](const QString &typed) { return valueFormula(typed, dRow, dCol, &why); };
    switch (kind) {
    case Kind::Greater:
    case Kind::Less:
    case Kind::Equal:
        rule.type = CondRule::Type::CellIs;
        rule.op = kind == Kind::Greater ? CondRule::Operator::GreaterThan : kind == Kind::Less ? CondRule::Operator::LessThan : CondRule::Operator::Equal;
        rule.formula1 = formula(answer.value1);
        break;
    case Kind::Between: {
        if (answer.value2.trimmed().isEmpty()) {
            notice(tr("Conditional Formatting"), tr("Type the two values the cells are between."));
            return;
        }
        rule.type = CondRule::Type::CellIs;
        rule.op = CondRule::Operator::Between;
        QString from = formula(answer.value1), to = formula(answer.value2);
        bool a = false, b = false;
        const double x = from.toDouble(&a), y = to.toDouble(&b);
        if (a && b && x > y) std::swap(from, to);   // (from the larger to the smaller: the other way round)
        rule.formula1 = from;
        rule.formula2 = to;
        break;
    }
    case Kind::Contains: {
        rule.type = CondRule::Type::ContainsText;
        rule.text = answer.value1;
        QString quoted = answer.value1;
        quoted.replace(QLatin1Char('"'), QStringLiteral("\"\""));
        rule.formula1 = QStringLiteral("NOT(ISERROR(SEARCH(\"%1\",%2)))").arg(quoted, jp::office::sheet::cellName(range.row1, range.col1));
        break;
    }
    case Kind::Duplicate: rule.type = CondRule::Type::DuplicateValues; break;
    case Kind::Top:
    case Kind::Bottom:
    case Kind::TopPercent:
    case Kind::BottomPercent: {
        rule.type = CondRule::Type::Top10;
        rule.percent = kind == Kind::TopPercent || kind == Kind::BottomPercent;
        rule.bottom = kind == Kind::Bottom || kind == Kind::BottomPercent;
        bool ok = false;
        const int n = QLocale().toInt(answer.value1.trimmed(), &ok);
        rule.rank = std::clamp(ok ? n : 10, 1, rule.percent ? 100 : 1000);
        break;
    }
    case Kind::Above:
    case Kind::Below:
        rule.type = CondRule::Type::AboveAverage;
        rule.above = kind == Kind::Above;
        break;
    }
    if (!why.isEmpty()) {
        notice(tr("Conditional Formatting"), why);
        return;
    }
    const crunch::EditSession::Outcome o = m_session->addCondRule(m_sheet, rule, formatOf(answer.preset));
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
        any = !rules.isEmpty();
        constexpr int kLook = 2000;   // (a sheet with more rules than that: the choice is on, and Clear looks for itself)
        if (rules.size() > kLook) onSelection = true;
        for (qsizetype at = 0; at < std::min<qsizetype>(rules.size(), kLook) && !onSelection; ++at) {
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

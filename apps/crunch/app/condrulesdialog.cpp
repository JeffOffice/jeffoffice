#include "app/uistyle.h"
#include "app/condrulesdialog.h"

#include "app/condpresets.h"
#include "grid/condformat.h"
#include "sheet/cellref.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QRegularExpression>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::ColorSpec;
using jp::office::sheet::CondRule;
using jp::office::sheet::CondValue;
using Kind = CondSpec::Kind;

namespace {

struct KindName {
    Kind kind;
    const char *text;
};
const KindName kKinds[] = {{Kind::CellValue, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Cell value")},
                           {Kind::TextContains, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Text that contains")},
                           {Kind::TextNotContains, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Text that does not contain")},
                           {Kind::TextBegins, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Text that begins with")},
                           {Kind::TextEnds, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Text that ends with")},
                           {Kind::DateOccurring, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "A date occurring")},
                           {Kind::Blanks, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Blank cells")},
                           {Kind::NoBlanks, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Cells that are not blank")},
                           {Kind::Errors, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Cells with errors")},
                           {Kind::NoErrors, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Cells without errors")},
                           {Kind::Top, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Top items")},
                           {Kind::Bottom, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Bottom items")},
                           {Kind::TopPercent, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Top percent")},
                           {Kind::BottomPercent, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Bottom percent")},
                           {Kind::Above, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Above average")},
                           {Kind::Below, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Below average")},
                           {Kind::Duplicate, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Duplicate values")},
                           {Kind::Unique, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Unique values")},
                           {Kind::Formula, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "A formula")},
                           {Kind::ColorScale, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Color scale")},
                           {Kind::DataBar, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Data bar")},
                           {Kind::IconSet, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Icon set")}};

// Pages of the stack: 0 nothing more, 1 a cell value, 2 text, 3 dates, 4 a count, 5 a formula, 6 color scale, 7 data bar, 8 icon set.
int pageOf(Kind k)
{
    switch (k) {
    case Kind::CellValue: return 1;
    case Kind::TextContains:
    case Kind::TextNotContains:
    case Kind::TextBegins:
    case Kind::TextEnds: return 2;
    case Kind::DateOccurring: return 3;
    case Kind::Top:
    case Kind::Bottom:
    case Kind::TopPercent:
    case Kind::BottomPercent: return 4;
    case Kind::Formula: return 5;
    case Kind::ColorScale: return 6;
    case Kind::DataBar: return 7;
    case Kind::IconSet: return 8;
    default: return 0;
    }
}

struct OpName {
    CondRule::Operator op;
    const char *text;
};
const OpName kOps[] = {{CondRule::Operator::GreaterThan, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "greater than")},
                       {CondRule::Operator::LessThan, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "less than")},
                       {CondRule::Operator::Equal, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "equal to")},
                       {CondRule::Operator::NotEqual, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "not equal to")},
                       {CondRule::Operator::GreaterThanOrEqual, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "greater than or equal to")},
                       {CondRule::Operator::LessThanOrEqual, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "less than or equal to")},
                       {CondRule::Operator::Between, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "between")},
                       {CondRule::Operator::NotBetween, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "not between")}};

struct TypeName {
    CondValue::Type type;
    const char *text;
};
const TypeName kTypes[] = {{CondValue::Type::Min, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Lowest value")},
                           {CondValue::Type::Number, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Number")},
                           {CondValue::Type::Percent, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Percent")},
                           {CondValue::Type::Formula, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Formula")},
                           {CondValue::Type::Percentile, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Percentile")},
                           {CondValue::Type::Max, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Highest value")},
                           {CondValue::Type::AutoMin, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Automatic")},
                           {CondValue::Type::AutoMax, QT_TRANSLATE_NOOP("jp::CondRuleDialog", "Automatic")}};

// A list of the kinds of value a boundary may be: a scale or a bar's end (`ends`), or an icon's step (no lowest or highest).
QComboBox *typeCombo(QWidget *parent, CondValue::Type now, bool low, bool high, bool automatic)
{
    auto *c = new QComboBox(parent);
    for (const TypeName &t : kTypes) {
        const bool autoType = t.type == CondValue::Type::AutoMin || t.type == CondValue::Type::AutoMax;
        if (t.type == CondValue::Type::Min && !low) continue;
        if (t.type == CondValue::Type::Max && !high) continue;
        if (autoType && (!automatic || (t.type == CondValue::Type::AutoMin) != low)) continue;
        c->addItem(QCoreApplication::translate("jp::CondRuleDialog", t.text), int(t.type));
    }
    const int at = c->findData(int(now));
    c->setCurrentIndex(at >= 0 ? at : 0);
    return c;
}

CondValue valueOf(QComboBox *type, QLineEdit *text)
{
    CondValue v;
    v.type = CondValue::Type(type->currentData().toInt());
    if (v.type == CondValue::Type::Number || v.type == CondValue::Type::Percent || v.type == CondValue::Type::Percentile || v.type == CondValue::Type::Formula) v.value = text->text().trimmed();
    return v;
}

QString valueTextOf(const CondValue &v) { return v.value; }

// What a rule's value is as typed: a text in quotes without them, a formula that is not a number or TRUE and FALSE with an equals sign.
QString typedOf(const QString &formula)
{
    if (formula.size() >= 2 && formula.startsWith(QLatin1Char('"')) && formula.endsWith(QLatin1Char('"'))) {
        QString t = formula.mid(1, formula.size() - 2);
        t.replace(QStringLiteral("\"\""), QStringLiteral("\""));
        return t;
    }
    bool number = false;
    formula.toDouble(&number);
    if (number || formula == QLatin1String("TRUE") || formula == QLatin1String("FALSE") || formula.isEmpty()) return formula;
    return QLatin1Char('=') + formula;
}

} // namespace

CondColorButton::CondColorButton(const ColorSpec &color, QWidget *parent) : QPushButton(parent)
{
    setObjectName(QStringLiteral("condColor"));
    setFixedWidth(64);
    setColor(color);
    connect(this, &QPushButton::clicked, this, [this] {
        const QColor now = m_color.isSet() ? QColor::fromRgb(m_color.rgb) : QColor(Qt::white);
        const QColor picked = QColorDialog::getColor(now, this, tr("Color"));
        if (picked.isValid()) setColor(ColorSpec::fromRgb(picked.rgb() & 0xFFFFFF));
    });
}

void CondColorButton::setColor(const ColorSpec &color)
{
    m_color = color;
    QPixmap pm(40, 14);
    pm.fill(color.isSet() ? QColor::fromRgb(color.rgb) : QColor(Qt::transparent));
    QPainter p(&pm);
    p.setPen(Qt::gray);
    p.drawRect(pm.rect().adjusted(0, 0, -1, -1));
    setIcon(QIcon(pm));
    setIconSize(pm.size());
    setAccessibleName(tr("Color"));
}

CondRuleDialog::CondRuleDialog(const CondSpec &start, bool editing, QWidget *parent) : QDialog(parent), m_start(start)
{
    setObjectName(QStringLiteral("condRuleDialog"));
    setWindowTitle(editing ? tr("Edit Rule") : tr("New Rule"));
    build(start, editing);
}

void CondRuleDialog::build(const CondSpec &start, bool editing)
{
    auto *layout = new QVBoxLayout(this);
    auto *kindRow = new QFormLayout;
    m_kind = new QComboBox(this);
    m_kind->setObjectName(QStringLiteral("condKind"));
    for (const KindName &k : kKinds) m_kind->addItem(tr(k.text), int(k.kind));
    kindRow->addRow(tr("Format cells with"), m_kind);
    layout->addLayout(kindRow);
    m_pages = new QStackedWidget(this);
    layout->addWidget(m_pages);
    auto formPage = [&](QFormLayout **out) {
        auto *w = new QWidget(m_pages);
        auto *f = new QFormLayout(w);
        f->setContentsMargins(0, 0, 0, 0);
        m_pages->addWidget(w);
        *out = f;
        return w;
    };
    QFormLayout *f = nullptr;
    formPage(&f);   // 0
    f->addRow(new QLabel(tr("The cells that meet this are formatted."), this));
    formPage(&f);   // 1
    m_op = new QComboBox(this);
    m_op->setObjectName(QStringLiteral("condOp"));
    for (const OpName &o : kOps) m_op->addItem(tr(o.text), int(o.op));
    m_v1 = new QLineEdit(this);
    m_v1->setObjectName(QStringLiteral("condValue1"));
    m_v2 = new QLineEdit(this);
    m_v2->setObjectName(QStringLiteral("condValue2"));
    f->addRow(tr("Cell value"), m_op);
    f->addRow(tr("Value"), m_v1);
    f->addRow(tr("And"), m_v2);
    connect(m_op, &QComboBox::currentIndexChanged, this, [this] {
        const auto op = CondRule::Operator(m_op->currentData().toInt());
        m_v2->setEnabled(op == CondRule::Operator::Between || op == CondRule::Operator::NotBetween);
    });
    formPage(&f);   // 2
    m_text = new QLineEdit(this);
    m_text->setObjectName(QStringLiteral("condText"));
    f->addRow(tr("Text"), m_text);
    formPage(&f);   // 3
    m_period = new QComboBox(this);
    m_period->setObjectName(QStringLiteral("condPeriod"));
    for (const condpresets::Period &p : condpresets::periods()) m_period->addItem(QCoreApplication::translate("jp::CrunchWindow", p.text), QString::fromLatin1(p.name));
    f->addRow(tr("Dates"), m_period);
    formPage(&f);   // 4
    m_count = new QSpinBox(this);
    m_count->setObjectName(QStringLiteral("condCount"));
    m_count->setRange(1, 1000);
    f->addRow(tr("How many"), m_count);
    formPage(&f);   // 5
    m_formula = new QLineEdit(this);
    m_formula->setObjectName(QStringLiteral("condFormula"));
    f->addRow(tr("Formula that is true for the cells to format"), m_formula);
    m_pages->addWidget(scalePage(start.visual));   // 6
    m_pages->addWidget(barPage(start.visual));     // 7
    m_pages->addWidget(iconPage(start.visual));    // 8

    auto *looks = new QFormLayout;
    m_looksForm = looks;
    m_lookBox = new CondLookBox(editing, start.currentLook, this);
    looks->addRow(tr("Cells look like this"), m_lookBox);
    m_stop = new QCheckBox(tr("Stop if true"), this);
    m_stop->setObjectName(QStringLiteral("condStop"));
    looks->addRow(QString(), m_stop);
    layout->addLayout(looks);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // what the rule is now
    m_kind->setCurrentIndex(std::max(0, m_kind->findData(int(start.kind))));
    m_op->setCurrentIndex(std::max(0, m_op->findData(int(start.op))));
    m_v1->setText(start.v1);
    m_v2->setText(start.v2);
    m_text->setText(start.text);
    m_period->setCurrentIndex(std::max(0, m_period->findData(start.period)));
    m_count->setValue(start.count);
    m_formula->setText(start.v1);
    m_stop->setChecked(start.stopIfTrue);
    CondLookBox::Choice startLook;
    startLook.keep = editing && start.keepLook;
    startLook.preset = start.look;
    startLook.custom = start.useCustom;
    startLook.look = start.custom;
    m_lookBox->setChoice(startLook);
    Q_EMIT m_op->currentIndexChanged(m_op->currentIndex());
    connect(m_kind, &QComboBox::currentIndexChanged, this, [this] { showPage(); });
    showPage();
}

void CondRuleDialog::setLookContext(const CondLookContext &context) { m_lookBox->setContext(context); }

void CondRuleDialog::showPage()
{
    const Kind k = Kind(m_kind->currentData().toInt());
    m_pages->setCurrentIndex(pageOf(k));
    // (the pages take the room of the page that is shown, not of the tallest)
    for (int i = 0; i < m_pages->count(); ++i)
        m_pages->widget(i)->setSizePolicy(QSizePolicy::Preferred, i == m_pages->currentIndex() ? QSizePolicy::Preferred : QSizePolicy::Ignored);
    CondSpec probe;
    probe.kind = k;
    m_looksForm->setRowVisible(m_lookBox, probe.hasLook());   // (a color scale, data bar, or icon set has no look of font and fill)
    if (isVisible()) {
        layout()->activate();
        adjustSize();
    }
}

void CondRuleDialog::setKind(Kind kind)
{
    m_kind->setCurrentIndex(std::max(0, m_kind->findData(int(kind))));
}

QWidget *CondRuleDialog::scalePage(const CondRule &rule)
{
    auto *w = new QWidget(m_pages);
    auto *f = new QFormLayout(w);
    f->setContentsMargins(0, 0, 0, 0);
    const bool three = rule.type == CondRule::Type::ColorScale ? rule.colors.size() == 3 : true;
    m_threeColors = new QCheckBox(tr("Use three colors"), w);
    m_threeColors->setObjectName(QStringLiteral("scaleThree"));
    m_threeColors->setChecked(three);
    f->addRow(m_threeColors);
    const QStringList names = {tr("Lowest"), tr("Midpoint"), tr("Highest")};
    for (int i = 0; i < 3; ++i) {
        const bool scale = rule.type == CondRule::Type::ColorScale;
        const int vi = scale && rule.values.size() == 2 && i == 2 ? 1 : i;
        CondValue v;
        v.type = i == 0 ? CondValue::Type::Min : i == 1 ? CondValue::Type::Percentile : CondValue::Type::Max;
        if (i == 1) v.value = QStringLiteral("50");
        if (scale && vi < rule.values.size() && !(i == 1 && rule.values.size() == 2)) v = rule.values[vi];
        const QVector<quint32> defaults = {0xF8696B, 0xFFEB84, 0x63BE7B};
        ColorSpec c = ColorSpec::fromRgb(defaults[i]);
        if (scale) {
            const int ci = rule.colors.size() == 2 && i == 2 ? 1 : i;
            if (!(i == 1 && rule.colors.size() == 2) && ci < rule.colors.size()) c = rule.colors[ci];
        }
        m_scaleType[i] = typeCombo(w, v.type, i != 2 || true, true, false);
        m_scaleType[i]->setObjectName(QStringLiteral("scaleType%1").arg(i));
        m_scaleValue[i] = new QLineEdit(valueTextOf(v), w);
        m_scaleValue[i]->setObjectName(QStringLiteral("scaleValue%1").arg(i));
        m_scaleColor[i] = new CondColorButton(c, w);
        m_scaleColor[i]->setObjectName(QStringLiteral("scaleColor%1").arg(i));
        auto *row = new QHBoxLayout;
        row->addWidget(m_scaleType[i]);
        row->addWidget(m_scaleValue[i]);
        row->addWidget(m_scaleColor[i]);
        f->addRow(names[i], row);
    }
    connect(m_threeColors, &QCheckBox::toggled, this, [this](bool on) {
        m_scaleType[1]->setEnabled(on);
        m_scaleValue[1]->setEnabled(on);
        m_scaleColor[1]->setEnabled(on);
    });
    Q_EMIT m_threeColors->toggled(three);
    return w;
}

QWidget *CondRuleDialog::barPage(const CondRule &rule)
{
    auto *w = new QWidget(m_pages);
    auto *f = new QFormLayout(w);
    f->setContentsMargins(0, 0, 0, 0);
    const bool bar = rule.type == CondRule::Type::DataBar;
    CondValue lo, hi;
    lo.type = CondValue::Type::Min;
    hi.type = CondValue::Type::Max;
    if (bar && rule.values.size() >= 2) {
        lo = rule.values[0];
        hi = rule.values[1];
    }
    m_barMin = typeCombo(w, lo.type, true, true, true);
    m_barMin->setObjectName(QStringLiteral("barMin"));
    m_barMinValue = new QLineEdit(valueTextOf(lo), w);
    m_barMinValue->setObjectName(QStringLiteral("barMinValue"));
    m_barMax = typeCombo(w, hi.type, false, true, true);
    m_barMax->setObjectName(QStringLiteral("barMax"));
    m_barMaxValue = new QLineEdit(valueTextOf(hi), w);
    m_barMaxValue->setObjectName(QStringLiteral("barMaxValue"));
    auto *r1 = new QHBoxLayout, *r2 = new QHBoxLayout;
    r1->addWidget(m_barMin);
    r1->addWidget(m_barMinValue);
    r2->addWidget(m_barMax);
    r2->addWidget(m_barMaxValue);
    f->addRow(tr("Shortest bar"), r1);
    f->addRow(tr("Longest bar"), r2);
    m_barFill = new QComboBox(w);
    m_barFill->setObjectName(QStringLiteral("barFill"));
    m_barFill->addItem(tr("Gradient fill"), 1);
    m_barFill->addItem(tr("Solid fill"), 0);
    m_barFill->setCurrentIndex(!bar || rule.gradient ? 0 : 1);
    f->addRow(tr("Fill"), m_barFill);
    m_barColor = new CondColorButton(bar && !rule.colors.isEmpty() ? rule.colors[0] : ColorSpec::fromRgb(0x638EC6), w);
    m_barColor->setObjectName(QStringLiteral("barColor"));
    f->addRow(tr("Bar color"), m_barColor);
    m_barBorderOn = new QCheckBox(tr("Border"), w);
    m_barBorderOn->setObjectName(QStringLiteral("barBorderOn"));
    m_barBorderOn->setChecked(bar && rule.border);
    m_barBorder = new CondColorButton(bar && rule.borderColor.isSet() ? rule.borderColor : m_barColor->color(), w);
    m_barBorder->setObjectName(QStringLiteral("barBorder"));
    auto *r3 = new QHBoxLayout;
    r3->addWidget(m_barBorder, 0, Qt::AlignVCenter);
    r3->addWidget(m_barBorderOn, 0, Qt::AlignVCenter);
    r3->addStretch();
    f->addRow(QString(), r3);
    m_barNegative = new CondColorButton(bar && rule.negativeColor.isSet() ? rule.negativeColor : ColorSpec::fromRgb(0xFF0000), w);
    m_barNegative->setObjectName(QStringLiteral("barNegative"));
    m_barNegativeSame = new QCheckBox(tr("Same color as positive bars"), w);
    m_barNegativeSame->setObjectName(QStringLiteral("barNegativeSame"));
    m_barNegativeSame->setChecked(bar && rule.negativeSameColor);
    auto *r4 = new QHBoxLayout;
    r4->addWidget(m_barNegative);
    r4->addWidget(m_barNegativeSame);
    f->addRow(tr("Negative bar color"), r4);
    m_barAxis = new QComboBox(w);
    m_barAxis->setObjectName(QStringLiteral("barAxis"));
    m_barAxis->addItem(tr("Automatic"), int(CondRule::Axis::Automatic));
    m_barAxis->addItem(tr("Middle of the cell"), int(CondRule::Axis::Middle));
    m_barAxis->addItem(tr("No axis"), int(CondRule::Axis::None));
    m_barAxis->setCurrentIndex(bar ? std::max(0, m_barAxis->findData(int(rule.axis))) : 0);
    f->addRow(tr("Axis"), m_barAxis);
    m_barDirection = new QComboBox(w);
    m_barDirection->setObjectName(QStringLiteral("barDirection"));
    m_barDirection->addItem(tr("Left to right"), 0);
    m_barDirection->addItem(tr("Right to left"), 1);
    m_barDirection->setCurrentIndex(bar && rule.rightToLeft ? 1 : 0);
    f->addRow(tr("Bar direction"), m_barDirection);
    m_barOnly = new QCheckBox(tr("Show bar only"), w);
    m_barOnly->setObjectName(QStringLiteral("barOnly"));
    m_barOnly->setChecked(bar && !rule.showValue);
    f->addRow(m_barOnly);
    return w;
}

QWidget *CondRuleDialog::iconPage(const CondRule &rule)
{
    auto *w = new QWidget(m_pages);
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    auto *top = new QFormLayout;
    m_iconSet = new QComboBox(w);
    m_iconSet->setObjectName(QStringLiteral("iconSet"));
    for (const QString &name : jp::grid::iconSetNames()) m_iconSet->addItem(QIcon(condpresets::iconSetStrip(name, 18)), condpresets::iconSetLabel(name), name);
    const bool icons = rule.type == CondRule::Type::IconSet;
    m_iconSet->setCurrentIndex(std::max(0, m_iconSet->findData(icons ? jp::grid::iconSetOrDefault(rule.iconSet) : QStringLiteral("3Arrows"))));
    top->addRow(tr("Icon style"), m_iconSet);
    v->addLayout(top);
    m_iconRows = new QWidget(w);
    new QFormLayout(m_iconRows);
    v->addWidget(m_iconRows);
    m_iconReverse = new QCheckBox(tr("Reverse icon order"), w);
    m_iconReverse->setObjectName(QStringLiteral("iconReverse"));
    m_iconReverse->setChecked(icons && rule.reverse);
    m_iconOnly = new QCheckBox(tr("Show icon only"), w);
    m_iconOnly->setObjectName(QStringLiteral("iconOnly"));
    m_iconOnly->setChecked(icons && !rule.showValue);
    v->addWidget(m_iconReverse);
    v->addWidget(m_iconOnly);
    if (icons) m_iconStart = rule;
    connect(m_iconSet, &QComboBox::currentIndexChanged, this, [this] { fillIconRows(); });
    fillIconRows();
    return w;
}

// One line for each icon above the lowest: the icon is for values from this one up (or over it), as a percent, a number, a percentile, or a formula.
void CondRuleDialog::fillIconRows()
{
    const QString set = m_iconSet->currentData().toString();
    const int n = std::max(2, jp::grid::iconSetCount(set));
    auto *form = static_cast<QFormLayout *>(m_iconRows->layout());
    while (form->rowCount() > 0) form->removeRow(0);
    m_iconType.clear();
    m_iconValue.clear();
    m_iconGte.clear();
    const bool same = m_iconStart.type == CondRule::Type::IconSet && m_iconStart.values.size() == n;
    for (int i = n - 1; i >= 1; --i) {
        CondValue val;
        val.type = CondValue::Type::Percent;
        val.value = QString::number(i * 100 / n);
        if (same) val = m_iconStart.values[i];
        auto *type = typeCombo(m_iconRows, val.type, false, false, false);
        type->setObjectName(QStringLiteral("iconType%1").arg(i));
        auto *text = new QLineEdit(valueTextOf(val), m_iconRows);
        text->setObjectName(QStringLiteral("iconValue%1").arg(i));
        auto *gte = new QComboBox(m_iconRows);
        gte->setObjectName(QStringLiteral("iconGte%1").arg(i));
        gte->addItem(QStringLiteral(">="), 1);
        gte->addItem(QStringLiteral(">"), 0);
        gte->setCurrentIndex(val.gte ? 0 : 1);
        auto *row = new QHBoxLayout;
        auto *icon = new QLabel(m_iconRows);
        icon->setTextFormat(Qt::PlainText);
        icon->setPixmap(jp::grid::iconSetPixmap(set, i, 16));
        row->addWidget(icon);
        row->addWidget(new QLabel(tr("When value is"), m_iconRows));
        row->addWidget(gte);
        row->addWidget(text);
        row->addWidget(type);
        form->addRow(row);
        m_iconType.prepend(type);
        m_iconValue.prepend(text);
        m_iconGte.prepend(gte);
    }
}

CondSpec CondRuleDialog::spec() const
{
    CondSpec s;
    s.kind = Kind(m_kind->currentData().toInt());
    s.op = CondRule::Operator(m_op->currentData().toInt());
    s.v1 = s.kind == Kind::Formula ? m_formula->text() : m_v1->text();
    s.v2 = m_v2->text();
    s.text = m_text->text();
    s.period = m_period->currentData().toString();
    s.count = m_count->value();
    s.stopIfTrue = m_stop->isChecked();
    const CondLookBox::Choice look = m_lookBox->choice();
    s.keepLook = look.keep;
    s.look = look.preset;
    s.useCustom = look.custom;
    s.custom = look.look;
    s.currentLook = m_start.currentLook;
    CondRule r;
    r.type = CondRule::Type::ColorScale;
    if (s.kind == Kind::ColorScale) {
        const bool three = m_threeColors->isChecked();
        for (int i = 0; i < 3; ++i) {
            if (i == 1 && !three) continue;
            r.values.append(valueOf(m_scaleType[i], m_scaleValue[i]));
            r.colors.append(m_scaleColor[i]->color());
        }
    } else if (s.kind == Kind::DataBar) {
        r.type = CondRule::Type::DataBar;
        r.values = {valueOf(m_barMin, m_barMinValue), valueOf(m_barMax, m_barMaxValue)};
        r.colors = {m_barColor->color()};
        r.gradient = m_barFill->currentData().toInt() != 0;
        r.border = m_barBorderOn->isChecked();
        r.borderColor = m_barBorder->color();
        r.negativeColor = m_barNegative->color();
        r.negativeBorderColor = m_barNegative->color();
        r.negativeSameColor = m_barNegativeSame->isChecked();
        r.axis = CondRule::Axis(m_barAxis->currentData().toInt());
        r.rightToLeft = m_barDirection->currentData().toInt() != 0;
        r.showValue = !m_barOnly->isChecked();
    } else if (s.kind == Kind::IconSet) {
        r.type = CondRule::Type::IconSet;
        r.iconSet = m_iconSet->currentData().toString();
        CondValue first;
        first.type = CondValue::Type::Percent;
        first.value = QStringLiteral("0");
        r.values = {first};
        for (int i = 0; i < m_iconType.size(); ++i) {
            CondValue v = valueOf(m_iconType[i], m_iconValue[i]);
            v.gte = m_iconGte[i]->currentData().toInt() != 0;
            r.values.append(v);
        }
        r.reverse = m_iconReverse->isChecked();
        r.showValue = !m_iconOnly->isChecked();
    }
    s.visual = r;
    return s;
}

// ---- Manage Rules

namespace {

QString rangesText(const QVector<CellRange> &ranges)
{
    QStringList names;
    for (const CellRange &r : ranges)
        if (r.valid) names << jp::office::sheet::rangeName(r);
    return names.join(QLatin1Char(' '));
}

} // namespace

CondManageDialog::CondManageDialog(const QVector<Item> &items, const Editor &editor, QWidget *parent) : QDialog(parent), m_items(items), m_editor(editor)
{
    setObjectName(QStringLiteral("condManageDialog"));
    setWindowTitle(tr("Manage Rules"));
    resize(640, 360);
    auto *layout = new QVBoxLayout(this);
    auto *row = new QHBoxLayout;
    layout->addLayout(row);
    auto button = [&](const char *name, const QString &text, const std::function<void()> &fn) {
        auto *b = new QPushButton(text, this);
        b->setObjectName(QString::fromLatin1(name));
        connect(b, &QPushButton::clicked, this, [fn] { fn(); });
        row->addWidget(b);
    };
    button("condNew", tr("New Rule…"), [this] { addRule(); });
    button("condEdit", tr("Edit Rule…"), [this] { editSelected(); });
    button("condDelete", tr("Delete Rule"), [this] { deleteSelected(); });
    button("condUp", tr("Move Up"), [this] { moveSelected(-1); });
    button("condDown", tr("Move Down"), [this] { moveSelected(1); });
    row->addStretch();
    m_table = new QTableWidget(0, 4, this);
    m_table->setObjectName(QStringLiteral("condRules"));
    m_table->setHorizontalHeaderLabels({tr("Rule"), tr("Format"), tr("Applies to"), tr("Stop if true")});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(0, 230);
    m_table->setColumnWidth(1, 110);
    m_table->setColumnWidth(2, 130);
    layout->addWidget(m_table);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_table, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *it) {
        if (m_filling) return;
        if (it->column() == 2 && !setRangesOf(it->row(), it->text())) refill();
        else if (it->column() == 3) setStopIfTrue(it->row(), it->checkState() == Qt::Checked);
    });
    refill();
}

int CondManageDialog::selectedRow() const
{
    const QList<QTableWidgetSelectionRange> ranges = m_table->selectedRanges();
    return ranges.isEmpty() ? -1 : ranges.first().topRow();
}

void CondManageDialog::select(int row)
{
    m_table->selectRow(row);
}

void CondManageDialog::refill()
{
    const int keep = selectedRow();
    m_filling = true;
    m_table->setRowCount(int(m_items.size()));
    for (int i = 0; i < m_items.size(); ++i) {
        const Item &it = m_items[i];
        auto *rule = new QTableWidgetItem(condpresets::describe(it.rule));
        rule->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        if (it.rule.type == CondRule::Type::DataBar && !it.rule.colors.isEmpty() && it.rule.colors[0].isSet())
            rule->setIcon(QIcon(condpresets::barPixmap(it.rule.colors[0].rgb, it.rule.gradient, 16)));
        else if (it.rule.type == CondRule::Type::ColorScale) {
            QVector<quint32> colors;
            for (const ColorSpec &c : it.rule.colors) colors.append(c.rgb);
            rule->setIcon(QIcon(condpresets::scalePixmap(colors, 16)));
        } else if (it.rule.type == CondRule::Type::IconSet) {
            rule->setIcon(QIcon(condpresets::iconSetStrip(jp::grid::iconSetOrDefault(it.rule.iconSet), 16)));
        }
        m_table->setItem(i, 0, rule);
        auto *look = new QTableWidgetItem(it.look.isEmpty() ? QString() : tr("AaBbCcYyZz"));
        look->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        if (it.look.fontColor.isSet()) look->setForeground(QColor::fromRgb(it.look.fontColor.rgb));
        if (it.look.hasFill && (it.look.fill.bg.isSet() || it.look.fill.fg.isSet())) look->setBackground(QColor::fromRgb((it.look.fill.bg.isSet() ? it.look.fill.bg : it.look.fill.fg).rgb));
        QFont f = look->font();
        if (it.look.bold.value_or(false)) f.setBold(true);
        if (it.look.italic.value_or(false)) f.setItalic(true);
        if (it.look.strike.value_or(false)) f.setStrikeOut(true);
        if (it.look.underline.value_or(jp::office::sheet::Underline::None) != jp::office::sheet::Underline::None) f.setUnderline(true);
        look->setFont(f);
        m_table->setItem(i, 1, look);
        auto *where = new QTableWidgetItem(rangesText(it.rule.ranges));
        m_table->setItem(i, 2, where);
        auto *stop = new QTableWidgetItem;
        stop->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        stop->setCheckState(it.rule.stopIfTrue ? Qt::Checked : Qt::Unchecked);
        m_table->setItem(i, 3, stop);
    }
    m_filling = false;
    if (keep >= 0 && keep < m_items.size()) m_table->selectRow(keep);
}

QVector<crunch::EditSession::CondManaged> CondManageDialog::result() const
{
    QVector<crunch::EditSession::CondManaged> out;
    for (const Item &it : m_items) {
        crunch::EditSession::CondManaged m;
        m.rule = it.rule;
        m.changed = it.changed;
        m.newLook = it.newLook;
        m.look = it.look;
        out.append(m);
    }
    return out;
}

void CondManageDialog::addRule()
{
    Item made;
    if (!m_editor || !m_editor(nullptr, jp::office::sheet::DifferentialFormat(), &made)) return;
    made.changed = true;
    made.rule.uid = 0;
    m_items.prepend(made);
    refill();
    m_table->selectRow(0);
}

void CondManageDialog::editSelected()
{
    const int row = selectedRow();
    if (row < 0 || row >= m_items.size() || !m_editor) return;
    Item made = m_items[row];
    if (!m_editor(&m_items[row].rule, m_items[row].look, &made)) return;
    made.changed = true;
    m_items[row] = made;
    refill();
}

void CondManageDialog::deleteSelected()
{
    const int row = selectedRow();
    if (row < 0 || row >= m_items.size()) return;
    m_items.remove(row);
    refill();
    if (!m_items.isEmpty()) m_table->selectRow(std::min(row, int(m_items.size()) - 1));
}

void CondManageDialog::moveSelected(int by)
{
    const int row = selectedRow(), to = row + by;
    if (row < 0 || to < 0 || to >= m_items.size()) return;
    std::swap(m_items[row], m_items[to]);
    refill();
    m_table->selectRow(to);
}

bool CondManageDialog::setRangesOf(int row, const QString &text)
{
    if (row < 0 || row >= m_items.size()) return false;
    QVector<CellRange> ranges;
    for (const QString &part : text.split(QRegularExpression(QStringLiteral("[\\s,;]+")), Qt::SkipEmptyParts)) {
        CellRange r;
        if (!jp::office::sheet::parseRangeRef(part, &r) || !r.valid) return false;
        ranges.append(r);
        if (ranges.size() > 1000) return false;
    }
    if (ranges.isEmpty()) return false;
    m_items[row].rule.ranges = ranges;
    m_items[row].changed = true;
    return true;
}

void CondManageDialog::setStopIfTrue(int row, bool stop)
{
    if (row < 0 || row >= m_items.size() || m_items[row].rule.stopIfTrue == stop) return;
    m_items[row].rule.stopIfTrue = stop;
    m_items[row].changed = true;
}

} // namespace jp

namespace jp {

CondSpec specOfRule(const CondRule &r)
{
    using T = CondRule::Type;
    CondSpec s;
    s.stopIfTrue = r.stopIfTrue;
    s.keepLook = true;
    switch (r.type) {
    case T::CellIs:
        s.kind = Kind::CellValue;
        s.op = r.op;
        s.v1 = typedOf(r.formula1);
        s.v2 = typedOf(r.formula2);
        break;
    case T::Expression:
        s.kind = Kind::Formula;
        s.v1 = QLatin1Char('=') + r.formula1;
        break;
    case T::ContainsText: s.kind = Kind::TextContains; s.text = r.text; break;
    case T::NotContainsText: s.kind = Kind::TextNotContains; s.text = r.text; break;
    case T::BeginsWith: s.kind = Kind::TextBegins; s.text = r.text; break;
    case T::EndsWith: s.kind = Kind::TextEnds; s.text = r.text; break;
    case T::TimePeriod: s.kind = Kind::DateOccurring; s.period = r.timePeriod; break;
    case T::ContainsBlanks: s.kind = Kind::Blanks; break;
    case T::NotContainsBlanks: s.kind = Kind::NoBlanks; break;
    case T::ContainsErrors: s.kind = Kind::Errors; break;
    case T::NotContainsErrors: s.kind = Kind::NoErrors; break;
    case T::Top10:
        s.kind = r.percent ? (r.bottom ? Kind::BottomPercent : Kind::TopPercent) : (r.bottom ? Kind::Bottom : Kind::Top);
        s.count = r.rank;
        break;
    case T::AboveAverage: s.kind = r.above ? Kind::Above : Kind::Below; break;
    case T::DuplicateValues: s.kind = Kind::Duplicate; break;
    case T::UniqueValues: s.kind = Kind::Unique; break;
    case T::ColorScale: s.kind = Kind::ColorScale; s.visual = r; break;
    case T::DataBar: s.kind = Kind::DataBar; s.visual = r; break;
    case T::IconSet: s.kind = Kind::IconSet; s.visual = r; break;
    }
    return s;
}

} // namespace jp

#include "app/filterdialogs.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>

namespace jp {

using jp::office::sheet::FilterColumn;

QString CustomFilterDialog::escaped(const QString &text)
{
    QString out;
    for (const QChar c : text) {
        if (c == QLatin1Char('~') || c == QLatin1Char('*') || c == QLatin1Char('?')) out += QLatin1Char('~');
        out += c;
    }
    return out;
}

namespace {

// The text with the marks of a pattern that are not escaped: whether it has any.
bool hasWildcards(const QString &pattern)
{
    for (qsizetype i = 0; i < pattern.size(); ++i) {
        if (pattern.at(i) == QLatin1Char('~') && i + 1 < pattern.size()) {
            ++i;
            continue;
        }
        if (pattern.at(i) == QLatin1Char('*') || pattern.at(i) == QLatin1Char('?')) return true;
    }
    return false;
}

QString unescaped(const QString &pattern)
{
    QString out;
    for (qsizetype i = 0; i < pattern.size(); ++i) {
        if (pattern.at(i) == QLatin1Char('~') && i + 1 < pattern.size()) ++i;
        out += pattern.at(i);
    }
    return out;
}

} // namespace

CustomFilterDialog::CustomFilterDialog(const QString &column, bool dates, jp::calc::DateSystem system, FilterAsk ask, const FilterColumn *current, QWidget *parent)
    : QDialog(parent), m_dates(dates), m_system(system)
{
    setObjectName(QStringLiteral("customFilterDialog"));
    setWindowTitle(tr("Custom Filter"));
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Show rows where the value in \"%1\"").arg(column.left(60))));
    auto *grid = new QGridLayout;
    layout->addLayout(grid);
    const QStringList numberOps = {tr("equals"), tr("does not equal"), tr("is greater than"), tr("is greater than or equal to"), tr("is less than"), tr("is less than or equal to"),
                                   tr("begins with"), tr("does not begin with"), tr("ends with"), tr("does not end with"), tr("contains"), tr("does not contain")};
    const QStringList dateOps = {tr("equals"), tr("does not equal"), tr("is after"), tr("is on or after"), tr("is before"), tr("is on or before")};
    for (int row = 0; row < 2; ++row) {
        m_op[row] = new QComboBox;
        m_op[row]->setObjectName(QStringLiteral("filterOperator%1").arg(row));
        m_op[row]->setAccessibleName(row == 0 ? tr("First comparison") : tr("Second comparison"));
        if (row == 1) m_op[row]->addItem(tr("(none)"));
        m_op[row]->addItems(dates ? dateOps : numberOps);
        grid->addWidget(m_op[row], row == 0 ? 0 : 2, 0);
        if (dates) {
            m_date[row] = new QDateEdit(QDate::currentDate());
            m_date[row]->setObjectName(QStringLiteral("filterDate%1").arg(row));
            m_date[row]->setCalendarPopup(true);
            m_date[row]->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));
            m_date[row]->setAccessibleName(row == 0 ? tr("First day") : tr("Second day"));
            grid->addWidget(m_date[row], row == 0 ? 0 : 2, 1);
        } else {
            m_text[row] = new QLineEdit;
            m_text[row]->setObjectName(QStringLiteral("filterValue%1").arg(row));
            m_text[row]->setAccessibleName(row == 0 ? tr("First value") : tr("Second value"));
            grid->addWidget(m_text[row], row == 0 ? 0 : 2, 1);
        }
    }
    auto *joiner = new QHBoxLayout;
    m_and = new QRadioButton(tr("And"));
    m_and->setObjectName(QStringLiteral("filterAnd"));
    m_or = new QRadioButton(tr("Or"));
    m_or->setObjectName(QStringLiteral("filterOr"));
    m_and->setChecked(true);
    joiner->addWidget(m_and);
    joiner->addWidget(m_or);
    joiner->addStretch(1);
    grid->addLayout(joiner, 1, 0, 1, 2);
    if (!dates) layout->addWidget(new QLabel(tr("Use ? for any one character and * for any run of characters.")));
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    // what the rows begin as
    if (current && current->kind == FilterColumn::Kind::Custom && !current->conditions.isEmpty()) {
        for (int row = 0; row < std::min<int>(2, int(current->conditions.size())); ++row) {
            const FilterColumn::Condition &c = current->conditions[row];
            UiOp op = UiOp(int(c.op));
            QString text = c.value;
            if (!dates && (c.op == FilterColumn::Op::Equal || c.op == FilterColumn::Op::NotEqual) && hasWildcards(c.value)) {
                const bool no = c.op == FilterColumn::Op::NotEqual;
                if (c.value.endsWith(QLatin1Char('*')) && !hasWildcards(c.value.chopped(1))) {
                    op = no ? NotBeginsWith : BeginsWith;
                    text = unescaped(c.value.chopped(1));
                } else if (c.value.startsWith(QLatin1Char('*')) && !hasWildcards(c.value.mid(1))) {
                    op = no ? NotEndsWith : EndsWith;
                    text = unescaped(c.value.mid(1));
                } else if (c.value.size() >= 2 && c.value.startsWith(QLatin1Char('*')) && c.value.endsWith(QLatin1Char('*')) && !hasWildcards(c.value.mid(1, c.value.size() - 2))) {
                    op = no ? NotContains : Contains;
                    text = unescaped(c.value.mid(1, c.value.size() - 2));
                }
            }
            setRow(row, op, text);
        }
        (current->both ? m_and : m_or)->setChecked(true);
    } else {
        UiOp first = Equals;
        switch (ask) {
        case FilterAsk::NotEquals: first = NotEquals; break;
        case FilterAsk::Greater: case FilterAsk::After: first = Greater; break;
        case FilterAsk::GreaterEqual: first = GreaterEqual; break;
        case FilterAsk::Less: case FilterAsk::Before: first = Less; break;
        case FilterAsk::LessEqual: first = LessEqual; break;
        case FilterAsk::BeginsWith: first = BeginsWith; break;
        case FilterAsk::NotBeginsWith: first = NotBeginsWith; break;
        case FilterAsk::EndsWith: first = EndsWith; break;
        case FilterAsk::NotEndsWith: first = NotEndsWith; break;
        case FilterAsk::Contains: first = Contains; break;
        case FilterAsk::NotContains: first = NotContains; break;
        case FilterAsk::Between:
            first = GreaterEqual;
            setRow(1, LessEqual, QString());
            break;
        default: break;
        }
        setRow(0, first, QString());
    }
}

// The row's controls say the comparison: the boxes of dates have six comparisons (equals, does not equal, after, on or after, before, on or before).
void CustomFilterDialog::setRow(int row, UiOp op, const QString &text)
{
    if (m_dates) {
        // (the filter's comparisons in the order of the date boxes)
        static const int at[] = {0, 1, 2, 3, 4, 5};
        const int place = op <= LessEqual ? at[op] : 0;
        m_op[row]->setCurrentIndex(place + (row == 1 ? 1 : 0));
        bool ok = false;
        const double serial = text.toDouble(&ok);
        int y = 0, m = 0, d = 0;
        if (ok && jp::calc::serialToDate(serial, m_system, &y, &m, &d)) m_date[row]->setDate(QDate(y, m, d));
    } else {
        m_op[row]->setCurrentIndex(int(op) + (row == 1 ? 1 : 0));
        m_text[row]->setText(text);
    }
}

FilterColumn CustomFilterDialog::result() const
{
    FilterColumn c;
    c.kind = FilterColumn::Kind::Custom;
    c.both = m_and->isChecked();
    for (int row = 0; row < 2; ++row) {
        const int index = m_op[row]->currentIndex() - (row == 1 ? 1 : 0);
        if (index < 0) continue;
        FilterColumn::Condition cond;
        if (m_dates) {
            // the six comparisons of the date boxes: equals, does not equal, after, on or after, before, on or before
            static const FilterColumn::Op ops[] = {FilterColumn::Op::Equal, FilterColumn::Op::NotEqual, FilterColumn::Op::Greater, FilterColumn::Op::GreaterEqual, FilterColumn::Op::Less,
                                                   FilterColumn::Op::LessEqual};
            cond.op = ops[std::clamp(index, 0, 5)];
            double serial = 0;
            const QDate d = m_date[row]->date();
            jp::calc::dateToSerial(d.year(), d.month(), d.day(), m_system, &serial);
            // (a day is the whole day: before a day is before its start, and on or before it is up to its last moment)
            if (cond.op == FilterColumn::Op::LessEqual) {
                cond.op = FilterColumn::Op::Less;
                serial += 1;
            } else if (cond.op == FilterColumn::Op::Greater) {
                cond.op = FilterColumn::Op::GreaterEqual;
                serial += 1;
            } else if (cond.op == FilterColumn::Op::Equal || cond.op == FilterColumn::Op::NotEqual) {
                // (equals a day: from its start up to the start of the next; that is two comparisons, so it is spelled so)
                c.conditions.append({cond.op == FilterColumn::Op::Equal ? FilterColumn::Op::GreaterEqual : FilterColumn::Op::Less, jp::calc::numberToText(serial)});
                c.conditions.append({cond.op == FilterColumn::Op::Equal ? FilterColumn::Op::Less : FilterColumn::Op::GreaterEqual, jp::calc::numberToText(serial + 1)});
                c.both = cond.op == FilterColumn::Op::Equal;
                return c;
            }
            cond.value = jp::calc::numberToText(serial);
        } else {
            const QString text = m_text[row]->text();
            switch (UiOp(index)) {
            case Equals: case NotEquals: case Greater: case GreaterEqual: case Less: case LessEqual: {
                cond.op = FilterColumn::Op(int(index));
                bool ok = false;
                const double n = QLocale().toDouble(text.trimmed(), &ok);
                cond.value = ok ? jp::calc::numberToText(n) : text;
                break;
            }
            case BeginsWith: cond = {FilterColumn::Op::Equal, escaped(text) + QLatin1Char('*')}; break;
            case NotBeginsWith: cond = {FilterColumn::Op::NotEqual, escaped(text) + QLatin1Char('*')}; break;
            case EndsWith: cond = {FilterColumn::Op::Equal, QLatin1Char('*') + escaped(text)}; break;
            case NotEndsWith: cond = {FilterColumn::Op::NotEqual, QLatin1Char('*') + escaped(text)}; break;
            case Contains: cond = {FilterColumn::Op::Equal, QLatin1Char('*') + escaped(text) + QLatin1Char('*')}; break;
            case NotContains: cond = {FilterColumn::Op::NotEqual, QLatin1Char('*') + escaped(text) + QLatin1Char('*')}; break;
            case Nothing: continue;
            }
        }
        c.conditions.append(cond);
    }
    return c;
}

TopFilterDialog::TopFilterDialog(const QString &column, const FilterColumn *current, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("topFilterDialog"));
    setWindowTitle(tr("Top 10 Filter"));
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Show the values of \"%1\" that are").arg(column.left(60))));
    auto *row = new QHBoxLayout;
    m_which = new QComboBox;
    m_which->setObjectName(QStringLiteral("topWhich"));
    m_which->setAccessibleName(tr("Top or bottom"));
    m_which->addItems({tr("Top"), tr("Bottom")});
    m_count = new QDoubleSpinBox;
    m_count->setObjectName(QStringLiteral("topCount"));
    m_count->setAccessibleName(tr("How many"));
    m_count->setDecimals(0);
    m_count->setRange(1, 500000);
    m_count->setValue(10);
    m_unit = new QComboBox;
    m_unit->setObjectName(QStringLiteral("topUnit"));
    m_unit->setAccessibleName(tr("Items or percent"));
    m_unit->addItems({tr("Items"), tr("Percent")});
    row->addWidget(m_which);
    row->addWidget(m_count);
    row->addWidget(m_unit);
    layout->addLayout(row);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    if (current && current->kind == FilterColumn::Kind::Top10) {
        m_which->setCurrentIndex(current->top ? 0 : 1);
        m_count->setValue(current->count);
        m_unit->setCurrentIndex(current->percent ? 1 : 0);
    }
    connect(m_unit, &QComboBox::currentIndexChanged, this, [this](int i) { m_count->setMaximum(i == 1 ? 100 : 500000); });
}

FilterColumn TopFilterDialog::result() const
{
    FilterColumn c;
    c.kind = FilterColumn::Kind::Top10;
    c.top = m_which->currentIndex() == 0;
    c.percent = m_unit->currentIndex() == 1;
    c.count = m_count->value();
    return c;
}

} // namespace jp

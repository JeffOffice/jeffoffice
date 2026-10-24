// What a person does with a block of cells besides typing in it: AutoSum, fill, the clipboard. Each makes the changes for one step of the undo stack.
#include "core/editsession.h"

#include "sheet/cellref.h"

#include <algorithm>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::calc::Value;

Value EditSession::valueAt(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->calc || sheet < 0 || sheet >= m_book->calc->sheetCount()) return Value();
    const Sheet &s = m_book->calc->sheet(sheet);
    const Cell *c = s.cells.find(row, col);
    if (!c) return Value();
    Value v;
    if ((c->hasFormula() || (c->flags & ArrayMember)) && m_values->find(sheet, row, col, &v)) return v;
    switch (c->type) {
    case CellType::Empty: return Value();
    case CellType::Number:
    case CellType::Date: return Value::number(c->number);
    case CellType::SharedString: return Value::text(m_book->book->sharedStrings().at(c->textIndex()));
    case CellType::InlineString:
    case CellType::FormulaString: return Value::text(s.texts.value(c->textIndex()));
    case CellType::Bool: return Value::boolean(c->number != 0);
    case CellType::Error: return Value::error(jp::calc::ErrorKind(std::max(0, std::min(int(jp::calc::ErrorKind::Unknown), int(c->number)))));
    }
    return Value();
}

namespace {

QString rangeText(quint32 row1, quint32 col1, quint32 row2, quint32 col2)
{
    return row1 == row2 && col1 == col2 ? cellName(row1, col1) : cellName(row1, col1) + QLatin1Char(':') + cellName(row2, col2);
}

} // namespace

EditSession::AutoSumResult EditSession::autoSum(int sheet, const CellRange &sel, const QString &function)
{
    AutoSumResult out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.outcome.ok = false;
        out.outcome.problem = why;
        return out;
    }
    auto isNumber = [&](quint32 r, quint32 c) { return valueAt(sheet, r, c).isNumber(); };
    auto isEmptyCell = [&](quint32 r, quint32 c) { return valueAt(sheet, r, c).isEmpty() && !m_book->calc->content(sheet, r, c).isFormula; };
    // the end of the data of a sheet that is selected whole is the end of the cells
    quint32 usedRows = 0, usedCols = 0;
    {
        const Sheet &s = m_book->calc->sheet(sheet);
        usedRows = s.extentRows;
        usedCols = s.extentCols;
    }
    const quint32 row1 = sel.row1, col1 = sel.col1;
    // (a selection of whole columns or rows reaches as far as the cells do)
    const quint32 lastRow = sel.row2 >= jp::calc::kGridRows - 1 ? std::min<quint32>(sel.row2, std::max<quint32>(usedRows, row1 + 1) - 1) : sel.row2;
    const quint32 lastCol = sel.col2 >= jp::calc::kGridCols - 1 ? std::min<quint32>(sel.col2, std::max<quint32>(usedCols, col1 + 1) - 1) : sel.col2;
    const bool oneCell = sel.row1 == sel.row2 && sel.col1 == sel.col2;
    auto formula = [&](quint32 r1, quint32 c1, quint32 r2, quint32 c2) { return function + QLatin1Char('(') + rangeText(r1, c1, r2, c2) + QLatin1Char(')'); };
    if (oneCell) {
        const quint32 row = sel.row1, col = sel.col1;
        QString text;
        int back = 1;
        if (row > 0 && isNumber(row - 1, col)) {
            quint32 top = row - 1;
            while (top > 0 && isNumber(top - 1, col)) --top;
            text = formula(top, col, row - 1, col);
            back = 0;
        } else if (col > 0 && isNumber(row, col - 1)) {
            quint32 left = col - 1;
            while (left > 0 && isNumber(row, left - 1)) --left;
            text = formula(row, left, row, col - 1);
            back = 0;
        } else {
            text = function + QStringLiteral("()");
        }
        QString problem;
        if (!canEditCell(sheet, row, col, &problem)) {
            out.outcome.ok = false;
            out.outcome.problem = problem;
            return out;
        }
        out.kind = AutoSumResult::Kind::Edit;
        out.row = row;
        out.col = col;
        out.text = QLatin1Char('=') + formulaToLocale(text, m_locale);
        out.cursorBack = back ? back : 0;
        return out;
    }
    // a block: the totals go under it, or right of it when it is one row
    Step step;
    auto put = [&](quint32 r, quint32 c, const QString &text) {
        QString problem;
        if (!canEditCell(sheet, r, c, &problem)) {
            out.outcome.ok = false;
            out.outcome.problem = problem;
            return false;
        }
        CellChange ch;
        bool none = false;
        if (!typedChange(sheet, r, c, QLatin1Char('=') + formulaToLocale(text, m_locale), &ch, &none, &out.outcome)) return false;
        if (!none) step.cells.push_back(std::move(ch));
        return true;
    };
    if (sel.row2 > sel.row1) {
        // the last row of the selection is where the totals go when it is empty; else under it
        bool emptyLast = true;
        for (quint32 c = col1; c <= lastCol && emptyLast; ++c) emptyLast = isEmptyCell(lastRow, c);
        const quint32 totals = emptyLast ? lastRow : lastRow + 1;
        if (totals > row1) {
            for (quint32 c = col1; c <= lastCol; ++c) {
                bool any = false;
                for (quint32 r = row1; r < totals && !any; ++r) any = isNumber(r, c);
                if (any && !put(totals, c, formula(row1, c, totals - 1, c))) return out;
            }
        }
    } else {
        bool emptyLast = isEmptyCell(row1, lastCol);
        const quint32 totals = emptyLast ? lastCol : lastCol + 1;
        if (totals > col1) {
            bool any = false;
            for (quint32 c = col1; c < totals && !any; ++c) any = isNumber(row1, c);
            if (any && !put(row1, totals, formula(row1, col1, row1, totals - 1))) return out;
        }
    }
    if (step.cells.empty()) return out;
    if (qint64(step.cells.size()) > kMaxCellsPerEdit) {
        out.outcome.ok = false;
        out.outcome.problem = tr("That is more cells than JeffCrunch changes at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    out.kind = AutoSumResult::Kind::Done;
    pushStep(std::move(step), tr("AutoSum"));
    return out;
}

} // namespace jp::crunch

// ---- fill

#include "calc/dates.h"
#include "calc/numfmt.h"
#include "sheet/refshift.h"

#include <QDate>
#include <QRegularExpression>

#include <cmath>
#include <vector>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

// The names a series of text can run through: the weekdays and the months, long and short, in the person's language and in English. `kind` is what it is, `index` where the text is in
// it (0 based), and `size` how many there are.
struct NameSeries {
    QStringList names;     // as the language has them, in turn
    int index = -1;
};

QStringList weekdayNames(const QLocale &l, QLocale::FormatType t)
{
    QStringList out;
    for (int d = 1; d <= 7; ++d) out << l.dayName(d, t);
    return out;
}
QStringList monthNames(const QLocale &l, QLocale::FormatType t)
{
    QStringList out;
    for (int m = 1; m <= 12; ++m) out << l.monthName(m, t);
    return out;
}

// The name list that holds the text (without regard to capitals), with its place; empty if none does.
NameSeries findNames(const QString &text, const QLocale &locale)
{
    static const QLocale english(QLocale::English, QLocale::UnitedStates);
    const QString t = text.trimmed();
    for (const QLocale *l : {&locale, &english})
        for (const QLocale::FormatType type : {QLocale::LongFormat, QLocale::ShortFormat}) {
            for (const QStringList &names : {weekdayNames(*l, type), monthNames(*l, type)}) {
                for (int i = 0; i < names.size(); ++i)
                    if (!names[i].isEmpty() && names[i].compare(t, Qt::CaseInsensitive) == 0) {
                        NameSeries s;
                        s.names = names;
                        s.index = i;
                        return s;
                    }
            }
        }
    return {};
}

// The text in the same capitals as the model: all upper case, all lower, or as it is (a capital first).
QString inStyleOf(const QString &model, const QString &text)
{
    if (model.size() > 1 && model == model.toUpper()) return text.toUpper();
    if (model == model.toLower()) return text.toLower();
    return text;
}

double cleaned(double v) { return jp::calc::roundTo15Digits(v); }

bool sameMeaning(const Calculation::CellContent &a, const Calculation::CellContent &b)
{
    return a.exists == b.exists && a.isFormula == b.isFormula && a.formula == b.formula && a.value == b.value;
}

// A date moved by months (or years), keeping the day as far as the month has it.
bool addMonths(double serial, jp::calc::DateSystem system, qint64 months, double *out)
{
    int y = 0, m = 0, d = 0;
    if (!jp::calc::serialToDate(serial, system, &y, &m, &d)) return false;
    const double fraction = serial - std::floor(serial);
    qint64 total = qint64(y) * 12 + (m - 1) + months;
    const int ny = int(total >= 0 ? total / 12 : -((11 - total) / 12));
    const int nm = int(total - qint64(ny) * 12) + 1;
    const int last = QDate(ny, nm, 1).isValid() ? QDate(ny, nm, 1).daysInMonth() : 28;
    double result = 0;
    if (!jp::calc::dateToSerial(ny, nm, std::min(d, last), system, &result)) return false;
    *out = result + fraction;
    return true;
}

// The date `steps` weekdays on (a Saturday and a Sunday are not counted).
bool addWeekdays(double serial, jp::calc::DateSystem system, qint64 steps, double *out)
{
    int y = 0, m = 0, d = 0;
    if (!jp::calc::serialToDate(serial, system, &y, &m, &d)) return false;
    QDate date(y, m, d);
    if (!date.isValid()) return false;
    const int direction = steps < 0 ? -1 : 1;
    for (qint64 left = std::llabs(steps); left > 0;) {
        date = date.addDays(direction);
        if (date.dayOfWeek() < 6) --left;
    }
    double result = 0;
    if (!jp::calc::dateToSerial(date.year(), date.month(), date.day(), system, &result)) return false;
    *out = result + (serial - std::floor(serial));
    return true;
}

} // namespace

EditSession::Outcome EditSession::fill(int sheet, const CellRange &source, FillDirection dir, quint32 count) { return fill(sheet, source, dir, count, FillOptions()); }

EditSession::Outcome EditSession::fill(int sheet, const CellRange &source, FillDirection dir, quint32 count, const FillOptions &opt)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (count == 0 || !source.valid) return out;
    const bool vertical = dir == FillDirection::Down || dir == FillDirection::Up;
    const bool forward = dir == FillDirection::Down || dir == FillDirection::Right;
    const quint32 lines = vertical ? source.col2 - source.col1 + 1 : source.row2 - source.row1 + 1;
    const quint32 n = vertical ? source.row2 - source.row1 + 1 : source.col2 - source.col1 + 1;
    // how far the grid lets it go
    const quint32 sourceEdge = vertical ? (forward ? source.row2 : source.row1) : (forward ? source.col2 : source.col1);
    const quint64 room = forward ? quint64((vertical ? jp::calc::kGridRows : jp::calc::kGridCols) - 1 - sourceEdge) : quint64(sourceEdge);
    count = quint32(std::min<quint64>(count, room));
    if (count == 0) return out;
    if (qint64(lines) * count > kMaxCellsPerEdit) {
        out.ok = false;
        out.problem = tr("That is more cells than JeffCrunch changes at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    const Calculation *calc = m_book->calc.get();
    const jp::calc::DateSystem system = m_book->book->info().dateSystem;
    const bool hasShared = !m_book->book->sharedStringsPart().isEmpty();
    Step step;
    for (quint32 line = 0; line < lines; ++line) {
        // the cells of the strip, in the order the fill goes on from
        struct Src {
            quint32 row, col;
            Calculation::CellContent content;
        };
        std::vector<Src> seq(n);
        for (quint32 i = 0; i < n; ++i) {
            const quint32 k = forward ? i : n - 1 - i;
            Src &sc = seq[i];
            sc.row = vertical ? source.row1 + k : source.row1 + line;
            sc.col = vertical ? source.col1 + line : source.col1 + k;
            sc.content = calc->content(sheet, sc.row, sc.col);
        }
        // what kind of series the strip is
        enum class Mode { Cycle, Numbers, Growth, Dates, Names, TextNumber } mode = Mode::Cycle;
        std::vector<double> numbers;
        bool allNumbers = true, anyFormula = false, allText = true;
        for (const Src &sc : seq) {
            if (sc.content.isFormula) anyFormula = true;
            if (!sc.content.exists || sc.content.isFormula || !sc.content.value.isNumber()) allNumbers = false;
            else numbers.push_back(sc.content.value.number());
            if (!sc.content.exists || sc.content.isFormula || !sc.content.value.isText()) allText = false;
        }
        double base = 0, stepValue = 0;
        DateUnit unit = opt.unit;
        NameSeries names;
        int nameStep = 1;
        QString textPrefix;
        int textWidth = 0;
        const FillKind kind = opt.kind;
        if (kind != FillKind::Copy && !anyFormula) {
            if (allNumbers) {
                bool dateStyle = false;
                const QString code = m_book->book->styles().formatCode(seq.back().content.style, m_locale);
                dateStyle = jp::calc::NumberFormat(code).kind() == jp::calc::FormatKind::Date;
                if (kind == FillKind::Growth) {
                    mode = Mode::Growth;
                    base = numbers.back();
                    stepValue = opt.hasStep ? opt.step : (numbers.size() >= 2 && numbers[numbers.size() - 2] != 0 ? numbers.back() / numbers[numbers.size() - 2] : 2);
                } else if (kind == FillKind::Date || (kind == FillKind::Auto && dateStyle)) {
                    mode = Mode::Dates;
                    base = numbers.back();
                    stepValue = opt.hasStep ? opt.step : (forward ? 1 : -1);
                    if (!opt.hasStep && n >= 2) {
                        // the months or years that the dates are apart, if they are: else the days
                        const double d0 = numbers[numbers.size() - 2], d1 = numbers.back();
                        int y0, m0, dd0, y1, m1, dd1;
                        if (jp::calc::serialToDate(d0, system, &y0, &m0, &dd0) && jp::calc::serialToDate(d1, system, &y1, &m1, &dd1) && dd0 == dd1 && (y1 != y0 || m1 != m0)) {
                            const qint64 months = qint64(y1 - y0) * 12 + (m1 - m0);
                            if (kind == FillKind::Auto) {
                                if (months % 12 == 0) {
                                    unit = DateUnit::Year;
                                    stepValue = double(months / 12);
                                } else {
                                    unit = DateUnit::Month;
                                    stepValue = double(months);
                                }
                            }
                        } else {
                            stepValue = d1 - d0;
                        }
                    }
                } else if (kind == FillKind::Linear || (kind == FillKind::Auto && n >= 2)) {
                    mode = Mode::Numbers;
                    base = numbers.back();
                    if (opt.hasStep) {
                        stepValue = opt.step;
                    } else if (numbers.size() >= 2) {
                        // a trend through the numbers (the step itself when they go by steps of one size)
                        bool equal = true;
                        const double first = numbers[1] - numbers[0];
                        for (size_t i = 2; i < numbers.size(); ++i)
                            if (std::fabs((numbers[i] - numbers[i - 1]) - first) > 1e-9 * std::max(1.0, std::fabs(first))) equal = false;
                        if (equal) {
                            stepValue = first;
                        } else {
                            const double nn = double(numbers.size());
                            double sx = 0, sy = 0, sxy = 0, sxx = 0;
                            for (size_t i = 0; i < numbers.size(); ++i) {
                                sx += double(i);
                                sy += numbers[i];
                                sxy += double(i) * numbers[i];
                                sxx += double(i) * double(i);
                            }
                            const double slope = (nn * sxy - sx * sy) / (nn * sxx - sx * sx);
                            const double intercept = (sy - slope * sx) / nn;
                            stepValue = slope;
                            base = intercept + slope * (nn - 1);
                        }
                    } else {
                        stepValue = forward ? 1 : -1;
                    }
                }
            } else if (allText && kind != FillKind::Linear && kind != FillKind::Growth && kind != FillKind::Date) {
                // the names of weekdays and months, or a text that ends in a number
                names = findNames(seq.back().content.value.text(), m_locale);
                if (names.index >= 0) {
                    bool all = true;
                    std::vector<int> at;
                    for (const Src &sc : seq) {
                        const NameSeries other = findNames(sc.content.value.text(), m_locale);
                        if (other.index < 0 || other.names != names.names) all = false;
                        else at.push_back(other.index);
                    }
                    if (all) {
                        mode = Mode::Names;
                        if (at.size() < 2 && !forward) nameStep = int(names.names.size()) - 1;
                        if (at.size() >= 2) {
                            nameStep = at.back() - at[at.size() - 2];
                            for (size_t i = 2; i < at.size(); ++i)
                                if (((at[i] - at[i - 1]) - nameStep) % names.names.size() != 0) all = false;
                            nameStep = ((nameStep % names.names.size()) + names.names.size()) % names.names.size();
                            if (!all || nameStep == 0) mode = Mode::Cycle;
                        }
                    }
                }
                if (mode == Mode::Cycle) {
                    static const QRegularExpression trailing(QStringLiteral("^(.*?)(\\d+)$"));
                    bool all = true;
                    std::vector<double> digits;
                    for (const Src &sc : seq) {
                        const QRegularExpressionMatch m = trailing.match(sc.content.value.text());
                        if (!m.hasMatch() || m.captured(2).size() > 15) {
                            all = false;
                            break;
                        }
                        if (digits.empty()) textPrefix = m.captured(1);
                        else if (m.captured(1) != textPrefix) all = false;
                        digits.push_back(m.captured(2).toDouble());
                        textWidth = m.captured(2).size() > 1 && m.captured(2).startsWith(QLatin1Char('0')) ? int(m.captured(2).size()) : 0;
                    }
                    if (all && !digits.empty()) {
                        mode = Mode::TextNumber;
                        base = digits.back();
                        stepValue = digits.size() >= 2 ? digits.back() - digits[digits.size() - 2] : (forward ? 1 : -1);
                    }
                }
            }
        }
        const bool single = n == 1;
        if (mode == Mode::Cycle && kind == FillKind::Linear && single) mode = Mode::Cycle;
        for (quint32 k = 1; k <= count; ++k) {
            const quint32 row = vertical ? (forward ? source.row2 + k : source.row1 - k) : source.row1 + line;
            const quint32 col = vertical ? source.col1 + line : (forward ? source.col2 + k : source.col1 - k);
            QString problem;
            if (!canEditCell(sheet, row, col, &problem)) {
                out.ok = false;
                out.problem = problem;
                return out;
            }
            if (m_book->book->tableHeaderAt(sheet, row, col, nullptr) >= 0) {
                out.ok = false;
                out.problem = tr("The first row of a table holds the names of its columns, so it can't be filled over. Fill the cells below it, or type each name.");
                return out;
            }
            const Src &from = seq[(k - 1) % n];
            CellChange ch;
            ch.sheet = sheet;
            ch.row = row;
            ch.col = quint16(col);
            ch.before = calc->content(sheet, row, col);
            ch.after = from.content;
            ch.after.hasStored = false;
            ch.after.stored = Value();
            if (mode == Mode::Cycle || from.content.isFormula) {
                if (ch.after.isFormula) ch.after.formula = offsetFormula(from.content.formula, qint32(qint64(row) - qint64(from.row)), qint32(qint64(col) - qint64(from.col)));
            } else {
                ch.after.sharedString = -1;
                const double kk = double(k);
                switch (mode) {
                case Mode::Numbers: {
                    const double v = cleaned(base + kk * stepValue);
                    if (opt.hasStop && ((stepValue >= 0 && v > opt.stop) || (stepValue < 0 && v < opt.stop))) goto stripDone;
                    ch.after.value = Value::number(v);
                    break;
                }
                case Mode::Growth: {
                    const double v = cleaned(base * std::pow(stepValue, kk));
                    if (!std::isfinite(v) || (opt.hasStop && ((stepValue >= 1 && v > opt.stop) || (stepValue < 1 && v < opt.stop)))) goto stripDone;
                    ch.after.value = Value::number(v);
                    break;
                }
                case Mode::Dates: {
                    double v = base;
                    bool ok = false;
                    const qint64 steps = qint64(std::llround(stepValue * kk));
                    switch (unit) {
                    case DateUnit::Day: v = base + stepValue * kk; ok = true; break;
                    case DateUnit::Weekday: ok = addWeekdays(base, system, steps, &v); break;
                    case DateUnit::Month: ok = addMonths(base, system, steps, &v); break;
                    case DateUnit::Year: ok = addMonths(base, system, steps * 12, &v); break;
                    }
                    if (!ok) goto stripDone;
                    if (opt.hasStop && ((stepValue >= 0 && v > opt.stop) || (stepValue < 0 && v < opt.stop))) goto stripDone;
                    ch.after.value = Value::number(v);
                    break;
                }
                case Mode::Names: {
                    const int size = int(names.names.size());
                    const int at = ((names.index + qint64(nameStep) * (forward ? int(k) : int(k))) % size + size) % size;
                    const QString shown = inStyleOf(seq.back().content.value.text(), names.names[at]);
                    ch.after.value = Value::text(shown);
                    ch.textToShare = hasShared;
                    break;
                }
                case Mode::TextNumber: {
                    const double v = base + kk * stepValue;
                    if (v < 0) goto stripDone;
                    QString digits = QString::number(qint64(std::llround(v)));
                    if (textWidth > digits.size()) digits.prepend(QString(textWidth - int(digits.size()), QLatin1Char('0')));
                    ch.after.value = Value::text(textPrefix + digits);
                    ch.textToShare = hasShared;
                    break;
                }
                case Mode::Cycle: break;
                }
                // (a text taken from the shared strings keeps its place there)
            }
            if (mode == Mode::Cycle && ch.after.value.isText() && ch.after.sharedString < 0 && hasShared) ch.textToShare = true;
            if (!(sameMeaning(ch.before, ch.after) && ch.before.style == ch.after.style)) step.cells.push_back(std::move(ch));
        }
    stripDone:;
    }
    if (step.empty()) return out;
    pushStep(std::move(step), tr("Fill"));
    return out;
}

} // namespace jp::crunch

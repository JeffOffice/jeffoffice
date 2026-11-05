#include "core/consolidate.h"

#include "calc/value.h"
#include "core/formulaspot.h"
#include "sheet/cellref.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QUndoStack>

#include <algorithm>
#include <cmath>
#include <map>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

EditSession::Outcome refuse(const QString &why)
{
    EditSession::Outcome out;
    out.ok = false;
    out.problem = why;
    return out;
}

QString sheetPrefix(const EditSession &s, int sheet)
{
    const QString name = s.workbook()->info().sheets[size_t(sheet)].name;
    static const QRegularExpression plain(QStringLiteral("^[A-Za-z_][A-Za-z0-9_]*$"));
    static const QRegularExpression looksLikeCell(QStringLiteral("^[A-Za-z]{1,3}[0-9]+$"));
    if (plain.match(name).hasMatch() && !looksLikeCell.match(name).hasMatch()) return name + QLatin1Char('!');
    return QLatin1Char('\'') + QString(name).replace(QLatin1Char('\''), QStringLiteral("''")) + QStringLiteral("'!");
}

// The function of the numbers.
double combine(int code, const std::vector<double> &n)
{
    if (code == 3) return double(n.size());
    if (code == 2) return double(n.size());
    if (n.empty()) return std::nan("");
    double sum = 0;
    for (double v : n) sum += v;
    const double mean = sum / double(n.size());
    auto squares = [&] {
        double q = 0;
        for (double v : n) q += (v - mean) * (v - mean);
        return q;
    };
    switch (code) {
    case 9: return sum;
    case 1: return mean;
    case 4: return *std::max_element(n.begin(), n.end());
    case 5: return *std::min_element(n.begin(), n.end());
    case 6: {
        double p = 1;
        for (double v : n) p *= v;
        return p;
    }
    case 7: return n.size() < 2 ? std::nan("") : std::sqrt(squares() / double(n.size() - 1));
    case 8: return std::sqrt(squares() / double(n.size()));
    case 10: return n.size() < 2 ? std::nan("") : squares() / double(n.size() - 1);
    case 11: return squares() / double(n.size());
    default: return std::nan("");
    }
}

QString functionName(int code)
{
    switch (code) {
    case 9: return QStringLiteral("SUM");
    case 1: return QStringLiteral("AVERAGE");
    case 3: return QStringLiteral("COUNTA");
    case 2: return QStringLiteral("COUNT");
    case 4: return QStringLiteral("MAX");
    case 5: return QStringLiteral("MIN");
    case 6: return QStringLiteral("PRODUCT");
    case 7: return QStringLiteral("STDEV");
    case 8: return QStringLiteral("STDEVP");
    case 10: return QStringLiteral("VAR");
    case 11: return QStringLiteral("VARP");
    }
    return QString();
}

QString labelOf(const EditSession &s, int sheet, quint32 row, quint32 col)
{
    const Value v = s.valueAt(sheet, row, col);
    if (v.isText()) return v.text();
    if (v.isNumber()) return jp::calc::numberToText(v.number());
    return QString();
}

} // namespace

EditSession::Outcome consolidate(EditSession &session, const ConsolidateSpec &spec, CellRange *written, const std::function<void()> &beforeEnd)
{
    QString why;
    if (!session.canEdit(&why) || !session.canEditSheet(spec.sheet, &why)) return refuse(why);
    if (spec.sources.empty()) return refuse(QCoreApplication::translate("jp::crunch::Consolidate", "Add at least one range to combine."));
    if (spec.sources.size() > kMaxConsolidateSources) return refuse(QCoreApplication::translate("jp::crunch::Consolidate", "There are too many ranges to combine."));
    if (functionName(spec.function).isEmpty()) return refuse(QCoreApplication::translate("jp::crunch::Consolidate", "Choose a function to use."));
    const int sheets = int(session.workbook()->info().sheets.size());
    for (const ConsolidateSource &src : spec.sources)
        if (!src.range.valid || src.sheet < 0 || src.sheet >= sheets || session.workbook()->info().sheets[size_t(src.sheet)].deleted) return refuse(QCoreApplication::translate("jp::crunch::Consolidate", "One of the ranges is not on a sheet of this workbook."));
    const bool top = spec.topLabels, left = spec.leftLabels;
    // the rows and columns of the result: by position the most any range has; by label every label once, in the order they are first met
    QStringList rowLabels, colLabels;
    quint32 rows = 0, cols = 0;
    for (const ConsolidateSource &src : spec.sources) {
        const quint32 r0 = src.range.row1 + (top ? 1 : 0), c0 = src.range.col1 + (left ? 1 : 0);
        if (r0 > src.range.row2 || c0 > src.range.col2) continue;
        rows = std::max(rows, src.range.row2 - r0 + 1);
        cols = std::max(cols, src.range.col2 - c0 + 1);
        if (left)
            for (quint32 r = r0; r <= src.range.row2; ++r) {
                const QString l = labelOf(session, src.sheet, r, src.range.col1);
                if (!l.isEmpty() && !rowLabels.contains(l, Qt::CaseInsensitive)) rowLabels << l;
            }
        if (top)
            for (quint32 c = c0; c <= src.range.col2; ++c) {
                const QString l = labelOf(session, src.sheet, src.range.row1, c);
                if (!l.isEmpty() && !colLabels.contains(l, Qt::CaseInsensitive)) colLabels << l;
            }
    }
    const quint32 outRows = left ? quint32(rowLabels.size()) : rows, outCols = top ? quint32(colLabels.size()) : cols;
    if (outRows == 0 || outCols == 0) return refuse(QCoreApplication::translate("jp::crunch::Consolidate", "The ranges hold nothing to combine."));
    const quint32 headRows = top ? 1 : 0, headCols = left ? 1 : 0;
    if (qint64(outRows + headRows) * (outCols + headCols) > kMaxCellsPerEdit) return refuse(QCoreApplication::translate("jp::crunch::Consolidate", "The result would fill too many cells at once."));
    if (qint64(spec.row) + outRows + headRows > jp::calc::kGridRows || qint64(spec.col) + outCols + headCols > jp::calc::kGridCols) return refuse(QCoreApplication::translate("jp::crunch::Consolidate", "The result would go past the edge of the sheet."));
    const QChar sep = argumentSeparator(session.locale());
    QUndoStack *stack = session.undoStack();
    stack->beginMacro(QCoreApplication::translate("jp::crunch::Consolidate", "Consolidate"));
    EditSession::Outcome out;
    auto put = [&](quint32 r, quint32 c, const QString &text) {
        if (out.ok) out = session.type(spec.sheet, spec.row + r, spec.col + c, text, false);
    };
    for (quint32 j = 0; j < outCols; ++j)
        if (top) put(0, headCols + j, colLabels[int(j)]);
    for (quint32 i = 0; i < outRows; ++i) {
        if (left) put(headRows + i, 0, rowLabels[int(i)]);
        for (quint32 j = 0; j < outCols && out.ok; ++j) {
            std::vector<double> numbers;
            QStringList refs;
            for (const ConsolidateSource &src : spec.sources) {
                const quint32 r0 = src.range.row1 + (top ? 1 : 0), c0 = src.range.col1 + (left ? 1 : 0);
                if (r0 > src.range.row2 || c0 > src.range.col2) continue;
                qint64 r = -1, c = -1;
                if (left) {
                    for (quint32 k = r0; k <= src.range.row2 && r < 0; ++k)
                        if (labelOf(session, src.sheet, k, src.range.col1).compare(rowLabels[int(i)], Qt::CaseInsensitive) == 0) r = k;
                } else {
                    r = qint64(r0) + i;
                    if (r > src.range.row2) r = -1;
                }
                if (top) {
                    for (quint32 k = c0; k <= src.range.col2 && c < 0; ++k)
                        if (labelOf(session, src.sheet, src.range.row1, k).compare(colLabels[int(j)], Qt::CaseInsensitive) == 0) c = k;
                } else {
                    c = qint64(c0) + j;
                    if (c > src.range.col2) c = -1;
                }
                if (r < 0 || c < 0) continue;
                const Value v = session.valueAt(src.sheet, quint32(r), quint32(c));
                if (v.isNumber() || (spec.function == 3 && !v.isEmpty())) {
                    if (v.isNumber()) numbers.push_back(v.number());
                    else numbers.push_back(0);
                    refs << (src.sheet == spec.sheet ? QString() : sheetPrefix(session, src.sheet)) + cellName(quint32(r), quint32(c));
                }
            }
            if (refs.isEmpty()) continue;
            if (spec.links) put(headRows + i, headCols + j, QLatin1Char('=') + functionName(spec.function) + QLatin1Char('(') + refs.join(sep) + QLatin1Char(')'));
            else {
                const double v = combine(spec.function, numbers);
                if (std::isfinite(v)) put(headRows + i, headCols + j, QString::number(v, 'g', 15));
            }
        }
    }
    if (beforeEnd) beforeEnd();
    stack->endMacro();
    if (!out.ok) {
        stack->undo();
        return out;
    }
    if (written) {
        CellRange r;
        r.valid = true;
        r.row1 = spec.row;
        r.col1 = spec.col;
        r.row2 = spec.row + outRows + headRows - 1;
        r.col2 = spec.col + outCols + headCols - 1;
        *written = r;
    }
    return EditSession::Outcome();
}

} // namespace jp::crunch

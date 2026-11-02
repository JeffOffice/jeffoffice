// A formula worked out on the spot for the Function Arguments dialog: read-only, bounded, and never written to the sheet.
#include "core/editsession.h"

#include "calc/eval.h"

#include <QCoreApplication>

#include <algorithm>

namespace jp::crunch {

namespace {

using jp::calc::ErrorKind;
using jp::calc::Value;

constexpr qint64 kPreviewWork = 1000000;   // (steps: a few hundred thousand cells; past it the dialog says so, and typing goes on)
constexpr int kShownRows = 4, kShownCols = 6;

QString errorWords(ErrorKind e)
{
    switch (e) {
    case ErrorKind::Null: return QCoreApplication::translate("jp::crunch::EditSession", "The two blocks of cells have no cell in common.");
    case ErrorKind::Div0: return QCoreApplication::translate("jp::crunch::EditSession", "Something is divided by zero or by an empty cell.");
    case ErrorKind::Value: return QCoreApplication::translate("jp::crunch::EditSession", "A value is of the wrong kind, such as text where a number is needed.");
    case ErrorKind::Ref: return QCoreApplication::translate("jp::crunch::EditSession", "A reference points to a cell that is not there.");
    case ErrorKind::Name: return QCoreApplication::translate("jp::crunch::EditSession", "A name or function is not known.");
    case ErrorKind::Num: return QCoreApplication::translate("jp::crunch::EditSession", "A number is not allowed here, or is too large or too small.");
    case ErrorKind::NA: return QCoreApplication::translate("jp::crunch::EditSession", "A value is not available: what was looked for was not found.");
    case ErrorKind::Spill: return QCoreApplication::translate("jp::crunch::EditSession", "The result does not fit: cells where it would go are not empty.");
    default: break;
    }
    return QCoreApplication::translate("jp::crunch::EditSession", "The formula gives an error.");
}

QString one(const Value &v, const QLocale &locale)
{
    if (v.isNumber()) return locale.toString(v.number(), 'g', 10);
    if (v.isText()) return QLatin1Char('"') + v.text().left(80).replace(QLatin1Char('"'), QStringLiteral("\"\"")) + (v.text().size() > 80 ? QStringLiteral("…") : QString()) + QLatin1Char('"');
    if (v.isBool()) return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    if (v.isError()) return jp::calc::errorKindText(v.errorKind());
    return QString();
}

QString shownValue(const Value &v, const QLocale &locale)
{
    if (!v.isArray()) return one(v, locale);
    QString out = QStringLiteral("{");
    const int rows = std::min(v.rows(), kShownRows), cols = std::min(v.cols(), kShownCols);
    for (int r = 0; r < rows; ++r) {
        if (r > 0) out += QLatin1Char(';');
        for (int c = 0; c < cols; ++c) {
            if (c > 0) out += QLatin1Char(',');
            out += one(v.at(r, c), locale);
        }
        if (cols < v.cols()) out += QStringLiteral(",…");
    }
    if (rows < v.rows()) out += QStringLiteral(";…");
    return out + QLatin1Char('}');
}

} // namespace

EditSession::Preview EditSession::previewFormula(const QString &typed, int sheet, quint32 row, quint32 col) const
{
    Preview out;
    if (!m_book || !m_book->calc || typed.trimmed().isEmpty()) return out;
    const QString text = jp::office::sheet::formulaFromLocale(typed, m_locale);
    jp::calc::ParseOptions po;
    po.arrayFormula = true;   // (a block of cells is shown whole, as {1,2,3})
    po.row = row;
    po.col = col;
    const jp::calc::ParseResult parsed = jp::calc::parseFormula(text, po);
    if (!parsed.ok()) {
        out.problem = QCoreApplication::translate("jp::crunch::EditSession", "This is not complete or not a formula JeffCrunch can read yet.");
        return out;
    }
    jp::calc::Evaluator ev;
    ev.setWorkLimit(kPreviewWork);
    jp::calc::EvalContext ctx;
    ctx.source = m_book->calc.get();
    ctx.sheet = std::max(0, sheet);
    ctx.row = row;
    ctx.col = col;
    ctx.dateSystem = m_book->book->info().dateSystem;
    const Value v = ev.evaluate(*parsed.program, ctx);
    if (ev.status() != jp::calc::EvalStatus::Ok) {
        out.tooMuch = true;
        out.problem = QCoreApplication::translate("jp::crunch::EditSession", "This takes too long to work out here. Press OK and the sheet will work it out.");
        return out;
    }
    out.ok = true;
    out.shown = shownValue(v, m_locale);
    if (v.isError()) {
        out.error = true;
        out.problem = errorWords(v.errorKind());
    } else if (v.isArray() && v.rows() > 0 && v.at(0, 0).isError() && v.rows() * v.cols() == 1) {
        out.error = true;
        out.problem = errorWords(v.at(0, 0).errorKind());
    }
    return out;
}

} // namespace jp::crunch

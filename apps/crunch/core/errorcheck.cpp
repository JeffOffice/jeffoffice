#include "core/errorcheck.h"

#include "calc/value.h"
#include "sheet/calculation.h"

#include <QCoreApplication>
#include <QLocale>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::ErrorKind;
using jp::calc::Value;

namespace {

QString errorSentence(ErrorKind e)
{
    const char *c = "jp::crunch::ErrorCheck";
    switch (e) {
    case ErrorKind::Null: return QCoreApplication::translate(c, "The formula gives an error: the two blocks of cells it names have no cell in common.");
    case ErrorKind::Div0: return QCoreApplication::translate(c, "The formula gives an error: it divides by zero or by an empty cell.");
    case ErrorKind::Value: return QCoreApplication::translate(c, "The formula gives an error: a value is of the wrong kind, such as text where a number is needed.");
    case ErrorKind::Ref: return QCoreApplication::translate(c, "The formula gives an error: it points to a cell that is not there.");
    case ErrorKind::Name: return QCoreApplication::translate(c, "The formula gives an error: it uses a name or function that is not known.");
    case ErrorKind::Num: return QCoreApplication::translate(c, "The formula gives an error: a number in it is not allowed, or is too large or too small.");
    case ErrorKind::NA: return QCoreApplication::translate(c, "The formula gives an error: a value it looks for is not available.");
    case ErrorKind::Spill: return QCoreApplication::translate(c, "The formula gives an error: its results do not fit, because cells where they would go are not empty.");
    default: return QCoreApplication::translate(c, "The formula gives an error.");
    }
}

bool looksLikeNumber(const QString &text)
{
    const QString t = text.trimmed();
    if (t.isEmpty()) return false;
    bool ok = false;
    QLocale::c().toDouble(t, &ok);
    if (!ok) QLocale().toDouble(t, &ok);
    return ok;
}

} // namespace

std::vector<FormulaProblem> findFormulaProblems(const EditSession &session, int sheet, bool *truncated)
{
    std::vector<FormulaProblem> out;
    if (truncated) *truncated = false;
    const Calculation *calc = session.calculation();
    if (!calc || sheet < 0 || sheet >= calc->sheetCount()) return out;
    bool full = false;
    calc->sheet(sheet).cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
        if (full || (c.flags & ArrayMember)) return;
        FormulaProblem p;
        p.sheet = sheet;
        p.row = row;
        p.col = col;
        if (c.hasFormula()) {
            const Value v = session.valueAt(sheet, row, col);
            if (!v.isError()) return;
            p.kind = FormulaProblem::Kind::ErrorValue;
            p.explanation = errorSentence(v.errorKind());
        } else if (c.type == CellType::SharedString || c.type == CellType::InlineString) {
            const Value v = session.valueAt(sheet, row, col);
            if (!v.isText() || !looksLikeNumber(v.text())) return;
            p.kind = FormulaProblem::Kind::NumberAsText;
            p.explanation = QCoreApplication::translate("jp::crunch::ErrorCheck", "This number is kept as text, so formulas may skip it. Retype it, or change its format, to make it a number.");
        } else {
            return;
        }
        if (out.size() >= kMaxFormulaProblems) {
            full = true;
            if (truncated) *truncated = true;
            return;
        }
        out.push_back(p);
    });
    std::sort(out.begin(), out.end(), [](const FormulaProblem &a, const FormulaProblem &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
    return out;
}

} // namespace jp::crunch

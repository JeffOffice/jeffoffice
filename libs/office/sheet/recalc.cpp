#include "sheet/recalc.h"

#include "sheet/calculation.h"

#include "calc/budget.h"
#include "calc/formula.h"
#include "calc/functions.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace jp::office::sheet {

using namespace jp::calc;

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// the cells of a workbook

WorkbookSource::WorkbookSource(const Workbook &book, qint64 maxBytes) : m_book(book), m_maxBytes(maxBytes)
{
    const WorkbookInfo &info = book.info();
    m_sheets.resize(size_t(info.sheets.size()));
    m_rows.assign(m_sheets.size(), 0);
    m_cols.assign(m_sheets.size(), 0);
    for (int i = 0; i < info.sheets.size(); ++i) {
        const QString key = info.sheets[i].name.toCaseFolded();
        if (!m_byName.contains(key)) m_byName.insert(key, i);
    }
    for (const DefinedName &n : info.definedNames) {
        const QString key = QString::number(n.localSheet) + QLatin1Char('|') + n.name.toUpper();
        if (!m_names.contains(key)) m_names.insert(key, n.ref);
    }
}

bool WorkbookSource::loadAll()
{
    bool whole = true, stopped = false;
    qint64 held = 0;
    for (int i = 0; i < m_book.info().sheets.size(); ++i) {
        if (m_book.info().sheets[i].kind == SheetKind::Chartsheet) {
            m_book.readSheet(i, &m_sheets[size_t(i)]);   // (no cells: what it says of printing is read)
            continue;
        }
        Sheet &s = m_sheets[size_t(i)];
        if (stopped || held > m_maxBytes || (m_progress && !m_progress(i, 0))) {
            // the sheets read so far take all that a workbook may, or the load was stopped: this one is left out
            whole = false;
            stopped = stopped || (held <= m_maxBytes);
            m_notWhole << m_book.info().sheets[i].name;
            continue;
        }
        std::function<bool(qint64)> progress;
        if (m_progress)
            progress = [&](qint64 cells) {
                if (m_progress(i, cells)) return true;
                stopped = true;
                return false;
            };
        if (!m_book.readSheet(i, &s, progress)) {
            whole = false;
            m_notWhole << m_book.info().sheets[i].name;
        }
        held += s.stats.heldBytes;
        quint32 rows = 0, cols = 0;
        s.cells.forEach([&](const Cell &c) {
            if (c.type == CellType::Empty) return;
            rows = std::max(rows, c.row + 1);
            cols = std::max(cols, quint32(c.col) + 1);
        });
        m_rows[size_t(i)] = rows;
        m_cols[size_t(i)] = cols;
    }
    return whole;
}

int WorkbookSource::sheetIndex(const QString &name) const { return m_byName.value(name.toCaseFolded(), -1); }

Value WorkbookSource::valueOf(const Sheet &sheet, const Cell &c) const
{
    switch (c.type) {
    case CellType::Empty: return Value();
    case CellType::Number:
    case CellType::Date: return Value::number(c.number);
    case CellType::SharedString: return Value::text(m_book.sharedStrings().at(c.textIndex()));
    case CellType::InlineString:
    case CellType::FormulaString: return Value::text(sheet.texts.value(c.textIndex()));
    case CellType::Bool: return Value::boolean(c.number != 0);
    case CellType::Error: return Value::error(ErrorKind(std::max(0, std::min(int(ErrorKind::Unknown), int(c.number)))));
    }
    return Value();
}

Value WorkbookSource::cell(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size()) || col >= kMaxColumns) return Value();
    const Sheet &s = m_sheets[size_t(sheet)];
    const Cell *c = s.cells.find(row, col);
    return c ? valueOf(s, *c) : Value();
}

void WorkbookSource::visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const Value &)> &f) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return;
    const Sheet &s = m_sheets[size_t(sheet)];
    CellStore::RangeWalk walk;
    s.cells.forEachInRange(row1, col1, row2, std::min<quint32>(col2, kMaxColumns - 1), [&](const Cell &c) {
        if (c.type == CellType::Empty) return true;
        return f(c.row, c.col, valueOf(s, c));
    }, &walk);
    jp::calc::budget::addWork((walk.compared + walk.stepped) / 4);   // (the rows walked over to find the cells are work too)
}

void WorkbookSource::usedExtent(int sheet, quint32 *rows, quint32 *cols) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) {
        *rows = *cols = 0;
        return;
    }
    *rows = m_rows[size_t(sheet)];
    *cols = m_cols[size_t(sheet)];
}

bool WorkbookSource::isSubtotal(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return false;
    const Sheet &s = m_sheets[size_t(sheet)];
    const Cell *c = s.cells.find(row, col);
    if (!c || !c->hasFormula() || c->formula < 0 || c->formula >= s.formulas.size()) return false;
    const qint32 text = s.formulas[c->formula].text;
    if (text < 0 || text >= s.formulaTexts.size()) return false;
    if (m_subtotalTexts.size() != m_sheets.size()) m_subtotalTexts.resize(m_sheets.size());
    std::vector<char> &known = m_subtotalTexts[size_t(sheet)];
    if (known.size() != size_t(s.formulaTexts.size())) known.assign(size_t(s.formulaTexts.size()), -1);
    if (known[size_t(text)] < 0) known[size_t(text)] = s.formulaTexts[text].contains(QLatin1String("SUBTOTAL("), Qt::CaseInsensitive) ? 1 : 0;
    return known[size_t(text)] == 1;
}

QString WorkbookSource::definedName(int sheet, const QString &name) const
{
    const QString upper = name.toUpper();
    if (sheet >= 0) {
        const auto own = m_names.constFind(QString::number(sheet) + QLatin1Char('|') + upper);
        if (own != m_names.constEnd()) return own.value();
    }
    return m_names.value(QStringLiteral("-1|") + upper);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// the check

namespace {

QString display(const Value &v)
{
    switch (v.kind()) {
    case Value::Kind::Empty: return QString();
    case Value::Kind::Number: return QString::number(v.number(), 'g', 17);
    case Value::Kind::Text: return v.text();
    case Value::Kind::Bool: return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case Value::Kind::Error: return errorKindText(v.errorKind());
    case Value::Kind::Array: return QStringLiteral("{array}");
    }
    return {};
}

// What kind a result is, for the patterns.
QString kindOf(const Value &v)
{
    switch (v.kind()) {
    case Value::Kind::Number: return QStringLiteral("number");
    case Value::Kind::Text: return v.text().isEmpty() ? QStringLiteral("empty text") : QStringLiteral("text");
    case Value::Kind::Bool: return QStringLiteral("bool");
    case Value::Kind::Error: return errorKindText(v.errorKind());
    default: return QStringLiteral("none");
    }
}

// Two numbers are the same if they agree to 15 significant digits, or are a few units of the last place apart: two math libraries differ there, and a pair of
// numbers that round to 15 digits on either side of a boundary would be called different for one unit in the 17th.
bool sameNumber(double a, double b)
{
    if (sameTo15Digits(a, b)) return true;
    return std::isfinite(a) && std::isfinite(b) && std::fabs(a - b) <= 4 * 2.220446049250313e-16 * std::max(std::fabs(a), std::fabs(b));
}

bool sameResult(const Value &ours, const Value &stored, bool storedIsDate)
{
    Q_UNUSED(storedIsDate);
    switch (stored.kind()) {
    case Value::Kind::Number: return ours.isNumber() && sameNumber(ours.number(), stored.number());
    case Value::Kind::Bool: return ours.isBool() && ours.boolean() == stored.boolean();
    case Value::Kind::Error: return ours.isError() && ours.errorKind() == stored.errorKind();
    case Value::Kind::Text: return ours.isText() && ours.text() == stored.text();
    default: return false;
    }
}

struct Compiled {
    std::shared_ptr<const Program> program;
    QString error;
    bool tried = false;
};

} // namespace

bool recalcWorkbook(const Workbook &book, const RecalcOptions &options, const std::function<void(const RecalcCell &)> &each, RecalcReport *report, QString *error)
{
    QElapsedTimer timer;
    timer.start();
    WorkbookSource source(book, options.workbookBytes);
    *report = RecalcReport();
    report->manualCalc = book.info().calcMode == CalcMode::Manual;
    report->whole = source.loadAll();
    const int sheets = int(book.info().sheets.size());
    if (sheets == 0) {
        if (error) *error = QStringLiteral("The workbook has no sheets.");
        return false;
    }
    Evaluator evaluator;
    evaluator.seedRandom(1);
    if (options.workLimit > 0) evaluator.setWorkLimit(options.workLimit);
    EvalContext ctx;
    ctx.source = &source;
    ctx.dateSystem = book.info().dateSystem;
    ctx.now = 45366.5;
    qint64 kept = 0;
    bool timeUp = false;
    for (int si = 0; si < sheets; ++si) {
        const Sheet &sheet = source.sheet(si);
        if (sheet.formulaCells == 0) continue;
        const QString sheetName = book.info().sheets[si].name;
        std::vector<Compiled> plain(size_t(sheet.formulaTexts.size())), array(size_t(sheet.formulaTexts.size()));
        ctx.sheet = si;
        sheet.cells.forEach([&](const Cell &c) {
            if (!c.hasFormula()) return;
            ++report->formulaCells;
            RecalcCell out;
            out.cls = RecalcClass::Match;
            Value storedValue, oursValue;
            bool haveValues = false;
            auto finish = [&](RecalcClass cls) {
                out.cls = cls;
                ++report->byClass[char(cls)];
                if (cls == RecalcClass::Match) {
                    if (kept >= options.keepMatches) return;
                    ++kept;
                }
                if (!each) return;
                out.sheet = sheetName;
                out.cell = cellName(c.row, c.col);
                out.formula = sheet.formulaText(c);
                if (haveValues) {
                    out.stored = display(storedValue);
                    out.ours = display(oursValue);
                }
                each(out);
            };
            if (timeUp || (options.seconds > 0 && double(timer.nsecsElapsed()) > options.seconds * 1e9)) {
                timeUp = true;
                finish(RecalcClass::Skipped);
                return;
            }
            if (c.flags & DataTable) {
                finish(RecalcClass::DataTable);
                return;
            }
            const Formula &f = c.formula >= 0 && c.formula < sheet.formulas.size() ? sheet.formulas[c.formula] : Formula();
            if (f.text < 0 || f.text >= sheet.formulaTexts.size() || sheet.formulaTexts[f.text].trimmed().isEmpty()) {
                // an element that holds no formula (the file marks the cell, but there is nothing to work out)
                finish(RecalcClass::NoResult);
                return;
            }
            Compiled &slot = (c.flags & ArrayFormula) ? array[size_t(f.text)] : plain[size_t(f.text)];
            if (!slot.tried) {
                slot.tried = true;
                ParseOptions po;
                po.arrayFormula = (c.flags & ArrayFormula) != 0;
                const ParseResult pr = parseFormula(sheet.formulaTexts[f.text], po);
                slot.program = pr.program;
                slot.error = pr.error;
            }
            if (!slot.program) {
                storedValue = source.valueOf(sheet, c);
                oursValue = Value::text(slot.error);
                haveValues = true;
                finish(RecalcClass::ParseError);
                return;
            }
            const Program &p = *slot.program;
            for (const auto &call : p.calls) out.functions << QString::fromLatin1(functionAt(call.first).name);
            if (!p.unknownFunctions.isEmpty()) {
                for (const QString &name : p.unknownFunctions) ++report->unsupportedFunctions[name];
                out.functions << p.unknownFunctions;
                finish(RecalcClass::Unsupported);
                return;
            }
            // a rate found by trying numbers: the file's digits are those of one program's iteration, so it is kept apart
            bool iterative = false;
            for (const auto &call : p.calls) iterative = iterative || (functionAt(call.first).flags & kFnIterative);
            if (iterative) {
                finish(RecalcClass::Iterated);
                return;
            }
            if (p.external) {
                finish(RecalcClass::External);
                return;
            }
            if (p.structured) {
                finish(RecalcClass::Structured);
                return;
            }
            const bool volatileFormula = p.isVolatile;
            if (volatileFormula) {
                // The ones that change by themselves can't be checked; the ones that only depend on cells can, and are counted apart.
                bool changes = false;
                for (const auto &call : p.calls) {
                    const char *name = functionAt(call.first).name;
                    if ((functionAt(call.first).flags & kFnVolatile) && strcmp(name, "OFFSET") != 0 && strcmp(name, "INDIRECT") != 0) changes = true;
                }
                if (changes || !options.volatileToo) {
                    finish(RecalcClass::Volatile);
                    return;
                }
            }
            const Value stored = source.valueOf(sheet, c);
            if (c.type == CellType::Empty) {
                finish(RecalcClass::NoResult);
                return;
            }
            ctx.row = c.row;
            ctx.col = c.col;
            ctx.shiftRows = f.dRow;
            ctx.shiftCols = f.dCol;
            Value ours = evaluator.evaluate(p, ctx);
            storedValue = stored;
            if (const int met = evaluator.unsupportedKinds()) {
                oursValue = ours;
                haveValues = true;
                finish(met & 1 ? RecalcClass::External : met & 2 ? RecalcClass::Structured : RecalcClass::Unsupported);
                return;
            }
            if (evaluator.status() != EvalStatus::Ok) {
                oursValue = Value();
                haveValues = true;
                finish(RecalcClass::Limit);
                return;
            }
            if (ours.isArray()) ours = ours.at(0, 0);
            oursValue = ours;
            haveValues = true;
            const bool match = sameResult(ours, stored, c.type == CellType::Date);
            if (volatileFormula) {
                ++report->volatileChecked;
                if (match) ++report->volatileMatched;
                finish(RecalcClass::Volatile);
                return;
            }
            QStringList names = out.functions;
            names.removeDuplicates();
            for (const auto &call : p.calls) {
                const QString name = QString::fromLatin1(functionAt(call.first).name);
                FunctionTally &t = report->functions[name];
                t.calls += call.second;
                ++t.cells;
                if (match) {
                    t.matchedCalls += call.second;
                    ++t.matchedCells;
                }
            }
            out.functions = names;
            if (match) {
                finish(RecalcClass::Match);
                return;
            }
            QStringList fns = names;
            fns.sort();
            ++report->patterns[kindOf(stored) + QStringLiteral(" -> ") + kindOf(ours) + QStringLiteral(": ") + fns.join(QLatin1Char('+'))];
            finish(RecalcClass::Mismatch);
        });
    }
    report->seconds = timer.elapsed() / 1000.0;
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// the full recalculation

bool recalcFull(const Workbook &book, const FullOptions &options, const std::function<void(const RecalcCell &)> &each, FullReport *report, QString *error)
{
    QElapsedTimer timer;
    timer.start();
    *report = FullReport();
    if (book.info().sheets.isEmpty()) {
        if (error) *error = QStringLiteral("The workbook has no sheets.");
        return false;
    }
    CalcOptions copts;
    copts.keepApart = true;
    copts.workLimit = options.workLimit;
    copts.now = options.now;
    copts.workbookBytes = options.workbookBytes;
    copts.verify = [](const Value &stored, const Value &ours) { return sameResult(ours.isArray() ? ours.at(0, 0) : ours, stored, false); };
    Calculation calc(book, copts);
    report->manualCalc = book.info().calcMode == CalcMode::Manual;
    report->whole = calc.load();
    report->loadSeconds = double(timer.nsecsElapsed()) / 1e9;
    QElapsedTimer calcTimer;
    calcTimer.start();
    for (;;) {
        if (options.seconds > 0 && double(timer.nsecsElapsed()) > options.seconds * 1e9) break;   // (what is not reached is kept apart)
        const Calculation::Progress p = calc.recalculate(options.seconds > 0 ? 200000 : 0);
        report->evaluations += p.evaluated;
        if (p.finished) break;
    }
    report->calcSeconds = double(calcTimer.nsecsElapsed()) / 1e9;
    qint64 kept = 0;
    calc.forEachFormula([&](const Calculation::FormulaCell &fc) {
        ++report->formulaCells;
        const Sheet &sheet = calc.sheet(fc.sheet);
        const Cell &c = *fc.cell;
        RecalcCell out;
        bool haveValues = false;
        auto finish = [&](RecalcClass cls) {
            out.cls = cls;
            ++report->byClass[char(cls)];
            if (cls == RecalcClass::Match) {
                if (kept >= options.keepMatches) return;
                ++kept;
            }
            if (!each) return;
            out.sheet = book.info().sheets[fc.sheet].name;
            out.cell = cellName(c.row, c.col);
            out.formula = sheet.formulaText(c);
            if (haveValues) {
                out.stored = display(fc.stored);
                out.ours = display(fc.ours);
            }
            each(out);
        };
        if (fc.program) {
            for (const auto &call : fc.program->calls) out.functions << QString::fromLatin1(functionAt(call.first).name);
            out.functions << fc.program->unknownFunctions;
        }
        if (fc.circular) {
            ++report->circular;
            if (fc.stored.isNumber() && fc.stored.number() == 0) ++report->circularStoredZero;
        }
        out.cascaded = fc.cascaded;
        if (!fc.worked) {
            finish(RecalcClass::Skipped);
            return;
        }
        if (fc.kept) {
            if (fc.kept == 'U' && fc.program)
                for (const QString &name : fc.program->unknownFunctions) ++report->unsupportedFunctions[name];
            haveValues = fc.kept == 'P' || fc.kept == 'L' || fc.kept == 'I' || fc.kept == 'C';
            finish(RecalcClass(fc.kept));
            return;
        }
        Value ours = fc.ours;
        if (ours.isArray()) ours = ours.at(0, 0);
        const bool match = sameResult(ours, fc.stored, c.type == CellType::Date);
        haveValues = true;
        if (fc.tainted) {
            if (match) ++report->taintedMatched;
            finish(RecalcClass::Tainted);
            return;
        }
        bool dynamic = false;
        if (fc.program)
            for (const auto &call : fc.program->calls) {
                const char *name = functionAt(call.first).name;
                if (strcmp(name, "OFFSET") == 0 || strcmp(name, "INDIRECT") == 0) dynamic = true;
            }
        if (dynamic) {
            ++report->volatileChecked;
            if (match) ++report->volatileMatched;
            finish(RecalcClass::Volatile);
            return;
        }
        if (fc.program) {
            for (const auto &call : fc.program->calls) {
                FunctionTally &t = report->functions[QString::fromLatin1(functionAt(call.first).name)];
                t.calls += call.second;
                ++t.cells;
                if (match) {
                    t.matchedCalls += call.second;
                    ++t.matchedCells;
                }
            }
        }
        if (match) {
            finish(RecalcClass::Match);
            return;
        }
        QStringList fns = out.functions;
        fns.removeDuplicates();
        fns.sort();
        // a mismatch that follows another is told by its own pattern: the commonest are the roots
        if (!fc.cascaded) {
            ++report->rootMismatches;
            ++report->patterns[kindOf(fc.stored) + QStringLiteral(" -> ") + kindOf(ours) + QStringLiteral(": ") + fns.join(QLatin1Char('+'))];
        }
        finish(RecalcClass::Mismatch);
    });
    report->graphEntries = calc.graph().entryCount();
    report->graphBytes = calc.graph().memoryBytes();
    report->untracked = calc.untrackedCount();
    for (int i = 0; i < calc.sheetCount(); ++i) report->sheetXmlBytes = std::max(report->sheetXmlBytes, calc.sheet(i).stats.xmlBytes);
    report->seconds = double(timer.nsecsElapsed()) / 1e9;
    return true;
}

} // namespace jp::office::sheet

#include "calc/eval.h"

#include "calc/machine.h"

#include <QDateTime>

#include <algorithm>
#include <cmath>

namespace jp::calc {

namespace {

bool isErr(const Value &v) { return v.kind() == Value::Kind::Error; }

// About the bytes a program takes (what the cache of INDIRECT's texts counts).
qint64 programBytes(const Program &p)
{
    qint64 n = qint64(sizeof(Program)) + qint64(p.code.size() * sizeof(Instr)) + qint64(p.numbers.size() * sizeof(double)) + qint64(p.refs.size() * sizeof(RefLit));
    for (const QString &t : p.texts) n += 32 + 2 * qint64(t.size());
    for (const Value &a : p.arrays) n += qint64(a.rows()) * a.cols() * qint64(sizeof(Value));
    return n;
}

// The texts INDIRECT and the defined names are worked out from are kept, parsed, to be worked out again; up to this many bytes (then they are all let go).
constexpr qint64 kMaxCachedTextBytes = 16000000;

// Is the program only references and names, with the operators between references? (A call, a number, a sum are none.)
bool namesAReference(const Program &p)
{
    for (const Instr &in : p.code) {
        switch (in.op) {
        case Op::PushRef: case Op::PushName: case Op::PushTable: case Op::PushExternal: case Op::Range: case Op::Union: case Op::Intersect: break;
        default: return false;
        }
    }
    return !p.code.empty();
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// the machine

Machine::Machine()
{
    std::random_device device;
    rng.seed((quint64(device()) << 32) ^ quint64(device()));
}

double clockSerial(DateSystem system)
{
    const QDateTime t = QDateTime::currentDateTime();
    double serial = 0;
    if (!dateToSerial(t.date().year(), t.date().month(), t.date().day(), system, &serial)) return 0;
    return serial + double(t.time().msecsSinceStartOfDay()) / 86400000.0;
}

double Machine::now() const
{
    if (ctx.now > 0) return ctx.now;
    if (!m_clockRead) {
        m_clock = clockSerial(ctx.dateSystem);
        m_clockRead = true;
    }
    return m_clock;
}

bool Machine::bindSheets(const Program &p)
{
    const qint64 generation = ctx.source->generation();
    if (p.boundTo == ctx.source && p.boundGeneration == generation && p.boundSheets.size() == size_t(p.sheetNames.size())) return true;
    p.boundSheets.assign(size_t(p.sheetNames.size()), -1);
    for (int i = 0; i < p.sheetNames.size(); ++i) p.boundSheets[size_t(i)] = ctx.source->sheetIndex(p.sheetNames[i]);
    p.boundTo = ctx.source;
    p.boundGeneration = generation;
    return true;
}

bool Machine::resolveRef(const Program &p, const RefLit &r, RefArea *area, ErrorKind *error)
{
    qint32 s1 = r.sheet1 >= 0 ? p.boundSheets[size_t(r.sheet1)] : ctx.sheet;
    qint32 s2 = r.sheet2 >= 0 ? p.boundSheets[size_t(r.sheet2)] : s1;
    if (s1 < 0 || s2 < 0 || s1 >= ctx.source->sheetCount() || s2 >= ctx.source->sheetCount()) {
        *error = ErrorKind::Ref;
        return false;
    }
    if (s2 < s1) std::swap(s1, s2);
    const qint64 dr = ctx.shiftRows, dc = ctx.shiftCols;
    qint64 r1 = (r.flags & kRefRow1Abs) ? qint64(r.row1) : qint64(r.row1) + dr;
    qint64 r2 = (r.flags & kRefRow2Abs) ? qint64(r.row2) : qint64(r.row2) + dr;
    qint64 c1 = (r.flags & kRefCol1Abs) ? qint64(r.col1) : qint64(r.col1) + dc;
    qint64 c2 = (r.flags & kRefCol2Abs) ? qint64(r.col2) : qint64(r.col2) + dc;
    if (wrapRefs) {
        // the references of a defined name come round the other side of the sheet
        auto wrap = [](qint64 v, qint64 n) { return v >= 0 ? v % n : ((v % n) + n) % n; };
        if (!(r.flags & kRefRow1Abs)) r1 = wrap(r1, kGridRows);
        if (!(r.flags & kRefRow2Abs)) r2 = wrap(r2, kGridRows);
        if (!(r.flags & kRefCol1Abs)) c1 = wrap(c1, kGridCols);
        if (!(r.flags & kRefCol2Abs)) c2 = wrap(c2, kGridCols);
    }
    if (r1 > r2) std::swap(r1, r2);
    if (c1 > c2) std::swap(c1, c2);
    if (r1 < 0 || r2 >= qint64(kGridRows) || c1 < 0 || c2 >= qint64(kGridCols)) {
        *error = ErrorKind::Ref;
        return false;
    }
    area->sheet1 = s1;
    area->sheet2 = s2;
    area->row1 = quint32(r1);
    area->row2 = quint32(r2);
    area->col1 = quint32(c1);
    area->col2 = quint32(c2);
    return true;
}

Value Machine::intersect(const RefArea &a) const
{
    if (a.sheet1 != a.sheet2) return Value::error(ErrorKind::Value);
    quint32 row, col;
    if (a.col1 == a.col2) {
        if (ctx.row < a.row1 || ctx.row > a.row2) return Value::error(ErrorKind::Value);
        row = ctx.row;
        col = a.col1;
    } else if (a.row1 == a.row2) {
        if (ctx.col < a.col1 || ctx.col > a.col2) return Value::error(ErrorKind::Value);
        row = a.row1;
        col = ctx.col;
    } else {
        if (ctx.row < a.row1 || ctx.row > a.row2 || ctx.col < a.col1 || ctx.col > a.col2) return Value::error(ErrorKind::Value);
        row = ctx.row;
        col = ctx.col;
    }
    return ctx.source->cell(a.sheet1, row, col);
}

Value Machine::scalar(const Item &item, bool keepArrays)
{
    switch (item.kind) {
    case Item::Kind::Missing: return Value();
    case Item::Kind::Value:
        if (item.value.isArray() && !keepArrays) return item.value.at(0, 0);
        return item.value;
    case Item::Kind::Ref:
        if (item.more) return Value::error(ErrorKind::Value);
        if (item.area.single()) return ctx.source->cell(item.area.sheet1, item.area.row1, item.area.col1);
        if (keepArrays) return toArray(item);
        return intersect(item.area);
    }
    return Value();
}

Value Machine::toArray(const Item &item)
{
    if (item.kind != Item::Kind::Ref) return item.kind == Item::Kind::Value ? item.value : Value();
    if (item.more || item.area.sheet1 != item.area.sheet2) return Value::error(ErrorKind::Value);
    const RefArea &a = item.area;
    if (a.rows() * a.cols() > kMaxArrayCells) {
        tooLarge();
        return Value::error(ErrorKind::Value);
    }
    if (!charge(a.rows() * a.cols() / 2 + 1)) return Value::error(ErrorKind::Value);
    Value array = newArray(int(a.rows()), int(a.cols()));
    if (!array.isArray()) return array;
    ctx.source->visit(a.sheet1, a.row1, a.col1, a.row2, a.col2, [&](quint32 r, quint32 c, const Value &v) {
        if (!charge(1)) return false;
        if (!v.isEmpty()) array.set(int(r - a.row1), int(c - a.col1), v);
        return true;
    });
    return array;
}

Item Machine::unaryOp(Op op, quint8 flags, const Item &item)
{
    const bool keep = flags & kInstrArray;
    const Value a = scalar(item, keep);
    auto one = [&](const Value &v) -> Value {
        double x;
        ErrorKind e = ErrorKind::Value;
        if (!coerceNumber(v, ctx.dateSystem, &x, &e)) return Value::error(e);
        return numberResult(op == Op::Neg ? 0.0 - x : x / 100.0);
    };
    if (!a.isArray()) return Item::ofValue(one(a));
    if (qint64(a.rows()) * a.cols() > kMaxArrayCells) {
        tooLarge();
        return Item::ofError(ErrorKind::Value);
    }
    if (!charge(qint64(a.rows()) * a.cols())) return Item::ofError(ErrorKind::Value);
    Value r = newArray(a.rows(), a.cols());
    if (!r.isArray()) return Item::ofValue(r);
    for (int i = 0; i < a.rows(); ++i)
        for (int j = 0; j < a.cols(); ++j) r.set(i, j, one(a.at(i, j)));
    return Item::ofValue(r);
}

namespace {

// One operation on two values. A text a join makes is charged before it is made: by its size as an element of an array (`element`, with the budget for the text of
// arrays), else as a step for every 64 characters.
Value scalarBinary(Op op, const Value &a, const Value &b, bool final, DateSystem system, Machine *machine = nullptr, bool element = false)
{
    switch (op) {
    case Op::Add: case Op::Sub: case Op::Mul: case Op::Div: case Op::Pow: {
        double x, y;
        ErrorKind e = ErrorKind::Value;
        if (!coerceNumber(a, system, &x, &e)) return Value::error(e);
        if (!coerceNumber(b, system, &y, &e)) return Value::error(e);
        double r = 0;
        switch (op) {
        case Op::Add:
            r = x + y;
            if (final && r != 0 && (x < 0) != (y < 0) && std::fabs(r) <= std::max(std::fabs(x), std::fabs(y)) * kOperatorSnap) r = 0;
            break;
        case Op::Sub:
            r = x - y;
            if (final && r != 0 && (x < 0) == (y < 0) && std::fabs(r) <= std::max(std::fabs(x), std::fabs(y)) * kOperatorSnap) r = 0;
            break;
        case Op::Mul: r = x * y; break;
        case Op::Div:
            if (y == 0) return Value::error(ErrorKind::Div0);
            r = x / y;
            break;
        default:
            if (x == 0 && y == 0) return Value::error(ErrorKind::Num);
            if (x == 0 && y < 0) return Value::error(ErrorKind::Div0);
            if (x < 0 && y != std::floor(y)) return Value::error(ErrorKind::Num);
            r = powerOf(x, y);
            break;
        }
        return numberResult(r);
    }
    case Op::Concat: {
        QString x, y;
        ErrorKind e = ErrorKind::Value;
        if (!coerceText(a, &x, &e)) return Value::error(e);
        if (!coerceText(b, &y, &e)) return Value::error(e);
        const qint64 size = qint64(x.size()) + y.size();
        if (size > kMaxValueTextChars) return Value::error(ErrorKind::Value);
        if (machine && !(element ? machine->chargeText(size) : machine->charge(size / 64))) return Value::error(ErrorKind::Value);
        return Value::text(x + y);
    }
    default: {
        if (isErr(a)) return a;
        if (isErr(b)) return b;
        const int c = compareValues(a, b);
        switch (op) {
        case Op::Eq: return Value::boolean(c == 0);
        case Op::Ne: return Value::boolean(c != 0);
        case Op::Lt: return Value::boolean(c < 0);
        case Op::Le: return Value::boolean(c <= 0);
        case Op::Gt: return Value::boolean(c > 0);
        default: return Value::boolean(c >= 0);
        }
    }
    }
}

} // namespace

Item Machine::binary(Op op, quint8 flags, const Item &ia, const Item &ib)
{
    const bool keep = flags & kInstrArray;
    const Value a = scalar(ia, keep);
    const Value b = scalar(ib, keep);
    if (!a.isArray() && !b.isArray()) return Item::ofValue(scalarBinary(op, a, b, flags & kInstrFinal, ctx.dateSystem, this));
    const int rows = std::max(a.rows(), b.rows()), cols = std::max(a.cols(), b.cols());
    if (qint64(rows) * cols > kMaxArrayCells) {
        tooLarge();
        return Item::ofError(ErrorKind::Value);
    }
    if (!charge(qint64(rows) * cols)) return Item::ofError(ErrorKind::Value);
    Value r = newArray(rows, cols);
    if (!r.isArray()) return Item::ofValue(r);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) {
            const Value &x = a.at(a.rows() == 1 ? 0 : i, a.cols() == 1 ? 0 : j);
            const Value &y = b.at(b.rows() == 1 ? 0 : i, b.cols() == 1 ? 0 : j);
            const Value e = scalarBinary(op, x, y, false, ctx.dateSystem, this, true);
            if (!charge(0)) return Item::ofError(ErrorKind::Value);   // (the steps of comparing texts count here)
            r.set(i, j, e);
        }
    return Item::ofValue(r);
}

Item Machine::referenceOp(Op op, const Item &a, const Item &b)
{
    if (!a.isRef() || !b.isRef()) {
        // an error in either stands; anything else is no reference
        const Value x = a.kind == Item::Kind::Value ? a.value : Value(), y = b.kind == Item::Kind::Value ? b.value : Value();
        if (isErr(x)) return Item::ofValue(x);
        if (isErr(y)) return Item::ofValue(y);
        return Item::ofError(ErrorKind::Value);
    }
    if (op == Op::Union) {
        Item r = a;
        auto list = std::make_shared<std::vector<RefArea>>();
        if (a.more) *list = *a.more;
        list->push_back(b.area);
        if (b.more) list->insert(list->end(), b.more->begin(), b.more->end());
        if (list->size() > 1000) return Item::ofError(ErrorKind::Value);
        r.more = list;
        return r;
    }
    if (a.more || b.more || a.area.sheet1 != b.area.sheet1 || a.area.sheet2 != b.area.sheet2) return Item::ofError(ErrorKind::Value);
    RefArea r = a.area;
    if (op == Op::Range) {
        r.row1 = std::min(a.area.row1, b.area.row1);
        r.col1 = std::min(a.area.col1, b.area.col1);
        r.row2 = std::max(a.area.row2, b.area.row2);
        r.col2 = std::max(a.area.col2, b.area.col2);
        return Item::ofRef(r);
    }
    r.row1 = std::max(a.area.row1, b.area.row1);
    r.col1 = std::max(a.area.col1, b.area.col1);
    r.row2 = std::min(a.area.row2, b.area.row2);
    r.col2 = std::min(a.area.col2, b.area.col2);
    if (r.row1 > r.row2 || r.col1 > r.col2) return Item::ofError(ErrorKind::Null);
    return Item::ofRef(r);
}

Item Machine::callFunction(const FunctionInfo &f, std::vector<Item> &args, bool arrayMode)
{
    const int n = int(args.size());
    bool lift = false;
    for (int i = 0; i < n; ++i) {
        const char cls = paramClass(f, i);
        Item &it = args[size_t(i)];
        if (cls == 'v') {
            if (it.isMissing()) continue;
            if (it.isRef() || (it.kind == Item::Kind::Value && it.value.isArray())) {
                const Value v = scalar(it, arrayMode);
                if (v.isArray()) lift = true;
                it = Item::ofValue(v);
            }
        } else if (cls == 'x') {
            if (!it.isRef() && !it.isMissing()) {
                if (it.kind == Item::Kind::Value && isErr(it.value)) return it;
                return Item::ofError(ErrorKind::Value);
            }
        }
    }
    if (!lift) {
        Call call(*this, args, arrayMode, f);
        const Value r = f.impl(call);
        return call.hasResult() ? call.result() : Item::ofValue(r);
    }
    // A function of values, given arrays: one call for each element, the arrays that have one row or one column stretched to the others.
    int rows = 1, cols = 1;
    for (int i = 0; i < n; ++i) {
        if (paramClass(f, i) != 'v' || args[size_t(i)].kind != Item::Kind::Value) continue;
        rows = std::max(rows, args[size_t(i)].value.rows());
        cols = std::max(cols, args[size_t(i)].value.cols());
    }
    if (qint64(rows) * cols > kMaxArrayCells) {
        tooLarge();
        return Item::ofError(ErrorKind::Value);
    }
    if (!charge(qint64(rows) * cols)) return Item::ofError(ErrorKind::Value);
    Value result = newArray(rows, cols);
    if (!result.isArray()) return Item::ofValue(result);
    std::vector<Item> one(args.size());
    for (int r = 0; r < rows && !over; ++r)
        for (int c = 0; c < cols; ++c) {
            if (!charge(1)) break;
            for (int i = 0; i < n; ++i) {
                const Item &it = args[size_t(i)];
                if (paramClass(f, i) == 'v' && it.kind == Item::Kind::Value && it.value.isArray())
                    one[size_t(i)] = Item::ofValue(it.value.at(it.value.rows() == 1 ? 0 : r, it.value.cols() == 1 ? 0 : c));
                else
                    one[size_t(i)] = it;
            }
            Call call(*this, one, false, f);
            Value v = f.impl(call);
            if (call.hasResult()) v = scalar(call.result(), false);
            if (v.isArray()) v = v.at(0, 0);
            if (v.isText() && !chargeText(v.text().size())) break;
            result.set(r, c, v);
        }
    return Item::ofValue(result);
}

Item Machine::runProgram(const Program &p, bool)
{
    bindSheets(p);
    const size_t base = stack.size();
    const Instr *code = p.code.data();
    const size_t count = p.code.size();
    size_t pc = 0;
    auto bad = [&]() {
        stack.resize(base);
        return Item::ofError(ErrorKind::Value);
    };
    auto pop = [&]() {
        Item it = std::move(stack.back());
        stack.pop_back();
        return it;
    };
    auto have = [&](size_t k) { return stack.size() >= base + k; };
    while (pc < count) {
        if (over) {
            stack.resize(base);
            return Item::ofError(ErrorKind::Value);
        }
        const Instr &in = code[pc++];
        if (!charge(1)) break;
        switch (in.op) {
        case Op::PushNumber: stack.push_back(Item::ofValue(Value::number(p.numbers[size_t(in.operand)]))); break;
        case Op::PushText: stack.push_back(Item::ofValue(Value::text(p.texts[size_t(in.operand)]))); break;
        case Op::PushBool: stack.push_back(Item::ofValue(Value::boolean(in.operand != 0))); break;
        case Op::PushError: stack.push_back(Item::ofError(ErrorKind(in.operand))); break;
        case Op::PushMissing: stack.push_back(Item::missing()); break;
        case Op::PushArray: stack.push_back(Item::ofValue(p.arrays[size_t(in.operand)])); break;
        case Op::PushRef: {
            RefArea area;
            ErrorKind e = ErrorKind::Ref;
            if (resolveRef(p, p.refs[size_t(in.operand)], &area, &e))
                stack.push_back(Item::ofRef(area));
            else
                stack.push_back(Item::ofError(e));
            break;
        }
        case Op::PushName: {
            const QString &name = p.texts[size_t(in.operand)];
            int sheet = ctx.sheet;
            if (in.argc > 0) {
                sheet = p.boundSheets[size_t(in.argc) - 1];
                if (sheet < 0) {
                    stack.push_back(Item::ofError(ErrorKind::Ref));
                    break;
                }
            }
            const QString formula = ctx.source->definedName(sheet, name);
            if (formula.isEmpty()) {
                stack.push_back(Item::ofError(ErrorKind::Name));
                break;
            }
            // The name is known by what it is: the sheet whose own name it is, or the workbook's (when the sheet has none of its own, the workbook's is the one that answers).
            const int scope = formula == ctx.source->definedName(-1, name) ? -1 : sheet;
            stack.push_back(evaluateText(formula, false, true, sheet, QString::number(scope) + QLatin1Char('|') + name.toUpper()));
            break;
        }
        case Op::PushTable: unsupported |= 2; stack.push_back(Item::ofError(ErrorKind::Ref)); break;
        case Op::PushExternal: unsupported |= 1; stack.push_back(Item::ofError(ErrorKind::Ref)); break;
        case Op::Add: case Op::Sub: case Op::Mul: case Op::Div: case Op::Pow: case Op::Concat:
        case Op::Eq: case Op::Ne: case Op::Lt: case Op::Le: case Op::Gt: case Op::Ge: {
            if (!have(2)) return bad();
            const Item b = pop();
            const Item a = pop();
            stack.push_back(binary(in.op, in.flags, a, b));
            break;
        }
        case Op::Range: case Op::Union: case Op::Intersect: {
            if (!have(2)) return bad();
            const Item b = pop();
            const Item a = pop();
            stack.push_back(referenceOp(in.op, a, b));
            break;
        }
        case Op::Neg: case Op::Percent: {
            if (!have(1)) return bad();
            const Item a = pop();
            stack.push_back(unaryOp(in.op, in.flags, a));
            break;
        }
        case Op::Single: {
            if (!have(1)) return bad();
            const Item a = pop();
            stack.push_back(Item::ofValue(scalar(a, false)));
            break;
        }
        case Op::Spill: {
            if (!have(1)) return bad();
            pop();
            unsupported |= 2;
            stack.push_back(Item::ofError(ErrorKind::Ref));
            break;
        }
        case Op::Call: {
            const size_t argc = in.argc;
            if (!have(argc)) return bad();
            std::vector<Item> args(argc);
            for (size_t k = argc; k-- > 0;) args[k] = pop();
            if (in.operand < 0) {
                unsupported |= 4;
                stack.push_back(Item::ofError(ErrorKind::Name));
                break;
            }
            Item r = callFunction(functionAt(in.operand), args, in.flags & kInstrArray);
            stack.push_back(std::move(r));
            break;
        }
        case Op::Jump: pc += size_t(in.operand); break;
        case Op::IfJump: {
            if (!have(1)) return bad();
            const Item cond = pop();
            const Value v = scalar(cond, false);
            bool truth = false;
            if (isErr(v)) {
                stack.push_back(Item::ofValue(v));
                pc += in.argc;
                break;
            }
            if (v.isNumber() || v.isBool()) {
                truth = v.number() != 0;
            } else if (v.isText()) {
                if (v.text().compare(QLatin1String("TRUE"), Qt::CaseInsensitive) == 0) {
                    truth = true;
                } else if (v.text().compare(QLatin1String("FALSE"), Qt::CaseInsensitive) != 0) {
                    stack.push_back(Item::ofError(ErrorKind::Value));
                    pc += in.argc;
                    break;
                }
            }
            if (!truth) pc += size_t(in.operand);
            break;
        }
        case Op::TryJump: {
            if (!have(1)) return bad();
            const Value v = scalar(stack.back(), false);
            const bool caught = isErr(v) && ((in.flags & 1) == 0 || v.errorKind() == ErrorKind::NA);
            if (caught) {
                pop();
            } else {
                stack.back() = Item::ofValue(v);
                pc += size_t(in.operand);
            }
            break;
        }
        case Op::Choose: {
            if (!have(1)) return bad();
            const Item index = pop();
            const Value v = scalar(index, false);
            double x;
            ErrorKind e = ErrorKind::Value;
            if (!coerceNumber(v, ctx.dateSystem, &x, &e)) {
                stack.push_back(Item::ofError(e));
                pc += size_t(in.operand);
                break;
            }
            const double k = std::trunc(x);
            if (!(k >= 1) || k > double(in.argc)) {
                stack.push_back(Item::ofError(ErrorKind::Value));
                pc += size_t(in.operand);
                break;
            }
            pc += size_t(k) - 1;
            break;
        }
        }
    }
    if (over || stack.size() != base + 1) {
        stack.resize(base);
        return Item::ofError(ErrorKind::Value);
    }
    return pop();
}

Item Machine::evaluateText(const QString &text, bool r1c1, bool relativeToA1, int, const QString &identity)
{
    // Looking a text up takes as long as it is (its key is made and hashed).
    if (!charge(2 + qint64(text.size()) / 64)) return Item::ofError(ErrorKind::Value);
    if (nameDepth >= kMaxNameDepth) {
        if (status == EvalStatus::Ok) status = EvalStatus::TooDeep;
        return Item::ofError(ErrorKind::Ref);
    }
    // (A text in R1C1 form means what it says relative to the cell it is worked out for, so it is kept for that cell.)
    QString key = QString(r1c1 ? QLatin1Char('R') : QLatin1Char('A')) + (relativeToA1 ? QLatin1Char('n') : QLatin1Char('t'));
    if (r1c1) key += QString::number(ctx.row) + QLatin1Char(',') + QString::number(ctx.col) + QLatin1Char(':');
    key += text;
    // A text that is asked for while it is being worked out is a circle: a name that stands for itself, directly or through others. It is no number, and is not worked out
    // again inside itself (a name of three uses of itself would be three to the sixteenth evaluations to the depth that stops it).
    const QString &who = identity.isEmpty() ? key : identity;
    for (const QString &active : m_active)
        if (active == who) {
            if (status == EvalStatus::Ok) status = EvalStatus::TooDeep;
            return Item::ofError(ErrorKind::Ref);
        }
    auto it = m_texts.find(key);
    if (it == m_texts.end()) {
        if (m_texts.size() > 4096 || m_textsBytes > kMaxCachedTextBytes) {
            m_texts.clear();
            m_textsBytes = 0;
        }
        ParseOptions options;
        options.r1c1 = r1c1;
        options.definedName = relativeToA1;
        options.row = ctx.row;
        options.col = ctx.col;
        Cached c;
        const ParseResult r = parseFormula(text, options);
        if (r.ok()) c.program = r.program;
        m_textsBytes += 64 + 2 * qint64(key.size()) + (c.program ? programBytes(*c.program) : 0);
        it = m_texts.insert(key, c);
    }
    if (!it->program) return Item::ofError(relativeToA1 ? ErrorKind::Name : ErrorKind::Ref);
    const std::shared_ptr<const Program> program = it->program;
    // What INDIRECT is given names a reference (a cell, a range, a name): a formula that works one out is not a reference to take.
    if (!relativeToA1 && !namesAReference(*program)) return Item::ofError(ErrorKind::Ref);
    if (program->external) unsupported |= 1;
    if (program->structured) unsupported |= 2;
    if (!program->unknownFunctions.isEmpty()) unsupported |= 4;
    const int savedRows = ctx.shiftRows, savedCols = ctx.shiftCols;
    ctx.shiftRows = relativeToA1 ? int(ctx.row) : 0;
    ctx.shiftCols = relativeToA1 ? int(ctx.col) : 0;
    const bool savedWrap = wrapRefs;
    wrapRefs = relativeToA1;
    ++nameDepth;
    m_active.push_back(who);
    Item r = runProgram(*program, false);
    m_active.pop_back();
    --nameDepth;
    wrapRefs = savedWrap;
    ctx.shiftRows = savedRows;
    ctx.shiftCols = savedCols;
    return r;
}

Value Machine::run(const Program &p, const EvalContext &c)
{
    ctx = c;
    work = 0;
    over = false;
    status = EvalStatus::Ok;
    nameDepth = 0;
    m_active.clear();
    unsupported = 0;
    m_clockRead = false;
    peakCells = 0;
    textMade = 0;
    stack.clear();
    if (!ctx.source) return Value::error(ErrorKind::Value);
    // a date written as a month and a day alone is in the year of the clock this evaluation reads
    struct YearScope {
        explicit YearScope(int year) { setDefaultYear(year); }
        ~YearScope() { setDefaultYear(0); }
    };
    int clockYear = 0, clockMonth = 0, clockDay = 0;
    serialToDate(std::floor(now()), ctx.dateSystem, &clockYear, &clockMonth, &clockDay);
    const YearScope yearScope(clockYear);
    // the arrays alive at once are counted from here, and one more than the budget allows is refused (and stops the evaluation at its next step)
    struct BudgetScope {
        explicit BudgetScope(qint64 limit) { budget::beginEvaluation(limit); }
        ~BudgetScope() { budget::endEvaluation(); }
    };
    const BudgetScope budgetScope(kMaxLiveArrayCells);
    const Item r = runProgram(p, p.arrayFormula);
    peakCells = budget::peakCells();
    if (!over && budget::refused()) tooLarge();
    if (over) return Value::error(ErrorKind::Value);
    Value v;
    switch (r.kind) {
    case Item::Kind::Missing: v = Value::number(0); break;
    case Item::Kind::Value: v = r.value; break;
    case Item::Kind::Ref: v = scalar(r, p.arrayFormula); break;
    }
    if (v.isArray() && !p.arrayFormula) v = v.at(0, 0);
    if (v.isEmpty()) v = Value::number(0);
    if (v.isArray()) {
        // the empty cells of an array a formula makes show as 0
        for (int i = 0; i < v.rows(); ++i)
            for (int j = 0; j < v.cols(); ++j)
                if (v.at(i, j).isEmpty()) v.set(i, j, Value::number(0));
    }
    return v;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// the evaluator

struct Evaluator::Impl {
    Machine machine;
};

Evaluator::Evaluator() : d(new Impl) {}
Evaluator::~Evaluator() = default;

Value Evaluator::evaluate(const Program &program, const EvalContext &context) { return d->machine.run(program, context); }
EvalStatus Evaluator::status() const { return d->machine.status; }
int Evaluator::unsupportedKinds() const { return d->machine.unsupported; }
qint64 Evaluator::work() const { return d->machine.work; }
qint64 Evaluator::peakArrayCells() const { return d->machine.peakCells; }
qint64 Evaluator::madeTextChars() const { return d->machine.textMade; }
void Evaluator::setWorkLimit(qint64 steps) { d->machine.limit = steps; }
void Evaluator::seedRandom(quint64 seed) { d->machine.rng.seed(seed); }

} // namespace jp::calc

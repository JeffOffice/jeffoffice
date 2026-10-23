#include "sheet/formatcases.h"

#include "calc/numfmt.h"
#include "sheet/cellref.h"

#include <QHash>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

namespace {

// The most cases looked for in one workbook (8,920 formulas call TEXT in the 8,371 workbooks used for tuning, in 97 of them); past it the rest are not looked at.
constexpr qint64 kMaxCandidates = 1000000;

enum class OperandKind : quint8 { Literal, Cell, Name };

struct Operand {
    OperandKind kind = OperandKind::Literal;
    bool negate = false;
    double value = 0;          // a number in the formula, or (once found) the number of the cell
    QString sheet;             // the sheet named in front of the cell; empty for the formula's own
    QString name;              // a defined name
    quint32 row = 0, col = 0;
};

enum class Fn : quint8 { Text, Fixed, Dollar };

struct Call {
    Fn fn = Fn::Text;
    Operand x;
    QString code;
    int decimals = 2;
    bool noCommas = false;
};

enum class PieceKind : quint8 { Text, Call, Ref };

struct Piece {
    PieceKind kind = PieceKind::Text;
    QString text;              // the text of a string in the formula, or (once found) of the cell
    Call call;
    Operand ref;
    bool refIsNumber = false;  // the cell joined holds a number, which is General's text
    double refNumber = 0;
};

bool isIdentStart(QChar c) { return c.isLetter() || c == QLatin1Char('_'); }
bool isIdentChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.'); }

// Reads a formula that is text joined with & (and CONCATENATE) from strings in quotes, single cells and names, and calls of TEXT, FIXED, and DOLLAR on a number
// or a cell; or that, as DATEVALUE of such text, is a day number. Anything else in the formula makes it no case. A join with no call is read as well (its
// cells that hold numbers are written as General writes them, which is a case too if there is one).
class Scanner {
public:
    explicit Scanner(const QString &f) : s(f) {}

    bool parse(QVector<Piece> *out, bool *dateValue)
    {
        *dateValue = false;
        skip();
        int j = i;
        while (j < s.size() && isIdentChar(s[j])) ++j;
        if (s.mid(i, j - i).compare(QLatin1String("DATEVALUE"), Qt::CaseInsensitive) == 0) {
            int k = j;
            while (k < s.size() && s[k].isSpace()) ++k;
            if (k < s.size() && s[k] == QLatin1Char('(')) {
                i = k + 1;
                *dateValue = true;
                if (!concat(out, 0) || !eat(QLatin1Char(')'))) return false;
            }
        }
        if (!*dateValue && !concat(out, 0)) return false;
        skip();
        if (i != s.size()) return false;
        for (const Piece &p : *out)
            if (p.kind == PieceKind::Call || (p.kind == PieceKind::Ref && !*dateValue)) return true;
        return false;
    }

    // One cell, and the sheet it's in if the text says; nothing else (a defined name's text).
    static bool singleCell(const QString &text, Operand *x)
    {
        Scanner sc(text);
        sc.skip();
        if (!sc.cellRef(x) || x->kind != OperandKind::Cell) return false;
        sc.skip();
        return sc.i == sc.s.size();
    }

private:
    const QString &s;
    int i = 0;

    void skip()
    {
        while (i < s.size() && s[i].isSpace()) ++i;
    }
    bool eat(QChar c)
    {
        skip();
        if (i < s.size() && s[i] == c) {
            ++i;
            return true;
        }
        return false;
    }

    bool stringLiteral(QString *out)
    {
        skip();
        if (i >= s.size() || s[i] != QLatin1Char('"')) return false;
        ++i;
        QString r;
        for (;;) {
            if (i >= s.size()) return false;
            if (s[i] == QLatin1Char('"')) {
                if (i + 1 < s.size() && s[i + 1] == QLatin1Char('"')) {
                    r += QLatin1Char('"');
                    i += 2;
                    continue;
                }
                ++i;
                break;
            }
            r += s[i++];
        }
        *out = r;
        return true;
    }

    bool concat(QVector<Piece> *out, int depth)
    {
        if (depth > 8) return false;
        for (;;) {
            if (!item(out, depth)) return false;
            if (!eat(QLatin1Char('&'))) return true;
        }
    }

    bool item(QVector<Piece> *out, int depth)
    {
        skip();
        if (i >= s.size()) return false;
        if (s[i] == QLatin1Char('"')) {
            Piece p;
            if (!stringLiteral(&p.text)) return false;
            out->append(p);
            return true;
        }
        if (s[i] == QLatin1Char('(')) {
            ++i;
            if (!concat(out, depth + 1)) return false;
            return eat(QLatin1Char(')'));
        }
        if (s[i] != QLatin1Char('\'') && !isIdentStart(s[i])) return false;
        const int start = i;
        int j = i;
        while (j < s.size() && isIdentChar(s[j])) ++j;
        const QString name = s.mid(i, j - i).toUpper();
        int k = j;
        while (k < s.size() && s[k].isSpace()) ++k;
        if (s[i] == QLatin1Char('\'') || k >= s.size() || s[k] != QLatin1Char('(')) {
            // A cell, or a name.
            i = start;
            Piece p;
            p.kind = PieceKind::Ref;
            if (!cellRef(&p.ref) || p.ref.kind == OperandKind::Literal || !joinDelimiter()) return false;
            out->append(p);
            return true;
        }
        i = k + 1;
        if (name == QLatin1String("CONCATENATE") || name == QLatin1String("CONCAT") || name == QLatin1String("_XLFN.CONCAT")) {
            for (;;) {
                if (!concat(out, depth + 1)) return false;
                if (eat(QLatin1Char(','))) continue;
                return eat(QLatin1Char(')'));
            }
        }
        Piece p;
        p.kind = PieceKind::Call;
        if (name == QLatin1String("TEXT")) {
            p.call.fn = Fn::Text;
            if (!operand(&p.call.x) || !eat(QLatin1Char(',')) || !stringLiteral(&p.call.code)) return false;
        } else if (name == QLatin1String("FIXED") || name == QLatin1String("DOLLAR")) {
            p.call.fn = name == QLatin1String("FIXED") ? Fn::Fixed : Fn::Dollar;
            if (!operand(&p.call.x)) return false;
            if (eat(QLatin1Char(','))) {
                if (!integer(&p.call.decimals)) return false;
                if (p.call.fn == Fn::Fixed && eat(QLatin1Char(','))) {
                    if (!boolean(&p.call.noCommas)) return false;
                }
            }
        } else {
            return false;
        }
        if (!eat(QLatin1Char(')'))) return false;
        out->append(p);
        return true;
    }

    bool integer(int *out)
    {
        skip();
        int j = i;
        if (j < s.size() && (s[j] == QLatin1Char('-') || s[j] == QLatin1Char('+'))) ++j;
        const int digits = j;
        while (j < s.size() && s[j].isDigit()) ++j;
        if (j == digits || j - digits > 4) return false;
        *out = QStringView(s).mid(i, j - i).toInt();
        i = j;
        return argDelimiter();
    }

    bool boolean(bool *out)
    {
        skip();
        int j = i;
        while (j < s.size() && isIdentChar(s[j])) ++j;
        const QString word = s.mid(i, j - i).toUpper();
        if (word == QLatin1String("TRUE") || word == QLatin1String("1")) *out = true;
        else if (word == QLatin1String("FALSE") || word == QLatin1String("0")) *out = false;
        else return false;
        i = j;
        return argDelimiter();
    }

    // The argument ends here: a comma or the closing parenthesis comes next.
    bool argDelimiter()
    {
        skip();
        return i < s.size() && (s[i] == QLatin1Char(',') || s[i] == QLatin1Char(')'));
    }

    // The text joined to ends here: the next is &, a comma or closing parenthesis of CONCATENATE, or the end.
    bool joinDelimiter()
    {
        skip();
        return i >= s.size() || s[i] == QLatin1Char('&') || s[i] == QLatin1Char(',') || s[i] == QLatin1Char(')');
    }

    // A cell (with the sheet in front of it if it is another's), or a name.
    bool cellRef(Operand *x)
    {
        QString sheet;
        if (i < s.size() && s[i] == QLatin1Char('\'')) {
            ++i;
            for (;;) {
                if (i >= s.size()) return false;
                if (s[i] == QLatin1Char('\'')) {
                    if (i + 1 < s.size() && s[i + 1] == QLatin1Char('\'')) {
                        sheet += QLatin1Char('\'');
                        i += 2;
                        continue;
                    }
                    ++i;
                    break;
                }
                sheet += s[i++];
            }
            if (i >= s.size() || s[i] != QLatin1Char('!')) return false;
            ++i;
        } else {
            int j = i;
            while (j < s.size() && (isIdentChar(s[j]) || s[j] == QLatin1Char('$'))) ++j;
            if (j < s.size() && s[j] == QLatin1Char('!')) {
                sheet = s.mid(i, j - i);
                i = j + 1;
            }
        }
        int j = i;
        while (j < s.size() && (s[j].isLetterOrNumber() || s[j] == QLatin1Char('$') || s[j] == QLatin1Char('_') || s[j] == QLatin1Char('.'))) ++j;
        const QStringView token = QStringView(s).mid(i, j - i);
        quint32 row = 0, col = 0;
        if (parseCellRef(token, &row, &col)) {
            x->kind = OperandKind::Cell;
            x->sheet = sheet;
            x->row = row;
            x->col = col;
            i = j;
            return true;
        }
        // A name: not a cell, and not another sheet's.
        if (sheet.isEmpty() && !token.isEmpty() && isIdentStart(token[0]) && !token.contains(QLatin1Char('$'))) {
            x->kind = OperandKind::Name;
            x->name = token.toString();
            i = j;
            return true;
        }
        return false;
    }

    // A number, or one cell or name, and nothing more to the argument.
    bool operand(Operand *x)
    {
        skip();
        bool negate = false;
        if (i < s.size() && (s[i] == QLatin1Char('-') || s[i] == QLatin1Char('+'))) {
            negate = s[i] == QLatin1Char('-');
            ++i;
            skip();
        }
        if (i >= s.size()) return false;
        if (s[i].isDigit() || s[i] == QLatin1Char('.')) {
            int j = i;
            while (j < s.size() && (s[j].isDigit() || s[j] == QLatin1Char('.'))) ++j;
            if (j < s.size() && (s[j] == QLatin1Char('e') || s[j] == QLatin1Char('E'))) {
                int k = j + 1;
                if (k < s.size() && (s[k] == QLatin1Char('+') || s[k] == QLatin1Char('-'))) ++k;
                if (k < s.size() && s[k].isDigit()) {
                    while (k < s.size() && s[k].isDigit()) ++k;
                    j = k;
                }
            }
            bool ok = false;
            const double v = QStringView(s).mid(i, j - i).toDouble(&ok);
            if (!ok || !std::isfinite(v)) return false;
            x->kind = OperandKind::Literal;
            x->value = negate ? -v : v;
            i = j;
            return argDelimiter();
        }
        if (!cellRef(x)) return false;
        x->negate = negate;
        return argDelimiter();
    }
};

// The text a cell gives when it is joined to other text: a number as General writes it, and nothing for an empty cell. False for an error.
bool textOfCell(const Sheet &sheet, const SharedStrings &shared, const Cell *c, QString *out, bool *isNumber, double *number)
{
    out->clear();
    *isNumber = false;
    if (!c) return true;
    switch (c->type) {
    case CellType::Empty: return true;
    case CellType::Number:
    case CellType::Date: {
        const jp::calc::Formatted f = jp::calc::NumberFormat().format(c->number);
        if (f.failed) return false;
        *out = f.text;
        *isNumber = true;
        *number = c->number;
        return true;
    }
    case CellType::SharedString:
    case CellType::InlineString:
    case CellType::FormulaString: *out = sheet.text(*c, shared); return true;
    case CellType::Bool: *out = c->number != 0 ? QStringLiteral("TRUE") : QStringLiteral("FALSE"); return true;
    case CellType::Error: return false;
    }
    return false;
}

bool numberOf(const Cell *c, double *value)
{
    if (!c || (c->type != CellType::Number && c->type != CellType::Date)) return false;
    *value = c->number;
    return true;
}

} // namespace

struct FormatCaseFinder::Impl {
    const Workbook &book;
    struct Candidate {
        int sheet = 0;
        quint32 row = 0, col = 0;
        QString stored;
        bool dateValue = false;
        QVector<Piece> pieces;
        bool dropped = false;
    };
    QVector<Candidate> candidates;
    // The operands waiting for a sheet that isn't the formula's own: the candidate and the piece.
    struct Waiting {
        int candidate, piece;
    };
    QHash<int, QVector<Waiting>> waiting;

    explicit Impl(const Workbook &b) : book(b) {}

    int sheetIndex(const QString &name) const
    {
        const QVector<SheetEntry> &sheets = book.info().sheets;
        for (int k = 0; k < sheets.size(); ++k)
            if (sheets[k].kind == SheetKind::Worksheet && sheets[k].name.compare(name, Qt::CaseInsensitive) == 0) return k;
        return -1;
    }

    // The defined names by their names in lower case (built when the first is asked for).
    mutable QHash<QString, QVector<int>> nameIndex;
    mutable bool indexed = false;

    // A name for one cell, as that cell: a name of the sheet itself before one of the workbook.
    bool resolveName(Operand *x, int sheet) const
    {
        const QVector<DefinedName> &names = book.info().definedNames;
        if (!indexed) {
            indexed = true;
            for (int k = 0; k < names.size(); ++k) nameIndex[names[k].name.toLower()].append(k);
        }
        const DefinedName *found = nullptr;
        const QVector<int> same = nameIndex.value(x->name.toLower());
        // (A name once for the workbook and once for each sheet is as many as there can be.)
        for (int k = 0; k < same.size() && k <= kMaxSheets; ++k) {
            const DefinedName &n = names[same[k]];
            if (n.localSheet == sheet) {
                found = &n;
                break;
            }
            if (n.localSheet < 0 && !found) found = &n;
        }
        if (!found) return false;
        Operand cell;
        if (!Scanner::singleCell(found->ref, &cell)) return false;
        cell.negate = x->negate;
        *x = cell;
        return true;
    }

    // Takes the value a piece needs from a cell: the number of a call's, the text of a joined cell.
    static bool fetch(Piece *p, const Sheet &sheet, const SharedStrings &shared, const Cell *c)
    {
        if (p->kind == PieceKind::Call) {
            double v = 0;
            if (!numberOf(c, &v)) return false;
            p->call.x.value = p->call.x.negate ? -v : v;
            return true;
        }
        return textOfCell(sheet, shared, c, &p->text, &p->refIsNumber, &p->refNumber);
    }
};

FormatCaseFinder::FormatCaseFinder(const Workbook &book) : d(new Impl(book)) {}
FormatCaseFinder::~FormatCaseFinder() { delete d; }

void FormatCaseFinder::addSheet(int index, const Sheet &sheet)
{
    // Which formulas call one of the three, or join text.
    QVector<char> relevant(sheet.formulaTexts.size(), 0);
    for (int t = 0; t < sheet.formulaTexts.size(); ++t) {
        const QString &f = sheet.formulaTexts[t];
        relevant[t] = f.contains(QLatin1String("TEXT("), Qt::CaseInsensitive) || f.contains(QLatin1String("FIXED("), Qt::CaseInsensitive)
                      || f.contains(QLatin1String("DOLLAR("), Qt::CaseInsensitive) || f.contains(QLatin1Char('&')) || f.contains(QLatin1String("CONCAT"), Qt::CaseInsensitive);
    }
    const SharedStrings &shared = d->book.sharedStrings();
    sheet.cells.forEach([&](const Cell &c) {
        if (!c.hasFormula() || (c.type != CellType::FormulaString && c.type != CellType::Number) || c.formula < 0) return;
        const int text = sheet.formulas[c.formula].text;
        if (text < 0 || !relevant[text]) return;
        if (d->candidates.size() >= kMaxCandidates) return;
        Impl::Candidate cand;
        cand.sheet = index;
        cand.row = c.row;
        cand.col = c.col;
        const QString formula = sheet.formulaText(c);
        if (formula.size() > 8192) return;
        Scanner scanner(formula);
        if (!scanner.parse(&cand.pieces, &cand.dateValue)) return;
        // A text is stored for text, and a number for DATEVALUE of it.
        if (cand.dateValue != (c.type == CellType::Number)) return;
        cand.stored = cand.dateValue ? QString::number(c.number, 'g', 15) : sheet.text(c, shared);
        // The cells of this sheet are at hand; those of others are asked for later.
        const int at = int(d->candidates.size());
        for (int p = 0; p < cand.pieces.size() && !cand.dropped; ++p) {
            Piece &piece = cand.pieces[p];
            if (piece.kind == PieceKind::Text) continue;
            Operand &x = piece.kind == PieceKind::Call ? piece.call.x : piece.ref;
            if (x.kind == OperandKind::Literal) continue;
            if (x.kind == OperandKind::Name && !d->resolveName(&x, index)) {
                cand.dropped = true;
                break;
            }
            int target = index;
            if (!x.sheet.isEmpty()) target = d->sheetIndex(x.sheet);
            if (target < 0) {
                cand.dropped = true;
            } else if (target == index) {
                if (!Impl::fetch(&piece, sheet, shared, sheet.cells.find(x.row, x.col))) cand.dropped = true;
            } else {
                d->waiting[target].append({at, p});
            }
        }
        if (cand.dropped) cand.pieces.clear();   // (Waiting entries already made for it are skipped when they're reached.)
        d->candidates.append(cand);
    });
}

QVector<FormatCase> FormatCaseFinder::finish()
{
    // The other sheets, one at a time.
    for (auto it = d->waiting.cbegin(); it != d->waiting.cend(); ++it) {
        Sheet other;
        d->book.readSheet(it.key(), &other);
        for (const Impl::Waiting &w : it.value()) {
            Impl::Candidate &cand = d->candidates[w.candidate];
            if (cand.dropped || w.piece >= cand.pieces.size()) continue;
            Piece &piece = cand.pieces[w.piece];
            Operand &x = piece.kind == PieceKind::Call ? piece.call.x : piece.ref;
            if (!Impl::fetch(&piece, other, d->book.sharedStrings(), other.cells.find(x.row, x.col))) {
                cand.dropped = true;
                cand.pieces.clear();
            }
        }
    }
    const DateSystem system = d->book.info().dateSystem;
    QVector<FormatCase> cases;
    for (const Impl::Candidate &cand : d->candidates) {
        if (cand.dropped || cand.pieces.isEmpty()) continue;
        FormatCase fc;
        fc.sheet = d->book.info().sheets.value(cand.sheet).name;
        fc.cell = cellName(cand.row, cand.col);
        fc.stored = cand.stored;
        QString text;
        bool first = true;
        bool anyCall = false;
        for (const Piece &p : cand.pieces) anyCall = anyCall || p.kind == PieceKind::Call;
        // A join with no call is a case of General when it has a number in it.
        const Piece *firstNumber = nullptr;
        for (const Piece &p : cand.pieces)
            if (!firstNumber && p.kind == PieceKind::Ref && p.refIsNumber) firstNumber = &p;
        if (!anyCall) {
            if (!firstNumber) continue;
            fc.function = QStringLiteral("&");
            fc.value = firstNumber->refNumber;
            fc.code = QStringLiteral("General");
            first = false;
        }
        for (const Piece &p : cand.pieces) {
            if (p.kind != PieceKind::Call) {
                text += p.text;
                continue;
            }
            const Call &c = p.call;
            QString t;
            bool ok = false;
            switch (c.fn) {
            case Fn::Text: ok = jp::calc::formatTextFunction(c.x.value, c.code, system, &t); break;
            case Fn::Fixed: ok = jp::calc::formatFixed(c.x.value, c.decimals, c.noCommas, &t); break;
            case Fn::Dollar: ok = jp::calc::formatDollar(c.x.value, c.decimals, &t); break;
            }
            text += ok ? t : QStringLiteral("#VALUE!");
            if (first) {
                first = false;
                const QString fn = c.fn == Fn::Text ? QStringLiteral("TEXT") : c.fn == Fn::Fixed ? QStringLiteral("FIXED") : QStringLiteral("DOLLAR");
                fc.function = cand.dateValue ? QStringLiteral("DATEVALUE(") + fn + QLatin1Char(')') : fn;
                fc.value = c.x.value;
                fc.code = c.fn == Fn::Text ? c.code : c.fn == Fn::Fixed ? QString::number(c.decimals) + (c.noCommas ? QStringLiteral(",TRUE") : QString()) : QString::number(c.decimals);
            }
        }
        if (cand.dateValue) {
            double serial = 0;
            fc.ours = jp::calc::dateValueOf(text, system, &serial) ? QString::number(serial, 'g', 15) : QStringLiteral("#VALUE!");
        } else {
            fc.ours = text;
        }
        fc.match = fc.ours == fc.stored;
        cases.append(fc);
    }
    d->candidates.clear();
    d->waiting.clear();
    return cases;
}

} // namespace jp::office::sheet

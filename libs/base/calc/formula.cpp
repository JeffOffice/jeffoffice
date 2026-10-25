#include "calc/formula.h"

#include "calc/functions.h"

#include <QCoreApplication>
#include <QHash>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace jp::calc {

namespace {

constexpr int kMaxWordChars = 1000;

// (What is wrong with a formula is told to the person who typed it, in plain words, and QCoreApplication::translate gives it in the language of the program; the context is written out
// in each call so that lupdate finds the text.)
// ---- the pieces of the text

enum class T : quint8 { End, Number, Text, Bool, Error, Ref, Name, Func, Table, External, Op, LParen, RParen, LBrace, RBrace, Comma, Semi };

struct Token {
    T kind = T::End;
    bool space = false;      // white space came before it
    char op = 0;             // Op: + - * / ^ & = < > : % @ # and l (<=), g (>=), n (<>)
    int pos = 0;
    int end = 0;             // after its last character
    int body = -1;           // Ref: where the cells begin, after the sheet and "!" in front (-1: it has no sheet in front)
    double num = 0;
    QString text;            // Text: the text; Name, Func, Table, External: as written
    RefLit ref;              // Ref
    int sheetPlus1 = 0;      // Name: the sheet it is written with (a place in the sheet names, plus 1)
    ErrorKind error = ErrorKind::Null;
};

bool isDigit(QChar c) { return c.unicode() >= '0' && c.unicode() <= '9'; }
bool isAsciiLetter(QChar c) { return (c.unicode() >= 'a' && c.unicode() <= 'z') || (c.unicode() >= 'A' && c.unicode() <= 'Z'); }
bool isIdent(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char('\\'); }
bool isSpace(QChar c) { return c == QLatin1Char(' ') || c == QLatin1Char('\t') || c == QLatin1Char('\n') || c == QLatin1Char('\r'); }

const char *const kErrorLiterals[] = {"#NULL!", "#DIV/0!", "#VALUE!", "#REF!", "#NAME?", "#NUM!", "#N/A", "#GETTING_DATA", "#SPILL!", "#CALC!", "#FIELD!", "#BLOCKED!", "#CONNECT!", "#EXTERNAL!", "#UNKNOWN!"};

// The name without the prefixes a newer function carries in a file; `prefixed` says whether it had _xlfn. in front.
QString stripFunctionPrefix(QString name, bool *prefixed = nullptr)
{
    if (prefixed) *prefixed = false;
    for (bool stripped = true; stripped;) {
        stripped = false;
        for (const char *prefix : {"_xlfn.", "_xlws."})
            if (name.startsWith(QLatin1String(prefix), Qt::CaseInsensitive)) {
                if (prefixed && prefix[3] == 'f') *prefixed = true;
                name.remove(0, int(strlen(prefix)));
                stripped = true;
            }
    }
    return name;
}

// ---- the parser

class Parser {
public:
    Parser(const QString &text, const ParseOptions &options, Program *program) : s(text), n(int(text.size())), opt(options), prog(*program), out(&prog.code) {}

    bool run()
    {
        if (n > kMaxFormulaChars) return fail(0, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
        if (!lex()) return false;
        if (!expression()) return false;
        if (opt.definedName) {
            while (peek().kind == T::Comma) {
                advance();
                if (!expression()) return false;
                emit(Op::Union);
            }
        }
        if (peek().kind != T::End) return fail(peek().pos, leftOver(peek()));
        // the last operation of the formula, if it is a sum or a difference that was not in parentheses
        if (!prog.code.empty() && lastGroupEnd != int(prog.code.size())) {
            Instr &last = prog.code.back();
            if (last.op == Op::Add || last.op == Op::Sub) last.flags |= kInstrFinal;
        }
        if (prog.code.size() > size_t(kMaxFormulaInstructions)) return fail(n, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
        return true;
    }

    // The references of the text, where they are in it (see findReferences); false if the text can't be read as a formula.
    bool references(std::vector<ReferenceAt> *out, bool withNames)
    {
        if (n > kMaxFormulaChars || !lex()) return false;
        for (const Token &t : toks) {
            const bool name = withNames && t.kind == T::Name && t.sheetPlus1 > 0 && t.body >= 0;
            if (t.kind != T::Ref && !name) continue;
            ReferenceAt at;
            at.start = t.pos;
            at.end = t.end;
            at.body = t.body >= 0 ? t.body : t.pos;
            at.afterWorkbook = t.pos > 0 && s[t.pos - 1] == QLatin1Char(']');
            if (name) {
                at.isName = true;
                if (size_t(t.sheetPlus1 - 1) < size_t(prog.sheetNames.size())) at.sheet1 = prog.sheetNames[t.sheetPlus1 - 1];
                out->push_back(std::move(at));
                continue;
            }
            at.ref = t.ref;
            if (t.ref.sheet1 >= 0 && size_t(t.ref.sheet1) < size_t(prog.sheetNames.size())) at.sheet1 = prog.sheetNames[t.ref.sheet1];
            if (t.ref.sheet2 >= 0 && size_t(t.ref.sheet2) < size_t(prog.sheetNames.size())) at.sheet2 = prog.sheetNames[t.ref.sheet2];
            out->push_back(std::move(at));
        }
        return true;
    }

    QString error;
    int errorPos = 0;

private:
    const QString &s;
    const int n;
    const ParseOptions &opt;
    Program &prog;
    std::vector<Instr> *out;
    std::vector<Token> toks;
    size_t p = 0;
    int depth = 0;
    int arrayDepth = 0;
    int lastGroupEnd = -1;   // the size of the code when the last parenthesis closed

    bool fail(int pos, const QString &why)
    {
        if (error.isEmpty()) {
            error = why;
            errorPos = std::max(0, std::min(pos, n));
        }
        return false;
    }

    // ---- lexing

    int internSheet(const QString &name)
    {
        for (int i = 0; i < prog.sheetNames.size(); ++i)
            if (prog.sheetNames[i].compare(name, Qt::CaseInsensitive) == 0) return i;
        prog.sheetNames.append(name);
        return int(prog.sheetNames.size()) - 1;
    }

    // $?LETTERS$?DIGITS at i, a cell of the grid, with no word going on from it. `end` is where it ends.
    bool cellAt(int i, int *end, quint32 *row, quint32 *col, bool *rowAbs, bool *colAbs) const
    {
        int j = i;
        bool ca = false, ra = false;
        if (j < n && s[j] == QLatin1Char('$')) {
            ca = true;
            ++j;
        }
        const int l0 = j;
        quint32 c = 0;
        while (j < n && isAsciiLetter(s[j]) && j - l0 < 4) {
            c = c * 26 + quint32(s[j].toUpper().unicode() - 'A' + 1);
            ++j;
        }
        if (j == l0 || j - l0 > 3 || c > kGridCols) return false;
        if (j < n && s[j] == QLatin1Char('$')) {
            ra = true;
            ++j;
        }
        const int d0 = j;
        quint32 r = 0;
        while (j < n && isDigit(s[j]) && j - d0 < 8) {
            r = r * 10 + quint32(s[j].unicode() - '0');
            ++j;
        }
        if (j == d0 || r < 1 || r > kGridRows) return false;
        if (j < n && (isIdent(s[j]) || s[j] == QLatin1Char('(') || s[j] == QLatin1Char('[') || s[j] == QLatin1Char('!') || s[j] == QLatin1Char('$'))) return false;
        *end = j;
        *row = r - 1;
        *col = c - 1;
        *rowAbs = ra;
        *colAbs = ca;
        return true;
    }

    // $?LETTERS at i that name a column of the grid, with no word going on from it.
    bool columnAt(int i, int *end, quint32 *col, bool *abs) const
    {
        int j = i;
        bool a = false;
        if (j < n && s[j] == QLatin1Char('$')) {
            a = true;
            ++j;
        }
        const int l0 = j;
        quint32 c = 0;
        while (j < n && isAsciiLetter(s[j]) && j - l0 < 4) {
            c = c * 26 + quint32(s[j].toUpper().unicode() - 'A' + 1);
            ++j;
        }
        if (j == l0 || j - l0 > 3 || c > kGridCols) return false;
        if (j < n && (isIdent(s[j]) || s[j] == QLatin1Char('(') || s[j] == QLatin1Char('[') || s[j] == QLatin1Char('!') || s[j] == QLatin1Char('$'))) return false;
        *end = j;
        *col = c - 1;
        *abs = a;
        return true;
    }

    // $?DIGITS at i that are a row of the grid.
    bool rowAt(int i, int *end, quint32 *row, bool *abs) const
    {
        int j = i;
        bool a = false;
        if (j < n && s[j] == QLatin1Char('$')) {
            a = true;
            ++j;
        }
        const int d0 = j;
        quint32 r = 0;
        while (j < n && isDigit(s[j]) && j - d0 < 8) {
            r = r * 10 + quint32(s[j].unicode() - '0');
            ++j;
        }
        if (j == d0 || r < 1 || r > kGridRows) return false;
        if (j < n && (isIdent(s[j]) || s[j] == QLatin1Char('(') || s[j] == QLatin1Char('['))) return false;
        *end = j;
        *row = r - 1;
        *abs = a;
        return true;
    }

    // A reference of cells at i, as A1 writes one: a cell, an area, whole columns, or whole rows. Fills `ref` (the sheets are the caller's).
    bool cellsAt(int i, int *end, RefLit *ref) const
    {
        quint32 r1, c1;
        bool ra1, ca1;
        int e1;
        if (cellAt(i, &e1, &r1, &c1, &ra1, &ca1)) {
            quint32 r2 = r1, c2 = c1;
            bool ra2 = ra1, ca2 = ca1;
            bool area = false;
            int e2 = e1;
            if (e1 < n && s[e1] == QLatin1Char(':') && cellAt(e1 + 1, &e2, &r2, &c2, &ra2, &ca2)) {
                area = true;
            } else {
                e2 = e1;
            }
            fillArea(ref, r1, c1, ra1, ca1, r2, c2, ra2, ca2);
            if (area) ref->flags |= kRefArea;
            *end = e2;
            return true;
        }
        bool a1, a2;
        int e;
        quint32 cc2 = 0, rr2 = 0;
        if (columnAt(i, &e1, &c1, &a1) && e1 < n && s[e1] == QLatin1Char(':') && columnAt(e1 + 1, &e, &cc2, &a2)) {
            fillArea(ref, 0, c1, true, a1, kGridRows - 1, cc2, true, a2);
            ref->flags |= kRefWholeCols;
            *end = e;
            return true;
        }
        if (rowAt(i, &e1, &r1, &a1) && e1 < n && s[e1] == QLatin1Char(':') && rowAt(e1 + 1, &e, &rr2, &a2)) {
            fillArea(ref, r1, 0, a1, true, rr2, kGridCols - 1, a2, true);
            ref->flags |= kRefWholeRows;
            *end = e;
            return true;
        }
        return false;
    }

    static void fillArea(RefLit *ref, quint32 r1, quint32 c1, bool ra1, bool ca1, quint32 r2, quint32 c2, bool ra2, bool ca2)
    {
        if (r1 > r2) {
            std::swap(r1, r2);
            std::swap(ra1, ra2);
        }
        if (c1 > c2) {
            std::swap(c1, c2);
            std::swap(ca1, ca2);
        }
        ref->row1 = r1;
        ref->col1 = c1;
        ref->row2 = r2;
        ref->col2 = c2;
        ref->flags = quint8((ra1 ? kRefRow1Abs : 0) | (ca1 ? kRefCol1Abs : 0) | (ra2 ? kRefRow2Abs : 0) | (ca2 ? kRefCol2Abs : 0));
    }

    // R1C1 form at i: R, C, RC, R[1]C[-2], R2C3, with a row, a column, or both. `end` is where it ends.
    struct R1C1Part {
        bool has = false, abs = false;
        qint64 value = 0;   // 1-based when absolute, an offset when not
    };
    bool r1c1PartAt(int *i, QChar letter, R1C1Part *part) const
    {
        int j = *i;
        *part = R1C1Part();
        if (j >= n || s[j].toUpper() != letter) return false;
        ++j;
        part->has = true;
        if (j < n && s[j] == QLatin1Char('[')) {
            ++j;
            bool neg = false;
            if (j < n && (s[j] == QLatin1Char('-') || s[j] == QLatin1Char('+'))) neg = s[j++] == QLatin1Char('-');
            const int d0 = j;
            qint64 v = 0;
            while (j < n && isDigit(s[j]) && j - d0 < 8) v = v * 10 + (s[j++].unicode() - '0');
            if (j == d0 || j >= n || s[j] != QLatin1Char(']')) {
                *part = R1C1Part();
                return false;
            }
            ++j;
            part->abs = false;
            part->value = neg ? -v : v;
        } else {
            const int d0 = j;
            qint64 v = 0;
            while (j < n && isDigit(s[j]) && j - d0 < 8) v = v * 10 + (s[j++].unicode() - '0');
            if (j == d0) {
                part->abs = false;
                part->value = 0;
            } else {
                part->abs = true;
                part->value = v;
            }
        }
        *i = j;
        return true;
    }
    bool r1c1At(int i, int *end, R1C1Part *row, R1C1Part *col) const
    {
        int j = i;
        *row = R1C1Part();
        *col = R1C1Part();
        const bool hasRow = r1c1PartAt(&j, QLatin1Char('R'), row);
        const bool hasCol = r1c1PartAt(&j, QLatin1Char('C'), col);
        if (!hasRow && !hasCol) return false;
        if (j < n && (isIdent(s[j]) || s[j] == QLatin1Char('(') || s[j] == QLatin1Char('[') || s[j] == QLatin1Char('!'))) return false;
        *end = j;
        return true;
    }
    // The reference at i in R1C1 form, as the A1 reference it comes to for the cell the text is written for.
    bool r1c1RefAt(int i, int *end, RefLit *ref)
    {
        R1C1Part r1, c1, r2, c2;
        int e1;
        if (!r1c1At(i, &e1, &r1, &c1)) return false;
        int e2 = e1;
        bool area = false;
        if (e1 < n && s[e1] == QLatin1Char(':') && r1c1At(e1 + 1, &e2, &r2, &c2)) {
            area = true;
        } else {
            r2 = r1;
            c2 = c1;
            e2 = e1;
        }
        auto resolve = [&](const R1C1Part &part, quint32 origin, bool isRow, quint32 *v, bool *abs) {
            if (!part.has) return true;
            qint64 x = part.abs ? part.value - 1 : qint64(origin) + part.value;
            if (x < 0 || x >= qint64(isRow ? kGridRows : kGridCols)) return false;
            *v = quint32(x);
            *abs = part.abs;
            return true;
        };
        quint32 ra = 0, ca = 0, rb = 0, cb = 0;
        bool raa = true, caa = true, rba = true, cba = true;
        if (!resolve(r1, opt.row, true, &ra, &raa) || !resolve(c1, opt.col, false, &ca, &caa) || !resolve(r2, opt.row, true, &rb, &rba) || !resolve(c2, opt.col, false, &cb, &cba)) {
            fail(i, QCoreApplication::translate("jp::calc::Formula", "That reference is outside the sheet. The last column is XFD, and the last row is 1,048,576."));
            return false;
        }
        const bool wholeRows = !c1.has && !c2.has, wholeCols = !r1.has && !r2.has;
        if (wholeRows) {
            ca = 0;
            cb = kGridCols - 1;
            caa = cba = true;
        }
        if (wholeCols) {
            ra = 0;
            rb = kGridRows - 1;
            raa = rba = true;
        }
        fillArea(ref, ra, ca, raa, caa, rb, cb, rba, cba);
        if (wholeRows) ref->flags |= kRefWholeRows;
        if (wholeCols) ref->flags |= kRefWholeCols;
        if (area && !wholeRows && !wholeCols) ref->flags |= kRefArea;
        *end = e2;
        return true;
    }

    // The end of the text in quotes that starts at i; -1 if it never closes. `text` gets what it says (a doubled quote is one).
    int quotedEnd(int i, QString *text) const
    {
        const QChar q = s[i];
        QString t;
        for (int j = i + 1; j < n; ++j) {
            if (s[j] == q) {
                if (j + 1 < n && s[j + 1] == q) {
                    t += q;
                    ++j;
                    continue;
                }
                if (text) *text = t;
                return j + 1;
            }
            t += s[j];
        }
        return -1;
    }

    // The end of the square brackets that open at i, which may hold more of them; an apostrophe makes the next character plain. -1 if they never close.
    int bracketsEnd(int i) const
    {
        int d = 0;
        for (; i < n; ++i) {
            if (s[i] == QLatin1Char('\'') && i + 1 < n) {
                ++i;
            } else if (s[i] == QLatin1Char('[')) {
                ++d;
            } else if (s[i] == QLatin1Char(']') && --d == 0) {
                return i + 1;
            }
        }
        return -1;
    }

    int wordEnd(int i) const
    {
        int j = i;
        while (j < n && isIdent(s[j])) ++j;
        return j;
    }

    static ErrorKind errorAt(const QString &s, int i, int *end, bool *ok)
    {
        *ok = false;
        int bestLen = 0;
        ErrorKind best = ErrorKind::Null;
        for (int k = 0; k < int(sizeof(kErrorLiterals) / sizeof(kErrorLiterals[0])); ++k) {
            const int len = int(strlen(kErrorLiterals[k]));
            if (i + len <= s.size() && QStringView(s).mid(i, len).compare(QLatin1String(kErrorLiterals[k]), Qt::CaseInsensitive) == 0 && len > bestLen) {
                bestLen = len;
                best = ErrorKind(k);
                *ok = true;
            }
        }
        *end = i + bestLen;
        return best;
    }

    // What follows a sheet prefix at i: a reference, an error, or a name (with the sheet scope).
    bool lexBody(int i, Token *t, int *end, int sheet1, int sheet2)
    {
        if (i >= n) return false;
        if (s[i] == QLatin1Char('#')) {
            bool ok;
            const ErrorKind e = errorAt(s, i, end, &ok);
            if (!ok) return false;
            t->kind = T::Error;
            t->error = e;
            return true;
        }
        RefLit ref;
        int e;
        if (opt.r1c1 ? r1c1RefAt(i, &e, &ref) : cellsAt(i, &e, &ref)) {
            ref.sheet1 = sheet1;
            ref.sheet2 = sheet2;
            t->kind = T::Ref;
            t->ref = ref;
            *end = e;
            return true;
        }
        if (!error.isEmpty()) return false;
        if (s[i] == QLatin1Char('\'') || s[i] == QLatin1Char('[') || !(isIdent(s[i]) )) return false;
        const int w = wordEnd(i);
        if (w - i > kMaxWordChars) return fail(i, QCoreApplication::translate("jp::calc::Formula", "A name in the formula is too long.")), false;
        if (w < n && s[w] == QLatin1Char('[')) {
            const int close = bracketsEnd(w);
            if (close < 0) return fail(w, QCoreApplication::translate("jp::calc::Formula", "A [ is never closed. Add a ] after it.")), false;
            t->kind = T::Table;
            *end = close;
            return true;
        }
        t->kind = T::Name;
        t->text = s.mid(i, w - i);
        t->sheetPlus1 = sheet1 + 1;
        *end = w;
        return true;
    }

    bool lex()
    {
        int i = 0;
        bool space = false;
        auto push = [&](Token t, int start, int end) {
            t.pos = start;
            t.end = end;
            t.space = space;
            space = false;
            toks.push_back(std::move(t));
            i = end;
            return int(toks.size()) <= kMaxFormulaInstructions;
        };
        auto afterOperand = [&]() {
            if (toks.empty() || space) return false;
            const T k = toks.back().kind;
            return k == T::Ref || k == T::Name || k == T::RParen || k == T::Table;
        };
        while (i < n) {
            const QChar c = s[i];
            if (isSpace(c)) {
                space = true;
                ++i;
                continue;
            }
            Token t;
            if (c == QLatin1Char('"')) {
                QString text;
                const int e = quotedEnd(i, &text);
                if (e < 0) return fail(i, QCoreApplication::translate("jp::calc::Formula", "A text begins with a quote (\") that is never closed. Add the closing quote."));
                t.kind = T::Text;
                t.text = text;
                if (!push(t, i, e)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                continue;
            }
            if (c == QLatin1Char('\'')) {
                QString name;
                const int e = quotedEnd(i, &name);
                if (e < 0) return fail(i, QCoreApplication::translate("jp::calc::Formula", "A sheet's name begins with an apostrophe (') that is never closed. Add the closing apostrophe."));
                if (e >= n || s[e] != QLatin1Char('!')) return fail(i, QCoreApplication::translate("jp::calc::Formula", "A sheet's name in apostrophes must be followed by an exclamation mark and a cell, as in 'My Sheet'!A1."));
                if (name.contains(QLatin1Char('['))) {
                    int end;
                    Token body;
                    if (!lexBody(e + 1, &body, &end, -1, -1)) return fail(e + 1, QCoreApplication::translate("jp::calc::Formula", "A sheet's name must be followed by a cell or a range, as in Sheet1!A1."));
                    t.kind = T::External;
                    t.text = s.mid(i, end - i);
                    if (!push(t, i, end)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                    continue;
                }
                int s1 = -1, s2 = -1;
                const int colon = name.indexOf(QLatin1Char(':'));
                if (colon > 0 && colon < name.size() - 1) {
                    s1 = internSheet(name.left(colon));
                    s2 = internSheet(name.mid(colon + 1));
                } else {
                    s1 = internSheet(name);
                }
                int end;
                t.body = e + 1;
                if (!lexBody(e + 1, &t, &end, s1, s2)) return error.isEmpty() ? fail(e + 1, QCoreApplication::translate("jp::calc::Formula", "A sheet's name must be followed by a cell or a range, as in Sheet1!A1.")) : false;
                if (!push(t, i, end)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                continue;
            }
            if (c == QLatin1Char('[')) {
                const int close = bracketsEnd(i);
                if (close < 0) return fail(i, QCoreApplication::translate("jp::calc::Formula", "A [ is never closed. Add a ] after it."));
                // [1]Sheet1!A1 or [1]!Name: another workbook; else the columns of a table with no name before them ([@Col])
                bool digits = close - i > 2;
                for (int k = i + 1; k < close - 1; ++k)
                    if (!isDigit(s[k])) digits = false;
                if (digits && close < n && (isIdent(s[close]) || s[close] == QLatin1Char('!'))) {
                    int w = close;
                    if (s[w] != QLatin1Char('!')) w = wordEnd(w);
                    if (w < n && s[w] == QLatin1Char('!')) {
                        // [1]!'a name in quotes', and [0]!Function(...) of an add-in or a macro of another workbook
                        if (w + 1 < n && s[w + 1] == QLatin1Char('\'')) {
                            const int e = quotedEnd(w + 1, nullptr);
                            if (e < 0) return fail(w + 1, QCoreApplication::translate("jp::calc::Formula", "A sheet's name begins with an apostrophe (') that is never closed. Add the closing apostrophe."));
                            t.kind = T::External;
                            t.text = s.mid(i, e - i);
                            if (!push(t, i, e)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                            continue;
                        }
                        const int fe = wordEnd(w + 1);
                        if (fe > w + 1 && fe < n && s[fe] == QLatin1Char('(')) {
                            t.kind = T::Func;
                            t.text = s.mid(i, fe - i);
                            if (!push(t, i, fe)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                            continue;
                        }
                        int end;
                        Token body;
                        if (!lexBody(w + 1, &body, &end, -1, -1)) return error.isEmpty() ? fail(w + 1, QCoreApplication::translate("jp::calc::Formula", "A sheet's name must be followed by a cell or a range, as in Sheet1!A1.")) : false;
                        t.kind = T::External;
                        t.text = s.mid(i, end - i);
                        if (!push(t, i, end)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                        continue;
                    }
                }
                t.kind = T::Table;
                t.text = s.mid(i, close - i);
                if (!push(t, i, close)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                continue;
            }
            if (c == QLatin1Char('#')) {
                bool ok;
                int end;
                const ErrorKind e = errorAt(s, i, &end, &ok);
                if (ok) {
                    t.kind = T::Error;
                    t.error = e;
                    if (!push(t, i, end)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                    continue;
                }
                if (afterOperand()) {
                    t.kind = T::Op;
                    t.op = '#';
                    push(t, i, i + 1);
                    continue;
                }
                return fail(i, QCoreApplication::translate("jp::calc::Formula", "A # starts an error such as #N/A. This one is not an error."));
            }
            if (isDigit(c) || (c == QLatin1Char('.') && i + 1 < n && isDigit(s[i + 1])) || (c == QLatin1Char('$') && i + 1 < n && isDigit(s[i + 1]))) {
                RefLit ref;
                int end;
                if (!opt.r1c1 && cellsAt(i, &end, &ref)) {
                    t.kind = T::Ref;
                    t.ref = ref;
                    if (!push(t, i, end)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                    continue;
                }
                if (c == QLatin1Char('$')) return fail(i, QCoreApplication::translate("jp::calc::Formula", "A $ belongs in a cell reference, as in $A$1."));
                int j = i;
                while (j < n && isDigit(s[j])) ++j;
                if (j < n && s[j] == QLatin1Char('.')) {
                    ++j;
                    while (j < n && isDigit(s[j])) ++j;
                }
                if (j < n && (s[j] == QLatin1Char('E') || s[j] == QLatin1Char('e'))) {
                    int k = j + 1;
                    if (k < n && (s[k] == QLatin1Char('+') || s[k] == QLatin1Char('-'))) ++k;
                    if (k < n && isDigit(s[k])) {
                        while (k < n && isDigit(s[k])) ++k;
                        j = k;
                    }
                }
                if (j < n && (isIdent(s[j]) || s[j] == QLatin1Char('$'))) return fail(i, QCoreApplication::translate("jp::calc::Formula", "A number can't run into letters. Put a mark between them, such as an operator."));
                bool ok = false;
                const double v = QStringView(s).mid(i, j - i).toDouble(&ok);
                if (!ok || !std::isfinite(v)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "This number is not valid."));
                t.kind = T::Number;
                t.num = v;
                if (!push(t, i, j)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                continue;
            }
            if (c == QLatin1Char('$') || c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('\\')) {
                // a sheet's name before "!" (or two before it for sheets one through the other)
                if (c != QLatin1Char('$')) {
                    const int w = wordEnd(i);
                    if (w - i > kMaxWordChars) return fail(i, QCoreApplication::translate("jp::calc::Formula", "A name in the formula is too long."));
                    int bang = -1, s1 = -1, s2 = -1;
                    if (w < n && s[w] == QLatin1Char('!')) {
                        bang = w;
                        s1 = internSheet(s.mid(i, w - i));
                    } else if (w + 1 < n && s[w] == QLatin1Char(':') && isIdent(s[w + 1])) {
                        const int w2 = wordEnd(w + 1);
                        if (w2 < n && s[w2] == QLatin1Char('!')) {
                            bang = w2;
                            s1 = internSheet(s.mid(i, w - i));
                            s2 = internSheet(s.mid(w + 1, w2 - w - 1));
                        }
                    }
                    if (bang >= 0) {
                        int end;
                        t.body = bang + 1;
                        if (!lexBody(bang + 1, &t, &end, s1, s2)) return error.isEmpty() ? fail(bang + 1, QCoreApplication::translate("jp::calc::Formula", "A sheet's name must be followed by a cell or a range, as in Sheet1!A1.")) : false;
                        if (!push(t, i, end)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                        continue;
                    }
                }
                RefLit ref;
                int end;
                if (opt.r1c1 ? r1c1RefAt(i, &end, &ref) : cellsAt(i, &end, &ref)) {
                    t.kind = T::Ref;
                    t.ref = ref;
                    if (!push(t, i, end)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                    continue;
                }
                if (!error.isEmpty()) return false;
                if (c == QLatin1Char('$')) return fail(i, QCoreApplication::translate("jp::calc::Formula", "A $ belongs in a cell reference, as in $A$1."));
                const int w = wordEnd(i);
                if (w - i > kMaxWordChars) return fail(i, QCoreApplication::translate("jp::calc::Formula", "A name in the formula is too long."));
                const QString word = s.mid(i, w - i);
                if (w < n && s[w] == QLatin1Char('(')) {
                    t.kind = T::Func;
                    t.text = word;
                } else if (w < n && s[w] == QLatin1Char('[')) {
                    const int close = bracketsEnd(w);
                    if (close < 0) return fail(w, QCoreApplication::translate("jp::calc::Formula", "A [ is never closed. Add a ] after it."));
                    t.kind = T::Table;
                    t.text = s.mid(i, close - i);
                    if (!push(t, i, close)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                    continue;
                } else if (word.compare(QLatin1String("TRUE"), Qt::CaseInsensitive) == 0 || word.compare(QLatin1String("FALSE"), Qt::CaseInsensitive) == 0) {
                    t.kind = T::Bool;
                    t.num = word.size() == 4 ? 1 : 0;
                } else {
                    t.kind = T::Name;
                    t.text = word;
                }
                if (!push(t, i, w)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
                continue;
            }
            // operators and punctuation
            t.kind = T::Op;
            int len = 1;
            switch (c.unicode()) {
            case '+': case '-': case '*': case '/': case '^': case '&': case '=': case ':': case '%': case '@': t.op = char(c.unicode()); break;
            case '<':
                if (i + 1 < n && s[i + 1] == QLatin1Char('=')) { t.op = 'l'; len = 2; }
                else if (i + 1 < n && s[i + 1] == QLatin1Char('>')) { t.op = 'n'; len = 2; }
                else t.op = '<';
                break;
            case '>':
                if (i + 1 < n && s[i + 1] == QLatin1Char('=')) { t.op = 'g'; len = 2; }
                else t.op = '>';
                break;
            case '(': t.kind = T::LParen; break;
            case ')': t.kind = T::RParen; break;
            case '{': t.kind = T::LBrace; break;
            case '}': t.kind = T::RBrace; break;
            case ',': t.kind = T::Comma; break;
            case ';': t.kind = T::Semi; break;
            default: return fail(i, QCoreApplication::translate("jp::calc::Formula", "The character \"%1\" can't be in a formula.").arg(c));
            }
            if (!push(t, i, i + len)) return fail(i, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
        }
        Token end;
        end.kind = T::End;
        end.pos = n;
        end.space = space;
        toks.push_back(end);
        return true;
    }

    // ---- parsing

    const Token &peek(size_t ahead = 0) const { return toks[std::min(p + ahead, toks.size() - 1)]; }
    void advance()
    {
        if (p + 1 < toks.size()) ++p;
    }
    // What is wrong where a value or a reference is needed and `t` is what was found.
    static QString missingValue(const Token &t)
    {
        switch (t.kind) {
        case T::End: return QCoreApplication::translate("jp::calc::Formula", "The formula ends where a value is needed. Type the rest of it.");
        case T::RParen: return QCoreApplication::translate("jp::calc::Formula", "There's a ) where a value is needed. A value is missing before it.");
        case T::Comma: return QCoreApplication::translate("jp::calc::Formula", "There's a comma where a value is needed. A value is missing before it.");
        case T::Semi: return QCoreApplication::translate("jp::calc::Formula", "There's a semicolon where a value is needed. A value is missing before it.");
        case T::RBrace: return QCoreApplication::translate("jp::calc::Formula", "There's a } where a value is needed. A value is missing before it.");
        case T::Op: return QCoreApplication::translate("jp::calc::Formula", "An operator is where a value is needed. A value is missing before it.");
        default: return QCoreApplication::translate("jp::calc::Formula", "A value is needed here.");
        }
    }
    // What is wrong where a complete formula was read and `t` is what is left over.
    static QString leftOver(const Token &t)
    {
        switch (t.kind) {
        case T::RParen: return QCoreApplication::translate("jp::calc::Formula", "There's a ) without a matching (.");
        case T::RBrace: return QCoreApplication::translate("jp::calc::Formula", "There's a } without a matching {.");
        case T::Comma: return QCoreApplication::translate("jp::calc::Formula", "There's a comma outside the arguments of a function.");
        case T::Semi: return QCoreApplication::translate("jp::calc::Formula", "There's a semicolon outside a list between { and }.");
        default: return QCoreApplication::translate("jp::calc::Formula", "An operator is missing here, between two values: put one, such as + or &, before this.");
        }
    }

    bool emit(Op op, qint32 operand = 0, quint16 argc = 0, quint8 flags = 0)
    {
        const bool computes = (op >= Op::Add && op <= Op::Ge) || op == Op::Neg || op == Op::Percent || op == Op::Call;
        if (arrayMode() && computes) flags |= kInstrArray;
        if (out->size() >= size_t(kMaxFormulaInstructions)) return fail(toks[std::min(p, toks.size() - 1)].pos, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
        Instr in;
        in.op = op;
        in.flags = flags;
        in.argc = argc;
        in.operand = operand;
        out->push_back(in);
        return true;
    }

    bool arrayMode() const { return prog.arrayFormula || arrayDepth > 0; }

    bool enter()
    {
        if (++depth > kMaxFormulaDepth) return fail(peek().pos, QCoreApplication::translate("jp::calc::Formula", "The formula has too many brackets inside one another."));
        return true;
    }

    static int precedence(const Token &t)
    {
        if (t.kind != T::Op) return 0;
        switch (t.op) {
        case '=': case '<': case '>': case 'l': case 'g': case 'n': return 1;
        case '&': return 2;
        case '+': case '-': return 3;
        case '*': case '/': return 4;
        case '^': return 5;
        default: return 0;
        }
    }
    static Op binaryOp(char c)
    {
        switch (c) {
        case '=': return Op::Eq;
        case '<': return Op::Lt;
        case '>': return Op::Gt;
        case 'l': return Op::Le;
        case 'g': return Op::Ge;
        case 'n': return Op::Ne;
        case '&': return Op::Concat;
        case '+': return Op::Add;
        case '-': return Op::Sub;
        case '*': return Op::Mul;
        case '/': return Op::Div;
        default: return Op::Pow;
        }
    }

    bool expression() { return binary(1); }

    bool binary(int level)
    {
        if (!unary()) return false;
        for (;;) {
            const Token &t = peek();
            const int prec = precedence(t);
            if (prec == 0 || prec < level) return true;
            const char op = t.op;
            advance();
            if (!binary(prec + 1)) return false;
            if (!emit(binaryOp(op))) return false;
        }
    }

    bool unary()
    {
        // signs and "@" in front apply to what follows, the last one first
        std::vector<char> prefix;
        while (peek().kind == T::Op && (peek().op == '-' || peek().op == '+' || peek().op == '@')) {
            prefix.push_back(peek().op);
            advance();
            if (prefix.size() > size_t(kMaxFormulaInstructions)) return fail(peek().pos, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
        }
        if (!intersect()) return false;
        for (size_t k = prefix.size(); k-- > 0;) {
            if (prefix[k] == '-' && !emit(Op::Neg)) return false;
            if (prefix[k] == '@' && !emit(Op::Single)) return false;
        }
        while (peek().kind == T::Op && peek().op == '%') {
            advance();
            if (!emit(Op::Percent)) return false;
        }
        return true;
    }

    static bool startsOperand(const Token &t)
    {
        return t.kind == T::Ref || t.kind == T::Name || t.kind == T::Func || t.kind == T::Table || t.kind == T::External || t.kind == T::LParen;
    }

    bool intersect()
    {
        if (!range()) return false;
        while (peek().space && startsOperand(peek())) {
            if (!range()) return false;
            if (!emit(Op::Intersect)) return false;
        }
        return true;
    }

    bool range()
    {
        if (!primary()) return false;
        while (peek().kind == T::Op && peek().op == ':') {
            advance();
            if (!primary()) return false;
            if (!emit(Op::Range)) return false;
        }
        return true;
    }

    int addText(const QString &t)
    {
        prog.texts.push_back(t);
        return int(prog.texts.size()) - 1;
    }

    bool primary()
    {
        const Token t = peek();
        switch (t.kind) {
        case T::Number: {
            advance();
            prog.numbers.push_back(t.num);
            return emit(Op::PushNumber, int(prog.numbers.size()) - 1);
        }
        case T::Text:
            advance();
            return emit(Op::PushText, addText(t.text));
        case T::Bool:
            advance();
            return emit(Op::PushBool, t.num != 0 ? 1 : 0);
        case T::Error:
            advance();
            return emit(Op::PushError, int(t.error));
        case T::Ref:
            advance();
            prog.refs.push_back(t.ref);
            if (!emit(Op::PushRef, int(prog.refs.size()) - 1)) return false;
            return spill();
        case T::Name:
            advance();
            if (t.sheetPlus1 > 65534) return fail(t.pos, QCoreApplication::translate("jp::calc::Formula", "The formula names more sheets than JeffCrunch can read."));
            if (!emit(Op::PushName, addText(t.text), quint16(t.sheetPlus1))) return false;
            return spill();
        case T::Table:
            advance();
            prog.structured = true;
            if (!emit(Op::PushTable, addText(t.text))) return false;
            return spill();
        case T::External:
            advance();
            prog.external = true;
            if (!emit(Op::PushExternal, addText(t.text))) return false;
            return spill();
        case T::Func:
            if (!call()) return false;
            return spill();
        case T::LParen: {
            if (!enter()) return false;
            advance();
            if (!expression()) return false;
            while (peek().kind == T::Comma) {   // a union of references
                advance();
                if (!expression()) return false;
                if (!emit(Op::Union)) return false;
            }
            if (peek().kind != T::RParen) return fail(peek().pos, peek().kind == T::End ? QCoreApplication::translate("jp::calc::Formula", "A ( is never closed. Add a ) at the end.") : QCoreApplication::translate("jp::calc::Formula", "A ) is missing here: the ( before it is not closed."));
            advance();
            --depth;
            lastGroupEnd = out == &prog.code ? int(out->size()) : -1;
            return spill();
        }
        case T::LBrace:
            return arrayConstant();
        default:
            return fail(t.pos, missingValue(t));
        }
    }

    // A reference followed by "#" stands for the cells a formula there spills into.
    bool spill()
    {
        if (peek().kind == T::Op && peek().op == '#' && !peek().space) {
            advance();
            prog.structured = true;
            return emit(Op::Spill);
        }
        return true;
    }

    bool arrayConstant()
    {
        if (!enter()) return false;
        advance();   // {
        std::vector<std::vector<Value>> rows(1);
        for (;;) {
            // one element
            bool negative = false;
            if (peek().kind == T::Op && (peek().op == '-' || peek().op == '+')) {
                negative = peek().op == '-';
                advance();
                if (peek().kind != T::Number) return fail(peek().pos, QCoreApplication::translate("jp::calc::Formula", "Between { and } there can only be numbers, texts, TRUE, FALSE, and errors."));
            }
            const Token t = peek();
            Value v;
            switch (t.kind) {
            case T::Number: v = Value::number(negative ? -t.num : t.num); break;
            case T::Text: v = Value::text(t.text); break;
            case T::Bool: v = Value::boolean(t.num != 0); break;
            case T::Error: v = Value::error(t.error); break;
            default: return fail(t.pos, QCoreApplication::translate("jp::calc::Formula", "Between { and } there can only be numbers, texts, TRUE, FALSE, and errors."));
            }
            advance();
            rows.back().push_back(v);
            if (qint64(rows.size()) * qint64(rows.front().size()) > kMaxArrayConstantCells) return fail(t.pos, QCoreApplication::translate("jp::calc::Formula", "The list between { and } is too large."));
            if (peek().kind == T::Comma) {
                advance();
            } else if (peek().kind == T::Semi) {
                advance();
                if (rows.back().size() != rows.front().size()) return fail(peek().pos, QCoreApplication::translate("jp::calc::Formula", "Each row between { and } must have the same number of values."));
                rows.emplace_back();
            } else if (peek().kind == T::RBrace) {
                advance();
                break;
            } else {
                return fail(peek().pos, QCoreApplication::translate("jp::calc::Formula", "A comma, a semicolon, or a } is missing in the list between { and }."));
            }
        }
        if (rows.back().size() != rows.front().size()) return fail(peek().pos, QCoreApplication::translate("jp::calc::Formula", "Each row between { and } must have the same number of values."));
        --depth;
        Value a = Value::array(int(rows.size()), int(rows.front().size()));
        for (size_t r = 0; r < rows.size(); ++r)
            for (size_t c = 0; c < rows[r].size(); ++c) a.set(int(r), int(c), rows[r][c]);
        prog.arrays.push_back(a);
        return emit(Op::PushArray, int(prog.arrays.size()) - 1);
    }

    // An argument of a call: the code for it, or nothing when it is left out.
    bool argument(char cls, bool *missing)
    {
        const bool array = cls == 'a';
        if (array) ++arrayDepth;
        *missing = peek().kind == T::Comma || peek().kind == T::RParen;
        bool ok = true;
        if (!*missing) ok = expression();
        if (array) --arrayDepth;
        return ok;
    }

    bool call()
    {
        const Token t = peek();
        advance();   // the name
        advance();   // "("
        if (!enter()) return false;
        bool prefixed = false;
        QString name = stripFunctionPrefix(t.text, &prefixed).toUpper();
        int fi = functionIndex(name);
        // a function that came later is written with its prefix; without it, the name is some other function's
        if (fi >= 0 && (functionAt(fi).flags & kFnFuture) && !prefixed) fi = -1;
        const FunctionInfo *fn = fi >= 0 ? &functionAt(fi) : nullptr;
        if (name == QLatin1String("SINGLE") && !fn) {
            // the form in which a newer file writes "@"
            bool missing;
            if (peek().kind == T::RParen) return fail(t.pos, QCoreApplication::translate("jp::calc::Formula", "SINGLE needs a cell or a range in parentheses."));
            if (!argument('r', &missing) || missing) return missing ? fail(peek().pos, QCoreApplication::translate("jp::calc::Formula", "SINGLE needs a cell or a range in parentheses.")) : false;
            if (peek().kind != T::RParen) return fail(peek().pos, QCoreApplication::translate("jp::calc::Formula", "A ) is missing after the argument."));
            advance();
            --depth;
            return emit(Op::Single);
        }
        const bool lazy = fn && (fn->flags & kFnLazy) && !arrayMode();
        std::vector<std::vector<Instr>> parts;   // the code of each argument, when the function works out only some of them
        std::vector<bool> missingArgs;
        int count = 0;
        if (peek().kind != T::RParen) {
            for (;;) {
                if (count >= kMaxFormulaArgs) return fail(peek().pos, QCoreApplication::translate("jp::calc::Formula", "This function has more arguments than JeffCrunch can read."));
                const char cls = fn ? paramClass(*fn, count) : 'r';
                bool missing = false;
                if (lazy) {
                    std::vector<Instr> part;
                    std::vector<Instr> *saved = out;
                    out = &part;
                    const bool ok = argument(cls, &missing);
                    out = saved;
                    if (!ok) return false;
                    if (missing) {
                        Instr zero;
                        prog.numbers.push_back(0);
                        zero.op = Op::PushNumber;
                        zero.operand = int(prog.numbers.size()) - 1;
                        part.push_back(zero);
                    }
                    parts.push_back(std::move(part));
                } else {
                    if (!argument(cls, &missing)) return false;
                    if (missing && !emit(Op::PushMissing)) return false;
                }
                ++count;
                if (peek().kind == T::Comma) {
                    advance();
                    if (peek().kind == T::RParen) {   // a last argument left out
                        if (lazy) {
                            std::vector<Instr> part;
                            Instr zero;
                            prog.numbers.push_back(0);
                            zero.op = Op::PushNumber;
                            zero.operand = int(prog.numbers.size()) - 1;
                            part.push_back(zero);
                            parts.push_back(std::move(part));
                        } else if (!emit(Op::PushMissing)) {
                            return false;
                        }
                        ++count;
                        break;
                    }
                    continue;
                }
                break;
            }
        }
        if (peek().kind != T::RParen) return fail(peek().pos, peek().kind == T::End ? QCoreApplication::translate("jp::calc::Formula", "A ( is never closed. Add a ) at the end.") : QCoreApplication::translate("jp::calc::Formula", "A comma or a ) is missing here, after the argument."));
        advance();
        --depth;
        if (!fn) {
            if (!prog.unknownFunctions.contains(name)) prog.unknownFunctions.append(name);
            return emit(Op::Call, -(addText(name) + 1), quint16(count));
        }
        if (count < int(fn->minArgs) || (fn->maxArgs != 255 && count > int(fn->maxArgs))) return fail(t.pos, QCoreApplication::translate("jp::calc::Formula", "%1 can't take the number of arguments given (%2). Check how many it takes.").arg(name).arg(count));
        if (fn->flags & kFnVolatile) prog.isVolatile = true;
        bool seen = false;
        for (auto &c : prog.calls)
            if (c.first == fi) {
                ++c.second;
                seen = true;
            }
        if (!seen) prog.calls.push_back({fi, 1});
        if (!lazy) return emit(Op::Call, fi, quint16(count));
        if (!lazyCall(name, parts)) return false;
        lastGroupEnd = out == &prog.code ? int(out->size()) : -1;
        return true;
    }

    void append(const std::vector<Instr> &code) { out->insert(out->end(), code.begin(), code.end()); }

    bool lazyCall(const QString &name, std::vector<std::vector<Instr>> &parts)
    {
        const qint64 total = [&] {
            qint64 sum = 0;
            for (const auto &part : parts) sum += qint64(part.size());
            return sum;
        }();
        if (total + qint64(out->size()) + qint64(parts.size()) * 2 + 4 > kMaxFormulaInstructions) return fail(peek().pos, QCoreApplication::translate("jp::calc::Formula", "The formula is too long."));
        if (name == QLatin1String("IF")) {
            if (parts.size() == 2) {
                Instr no;
                no.op = Op::PushBool;
                no.operand = 0;
                parts.push_back({no});
            }
            append(parts[0]);
            Instr j;
            j.op = Op::IfJump;
            j.operand = qint32(parts[1].size()) + 1;
            j.argc = quint16(parts[1].size() + 1 + parts[2].size());
            out->push_back(j);
            append(parts[1]);
            Instr go;
            go.op = Op::Jump;
            go.operand = qint32(parts[2].size());
            out->push_back(go);
            append(parts[2]);
            return true;
        }
        if (name == QLatin1String("IFERROR") || name == QLatin1String("IFNA")) {
            append(parts[0]);
            Instr j;
            j.op = Op::TryJump;
            j.flags = name == QLatin1String("IFNA") ? 1 : 0;
            j.operand = qint32(parts[1].size());
            out->push_back(j);
            append(parts[1]);
            return true;
        }
        // CHOOSE(index, value1, value2, ...)
        const int count = int(parts.size()) - 1;
        append(parts[0]);
        const qint64 chooseAt = qint64(out->size());
        qint64 end = chooseAt + 1 + count;
        for (int k = 1; k <= count; ++k) end += qint64(parts[size_t(k)].size()) + 1;
        Instr ch;
        ch.op = Op::Choose;
        ch.argc = quint16(count);
        ch.operand = qint32(end - (chooseAt + 1));
        out->push_back(ch);
        qint64 start = chooseAt + 1 + count;
        for (int k = 1; k <= count; ++k) {
            Instr j;
            j.op = Op::Jump;
            j.operand = qint32(start - (chooseAt + k + 1));
            out->push_back(j);
            start += qint64(parts[size_t(k)].size()) + 1;
        }
        for (int k = 1; k <= count; ++k) {
            append(parts[size_t(k)]);
            Instr j;
            j.op = Op::Jump;
            j.operand = qint32(end - (qint64(out->size()) + 1));
            out->push_back(j);
        }
        return true;
    }
};

// ---- the text of a program

bool nameNeedsQuotes(const QString &name)
{
    if (name.isEmpty() || isDigit(name[0])) return true;
    for (const QChar c : name)
        if (!c.isLetterOrNumber() && c != QLatin1Char('_') && c != QLatin1Char('.')) return true;
    // a name that reads as a cell
    if (name.size() <= 7) {
        int i = 0;
        while (i < name.size() && isAsciiLetter(name[i])) ++i;
        int j = i;
        while (j < name.size() && isDigit(name[j])) ++j;
        if (i >= 1 && i <= 3 && j > i && j == name.size()) return true;
    }
    return false;
}

QString sheetPrefix(const QStringList &names, int s1, int s2)
{
    if (s1 < 0) return {};
    const QString a = names.value(s1);
    const QString b = s2 >= 0 ? names.value(s2) : QString();
    const QString all = s2 >= 0 ? a + QLatin1Char(':') + b : a;
    if (nameNeedsQuotes(a) || (s2 >= 0 && nameNeedsQuotes(b))) {
        QString q = all;
        q.replace(QLatin1Char('\''), QLatin1String("''"));
        return QLatin1Char('\'') + q + QLatin1String("'!");
    }
    return all + QLatin1Char('!');
}

QString refText(const RefLit &r)
{
    auto col = [](quint32 c, bool abs) { return QString(abs ? "$" : "") + columnLetters(c); };
    auto row = [](quint32 rw, bool abs) { return QString(abs ? "$" : "") + QString::number(qint64(rw) + 1); };
    if (r.flags & kRefWholeCols) return col(r.col1, r.flags & kRefCol1Abs) + QLatin1Char(':') + col(r.col2, r.flags & kRefCol2Abs);
    if (r.flags & kRefWholeRows) return row(r.row1, r.flags & kRefRow1Abs) + QLatin1Char(':') + row(r.row2, r.flags & kRefRow2Abs);
    QString t = col(r.col1, r.flags & kRefCol1Abs) + row(r.row1, r.flags & kRefRow1Abs);
    if ((r.flags & kRefArea) || r.row1 != r.row2 || r.col1 != r.col2) t += QLatin1Char(':') + col(r.col2, r.flags & kRefCol2Abs) + row(r.row2, r.flags & kRefRow2Abs);
    return t;
}

QString valueText(const Value &v)
{
    switch (v.kind()) {
    case Value::Kind::Number: return QString::number(v.number(), 'g', 15);
    case Value::Kind::Text: {
        QString t = v.text();
        t.replace(QLatin1Char('"'), QLatin1String("\"\""));
        return QLatin1Char('"') + t + QLatin1Char('"');
    }
    case Value::Kind::Bool: return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case Value::Kind::Error: return errorKindText(v.errorKind());
    default: return QStringLiteral("_");
    }
}

} // namespace

QString columnLetters(quint32 col)
{
    QString name;
    for (quint32 v = col + 1; v > 0; v = (v - 1) / 26) name.prepend(QChar(ushort('A' + (v - 1) % 26)));
    return name;
}

QString Program::dump() const
{
    QStringList parts;
    for (size_t pc = 0; pc < code.size(); ++pc) {
        const Instr &in = code[pc];
        switch (in.op) {
        case Op::PushNumber: parts << QString::number(numbers.at(size_t(in.operand)), 'g', 15); break;
        case Op::PushText: parts << valueText(Value::text(texts.at(size_t(in.operand)))); break;
        case Op::PushBool: parts << (in.operand ? QStringLiteral("TRUE") : QStringLiteral("FALSE")); break;
        case Op::PushError: parts << errorKindText(ErrorKind(in.operand)); break;
        case Op::PushMissing: parts << QStringLiteral("_"); break;
        case Op::PushRef: {
            const RefLit &r = refs.at(size_t(in.operand));
            parts << sheetPrefix(sheetNames, r.sheet1, r.sheet2) + refText(r);
            break;
        }
        case Op::PushName: parts << QStringLiteral("name:") + sheetPrefix(sheetNames, int(in.argc) - 1, -1) + texts.at(size_t(in.operand)); break;
        case Op::PushArray: {
            const Value &a = arrays.at(size_t(in.operand));
            QString t = QStringLiteral("{");
            for (int r = 0; r < a.rows(); ++r) {
                if (r) t += QLatin1Char(';');
                for (int c = 0; c < a.cols(); ++c) {
                    if (c) t += QLatin1Char(',');
                    t += valueText(a.at(r, c));
                }
            }
            parts << t + QLatin1Char('}');
            break;
        }
        case Op::PushTable: parts << QStringLiteral("table:") + texts.at(size_t(in.operand)); break;
        case Op::PushExternal: parts << QStringLiteral("ext:") + texts.at(size_t(in.operand)); break;
        case Op::Add: parts << QStringLiteral("+"); break;
        case Op::Sub: parts << QStringLiteral("-"); break;
        case Op::Mul: parts << QStringLiteral("*"); break;
        case Op::Div: parts << QStringLiteral("/"); break;
        case Op::Pow: parts << QStringLiteral("^"); break;
        case Op::Concat: parts << QStringLiteral("&"); break;
        case Op::Eq: parts << QStringLiteral("="); break;
        case Op::Ne: parts << QStringLiteral("<>"); break;
        case Op::Lt: parts << QStringLiteral("<"); break;
        case Op::Le: parts << QStringLiteral("<="); break;
        case Op::Gt: parts << QStringLiteral(">"); break;
        case Op::Ge: parts << QStringLiteral(">="); break;
        case Op::Range: parts << QStringLiteral(":"); break;
        case Op::Union: parts << QStringLiteral(","); break;
        case Op::Intersect: parts << QStringLiteral("isect"); break;
        case Op::Neg: parts << QStringLiteral("neg"); break;
        case Op::Percent: parts << QStringLiteral("pct"); break;
        case Op::Single: parts << QStringLiteral("@"); break;
        case Op::Spill: parts << QStringLiteral("#"); break;
        case Op::Call:
            if (in.operand >= 0)
                parts << QString::fromLatin1(functionAt(in.operand).name) + QLatin1Char('/') + QString::number(in.argc);
            else
                parts << texts.at(size_t(-in.operand - 1)) + QLatin1Char('/') + QString::number(in.argc) + QLatin1Char('?');
            break;
        case Op::Jump: parts << QStringLiteral("jump %1").arg(qint64(pc) + 1 + in.operand); break;
        case Op::IfJump: parts << QStringLiteral("ifjump %1").arg(qint64(pc) + 1 + in.operand); break;
        case Op::TryJump: parts << QStringLiteral("%1 %2").arg(in.flags & 1 ? "trynajump" : "tryjump").arg(qint64(pc) + 1 + in.operand); break;
        case Op::Choose: parts << QStringLiteral("choose %1").arg(qint64(pc) + 1 + in.operand); break;
        }
    }
    return parts.join(QLatin1Char(' '));
}

ParseResult parseFormula(const QString &text, const ParseOptions &options)
{
    ParseResult result;
    auto program = std::make_shared<Program>();
    program->arrayFormula = options.arrayFormula;
    Parser parser(text, options, program.get());
    if (!parser.run()) {
        result.error = parser.error.isEmpty() ? QCoreApplication::translate("jp::calc::Formula", "This is not a formula.") : parser.error;
        result.errorPos = parser.errorPos;
        return result;
    }
    result.program = program;
    return result;
}

bool findReferences(const QString &text, const ParseOptions &options, std::vector<ReferenceAt> *out, bool withNames)
{
    Program program;
    Parser parser(text, options, &program);
    return parser.references(out, withNames);
}

QString referenceCellsText(const RefLit &ref) { return refText(ref); }

} // namespace jp::calc

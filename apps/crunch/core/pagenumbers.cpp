#include "core/pagenumbers.h"

#include "sheet/headerfooter.h"

#include <QCoreApplication>
#include <QVector>

namespace jp::crunch {

namespace {

// A part of a header or footer string as the codes and the text it is made of: "&&" is an ampersand, "&P" and "&N" the page's number and the count, the codes of bold, italic, underline,
// and the like, a size, or a font are formatting, and the other codes (&D, &F, ...) are fields that are not page numbers.
struct Token {
    enum Kind { Literal, Page, Count, Format, Other } kind = Literal;
    int from = 0, length = 1;
    QChar ch;   // a literal's character
};

QVector<Token> tokensOf(const QString &part)
{
    QVector<Token> out;
    for (int i = 0; i < part.size();) {
        Token t;
        t.from = i;
        if (part[i] != QLatin1Char('&') || i + 1 >= part.size()) {
            t.ch = part[i];
            i += 1;
        } else {
            const QChar c = part[i + 1];
            if (c == QLatin1Char('&')) {
                t.ch = QLatin1Char('&');
                t.length = 2;
            } else if (c == QLatin1Char('P')) {
                t.kind = Token::Page;
                t.length = 2;
            } else if (c == QLatin1Char('N')) {
                t.kind = Token::Count;
                t.length = 2;
            } else if (c.isDigit()) {
                t.kind = Token::Format;
                int k = i + 1;
                while (k < part.size() && part[k].isDigit()) ++k;
                t.length = k - i;
            } else if (c == QLatin1Char('"')) {
                t.kind = Token::Format;
                const int close = part.indexOf(QLatin1Char('"'), i + 2);
                t.length = close < 0 ? int(part.size()) - i : close + 1 - i;
            } else if (QStringLiteral("BIUESXYOH").contains(c)) {
                t.kind = Token::Format;
                t.length = 2;
            } else {
                t.kind = Token::Other;
                t.length = 2;
            }
            i += t.length;
        }
        out << t;
    }
    return out;
}

// Whether a part is nothing but the page number and count, formatting, and the words "Page" and "of" round them.
bool onlyNumbers(const QString &part)
{
    QString words;
    for (const Token &t : tokensOf(part)) {
        if (t.kind == Token::Other) return false;
        if (t.kind == Token::Literal) words += t.ch;
    }
    words.remove(QCoreApplication::translate("jp::PageNumbers", "Page"), Qt::CaseInsensitive);
    words.remove(QCoreApplication::translate("jp::PageNumbers", "of"), Qt::CaseInsensitive);
    return words.trimmed().isEmpty();
}

bool hasCodes(const QString &part)
{
    for (const Token &t : tokensOf(part))
        if (t.kind == Token::Page || t.kind == Token::Count) return true;
    return false;
}

int ownPart(const PageNumberDesign &d) { return d.side == PageNumberSide::Left ? 0 : d.side == PageNumberSide::Right ? 2 : 1; }

} // namespace

bool offersPageNumberDesign(const PageNumberDesign &d) { return pageNumberIsTextOnly(d); }

QString pageNumberCode(const PageNumberDesign &d)
{
    const PageNumberType type = pageNumberType(d);
    QString code;
    if (type.bold) code += QStringLiteral("&B");
    if (type.size > 0) code += QLatin1Char('&') + QString::number(int(type.size));
    for (const PageNumberPiece &piece : pageNumberPieces(d)) {
        if (piece.kind == PageNumberPiece::Page) code += QStringLiteral("&P");
        else if (piece.kind == PageNumberPiece::Count) code += QStringLiteral("&N");
        else code += jp::office::sheet::escapeHeaderFooterText(piece.text);
    }
    return code;
}

bool partHoldsOtherText(const QString &text, const PageNumberDesign &d)
{
    QString parts[3];
    jp::office::sheet::splitHeaderFooter(text, &parts[0], &parts[1], &parts[2]);
    const QString &own = parts[ownPart(d)];
    return !own.trimmed().isEmpty() && !onlyNumbers(own);
}

QString withPageNumberDesign(const QString &text, const PageNumberDesign &d, bool keepText, QString *refused)
{
    QString parts[3];
    jp::office::sheet::splitHeaderFooter(text, &parts[0], &parts[1], &parts[2]);
    const int own = ownPart(d);
    for (int i = 0; i < 3; ++i)
        if (i != own && hasCodes(parts[i]) && onlyNumbers(parts[i])) parts[i].clear();
    const QString code = pageNumberCode(d);
    // Kept, the words stay and the number follows them after a space.
    const QString made = keepText && !parts[own].trimmed().isEmpty() && !onlyNumbers(parts[own]) ? parts[own] + QLatin1Char(' ') + code : code;
    if (made.size() > kMostPartChars) {
        if (refused) *refused = QCoreApplication::translate("jp::PageNumbers", "That part of the header or footer would be longer than %1 characters, so the page number was not put in.").arg(kMostPartChars);
        return text;
    }
    parts[own] = made;
    return jp::office::sheet::composeHeaderFooter(parts[0], parts[1], parts[2]);
}

QString withoutPageNumbers(const QString &text)
{
    QString parts[3];
    jp::office::sheet::splitHeaderFooter(text, &parts[0], &parts[1], &parts[2]);
    for (QString &part : parts) {
        if (!hasCodes(part)) continue;
        if (onlyNumbers(part)) {
            part.clear();
            continue;
        }
        QString kept;
        for (const Token &t : tokensOf(part))
            if (t.kind != Token::Page && t.kind != Token::Count) kept += part.mid(t.from, t.length);
        part = kept;
    }
    return jp::office::sheet::composeHeaderFooter(parts[0], parts[1], parts[2]);
}

bool hasPageNumbers(const QString &text)
{
    QString parts[3];
    jp::office::sheet::splitHeaderFooter(text, &parts[0], &parts[1], &parts[2]);
    return hasCodes(parts[0]) || hasCodes(parts[1]) || hasCodes(parts[2]);
}

} // namespace jp::crunch

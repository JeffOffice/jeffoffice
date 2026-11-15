#include "core/pagenumbers.h"

#include "sheet/headerfooter.h"

#include <QCoreApplication>
#include <QRegularExpression>

namespace jp::crunch {

namespace {

// A part with the page number codes taken out; and whether what is left is nothing but the formatting codes, the words of "Page X of Y", and spaces.
bool onlyNumbers(const QString &part)
{
    QString rest = part;
    rest.remove(QRegularExpression(QStringLiteral("&(?:P|N|B|I|U|E|S|X|Y|O|H|\\d+|\"[^\"]*\")")));
    rest.remove(QCoreApplication::translate("jp::PageNumbers", "Page"), Qt::CaseInsensitive);
    rest.remove(QCoreApplication::translate("jp::PageNumbers", "of"), Qt::CaseInsensitive);
    return rest.trimmed().isEmpty();
}

bool hasCodes(const QString &part) { return part.contains(QLatin1String("&P")) || part.contains(QLatin1String("&N")); }

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

QString withPageNumberDesign(const QString &text, const PageNumberDesign &d)
{
    QString parts[3];
    jp::office::sheet::splitHeaderFooter(text, &parts[0], &parts[1], &parts[2]);
    const int own = d.side == PageNumberSide::Left ? 0 : d.side == PageNumberSide::Right ? 2 : 1;
    for (int i = 0; i < 3; ++i)
        if (i != own && hasCodes(parts[i]) && onlyNumbers(parts[i])) parts[i].clear();
    parts[own] = pageNumberCode(d);
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
        part.remove(QStringLiteral("&P")).remove(QStringLiteral("&N"));
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

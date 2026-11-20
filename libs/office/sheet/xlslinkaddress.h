#pragma once
// The address of another workbook as the old binary format keeps it in the book record of a link: one character that says it is a file's address (1), then the path with the
// format's own codes in place of its marks: 1 and a drive letter (or "@" for a network path, the server's name following) for "C:\" and "\\", 2 for the root of the drive the file is on,
// 3 between folders, 4 for each parent folder ("..\"), and 5 with the length of the address and the address for a web address. No brackets are written round the file's name.
// The codes are those of the format's published description as its record layout gives them, checked against the addresses in real files and in what the other open-source suite writes.

#include <QString>
#include <QStringList>
#include <QUrl>

#include <algorithm>

namespace jp::office::sheet {

// The characters to write for an address (a Windows path, a network path, a relative path, a web address, or a "file:" address, which is percent-decoded first).
inline QString xlsEncodeLinkAddress(const QString &target)
{
    QString t = target;
    const QChar code1(1), code2(2), code3(3), code4(4), code5(5);
    if (t.startsWith(QLatin1String("http://"), Qt::CaseInsensitive) || t.startsWith(QLatin1String("https://"), Qt::CaseInsensitive) || t.startsWith(QLatin1String("ftp://"), Qt::CaseInsensitive))
        return QString(code1) + code5 + QChar(ushort(std::min<qsizetype>(t.size(), 0xFFFF))) + t.left(0xFFFF);
    bool network = false;
    if (t.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) {
        QString rest = t.mid(5);
        if (rest.startsWith(QLatin1String("//"))) {
            rest.remove(0, 2);
            if (!rest.startsWith(QLatin1Char('/')) || rest.startsWith(QLatin1String("//"))) network = true;
        }
        t = QUrl::fromPercentEncoding(rest.toUtf8());
        if (network) {
            while (t.startsWith(QLatin1Char('/'))) t.remove(0, 1);
            t.prepend(QLatin1String("//"));
        } else if (t.size() > 2 && t[0] == QLatin1Char('/') && t[2] == QLatin1Char(':')) {
            t.remove(0, 1);   // (file:///C:/x)
        }
    }
    t.replace(QLatin1Char('\\'), QLatin1Char('/'));
    QString lead;
    if (t.startsWith(QLatin1String("//"))) {
        lead = QString(code1) + QLatin1Char('@');
        t.remove(0, 2);
    } else if (t.size() >= 2 && t[1] == QLatin1Char(':') && t[0].isLetter()) {
        lead = QString(code1) + t[0];
        t.remove(0, 2);
    } else if (t.startsWith(QLatin1Char('/'))) {
        lead = QString(code2);
    }
    QStringList parts;
    int up = 0;
    for (const QString &s : t.split(QLatin1Char('/'))) {
        if (s.isEmpty() || s == QLatin1String(".")) continue;
        if (s == QLatin1String("..")) {
            if (!parts.isEmpty()) parts.removeLast();
            else if (lead.isEmpty()) ++up;
            continue;
        }
        parts << s;
    }
    return QString(code1) + lead + QString(up, code4) + parts.join(code3);
}

// The address an encoded one stands for, written as a Windows path ("C:\data\a.xlsx", "\\host\share\a.xlsx", "..\a.xlsx", "a.xlsx") or as the web address it is; empty if it is no file's
// address (the text of a link of another kind), or is one this program cannot name (a folder of the program that made the file).
inline QString xlsDecodeLinkAddress(const QString &encoded)
{
    if (encoded.isEmpty() || encoded[0].unicode() != 1) return QString();
    QString out;
    qsizetype i = 1;
    if (i < encoded.size()) {
        const ushort c = encoded[i].unicode();
        if (c == 1) {
            if (i + 1 >= encoded.size()) return QString();
            if (encoded[i + 1] == QLatin1Char('@')) out = QStringLiteral("\\\\");
            else out = QString(encoded[i + 1]) + QStringLiteral(":\\");
            i += 2;
        } else if (c == 2) {
            out = QStringLiteral("\\");
            ++i;
        } else if (c == 5) {
            if (i + 1 >= encoded.size()) return QString();
            const qsizetype n = encoded[i + 1].unicode();
            if (i + 2 + n > encoded.size()) return QString();
            out = encoded.mid(i + 2, n);
            i += 2 + n;
        } else if (c >= 6 && c <= 8) {
            return QString();
        }
    }
    for (; i < encoded.size(); ++i) {
        const ushort c = encoded[i].unicode();
        if (c == 3) out += QLatin1Char('\\');
        else if (c == 4) out += QStringLiteral("..\\");
        else if (c < 0x20) return QString();
        else out += encoded[i];
    }
    return out;
}

} // namespace jp::office::sheet

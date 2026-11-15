#include "core/remotepath.h"

#include <QUrl>

namespace jp {

bool isOnAnotherComputer(const QString &addressIn)
{
    QString s = addressIn.trimmed();
    if (s.isEmpty()) return false;
    const bool fileAddress = s.startsWith(QLatin1String("file:"), Qt::CaseInsensitive);
    if (fileAddress) {
        // (only a file: address is percent-encoded; a plain path is what it is)
        s = QUrl::fromPercentEncoding(s.mid(5).toUtf8());
    } else if (s.contains(QLatin1String("://"))) {
        return false;   // a web (or other) address
    }
    s.replace(QLatin1Char('\\'), QLatin1Char('/'));
    // \\?\C:\x and \\.\C:\x name a drive of this computer (only \\?\UNC\host\share is on another one); any other device name is taken for another computer, as before.
    if (!fileAddress && (s.startsWith(QLatin1String("//?/")) || s.startsWith(QLatin1String("//./")))) {
        const QString rest = s.mid(4);
        if (rest.size() >= 2 && rest[0].isLetter() && rest[0].unicode() < 128 && rest[1] == QLatin1Char(':') && (rest.size() == 2 || rest[2] == QLatin1Char('/'))) return false;
        return true;
    }
    if (!fileAddress) return s.startsWith(QLatin1String("//"));
    // file://host/share and file://///host/share are on another computer; file:///path (an empty host) and file://localhost/path are not.
    if (s.startsWith(QLatin1String("//"))) {
        s.remove(0, 2);
        if (s.startsWith(QLatin1Char('/'))) return s.startsWith(QLatin1String("//"));
        const QString host = s.left(s.indexOf(QLatin1Char('/'))).toLower();
        return !host.isEmpty() && host != QLatin1String("localhost");
    }
    return false;
}

bool isWindowsSpecialPath(const QString &pathIn, bool streams)
{
    QString p = pathIn;
    p.replace(QLatin1Char('\\'), QLatin1Char('/'));
    // (the prefixes of a drive, and a drive letter's colon, are taken off before the rest is looked at)
    if (p.startsWith(QLatin1String("//?/")) || p.startsWith(QLatin1String("//./"))) p.remove(0, 4);
    if (p.startsWith(QLatin1Char('/')) && p.size() >= 3 && p[1].isLetter() && p[2] == QLatin1Char(':')) p.remove(0, 1);   // (/C:/x, from a file: address)
    if (p.size() >= 2 && p[0].isLetter() && p[0].unicode() < 128 && p[1] == QLatin1Char(':')) p.remove(0, 2);
    if (streams && p.contains(QLatin1Char(':'))) return true;
    static const QStringList plain = {QStringLiteral("con"), QStringLiteral("prn"), QStringLiteral("aux"), QStringLiteral("nul"), QStringLiteral("conin$"), QStringLiteral("conout$")};
    for (const QString &part : p.split(QLatin1Char('/'), Qt::SkipEmptyParts)) {
        QString name = part.section(QLatin1Char('.'), 0, 0).trimmed().toLower();
        // (a stream's name, "con:x", has no ending of its own: the part before a colon is the name)
        name = name.section(QLatin1Char(':'), 0, 0).trimmed();
        if (plain.contains(name)) return true;
        if (name.size() == 4 && (name.startsWith(QLatin1String("com")) || name.startsWith(QLatin1String("lpt"))) && name[3] >= QLatin1Char('1') && name[3] <= QLatin1Char('9')) return true;
    }
    return false;
}

} // namespace jp

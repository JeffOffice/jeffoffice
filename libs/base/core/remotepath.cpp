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

} // namespace jp

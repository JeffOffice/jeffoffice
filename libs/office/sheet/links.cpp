#include "sheet/links.h"

#include <QRegularExpression>

namespace jp::office::sheet {

namespace {

QString schemeOf(const QString &target)
{
    static const QRegularExpression scheme(QStringLiteral("^\\s*([A-Za-z][A-Za-z0-9+.\\-]*):"));
    const QRegularExpressionMatch m = scheme.match(target);
    if (!m.hasMatch()) return QString();
    const QString s = m.captured(1).toLower();
    return s.size() == 1 ? QString() : s;   // (one letter is a drive: "C:\folder\file")
}

} // namespace

LinkKind targetKind(const QString &target)
{
    const QString s = schemeOf(target);
    if (s.isEmpty() || s == QLatin1String("file")) return LinkKind::File;
    if (s == QLatin1String("http") || s == QLatin1String("https")) return LinkKind::Web;
    if (s == QLatin1String("mailto")) return LinkKind::Mail;
    if (s == QLatin1String("javascript") || s == QLatin1String("vbscript") || s == QLatin1String("data") || s == QLatin1String("livescript") || s == QLatin1String("mocha") || s == QLatin1String("blob"))
        return LinkKind::Unsafe;
    return LinkKind::Other;
}

LinkKind linkKind(const LinkSpec &link) { return link.target.isEmpty() ? LinkKind::Place : targetKind(link.target); }

QString webAddressFromTyped(const QString &typed)
{
    QString t = typed.trimmed();
    if (t.isEmpty()) return QString();
    if (!schemeOf(t).isEmpty()) return t;
    // (a file's name or path, not a web address)
    static const QRegularExpression path(QStringLiteral("^(?:[A-Za-z]:[\\\\/]|\\\\\\\\|/|\\./|\\.\\./|~/)"));
    if (path.match(t).hasMatch()) return t;
    static const QRegularExpression mail(QStringLiteral("^[^\\s@/\\\\]+@[^\\s@/\\\\]+\\.[^\\s@/\\\\]+$"));
    if (mail.match(t).hasMatch()) return QStringLiteral("mailto:") + t;
    return QStringLiteral("https://") + t;
}

} // namespace jp::office::sheet

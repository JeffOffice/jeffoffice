#include "sheet/links.h"

#include <QRegularExpression>
#include <QSet>
#include <QUrl>

#include <algorithm>

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

bool namesAProgram(const QString &target)
{
    static const QSet<QString> programs = {QStringLiteral("exe"), QStringLiteral("bat"), QStringLiteral("cmd"), QStringLiteral("com"), QStringLiteral("lnk"), QStringLiteral("ps1"), QStringLiteral("ps1xml"),
                                           QStringLiteral("psm1"), QStringLiteral("vbs"), QStringLiteral("vbe"), QStringLiteral("js"), QStringLiteral("jse"), QStringLiteral("msi"), QStringLiteral("msp"),
                                           QStringLiteral("scr"), QStringLiteral("pif"), QStringLiteral("hta"), QStringLiteral("wsf"), QStringLiteral("wsh"), QStringLiteral("jar"), QStringLiteral("reg"),
                                           QStringLiteral("cpl"), QStringLiteral("msc"), QStringLiteral("scf"), QStringLiteral("url"), QStringLiteral("inf"), QStringLiteral("dll"), QStringLiteral("sys"),
                                           QStringLiteral("app"), QStringLiteral("command"), QStringLiteral("sh"), QStringLiteral("desktop"), QStringLiteral("run"), QStringLiteral("appref-ms"),
                                           QStringLiteral("application"), QStringLiteral("gadget"), QStringLiteral("library-ms"), QStringLiteral("settingcontent-ms"), QStringLiteral("action"), QStringLiteral("workflow")};
    QString t = QUrl::fromPercentEncoding(target.left(kMaxLinkTarget).toUtf8());
    const int cut = int(t.indexOf(QRegularExpression(QStringLiteral("[?#]"))));
    if (cut >= 0) t.truncate(cut);
    // (a system drops what trails a name: spaces, dots, quotes, and the NUL that ends a string)
    while (!t.isEmpty() && (t.back().isSpace() || t.back() == QLatin1Char('.') || t.back() == QLatin1Char('"') || t.back() == QLatin1Char('\'') || t.back() == QChar(0))) t.chop(1);
    const int slash = int(std::max(t.lastIndexOf(QLatin1Char('/')), t.lastIndexOf(QLatin1Char('\\'))));
    const QString name = t.mid(slash + 1);
    // (an alternate stream, "file.txt:evil.exe", is taken for what follows the last colon too)
    for (const QString &part : name.split(QLatin1Char(':'))) {
        const int dot = int(part.lastIndexOf(QLatin1Char('.')));
        if (dot >= 0 && programs.contains(part.mid(dot + 1).trimmed().toLower())) return true;
    }
    return false;
}

QString elidedMiddle(const QString &text, int most)
{
    if (text.size() <= most || most < 8) return text;
    const int head = (most - 3) * 2 / 3, tail = most - 3 - head;
    return text.left(head) + QStringLiteral("...") + text.right(tail);
}

LinkKind targetKind(const QString &target)
{
    const QString s = schemeOf(target);
    if (s.isEmpty() || s == QLatin1String("file")) return namesAProgram(target) ? LinkKind::Unsafe : LinkKind::File;
    // (a name before a colon that is no scheme of a web page or a mail, such as "doc.txt:evil.exe", or an address of another kind that names a program, is not opened)
    if (s != QLatin1String("http") && s != QLatin1String("https") && s != QLatin1String("mailto") && namesAProgram(target)) return LinkKind::Unsafe;
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

#include "office/hyperlink.h"

#include <QRegularExpression>
#include <QUrl>
#include "office/drawingml.h"
#include "office/package.h"
#include "office/xml.h"

namespace jp::office {

LinkTarget parseLink(const QString &linkIn)
{
    LinkTarget t;
    const QString link = linkIn.trimmed();
    if (link.isEmpty()) return t;
    if (link.startsWith(QLatin1Char('#'))) {
        const QString word = link.mid(1).toLower();
        if (word == QLatin1String("next")) t.kind = LinkKind::Next;
        else if (word == QLatin1String("previous")) t.kind = LinkKind::Previous;
        else if (word == QLatin1String("first")) t.kind = LinkKind::First;
        else if (word == QLatin1String("last")) t.kind = LinkKind::Last;
        else if (word == QLatin1String("end")) t.kind = LinkKind::EndShow;
        else if (word.startsWith(QLatin1String("page"))) {
            bool ok = false;
            const int n = word.mid(4).toInt(&ok);
            if (ok && n >= 1) {
                t.kind = LinkKind::Slide;
                t.slide = n;
            }
        }
        return t;
    }
    if (link.startsWith(QLatin1String("http://"), Qt::CaseInsensitive) || link.startsWith(QLatin1String("https://"), Qt::CaseInsensitive)) {
        t.kind = LinkKind::Web;
        t.address = link;
    } else if (link.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
        t.kind = LinkKind::Mail;
        t.address = link;
    } else if (link.startsWith(QLatin1String("file:"), Qt::CaseInsensitive) || link.startsWith(QLatin1String("\\\\")) || link.startsWith(QLatin1String("//")) ||
               QRegularExpression(QStringLiteral("^[A-Za-z]:[\\\\/]")).match(link).hasMatch() ||
               !QRegularExpression(QStringLiteral("^[A-Za-z][A-Za-z0-9+.-]+:")).match(link).hasMatch()) {
        // A path; a link with another scheme is nothing that is ever followed.
        t.kind = LinkKind::File;
        t.address = link;
    }
    return t;
}

QString slideLink(int number) { return QStringLiteral("#page%1").arg(number); }

bool isRemoteFile(const QString &linkIn)
{
    const QString link = linkIn.trimmed();
    if (link.startsWith(QLatin1String("\\\\")) || link.startsWith(QLatin1String("//"))) return true;
    if (!link.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) return false;
    // "file:///path" is this computer's; "file://host/path", "file:////host/share", and "file:\\host" are not.
    QString rest = link.mid(5);
    rest.replace(QLatin1Char('\\'), QLatin1Char('/'));
    if (!rest.startsWith(QLatin1String("//"))) return false;
    rest = rest.mid(2);
    if (rest.startsWith(QLatin1Char('/'))) return rest.startsWith(QLatin1String("//"));   // empty host: local, unless four slashes
    const QString host = rest.left(rest.indexOf(QLatin1Char('/'))).toLower();
    return !host.isEmpty() && host != QLatin1String("localhost");
}

QString linkAddress(const QString &link) { return parseLink(link).address; }

QString linkOfClick(const QDomElement &click, const DrawingContext &ctx)
{
    if (click.isNull()) return QString();
    const QString id = attrNs(click, ns::R, QStringLiteral("id"));
    const QString action = attr(click, QStringLiteral("action"));
    static const QLatin1String jump("ppaction://hlinkshowjump?jump=");
    if (action.startsWith(jump)) {
        const QString what = action.mid(jump.size());
        if (what == QLatin1String("nextslide")) return QStringLiteral("#next");
        if (what == QLatin1String("previousslide")) return QStringLiteral("#previous");
        if (what == QLatin1String("firstslide")) return QStringLiteral("#first");
        if (what == QLatin1String("lastslide")) return QStringLiteral("#last");
        if (what == QLatin1String("endshow")) return QStringLiteral("#end");
        return QString();
    }
    if (id.isEmpty() || !ctx.package) return QString();
    const Relationship rel = ctx.package->relationship(ctx.part, id);
    if (rel.id.isEmpty()) return QString();
    if (action == QLatin1String("ppaction://hlinksldjump")) {
        const int n = !rel.external && ctx.slideNumberOf ? ctx.slideNumberOf(rel.target) : 0;
        return n >= 1 ? slideLink(n) : QString();
    }
    return rel.external ? rel.target : QString();
}

} // namespace jp::office

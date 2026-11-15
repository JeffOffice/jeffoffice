#include "office/hyperlink.h"

#include "core/remotepath.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <QDomElement>
#include <atomic>
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
               QRegularExpression(QStringLiteral("^[A-Za-z]:[\\\\/]")).match(link).hasMatch()) {
        t.kind = LinkKind::File;
        t.address = link;
    } else if (!QRegularExpression(QStringLiteral("^[A-Za-z][A-Za-z0-9+.-]+:")).match(link).hasMatch()) {
        // No scheme: a path when it looks like one (a slash or backslash, or a dot or tilde in front), else a web address typed without its "https://" (www.example.com),
        // as it always was.
        const bool path = link.contains(QLatin1Char('/')) || link.contains(QLatin1Char('\\')) || link.startsWith(QLatin1Char('.')) || link.startsWith(QLatin1Char('~'));
        t.kind = path ? LinkKind::File : LinkKind::Web;
        t.address = link;
    }
    return t;
}

QString slideLink(int number) { return QStringLiteral("#page%1").arg(number); }

bool isRemoteFile(const QString &link) { return isOnAnotherComputer(link); }

namespace {
std::atomic<int> g_fileLooks{0};
}

int linkFileLooks() { return g_fileLooks.load(); }

// A file that runs when it is opened: by its name's ending (the ones a computer runs or interprets), or, for a file on this computer, by being marked as one that runs.
// A file on another computer is judged by its name alone: it is never looked at.
bool isProgramFile(const QString &link)
{
    const LinkTarget t = parseLink(link);
    if (t.kind != LinkKind::File) return false;
    const bool remote = isOnAnotherComputer(t.address);
    QString path = t.address;
    if (path.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) path = QUrl(path).toLocalFile();
    static const QStringList endings = {QStringLiteral("exe"), QStringLiteral("bat"), QStringLiteral("cmd"), QStringLiteral("com"), QStringLiteral("msi"), QStringLiteral("ps1"),
                                        QStringLiteral("vbs"), QStringLiteral("vbe"), QStringLiteral("js"), QStringLiteral("jse"), QStringLiteral("wsf"), QStringLiteral("hta"),
                                        QStringLiteral("sh"), QStringLiteral("command"), QStringLiteral("app"), QStringLiteral("desktop"), QStringLiteral("jar"),
                                        QStringLiteral("py"), QStringLiteral("scr"), QStringLiteral("pif"), QStringLiteral("lnk"), QStringLiteral("reg"), QStringLiteral("msc"),
                                        QStringLiteral("cpl"), QStringLiteral("dll"), QStringLiteral("appimage"), QStringLiteral("run"), QStringLiteral("pl"), QStringLiteral("rb")};
    QString name = (path.isEmpty() ? t.address : path);
    name.replace(QLatin1Char('\\'), QLatin1Char('/'));
    name = name.section(QLatin1Char('/'), -1).trimmed();
    while (name.endsWith(QLatin1Char('.')) || name.endsWith(QLatin1Char(' '))) name.chop(1);   // (Windows ignores a trailing dot or space)
    if (name.contains(QLatin1Char('.')) && endings.contains(name.section(QLatin1Char('.'), -1).toLower())) return true;
    if (remote || path.isEmpty()) return false;
    ++g_fileLooks;
    const QFileInfo info(path);
    return info.isFile() && info.isExecutable();
}

QString linkAddress(const QString &link) { return parseLink(link).address; }

QString linkOfClick(const QDomElement &click, const DrawingContext &ctx)
{
    if (click.isNull()) return QString();
    const QString id = attrNs(click, ns::R, QStringLiteral("id"));
    const QString action = attr(click, QStringLiteral("action"));
    auto relationship = [&] { return id.isEmpty() || !ctx.package ? Relationship() : ctx.package->relationship(ctx.part, id); };
    // A reader that has no slides (a word processor's or a publication's) reads an address and nothing else, as it always has.
    if (!ctx.slideNumberOf) {
        const Relationship rel = relationship();
        return !rel.id.isEmpty() && rel.external && !rel.target.isEmpty() ? rel.target : QString();
    }
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
    const Relationship rel = relationship();
    if (rel.id.isEmpty()) return QString();
    if (action == QLatin1String("ppaction://hlinksldjump")) {
        if (rel.external || !rel.type.endsWith(QLatin1String("/slide"))) return QString();
        const int n = ctx.slideNumberOf(rel.target);
        return n >= 1 ? slideLink(n) : QString();
    }
    // A plain link, or one to a file: an address. A program, a macro, another presentation, a custom show, a sound or movie, and any other action are not links: they stay text.
    if (!action.isEmpty() && action != QLatin1String("ppaction://hlinkfile")) return QString();
    return rel.external && rel.type.endsWith(QLatin1String("/hyperlink")) ? rel.target : QString();
}


QString keptClickOf(const QDomElement &click, const DrawingContext &ctx)
{
    if (click.isNull() || !ctx.slideNumberOf) return QString();
    const QString action = attr(click, QStringLiteral("action"));
    if (action.isEmpty() || action.startsWith(QLatin1String("ppaction://media")) || action.startsWith(QLatin1String("ppaction://hlinkshowjump")) ||
        action == QLatin1String("ppaction://hlinksldjump") || action == QLatin1String("ppaction://hlinkfile"))
        return QString();
    QString target;
    const QString id = attrNs(click, ns::R, QStringLiteral("id"));
    if (!id.isEmpty() && ctx.package) {
        const Relationship rel = ctx.package->relationship(ctx.part, id);
        if (!rel.id.isEmpty() && rel.external) target = rel.target;
    }
    QString xml;
    QTextStream out(&xml);
    click.save(out, -1);
    xml.remove(QRegularExpression(QStringLiteral("\\s+xmlns:\\w+=\"[^\"]*\"")));   // (the document's own declarations, which the DOM repeats on each element)
    xml.remove(QRegularExpression(QStringLiteral("\\s+(?:\\w+:)?id=\"[^\"]*\"")));   // (the relationship id: a new one is made when it is written)
    return target + QChar(0x1F) + xml;
}

} // namespace jp::office

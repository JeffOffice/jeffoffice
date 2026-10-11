#include "app/help.h"
#include "core/appinfo.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QSet>

namespace jp {
namespace help {

namespace {

// The page that holds the contents; the other special pages begin with "_".
const QString kHome = QStringLiteral("Home");
const QString kSidebar = QStringLiteral("_Sidebar");
const QString kFooter = QStringLiteral("_Footer");

} // namespace

QString wikiPageName(const QString &title)
{
    // A wiki page name can't hold / \ : * ? " < > | # %, and a link to one
    // can't hold brackets or parentheses. Commas and apostrophes are fine.
    static const QRegularExpression unsafe(QStringLiteral("[\\\\/:*?\"<>|#%\\[\\]()\\p{Cc}]"), QRegularExpression::UseUnicodePropertiesOption);
    QString name = QString(title).remove(unsafe).simplified().replace(QLatin1Char(' '), QLatin1Char('-'));
    while (name.startsWith(QLatin1Char('-')) || name.startsWith(QLatin1Char('.'))) name.remove(0, 1);
    while (name.endsWith(QLatin1Char('-')) || name.endsWith(QLatin1Char('.'))) name.chop(1);
    return name;
}

bool wikiLinks(QString &md, const QHash<QString, QString> &pages, QString *error)
{
    static const QRegularExpression link(QStringLiteral("\\]\\(([^)]*)\\)"));
    static const QRegularExpression web(QStringLiteral("^([A-Za-z][A-Za-z0-9+.-]*:|#)"));   // https:, mailto:, or a place on this page
    QString out;
    qsizetype last = 0;
    for (auto m = link.globalMatch(md); m.hasNext();) {
        const auto hit = m.next();
        const QString target = hit.captured(1).trimmed();
        if (web.match(target).hasMatch()) continue;
        QString id = target, anchor;
        if (const qsizetype h = id.indexOf(QLatin1Char('#')); h >= 0) {
            anchor = id.mid(h);
            id.truncate(h);
        }
        if (id.startsWith(QLatin1String("./"))) id.remove(0, 2);
        if (id.endsWith(QLatin1String(".md"))) id.chop(3);
        const auto page = pages.constFind(id);
        if (page == pages.constEnd()) {
            if (error) *error = QCoreApplication::translate("Help", "A link names the topic \"%1\", which doesn't exist.").arg(target);
            return false;
        }
        out += md.mid(last, hit.capturedStart(1) - last) + *page + anchor;
        last = hit.capturedEnd(1);
    }
    out += md.mid(last);
    md = out;
    return true;
}

bool exportWiki(const QString &dir, AppWindow *win, QString *error)
{
    auto fail = [error](const QString &why) {
        if (error) *error = why;
        return false;
    };
    const QStringList ids = topicIds();
    if (!ids.contains(QLatin1String("index"))) return fail(QCoreApplication::translate("Help", "Help has no contents topic."));

    // Every topic's page name first: links name topics, not pages.
    QHash<QString, QString> pages;   // topic id -> page name
    QSet<QString> taken{kSidebar.toLower(), kFooter.toLower()};   // a wiki's page names ignore case
    for (const QString &id : ids) {
        const QString name = id == QLatin1String("index") ? kHome : wikiPageName(topic(id).title);
        if (name.isEmpty()) return fail(QCoreApplication::translate("Help", "The topic \"%1\" has no title to name its page from.").arg(id));
        if (taken.contains(name.toLower())) return fail(QCoreApplication::translate("Help", "The topic \"%1\" would have the page name \"%2\", which another page has.").arg(id, name));
        taken.insert(name.toLower());
        pages.insert(id, name);
    }

    static const QRegularExpression title(QStringLiteral("\\A#[ \\t]+([^\\n]*)\\n")), blankLines(QStringLiteral("\\n{3,}"));
    QHash<QString, QString> files;   // file name -> text
    QString home;
    for (const QString &id : ids) {
        const Topic t = topic(id);
        QString md = displayMarkdown(t, win).replace(QLatin1String("\r\n"), QLatin1String("\n"));
        QString why;
        if (!wikiLinks(md, pages, &why)) return fail(QStringLiteral("%1: %2").arg(id, why));
        // The wiki shows the page's name as its title; keep the heading only
        // when that would read differently (the contents, "Home").
        if (const auto m = title.match(md); m.hasMatch() && m.captured(1).trimmed() == t.title && QString(pages[id]).replace(QLatin1Char('-'), QLatin1Char(' ')) == t.title)
            md.remove(0, m.capturedLength());
        // The contents' first lines invite a search in the Help pane.
        if (id == QLatin1String("index"))
            if (const qsizetype sections = md.indexOf(QLatin1String("\n## ")); sections >= 0) {
                const qsizetype start = md.startsWith(QLatin1String("# ")) ? md.indexOf(QLatin1Char('\n')) + 1 : 0;
                md = md.left(start) + (start ? QStringLiteral("\n") : QString())
                     + QCoreApplication::translate("Help", "This is %1's built-in Help, as web pages. In the program, press **F1** at any time for help on what you're doing, or type in Help's search box to find a topic or a command to run.").arg(appInfo().name)
                     + QLatin1Char('\n') + md.mid(sections);
            }
        // The comments leave blank lines behind.
        md.replace(blankLines, QStringLiteral("\n\n"));
        while (md.startsWith(QLatin1Char('\n'))) md.remove(0, 1);
        while (md.endsWith(QLatin1Char('\n')) || md.endsWith(QLatin1Char(' '))) md.chop(1);
        md += QLatin1Char('\n');
        if (id == QLatin1String("index")) home = md;
        files.insert(pages[id] + QStringLiteral(".md"), md);
    }

    // The sidebar is the contents' sections, with their headings in bold.
    QString side = QStringLiteral("**[%1](%2)**\n\n").arg(topic(QStringLiteral("index")).title, kHome);
    const qsizetype sections = home.indexOf(QLatin1String("\n## "));
    side += QString(home.mid(sections < 0 ? 0 : sections + 1)).replace(QRegularExpression(QStringLiteral("^## (.+)$"), QRegularExpression::MultilineOption), QStringLiteral("**\\1**"));
    files.insert(kSidebar + QStringLiteral(".md"), side);
    files.insert(kFooter + QStringLiteral(".md"),
                 QCoreApplication::translate("Help", "These pages are generated from %1's built-in Help (`resources/help/en` in the [repository](%2)) and are replaced on each update, so edits belong in the repository.")
                     .arg(appInfo().name, QStringLiteral("https://github.com/") + appInfo().repository + QStringLiteral("/tree/main/resources/help/en"))
                     + QLatin1Char('\n'));

    // Only into a folder with nothing in it: the pages replace a wiki's whole
    // contents, so the folder must hold exactly these.
    QDir out(dir);
    if (out.exists() && !out.isEmpty(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden)) return fail(QCoreApplication::translate("Help", "The folder \"%1\" isn't empty.").arg(dir));
    if (!QDir().mkpath(dir)) return fail(QCoreApplication::translate("Help", "Couldn't make the folder \"%1\".").arg(dir));
    for (auto it = files.constBegin(); it != files.constEnd(); ++it) {
        QFile f(out.filePath(it.key()));
        if (!f.open(QIODevice::WriteOnly) || f.write(it.value().toUtf8()) < 0) return fail(QCoreApplication::translate("Help", "Couldn't write \"%1\": %2").arg(f.fileName(), f.errorString()));
    }
    return true;
}

} // namespace help
} // namespace jp

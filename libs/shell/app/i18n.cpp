#include "app/i18n.h"

#include "app/settings.h"
#include "core/appinfo.h"

#include <QCoreApplication>
#include <QDir>
#include <QHash>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include <utility>

namespace jp {

namespace {
// The apps share one set of translations: :/i18n/jeffoffice_<language>.qm.
const QLatin1String kSuiteFiles("jeffoffice");
}

QStringList availableUiLanguages()
{
    QStringList out;
    const QString prefix = kSuiteFiles + QLatin1Char('_');
    for (const QString &f : QDir(QStringLiteral(":/i18n")).entryList({prefix + QStringLiteral("*.qm")}, QDir::Files))
        if (f != prefix + QStringLiteral("en.qm")) out << f.mid(prefix.size(), f.size() - prefix.size() - 3);   // <fileName>_<language>.qm; English is the text itself
    return out;
}

QString uiLanguageName(const QString &language)
{
    // The language's plain name ("português", not "português europeu"),
    // then the country.
    QString name = QLocale(language.section(QLatin1Char('_'), 0, 0)).nativeLanguageName();
    if (name.startsWith(QLatin1String("American English"), Qt::CaseInsensitive)) name = QStringLiteral("English");   // ("en" alone is the United States', and the country follows)
    if (language.contains(QLatin1Char('_'))) name += QStringLiteral(" (%1)").arg(QLocale(language).nativeTerritoryName());
    if (!name.isEmpty()) name[0] = name[0].toUpper();
    return name;
}

namespace {
QString s_language = QStringLiteral("en");
QList<QTranslator *> s_installed;   // what installTranslations() put in, to take out again
QString installTranslationsFor();
}

QString currentUiLanguage() { return s_language; }

QString pickUiLanguage(const QStringList &wanted, const QStringList &have)
{
    // Countries that write English as one of the variants, when the system names theirs and the apps have no file for it.
    static const QHash<QString, QString> neighbor = {
        {QStringLiteral("en_NZ"), QStringLiteral("en_AU")}, {QStringLiteral("en_IE"), QStringLiteral("en_GB")}, {QStringLiteral("en_ZA"), QStringLiteral("en_GB")},
        {QStringLiteral("en_IN"), QStringLiteral("en_GB")}, {QStringLiteral("en_SG"), QStringLiteral("en_GB")}, {QStringLiteral("en_HK"), QStringLiteral("en_GB")},
        {QStringLiteral("en_MT"), QStringLiteral("en_GB")}};
    for (const QString &w : wanted) {
        if (have.contains(w)) return w;
        const QString near = neighbor.value(w);
        if (!near.isEmpty() && have.contains(near)) return near;
        if (w.startsWith(QLatin1String("en"))) return QString();   // the text is English already
        const QString base = w.section(QLatin1Char('_'), 0, 0);
        if (have.contains(base)) return base;
    }
    return QString();
}

QString installTranslations()
{
    s_language = installTranslationsFor();
    return s_language;
}

namespace {
QString installTranslationsFor()
{
    // A translator's own file, to see it in place before it's built in:
    // <FILENAME>_TRANSLATION=/path/to/jeffoffice_xx.qm (JEFFPUB_TRANSLATION
    // for JeffPub, JEFFDECK_TRANSLATION for JeffDeck).
    const QByteArray variable = (appInfo().fileName.toUpper() + QStringLiteral("_TRANSLATION")).toLatin1();
    if (const QString own = qEnvironmentVariable(variable.constData()); !own.isEmpty()) {
        auto *t = new QTranslator(QCoreApplication::instance());
        if (t->load(own)) {
            QCoreApplication::installTranslator(t);
            return t->language();
        }
    }
    const QStringList have = availableUiLanguages();
    QStringList wanted;
    const QString chosen = Settings::get().value(QStringLiteral("ui/language")).toString();
    if (!chosen.isEmpty()) wanted << chosen;
    else
        for (QString l : QLocale::system().uiLanguages()) wanted << l.replace(QLatin1Char('-'), QLatin1Char('_'));
    // (a language chosen in Options is looked up as it is; "en" is the text itself)
    const QString lang = chosen == QLatin1String("en") ? QString() : pickUiLanguage(wanted, have);
    // Each call starts from none, so a test (or a later change of language) is not stacked on the last.
    for (QTranslator *t : std::exchange(s_installed, {})) {
        QCoreApplication::removeTranslator(t);
        delete t;
    }
    auto install = [](QTranslator *t, bool loaded) {
        if (loaded) {
            QCoreApplication::installTranslator(t);
            s_installed << t;
        } else delete t;
    };
    auto installFile = [&install](const QString &file) {
        auto *t = new QTranslator(QCoreApplication::instance());
        install(t, t->load(file));
    };
    // English: only its plural forms ("1 page", "2 pages") need a file. The variants (en_GB, en_CA, en_AU) go on top of it and hold
    // only the texts they spell differently; any other text is the American English one.
    if (lang.isEmpty() || lang.startsWith(QLatin1String("en_"))) installFile(QStringLiteral(":/i18n/%1_en.qm").arg(kSuiteFiles));
    if (lang.isEmpty()) return QStringLiteral("en");
    installFile(QStringLiteral(":/i18n/%1_%2.qm").arg(kSuiteFiles, lang));
    if (lang.startsWith(QLatin1String("en_"))) return lang;   // (Qt's own text is English already)
    // Qt's own text (standard buttons such as Cancel, file dialogs): built
    // in beside the apps' (Portugal's falls back to Brazil's, the one Qt
    // has), else beside the program, else where Qt keeps it.
    auto *qt = new QTranslator(QCoreApplication::instance());
    QStringList builtIn{lang, lang.section(QLatin1Char('_'), 0, 0)};
    if (lang.startsWith(QLatin1String("pt"))) builtIn << QStringLiteral("pt_BR");
    bool found = false;
    for (const QString &l : builtIn)
        if ((found = qt->load(QStringLiteral(":/i18n/qtbase_%1.qm").arg(l)))) break;
    for (const QString &dir : {QCoreApplication::applicationDirPath() + QStringLiteral("/translations"), QLibraryInfo::path(QLibraryInfo::TranslationsPath)})
        if (!found) found = qt->load(QStringLiteral("qtbase_%1").arg(lang), dir);
    install(qt, found);
    return lang;
}
} // namespace

} // namespace jp

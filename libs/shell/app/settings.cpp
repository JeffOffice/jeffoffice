#include "app/settings.h"

#include "core/appinfo.h"
#include <algorithm>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLocale>
#include <QRegularExpression>

namespace jp {

Settings &Settings::get()
{
    static Settings s;
    return s;
}

void copyMissingSettings(const QSettings &from, QSettings &to)
{
    for (const QString &key : from.allKeys())
        if (!to.contains(key)) to.setValue(key, from.value(key));
}

bool moveDataFolder(const QString &from, const QString &to)
{
    if (!QFileInfo(from).isDir() || QFileInfo::exists(to)) return false;
    QDir().mkpath(QFileInfo(to).path());
    if (QDir().rename(from, to)) return true;
    // Another drive, or a file held open: copy, and leave the old folder.
    QDirIterator it(from, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    bool ok = QDir().mkpath(to);
    while (it.hasNext()) {
        const QFileInfo fi = it.nextFileInfo();
        const QString dest = to + QLatin1Char('/') + QDir(from).relativeFilePath(fi.filePath());
        ok = (fi.isDir() ? QDir().mkpath(dest) : QFile::copy(fi.filePath(), dest)) && ok;
    }
    return ok;
}

bool suiteWide(const QString &key)
{
    static const QStringList keys = {QStringLiteral("ui/theme"), QStringLiteral("ui/language"), QStringLiteral("user/name"),
                                     QStringLiteral("user/initials"), QStringLiteral("telemetry/enabled"), QStringLiteral("telemetry/install"),
                                     QStringLiteral("proof/checkAsYouType")};
    return key.startsWith(QLatin1String("updates/")) || keys.contains(key);
}

void moveToSuite(QSettings &app, QSettings &suite)
{
    const QString marker = QStringLiteral("migration/suite");
    if (!app.value(marker).toBool()) {
        for (const QString &key : app.allKeys())
            if (suiteWide(key) && !suite.contains(key)) suite.setValue(key, app.value(key));
        app.setValue(marker, true);
    }
    // The statistics' install id joined the suite later (JeffDeck sends the same one), so an app that had moved its keys
    // already brings its id over now. It moves rather than copies: turning statistics off clears the suite's, and no
    // copy of the id stays where that can't reach it.
    const QString install = QStringLiteral("telemetry/install"), idMarker = QStringLiteral("migration/suiteInstall");
    if (app.value(idMarker).toBool()) return;
    if (app.contains(install)) {
        if (!suite.contains(install)) suite.setValue(install, app.value(install));
        app.remove(install);
    }
    app.setValue(idMarker, true);
}

void migrateSpellSetting(QSettings &app, QSettings &suite)
{
    const QString marker = QStringLiteral("migration/spellSetting"), key = QStringLiteral("proof/checkAsYouType");
    if (app.value(marker).toBool()) return;
    // Off in the old key of JeffCrunch or JeffWrite: off in the shared one, unless a choice was made there already.
    if (!suite.contains(key))
        for (const char *old : {"proof/spellAsYouType", "proof/asYouType"})
            if (app.contains(QLatin1String(old)) && !app.value(QLatin1String(old)).toBool()) suite.setValue(key, false);
    app.setValue(marker, true);
}

SpellSettingNotifier &SpellSettingNotifier::get()
{
    static SpellSettingNotifier n;
    return n;
}

bool spellAsYouType() { return Settings::get().value(QStringLiteral("proof/checkAsYouType"), true).toBool(); }

void setSpellAsYouType(bool on)
{
    if (spellAsYouType() == on) return;
    Settings::get().setValue(QStringLiteral("proof/checkAsYouType"), on);
    Q_EMIT SpellSettingNotifier::get().changed(on);
}

Unit defaultUnit(const QLocale &region)
{
    return region.measurementSystem() == QLocale::ImperialUSSystem ? Unit::Inch : Unit::Centimeter;
}

bool usesLetterPaper(const QLocale &region)
{
    static const QList<QLocale::Territory> letter = {QLocale::UnitedStates, QLocale::Canada,     QLocale::Mexico,   QLocale::Philippines, QLocale::Colombia,
                                                     QLocale::Chile,        QLocale::Venezuela,  QLocale::CostaRica, QLocale::Guatemala,   QLocale::Panama,
                                                     QLocale::DominicanRepublic, QLocale::ElSalvador, QLocale::Nicaragua, QLocale::Honduras, QLocale::PuertoRico};
    return letter.contains(region.territory());
}

Settings::Settings()
    // (in the system's own store: QSettings::defaultFormat() is it, except in a test run, which keeps its settings in files of its own)
    : m_s(QSettings::defaultFormat(), QSettings::UserScope, appInfo().organization, appInfo().name), m_suite(QSettings::defaultFormat(), QSettings::UserScope, appInfo().organization, QStringLiteral("JeffOffice"))
{
    moveToSuite(m_s, m_suite);
    migrateSpellSetting(m_s, m_suite);
    m_unit = Unit(m_s.value("units", int(defaultUnit(QLocale::system()))).toInt());
}

void Settings::setUnit(Unit u)
{
    m_unit = u;
    m_s.setValue("units", int(u));
}

QString Settings::unitSuffix() const
{
    switch (m_unit) {
    case Unit::Inch: return QStringLiteral("\"");
    case Unit::Centimeter: return QStringLiteral(" cm");
    case Unit::Millimeter: return QStringLiteral(" mm");
    case Unit::Point: return QStringLiteral(" pt");
    case Unit::Pica: return QStringLiteral(" pi");
    }
    return {};
}

static double perPoint(Unit u)
{
    switch (u) {
    case Unit::Inch: return 1.0 / 72.0;
    case Unit::Centimeter: return 2.54 / 72.0;
    case Unit::Millimeter: return 25.4 / 72.0;
    case Unit::Point: return 1.0;
    case Unit::Pica: return 1.0 / 12.0;
    }
    return 1.0;
}

double Settings::toUnit(double pt) const { return pt * perPoint(m_unit); }
double Settings::fromUnit(double v) const { return v / perPoint(m_unit); }

QString Settings::format(double pt, int decimals) const
{
    QString n = QString::number(toUnit(pt), 'f', decimals);
    if (n.contains('.')) { while (n.endsWith('0')) n.chop(1); if (n.endsWith('.')) n.chop(1); }
    // with the computer's own decimal mark, as the number boxes beside it show theirs
    return n.replace(QLatin1Char('.'), QLocale().decimalPoint()) + unitSuffix();
}

QString Settings::formatPair(double x, double y) const
{
    const QString between = QLocale().decimalPoint() == QStringLiteral(",") ? QStringLiteral("; ") : QStringLiteral(", ");
    return format(x) + between + format(y);
}

bool Settings::parse(const QString &text, double *pt) const
{
    static const QRegularExpression re(QStringLiteral("^\\s*(-?[0-9]*\\.?[0-9]+)\\s*(\"|in|inch|inches|cm|mm|pt|pi|px)?\\s*$"),
                                       QRegularExpression::CaseInsensitiveOption);
    // the computer's decimal mark or a period, whichever was typed
    QString t = text;
    const QString mark = QLocale().decimalPoint();
    if (mark != QStringLiteral(".")) t.replace(mark, QStringLiteral("."));
    const auto m = re.match(t);
    if (!m.hasMatch()) return false;
    const double v = m.captured(1).toDouble();
    const QString u = m.captured(2).toLower();
    Unit unit = m_unit;
    if (u == "\"" || u.startsWith("in")) unit = Unit::Inch;
    else if (u == "cm") unit = Unit::Centimeter;
    else if (u == "mm") unit = Unit::Millimeter;
    else if (u == "pt") unit = Unit::Point;
    else if (u == "pi") unit = Unit::Pica;
    else if (u == "px") { *pt = v * 0.75; return true; }
    *pt = v / perPoint(unit);
    return true;
}

QStringList Settings::recentFiles() const { return m_s.value("recent").toStringList(); }

QJsonArray Settings::customPageSizes() const
{
    return QJsonDocument::fromJson(m_s.value("pageSizes/custom").toByteArray()).array();
}

void Settings::setCustomPageSizes(const QJsonArray &sizes)
{
    m_s.setValue("pageSizes/custom", QJsonDocument(sizes).toJson(QJsonDocument::Compact));
}

void Settings::addRecentFile(const QString &path)
{
    QStringList l = recentFiles();
    l.removeAll(path);
    l.prepend(path);
    const int keep = std::clamp(m_s.value("recentCount", 25).toInt(), 0, 50);   // (the range the Options box allows: a hand-edited file may hold any number)
    while (l.size() > keep) l.removeLast();
    m_s.setValue("recent", l);
}

void Settings::removeRecentFile(const QString &path)
{
    QStringList l = recentFiles();
    l.removeAll(path);
    m_s.setValue("recent", l);
}

QStringList Settings::pinnedFiles() const { return m_s.value("pinned").toStringList(); }

void Settings::setPinned(const QString &path, bool pinned)
{
    QStringList l = pinnedFiles();
    l.removeAll(path);
    if (pinned) l.prepend(path);
    m_s.setValue("pinned", l);
}

QString Settings::userName() const
{
    QString n = value(QStringLiteral("user/name")).toString();
    if (n.isEmpty()) n = qEnvironmentVariable("USER", qEnvironmentVariable("USERNAME"));
    return n;
}

} // namespace jp

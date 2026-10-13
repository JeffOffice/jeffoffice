#pragma once
// User preferences (stored with QSettings) and measurement units.

#include <QJsonArray>
#include <QLocale>
#include <QSettings>
#include <QString>
#include <QStringList>

namespace jp {

enum class Unit { Inch, Centimeter, Millimeter, Point, Pica };

// The unit a region measures pages in, until Options sets one: inches where
// it measures in inches (the United States), else centimeters, as Office does.
Unit defaultUnit(const QLocale &region);

// For an app that kept its settings and files under another name: copies
// the keys `to` lacks, and moves the folder unless `to` already exists.
void copyMissingSettings(const QSettings &from, QSettings &to);
bool moveDataFolder(const QString &from, const QString &to);

// Whether a setting belongs to the whole suite rather than one app: the
// theme, the language, the name and initials of the person using it, the
// updates (setup installs every app at once), whether to send statistics,
// and the random id those statistics are sent under. Those are kept in the
// suite's own store, so a choice made in one app holds in all of them;
// everything else stays with the app (the statistics' counts too).
bool suiteWide(const QString &key);
// Copies the suite-wide settings an app kept itself (before there was a
// suite store) that the suite store doesn't have yet (setup may already have
// written the statistics choice there), the first time. The app's copies
// stay, for an older version that reads them.
void moveToSuite(QSettings &app, QSettings &suite);

class Settings {
public:
    static Settings &get();

    Unit unit() const { return m_unit; }
    void setUnit(Unit u);
    QString unitSuffix() const;
    double toUnit(double pt) const;
    double fromUnit(double v) const;
    QString format(double pt, int decimals = 3) const;   // "8.125\"", trailing zeros dropped
    bool parse(const QString &text, double *pt) const;   // accepts any unit suffix

    QStringList recentFiles() const;
    void addRecentFile(const QString &path);
    void removeRecentFile(const QString &path);
    QStringList pinnedFiles() const;
    void setPinned(const QString &path, bool pinned);

    // Page sizes the user created: each {"name", "setup" (a page setup)}.
    QJsonArray customPageSizes() const;
    void setCustomPageSizes(const QJsonArray &sizes);

    // From the app's store, or the suite's for a suite-wide key.
    QVariant value(const QString &key, const QVariant &def = QVariant()) const { return store(key).value(key, def); }
    void setValue(const QString &key, const QVariant &v) { store(key).setValue(key, v); }

    // Writes what's pending and reads what another app changed meanwhile
    // (a choice made in one stays unseen by a running other until then).
    void sync() { m_s.sync(); m_suite.sync(); }

    QString userName() const;
    int autoRecoverMinutes() const { return m_s.value("save/autoRecoverMinutes", 10).toInt(); }
    double nudge() const { return m_s.value("edit/nudge", 9.0).toDouble(); }   // points (0.125 in)

private:
    Settings();
    QSettings &store(const QString &key) { return suiteWide(key) ? m_suite : m_s; }
    const QSettings &store(const QString &key) const { return suiteWide(key) ? m_suite : m_s; }
    QSettings m_s;       // this app's
    QSettings m_suite;   // the whole suite's
    Unit m_unit = Unit::Inch;
};

} // namespace jp

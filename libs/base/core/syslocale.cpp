#include "core/syslocale.h"

#include <QCoreApplication>
#include <QEvent>
#include <QMetaObject>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>

#include <memory>

namespace jp {

namespace {

QMutex g_lock;
std::shared_ptr<const SystemLocale> g_copy;

bool onMainThread()
{
    const QCoreApplication *app = QCoreApplication::instance();
    return !app || QThread::currentThread() == app->thread();
}

// The one place that asks the system. (Main thread.)
std::shared_ptr<const SystemLocale> ask()
{
    auto out = std::make_shared<SystemLocale>();
    const QLocale now;   // (the default locale: the system's, unless a test chose another)
    out->locale = now.language() == QLocale::AnyLanguage || now.language() == QLocale::C ? QLocale::c() : QLocale(now.language(), now.script(), now.territory());
    out->shortDate = now.dateFormat(QLocale::ShortFormat);
    out->longDate = now.dateFormat(QLocale::LongFormat);
    out->shortTime = now.timeFormat(QLocale::ShortFormat);
    out->longTime = now.timeFormat(QLocale::LongFormat);
    out->shortDateTime = now.dateTimeFormat(QLocale::ShortFormat);
    out->longDateTime = now.dateTimeFormat(QLocale::LongFormat);
    out->decimalPoint = now.decimalPoint();
    out->groupSeparator = now.groupSeparator();
    out->uiLanguages = now.uiLanguages();
    return out;
}

// What the person's signs do to a number the made locale wrote.
QString withSigns(const SystemLocale &s, QString text)
{
    const QString decimal = s.locale.decimalPoint(), group = s.locale.groupSeparator();
    if (decimal == s.decimalPoint && group == s.groupSeparator) return text;
    // (through a placeholder, so that swapped signs, a comma and a point, do not undo each other)
    const QChar mark(0xE000), mark2(0xE001);
    if (!group.isEmpty()) text.replace(group, QString(mark2));
    if (!decimal.isEmpty()) text.replace(decimal, QString(mark));
    text.replace(mark, s.decimalPoint);
    text.replace(mark2, s.groupSeparator);
    return text;
}

// What a worker gets before the main thread has asked: the C locale's own ways, never the system's.
std::shared_ptr<const SystemLocale> uncaptured()
{
    static const std::shared_ptr<const SystemLocale> none = [] {
        auto out = std::make_shared<SystemLocale>();
        out->locale = QLocale::c();
        out->shortDate = QStringLiteral("yyyy-MM-dd");
        out->longDate = QStringLiteral("dddd, MMMM d, yyyy");
        out->shortTime = QStringLiteral("HH:mm");
        out->longTime = QStringLiteral("HH:mm:ss");
        out->shortDateTime = out->shortDate + QLatin1Char(' ') + out->shortTime;
        out->longDateTime = out->longDate + QLatin1Char(' ') + out->longTime;
        out->decimalPoint = QStringLiteral(".");
        out->groupSeparator = QStringLiteral(",");
        return std::shared_ptr<const SystemLocale>(out);
    }();
    return none;
}

// (the language, script, and territory of the default are fixed numbers, not a question to the system: cheap enough for every call on the main thread)
bool same(const SystemLocale &a, const QLocale &now)
{
    const QLocale &had = a.locale;
    return (had.language() == now.language() || (had.language() == QLocale::C && now.language() == QLocale::AnyLanguage)) && had.script() == now.script() && had.territory() == now.territory();
}

// The system's own change of settings (a new regional format) is a LocaleChange event to the application; the copy is asked again once Qt has taken it in.
class ChangeWatch : public QObject {
public:
    explicit ChangeWatch(QObject *parent) : QObject(parent) {}
    bool eventFilter(QObject *, QEvent *event) override
    {
        if (event->type() == QEvent::LocaleChange) QMetaObject::invokeMethod(this, [] { captureSystemLocale(); }, Qt::QueuedConnection);
        return false;
    }
};

} // namespace

void captureSystemLocale()
{
    if (!onMainThread()) return;   // (a worker never asks; the copy it has is the main thread's)
    auto made = ask();
    {
        const QMutexLocker lock(&g_lock);
        g_copy = std::move(made);
    }
    static bool watching = false;
    if (!watching && QCoreApplication::instance()) {
        watching = true;
        QCoreApplication::instance()->installEventFilter(new ChangeWatch(QCoreApplication::instance()));
    }
}

SystemLocale systemLocale()
{
    std::shared_ptr<const SystemLocale> held;
    {
        const QMutexLocker lock(&g_lock);
        held = g_copy;
    }
    if (onMainThread()) {
        if (!held || !same(*held, QLocale())) {
            captureSystemLocale();
            const QMutexLocker lock(&g_lock);
            held = g_copy;
        }
    } else if (!held) {
        held = uncaptured();
    }
    return *held;
}

QString SystemLocale::toString(const QDate &date, QLocale::FormatType type) const { return locale.toString(date, type == QLocale::ShortFormat ? shortDate : longDate); }

QString SystemLocale::toString(const QTime &time, QLocale::FormatType type) const { return locale.toString(time, type == QLocale::ShortFormat ? shortTime : longTime); }

QString SystemLocale::toString(const QDateTime &when, QLocale::FormatType type) const { return locale.toString(when, type == QLocale::ShortFormat ? shortDateTime : longDateTime); }

QString SystemLocale::toString(qint64 number) const { return withSigns(*this, locale.toString(number)); }

QString SystemLocale::toString(double number, char format, int precision) const { return withSigns(*this, locale.toString(number, format, precision)); }

} // namespace jp

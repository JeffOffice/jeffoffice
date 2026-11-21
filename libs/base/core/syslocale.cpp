#include "core/syslocale.h"

#include <QCoreApplication>
#include <QEvent>
#include <QMetaObject>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>

#include <atomic>
#include <memory>

namespace jp {

namespace {

QMutex g_lock;
std::shared_ptr<const SystemLocale> g_copy;
QThread *g_mainThread = nullptr;   // (the thread that captured first when there is no application object: a test without one)
std::atomic<qint64> g_uncapturedReads{0};

// The main thread is the application's. With no application object (a test) it is the thread that made the first, explicit capture; before that, no thread is.
bool onMainThread()
{
    const QCoreApplication *app = QCoreApplication::instance();
    if (app) return QThread::currentThread() == app->thread();
    const QMutexLocker lock(&g_lock);
    return g_mainThread && QThread::currentThread() == g_mainThread;
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
    out->currencySign = now.currencySymbol(QLocale::CurrencySymbol);
    out->amText = now.amText();
    out->pmText = now.pmText();
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
        out->currencySign = QStringLiteral("$");
        out->amText = QStringLiteral("AM");
        out->pmText = QStringLiteral("PM");
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

bool sameText(const SystemLocale &a, const SystemLocale &b)
{
    return a.shortDate == b.shortDate && a.longDate == b.longDate && a.shortTime == b.shortTime && a.longTime == b.longTime && a.shortDateTime == b.shortDateTime
        && a.longDateTime == b.longDateTime && a.decimalPoint == b.decimalPoint && a.groupSeparator == b.groupSeparator && a.currencySign == b.currencySign && a.amText == b.amText
        && a.pmText == b.pmText;
}

// The system's own change of settings (a new regional format) is a LocaleChange event to the application; the copy is asked again once Qt has taken it in.
class ChangeWatch : public QObject {
public:
    explicit ChangeWatch(QObject *parent) : QObject(parent) {}
    bool eventFilter(QObject *, QEvent *event) override
    {
        if (event->type() == QEvent::LocaleChange) QMetaObject::invokeMethod(this, [] { captureSystemLocale(); }, Qt::QueuedConnection);
        // (a format set in the system's settings while the program ran may come with no LocaleChange: the strings are compared again when the program is brought to the front)
        if (event->type() == QEvent::ApplicationActivate) QMetaObject::invokeMethod(this, [] { refreshSystemLocaleIfChanged(); }, Qt::QueuedConnection);
        return false;
    }
};

} // namespace

void captureSystemLocale()
{
    if (!QCoreApplication::instance()) {   // (no application object: the first explicit capture names the main thread)
        const QMutexLocker lock(&g_lock);
        if (!g_mainThread) g_mainThread = QThread::currentThread();
    }
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

void refreshSystemLocaleIfChanged()
{
    if (!onMainThread()) return;
    std::shared_ptr<const SystemLocale> held;
    {
        const QMutexLocker lock(&g_lock);
        held = g_copy;
    }
    auto made = ask();
    if (held && same(*held, made->locale) && sameText(*held, *made)) return;
    const QMutexLocker lock(&g_lock);
    g_copy = std::move(made);
}

qint64 uncapturedSystemLocaleReadsForTest() { return g_uncapturedReads.load(); }

void forgetSystemLocaleForTest()
{
    const QMutexLocker lock(&g_lock);
    g_copy.reset();
    g_mainThread = nullptr;
}

void setSystemLocaleForTest(const SystemLocale &person)
{
    const QMutexLocker lock(&g_lock);
    if (!QCoreApplication::instance() && !g_mainThread) g_mainThread = QThread::currentThread();
    g_copy = std::make_shared<const SystemLocale>(person);
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
        ++g_uncapturedReads;   // (no capture yet, or not the main thread: the fixed C-like default, never an ask of the system)
        held = uncaptured();
    }
    return *held;
}

QString SystemLocale::toString(const QDate &date, QLocale::FormatType type) const { return locale.toString(date, type == QLocale::ShortFormat ? shortDate : longDate); }

QString SystemLocale::toString(const QTime &time, QLocale::FormatType type) const { return locale.toString(time, type == QLocale::ShortFormat ? shortTime : longTime); }

QString SystemLocale::toString(const QDateTime &when, QLocale::FormatType type) const { return locale.toString(when, type == QLocale::ShortFormat ? shortDateTime : longDateTime); }

QString SystemLocale::toString(qint64 number) const { return withSigns(*this, locale.toString(number)); }

QString SystemLocale::toString(double number, char format, int precision) const { return withSigns(*this, locale.toString(number, format, precision)); }

QString SystemLocale::formattedDataSize(qint64 bytes, int precision, QLocale::DataSizeFormats format) const { return withSigns(*this, locale.formattedDataSize(bytes, precision, format)); }

} // namespace jp

// The person's locale on a worker thread (libs/base/core/syslocale.h): the system is asked on the main thread alone, a worker takes the copy, and a worker formats a date or a number as the
// main thread does, in the person's own format strings and signs. A stand-in for the system (Qt's QSystemLocale) answers and counts the questions, and the thread each came from.
#include "odshelpers.h"

#include "../testrunner.h"
#include "core/syslocale.h"
#include "calc/numfmt.h"
#include "core/workthread.h"
#include "sheet/styles.h"

#include <QLocale>
#include <QThread>

#include <private/qlocale_p.h>

#include <atomic>

using namespace jp;

namespace {

// The system's stand-in: counts the questions it is asked, and those from a thread other than the one that made it (the main thread).
class Watching : public QSystemLocale {
public:
    Watching() : m_main(QThread::currentThread()) {}
    QVariant query(QueryType type, QVariant &&) const override
    {
        ++asked;
        if (QThread::currentThread() != m_main) ++offMain;
        return answer(type);
    }
    mutable std::atomic<int> asked{0}, offMain{0};

protected:
    virtual QVariant answer(QueryType) const { return QVariant(); }   // (nothing of its own: Qt's own data)

private:
    QThread *m_main;
};

// A person who set their own short date, time, and signs in the system's settings.
class Custom : public Watching {
protected:
    QVariant answer(QueryType type) const override
    {
        switch (type) {
        case DateFormatShort: return QStringLiteral("dd.MM.yy");
        case TimeFormatShort: return QStringLiteral("H.mm");
        case DecimalPoint: return QStringLiteral(",");
        case GroupSeparator: return QStringLiteral(".");
        default: return QVariant();
        }
    }
};

// The default locale is the system's for the length of a test, and what it was before is put back.
struct SystemDefault {
    QLocale before;
    SystemDefault() : before(QLocale()) { QLocale::setDefault(QLocale::system()); }
    ~SystemDefault()
    {
        QLocale::setDefault(before);
        captureSystemLocale();
    }
};

// The code of the built-in short date (format 14) as the workbook's styles say it.
QString shortDateCode()
{
    jp::office::sheet::Styles styles;
    styles.cellXfs << 14;
    return styles.formatCode(0);
}

template <typename F>
void onWorker(F &&f)
{
    WorkThread t(std::forward<F>(f));
    t.join();
}

} // namespace

class SheetSystemLocaleTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Opening a file of values, reading a number format, and every way of writing a date or a number on a worker ask the system nothing.
    void aWorkerNeverAsksTheSystem()
    {
        Watching system;
        const SystemDefault scope;
        QLocale().toString(QDate(2026, 11, 20), QLocale::LongFormat);
        if (system.asked == 0) QSKIP("this platform's default locale does not ask a system stand-in");
        captureSystemLocale();
        system.asked = 0;
        system.offMain = 0;
        bool opened = false;
        QString text;
        onWorker([&] {
            Workbook wb;
            QString why;
            opened = odstest::openBytes("a,b\n1.5,2\n", QStringLiteral("a.csv"), &wb, &why);
            const SystemLocale person = systemLocale();
            text = shortDateCode() + person.toString(QDate(2026, 11, 20), QLocale::LongFormat) + person.toString(QTime(13, 5), QLocale::ShortFormat)
                   + person.toString(QDateTime(QDate(2026, 11, 20), QTime(13, 5)), QLocale::LongFormat) + person.toString(1234567) + person.toString(2.5) + person.locale.monthName(3)
                   + person.locale.dateFormat() + workerLocale().toString(12.5) + fixedText(QDateTime(QDate(2026, 11, 20), QTime(13, 5)), QStringLiteral("yyyy-MM-dd'T'HH:mm:ss'Z'"));
        });
        QVERIFY(opened);
        QVERIFY2(text.endsWith(QStringLiteral("2026-11-20T13:05:00Z")), qPrintable(text));
        QCOMPARE(system.offMain.load(), 0);
    }

    // Before the fix a worker's open of a file of values asked the system from its own thread: this is the question that was asked there.
    void aDefaultLocaleOnAWorkerIsAskedOfTheSystem()
    {
        Watching system;
        const SystemDefault scope;
        QLocale().toString(QDate(2026, 11, 20), QLocale::LongFormat);
        if (system.asked == 0) QSKIP("this platform's default locale does not ask a system stand-in");
        system.offMain = 0;
        onWorker([] { QLocale().toString(QDate(2026, 11, 20), QLocale::LongFormat); });
        QVERIFY(system.offMain.load() > 0);
    }

    // A person's own short date and signs: a date and a number written on a worker are the ones the main thread writes.
    void aWorkerWritesWhatTheMainThreadWritesInTheCustomFormats()
    {
        Custom system;
        const SystemDefault scope;
        captureSystemLocale();
        const QDate day(2026, 11, 20);
        const QString onMain = QLocale().toString(day, QLocale::ShortFormat);
        if (onMain != QStringLiteral("20.11.26")) QSKIP("this platform's default locale does not take a system stand-in's formats");
        const QString timeMain = QLocale().toString(QTime(13, 5), QLocale::ShortFormat), numberMain = QLocale().toString(qint64(1234567)), decimalMain = QLocale().toString(2.5);
        QString dateWorker, timeWorker, numberWorker, decimalWorker, codeWorker;
        onWorker([&] {
            const SystemLocale person = systemLocale();
            dateWorker = person.toString(day, QLocale::ShortFormat);
            timeWorker = person.toString(QTime(13, 5), QLocale::ShortFormat);
            numberWorker = person.toString(qint64(1234567));
            decimalWorker = person.toString(2.5);
            codeWorker = shortDateCode();
        });
        QCOMPARE(dateWorker, onMain);
        QCOMPARE(timeWorker, timeMain);
        QCOMPARE(numberWorker, numberMain);
        QCOMPARE(decimalWorker, decimalMain);
        QCOMPARE(codeWorker, jp::calc::builtinFormatCode(14, QLocale()));   // (the file format's built-in short date, in the person's own order)
        QVERIFY2(codeWorker.startsWith(QLatin1String("dd")), qPrintable(codeWorker));
        QCOMPARE(system.offMain.load(), 0);
    }

    // The main thread changes the locale 2,000 times while a worker formats 2,000 times: every text the worker made is one whole copy's (never an English format with another's signs),
    // and the system is asked of nothing from the worker.
    void theMainThreadChangingTheLocaleDoesNotReachAWorker()
    {
        Watching system;
        const SystemDefault scope;
        const QLocale english(QLocale::English, QLocale::UnitedStates), german(QLocale::German, QLocale::Germany);
        const QDate day(2026, 11, 20);
        QLocale::setDefault(english);
        captureSystemLocale();
        const QString inEnglish = systemLocale().toString(day, QLocale::LongFormat) + systemLocale().toString(1234567.5, 'f', 1);
        QLocale::setDefault(german);
        captureSystemLocale();
        const QString inGerman = systemLocale().toString(day, QLocale::LongFormat) + systemLocale().toString(1234567.5, 'f', 1);
        QVERIFY(inEnglish != inGerman);
        system.offMain = 0;
        std::atomic<int> made{0}, mixed{0};
        WorkThread worker([&] {
            for (int i = 0; i < 2000; ++i) {
                const SystemLocale person = systemLocale();
                const QString text = person.toString(day, QLocale::LongFormat) + person.toString(1234567.5, 'f', 1);
                if (text != inEnglish && text != inGerman) ++mixed;
                ++made;
            }
        });
        for (int i = 0; i < 2000; ++i) {
            QLocale::setDefault(i % 2 ? german : english);
            captureSystemLocale();
            systemLocale();
        }
        worker.join();
        QCOMPARE(made.load(), 2000);
        QCOMPARE(mixed.load(), 0);
        QCOMPARE(system.offMain.load(), 0);
    }

    // On the main thread the copy follows the default locale at once (a test that sets it sees its own choice, as QLocale() did).
    void theMainThreadSeesTheDefaultLocaleAtOnce()
    {
        const SystemDefault scope;
        QLocale::setDefault(QLocale(QLocale::German, QLocale::Germany));
        QCOMPARE(systemLocale().locale.language(), QLocale::German);
        QCOMPARE(systemLocale().toString(1234567.5, 'f', 1), QStringLiteral("1.234.567,5"));
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
        QCOMPARE(systemLocale().toString(1234567.5, 'f', 1), QStringLiteral("1,234,567.5"));
    }
};

JP_TEST_CLASS(SheetSystemLocaleTests)
#include "tst_sheet_syslocale.moc"

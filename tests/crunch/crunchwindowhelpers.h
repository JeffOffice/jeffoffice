// What the tests of JeffCrunch's window share: a window with its message box standing in, a meter of how long the event loop was held up, a count of paints, and a workbook of numbers
// with formulas.
#pragma once

#include "app/crunchwindow.h"
#include "core/threadtime.h"
#include "crunchbooks.h"

#include <QSignalSpy>
#include <QTimer>

namespace crunchtest {

using namespace jp;

// A window with the notifier standing in for message boxes: what it was told is kept.
struct Window {
    CrunchWindow w;
    QStringList told;
    Window()
    {
        w.setNotifier([this](const QString &title, const QString &text) { told << title + QStringLiteral(": ") + text; });
        w.resize(1100, 640);
        w.show();
    }
    // Opens a workbook and waits until it is read and every formula has its value.
    bool open(const QString &path)
    {
        QSignalSpy first(&w, &CrunchWindow::firstScreenShown), done(&w, &CrunchWindow::calculationFinished), loaded(&w, &CrunchWindow::loadFinished);
        if (!w.openFile(path)) return false;
        return first.wait(20000) && (loaded.count() > 0 || loaded.wait(20000)) && (done.count() > 0 || done.wait(20000));
    }
};

// How long the program's event loop did not answer, by the processor time of its thread (which counts only the time the thread was working, not the time it was waiting, or the
// machine was another program's): a timer of a millisecond ticks whenever the loop gets to it, and the most work the thread did between two ticks is the longest the loop was held up
// (whatever it was busy with: a paint, a batch of values, a slice of anything).
class LoopMeter : public QObject {
public:
    LoopMeter()
    {
        m_tick.setInterval(1);
        connect(&m_tick, &QTimer::timeout, this, [this] {
            const qint64 now = jp::threadCpuMicros();
            if (m_last) m_longest = std::max(m_longest, now - m_last);
            m_last = now;
        });
        m_last = jp::threadCpuMicros();
        m_tick.start();
    }
    qint64 longestMicros() const { return m_longest; }
    void reset()
    {
        m_longest = 0;
        m_last = jp::threadCpuMicros();
    }

private:
    QTimer m_tick;
    qint64 m_last = 0, m_longest = 0;
};

// The paints of a widget: how many, and the area they drew.
class PaintCounter : public QObject {
public:
    explicit PaintCounter(QWidget *w) : QObject(w) { w->installEventFilter(this); }
    int paints = 0;
    qint64 area = 0;
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (e->type() == QEvent::Paint) {
            ++paints;
            const QRect r = static_cast<QPaintEvent *>(e)->rect();
            area += qint64(r.width()) * r.height();
        }
        return false;
    }
};

// A workbook of `rows` rows with a number (the row's) in A and twice it in B, and the formulas in D1, D2, ... (those given), each with `stored` for the result the file kept.
inline QByteArray rowsWithFormulas(int rows, const QStringList &formulas, const QString &stored = QStringLiteral("-1"), int firstFormulaRow = 1)
{
    QString xml;
    xml.reserve(rows * 70);
    for (int r = 1; r <= rows; ++r) {
        xml += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>%2</v></c>").arg(r).arg(r * 2);
        const int k = r - firstFormulaRow;
        if (k >= 0 && k < formulas.size()) xml += QStringLiteral("<c r=\"D%1\"><f>%2</f><v>%3</v></c>").arg(r).arg(formulas[k].toHtmlEscaped(), stored);
        xml += QStringLiteral("</row>");
    }
    return buildXlsx({worksheet(xml)});
}


} // namespace crunchtest

#pragma once
// The small toolbar of Rehearse Timings, at the top left of the slide show:
// Next (the same as a click), Pause (the clocks stop until it is pressed
// again), the time on this slide, Repeat (the slide's clock starts again), and
// the total time. The show keeps the clocks (SlideShow::slideClock and
// totalClock); this shows them.

#include <QPointer>
#include <QTimer>
#include <QWidget>

class QAbstractButton;
class QLabel;

namespace jp {

class SlideShow;

class RehearseBar : public QWidget {
    Q_OBJECT
public:
    explicit RehearseBar(SlideShow *show);

    // A time as the bar shows it: "0:00:05", "1:02:03" (hours, minutes, seconds).
    static QString clockText(qint64 ms);
    QString slideText() const;
    QString totalText() const;

public Q_SLOTS:
    // Reads the clocks again.
    void refresh();

private:
    QAbstractButton *button(const QString &name, const QString &icon, const QString &tip);
    QPointer<SlideShow> m_show;
    QAbstractButton *m_pause = nullptr;
    QLabel *m_slide = nullptr, *m_total = nullptr;
    QTimer m_tick;
};

} // namespace jp

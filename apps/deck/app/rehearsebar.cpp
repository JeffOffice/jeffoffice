#include "app/rehearsebar.h"

#include "app/icons.h"
#include "app/slideshow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include "app/plaintext.h"

namespace jp {

RehearseBar::RehearseBar(SlideShow *show) : QWidget(show), m_show(show)
{
    setObjectName(QStringLiteral("rehearseBar"));
    setAccessibleName(tr("Rehearse Timings"));
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet(QStringLiteral("#rehearseBar { background: %1; border: 1px solid %2; border-radius: 6px; } #rehearseBar QLabel { color: %3; }")
                      .arg(uiPanel().name(), uiLine().name(), uiText().name()));
    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(8, 4, 8, 4);
    row->setSpacing(6);
    QAbstractButton *next = button(QStringLiteral("rehearseNext"), QStringLiteral("arrow-right"), tr("Next"));
    m_pause = button(QStringLiteral("rehearsePause"), QStringLiteral("pause"), tr("Pause Recording"));
    m_slide = plainLabel(new QLabel(QStringLiteral("0:00:00"), this));
    m_slide->setObjectName(QStringLiteral("rehearseSlideTime"));
    m_slide->setAccessibleName(tr("Time on this slide"));
    m_slide->setMinimumWidth(64);
    m_slide->setAlignment(Qt::AlignCenter);
    QAbstractButton *repeat = button(QStringLiteral("rehearseRepeat"), QStringLiteral("rotate-ccw"), tr("Repeat the slide: its time starts again"));
    m_total = plainLabel(new QLabel(QStringLiteral("0:00:00"), this));
    m_total->setObjectName(QStringLiteral("rehearseTotalTime"));
    m_total->setAccessibleName(tr("Total time"));
    m_total->setMinimumWidth(64);
    m_total->setAlignment(Qt::AlignCenter);
    QFont dim = m_total->font();
    dim.setPointSizeF(dim.pointSizeF() * 0.9);
    m_total->setFont(dim);
    m_total->setToolTip(tr("Total time"));
    m_slide->setToolTip(tr("Time on this slide"));
    row->addWidget(next);
    row->addWidget(m_pause);
    row->addWidget(m_slide);
    row->addWidget(repeat);
    row->addSpacing(4);
    row->addWidget(m_total);
    adjustSize();
    connect(next, &QAbstractButton::clicked, this, [this] { if (m_show) m_show->next(); });
    connect(m_pause, &QAbstractButton::clicked, this, [this] {
        if (!m_show) return;
        m_show->pauseRecording(!m_show->isRecordingPaused());
        m_pause->setIcon(icon(m_show->isRecordingPaused() ? QStringLiteral("play") : QStringLiteral("pause")));
        m_pause->setToolTip(m_show->isRecordingPaused() ? tr("Resume Recording") : tr("Pause Recording"));
        refresh();
    });
    connect(repeat, &QAbstractButton::clicked, this, [this] {
        if (m_show) m_show->repeatSlide();
        refresh();
    });
    connect(show, &SlideShow::changed, this, &RehearseBar::refresh);
    m_tick.setInterval(200);
    connect(&m_tick, &QTimer::timeout, this, &RehearseBar::refresh);
    m_tick.start();
}

QAbstractButton *RehearseBar::button(const QString &name, const QString &iconName, const QString &tip)
{
    auto *b = new QToolButton(this);
    b->setObjectName(name);
    b->setIcon(icon(iconName));
    b->setIconSize(QSize(20, 20));
    b->setToolTip(jp::plainToolTip(tip));
    b->setAccessibleName(tip);
    b->setAutoRaise(true);
    b->setFocusPolicy(Qt::NoFocus);   // keys go to the show, not to a button
    return b;
}

QString RehearseBar::clockText(qint64 ms)
{
    const qint64 seconds = std::max<qint64>(0, ms) / 1000;
    return QStringLiteral("%1:%2:%3").arg(seconds / 3600).arg(seconds / 60 % 60, 2, 10, QLatin1Char('0')).arg(seconds % 60, 2, 10, QLatin1Char('0'));
}

QString RehearseBar::slideText() const { return m_slide->text(); }
QString RehearseBar::totalText() const { return m_total->text(); }

void RehearseBar::refresh()
{
    if (!m_show) return;
    if (const QString t = clockText(m_show->slideClock()); t != m_slide->text()) m_slide->setText(t);
    if (const QString t = clockText(m_show->totalClock()); t != m_total->text()) m_total->setText(t);
}

} // namespace jp

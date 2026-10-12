#include "app/presenterview.h"

#include "app/settings.h"
#include "app/slideshow.h"
#include "core/deck.h"
#include "core/document.h"

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDateTime>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QMouseEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace jp {

namespace {
constexpr int kMinNotes = 10, kMaxNotes = 72;
const QColor kBars(0x10, 0x10, 0x10);
}

// A slide drawn to fill the widget, or the end of the show.
class SlideView : public QWidget {
public:
    SlideView(SlideShow *show, const QString &endText, QWidget *parent) : QWidget(parent), m_show(show), m_endText(endText)
    {
        setMinimumSize(160, 90);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAttribute(Qt::WA_OpaquePaintEvent);
    }
    int slide() const { return m_slide; }
    void setSlide(int slide)
    {
        if (slide == m_slide) return;
        m_slide = slide;
        m_image = QImage();
        update();
    }
    QImage image()
    {
        if (!m_show) return m_image;
        const QSize wanted = size().expandedTo(minimumSize());
        const qreal dpr = devicePixelRatioF();
        if (m_image.isNull() || m_size != wanted || m_ratio != dpr) {
            m_size = wanted;
            m_ratio = dpr;
            m_image = renderSlideImage(m_show->document(), m_show->cache(), m_slide, wanted, dpr, kBars);
            if (m_slide < 0) {
                QPainter p(&m_image);
                p.setPen(Qt::white);
                QFont f = font();
                f.setPixelSize(std::max(10, wanted.height() / 10));
                p.setFont(f);
                p.drawText(QRect(QPoint(0, 0), wanted), Qt::AlignCenter | Qt::TextWordWrap, m_endText);
            }
        }
        return m_image;
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.fillRect(rect(), kBars);
        p.drawImage(0, 0, image());
    }

private:
    QPointer<SlideShow> m_show;
    QString m_endText;
    int m_slide = -2;   // -1 is the end, -2 nothing drawn yet
    QImage m_image;
    QSize m_size;
    qreal m_ratio = 1;
};

PresenterView::PresenterView(SlideShow *show) : QWidget(nullptr, Qt::Window), m_show(show)
{
    setObjectName(QStringLiteral("presenterView"));
    setWindowTitle(tr("Presenter View"));
    setAccessibleName(tr("Presenter View"));
    setFocusPolicy(Qt::StrongFocus);
    resize(1100, 700);
    setStyleSheet(QStringLiteral(
        "QWidget#presenterView { background: #1c1c1c; }"
        "#presenterView QLabel { color: #f0f0f0; }"
        "#presenterView QPushButton { background: #3b3b3b; color: #f0f0f0; border: 1px solid #5a5a5a; border-radius: 4px; padding: 6px 14px; }"
        "#presenterView QPushButton:hover { background: #4a4a4a; }"
        "#presenterView QPushButton:checked { background: #b6420f; border-color: #b6420f; }"
        "#presenterView QPushButton:disabled { color: #888888; }"
        "#presenterView QPlainTextEdit, #presenterView QListWidget { background: #262626; color: #f4f4f4; border: 1px solid #444444; }"
        "#presenterView QListWidget::item { color: #f0f0f0; padding: 4px; }"
        "#presenterView QListWidget::item:selected { background: #b6420f; }"));

    m_notesSize = std::clamp(Settings::get().value(QStringLiteral("show/notesSize"), 20).toInt(), kMinNotes, kMaxNotes);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(10);

    auto *top = new QHBoxLayout;
    m_timerLabel = new QLabel(QStringLiteral("00:00"), this);
    m_timerLabel->setObjectName(QStringLiteral("timerLabel"));
    m_timerLabel->setAccessibleName(tr("Time since the slide show started"));
    QFont big = m_timerLabel->font();
    big.setPointSize(28);
    big.setBold(true);
    m_timerLabel->setFont(big);
    m_pause = button(QStringLiteral("pauseButton"), tr("Pause"));
    QAbstractButton *reset = button(QStringLiteral("resetButton"), tr("Reset"));
    m_clock = new QLabel(this);
    m_clock->setObjectName(QStringLiteral("clockLabel"));
    m_clock->setAccessibleName(tr("Time of day"));
    m_clock->setFont(big);
    top->addWidget(m_timerLabel);
    top->addSpacing(10);
    top->addWidget(m_pause);
    top->addWidget(reset);
    top->addStretch(1);
    top->addWidget(m_clock);
    root->addLayout(top);

    m_pages = new QStackedWidget(this);
    auto *main = new QWidget(m_pages);
    auto *mainLayout = new QHBoxLayout(main);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(14);
    m_current = new SlideView(show, tr("End of slide show"), main);
    m_current->setObjectName(QStringLiteral("currentSlide"));
    m_current->setAccessibleName(tr("Current slide"));
    m_current->installEventFilter(this);
    mainLayout->addWidget(m_current, 3);
    auto *side = new QVBoxLayout;
    side->setSpacing(6);
    auto *nextLabel = new QLabel(tr("Next"), main);
    nextLabel->setObjectName(QStringLiteral("nextLabel"));
    QFont heading = nextLabel->font();
    heading.setPointSize(14);
    heading.setBold(true);
    nextLabel->setFont(heading);
    m_nextView = new SlideView(show, tr("End of slide show"), main);
    m_nextView->setObjectName(QStringLiteral("nextSlide"));
    m_nextView->setAccessibleName(tr("Next slide"));
    auto *notesRow = new QHBoxLayout;
    auto *notesLabel = new QLabel(tr("Notes"), main);
    notesLabel->setFont(heading);
    QAbstractButton *smaller = button(QStringLiteral("notesSmaller"), tr("A-"));
    QAbstractButton *bigger = button(QStringLiteral("notesBigger"), tr("A+"));
    smaller->setToolTip(tr("Smaller text"));
    bigger->setToolTip(tr("Bigger text"));
    smaller->setAccessibleName(tr("Smaller text"));
    bigger->setAccessibleName(tr("Bigger text"));
    notesRow->addWidget(notesLabel);
    notesRow->addStretch(1);
    notesRow->addWidget(smaller);
    notesRow->addWidget(bigger);
    m_notes = new QPlainTextEdit(main);
    m_notes->setObjectName(QStringLiteral("notesText"));
    m_notes->setAccessibleName(tr("Notes"));
    m_notes->setReadOnly(true);
    m_notes->setFocusPolicy(Qt::NoFocus);
    side->addWidget(nextLabel);
    side->addWidget(m_nextView, 2);
    side->addLayout(notesRow);
    side->addWidget(m_notes, 3);
    mainLayout->addLayout(side, 2);
    m_pages->addWidget(main);
    m_grid = new QListWidget(m_pages);
    m_grid->setObjectName(QStringLiteral("slideGrid"));
    m_grid->setAccessibleName(tr("All slides"));
    m_grid->setFocusPolicy(Qt::NoFocus);
    m_grid->setViewMode(QListView::IconMode);
    m_grid->setIconSize(QSize(224, 126));
    m_grid->setResizeMode(QListView::Adjust);
    m_grid->setMovement(QListView::Static);
    m_grid->setSpacing(10);
    m_pages->addWidget(m_grid);
    root->addWidget(m_pages, 1);

    auto *bottom = new QHBoxLayout;
    QAbstractButton *previous = button(QStringLiteral("previousButton"), tr("Previous"));
    QAbstractButton *next = button(QStringLiteral("nextButton"), tr("Next"));
    m_counter = new QLabel(this);
    m_counter->setObjectName(QStringLiteral("slideCounter"));
    m_counter->setAlignment(Qt::AlignCenter);
    m_counter->setMinimumWidth(140);
    m_counter->setFont(heading);
    m_black = button(QStringLiteral("blackButton"), tr("Black Screen"));
    m_black->setCheckable(true);
    m_all = button(QStringLiteral("allSlidesButton"), tr("See All Slides"));
    m_all->setCheckable(true);
    QAbstractButton *end = button(QStringLiteral("endButton"), tr("End Slide Show"));
    bottom->addWidget(previous);
    bottom->addWidget(m_counter);
    bottom->addWidget(next);
    bottom->addStretch(1);
    bottom->addWidget(m_black);
    bottom->addWidget(m_all);
    bottom->addWidget(end);
    root->addLayout(bottom);

    connect(previous, &QAbstractButton::clicked, this, [this] { if (m_show) m_show->previous(); });
    connect(next, &QAbstractButton::clicked, this, [this] { if (m_show) m_show->next(); });
    connect(m_black, &QAbstractButton::clicked, this, [this] { if (m_show) m_show->toggleBlank(Qt::black); });
    connect(m_all, &QAbstractButton::clicked, this, [this](bool on) { showAllSlides(on); });
    connect(end, &QAbstractButton::clicked, this, [this] { if (m_show) m_show->close(); });
    connect(m_pause, &QAbstractButton::clicked, this, [this] { setPaused(!m_paused); });
    connect(reset, &QAbstractButton::clicked, this, &PresenterView::resetTimer);
    connect(smaller, &QAbstractButton::clicked, this, [this] { setNotesPointSize(m_notesSize - 2); });
    connect(bigger, &QAbstractButton::clicked, this, [this] { setNotesPointSize(m_notesSize + 2); });
    connect(m_grid, &QListWidget::itemClicked, this, [this](QListWidgetItem *it) { jumpTo(m_grid->row(it)); });

    connect(show, &SlideShow::changed, this, &PresenterView::refresh);
    connect(show, &SlideShow::ended, this, [this] {
        hide();
        deleteLater();
    });
    connect(show, &QObject::destroyed, this, [this] { deleteLater(); });
    m_tick.setInterval(200);
    connect(&m_tick, &QTimer::timeout, this, &PresenterView::tick);
    m_since.start();
    m_tick.start();
    setNotesPointSize(m_notesSize);
    refresh();
    tick();
}

QAbstractButton *PresenterView::button(const QString &name, const QString &text)
{
    auto *b = new QPushButton(text, this);
    b->setObjectName(name);
    b->setFocusPolicy(Qt::NoFocus);   // keys go to the show, not to a button
    return b;
}

// ---- following the show ----

void PresenterView::refresh()
{
    if (!m_show) return;
    const Document *doc = m_show->document();
    const int slide = m_show->targetSlide();
    m_next = slide >= 0 ? m_show->slideAfter(slide) : -1;
    m_current->setSlide(slide);
    m_nextView->setSlide(m_next);
    m_counter->setText(slide >= 0 ? tr("Slide %1 of %2").arg(slide + 1).arg(doc->pages.size()) : tr("End of slide show"));
    const QString notes = slide >= 0 ? deck::notesText(*doc, *doc->pages[slide]) : QString();
    if (notes != m_notes->toPlainText()) m_notes->setPlainText(notes);
    m_black->setChecked(m_show->blankColor() == QColor(Qt::black));
    if (isShowingAllSlides() && slide >= 0 && slide < m_grid->count()) m_grid->setCurrentRow(slide);
}

QString PresenterView::counterText() const { return m_counter->text(); }

QString PresenterView::notesText() const { return m_notes->toPlainText(); }

QImage PresenterView::nextImage() const { return m_nextView->image(); }

void PresenterView::setNotesPointSize(int pt)
{
    m_notesSize = std::clamp(pt, kMinNotes, kMaxNotes);
    QFont f = m_notes->font();
    f.setPointSize(m_notesSize);
    m_notes->setFont(f);
    Settings::get().setValue(QStringLiteral("show/notesSize"), m_notesSize);
}

// ---- the timer and the clock ----

qint64 PresenterView::timerElapsed() const { return m_paused ? m_before : m_before + m_since.elapsed(); }

void PresenterView::setTimerElapsed(qint64 ms)
{
    m_before = std::max<qint64>(0, ms);
    m_since.restart();
    tick();
}

void PresenterView::resetTimer() { setTimerElapsed(0); }

void PresenterView::setPaused(bool paused)
{
    if (paused == m_paused) return;
    if (paused) m_before = timerElapsed();
    else m_since.restart();
    m_paused = paused;
    m_pause->setText(paused ? tr("Resume") : tr("Pause"));
    tick();
}

QString PresenterView::timerText() const
{
    const qint64 seconds = timerElapsed() / 1000;
    const qint64 h = seconds / 3600, m = seconds / 60 % 60, s = seconds % 60;
    return h ? QStringLiteral("%1:%2:%3").arg(h).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0'))
             : QStringLiteral("%1:%2").arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0'));
}

void PresenterView::tick()
{
    if (const QString text = timerText(); text != m_timerLabel->text()) m_timerLabel->setText(text);
    const QString now = QLocale::system().toString(QTime::currentTime(), QLocale::ShortFormat);
    if (now != m_clock->text()) m_clock->setText(now);
}

// ---- all the slides ----

bool PresenterView::isShowingAllSlides() const { return m_pages->currentWidget() == m_grid; }

void PresenterView::showAllSlides(bool on)
{
    if (on) buildGrid();
    m_pages->setCurrentWidget(on ? static_cast<QWidget *>(m_grid) : m_pages->widget(0));
    m_all->setChecked(on);
    if (on && m_show) m_grid->setCurrentRow(std::max(0, m_show->targetSlide()));
}

void PresenterView::buildGrid()
{
    if (!m_show) return;
    const Document *doc = m_show->document();
    if (m_grid->count() == doc->pages.size()) return;
    m_grid->clear();
    for (int i = 0; i < doc->pages.size(); ++i) {
        QImage thumb = renderSlideImage(doc, m_show->cache(), i, m_grid->iconSize(), devicePixelRatioF(), kBars);
        auto *it = new QListWidgetItem(QIcon(QPixmap::fromImage(thumb)), doc->pages[i]->hidden ? tr("%1 (hidden)").arg(i + 1) : QString::number(i + 1));
        it->setTextAlignment(Qt::AlignHCenter);
        it->setData(Qt::AccessibleTextRole, tr("Slide %1").arg(i + 1));
        m_grid->addItem(it);
    }
}

void PresenterView::jumpTo(int slide)
{
    if (m_show) m_show->goTo(slide);
    showAllSlides(false);
}

// ---- keys, clicks, closing ----

void PresenterView::keyPressEvent(QKeyEvent *e)
{
    // The show reads the keys: the same ones work in both windows.
    if (m_show) QCoreApplication::sendEvent(m_show, e);
    else QWidget::keyPressEvent(e);
}

bool PresenterView::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_current && e->type() == QEvent::MouseButtonPress && m_show) {
        if (static_cast<QMouseEvent *>(e)->button() == Qt::LeftButton) m_show->next();
        return true;
    }
    return QWidget::eventFilter(o, e);
}

// Closing Presenter View ends the show.
void PresenterView::closeEvent(QCloseEvent *e)
{
    e->accept();
    if (m_show) m_show->close();
}

} // namespace jp

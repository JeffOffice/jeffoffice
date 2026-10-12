// The slide show's side of JeffDeck's window: the Transitions tab (the
// transition of the slide being edited, its timing, and a preview), Presenter
// View's commands, the star that marks slides with a transition, and the Slide
// Show tab's Set Up Slide Show, custom shows, Rehearse Timings, and Use Timings,
// with what a show leaves behind (ink, and new times) kept or discarded when it ends.

#include "app/deckcanvas.h"
#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/icons.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/presenterview.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/slideshow.h"
#include "app/showdialogs.h"
#include "app/slidespane.h"
#include "app/widgets.h"
#include "canvas/canvas.h"
#include "core/appinfo.h"
#include "core/document.h"
#include "core/ink.h"
#include "core/showsettings.h"
#include "core/transition.h"

#include <QAction>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QPolygonF>
#include <QtMath>
#include <QScreen>
#include <QSignalBlocker>
#include <QStyledItemDelegate>
#include <cmath>
#include <memory>

namespace jp {

namespace {

const QColor kStar(0xB6, 0x42, 0x0F);

deck::Transition currentTransition(Editor *ed)
{
    const Document *d = ed->doc();
    const int i = ed->currentPage();
    return d && i >= 0 && i < d->pages.size() ? deck::transitionOf(*d->pages[i]) : deck::Transition();
}

// Changes the transition of the slide being edited, as one undo step; nothing
// when it comes out the same.
void editTransition(Editor *ed, const QString &label, const std::function<void(deck::Transition &)> &change)
{
    Document *d = ed->doc();
    const int i = ed->currentPage();
    if (!d || i < 0 || i >= d->pages.size()) return;
    const deck::Transition before = deck::transitionOf(*d->pages[i]);
    deck::Transition t = before;
    change(t);
    if (deck::Transition::fromJson(t.toJson()) == before) return;
    ed->endTextEdit();
    ed->beginChange(label);
    deck::setTransition(*d->pages[i], t);
    ed->endChange();
}

// Changes the slide show's settings, as one undo step; nothing when they come out the same.
void editShowSettings(Editor *ed, const QString &label, const std::function<void(deck::ShowSettings &)> &change)
{
    Document *d = ed->doc();
    const deck::ShowSettings before = deck::showSettingsOf(*d);
    deck::ShowSettings now = before;
    change(now);
    if (deck::ShowSettings::fromJson(now.toJson()) == before) return;
    ed->endTextEdit();
    ed->beginChange(label);
    deck::setShowSettings(*d, now);
    ed->endChange();
}

// What a transition looks like halfway, between two sample slides: the real
// drawing, so that the gallery shows what each one does.
QIcon transitionIcon(const QString &type)
{
    return drawnIcon([type](QPainter *p, const QRectF &rc) {
        const QSize size = rc.size().toSize();
        const qreal dpr = p->device()->devicePixelRatioF();
        auto sample = [&](const QColor &paper, const QColor &ink) {
            QImage img(QSize(qRound(size.width() * dpr), qRound(size.height() * dpr)), QImage::Format_RGB32);
            img.setDevicePixelRatio(dpr);
            img.fill(paper);
            QPainter q(&img);
            q.setPen(Qt::NoPen);
            q.setBrush(ink);
            q.drawRoundedRect(QRectF(size.width() * 0.16, size.height() * 0.2, size.width() * 0.68, size.height() * 0.2), 1.5, 1.5);
            q.drawRoundedRect(QRectF(size.width() * 0.16, size.height() * 0.52, size.width() * 0.5, size.height() * 0.1), 1.5, 1.5);
            q.drawRoundedRect(QRectF(size.width() * 0.16, size.height() * 0.7, size.width() * 0.4, size.height() * 0.1), 1.5, 1.5);
            return img;
        };
        const QImage from = sample(QColor(0xDC, 0xE3, 0xEB), QColor(0x5B, 0x6B, 0x7D));
        const QImage to = sample(QColor(0xF6, 0xDC, 0xCE), kStar);
        QImage frame(from.size(), QImage::Format_RGB32);
        frame.setDevicePixelRatio(dpr);
        {
            QPainter q(&frame);
            deck::Transition t;
            t.type = type;
            paintTransition(&q, t, from, to, type == QLatin1String("none") ? 0 : 0.5);
        }
        p->drawImage(rc.topLeft(), frame);
        p->setPen(QPen(QColor(0x88, 0x88, 0x88), 1));
        p->setBrush(Qt::NoBrush);
        p->drawRect(rc.adjusted(0.5, 0.5, -0.5, -0.5));
    });
}

// Draws the delegate it wraps, and a small star under the number of a slide
// that has a transition.
class TransitionMarkDelegate : public QStyledItemDelegate {
public:
    TransitionMarkDelegate(Editor *ed, QAbstractItemDelegate *inner, QObject *parent) : QStyledItemDelegate(parent), m_ed(ed), m_inner(inner) {}

    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (m_inner) m_inner->paint(p, option, index);
        else QStyledItemDelegate::paint(p, option, index);
        const Document *d = m_ed->doc();
        const int row = index.row();
        if (!d || row < 0 || row >= d->pages.size() || deck::transitionOf(*d->pages[row]).type == QLatin1String("none")) return;
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const QRect text = opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
        const int h = opt.fontMetrics.height();
        const QPointF center(text.left() + 7, text.center().y() + h / 2.0 + 8);
        QPolygonF star;
        for (int i = 0; i < 10; ++i) {
            const double r = i % 2 ? 3.0 : 7.0, a = -M_PI / 2 + i * M_PI / 5;
            star << center + QPointF(r * std::cos(a), r * std::sin(a));
        }
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(Qt::NoPen);
        p->setBrush(kStar);
        p->drawPolygon(star);
        p->restore();
    }
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return m_inner ? m_inner->sizeHint(option, index) : QStyledItemDelegate::sizeHint(option, index);
    }

private:
    Editor *m_ed;
    QAbstractItemDelegate *m_inner;
};

} // namespace

void DeckWindow::createShowActions()
{
    Editor *ed = m_ed;
    const QString label = tr("Transition"), applyLabel = tr("Apply Transition To All Slides");

    // Presenter View: Alt+F5 rehearses in windows, on any number of screens.
    mk("show.presenterView", tr("Presenter View"), "monitor-up", QKeySequence(Qt::ALT | Qt::Key_F5), [this] {
        playShow(m_ed->currentPage(), deck::ShowPlan(), false, true);
    });
    QAction *use = mk("show.usePresenterView", tr("Use Presenter View"), QString(), QKeySequence(), [this] {
        SlideShow::setUsePresenterView(act("show.usePresenterView")->isChecked());
    }, true);
    use->setChecked(SlideShow::usePresenterView());

    // Set Up: the show's settings are the presentation's; the commands show what it has.
    mk("show.setUp", tr("Set Up Slide Show"), "monitor-cog", QKeySequence(), [this] { setUpShow(); });
    mk("show.customShows", tr("Custom Shows…"), "list-video", QKeySequence(), [this] { editCustomShows(); });
    mk("show.rehearse", tr("Rehearse Timings"), "timer", QKeySequence(), [this] { rehearseTimings(); });
    mk("show.useTimings", tr("Use Timings"), QString(), QKeySequence(), [this, ed] {
        const bool on = act("show.useTimings")->isChecked();
        editShowSettings(ed, tr("Use Timings"), [on](deck::ShowSettings &s) { s.useTimings = on; });
    }, true);
    auto syncTimings = [this, ed] {
        if (const Document *d = ed->doc()) act("show.useTimings")->setChecked(deck::showSettingsOf(*d).useTimings);
    };
    connect(ed, &Editor::changed, this, syncTimings);
    connect(ed, &Editor::documentReplaced, this, syncTimings);
    syncTimings();

    // The transitions, and the ways each goes.
    const struct { const char *type; QString text; } kinds[] = {
        {"none", tr("None")},       {"fade", tr("Fade")},   {"push", tr("Push")},       {"wipe", tr("Wipe")}, {"split", tr("Split")},
        {"reveal", tr("Reveal")},   {"cover", tr("Cover")}, {"uncover", tr("Uncover")}, {"zoom", tr("Zoom")}, {"cut", tr("Cut")},
    };
    for (const auto &k : kinds) {
        const QString type = QString::fromLatin1(k.type);
        mk(QStringLiteral("transition.") + type, k.text, QString(), QKeySequence(), [ed, type, label] {
            editTransition(ed, label, [&type](deck::Transition &t) {
                t.type = type;
                // A direction that means nothing to the new type gives way to its own.
                if (t.hasInOut() != (t.dir == QLatin1String("in") || t.dir == QLatin1String("out"))) t.dir = deck::Transition::defaultDirection(type);
            });
        }, true);
    }
    const struct { const char *dir; QString text; } directions[] = {
        {"l", tr("From Right")}, {"r", tr("From Left")}, {"u", tr("From Bottom")}, {"d", tr("From Top")}, {"out", tr("Out")}, {"in", tr("In")},
    };
    for (const auto &k : directions) {
        const QString dir = QString::fromLatin1(k.dir);
        mk(QStringLiteral("transition.dir.") + dir, k.text, QString(), QKeySequence(), [ed, dir, label] {
            editTransition(ed, label, [&dir](deck::Transition &t) { t.dir = dir; });
        }, true);
    }

    // Preview plays the transition into the current slide over the slide area.
    mk("transition.preview", tr("Preview"), "play", QKeySequence(), [this] {
        delete m_canvas->findChild<TransitionPreview *>(QStringLiteral("transitionPreview"), Qt::FindDirectChildrenOnly);
        auto *preview = new TransitionPreview(m_ed->doc(), &m_ed->cache(), m_canvas);
        preview->play(m_ed->currentPage());
    });
    mk("transition.applyToAll", tr("Apply To All"), "copy-check", QKeySequence(), [ed, applyLabel] {
        Document *d = ed->doc();
        const deck::Transition t = currentTransition(ed);
        ed->endTextEdit();
        ed->beginChange(applyLabel);
        for (const auto &page : d->pages) deck::setTransition(*page, t);
        ed->endChange();
    });

    // The commands show what the slide has.
    auto sync = [this] {
        const deck::Transition t = currentTransition(m_ed);
        for (const QString &type : deck::transitionTypes()) act(QStringLiteral("transition.") + type)->setChecked(t.type == type);
        for (const QString &dir : {QStringLiteral("l"), QStringLiteral("r"), QStringLiteral("u"), QStringLiteral("d")}) {
            QAction *a = act(QStringLiteral("transition.dir.") + dir);
            a->setEnabled(t.hasDirection());
            a->setChecked(t.hasDirection() && t.dir == dir);
        }
        for (const QString &dir : {QStringLiteral("out"), QStringLiteral("in")}) {
            QAction *a = act(QStringLiteral("transition.dir.") + dir);
            a->setEnabled(t.hasInOut());
            a->setChecked(t.hasInOut() && t.dir == dir);
        }
        act("transition.preview")->setEnabled(t.type != QLatin1String("none"));
    };
    connect(ed, &Editor::changed, this, sync);
    connect(ed, &Editor::pageChanged, this, sync);
    connect(ed, &Editor::documentReplaced, this, sync);
    sync();

    // The slide thumbnails mark the slides that have one.
    m_slides->setItemDelegate(new TransitionMarkDelegate(ed, m_slides->itemDelegate(), m_slides));
    connect(ed, &Editor::changed, m_slides->viewport(), qOverload<>(&QWidget::update));
}

void DeckWindow::addShowParts(RibbonParts &parts)
{
    Editor *ed = m_ed;
    // Each control that shows the slide's transition says how to read it again.
    auto syncs = std::make_shared<QList<std::function<void()>>>();
    auto syncAll = [syncs] {
        for (const auto &fn : *syncs) fn();
    };
    connect(ed, &Editor::changed, this, syncAll);
    connect(ed, &Editor::pageChanged, this, syncAll);
    connect(ed, &Editor::documentReplaced, this, syncAll);
    const QString label = tr("Transition");
    // The After box needs the seconds beside it.
    auto seconds = std::make_shared<QPointer<QDoubleSpinBox>>();

    parts.widgets[QStringLiteral("transitionGallery")] = [this, syncs] {
        auto *gallery = new Gallery(QSize(64, 40), 5);
        gallery->setObjectName(QStringLiteral("transitionGallery"));
        gallery->setTitle(tr("Transition to This Slide"));
        QVector<GalleryItem> items;
        for (const QString &type : deck::transitionTypes()) items << GalleryItem{type, act(QStringLiteral("transition.") + type)->text(), transitionIcon(type), QString()};
        gallery->setItems(items);
        connect(gallery, &Gallery::activated, this, [this](const QString &type) { act(QStringLiteral("transition.") + type)->trigger(); });
        syncs->append([gallery, this] { gallery->setCurrent(currentTransition(m_ed).type); });
        syncs->last()();
        return gallery;
    };
    // Effect Options: the directions or in and out of the slide's transition type.
    parts.menus[QStringLiteral("effectOptions")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        auto fill = [this, menu] {
            menu->clear();
            const deck::Transition t = currentTransition(m_ed);
            QStringList ids;
            if (t.hasInOut()) ids = {QStringLiteral("out"), QStringLiteral("in")};
            else if (t.hasDirection()) ids = {QStringLiteral("l"), QStringLiteral("r"), QStringLiteral("u"), QStringLiteral("d")};
            for (const QString &id : ids) menu->addAction(act(QStringLiteral("transition.dir.") + id));
            if (ids.isEmpty()) menu->addAction(tr("No Effect Options"))->setEnabled(false);
        };
        connect(menu, &QMenu::aboutToShow, menu, fill);
        fill();
        return menu;
    };

    parts.widgets[QStringLiteral("transitionDuration")] = [this, ed, syncs, label] {
        auto *spin = new QDoubleSpinBox();
        spin->setObjectName(QStringLiteral("transitionDuration"));
        spin->setRange(0.01, 59);
        spin->setDecimals(2);
        spin->setSingleStep(0.25);
        spin->setSuffix(tr(" s"));
        spin->setKeyboardTracking(false);
        spin->setFixedWidth(84);
        spin->setToolTip(tr("Duration: how many seconds the transition takes."));
        spin->setAccessibleName(tr("Duration"));
        connect(spin, &QDoubleSpinBox::valueChanged, this, [ed, label](double v) {
            editTransition(ed, label, [v](deck::Transition &t) { t.duration = v; });
        });
        syncs->append([spin, ed] {
            const QSignalBlocker block(spin);
            spin->setValue(currentTransition(ed).duration);
        });
        syncs->last()();
        return spin;
    };
    parts.widgets[QStringLiteral("advanceOnClick")] = [this, ed, syncs, label] {
        auto *box = new QCheckBox(tr("On Mouse Click"));
        box->setObjectName(QStringLiteral("advanceOnClick"));
        box->setToolTip(tr("On Mouse Click: the slide show moves on from this slide when the presenter clicks."));
        connect(box, &QCheckBox::toggled, this, [ed, label](bool on) {
            editTransition(ed, label, [on](deck::Transition &t) { t.advanceOnClick = on; });
        });
        syncs->append([box, ed] {
            const QSignalBlocker block(box);
            box->setChecked(currentTransition(ed).advanceOnClick);
        });
        syncs->last()();
        return box;
    };
    parts.widgets[QStringLiteral("advanceAfter")] = [this, ed, syncs, label, seconds] {
        auto *box = new QCheckBox(tr("After:"));
        box->setObjectName(QStringLiteral("advanceAfter"));
        box->setToolTip(tr("After: the slide show moves on from this slide by itself after this many seconds."));
        connect(box, &QCheckBox::toggled, this, [ed, label, seconds](bool on) {
            const double secs = *seconds ? (*seconds)->value() : 5.0;
            editTransition(ed, label, [on, secs](deck::Transition &t) {
                if (on) t.advanceAfter = secs;
                else t.advanceAfter.reset();
            });
        });
        syncs->append([box, ed] {
            const QSignalBlocker block(box);
            box->setChecked(currentTransition(ed).advanceAfter.has_value());
        });
        syncs->last()();
        return box;
    };
    parts.widgets[QStringLiteral("advanceAfterSeconds")] = [this, ed, syncs, label, seconds] {
        auto *spin = new QDoubleSpinBox();
        *seconds = spin;
        spin->setObjectName(QStringLiteral("advanceAfterSeconds"));
        spin->setRange(0, 3600);
        spin->setDecimals(1);
        spin->setSingleStep(1);
        spin->setValue(5);
        spin->setSuffix(tr(" s"));
        spin->setKeyboardTracking(false);
        spin->setFixedWidth(76);
        spin->setToolTip(tr("Seconds the slide stays before the slide show moves on."));
        spin->setAccessibleName(tr("Seconds before moving on"));
        connect(spin, &QDoubleSpinBox::valueChanged, this, [ed, label](double v) {
            editTransition(ed, label, [v](deck::Transition &t) {
                if (t.advanceAfter) t.advanceAfter = v;
            });
        });
        syncs->append([spin, ed] {
            const QSignalBlocker block(spin);
            const auto after = currentTransition(ed).advanceAfter;
            spin->setEnabled(after.has_value());
            if (after) spin->setValue(*after);
        });
        syncs->last()();
        return spin;
    };

    // Slide Show > Set Up > Use Timings, and Custom Slide Show's menu: the shows by name, and Custom Shows….
    parts.widgets[QStringLiteral("useTimings")] = [this] {
        QAction *a = act(QStringLiteral("show.useTimings"));
        auto *box = new QCheckBox(a->text());
        box->setObjectName(QStringLiteral("useTimings"));
        box->setToolTip(tr("Use Timings: the slide show moves on by itself after the seconds each slide is set to wait. Without it, only the presenter moves it on."));
        box->setChecked(a->isChecked());
        connect(box, &QCheckBox::clicked, a, [a] { a->trigger(); });
        connect(a, &QAction::toggled, box, [box](bool on) {
            const QSignalBlocker block(box);
            box->setChecked(on);
        });
        return box;
    };
    parts.menus[QStringLiteral("customShows")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        auto fill = [this, menu] {
            menu->clear();
            for (const deck::CustomShow &show : deck::customShowsOf(*m_ed->doc())) {
                QAction *a = menu->addAction(QString(show.name).replace(QLatin1Char('&'), QStringLiteral("&&")));
                const qint64 id = show.id;
                connect(a, &QAction::triggered, this, [this, id] { startCustomShow(id); });
            }
            if (!menu->isEmpty()) menu->addSeparator();
            menu->addAction(act(QStringLiteral("show.customShows")));
        };
        connect(menu, &QMenu::aboutToShow, menu, fill);
        fill();
        return menu;
    };

    // Slide Show > Monitors.
    parts.widgets[QStringLiteral("usePresenterView")] = [this] {
        QAction *a = act(QStringLiteral("show.usePresenterView"));
        auto *box = new QCheckBox(a->text());
        box->setObjectName(QStringLiteral("usePresenterView"));
        box->setToolTip(tr("Use Presenter View: with two screens, the slide show goes to one and the notes, timer, and next slide to the other."));
        box->setChecked(a->isChecked());
        connect(box, &QCheckBox::clicked, a, [a] { a->trigger(); });
        connect(a, &QAction::toggled, box, [box](bool on) {
            const QSignalBlocker block(box);
            box->setChecked(on);
        });
        return box;
    };
}

} // namespace jp

namespace jp {

// ---- playing the show ----

void DeckWindow::startShowFromBeginning()
{
    const Document &d = *m_ed->doc();
    const deck::ShowSettings settings = deck::showSettingsOf(d);
    const deck::ShowPlan plan = deck::planOf(d, settings);
    if (plan.none) {
        tellAboutShow(tr("The slide show is set to play slides %1 to %2, and the presentation has only %3. Change this in Set Up Slide Show.")
                          .arg(settings.from).arg(settings.to).arg(d.pages.size()));
        return;
    }
    playShow(0, plan, false);
}

void DeckWindow::startCustomShow(qint64 id)
{
    const Document &d = *m_ed->doc();
    const QVector<deck::CustomShow> shows = deck::customShowsOf(d);
    const deck::CustomShow *show = deck::customShowById(shows, id);
    if (!show) return;
    const deck::ShowPlan plan = deck::planOfCustomShow(d, *show);
    if (!plan.slides.isEmpty()) playShow(plan.slides.first(), plan, false);
}

void DeckWindow::rehearseTimings() { playShow(0, deck::ShowPlan(), true); }

// Starts the show as the presentation is set up: the slides `plan` names (every slide when it names none) from `slide`; `recording`
// times the slides (Rehearse Timings), as a speaker's show that does not loop. `rehearsal` is the window Alt+F5 opens, with Presenter View
// beside it: the set up applies to it, except that it is a window for a speaker, never a kiosk.
void DeckWindow::playShow(int slide, const deck::ShowPlan &plan, bool recording, bool rehearsal)
{
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    if (m_show) m_show->close();
    deck::ShowSettings settings = deck::showSettingsOf(*m_ed->doc());
    if (recording) {
        settings.type = deck::ShowSettings::Type::Speaker;
        settings.loop = false;
    }
    if (rehearsal) settings.type = deck::ShowSettings::Type::Speaker;
    m_show = new SlideShow(m_ed->doc(), &m_ed->cache(), slide);
    if (rehearsal) m_show->setRehearsal(true);
    m_show->setSettings(settings);
    if (!plan.slides.isEmpty()) m_show->setSlides(plan.slides, plan.hidden);
    if (recording) m_show->setRecording(true);
    QScreen *s = screen();
    if (rehearsal) {
        if (s) m_show->setGeometry(s->availableGeometry());
    } else if (m_show->isWindowed()) {
        // By an individual, in a window: three quarters of the screen, in the middle.
        if (s) {
            const QRect area = s->availableGeometry();
            const QSize size(area.width() * 3 / 4, area.height() * 3 / 4);
            m_show->setGeometry(QRect(area.center() - QPoint(size.width() / 2, size.height() / 2), size));
        }
    } else if (s) {
        // On the window's screen, the whole screen.
        m_show->setGeometry(s->geometry());
    }
    watchShow(m_show, recording);
    if (m_show->isWindowed()) m_show->show();
    else m_show->showFullScreen();
    m_show->activateWindow();
    m_show->setFocus();
}

// When the show ends the window is back on the last slide the show showed, and asks about what the show left (once the show is gone).
void DeckWindow::watchShow(SlideShow *show, bool recording)
{
    connect(show, &SlideShow::destroyed, this, [this] { activateWindow(); });
    // A show in a window leaves the deck open to editing: it follows the slides by their pages; a presentation put in the window in
    // place of this one ends it.
    connect(m_ed, &Editor::changed, show, &SlideShow::documentChanged);
    connect(m_ed, &Editor::documentReplaced, show, [this, show] {
        show->rebind(m_ed->doc());
        show->close();
    });
    connect(show, &SlideShow::ended, this, [this, show, recording](int last) {
        if (last >= 0 && last < m_ed->doc()->pages.size()) m_ed->setCurrentPage(last);
        const QHash<QString, QVector<deck::InkStroke>> ink = show->allInk();
        const QMap<int, double> seconds = recording ? show->recordedSeconds() : QMap<int, double>();
        const double total = recording ? show->recordedTotal() : 0;
        QMetaObject::invokeMethod(this, [this, ink, seconds, total, recording] { afterShow(ink, seconds, total, recording); }, Qt::QueuedConnection);
    });
}

void DeckWindow::afterShow(const QHash<QString, QVector<deck::InkStroke>> &ink, const QMap<int, double> &seconds, double total, bool recording)
{
    Document *d = m_ed->doc();
    // Ink: a freeform line for each stroke on the slide it was drawn on (by its page: the slides may have moved or gone), all as one step.
    // Ink on slides that are gone is nothing to keep.
    const QHash<QString, int> pages = deck::pageIndexById(*d);
    QList<QPair<int, QVector<deck::InkStroke>>> drawn;
    for (auto it = ink.cbegin(); it != ink.cend(); ++it)
        if (const auto at = pages.constFind(it.key()); at != pages.constEnd() && !it.value().isEmpty()) drawn << qMakePair(at.value(), it.value());
    if (!drawn.isEmpty() && askAboutShow(tr("Do you want to keep your ink annotations?"), tr("Keep"), tr("Discard"))) {
        m_ed->endTextEdit();
        m_ed->beginChange(tr("Keep Ink"));
        for (const auto &slide : std::as_const(drawn)) deck::addInk(*d->pages[slide.first], slide.second);
        m_ed->endChange();
    }
    // Times: each slide's After, and Use Timings on.
    if (recording && !seconds.isEmpty() &&
        askAboutShow(tr("The slide show took %1. Do you want to keep the new slide timings?").arg(RehearseBar::clockText(qRound64(total * 1000))), tr("Yes"), tr("No"))) {
        m_ed->endTextEdit();
        m_ed->beginChange(tr("Rehearse Timings"));
        for (auto it = seconds.cbegin(); it != seconds.cend(); ++it) {
            if (it.key() < 0 || it.key() >= d->pages.size()) continue;
            deck::Transition t = deck::transitionOf(*d->pages[it.key()]);
            t.advanceAfter = std::max(0.1, std::round(it.value() * 10) / 10);
            deck::setTransition(*d->pages[it.key()], t);
        }
        deck::ShowSettings settings = deck::showSettingsOf(*d);
        settings.useTimings = true;
        deck::setShowSettings(*d, settings);
        m_ed->endChange();
    }
}

bool DeckWindow::askAboutShow(const QString &question, const QString &keep, const QString &discard)
{
    if (m_showQuestion) return m_showQuestion(question);
    QMessageBox box(QMessageBox::Question, appInfo().name, question, QMessageBox::NoButton, this);
    QPushButton *yes = box.addButton(keep, QMessageBox::AcceptRole);
    QPushButton *no = box.addButton(discard, QMessageBox::RejectRole);
    box.setDefaultButton(yes);
    box.setEscapeButton(no);   // Esc has just ended the show: it must not also change the slides
    box.exec();
    return box.clickedButton() == yes;
}

void DeckWindow::tellAboutShow(const QString &text)
{
    if (m_showQuestion) {
        m_showQuestion(text);
        return;
    }
    QMessageBox::information(this, appInfo().name, text);
}

// ---- Set Up Slide Show and Custom Shows ----

void DeckWindow::setUpShow()
{
    const Document *d = m_ed->doc();
    SetUpShowDialog dlg(deck::showSettingsOf(*d), deck::customShowsOf(*d), int(d->pages.size()), d->colors, this);
    if (dlg.exec() != QDialog::Accepted) return;
    const deck::ShowSettings chosen = dlg.settings();
    editShowSettings(m_ed, tr("Set Up Slide Show"), [&chosen](deck::ShowSettings &s) { s = chosen; });
}

void DeckWindow::editCustomShows()
{
    Document *d = m_ed->doc();
    const QVector<deck::CustomShow> before = deck::customShowsOf(*d);
    CustomShowsDialog dlg(*d, before, this);
    dlg.exec();
    const QVector<deck::CustomShow> after = dlg.shows();
    if (after != before) {
        m_ed->endTextEdit();
        m_ed->beginChange(tr("Custom Shows"));
        deck::setCustomShows(*d, after);   // a show set to play one that is gone plays every slide
        m_ed->endChange();
    }
    if (dlg.playId() >= 0) startCustomShow(dlg.playId());
}

} // namespace jp

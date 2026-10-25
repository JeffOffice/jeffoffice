// The animations side of JeffDeck's window: the Animations tab (the effect of
// the selected shape, how it goes, how it starts and how long it takes), the
// Animation Pane on the right, Preview, and the numbers by the shapes. Every
// change is one step that undoes, made through the Editor on the slide's
// animations (core/animation.h).

#include "app/deckcanvas.h"
#include "app/deckwindow.h"

#include "app/animationdialogs.h"
#include "app/animationpane.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/widgets.h"
#include "canvas/canvas.h"
#include "core/animation.h"
#include "core/document.h"

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QMenu>
#include <QPainter>
#include <QPointer>
#include <QToolButton>
#include <QSignalBlocker>
#include <QStatusBar>
#include <algorithm>
#include <memory>
#include "app/plaintext.h"

namespace jp {

namespace {

const QSize kGalleryItem(64, 52);   // a gallery picture: the slide, and the name under it

// What the controls do, kept in one place. The pane shows the sequence and the ribbon says what was asked; this reads the
// slide's animations, changes them as one undo step, and keeps what shows them in step.
struct Anims {
    DeckWindow *w = nullptr;
    Editor *ed = nullptr;
    AnimationPane *pane = nullptr;
    QVector<int> picked;          // the rows picked in the pane, until a pick on the slide takes them back
    bool pickingShapes = false;   // the selection is being changed from the pane
    int painterFrom = 0;          // the shape whose effects Animation Painter copies, while it waits for the next shape
    QString cachedPage;
    quint64 cachedSignature = 0;
    deck::AnimSequence cached;
    // What shows the effects picked (the commands' and controls' states) says so here, to be told when the pick changes.
    QList<std::function<void()>> listeners;
    void picksChanged() const
    {
        for (const auto &fn : listeners) fn();
    }

    Document *doc() const { return ed->doc(); }
    Page *slide() const
    {
        Document *d = doc();
        const int i = ed->currentPage();
        return d && i >= 0 && i < d->pages.size() ? d->pages[i].get() : nullptr;
    }

    // The slide's sequence, read again only when its timing changed.
    const deck::AnimSequence &sequence()
    {
        static const deck::AnimSequence none;
        const Page *p = slide();
        if (!p) return none;
        const quint64 signature = deck::animationSignature(*doc(), *p);
        if (p->id != cachedPage || signature != cachedSignature) {
            cachedPage = p->id;
            cachedSignature = signature;
            cached = signature ? deck::readAnimations(*p, doc()) : deck::AnimSequence();
            // The rows picked that the sequence no longer has are not picked.
            picked.erase(std::remove_if(picked.begin(), picked.end(), [this](int r) { return r >= cached.effects.size(); }), picked.end());
        }
        return cached;
    }

    // Changes the slide's animations as one step; nothing when they come out the same. The rows picked in the pane stay picked
    // when the change keeps the effects where they are, and are let go otherwise.
    bool edit(const QString &label, const std::function<void(deck::AnimSequence &, Page &)> &fn, bool keepPicked = false)
    {
        Page *p = slide();
        if (!p) return false;
        deck::AnimSequence seq = deck::readAnimations(*p, doc());
        if (!seq.editable) {
            w->statusBar()->showMessage(QCoreApplication::translate("Animation", "The animations on this slide can't be changed here."), 6000);
            return false;
        }
        ed->endTextEdit();
        ed->beginChange(label);
        fn(seq, *p);
        deck::setAnimations(*doc(), *p, seq);
        ed->endChange();
        if (!keepPicked) {
            picked.clear();
            pane->selectRows({});
            picksChanged();
        }
        return true;
    }

    // The numbers of the shapes selected on the slide, in the order they are selected; a shape that has none yet gets one when `create`.
    QVector<int> selectedShapes(Page &p, bool create) const
    {
        QVector<int> out;
        for (const QString &id : ed->selection()) {
            for (const ItemPtr &it : p.items) {
                if (it->id != id) continue;
                const int n = create ? deck::shapeNumber(p, id) : it->sourceId;
                if (n > 0) out << n;
            }
        }
        return out;
    }

    // The effects a command means: the rows picked in the pane, else those of the shapes selected on the slide.
    QVector<int> targets(const deck::AnimSequence &seq, Page &p) const
    {
        QVector<int> rows;
        for (int r : picked)
            if (r >= 0 && r < seq.effects.size()) rows << r;
        if (!rows.isEmpty()) return rows;
        const QVector<int> shapes = selectedShapes(p, false);
        for (int i = 0; i < seq.effects.size(); ++i)
            if (shapes.contains(seq.effects[i].shape)) rows << i;
        return rows;
    }

    // The build an effect is in: its shape's effects of the same group that aim at paragraphs, beside it; else just it.
    static QVector<int> buildOf(const deck::AnimSequence &seq, int row)
    {
        const deck::Anim &a = seq.effects[row];
        QVector<int> out{row};
        if (!a.paragraphs()) return out;
        for (int i = row - 1; i >= 0 && seq.effects[i].shape == a.shape && seq.effects[i].group == a.group && seq.effects[i].paragraphs(); --i) out.prepend(i);
        for (int i = row + 1; i < seq.effects.size() && seq.effects[i].shape == a.shape && seq.effects[i].group == a.group && seq.effects[i].paragraphs(); ++i) out.append(i);
        return out;
    }
    // The rows and the rest of their builds, once each.
    static QVector<int> withBuilds(const deck::AnimSequence &seq, const QVector<int> &rows)
    {
        QSet<int> all;
        for (int r : rows)
            for (int b : buildOf(seq, r)) all.insert(b);
        QVector<int> out(all.begin(), all.end());
        std::sort(out.begin(), out.end());
        return out;
    }

    bool hasText(const Page &p, int shape) const { return !deck::paragraphGroups(*doc(), p, shape).isEmpty(); }

    // ---- what the controls ask ----

    // An effect of this kind on the shapes selected: the effect picked in the pane (or the first of each shape's) becomes it, and a shape with none gets one.
    void apply(const QString &kindId)
    {
        const deck::EffectKind *kind = deck::effectKind(kindId);
        if (!kind) return;
        const QVector<int> rows = picked;
        edit(QCoreApplication::translate("Animation", "Animation"), [&](deck::AnimSequence &seq, Page &p) {
            const QVector<int> shapes = selectedShapes(p, true);
            QVector<int> change;
            if (!rows.isEmpty()) {
                for (int r : rows)
                    if (r < seq.effects.size()) change << r;
            } else {
                for (int s : shapes) {
                    const QVector<int> have = seq.effectsOf(s);
                    if (!have.isEmpty()) change << have.first();
                }
            }
            for (int r : withBuilds(seq, change)) deck::changeKind(seq.effects[r], *kind);
            if (!rows.isEmpty()) return;
            // The first shape starts on a click, and the others with it.
            bool first = true;
            for (int s : shapes) {
                if (!seq.effectsOf(s).isEmpty()) continue;
                deck::Anim a = deck::makeAnim(*kind, s);
                a.start = first ? deck::EffectStart::OnClick : deck::EffectStart::WithPrevious;
                seq.add(a);
                first = false;
            }
        }, !rows.isEmpty());
    }

    // Another effect on each shape selected, at the end.
    void add(const QString &kindId)
    {
        const deck::EffectKind *kind = deck::effectKind(kindId);
        if (!kind) return;
        edit(QCoreApplication::translate("Animation", "Add Animation"), [&](deck::AnimSequence &seq, Page &p) {
            bool first = true;
            for (int s : selectedShapes(p, true)) {
                deck::Anim a = deck::makeAnim(*kind, s);
                a.start = first ? deck::EffectStart::OnClick : deck::EffectStart::WithPrevious;
                seq.add(a);
                first = false;
            }
        });
    }

    void removeRows(const QVector<int> &rows)
    {
        edit(QCoreApplication::translate("Animation", "Remove Animation"), [&](deck::AnimSequence &seq, Page &) {
            for (int i = int(rows.size()) - 1; i >= 0; --i) seq.remove(rows[i]);
        });
        picked.clear();
    }

    // No animation on the shapes selected (or the rows picked).
    void clear()
    {
        const QVector<int> rows = picked;
        edit(QCoreApplication::translate("Animation", "Remove Animation"), [&](deck::AnimSequence &seq, Page &p) {
            QVector<int> remove = rows;
            if (remove.isEmpty()) remove = targets(seq, p);
            std::sort(remove.begin(), remove.end());
            for (int i = int(remove.size()) - 1; i >= 0; --i)
                if (remove[i] < seq.effects.size()) seq.remove(remove[i]);
        });
        picked.clear();
    }

    void setStart(deck::EffectStart start)
    {
        edit(QCoreApplication::translate("Animation", "Start"), [&](deck::AnimSequence &seq, Page &p) {
            for (int r : targets(seq, p)) seq.effects[r].start = start;
        }, true);
    }

    // Changes each effect the command means, with its whole build (the paragraphs of a list change together).
    void change(const QString &label, const std::function<void(deck::Anim &)> &fn)
    {
        edit(label, [&](deck::AnimSequence &seq, Page &p) {
            for (int r : withBuilds(seq, targets(seq, p))) fn(seq.effects[r]);
        }, true);
    }

    // The rows picked each move one place earlier (-1) or later (1), staying in the order they are in.
    void moveRows(const QVector<int> &rows, int by)
    {
        QVector<int> after;
        edit(QCoreApplication::translate("Animation", "Reorder Animation"), [&](deck::AnimSequence &seq, Page &) {
            QVector<bool> chosen(seq.effects.size(), false);
            for (int r : rows)
                if (r >= 0 && r < chosen.size()) chosen[r] = true;
            const int n = int(chosen.size());
            if (by < 0) {
                for (int i = 1; i < n; ++i)
                    if (chosen[i] && !chosen[i - 1]) {
                        seq.move(i, i - 1);
                        std::swap(chosen[i], chosen[i - 1]);
                    }
            } else {
                for (int i = n - 2; i >= 0; --i)
                    if (chosen[i] && !chosen[i + 1]) {
                        seq.move(i, i + 1);
                        std::swap(chosen[i], chosen[i + 1]);
                    }
            }
            for (int i = 0; i < n; ++i)
                if (chosen[i]) after << i;
        });
        picked = after;
        pane->selectRows(after);
        picksChanged();
    }

    // ---- the picture of it ----

    // The pane's rows picked mean these shapes on the slide.
    void selectShapesOf(const QVector<int> &rows)
    {
        Page *p = slide();
        if (!p) return;
        const deck::AnimSequence &seq = sequence();
        QStringList ids;
        for (int r : rows) {
            if (r < 0 || r >= seq.effects.size()) continue;
            walkItems(p->items, [&](const ItemPtr &it) {
                if (it->sourceId == seq.effects[r].shape && !ids.contains(it->id)) ids << it->id;
            });
        }
        pickingShapes = true;
        ed->select(ids);
        pickingShapes = false;
    }
};

// The click step a row plays at, as the preview counts steps from 0.
int stepOfRow(const deck::AnimSequence &seq, int row)
{
    if (row < 0 || row >= seq.effects.size()) return 0;
    const bool byItself = !seq.effects.isEmpty() && seq.effects.first().start != deck::EffectStart::OnClick;
    return std::max(0, seq.clickOf(row) - (byItself ? 0 : 1));
}

} // namespace

void DeckWindow::createAnimationActions()
{
    auto st = std::make_shared<Anims>();
    st->w = this;
    st->ed = m_ed;
    m_animPane = new AnimationPane(m_ed);
    st->pane = m_animPane;
    m_animDock = new QDockWidget(tr("Animation Pane"), this);
    m_animDock->setObjectName(QStringLiteral("animationDock"));
    m_animDock->setFeatures(QDockWidget::DockWidgetClosable);
    m_animDock->setWidget(m_animPane);
    addDockWidget(Qt::RightDockWidgetArea, m_animDock);
    m_animDock->hide();

    const QVector<deck::EffectKind> &kinds = deck::effectKinds();
    for (const deck::EffectKind &k : kinds) {
        const QString id = k.id;
        mk(QStringLiteral("anim.set.") + id, k.name, QString(), QKeySequence(), [st, id] { st->apply(id); });
        mk(QStringLiteral("anim.add.") + id, k.name, QString(), QKeySequence(), [st, id] { st->add(id); });
    }
    mk("anim.none", tr("None"), QString(), QKeySequence(), [st] { st->clear(); });

    // Preview plays the slide's effects over the slide, with no clicks; Play All and Play From (the pane) do too.
    auto play = [this, st](int row) {
        Page *p = st->slide();
        if (!p || !m_canvas->isVisible()) return;
        delete m_canvas->findChild<AnimationPreview *>(QStringLiteral("animationPreview"), Qt::FindDirectChildrenOnly);
        auto *preview = new AnimationPreview(m_ed->doc(), &m_ed->cache(), m_canvas);
        const QRect area = QRectF(m_canvas->pageToView(QPointF(0, 0)), m_canvas->pageToView(QPointF(m_ed->doc()->pageSize().width(), m_ed->doc()->pageSize().height()))).toAlignedRect();
        const deck::AnimSequence seq = deck::readAnimations(*p, m_ed->doc());
        connect(preview, &AnimationPreview::stepChanged, this, [this, seq](int step) {
            int shown = -1;
            if (step >= 0) {
                const bool byItself = !seq.effects.isEmpty() && seq.effects.first().start != deck::EffectStart::OnClick;
                for (int i = 0; i < seq.effects.size() && shown < 0; ++i)
                    if (seq.clickOf(i) - (byItself ? 0 : 1) == step) shown = i;
            }
            m_animPane->setPlaying(shown);
        });
        connect(preview, &AnimationPreview::finished, preview, &QObject::deleteLater);
        if (!preview->play(m_ed->currentPage(), area, row < 0 ? 0 : stepOfRow(seq, row))) {
            delete preview;
            statusBar()->showMessage(tr("This slide has no animation to play."), 4000);
        }
    };
    mk("anim.preview", tr("Preview"), "play", QKeySequence(), [play] { play(-1); });
    connect(m_animPane, &AnimationPane::playRequested, this, [play](int row) { play(row); });

    mk("anim.pane", tr("Animation Pane"), "list-video", QKeySequence(), [this] { showAnimationPane(act("anim.pane")->isChecked()); }, true);
    connect(m_animDock, &QDockWidget::visibilityChanged, this, [pane = QPointer<QAction>(act("anim.pane"))](bool on) {
        if (!pane) return;
        const QSignalBlocker block(pane);
        pane->setChecked(on);
    });
    // Animation Painter copies the animation of the shape selected onto the shape selected next.
    mk("anim.painter", tr("Animation Painter"), "paintbrush", QKeySequence(), [this, st] {
        Page *p = st->slide();
        if (!p) return;
        const QVector<int> shapes = st->selectedShapes(*p, false);
        if (shapes.size() != 1 || st->sequence().effectsOf(shapes.first()).isEmpty()) {
            statusBar()->showMessage(tr("Select a shape that has an animation, then choose Animation Painter."), 6000);
            return;
        }
        st->painterFrom = shapes.first();
        statusBar()->showMessage(tr("Select the shape to give the animation to."), 8000);
    });
    mk("anim.earlier", tr("Move Earlier"), "arrow-up", QKeySequence(), [this, st] {
        Page *p = st->slide();
        if (p) st->moveRows(st->targets(st->sequence(), *p), -1);
    });
    mk("anim.later", tr("Move Later"), "arrow-down", QKeySequence(), [this, st] {
        Page *p = st->slide();
        if (p) st->moveRows(st->targets(st->sequence(), *p), 1);
    });

    // The dialog: the effect picked, on its Effect tab or its Timing tab.
    auto options = [this, st](AnimationOptionsDialog::Tab tab) {
        Page *p = st->slide();
        if (!p) return;
        const deck::AnimSequence seq = deck::readAnimations(*p, m_ed->doc());
        const QVector<int> rows = st->targets(seq, *p);
        if (rows.isEmpty()) return;
        const deck::Anim shown = seq.effects[rows.first()];
        AnimationOptionsDialog dlg(shown, st->hasText(*p, shown.shape), tab, this);
        if (dlg.exec() != QDialog::Accepted) return;
        const deck::Anim r = dlg.result();
        const bool byParagraph = dlg.byParagraph();
        st->edit(tr("Effect Options"), [&](deck::AnimSequence &s, Page &page) {
            for (int i : Anims::withBuilds(s, rows)) {
                deck::Anim &e = s.effects[i];
                if (rows.contains(i)) e.start = r.start;   // each paragraph of a build keeps its own start
                if (e.custom()) continue;
                e.variant = r.variant;
                e.amount = r.amount;
                e.smoothStart = r.smoothStart;
                e.smoothEnd = r.smoothEnd;
                e.delay = r.delay;
                if (e.duration > 0) e.duration = r.duration;
                e.repeat = r.repeat;
                e.rewind = r.rewind;
                e.after = e.cls == deck::EffectClass::Exit ? deck::AfterEffect::None : r.after;
            }
            const int row = rows.first();
            if (!s.effects[row].custom() && s.effects[row].paragraphs() != byParagraph) deck::setByParagraph(s, row, byParagraph, *st->doc(), page);
        }, true);
    };
    mk("anim.options", tr("Effect Options…"), "sliders-horizontal", QKeySequence(), [options] { options(AnimationOptionsDialog::EffectTab); });
    mk("anim.timing", tr("Timing…"), "clock", QKeySequence(), [options] { options(AnimationOptionsDialog::TimingTab); });
    mk("anim.start.click", tr("On Click"), QString(), QKeySequence(), [st] { st->setStart(deck::EffectStart::OnClick); }, true);
    mk("anim.start.with", tr("With Previous"), QString(), QKeySequence(), [st] { st->setStart(deck::EffectStart::WithPrevious); }, true);
    mk("anim.start.after", tr("After Previous"), QString(), QKeySequence(), [st] { st->setStart(deck::EffectStart::AfterPrevious); }, true);
    mk("anim.remove", tr("Remove"), "trash-2", QKeySequence(), [st] { st->clear(); });

    // The pane asks.
    connect(m_animPane, &AnimationPane::rowsChosen, this, [st](const QVector<int> &rows) {
        st->picked = rows;
        st->selectShapesOf(rows);
    });
    connect(m_animPane, &AnimationPane::moveRequested, this, [st](int from, int to) {
        st->edit(QCoreApplication::translate("Animation", "Reorder Animation"), [&](deck::AnimSequence &seq, Page &) { seq.move(from, to); });
        st->picked = {std::clamp(to, 0, std::max(0, int(st->sequence().effects.size()) - 1))};
        st->pane->selectRows(st->picked);
        st->picksChanged();
    });
    connect(m_animPane, &AnimationPane::reorderRequested, this, [st](int by) {
        const QVector<int> rows = st->pane->selectedRows();
        if (!rows.isEmpty()) st->moveRows(rows, by);
    });
    connect(m_animPane, &AnimationPane::removeRequested, this, [st](const QVector<int> &rows) { st->removeRows(rows); });
    connect(m_animPane, &AnimationPane::optionsRequested, this, [this](int) { act("anim.options")->trigger(); });
    connect(m_animPane, &AnimationPane::menuRequested, this, [this, st](int row, const QPoint &at) {
        const deck::Anim *a = st->pane->effect(row);
        auto *menu = new QMenu(m_animPane);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        const struct { const char *id; deck::EffectStart start; } starts[] = {{"anim.start.click", deck::EffectStart::OnClick}, {"anim.start.with", deck::EffectStart::WithPrevious},
                                                                           {"anim.start.after", deck::EffectStart::AfterPrevious}};
        for (const auto &s : starts) {
            QAction *item = act(QString::fromLatin1(s.id));
            item->setEnabled(a != nullptr);
            item->setChecked(a && a->start == s.start);
            menu->addAction(item);
        }
        menu->addSeparator();
        menu->addAction(act("anim.options"));
        menu->addAction(act("anim.timing"));
        menu->addSeparator();
        menu->addAction(act("anim.remove"));
        menu->popup(at);
    });

    // Which shapes the pane's picks and the slide's selection mean follow each other.
    connect(m_ed, &Editor::selectionChanged, this, [st] {
        if (st->pickingShapes) return;
        st->picked.clear();
        Page *p = st->slide();
        QSet<int> numbers;
        if (p)
            for (int n : st->selectedShapes(*p, false)) numbers.insert(n);
        st->pane->selectShapes(numbers);
        // Animation Painter: the next shape selected gets the animation.
        if (st->painterFrom <= 0 || !p) return;
        QString fromId;
        walkItems(p->items, [&](const ItemPtr &it) {
            if (it->sourceId == st->painterFrom && fromId.isEmpty()) fromId = it->id;
        });
        QStringList others = st->ed->selection();
        others.removeAll(fromId);
        if (others.isEmpty()) return;
        const int from = st->painterFrom;
        st->painterFrom = 0;
        st->edit(QCoreApplication::translate("Animation", "Animation Painter"), [&](deck::AnimSequence &seq, Page &page) {
            QVector<int> to;
            for (const QString &id : others) {
                const int n = deck::shapeNumber(page, id);
                if (n > 0) to << n;
            }
            deck::copyEffects(seq, from, to);
        });
    });

    // The numbers by the shapes show while the tab or the pane does.
    auto badges = [canvas = QPointer<DeckCanvas>(m_canvas), dock = QPointer<QDockWidget>(m_animDock), ribbon = QPointer<Ribbon>(m_ribbon), st] {
        if (!canvas || !dock || !ribbon) return;   // the window is going away
        QVector<DeckCanvas::AnimationBadge> list;
        const bool shown = dock->isVisible() || (ribbon->current() && ribbon->current() == ribbon->tab(QStringLiteral("Animations")));
        Page *p = st->slide();
        if (shown && p && st->ed->masterView().isEmpty()) {
            const deck::AnimSequence &seq = st->sequence();
            const QVector<int> picked = st->pane->selectedRows();
            QHash<int, int> slots;
            for (int i = 0; i < seq.effects.size(); ++i) {
                const deck::Anim &a = seq.effects[i];
                QRectF box;
                bool found = false;
                walkItems(p->items, [&](const ItemPtr &it) {
                    if (!found && it->sourceId == a.shape) {
                        box = it->bounds();
                        found = true;
                    }
                });
                if (!found) continue;
                DeckCanvas::AnimationBadge b;
                b.at = box.topLeft();
                b.text = QString::number(seq.clickOf(i));
                b.picked = picked.contains(i);
                b.slot = slots[a.shape]++;
                list << b;
            }
        }
        canvas->setAnimationBadges(list);
    };
    connect(m_ed, &Editor::changed, this, badges);
    connect(m_ed, &Editor::pageChanged, this, badges);
    connect(m_ed, &Editor::documentReplaced, this, badges);
    connect(m_ribbon, &Ribbon::tabChanged, this, badges);
    connect(m_animDock, &QDockWidget::visibilityChanged, this, badges);
    connect(m_animPane, &AnimationPane::rowsChosen, this, badges);
    connect(m_ed, &Editor::pageChanged, this, [st] { st->picked.clear(); });

    // What the commands may do, from the slide and the selection.
    auto sync = [this, st] {
        Page *p = st->slide();
        const bool any = p && (!st->ed->selection().isEmpty() || !st->picked.isEmpty());
        const deck::AnimSequence &seq = st->sequence();
        const bool has = !seq.effects.isEmpty();
        act("anim.preview")->setEnabled(has);
        act("anim.painter")->setEnabled(p && st->selectedShapes(*p, false).size() == 1);
        const QVector<int> rows = p ? st->targets(seq, *p) : QVector<int>();
        act("anim.earlier")->setEnabled(!rows.isEmpty() && rows.first() > 0);
        act("anim.later")->setEnabled(!rows.isEmpty() && rows.last() < seq.effects.size() - 1);
        act("anim.options")->setEnabled(!rows.isEmpty());
        act("anim.timing")->setEnabled(!rows.isEmpty());
        act("anim.remove")->setEnabled(!rows.isEmpty());
        Q_UNUSED(any)
    };
    st->listeners << sync;
    // The listeners hold the controls that hold this, so they let go when the window does.
    QObject::connect(this, &QObject::destroyed, [st] { st->listeners.clear(); });
    connect(m_ed, &Editor::changed, this, sync);
    connect(m_ed, &Editor::selectionChanged, this, sync);
    connect(m_ed, &Editor::pageChanged, this, sync);
    connect(m_ed, &Editor::documentReplaced, this, sync);
    connect(m_animPane, &AnimationPane::rowsChosen, this, sync);
    sync();
    m_animations = st;
}

void DeckWindow::showAnimationPane(bool on)
{
    if (!m_animDock) return;
    m_animDock->setVisible(on);
    if (on) {
        m_animDock->raise();
        resizeDocks({m_animDock}, {320}, Qt::Horizontal);
    }
}

void DeckWindow::addAnimationParts(RibbonParts &parts)
{
    auto st = std::static_pointer_cast<Anims>(m_animations);
    auto syncs = std::make_shared<QList<std::function<void()>>>();
    auto syncAll = [syncs] {
        for (const auto &fn : *syncs) fn();
    };
    st->listeners << syncAll;
    connect(m_ed, &Editor::changed, this, syncAll);
    connect(m_ed, &Editor::selectionChanged, this, syncAll);
    connect(m_ed, &Editor::pageChanged, this, syncAll);
    connect(m_ed, &Editor::documentReplaced, this, syncAll);
    connect(m_animPane, &AnimationPane::rowsChosen, this, syncAll);

    // The kinds, in sections, for the gallery and for Add Animation.
    auto items = [this](bool none) {
        QVector<GalleryItem> list;
        if (none) list << GalleryItem{QStringLiteral("none"), tr("None"), noEffectIcon(), QString()};
        for (const deck::EffectKind &k : deck::effectKinds()) {
            const QString section = k.cls == deck::EffectClass::Entrance ? tr("Entrance") : k.cls == deck::EffectClass::Emphasis ? tr("Emphasis")
                                    : k.cls == deck::EffectClass::Exit ? tr("Exit") : tr("Motion Paths");
            list << GalleryItem{k.id, k.name, effectIcon(k, QString(), true), section};
        }
        return list;
    };
    auto itemsNone = std::make_shared<std::function<QVector<GalleryItem>(bool)>>(items);
    parts.widgets[QStringLiteral("animationGallery")] = [this, st, syncs, itemsNone] {
        auto *gallery = new Gallery(kGalleryItem, 5);
        gallery->setObjectName(QStringLiteral("animationGallery"));
        gallery->setTitle(tr("Animation"));
        gallery->setItems((*itemsNone)(true));
        auto pick = [this](const QString &id) { act(id == QLatin1String("none") ? QStringLiteral("anim.none") : QStringLiteral("anim.set.") + id)->trigger(); };
        connect(gallery, &Gallery::activated, this, pick);
        // Show all opens the whole list nine across, so that each section's eight effects are in one row.
        const QList<QToolButton *> buttons = gallery->findChildren<QToolButton *>();
        if (!buttons.isEmpty()) {
            QToolButton *all = buttons.last();
            all->disconnect(gallery);
            connect(all, &QToolButton::clicked, gallery, [gallery, itemsNone, pick] { galleryPopup((*itemsNone)(true), kGalleryItem, 9, {}, pick, gallery); });
        }
        syncs->append([gallery, st] {
            Page *p = st->slide();
            const QVector<int> rows = p ? st->targets(st->sequence(), *p) : QVector<int>();
            gallery->setEnabled(p && (!st->ed->selection().isEmpty() || !rows.isEmpty()));
            const deck::Anim *a = rows.isEmpty() ? nullptr : &st->sequence().effects[rows.first()];
            gallery->setCurrent(a && !a->custom() ? a->kind : QString());
        });
        syncs->last()();
        return gallery;
    };
    parts.widgets[QStringLiteral("addAnimation")] = [this, st, syncs, itemsNone] {
        auto *button = new GalleryButton(icon("plus"), tr("Add Animation"), kGalleryItem, 9, true);
        button->setObjectName(QStringLiteral("addAnimation"));
        button->setItemsProvider([itemsNone] { return (*itemsNone)(false); });
        connect(button, &GalleryButton::activated, this, [this](const QString &id) { act(QStringLiteral("anim.add.") + id)->trigger(); });
        syncs->append([button, st] { button->setEnabled(st->slide() && !st->ed->selection().isEmpty()); });
        syncs->last()();
        return button;
    };

    // Effect Options: what the effect picked has to choose from.
    parts.menus[QStringLiteral("animEffectOptions")] = [this, st](QWidget *owner) {
        auto *menu = new QMenu(owner);
        auto fill = [this, st, menu] {
            menu->clear();
            Page *p = st->slide();
            const deck::AnimSequence seq = p ? deck::readAnimations(*p, m_ed->doc()) : deck::AnimSequence();
            const QVector<int> rows = p ? st->targets(seq, *p) : QVector<int>();
            const deck::Anim *a = rows.isEmpty() ? nullptr : &seq.effects[rows.first()];
            const deck::EffectKind *kind = a && !a->custom() ? deck::effectKind(a->kind) : nullptr;
            if (!kind) {
                menu->addAction(tr("No Effect Options"))->setEnabled(false);
                return;
            }
            const bool size = kind->amount == deck::EffectKind::Amount::Size, spin = kind->amount == deck::EffectKind::Amount::Spin;
            if (kind->variants.size() > 1) {
                menu->addSection(kind->cls == deck::EffectClass::MotionPath ? tr("Path") : tr("Direction"));
                for (const deck::EffectVariant &v : kind->variants) {
                    QAction *item = menu->addAction(v.name);
                    item->setCheckable(true);
                    item->setChecked(a->variant == v.key);
                    const QString key = v.key;
                    connect(item, &QAction::triggered, menu, [st, key] { st->change(QCoreApplication::translate("Animation", "Effect Options"), [key](deck::Anim &e) { if (!e.custom()) e.variant = key; }); });
                }
            }
            if (size || spin) {
                menu->addSection(tr("Amount"));
                struct Preset { QString name; double value; };
                const QVector<Preset> presets = size ? QVector<Preset>{{tr("Tiny"), 25}, {tr("Smaller"), 50}, {tr("Larger"), 150}, {tr("Huge"), 400}}
                                                     : QVector<Preset>{{tr("Quarter Spin"), 90}, {tr("Half Spin"), 180}, {tr("Full Spin"), 360}, {tr("Two Spins"), 720}};
                for (const Preset &pr : presets) {
                    QAction *item = menu->addAction(pr.name);
                    item->setCheckable(true);
                    item->setChecked(qFuzzyCompare(a->amount, pr.value));
                    const double value = pr.value;
                    connect(item, &QAction::triggered, menu, [st, value] { st->change(QCoreApplication::translate("Animation", "Effect Options"), [value](deck::Anim &e) { if (!e.custom()) e.amount = value; }); });
                }
            }
            if (st->hasText(*p, a->shape)) {
                menu->addSection(tr("Sequence"));
                for (bool byParagraph : {false, true}) {
                    QAction *item = menu->addAction(byParagraph ? tr("By Paragraph") : tr("As One Object"));
                    item->setCheckable(true);
                    item->setChecked(a->paragraphs() == byParagraph);
                    const int row = rows.first();
                    connect(item, &QAction::triggered, menu, [st, row, byParagraph] {
                        st->edit(QCoreApplication::translate("Animation", "Effect Options"), [&](deck::AnimSequence &s, Page &page) { deck::setByParagraph(s, row, byParagraph, *st->doc(), page); });
                    });
                }
            }
            menu->addSeparator();
            QAction *more = menu->addAction(icon("sliders-horizontal"), tr("Show Additional Effect Options…"));
            connect(more, &QAction::triggered, menu, [this] { act("anim.options")->trigger(); });
        };
        connect(menu, &QMenu::aboutToShow, menu, fill);
        fill();
        return menu;
    };

    // Timing: start, duration, and delay, of the effects picked.
    parts.widgets[QStringLiteral("animStart")] = [this, st, syncs] {
        auto *box = new QComboBox();
        box->setObjectName(QStringLiteral("animStartBox"));
        box->setAccessibleName(tr("Start"));
        box->setToolTip(tr("Start: when the effect begins, at a click, with the effect before it, or when that one ends."));
        for (deck::EffectStart s : {deck::EffectStart::OnClick, deck::EffectStart::WithPrevious, deck::EffectStart::AfterPrevious}) box->addItem(startLabel(s), int(s));
        box->setFixedWidth(140);
        connect(box, &QComboBox::activated, this, [st, box](int i) { st->setStart(deck::EffectStart(box->itemData(i).toInt())); });
        syncs->append([box, st] {
            Page *p = st->slide();
            const deck::AnimSequence &seq = st->sequence();
            const QVector<int> rows = p ? st->targets(seq, *p) : QVector<int>();
            const QSignalBlocker block(box);
            box->setEnabled(!rows.isEmpty());
            int shown = -1;
            if (!rows.isEmpty()) {
                shown = int(seq.effects[rows.first()].start);
                for (int r : rows)
                    if (int(seq.effects[r].start) != shown) shown = -1;
            }
            box->setCurrentIndex(shown < 0 ? -1 : box->findData(shown));
        });
        syncs->last()();
        return box;
    };
    auto seconds = [this, st, syncs](const QString &name, const QString &tip, double minimum, bool duration) {
        auto *spin = new QDoubleSpinBox();
        spin->setObjectName(name);
        spin->setRange(minimum, 59);
        spin->setDecimals(2);
        spin->setSingleStep(0.25);
        spin->setSuffix(tr(" s"));
        spin->setKeyboardTracking(false);
        spin->setFixedWidth(84);
        spin->setToolTip(jp::plainToolTip(tip));
        spin->setAccessibleName(duration ? tr("Duration") : tr("Delay"));
        connect(spin, &QDoubleSpinBox::valueChanged, this, [st, duration](double v) {
            const int ms = qRound(v * 1000);
            st->change(duration ? QCoreApplication::translate("Animation", "Duration") : QCoreApplication::translate("Animation", "Delay"), [ms, duration](deck::Anim &e) {
                if (e.custom()) return;
                if (duration) {
                    if (e.duration > 0) e.duration = ms;
                } else {
                    e.delay = ms;
                }
            });
        });
        syncs->append([spin, st, duration] {
            Page *p = st->slide();
            const deck::AnimSequence &seq = st->sequence();
            const QVector<int> rows = p ? st->targets(seq, *p) : QVector<int>();
            const QSignalBlocker block(spin);
            const deck::Anim *a = rows.isEmpty() ? nullptr : &seq.effects[rows.first()];
            spin->setEnabled(a && !a->custom() && (!duration || a->duration > 0));
            if (a) spin->setValue((duration ? std::max(a->duration, 10) : a->delay) / 1000.0);
        });
        syncs->last()();
        return spin;
    };
    parts.widgets[QStringLiteral("animDuration")] = [seconds] { return seconds(QStringLiteral("animDuration"), QCoreApplication::translate("Animation", "Duration: how many seconds the effect takes."), 0.01, true); };
    parts.widgets[QStringLiteral("animDelay")] = [seconds] { return seconds(QStringLiteral("animDelay"), QCoreApplication::translate("Animation", "Delay: how many seconds the effect waits before it begins."), 0, false); };
}

} // namespace jp

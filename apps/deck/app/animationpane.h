#pragma once
// The Animation Pane, on the right of the window: the slide's effects in the
// order they play, each with its click number, a picture of its kind, the
// name of its shape, and a bar for when it plays after its click. A click on
// a row selects it (and its shape on the slide); a drag, or the arrows,
// reorder; the buttons play the effects (Play All, Play From the selected
// one); a menu on each row starts it another way, opens its options and
// timing, or removes it. The pane shows the sequence and says what was asked;
// the window changes it (deckwindow_animations.cpp), one undo step at a time,
// and the rows follow whatever changes the slide's animations, an undo included.

#include "core/animation.h"

#include <QColor>
#include <QElapsedTimer>
#include <QIcon>
#include <QImage>
#include <QSet>
#include <QTimer>
#include <QWidget>

class QLabel;
class QListWidget;
class QPushButton;

namespace jp {

class Document;
class Editor;
class LayoutCache;

// What an effect's kind looks like halfway through, drawn small on a slide
// (a motion path: the path itself): the gallery's and the pane's pictures.
// `named` puts the effect's name under the picture, as the gallery shows it.
QIcon effectIcon(const deck::EffectKind &kind, const QString &variant = QString(), bool named = false);
// The gallery's picture for no effect, with its name.
QIcon noEffectIcon();
// The color of an effect's class: entrance green, emphasis amber, exit red, motion path blue, kept gray.
QColor effectColor(deck::EffectClass cls);
// "Fade", "Fly In: From Left", "Lines: Down", or "Custom" for what is kept as it was written.
QString effectLabel(const deck::Anim &anim);
// "On Click", "With Previous", "After Previous".
QString startLabel(deck::EffectStart start);
// "Entrance", "Emphasis", "Exit", "Motion Path": the section a kind is in.
QString effectClassLabel(deck::EffectClass cls);

class AnimationPane : public QWidget {
    Q_OBJECT
public:
    explicit AnimationPane(Editor *ed, QWidget *parent = nullptr);
    ~AnimationPane() override;
    struct Row;

    QListWidget *list() const { return m_list; }
    int rowCount() const;
    // The rows picked, in order.
    QVector<int> selectedRows() const;
    // Picks these rows, without telling anyone.
    void selectRows(const QVector<int> &rows);
    // Picks the rows whose shapes have these numbers, without telling anyone.
    void selectShapes(const QSet<int> &numbers);
    // The effect a row stands for, as the sequence has it.
    const deck::Anim *effect(int row) const;
    // Lists the effects again when the slide's are not what the rows show.
    void reload();
    // The row playing now (its click step), drawn marked; -1 for none.
    void setPlaying(int row);

Q_SIGNALS:
    // The user picked rows.
    void rowsChosen(const QVector<int> &rows);
    // The effect at `from` is to move to `to`.
    void moveRequested(int from, int to);
    // The Re-Order arrows: earlier (-1) or later (1).
    void reorderRequested(int by);
    void playRequested(int fromRow);      // -1: all of them
    void menuRequested(int row, const QPoint &global);
    void optionsRequested(int row);
    void removeRequested(const QVector<int> &rows);

protected:
    QSize sizeHint() const override { return QSize(300, 400); }
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void rebuild();
    QListWidget *m_list = nullptr;
    QPushButton *m_playAll = nullptr, *m_playFrom = nullptr, *m_earlier = nullptr, *m_later = nullptr;
    QLabel *m_empty = nullptr;
    Editor *m_ed;
    deck::AnimSequence m_seq;
    QVector<Row> m_rows;
    int m_scale = 1000;       // ms the bars run to
    QString m_page;
    quint64 m_signature = 0;
    bool m_quiet = false;
};

// Plays a slide's effects over a widget (the editing canvas), one click step after another with no click between, as the
// Preview and Play buttons do; a click or Esc ends it, and it hides itself when it is over.
class AnimationPreview : public QWidget {
    Q_OBJECT
public:
    AnimationPreview(const Document *doc, LayoutCache *cache, QWidget *parent);
    // Plays the effects of `slide` from click step `fromStep` (0 is the first), the slide drawn in `area` of the widget.
    // False when the slide has nothing to play.
    bool play(int slide, const QRect &area, int fromStep = 0);
    bool isPlaying() const { return m_timer.isActive(); }
    // Ends it at once (the presentation it plays is gone): no step is drawn after this.
    void cancel()
    {
        m_timer.stop();
        hide();
        deleteLater();
    }
    // The click step playing.
    int step() const { return m_step; }

Q_SIGNALS:
    void stepChanged(int step);
    void finished();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override { stop(); }
    void keyPressEvent(QKeyEvent *e) override;

private:
    void tick();
    void stop();
    const Document *m_doc;
    LayoutCache *m_cache;
    QTimer m_timer;
    QElapsedTimer m_clock;
    deck::Timeline m_timeline;
    deck::Geometry m_geometry;
    QVector<double> m_starts;     // ms into the preview that each click step begins
    double m_total = 0;
    int m_slide = 0, m_first = 0, m_step = -1;
    QRect m_area;
    QImage m_frame;
};

} // namespace jp

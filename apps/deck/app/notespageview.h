#pragma once
// The Notes Page view and the Notes Master and Handout Master views: one page the way it prints, in a scroll area with its own zoom.
//
// Notes Page shows the page of the slide being edited: the slide's picture above its notes, laid out from the notes master (header, date, footer, and number where they are
// on). The notes are in a box on the page that is typed in; what is typed is saved as one undo step once typing pauses, as the notes under the slide are. PageUp and PageDown
// go to the previous and next slide.
// Notes Master shows the places of the notes page (the slide picture, the notes, the header, date, footer, and number) and Handout Master those of a handout (where its slides
// go, and the header, date, footer, and number). Each is dragged to move it and by its lower right corner to size it; each change is one undo step.

#include "core/notesmaster.h"

#include <QAbstractScrollArea>
#include <QColor>
#include <QRectF>

namespace jp {

class Editor;
class NotesPane;

class NotesPageView : public QAbstractScrollArea {
    Q_OBJECT
public:
    enum class Mode { NotesPage, NotesMaster, HandoutMaster };
    explicit NotesPageView(Editor *ed, QWidget *parent = nullptr);

    Mode mode() const { return m_mode; }
    void setMode(Mode mode);
    // Pixels across the screen for each point of the page is zoom * the screen's dots to the inch / 72; 1 is a page as wide as it is printed.
    double zoom() const { return m_zoom; }
    void setZoom(double zoom);
    double fitZoom() const;
    // Zooms to fit the page in the view: now when the view has its size, else as soon as it has.
    void fitToView();
    // The page's size in points: the locale's paper (a handout's turned across when its master says so).
    QSizeF paper() const;
    // Where the page and its places are in the viewport.
    QRectF pageRect() const;
    QRectF slideImageRect() const;   // the slide's picture (the notes page: as drawn, in the slide's shape)
    QRectF notesRect() const;
    QRectF placeRect(deck::NotesPart part) const;
    QRectF placeRect(deck::HandoutPart part) const;
    NotesPane *notesBox() const { return m_box; }
    // The label of a place, as drawn: its words cut short with "..." to the box's width, in a size that fits its height (a box too small for any words has none). Static so a test can check
    // that what is drawn fits at every zoom.
    static QFont labelFont(const QFont &base, const QRectF &box, double scale);
    static QString labelText(const QString &text, const QFont &font, const QRectF &box);
    // How many times the layout was read from the presentation (the layouts are kept while the document is as it was, so a drag does not read them for each move).
    int layoutReads() const { return m_reads; }
    void commit();   // saves the notes typed
    void reload();

    // Changes the selected place's rectangle and records it as a step (what a drag does when it ends), or clears the layout to the standard.
    void setPlace(deck::NotesPart part, const QRectF &fractions);
    void setPlace(deck::HandoutPart part, const QRectF &fractions);
    void resetLayout();
    void setBackground(const QColor &color);   // an invalid color: the page's own white
    void setHandoutCount(int perPage);
    void setHandoutLandscape(bool landscape);

Q_SIGNALS:
    void zoomChanged(double zoom);

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private:
    struct Hit {
        int part = -1;   // NotesPart or HandoutPart as an int, -1 for none
        bool corner = false;
    };
    double scale() const;   // pixels to the point
    void layoutBox();
    void updateRanges();
    Hit hitAt(const QPointF &at) const;
    QRectF toView(const QRectF &fractions) const;
    QRectF toFractions(const QRectF &view) const;
    const deck::NotesMaster &notesNow() const;
    const deck::HandoutMaster &handoutNow() const;
    void paintMaster(QPainter &p);
    void change(const QString &label, const std::function<void(Document &)> &fn);

    Editor *m_ed;
    NotesPane *m_box;
    Mode m_mode = Mode::NotesPage;
    double m_zoom = 1;
    Hit m_drag;
    QPointF m_dragFrom;
    QRectF m_dragStart, m_dragNow;   // fractions
    int m_selected = -1;
    mutable bool m_notesKept = false, m_handoutKept = false;
    mutable deck::NotesMaster m_notesLayout;
    mutable deck::HandoutMaster m_handoutLayout;
    mutable int m_reads = 0;
    bool m_fitPending = false;
};

} // namespace jp

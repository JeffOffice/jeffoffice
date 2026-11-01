#pragma once
// The horizontal ruler above the pages: the text area of the page the caret is on between its margins, the indents of the paragraph the caret is in (first line, hanging, left,
// and right), and its tab stops. The markers are dragged with the mouse (and the margins too); a click on the ruler adds a tab stop of the kind the box at its left end shows, and a
// click on that box changes the kind. Each drag is one change to the document, so one Undo takes it back.

#include "core/rulerops.h"

#include <QTextOption>
#include <QWidget>

namespace jp::write {

class PageView;

class Ruler : public QWidget {
    Q_OBJECT
public:
    explicit Ruler(PageView *view, QWidget *parent = nullptr);
    QSize sizeHint() const override { return QSize(200, kHeight); }
    QSize minimumSizeHint() const override { return QSize(50, kHeight); }

    static constexpr int kHeight = 30;
    // The kinds of marker, for tests and for the drag in progress.
    enum class Marker { None, FirstLine, Hanging, LeftIndent, RightIndent, LeftMargin, RightMargin, TabStop, TabKind, Blank };
    // Where the marker is in the ruler (its center, in pixels); tab stops are counted from 0 in order. False where it is not shown.
    bool markerPoint(Marker marker, int index, QPointF *point) const;
    // The marker at a point of the ruler, and for a tab stop its index.
    Marker markerAt(const QPointF &point, int *index = nullptr) const;
    // The kind of tab stop a click adds.
    QTextOption::TabType tabKind() const { return m_tabKind; }
    void setTabKind(QTextOption::TabType kind);
    // Whether the ruler shows a paragraph at all (a document, with the pages laid out as far as the caret).
    bool shows() const { return m_state.valid; }
    // The ruler's own numbers, in points: the left edge of the text area from the page's, its width, and the paragraph's indents (as the markers show them while one is dragged).
    double textLeft() const { return m_state.textLeft; }
    double textWidth() const { return m_state.textWidth; }
    double leftIndent() const { return m_state.left; }
    double rightIndent() const { return m_state.right; }
    double firstLineIndent() const { return m_state.first; }
    QVector<TabStopEntry> tabStops() const { return m_state.tabs; }

public Q_SLOTS:
    // Reads the caret's paragraph and the page again and draws.
    void sync();

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    bool event(QEvent *e) override;

private:
    struct State {
        bool valid = false;
        double pageLeftPx = 0;     // the page's left edge, in this widget
        double scale = 1;          // pixels to the point
        double paperWidth = 612, textLeft = 72, textWidth = 468;
        double left = 0, right = 0, first = 0;   // the paragraph's indents (the first line's from the left indent)
        QVector<TabStopEntry> tabs;
        int section = 0;
    };
    double xOf(double pointsFromText) const { return m_state.pageLeftPx + (m_state.textLeft + pointsFromText) * m_state.scale; }
    double pointsAt(double x) const { return (x - m_state.pageLeftPx) / m_state.scale - m_state.textLeft; }
    double snapped(double points, Qt::KeyboardModifiers modifiers) const;
    State read() const;
    void commit();
    QTextCursor cursor() const;

    PageView *m_view;
    State m_state;
    QTextOption::TabType m_tabKind = QTextOption::LeftTab;
    // The drag in progress: the marker, its tab stop's index and where that stop was, and the value it has now (in points, as that marker counts).
    Marker m_drag = Marker::None;
    int m_dragIndex = -1;
    double m_dragFrom = 0, m_dragValue = 0;
    bool m_dragMoved = false;
    bool m_dragOut = false;
};

} // namespace jp::write

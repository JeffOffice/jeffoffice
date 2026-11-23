#pragma once
// The style area: a strip down the left edge of Draft and Outline view that names the style of each paragraph beside its first line. It is a child of the scroll area it
// belongs to (the pages or the outline), takes its width from the area's viewport margin, and asks a function for the rows on show: the work is for those paragraphs only.
// A click selects the paragraph, a double click asks for its style's box, and the edge toward the text is dragged to change the width.

#include <QPointer>
#include <QString>
#include <QVector>
#include <QWidget>
#include <functional>

class QAbstractScrollArea;
class QLabel;

namespace jp::write {

struct StyleRow {
    int top = 0, height = 0;   // in the scroll area's viewport, in pixels
    int position = 0;          // where the paragraph begins in the text
    QString name;
};

class StyleArea : public QWidget {
    Q_OBJECT
public:
    using RowSource = std::function<QVector<StyleRow>()>;
    static constexpr int kLeastWidth = 30, kMostWidth = 400;

    // `setMargin` makes the room at the left of the host's viewport (setViewportMargins is the host's own to call).
    StyleArea(QAbstractScrollArea *host, RowSource source, std::function<void(int)> setMargin);
    // The width in pixels: 0 hides the strip (and it then costs nothing); any other is kept between kLeastWidth and kMostWidth.
    void setAreaWidth(int width);
    int areaWidth() const { return m_width; }
    // The rows as last made, and the names beside them.
    QVector<StyleRow> rows() const { return m_rows; }
    QStringList shownNames() const;
    // Makes the rows again now (what is on show); scheduleRefresh waits for the event loop, so that a burst of changes makes them once.
    void refresh();
    void scheduleRefresh();
    // How many times the rows were made (a cost test counts them).
    qint64 refreshes() const { return m_refreshes; }
    // The row whose name is at a point of the strip, or -1.
    int rowAt(const QPoint &point) const;
    // Whether the name of a row is cut short to fit the strip.
    bool nameCut(int row) const;
    // The row the keyboard is on, or -1.
    int currentRow() const { return m_current; }

Q_SIGNALS:
    void rowClicked(int position);
    void rowDoubleClicked(int position);
    void widthDragged(int width);

protected:
    bool event(QEvent *e) override;
    bool eventFilter(QObject *watched, QEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void inputMethodEvent(QInputMethodEvent *e) override;
    void hideEvent(QHideEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

private:
    bool onEdge(const QPoint &p) const;
    void place();
    void fitLabels();
    QAbstractScrollArea *m_host;
    RowSource m_source;
    std::function<void(int)> m_setMargin;
    int m_width = 0;
    QVector<StyleRow> m_rows;
    QVector<QLabel *> m_labels;   // one for each row on show (kept for the next ones)
    bool m_pending = false, m_dragging = false;
    int m_dragFrom = 0, m_dragWidth = 0, m_current = -1;
    qint64 m_refreshes = 0;
};

} // namespace jp::write

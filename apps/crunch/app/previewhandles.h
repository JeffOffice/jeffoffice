#pragma once
// The handles over the Print screen's preview of a page: a guide for each of the four margins and a handle on the right edge of each column the page shows. A handle is dragged with the mouse
// or moved with the arrow keys when it has the focus (it is a control of its own, with a name a screen reader says); either way the one new place is given to `moved` once, so a drag is one
// step of Undo. The handles are told where the page is in the preview and where each one is in the page's own points; they never change the sheet themselves.

#include <QObject>
#include <QRectF>
#include <QSizeF>
#include <QString>

#include <functional>
#include <vector>

class QWidget;

namespace jp {

class PreviewHandle;

class PreviewHandles : public QObject {
public:
    enum class Kind { MarginLeft, MarginRight, MarginTop, MarginBottom, ColumnRight };
    struct Spec {
        Kind kind = Kind::MarginLeft;
        qint64 column = 0;          // for a column's handle
        double position = 0;        // across the page for a vertical handle, down it for a horizontal one (points from the page's top left)
        double from = 0, to = 0;    // how far along the other way it reaches (points)
        double lowest = 0, highest = 0;   // the places it may be moved between (points)
        QString name;
    };
    // The handles are made on `surface`, the widget the preview is drawn on.
    explicit PreviewHandles(QWidget *surface);
    // The handles as these say, the page being at `page` (pixels of the surface) and `points` in size. The ones already there stay (and keep the focus) when they are named again.
    void place(const QRectF &page, const QSizeF &points, const std::vector<Spec> &specs);
    void clear();
    int count() const;
    // Called with the handle's kind and column and the new place in points, when a drag ends or a key moves it.
    std::function<void(Kind, qint64, double)> moved;
    // How far an arrow key moves a handle, in points.
    static constexpr double kKeyStep = 3.6;

private:
    friend class PreviewHandle;
    QWidget *m_surface;
    QRectF m_page;
    QSizeF m_points;
    std::vector<PreviewHandle *> m_handles;
};

} // namespace jp

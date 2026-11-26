#pragma once
// The bullets and numbers of the list items in a text view (a QTextEdit): a text view lays a list item's first line where the printed page puts the marker, so the marker is
// off the edge of the view (and where a level is indented, it is Qt's own plain disc or number, which knows nothing of a bullet's character, font, size, and color). This puts a
// gutter on the left of the text (or the right, for a right-to-left paragraph), only while the words in view have a list item, and draws the markers of the paragraphs in view
// itself: the text is the page text engine's (listMarkerText, so a number counts as it does on the printed page), the font, size, and color are what the list says. The words, the
// story, and the formats are never changed. The work of a paint is bounded by the paragraphs in view.

#include "core/color.h"
#include "core/export.h"

#include <QFont>
#include <QHash>
#include <QImage>
#include <QObject>
#include <QPointer>
#include <QRectF>
#include <QString>
#include <QVector>
#include <array>
#include <functional>

class QTextEdit;
class QTextBlock;

namespace jp {

class JO_COMMON_EXPORT ListMarkers : public QObject {
    Q_OBJECT
public:
    // `margins` puts the gutter on the view (QAbstractScrollArea::setViewportMargins is the view's own to call: the left and right widths); `colors` gives the scheme a bullet's theme
    // color is of (null: the standard one).
    ListMarkers(QTextEdit *edit, std::function<void(int, int)> margins, std::function<const ColorScheme *()> colors = {});
    ~ListMarkers() override;

    // How big the words are shown, as a multiple of the size the document gives them (see StoryEdit::setDisplayScale); the markers follow.
    void setScale(double scale);
    // The words, the document the view shows, its size, or its scrolling changed: the gutter and the markers follow. Cheap when nothing changed.
    void refresh();

    // The gutter's width on each side of the text now, in pixels (0: no list item in view).
    int gutterLeft() const { return m_left; }
    int gutterRight() const { return m_right; }

    // The markers of the paragraphs in view, as they are drawn: the paragraph's place in the document, the marker's text, and where it is (in the view widget's own coordinates).
    struct Marker {
        int blockPos = 0;
        QString text;
        QRectF rect;
        double baseline = 0;       // (y, the baseline of the paragraph's first line)
        bool rightToLeft = false;
    };
    QVector<Marker> visibleMarkers() const;

    // Counted work (for the tests): the markers whose look was made (a paragraph's marker is made once for a state of the words and the size), and the times the paragraphs in view
    // were gone through.
    static int looksBuilt() { return s_built; }
    static int viewRuns() { return s_runs; }

protected:
    bool eventFilter(QObject *watched, QEvent *e) override;

private:
    struct Look {
        QString text;
        QFont font;
        QColor color;
        QImage picture;
        double width = 0, ascent = 0, height = 0;
    };
    class Layer;
    const Look &lookOf(const QTextBlock &b);
    void run(bool margins);
    void paint(QPainter &p);
    QTextEdit *m_edit;
    std::function<void(int, int)> m_margins;
    std::function<const ColorScheme *()> m_colors;
    Layer *m_layer = nullptr;
    double m_scale = 1.0;
    QHash<int, Look> m_looks;
    std::array<qint64, 4> m_lookKey = {};
    std::array<qint64, 8> m_viewKey = {}, m_gutterKey = {};   // (what the paragraphs in view were gone through for, and what the gutter was last worked out for)
    QVector<int> m_view;   // the list items in view, by position
    bool m_busy = false, m_pending = false;
    int m_left = 0, m_right = 0;
    static inline int s_built = 0;
    static inline int s_runs = 0;
};

} // namespace jp

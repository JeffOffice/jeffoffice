#pragma once
// Controls JeffDeck's ribbon and menus hold: the slide layouts as small
// pictures to choose from, and the theme's colors.

#include <QImage>
#include <QVector>
#include <QWidget>

namespace jp {

class Document;
class Editor;
struct MasterPage;

// A small picture of a slide layout, `width` pixels wide: the background,
// what the slide master draws on every slide, and the layout's placeholders
// with their sample text, outlined where a placeholder holds none.
QImage layoutPicture(const Document &d, const MasterPage &layout, int width);

// The presentation's layouts, each as its picture with its name below, three
// across. Arrow keys move between them and Enter chooses; the current slide's
// layout has an accent outline.
class LayoutGallery : public QWidget {
    Q_OBJECT
public:
    explicit LayoutGallery(Editor *ed, QWidget *parent = nullptr);
    // Reads the layouts again, marking the current slide's.
    void reload();
    int count() const { return int(m_tiles.size()); }
    QString typeAt(int i) const { return m_tiles.value(i).type; }
    QSize sizeHint() const override;

Q_SIGNALS:
    void chosen(const QString &layoutType);

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

private:
    struct Tile {
        QString type, name;
        QImage picture;
    };
    QRect tileRect(int i) const;
    int tileAt(const QPoint &p) const;
    Editor *m_ed;
    QVector<Tile> m_tiles;
    QString m_current;
    int m_hover = -1;
};

// The theme's twelve colors as two rows of swatches, each named in its tip.
class ThemeColors : public QWidget {
    Q_OBJECT
public:
    explicit ThemeColors(Editor *ed, QWidget *parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }
    // The swatch under a point, or -1.
    int swatchAt(const QPoint &p) const;
    QRect swatchRect(int i) const;

protected:
    void paintEvent(QPaintEvent *) override;
    bool event(QEvent *e) override;

private:
    Editor *m_ed;
};

} // namespace jp

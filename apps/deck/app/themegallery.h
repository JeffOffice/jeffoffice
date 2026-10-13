#pragma once
// The Design tab's galleries: the themes as pictures of their title slides,
// and the variants of the presentation's theme with the menu of colors,
// fonts, and background styles beside them.

#include <QFrame>
#include <QImage>
#include <QPointer>
#include <QVector>
#include <functional>

class QMenu;
class QToolButton;

namespace jp {

class Document;
class Editor;

// A picture of the Title Slide of built-in theme `theme` in its variant `variant`, `width` pixels across at `dpr` (kept once drawn).
QImage themePicture(int theme, int variant, int width, qreal dpr);

// Pictures to choose from, in rows of `columns` with the arrow keys moving between them and Enter choosing, each with a tip (and a name
// below it when `names`). The pictures are drawn when they are first needed. The current one has an accent outline.
class PictureGrid : public QWidget {
    Q_OBJECT
public:
    PictureGrid(int pictureWidth, int columns, bool names, QWidget *parent = nullptr);
    // What the tiles are: each one's name, and the colors to mark along the foot of its picture (none for none).
    struct Tile {
        QString name;
        QVector<QColor> chips;
    };
    void setTiles(const QVector<Tile> &tiles);
    // How the picture of tile `i` is drawn (`width` pixels across at `dpr`); called when it is first needed, and again after reload().
    void setPictures(const std::function<QImage(int tile, int width, qreal dpr)> &provider) { m_provider = provider; }
    void reload();   // the pictures are drawn again when next needed
    void setCurrent(int tile);
    int current() const { return m_current; }
    int count() const { return int(m_tiles.size()); }
    QString nameAt(int i) const { return m_tiles.value(i).name; }
    QRect tileRect(int i) const;   // in the grid's coordinates, empty when its row is scrolled out of view
    int tileAt(const QPoint &p) const;
    // The rows shown (0: all), the first of them, and how many rows there are.
    void setVisibleRows(int rows);
    int visibleRows() const { return m_visibleRows; }
    int firstRow() const { return m_firstRow; }
    void setFirstRow(int row);
    int rows() const { return (count() + m_columns - 1) / m_columns; }
    int columns() const { return m_columns; }
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }

Q_SIGNALS:
    void chosen(int tile);
    // A right click on a tile, or the menu key with one under the keyboard: where to put a menu.
    void contextRequested(int tile, const QPoint &globalPos);

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private:
    QSize pictureSize() const;
    const QImage &pictureOf(int i);
    int m_width, m_columns;
    bool m_names;
    QVector<Tile> m_tiles;
    QVector<QImage> m_pictures;
    std::function<QImage(int, int, qreal)> m_provider;
    int m_current = -1, m_hover = -1, m_visibleRows = 0, m_firstRow = 0;
};

// Design > Themes: a row of themes with buttons to scroll the rows and to open them all, as the other galleries have. A click applies
// the theme to every slide; a right click offers every slide, the selected slides, and Browse for Themes.
class ThemeGallery : public QFrame {
    Q_OBJECT
public:
    explicit ThemeGallery(Editor *ed, QWidget *parent = nullptr);
    // The inline row of themes, and the theme the presentation has (a built-in one's number, or -1).
    PictureGrid *grid() const { return m_grid; }
    int currentTheme() const { return m_grid->current(); }
    // Marks the presentation's theme.
    void reload();
    // The menu a right click on a theme offers: Apply to All Slides, Apply to Selected Slides, and Browse for Themes. The caller owns it.
    QMenu *contextMenu(int theme, QWidget *parent = nullptr);
    // Opens all the themes in a popup (what the last button does); null if it was shown already.
    QFrame *openAll();

Q_SIGNALS:
    void themeChosen(int theme, bool allSlides);
    void browseRequested();

private:
    void setUp(PictureGrid *grid);
    Editor *m_ed;
    PictureGrid *m_grid;
    QToolButton *m_up, *m_down;
};

// Design > Variants: the four variants of the presentation's theme as small pictures, and a button that opens the menu of Colors, Fonts,
// and Background Styles. A click on a variant chooses it.
class VariantGallery : public QFrame {
    Q_OBJECT
public:
    explicit VariantGallery(Editor *ed, QWidget *parent = nullptr);
    PictureGrid *grid() const { return m_grid; }
    QToolButton *moreButton() const { return m_more; }
    // The menu the button opens; the gallery doesn't own it.
    void setMenu(QMenu *menu);
    // Reads the presentation's variants again, and marks the one it shows (the pictures are drawn again only when the theme or its
    // colors or background changed).
    void reload();

Q_SIGNALS:
    void variantChosen(int variant);

private:
    Editor *m_ed;
    PictureGrid *m_grid;
    QToolButton *m_more;
    QString m_signature;
};

// The twelve background styles of a theme as swatches (four across, three down), the current one outlined.
class BackgroundStyles : public QWidget {
    Q_OBJECT
public:
    explicit BackgroundStyles(Editor *ed, QWidget *parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }
    QRect swatchRect(int style) const;
    int styleAt(const QPoint &p) const;
    int current() const;

Q_SIGNALS:
    void chosen(int style);

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    bool event(QEvent *e) override;

private:
    Editor *m_ed;
    int m_hover = -1;
};

} // namespace jp

#pragma once
// The slide show: the slides full screen, one at a time. Right, Down, Space,
// Enter, Page Down, N, or a click go forward; Left, Up, Backspace, Page Up,
// or P go back; Home and End jump to the first and last; a number and Enter
// go to that slide; B and W blank the screen black or white; Esc ends it.
// Hidden slides are skipped. After the last slide a black screen says the
// show has ended.

#include <QWidget>

namespace jp {

class Document;
class LayoutCache;

class SlideShow : public QWidget {
    Q_OBJECT
public:
    SlideShow(const Document *doc, LayoutCache *cache, int startSlide, QWidget *parent = nullptr);
    int currentSlide() const { return m_slide; }   // -1 at the end
    void next();
    void previous();
    void goTo(int slide);

Q_SIGNALS:
    void ended(int lastSlide);

protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    int nextShown(int from, int step) const;
    const Document *m_doc;
    LayoutCache *m_cache;
    int m_slide = 0;
    bool m_atEnd = false;
    QColor m_blank;            // a blanked screen's color; invalid when showing
    QString m_typed;           // digits typed before Enter
};

} // namespace jp

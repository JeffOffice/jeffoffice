#pragma once
// The formula bar. It is a line of text with the interface of one (QLineEdit), so everything that types into the bar, reads it, or points from it works as it always did; Expand makes the same
// widget several lines tall: the text is then drawn with its line breaks (the ones typed with Alt+Enter, and wrapped lines for a long formula), the caret moves by line with Up and Down, and a
// scroll bar and a handle at the bottom edge (drag it, or press Up or Down on it) change what shows and how tall the bar is.

#include <QLineEdit>
#include <QVariant>
#include <QTextLayout>
#include <QVector>

class QScrollBar;

namespace jp {

class FormulaBarGrip;

class FormulaBarEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit FormulaBarEdit(QWidget *parent = nullptr);

    static constexpr int kMinLines = 2, kMaxLines = 20, kDefaultLines = 5;

    bool isExpanded() const { return m_expanded; }
    void setExpanded(bool on);
    // How many lines tall the bar is while expanded (kMinLines to kMaxLines).
    int expandedLines() const { return m_lines; }
    void setExpandedLines(int lines);
    // The first line shown, and how many lines the text takes at the bar's width (for the scroll bar; for tests).
    int firstLine() const { return m_firstLine; }
    int lineCount() const;
    QScrollBar *scrollBar() const { return m_scroll; }
    QWidget *grip() const;
    int lineOfPosition(int position) const;

Q_SIGNALS:
    void expandedChanged(bool on);
    void linesChanged(int lines);

protected:
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void inputMethodEvent(QInputMethodEvent *e) override;
    void changeEvent(QEvent *e) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

private:
    void layoutText();
    void applyHeight();
    int positionAt(const QPoint &p) const;
    void scrollToCaret();
    void updateScroll();
    int lineHeight() const;
    QRect textRect() const;
    int moveByLines(int delta) const;
    QString m_laidText;
    QTextLayout m_layout;
    QVector<QTextLayout::FormatRange> m_colors;   // the reference colors, as the window gave them
    bool m_expanded = false;
    int m_lines = kDefaultLines;
    int m_firstLine = 0;
    int m_anchor = -1;
    bool m_dirtyLayout = true;
    QScrollBar *m_scroll = nullptr;
    FormulaBarGrip *m_grip = nullptr;
    int m_oneLine = 0;
};

} // namespace jp

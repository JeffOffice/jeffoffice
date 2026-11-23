#pragma once
// Outline view (View > Outline): the document's paragraphs as a tree, a heading with the headings and text under it, to promote, demote, and move as a whole, and to show down to a level.

#include <QSet>
#include <QTreeWidget>

namespace jp::write {
class WriteDoc;
class StyleArea;
struct StyleRow;
}

namespace jp {

class WriteOutlineView : public QTreeWidget {
    Q_OBJECT
public:
    explicit WriteOutlineView(QWidget *parent = nullptr);
    // The document shown (not owned; null for none). The rows are made again with rebuild().
    void setDocument(write::WriteDoc *doc);
    // Makes the rows again from the text, keeping what was collapsed (a heading by its words and level) and what was selected (by place).
    void rebuild();
    // Headings down to `level` (1 to 9) are shown, with no body text; 0 shows everything.
    void setShowLevel(int level);
    int showLevel() const { return m_level; }
    // Each heading in a size and weight of its level (otherwise every row is in the same plain type).
    void setShowFormatting(bool on);
    bool showFormatting() const { return m_formatting; }
    // Body text as its first line only (otherwise all of it, wrapped).
    void setFirstLineOnly(bool on);
    bool firstLineOnly() const { return m_firstLine; }
    // The rows selected: the place in the text of the first and of the last, or -1 and -1 for none.
    QPair<int, int> selectedRange() const;
    // Selects the row of the paragraph that holds a position in the text, and shows it.
    void selectParagraphAt(int position);
    // The place in the text of the paragraph of the row; its outline level (9 for body text).
    static int positionOf(const QTreeWidgetItem *item);
    static int levelOf(const QTreeWidgetItem *item);
    // Expands or collapses the selected rows' headings (or all of them where none is selected).
    void expandSelected(bool expand);
    // The style area (a strip at the left that names the style of each paragraph beside its row): its width in pixels, 0 for none (the strip is not made until a width is set).
    void setStyleAreaWidth(int width);
    int styleAreaWidth() const { return m_styleWidth; }
    write::StyleArea *styleArea() const { return m_styleArea; }
    // The rows on show that have a paragraph, with their style names.
    QVector<write::StyleRow> styleRows() const;
    // Whether the headings are collapsed: for tests, the words of each row that is shown.
    QStringList shownRows() const;

Q_SIGNALS:
    void styleAreaWidthDragged(int width);
    // A double click (or Enter) on a style's name: the paragraph is where it is, and its style's box is wanted.
    void styleNameActivated(int position);
    // Enter on a row (not in its box): a new paragraph after it. Tab and Shift+Tab: its level one lower or higher (demote and promote).
    void newParagraphWanted();
    void levelKeyPressed(int by);
    // Words typed into a row: the place in the text of its paragraph and the new words.
    void textTyped(int position, const QString &text);

protected:
    bool event(QEvent *e) override;
    void updateGeometries() override;   // (the tree sets the viewport's margins itself; the strip's room is put back)
    void keyPressEvent(QKeyEvent *e) override;

private:
    void applyLevel();
    void applyLook(QTreeWidgetItem *item);
    write::WriteDoc *m_doc = nullptr;
    write::StyleArea *m_styleArea = nullptr;
    int m_styleWidth = 0, m_styleMargin = 0;
    bool m_marginBusy = false;
    int m_level = 0;
    bool m_formatting = true, m_firstLine = false;
    bool m_building = false;
    QSet<QString> m_collapsed;   // (level and words of a collapsed heading)
};

} // namespace jp

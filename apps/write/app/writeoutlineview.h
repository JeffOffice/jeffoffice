#pragma once
// Outline view (View > Outline): the document's paragraphs as a tree, a heading with the headings and text under it, to promote, demote, and move as a whole, and to show down to a level.

#include <QSet>
#include <QTreeWidget>

namespace jp::write {
class WriteDoc;
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
    // Whether the headings are collapsed: for tests, the words of each row that is shown.
    QStringList shownRows() const;

Q_SIGNALS:
    // Words typed into a row: the place in the text of its paragraph and the new words.
    void textTyped(int position, const QString &text);

private:
    void applyLevel();
    void applyLook(QTreeWidgetItem *item);
    write::WriteDoc *m_doc = nullptr;
    int m_level = 0;
    bool m_formatting = true, m_firstLine = false;
    bool m_building = false;
    QSet<QString> m_collapsed;   // (level and words of a collapsed heading)
};

} // namespace jp

#pragma once
// The drop-down of a filter's button: Sort A–Z and Z–A (and by color), Clear Filter, Filter by Color, the number, text, or date filters, and the list of the values in the column with a
// search box and (Select All). It shows at once and the list is filled when the values are listed (the window lists them a slice at a time). It decides nothing: what was chosen is told by
// signals, and the window changes the filter.

#include "core/filterlist.h"
#include "sheet/sheetmodel.h"

#include <QMenu>

#include <functional>

class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

namespace jp {

// What the number, text, and date filters of the menu ask the window to ask the person for.
enum class FilterAsk {
    Equals, NotEquals, Greater, GreaterEqual, Less, LessEqual, Between, Top10, AboveAverage, BelowAverage, Custom,
    BeginsWith, NotBeginsWith, EndsWith, NotEndsWith, Contains, NotContains,
    Before, After
};

class FilterPopup : public QMenu {
    Q_OBJECT
public:
    struct Input {
        QString columnName;
        bool filtered = false;                                        // the column has a condition
        jp::office::sheet::FilterColumn current;                     // and this is it
        std::function<QColor(const jp::office::sheet::ColorSpec &)> toColor;   // how a color of the workbook looks
        int sortDirection = 0;                                        // 1 if the cells are sorted ascending by this column, -1 descending
    };
    explicit FilterPopup(const Input &input, QWidget *parent = nullptr);
    // The values of the column are listed: the list is filled in, and the menus that depend on it (colors, the kind of filters) are made.
    void setList(const jp::crunch::FilterList &list);
    // The listing is going on (a sentence in place of the list).
    void setLoading(bool on);
    QTreeWidget *tree() const { return m_tree; }
    QLineEdit *search() const { return m_search; }
    bool listed() const { return m_listed; }
    // The conditions of the list as it is checked now (None if every value is checked).
    jp::office::sheet::FilterColumn conditionOfList() const;

Q_SIGNALS:
    void sortRequested(bool descending);
    void sortByColorRequested(bool font, const jp::office::sheet::ColorSpec &color);   // an unset color: the cells with none
    void clearRequested();
    void conditionChosen(const jp::office::sheet::FilterColumn &condition);
    void askRequested(jp::FilterAsk what);

private:
    void buildMenus();
    void fillTree();
    void searched(const QString &text);
    void itemChanged(QTreeWidgetItem *item);
    void refreshParents(QTreeWidgetItem *from);
    bool leafChecked(QTreeWidgetItem *item) const;
    Input m_input;
    jp::crunch::FilterList m_list;
    bool m_listed = false;
    QLineEdit *m_search = nullptr;
    QTreeWidget *m_tree = nullptr;
    QLabel *m_note = nullptr;
    QPushButton *m_ok = nullptr, *m_cancel = nullptr;
    QMenu *m_sortColors = nullptr, *m_filterColors = nullptr, *m_kindMenu = nullptr;
    QAction *m_clear = nullptr;
    QTreeWidgetItem *m_all = nullptr, *m_blanks = nullptr;
    bool m_busy = false;
};

} // namespace jp

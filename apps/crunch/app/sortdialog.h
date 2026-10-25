#pragma once
// The Sort dialog: the levels of a sort (the column or row to sort by, on its values, its cell color, or its font color; in which order, or with which color first), whether the first row is
// a header, whether case matters, and whether to sort from left to right. It reads the names of the columns, and the colors in a column, through the functions it is given.

#include "core/sortspec.h"

#include <QColor>
#include <QDialog>
#include <QPair>

#include <functional>

class QCheckBox;
class QComboBox;
class QPushButton;
class QTableWidget;

namespace jp {

class SortDialog : public QDialog {
    Q_OBJECT
public:
    struct Colors {
        QVector<QPair<jp::office::sheet::ColorSpec, QColor>> colors;   // the colors in the column, and how they look
        bool none = false;                                              // some cells have no color
    };
    struct Provider {
        // The names of the columns (rows, for a sort from left to right) of the cells, with or without the first row taken for names.
        std::function<QStringList(bool header, bool leftToRight)> names;
        // The colors in a column (row) of the cells.
        std::function<Colors(int key, bool font, bool leftToRight)> colors;
    };
    SortDialog(const Provider &provider, const jp::crunch::SortSpec &initial, QWidget *parent = nullptr);
    jp::crunch::SortSpec spec() const;

private:
    void addLevel(const jp::crunch::SortLevel &level);
    void fillRow(int row, const jp::crunch::SortLevel &level);
    void refillNames();
    void fillOrder(int row, const jp::crunch::SortLevel *level);
    jp::crunch::SortLevel levelOf(int row) const;
    void renumber();
    Provider m_provider;
    QTableWidget *m_levels = nullptr;
    QCheckBox *m_header = nullptr, *m_case = nullptr, *m_leftToRight = nullptr;
    QPushButton *m_add = nullptr, *m_delete = nullptr, *m_copy = nullptr, *m_up = nullptr, *m_down = nullptr;
    QVector<QStringList> m_lists;    // the lists the order boxes offer, by their place (after "A to Z" and "Z to A")
};

} // namespace jp

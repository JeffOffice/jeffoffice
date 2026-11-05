#pragma once
// The question of Subtotal: at each change in which column, with which function, and which columns to total.

#include "core/subtotal.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QListWidget;

namespace jp {

class SubtotalDialog : public QDialog {
    Q_OBJECT
public:
    // `names` are the headings of the columns of the list, in order (the first column of the list is 0).
    explicit SubtotalDialog(const QStringList &names, QWidget *parent = nullptr);
    // Places among the columns of the list.
    int changeColumn() const;
    int function() const;
    QList<int> totalColumns() const;
    bool replace() const;
    // The dialog ended with Remove All.
    bool removeAll() const { return m_removeAll; }

private:
    QComboBox *m_at, *m_function;
    QListWidget *m_columns;
    QCheckBox *m_replace;
    bool m_removeAll = false;
};

} // namespace jp

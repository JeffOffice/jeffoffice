#pragma once
// The question of Advanced Filter: the list, the conditions, and whether to hide rows in place or copy the rows that meet them somewhere else.

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QRadioButton;

namespace jp {

class AdvancedFilterDialog : public QDialog {
    Q_OBJECT
public:
    // `list` is the list around the cell with the cursor, as a range name ("A1:C20").
    explicit AdvancedFilterDialog(const QString &list, QWidget *parent = nullptr);
    QString list() const;
    QString criteria() const;
    // The cell the copy begins at; empty to filter in place.
    QString copyTo() const;
    bool unique() const;

private:
    QRadioButton *m_inPlace, *m_copy;
    QLineEdit *m_list, *m_criteria, *m_copyTo;
    QCheckBox *m_unique;
};

} // namespace jp

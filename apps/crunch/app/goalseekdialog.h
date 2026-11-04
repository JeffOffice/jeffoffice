#pragma once
// The question of Goal Seek: which formula cell, what it should come to, and which cell may be changed to get there.

#include <QDialog>

class QLineEdit;

namespace jp {

class GoalSeekDialog : public QDialog {
    Q_OBJECT
public:
    // `setCell` is the cell with the cursor, as a cell name ("B3").
    explicit GoalSeekDialog(const QString &setCell, QWidget *parent = nullptr);
    QString setCell() const;
    QString toValue() const;
    QString changeCell() const;

private:
    QLineEdit *m_set, *m_value, *m_change;
};

} // namespace jp

#pragma once
// The Watch Window: a small window that stays in view and lists cells the person watches, with their sheet, name, value, and formula.

#include <QDialog>
#include <QStringList>
#include <QVector>

class QTableWidget;

namespace jp {

class WatchWindow : public QDialog {
    Q_OBJECT
public:
    explicit WatchWindow(QWidget *parent = nullptr);
    // The rows shown: sheet, cell, value, formula.
    void setRows(const QVector<QStringList> &rows);
    int rowCount() const;
    QTableWidget *table() const { return m_table; }

Q_SIGNALS:
    void addRequested();
    void deleteRequested(const QList<int> &rows);   // the places of the rows chosen
    void goToRequested(int row);                    // a row was double-clicked

private:
    QTableWidget *m_table;
};

} // namespace jp

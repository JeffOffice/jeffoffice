#pragma once
// Data > Queries & Connections: every query of the workbook (a table made by Get Data) with its name, the file it reads, when it last ran, the rows it gave, and whether it can be run, and the commands
// for it. The pane only lists and asks: the window reads the files and changes the workbook.

#include "core/editsession.h"

#include <QDialog>
#include <QVector>

class QLabel;
class QPushButton;
class QTableWidget;

namespace jp {

class ConnectionsPane : public QDialog {
    Q_OBJECT
public:
    // `canChange` is false for a workbook that is read-only (the commands are then off; the list is still shown).
    ConnectionsPane(crunch::EditSession *session, bool canChange, QWidget *parent = nullptr);
    QTableWidget *table() const { return m_table; }
    QPushButton *refreshButton() const { return m_refresh; }
    QPushButton *refreshAllButton() const { return m_refreshAll; }
    int queryCount() const { return int(m_tables.size()); }
    // The table (its place in the workbook's list) the row is for.
    int tableOfRow(int row) const { return m_tables.value(row, -1); }
    // Reads the workbook again: after a refresh, or an Undo.
    void reload();
    QString note() const;
    void setNote(const QString &text);

Q_SIGNALS:
    void refreshRequested(const QVector<int> &tables, bool all);

private:
    crunch::EditSession *m_session;
    bool m_canChange;
    QTableWidget *m_table = nullptr;
    QPushButton *m_refresh = nullptr, *m_refreshAll = nullptr;
    QLabel *m_note = nullptr;
    QVector<int> m_tables;
    void updateButtons();
};

}   // namespace jp

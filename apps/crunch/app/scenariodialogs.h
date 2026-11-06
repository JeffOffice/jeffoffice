#pragma once
// The Scenario Manager's two dialogs: the list of a sheet's scenarios with its buttons (Add, Delete, Edit, Show, Summary), and the one that asks for a scenario's name, changing cells, note,
// and values. They ask; the window does (CrunchWindow::addScenario and the others).

#include "sheet/sheetmodel.h"

#include <QDialog>

#include <functional>

class QCheckBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTableWidget;

namespace jp {

class ScenarioManagerDialog : public QDialog {
    Q_OBJECT
public:
    explicit ScenarioManagerDialog(QWidget *parent = nullptr);
    // The list as it is now (the one chosen stays chosen, by its place).
    void setScenarios(const jp::office::sheet::ScenarioList &list);
    int currentIndex() const;

Q_SIGNALS:
    void addRequested();
    void editRequested(int index);
    void deleteRequested(int index);
    void showRequested(int index);
    void summaryRequested();

private:
    void refreshButtons();
    QListWidget *m_list = nullptr;
    QLabel *m_about = nullptr;
    QPushButton *m_edit = nullptr, *m_delete = nullptr, *m_show = nullptr, *m_summary = nullptr;
    jp::office::sheet::ScenarioList m_scenarios;
};

class ScenarioEditDialog : public QDialog {
    Q_OBJECT
public:
    // `adding`: the changing cells can be written (an edit keeps the scenario's own). `currentText` gives what a cell holds now, for the value a new scenario starts with.
    ScenarioEditDialog(bool adding, std::function<QString(const QString &)> currentText, QWidget *parent = nullptr);
    void setName(const QString &name);
    void setComment(const QString &comment);
    // Prevent changes: the scenario can't be edited or deleted while the sheet is protected.
    void setLocked(bool locked);
    bool locked() const;
    // The cells (as text) and the values the dialog shows for them.
    void setCells(const QString &text, const QStringList &values);
    QString name() const;
    QString cellsText() const;
    QString comment() const;
    QStringList values() const;

private:
    void cellsChanged();
    void refreshOk();
    bool m_adding;
    std::function<QString(const QString &)> m_currentText;
    QLineEdit *m_name = nullptr, *m_cells = nullptr, *m_comment = nullptr;
    QCheckBox *m_locked = nullptr;
    QLabel *m_problem = nullptr;
    QTableWidget *m_table = nullptr;
    QPushButton *m_ok = nullptr;
    bool m_filling = false;
};

} // namespace jp

#pragma once
// The Find and Replace dialog: what to look for (with the marks ? and * for any one character and any run, and ~ to mean one of them itself), what to put in its place, and where to look
// (this sheet or the workbook, by rows or by columns, in formulas, values, or notes, matching case, matching whole cells). It stays open beside the sheet. It does no searching itself:
// it says what was asked (the window searches, a few thousand cells at a time) and shows the cells a Find All found, as a list to click.

#include "core/findrun.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTabWidget;
class QTreeWidget;

namespace jp {

class FindDialog : public QDialog {
    Q_OBJECT
public:
    explicit FindDialog(QWidget *parent = nullptr);

    enum class Tab { Find, Replace };
    void setTab(Tab tab);
    Tab tab() const;
    void setFindText(const QString &text);
    QString findText() const;
    QString replacement() const;
    // What the controls say, as options for a search (`sheet` and `within` are the window's to fill).
    crunch::FindOptions options() const;
    void setOptions(const crunch::FindOptions &options);
    // A sentence at the bottom ("3 cells found", "Replaced 4 places"), and the cells a Find All found (the names of the sheets are by their place in the workbook).
    void setStatus(const QString &text);
    void showResults(const std::vector<crunch::FoundCell> &hits, const QStringList &sheetNames, bool truncated);
    void clearResults();
    // Work is going on (a search is being made): the buttons that start a search are off, and Cancel stands where Close was.
    void setBusy(bool busy);
    QTreeWidget *results() const { return m_results; }

Q_SIGNALS:
    void findNextRequested();
    void findAllRequested();
    void replaceRequested();
    void replaceAllRequested();
    void cancelRequested();
    void resultChosen(int sheet, quint32 row, quint32 col);

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    void updateTab();
    QTabWidget *m_tabs = nullptr;
    QLineEdit *m_find = nullptr, *m_replace = nullptr;
    QComboBox *m_within = nullptr, *m_search = nullptr, *m_lookIn = nullptr;
    QCheckBox *m_case = nullptr, *m_whole = nullptr;
    QLabel *m_status = nullptr, *m_replaceLabel = nullptr;
    QPushButton *m_findAll = nullptr, *m_findNext = nullptr, *m_replaceOne = nullptr, *m_replaceAll = nullptr, *m_close = nullptr;
    QTreeWidget *m_results = nullptr;
    bool m_busy = false;
};

} // namespace jp

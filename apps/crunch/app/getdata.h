#pragma once
// Data > Get Data > From File: the rows of a JSON or XML file, or of a sheet, table, or named range of another workbook, put on the sheet as a table that remembers its file. The window asks what to
// load (the file, the list in it, the choice of types) and shows the first rows before anything is changed; the file is read on a thread of its own behind a box with Cancel.

#include "sheet/datasource.h"
#include "sheet/datasteps.h"

#include <QDialog>
#include <QString>

#include <atomic>
#include <functional>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QTableWidget;

namespace jp {

// What the person chose: the source and the cell where the table goes (as written, "B2").
struct GetDataRequest {
    jp::office::sheet::DataConnection conn;
    QString place;
};

// What reading a source gave: the lists it offers, the rows of the list chosen (the first `previewRows`, or all of them), and the kind of each column.
struct GetDataFetch {
    bool ok = false;
    QString why;
    QStringList lists;
    jp::office::sheet::DataRows rows;
    QVector<char> kinds;
    int failedStep = -1;   // the step (from 0) that could not run, when one did not
};

// Reads the source of `conn`; the list is the first one the file offers when conn.list is empty. Safe on any thread: it asks `cancelled` as it goes.
GetDataFetch fetchGetData(const jp::office::sheet::DataConnection &conn, int previewRows, const std::function<bool()> &cancelled);

// The text with every character that could fake what is read shown as visible code, never raw: line breaks, other control and formatting characters (a direction override among them), and characters with no
// meaning of their own. A character is written as <U+202E>. Used for a path and a name in the box that asks before a file is read.
QString getDataVisibleText(const QString &text);

// The kind of source a path is, by its ending and then by its first characters: "json", "xml", "workbook", or empty.
QString guessGetDataKind(const QString &path);

// Runs `work` on a thread of its own and returns when it is done (the window's events go on). The box with Cancel shows after a moment; Cancel sets `cancel`. False if it was cancelled.
bool runGetDataWork(QWidget *parent, const QString &text, std::atomic<bool> *cancel, const std::function<void()> &work);

class GetDataDialog : public QDialog {
    Q_OBJECT
public:
    GetDataDialog(const QString &place, QWidget *parent);
    GetDataRequest request() const;
    // For tests and for the window: the same as typing and picking.
    void setPath(const QString &path);
    void chooseList(const QString &list);
    QString message() const;
    int previewRows() const;
    int previewColumns() const;
    QString previewCell(int row, int column) const;
    QStringList listNames() const;
    QPushButton *loadButton() const { return m_load; }

private:
    void refreshPreview();
    QLineEdit *m_path = nullptr, *m_place = nullptr;
    QComboBox *m_list = nullptr, *m_mode = nullptr;
    QCheckBox *m_asText = nullptr;
    QTableWidget *m_preview = nullptr;
    QLabel *m_message = nullptr;
    QPushButton *m_load = nullptr;
    QString m_kind;
    bool m_loadingLists = false;
};

}   // namespace jp

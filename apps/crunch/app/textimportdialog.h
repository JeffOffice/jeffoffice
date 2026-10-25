#pragma once
// The dialog that opens a text file of values: how its fields are separated and quoted, the code page, the row to begin at, and how each column is read (as it is typed, as text,
// as a date in an order, or left out), with a preview of the first rows that follows every change. The preview reads at most the first megabyte of the file and shows 200 rows and 100
// columns, each field cut to 120 characters, so a huge file or a very long line opens it at once.

#include "sheet/csvimport.h"

#include <QByteArray>
#include <QDialog>
#include <QLocale>

#include <memory>
#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QTableWidget;

namespace jp {

struct TextImportRequest {
    QString path;
    QByteArray head;                              // the first bytes of the file (at most jp::office::sheet::kCsvPreviewBytes)
    qint64 size = 0;                              // the file's size
    QLocale locale;
    jp::office::sheet::CsvImport choices;         // in: what the file is found to be; out: what the person chose
};

class TextImportDialog : public QDialog {
    Q_OBJECT
public:
    explicit TextImportDialog(const TextImportRequest &request, QWidget *parent = nullptr);
    // What the dialog says now.
    jp::office::sheet::CsvImport choices() const;
    // The preview's rows and columns now (for tests).
    int previewRows() const;
    int previewColumns() const;

private:
    void refresh();
    void fillKinds();
    TextImportRequest m_request;
    QComboBox *m_encoding = nullptr, *m_quote = nullptr;
    QCheckBox *m_comma = nullptr, *m_semicolon = nullptr, *m_tab = nullptr, *m_space = nullptr, *m_pipe = nullptr, *m_merge = nullptr;
    QLineEdit *m_other = nullptr;
    QSpinBox *m_start = nullptr;
    QTableWidget *m_table = nullptr;
    QLabel *m_note = nullptr;
    std::vector<QComboBox *> m_kinds;
    QVector<jp::office::sheet::CsvColumn> m_columns;   // what the person chose for each column so far
    bool m_filling = false;
};

} // namespace jp

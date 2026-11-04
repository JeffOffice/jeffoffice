#pragma once
// The questions of the Data tools: which columns to compare for Remove Duplicates, and how to cut a column of text for Text to Columns.

#include "core/editsession.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QRadioButton;

namespace jp {

class RemoveDuplicatesDialog : public QDialog {
    Q_OBJECT
public:
    // `names` are the names of the columns of the cells selected (their headers, or their letters); `hasHeader` says the first row is a header.
    RemoveDuplicatesDialog(const QStringList &names, bool hasHeader, QWidget *parent = nullptr);
    // The places of the columns that are checked, and whether the first row is a header.
    std::vector<int> columns() const;
    bool hasHeader() const;
    // For the tests and the keyboard: checks all of the columns, or none.
    void checkAll(bool on);

private:
    QListWidget *m_list;
    QCheckBox *m_header;
};

class TextToColumnsDialog : public QDialog {
    Q_OBJECT
public:
    // `lines` are the text of the cells (shown cut in a preview); `destination` is where the pieces go, as a cell name ("B2").
    TextToColumnsDialog(const QStringList &lines, const QString &destination, QWidget *parent = nullptr);
    crunch::EditSession::SplitSpec spec() const;
    QString destination() const;

private:
    void updatePreview();
    QStringList m_lines;
    QRadioButton *m_delimited, *m_fixed;
    QCheckBox *m_tab, *m_semicolon, *m_comma, *m_space, *m_other, *m_consecutive;
    QLineEdit *m_otherText, *m_breaks, *m_destination;
    QComboBox *m_qualifier, *m_format;
    QLabel *m_preview;
};

} // namespace jp

#pragma once
// The question before a sheet is saved as a file of values: what the file will not have (the text the window makes), and how it is written: the separator between the cells, the
// encoding, how the lines end, and whether the cells are written as they are shown or as they are stored. The answers are the window's own three (save the values, save the
// workbook as .xlsx instead, or give it up).

#include "core/csvexport.h"

#include <QDialog>

class QComboBox;
class QLineEdit;

namespace jp {

class CsvSaveDialog : public QDialog {
    Q_OBJECT
public:
    enum class Answer { Cancel, SaveValues, SaveXlsx };
    CsvSaveDialog(const QString &explanation, const crunch::CsvWrite &start, QWidget *parent = nullptr);
    // The choices now. `sameAsFile` stays as it was given when they are the ones it was given with, and is off when any of them was changed.
    crunch::CsvWrite options() const;
    Answer answer() const { return m_answer; }

private:
    void refreshOther();

    crunch::CsvWrite m_start;
    QComboBox *m_separator, *m_encoding, *m_lineEnd, *m_values;
    QLineEdit *m_other;
    Answer m_answer = Answer::Cancel;
};

} // namespace jp

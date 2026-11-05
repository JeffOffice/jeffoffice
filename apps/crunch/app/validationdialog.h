#pragma once
// The Data Validation dialog: what may be entered in the selected cells (Settings), the message shown when a cell is selected (Input Message), and the alert shown for what is not allowed (Error Alert).

#include "sheet/sheetmodel.h"

#include <QDialog>
#include <optional>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QTabWidget;

namespace jp {

class ValidationDialog : public QDialog {
    Q_OBJECT
public:
    // `rule` is the rule the cell with the cursor has, if any (the boxes start from it).
    ValidationDialog(const std::optional<jp::office::sheet::Validation> &rule, QWidget *parent = nullptr);
    // The rule the boxes describe, without its cells; empty if the limits typed cannot be read (`problem` says what is the matter).
    std::optional<jp::office::sheet::Validation> rule(QString *problem) const;
    // The dialog ended with Clear All.
    bool clearAll() const { return m_clearAll; }

private:
    void refreshBoxes();
    QComboBox *m_allow, *m_data, *m_style;
    QLineEdit *m_first, *m_second, *m_promptTitle, *m_errorTitle;
    QLabel *m_firstLabel, *m_secondLabel;
    QCheckBox *m_blank, *m_dropDown, *m_showInput, *m_showError;
    QPlainTextEdit *m_prompt, *m_error;
    bool m_clearAll = false;
};

} // namespace jp

#pragma once
// The question of Text to Columns: how to cut a column of text.

#include "core/editsession.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QRadioButton;

namespace jp {

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

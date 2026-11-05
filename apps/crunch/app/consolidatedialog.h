#pragma once
// The question of Consolidate: the ranges to combine, the function, where the labels are, whether to link, and where the result goes.

#include <QDialog>
#include <QStringList>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QListWidget;

namespace jp {

class ConsolidateDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConsolidateDialog(const QString &destination, QWidget *parent = nullptr);
    QStringList references() const;
    int function() const;
    bool topLabels() const;
    bool leftLabels() const;
    bool links() const;
    QString destination() const;

private:
    QComboBox *m_function;
    QLineEdit *m_reference, *m_destination;
    QListWidget *m_list;
    QCheckBox *m_top, *m_left, *m_links;
};

} // namespace jp

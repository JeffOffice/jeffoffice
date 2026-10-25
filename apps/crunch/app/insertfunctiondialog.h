#pragma once
// The Pick a Function dialog: the functions of the formula language, by kind or found by a search of their names and what they do, with the arguments of the one picked and a sentence on
// what it does. It only picks; the window puts the function in the formula.

#include <QDialog>

class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;

namespace jp {

class InsertFunctionDialog : public QDialog {
    Q_OBJECT
public:
    explicit InsertFunctionDialog(QWidget *parent = nullptr, const QString &search = QString());
    // The name of the function picked, or empty.
    QString chosen() const;
    QLineEdit *searchBox() const { return m_search; }
    QComboBox *categoryBox() const { return m_category; }
    QListWidget *list() const { return m_list; }
    QListWidget *argumentList() const { return m_arguments; }
    QLabel *signatureLabel() const { return m_signature; }
    QLabel *summaryLabel() const { return m_summary; }

private:
    void refill();
    void showPicked();
    QLineEdit *m_search = nullptr;
    QComboBox *m_category = nullptr;
    QListWidget *m_list = nullptr, *m_arguments = nullptr;
    QLabel *m_signature = nullptr, *m_summary = nullptr, *m_count = nullptr;
    QPushButton *m_ok = nullptr;
};

} // namespace jp

#pragma once
// The Error Checking dialog: one cell that may be wrong at a time, with what is the matter, and Previous, Next, and Ignore Error.

#include <QDialog>
#include <QStringList>

class QLabel;
class QPushButton;

namespace jp {

class ErrorCheckDialog : public QDialog {
    Q_OBJECT
public:
    // `cells` are the names of the cells ("B3"), `messages` what is the matter with each; `first` is the one to show first.
    ErrorCheckDialog(const QStringList &cells, const QStringList &messages, int first, QWidget *parent = nullptr);
    int current() const { return m_current; }
    int count() const { return int(m_cells.size()); }

Q_SIGNALS:
    void showing(int index);   // a cell is shown (the window goes to it)
    void ignored(int index);   // the person chose to ignore this one (it is no longer in the list)

private:
    void showProblem(int index);
    QStringList m_cells, m_messages;
    int m_current = 0;
    QLabel *m_title, *m_text;
    QPushButton *m_previous, *m_next, *m_ignore;
};

} // namespace jp

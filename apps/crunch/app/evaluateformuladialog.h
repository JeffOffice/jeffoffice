#pragma once
// The Evaluate Formula dialog: the formula of a cell with the next part underlined, and Evaluate to put that part's value in its place.

#include "core/formulasteps.h"

#include <QDialog>
#include <memory>

class QLabel;
class QPushButton;
class QTextEdit;

namespace jp {

class EvaluateFormulaDialog : public QDialog {
    Q_OBJECT
public:
    EvaluateFormulaDialog(const QString &cell, const QString &expression, crunch::FormulaSteps::Eval eval, QWidget *parent = nullptr);
    // What the box shows now.
    QString shownText() const;
    crunch::FormulaSteps &steps() { return *m_steps; }

private:
    void refresh();
    std::unique_ptr<crunch::FormulaSteps> m_steps;
    QTextEdit *m_box;
    QLabel *m_note;
    QPushButton *m_evaluate;
    int m_taken = 0;
};

} // namespace jp

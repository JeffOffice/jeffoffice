#pragma once
// Help while a formula is typed, in the cell or in the formula bar: after "=" and some letters a short list of the functions and names that begin so (Tab takes the one that is marked; Enter
// takes it once the arrows were used; Escape closes the list), and, with the cursor inside the brackets of a function, its arguments under the cell with the one being typed in bold.

#include "core/formulaspot.h"

#include <QObject>
#include <QPointer>

#include <functional>

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

namespace jp {

class FormulaHelper : public QObject {
    Q_OBJECT
public:
    explicit FormulaHelper(QObject *parent = nullptr);
    ~FormulaHelper() override;
    // The line is helped (any number of them: the cell and the formula bar).
    void attach(QLineEdit *line);
    // Where the functions and names come from: the index to look in, and a sentence about each one found (what a function does, what a name stands for); asked when the list is shown.
    void setSources(const std::function<const crunch::CompletionIndex &()> &index, const std::function<QString(const crunch::Completion &)> &detail);
    // The separator between arguments the person types (as the locale writes it).
    void setSeparator(const std::function<QChar()> &separator) { m_separator = separator; }
    void close();
    bool isListOpen() const;
    bool isHintOpen() const;
    QListWidget *list() const { return m_list; }
    QLabel *hint() const { return m_hint; }
    QLabel *detail() const { return m_detail; }
    // Takes the marked entry of the list into the line; false if the list is not open.
    bool acceptMarked();
    // What the list shows now (the names, in order), for tests.
    QStringList offered() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void refresh();
    void showList(QLineEdit *line, const std::vector<crunch::Completion> &found, const crunch::FormulaSpot &spot);
    void showHint(QLineEdit *line, const crunch::FormulaSpot &spot);
    void place(QLineEdit *line);
    void markChanged();
    QPointer<QLineEdit> m_line;
    QWidget *m_box = nullptr;
    QListWidget *m_list = nullptr;
    QLabel *m_detail = nullptr, *m_hint = nullptr;
    std::function<const crunch::CompletionIndex &()> m_index;
    std::function<QString(const crunch::Completion &)> m_detailOf;
    std::function<QChar()> m_separator;
    std::vector<crunch::Completion> m_found;
    crunch::FormulaSpot m_spot;
    bool m_arrowed = false;
    bool m_working = false;
};

} // namespace jp

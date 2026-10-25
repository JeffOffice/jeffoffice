#pragma once
// The two questions a filter asks that are not a list: a condition on a column (up to two comparisons joined by and or or: equals, is greater than, begins with, contains, ...; for dates, on
// or before a day, and so on), and the top or bottom of the values.

#include "app/filterpopup.h"
#include "calc/dates.h"

#include <QDialog>

class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLineEdit;
class QRadioButton;
class QStackedWidget;

namespace jp {

class CustomFilterDialog : public QDialog {
    Q_OBJECT
public:
    // `column` is the column's name; `dates` asks for days to be typed (and turned to the numbers the cells hold, by the date system); `ask` is the comparison the first row begins with; and
    // `current` the condition the column has, if it has a custom one.
    CustomFilterDialog(const QString &column, bool dates, jp::calc::DateSystem system, FilterAsk ask, const jp::office::sheet::FilterColumn *current, QWidget *parent = nullptr);
    jp::office::sheet::FilterColumn result() const;
    // For tests: the controls of row 0 and 1.
    QComboBox *operatorBox(int row) const { return m_op[row]; }
    QLineEdit *valueEdit(int row) const { return m_text[row]; }
    QDateEdit *dateEdit(int row) const { return m_date[row]; }
    QRadioButton *andButton() const { return m_and; }
    QRadioButton *orButton() const { return m_or; }
    // The pattern a comparison with text is made of (begins with "a*b" is the text a~*b followed by a star).
    static QString escaped(const QString &text);

private:
    enum UiOp { Equals, NotEquals, Greater, GreaterEqual, Less, LessEqual, BeginsWith, NotBeginsWith, EndsWith, NotEndsWith, Contains, NotContains, Nothing };
    void setRow(int row, UiOp op, const QString &text);
    bool m_dates;
    jp::calc::DateSystem m_system;
    QComboBox *m_op[2] = {nullptr, nullptr};
    QLineEdit *m_text[2] = {nullptr, nullptr};
    QDateEdit *m_date[2] = {nullptr, nullptr};
    QRadioButton *m_and = nullptr, *m_or = nullptr;
};

class TopFilterDialog : public QDialog {
    Q_OBJECT
public:
    explicit TopFilterDialog(const QString &column, const jp::office::sheet::FilterColumn *current, QWidget *parent = nullptr);
    jp::office::sheet::FilterColumn result() const;
    QComboBox *whichBox() const { return m_which; }
    QDoubleSpinBox *countBox() const { return m_count; }
    QComboBox *unitBox() const { return m_unit; }

private:
    QComboBox *m_which = nullptr, *m_unit = nullptr;
    QDoubleSpinBox *m_count = nullptr;
};

} // namespace jp

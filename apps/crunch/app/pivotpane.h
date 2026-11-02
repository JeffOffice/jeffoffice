#pragma once
// The Pivot Table Fields pane and the Value Field Settings dialog. The pane lists the fields of the data a table is made from, with a check for each, and shows where each checked field is: the
// four boxes Filters, Columns, Rows, and Values. A field is checked in (a field of numbers goes to the values, any other to the rows), moved from a box to another, put in another place of its box,
// or taken out; each is a request for the table the pane shows with that change made, which the window gives to the session as one step.

#include "sheet/pivot.h"

#include <QDialog>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QToolButton;

namespace jp {

class PivotFieldsPane : public QWidget {
    Q_OBJECT
public:
    explicit PivotFieldsPane(QWidget *parent = nullptr);
    // Shows a table: its name, the names of its fields (the cache's, or the table's own), whether each field holds numbers, and the table's layout.
    void showTable(const QString &name, const jp::office::sheet::PivotTable &table, const QStringList &fieldNames, const QVector<bool> &numeric, const QVector<QString> &dataNames = QVector<QString>());
    void showNothing();
    // What a click on the pane does, for the tests and for a keyboard: the same as the controls.
    void checkField(int field, bool on);
    void moveSelected(int box);          // 0 Filters, 1 Columns, 2 Rows, 3 Values
    void removeSelected();
    void shiftSelected(int by);
    bool select(int box, int row);
    QListWidget *fieldList() const { return m_fields; }
    QListWidget *box(int box) const { return m_boxes[box]; }

Q_SIGNALS:
    // The table with the change made, and the words for the step of Undo.
    void changeRequested(const jp::office::sheet::PivotTable &table, const QString &text);
    // The value at this place of the table's values was asked to be set (a double click, or the button).
    void valueSettingsRequested(int dataIndex);

private:
    void fillBoxes();
    int selectedBox() const;
    void changed(const jp::office::sheet::PivotTable &table, const QString &text);
    // The entry of a box: what it stands for (a field, or the values), for the table as it is shown.
    struct Entry {
        int box = -1;
        int index = -1;   // the place in the box's list
        int field = -1;   // the field, or -2 for the values
        int data = -1;    // for the values box: the place among the data fields
    };
    Entry entryOf(QListWidgetItem *item) const;
    jp::office::sheet::PivotDataField newValue(const jp::office::sheet::PivotTable &t, int field) const;
    jp::office::sheet::PivotTable m_table;
    QStringList m_names;
    QVector<bool> m_numeric;
    QLabel *m_title = nullptr;
    QListWidget *m_fields = nullptr;
    QListWidget *m_boxes[4] = {};
    QToolButton *m_moveTo = nullptr;
    QToolButton *m_up = nullptr, *m_down = nullptr, *m_remove = nullptr, *m_settings = nullptr;
    bool m_filling = false;
};

// Value Field Settings: the name the value shows, and how it is totaled. `functions` and their words: sum, count, countNums, average, max, min, product, stdDev, stdDevp, var, varp.
class ValueFieldDialog : public QDialog {
    Q_OBJECT
public:
    ValueFieldDialog(const QString &sourceName, const QString &name, const QString &function, QWidget *parent = nullptr);
    QString name() const;
    QString function() const;

private:
    QLineEdit *m_name = nullptr;
    QListWidget *m_functions = nullptr;
    QLabel *m_source = nullptr;
};

} // namespace jp

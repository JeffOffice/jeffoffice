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
class QPushButton;
class QListWidget;
class QListWidgetItem;
class QToolButton;

namespace jp {

class PivotFieldsPane : public QWidget {
    Q_OBJECT
public:
    explicit PivotFieldsPane(QWidget *parent = nullptr);
    // Shows a table: its name, the names of its fields (the cache's, or the table's own), whether each field holds numbers, and the table's layout.
    void showTable(const QString &name, const jp::office::sheet::PivotTable &table, const QStringList &fieldNames, const QVector<bool> &numeric, const void *identity = nullptr);   // (`identity`: what tells this version of the table from another)
    void showNothing();
    // What a click on the pane does, for the tests and for a keyboard: the same as the controls.
    void checkField(int field, bool on);
    void moveSelected(int box);          // 0 Filters, 1 Columns, 2 Rows, 3 Values
    void removeSelected();
    void shiftSelected(int by);
    bool select(int box, int row);
    QListWidget *fieldList() const { return m_fields; }
    QListWidget *box(int box) const { return m_boxes[box]; }
    // The field of the entry chosen in the Filters, Columns, or Rows box (-1 when none, or when the entry is the values).
    int selectedField() const;
    // The search box and Defer Layout Update, for the tests: the same as the controls.
    void search(const QString &text);
    void setDeferred(bool on);
    void applyDeferred();
    bool hasPending() const { return m_pending; }

Q_SIGNALS:
    // The table with the change made, and the words for the step of Undo.
    void changeRequested(const jp::office::sheet::PivotTable &table, const QString &text);
    // The value at this place of the table's values was asked to be set (a double click, or the button).
    void valueSettingsRequested(int dataIndex);
    void fieldSelectionChanged();   // a field was picked in a box, or none is picked now
    // The field at this place of the table's fields was asked to be set (a double click on a field of a box, or the button).
    void fieldSettingsRequested(int field);

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
    QToolButton *m_up = nullptr, *m_down = nullptr, *m_remove = nullptr, *m_settings = nullptr, *m_fieldSettings = nullptr;
    bool m_filling = false;
    QLineEdit *m_search = nullptr;
    QCheckBox *m_deferBox = nullptr;
    QToolButton *m_update = nullptr;
    bool m_pending = false;   // Defer Layout Update: changes are kept here until Update
    QString m_pendingText;
    QString m_shownName, m_pendingName;   // the table shown, and the one the waiting changes were made on
    quintptr m_shownStamp = 0, m_pendingStamp = 0;
};

// What Group Field asks: the parts of a date to group by (seconds, minutes, hours, days, months, quarters, years), or for numbers (no parts) where to start and end and the width of a group.
struct GroupChoice {
    bool isDate = false;
    QStringList levels;
    QString startText, endText, stepText;   // what the boxes hold at first (so that a bound left as it was stays automatic)
    double start = 0, end = 0, step = 1;
    bool autoStart = true, autoEnd = true;
};

class GroupDialog : public QDialog {
    Q_OBJECT
public:
    explicit GroupDialog(const GroupChoice &choice, QWidget *parent = nullptr);
    GroupChoice choice() const;

private:
    GroupChoice m_choice;
    QLineEdit *m_start = nullptr, *m_end = nullptr, *m_step = nullptr;
    QListWidget *m_levels = nullptr;
};

// What Calculated Field asks: a name and a formula over the table's fields; the fields that can be named; and the calculated fields there are (name and formula), to pick one to change or delete.
struct CalculatedFieldChoice {
    enum Action : quint8 { Add, Delete } action = Add;
    QString name, formula;
    QStringList fieldNames;
    QStringList existingNames, existingFormulas;
};

class CalculatedFieldDialog : public QDialog {
    Q_OBJECT
public:
    explicit CalculatedFieldDialog(const CalculatedFieldChoice &choice, QWidget *parent = nullptr);
    CalculatedFieldChoice choice() const;

private:
    void nameChanged();
    CalculatedFieldChoice m_choice;
    QComboBox *m_name = nullptr;
    QLineEdit *m_formula = nullptr;
    QListWidget *m_fields = nullptr;
    QPushButton *m_add = nullptr, *m_delete = nullptr;
};

// What Pivot Table Options sets: the name, how labels, errors, and empty cells show, the grand totals, what a refresh keeps, and whether the data is read again when the file opens.
struct PivotOptionsChoice {
    QString name;
    bool mergeLabels = false, showError = false, showMissing = true;
    QString errorText, missingText;
    bool rowGrandTotals = true, colGrandTotals = true;   // (the file's two: totals of the rows at the right, of the columns at the bottom)
    bool preserveFormatting = true, autofit = false, showHeaders = true, refreshOnOpen = false;
    bool generateGetPivotData = true;
    bool hasTable = false;   // (without a table only the last choice is offered)
};

class PivotOptionsDialog : public QDialog {
    Q_OBJECT
public:
    explicit PivotOptionsDialog(const PivotOptionsChoice &choice, QWidget *parent = nullptr);
    PivotOptionsChoice choice() const;

private:
    PivotOptionsChoice m_choice;
    QLineEdit *m_name = nullptr, *m_error = nullptr, *m_missing = nullptr;
    QCheckBox *m_merge = nullptr, *m_showError = nullptr, *m_showMissing = nullptr, *m_rowGrand = nullptr, *m_colGrand = nullptr, *m_preserve = nullptr, *m_autofit = nullptr, *m_headers = nullptr,
              *m_refresh = nullptr, *m_generate = nullptr;
};

// What Field Settings sets for one field of the rows, columns, or filters: its name, whether it has a subtotal, the form it is shown in, a blank line after each item, repeated labels, its sort, a filter,
// and which items show.
struct FieldSettings {
    int field = 0;
    QString sourceName, name;
    bool subtotals = true, blankRow = false, repeatLabels = false;
    int form = 2;                  // 0 tabular, 1 outline, 2 compact
    int sort = 0;                  // 0 manual, 1 ascending, 2 descending (by the labels)
    int sortByData = -1;           // a value to sort by (a data field's place), or -1
    bool sortByDescending = true;  // (with sortByData)
    jp::office::sheet::PivotFilter filter;
    struct Item {
        QString label;
        int x = 0;
        bool shown = true;
    };
    QVector<Item> items;
    bool itemsChanged = false;
    QStringList dataNames;         // the table's values, for a sort or a filter by value
};

class FieldSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit FieldSettingsDialog(const FieldSettings &settings, QWidget *parent = nullptr);
    FieldSettings settings() const;

private:
    FieldSettings m_settings;
    QLineEdit *m_name = nullptr, *m_text = nullptr, *m_text2 = nullptr;
    QComboBox *m_subtotals = nullptr, *m_form = nullptr, *m_sort = nullptr, *m_sortData = nullptr, *m_filter = nullptr, *m_filterData = nullptr;
    QCheckBox *m_blank = nullptr, *m_repeat = nullptr;
    QListWidget *m_items = nullptr;
};

// What Value Field Settings sets for one value: its name and function, how it is shown (as a share of the total, against another item, ...), and its number format as a code ("" keeps the
// format the cells have; the file's own number format id is `numFmtId`).
struct ValueFieldSettings {
    QString name, function = QStringLiteral("sum"), showAs = QStringLiteral("normal");
    int baseField = 0, baseItem = 0;
    QString numFmtCode;
    int numFmtId = 0;
    // for the dialog: the fields on the axes, each with its items (the base of a difference, a percent of, a percent difference, or a running total)
    struct Base {
        int field = 0;
        QString name;
        QStringList items;
    };
    QVector<Base> bases;
    QString sourceName;
};

// Value Field Settings: the name the value shows, how it is totaled (`functions` and their words: sum, count, countNums, average, max, min, product, stdDev, stdDevp, var, varp), how it is shown
// (Show Values As), and its number format.
class ValueFieldDialog : public QDialog {
    Q_OBJECT
public:
    ValueFieldDialog(const QString &sourceName, const QString &name, const QString &function, QWidget *parent = nullptr);
    explicit ValueFieldDialog(const ValueFieldSettings &settings, QWidget *parent = nullptr);
    QString name() const;
    QString function() const;
    ValueFieldSettings settings() const;

private:
    void build(const ValueFieldSettings &settings);
    void baseChanged();
    QLineEdit *m_name = nullptr;
    QListWidget *m_functions = nullptr;
    QLabel *m_source = nullptr;
    QComboBox *m_showAs = nullptr, *m_baseField = nullptr, *m_baseItem = nullptr, *m_format = nullptr;
    ValueFieldSettings m_settings;
};

} // namespace jp

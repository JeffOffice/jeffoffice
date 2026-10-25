#pragma once
// The names of the workbook: a list of them (each with what it stands for, the value that has, the sheet it belongs to, and its comment) that can be filtered, with a way to make a name,
// change one, and take one away, and the small dialog that makes or changes one (and says, as the name is typed, what is wrong with it).

#include "core/editsession.h"

#include <QAbstractTableModel>
#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableView;

namespace jp {

// What one name is made of. For a new name `id` is -1.
class NameEditDialog : public QDialog {
    Q_OBJECT
public:
    // `initial.scope` is where a new name belongs (-1: the workbook); a name that is changed stays where it is.
    NameEditDialog(crunch::EditSession *session, int id, const crunch::EditSession::NameDraft &initial, const QStringList &sheets, QWidget *parent = nullptr);
    crunch::EditSession::NameDraft draft() const;
    QLineEdit *nameBox() const { return m_name; }
    QLineEdit *textBox() const { return m_text; }
    QLineEdit *commentBox() const { return m_comment; }
    QComboBox *scopeBox() const { return m_scope; }
    QLabel *problemLabel() const { return m_problem; }
    QPushButton *okButton() const { return m_ok; }

private:
    void check();
    crunch::EditSession *m_session;
    int m_id;
    QLineEdit *m_name = nullptr, *m_text = nullptr, *m_comment = nullptr;
    QComboBox *m_scope = nullptr;
    QLabel *m_problem = nullptr;
    QPushButton *m_ok = nullptr;
};

// The rows of the list: the names that are listed and fit the filter, at most crunch::kMaxNamesIndexed of them.
class NamesModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { NameColumn, ValueColumn, RefersColumn, ScopeColumn, CommentColumn, ColumnCount };
    explicit NamesModel(crunch::EditSession *session, QObject *parent = nullptr);
    void setFilter(const QString &text);
    void reload();
    int idAt(int row) const { return row >= 0 && row < int(m_ids.size()) ? m_ids[size_t(row)] : -1; }
    int rowOf(int id) const;
    // How many names fit the filter, and whether some were left out of the list because there were more than it works over.
    int total() const { return m_total; }
    int leftOut() const { return m_leftOut; }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override { return parent.isValid() ? 0 : int(m_ids.size()); }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override { return parent.isValid() ? 0 : ColumnCount; }
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QString scopeText(const crunch::NameEntry &e) const;
    crunch::EditSession *m_session;
    QString m_filter;
    std::vector<int> m_ids;
    int m_total = 0, m_leftOut = 0;
};

class NamesDialog : public QDialog {
    Q_OBJECT
public:
    // `sheet` is the sheet that is open (a new name belongs to the workbook, and the cells selected are what it is first given); `selection` is the text of the cells selected, "Sheet1!$A$1:$B$2".
    NamesDialog(crunch::EditSession *session, int sheet, const QString &selection, QWidget *parent = nullptr);
    QLineEdit *filterBox() const { return m_filter; }
    QTableView *table() const { return m_table; }
    NamesModel *model() const { return m_model; }
    QLabel *countLabel() const { return m_count; }
    QPushButton *newButton() const { return m_new; }
    QPushButton *editButton() const { return m_edit; }
    QPushButton *deleteButton() const { return m_delete; }
    // For tests: what the small dialog is asked, instead of showing it (it gives false for Cancel).
    void setEditAsker(const std::function<bool(crunch::EditSession::NameDraft *)> &asker) { m_asker = asker; }
    void newName();
    void editName();
    void deleteName();

private:
    void refresh();
    void update();
    QStringList sheetNames() const;
    bool ask(int id, crunch::EditSession::NameDraft *draft);
    crunch::EditSession *m_session;
    int m_sheet;
    QString m_selection;
    NamesModel *m_model = nullptr;
    QLineEdit *m_filter = nullptr;
    QTableView *m_table = nullptr;
    QLabel *m_count = nullptr, *m_notice = nullptr;
    QPushButton *m_new = nullptr, *m_edit = nullptr, *m_delete = nullptr;
    std::function<bool(crunch::EditSession::NameDraft *)> m_asker;
};

} // namespace jp

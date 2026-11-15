#pragma once
// The data entry form (Data > Form): the list round the cell, one record at a time. The fields are labeled by the list's header row; a field that holds a formula is shown read-only. New, Delete (asks
// first), Restore (takes back what was typed in this record), Find Prev and Find Next (the records that meet the criteria, when there are some), Criteria (type conditions for the fields, such as >100,
// then Find Next), and Close. Each record changed, added, or deleted is one step of Undo, done as the person leaves the record (the model is core/dataform.h).

#include "core/dataform.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <functional>
#include <vector>

class QFormLayout;

namespace jp {

class DataFormDialog : public QDialog {
    Q_OBJECT
public:
    // `session` must outlive the dialog. Use ready() before showing it: a cell with no list round it gives no form, and problem() says why.
    DataFormDialog(crunch::EditSession *session, int sheet, quint32 row, quint32 col, const QString &title, QWidget *parent = nullptr);
    bool ready() const { return m_ready; }
    QString problem() const { return m_problem; }

    enum class Mode { Record, New, Criteria };
    Mode mode() const { return m_mode; }
    const crunch::DataForm &form() const { return m_form; }
    // The question before a delete ("Delete this record?"), and how a refusal is told: replaceable (for tests); the defaults are message boxes.
    void setDeleteAsker(const std::function<bool(const QString &)> &asker) { m_deleteAsker = asker; }
    void setNotifier(const std::function<void(const QString &)> &notifier) { m_notifier = notifier; }

    // What the window shows, and the commands under the buttons (the buttons call these).
    QLabel *positionLabel() const { return m_position; }
    QLabel *statusLabel() const { return m_status; }
    QLineEdit *fieldBox(int field) const { return field >= 0 && field < int(m_boxes.size()) ? m_boxes[size_t(field)] : nullptr; }
    int fieldBoxCount() const { return int(m_boxes.size()); }
    QLabel *fieldLabel(int field) const { return field >= 0 && field < int(m_labels.size()) ? m_labels[size_t(field)] : nullptr; }
    QPushButton *newButton() const { return m_new; }
    QPushButton *deleteButton() const { return m_delete; }
    QPushButton *restoreButton() const { return m_restore; }
    QPushButton *findPreviousButton() const { return m_previous; }
    QPushButton *findNextButton() const { return m_next; }
    QPushButton *criteriaButton() const { return m_criteria; }
    QPushButton *closeButton() const { return m_close; }
    void newRecord();
    void deleteRecord();
    void restore();
    void findPrevious();
    void findNext();
    void toggleCriteria();
    bool closeForm();

Q_SIGNALS:
    // A record was changed, added, or deleted (cells changed).
    void recordsChanged();

private:
    void build(const QString &title);
    void showRecord(qint64 record);
    void showBlank(Mode mode);
    QStringList typed() const;
    bool commitCurrent(bool *added = nullptr);   // the record's texts are written (or the new record added); false, with the problem told, when that is refused: the form stays where it is
    void tell(const QString &text);
    void updateButtons();
    crunch::DataForm m_form;
    bool m_ready = false;
    QString m_problem;
    Mode m_mode = Mode::Record;
    QString m_title;
    QFormLayout *m_fields = nullptr;
    std::vector<QLineEdit *> m_boxes;
    std::vector<QLabel *> m_labels;
    QLabel *m_position = nullptr, *m_status = nullptr;
    QPushButton *m_new = nullptr, *m_delete = nullptr, *m_restore = nullptr, *m_previous = nullptr, *m_next = nullptr, *m_criteria = nullptr, *m_close = nullptr;
    std::function<bool(const QString &)> m_deleteAsker;
    std::function<void(const QString &)> m_notifier;
    crunch::EditSession *m_session = nullptr;
};

} // namespace jp

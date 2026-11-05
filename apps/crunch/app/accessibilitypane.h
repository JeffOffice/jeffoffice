#pragma once
// The Accessibility Checker: a small window that lists what in the workbook is hard for some readers to use, with Check Again, and goes to a problem when it is clicked.

#include "core/accesscheck.h"

#include <QDialog>

class QLabel;
class QListWidget;

namespace jp {

class AccessibilityPane : public QDialog {
    Q_OBJECT
public:
    explicit AccessibilityPane(QWidget *parent = nullptr);
    void setIssues(const std::vector<crunch::AccessIssue> &issues);
    int issueCount() const;
    QListWidget *list() const { return m_list; }

Q_SIGNALS:
    void recheckRequested();
    void goToRequested(int sheet, quint32 row, quint32 col);

private:
    QLabel *m_summary;
    QListWidget *m_list;
};

} // namespace jp

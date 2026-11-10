#pragma once
// The Accessibility Checker: a small window that lists what in a file is hard for some readers to use, with Check Again, and goes to a problem when it is clicked.
// The same in the apps that have it; each app finds the problems and says what "go to" is for its kind of file (`where` is whatever it needs, handed back).

#include <QDialog>
#include <QVariantList>
#include <QVector>

class QHBoxLayout;
class QLabel;
class QPushButton;
class QListWidget;

namespace jp {

struct AccessFinding {
    enum Level { Error, Warning, Tip };
    Level level = Warning;
    QString message;
    QVariantList where;
};

class AccessibilityPane : public QDialog {
    Q_OBJECT
public:
    explicit AccessibilityPane(QWidget *parent = nullptr);
    void setFindings(const QVector<AccessFinding> &findings);
    int issueCount() const;
    QListWidget *list() const { return m_list; }

    // A button beside Check Again, for an app with more to offer than the list (the pages' Reading Order).
    QPushButton *addButton(const QString &text, const QString &objectName);

Q_SIGNALS:
    void recheckRequested();
    void goToRequested(const QVariantList &where);

private:
    QLabel *m_summary;
    QListWidget *m_list;
    QHBoxLayout *m_buttons;
};

} // namespace jp

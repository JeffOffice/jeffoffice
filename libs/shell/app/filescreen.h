#pragma once
// The File screen's frame, the same in every app: a sidebar with the app's
// badge and name, a back button, and the commands in groups, beside the page
// chosen. The app adds its sections, pages, and commands, and builds each
// page when it is shown.

#include <QHash>
#include <QWidget>
#include <functional>

class QAbstractButton;
class QButtonGroup;
class QFrame;
class QLabel;
class QStackedWidget;
class QVBoxLayout;

namespace jp {

class FileScreen : public QWidget {
    Q_OBJECT
public:
    explicit FileScreen(QWidget *parent = nullptr);

    // The button that leaves the screen: "Back to publication".
    void setBackButton(const QString &text, const QString &toolTip);
    // Builds the page `key` each time it is shown (null: no such page).
    void setPageBuilder(const std::function<QWidget *(const QString &key)> &builder) { m_builder = builder; }
    void addSection(const QString &title);
    // An entry that shows the page `key`; `icon` is a name for icon().
    void addPage(const QString &key, const QString &icon, const QString &label);
    // A page the app shows itself, with no entry in the sidebar.
    void addHiddenPage(const QString &key);
    // An entry that runs something instead of showing a page.
    void addCommand(const QString &key, const QString &icon, const QString &label, const std::function<void()> &run);
    void showPage(const QString &key);
    QString currentPage() const;   // "info", "new", "print"...

Q_SIGNALS:
    void closeRequested();

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    void insertEntry(QWidget *w);
    void rebuild(const QString &key);
    void restyle();   // the colors that follow the light or dark look
    QFrame *m_side = nullptr;
    QVBoxLayout *m_sideLayout = nullptr;
    QAbstractButton *m_back = nullptr;
    QLabel *m_version = nullptr;
    QHash<QString, QAbstractButton *> m_navItems;
    QButtonGroup *m_navGroup = nullptr;
    QStackedWidget *m_stack = nullptr;
    QHash<QString, int> m_index;
    std::function<QWidget *(const QString &)> m_builder;
    int m_next = 0;   // where the next entry goes in the sidebar
};

} // namespace jp

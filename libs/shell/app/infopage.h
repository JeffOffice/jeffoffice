#pragma once
// The Info page of the File screen, the same in JeffWrite and JeffCrunch (and open to the other apps): where the file is (with Show in Folder and Copy Path), the Protect cards
// (Mark as Final and Encrypt with Password), the Inspect and Compatibility cards, and the file's properties to read and edit, with the custom ones in a box of their own. The app
// gives the page its file's facts and what each card does; the page keeps nothing of its own.

#include "office/packageprops.h"

#include <QDialog>
#include <QImage>
#include <QStringList>
#include <QVector>
#include <functional>

class QLabel;
class QPushButton;
class QStandardItemModel;
class QTableView;
class QWidget;

namespace jp {

struct InfoHost {
    enum class Kind { Document, Workbook };
    Kind kind = Kind::Document;
    std::function<QString()> path;   // the file's path, empty when it has none yet
    std::function<office::PackageProps()> props;
    // Applies the properties the person edited (the app marks its file as changed).
    std::function<void(const office::PackageProps &)> setProps;
    // The rows of the "Statistics" part (a label and its value, in the order shown), for the counts the file has.
    std::function<QVector<QPair<QString, QString>>()> statistics;
    std::function<QString()> markFinalBlocked;   // why the file can't be marked as final ("" when it can); none: always
    std::function<bool()> markedFinal;
    std::function<void(bool)> setMarkedFinal;
    std::function<bool()> hasPassword;
    std::function<void()> encrypt;   // asks for the password (the app does what follows)
    std::function<void()> inspect;
    std::function<void()> checkCompatibility;   // none: no card
    std::function<void()> versionHistory;       // none: no card
    // The page the person is shown the file on, drawn small; none: no picture.
    std::function<QImage()> thumbnail;
};

// The page, built for the host's file as it is now.
QWidget *buildInfoPage(QWidget *parent, const InfoHost &host);

// What Show in Folder opens for a file's path; a path with nothing there gives "".
QString folderOfFile(const QString &path);
// The path as the system writes it, for Copy Path.
QString pathForCopy(const QString &path);

// The box that lists, adds, changes, and removes a file's custom properties: a table over a model (a file may have thousands), the kind of each chosen in a list that shows when the cell is edited.
// A property of a kind this program does not model is listed, not editable, and kept as it was (or deleted with its row).
class CustomPropertiesDialog : public QDialog {
    Q_OBJECT
public:
    CustomPropertiesDialog(const QVector<office::CustomProperty> &now, QWidget *parent);
    // What the table says; the properties not modeled are the file's own, as they were.
    QVector<office::CustomProperty> result() const;
    // The first thing wrong with the table (a name that is empty, used twice, or kept for the program's own use; a value that its kind can't hold), "" when nothing is.
    QString problem() const;
    // For tests: the row count, a way to add one, and to change a cell.
    int rowCount() const;
    void addRow(const office::CustomProperty &p);
    void setCell(int row, int column, const QString &text);

protected:
    void accept() override;

private:
    void addEmptyRow();
    QStandardItemModel *m_model = nullptr;
    QTableView *m_table = nullptr;
};

// One kind of thing Inspect finds and can take out.
struct InspectItem {
    QString title;                 // "Comments"
    QString detail;                // what it is and why to remove it
    std::function<int()> count;    // how many there are now
    std::function<int()> remove;   // takes them all out; how many were
    bool removable = true;
    QString action;                // the button's words, "Remove All" when empty
};

// The Inspect box: each kind with how many were found and a button that removes them all. It looks again after each removal.
class InspectDialog : public QDialog {
    Q_OBJECT
public:
    InspectDialog(const QString &heading, const QVector<InspectItem> &items, QWidget *parent);
    // For tests: what the rows say, and a press of a row's Remove All.
    QStringList summaries() const;
    void removeRow(int row);

private:
    void refresh();
    QVector<InspectItem> m_items;
    QVector<QLabel *> m_counts;
    QVector<QPushButton *> m_buttons;
};

} // namespace jp

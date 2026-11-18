#pragma once
// The two boxes of objects between apps, the same in the three apps that have them: the one that asks where a new object comes from (a file and a range or page, or what was copied in the
// other program), and the one that lists the objects of a document with what can be done to each. An app gives the list box what each action does; the box keeps nothing of its own.

#include "app/objects.h"
#include "core/export.h"

#include <QDialog>
#include <QString>
#include <QVector>

#include <functional>

class QTreeWidget;
class QPushButton;

namespace jp::objects {

enum class Source { Range, Chart, Page };

struct SourceChoice {
    bool copied = false;   // use what was copied in the other program
    QString path;
    QString detail;        // the range (A1:C5) or the page number
};

// Asks where the object comes from; false if cancelled. `copiedAvailable` is whether the clipboard holds an object of the kind (the choice to use it is then offered, and chosen).
JO_COMMON_EXPORT bool askSource(QWidget *parent, Source source, bool copiedAvailable, SourceChoice *choice);
// The words of the box's title for a source ("From JeffCrunch Range").
JO_COMMON_EXPORT QString sourceTitle(Source source);

struct ObjectRow {
    int id = 0;           // what the app calls the object by
    QString name;
    QString kind;         // in words (objects::kindWords)
    QString size;
    bool convertible = true;   // an object of ours that can become a picture
};

struct ListHost {
    std::function<QVector<ObjectRow>()> rows;
    std::function<void(int id)> open;
    std::function<bool(int id)> convertToPicture;
    std::function<bool(int id)> remove;
};

class JO_COMMON_EXPORT ObjectListDialog : public QDialog {
    Q_OBJECT
public:
    ObjectListDialog(const ListHost &host, QWidget *parent);
    // For tests: the rows shown, and the selection.
    int rowCount() const;
    QString textAt(int row, int column) const;
    void select(int row);

private:
    void fill();
    int currentId() const;
    ListHost m_host;
    QTreeWidget *m_list = nullptr;
    QPushButton *m_open = nullptr, *m_convert = nullptr, *m_delete = nullptr;
};

// The size of an object as the list shows it ("3.0 by 2.0 in").
JO_COMMON_EXPORT QString sizeWords(qint64 widthEmu, qint64 heightEmu);

} // namespace jp::objects

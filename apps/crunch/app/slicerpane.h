// The Slicers pane: each slicer of the workbook as a caption over a set of buttons (one for each item of its field), each timeline as its span, and the tables each filters.
#pragma once

#include <QStringList>
#include <QVector>
#include <QWidget>

class QAbstractButton;
class QVBoxLayout;

namespace jp {

class SlicerPane : public QWidget {
    Q_OBJECT
public:
    struct Entry {
        QString caption, style;
        bool timeline = false, usable = false;      // (a slicer that filters a table JeffCrunch can read)
        QStringList items, tables;
        QVector<bool> chosen;
        QString span, level;                        // (a timeline) what it chooses
        int columns = 1;
    };
    explicit SlicerPane(QWidget *parent = nullptr);
    void setEntries(const QVector<Entry> &entries);
    int slicerCount() const { return m_buttons.size(); }
    QAbstractButton *button(int slicer, int item) const;   // (for tests)

Q_SIGNALS:
    void itemClicked(int slicer, int item, bool extend);   // `extend`: Ctrl was held (the item is added to the chosen or taken from them)
    void connectionsRequested(int slicer);

private:
    QVBoxLayout *m_layout = nullptr;
    QWidget *m_body = nullptr;
    QVector<QVector<QAbstractButton *>> m_buttons;
    QStringList m_shape;   // what the buttons were made for
    bool m_built = false;
};

} // namespace jp

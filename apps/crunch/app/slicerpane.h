// The Slicers pane: each slicer of the workbook as a caption over a set of buttons (one for each item of its field), each timeline as its span, and the tables each filters.
#pragma once

#include <QStringList>
#include <QVector>
#include <QWidget>

class QAbstractButton;
class QLabel;
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
    static constexpr int kMaxButtons = 1000;   // the buttons one slicer shows (a field has up to a million items): the rest are told of in a note
    bool hasMoreNote(int slicer) const { return slicer >= 0 && slicer < m_notes.size() && m_notes[slicer]; }
    QString moreNoteText(int slicer) const;
    QString captionOf(int slicer) const { return slicer >= 0 && slicer < m_captions.size() ? m_captions[slicer] : QString(); }   // (for tests)
    int slicerCount() const { return m_buttons.size(); }
    QAbstractButton *button(int slicer, int item) const;   // (for tests)
    void clickItem(int slicer, int item, bool extend) { Q_EMIT itemClicked(slicer, item, extend); }   // (what a click on a button does; for tests, with Ctrl as `extend`)

Q_SIGNALS:
    void itemClicked(int slicer, int item, bool extend);   // `extend`: Ctrl was held (the item is added to the chosen or taken from them)
    void connectionsRequested(int slicer);

private:
    QVBoxLayout *m_layout = nullptr;
    QWidget *m_body = nullptr;
    QVector<QVector<QAbstractButton *>> m_buttons;
    QVector<QLabel *> m_notes;
    QStringList m_captions;   // the captions as the boxes show them
    QStringList m_shape;   // what the buttons were made for
    bool m_built = false;
};

} // namespace jp

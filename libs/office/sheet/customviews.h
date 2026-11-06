#pragma once
// Custom views: names for ways of looking at the workbook (which sheet is in front, which rows and columns are hidden, the zoom, the frozen panes, the gridlines and headings, and the size of
// the window), shown again and kept with the workbook. The file holds each as a `customWorkbookView`; the standard says nothing about the hidden rows, so they are kept in an extension of
// the element (ours, ignored by other programs), and a view another program made, with none, still lists and shows its sheet and window.

#include <QByteArray>
#include <QPair>
#include <QString>
#include <QVector>

class QXmlStreamReader;

namespace jp::office::sheet {

constexpr int kMaxCustomViews = 200;
constexpr int kMaxCustomViewSheets = 1000;
// The most runs of hidden rows and columns one view holds, all its sheets together. Reading, making a view, and showing one all use this one number, so a view that is kept reads back as it was.
constexpr int kMaxCustomViewSpans = 20000;
constexpr int kMaxCustomViewNameChars = 255;
// The size of a sheet.
constexpr quint32 kCustomViewRows = 1048576, kCustomViewColumns = 16384;
// The most rows and columns a view may hold frozen (more would leave nothing to scroll): a view that asks for more is read with no frozen panes.
constexpr quint32 kCustomViewMaxFrozenRows = 1000, kCustomViewMaxFrozenCols = 200;

// One sheet as a view left it. Rows and columns count from 0; a span is first and last, both included.
struct CustomViewSheet {
    QString name;
    int zoom = 100;                     // percent
    bool frozen = false;
    quint32 frozenRows = 0, frozenCols = 0;
    quint32 activeRow = 0, activeCol = 0;
    QVector<QPair<quint32, quint32>> hiddenRows, hiddenCols;
    bool operator==(const CustomViewSheet &o) const
    {
        return name == o.name && zoom == o.zoom && frozen == o.frozen && frozenRows == o.frozenRows && frozenCols == o.frozenCols
            && activeRow == o.activeRow && activeCol == o.activeCol && hiddenRows == o.hiddenRows && hiddenCols == o.hiddenCols;
    }
};

struct CustomView {
    QString name, guid;
    QString activeSheet;                // by name, so it follows the sheet when the tabs are moved
    int activeSheetId = 1;              // the file's `activeSheetId` (the sheet's sheetId)
    int windowWidth = 0, windowHeight = 0, xWindow = 0, yWindow = 0;
    bool gridlines = true, headings = true, formulaBar = true;
    bool hasState = false;              // the view holds the sheets below (a view another program made does not)
    QVector<CustomViewSheet> sheets;
    // The attributes of a view this program does not look at (autoUpdate, onlySync, and the others), kept as they were.
    QVector<QPair<QString, QString>> otherAttributes;
    // What other programs put in the view's `extLst` (each `ext` that is not ours), kept as it was read.
    QByteArray foreignExt;
    bool operator==(const CustomView &o) const
    {
        return name == o.name && guid == o.guid && activeSheet == o.activeSheet && activeSheetId == o.activeSheetId && windowWidth == o.windowWidth && windowHeight == o.windowHeight
            && xWindow == o.xWindow && yWindow == o.yWindow && gridlines == o.gridlines && headings == o.headings && formulaBar == o.formulaBar && hasState == o.hasState
            && sheets == o.sheets && otherAttributes == o.otherAttributes && foreignExt == o.foreignExt;
    }
};

// A sheet was renamed: the views name it by its new name. A sheet was deleted: the views no longer hold it.
void renameCustomViewSheet(QVector<CustomView> *views, const QString &from, const QString &to);
void dropCustomViewSheet(QVector<CustomView> *views, const QString &name);

// How many spans a view holds in all.
int customViewSpanCount(const CustomView &view);

// The namespace of our extension, and its `ext` uri.
extern const char kCustomViewNs[];
extern const char kCustomViewExtUri[];

// A new guid in the braces the file uses.
QString newCustomViewGuid();

// Spans as the file's text ("2:4 7", counting from 1) and back; the text of a hostile file is read as far as it makes sense.
QString spansToText(const QVector<QPair<quint32, quint32>> &spans);
// Reads at most `most` spans, stopping there without looking at the rest; each is kept inside `lines` rows (or columns) or dropped.
QVector<QPair<quint32, quint32>> spansFromText(const QString &text, quint32 lines = kCustomViewRows, int most = kMaxCustomViewSpans);

// Spans sorted, with the ones that touch or overlap joined; and the part of `a` that `b` does not cover (both given normalized).
QVector<QPair<quint32, quint32>> normalizedSpans(QVector<QPair<quint32, quint32>> spans);
QVector<QPair<quint32, quint32>> spansWithout(const QVector<QPair<quint32, quint32>> &a, const QVector<QPair<quint32, quint32>> &b);

// Reads the `customWorkbookView` the reader is on (its attributes), and the element of ours that sits inside it (`readCustomViewState` on the `view` element, `readCustomViewSheet` on each
// `sheet`).
void readCustomViewAttributes(const QXmlStreamReader &r, CustomView *view);
// Copies the `ext` elements of a view that are not ours: feed it each token after the view's start tag; `feed` is true once the view's end tag has been given.
class CustomViewForeign {
public:
    CustomViewForeign();
    ~CustomViewForeign();
    bool feed(const QXmlStreamReader &r);
    QByteArray bytes() const { return m_bytes; }

private:
    QByteArray m_bytes;
    int m_level = 0;
    bool m_copying = false;
};
void readCustomViewState(const QXmlStreamReader &r, CustomView *view);
void readCustomViewSheet(const QXmlStreamReader &r, CustomView *view);

// The element `customWorkbookViews` for the views (empty for none).
QByteArray customWorkbookViewsElement(const QVector<CustomView> &views, const QByteArray &prefix);

} // namespace jp::office::sheet

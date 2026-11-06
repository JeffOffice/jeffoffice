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
constexpr int kMaxCustomViewSpans = 20000;
constexpr int kMaxCustomViewNameChars = 255;

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
    bool operator==(const CustomView &o) const
    {
        return name == o.name && guid == o.guid && activeSheet == o.activeSheet && activeSheetId == o.activeSheetId && windowWidth == o.windowWidth && windowHeight == o.windowHeight
            && xWindow == o.xWindow && yWindow == o.yWindow && gridlines == o.gridlines && headings == o.headings && formulaBar == o.formulaBar && hasState == o.hasState
            && sheets == o.sheets && otherAttributes == o.otherAttributes;
    }
};

// The namespace of our extension, and its `ext` uri.
extern const char kCustomViewNs[];
extern const char kCustomViewExtUri[];

// A new guid in the braces the file uses.
QString newCustomViewGuid();

// Spans as the file's text ("2:4 7", counting from 1) and back; the text of a hostile file is read as far as it makes sense.
QString spansToText(const QVector<QPair<quint32, quint32>> &spans);
QVector<QPair<quint32, quint32>> spansFromText(const QString &text);

// Spans sorted, with the ones that touch or overlap joined; and the part of `a` that `b` does not cover (both given normalized).
QVector<QPair<quint32, quint32>> normalizedSpans(QVector<QPair<quint32, quint32>> spans);
QVector<QPair<quint32, quint32>> spansWithout(const QVector<QPair<quint32, quint32>> &a, const QVector<QPair<quint32, quint32>> &b);

// Reads the `customWorkbookView` the reader is on (its attributes), and the element of ours that sits inside it (`readCustomViewState` on the `view` element, `readCustomViewSheet` on each
// `sheet`).
void readCustomViewAttributes(const QXmlStreamReader &r, CustomView *view);
void readCustomViewState(const QXmlStreamReader &r, CustomView *view);
void readCustomViewSheet(const QXmlStreamReader &r, CustomView *view);

// The element `customWorkbookViews` for the views (empty for none).
QByteArray customWorkbookViewsElement(const QVector<CustomView> &views, const QByteArray &prefix);

} // namespace jp::office::sheet

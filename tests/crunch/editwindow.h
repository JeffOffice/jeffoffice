// What the tests of JeffCrunch's editing share: the sheet most of them edit, a window with a folder of its own and the keys a person presses, and what a saved file holds of a cell.
#pragma once

#include "app/crunchwindow.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "grid/gridview.h"

#include <QAction>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QKeySequence>
#include <QTest>

namespace crunchtest {

using namespace jp;

// The sheet most of these tests edit:
//        A            B              C          D
//   1    1            =A1*2 (999)    hello
//   2    10           =SUM(A1:A2)    "          2024-01-02 shown as a date (a number with a date format)
//   3    0.25 (0%)    =A3+1
// with the styles: 1 percent, 2 a date, 3 text.
inline QByteArray editBook(const QByteArray &extraRows = QByteArray())
{
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"9\" applyNumberFormat=\"1\"/><xf numFmtId=\"14\" applyNumberFormat=\"1\"/><xf numFmtId=\"49\" applyNumberFormat=\"1\"/>"));
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1*2</f><v>999</v></c><c r=\"C1\" t=\"s\"><v>0</v></c></row>"
                                        "<row r=\"2\"><c r=\"A2\"><v>10</v></c><c r=\"B2\"><f>SUM(A1:A2)</f><v>999</v></c><c r=\"D2\" s=\"2\"><v>45293</v></c></row>"
                                        "<row r=\"3\"><c r=\"A3\" s=\"1\"><v>0.25</v></c><c r=\"B3\"><f>A3+1</f><v>999</v></c><c r=\"C3\" s=\"3\"><v>7</v></c></row>")
        + QString::fromUtf8(extraRows);
    return buildXlsx({worksheet(rows)}, sharedStringsXml({QStringLiteral("<t>hello</t>")}), QString(), QString(), styles);
}

struct EditWindow {
    Folder folder;
    Window win;
    QString path;
    bool open(const QByteArray &bytes, const QString &name = QStringLiteral("book.xlsx"))
    {
        path = folder.write(name, bytes);
        return win.open(path);
    }
    grid::GridView *view() { return win.w.view(); }
    void select(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        view()->selectCell(r, c);
    }
    QString selected() { return view()->selectionName(); }
    // A copy of the file as it is now, under another name (a file that is open in a window is not opened in a second: the copy is).
    QString copyOfFile()
    {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) return QString();
        return folder.write(QStringLiteral("copy%1.xlsx").arg(++m_copies), f.readAll());
    }
    int m_copies = 0;
    // The value the grid shows for the cell, and the text it shows.
    QString valueAt(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        const grid::Cell *cell = view()->sheet().cellAt(r, c);
        return cell ? view()->sheet().valueOf(*cell).debug() : QStringLiteral("-");
    }
    // The text of the cell in its format, whole (the cell may be too narrow to show it).
    QString shownAt(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        const grid::Cell *cell = view()->sheet().cellAt(r, c);
        return cell ? view()->sheet().textOf(*cell) : QString();
    }
    // Types text into the selected cell as a person does: the first character at the grid (which starts the entry), the rest into the cell's line.
    void type(const QString &text)
    {
        QTest::keyClick(view(), text.at(0).toLatin1());
        if (text.size() > 1) QTest::keyClicks(win.w.cellEditor(), text.mid(1));
    }
    void key(Qt::Key k, Qt::KeyboardModifiers m = Qt::NoModifier) { QTest::keyClick(win.w.cellEditor(), k, m); }
    // Waits until the worker has worked out what the edits made. The window says it is calculating only for what its own commands did; an edit made through the session() does not set
    // that, so the worker itself is asked: it is idle with nothing left to work out (seen with it held), and what it sent is taken. (Not a pause: a worker that has not yet woken to
    // an edit is idle too, but the edit has left formulas for it to work out.)
    bool settle()
    {
        if (!QTest::qWaitFor([&] { return !win.w.isCalculating(); }, 20000)) return false;
        auto &loader = win.w.workbookLoader();
        if (!loader.result()) return true;
        QElapsedTimer waited;
        waited.start();
        for (;;) {
            QTest::qWait(1);   // (the batches that are on their way are taken)
            bool done = false;
            if (loader.result()->calcIdle.load()) {
                auto hold = loader.hold();
                done = !hold || !hold->calculation()->isDirty();
            }
            if (done) break;
            if (waited.elapsed() > 20000) return false;
        }
        QCoreApplication::processEvents();
        return true;
    }
    void enter(const QString &text)
    {
        type(text);
        key(Qt::Key_Return);
    }
};

// The keys of a standard command on this computer (Cmd+Z on a Mac, Ctrl+Z elsewhere; redo is Ctrl+Y on Windows, Cmd+Shift+Z on a Mac, Ctrl+Shift+Z or Ctrl+Y on Linux): the first the
// platform gives.
inline void pressStandard(QWidget *w, QKeySequence::StandardKey command)
{
    const QKeySequence keys(command);
    const QKeyCombination k = keys[0];
    QTest::keyClick(w, k.key(), k.keyboardModifiers());
}
inline void pressUndo(QWidget *w) { pressStandard(w, QKeySequence::Undo); }
inline void pressRedo(QWidget *w) { pressStandard(w, QKeySequence::Redo); }


// The formula of a cell of a saved file, as the file holds it.
inline QString savedFormula(const QString &path, const QString &ref, int sheet = 0)
{
    Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return why;
    Workbook b;
    if (!b.open(p, &why)) return why;
    Sheet s;
    if (!b.readSheet(sheet, &s)) return QStringLiteral("unreadable");
    const Cell *c = at(s, ref);
    return c ? s.formulaText(*c) : QStringLiteral("-");
}
// The cell of a saved file, as the file holds it.
inline QString savedCell(const QString &path, const QString &ref, int sheet = 0)
{
    Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return why;
    Workbook b;
    if (!b.open(p, &why)) return why;
    Sheet s;
    if (!b.readSheet(sheet, &s)) return QStringLiteral("unreadable");
    return show(s, b.sharedStrings(), ref);
}

inline CellRange rangeOf(const QString &ref)
{
    CellRange r;
    parseRangeRef(ref, &r);
    return r;
}

// A sheet from the cells' texts: "A1=1" a number, "B2=text", "C3==A1*2" a formula, "D4=@2024-01-15" a date (with the date format, style 1), "E5=%0.25" a number with the percent format
// (style 2). The cells come in any order.
inline QByteArray sheetOf(const QStringList &specs)
{
    struct Item {
        quint32 row, col;
        QString xml;
    };
    QVector<Item> items;
    for (const QString &spec : specs) {
        const int eq = int(spec.indexOf(QLatin1Char('=')));
        const QString ref = spec.left(eq), value = spec.mid(eq + 1);
        quint32 row = 0, col = 0;
        parseCellRef(ref, &row, &col);
        QString xml;
        if (value.startsWith(QLatin1Char('='))) {
            xml = QStringLiteral("<c r=\"%1\"><f>%2</f><v>0</v></c>").arg(ref, value.mid(1).toHtmlEscaped());
        } else if (value.startsWith(QLatin1Char('@'))) {
            const QDate d = QDate::fromString(value.mid(1), Qt::ISODate);
            double serial = 0;
            jp::calc::dateToSerial(d.year(), d.month(), d.day(), jp::calc::DateSystem::Date1900, &serial);
            xml = QStringLiteral("<c r=\"%1\" s=\"1\"><v>%2</v></c>").arg(ref).arg(serial, 0, 'f', 0);
        } else if (value.startsWith(QLatin1Char('%'))) {
            xml = QStringLiteral("<c r=\"%1\" s=\"2\"><v>%2</v></c>").arg(ref, value.mid(1));
        } else {
            bool number = false;
            value.toDouble(&number);
            xml = number ? QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value)
                         : QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, value.toHtmlEscaped());
        }
        items.append({row, col, xml});
    }
    std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
    QString rows;
    for (int i = 0; i < items.size();) {
        const quint32 row = items[i].row;
        rows += QStringLiteral("<row r=\"%1\">").arg(row + 1);
        for (; i < items.size() && items[i].row == row; ++i) rows += items[i].xml;
        rows += QStringLiteral("</row>");
    }
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"14\" applyNumberFormat=\"1\"/><xf numFmtId=\"9\" applyNumberFormat=\"1\"/>"));
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), styles);
}

// What the cells of a range show, in order of row and then column, joined with "|".
inline QString shown(EditWindow &e, const QString &range)
{
    const CellRange r = rangeOf(range);
    QStringList out;
    for (quint32 row = r.row1; row <= r.row2; ++row)
        for (quint32 col = r.col1; col <= r.col2; ++col) out << e.shownAt(cellName(row, col));
    return out.join(QLatin1Char('|'));
}


} // namespace crunchtest

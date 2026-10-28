// The model check of rows, columns, and cells inserted and deleted: see structmodel.h.
#include "structmodel.h"

#include "app/crunchwindow.h"
#include "calc/eval.h"
#include "calc/formula.h"
#include "core/editsession.h"
#include "core/loader.h"
#include "core/sheetobjects.h"
#include "grid/gridview.h"
#include "io/zip.h"
#include "office/package.h"
#include "sheet/cellref.h"
#include "sheet/calculation.h"
#include "sheet/chartdata.h"

#include <QCoreApplication>
#include <QDate>
#include <QElapsedTimer>
#include <QFile>
#include <QLocale>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <QTemporaryDir>
#include <QTextStream>
#include <QThread>

#include <algorithm>
#include <functional>
#include <map>
#include <memory>

using namespace jp;
using namespace jp::office;
using namespace jp::office::sheet;

namespace jp::structmodel {

// (jp has a CellRange and a PageSetup of its own, for the pages of a document)
using jp::office::sheet::CellRange;
using jp::office::sheet::PageSetup;
using jp::office::sheet::Table;

namespace {

// ---- numbers that are the same on every machine

struct Rng {
    quint64 s;
    explicit Rng(quint64 seed) : s(seed * 0x9E3779B97F4A7C15ull + 0xD1B54A32D192ED03ull) {}
    quint64 next()
    {
        quint64 z = (s += 0x9E3779B97F4A7C15ull);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    }
    int bounded(int n) { return n <= 1 ? 0 : int(next() % quint64(n)); }
};

quint64 fnv(const QByteArray &bytes, quint64 h = 1469598103934665603ull)
{
    for (const char c : bytes) {
        h ^= quint8(c);
        h *= 1099511628211ull;
    }
    return h;
}
quint64 fnv(const QString &text, quint64 h = 1469598103934665603ull) { return fnv(text.toUtf8(), h); }

const char *const kSheetNames[3] = {"Data", "Calc", "Extra"};

} // namespace

#include "structmodel_book.inc"

namespace {

// ---- the steps

QString placeName(int sheet, int row, int col) { return QStringLiteral("%1!%2").arg(QLatin1String(kSheetNames[std::clamp(sheet, 0, 2)]), cellName(quint32(std::max(0, row)), quint32(std::max(0, col)))); }
QString blockName(int sheet, int row1, int col1, int row2, int col2) { return QStringLiteral("%1!%2:%3").arg(QLatin1String(kSheetNames[std::clamp(sheet, 0, 2)]), cellName(quint32(row1), quint32(col1)), cellName(quint32(row2), quint32(col2))); }

CellRange rangeOf(int row1, int col1, int row2, int col2)
{
    CellRange r;
    r.valid = true;
    r.row1 = quint32(std::min(row1, row2));
    r.row2 = quint32(std::max(row1, row2));
    r.col1 = quint32(std::min(col1, col2));
    r.col2 = quint32(std::max(col1, col2));
    return r;
}

const char *const kFillWords[4] = {"down", "right", "up", "left"};
const char *const kPasteWords[4] = {"all", "formulas", "values", "formats"};

} // namespace

QString Step::words() const
{
    switch (kind) {
    case InsertRows: return QStringLiteral("insert %1 row(s) at %2").arg(count).arg(placeName(sheet, row1, 0));
    case DeleteRows: return QStringLiteral("delete %1 row(s) at %2").arg(count).arg(placeName(sheet, row1, 0));
    case InsertColumns: return QStringLiteral("insert %1 column(s) at %2").arg(count).arg(placeName(sheet, 0, col1));
    case DeleteColumns: return QStringLiteral("delete %1 column(s) at %2").arg(count).arg(placeName(sheet, 0, col1));
    case InsertCellsDown: return QStringLiteral("insert cells %1, shift down").arg(blockName(sheet, row1, col1, row2, col2));
    case InsertCellsRight: return QStringLiteral("insert cells %1, shift right").arg(blockName(sheet, row1, col1, row2, col2));
    case DeleteCellsUp: return QStringLiteral("delete cells %1, shift up").arg(blockName(sheet, row1, col1, row2, col2));
    case DeleteCellsLeft: return QStringLiteral("delete cells %1, shift left").arg(blockName(sheet, row1, col1, row2, col2));
    case Type: return QStringLiteral("type %1 in %2").arg(text, placeName(sheet, row1, col1));
    case Fill: return QStringLiteral("fill %1 %2 from %3").arg(count).arg(QLatin1String(kFillWords[option & 3]), blockName(sheet, row1, col1, row2, col2));
    case Paste: return QStringLiteral("copy %1 and paste %2 at %3").arg(blockName(sheet, row1, col1, row2, col2), QLatin1String(kPasteWords[option & 3]), placeName(toSheet, toRow, toCol));
    case Cut: return QStringLiteral("cut %1 and paste at %2").arg(blockName(sheet, row1, col1, row2, col2), placeName(toSheet, toRow, toCol));
    case Sort: return QStringLiteral("sort %1 by column %2%3").arg(blockName(sheet, row1, col1, row2, col2)).arg(option / 2 + 1).arg((option & 1) ? QStringLiteral(" descending") : QString());
    case Undo: return QStringLiteral("undo");
    case Redo: return QStringLiteral("redo");
    case Reopen: return QStringLiteral("save to memory and open again");
    case Save: return QStringLiteral("save to the file");
    case Merge: return QStringLiteral("%1 %2").arg(option ? QStringLiteral("unmerge") : QStringLiteral("merge"), blockName(sheet, row1, col1, row2, col2));
    case ResizeRows: return QStringLiteral("%1 rows %2 to %3 of %4").arg(option == 0 ? QStringLiteral("set the height of") : option == 1 ? QStringLiteral("hide") : QStringLiteral("show")).arg(row1 + 1).arg(row2 + 1).arg(QLatin1String(kSheetNames[std::clamp(sheet, 0, 2)]));
    case ResizeColumns: return QStringLiteral("%1 columns %2 to %3 of %4").arg(option == 0 ? QStringLiteral("set the width of") : option == 1 ? QStringLiteral("hide") : QStringLiteral("show"), columnName(quint32(col1)), columnName(quint32(col2)), QLatin1String(kSheetNames[std::clamp(sheet, 0, 2)]));
    case SetNote: return QStringLiteral("put the note %1 on %2").arg(text, placeName(sheet, row1, col1));
    case SetLink: return QStringLiteral("link %1 to %2").arg(blockName(sheet, row1, col1, row2, col2), text);
    case DefineName: return QStringLiteral("define the name %1").arg(text);
    case SetPage: return QStringLiteral("page setup of %1: %2").arg(QLatin1String(kSheetNames[std::clamp(sheet, 0, 2)]), option == 0 ? QStringLiteral("a break before row %1").arg(row1 + 1) : option == 1 ? QStringLiteral("print area %1").arg(blockName(sheet, row1, col1, row2, col2)) : option == 2 ? QStringLiteral("no print area") : QStringLiteral("a break before column %1").arg(columnName(quint32(col1))));
    case KindCount: break;
    }
    return QString();
}

QString Step::line() const
{
    return QStringLiteral("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12").arg(int(kind)).arg(sheet).arg(row1).arg(col1).arg(row2).arg(col2).arg(count).arg(toSheet).arg(toRow).arg(toCol).arg(option).arg(QString::fromLatin1(text.toUtf8().toHex()));
}

bool Step::fromLine(const QString &line, Step *out)
{
    const QStringList f = line.trimmed().split(QLatin1Char(' '), Qt::KeepEmptyParts);
    if (f.size() < 11) return false;
    Step s;
    int k = f[0].toInt();
    if (k < 0 || k >= KindCount) return false;
    s.kind = Kind(k);
    s.sheet = f[1].toInt();
    s.row1 = f[2].toInt();
    s.col1 = f[3].toInt();
    s.row2 = f[4].toInt();
    s.col2 = f[5].toInt();
    s.count = f[6].toInt();
    s.toSheet = f[7].toInt();
    s.toRow = f[8].toInt();
    s.toCol = f[9].toInt();
    s.option = f[10].toInt();
    if (f.size() > 11) s.text = QString::fromUtf8(QByteArray::fromHex(f[11].toLatin1()));
    *out = s;
    return true;
}

namespace {

int pickRow(Rng &r)
{
    switch (r.bounded(10)) {
    case 0:
    case 1: return 0;
    case 2: return 1 + r.bounded(3);
    case 3:
    case 4:
    case 5: return 4 + r.bounded(26);
    case 6: return 29 + r.bounded(2);
    case 7: return 30 + r.bounded(10);
    case 8: return 38 + r.bounded(4);
    default: return 40 + r.bounded(8);
    }
}

int pickCol(Rng &r)
{
    switch (r.bounded(8)) {
    case 0: return 0;
    case 1: return 1 + r.bounded(3);
    case 2:
    case 3:
    case 4: return 4 + r.bounded(8);
    case 5: return 11 + r.bounded(2);
    case 6: return 12 + r.bounded(3);
    default: return r.bounded(12);
    }
}

int pickSheet(Rng &r)
{
    const int x = r.bounded(20);
    return x < 11 ? 0 : x < 16 ? 1 : 2;
}

// A formula to type; it is typed in a column from E on, and reads cells of the columns A to D (and other sheets), so that few of them read themselves.
// With `extra` (and only then does it draw anything more) one in five is of another kind: a range written back to front, a function whose name could be a cell past XFD (ATAN2), or a name that could be.
QString randomFormula(Rng &r, bool extra)
{
    auto row = [&] { return 1 + r.bounded(30); };
    if (extra && r.bounded(5) == 0) {
        const int a = row(), b = row();
        switch (r.bounded(6)) {
        case 0: return QStringLiteral("=SUM(A%1:A%2)").arg(a + 3).arg(a);
        case 1: return QStringLiteral("=SUM(B%1:C%2)+A%3").arg(b + 4).arg(b).arg(a);
        case 2: return QStringLiteral("=SUM(Calc!A%1:A%2)").arg(a + 2).arg(a);
        case 3: return QStringLiteral("=ATAN2(A%1,B%2)").arg(a).arg(b);
        case 4: return QStringLiteral("=A%1+XFE%2").arg(a).arg(b);
        default: return QStringLiteral("=SUM(D%1:A%1)").arg(a);
        }
    }
    auto pair = [&](int *a, int *b) {
        *a = row();
        *b = *a + r.bounded(8);
    };
    int a = 0, b = 0;
    switch (r.bounded(26)) {
    case 0: return QStringLiteral("=A%1+B%2").arg(row()).arg(row());
    case 1: return QStringLiteral("=$A$%1*2").arg(row());
    case 2: pair(&a, &b); return QStringLiteral("=SUM(A%1:A%2)").arg(a).arg(b);
    case 3: pair(&a, &b); return QStringLiteral("=SUM($A%1:B$%2)").arg(a).arg(b);
    case 4: return QStringLiteral("=Calc!A%1+1").arg(row());
    case 5: return QStringLiteral("=Calc!B%1*Extra!A%2").arg(row()).arg(1 + r.bounded(20));
    case 6: pair(&a, &b); return QStringLiteral("=SUM(Data:Extra!A%1:A%2)").arg(a).arg(b);
    case 7: return QStringLiteral("=SUM(Total)+A%1").arg(row());
    case 8: return QStringLiteral("=SUM(B:B)");
    case 9: return QStringLiteral("=SUM(%1:%2)").arg(33 + r.bounded(4)).arg(36 + r.bounded(4));
    case 10: return QStringLiteral("=INDEX(A1:C20,%1,2)").arg(1 + r.bounded(15));
    case 11: return QStringLiteral("=OFFSET(A%1,1,1)").arg(row());
    case 12: return QStringLiteral("=INDIRECT(\"A%1\")").arg(row());
    case 13: return QStringLiteral("=ROW(A%1)").arg(row());
    case 14: return QStringLiteral("=COLUMN(C%1)").arg(row());
    case 15: return QStringLiteral("=ADDRESS(ROW(A%1),COLUMN(B%1))").arg(row());
    case 16: pair(&a, &b); return QStringLiteral("=SUMPRODUCT(ROW(A%1:A%2))").arg(a).arg(b);
    case 17: return QStringLiteral("=SUM(Tbl[Qty])");
    case 18: return QStringLiteral("=ROW()+COLUMN()");
    case 19: pair(&a, &b); return QStringLiteral("=SUM(Extra!B%1:B%2)").arg(a).arg(b);
    case 20: return QStringLiteral("=VLOOKUP(%1,A1:C30,2,FALSE)").arg(1 + r.bounded(20));
    case 21: return QStringLiteral("=A%1&\"x\"").arg(row());
    case 22: return QStringLiteral("=IF(A%1>5,B%1,C%1)").arg(row());
    case 23: return QStringLiteral("=SUM(RowSet)");
    case 24: pair(&a, &b); return QStringLiteral("=MAX(Calc!A%1:A%2)").arg(a).arg(b);
    default: return QStringLiteral("=Data!A%1-Extra!A%2").arg(row()).arg(1 + r.bounded(20));
    }
}

// The weights of the kinds of step (out of 100), in the order of Step::Kind.
const int kWeights[Step::KindCount] = {8, 8, 5, 5, 3, 3, 3, 3, 14, 5, 6, 5, 3, 11, 2, 1, 1, 2, 2, 2, 2, 2, 2, 3};

Step makeStep(Rng &r, bool afterUndo, bool extra)
{
    Step s;
    s.sheet = pickSheet(r);
    // (a redo is only of use right after an undo: then it is likely)
    int x = r.bounded(100);
    Step::Kind kind = Step::Undo;
    if (afterUndo && r.bounded(100) < 55) {
        kind = r.bounded(5) < 4 ? Step::Redo : Step::Undo;
    } else {
        for (int k = 0; k < Step::KindCount; ++k) {
            if (x < kWeights[k]) {
                kind = Step::Kind(k);
                break;
            }
            x -= kWeights[k];
        }
    }
    s.kind = kind;
    auto count = [&] { return r.bounded(2) ? 1 : 1 + r.bounded(3); };
    auto block = [&] {
        s.row1 = pickRow(r);
        s.col1 = pickCol(r);
        s.row2 = s.row1 + r.bounded(3);
        s.col2 = s.col1 + r.bounded(3);
    };
    // a block of the first four columns (the data) or of the others (the formulas)
    auto zoneBlock = [&](int zone, int *row1, int *col1, int *row2, int *col2) {
        *row1 = r.bounded(40);
        *row2 = *row1 + r.bounded(4);
        if (zone == 0) {
            *col1 = r.bounded(4);
            *col2 = std::min(3, *col1 + r.bounded(3));
        } else {
            *col1 = 4 + r.bounded(8);
            *col2 = std::min(11, *col1 + r.bounded(3));
        }
    };
    switch (kind) {
    case Step::InsertRows:
    case Step::DeleteRows:
        s.row1 = pickRow(r);
        s.count = count();
        break;
    case Step::InsertColumns:
    case Step::DeleteColumns:
        s.col1 = pickCol(r);
        s.count = count();
        break;
    case Step::InsertCellsDown:
    case Step::InsertCellsRight:
    case Step::DeleteCellsUp:
    case Step::DeleteCellsLeft: block(); break;
    case Step::Type: {
        s.row1 = r.bounded(45);
        const int what = r.bounded(10);
        if (what < 4) {
            s.col1 = pickCol(r);
            s.text = what < 3 ? QString::number(r.bounded(100)) : QStringLiteral("%1.%2").arg(r.bounded(20)).arg(5);
        } else if (what == 4) {
            s.col1 = pickCol(r);
            s.text = QStringLiteral("txt%1").arg(r.bounded(8));
        } else {
            s.col1 = 4 + r.bounded(8);
            s.text = randomFormula(r, extra);
        }
        break;
    }
    case Step::Fill:
        zoneBlock(r.bounded(2), &s.row1, &s.col1, &s.row2, &s.col2);
        if (r.bounded(2)) s.row2 = s.row1;
        s.option = r.bounded(4);
        s.count = 1 + r.bounded(6);
        break;
    case Step::Paste:
    case Step::Cut: {
        const int zone = r.bounded(2);
        zoneBlock(zone, &s.row1, &s.col1, &s.row2, &s.col2);
        s.toSheet = r.bounded(4) ? s.sheet : pickSheet(r);
        s.toRow = r.bounded(42);
        s.toCol = zone == 0 ? r.bounded(4) : 4 + r.bounded(8);
        if (kind == Step::Paste) s.option = r.bounded(4) ? 0 : r.bounded(4);
        break;
    }
    case Step::Sort:
        s.row1 = 0;
        s.row2 = 3 + r.bounded(27);
        s.col1 = 0;
        s.col2 = 3;
        s.option = r.bounded(4) * 2 + r.bounded(2);
        break;
    case Step::Merge:
        block();
        s.option = r.bounded(3) == 0 ? 1 : 0;
        break;
    case Step::ResizeRows:
        s.row1 = pickRow(r);
        s.row2 = s.row1 + r.bounded(3);
        s.option = r.bounded(3);
        s.count = 1 + r.bounded(5);
        break;
    case Step::ResizeColumns:
        s.col1 = pickCol(r);
        s.col2 = s.col1 + r.bounded(3);
        s.option = r.bounded(3);
        s.count = 1 + r.bounded(5);
        break;
    case Step::SetNote:
        s.row1 = pickRow(r);
        s.col1 = pickCol(r);
        s.text = QStringLiteral("n%1").arg(r.bounded(100));
        break;
    case Step::SetLink: {
        block();
        s.row2 = s.row1;
        s.col2 = s.col1 + r.bounded(2);
        s.option = r.bounded(3);
        s.toSheet = pickSheet(r);
        const QString cell = cellName(quint32(r.bounded(30)), quint32(r.bounded(6)));
        s.text = s.option == 0 ? QStringLiteral("https://example.org/%1").arg(r.bounded(9)) : QStringLiteral("%1!%2").arg(QLatin1String(kSheetNames[s.option == 1 ? s.sheet : s.toSheet]), cell);
        break;
    }
    case Step::DefineName: {
        static const char *const names[4] = {"NmA", "NmB", "NmC", "Total"};
        const int a = r.bounded(25), c = r.bounded(8);
        const int shape = r.bounded(4);
        const QString range = shape == 0 ? QStringLiteral("Data!$%1$%2").arg(columnName(quint32(c)), QString::number(a + 1))
                              : shape == 1 ? QStringLiteral("Calc!$%1$%2:$%3$%4").arg(columnName(quint32(c)), QString::number(a + 1), columnName(quint32(c + 1)), QString::number(a + 5))
                              : shape == 2 ? QStringLiteral("Extra!$%1:$%1").arg(columnName(quint32(c)))
                                           : QStringLiteral("Data!$%1:$%1").arg(QString::number(a + 1));
        s.text = QStringLiteral("%1==%2").arg(QLatin1String(names[r.bounded(4)]), range);
        break;
    }
    case Step::SetPage:
        s.option = r.bounded(4);
        s.row1 = pickRow(r);
        s.col1 = pickCol(r);
        if (s.option == 0) s.row1 = std::max(1, s.row1);   // (a break is before a row or column that is not the first)
        if (s.option == 3) s.col1 = std::max(1, s.col1);
        s.row2 = s.row1 + 2 + r.bounded(10);
        s.col2 = s.col1 + 1 + r.bounded(5);
        break;
    case Step::Undo:
    case Step::Redo:
    case Step::Reopen:
    case Step::Save:
    case Step::KindCount: break;
    }
    return s;
}

// ---- a window with a workbook in it

bool waitFor(const std::function<bool()> &done, int millis)
{
    QElapsedTimer t;
    t.start();
    while (!done()) {
        if (t.elapsed() > millis) return false;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        QThread::usleep(300);
    }
    return true;
}

struct Live {
    QTemporaryDir dir;
    std::unique_ptr<CrunchWindow> win;
    QStringList told;
    QString path;
    qint64 lastEpoch = -1;
    int serial = 0;
    bool saveFinished = false, saveOk = false;
    QString problem;   // what open, settle, or recalculateAll waited for and did not get

    bool open(const QByteArray &bytes, bool visible = true)
    {
        problem.clear();
        path = dir.filePath(QStringLiteral("book%1.xlsx").arg(++serial));
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly)) {
            problem = QStringLiteral("the file %1 could not be written").arg(path);
            return false;
        }
        f.write(bytes);
        f.close();
        win = std::make_unique<CrunchWindow>();
        win->setNotifier([this](const QString &title, const QString &text) { told << title + QStringLiteral(": ") + text; });
        win->resize(1100, 640);
        if (visible) win->show();
        bool first = false, loaded = false, loadedOk = true, done = false;
        const QMetaObject::Connection c1 = QObject::connect(win.get(), &CrunchWindow::firstScreenShown, win.get(), [&] { first = true; });
        const QMetaObject::Connection c2 = QObject::connect(win.get(), &CrunchWindow::loadFinished, win.get(), [&](bool ok) {
            loaded = true;
            loadedOk = ok;
        });
        const QMetaObject::Connection c3 = QObject::connect(win.get(), &CrunchWindow::calculationFinished, win.get(), [&] { done = true; });
        QObject::connect(&win->workbookLoader(), &crunch::WorkbookLoader::calcBatch, win.get(), [this](const std::shared_ptr<crunch::CalcBatch> &b) {
            if (b->last && !b->stopped) lastEpoch = b->epoch;
        });
        QObject::connect(win.get(), &CrunchWindow::saved, win.get(), [this](const QString &, bool ok) {
            saveFinished = true;
            saveOk = ok;
        });
        const bool started = win->openFile(path);
        const bool ok = started && waitFor([&] { return first && loaded && done; }, 60000);
        QObject::disconnect(c1);
        QObject::disconnect(c2);
        QObject::disconnect(c3);
        if (!ok || !loadedOk || !win->session()) {
            problem = !started ? QStringLiteral("the window did not start the open")
                      : !loadedOk && loaded ? QStringLiteral("the load finished with failure")
                      : QStringLiteral("after 60 s: first screen %1, load %2, calculation %3").arg(first ? "shown" : "NOT shown", loaded ? "finished" : "NOT finished", done ? "finished" : "NOT finished");
            if (ok && loadedOk) problem = QStringLiteral("the window has no session");
            return false;
        }
        win->session()->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        win->session()->setToday(QDate(2026, 10, 26));
        return settle();
    }
    // Waits until the worker has worked out what the edits made, and the window has taken what it sent.
    bool settle()
    {
        if (!waitFor([&] { return !win->isCalculating(); }, 60000)) {
            problem = QStringLiteral("the window was still calculating after 60 s");
            return false;
        }
        auto &loader = win->workbookLoader();
        if (!loader.result()) return true;
        QElapsedTimer waited;
        waited.start();
        for (;;) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 2);
            QThread::usleep(500);
            bool done = false;
            if (loader.result()->calcIdle.load()) {
                auto hold = loader.hold();
                done = !hold || !hold->calculation()->isDirty();
            }
            if (done) break;
            if (waited.elapsed() > 60000) {
                problem = QStringLiteral("the worker had not gone idle after 60 s");
                return false;
            }
        }
        QCoreApplication::processEvents();
        return true;
    }
    // Every formula worked out again from nothing.
    bool recalculateAll()
    {
        if (!win->workbookLoader().recalculate()) {
            problem = QStringLiteral("the loader refused a whole calculation");
            return false;
        }
        if (!waitFor([&] { return lastEpoch >= win->workbookLoader().changesMade(); }, 60000)) {
            problem = QStringLiteral("the whole calculation did not finish after 60 s");
            return false;
        }
        return settle();
    }
    crunch::EditSession *session() const { return win->session(); }
};

// ---- the state of a workbook, as lines

QString valueText(const calc::Value &v)
{
    if (v.isNumber()) return QStringLiteral("n:") + QString::number(calc::roundTo15Digits(v.number()), 'g', 15);
    return v.debug();
}

QString sheetNameOf(int sheet) { return QLatin1String(kSheetNames[std::clamp(sheet, 0, 2)]); }

QString rangesText(const QVector<CellRange> &ranges, QChar separator = QLatin1Char(' '))
{
    QStringList out;
    for (const CellRange &r : ranges) out << (r.valid ? rangeName(r) : QStringLiteral("-"));
    return out.join(separator);
}

using State = QMap<QString, QString>;

// What a session holds, as lines (key to text): the formula or constant of every cell and the value it shows, and each thing that follows the cells.
void describe(Live &live, State *out)
{
    crunch::EditSession *s = live.session();
    const Calculation *calc = s->calculation();
    auto put = [&](const QString &key, const QString &text) { (*out)[key] = text; };
    for (int sheet = 0; sheet < calc->sheetCount(); ++sheet) {
        std::vector<std::pair<quint32, quint32>> cells;
        calc->sheet(sheet).cells.forEach([&](quint32 row, quint32 col, const Cell &c) { cells.emplace_back(row, col); });
        for (const auto &cell : cells) {
            const QString text = s->editText(sheet, cell.first, cell.second);
            const calc::Value v = s->valueAt(sheet, cell.first, cell.second);
            if (text.isEmpty() && v.isEmpty()) continue;
            const QString at = QStringLiteral("%1|%2").arg(sheet).arg(cellName(cell.first, cell.second));
            put(QStringLiteral("cell|") + at, text);
            put(QStringLiteral("val|") + at, valueText(v));
            if (text.startsWith(QLatin1Char('=')) && calc->isCircular(sheet, cell.first, cell.second)) put(QStringLiteral("circ|") + at, QStringLiteral("1"));
        }
        const Sheet &model = calc->sheet(sheet);
        QVector<CellRange> merges = calc->mergesOf(sheet);
        for (const CellRange &m : merges) put(QStringLiteral("merge|%1|%2").arg(sheet).arg(rangeName(m)), QString());
        int i = 0;
        for (const CondRule &rule : model.conditionalFormats)
            put(QStringLiteral("cf|%1|%2").arg(sheet).arg(i++), QStringLiteral("%1|%2|%3|%4|p%5").arg(rangesText(rule.ranges, QLatin1Char(',')), rule.formula1, rule.formula2, rule.text).arg(rule.priority));
        i = 0;
        for (const Validation &v : model.validations) put(QStringLiteral("dv|%1|%2").arg(sheet).arg(i++), QStringLiteral("%1|t%2|%3|%4").arg(rangesText(v.ranges, QLatin1Char(',')), QString::number(int(v.type)), v.formula1, v.formula2));
        const AutoFilter &filter = calc->autoFilterOf(sheet);
        if (filter.ref.valid) put(QStringLiteral("filter|%1").arg(sheet), rangeName(filter.ref));
        for (quint32 col = 0; col < 24; ++col) {
            const auto runs = calc->columnRuns(sheet, col, col);
            if (runs.empty()) continue;
            const ColumnSpan &c = runs.front();
            const double standard = calc->sheetFormatOf(sheet).defaultColWidth;
            if (std::abs(c.width - standard) < 0.005 && !c.hidden) continue;
            put(QStringLiteral("col|%1|%2").arg(sheet).arg(col), QStringLiteral("%1%2").arg(c.width, 0, 'f', 2).arg(c.hidden ? QStringLiteral(" hidden") : QString()));
        }
        for (quint32 row = 0; row < 64; ++row) {
            const auto specs = calc->rowSpecsIn(sheet, row, row);
            if (specs.empty()) continue;
            const RowSpec &r = specs.front();
            if (std::abs(r.height - calc->sheetFormatOf(sheet).defaultRowHeight) < 0.005 && !r.hidden) continue;
            put(QStringLiteral("row|%1|%2").arg(sheet).arg(row), QStringLiteral("%1%2").arg(r.height, 0, 'f', 2).arg(r.hidden ? QStringLiteral(" hidden") : QString()));
        }
        const PageSetup &page = s->pages().sheet(sheet);
        QStringList breaks;
        for (const PageBreak &b : page.rowBreaks) breaks << QString::number(b.before);
        put(QStringLiteral("rowbreaks|%1").arg(sheet), breaks.join(QLatin1Char(' ')));
        breaks.clear();
        for (const PageBreak &b : page.colBreaks) breaks << QString::number(b.before);
        put(QStringLiteral("colbreaks|%1").arg(sheet), breaks.join(QLatin1Char(' ')));
        put(QStringLiteral("printarea|%1").arg(sheet), rangesText(page.printArea));
        put(QStringLiteral("titles|%1").arg(sheet),
            QStringLiteral("%1 %2 %3").arg(page.titles.hasRows ? QStringLiteral("%1:%2").arg(page.titles.row1).arg(page.titles.row2) : QStringLiteral("-"),
                                           page.titles.hasCols ? QStringLiteral("%1:%2").arg(page.titles.col1).arg(page.titles.col2) : QStringLiteral("-"), QString()));
        for (const auto &note : s->anchors().notes(sheet)) put(QStringLiteral("note|%1|%2").arg(sheet).arg(cellName(quint32(note.first >> 32), quint32(note.first & 0xFFFFFFFF))), note.second.text);
        QStringList links;
        for (const LinkSpec &l : s->anchors().links(sheet)) links << QStringLiteral("%1>%2%3").arg(rangeName(l.range), l.location, l.target);
        links.sort();
        put(QStringLiteral("links|%1").arg(sheet), links.join(QLatin1Char(' ')));
    }
    const crunch::NameTable &names = s->names();
    for (int id = 0; id < names.slots(); ++id) {
        if (!s->nameListed(id)) continue;
        put(QStringLiteral("name|%1|%2").arg(names.at(id).scope).arg(names.at(id).name.toUpper()), s->nameTextOf(id));
    }
    for (const Table &table : s->workbook()->tables())
        put(QStringLiteral("table|%1").arg(table.name), QStringLiteral("%1 %2 h%3 t%4 %5").arg(table.sheet).arg(rangeName(rangeOf(int(table.row1), int(table.col1), int(table.row2), int(table.col2)))).arg(table.headerRows).arg(table.totalsRows).arg(table.columns.join(QLatin1Char(','))));
    if (crunch::SheetObjects *objects = live.win->objects()) {
        for (int sheet = 0; sheet < calc->sheetCount(); ++sheet) {
            objects->counts(sheet);   // (the drawing is listed)
            for (const int id : objects->idsOf(sheet)) {
                QStringList reads;
                for (const ChartBlock &b : objects->chartReads(sheet, id)) reads << QStringLiteral("%1!%2").arg(b.sheet).arg(rangeName(b.range));
                const DrawingAnchor a = objects->anchorOf(sheet, id);
                put(QStringLiteral("chart|%1|%2").arg(sheet).arg(id),
                    QStringLiteral("%1 | %2:%3-%4:%5").arg(reads.join(QLatin1Char(' '))).arg(a.from.col).arg(a.from.row).arg(a.to.col).arg(a.to.row));
            }
        }
    }
}

quint64 digestOf(const State &state)
{
    quint64 h = 1469598103934665603ull;
    for (auto it = state.constBegin(); it != state.constEnd(); ++it) {
        h = fnv(it.key(), h);
        h = fnv(QByteArrayLiteral("\t"), h);
        h = fnv(it.value(), h);
        h = fnv(QByteArrayLiteral("\n"), h);
    }
    return h;
}

// The lines that differ between what the session that was edited holds (a) and what the file read again holds (b); the first is the one to report.
QStringList differences(const State &a, const State &b, const QSet<QString> &skip)
{
    QStringList out;
    QSet<QString> keys;
    for (auto it = a.constBegin(); it != a.constEnd(); ++it) keys.insert(it.key());
    for (auto it = b.constBegin(); it != b.constEnd(); ++it) keys.insert(it.key());
    QStringList sorted(keys.begin(), keys.end());
    sorted.sort();
    for (const QString &key : sorted) {
        if (skip.contains(key)) continue;
        const bool ha = a.contains(key), hb = b.contains(key);
        if (ha && hb && a.value(key) == b.value(key)) continue;
        // (a cell that is just a number 0 against none is the same cell to everything that reads it)
        out << QStringLiteral("%1: edited [%2] against read again [%3]").arg(key, ha ? a.value(key) : QStringLiteral("none"), hb ? b.value(key) : QStringLiteral("none"));
    }
    return out;
}

// For the tests of the harness: what the file read again holds of one kind of thing is changed (a line is altered, or one is made when there is none), so that the check must say so.
void tamper(State *state, int fault)
{
    static const char *const kinds[16] = {"merge", "cf", "dv", "name", "table", "chart", "note", "links", "cell", "val", "filter", "printarea", "rowbreaks", "colbreaks", "col", "row"};
    const QString prefix = QLatin1String(kinds[std::clamp(fault, 1, 16) - 1]) + QLatin1Char('|');
    for (auto it = state->begin(); it != state->end(); ++it)
        if (it.key().startsWith(prefix)) {
            it.value() += QStringLiteral("!");
            return;
        }
    state->insert(prefix + QStringLiteral("made up"), QStringLiteral("!"));
}

// What the window holds for the formulas against what the calculation has.
QString storeDiffers(Live &live)
{
    const Calculation *calc = live.session()->calculation();
    for (int s = 0; s < calc->sheetCount(); ++s) {
        QString problem;
        // the cells held that are in rows or columns that are deleted, and the cells that are not, are what the store says (kept as the order is edited, and as steps are let go of)
        const CellStore &store = calc->sheet(s).cells;
        qint64 walked = 0;
        store.forEach([&](quint32, quint32, const Cell &) { ++walked; });
        if (walked != store.cellCount()) return QStringLiteral("%1 holds %2 live cells and says %3").arg(sheetNameOf(s)).arg(walked).arg(store.cellCount());
        if (store.deadCountByScan() != store.deadCount()) return QStringLiteral("%1 holds %2 dead cells and says %3").arg(sheetNameOf(s)).arg(store.deadCountByScan()).arg(store.deadCount());
        calc->sheet(s).cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            if (!problem.isEmpty() || !(c.hasFormula() || (c.flags & ArrayMember))) return;
            if (c.hasFormula() && calc->isCircular(s, row, col)) return;
            calc::Value held;
            const calc::Value now = calc->cell(s, row, col);
            if (live.win->values().find(s, c.rowId, c.colId, &held) && valueText(held) != valueText(now))
                problem = QStringLiteral("%1 shows %2 and is %3").arg(placeName(s, int(row), int(col)), valueText(held), valueText(now));
        });
        if (!problem.isEmpty()) return problem;
    }
    return QString();
}

// ---- a second opinion: what rows or columns inserted or deleted do to everything that follows the cells, worked out by code that shares nothing with the engine's (adjustRange, shiftFormula,
// the rewriters of the saver): ranges are renumbered, and the references in formulas are found in their text and renumbered. It is held to the state of the session before and after a step
// that inserts or deletes whole rows or columns (a state that is not worked out here, such as the one after Undo, a cut, or cells shifted, is taken from the session as it is, so it
// is the one step that is checked). Three-D references are positional (an edit of one sheet does not change them), as they are in the engine and in the plan.

struct Plain {
    bool rows = false, insert = false;
    int sheet = 0, at = 0, count = 1;
};

bool plainOf(const Step &st, Plain *e)
{
    switch (st.kind) {
    case Step::InsertRows: *e = {true, true, st.sheet, st.row1, st.count}; return true;
    case Step::DeleteRows: *e = {true, false, st.sheet, st.row1, st.count}; return true;
    case Step::InsertColumns: *e = {false, true, st.sheet, st.col1, st.count}; return true;
    case Step::DeleteColumns: *e = {false, false, st.sheet, st.col1, st.count}; return true;
    default: return false;
    }
}

// A row or column (counted from 0) after the edit; false if it is deleted.
bool indexAfter(const Plain &e, int i, int *out)
{
    if (e.insert) {
        *out = i >= e.at ? i + e.count : i;
        return true;
    }
    if (i < e.at) *out = i;
    else if (i >= e.at + e.count) *out = i - e.count;
    else return false;
    return true;
}

// The rows or columns a to b (both in the span) after the edit: the ends move as cells do, and the ones that are deleted are not in it; false if none is left.
bool spanAfter(const Plain &e, int a, int b, int *na, int *nb)
{
    if (e.insert) {
        *na = a >= e.at ? a + e.count : a;
        *nb = b >= e.at ? b + e.count : b;
        return true;
    }
    *na = a < e.at ? a : (a >= e.at + e.count ? a - e.count : e.at);
    *nb = b < e.at ? b : (b >= e.at + e.count ? b - e.count : e.at - 1);
    return *na <= *nb;
}

// A block of cells of the edited sheet after the edit; false if it is gone.
bool blockAfter(const Plain &e, CellRange *r)
{
    int a = 0, b = 0;
    if (e.rows) {
        if (!spanAfter(e, int(r->row1), int(r->row2), &a, &b)) return false;
        r->row1 = quint32(a);
        r->row2 = quint32(b);
    } else {
        if (!spanAfter(e, int(r->col1), int(r->col2), &a, &b)) return false;
        r->col1 = quint32(a);
        r->col2 = quint32(b);
    }
    return true;
}

int sheetNumber(QString name)
{
    if (name.size() >= 2 && name.startsWith(QLatin1Char('\'')) && name.endsWith(QLatin1Char('\''))) name = name.mid(1, name.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
    for (int i = 0; i < 3; ++i)
        if (name.compare(QLatin1String(kSheetNames[i]), Qt::CaseInsensitive) == 0) return i;
    return -1;
}

bool isWordChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.'); }

int columnNumber(const QString &letters)
{
    int c = 0;
    for (const QChar ch : letters) c = c * 26 + (ch.toUpper().unicode() - 'A' + 1);
    return c - 1;
}

// The references of a formula's text, renumbered for the edit (the text between them is kept as it is); `formulaSheet` is the sheet the formula is on. A reference to cells that are deleted is `#REF!`.
QString textAfter(const QString &text, int formulaSheet, const Plain &e)
{
    QString out;
    const int n = int(text.size());
    int i = 0;
    while (i < n) {
        const QChar c = text[i];
        if (c == QLatin1Char('"')) {   // a text
            int j = i + 1;
            while (j < n && !(text[j] == QLatin1Char('"') && !(j + 1 < n && text[j + 1] == QLatin1Char('"')))) j += (text[j] == QLatin1Char('"')) ? 2 : 1;
            out += text.mid(i, j + 1 - i);
            i = j + 1;
            continue;
        }
        if (c == QLatin1Char('[')) {   // the columns of a table
            int depth = 0, j = i;
            for (; j < n; ++j) {
                if (text[j] == QLatin1Char('[')) ++depth;
                else if (text[j] == QLatin1Char(']') && --depth == 0) break;
            }
            out += text.mid(i, j + 1 - i);
            i = j + 1;
            continue;
        }
        if (i > 0 && (isWordChar(text[i - 1]) || text[i - 1] == QLatin1Char('$') || text[i - 1] == QLatin1Char('!'))) {
            out += c;
            ++i;
            continue;
        }
        // a sheet's name and "!"
        int j = i;
        QString prefix;
        int sheet = formulaSheet;
        bool threeD = false, hasPrefix = false;
        auto name = [&](int *at) {
            int k = *at;
            if (k < n && text[k] == QLatin1Char('\'')) {
                ++k;
                while (k < n && !(text[k] == QLatin1Char('\'') && !(k + 1 < n && text[k + 1] == QLatin1Char('\'')))) k += (text[k] == QLatin1Char('\'')) ? 2 : 1;
                ++k;
            } else {
                while (k < n && isWordChar(text[k])) ++k;
            }
            const QString word = text.mid(*at, k - *at);
            *at = k;
            return word;
        };
        {
            int k = j;
            const QString first = name(&k);
            if (!first.isEmpty() && k < n && (text[k] == QLatin1Char('!') || text[k] == QLatin1Char(':'))) {
                int m = k;
                bool ok = true;
                QString second;
                if (text[m] == QLatin1Char(':')) {
                    ++m;
                    second = name(&m);
                    ok = !second.isEmpty() && m < n && text[m] == QLatin1Char('!') && sheetNumber(first) >= 0 && sheetNumber(second) >= 0;
                    threeD = ok;
                }
                if (ok && m < n && text[m] == QLatin1Char('!') && (sheetNumber(first) >= 0 || threeD)) {
                    hasPrefix = true;
                    sheet = threeD ? -1 : sheetNumber(first);
                    prefix = text.mid(j, m + 1 - j);
                    j = m + 1;
                }
            }
        }
        // the body: a cell or block, whole columns, or whole rows
        struct Part {
            bool colAbs = false, rowAbs = false;
            QString letters;
            int row = -1;
        };
        auto cellPart = [&](int *at, Part *p) {
            int k = *at;
            p->colAbs = k < n && text[k] == QLatin1Char('$');
            if (p->colAbs) ++k;
            const int l0 = k;
            while (k < n && text[k].isLetter() && text[k].unicode() < 128) ++k;
            if (k == l0 || k - l0 > 3 || columnNumber(text.mid(l0, k - l0)) > 16383) return false;   // (a word past XFD is a name)
            p->letters = text.mid(l0, k - l0);
            p->rowAbs = k < n && text[k] == QLatin1Char('$');
            if (p->rowAbs) ++k;
            const int d0 = k;
            while (k < n && text[k].isDigit()) ++k;
            if (k == d0 || k - d0 > 7) return false;
            p->row = text.mid(d0, k - d0).toInt() - 1;
            if (p->row < 0) return false;
            *at = k;
            return true;
        };
        auto columnOnly = [&](int *at, Part *p) {
            int k = *at;
            p->colAbs = k < n && text[k] == QLatin1Char('$');
            if (p->colAbs) ++k;
            const int l0 = k;
            while (k < n && text[k].isLetter() && text[k].unicode() < 128) ++k;
            if (k == l0 || k - l0 > 3 || columnNumber(text.mid(l0, k - l0)) > 16383) return false;   // (a word past XFD is a name)
            p->letters = text.mid(l0, k - l0);
            *at = k;
            return true;
        };
        auto rowOnly = [&](int *at, Part *p) {
            int k = *at;
            p->rowAbs = k < n && text[k] == QLatin1Char('$');
            if (p->rowAbs) ++k;
            const int d0 = k;
            while (k < n && text[k].isDigit()) ++k;
            if (k == d0 || k - d0 > 7) return false;
            p->row = text.mid(d0, k - d0).toInt() - 1;
            if (p->row < 0) return false;
            *at = k;
            return true;
        };
        auto after = [&](int at) { return at >= n || !(isWordChar(text[at]) || text[at] == QLatin1Char('(') || text[at] == QLatin1Char('[') || text[at] == QLatin1Char('!') || text[at] == QLatin1Char('$')); };
        int end = j, kind = -1;
        Part a, b;
        bool two = false;
        {
            int k = j;
            if (cellPart(&k, &a)) {
                int m = k;
                if (m < n && text[m] == QLatin1Char(':') && (++m, cellPart(&m, &b))) {
                    two = true;
                    k = m;
                }
                if (after(k)) {
                    kind = 0;
                    end = k;
                }
            }
            if (kind < 0) {
                int m = j;
                if (columnOnly(&m, &a) && m < n && text[m] == QLatin1Char(':') && (++m, columnOnly(&m, &b)) && after(m)) {
                    kind = 1;
                    end = m;
                    two = true;
                }
            }
            if (kind < 0) {
                int m = j;
                if (rowOnly(&m, &a) && m < n && text[m] == QLatin1Char(':') && (++m, rowOnly(&m, &b)) && after(m)) {
                    kind = 2;
                    end = m;
                    two = true;
                }
            }
        }
        if (kind < 0) {
            out += c;
            ++i;
            continue;
        }
        const QString original = text.mid(i, end - i);
        if (threeD || sheet != e.sheet) {
            out += original;
            i = end;
            continue;
        }
        auto letters = [](int col) { return columnName(quint32(col)); };
        auto dollar = [](bool abs) { return abs ? QStringLiteral("$") : QString(); };
        QString body;
        bool gone = false;
        if (kind == 0) {
            int r1 = a.row, r2 = two ? b.row : a.row, c1 = columnNumber(a.letters), c2 = two ? columnNumber(b.letters) : c1;
            if (!two) {
                int moved = 0;
                if (e.rows) {
                    if (!indexAfter(e, r1, &moved)) gone = true;
                    r1 = r2 = moved;
                } else {
                    if (!indexAfter(e, c1, &moved)) gone = true;
                    c1 = c2 = moved;
                }
            } else {
                CellRange r;
                r.valid = true;
                r.row1 = quint32(std::min(r1, r2));
                r.row2 = quint32(std::max(r1, r2));
                r.col1 = quint32(std::min(c1, c2));
                r.col2 = quint32(std::max(c1, c2));
                // (the ends that were the other way round are written as they were)
                const bool rowsSwapped = r1 > r2, colsSwapped = c1 > c2;
                if (!blockAfter(e, &r)) gone = true;
                r1 = rowsSwapped ? int(r.row2) : int(r.row1);
                r2 = rowsSwapped ? int(r.row1) : int(r.row2);
                c1 = colsSwapped ? int(r.col2) : int(r.col1);
                c2 = colsSwapped ? int(r.col1) : int(r.col2);
            }
            if (!gone) {
                body = dollar(a.colAbs) + letters(c1) + dollar(a.rowAbs) + QString::number(r1 + 1);
                if (two) body += QLatin1Char(':') + dollar(b.colAbs) + letters(c2) + dollar(b.rowAbs) + QString::number(r2 + 1);
            }
        } else if (kind == 1) {
            if (e.rows) {
                body = original.mid(prefix.size());
            } else {
                CellRange r;
                r.valid = true;
                r.row1 = r.row2 = 0;
                r.col1 = quint32(columnNumber(a.letters));
                r.col2 = quint32(columnNumber(b.letters));
                if (!blockAfter(e, &r)) gone = true;
                else body = dollar(a.colAbs) + letters(int(r.col1)) + QLatin1Char(':') + dollar(b.colAbs) + letters(int(r.col2));
            }
        } else {
            if (!e.rows) {
                body = original.mid(prefix.size());
            } else {
                CellRange r;
                r.valid = true;
                r.col1 = r.col2 = 0;
                r.row1 = quint32(a.row);
                r.row2 = quint32(b.row);
                if (!blockAfter(e, &r)) gone = true;
                else body = dollar(a.rowAbs) + QString::number(r.row1 + 1) + QLatin1Char(':') + dollar(b.rowAbs) + QString::number(r.row2 + 1);
            }
        }
        out += gone ? QStringLiteral("#REF!") : (hasPrefix ? prefix : QString()) + body;
        i = end;
    }
    return out;
}

// What the text of a formula is compared as: a block of one cell written as a block is that cell, and a deleted reference is #REF! whatever sheet it was written with.
QString canonical(QString text)
{
    static const QRegularExpression same(QStringLiteral("(\\$?[A-Z]+\\$?[0-9]+):\\1"));
    static const QRegularExpression sheetRef(QStringLiteral("(?:'[^']+'|[A-Za-z0-9_.]+)!#REF!"));
    text.replace(sheetRef, QStringLiteral("#REF!"));
    for (int i = 0; i < 4; ++i) text.replace(same, QStringLiteral("\\1"));
    // a block written back to front is written in order when the engine's longer way of reading a text writes it again (and as it was when the shorter way does): the same cells, so in order here (the "$"
    // of its corners left out)
    static const QRegularExpression block(QStringLiteral("\\$?([A-Z]+)\\$?([0-9]+):\\$?([A-Z]+)\\$?([0-9]+)"));
    QString out;
    int at = 0;
    for (QRegularExpressionMatchIterator it = block.globalMatch(text); it.hasNext();) {
        const QRegularExpressionMatch m = it.next();
        if (m.capturedStart() > 0 && (text[m.capturedStart() - 1].isLetterOrNumber() || text[m.capturedStart() - 1] == QLatin1Char('_') || text[m.capturedStart() - 1] == QLatin1Char('.'))) continue;
        const int c1 = columnNumber(m.captured(1)), c2 = columnNumber(m.captured(3)), r1 = m.captured(2).toInt(), r2 = m.captured(4).toInt();
        if (c1 <= c2 && r1 <= r2) continue;
        out += text.mid(at, m.capturedStart() - at) + columnName(quint32(std::min(c1, c2))) + QString::number(std::min(r1, r2)) + QLatin1Char(':') + columnName(quint32(std::max(c1, c2))) + QString::number(std::max(r1, r2));
        at = m.capturedEnd();
    }
    return out + text.mid(at);
}

QStringList splitRanges(const QString &text, QChar separator)
{
    return text.isEmpty() || text == QLatin1String("-") ? QStringList() : text.split(separator, Qt::SkipEmptyParts);
}

bool rangeOfText(const QString &text, CellRange *r) { return parseRangeRef(text, r) && r->valid; }

// The state the session should be in after an edit of whole rows or columns, for the things worked out here (`modeled` says which keys of the state they are); keys that are "any" (a value that
// the engine may or may not hold) are in `anyOf`.
struct Prediction {
    State state;
    QSet<QString> any;   // keys not to compare
};

QString rangeAfterText(const QString &text, const Plain &e, bool *gone)
{
    CellRange r;
    *gone = false;
    if (!rangeOfText(text, &r)) return text;
    if (!blockAfter(e, &r)) {
        *gone = true;
        return QString();
    }
    return rangeName(r);
}

Prediction predict(const State &before, const Plain &e)
{
    Prediction p;
    State &out = p.state;
    // cells: the keys move, the formulas are renumbered
    for (auto it = before.constBegin(); it != before.constEnd(); ++it) {
        const QStringList k = it.key().split(QLatin1Char('|'));
        const QString kind = k[0];
        if (kind == QLatin1String("cell")) {
            const int sheet = k[1].toInt();
            quint32 row = 0, col = 0;
            parseCellRef(k[2], &row, &col);
            QString text = it.value();
            if (text.startsWith(QLatin1Char('='))) text = QStringLiteral("=") + textAfter(text.mid(1), sheet, e);
            if (sheet == e.sheet) {
                int moved = 0;
                if (!indexAfter(e, e.rows ? int(row) : int(col), &moved)) continue;
                if (e.rows) row = quint32(moved);
                else col = quint32(moved);
            }
            out.insert(QStringLiteral("cell|%1|%2").arg(sheet).arg(cellName(row, col)), text);
        } else if (kind == QLatin1String("merge")) {
            const int sheet = k[1].toInt();
            QString range = k[2];
            if (sheet == e.sheet) {
                bool gone = false;
                range = rangeAfterText(range, e, &gone);
                CellRange r;
                if (gone || !rangeOfText(range, &r) || r.cellCount() <= 1) continue;
            }
            out.insert(QStringLiteral("merge|%1|%2").arg(sheet).arg(range), QString());
        } else if (kind == QLatin1String("cf") || kind == QLatin1String("dv")) {
            // (a rule goes with its cells; its formulas are not worked out here, and are not compared)
            const int sheet = k[1].toInt();
            QStringList parts = it.value().split(QLatin1Char('|'));
            QStringList ranges;
            for (const QString &r : splitRanges(parts[0], QLatin1Char(','))) {
                if (sheet != e.sheet) {
                    ranges << r;
                    continue;
                }
                bool gone = false;
                const QString now = rangeAfterText(r, e, &gone);
                if (!gone) ranges << now;
            }
            if (ranges.isEmpty()) continue;
            out.insert(QStringLiteral("%1|%2|%3").arg(kind, k[1], k[2]), ranges.join(QLatin1Char(',')));
        } else if (kind == QLatin1String("filter")) {
            const int sheet = k[1].toInt();
            QString range = it.value();
            if (sheet == e.sheet) {
                bool gone = false;
                range = rangeAfterText(range, e, &gone);
                if (gone) continue;
            }
            out.insert(it.key(), range);
        } else if (kind == QLatin1String("printarea")) {
            const int sheet = k[1].toInt();
            QStringList ranges;
            for (const QString &r : splitRanges(it.value(), QLatin1Char(' '))) {
                bool gone = false;
                const QString now = sheet == e.sheet ? rangeAfterText(r, e, &gone) : r;
                if (!gone) ranges << now;
            }
            out.insert(it.key(), ranges.join(QLatin1Char(' ')));
        } else if (kind == QLatin1String("titles")) {
            const int sheet = k[1].toInt();
            QString value = it.value();
            if (sheet == e.sheet) {
                const QStringList w = value.split(QLatin1Char(' '));
                auto span = [&](const QString &t, bool rowsPart) {
                    if (t == QLatin1String("-") || rowsPart != e.rows) return t;
                    int a = t.section(QLatin1Char(':'), 0, 0).toInt(), b = t.section(QLatin1Char(':'), 1, 1).toInt(), na = 0, nb = 0;
                    return spanAfter(e, a, b, &na, &nb) ? QStringLiteral("%1:%2").arg(na).arg(nb) : QStringLiteral("-");
                };
                value = span(w.value(0), true) + QLatin1Char(' ') + span(w.value(1), false) + QLatin1Char(' ');
            }
            out.insert(it.key(), value);
        } else if (kind == QLatin1String("rowbreaks") || kind == QLatin1String("colbreaks")) {
            const int sheet = k[1].toInt();
            QStringList list;
            const bool mine = sheet == e.sheet && (kind == QLatin1String("rowbreaks")) == e.rows;
            for (const QString &b : splitRanges(it.value(), QLatin1Char(' '))) {
                if (!mine) {
                    list << b;
                    continue;
                }
                const int v = b.toInt();
                if (e.insert) list << QString::number(v >= e.at ? v + e.count : v);
                else if (v < e.at) list << b;
                else if (v >= e.at + e.count) list << QString::number(v - e.count);
                else p.any.insert(it.key());   // (a break in rows that are deleted: either goes)
            }
            out.insert(it.key(), list.join(QLatin1Char(' ')));
        } else if (kind == QLatin1String("note")) {
            const int sheet = k[1].toInt();
            quint32 row = 0, col = 0;
            parseCellRef(k[2], &row, &col);
            if (sheet == e.sheet) {
                int moved = 0;
                if (!indexAfter(e, e.rows ? int(row) : int(col), &moved)) continue;
                if (e.rows) row = quint32(moved);
                else col = quint32(moved);
            }
            out.insert(QStringLiteral("note|%1|%2").arg(sheet).arg(cellName(row, col)), it.value());
        } else if (kind == QLatin1String("links")) {
            const int sheet = k[1].toInt();
            QStringList list;
            for (const QString &l : splitRanges(it.value(), QLatin1Char(' '))) {
                QString range = l.section(QLatin1Char('>'), 0, 0), rest = l.section(QLatin1Char('>'), 1);
                if (sheet == e.sheet) {
                    bool gone = false;
                    range = rangeAfterText(range, e, &gone);
                    if (gone) continue;
                }
                // (a place follows the cells it names, on whatever sheet the link is)
                if (rest.contains(QLatin1Char('!'))) rest = textAfter(rest, sheet, e);
                list << range + QLatin1Char('>') + rest;
            }
            list.sort();
            out.insert(it.key(), list.join(QLatin1Char(' ')));
        } else if (kind == QLatin1String("table")) {
            const QStringList w = it.value().split(QLatin1Char(' '));
            QString value = it.value();
            if (w.value(0).toInt() == e.sheet) {
                bool gone = false;
                const QString range = rangeAfterText(w.value(1), e, &gone);
                if (gone) continue;
                QString rest = it.value().mid(w.value(0).size() + 1 + w.value(1).size());   // " hN tN columns"
                CellRange was;
                if (!e.rows && rangeOfText(w.value(1), &was) && int(was.col2) >= e.at && (e.insert ? (e.at > int(was.col1) && e.at <= int(was.col2)) : int(was.col1) < e.at + e.count)) {
                    if (e.insert) {
                        p.any.insert(it.key());   // (columns put inside a table are named by the table)
                        for (int c = e.at; c < e.at + e.count; ++c)
                            for (quint32 r = was.row1; r <= was.row2; ++r) {
                                p.any.insert(QStringLiteral("cell|%1|%2").arg(e.sheet).arg(cellName(r, quint32(c))));
                                p.any.insert(QStringLiteral("val|%1|%2").arg(e.sheet).arg(cellName(r, quint32(c))));
                            }
                    } else {
                        // columns deleted from a table are columns of it no more
                        const int sp = int(rest.lastIndexOf(QLatin1Char(' ')));
                        QStringList columns = rest.mid(sp + 1).split(QLatin1Char(','));
                        QStringList kept;
                        for (int c = 0; c < columns.size(); ++c) {
                            const int col = int(was.col1) + c;
                            if (col < e.at || col >= e.at + e.count) kept << columns[c];
                        }
                        rest = rest.left(sp + 1) + kept.join(QLatin1Char(','));
                    }
                }
                value = w.value(0) + QLatin1Char(' ') + range + rest;
                // a row put inside a table's data has the formulas of its calculated columns: not worked out here
                CellRange before;
                if (e.rows && e.insert && rangeOfText(w.value(1), &before) && e.at > int(before.row1) && e.at <= int(before.row2))
                    for (int r = e.at; r < e.at + e.count; ++r)
                        for (quint32 c = before.col1; c <= before.col2; ++c) {
                            p.any.insert(QStringLiteral("cell|%1|%2").arg(e.sheet).arg(cellName(quint32(r), c)));
                            p.any.insert(QStringLiteral("val|%1|%2").arg(e.sheet).arg(cellName(quint32(r), c)));
                        }
            }
            out.insert(it.key(), value);
        } else if (kind == QLatin1String("name")) {
            const int scope = k[1].toInt();
            QString text = it.value();
            if (text.startsWith(QLatin1Char('='))) text = QStringLiteral("=") + textAfter(text.mid(1), std::max(scope, 0), e);
            out.insert(it.key(), text);
        } else if (kind == QLatin1String("chart")) {
            // (the cells it reads; where it is drawn is not worked out here)
            const int sheet = k[1].toInt();
            Q_UNUSED(sheet);
            const QString reads = it.value().section(QStringLiteral(" | "), 0, 0);
            QStringList list;
            for (const QString &b : splitRanges(reads, QLatin1Char(' '))) {
                const int bang = int(b.indexOf(QLatin1Char('!')));
                if (bang < 0 || b.left(bang).toInt() != e.sheet) {
                    list << b;
                    continue;
                }
                bool gone = false;
                const QString now = rangeAfterText(b.mid(bang + 1), e, &gone);
                if (!gone) list << b.left(bang + 1) + now;
            }
            out.insert(it.key(), list.join(QLatin1Char(' ')));
        } else if (kind == QLatin1String("row") || kind == QLatin1String("col")) {
            const int sheet = k[1].toInt();
            int index = k[2].toInt();
            if (sheet == e.sheet && (kind == QLatin1String("row")) == e.rows) {
                int moved = 0;
                if (!indexAfter(e, index, &moved)) continue;
                index = moved;
            }
            out.insert(QStringLiteral("%1|%2|%3").arg(kind, k[1]).arg(index), it.value());
        }
    }
    if (!e.rows && e.insert) {
        // (a column put inside a run of columns that has a width has it too: what the inserted columns have is not worked out here)
        for (int c = e.at; c < e.at + e.count; ++c) p.any.insert(QStringLiteral("col|%1|%2").arg(e.sheet).arg(c));
    }
    return p;
}

// The keys of the state that `predict` works out, as a prefix check.
bool modeled(const QString &key)
{
    static const char *const kinds[] = {"cell|", "merge|", "cf|", "dv|", "filter|", "printarea|", "titles|", "rowbreaks|", "colbreaks|", "note|", "links|", "table|", "name|", "chart|", "row|", "col|"};
    for (const char *k : kinds)
        if (key.startsWith(QLatin1String(k))) return true;
    return false;
}

// What of a state is compared with the prediction: the text of formulas as they are compared (see `canonical`), the ranges of rules, and the cells charts read.
QString comparable(const QString &key, const QString &value)
{
    if (key.startsWith(QLatin1String("cell|")) && value.startsWith(QLatin1Char('='))) return canonical(value);
    if (key.startsWith(QLatin1String("name|")) || key.startsWith(QLatin1String("links|"))) return canonical(value);
    if (key.startsWith(QLatin1String("cf|")) || key.startsWith(QLatin1String("dv|"))) return value.section(QLatin1Char('|'), 0, 0);
    if (key.startsWith(QLatin1String("chart|"))) return value.section(QStringLiteral(" | "), 0, 0);
    if (key.startsWith(QLatin1String("val|"))) return QString();
    return value;
}

QStringList modelDifferences(const Prediction &p, const State &actual)
{
    QStringList out;
    QSet<QString> keys;
    for (auto it = p.state.constBegin(); it != p.state.constEnd(); ++it) keys.insert(it.key());
    for (auto it = actual.constBegin(); it != actual.constEnd(); ++it)
        if (modeled(it.key())) keys.insert(it.key());
    QStringList sorted(keys.begin(), keys.end());
    sorted.sort();
    // (the rules are numbered in order: a rule that is gone makes the ones after it move up, so the lists of a sheet are compared)
    for (const QString &key : sorted) {
        if (p.any.contains(key)) continue;
        const bool want = p.state.contains(key), have = actual.contains(key);
        if (key.startsWith(QLatin1String("cf|")) || key.startsWith(QLatin1String("dv|"))) continue;   // (compared as lists below)
        if (want && have && canonical(comparable(key, p.state.value(key))) == canonical(comparable(key, actual.value(key)))) continue;
        out << QStringLiteral("%1: the model says [%2], the session holds [%3]").arg(key, want ? p.state.value(key) : QStringLiteral("none"), have ? actual.value(key) : QStringLiteral("none"));
    }
    for (const QString &kind : {QStringLiteral("cf"), QStringLiteral("dv")})
        for (int sheet = 0; sheet < 3; ++sheet) {
            QStringList want, have;
            const QString prefix = QStringLiteral("%1|%2|").arg(kind).arg(sheet);
            for (auto it = p.state.constBegin(); it != p.state.constEnd(); ++it)
                if (it.key().startsWith(prefix)) want << it.value();
            for (auto it = actual.constBegin(); it != actual.constEnd(); ++it)
                if (it.key().startsWith(prefix)) have << it.value().section(QLatin1Char('|'), 0, 0);
            std::sort(want.begin(), want.end());
            std::sort(have.begin(), have.end());
            if (want != have) out << QStringLiteral("%1 of sheet %2: the model says [%3], the session holds [%4]").arg(kind).arg(sheet).arg(want.join(QStringLiteral("; ")), have.join(QStringLiteral("; ")));
        }
    return out;
}

// ---- the graph

// A source of cells that writes down what a formula asks of it.
class Recorder : public calc::CellSource {
public:
    explicit Recorder(const Calculation &c) : m_calc(c) {}
    std::vector<calc::CellArea> reads;
    int sheetCount() const override { return m_calc.sheetCount(); }
    int sheetIndex(const QString &name) const override { return m_calc.sheetIndex(name); }
    calc::Value cell(int sheet, quint32 row, quint32 col) const override
    {
        calc::CellArea a;
        a.sheet1 = a.sheet2 = sheet;
        a.row1 = a.row2 = row;
        a.col1 = a.col2 = col;
        const_cast<Recorder *>(this)->reads.push_back(a);
        return m_calc.cell(sheet, row, col);
    }
    void visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const calc::Value &)> &f) const override
    {
        calc::CellArea a;
        a.sheet1 = a.sheet2 = sheet;
        a.row1 = row1;
        a.col1 = col1;
        a.row2 = row2;
        a.col2 = col2;
        const_cast<Recorder *>(this)->reads.push_back(a);
        m_calc.visit(sheet, row1, col1, row2, col2, f);
    }
    void usedExtent(int sheet, quint32 *rows, quint32 *cols) const override { m_calc.usedExtent(sheet, rows, cols); }
    QString definedName(int sheet, const QString &name) const override { return m_calc.definedName(sheet, name); }
    calc::StructuredAnswer structuredRef(const QString &text, int sheet, quint32 row, quint32 col, calc::CellArea *area, calc::ErrorKind *error) const override
    {
        return m_calc.structuredRef(text, sheet, row, col, area, error);
    }
    int rowHiddenKind(int sheet, quint32 row) const override { return m_calc.rowHiddenKind(sheet, row); }
    bool isSubtotal(int sheet, quint32 row, quint32 col) const override { return m_calc.isSubtotal(sheet, row, col); }
    qint64 generation() const override { return m_calc.generation(); }

private:
    const Calculation &m_calc;
};

// What a formula reads, found by working it out again over a Recorder (no evaluation of the session's own is used): the blocks, once each. False for a cell that is no formula the graph follows
// (an array formula or a data table, or a text that is not a formula).
bool readsOfFormula(const Calculation *calc, int sheet, quint32 row, quint32 col, std::vector<calc::CellArea> *reads)
{
    const Calculation::CellContent content = calc->content(sheet, row, col);
    if (!content.isFormula || content.arrayRange.valid || content.dataTable) return false;
    const calc::ParseResult pr = calc::parseFormula(content.formula, calc::ParseOptions());
    if (!pr.program) return false;
    Recorder source(*calc);
    calc::Evaluator evaluator;
    calc::EvalContext ctx;
    ctx.source = &source;
    ctx.sheet = sheet;
    ctx.row = row;
    ctx.col = col;
    evaluator.evaluate(*pr.program, ctx);
    *reads = source.reads;
    std::sort(reads->begin(), reads->end());
    reads->erase(std::unique(reads->begin(), reads->end()), reads->end());
    return true;
}

// The values that are not compared, as keys of the state: those of formulas in a circle (which have no value to agree on) and of every formula that reads one of them, directly or through others.
QSet<QString> circleTaint(Live &live)
{
    const Calculation *calc = live.session()->calculation();
    struct Cellular {
        int sheet;
        quint32 row, col;
        std::vector<calc::CellArea> reads;
        bool tainted = false, known = false;
    };
    std::vector<Cellular> all;
    bool any = false;
    for (int s = 0; s < calc->sheetCount(); ++s)
        calc->sheet(s).cells.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) {
            if (!c.hasFormula()) return;
            Cellular f{s, cellRow, cellCol, {}, calc->isCircular(s, cellRow, cellCol), false};
            any = any || f.tainted;
            all.push_back(std::move(f));
        });
    QSet<QString> keys;
    if (!any) return keys;
    for (Cellular &f : all) f.known = readsOfFormula(calc, f.sheet, f.row, f.col, &f.reads);
    for (bool changed = true; changed;) {
        changed = false;
        for (Cellular &f : all) {
            if (f.tainted || !f.known) continue;
            for (const calc::CellArea &a : f.reads) {
                bool hit = false;
                for (const Cellular &g : all)
                    if (g.tainted && g.sheet >= a.sheet1 && g.sheet <= a.sheet2 && g.row >= a.row1 && g.row <= a.row2 && g.col >= a.col1 && g.col <= a.col2) {
                        hit = true;
                        break;
                    }
                if (hit) {
                    f.tainted = true;
                    changed = true;
                    break;
                }
            }
        }
    }
    for (const Cellular &f : all)
        if (f.tainted) keys.insert(QStringLiteral("val|%1|%2").arg(f.sheet).arg(cellName(f.row, f.col)));
    return keys;
}

// Every formula is worked out again over a Recorder, and what it read must be what the graph says the formula reads: asking the graph what reads that cell (or block) names the formula.
// Returns the first formula that reads what the graph does not hold for it. `formulas` and `reads` count what was looked at.
QString graphMisses(Live &live, qint64 *formulas, qint64 *reads, bool fakeRead)
{
    const Calculation *calc = live.session()->calculation();
    for (int s = 0; s < calc->sheetCount(); ++s) {
        std::vector<std::pair<quint32, quint32>> cells;
        calc->sheet(s).cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            if (c.hasFormula()) cells.emplace_back(row, col);
        });
        for (const auto &cell : cells) {
            const Calculation::CellContent content = calc->content(s, cell.first, cell.second);
            std::vector<calc::CellArea> areas;
            const quint32 node = calc->nodeAt(s, cell.first, cell.second);
            if (node == 0xFFFFFFFFu || !readsOfFormula(calc, s, cell.first, cell.second, &areas)) continue;
            ++*formulas;
            if (fakeRead) {
                calc::CellArea far;
                far.sheet1 = far.sheet2 = 0;
                far.row1 = far.row2 = 5000;
                far.col1 = far.col2 = 100;
                areas.push_back(far);
                fakeRead = false;
            }
            for (const calc::CellArea &a : areas) {
                ++*reads;
                std::vector<quint32> readers;
                calc->readersOf(a, &readers);
                if (std::find(readers.begin(), readers.end(), node) == readers.end())
                    return QStringLiteral("%1 (=%2) reads %3!%4:%5, which the graph does not say it reads")
                        .arg(placeName(s, int(cell.first), int(cell.second)), content.formula, sheetNameOf(a.sheet1), cellName(a.row1, a.col1), cellName(a.row2, a.col2));
            }
        }
    }
    return QString();
}

// ---- one step

crunch::EditSession::Outcome execute(Live &live, const Step &st, bool *didSomething)
{
    using Session = crunch::EditSession;
    Session *s = live.session();
    Session::Outcome o;
    *didSomething = false;
    const CellRange block = rangeOf(st.row1, st.col1, st.row2, st.col2);
    switch (st.kind) {
    case Step::InsertRows: o = s->insertRows(st.sheet, quint32(st.row1), quint32(st.count)); break;
    case Step::DeleteRows: o = s->deleteRows(st.sheet, quint32(st.row1), quint32(st.count)); break;
    case Step::InsertColumns: o = s->insertColumns(st.sheet, quint32(st.col1), quint32(st.count)); break;
    case Step::DeleteColumns: o = s->deleteColumns(st.sheet, quint32(st.col1), quint32(st.count)); break;
    case Step::InsertCellsDown: o = s->insertCells(st.sheet, block, true); break;
    case Step::InsertCellsRight: o = s->insertCells(st.sheet, block, false); break;
    case Step::DeleteCellsUp: o = s->deleteCells(st.sheet, block, true); break;
    case Step::DeleteCellsLeft: o = s->deleteCells(st.sheet, block, false); break;
    case Step::Type: o = s->type(st.sheet, quint32(st.row1), quint32(st.col1), st.text); break;
    case Step::Fill: {
        static const Session::FillDirection dirs[4] = {Session::FillDirection::Down, Session::FillDirection::Right, Session::FillDirection::Up, Session::FillDirection::Left};
        o = s->fill(st.sheet, block, dirs[st.option & 3], quint32(st.count));
        break;
    }
    case Step::Paste: {
        Session::PasteOptions options;
        static const Session::PasteOptions::What what[4] = {Session::PasteOptions::What::All, Session::PasteOptions::What::Formulas, Session::PasteOptions::What::Values, Session::PasteOptions::What::Formats};
        options.what = what[st.option & 3];
        const CellRange target = rangeOf(st.toRow, st.toCol, st.toRow, st.toCol);
        const crunch::ClipData clip = s->copyCells(st.sheet, block, false);
        o = clip.valid() ? s->paste(st.toSheet, target, clip, options) : Session::Outcome{false, QStringLiteral("nothing to copy")};
        break;
    }
    case Step::Cut: o = s->moveCells(st.sheet, block, st.toSheet, quint32(st.toRow), quint32(st.toCol)); break;
    case Step::Sort: {
        crunch::SortSpec spec;
        crunch::SortLevel level;
        level.key = st.option / 2;
        level.descending = st.option & 1;
        spec.levels.append(level);
        o = s->sortRange(st.sheet, block, spec);
        break;
    }
    case Step::Undo:
        if (s->canUndo() && s->undoStack()->canUndo()) {
            s->undoStack()->undo();
            *didSomething = true;
        }
        return o;
    case Step::Redo:
        if (s->undoStack()->canRedo()) {
            s->undoStack()->redo();
            *didSomething = true;
        }
        return o;
    case Step::Save:
        if (live.win->isModified()) {
            live.saveFinished = false;
            if (!live.win->save()) return Session::Outcome{false, QStringLiteral("the save did not start")};
            if (!waitFor([&] { return live.saveFinished; }, 60000) || !live.saveOk) return Session::Outcome{false, QStringLiteral("the save failed")};
            *didSomething = true;
        }
        return o;
    case Step::Merge: o = s->mergeCells(st.sheet, block, st.option ? Session::MergeKind::Unmerge : Session::MergeKind::Merge, true); break;
    case Step::ResizeRows:
    case Step::ResizeColumns: {
        Session::SizeChange change;
        if (st.option == 0) {
            change.setSize = true;
            change.size = st.kind == Step::ResizeRows ? 12.0 + st.count * 6 : 6.0 + st.count * 3;
        } else {
            change.setHidden = true;
            change.hidden = st.option == 1;
        }
        o = st.kind == Step::ResizeRows ? s->resizeRows(st.sheet, quint32(st.row1), quint32(st.row2), change, QStringLiteral("rows")) : s->resizeColumns(st.sheet, quint32(st.col1), quint32(st.col2), change, QStringLiteral("columns"));
        break;
    }
    case Step::SetNote: o = s->setNote(st.sheet, quint32(st.row1), quint32(st.col1), st.text, QStringLiteral("a"), false, QStringLiteral("note")); break;
    case Step::SetLink: {
        LinkSpec link;
        link.range = block;
        if (st.option == 0) link.target = st.text;
        else link.location = st.text;
        o = s->setLink(st.sheet, block, link, QStringLiteral("link"));
        break;
    }
    case Step::DefineName: {
        Session::NameDraft draft;
        draft.name = st.text.section(QStringLiteral("=="), 0, 0);
        draft.text = QStringLiteral("=") + st.text.section(QStringLiteral("=="), 1);
        const int existing = s->names().find(-1, draft.name);
        o = existing >= 0 ? s->changeName(existing, draft) : s->defineName(draft);
        break;
    }
    case Step::SetPage: {
        PageSetup page = s->pages().sheet(st.sheet);
        if (st.option == 0 || st.option == 3) {
            PageBreak b;
            b.before = quint32(st.option == 0 ? st.row1 : st.col1);
            b.max = st.option == 0 ? 16383 : 1048575;
            QVector<PageBreak> &list = st.option == 0 ? page.rowBreaks : page.colBreaks;
            bool there = false;
            for (const PageBreak &x : list) there = there || x.before == b.before;
            if (!there) list.append(b);
        } else if (st.option == 1) {
            page.printArea = {block};
        } else {
            page.printArea.clear();
        }
        o = s->setPageSetup(st.sheet, page, QStringLiteral("page"));
        break;
    }
    case Step::Reopen:
    case Step::KindCount: return o;
    }
    if (o.confirm) o.ok = false;
    if (o.ok) *didSomething = true;
    s->finishFills();
    return o;
}

} // namespace

QString modelRewrite(const QString &formula, int formulaSheet, const Step &edit)
{
    Plain e;
    return plainOf(edit, &e) ? textAfter(formula, formulaSheet, e) : formula;
}

std::vector<Step> generate(quint32 seed, int count, bool extra)
{
    Rng r(seed);
    std::vector<Step> steps;
    for (int i = 0; i < count; ++i) steps.push_back(makeStep(r, !steps.empty() && (steps.back().kind == Step::Undo || steps.back().kind == Step::Redo), extra));
    return steps;
}

Result run(const std::vector<Step> &steps, const Options &options)
{
    // (every workbook the run opens, the one that is edited and the ones read again, holds its formulas as the options say)
    struct FrameScope {
        bool was;
        FrameScope(bool on, bool off) : was(jp::office::sheet::Calculation::idFrameDefault())
        {
            if (on) jp::office::sheet::Calculation::setIdFrameDefault(true);
            if (off) jp::office::sheet::Calculation::setIdFrameDefault(false);
        }
        ~FrameScope() { jp::office::sheet::Calculation::setIdFrameDefault(was); }
    } frameScope(options.idFrame, options.forceOff);
    Result result;
    auto fail = [&](int step, const QString &kind, const QString &what) {
        if (result.failure.ok()) {
            result.failure.failed = true;
            result.failure.step = step + 1;
            result.failure.kind = kind;
            result.failure.what = what;
        }
    };
    auto live = std::make_unique<Live>();
    if (!live->open(baseWorkbook())) {
        fail(0, QStringLiteral("open"), QStringLiteral("the base workbook did not open: ") + live->problem);
        return result;
    }
    State before;
    describe(*live, &before);
    // the checks of the state the session is in now (after the step `index`, from 0; -1 is the workbook before any step)
    auto checkNow = [&](int index) {
        ++result.checks;
        const QString stale = storeDiffers(*live);
        if (!stale.isEmpty()) {
            fail(index, QStringLiteral("store"), stale);
            return;
        }
        const QSet<QString> tainted = circleTaint(*live);
        if (!tainted.isEmpty()) ++result.circles;
        QByteArray bytes;
        QString why;
        if (!live->win->snapshotBytesForTest(&bytes, &why)) {
            fail(index, QStringLiteral("open"), QStringLiteral("the workbook could not be saved: ") + why);
            return;
        }
        QElapsedTimer timer;
        timer.start();
        Live again;
        if (!again.open(bytes, false) || !again.recalculateAll()) {
            fail(index, QStringLiteral("open"), QStringLiteral("the saved workbook did not open and work out: ") + again.problem);
            return;
        }
        result.msOpen += timer.restart();
        State now, reread;
        describe(*live, &now);
        describe(again, &reread);
        result.msDescribe += timer.restart();
        if (options.fault > 0 && options.fault < 100 && index >= 0) tamper(&reread, options.fault);
        const QStringList diff = differences(now, reread, tainted);
        if (!diff.isEmpty()) {
            fail(index, diff.first().section(QLatin1Char('|'), 0, 0), QStringLiteral("%1 difference(s) after the file is saved and read again: %2").arg(diff.size()).arg(diff.mid(0, 6).join(QStringLiteral("; "))));
            return;
        }
        if (options.graph) {
            timer.restart();
            const QString miss = graphMisses(*live, &result.graphFormulas, &result.graphReads, options.fault == 100 && index >= 0);
            result.msGraph += timer.elapsed();
            if (!miss.isEmpty()) fail(index, QStringLiteral("graph"), miss);
        }
    };
    if (options.check) checkNow(-1);
    QElapsedTimer stepClock;
    for (size_t i = 0; i < steps.size() && result.failure.ok(); ++i) {
        const Step &st = steps[i];
        stepClock.start();
        const int index = int(i);
        result.stepsText.push_back(st.words());
        bool didSomething = false;
        if (st.kind == Step::Reopen) {
            QByteArray bytes;
            QString why;
            if (!live->win->snapshotBytesForTest(&bytes, &why)) {
                fail(index, QStringLiteral("open"), QStringLiteral("the workbook could not be saved: ") + why);
                break;
            }
            auto next = std::make_unique<Live>();
            if (!next->open(bytes)) {
                fail(index, QStringLiteral("open"), QStringLiteral("the saved workbook did not open: ") + next->problem);
                break;
            }
            live = std::move(next);
            didSomething = true;
        } else {
            const qint64 movedBefore = live->session()->calculation()->editStats().cellsMoved;
            const crunch::EditSession::Outcome o = execute(*live, st, &didSomething);
            if (didSomething && (st.kind == Step::InsertRows || st.kind == Step::DeleteRows || st.kind == Step::InsertColumns || st.kind == Step::DeleteColumns)) {
                ++result.lineEdits;
                result.lineEditCellsMoved += live->session()->calculation()->editStats().cellsMoved - movedBefore;
            }
            if (!o.ok) result.stepsText.back() += QStringLiteral("  [refused: %1]").arg(o.problem.left(60));
            else if (!didSomething && (st.kind == Step::Undo || st.kind == Step::Redo || st.kind == Step::Save)) result.stepsText.back() += QStringLiteral("  [nothing to do]");
            if (!o.ok && st.kind != Step::Undo && st.kind != Step::Redo && st.kind != Step::Save && st.kind != Step::Reopen) ++result.refused;
            else if (st.kind == Step::Undo && didSomething) ++result.undone;
            else if (st.kind == Step::Redo && didSomething) ++result.redone;
            if (!live->settle()) {
                fail(index, QStringLiteral("settle"), QStringLiteral("the worker did not finish"));
                break;
            }
            if (!o.ok && st.kind != Step::Undo && st.kind != Step::Redo && st.kind != Step::Save) {
                // a step the workbook did not take changed nothing
                State after;
                describe(*live, &after);
                if (digestOf(after) != digestOf(before)) {
                    const QStringList diff = differences(before, after, QSet<QString>());
                    fail(index, QStringLiteral("refused"), QStringLiteral("the step was refused (%1) but changed the workbook: %2").arg(o.problem, diff.mid(0, 4).join(QStringLiteral("; "))));
                    break;
                }
            }
        }
        result.msSteps += stepClock.elapsed();
        State now;
        describe(*live, &now);
        result.digests.push_back(digestOf(now));
        // a plain insert or delete of rows or columns that was taken: what follows the cells is what the second opinion says
        Plain plainEdit;
        if (options.check && didSomething && plainOf(st, &plainEdit)) {
            ++result.modelChecks;
            Prediction said = predict(before, plainEdit);
            if (options.fault == 101) said.state.insert(QStringLiteral("merge|0|A1:A2"), QString());
            const QStringList diff = modelDifferences(said, now);
            if (!diff.isEmpty()) {
                fail(index, QStringLiteral("model"), QStringLiteral("%1 difference(s) from what the model of the edit says: %2").arg(diff.size()).arg(diff.mid(0, 6).join(QStringLiteral("; "))));
                break;
            }
        }
        before = now;
        const bool last = i + 1 == steps.size();
        if (options.check && (last || (index + 1) % std::max(1, options.checkEvery) == 0)) checkNow(index);
    }
    {
        State last;
        describe(*live, &last);
        for (auto it = last.constBegin(); it != last.constEnd(); ++it) result.finalState += it.key() + QStringLiteral(" = ") + it.value() + QLatin1Char('\n');
    }
    if (result.failure.ok()) {
        QByteArray bytes;
        QString why;
        if (live->win->snapshotBytesForTest(&bytes, &why)) {
            QMap<QString, QByteArray> parts;
            jp::readZip(bytes, parts);
            quint64 all = 1469598103934665603ull;
            for (auto it = parts.constBegin(); it != parts.constEnd(); ++it) {
                const quint64 h = fnv(it.value());
                if (it.value().size() < 4096) result.smallParts[it.key()] = it.value();
                all = fnv(it.key(), all);
                all = fnv(QByteArray::number(h, 16), all);
                result.parts << QStringLiteral("%1 %2").arg(it.key()).arg(h, 16, 16, QLatin1Char('0'));
            }
            result.partsDigest = all;
        }
    }
    return result;
}

std::vector<Step> shrink(const std::vector<Step> &steps, const Options &options, const Failure &failure)
{
    std::vector<Step> current(steps.begin(), steps.begin() + std::min<size_t>(steps.size(), size_t(failure.step)));
    auto stillFails = [&](const std::vector<Step> &candidate) {
        Options o = options;
        const Result r = run(candidate, o);
        return !r.failure.ok() && r.failure.kind == failure.kind;
    };
    // (chunks, from half the steps to one: a step that is dropped may make a later one fail alike, so the passes go on until none can be dropped)
    for (size_t chunk = std::max<size_t>(1, current.size() / 2); chunk >= 1;) {
        bool dropped = false;
        for (size_t at = 0; at + chunk <= current.size();) {
            std::vector<Step> candidate;
            candidate.insert(candidate.end(), current.begin(), current.begin() + std::ptrdiff_t(at));
            candidate.insert(candidate.end(), current.begin() + std::ptrdiff_t(at + chunk), current.end());
            if (!candidate.empty() && stillFails(candidate)) {
                current = candidate;
                dropped = true;
            } else {
                at += chunk;
            }
        }
        if (chunk == 1 && !dropped) break;
        if (chunk > 1) chunk = std::max<size_t>(1, chunk / 2);
    }
    return current;
}

QString report(quint32 seed, const std::vector<Step> &steps, const Failure &failure)
{
    QString out = QStringLiteral("seed %1: after step %2 (%3): %4\n").arg(seed).arg(failure.step).arg(failure.kind, failure.what);
    out += QStringLiteral("the steps (jcrunchtool model-replay reads these lines):\n");
    for (size_t i = 0; i < steps.size(); ++i) out += QStringLiteral("  %1  # %2\n").arg(steps[i].line(), steps[i].words());
    return out;
}

QString goldenText(const std::vector<quint32> &seeds, int stepCount)
{
    QString out = QStringLiteral("# The state of the workbook after each step of the model check (docs/jeffcrunch-rowids.md, Round 0), as 64 bits, for a fixed set of seeds, made by\n"
                                 "# jcrunchtool model-golden. A change that keeps what the engine means leaves this file as it is; a change that does not says why in the commit that changes it.\n"
                                 "# seed <n> <steps>: one digest for each step; parts <n>: the parts a save writes after the last step; part <n> <name> <digest>: each of them.\n");
    Options options;
    options.check = false;
    for (const quint32 seed : seeds) {
        const Result r = run(generate(seed, stepCount), options);
        QStringList digests;
        for (const quint64 d : r.digests) digests << QStringLiteral("%1").arg(d, 16, 16, QLatin1Char('0'));
        out += QStringLiteral("seed %1 %2\n").arg(seed).arg(stepCount);
        for (int i = 0; i < digests.size(); i += 8) out += QStringLiteral(" ") + digests.mid(i, 8).join(QLatin1Char(' ')) + QLatin1Char('\n');
        out += QStringLiteral("parts %1 %2\n").arg(seed).arg(r.partsDigest, 16, 16, QLatin1Char('0'));
        for (const QString &part : r.parts) out += QStringLiteral("part %1 %2\n").arg(seed).arg(part);   // (name and digest: a difference names its part)
    }
    return out;
}

int command(const QStringList &args)
{
    QTextStream out(stdout);
    QTextStream err(stderr);
    const QLocale saved = QLocale();
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    auto finish = [&](int code) {
        QLocale::setDefault(saved);
        return code;
    };
    if (args.size() >= 3 && args[0] == QLatin1String("model-check")) {
        // model-check <seeds> <steps> [first seed] [every=n] [nograph] [idframe] [wide] [noshrink]
        const int seeds = args[1].toInt(), stepCount = args[2].toInt();
        const quint32 first = args.size() > 3 && !args[3].contains(QLatin1Char('=')) && args[3] != QLatin1String("nograph") && args[3] != QLatin1String("idframe") && args[3] != QLatin1String("wide") && args[3] != QLatin1String("noshrink") ? args[3].toUInt() : 1;
        Options options;
        bool shrinkIt = true, wide = false;
        for (const QString &a : args.mid(3)) {
            if (a == QLatin1String("nograph")) options.graph = false;
            else if (a == QLatin1String("idframe")) options.idFrame = true;
            else if (a == QLatin1String("wide")) wide = true;
            else if (a == QLatin1String("noshrink")) shrinkIt = false;
            else if (a.startsWith(QLatin1String("every="))) options.checkEvery = std::max(1, a.mid(6).toInt());
        }
        int failed = 0;
        qint64 refused = 0, undone = 0, redone = 0, circles = 0, steps = 0, formulas = 0, reads = 0, modelled = 0, msSteps = 0, msOpen = 0, msDescribe = 0, msGraph = 0;
        QElapsedTimer clock;
        clock.start();
        for (quint32 seed = first; seed < first + quint32(seeds); ++seed) {
            const std::vector<Step> list = generate(seed, stepCount, wide);
            const Result r = run(list, options);
            steps += qint64(r.digests.size());
            refused += r.refused;
            undone += r.undone;
            redone += r.redone;
            circles += r.circles;
            modelled += r.modelChecks;
            msSteps += r.msSteps;
            msOpen += r.msOpen;
            msDescribe += r.msDescribe;
            msGraph += r.msGraph;
            formulas += r.graphFormulas;
            reads += r.graphReads;
            if (!r.failure.ok()) {
                ++failed;
                out << "FAILED " << seed << ": " << r.failure.kind << ": " << r.failure.what << "\n";
                out.flush();
                const std::vector<Step> small = shrinkIt ? shrink(list, options, r.failure) : std::vector<Step>(list.begin(), list.begin() + std::min<size_t>(list.size(), size_t(r.failure.step)));
                Options again = options;
                const Result rr = run(small, again);
                out << report(seed, small, rr.failure.ok() ? r.failure : rr.failure);
                out.flush();
            }
        }
        out << "model-check: " << seeds << " seeds of " << stepCount << " steps from seed " << first << ": " << failed << " failed; " << steps << " steps, " << refused << " refused, " << undone << " undone, " << redone
            << " redone, " << circles << " with a circle; " << modelled << " edits held to the model; the graph check worked out " << formulas << " formulas that read " << reads << " blocks; " << QString::number(double(clock.elapsed()) / 1000.0, 'f', 1) << " s (the steps " << msSteps / 1000 << " s, opening the saved workbooks " << msOpen / 1000 << " s, describing " << msDescribe / 1000 << " s, the graph check " << msGraph / 1000 << " s)\n";
        return finish(failed ? 1 : 0);
    }
    if (args.size() >= 3 && args[0] == QLatin1String("model-golden")) {
        // model-golden <steps> <seed> [<seed> ...]: the golden file's text on stdout
        std::vector<quint32> seeds;
        for (const QString &a : args.mid(2)) seeds.push_back(a.toUInt());
        out << goldenText(seeds, args[1].toInt());
        return finish(0);
    }
    if (args.size() >= 2 && args[0] == QLatin1String("model-replay")) {
        QFile f(args[1]);
        if (!f.open(QIODevice::ReadOnly)) {
            err << "model-replay: cannot read " << args[1] << "\n";
            return finish(2);
        }
        std::vector<Step> list;
        for (const QString &line : QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'))) {
            Step s;
            if (Step::fromLine(line.section(QLatin1Char('#'), 0, 0), &s)) list.push_back(s);
        }
        Options options;
        const Result r = run(list, options);
        if (r.failure.ok()) out << "model-replay: " << list.size() << " steps, no failure\n";
        else out << report(0, list, r.failure);
        return finish(r.failure.ok() ? 0 : 1);
    }
    if (args.size() == 2 && args[0] == QLatin1String("model-book")) {
        QFile f(args[1]);
        if (!f.open(QIODevice::WriteOnly)) return finish(2);
        f.write(baseWorkbook());
        return finish(0);
    }
    if ((args.size() == 3 || args.size() == 2) && args[0] == QLatin1String("model-dump")) {
        std::vector<Step> list;
        if (args.size() == 2) {
            QFile f(args[1]);
            if (!f.open(QIODevice::ReadOnly)) return finish(2);
            for (const QString &line : QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'))) {
                Step s;
                if (Step::fromLine(line.section(QLatin1Char('#'), 0, 0), &s)) list.push_back(s);
            }
        } else {
            list = generate(args[1].toUInt(), args[2].toInt());
        }
        Options options;
        options.check = false;
        const Result r = run(list, options);
        for (const QString &t : r.stepsText) out << t << "\n";
        out << r.finalState;
        return finish(0);
    }
    err << "usage: jcrunchtool model-check <seeds> <steps> [first seed] [every=n] [nograph] [idframe] [wide] [noshrink] | model-golden <steps> <seed>... | model-replay <file> | model-dump <seed> <steps>|<file> | model-book <out.xlsx>\n";
    return finish(2);
}

} // namespace jp::structmodel

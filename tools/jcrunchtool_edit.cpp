// The editing part of jcrunchtool (docs/jeffcrunch.md, wave 4b): the yardstick for saving, and how fast an edit is on the screen.
//   jcrunchtool roundtrip-one <file.xlsx>   opens the workbook as the window does (read whole, every formula worked out), and
//                                           (1) saves it untouched, reads the new file, and compares it with the old: the parts (bytes), the cells, formulas, and formats;
//                                           (2) makes one edit in a cell (a number read by formulas if there is one) the way the window does, lets the worker work out what depends on it, saves,
//                                           reads the new file, and compares: the cell has what was typed, every formula that depends on it has its new result, nothing else differs, and the
//                                           parts that were not changed are the old file's, byte for byte.
//                                           One line of tab-separated fields: path, exit (0 done, 1 not opened, 2 opened in part (the window does not change or save it)), seconds, peak MB, then
//                                           for the untouched save: parts, parts the same, cells the same (1 or 0), differences; for the edit: the cell, formulas that depend on it, formulas whose
//                                           result changed, differences that are unexpected, parts not meant to change, of them the same, whether the cell is right, differences; and the
//                                           milliseconds each took.
//                                           (3) then an edit script on the same open workbook (a row inserted, a column inserted, a block pasted below the cells, a fill, a row deleted, a column
//                                           deleted: those the workbook allows), saved and read back, and compared with the model the window has: every cell (what it holds, its format, and
//                                           what it shows), the merged ranges, and the parts that were not meant to change; the fields after the edit's: the steps made and refused, the cells
//                                           compared and right, the differences, the parts not meant to change and the same, and the milliseconds.
//   jcrunchtool roundtrip <keys.txt> <out-dir> [jobs]   runs roundtrip-one on each workbook listed, each in a child process that has 300 seconds and 6 GB, and writes <out-dir>/roundtrip.tsv; prints the
//                                           summary (tools/crunch_roundtrip.py does the work)
//   jcrunchtool edit-bench <file.xlsx> [edits=200]   opens the workbook in the model of the window and makes that many edits, each in a cell a few rows apart, as a person typing; for each, the
//                                           time the window's thread takes to put the cell in (and the time it waits for the worker to let go), to draw it again, and the time the worker takes to
//                                           the first values and to work out all that depends: the middle, the 95th percentile, the 99th, and the longest of each, by the thread's own processor time
//                                           where there is one and by the wall's besides
//   jcrunchtool edit-bench-make [rows=100000] [cols=20] [readers=0] [edits=200]   the same on a workbook made here: numbers in rows and columns, a formula in the last column of each row that reads the
//                                           first, and, with readers, a cell that that many formulas read

#include "core/editsession.h"
#include "core/loader.h"
#include "core/threadtime.h"
#include "grid/gridview.h"
#include "grid/valuestore.h"
#include "io/zip.h"
#include "io/zipindex.h"
#include "office/package.h"
#include "sheet/xlsxsave.h"

#include "app/crunchwindow.h"
#include "grid/gridview.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QKeyEvent>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>
#include <QTextStream>
#include <QThread>
#include <QTimer>

#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <unordered_set>

using namespace jp;
using namespace jp::office;
using namespace jp::office::sheet;

namespace {

QTextStream &out()
{
    static QTextStream s(stdout);
    return s;
}

QTextStream &err()
{
    static QTextStream s(stderr);
    return s;
}

double peakMegabytes()
{
    QFile status(QStringLiteral("/proc/self/status"));
    if (!status.open(QIODevice::ReadOnly)) return -1;
    for (const QByteArray &line : status.readAll().split('\n'))
        if (line.startsWith("VmHWM:")) return line.mid(6).trimmed().split(' ').value(0).toDouble() / 1024.0;
    return -1;
}

double percentile(std::vector<double> v, double p)
{
    if (v.empty()) return 0;
    std::sort(v.begin(), v.end());
    return v[std::min(v.size() - 1, size_t(std::floor(p * double(v.size()))))];
}

// A workbook opened and calculated the way the window does it, driven by hand: the loader on its worker, the batches put in a store and told to the session.
struct Opened {
    crunch::WorkbookLoader loader;
    grid::ValueStore values;
    std::unique_ptr<crunch::EditSession> session;
    bool finished = false, ok = false, lastSeen = false;
    qint64 lastEpoch = 0;
    qint64 firstBatchMicrosAfter = 0;
    QString message;
    QMetaObject::Connection c1, c2;
    Opened()
    {
        c1 = QObject::connect(&loader, &crunch::WorkbookLoader::finished, &loader, [this](bool good, const QString &m) {
            finished = true;
            ok = good;
            message = m;
        });
        c2 = QObject::connect(&loader, &crunch::WorkbookLoader::calcBatch, &loader, [this](const std::shared_ptr<crunch::CalcBatch> &b) {
            if (trackChanges)
                for (const grid::ValueStore::Item &it : b->items) {
                    jp::calc::Value old;
                    const bool had = values.find(it.sheet, it.row, it.col, &old);
                    const bool same = had && ((old.isNumber() && it.value.isNumber() && jp::calc::sameTo15Digits(old.number(), it.value.number())) || old == it.value);
                    if (!same) changedByEdit.insert(crunch::EditSession::keyOf(it.sheet, it.row, it.col));
                }
            if (session) session->noteResults(b->items);   // (before the store has them)
            values.apply(b->items);
            ++batches;
            if (!b->items.empty() && firstValuesAt < 0) firstValuesAt = clock.nsecsElapsed();
            if (b->last && !b->stopped) {
                lastSeen = true;
                lastEpoch = b->epoch;
                lastAt = clock.nsecsElapsed();
            }
        });
    }
    ~Opened() { loader.cancel(); }
    bool wait(const std::function<bool()> &done, int millis)
    {
        QElapsedTimer t;
        t.start();
        while (!done() && t.elapsed() < millis) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
            QThread::usleep(200);
        }
        return done();
    }
    bool open(const QString &path, int millis = 280000)
    {
        loader.start(path);
        if (!wait([&] { return finished; }, millis) || !ok) return false;
        session = std::make_unique<crunch::EditSession>(&loader, loader.result(), &values, nullptr);
        return wait([&] { return lastSeen; }, millis);
    }
    // Waits until all the changes made are worked out.
    bool settle(int millis = 280000) { return wait([&] { return lastSeen && lastEpoch >= loader.changesMade(); }, millis); }
    void beginEdit()
    {
        lastSeen = false;
        firstValuesAt = lastAt = -1;
        clock.restart();
    }
    qint64 batches = 0;
    qint64 firstValuesAt = -1, lastAt = -1;
    QElapsedTimer clock;
    // From the edit on: the formulas whose result the worker changed (by the store's value before it, if it had one).
    bool trackChanges = false;
    std::unordered_set<quint64> changedByEdit;
};

// ---- comparing two reads of a workbook

quint64 mix(quint64 h, quint64 v)
{
    h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    return h;
}

quint64 hashCell(const Sheet &s, const SharedStrings &shared, const Cell &c)
{
    quint64 h = 1469598103934665603ULL;
    h = mix(h, quint64(c.type));
    quint64 bits = 0;
    std::memcpy(&bits, &c.number, sizeof bits);
    if (c.type != CellType::SharedString && c.type != CellType::InlineString && c.type != CellType::FormulaString) h = mix(h, bits);
    h = mix(h, c.style);
    h = mix(h, quint64(c.flags & ~Tainted));
    const QString text = s.text(c, shared);
    h = mix(h, quint64(qHash(text)));
    if (c.hasFormula()) h = mix(h, quint64(qHash(s.formulaText(c))));
    return h;
}

bool storedIs(const Sheet &s, const SharedStrings &shared, const Cell &c, const jp::calc::Value &v)
{
    using jp::calc::Value;
    switch (c.type) {
    case CellType::Empty: return v.isEmpty() || (v.isNumber() && v.number() == 0);
    case CellType::Number:
    case CellType::Date: return (v.isNumber() && jp::calc::sameTo15Digits(v.number(), c.number)) || (v.isEmpty() && c.number == 0);
    case CellType::SharedString:
    case CellType::InlineString:
    case CellType::FormulaString: return v.isText() && v.text() == s.text(c, shared);
    case CellType::Bool: return v.isBool() && v.boolean() == (c.number != 0);
    case CellType::Error: return v.isError() && int(v.errorKind()) == int(c.number);
    }
    return false;
}

bool sameStored(const Sheet &sa, const SharedStrings &ha, const Cell &a, const Sheet &sb, const SharedStrings &hb, const Cell &b)
{
    if (a.type != b.type) {
        // (a number and a date are one thing to a person)
        const bool numeric = (a.type == CellType::Number || a.type == CellType::Date) && (b.type == CellType::Number || b.type == CellType::Date);
        const bool text = (a.type == CellType::SharedString || a.type == CellType::InlineString || a.type == CellType::FormulaString)
            && (b.type == CellType::SharedString || b.type == CellType::InlineString || b.type == CellType::FormulaString);
        if (!numeric && !text) return false;
    }
    switch (a.type) {
    case CellType::SharedString:
    case CellType::InlineString:
    case CellType::FormulaString: return sa.text(a, ha) == sb.text(b, hb);
    case CellType::Empty: return true;
    default: return a.number == b.number || jp::calc::sameTo15Digits(a.number, b.number);
    }
}

// Whether the cell is what was typed: the number, or (in a cell whose format is Text) the text of it.
bool isTyped(const Cell &c, const Sheet &s, const SharedStrings &shared, double value, const QString &typed)
{
    if (c.type == CellType::Number) return c.number == value;
    if (c.type == CellType::SharedString || c.type == CellType::InlineString || c.type == CellType::FormulaString) return s.text(c, shared) == typed;
    return false;
}

struct Report {
    QStringList differences;   // the kinds, once each
    void add(const QString &what)
    {
        if (!differences.contains(what) && differences.size() < 6) differences << what;
    }
};

// The zip entries, by name: their bytes as they lie in the file.
QMap<QString, QByteArray> rawEntries(const QByteArray &zip)
{
    QMap<QString, QByteArray> out;
    QVector<ZipEntry> dir;
    if (!readZipDirectory(zip, &dir)) return out;
    for (const ZipEntry &e : dir) out.insert(e.name, zip.mid(int(e.dataAt), int(e.csize)));
    return out;
}

QStringList entryNames(const QByteArray &zip)
{
    QStringList out;
    QVector<ZipEntry> dir;
    if (readZipDirectory(zip, &dir))
        for (const ZipEntry &e : dir) out << e.name;
    return out;
}

// The opened workbook of some bytes.
struct Read {
    QByteArray bytes;
    Package package;
    Workbook book;
    bool ok = false;
    QString why;
    explicit Read(const QByteArray &data) : bytes(data) { ok = package.readLazy(bytes, &why) && book.open(package, &why); }
};

QString fixed(double v) { return QString::number(v, 'f', 1); }

// ---- the edit script of the yardstick

bool sameShown(const jp::calc::Value &a, const jp::calc::Value &b)
{
    if (a.isNumber() && b.isNumber()) return jp::calc::sameTo15Digits(a.number(), b.number());
    if ((a.isEmpty() && b.isNumber() && b.number() == 0) || (b.isEmpty() && a.isNumber() && a.number() == 0)) return true;
    return a == b;
}

struct ScriptResult {
    int done = 0, refused = 0;
    qint64 compared = 0, right = 0;
    int parts = 0, partsSame = 0;
    double ms = 0;
    Report differences;
    bool ran = false;
};

// The model the window has, made again from the bytes of the file saved: the cells are compared one by one.
void compareModels(const Calculation *live, const Calculation *saved, ScriptResult *r)
{
    auto empty = [](const Calculation::CellContent &c) { return !c.exists || (!c.isFormula && c.value.isEmpty() && c.style == 0); };
    bool circular = false;
    for (int s = 0; s < live->sheetCount() && !circular; ++s)
        live->sheet(s).cells.forEach([&](const Cell &c) { circular = circular || live->isCircular(s, c.row, c.col); });
    if (live->sheetCount() != saved->sheetCount()) {
        r->differences.add(QStringLiteral("sheets"));
        return;
    }
    for (int s = 0; s < live->sheetCount(); ++s) {
        auto at = [&](quint32 row, quint32 col) {
            const Calculation::CellContent x = live->content(s, row, col), y = saved->content(s, row, col);
            if (empty(x) && empty(y)) return;
            ++r->compared;
            // (a cell of an array holds the result the file had, which is no part of what it means: what it shows is compared)
            const bool member = !x.editable && !x.isFormula;
            if (empty(x) != empty(y) || x.isFormula != y.isFormula || x.formula != y.formula || (!member && !sameShown(x.value, y.value)) || x.style != y.style) {
                r->differences.add(empty(x) != empty(y) ? QStringLiteral("a cell is there or not") : x.isFormula != y.isFormula || x.formula != y.formula ? QStringLiteral("a formula differs")
                                   : x.style != y.style ? QStringLiteral("a format differs") : QStringLiteral("a value differs"));
                return;
            }
            if (!circular && !sameShown(live->cell(s, row, col), saved->cell(s, row, col))) {
                r->differences.add(QStringLiteral("a result differs"));
                return;
            }
            ++r->right;
        };
        live->sheet(s).cells.forEach([&](const Cell &c) { at(c.row, c.col); });
        saved->sheet(s).cells.forEach([&](const Cell &c) {
            if (!live->sheet(s).cells.find(c.row, c.col)) at(c.row, c.col);
        });
        QVector<CellRange> ma = live->mergesOf(s), mb = saved->mergesOf(s);
        auto less = [](const CellRange &p, const CellRange &q) { return p.row1 != q.row1 ? p.row1 < q.row1 : p.col1 < q.col1; };
        std::sort(ma.begin(), ma.end(), less);
        std::sort(mb.begin(), mb.end(), less);
        ++r->compared;
        if (ma == mb) ++r->right;
        else r->differences.add(QStringLiteral("merged ranges differ"));
    }
}

// An edit script on the open workbook, saved and read back.
void runScript(Opened &w, const QByteArray &original, ScriptResult *r)
{
    QElapsedTimer timer;
    timer.start();
    crunch::EditSession *session = w.session.get();
    Calculation *calc = w.loader.result()->calc.get();
    const WorkbookInfo &info = w.loader.result()->book->info();
    int sheet = -1;
    for (int si = 0; si < int(info.sheets.size()) && sheet < 0; ++si)
        if (info.sheets[si].kind == SheetKind::Worksheet && calc->sheet(si).cells.cellCount() > 0 && !calc->sheet(si).isProtected) sheet = si;
    if (sheet < 0) return;
    r->ran = true;
    const quint32 rows = std::max<quint32>(2, calc->sheet(sheet).extentRows), cols = std::max<quint32>(2, calc->sheet(sheet).extentCols);
    auto step = [&](const crunch::EditSession::Outcome &o) {
        if (o.ok) ++r->done;
        else ++r->refused;
        w.settle();
    };
    auto block = [&](quint32 r1, quint32 c1, quint32 r2, quint32 c2) {
        CellRange b;
        b.valid = true;
        b.row1 = r1;
        b.col1 = c1;
        b.row2 = r2;
        b.col2 = c2;
        return b;
    };
    step(session->insertRows(sheet, rows / 2, 1));
    step(session->insertColumns(sheet, cols / 2, 1));
    const quint32 r2 = std::min<quint32>(rows - 1, 4), c2 = std::min<quint32>(cols - 1, 4);
    // a block copied from the top left and pasted below the cells; the first number of it filled down
    crunch::ClipData clip = session->copyCells(sheet, block(0, 0, r2, c2), false);
    const quint32 below = std::min<quint32>(calc->sheet(sheet).extentRows + 2, kMaxRows - 20);
    if (clip.valid()) step(session->paste(sheet, block(below, 0, below, 0), clip, crunch::EditSession::PasteOptions()));
    step(session->fill(sheet, block(below, 0, below, 0), crunch::EditSession::FillDirection::Down, 3));
    step(session->deleteRows(sheet, std::max<quint32>(1, rows / 3), 1));
    step(session->deleteColumns(sheet, std::max<quint32>(1, cols / 3), 1));
    // saved, and read again
    crunch::EditSession::Snapshot snap;
    QByteArray bytes;
    QString why;
    SaveReport report;
    if (!session->takeSnapshot(&snap, &why)) {
        r->differences.add(QStringLiteral("snapshot: ") + why);
        return;
    }
    if (!saveXlsx(*w.loader.result()->book, snap.changes, &bytes, &why, &report)) {
        r->differences.add(QStringLiteral("save failed: ") + why);
        return;
    }
    const QMap<QString, QByteArray> a = rawEntries(original), b = rawEntries(bytes);
    QSet<QString> changed;
    for (const QString &n : report.changedParts) changed.insert(n);
    for (auto it = a.constBegin(); it != a.constEnd(); ++it) {
        if (changed.contains(it.key()) || it.key().endsWith(QLatin1String("calcChain.xml"))) continue;
        ++r->parts;
        if (b.contains(it.key()) && b.value(it.key()) == it.value()) ++r->partsSame;
        else r->differences.add(QStringLiteral("a part that was not meant to change differs"));
    }
    Read after(bytes);
    if (!after.ok) {
        r->differences.add(QStringLiteral("the saved file does not open: ") + after.why);
        return;
    }
    CalcOptions options;
    options.keepApart = true;
    options.workOutWhatHasNoResult = true;
    Calculation saved(after.book, options);
    if (!saved.load()) {
        r->differences.add(QStringLiteral("the saved file does not load"));
        return;
    }
    while (!saved.recalculate().finished) {}
    compareModels(calc, &saved, r);
    r->ms = double(timer.nsecsElapsed()) / 1e6;
}

int roundtripOne(const QString &path)
{
    QElapsedTimer total;
    total.start();
    QByteArray original;
    QString why;
    if (!readWholeFile(path, &original, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    Read before(original);
    if (!before.ok) {
        err() << path << ": " << before.why << "\n";
        return 1;
    }
    // ---- (1) untouched
    QElapsedTimer t1;
    t1.start();
    QByteArray savedBytes;
    SaveReport report;
    QString saveWhy;
    int partsTotal = 0, partsSame = 0;
    bool cellsSame = true;
    Report untouched;
    if (!saveXlsx(before.book, SaveChanges(), &savedBytes, &saveWhy, &report)) {
        untouched.add(QStringLiteral("save failed: ") + saveWhy);
        cellsSame = false;
    } else {
        const QMap<QString, QByteArray> a = rawEntries(original), b = rawEntries(savedBytes);
        partsTotal = int(a.size());
        for (auto it = a.constBegin(); it != a.constEnd(); ++it)
            if (b.value(it.key()) == it.value() && b.contains(it.key())) ++partsSame;
            else untouched.add(QStringLiteral("part differs"));
        if (entryNames(original) != entryNames(savedBytes)) untouched.add(QStringLiteral("entries or their order differ"));
        Read after(savedBytes);
        if (!after.ok) {
            untouched.add(QStringLiteral("the saved file does not open: ") + after.why);
            cellsSame = false;
        } else {
            const WorkbookInfo &ia = before.book.info(), &ib = after.book.info();
            if (ia.sheets.size() != ib.sheets.size()) untouched.add(QStringLiteral("sheets"));
            for (int i = 0; i < std::min(ia.sheets.size(), ib.sheets.size()); ++i) {
                if (ia.sheets[i].name != ib.sheets[i].name || ia.sheets[i].state != ib.sheets[i].state) untouched.add(QStringLiteral("sheet names"));
                Sheet sa, sb;
                before.book.readSheet(i, &sa);
                after.book.readSheet(i, &sb);
                if (sa.cells.cellCount() != sb.cells.cellCount() || !sa.error.isEmpty() != !sb.error.isEmpty()) {
                    untouched.add(QStringLiteral("cell counts"));
                    cellsSame = false;
                    continue;
                }
                std::vector<const Cell *> pa, pb;
                pa.reserve(size_t(sa.cells.cellCount()));
                pb.reserve(size_t(sb.cells.cellCount()));
                sa.cells.forEach([&](const Cell &c) { pa.push_back(&c); });
                sb.cells.forEach([&](const Cell &c) { pb.push_back(&c); });
                for (size_t k = 0; k < pa.size() && k < pb.size(); ++k) {
                    if (pa[k]->row != pb[k]->row || pa[k]->col != pb[k]->col || hashCell(sa, before.book.sharedStrings(), *pa[k]) != hashCell(sb, after.book.sharedStrings(), *pb[k])) {
                        untouched.add(QStringLiteral("a cell differs"));
                        cellsSame = false;
                        break;
                    }
                }
            }
            if (before.book.styles().formats.size() != after.book.styles().formats.size() || before.book.styles().numFmts != after.book.styles().numFmts) {
                untouched.add(QStringLiteral("styles"));
                cellsSame = false;
            }
            if (before.book.sharedStrings().strings != after.book.sharedStrings().strings) {
                untouched.add(QStringLiteral("shared strings"));
                cellsSame = false;
            }
        }
    }
    const double untouchedMs = double(t1.nsecsElapsed()) / 1e6;

    // ---- (2) one edit
    Opened w;
    QString cellName_, editWhy, typedText;
    qint64 dependents = 0, resultsChanged = 0, unexpected = 0;
    int editParts = 0, editPartsSame = 0;
    bool cellRight = false;
    Report edited;
    double editMs = 0;
    int exit = 0;
    const bool opened = w.open(path);
    if (!opened) {
        err() << path << ": " << (w.message.isEmpty() ? QStringLiteral("not worked out in time") : w.message) << "\n";
        return w.finished && !w.ok ? 1 : 2;
    }
    if (!w.loader.result()->problem.isEmpty()) exit = 2;   // (a sheet in part: the window does not change or save it)
    if (exit == 0) {
        QElapsedTimer t2;
        t2.start();
        Calculation *calc = w.loader.result()->calc.get();
        const int sheetCount = int(w.loader.result()->book->info().sheets.size());
        int sheet = -1;
        quint32 row = 0, col = 0;
        // a number that formulas read, the first there is in the first sheet that has one (else any number, else the cell A1)
        for (int pass = 0; pass < 2 && sheet < 0; ++pass)
            for (int si = 0; si < sheetCount && sheet < 0; ++si) {
                if (w.loader.result()->book->info().sheets[si].kind != SheetKind::Worksheet) continue;
                qint64 looked = 0;
                calc->sheet(si).cells.forEach([&](const Cell &c) {
                    if (sheet >= 0 || ++looked > 3000000) return;
                    if (c.type != CellType::Number || c.hasFormula() || (c.flags & (ArrayMember | ArrayFormula))) return;
                    if (!w.session->canEditCell(si, c.row, c.col)) return;   // (a locked cell of a protected sheet is not changed)
                    if (pass == 0) {
                        std::vector<quint32> readers;
                        calc->graph().dependentsOfCell(si, c.row, c.col, &readers);
                        if (readers.empty()) return;
                    }
                    sheet = si;
                    row = c.row;
                    col = c.col;
                });
            }
        if (sheet < 0)
            for (int si = 0; si < sheetCount && sheet < 0; ++si)
                if (w.loader.result()->book->info().sheets[si].kind == SheetKind::Worksheet && w.session->canEditCell(si, 0, 0)) sheet = si;
        if (sheet < 0) {
            cellRight = true;   // (a workbook of chart sheets has no cell to change, nor has one whose sheets are protected and every cell locked)
            cellName_ = QStringLiteral("(no cell that can be changed)");
        } else {
            cellName_ = w.loader.result()->book->info().sheets[sheet].name + QLatin1Char('!') + cellName(row, col);
            const Calculation::CellContent old = calc->content(sheet, row, col);
            const double newValue = old.value.isNumber() ? old.value.number() + 1 : 12345;
            std::vector<quint32> readers;
            calc->graph().dependentsOfCell(sheet, row, col, &readers);
            dependents = qint64(readers.size());
            typedText = QString::number(newValue, 'g', 17);
            w.session->setLocale(QLocale::c());
            w.trackChanges = true;
            w.beginEdit();
            const crunch::EditSession::Outcome o = w.session->type(sheet, row, col, typedText);
            if (!o.ok) {
                edited.add(QStringLiteral("edit refused: ") + o.problem);
            } else if (!w.settle()) {
                edited.add(QStringLiteral("the worker did not finish"));
            } else {
                crunch::EditSession::Snapshot snap;
                QByteArray bytes;
                if (!w.session->takeSnapshot(&snap, &editWhy)) {
                    edited.add(QStringLiteral("snapshot: ") + editWhy);
                } else if (!saveXlsx(*w.loader.result()->book, snap.changes, &bytes, &editWhy, &report)) {
                    edited.add(QStringLiteral("save failed: ") + editWhy);
                } else {
                    resultsChanged = qint64(snap.results.size());
                    const QMap<QString, QByteArray> a = rawEntries(original), b = rawEntries(bytes);
                    QSet<QString> changed;
                    for (const QString &n : report.changedParts) changed.insert(n);
                    if (report.calcChainDropped)
                        for (auto it = a.constBegin(); it != a.constEnd(); ++it)
                            if (it.key().endsWith(QLatin1String("calcChain.xml"))) changed.insert(it.key());
                    for (auto it = a.constBegin(); it != a.constEnd(); ++it) {
                        if (changed.contains(it.key())) continue;
                        ++editParts;
                        if (b.contains(it.key()) && b.value(it.key()) == it.value()) ++editPartsSame;
                        else edited.add(QStringLiteral("an untouched part differs"));
                    }
                    const QString keepAs = qEnvironmentVariable("JC_ROUNDTRIP_KEEP");
                    if (!keepAs.isEmpty()) {
                        QFile kept(keepAs);
                        if (kept.open(QIODevice::WriteOnly)) kept.write(bytes);
                    }
                    Read after(bytes);
                    if (!after.ok) {
                        edited.add(QStringLiteral("the saved file does not open: ") + after.why);
                    } else {
                        for (int si = 0; si < sheetCount; ++si) {
                            Sheet sa, sb;
                            before.book.readSheet(si, &sa);
                            after.book.readSheet(si, &sb);
                            std::vector<const Cell *> pa, pb;
                            sa.cells.forEach([&](const Cell &c) { pa.push_back(&c); });
                            sb.cells.forEach([&](const Cell &c) { pb.push_back(&c); });
                            size_t i = 0, j = 0;
                            auto key = [](const Cell *c) { return (quint64(c->row) << 16) | c->col; };
                            const SharedStrings &ha = before.book.sharedStrings(), &hb = after.book.sharedStrings();
                            const quint64 editedKey = (quint64(row) << 16) | col;
                            while (i < pa.size() || j < pb.size()) {
                                const Cell *a1 = i < pa.size() ? pa[i] : nullptr, *b1 = j < pb.size() ? pb[j] : nullptr;
                                if (a1 && b1 && key(a1) == key(b1)) {
                                    const bool isEdited = si == sheet && key(a1) == editedKey;
                                    const Cell *model = calc->sheet(si).cells.find(b1->row, b1->col);
                                    const bool arrayMember = model && (model->flags & ArrayMember);
                                    if (isEdited) {
                                        cellRight = isTyped(*b1, sb, hb, newValue, typedText);
                                    } else if (b1->hasFormula() || arrayMember) {
                                        // a formula's result: the engine's now if the edit changed it, else the file's as it was
                                        const jp::calc::Value ours = calc->cell(si, b1->row, b1->col);
                                        if (w.changedByEdit.count(crunch::EditSession::keyOf(si, b1->row, b1->col))) {
                                            if (!storedIs(sb, hb, *b1, ours)) {
                                                ++unexpected;
                                                edited.add(QStringLiteral("a result is not the engine's"));
                                            }
                                        } else if (!sameStored(sa, ha, *a1, sb, hb, *b1)) {
                                            ++unexpected;
                                            edited.add(QStringLiteral("a result changed that the edit did not change"));
                                        }
                                        if (a1->style != b1->style || a1->hasFormula() != b1->hasFormula() || (a1->hasFormula() && sa.formulaText(*a1) != sb.formulaText(*b1))) {
                                            ++unexpected;
                                            edited.add(QStringLiteral("a formula or format changed"));
                                        }
                                    } else if (hashCell(sa, ha, *a1) != hashCell(sb, hb, *b1)) {
                                        ++unexpected;
                                        edited.add(QStringLiteral("a constant changed"));
                                    }
                                    ++i;
                                    ++j;
                                } else if (a1 && (!b1 || key(a1) < key(b1))) {
                                    ++unexpected;
                                    edited.add(QStringLiteral("a cell went"));
                                    ++i;
                                } else {
                                    if (!(si == sheet && key(b1) == editedKey)) {
                                        ++unexpected;
                                        edited.add(QStringLiteral("a cell came"));
                                    } else {
                                        cellRight = isTyped(*b1, sb, hb, newValue, typedText);
                                    }
                                    ++j;
                                }
                            }
                        }
                    }
                }
            }
        }
        editMs = double(t2.nsecsElapsed()) / 1e6;
    }
    ScriptResult script;
    if (exit == 0) {
        const qint64 cells = [&] {
            qint64 n = 0;
            for (int si = 0; si < int(w.loader.result()->book->info().sheets.size()); ++si) n += w.loader.result()->calc->sheet(si).cells.cellCount();
            return n;
        }();
        if (cells <= 6000000) runScript(w, original, &script);
        else script.differences.add(QStringLiteral("the script is not run on a workbook of more than 6,000,000 cells"));
    }
    if (!qEnvironmentVariableIsEmpty("JC_ROUNDTRIP_KEEP") && !cellRight) err() << "the edited cell is not right: " << cellName_ << "\n";
    const bool cellsOk = cellsSame && untouched.differences.isEmpty() && partsSame == partsTotal;
    out() << path << '\t' << exit << '\t' << fixed(double(total.nsecsElapsed()) / 1e9) << '\t' << fixed(peakMegabytes()) << '\t' << partsTotal << '\t' << partsSame << '\t' << (cellsOk ? 1 : 0) << '\t'
          << untouched.differences.join(QLatin1String("; ")) << '\t' << cellName_ << '\t' << dependents << '\t' << resultsChanged << '\t' << unexpected << '\t' << editParts << '\t' << editPartsSame << '\t'
          << (cellRight ? 1 : 0) << '\t' << edited.differences.join(QLatin1String("; ")) << '\t' << fixed(untouchedMs) << '\t' << fixed(editMs) << '\t' << script.done << '\t' << script.refused << '\t'
          << script.compared << '\t' << script.right << '\t' << script.parts << '\t' << script.partsSame << '\t' << script.differences.differences.join(QLatin1String("; ")) << '\t' << fixed(script.ms) << "\n";
    out().flush();
    return exit;
}

int roundtripSweep(const QString &keys, const QString &outDir, const QString &jobs)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::ForwardedChannels);
    QStringList args = {QStringLiteral(JCRUNCH_TOOLS_DIR "/crunch_roundtrip.py"), QCoreApplication::applicationFilePath(), keys, outDir};
    if (!jobs.isEmpty()) args << jobs;
    p.start(QStringLiteral("python3"), args);
    p.waitForFinished(-1);
    return p.exitCode();
}

} // namespace

// ---- how fast an edit is on the screen

namespace {

// A workbook made here: `rows` by `cols` numbers, a formula in the column after them that adds the row up, and, with `readers`, a cell Z1 (the column 26) that that many formulas read.
QByteArray benchWorkbook(int rows, int cols, int readers)
{
    QString xml;
    xml.reserve(qsizetype(rows) * cols * 28);
    const QString last = columnName(quint32(cols - 1)), sumColumn = columnName(quint32(cols));
    for (int r = 1; r <= std::max(rows, readers + 1); ++r) {
        const QString n = QString::number(r);
        xml += QStringLiteral("<row r=\"") + n + QStringLiteral("\">");
        if (r <= rows) {
            for (int c = 0; c < cols; ++c) xml += QStringLiteral("<c r=\"") + columnName(quint32(c)) + n + QStringLiteral("\"><v>") + QString::number((r * 7 + c * 13) % 1000) + QStringLiteral("</v></c>");
            xml += QStringLiteral("<c r=\"") + sumColumn + n + QStringLiteral("\"><f>SUM(A") + n + QLatin1Char(':') + last + n + QStringLiteral(")</f><v>0</v></c>");
        }
        if (r == 1 && readers > 0) xml += QStringLiteral("<c r=\"Z1\"><v>1</v></c>");
        if (r <= readers) xml += QStringLiteral("<c r=\"AB") + n + QStringLiteral("\"><f>$Z$1+") + n + QStringLiteral("</f><v>0</v></c>");
        xml += QStringLiteral("</row>");
    }
    const QByteArray sheet = ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheetData>" + xml + "</sheetData></worksheet>").toUtf8();
    PackageWriter w;
    const QString relBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/xl/workbook.xml"), QByteArray(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    w.addRelationship(QStringLiteral("/"), relBase + "officeDocument", QStringLiteral("/xl/workbook.xml"));
    w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), sheet, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    const QString id = w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "worksheet", QStringLiteral("/xl/worksheets/sheet1.xml"));
    const QByteArray workbook = ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"" + relBase.chopped(1)
                                 + "\"><sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"" + id + "\"/></sheets></workbook>").toUtf8();
    w.addPart(QStringLiteral("/xl/workbook.xml"), workbook, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    return w.finish();
}

void printStat(const char *what, const std::vector<double> &v)
{
    out() << "  " << what << ": middle " << QString::number(percentile(v, 0.5), 'f', 3) << " ms, 95th " << QString::number(percentile(v, 0.95), 'f', 3) << " ms, 99th " << QString::number(percentile(v, 0.99), 'f', 3)
          << " ms, longest " << QString::number(percentile(v, 1.0), 'f', 3) << " ms\n";
}

// Rows and columns inserted and deleted near the top, in the middle, and at the end of the cells, and a block of 100,000 cells pasted: for each, what the window's thread takes (its own processor
// time) from the call to the sheet drawn again (the sheet's picture is made again from the cells, and the values the window holds are moved), the worker's processor time for what must be
// worked out again, and the wall until it has.
void structureBench(CrunchWindow &w, int sheet)
{
    crunch::EditSession *session = w.session();
    const Calculation *calc = w.calculation();
    grid::GridView *v = w.view();
    if (!session || !calc) return;
    qint64 lastEpoch = -1;
    QObject::connect(&w.workbookLoader(), &crunch::WorkbookLoader::calcBatch, &w, [&](const std::shared_ptr<crunch::CalcBatch> &b) {
        if (b->last && !b->stopped) lastEpoch = b->epoch;
    });
    const quint32 usedRows = std::max<quint32>(20, calc->sheet(sheet).extentRows), usedCols = std::max<quint32>(2, calc->sheet(sheet).extentCols);
    std::vector<double> cpu, wallv;
    auto timed = [&](const QString &what, const std::function<crunch::EditSession::Outcome()> &fn) {
        const qint64 workerBefore = w.workerCpuMicros();
        QElapsedTimer wall;
        wall.start();
        const qint64 c0 = jp::threadCpuMicros();
        const crunch::EditSession::Outcome o = fn();
        v->viewport()->repaint();
        const qint64 c1 = jp::threadCpuMicros();
        const double wallMs = double(wall.nsecsElapsed()) / 1e6;
        QElapsedTimer waiting;
        waiting.start();
        while ((lastEpoch < w.workbookLoader().changesMade()) && waiting.elapsed() < 120000) QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        const double settleMs = double(wall.nsecsElapsed()) / 1e6;
        out() << "  " << what << (o.ok ? QString() : QStringLiteral(" (refused: %1)").arg(o.problem)) << ": the window's thread " << QString::number(double(c1 - c0) / 1000.0, 'f', 3) << " ms by its processor time, "
              << QString::number(wallMs, 'f', 3) << " ms by the wall; the worker " << QString::number(double(w.workerCpuMicros() - workerBefore) / 1000.0, 'f', 3) << " ms of processor time, all worked out by "
              << QString::number(settleMs, 'f', 1) << " ms\n";
        if (o.ok) {
            cpu.push_back(double(c1 - c0) / 1000.0);
            wallv.push_back(wallMs);
        }
    };
    out() << "rows, columns, and cells inserted and deleted, and a block pasted (the sheet has " << usedRows << " rows by " << usedCols << " columns used):\n";
    for (const quint32 row : {quint32(5), usedRows / 2, usedRows - 5}) {
        timed(QStringLiteral("insert a row at row %1").arg(row + 1), [&] { return session->insertRows(sheet, row, 1); });
        timed(QStringLiteral("delete a row at row %1").arg(row + 1), [&] { return session->deleteRows(sheet, row, 1); });
    }
    timed(QStringLiteral("insert a column at column 3"), [&] { return session->insertColumns(sheet, 2, 1); });
    timed(QStringLiteral("delete a column at column 3"), [&] { return session->deleteColumns(sheet, 2, 1); });
    {
        CellRange block;
        block.valid = true;
        block.col1 = 0;
        block.col2 = std::min<quint32>(usedCols, 20) - 1;
        block.row1 = 0;
        block.row2 = std::min<quint32>(usedRows, quint32(100000 / (block.col2 + 1))) - 1;
        crunch::ClipData clip;
        QElapsedTimer copyTime;
        copyTime.start();
        const qint64 k0 = jp::threadCpuMicros();
        clip = session->copyCells(sheet, block, false);
        out() << "  copy " << block.cellCount() << " cells: the window's thread " << QString::number(double(jp::threadCpuMicros() - k0) / 1000.0, 'f', 3) << " ms by its processor time ("
              << clip.cells.size() << " cells held)\n";
        CellRange target;
        target.valid = true;
        target.row1 = target.row2 = usedRows + 10;
        target.col1 = target.col2 = 0;
        timed(QStringLiteral("paste %1 cells").arg(clip.cells.size()), [&] { return session->paste(sheet, target, clip, crunch::EditSession::PasteOptions()); });
        timed(QStringLiteral("undo the paste"), [&] {
            w.undoStack()->undo();
            return crunch::EditSession::Outcome();
        });
    }
    if (!cpu.empty()) {
        const double within = double(std::count_if(cpu.begin(), cpu.end(), [](double x) { return x <= 16.0; })) / double(cpu.size());
        out() << "inserts, deletes, and pastes within 16 ms by the processor time: " << QString::number(within * 100, 'f', 1) << "% of " << cpu.size() << "\n";
    }
    Q_UNUSED(wallv);
}

// Edits in the window: each is a number typed over a cell that is on the screen, as a person types it, and finished with Enter. What is measured is what a person waits for: from the Enter key to
// the cell drawn again (the window's thread: its own processor time, and the wall besides, which has the wait for the worker to let go in it), and the worker's: its processor time, and the wall
// until every formula that depends has its value.
int editBench(const QString &pathIn, int edits, int rows, int cols, int readers)
{
    QTemporaryDir dir;
    QString path = pathIn;
    if (path.isEmpty()) {
        const QString keep = qEnvironmentVariable("JC_BENCH_FILE");
        path = keep.isEmpty() ? dir.filePath(QStringLiteral("bench.xlsx")) : keep;
        if (keep.isEmpty() || !QFile::exists(keep)) {
            QElapsedTimer made;
            made.start();
            const QByteArray bytes = benchWorkbook(rows, cols, readers);
            QFile f(path);
            if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) {
                err() << "edit-bench: the workbook could not be written\n";
                return 1;
            }
            f.close();
            out() << "workbook of " << rows << " rows by " << cols << " columns" << (readers ? QStringLiteral(", and a cell that %1 formulas read").arg(readers) : QString()) << ": " << bytes.size() / 1048576
                  << " MB, made in " << made.elapsed() << " ms\n";
        }
    }
    CrunchWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    w.resize(1280, 800);
    w.show();
    QEventLoop none;
    bool done = false;
    QObject::connect(&w, &CrunchWindow::calculationFinished, &w, [&] { done = true; });
    QElapsedTimer open;
    open.start();
    if (!w.openFile(path)) return 1;
    while (!done && open.elapsed() < 280000) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    if (!done) {
        err() << "edit-bench: the workbook was not worked out in time\n";
        return 1;
    }
    out() << "opened and worked out by " << open.elapsed() << " ms\n";
    // the cells: in a made workbook, spread over the sheet (one cell for the readers); in a file, numbers that formulas read, from the first sheet that has them
    struct Target {
        quint32 row, col;
    };
    std::vector<Target> targets;
    const int sheet = w.activeSheet();
    const Calculation *calc = w.calculation();
    if (readers > 0 && pathIn.isEmpty()) {
        for (int i = 0; i < edits; ++i) targets.push_back({0, 25});
    } else if (pathIn.isEmpty()) {
        for (int i = 0; i < edits; ++i) targets.push_back({quint32((i * 7919) % rows), quint32((i * 5) % cols)});
    } else {
        std::vector<Target> candidates;
        qint64 looked = 0;
        calc->sheet(sheet).cells.forEach([&](const Cell &c) {
            if (candidates.size() >= 100000 || ++looked > 5000000) return;
            if (c.type != CellType::Number || c.hasFormula() || (c.flags & (ArrayMember | ArrayFormula))) return;
            std::vector<quint32> r;
            calc->graph().dependentsOfCell(sheet, c.row, c.col, &r);
            if (!r.empty()) candidates.push_back({c.row, c.col});
        });
        if (candidates.empty()) {
            err() << "edit-bench: no number that a formula reads\n";
            return 1;
        }
        for (int i = 0; i < edits; ++i) targets.push_back(candidates[size_t(i) * candidates.size() / size_t(edits)]);
    }
    grid::GridView *v = w.view();
    const bool noWait = !qEnvironmentVariableIsEmpty("JC_BENCH_NOWAIT");
    std::vector<double> commitCpu, paintCpu, totalCpu, totalWall, holdWait, workerCpu, allValues;
    qint64 dependentsMax = 0;
    for (size_t i = 0; i < targets.size(); ++i) {
        const Target t = targets[i];
        v->selectCell(t.row, t.col);
        v->ensureVisible(t.row, t.col);
        QCoreApplication::processEvents();
        v->viewport()->repaint();
        // a number typed over the cell (the first key at the grid, which starts the entry, and the rest in the cell's line)
        const QString text = QString::number(1000 + int(i));
        QKeyEvent first(QEvent::KeyPress, Qt::Key_0 + (text[0].unicode() - '0'), Qt::NoModifier, QString(text[0]));
        QApplication::sendEvent(v, &first);
        w.cellEditor()->setText(text);
        if (!w.isEditing()) {
            err() << "edit-bench: the entry did not begin\n";
            return 1;
        }
        const qint64 workerBefore = w.workerCpuMicros();
        QElapsedTimer wall;
        wall.start();
        const qint64 c0 = jp::threadCpuMicros();
        QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QApplication::sendEvent(w.cellEditor(), &enter);
        const qint64 c1 = jp::threadCpuMicros();
        // (what the window draws for a changed cell: the strip of its row)
        {
            const QRect r = v->cellRect(t.row, t.col);
            v->viewport()->repaint(QRect(0, r.top(), v->viewport()->width(), std::max(1, r.height())));
        }
        const qint64 c2 = jp::threadCpuMicros();
        const double wallMs = double(wall.nsecsElapsed()) / 1e6;
        commitCpu.push_back(double(c1 - c0) / 1000.0);
        paintCpu.push_back(double(c2 - c1) / 1000.0);
        totalCpu.push_back(double(c2 - c0) / 1000.0);
        totalWall.push_back(wallMs);
        holdWait.push_back(double(w.session()->lastHoldWaitMicros()) / 1000.0);
        // until every formula that depends has its value
        // (JC_BENCH_NOWAIT: the next edit is made as soon as this one is drawn, with the worker still at work on what depends: the wait for it to let go is then what is measured)
        QElapsedTimer waiting;
        waiting.start();
        while (!noWait && w.isCalculating() && waiting.elapsed() < 120000) QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        allValues.push_back(double(wall.nsecsElapsed()) / 1e6);
        workerCpu.push_back(double(w.workerCpuMicros() - workerBefore) / 1000.0);
        dependentsMax = std::max<qint64>(dependentsMax, w.batchesTaken());
    }
    out() << targets.size() << " edits (the window's thread, by its own processor time, and by the wall):\n";
    printStat("the Enter key to the cell drawn again, processor time", totalCpu);
    printStat("   of it putting the cell in", commitCpu);
    printStat("   of it drawing", paintCpu);
    printStat("the Enter key to the cell drawn again, wall (the wait for the worker to let go is in it)", totalWall);
    printStat("   the wait for the worker to let go, wall", holdWait);
    printStat("the worker's processor time for what depends on the cell", workerCpu);
    printStat("until every formula that depends has its value, wall", allValues);
    const double within = double(std::count_if(totalWall.begin(), totalWall.end(), [](double x) { return x <= 16.0; })) / double(totalWall.size());
    const double withinCpu = double(std::count_if(totalCpu.begin(), totalCpu.end(), [](double x) { return x <= 16.0; })) / double(totalCpu.size());
    out() << "edits repainted within 16 ms: " << QString::number(withinCpu * 100, 'f', 1) << "% by the processor time, " << QString::number(within * 100, 'f', 1) << "% by the wall (the bar: 99%)\n";
    Q_UNUSED(dependentsMax);
    structureBench(w, sheet);
    return 0;
}

} // namespace

namespace jp::crunchtool {

int editMain(const QStringList &args)
{
    if (args.size() >= 2 && args.size() <= 3 && args[0] == QLatin1String("edit-bench")) return editBench(args[1], args.value(2, QStringLiteral("200")).toInt(), 0, 0, 0);
    if (args.size() >= 1 && args.size() <= 5 && args[0] == QLatin1String("edit-bench-make"))
        return editBench(QString(), args.value(4, QStringLiteral("200")).toInt(), args.value(1, QStringLiteral("100000")).toInt(), args.value(2, QStringLiteral("20")).toInt(), args.value(3, QStringLiteral("0")).toInt());
    if (args.size() == 2 && args[0] == QLatin1String("roundtrip-one")) return roundtripOne(args[1]);
    if ((args.size() == 3 || args.size() == 4) && args[0] == QLatin1String("roundtrip")) return roundtripSweep(args[1], args[2], args.value(3));
    return -1;
}

} // namespace jp::crunchtool

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
                if (w.loader.result()->book->info().sheets[si].kind == SheetKind::Worksheet) sheet = si;
        if (sheet < 0) {
            cellRight = true;   // (a workbook of chart sheets has no cell to change)
            cellName_ = QStringLiteral("(no sheet of cells)");
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
    if (!qEnvironmentVariableIsEmpty("JC_ROUNDTRIP_KEEP") && !cellRight) err() << "the edited cell is not right: " << cellName_ << "\n";
    const bool cellsOk = cellsSame && untouched.differences.isEmpty() && partsSame == partsTotal;
    out() << path << '\t' << exit << '\t' << fixed(double(total.nsecsElapsed()) / 1e9) << '\t' << fixed(peakMegabytes()) << '\t' << partsTotal << '\t' << partsSame << '\t' << (cellsOk ? 1 : 0) << '\t'
          << untouched.differences.join(QLatin1String("; ")) << '\t' << cellName_ << '\t' << dependents << '\t' << resultsChanged << '\t' << unexpected << '\t' << editParts << '\t' << editPartsSame << '\t'
          << (cellRight ? 1 : 0) << '\t' << edited.differences.join(QLatin1String("; ")) << '\t' << fixed(untouchedMs) << '\t' << fixed(editMs) << "\n";
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

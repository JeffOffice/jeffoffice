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
//                                           (3) then an edit script on the same open workbook (a row inserted, a column inserted, a block pasted below the cells, a fill, a block given a font, fill,
//                                           alignment, number format, and borders, two cells merged, a row deleted, a column deleted, and a sheet added, renamed, moved, and colored: those the
//                                           workbook allows), saved and read back, and compared with the model the window has: every cell (what it holds, its format by place and by how it
//                                           looks, and what it shows), the merged ranges, and the parts that were not meant to change; the fields after the edit's: the steps made and refused, the cells
//                                           compared and right, the differences, the parts not meant to change and the same, and the milliseconds.
//   jcrunchtool edit-names-roundtrip <file.xlsx>   opens the workbook as the window does, makes a name and changes the comment of the first name the file has (the other names are not touched),
//                                           saves, reads the new file, and compares every name (its scope, what it stands for, its comment, whether it is hidden), the bytes of the names that were not
//                                           touched, and every cell. One line: `names-roundtrip`, the path, the result (ok, or the first differences), the names the file has, the names that read back the same,
//                                           the names whose element is the file's, byte for byte, the cells, and the cells that read back the same.
//   jcrunchtool roundtrip <keys.txt> <out-dir> [jobs]   runs roundtrip-one on each workbook listed, each in a child process that has 300 seconds and 6 GB, and writes <out-dir>/roundtrip.tsv; prints the
//                                           summary (tools/crunch_roundtrip.py does the work)
//   jcrunchtool edit-bench <file.xlsx> [edits=200]   opens the workbook in the model of the window and makes that many edits, each in a cell a few rows apart, as a person typing; for each, the
//                                           time the window's thread takes to put the cell in (and the time it waits for the worker to let go), to draw it again, and the time the worker takes to
//                                           the first values and to work out all that depends: the middle, the 95th percentile, the 99th, and the longest of each, by the thread's own processor time
//                                           where there is one and by the wall's besides
//   jcrunchtool edit-recalc-check <file.xlsx> [edits=6] [seed=1]   opens the workbook, makes that many random edits of whole rows and columns (the window's way, each worked out by the worker), and
//                                           compares the value of every formula with a whole new calculation of the same cells (formulas that differ between two whole calculations with no edit, the
//                                           time and RAND, are left out). One line: path, formulas, edits made, refused, left out, compared, mismatches, the first mismatch; the exit code is 3 for a mismatch.
//   jcrunchtool edit-walk-bench <file.xlsx> [row edits=1000] [column edits=10]   the time to walk all the cells of the first sheet and from the end in slices, before and after those edits of rows and columns
//   jcrunchtool edit-struct-bench <file.xlsx> <label> [only]   one row and one column inserted and deleted at the top, the middle, and the end of the used cells (and undone), each timed (the window's
//                                           thread, the worker, the wall until it settles, the model's counted work, the texts made); one tab-separated line each (docs/jeffcrunch-rowids.md).
//                                           JC_UNDO_MB raises the limit that makes a big step ask first. tools/crunch_struct_bench_make.py makes the workbooks. After the times and the counted work come the
//                                           counters of what the edit did (cells moved and deleted, formulas visited and written again, formulas made again in the graph, the graph's work, names and rules
//                                           written again, the session's marks followed, the window's values moved and sizes and texts rebuilt, the bytes of Undo, the graph's entries) and then the
//                                           same for its Undo (the counters are in Calculation::editStats, EditSession::marksFollowed, ValueStore::relocated, GridSheet::viewCellsRebuiltForTest).
//   jcrunchtool edit-fill-bench <file.xlsx> <label> [edits=1000] [only]   after that many rows put in at places spread over the first half of the sheet, times a formula typed and filled down all the rows, a
//                                           block of values (all the rows by 10 columns) pasted (a step takes 100,000 cells, so in as many steps), and some rows sorted by the first column, each by the window's thread (the step made and put in) and the wall until the worker settles, with
//                                           the counters of what it did (texts turned into ids, formulas written again, formulas rebased)
//   jcrunchtool model-check <seeds> <steps> [first seed] [every=n] [nograph] [noshrink]   the model check of tests/crunch/structmodel.h: random steps on a small workbook, and after each the workbook is
//                                           saved to memory, opened again, and every formula worked out from nothing, and all of it compared (values, text of formulas, conditional formats, validations, names, tables,
//                                           charts, notes, links, merged cells, print areas, page breaks, filters, sizes); the graph must hold what each formula reads. A failure prints the seed and the fewest steps.
//   jcrunchtool model-replay <file>   runs the steps of a failure's report again;  model-dump <seed> <steps>|<file>   prints the state after them;  model-book <out.xlsx>   writes the workbook it begins with;
//   jcrunchtool model-golden <steps> <seed>...   prints the digests tests/crunch/structmodel_golden.txt keeps
//   jcrunchtool edit-save-struct-bench <file.xlsx> <label>   saves the workbook untouched and after structural edits at random places (JC_SAVE_EDITS=20,200 says how many), timing the session's snapshot and the save
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
#include "../tests/crunch/structmodel.h"

#ifdef __GLIBC__
#include <malloc.h>
#endif
#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QKeyEvent>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRandomGenerator>
#include <QRegularExpression>
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

// The cell at a place, as a block of one cell (what the graph is asked about, through Calculation::readersOf).
jp::calc::CellArea placeArea(int sheet, quint32 row, quint32 col)
{
    jp::calc::CellArea a;
    a.sheet1 = a.sheet2 = sheet;
    a.row1 = a.row2 = row;
    a.col1 = a.col2 = col;
    return a;
}

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

// A cell of a store's walk, with its place.
struct PlacedRef {
    quint32 row, col;
    const Cell *cell;
};

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
        loader.setManualAllowed(false);   // (the yardstick works every formula out, whatever the file asks for)
        c1 = QObject::connect(&loader, &crunch::WorkbookLoader::finished, &loader, [this](bool good, const QString &m) {
            finished = true;
            ok = good;
            message = m;
        });
        c2 = QObject::connect(&loader, &crunch::WorkbookLoader::calcBatch, &loader, [this](const std::shared_ptr<crunch::CalcBatch> &b) {
            if (loader.result() && loader.result()->calc) loader.result()->calc->valuesTaken(qint64(b->items.size()));
            if (session) session->dropStale(&b->items);   // (as the window does: what an edit of cells moved after it was worked out is let go of)
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
    qint64 otherCompared = 0, otherRight = 0;   // the defined names, rules of validation, tables, conditional formats, hyperlinks, and chart series
    int parts = 0, partsSame = 0;
    double ms = 0;
    Report differences;
    bool ran = false;
};

// Whether two cell formats (of two workbooks' styles) look the same: the font, fill, border, number format code, and the alignment and protection, wherever each is kept.
bool sameLook(const Styles &a, quint32 xa, const Styles &b, quint32 xb)
{
    if (xa >= quint32(a.formats.size()) || xb >= quint32(b.formats.size())) return xa == xb;
    CellFormat fa = a.formats[int(xa)], fb = b.formats[int(xb)];
    if (fa.font >= quint32(a.fonts.size()) || fb.font >= quint32(b.fonts.size()) || fa.fill >= quint32(a.fills.size()) || fb.fill >= quint32(b.fills.size()) || fa.border >= quint32(a.borders.size()) ||
        fb.border >= quint32(b.borders.size()))
        return false;
    if (!(a.fonts[int(fa.font)] == b.fonts[int(fb.font)]) || !(a.fills[int(fa.fill)] == b.fills[int(fb.fill)]) || !(a.borders[int(fa.border)] == b.borders[int(fb.border)])) return false;
    if (a.formatCode(xa, QLocale::c()) != b.formatCode(xb, QLocale::c())) return false;
    fa.font = fb.font = fa.fill = fb.fill = fa.border = fb.border = 0;
    fa.numFmtId = fb.numFmtId = 0;
    return fa == fb;
}

// The model the window has, made again from the bytes of the file saved: the cells are compared one by one. `savedIndex` says where each sheet of the model is in the file saved (a sheet added
// is put where the tabs show it, and a sheet moved is moved), and the formats are compared by how they look as well as by their place.
void compareModels(const Calculation *live, const Calculation *saved, const std::vector<int> &savedIndex, const Styles &liveStyles, const Styles &savedStyles, ScriptResult *r)
{
    std::unordered_map<quint32, bool> looks;
    auto looksSame = [&](quint32 xf) {
        const auto it = looks.find(xf);
        if (it != looks.end()) return it->second;
        return looks[xf] = sameLook(liveStyles, xf, savedStyles, xf);
    };
    // (JC_ROUNDTRIP_SHOW=1 writes the first cells that differ to the error output, with the two values)
    static const bool show = qEnvironmentVariableIntValue("JC_ROUNDTRIP_SHOW") != 0;
    int shown = 0;
    auto say = [&](int s, quint32 row, quint32 col, const char *what, const Calculation::CellContent &x, const Calculation::CellContent &y) {
        if (!show || shown++ >= 25) return;
        auto text = [](const jp::calc::Value &v) {
            return v.isNumber() ? QString::number(v.number(), 'g', 17) : v.isText() ? QLatin1Char('"') + v.text() + QLatin1Char('"') : v.isBool() ? QString::number(v.boolean()) : v.isError() ? QStringLiteral("error ") + QString::number(int(v.errorKind())) : QStringLiteral("(empty)");
        };
        qWarning().noquote() << "differs:" << what << "sheet" << s << cellName(row, col) << "live" << x.formula << text(x.value) << "saved" << y.formula << text(y.value);
    };
    auto empty = [](const Calculation::CellContent &c) { return !c.exists || (!c.isFormula && c.value.isEmpty() && c.style == 0); };
    bool circular = false;
    for (int s = 0; s < live->sheetCount() && !circular; ++s)
        live->sheet(s).cells.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) { circular = circular || live->isCircular(s, cellRow, cellCol); });
    if (live->sheetCount() != saved->sheetCount() || savedIndex.size() != size_t(live->sheetCount())) {
        r->differences.add(QStringLiteral("sheets"));
        return;
    }
    for (int s = 0; s < live->sheetCount(); ++s) {
        const int t = savedIndex[size_t(s)];
        auto at = [&](quint32 row, quint32 col) {
            const Calculation::CellContent x = live->content(s, row, col), y = saved->content(t, row, col);
            if (empty(x) && empty(y)) return;
            ++r->compared;
            // (a cell of an array holds the result the file had, which is no part of what it means: what it shows is compared)
            const bool member = !x.editable && !x.isFormula;
            if (empty(x) != empty(y) || x.isFormula != y.isFormula || x.formula != y.formula || (!member && !sameShown(x.value, y.value)) || x.style != y.style) {
                say(s, row, col, "content", x, y);
                r->differences.add(empty(x) != empty(y) ? QStringLiteral("a cell is there or not") : x.isFormula != y.isFormula || x.formula != y.formula ? QStringLiteral("a formula differs")
                                   : x.style != y.style ? QStringLiteral("a format differs") : QStringLiteral("a value differs"));
                return;
            }
            if (!looksSame(x.style)) {
                say(s, row, col, "look", x, y);
                r->differences.add(QStringLiteral("a format looks different"));
                return;
            }
            if (!circular && !sameShown(live->cell(s, row, col), saved->cell(t, row, col))) {
                Calculation::CellContent a = x, b = y;
                a.value = live->cell(s, row, col);
                b.value = saved->cell(t, row, col);
                say(s, row, col, "result", a, b);
                r->differences.add(QStringLiteral("a result differs"));
                return;
            }
            ++r->right;
        };
        live->sheet(s).cells.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) { at(cellRow, cellCol); });
        saved->sheet(t).cells.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) {
            if (!live->sheet(s).cells.find(cellRow, cellCol)) at(cellRow, cellCol);
        });
        QVector<CellRange> ma = live->mergesOf(s), mb = saved->mergesOf(t);
        auto less = [](const CellRange &p, const CellRange &q) { return p.row1 != q.row1 ? p.row1 < q.row1 : p.col1 < q.col1; };
        std::sort(ma.begin(), ma.end(), less);
        std::sort(mb.begin(), mb.end(), less);
        ++r->compared;
        if (ma == mb) ++r->right;
        else r->differences.add(QStringLiteral("merged ranges differ"));
    }
}

// ---- what the file holds besides the cells: names, validations, tables (the model is the check), and conditional formats, hyperlinks, and chart series (the file as it was, with the edits made
// to it by the same rules, is the check)

QString unescapeXml(QString s)
{
    s.replace(QStringLiteral("&lt;"), QStringLiteral("<")).replace(QStringLiteral("&gt;"), QStringLiteral(">")).replace(QStringLiteral("&quot;"), QStringLiteral("\"")).replace(QStringLiteral("&apos;"), QStringLiteral("'")).replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    return s;
}

// The text of an attribute of a tag ("" if it has none).
QString attributeOf(const QString &tag, const QString &name)
{
    const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("\\s%1=\"([^\"]*)\"").arg(name)).match(tag);
    return m.hasMatch() ? unescapeXml(m.captured(1)) : QString();
}

// "sqref \t formula \t formula" for each conditional format of a sheet's part, and "ref \t location" for each hyperlink.
QStringList conditionalFormatsOf(const QString &xml)
{
    QStringList out;
    static const QRegularExpression block(QStringLiteral("<conditionalFormatting\\b([^>]*)>(.*?)</conditionalFormatting>"), QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression formula(QStringLiteral("<formula>(.*?)</formula>"), QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = block.globalMatch(xml);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        QStringList parts{attributeOf(m.captured(1), QStringLiteral("sqref"))};
        QRegularExpressionMatchIterator f = formula.globalMatch(m.captured(2));
        while (f.hasNext()) parts << unescapeXml(f.next().captured(1));
        out << parts.join(QLatin1Char('\t'));
    }
    return out;
}

QStringList hyperlinksOf(const QString &xml)
{
    QStringList out;
    static const QRegularExpression link(QStringLiteral("<hyperlink\\b([^>]*)>"));
    QRegularExpressionMatchIterator it = link.globalMatch(xml);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        out << attributeOf(m.captured(1), QStringLiteral("ref")) + QLatin1Char('\t') + attributeOf(m.captured(1), QStringLiteral("location"));
    }
    return out;
}

QStringList chartSeriesOf(const QString &xml)
{
    QStringList out;
    static const QRegularExpression f(QStringLiteral("<c:f>(.*?)</c:f>"), QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = f.globalMatch(xml);
    while (it.hasNext()) out << unescapeXml(it.next().captured(1));
    return out;
}

void compareOthers(const Read &before, const Read &after, const Workbook &liveBook, const Calculation *live, const Calculation *saved, const std::vector<int> &savedIndex,
                   const std::vector<StructEdit> &edits, const std::vector<SheetOp> &ops, ScriptResult *r)
{
    // (a sheet renamed: the formulas of the file that name it follow, in the same way)
    auto renamed = [&](QString f) {
        for (const SheetOp &op : ops)
            if (op.kind == SheetOp::Kind::Rename) f = renameSheetInFormula(f, op.name, op.newName);
        return f;
    };
    auto note = [&](bool same, const char *what) {
        ++r->otherCompared;
        if (same) ++r->otherRight;
        else r->differences.add(QString::fromLatin1(what));
    };
    QStringList names;
    for (const SheetEntry &se : before.book.info().sheets) names << se.name;
    // the defined names (print areas among them), by the model
    {
        const QHash<QString, QString> &a = live->definedNameTexts(), &b = saved->definedNameTexts();
        note(a.size() == b.size(), "the number of defined names differs");
        static const bool show = qEnvironmentVariableIntValue("JC_ROUNDTRIP_SHOW") != 0;
        for (auto it = a.constBegin(); it != a.constEnd(); ++it) {
            // (a name of one sheet is kept by the sheet's place, "3|Name": the sheet is where the file has it)
            QString key = it.key();
            const int bar = key.indexOf(QLatin1Char('|'));
            bool numeric = false;
            const int sheetOf = bar > 0 ? key.left(bar).toInt(&numeric) : -1;
            if (numeric && sheetOf >= 0 && size_t(sheetOf) < savedIndex.size()) key = QString::number(savedIndex[size_t(sheetOf)]) + key.mid(bar);
            note(b.value(key) == it.value(), "a defined name differs");
            if (qEnvironmentVariableIntValue("JC_ROUNDTRIP_SHOW") == 2) qWarning().noquote() << "defined name:" << it.key() << "live" << it.value() << "saved" << b.value(key);
            if (show && b.value(key) != it.value()) qWarning().noquote() << "defined name differs:" << it.key() << "live" << it.value() << "saved" << b.value(key);
        }
    }
    // the rules of validation, by the model
    for (int s = 0; s < live->sheetCount() && s < saved->sheetCount() && size_t(s) < savedIndex.size(); ++s) {
        const QVector<Validation> &a = live->sheet(s).validations, &b = saved->sheet(savedIndex[size_t(s)]).validations;
        note(a.size() == b.size(), "the number of rules of validation differs");
        for (int i = 0; i < std::min(a.size(), b.size()); ++i)
            note(a[i].ranges == b[i].ranges && a[i].type == b[i].type && a[i].formula1 == b[i].formula1 && a[i].formula2 == b[i].formula2, "a rule of validation differs");
    }
    // the tables, by the model
    {
        const QVector<Table> &a = liveBook.tables(), &b = after.book.tables();
        note(a.size() == b.size(), "the number of tables differs");
        for (int i = 0; i < std::min(a.size(), b.size()); ++i)
            note(a[i].row1 == b[i].row1 && a[i].col1 == b[i].col1 && a[i].row2 == b[i].row2 && a[i].col2 == b[i].col2 && a[i].totalsRows == b[i].totalsRows && a[i].columns == b[i].columns, "a table differs");
    }
    // the conditional formats and hyperlinks of each sheet, and the series of each chart: the file as it was, with the edits made to it, against the file saved
    for (int s = 0; s < before.book.info().sheets.size(); ++s) {
        const SheetEntry &entry = before.book.info().sheets[s];
        if (entry.kind != SheetKind::Worksheet || entry.part.isEmpty()) continue;
        const QString was = QString::fromUtf8(before.package.part(entry.part)), now = QString::fromUtf8(after.package.part(entry.part));
        QStringList expected;
        for (const QString &rule : conditionalFormatsOf(was)) {
            QStringList parts = rule.split(QLatin1Char('\t'));
            QString sqref = parts.takeFirst();
            CellRange first;
            if (!parseRangeRef(sqref.section(QLatin1Char(' '), 0, 0), &first)) first = CellRange();
            for (const StructEdit &e : edits) {
                for (QString &f : parts) f = adjustRuleFormula(f, s, names, e, first);
                if (first.valid && !adjustRange(e, s, &first)) first = CellRange();
                if (!sqref.isEmpty()) sqref = adjustSqref(sqref, e, s);
            }
            for (QString &f : parts) f = renamed(f);
            if (!sqref.isEmpty()) expected << (QStringList{sqref} + parts).join(QLatin1Char('\t'));
        }
        QStringList got = conditionalFormatsOf(now);
        std::sort(expected.begin(), expected.end());
        std::sort(got.begin(), got.end());
        note(expected == got, "a conditional format differs");
        expected.clear();
        for (const QString &link : hyperlinksOf(was)) {
            QString ref = link.section(QLatin1Char('\t'), 0, 0), location = link.section(QLatin1Char('\t'), 1);
            for (const StructEdit &e : edits) {
                if (!ref.isEmpty()) ref = adjustSqref(ref, e, s);
                if (!location.isEmpty()) location = adjustFormula(location, s, names, e);
            }
            location = renamed(location);
            if (!ref.isEmpty()) expected << ref + QLatin1Char('\t') + location;
        }
        got = hyperlinksOf(now);
        std::sort(expected.begin(), expected.end());
        std::sort(got.begin(), got.end());
        note(expected == got, "a hyperlink differs");
    }
    static const QRegularExpression chartPart(QStringLiteral("/charts/chart\\d*\\.xml$"));
    for (const QString &part : before.package.partNames()) {
        if (!chartPart.match(part).hasMatch()) continue;
        QStringList expected = chartSeriesOf(QString::fromUtf8(before.package.part(part)));
        for (QString &f : expected)
            for (const StructEdit &e : edits) f = adjustFormula(f, 0, names, e);
        for (QString &f : expected) f = renamed(f);
        note(expected == chartSeriesOf(QString::fromUtf8(after.package.part(part))), "a chart's series differ");
    }
}

// Where each sheet of the model is in the file saved: the sheets are written in the order of the tabs (deleted ones are not written).
std::vector<int> savedIndexes(const Workbook &book)
{
    const WorkbookInfo &info = book.info();
    std::vector<int> out(size_t(info.sheets.size()), -1);
    int at = 0;
    for (const int i : info.order)
        if (i >= 0 && i < int(info.sheets.size()) && !info.sheets[size_t(i)].deleted) out[size_t(i)] = at++;
    return out;
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
        if (!o.ok && qEnvironmentVariableIntValue("JC_ROUNDTRIP_SHOW") != 0) qWarning().noquote() << "script step" << (r->done + r->refused) << "refused:" << o.problem;
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
    // formatting: a block given a font, a fill, an alignment, a number format, and borders, and two empty cells merged (the edits that follow move the formats with the cells)
    {
        const CellRange shown = block(0, 0, std::min<quint32>(rows - 1, 3), std::min<quint32>(cols - 1, 3));
        FormatChange change;
        change.fontName = QStringLiteral("Georgia");
        change.fontSize = 13;
        change.bold = true;
        change.italic = true;
        change.underline = Underline::Double;
        change.fontColor = ColorSpec::fromRgb(0xC00000);
        FillSpec fill;
        fill.pattern = PatternKind::Solid;
        fill.fg = ColorSpec::fromRgb(0xFFEB9C);
        change.fill = fill;
        change.numberFormat = QStringLiteral("#,##0.00");
        change.horizontal = HAlign::Center;
        change.wrap = true;
        step(session->formatCells(sheet, shown, change, QStringLiteral("Formatting")));
        BorderSide line;
        line.style = BorderStyle::Thin;
        line.color = ColorSpec::fromRgb(0x0000FF);
        using Session = crunch::EditSession;
        step(session->setBorders(sheet, shown, Session::EdgeLeft | Session::EdgeRight | Session::EdgeTop | Session::EdgeBottom | Session::EdgeInsideHorizontal | Session::EdgeInsideVertical, line,
                                 QStringLiteral("Borders")));
        const quint32 mergeRow = std::min<quint32>(rows + 3, kMaxRows - 3), mergeCol = std::min<quint32>(cols + 3, 16000);
        step(session->mergeCells(sheet, block(mergeRow, mergeCol, mergeRow, mergeCol + 1), Session::MergeKind::Merge));
    }
    step(session->deleteRows(sheet, std::max<quint32>(1, rows / 3), 1));
    step(session->deleteColumns(sheet, std::max<quint32>(1, cols / 3), 1));
    // sheets: one added before the sheet edited, that sheet renamed (the formulas that name it follow) and moved to the end, and its tab given a color
    step(session->insertSheet(sheet, QStringLiteral("Added Sheet"), nullptr));
    step(session->renameSheet(sheet, QStringLiteral("Edited Sheet")));
    step(session->moveSheet(sheet, -1));
    step(session->setTabColor(sheet, ColorSpec::fromRgb(0x00B050)));
    if (qEnvironmentVariableIntValue("JC_ROUNDTRIP_CELL") != 0) {
        // (JC_ROUNDTRIP_CELL=1: where the formulas of one text are, and the state of each and of what it reads)
        const QString want = qEnvironmentVariable("JC_ROUNDTRIP_TEXT");
        calc->forEachFormula([&](const Calculation::FormulaCell &f) {
            if (!f.cell) return;
            const quint32 row = calc->sheet(f.sheet).cells.rowPlaceOf(f.cell->rowId), col = calc->sheet(f.sheet).cells.colPlaceOf(f.cell->colId);
            const QString text = calc->content(f.sheet, row, col).formula;
            if (text != want) return;
            qWarning().noquote() << "cell:" << f.sheet << cellName(row, col) << text << "value" << f.ours.debug() << "circular" << calc->isCircular(f.sheet, row, col) << "tainted" << calc->isTainted(f.sheet, row, col);
        });
    }
    // (JC_ROUNDTRIP_STORE=1: the formulas whose value the window's store holds is not the calculation's)
    if (qEnvironmentVariableIntValue("JC_ROUNDTRIP_STORE") != 0) {
        int said = 0;
        qint64 differ = 0;
        calc->forEachFormula([&](const Calculation::FormulaCell &f) {
            if (!f.cell) return;
            const quint32 row = calc->sheet(f.sheet).cells.rowPlaceOf(f.cell->rowId), col = calc->sheet(f.sheet).cells.colPlaceOf(f.cell->colId);
            jp::calc::Value shown;
            const bool has = w.values.find(f.sheet, f.cell->rowId, f.cell->colId, &shown);
            if (!has && !(f.sheet == 0 && row == 94 && col == 4)) return;   // (a value the worker did not send is none)
            if (has && (shown == f.ours || (shown.isNumber() && f.ours.isNumber() && jp::calc::sameTo15Digits(shown.number(), f.ours.number())))) return;
            ++differ;
            if (said++ < 8) qWarning().noquote() << "store differs:" << f.sheet << cellName(row, col) << "store" << (has ? shown.debug() : QStringLiteral("(none)")) << "calculation" << f.ours.debug();
        });
        qWarning().noquote() << "formulas whose value in the window's store is not the calculation's:" << differ;
    }
    // (JC_ROUNDTRIP_RECALC=1: every formula is worked out again from nothing, and the first that the window had another value for are written to the error output)
    if (qEnvironmentVariableIntValue("JC_ROUNDTRIP_RECALC") != 0) {
        std::unordered_map<quint64, jp::calc::Value> before;
        calc->forEachFormula([&](const Calculation::FormulaCell &f) {
            if (f.cell) before[crunch::EditSession::keyOf(f.sheet, f.cell->rowId, f.cell->colId)] = f.ours;
        });
        w.beginEdit();
        w.loader.recalculate();
        w.settle();
        int said = 0;
        qint64 stale = 0;
        calc->forEachFormula([&](const Calculation::FormulaCell &f) {
            if (!f.cell) return;
            const auto it = before.find(crunch::EditSession::keyOf(f.sheet, f.cell->rowId, f.cell->colId));
            if (it == before.end() || (it->second.isNumber() && f.ours.isNumber() && jp::calc::sameTo15Digits(it->second.number(), f.ours.number())) || it->second == f.ours) return;
            ++stale;
            if (said++ < 12) {
                const quint32 row = calc->sheet(f.sheet).cells.rowPlaceOf(f.cell->rowId), col = calc->sheet(f.sheet).cells.colPlaceOf(f.cell->colId);
                qWarning().noquote() << "stale in the window:" << f.sheet << cellName(row, col) << calc->content(f.sheet, row, col).formula << "had" << it->second.debug() << "whole calculation" << f.ours.debug();
            }
        });
        qWarning().noquote() << "formulas the window had another value for than a whole calculation gives:" << stale;
    }
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
    const QString keepAs = qEnvironmentVariable("JC_ROUNDTRIP_KEEP_SCRIPT");   // (a file to keep the workbook saved after the script in, for a look at it)
    if (!keepAs.isEmpty()) {
        QFile kept(keepAs);
        if (kept.open(QIODevice::WriteOnly)) kept.write(bytes);
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
    const std::vector<int> savedIndex = savedIndexes(*w.loader.result()->book);
    compareModels(calc, &saved, savedIndex, w.loader.result()->book->styles(), after.book.styles(), r);
    {
        const Read orig(original);
        if (orig.ok) compareOthers(orig, after, *w.loader.result()->book, calc, &saved, savedIndex, snap.changes.edits, snap.changes.sheetOps, r);
    }
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
                std::vector<PlacedRef> pa, pb;
                pa.reserve(size_t(sa.cells.cellCount()));
                pb.reserve(size_t(sb.cells.cellCount()));
                sa.cells.forEach([&](quint32 row, quint32 col, const Cell &c) { pa.push_back({row, col, &c}); });
                sb.cells.forEach([&](quint32 row, quint32 col, const Cell &c) { pb.push_back({row, col, &c}); });
                for (size_t k = 0; k < pa.size() && k < pb.size(); ++k) {
                    if (pa[k].row != pb[k].row || pa[k].col != pb[k].col || hashCell(sa, before.book.sharedStrings(), *pa[k].cell) != hashCell(sb, after.book.sharedStrings(), *pb[k].cell)) {
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
                calc->sheet(si).cells.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) {
                    if (sheet >= 0 || ++looked > 3000000) return;
                    if (c.type != CellType::Number || c.hasFormula() || (c.flags & (ArrayMember | ArrayFormula))) return;
                    if (!w.session->canEditCell(si, cellRow, cellCol)) return;   // (a locked cell of a protected sheet is not changed)
                    if (pass == 0) {
                        std::vector<quint32> readers;
                        calc->readersOf(placeArea(si, cellRow, cellCol), &readers);
                        if (readers.empty()) return;
                    }
                    sheet = si;
                    row = cellRow;
                    col = cellCol;
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
            calc->readersOf(placeArea(sheet, row, col), &readers);
            dependents = qint64(readers.size());
            typedText = QString::number(newValue, 'g', 17);
            w.session->setLocale(QLocale::c());
            w.trackChanges = true;
            w.beginEdit();
            const crunch::EditSession::Outcome o = w.session->type(sheet, row, col, typedText, false);   // (the yardstick is for saving: a rule of validation does not stand in its way)
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
                            std::vector<PlacedRef> pa, pb;
                            sa.cells.forEach([&](quint32 row, quint32 col, const Cell &c) { pa.push_back({row, col, &c}); });
                            sb.cells.forEach([&](quint32 row, quint32 col, const Cell &c) { pb.push_back({row, col, &c}); });
                            size_t i = 0, j = 0;
                            auto key = [](const PlacedRef *c) { return (quint64(c->row) << 16) | c->col; };
                            const SharedStrings &ha = before.book.sharedStrings(), &hb = after.book.sharedStrings();
                            const quint64 editedKey = (quint64(row) << 16) | col;
                            while (i < pa.size() || j < pb.size()) {
                                const PlacedRef *a1 = i < pa.size() ? &pa[i] : nullptr, *b1 = j < pb.size() ? &pb[j] : nullptr;
                                if (a1 && b1 && key(a1) == key(b1)) {
                                    const bool isEdited = si == sheet && key(a1) == editedKey;
                                    const Cell *model = calc->sheet(si).cells.find(b1->row, b1->col);
                                    const bool arrayMember = model && (model->flags & ArrayMember);
                                    if (isEdited) {
                                        cellRight = isTyped(*b1->cell, sb, hb, newValue, typedText);
                                    } else if (b1->cell->hasFormula() || arrayMember) {
                                        // a formula's result: the engine's now if the edit changed it, else the file's as it was
                                        const jp::calc::Value ours = calc->cell(si, b1->row, b1->col);
                                        // (the results the engine changed are named by ids: those of the cell in the model, which is the live workbook)
                                        if (model && w.changedByEdit.count(crunch::EditSession::keyOf(si, model->rowId, model->colId))) {
                                            if (!storedIs(sb, hb, *b1->cell, ours)) {
                                                ++unexpected;
                                                edited.add(QStringLiteral("a result is not the engine's"));
                                            }
                                        } else if (!sameStored(sa, ha, *a1->cell, sb, hb, *b1->cell)) {
                                            ++unexpected;
                                            edited.add(QStringLiteral("a result changed that the edit did not change"));
                                        }
                                        if (a1->cell->style != b1->cell->style || a1->cell->hasFormula() != b1->cell->hasFormula() || (a1->cell->hasFormula() && sa.formulaText(*a1->cell) != sb.formulaText(*b1->cell))) {
                                            ++unexpected;
                                            edited.add(QStringLiteral("a formula or format changed"));
                                        }
                                    } else if (hashCell(sa, ha, *a1->cell) != hashCell(sb, hb, *b1->cell)) {
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
                                        cellRight = isTyped(*b1->cell, sb, hb, newValue, typedText);
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
          << script.compared << '\t' << script.right << '\t' << script.parts << '\t' << script.partsSame << '\t' << script.differences.differences.join(QLatin1String("; ")) << '\t' << fixed(script.ms) << '\t'
          << script.otherCompared << '\t' << script.otherRight << "\n";
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

// How fast the cells of the first sheet are walked, whole (forEach) and from the end in slices (the print layout's walk), when no row or column has been edited and after `colEdits` columns and `rowEdits`
// rows have been inserted and deleted at random places (the records are let go of at once, as a history that is trimmed lets go of them). One tab-separated line for each, best of three.
int walkBench(const QString &path, int rowEdits, int colEdits, const QString &label)
{
    QString why;
    Workbook book;
    if (!book.openFile(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    CalcOptions options;
    options.keepApart = true;
    Calculation calc(book, options);
    if (!calc.load()) {
        err() << "edit-walk-bench: the workbook did not load\n";
        return 1;
    }
    const Sheet &s = calc.sheet(0);
    auto resident = [] {
        QFile f(QStringLiteral("/proc/self/status"));
        if (!f.open(QIODevice::ReadOnly)) return qint64(-1);
        for (const QByteArray &line : f.readAll().split('\n'))
            if (line.startsWith("VmRSS:")) return line.mid(6).trimmed().split(' ').value(0).toLongLong() / 1024;
        return qint64(-1);
    };
    out() << "label\twhen\tcells\tfor_each_ms\ttail_sliced_ms\tslices\tresident_mb\tchecksum\n";
    auto measure = [&](const char *when) {
        double each = 1e18, tail = 1e18;
        qint64 slices = 0;
        double sum = 0;
        for (int rep = 0; rep < 3; ++rep) {
            QElapsedTimer t;
            t.start();
            sum = 0;
            s.cells.forEach([&](quint32 row, quint32 col, const Cell &c) { sum += c.number + double(row) + double(col); });
            each = std::min(each, double(t.nsecsElapsed()) / 1e6);
            t.restart();
            CellStore::RowTailCursor cursor;
            qint64 steps = 0;
            slices = 0;
            while (!s.cells.forEachRowTailSome(&cursor, steps + 100000, &steps, [&](quint32 row, quint32 col, const Cell &c) {
                sum += c.number + double(row) + double(col);
                return false;
            }))
                ++slices;
            tail = std::min(tail, double(t.nsecsElapsed()) / 1e6);
        }
        out() << label << '\t' << when << '\t' << s.cells.cellCount() << '\t' << QString::number(each, 'f', 1) << '\t' << QString::number(tail, 'f', 1) << '\t' << slices << '\t' << resident() << '\t' << QString::number(sum, 'f', 0) << "\n";
    };
    measure("start");
    QRandomGenerator rng(7);
    const quint32 usedRows = std::max<quint32>(2, s.extentRows), usedCols = std::max<quint32>(2, s.extentCols);
    QElapsedTimer edits;
    edits.start();
    auto edit = [&](const StructEdit &e) {
        Calculation::EditRecord rec;
        if (calc.applyEdit(e, &rec)) calc.forgetLines(&rec);
    };
    for (int i = 0; i < colEdits; ++i) edit(rng.bounded(2) ? StructEdit::insertColumns(0, rng.bounded(usedCols), 1) : StructEdit::deleteColumns(0, rng.bounded(usedCols), 1));
    for (int i = 0; i < rowEdits; ++i) edit(rng.bounded(2) ? StructEdit::insertRows(0, rng.bounded(usedRows), 1) : StructEdit::deleteRows(0, rng.bounded(usedRows), 1));
    out() << "# " << colEdits << " column edits and " << rowEdits << " row edits took " << edits.elapsed() << " ms\n";
    measure("after edits");
    return 0;
}

// One structural edit at a time on an open workbook, at the top, in the middle, and at the end of the used cells, for rows and for columns: for each, the window's thread (its own processor time,
// from the call to the sheet drawn again, and the wall), the worker's processor time and the wall until it has settled, the counted work the model reports (cells and formulas looked at or
// moved, texts made), and the same for the undo of the edit. `label` names the workbook in each line. One tab-separated line per case, after a header.
int structBench(const QString &path, const QString &label, const QString &only)
{
    CrunchWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    w.resize(1280, 800);
    w.show();
    bool done = false;
    QObject::connect(&w, &CrunchWindow::calculationFinished, &w, [&] { done = true; });
    QElapsedTimer open;
    open.start();
    if (!w.openFile(path)) return 1;
    while (!done && open.elapsed() < 600000) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    if (!done) {
        err() << "edit-struct-bench: the workbook was not worked out in time\n";
        return 1;
    }
    auto vm = [](const char *key) {
        QFile f(QStringLiteral("/proc/self/status"));
        if (!f.open(QIODevice::ReadOnly)) return qint64(-1);
        for (const QByteArray &line : f.readAll().split('\n'))
            if (line.startsWith(key)) return line.mid(int(strlen(key)) + 1).trimmed().split(' ').value(0).toLongLong() / 1024;
        return qint64(-1);
    };
    out() << "# opened and worked out by " << open.elapsed() << " ms; resident " << vm("VmRSS") << " MB, peak " << vm("VmHWM") << " MB\n";
    crunch::EditSession *session = w.session();
    const Calculation *calc = w.calculation();
    grid::GridView *v = w.view();
    const int sheet = w.activeSheet();
    qint64 lastEpoch = -1;
    QObject::connect(&w.workbookLoader(), &crunch::WorkbookLoader::calcBatch, &w, [&](const std::shared_ptr<crunch::CalcBatch> &b) {
        if (b->last && !b->stopped) lastEpoch = b->epoch;
    });
    if (qEnvironmentVariableIntValue("JC_UNDO_MB") > 0) session->setUndoByteLimit(qint64(qEnvironmentVariableIntValue("JC_UNDO_MB")) << 20);   // (the limit that makes a big step ask first)
    qint64 workBefore = 0, textsBefore = 0, workDelta = 0, textsDelta = 0;
    Calculation::EditStats statsBefore, statsDelta;
    session->setHoldHook([&](Calculation *c, bool after) {
        const qint64 texts = c->sheet(sheet).formulaTexts.size();
        if (!after) {
            workBefore = c->editWork();
            textsBefore = texts;
            statsBefore = c->editStats();
        } else {
            workDelta = c->editWork() - workBefore;
            textsDelta = texts - textsBefore;
            statsDelta = c->editStats().since(statsBefore);
        }
    });
    const quint32 rows = std::max<quint32>(20, calc->sheet(sheet).extentRows), cols = std::max<quint32>(3, calc->sheet(sheet).extentCols);
    // (the data columns end where the sum column begins: the middle and the end are inside the data)
    struct Case {
        QString name;
        std::function<crunch::EditSession::Outcome()> run;
    };
    std::vector<Case> cases;
    const quint32 rowPlaces[3] = {0, rows / 2, rows - 2};
    const char *where[3] = {"top", "middle", "end"};
    for (int i = 0; i < 3; ++i) {
        const quint32 at = rowPlaces[i];
        cases.push_back({QStringLiteral("insert row %1").arg(QLatin1String(where[i])), [=] { return session->insertRows(sheet, at, 1); }});
        cases.push_back({QStringLiteral("delete row %1").arg(QLatin1String(where[i])), [=] { return session->deleteRows(sheet, at, 1); }});
    }
    const quint32 colPlaces[3] = {0, cols / 2, cols - 2};
    for (int i = 0; i < 3; ++i) {
        const quint32 at = colPlaces[i];
        cases.push_back({QStringLiteral("insert column %1").arg(QLatin1String(where[i])), [=] { return session->insertColumns(sheet, at, 1); }});
        cases.push_back({QStringLiteral("delete column %1").arg(QLatin1String(where[i])), [=] { return session->deleteColumns(sheet, at, 1); }});
    }
    // (a row put in, and one taken out, inside the first thousand rows: where the table of the workbooks that have one is, and, for the others, a row near the top)
    cases.push_back({QStringLiteral("insert row inside"), [=] { return session->insertRows(sheet, 500, 1); }});
    cases.push_back({QStringLiteral("delete row inside"), [=] { return session->deleteRows(sheet, 500, 1); }});
    // (a row put in past every cell: nothing is read or seen there, so for formulas that only see their own row it is rigid)
    cases.push_back({QStringLiteral("insert row past the end"), [=] { return session->insertRows(sheet, rows + 5, 1); }});
    // (cells shifted one by one: the column of formulas moved down by one, which moves every formula in it and leaves what they read where it is)
    cases.push_back({QStringLiteral("insert cells down"), [=] {
        office::sheet::CellRange shifted;
        shifted.valid = true;
        shifted.row1 = shifted.row2 = 0;
        shifted.col1 = shifted.col2 = cols - 1;
        return session->insertCells(sheet, shifted, true);
    }});
    // (JC_BENCH_BIG=1: rows deleted in their hundreds of thousands, which Undo of used to hold whole: a quarter of the sheet down, half the sheet or 500,000 rows, whichever is fewer)
    if (qEnvironmentVariableIntValue("JC_BENCH_BIG") > 0) cases.push_back({QStringLiteral("delete 500000 rows"), [=] { return session->deleteRows(sheet, rows / 4, std::min<quint32>(500000, rows / 2)); }});
    // (counted work, besides the times: what the model, the session, and the window each did; the columns after used_cols are for the edit, and then for its Undo)
    out() << "label\tcase\tdo_cpu_ms\tdo_wall_ms\tworker_cpu_ms\tsettle_ms\twork_counted\ttexts_made\tundo_cpu_ms\tundo_wall_ms\tundo_work\tused_rows\tused_cols"
          << "\tcells_moved\tcells_gone\tformulas_visited\tformulas_rewritten\tnodes_remade\tgraph_work\tnames_rewritten\trules_rewritten\tmarks_followed\tvalues_moved\tview_cells\tundo_bytes\tgraph_entries"
          << "\tu_cells_moved\tu_formulas_visited\tu_formulas_rewritten\tu_nodes_remade\tu_graph_work\tu_marks_followed\tu_values_moved\tu_view_cells"
          << "\tevals\tsettle_after_ms\tu_evals\tgraph_bytes\tph_setup\tph_candidates\tph_rewrite\tph_cells\tph_graph\tph_other\tph_write"
          << "\teager\tnames_again\ttables_again\ttexts_to_ids\tu_eager\tu_names_again\tu_tables_again\n";
    auto settle = [&](QElapsedTimer &wall) {
        QElapsedTimer waiting;
        waiting.start();
        while ((lastEpoch < w.workbookLoader().changesMade()) && waiting.elapsed() < 300000) QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        return double(wall.nsecsElapsed()) / 1e6;
    };
    for (const Case &c : cases) {
        if (!only.isEmpty() && !c.name.contains(only)) continue;
        w.undoStack()->clear();   // (so that the bytes the stack holds after the edit are the edit's alone)
        const qint64 evalsBefore = calc->evaluations();
        const qint64 workerBefore = w.workerCpuMicros();
        const qint64 marksBefore = session->marksFollowed(), movedBefore = w.values().relocated(), viewBefore = grid::GridSheet::viewCellsRebuiltForTest(), undoBytesBefore = session->undoBytes();
        statsDelta = Calculation::EditStats();
        QElapsedTimer wall;
        wall.start();
        const qint64 c0 = jp::threadCpuMicros();
        const crunch::EditSession::Outcome o = c.run();
        v->viewport()->repaint();
        const qint64 c1 = jp::threadCpuMicros();
        const Calculation::EditStats did = statsDelta;
        const qint64 marks = session->marksFollowed() - marksBefore, moved = w.values().relocated() - movedBefore, viewCells = grid::GridSheet::viewCellsRebuiltForTest() - viewBefore;
        const qint64 undoBytes = session->undoBytes() - undoBytesBefore, graphEntries = calc->graph().entryCount();
        const double wallMs = double(wall.nsecsElapsed()) / 1e6;
        const double settleMs = o.ok ? settle(wall) : 0.0;
        const qint64 evals = calc->evaluations() - evalsBefore;   // (the formulas the worker worked out for the edit)
        const double workerMs = double(w.workerCpuMicros() - workerBefore) / 1000.0;
        const qint64 work = workDelta, texts = textsDelta;
        double undoCpu = 0, undoWall = 0;
        qint64 undoWork = 0, uMarks = 0, uMoved = 0, uView = 0, uEvals = 0;
        Calculation::EditStats undid;
        if (o.ok) {
            QElapsedTimer uw;
            uw.start();
            const qint64 uMarksBefore = session->marksFollowed(), uMovedBefore = w.values().relocated(), uViewBefore = grid::GridSheet::viewCellsRebuiltForTest();
            statsDelta = Calculation::EditStats();
            const qint64 uEvalsBefore = calc->evaluations();
            const qint64 u0 = jp::threadCpuMicros();
            w.undoStack()->undo();
            v->viewport()->repaint();
            undoCpu = double(jp::threadCpuMicros() - u0) / 1000.0;
            undoWall = double(uw.nsecsElapsed()) / 1e6;
            undoWork = workDelta;
            undid = statsDelta;
            uMarks = session->marksFollowed() - uMarksBefore;
            uMoved = w.values().relocated() - uMovedBefore;
            uView = grid::GridSheet::viewCellsRebuiltForTest() - uViewBefore;
            settle(uw);
            uEvals = calc->evaluations() - uEvalsBefore;
        }
        out() << label << '\t' << c.name << (o.ok ? QString() : QStringLiteral(" (refused: %1)").arg(o.problem)) << '\t' << QString::number(double(c1 - c0) / 1000.0, 'f', 2) << '\t' << QString::number(wallMs, 'f', 2) << '\t'
              << QString::number(workerMs, 'f', 1) << '\t' << QString::number(settleMs, 'f', 1) << '\t' << work << '\t' << texts << '\t' << QString::number(undoCpu, 'f', 2) << '\t' << QString::number(undoWall, 'f', 2)
              << '\t' << undoWork << '\t' << rows << '\t' << cols << '\t' << did.cellsMoved << '\t' << did.cellsGone << '\t' << did.formulasVisited << '\t' << did.formulasRewritten << '\t' << did.nodesRemade
              << '\t' << did.graphWork << '\t' << did.namesRewritten << '\t' << did.rulesRewritten << '\t' << marks << '\t' << moved << '\t' << viewCells << '\t' << undoBytes << '\t' << graphEntries << '\t'
              << undid.cellsMoved << '\t' << undid.formulasVisited << '\t' << undid.formulasRewritten << '\t' << undid.nodesRemade << '\t' << undid.graphWork << '\t' << uMarks << '\t' << uMoved << '\t' << uView
              << '\t' << evals << '\t' << QString::number(settleMs - wallMs, 'f', 1) << '\t' << uEvals << '\t' << calc->graph().memoryBytes()
              << '\t' << QString::number(double(did.phaseMicros[6]) / 1000.0, 'f', 1) << '\t' << QString::number(double(did.phaseMicros[0]) / 1000.0, 'f', 1) << '\t' << QString::number(double(did.phaseMicros[1]) / 1000.0, 'f', 1)
              << '\t' << QString::number(double(did.phaseMicros[2]) / 1000.0, 'f', 1) << '\t' << QString::number(double(did.phaseMicros[3]) / 1000.0, 'f', 1) << '\t' << QString::number(double(did.phaseMicros[4]) / 1000.0, 'f', 1)
              << '\t' << QString::number(double(did.phaseMicros[5]) / 1000.0, 'f', 1) << '\t' << did.eagerFormulas << '\t' << did.namesRegisteredAgain << '\t' << did.tablesRegisteredAgain << '\t' << did.textsToIds
              << '\t' << undid.eagerFormulas << '\t' << undid.namesRegisteredAgain << '\t' << undid.tablesRegisteredAgain << '\n';
        out().flush();
    }
    out() << "# after the edits: resident " << vm("VmRSS") << " MB, peak " << vm("VmHWM") << " MB\n";
    if (qEnvironmentVariableIntValue("JC_BENCH_RECALC") > 0) {
        // a whole recalculation, with a delete that can still be undone present (2) or not (1): the worker's processor time, and the resident memory before and after (the allocator's free memory given back)
        if (qEnvironmentVariableIntValue("JC_BENCH_RECALC") > 1) {
            // (2: rows deleted, which keep their formulas while the step can be undone; 3: rows put in, which leave the order of the rows not the file's and nothing dead)
            const crunch::EditSession::Outcome o = qEnvironmentVariableIntValue("JC_BENCH_RECALC") > 2 ? session->insertRows(sheet, 100, 2) : session->deleteRows(sheet, 100, 2);
            QElapsedTimer quiet;
            quiet.start();
            settle(quiet);
            if (!o.ok) err() << "delete refused\n";
        }
#ifdef __GLIBC__
        malloc_trim(0);
#endif
        const qint64 before = vm("VmRSS");
        const qint64 cpu0 = w.workerCpuMicros();
        QElapsedTimer all;
        all.start();
        if (std::unique_ptr<crunch::WorkbookLoader::Hold> hold = w.workbookLoader().hold()) {
            hold->calculation()->invalidateAll();
            hold->changed();
        }
        settle(all);
#ifdef __GLIBC__
        malloc_trim(0);
#endif
        out() << "# whole recalculation: worker " << (w.workerCpuMicros() - cpu0) / 1000 << " ms, wall " << all.elapsed() << " ms; resident " << before << " -> " << vm("VmRSS") << " MB\n";
    }
    if (qEnvironmentVariableIntValue("JC_BENCH_TWELVE") > 0) {
        // twelve edits of rows and columns, none undone, and what the process holds after them and the steps hold (the table of 4.13: resident memory after 12 structural edits)
        w.undoStack()->clear();
        const qint64 residentBefore = vm("VmRSS"), heldBefore = session->undoBytes();
        const quint32 r = rows, c = cols;
        const std::function<crunch::EditSession::Outcome()> twelve[12] = {
            [=] { return session->insertRows(sheet, 10, 2); },       [=] { return session->deleteRows(sheet, r / 2, 1); },     [=] { return session->insertColumns(sheet, 1, 1); },
            [=] { return session->deleteColumns(sheet, 1, 1); },     [=] { return session->insertRows(sheet, r - 100, 3); },   [=] { return session->deleteRows(sheet, 20, 3); },
            [=] { return session->insertRows(sheet, 5, 1); },        [=] { return session->insertColumns(sheet, 0, 1); },     [=] { return session->deleteColumns(sheet, qEnvironmentVariableIntValue("JC_BENCH_TWELVE") > 2 ? c + 3 : c / 2, 1); },
            [=] { return session->deleteRows(sheet, r / 4, 5); },    [=] { return session->insertRows(sheet, r / 3, 1); },    [=] { return session->insertRows(sheet, 3, 1); }};
        QElapsedTimer all;
        all.start();
        int refused = 0, number = 0;
        QElapsedTimer wall;
        for (const auto &edit : twelve) {
            const crunch::EditSession::Outcome o = edit();
            if (!o.ok) ++refused;
            wall.start();
            settle(wall);
            if (qEnvironmentVariableIntValue("JC_BENCH_TWELVE") > 1) {
                const qint64 resident = vm("VmRSS");
#ifdef __GLIBC__
                malloc_trim(0);   // (what the allocator holds and nothing uses is given back: what is left is what the workbook holds)
#endif
#ifdef __GLIBC__
                const struct mallinfo2 mi = mallinfo2();
                out() << "#   heap in use " << (mi.uordblks + mi.hblkhd) / (1 << 20) << " MB (small " << mi.uordblks / (1 << 20) << ", large " << mi.hblkhd / (1 << 20) << ")\n";
#endif
                out() << "#   edit " << ++number << ": resident " << resident << " MB, " << vm("VmRSS") << " MB after the allocator gives back what is free, the steps hold " << (session->undoBytes() - heldBefore) / 1024 << " KB; stamps " << calc->stampCount() << ", graph " << calc->graph().memoryBytes() / (1 << 20)
                      << " MB, values " << w.values().count() << ", results kept by the session " << session->resultMarks() << "\n";
            }
        }
        wall.start();
        settle(wall);
        out() << "# twelve edits (" << refused << " refused): resident " << residentBefore << " -> " << vm("VmRSS") << " MB, peak " << vm("VmHWM") << " MB; the steps hold " << (session->undoBytes() - heldBefore) / 1024
              << " KB; made and settled in " << all.elapsed() << " ms\n";
        if (qEnvironmentVariableIntValue("JC_BENCH_TWELVE") > 1) {
            // (the steps let go of, the dead cells they held let go of, and the allocator's free memory given back: what is left is what the edited workbook itself holds)
            w.undoStack()->clear();
            while (session->purgeSome(qint64(1) << 40)) {}
            wall.start();
            settle(wall);
#ifdef __GLIBC__
            malloc_trim(0);
#endif
            out() << "# the steps let go of: resident " << vm("VmRSS") << " MB\n";
        }
    }
    return 0;
}

// What fill, paste, and sort cost on the real store after many edits of rows (docs/jeffcrunch-rowids.md, 4.15).
int fillBench(const QString &path, const QString &label, int edits, const QString &only)
{
    CrunchWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    w.resize(1280, 800);
    w.show();
    bool done = false;
    QObject::connect(&w, &CrunchWindow::calculationFinished, &w, [&] { done = true; });
    QElapsedTimer open;
    open.start();
    if (!w.openFile(path)) return 1;
    while (!done && open.elapsed() < 600000) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    if (!done) {
        err() << "edit-fill-bench: the workbook was not worked out in time\n";
        return 1;
    }
    crunch::EditSession *session = w.session();
    const Calculation *calc = w.calculation();
    const int sheet = w.activeSheet();
    qint64 lastEpoch = -1;
    QObject::connect(&w.workbookLoader(), &crunch::WorkbookLoader::calcBatch, &w, [&](const std::shared_ptr<crunch::CalcBatch> &b) {
        if (b->last && !b->stopped) lastEpoch = b->epoch;
    });
    session->setUndoByteLimit(qint64(64000) << 20);
    auto settle = [&] {
        QElapsedTimer waiting;
        waiting.start();
        while ((lastEpoch < w.workbookLoader().changesMade()) && waiting.elapsed() < 600000) QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
    };
    const quint32 rows0 = std::max<quint32>(20, calc->sheet(sheet).extentRows), cols = std::max<quint32>(3, calc->sheet(sheet).extentCols);
    QElapsedTimer all;
    all.start();
    QRandomGenerator rng(12345);
    int made = 0;
    for (int i = 0; i < edits; ++i) {
        const quint32 at = 1 + rng.bounded(rows0 / 2);
        const crunch::EditSession::Outcome o = (i % 3 == 2) ? session->deleteRows(sheet, at, 1) : session->insertRows(sheet, at, 1 + rng.bounded(2));
        if (o.ok) ++made;
        if (i % 50 == 49) settle();
    }
    settle();
    const quint32 rows = std::max<quint32>(20, calc->sheet(sheet).extentRows);
    out() << "# " << made << " row edits made in " << all.elapsed() << " ms; rows now " << rows << "; runs " << calc->sheet(sheet).cells.rowRunCount() << "\n";
    out() << "label\tcase\twindow_cpu_ms\twall_ms\tsettle_ms\ttexts_to_ids\tformulas_rewritten\tformulas_rebased\tok\n";
    auto run = [&](const QString &name, const std::function<crunch::EditSession::Outcome()> &f) {
        if (!only.isEmpty() && !name.contains(only)) return;
        const Calculation::EditStats before = calc->editStats();
        const qint64 rebasedBefore = calc->formulasRebased();
        QElapsedTimer wall;
        wall.start();
        const qint64 c0 = jp::threadCpuMicros();
        const crunch::EditSession::Outcome o = f();
        const qint64 c1 = jp::threadCpuMicros();
        const double wallMs = double(wall.nsecsElapsed()) / 1e6;
        settle();
        const double settleMs = double(wall.nsecsElapsed()) / 1e6;
        const Calculation::EditStats d = calc->editStats().since(before);
        out() << label << '\t' << name << '\t' << QString::number(double(c1 - c0) / 1000.0, 'f', 1) << '\t' << QString::number(wallMs, 'f', 1) << '\t' << QString::number(settleMs, 'f', 1) << '\t' << d.textsToIds << '\t'
              << d.formulasRewritten << '\t' << (calc->formulasRebased() - rebasedBefore) << '\t' << (o.ok ? QStringLiteral("ok") : o.problem) << '\n';
        out().flush();
    };
    const quint32 fcol = cols + 8, pcol = cols + 12;
    run(QStringLiteral("type"), [&] { return session->type(sheet, 0, fcol, QStringLiteral("=A1*2+B1")); });
    // (one step takes at most 100,000 cells: a column filled down all the rows, and a block of all the rows by 10 columns, are made in as many steps as that takes, each from the end of the last)
    run(QStringLiteral("fill formula down all rows"), [&] {
        crunch::EditSession::Outcome o;
        for (quint32 done = 0; done + 1 < rows;) {
            const quint32 chunk = std::min<quint32>(rows - 1 - done, 99999);
            office::sheet::CellRange src;
            src.valid = true;
            src.row1 = src.row2 = done;
            src.col1 = src.col2 = fcol;
            o = session->fill(sheet, src, crunch::EditSession::FillDirection::Down, chunk);
            if (!o.ok) break;
            done += chunk;
        }
        return o;
    });
    run(QStringLiteral("paste values all rows x 10"), [&] {
        crunch::EditSession::Outcome o;
        for (quint32 at = 0; at < rows;) {
            const quint32 pasteRows = std::min<quint32>(rows - at, 10000);   // (10 columns: 100,000 cells a step)
            QVector<QStringList> table;
            table.reserve(int(pasteRows));
            for (quint32 r = 0; r < pasteRows; ++r) {
                QStringList line;
                for (int c = 0; c < 10; ++c) line << QString::number(qint64(((at + r) * 7 + quint32(c) * 13) % 1000));
                table.push_back(line);
            }
            office::sheet::CellRange target;
            target.valid = true;
            target.row1 = target.row2 = at;
            target.col1 = target.col2 = pcol;
            o = session->pasteText(sheet, target, table, crunch::EditSession::PasteOptions());
            if (!o.ok) break;
            at += pasteRows;
        }
        return o;
    });
    run(QStringLiteral("sort descending"), [&] {
        office::sheet::CellRange range;
        range.valid = true;
        range.row1 = 0;
        range.row2 = std::min<quint32>(rows, 100000 / (cols + 1)) - 1;   // (the most one step takes is 100,000 cells)
        range.col1 = 0;
        range.col2 = cols;   // (the data and the column of sums that reads each row)
        crunch::SortSpec spec;
        crunch::SortLevel level;
        level.key = 0;
        level.descending = true;
        spec.levels.push_back(level);
        return session->sortRange(sheet, range, spec);
    });
    out() << "# done in " << all.elapsed() << " ms\n";
    return 0;
}

// What saving costs after structural edits: the workbook is saved untouched, then after 20 and after 200 more structural edits (a row or a column inserted or deleted at random places, none undone),
// each time timing the session's snapshot (what it has changed) and the save itself (the file made from the old file and the changes), and the size of the file.
int saveStructBench(const QString &path, const QString &label)
{
    CrunchWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    w.resize(1280, 800);
    w.show();
    bool done = false;
    QObject::connect(&w, &CrunchWindow::calculationFinished, &w, [&] { done = true; });
    QElapsedTimer open;
    open.start();
    if (!w.openFile(path)) return 1;
    while (!done && open.elapsed() < 900000) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    if (!done) return 1;
    crunch::EditSession *session = w.session();
    const Calculation *calc = w.calculation();
    const int sheet = w.activeSheet();
    const quint32 rows = std::max<quint32>(20, calc->sheet(sheet).extentRows), cols = std::max<quint32>(3, calc->sheet(sheet).extentCols);
    out() << "label\tstructural_edits\tsnapshot_ms\tsave_ms\tbytes\tpeak_mb\n";
    QRandomGenerator rng(7);
    int made = 0;
    auto measure = [&] {
        crunch::EditSession::Snapshot snap;
        QByteArray bytes;
        QString why;
        SaveReport report;
        QElapsedTimer t;
        t.start();
        if (!session->takeSnapshot(&snap, &why)) {
            out() << label << '\t' << made << "\tsnapshot failed: " << why << '\n';
            return;
        }
        const double snapMs = double(t.nsecsElapsed()) / 1e6;
        t.restart();
        if (!saveXlsx(*w.workbookLoader().result()->book, snap.changes, &bytes, &why, &report)) {
            out() << label << '\t' << made << "\tsave failed: " << why << '\n';
            return;
        }
        QFile st(QStringLiteral("/proc/self/status"));
        qint64 peak = -1;
        if (st.open(QIODevice::ReadOnly))
            for (const QByteArray &line : st.readAll().split('\n'))
                if (line.startsWith("VmHWM:")) peak = line.mid(6).trimmed().split(' ').value(0).toLongLong() / 1024;
        out() << label << '\t' << made << '\t' << QString::number(snapMs, 'f', 1) << '\t' << QString::number(double(t.nsecsElapsed()) / 1e6, 'f', 1) << '\t' << bytes.size() << '\t' << peak << '\n';
        out().flush();
    };
    measure();
    if (qEnvironmentVariableIntValue("JC_UNDO_MB") > 0) session->setUndoByteLimit(qint64(qEnvironmentVariableIntValue("JC_UNDO_MB")) << 20);
    std::vector<int> stops;
    for (const QString &part : qEnvironmentVariable("JC_SAVE_EDITS", QStringLiteral("20,200")).split(QLatin1Char(','))) stops.push_back(part.toInt());
    for (const int upTo : stops) {
        while (made < upTo) {
            const quint32 at = quint32(rng.bounded(int(rows - 5)));
            const quint32 colAt = quint32(rng.bounded(int(cols - 2)));
            crunch::EditSession::Outcome o;
            switch (rng.bounded(4)) {
            case 0: o = session->insertRows(sheet, at, 1); break;
            case 1: o = session->deleteRows(sheet, at, 1); break;
            case 2: o = session->insertColumns(sheet, colAt, 1); break;
            default: o = session->deleteColumns(sheet, colAt, 1); break;
            }
            if (o.ok) ++made;
            else if (rng.bounded(50) == 0) break;
        }
        QElapsedTimer waiting;
        waiting.start();
        while (w.isCalculating() && waiting.elapsed() < 600000) QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        measure();
    }
    return 0;
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
        calc->sheet(sheet).cells.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) {
            if (candidates.size() >= 100000 || ++looked > 5000000) return;
            if (c.type != CellType::Number || c.hasFormula() || (c.flags & (ArrayMember | ArrayFormula))) return;
            std::vector<quint32> r;
            calc->readersOf(placeArea(sheet, cellRow, cellCol), &r);
            if (!r.empty()) candidates.push_back({cellRow, cellCol});
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
    // and on the sheet with the most cells, if it is not the one in view (a workbook of several: the window is brought to it)
    int biggest = sheet;
    for (int i = 0; i < calc->sheetCount(); ++i)
        if (calc->sheet(i).cells.cellCount() > calc->sheet(biggest).cells.cellCount()) biggest = i;
    if (biggest != sheet && calc->sheet(biggest).cells.cellCount() > 4 * calc->sheet(sheet).cells.cellCount()) {
        out() << "the sheet with the most cells: " << w.calculation()->sheet(biggest).cells.cellCount() << " cells\n";
        w.setActiveSheet(biggest);
        QCoreApplication::processEvents();
        structureBench(w, biggest);
    }
    return 0;
}

// The names of a workbook, edited and saved, and read back (docs/jeffcrunch.md): every name must come back as it was but for the two changes.
int namesRoundtripOne(const QString &path)
{
    auto line = [&](const QString &result, qint64 names = 0, qint64 namesSame = 0, qint64 bytesSame = 0, qint64 cells = 0, qint64 cellsSame = 0) {
        out() << "names-roundtrip\t" << path << '\t' << result << '\t' << names << '\t' << namesSame << '\t' << bytesSame << '\t' << cells << '\t' << cellsSame << "\n";
        out().flush();
    };
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        line(QStringLiteral("not opened"));
        return 1;
    }
    const QByteArray original = file.readAll();
    file.close();
    Opened w;
    if (!w.open(path)) {
        line(QStringLiteral("not opened"));
        return 1;
    }
    if (!w.loader.result()->problem.isEmpty()) {
        line(QStringLiteral("opened in part"));
        return 2;
    }
    Workbook &book = *w.loader.result()->book;
    const QVector<DefinedName> names = book.info().definedNames;
    if (names.isEmpty()) {
        line(QStringLiteral("no names"));
        return 0;
    }
    crunch::EditSession *session = w.session.get();
    session->setLocale(QLocale::c());
    int firstSheet = -1;
    for (int si = 0; si < book.info().sheets.size() && firstSheet < 0; ++si)
        if (book.info().sheets[si].kind == SheetKind::Worksheet && !book.info().sheets[si].deleted) firstSheet = si;
    QStringList notes;
    QString probeName = QStringLiteral("JcProbeName");
    for (int k = 0; session->names().find(-1, probeName) >= 0; ++k) probeName = QStringLiteral("JcProbeName%1").arg(k);
    int changedId = -1;
    QString changedName;
    if (firstSheet >= 0) {
        crunch::EditSession::NameDraft d;
        d.name = probeName;
        d.text = QStringLiteral("'") + QString(book.info().sheets[firstSheet].name).replace(QLatin1Char('\''), QLatin1String("''")) + QStringLiteral("'!$A$1");
        d.comment = QStringLiteral("probe <&> \"comment\"");
        const auto o = session->defineName(d);
        if (!o.ok) notes << QStringLiteral("probe refused: ") + o.problem.left(60);
    }
    for (int id = 0; id < session->names().slots() && changedId < 0; ++id) {
        if (!session->nameListed(id) || session->names().at(id).name == probeName) continue;
        crunch::EditSession::NameDraft d;
        const crunch::NameEntry &e = session->names().at(id);
        d.name = e.name;
        d.scope = e.scope;
        d.hidden = e.hidden;
        d.comment = QStringLiteral("changed by the check");
        d.text = session->nameTextOf(id);
        const auto o = session->changeName(id, d);
        if (o.ok) {
            changedId = id;
            changedName = e.name;
        }
    }
    // (a name that no formula uses leaves nothing to work out, and the worker then sends nothing: it is idle, and has nothing waiting)
    if (!w.wait([&] {
            if (!w.loader.result()->calcIdle.load()) return false;
            auto hold = w.loader.hold();
            return hold && !hold->calculation()->isDirty();
        }, 280000)) {
        auto hold = w.loader.hold();
        line(QStringLiteral("the worker did not finish (idle %1, waiting %2)").arg(w.loader.result()->calcIdle.load()).arg(hold ? hold->calculation()->dirtyCount() : -1));
        return 3;
    }
    crunch::EditSession::Snapshot snap;
    QString why;
    QByteArray bytes;
    if (!session->takeSnapshot(&snap, &why)) {
        line(QStringLiteral("snapshot: ") + why);
        return 3;
    }
    if (!saveXlsx(book, snap.changes, &bytes, &why)) {
        line(QStringLiteral("save failed: ") + why);
        return 3;
    }
    if (const QString keep = qEnvironmentVariable("JC_NAMES_KEEP"); !keep.isEmpty()) {
        QFile kept(keep);
        if (kept.open(QIODevice::WriteOnly)) kept.write(bytes);
    }
    Read after(bytes);
    if (!after.ok) {
        line(QStringLiteral("the saved file does not open: ") + after.why);
        return 3;
    }
    // every name of the file, by scope and name: the same as it was, but the two that were changed
    auto key = [](const DefinedName &n) { return QString::number(n.localSheet) + QLatin1Char('|') + n.name.toUpper(); };
    QHash<QString, DefinedName> now;
    for (const DefinedName &n : after.book.info().definedNames)
        if (!now.contains(key(n))) now.insert(key(n), n);
    qint64 same = 0;
    QStringList diffs = notes;
    auto add = [&](const QString &what) {
        if (diffs.size() < 4 && !diffs.contains(what)) diffs << what;
    };
    QSet<QString> seen;
    for (const DefinedName &n : names) {
        if (seen.contains(key(n))) {
            ++same;   // (a name the file repeats: the first stands)
            continue;
        }
        seen.insert(key(n));
        const auto at = now.constFind(key(n));
        if (at == now.constEnd()) {
            add(QStringLiteral("a name went"));
            continue;
        }
        const bool changed = changedId >= 0 && n.name == changedName && n.localSheet == session->names().at(changedId).scope;
        if (at->name != n.name) add(QStringLiteral("a name's capitals changed"));
        else if (at->hidden != n.hidden) add(QStringLiteral("hidden changed"));
        else if (at->ref != n.ref && !(changed && at->ref.trimmed() == n.ref.trimmed())) add(QStringLiteral("what a name stands for changed"));
        else if (at->comment != (changed ? QStringLiteral("changed by the check") : n.comment)) add(QStringLiteral("a comment changed"));
        else ++same;
    }
    if (!now.contains(QStringLiteral("-1|") + probeName.toUpper()) && firstSheet >= 0 && diffs.isEmpty()) add(QStringLiteral("the name that was made is not there"));
    if (qint64(after.book.info().definedNames.size()) != qint64(names.size()) + (firstSheet >= 0 ? 1 : 0)) add(QStringLiteral("the number of names"));
    // the names that were not touched: their elements are the file's, to the byte
    qint64 bytesSame = 0, untouched = 0;
    {
        const QByteArray before = rawEntries(original).value(QStringLiteral("xl/workbook.xml"));
        QByteArray inflatedBefore, inflatedAfter;
        QString err;
        Package pa, pb;
        pa.readLazy(original, &err);
        pb.readLazy(bytes, &err);
        const QByteArray a = pa.part(book.mainPart()), b = pb.part(after.book.mainPart());
        static const QRegularExpression element(QStringLiteral("<(?:[A-Za-z0-9]+:)?definedName\\b[^>]*?(?:/>|>.*?</(?:[A-Za-z0-9]+:)?definedName>)"), QRegularExpression::DotMatchesEverythingOption);
        const QString textA = QString::fromUtf8(a), textB = QString::fromUtf8(b);
        QRegularExpressionMatchIterator it = element.globalMatch(textA);
        while (it.hasNext()) {
            const QString e = it.next().captured(0);
            if (changedId >= 0 && e.contains(QLatin1String("\"") + changedName.toHtmlEscaped() + QLatin1String("\""))) continue;
            ++untouched;
            if (textB.contains(e)) ++bytesSame;
        }
        Q_UNUSED(before);
    }
    if (bytesSame != untouched) add(QStringLiteral("the element of a name that was not touched changed"));
    // every cell
    qint64 cells = 0, cellsSame = 0;
    for (int si = 0; si < book.info().sheets.size(); ++si) {
        Sheet sa, sb;
        Workbook &x = book;
        x.readSheet(si, &sa);
        after.book.readSheet(si, &sb);
        sa.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            ++cells;
            const Cell *e = sb.cells.find(row, col);
            if (e && hashCell(sa, x.sharedStrings(), c) == hashCell(sb, after.book.sharedStrings(), *e)) ++cellsSame;
        });
        if (sa.cells.cellCount() != sb.cells.cellCount()) add(QStringLiteral("cell count"));
    }
    if (cellsSame != cells) add(QStringLiteral("a cell changed"));
    line(diffs.isEmpty() ? QStringLiteral("ok") : diffs.join(QStringLiteral("; ")), qint64(names.size()), same, bytesSame, cells, cellsSame);
    return diffs.isEmpty() ? 0 : 2;
}

// The tables of a workbook, edited and saved, and read back: a file that is only opened and saved leaves every table part as it was (to the byte); with a row put in each table, every table
// comes back with its name, columns, totals, style, and filter, and every cell that held a constant holds it still (where the rows put in moved it).
int tablesRoundtripOne(const QString &path)
{
    auto line = [&](const QString &result, qint64 tables = 0, qint64 tablesOk = 0, qint64 partsSame = 0, qint64 parts = 0, qint64 cells = 0, qint64 cellsSame = 0) {
        out() << "tables-roundtrip\t" << path << '\t' << result << '\t' << tables << '\t' << tablesOk << '\t' << partsSame << '\t' << parts << '\t' << cells << '\t' << cellsSame << "\n";
        out().flush();
    };
    QByteArray original;
    QString why;
    if (!readWholeFile(path, &original, &why)) {
        line(QStringLiteral("not opened"));
        return 1;
    }
    // (1) opened and saved with nothing changed: the table parts are the file's
    qint64 parts = 0, partsSame = 0;
    {
        Read before(original);
        if (!before.ok) {
            line(QStringLiteral("not opened"));
            return 1;
        }
        QByteArray saved;
        if (!saveXlsx(before.book, SaveChanges(), &saved, &why)) {
            line(QStringLiteral("save failed: ") + why);
            return 3;
        }
        const QMap<QString, QByteArray> a = rawEntries(original), b = rawEntries(saved);
        for (auto it = a.constBegin(); it != a.constEnd(); ++it)
            if (it.key().startsWith(QLatin1String("xl/tables/")) && !it.key().contains(QLatin1String("_rels"))) {
                ++parts;
                if (b.contains(it.key()) && b.value(it.key()) == it.value()) ++partsSame;
            }
    }
    Opened w;
    if (!w.open(path)) {
        line(QStringLiteral("not opened"));
        return 1;
    }
    if (!w.loader.result()->problem.isEmpty()) {
        line(QStringLiteral("opened in part"));
        return 2;
    }
    Workbook &book = *w.loader.result()->book;
    const QVector<Table> tables = book.tables();
    if (tables.isEmpty()) {
        line(QStringLiteral("no tables"), 0, 0, partsSame, parts);
        return 0;
    }
    crunch::EditSession *session = w.session.get();
    session->setLocale(QLocale::c());
    struct Insert {
        int sheet;
        quint32 row;
    };
    std::vector<Insert> inserted;
    QStringList notes;
    // a row put in each table (before its last row of data), from the table that is lowest on its sheet up
    std::vector<int> order;
    for (int i = 0; i < tables.size(); ++i) order.push_back(i);
    std::sort(order.begin(), order.end(), [&](int a, int b) { return tables[a].sheet != tables[b].sheet ? tables[a].sheet < tables[b].sheet : tables[a].row1 > tables[b].row1; });
    QSet<QString> grown;
    for (const int i : order) {
        const Table &now = book.tables()[i];
        if (!now.hasData()) continue;
        const quint32 at = quint32(now.dataLast());
        const int sheet = now.sheet;
        const QString name = now.name;
        const auto o = session->insertRows(sheet, at, 1);
        if (o.ok) {
            inserted.push_back({sheet, at});
            grown.insert(name);
        } else {
            notes << QStringLiteral("a row was refused: ") + o.problem.left(50);
        }
    }
    if (!w.wait([&] {
            if (!w.loader.result()->calcIdle.load()) return false;
            auto hold = w.loader.hold();
            return hold && !hold->calculation()->isDirty();
        }, 280000)) {
        line(QStringLiteral("the worker did not finish"));
        return 3;
    }
    crunch::EditSession::Snapshot snap;
    QByteArray bytes;
    if (!session->takeSnapshot(&snap, &why)) {
        line(QStringLiteral("snapshot: ") + why);
        return 3;
    }
    if (!saveXlsx(book, snap.changes, &bytes, &why)) {
        line(QStringLiteral("save failed: ") + why);
        return 3;
    }
    if (const QString keep = qEnvironmentVariable("JC_TABLES_KEEP"); !keep.isEmpty()) {
        QFile kept(keep);
        if (kept.open(QIODevice::WriteOnly)) kept.write(bytes);
    }
    Read after(bytes);
    if (!after.ok) {
        line(QStringLiteral("the saved file does not open: ") + after.why);
        return 3;
    }
    QStringList diffs = notes;
    auto add = [&](const QString &what) {
        if (diffs.size() < 4 && !diffs.contains(what)) diffs << what;
    };
    // every table, by name: as it was, longer by the row put in it
    qint64 tablesOk = 0;
    QSet<int> ids;
    const QVector<Table> &back = after.book.tables();
    if (back.size() != tables.size()) add(QStringLiteral("the number of tables"));
    for (const Table &t : tables) {
        const Table *r = nullptr;
        for (const Table &c : back)
            if (c.name == t.name) r = &c;
        if (!r) {
            add(QStringLiteral("a table went"));
            continue;
        }
        bool ok = true;
        if (r->columns != t.columns) ok = false, add(QStringLiteral("columns"));
        if (r->totalsRows != t.totalsRows || r->headerRows != t.headerRows) ok = false, add(QStringLiteral("header or totals"));
        if (r->styleName != t.styleName || r->showFirstColumn != t.showFirstColumn || r->showLastColumn != t.showLastColumn || r->showRowStripes != t.showRowStripes || r->showColumnStripes != t.showColumnStripes)
            ok = false, add(QStringLiteral("style"));
        if (r->filterButtons != t.filterButtons || r->filter.columns.size() != t.filter.columns.size()) ok = false, add(QStringLiteral("filter"));
        if (r->col1 != t.col1 || r->col2 != t.col2 || r->row2 < t.row2 || r->row2 > t.row2 + quint32(inserted.size())) ok = false, add(QStringLiteral("range"));
        if (grown.contains(t.name) && r->row2 == t.row2 && r->sheet == t.sheet && inserted.size() == 1) ok = false, add(QStringLiteral("the table did not grow"));
        for (int c = 0; c < t.info.size() && c < r->info.size(); ++c)
            if (r->info[c].totalsFunction != t.info[c].totalsFunction || r->info[c].totalsLabel != t.info[c].totalsLabel || r->info[c].totalsFormula != t.info[c].totalsFormula) ok = false, add(QStringLiteral("totals"));
        if (ids.contains(r->id)) ok = false, add(QStringLiteral("two tables with one id"));
        ids.insert(r->id);
        if (ok) ++tablesOk;
    }
    // every constant of every sheet: still there, where the rows put in moved it
    qint64 cells = 0, cellsSame = 0;
    for (int si = 0; si < book.info().sheets.size(); ++si) {
        if (book.info().sheets[si].kind != SheetKind::Worksheet) continue;
        Sheet sa, sb;
        book.readSheet(si, &sa);
        after.book.readSheet(si, &sb);
        sa.cells.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) {
            if (c.hasFormula()) return;
            ++cells;
            quint32 row = cellRow;
            for (const Insert &in : inserted)
                if (in.sheet == si && row >= in.row) ++row;
            const Cell *e = sb.cells.find(row, cellCol);
            if (e && !e->hasFormula() && hashCell(sa, book.sharedStrings(), c) == hashCell(sb, after.book.sharedStrings(), *e)) ++cellsSame;
        });
    }
    if (cellsSame != cells) add(QStringLiteral("a cell changed"));
    line(diffs.isEmpty() ? QStringLiteral("ok") : diffs.join(QStringLiteral("; ")), qint64(tables.size()), tablesOk, partsSame, parts, cells, cellsSame);
    return diffs.isEmpty() ? 0 : 2;
}

} // namespace

namespace jp::crunchtool {

// jcrunchtool edit-recalc-check <file> [edits=6] [seed=1]: the workbook is opened as the window does and worked out; a few edits of whole rows and columns (inserted and deleted, in and past the used cells)
// are made through the session, each worked out by the worker; then every formula's value is compared with a whole new calculation of the same cells. What the edits left stale shows as a mismatch.
// Formulas whose value differs between two whole calculations with no edit between (the time, RAND, and what reads them) are left out. One tab-separated line: path, formulas, edits made, refused,
// formulas left out, compared, mismatches, and the first mismatch.
int editRecalcCheck(const QString &path, int edits, quint32 seed)
{
    Opened o;
    if (!o.open(path)) {
        out() << path << "\tnot opened\n";
        return 1;
    }
    Calculation *calc = o.loader.result()->calc.get();
    auto snapshot = [&] {
        std::unordered_map<quint64, jp::calc::Value> values;
        calc->forEachFormula([&](const Calculation::FormulaCell &f) {
            if (f.cell) values[crunch::EditSession::keyOf(f.sheet, f.cell->rowId, f.cell->colId)] = f.ours;
        });
        return values;
    };
    auto sameValue = [](const jp::calc::Value &a, const jp::calc::Value &b) { return (a.isNumber() && b.isNumber() && jp::calc::sameTo15Digits(a.number(), b.number())) || a == b; };
    auto workOutAgain = [&] {
        o.beginEdit();
        o.loader.recalculate();
        return o.settle();
    };
    const std::unordered_map<quint64, jp::calc::Value> first = snapshot();
    if (!workOutAgain()) {
        out() << path << "\tnot settled\n";
        return 1;
    }
    std::unordered_set<quint64> unstable;
    const std::unordered_map<quint64, jp::calc::Value> second = snapshot();
    for (const auto &entry : first) {
        const auto again = second.find(entry.first);
        if (again == second.end() || !sameValue(entry.second, again->second)) unstable.insert(entry.first);
    }
    const bool verbose = qEnvironmentVariableIsSet("JC_ERC_VERBOSE");
    QRandomGenerator rng(seed);
    QRandomGenerator cells(seed ^ 0x9E3779B9u);
    const bool cellEdits = qEnvironmentVariable("JC_ERC_CELLS") != QLatin1String("0");
    std::vector<int> sheets;
    for (int si = 0; si < calc->sheetCount(); ++si)
        if (o.loader.result()->book->info().sheets[si].kind == SheetKind::Worksheet && calc->sheet(si).extentRows > 0) sheets.push_back(si);
    int made = 0, refused = 0;
    for (int i = 0; i < edits && !sheets.empty(); ++i) {
        const int sheet = sheets[size_t(rng.bounded(quint32(sheets.size())))];
        const quint32 usedRows = std::max<quint32>(3, calc->sheet(sheet).extentRows), usedCols = std::max<quint32>(3, calc->sheet(sheet).extentCols);
        const quint32 count = 1 + rng.bounded(2);
        const bool rows = rng.bounded(2) == 0;
        const quint32 at = rows ? rng.bounded(usedRows + 2) : rng.bounded(usedCols + 2);
        const bool insert = rng.bounded(5) < 3;
        // Every other edit is of cells (a sort, a fill, a paste, a cut and paste, cells shifted), from a generator of its own so that the others are the seed's as they were (JC_ERC_CELLS=0 leaves them out).
        const int cellKind = cellEdits && cells.bounded(2) == 0 ? 1 + int(cells.bounded(7)) : 0;
        o.beginEdit();
        const auto changesBefore = o.loader.changesMade();
        crunch::EditSession::Outcome result;
        QString words;
        if (cellKind != 0) {
            const quint32 r1 = cells.bounded(std::min<quint32>(usedRows, 3000)), c1 = cells.bounded(std::min<quint32>(usedCols, 30));
            const quint32 h = 2 + cells.bounded(40), w = 1 + cells.bounded(4);
            office::sheet::CellRange block;
            block.valid = true;
            block.row1 = r1;
            block.col1 = c1;
            block.row2 = std::min<quint32>(r1 + h - 1, jp::calc::kGridRows - 1);
            block.col2 = std::min<quint32>(c1 + w - 1, jp::calc::kGridCols - 1);
            const quint32 tr = cells.bounded(std::min<quint32>(usedRows + 5, 3000)), tc = cells.bounded(std::min<quint32>(usedCols + 3, 30));
            const bool flag = cells.bounded(2) == 0;
            switch (cellKind) {
            case 1: {
                crunch::SortSpec spec;
                crunch::SortLevel level;
                level.key = int(cells.bounded(w));
                level.descending = flag;
                spec.levels.push_back(level);
                words = QStringLiteral("sort");
                result = o.session->sortRange(sheet, block, spec);
                break;
            }
            case 2: case 3: {
                office::sheet::CellRange src = block;
                const bool down = flag;
                if (down) src.row2 = src.row1 + (cellKind == 2 ? 0 : 1);
                else src.col2 = src.col1 + (cellKind == 2 ? 0 : 1);
                words = QStringLiteral("fill");
                result = o.session->fill(sheet, src, down ? crunch::EditSession::FillDirection::Down : crunch::EditSession::FillDirection::Right, h);
                break;
            }
            case 4: {
                const crunch::ClipData clip = o.session->copyCells(sheet, block, false);
                office::sheet::CellRange target = block;
                target.row1 = target.row2 = tr;
                target.col1 = target.col2 = tc;
                words = QStringLiteral("paste");
                result = o.session->paste(sheet, target, clip, crunch::EditSession::PasteOptions());
                break;
            }
            case 5: {
                words = QStringLiteral("cut and paste");
                result = o.session->moveCells(sheet, block, sheet, tr, tc);
                break;
            }
            case 6: words = QStringLiteral("insert cells"); result = o.session->insertCells(sheet, block, flag); break;
            default: words = QStringLiteral("delete cells"); result = o.session->deleteCells(sheet, block, flag); break;
            }
            if (verbose) err() << words << " sheet " << sheet << " rows " << block.row1 << '-' << block.row2 << " columns " << block.col1 << '-' << block.col2 << (result.ok ? "" : " (refused)") << '\n';
        } else {
            if (rows) result = insert ? o.session->insertRows(sheet, at, count) : o.session->deleteRows(sheet, at, count);
            else result = insert ? o.session->insertColumns(sheet, at, count) : o.session->deleteColumns(sheet, at, count);
            if (verbose) err() << (rows ? "row" : "column") << (insert ? " insert " : " delete ") << "sheet " << sheet << " at " << at << " count " << count << (result.ok ? "" : " (refused)") << '\n';
        }
        if (!result.ok) {
            ++refused;
            continue;
        }
        ++made;
        if (cellKind != 0 && o.loader.changesMade() == changesBefore) continue;   // (a sort of cells in order, a fill of what is there: nothing was changed, so no batch comes)
        if (!o.settle()) {
            out() << path << "\tnot settled after an edit\n";
            return 1;
        }
    }
    const std::unordered_map<quint64, jp::calc::Value> edited = snapshot();
    if (!workOutAgain()) {
        out() << path << "\tnot settled\n";
        return 1;
    }
    const std::unordered_map<quint64, jp::calc::Value> whole = snapshot();
    qint64 compared = 0, mismatches = 0;
    QString firstMismatch;
    for (const auto &entry : edited) {
        if (unstable.count(entry.first)) continue;
        const auto again = whole.find(entry.first);
        if (again == whole.end()) continue;
        ++compared;
        if (!sameValue(entry.second, again->second)) {
            if (++mismatches == 1 || (verbose && mismatches <= 5)) {
                const int sheet = int(entry.first >> 40);
                const Cell *cell = calc->sheet(sheet).cells.findById(quint32((entry.first >> 16) & 0xFFFFFF), quint32(entry.first & 0xFFFF));
                const quint32 row = calc->sheet(sheet).cells.rowPlaceOf(quint32((entry.first >> 16) & 0xFFFFFF)), col = calc->sheet(sheet).cells.colPlaceOf(quint32(entry.first & 0xFFFF));
                const QString text = cell && row != CellStore::kNoLine && col != CellStore::kNoLine ? calc->content(sheet, row, col).formula : QString();
                const QString what = QStringLiteral("sheet %1 %2 (=%3): %4 against %5").arg(sheet).arg(row == CellStore::kNoLine ? QStringLiteral("?") : cellName(row, col), text, entry.second.debug(), again->second.debug());
                if (mismatches == 1) firstMismatch = what;
                if (verbose) err() << "mismatch: " << what << '\n';
            }
        }
    }
    out() << path << '\t' << first.size() << '\t' << made << '\t' << refused << '\t' << unstable.size() << '\t' << compared << '\t' << mismatches << '\t' << firstMismatch << '\n';
    return mismatches == 0 ? 0 : 3;
}

int editMain(const QStringList &args)
{
    if (!args.isEmpty() && args[0].startsWith(QLatin1String("model-"))) return ::jp::structmodel::command(args);
    if (args.size() >= 2 && args.size() <= 3 && args[0] == QLatin1String("edit-bench")) return editBench(args[1], args.value(2, QStringLiteral("200")).toInt(), 0, 0, 0);
    if (args.size() >= 3 && args.size() <= 5 && args[0] == QLatin1String("edit-fill-bench")) return fillBench(args[1], args[2], args.value(3, QStringLiteral("1000")).toInt(), args.value(4));
    if (args.size() == 3 && args[0] == QLatin1String("edit-save-struct-bench")) return saveStructBench(args[1], args[2]);
    if (args.size() == 2 && args[0] == QLatin1String("edit-mem-model")) {
        // the heap in use after a column in the middle of the data is deleted, in the model alone (no window, no session): where the memory the edit leaves is
        QString why;
        Workbook book;
        if (!book.openFile(args[1], &why)) return 1;
        CalcOptions options;
        options.keepApart = true;
        Calculation calc(book, options);
        if (!calc.load()) return 1;
        auto heap = [](const char *what) {
#ifdef __GLIBC__
            malloc_trim(0);
            const struct mallinfo2 mi = mallinfo2();
            out() << what << ": heap in use " << (mi.uordblks + mi.hblkhd) / (1 << 20) << " MB\n";
#else
            Q_UNUSED(what);
#endif
        };
        auto finish = [&] { while (!calc.recalculate().finished) {} };
        finish();
        heap("worked out");
        Calculation::EditRecord rec;
        calc.applyEdit(StructEdit::deleteColumns(0, 2, 1), &rec);
        heap("column deleted");
        finish();
        heap("worked out again");
        return 0;
    }
    if (args.size() >= 2 && args.size() <= 4 && args[0] == QLatin1String("edit-walk-bench")) return walkBench(args[1], args.value(2, QStringLiteral("1000")).toInt(), args.value(3, QStringLiteral("10")).toInt(), QStringLiteral("walk"));
    if (args.size() >= 3 && args.size() <= 4 && args[0] == QLatin1String("edit-struct-bench")) return structBench(args[1], args[2], args.value(3));
    if (args.size() >= 1 && args.size() <= 5 && args[0] == QLatin1String("edit-bench-make"))
        return editBench(QString(), args.value(4, QStringLiteral("200")).toInt(), args.value(1, QStringLiteral("100000")).toInt(), args.value(2, QStringLiteral("20")).toInt(), args.value(3, QStringLiteral("0")).toInt());
    if (args.size() >= 2 && args.size() <= 4 && args[0] == QLatin1String("edit-recalc-check")) return editRecalcCheck(args[1], args.value(2, QStringLiteral("6")).toInt(), args.value(3, QStringLiteral("1")).toUInt());
    if (args.size() == 2 && args[0] == QLatin1String("roundtrip-one")) return roundtripOne(args[1]);
    if (args.size() == 2 && args[0] == QLatin1String("edit-names-roundtrip")) return namesRoundtripOne(args[1]);
    if (args.size() == 2 && args[0] == QLatin1String("edit-tables-roundtrip")) return tablesRoundtripOne(args[1]);
    if ((args.size() == 3 || args.size() == 4) && args[0] == QLatin1String("roundtrip")) return roundtripSweep(args[1], args[2], args.value(3));
    return -1;
}

} // namespace jp::crunchtool

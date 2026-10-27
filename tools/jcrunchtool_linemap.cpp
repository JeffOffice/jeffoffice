// The micro-bench of the order of a sheet's rows as ids (libs/base/calc/linemap.h, docs/jeffcrunch-rowids.md 4.2):
//   jcrunchtool linemap-bench [lines]   the time of each operation, in nanoseconds, in a map of `lines` lines (1,000,000 unless said) that holds 1, 1,000, and 20,000 runs,
//                                       and the bytes a run takes. The runs are made by random single edits, as a session makes them. One table: a row for each operation,
//                                       a column for each run count. (Times are for looking at; the tests count work and never time it.)
#include "calc/linemap.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QStringList>
#include <QTextStream>

#include <algorithm>
#include <vector>

using namespace jp::calc;

namespace jp::crunchtool {

namespace {

QTextStream &out()
{
    static QTextStream s(stdout);
    return s;
}

// Runs a batch and gives the nanoseconds for one call.
template <class F>
double perCall(int calls, F &&f)
{
    QElapsedTimer t;
    t.start();
    for (int i = 0; i < calls; ++i) f(i);
    return double(t.nsecsElapsed()) / calls;
}

struct Column {
    size_t runs = 0;
    std::vector<double> ns;
    double bytesPerRun = 0;
};

Column measure(uint32_t lines, size_t wantRuns)
{
    Column col;
    QRandomGenerator rng(uint32_t(wantRuns) * 31u + 7u);
    LineMapCore m(LineLimits::rows(), lines);
    if (wantRuns <= 1) {
        // one run that is not the identity: an erase and its Undo
        LineEdit e;
        m.erase(10, 5, &e);
        m.undo(e);
        m.forget(e);
    }
    while (m.runCount() < wantRuns) {
        LineEdit e;
        const uint32_t n = 1 + rng.bounded(3);
        const bool erase = m.size() > 10 && rng.bounded(2) == 0;
        if (erase) m.erase(rng.bounded(m.size() - n), n, &e);
        else m.insert(rng.bounded(m.size() + 1), n, &e);
    }
    col.runs = m.runCount();
    col.bytesPerRun = double(m.runBytes()) / double(m.runCount());
    std::vector<uint32_t> ps(4096), ids(4096);
    for (size_t i = 0; i < ps.size(); ++i) {
        ps[i] = rng.bounded(m.size());
        ids[i] = m.idAt(ps[i]);
    }
    volatile uint32_t sink = 0;
    const int reads = 2000000;
    col.ns.push_back(perCall(reads, [&](int i) { sink = m.idAt(ps[size_t(i) & 4095]); }));
    col.ns.push_back(perCall(reads, [&](int i) { sink = m.posOf(ids[size_t(i) & 4095]); }));
    {
        LineHint h;
        col.ns.push_back(perCall(reads, [&](int i) { sink = m.idAt(uint32_t(i) % m.size(), &h); }));
    }
    {
        LineHint h;
        col.ns.push_back(perCall(reads, [&](int i) { sink = m.posOf(uint32_t(i) % 900000u, &h); }));
    }
    {
        std::vector<IdRun> runs;
        col.ns.push_back(perCall(20000, [&](int i) {
            const uint32_t a = ps[size_t(i) & 4095] / 2;
            m.walk(a, a + 1000, &runs);
            sink = uint32_t(runs.size());
        }));
        col.ns.push_back(perCall(20000, [&](int i) {
            const uint32_t a = ps[size_t(i) & 4095] / 2;
            sink = uint32_t(m.runsFor(a, a + 100000)->size());
        }));
        col.ns.push_back(perCall(reads, [&](int) { sink = uint32_t(m.runsFor(1000, 900000)->size()); }));
    }
    // edits: the call, then its Undo and its Redo (undone again after), with the dead record let go of
    const int edits = 2000;
    double insert = 0, insertEnd = 0, erase = 0, undoInsert = 0, undoErase = 0, redoErase = 0;
    for (int pass = 0; pass < 3; ++pass) {
        for (int i = 0; i < edits; ++i) {
            const uint32_t at = pass == 1 ? m.size() : rng.bounded(m.size() - 10);
            LineEdit e;
            QElapsedTimer t;
            if (pass < 2) {
                t.start();
                m.insert(at, 1, &e);
                (pass == 0 ? insert : insertEnd) += double(t.nsecsElapsed());
                t.restart();
                m.undo(e);
                undoInsert += double(t.nsecsElapsed());
                m.forget(e);
            } else {
                t.start();
                m.erase(at, 1, &e);
                erase += double(t.nsecsElapsed());
                t.restart();
                m.undo(e);
                undoErase += double(t.nsecsElapsed());
                t.restart();
                m.redo(e);
                redoErase += double(t.nsecsElapsed());
                m.undo(e);
                m.forget(e);
            }
        }
    }
    col.ns.push_back(insert / edits);
    col.ns.push_back(insertEnd / edits);
    col.ns.push_back(erase / edits);
    col.ns.push_back(undoInsert / (2 * edits));
    col.ns.push_back(undoErase / edits);
    col.ns.push_back(redoErase / edits);
    // a block of 10% of the lines erased and brought back
    {
        LineEdit e;
        const uint32_t n = m.size() / 10, at = m.size() / 3;
        QElapsedTimer t;
        t.start();
        m.erase(at, n, &e);
        const double d = double(t.nsecsElapsed());
        t.restart();
        m.undo(e);
        col.ns.push_back(d);
        col.ns.push_back(double(t.nsecsElapsed()));
    }
    (void)sink;
    return col;
}

} // namespace

int lineMapBenchMain(const QStringList &args)
{
    const uint32_t lines = args.size() > 0 ? args[0].toUInt() : 1000000u;
    const std::vector<size_t> runCounts = {1, 1000, 20000};
    std::vector<Column> cols;
    for (size_t r : runCounts) cols.push_back(measure(lines, r));
    static const char *names[] = {"id at a position",         "position of an id",      "id at a position, hint, in order", "position of an id, hint, in order", "walk of 1,000 lines",
                                  "walk of 100,000 lines (miss)", "walk of 900,000 lines (cache hit)", "insert one in the middle", "insert one at the end", "erase one in the middle",
                                  "undo of an insert",         "undo of an erase",       "redo of an erase",                 "erase 10% of the lines",        "undo of that"};
    out() << "linemap-bench: " << lines << " lines; nanoseconds per call\n";
    out() << QString("%1").arg("operation", -40);
    for (const Column &c : cols) out() << QString("%1").arg(QString::number(c.runs) + " runs", 14);
    out() << "\n";
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        out() << QString("%1").arg(QString::fromLatin1(names[i]), -40);
        for (const Column &c : cols) out() << QString("%1").arg(QString::number(c.ns[i], 'f', c.ns[i] < 100 ? 1 : 0), 14);
        out() << "\n";
    }
    out() << QString("%1").arg("bytes per run", -40);
    for (const Column &c : cols) out() << QString("%1").arg(QString::number(c.bytesPerRun, 'f', 1), 14);
    out() << "\n";
    out().flush();
    return 0;
}

} // namespace jp::crunchtool

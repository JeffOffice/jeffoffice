// The model check of rows, columns, and cells inserted and deleted (tests/crunch/structmodel.h), run for named sequences and a few seeds; the digests of today's engine for a fixed set of seeds
// (tests/crunch/structmodel_golden.txt); and tests that the check itself can fail. `jcrunchtool model-check` runs as many seeds as asked for.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "../localepin.h"
#include "../testrunner.h"
#include "structmodel.h"

#include <QFile>
#include <QRegularExpression>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

using namespace jp;
using namespace jp::structmodel;

namespace {

Step rows(Step::Kind kind, int sheet, int at, int count = 1)
{
    Step s;
    s.kind = kind;
    s.sheet = sheet;
    s.row1 = at;
    s.count = count;
    return s;
}
Step columns(Step::Kind kind, int sheet, int at, int count = 1)
{
    Step s;
    s.kind = kind;
    s.sheet = sheet;
    s.col1 = at;
    s.count = count;
    return s;
}
Step plain(Step::Kind kind)
{
    Step s;
    s.kind = kind;
    return s;
}
Step typed(int sheet, int row, int col, const QString &text)
{
    Step s;
    s.kind = Step::Type;
    s.sheet = sheet;
    s.row1 = row;
    s.col1 = col;
    s.text = text;
    return s;
}
Step cells(Step::Kind kind, int sheet, int row1, int col1, int row2, int col2)
{
    Step s;
    s.kind = kind;
    s.sheet = sheet;
    s.row1 = row1;
    s.col1 = col1;
    s.row2 = row2;
    s.col2 = col2;
    return s;
}

QString line(const std::vector<Step> &steps)
{
    QStringList l;
    for (const Step &s : steps) l << s.line();
    return l.join(QLatin1Char('\n'));
}

QString failureText(quint32 seed, const std::vector<Step> &steps, const Options &options, const Result &r)
{
    if (r.failure.ok()) return QString();
    return report(seed, shrink(steps, options, r.failure), r.failure);
}

} // namespace

class CrunchStructModelTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
    }

    // No steps: the workbook opens, is saved and read again, every formula is worked out from nothing, and everything agrees (the base workbook has a formula of each kind of reference).
    void theBaseWorkbookAgreesWithItselfWhenSavedAndReadAgain()
    {
        PinnedLocale pin;
        const Result r = run({}, Options());
        QVERIFY2(r.failure.ok(), qPrintable(r.failure.what));
        QVERIFY(r.graphFormulas > 150);   // (the graph check looked at the formulas)
        QVERIFY(r.graphReads > r.graphFormulas);
    }

    // A column (a row) deleted, its neighbor deleted, both undone and done again, and one inserted between: the sequence the redesign is most likely to get wrong.
    void deletingANeighborThenUndoRedoThenInsertingBetween()
    {
        PinnedLocale pin;
        for (const bool byRow : {false, true}) {
            const Step::Kind del = byRow ? Step::DeleteRows : Step::DeleteColumns, ins = byRow ? Step::InsertRows : Step::InsertColumns;
            auto at = [&](Step::Kind k, int sheet, int pos, int count = 1) { return byRow ? rows(k, sheet, pos, count) : columns(k, sheet, pos, count); };
            const int pos = byRow ? 4 : 1;
            std::vector<Step> steps = {at(del, 0, pos), at(del, 0, pos), plain(Step::Undo), plain(Step::Redo), at(ins, 0, pos), plain(Step::Undo), plain(Step::Undo), plain(Step::Undo), plain(Step::Redo), plain(Step::Redo),
                                       at(ins, 0, pos + 1, 2), at(del, 0, pos - 1)};
            const Result r = run(steps, Options());
            QVERIFY2(r.failure.ok(), qPrintable(failureText(1, steps, Options(), r)));
            QVERIFY(r.undone >= 4 && r.redone >= 2);
            QCOMPARE(r.refused, 0);
            QVERIFY(r.lineEdits >= 4);
            QCOMPARE(r.lineEditCellsMoved, qint64(0));   // (no cell is moved by an edit of rows or columns)
        }
    }

    // Whole rows and columns (and names for them) are read where they are: rows and columns inserted at the first, at the last the workbook has, and past it.
    void insertsAtTheFirstAtTheLastAndPastTheLastOfTheCells()
    {
        PinnedLocale pin;
        std::vector<Step> steps;
        for (const int sheet : {0, 1}) {
            steps.push_back(rows(Step::InsertRows, sheet, 0));
            steps.push_back(rows(Step::InsertRows, sheet, 40, 2));   // (the last row is 41 now)
            steps.push_back(rows(Step::InsertRows, sheet, 60, 3));
            steps.push_back(columns(Step::InsertColumns, sheet, 0));
            steps.push_back(columns(Step::InsertColumns, sheet, 12));   // (the last column is L, now M)
            steps.push_back(columns(Step::InsertColumns, sheet, 20, 2));
            steps.push_back(rows(Step::DeleteRows, sheet, 40, 2));
            steps.push_back(columns(Step::DeleteColumns, sheet, 0));
            steps.push_back(rows(Step::DeleteRows, sheet, 0));
        }
        for (int i = 0; i < 6; ++i) steps.push_back(plain(Step::Undo));
        for (int i = 0; i < 3; ++i) steps.push_back(plain(Step::Redo));
        const Result r = run(steps, Options());
        QVERIFY2(r.failure.ok(), qPrintable(failureText(2, steps, Options(), r)));
    }

    // One step of every kind, with saves in between: cells shifted in all four ways, fill, copy and paste, cut and paste, sort, typing of formulas of every kind, and a workbook saved and opened again
    // (its shared formulas are the ones the saver writes).
    void oneStepOfEveryKind()
    {
        PinnedLocale pin;
        std::vector<Step> steps = {
            cells(Step::InsertCellsDown, 0, 3, 0, 4, 1),
            cells(Step::InsertCellsRight, 0, 5, 4, 6, 5),
            cells(Step::DeleteCellsUp, 0, 8, 0, 9, 2),
            cells(Step::DeleteCellsLeft, 0, 12, 4, 12, 6),
            typed(0, 2, 6, QStringLiteral("=SUM(A1:A5)+Calc!B3")),
            typed(1, 4, 5, QStringLiteral("=SUM(Data:Extra!A1:A3)")),
            typed(0, 3, 7, QStringLiteral("=ROW()+COLUMN()")),
            plain(Step::Reopen),
            rows(Step::InsertRows, 0, 2, 2),
            columns(Step::DeleteColumns, 0, 3),
            plain(Step::Undo),
            plain(Step::Save),
            rows(Step::DeleteRows, 0, 10, 3),
            plain(Step::Undo),
        };
        Step fill = cells(Step::Fill, 0, 5, 4, 5, 4);
        fill.count = 4;
        steps.push_back(fill);
        Step paste = cells(Step::Paste, 0, 0, 4, 3, 5);
        paste.toSheet = 1;
        paste.toRow = 20;
        paste.toCol = 6;
        steps.push_back(paste);
        Step cut = cells(Step::Cut, 0, 10, 4, 12, 5);
        cut.toSheet = 0;
        cut.toRow = 22;
        cut.toCol = 8;
        steps.push_back(cut);
        Step sort = cells(Step::Sort, 0, 0, 0, 20, 3);
        sort.option = 1;
        steps.push_back(sort);
        steps.push_back(plain(Step::Undo));
        steps.push_back(plain(Step::Redo));
        steps.push_back(columns(Step::InsertColumns, 1, 1));
        const Result r = run(steps, Options());
        QVERIFY2(r.failure.ok(), qPrintable(failureText(3, steps, Options(), r)));
        QVERIFY(r.undone >= 2 && r.redone >= 1);
    }

    // The same sequences and seeds with the formulas held in the id frame (round 5a; dark): every check passes, and no cell is moved.
    void theNamedSequencesAndSeedsAgreeWithTheFormulasHeldInIds()
    {
        PinnedLocale pin;
        Options ids;
        ids.idFrame = true;
        for (const bool byRow : {false, true}) {
            const Step::Kind del = byRow ? Step::DeleteRows : Step::DeleteColumns, ins = byRow ? Step::InsertRows : Step::InsertColumns;
            auto at = [&](Step::Kind k, int sheet, int pos, int count = 1) { return byRow ? rows(k, sheet, pos, count) : columns(k, sheet, pos, count); };
            const int pos = byRow ? 4 : 1;
            std::vector<Step> steps = {at(del, 0, pos), at(del, 0, pos), plain(Step::Undo), plain(Step::Redo), at(ins, 0, pos), plain(Step::Undo), plain(Step::Undo), plain(Step::Undo), plain(Step::Redo), plain(Step::Redo),
                                       at(ins, 0, pos + 1, 2), at(del, 0, pos - 1), typed(0, byRow ? 6 : 3, byRow ? 7 : 8, QStringLiteral("=SUM(A1:A5)+Calc!B3")), plain(Step::Save), at(ins, 0, 0), plain(Step::Reopen)};
            const Result r = run(steps, ids);
            QVERIFY2(r.failure.ok(), qPrintable(failureText(1, steps, ids, r)));
            QCOMPARE(r.lineEditCellsMoved, qint64(0));
        }
        const int seeds = qEnvironmentVariableIsSet("CRUNCH_MODEL_SEEDS") ? qEnvironmentVariableIntValue("CRUNCH_MODEL_SEEDS") : 4;
        for (int seed = 1; seed <= seeds; ++seed) {
            const std::vector<Step> steps = generate(quint32(seed), 25);
            const Result r = run(steps, ids);
            QVERIFY2(r.failure.ok(), qPrintable(failureText(quint32(seed), steps, ids, r)));
        }
    }

    // The seeds that also type ranges written back to front, ATAN2, and a word that could be a cell past XFD (another list of steps for the same seed, so the golden seeds are as they were): in places and in ids.
    void rangesBackToFrontAndWordsLikeCellsAgreeInBothModes()
    {
        PinnedLocale pin;
        const int seeds = qEnvironmentVariableIsSet("CRUNCH_MODEL_SEEDS") ? qEnvironmentVariableIntValue("CRUNCH_MODEL_SEEDS") : 4;
        // (the seeds make the kinds of formula the others do not, which is looked for in many more steps than are run)
        bool backToFront = false, wordsLikeCells = false;
        const QRegularExpression range(QStringLiteral("SUM\\((?:Calc!)?[A-D](\\d+):[A-D](\\d+)\\)"));
        for (int seed = 1; seed <= 40; ++seed)
            for (const Step &s : generate(quint32(seed), 60, true)) {
                const QRegularExpressionMatch m = range.match(s.text);
                backToFront = backToFront || (m.hasMatch() && m.captured(1).toInt() > m.captured(2).toInt());
                wordsLikeCells = wordsLikeCells || s.text.contains(QStringLiteral("ATAN2")) || s.text.contains(QStringLiteral("XFE"));
            }
        QVERIFY(backToFront && wordsLikeCells);
        for (int seed = 1; seed <= seeds; ++seed) {
            const std::vector<Step> steps = generate(quint32(seed), 30, true);
            for (const bool ids : {false, true}) {
                Options options;
                options.idFrame = ids;
                options.forceOff = !ids;
                const Result r = run(steps, options);
                QVERIFY2(r.failure.ok(), qPrintable(failureText(quint32(seed), steps, options, r)));
            }
        }
        QCOMPARE(generate(5, 20, false).size(), size_t(20));   // (and a seed that types more than a little is another list with the option)
        QVERIFY(line(generate(5, 200, false)) != line(generate(5, 200, true)));
    }

    // Seeds of random steps. A failure is told with the seed and the fewest steps that still fail.
    void randomStepsAgreeWithTheFileSavedAndReadAgain()
    {
        PinnedLocale pin;
        const int seeds = qEnvironmentVariableIsSet("CRUNCH_MODEL_SEEDS") ? qEnvironmentVariableIntValue("CRUNCH_MODEL_SEEDS") : 4;
        const int stepCount = qEnvironmentVariableIsSet("CRUNCH_MODEL_STEPS") ? qEnvironmentVariableIntValue("CRUNCH_MODEL_STEPS") : 20;
        const int first = qEnvironmentVariableIsSet("CRUNCH_MODEL_FIRST") ? qEnvironmentVariableIntValue("CRUNCH_MODEL_FIRST") : 1;
        int refused = 0, undone = 0, done = 0, lineEdits = 0;
        qint64 moved = 0;
        for (int seed = first; seed < first + seeds; ++seed) {
            const std::vector<Step> steps = generate(quint32(seed), stepCount);
            const Result r = run(steps, Options());
            QVERIFY2(r.failure.ok(), qPrintable(failureText(quint32(seed), steps, Options(), r)));
            refused += r.refused;
            undone += r.undone;
            done += int(r.digests.size());
            lineEdits += r.lineEdits;
            moved += r.lineEditCellsMoved;
        }
        QVERIFY(lineEdits > 0);
        QCOMPARE(moved, qint64(0));
        // (the seeds make steps that are taken, steps that are refused, and Undo)
        QVERIFY(done >= seeds * stepCount);   // (and CRUNCH_MODEL_SEEDS, CRUNCH_MODEL_STEPS, and CRUNCH_MODEL_FIRST make it longer)
        QVERIFY(refused < done);
    }

    // The same steps in the same order give the same state on every machine: the digests of the first steps of a few seeds, from today's engine (the file is made by `jcrunchtool model-golden`).
    // A change that keeps what the engine means leaves them as they are.
    void todaysEngineHasTheDigestsTheFileKeeps()
    {
        PinnedLocale pin;
        QFile f(QStringLiteral(JC_SOURCE_DIR "/tests/crunch/structmodel_golden.txt"));
        QVERIFY2(f.open(QIODevice::ReadOnly), "the golden file is missing");
        QMap<quint32, QStringList> digests;
        QMap<quint32, QString> parts;
        QMap<quint32, QMap<QString, QString>> eachPart;
        QMap<quint32, int> counts;
        quint32 seed = 0;
        for (const QString &line : QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'))) {
            if (line.startsWith(QLatin1Char('#')) || line.trimmed().isEmpty()) continue;
            const QStringList w = line.trimmed().split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (w[0] == QLatin1String("seed")) {
                seed = w[1].toUInt();
                counts[seed] = w[2].toInt();
            } else if (w[0] == QLatin1String("parts")) {
                parts[w[1].toUInt()] = w[2];
            } else if (w[0] == QLatin1String("part") && w.size() == 4) {
                eachPart[w[1].toUInt()][w[2]] = w[3];
            } else {
                digests[seed] += w;
            }
        }
        QVERIFY(digests.size() >= 4);
        // (with the formulas held in the id frame the state after every step is the same; the parts a save writes are not, until the saver is the next round's: it writes every formula whole)
        for (const bool idFrame : {false, true}) {
        Options options;
        options.check = false;
        options.idFrame = idFrame;
        options.forceOff = !idFrame;
        for (auto it = digests.constBegin(); it != digests.constEnd(); ++it) {
            const Result r = run(generate(it.key(), counts[it.key()]), options);
            QVERIFY(r.failure.ok());
            QCOMPARE(int(r.digests.size()), it.value().size());
            for (int i = 0; i < it.value().size(); ++i)
                QVERIFY2(QStringLiteral("%1").arg(r.digests[size_t(i)], 16, 16, QLatin1Char('0')) == it.value()[i], qPrintable(QStringLiteral("seed %1: the state after step %2 (%3) is not what the golden file has").arg(it.key()).arg(i + 1).arg(r.stepsText[size_t(i)])));
            if (idFrame) continue;
            // (part by part first, so a difference names the part)
            QMap<QString, QString> got;
            for (const QString &line : r.parts) {
                const QStringList w = line.split(QLatin1Char(' '));
                if (w.size() == 2) got[w[0]] = w[1];
            }
            const QMap<QString, QString> &want = eachPart.value(it.key());
            for (auto p = want.constBegin(); p != want.constEnd(); ++p) {
                if (got.value(p.key()) != p.value() && r.smallParts.contains(p.key()))
                    qInfo().noquote() << "the part as saved here:" << p.key() << "\n" << QString::fromUtf8(r.smallParts.value(p.key()));
                QVERIFY2(got.value(p.key()) == p.value(), qPrintable(QStringLiteral("seed %1: the saved part %2 is not what the golden file has").arg(it.key()).arg(p.key())));
            }
            for (auto p = got.constBegin(); p != got.constEnd(); ++p)
                QVERIFY2(want.isEmpty() || want.contains(p.key()), qPrintable(QStringLiteral("seed %1: the save writes a part the golden file doesn't have: %2").arg(it.key()).arg(p.key())));
            QCOMPARE(QStringLiteral("%1").arg(r.partsDigest, 16, 16, QLatin1Char('0')), parts.value(it.key()));
        }
        }
    }

    // The model of the edit (the second opinion) renumbers references in the text of a formula as the engine must: cells, blocks, whole columns and rows, other sheets (and not three-D references,
    // texts, or the names of functions that look like cells).
    void theModelOfAnEditRenumbersTheReferencesOfAFormula()
    {
        const QString f = QStringLiteral("A1+A3+$A$5+B2:B4+SUM(A:A)+Calc!A9+Data:Extra!A9+\"A9\"+LOG10(A4)+Tbl[[#This Row],[Qty]]+SUM(2:3)");
        QCOMPARE(modelRewrite(f, 0, rows(Step::InsertRows, 0, 2)), QStringLiteral("A1+A4+$A$6+B2:B5+SUM(A:A)+Calc!A9+Data:Extra!A9+\"A9\"+LOG10(A5)+Tbl[[#This Row],[Qty]]+SUM(2:4)"));
        QCOMPARE(modelRewrite(f, 0, rows(Step::InsertRows, 0, 1, 2)), QStringLiteral("A1+A5+$A$7+B4:B6+SUM(A:A)+Calc!A9+Data:Extra!A9+\"A9\"+LOG10(A6)+Tbl[[#This Row],[Qty]]+SUM(4:5)"));
        QCOMPARE(modelRewrite(f, 0, rows(Step::DeleteRows, 0, 1, 2)), QStringLiteral("A1+#REF!+$A$3+B2:B2+SUM(A:A)+Calc!A9+Data:Extra!A9+\"A9\"+LOG10(A2)+Tbl[[#This Row],[Qty]]+SUM(#REF!)"));
        QCOMPARE(modelRewrite(f, 0, columns(Step::DeleteColumns, 0, 0)), QStringLiteral("#REF!+#REF!+#REF!+A2:A4+SUM(#REF!)+Calc!A9+Data:Extra!A9+\"A9\"+LOG10(#REF!)+Tbl[[#This Row],[Qty]]+SUM(2:3)"));
        QCOMPARE(modelRewrite(f, 0, columns(Step::InsertColumns, 0, 1)), QStringLiteral("A1+A3+$A$5+C2:C4+SUM(A:A)+Calc!A9+Data:Extra!A9+\"A9\"+LOG10(A4)+Tbl[[#This Row],[Qty]]+SUM(2:3)"));
        QCOMPARE(modelRewrite(f, 0, columns(Step::InsertColumns, 0, 0)), QStringLiteral("B1+B3+$B$5+C2:C4+SUM(B:B)+Calc!A9+Data:Extra!A9+\"A9\"+LOG10(B4)+Tbl[[#This Row],[Qty]]+SUM(2:3)"));
        // another sheet's edit changes only what names that sheet
        QCOMPARE(modelRewrite(QStringLiteral("Data!A5+A5+Data!$B$1:$B$4"), 1, rows(Step::InsertRows, 0, 0)), QStringLiteral("Data!A6+A5+Data!$B$2:$B$5"));
        QCOMPARE(modelRewrite(QStringLiteral("Data!A5+A5+'Calc'!B1"), 1, rows(Step::InsertRows, 1, 0)), QStringLiteral("Data!A5+A6+'Calc'!B2"));
        // a block that has a row inserted inside it grows, one that has all its rows deleted is gone
        QCOMPARE(modelRewrite(QStringLiteral("SUM(A2:A5)+SUM(A2:A3)"), 0, rows(Step::InsertRows, 0, 3)), QStringLiteral("SUM(A2:A6)+SUM(A2:A3)"));
        QCOMPARE(modelRewrite(QStringLiteral("SUM(A2:A5)+SUM(A2:A3)"), 0, rows(Step::DeleteRows, 0, 1, 2)), QStringLiteral("SUM(A2:A3)+SUM(#REF!)"));
    }

    // The check can fail: each kind of thing it looks at, changed in the file read again, is told; so is a graph that lacks a cell a formula reads; a refused step is held to change nothing; and a
    // different step is a different digest.
    void theCheckSaysSoWhenSomethingDiffers()
    {
        PinnedLocale pin;
        const std::vector<Step> one = {rows(Step::InsertRows, 0, 3)};
        static const char *const kinds[16] = {"merge", "cf", "dv", "name", "table", "chart", "note", "links", "cell", "val", "filter", "printarea", "rowbreaks", "colbreaks", "col", "row"};
        for (int fault = 1; fault <= 16; ++fault) {
            Options options;
            options.graph = false;
            options.fault = fault;
            const Result r = run(one, options);
            QVERIFY2(!r.failure.ok(), qPrintable(QStringLiteral("a change to %1 was not told").arg(QLatin1String(kinds[fault - 1]))));
            QCOMPARE(r.failure.kind, QLatin1String(kinds[fault - 1]));
        }
        Options model;
        model.fault = 101;
        const Result mr = run(one, model);
        QVERIFY(!mr.failure.ok());
        QCOMPARE(mr.failure.kind, QStringLiteral("model"));
        Options graph;
        graph.fault = 100;
        const Result g = run(one, graph);
        QVERIFY(!g.failure.ok());
        QCOMPARE(g.failure.kind, QStringLiteral("graph"));
        // a failure is shrunk to the steps that matter, and says the seed
        std::vector<Step> many = {rows(Step::InsertRows, 0, 3), columns(Step::InsertColumns, 1, 2), typed(0, 40, 0, QStringLiteral("5")), rows(Step::DeleteRows, 1, 7), plain(Step::Undo)};
        Options bad;
        bad.fault = 2;
        const Result m = run(many, bad);
        QVERIFY(!m.failure.ok());
        const std::vector<Step> small = shrink(many, bad, m.failure);
        QCOMPARE(small.size(), size_t(1));
        QVERIFY(report(7, small, m.failure).startsWith(QStringLiteral("seed 7:")));
        Step back;
        QVERIFY(Step::fromLine(small[0].line(), &back));
        QCOMPARE(back.line(), small[0].line());
        // two different lists of steps leave different states
        Options plainRun;
        plainRun.check = false;
        const Result a = run({rows(Step::InsertRows, 0, 3)}, plainRun), b = run({rows(Step::InsertRows, 0, 4)}, plainRun);
        QVERIFY(a.digests[0] != b.digests[0]);
        const Result a2 = run({rows(Step::InsertRows, 0, 3)}, plainRun);
        QCOMPARE(a.digests[0], a2.digests[0]);
    }
};

JP_TEST_CLASS(CrunchStructModelTests)
#include "tst_crunch_structmodel.moc"

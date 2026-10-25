// Tests of protection in the window: Protect Sheet and Protect Workbook with and without a password, what a protected sheet refuses and what its protection allows, the message when it
// refuses, the password hashed and never kept as text, a password a file made, a file that asks for too much work, and the file keeping the settings and the hash. Every workbook is built
// here.
#include "app/anchordialogs.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "savedok.h"
#include "sheet/protection.h"
#include "../localepin.h"

#include <QAction>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;
using jp::office::sheet::PasswordHash;
using jp::office::sheet::SheetProtection;
using jp::office::sheet::WorkbookProtection;

namespace {

// A1:C3 hold 1 to 9; column B is unlocked (style 1), A and C are locked.
QByteArray protectBook(const QString &protectionXml = QString(), const QString &workbookProtection = QString())
{
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" applyProtection=\"1\"><protection locked=\"0\"/></xf><xf numFmtId=\"0\" applyProtection=\"1\"><protection hidden=\"1\"/></xf>"));
    QString rows;
    for (int r = 1; r <= 4; ++r)
        rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%2</v></c><c r=\"B%1\" s=\"1\"><v>%3</v></c><c r=\"C%1\"><v>%4</v></c><c r=\"D%1\" s=\"2\"><f>A%1+1</f><v>%5</v></c></row>").arg(r).arg(r).arg(5 - r).arg(r * 10).arg(r + 1);
    QByteArray book = buildXlsx({worksheet(rows, protectionXml)}, QByteArray(), QString(), QString(), styles);
    if (!workbookProtection.isEmpty()) {
        QMap<QString, QByteArray> all;
        jp::readZip(book, all);
        QByteArray wb = all.value(QStringLiteral("xl/workbook.xml"));
        wb.replace("<sheets>", workbookProtection.toUtf8() + "<sheets>");
        all[QStringLiteral("xl/workbook.xml")] = wb;
        jp::ZipWriter z;
        for (auto it = all.begin(); it != all.end(); ++it) z.add(it.key(), it.value(), true);
        book = z.finish();
    }
    return book;
}

QByteArray savedPart(const QString &path, const QString &part)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QMap<QString, QByteArray> parts;
    jp::readZip(f.readAll(), parts);
    return parts.value(part);
}

} // namespace

class CrunchProtectTests : public QObject {
    Q_OBJECT

    QLocale m_oldLocale;

    static void protectWith(EditWindow &e, const std::function<void(ProtectRequest *)> &choose)
    {
        e.win.w.anchorHooks().protectAsker = [choose](ProtectRequest *r) {
            choose(r);
            return true;
        };
    }

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
        m_oldLocale = QLocale();
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    void cleanupTestCase() { QLocale::setDefault(m_oldLocale); }

    void aProtectedSheetRefusesLockedCellsWithAClearMessageAndLetsUnlockedOnesBeChanged()
    {
        EditWindow e;
        QVERIFY(e.open(protectBook()));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(e.win.w.act(QStringLiteral("protect.sheet"))->text(), QStringLiteral("Protect Sheet"));
        protectWith(e, [](ProtectRequest *) {});
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QVERIFY(s->protectionOf(0).enabled);
        QCOMPARE(e.win.w.act(QStringLiteral("protect.sheet"))->text(), QStringLiteral("Unprotect Sheet"));
        const auto locked = s->type(0, 0, 0, QStringLiteral("99"));
        QVERIFY(!locked.ok);
        QVERIFY2(locked.problem.contains(QStringLiteral("protected")), qPrintable(locked.problem));
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QVERIFY(s->type(0, 0, 1, QStringLiteral("99")).ok);   // (B1 is unlocked)
        // what is not allowed says so
        QString why;
        QVERIFY(!s->allows(0, crunch::EditSession::Allow::FormatCells, &why));
        QVERIFY2(why.contains(QStringLiteral("formatting cells")), qPrintable(why));
        QVERIFY(!s->insertRows(0, 1, 1).ok);
        QVERIFY(!s->deleteColumns(0, 0, 1).ok);
        const auto object = s->deleteObject(0, 1, QStringLiteral("Delete"), crunch::ObjectState(), crunch::ObjectState());   // (drawing objects are protected too)
        QVERIFY(!object.ok);
        QVERIFY2(object.problem.contains(QStringLiteral("drawing objects")), qPrintable(object.problem));
        QVERIFY(!s->resizeRows(0, 0, 0, crunch::EditSession::SizeChange(), QStringLiteral("Row Height")).ok);
        // two steps were made (the entry in B1, and the protection), and Undo goes back over them in turn: the entry first, with the sheet still protected, then the protection (it has no
        // password, so nothing is asked)
        QCOMPARE(e.win.w.undoStack()->count(), 2);
        e.win.w.undoStack()->undo();
        QVERIFY(s->protectionOf(0).enabled);
        e.win.w.undoStack()->undo();
        QVERIFY(!s->protectionOf(0).enabled);
        QVERIFY(s->type(0, 0, 0, QStringLiteral("99")).ok);
    }

    // A cell that holds nothing has the format of its column (or its row), and so its lock: typing and pasting into an empty cell of an unlocked column are allowed on a protected sheet, and into
    // an empty cell of a locked one are not.
    void anEmptyCellHasTheLockOfItsColumn()
    {
        const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" applyProtection=\"1\"><protection locked=\"0\"/></xf>"));
        QString rows;
        for (int r = 1; r <= 3; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        rows += QStringLiteral("<row r=\"9\" s=\"1\" customFormat=\"1\"><c r=\"A9\"><v>9</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows, QStringLiteral("<sheetProtection sheet=\"1\"/>"), QStringLiteral("<cols><col min=\"5\" max=\"5\" width=\"9\" style=\"1\" customWidth=\"1\"/></cols>"))}, QByteArray(), QString(), QString(), styles)));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 4, 4, QStringLiteral("typed")).ok);              // E5: empty, in the unlocked column E
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("s:typed"));
        const auto refused = s->type(0, 4, 5, QStringLiteral("no"));          // F5: empty, locked
        QVERIFY(!refused.ok);
        QVERIFY2(refused.problem.contains(QStringLiteral("locked")), qPrintable(refused.problem));
        QVERIFY(s->type(0, 8, 3, QStringLiteral("row")).ok);                // D9: empty, in the unlocked row 9
        QVERIFY(!s->type(0, 7, 3, QStringLiteral("row")).ok);               // D8
        // pasted text the same
        crunch::EditSession::PasteOptions options;
        jp::office::sheet::CellRange target;
        jp::office::sheet::parseRangeRef(QStringLiteral("E7"), &target);
        QVERIFY(s->pasteText(0, target, {QStringList{QStringLiteral("p")}}, options).ok);
        jp::office::sheet::parseRangeRef(QStringLiteral("F7"), &target);
        QVERIFY(!s->pasteText(0, target, {QStringList{QStringLiteral("p")}}, options).ok);
    }

    // With Format Cells allowed on a protected sheet, the lock and the hiding of formulas (the Protection tab) still can't be changed: they are what the protection stands on.
    void theLockAndTheHidingOfFormulasCantBeChangedOnAProtectedSheet()
    {
        EditWindow e;
        QVERIFY(e.open(protectBook(QStringLiteral("<sheetProtection sheet=\"1\" formatCells=\"0\"/>"))));
        crunch::EditSession *s = e.win.w.session();
        jp::office::sheet::CellRange range;
        jp::office::sheet::parseRangeRef(QStringLiteral("A1:A2"), &range);
        jp::office::sheet::FormatChange bold, unlock, hide;
        bold.bold = true;
        unlock.locked = false;
        hide.formulaHidden = true;
        QVERIFY(s->formatCells(0, range, bold, QStringLiteral("Bold")).ok);   // (formatting is allowed)
        const auto a = s->formatCells(0, range, unlock, QStringLiteral("Unlock"));
        QVERIFY(!a.ok);
        QVERIFY2(a.problem.contains(QStringLiteral("locked")), qPrintable(a.problem));
        QVERIFY(!s->formatCells(0, range, hide, QStringLiteral("Hide")).ok);
        QVERIFY(s->cellIsLocked(0, 0, 0));
        // and on a sheet that is not protected they can
        EditWindow open;
        QVERIFY(open.open(protectBook()));
        QVERIFY(open.win.w.session()->formatCells(0, range, unlock, QStringLiteral("Unlock")).ok);
        QVERIFY(!open.win.w.session()->cellIsLocked(0, 0, 0));
    }

    void aFormulaTheFormatHidesIsNotShownOnAProtectedSheet()
    {
        EditWindow e;
        QVERIFY(e.open(protectBook()));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->editText(0, 0, 3), QStringLiteral("=A1+1"));
        QCOMPARE(s->barText(0, 0, 3, QStringLiteral("=A1+1")), QStringLiteral("=A1+1"));
        QCOMPARE(s->searchText(0, 0, 3), QStringLiteral("=A1+1"));
        protectWith(e, [](ProtectRequest *) {});
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QVERIFY(s->protectionOf(0).enabled);
        QCOMPARE(s->editText(0, 0, 3), QString());
        QCOMPARE(s->barText(0, 0, 3, QStringLiteral("=A1+1")), QString());
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:2"));   // (the value shows)
        QCOMPARE(s->editText(0, 0, 0), QStringLiteral("1"));              // (a cell that is not a formula is as it was)
        // the copy of it is its value, and the search of formulas does not look in it
        const crunch::ClipData clip = s->copyCells(0, [] {
            jp::office::sheet::CellRange r;
            jp::office::sheet::parseRangeRef(QStringLiteral("D1:D2"), &r);
            return r;
        }(), false);
        QCOMPARE(clip.cells.size(), size_t(2));
        for (const crunch::ClipCell &c : clip.cells) {
            QVERIFY(!c.content.isFormula);
            QVERIFY(c.content.formula.isEmpty());
            QVERIFY(c.content.value.isNumber());
        }
        QCOMPARE(s->searchText(0, 0, 3), QString());
        e.win.w.undoStack()->undo();
        QCOMPARE(s->editText(0, 0, 3), QStringLiteral("=A1+1"));
    }

    void whatTheProtectionAllowsIsAllowed()
    {
        EditWindow e;
        QVERIFY(e.open(protectBook()));
        crunch::EditSession *s = e.win.w.session();
        protectWith(e, [](ProtectRequest *r) {
            r->settings.allowInsertRows = true;
            r->settings.allowDeleteRows = true;
            r->settings.allowFormatRows = true;
            r->settings.allowSort = true;
        });
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QVERIFY(s->protectionOf(0).allowInsertRows);
        QVERIFY(s->insertRows(0, 4, 1).ok);                // (rows can be inserted)
        QVERIFY(!s->insertColumns(0, 0, 1).ok);            // (columns cannot)
        QVERIFY(!s->deleteRows(0, 0, 1).ok);               // (a row with locked cells in it cannot be deleted)
        QVERIFY(s->deleteRows(0, 4, 1).ok);                // (the empty row just inserted can)
        crunch::EditSession::SizeChange by;
        QVERIFY(s->allows(0, crunch::EditSession::Allow::FormatRows));
        // a sort of unlocked cells is allowed, of locked ones is not
        jp::crunch::SortSpec spec;
        jp::crunch::SortLevel level;
        spec.levels.append(level);
        jp::office::sheet::CellRange unlocked, lockedRange;
        jp::office::sheet::parseRangeRef(QStringLiteral("B1:B4"), &unlocked);
        jp::office::sheet::parseRangeRef(QStringLiteral("A1:A4"), &lockedRange);
        QVERIFY(s->sortRange(0, unlocked, spec).ok);
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:1"));
        QVERIFY(!s->sortRange(0, lockedRange, spec).ok);
    }

    void aPasswordIsHashedAndTheSheetIsUnprotectedOnlyWithIt()
    {
        EditWindow e;
        QVERIFY(e.open(protectBook()));
        crunch::EditSession *s = e.win.w.session();
        protectWith(e, [](ProtectRequest *r) { r->password = QStringLiteral("secret"); });
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QTRY_VERIFY(s->protectionOf(0).enabled);   // (the hash is made off the window's thread)
        const SheetProtection &p = s->protectionOf(0);
        QCOMPARE(p.password.algorithm, QStringLiteral("SHA-512"));
        QCOMPARE(p.password.spin, qint64(jp::office::sheet::kNewSpinCount));
        QVERIFY(!p.password.hash.isEmpty() && !p.password.salt.isEmpty() && p.password.legacy.isEmpty());
        QVERIFY(!p.password.hash.contains(QStringLiteral("secret")));
        // a wrong password leaves the protection, and says so
        QStringList asked;
        QString answer = QStringLiteral("wrong");
        e.win.w.anchorHooks().passwordAsker = [&](PasswordRequest *r) {
            asked << r->what;
            r->password = answer;
            return true;
        };
        e.win.told.clear();
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QTRY_VERIFY(!e.win.told.isEmpty());
        QVERIFY2(e.win.told.last().contains(QStringLiteral("not the right password")), qPrintable(e.win.told.join(QLatin1Char('\n'))));
        QVERIFY(s->protectionOf(0).enabled);
        QCOMPARE(asked, (QStringList{"Sheet1"}));
        // the right one takes it off
        answer = QStringLiteral("secret");
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QTRY_VERIFY(!s->protectionOf(0).enabled);
        QVERIFY(!s->protectionOf(0).password.isSet());
        // a person cancelling the question leaves things as they were
        e.win.w.undoStack()->undo();   // (the stack itself: the window's Undo asks, as the next test shows)
        QVERIFY(s->protectionOf(0).enabled);
        e.win.w.anchorHooks().passwordAsker = [](PasswordRequest *) { return false; };
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QVERIFY(s->protectionOf(0).enabled);
    }

    // Undo and Redo are no way round a password: Undo of putting a password on, and Redo of taking it off, ask for it (the sheet's, and the workbook's).
    void undoAndRedoOfAPasswordAskForIt()
    {
        EditWindow e;
        QVERIFY(e.open(protectBook()));
        crunch::EditSession *s = e.win.w.session();
        protectWith(e, [](ProtectRequest *r) { r->password = QStringLiteral("secret"); });
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QTRY_VERIFY(s->protectionOf(0).enabled);
        QString answer = QStringLiteral("wrong");
        int asked = 0;
        e.win.w.anchorHooks().passwordAsker = [&](PasswordRequest *r) {
            ++asked;
            r->password = answer;
            return true;
        };
        e.win.told.clear();
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QTRY_VERIFY(!e.win.told.isEmpty());
        QCOMPARE(asked, 1);
        QVERIFY(s->protectionOf(0).enabled);   // (still on)
        answer = QStringLiteral("secret");
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QTRY_VERIFY(!s->protectionOf(0).enabled);
        // Redo puts it back with no question (it protects)
        asked = 0;
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        QCOMPARE(asked, 0);
        QVERIFY(s->protectionOf(0).enabled);
        // taking it off with the password, and Undo (which protects again) asks nothing, but Redo (which takes it off) asks
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QTRY_VERIFY(!s->protectionOf(0).enabled);
        asked = 0;
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QCOMPARE(asked, 0);
        QVERIFY(s->protectionOf(0).enabled);
        answer = QStringLiteral("wrong");
        e.win.told.clear();
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        QTRY_VERIFY(!e.win.told.isEmpty());
        QVERIFY(s->protectionOf(0).enabled);
        QCOMPARE(asked, 1);
        // the workbook's structure is the same
        e.win.w.anchorHooks().workbookProtectAsker = [](WorkbookProtectRequest *r) {
            r->password = QStringLiteral("book");
            return true;
        };
        e.win.w.act(QStringLiteral("protect.workbook"))->trigger();
        QTRY_VERIFY(s->workbookProtection().lockStructure);
        asked = 0;
        answer = QStringLiteral("nope");
        e.win.told.clear();
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QTRY_VERIFY(!e.win.told.isEmpty());
        QCOMPARE(asked, 1);
        QVERIFY(s->workbookProtection().lockStructure);
        answer = QStringLiteral("book");
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QTRY_VERIFY(!s->workbookProtection().any());
    }

    void theFileKeepsTheSettingsAndTheHashAndNeverThePassword()
    {
        EditWindow e;
        QVERIFY(e.open(protectBook()));
        protectWith(e, [](ProtectRequest *r) {
            r->password = QStringLiteral("correct horse");
            r->settings.allowSort = true;
            r->settings.allowFormatCells = true;
            r->settings.allowSelectUnlocked = true;
        });
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QTRY_VERIFY(e.win.w.session()->protectionOf(0).enabled);
        const SheetProtection mine = e.win.w.session()->protectionOf(0);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray xml = savedPart(e.path, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(xml.contains("<sheetProtection"), xml.constData());
        QVERIFY(!xml.contains("correct horse") && !xml.contains("password="));
        QVERIFY(xml.contains("algorithmName=\"SHA-512\"") && xml.contains("spinCount=\"100000\""));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QVERIFY(second.w.session()->protectionOf(0) == mine);
        // and it is still the same password: the second window takes the protection off with it
        second.w.anchorHooks().passwordAsker = [](PasswordRequest *r) {
            r->password = QStringLiteral("correct horse");
            return true;
        };
        second.w.act(QStringLiteral("protect.sheet"))->trigger();
        QTRY_VERIFY(!second.w.session()->protectionOf(0).enabled);
    }

    void aPasswordTheFileMadeTheOlderWayIsCheckedAndKeptAsItWas()
    {
        EditWindow e;
        QVERIFY(e.open(protectBook(QStringLiteral("<sheetProtection sheet=\"1\" password=\"CBEB\" formatCells=\"0\"/>"))));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->protectionOf(0).enabled);
        QCOMPARE(s->protectionOf(0).password.legacy, QStringLiteral("CBEB"));
        QVERIFY(s->protectionOf(0).allowFormatCells);
        // a change to a cell the protection lets be changed: the file's own protection is left as it is
        QVERIFY(s->type(0, 0, 1, QStringLiteral("7")).ok);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray xml = savedPart(e.path, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(xml.contains("<sheetProtection sheet=\"1\" password=\"CBEB\" formatCells=\"0\"/>"), xml.constData());
        // the right password takes it off
        QString answer = QStringLiteral("nope");
        e.win.w.anchorHooks().passwordAsker = [&](PasswordRequest *r) {
            r->password = answer;
            return true;
        };
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QTRY_VERIFY(!e.win.told.isEmpty());   // (the check is over: it said the password is wrong)
        QVERIFY(s->protectionOf(0).enabled);
        answer = QStringLiteral("test");
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QTRY_VERIFY(!s->protectionOf(0).enabled);
    }

    void aFileThatAsksForAnAbsurdNumberOfRepeatsIsNotWorkedOut()
    {
        EditWindow e;
        QVERIFY(e.open(protectBook(QStringLiteral("<sheetProtection algorithmName=\"SHA-512\" hashValue=\"AAAA\" saltValue=\"BBBB\" spinCount=\"4000000000\" sheet=\"1\"/>"))));
        e.win.w.anchorHooks().passwordAsker = [](PasswordRequest *r) {
            r->password = QStringLiteral("anything");
            return true;
        };
        e.win.told.clear();
        QElapsedTimer t;
        t.start();
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QTRY_VERIFY(!e.win.told.isEmpty());
        QVERIFY(e.win.w.session()->protectionOf(0).enabled);
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("far more times")), qPrintable(e.win.told.join(QLatin1Char('\n'))));
        QVERIFY(t.elapsed() < 5000);
    }

    void aSheetThatDoesNotLetLockedCellsBeSelectedKeepsTheCursorOffThem()
    {
        EditWindow e;
        QVERIFY(e.open(protectBook(QStringLiteral("<sheetProtection sheet=\"1\" selectLockedCells=\"1\"/>"))));
        e.select(QStringLiteral("B2"));
        QCOMPARE(e.selected(), QStringLiteral("B2"));
        e.select(QStringLiteral("A3"));   // (locked)
        QCOMPARE(e.selected(), QStringLiteral("B2"));
        QVERIFY(e.win.w.statusText().contains(QStringLiteral("protected")));
    }

    // On a sheet that does not let locked cells be selected the arrow keys pass over them: the cursor goes to the next cell in that direction that it may rest on.
    void arrowKeysPassOverLockedCellsOnASheetThatDoesNotLetThemBeSelected()
    {
        const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" applyProtection=\"1\"><protection locked=\"0\"/></xf>"));
        QString rows;
        for (int r = 1; r <= 4; ++r) {
            rows += QStringLiteral("<row r=\"%1\">").arg(r);
            for (int c = 0; c < 6; ++c) rows += QStringLiteral("<c r=\"%1%2\" s=\"%3\"><v>%2</v></c>").arg(QChar('A' + c)).arg(r).arg(c % 2 == 1 ? 1 : 0);   // B, D, and F are unlocked
            rows += QStringLiteral("</row>");
        }
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows, QStringLiteral("<sheetProtection sheet=\"1\" selectLockedCells=\"1\"/>"))}, QByteArray(), QString(), QString(), styles)));
        e.select(QStringLiteral("B2"));
        QCOMPARE(e.selected(), QStringLiteral("B2"));
        QTest::keyClick(e.view(), Qt::Key_Right);
        QCOMPARE(e.selected(), QStringLiteral("D2"));   // (C2 is locked)
        QTest::keyClick(e.view(), Qt::Key_Right);
        QCOMPARE(e.selected(), QStringLiteral("F2"));
        QTest::keyClick(e.view(), Qt::Key_Right);        // (nothing to the right that it may rest on: it stays)
        QCOMPARE(e.selected(), QStringLiteral("F2"));
        QTest::keyClick(e.view(), Qt::Key_Left);
        QCOMPARE(e.selected(), QStringLiteral("D2"));
        QTest::keyClick(e.view(), Qt::Key_Down);         // (D3 is unlocked)
        QCOMPARE(e.selected(), QStringLiteral("D3"));
    }

    void theWorkbooksStructureCanBeLockedAndUnlockedWithAPassword()
    {
        EditWindow e;
        QVERIFY(e.open(protectBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertSheet(1, QStringLiteral("Extra")).ok);
        e.win.w.undoStack()->undo();
        e.win.w.anchorHooks().workbookProtectAsker = [](WorkbookProtectRequest *r) {
            r->password = QStringLiteral("book pw");
            return true;
        };
        e.win.w.act(QStringLiteral("protect.workbook"))->trigger();
        QTRY_VERIFY(s->workbookProtection().lockStructure);
        QVERIFY(s->workbookProtection().password.isSet());
        QCOMPARE(e.win.w.act(QStringLiteral("protect.workbook"))->text(), QStringLiteral("Unprotect Workbook"));
        // no sheet is added, renamed, moved, hidden, or deleted
        const auto added = s->insertSheet(1, QStringLiteral("Extra"));
        QVERIFY(!added.ok);
        QVERIFY2(added.problem.contains(QStringLiteral("locked")), qPrintable(added.problem));
        QVERIFY(!s->renameSheet(0, QStringLiteral("Other")).ok);
        QVERIFY(!s->setSheetHidden(0, true).ok);
        QVERIFY(!s->deleteSheet(0).ok);
        // saved, and the file has the lock and a hash
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray xml = savedPart(e.path, QStringLiteral("xl/workbook.xml"));
        QVERIFY2(xml.contains("<workbookProtection") && xml.contains("lockStructure=\"1\"") && xml.contains("workbookHashValue="), xml.constData());
        QVERIFY(!xml.contains("book pw"));
        // taken off with the password
        e.win.w.anchorHooks().passwordAsker = [](PasswordRequest *r) {
            r->password = QStringLiteral("book pw");
            return true;
        };
        e.win.w.act(QStringLiteral("protect.workbook"))->trigger();
        QTRY_VERIFY(!s->workbookProtection().any());
        QVERIFY(s->insertSheet(1, QStringLiteral("Extra")).ok);
    }
};

JP_TEST_CLASS(CrunchProtectTests)
#include "tst_crunch_protect.moc"

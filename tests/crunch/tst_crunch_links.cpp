// Tests of links on cells in the window: the tip a link shows, Ctrl+click and the commands that follow one (and the ones that must ask first or never go), putting a link on a cell with the
// dialog, changing and removing links as steps of Undo, links following rows and columns, and the file keeping them. Nothing here opens a browser: the window's opener is a function the
// test gives.
#include "app/anchordialogs.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "savedok.h"
#include "../localepin.h"

#include <QAction>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QToolTip>
#include <QUndoStack>
#include <QUrl>

using namespace jp;
using namespace crunchtest;
using jp::office::sheet::LinkSpec;

namespace {

// Two sheets. Sheet1 (A1:D3 as editBook has them) has links: A1 to a web page (with a screen tip), B1 to Sheet2!C2, C1 to javascript: (a file can say anything), D1 to a file beside the workbook, and
// D2 to an e-mail address.
QByteArray linksBook(bool hostile = false)
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c><c r=\"C1\"><v>3</v></c><c r=\"D1\"><v>4</v></c></row><row r=\"2\"><c r=\"D2\"><v>5</v></c></row>"
                                         "<row r=\"3\"><c r=\"A3\"><v>6</v></c></row>");
    const QString links = QStringLiteral("<hyperlinks><hyperlink ref=\"A1\" r:id=\"rId1\" tooltip=\"The first page\"/><hyperlink ref=\"B1\" location=\"Sheet2!C2\"/><hyperlink ref=\"C1\" r:id=\"rId2\"/>"
                                         "<hyperlink ref=\"D1\" r:id=\"rId3\"/><hyperlink ref=\"D2\" r:id=\"rId4\"/>")
                          + (hostile ? QStringLiteral("<hyperlink ref=\"A3\" r:id=\"rId5\" tooltip=\"&lt;b&gt;bold&lt;/b&gt;&lt;img src=&quot;file:///etc/hostname&quot;&gt;\"/><hyperlink ref=\"A4\" r:id=\"rId6\"/>") : QString())
                          + QStringLiteral("</hyperlinks>");
    QMap<QString, QByteArray> all;
    // (styles with the lists a named style goes in, so that the look of a link can be given)
    const QByteArray styles = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
                               "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills><borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                               "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs><cellXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/></cellXfs>"
                               "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles></styleSheet>";
    jp::readZip(buildXlsx({worksheet(rows, links), worksheet(QStringLiteral("<row r=\"2\"><c r=\"C2\"><v>9</v></c></row>"))}, QByteArray(), QString(), QString(), styles), all);
    const QString base = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink");
    auto rel = [&](const QString &id, const QString &target) { return QStringLiteral("<Relationship Id=\"%1\" Type=\"%2\" Target=\"%3\" TargetMode=\"External\"/>").arg(id, base, target.toHtmlEscaped()); };
    all[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")] = ("<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" + rel(QStringLiteral("rId1"), QStringLiteral("https://example.org/first"))
                                                                  + rel(QStringLiteral("rId2"), QStringLiteral("javascript:alert(1)")) + rel(QStringLiteral("rId3"), QStringLiteral("data/report.pdf"))
                                                                  + rel(QStringLiteral("rId4"), QStringLiteral("mailto:a@example.org"))
                                                                  + (hostile ? rel(QStringLiteral("rId5"), QStringLiteral("ftp://h/<img src=\"file:///etc/passwd\">")) + rel(QStringLiteral("rId6"), QStringLiteral("tools/clean.cmd")) : QString())
                                                                  + "</Relationships>").toUtf8();
    jp::ZipWriter z;
    for (auto it = all.begin(); it != all.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

QPoint centerOf(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.view()->cellRectAt(r, c).center();
}

// The links of a saved file (read again), as "A1->target" or "A1=>place", sorted.
QStringList savedLinks(const QString &path)
{
    Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return {why};
    Workbook b;
    if (!b.open(p, &why)) return {why};
    Sheet s;
    if (!b.readSheet(0, &s)) return {QStringLiteral("unreadable")};
    QStringList out;
    for (const LinkSpec &l : s.links) out << rangeName(l.range) + (l.outside() ? QStringLiteral("->") + l.target : QStringLiteral("=>") + l.location);
    out.sort();
    return out;
}

} // namespace

class CrunchLinksTests : public QObject {
    Q_OBJECT

    QLocale m_oldLocale;

    struct Opened {
        QList<QUrl> urls;
    };
    static void watchOpener(EditWindow &e, Opened *seen)
    {
        e.win.w.anchorHooks().opener = [seen](const QUrl &u) {
            seen->urls << u;
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

    void aLinkShowsWhereItGoesWhenThePointerRestsOnIt()
    {
        EditWindow e;
        QVERIFY(e.open(linksBook()));
        QCOMPARE(e.win.w.session()->anchors().links(0).size(), size_t(5));
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("A1")));
        QTRY_VERIFY(QToolTip::isVisible());
        QVERIFY2(QToolTip::text().contains(QStringLiteral("The first page")), qPrintable(QToolTip::text()));
        QVERIFY(QToolTip::text().contains(QStringLiteral("Ctrl+click")));
        // with no tip of its own it says the address
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("D2")));
        QTRY_VERIFY(QToolTip::text().contains(QStringLiteral("mailto:a@example.org")));
    }

    void aWebLinkOpensOnlyWhenTheCellIsCtrlClickedOrTheCommandIsGiven()
    {
        EditWindow e;
        QVERIFY(e.open(linksBook()));
        Opened seen;
        watchOpener(e, &seen);
        // nothing opens by itself: opening the file, pointing, and a plain click
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("A1")));
        QTest::qWait(500);
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, centerOf(e, QStringLiteral("A1")));
        QVERIFY(seen.urls.isEmpty());
        QCOMPARE(e.selected(), QStringLiteral("A1"));
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, centerOf(e, QStringLiteral("A1")));
        QCOMPARE(seen.urls, (QList<QUrl>{QUrl(QStringLiteral("https://example.org/first"))}));
        // the command does the same for the cell with the cursor, and a cell with no link does nothing
        e.select(QStringLiteral("D2"));
        e.win.w.act(QStringLiteral("link.open"))->trigger();
        QCOMPARE(seen.urls.last(), QUrl(QStringLiteral("mailto:a@example.org")));
        const int count = int(seen.urls.size());
        e.select(QStringLiteral("B3"));
        QVERIFY(!e.win.w.act(QStringLiteral("link.open"))->isEnabled());
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, centerOf(e, QStringLiteral("B3")));
        QCOMPARE(int(seen.urls.size()), count);
    }

    // What a file says in a tip or an address is shown as the words it is, never as markup (markup can name a picture to fetch).
    void aTipOrAnAddressFromAFileIsShownAsPlainText()
    {
        EditWindow e;
        QVERIFY(e.open(linksBook(true)));
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("A3")));
        QTRY_VERIFY(QToolTip::isVisible());
        const QString shown = QToolTip::text();
        QVERIFY2(!shown.contains(QStringLiteral("<img")) && !shown.contains(QStringLiteral("<b>")), qPrintable(shown));
        QVERIFY(shown.contains(QStringLiteral("&lt;img")));
        // and the question that names an address of another kind shows it as plain text too
        Opened seen;
        watchOpener(e, &seen);
        bool plain = false, named = false;
        QTimer::singleShot(400, &e.win.w, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                plain = box->textFormat() == Qt::PlainText;
                named = box->text().contains(QStringLiteral("<img src=\"file:///etc/passwd\">"));
                box->reject();
            }
        });
        e.select(QStringLiteral("B3"));
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, centerOf(e, QStringLiteral("A3")));
        QVERIFY(plain);
        QVERIFY(named);
        QVERIFY(seen.urls.isEmpty());   // (it was refused)
    }

    // A link to a program, a script, or a shortcut is never opened, and never asked about.
    void aLinkToAProgramIsNeverOpenedOrAskedAbout()
    {
        EditWindow e;
        QVERIFY(e.open(linksBook(true)));
        Opened seen;
        watchOpener(e, &seen);
        bool asked = false;
        e.win.w.setConfirmAsker([&asked](const QString &, const QString &) {
            asked = true;
            return true;
        });
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, centerOf(e, QStringLiteral("A4")));
        QVERIFY(seen.urls.isEmpty());
        QVERIFY(!asked);
        QVERIFY(!e.win.told.isEmpty());
        // and the dialog's address is refused the same way
        LinkRequest req;
        req.sheetNames = {QStringLiteral("Sheet1")};
        LinkDialog dialog(req);
        dialog.findChild<QLineEdit *>(QStringLiteral("linkAddress"))->setText(QStringLiteral("C:\\Tools\\Setup.EXE"));
        QString problem;
        QVERIFY(!dialog.result(&req, &problem));
        QVERIFY(!problem.isEmpty());
    }

    void aLinkThatRunsAScriptIsNeverOpened()
    {
        EditWindow e;
        QVERIFY(e.open(linksBook()));
        Opened seen;
        watchOpener(e, &seen);
        bool asked = false;
        e.win.w.setConfirmAsker([&asked](const QString &, const QString &) {
            asked = true;
            return true;   // (even if the person said yes it would not be opened)
        });
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, centerOf(e, QStringLiteral("C1")));
        QVERIFY(seen.urls.isEmpty());
        QVERIFY(!asked);
        QVERIFY(!e.win.told.isEmpty());
    }

    void aLinkToAFileAsksFirstAndOpensItBesideTheWorkbook()
    {
        EditWindow e;
        QVERIFY(e.open(linksBook()));
        Opened seen;
        watchOpener(e, &seen);
        QString asked;
        bool answer = false;
        e.win.w.setConfirmAsker([&](const QString &, const QString &text) {
            asked = text;
            return answer;
        });
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, centerOf(e, QStringLiteral("D1")));
        QVERIFY2(asked.contains(QStringLiteral("data/report.pdf")), qPrintable(asked));
        QVERIFY(seen.urls.isEmpty());
        answer = true;
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, centerOf(e, QStringLiteral("D1")));
        QCOMPARE(seen.urls.size(), qsizetype(1));
        QVERIFY(seen.urls[0].isLocalFile());
        QCOMPARE(QFileInfo(seen.urls[0].toLocalFile()).absoluteFilePath(), QFileInfo(e.path).absolutePath() + QStringLiteral("/data/report.pdf"));
    }

    void aLinkToAPlaceInTheWorkbookGoesThereEvenOnAnotherSheet()
    {
        EditWindow e;
        QVERIFY(e.open(linksBook()));
        Opened seen;
        watchOpener(e, &seen);
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, centerOf(e, QStringLiteral("B1")));
        QTRY_COMPARE(e.selected(), QStringLiteral("C2"));
        QCOMPARE(e.win.w.activeSheet(), 1);
        QVERIFY(seen.urls.isEmpty());   // (no program is asked)
    }

    void aLinkIsPutOnACellWithTheDialogAndItsWordsAndLookComeWithIt()
    {
        EditWindow e;
        QVERIFY(e.open(linksBook()));
        crunch::EditSession *s = e.win.w.session();
        LinkRequest seen;
        e.win.w.anchorHooks().linkAsker = [&seen](LinkRequest *r) {
            seen = *r;
            r->link.target = QStringLiteral("https://example.org/new");
            r->link.tip = QStringLiteral("Look here");
            r->shownTextOut = QStringLiteral("the new page");
            return true;
        };
        e.select(QStringLiteral("F5"));
        QVERIFY(e.win.w.act(QStringLiteral("insert.link"))->isEnabled());
        const int steps = e.win.w.undoStack()->count();
        e.win.w.act(QStringLiteral("insert.link"))->trigger();
        QCOMPARE(seen.where, QStringLiteral("F5"));
        QVERIFY(!seen.editing);
        QCOMPARE(seen.sheetNames, (QStringList{"Sheet1", "Sheet2"}));
        QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join(QLatin1Char('\n'))));
        const LinkSpec *l = s->anchors().linkAt(0, 4, 5);
        QVERIFY(l);
        QCOMPARE(l->target, QStringLiteral("https://example.org/new"));
        QCOMPARE(l->tip, QStringLiteral("Look here"));
        QCOMPARE(e.shownAt(QStringLiteral("F5")), QStringLiteral("the new page"));
        // a person sees one step
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QVERIFY(s->calculation()->content(0, 4, 5).style != 0);   // (the look of a link: a format of its own)
        e.win.w.undoStack()->undo();
        QVERIFY(!s->anchors().linkAt(0, 4, 5));
        QCOMPARE(e.shownAt(QStringLiteral("F5")), QString());
        e.win.w.undoStack()->redo();
        QVERIFY(s->anchors().linkAt(0, 4, 5));
        QCOMPARE(e.shownAt(QStringLiteral("F5")), QStringLiteral("the new page"));
    }

    // The other program refuses a file whose link has an address of more than 2,083 characters or a screen tip of more than 255, so a person can't make one (the limit is a thing a sentence
    // says, and the longest that is allowed is let through).
    void aLinkTheFileCouldNotHoldIsRefused()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        crunch::EditSession *s = e.win.w.session();
        const auto cell = [](const char *ref) {
            jp::office::sheet::CellRange r;
            jp::office::sheet::parseRangeRef(QString::fromLatin1(ref), &r);
            return r;
        };
        LinkSpec link;
        link.target = QStringLiteral("https://example.org/") + QString(jp::office::sheet::kMaxNewLinkAddress - 20, QLatin1Char('p'));
        link.tip = QString(jp::office::sheet::kMaxNewLinkTip, QLatin1Char('t'));
        QVERIFY(s->setLink(0, cell("A1"), link, QStringLiteral("Link")).ok);
        link.target += QLatin1Char('p');
        const auto longAddress = s->setLink(0, cell("A2"), link, QStringLiteral("Link"));
        QVERIFY(!longAddress.ok);
        QVERIFY2(longAddress.problem.contains(QString::number(jp::office::sheet::kMaxNewLinkAddress)), qPrintable(longAddress.problem));
        link.target = QStringLiteral("https://example.org/");
        link.tip += QLatin1Char('t');
        const auto longTip = s->setLink(0, cell("A2"), link, QStringLiteral("Link"));
        QVERIFY(!longTip.ok);
        QVERIFY2(longTip.problem.contains(QString::number(jp::office::sheet::kMaxNewLinkTip)), qPrintable(longTip.problem));
    }

    // A link on a protected sheet that allows links goes only where every cell of the range may be changed (each one looked at, the empty ones by the lock of their column), and a range that
    // is too large to look at is refused.
    void linksOnAProtectedSheetCheckEveryCellOfTheRange()
    {
        const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" applyProtection=\"1\"><protection locked=\"0\"/></xf>"));
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" s=\"1\"><v>1</v></c><c r=\"B1\"><v>2</v></c></row><row r=\"2\"><c r=\"A2\" s=\"1\"><v>3</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows, QStringLiteral("<sheetProtection sheet=\"1\" insertHyperlinks=\"0\"/>"), QStringLiteral("<cols><col min=\"4\" max=\"4\" width=\"9\" style=\"1\" customWidth=\"1\"/></cols>"))},
                                QByteArray(), QString(), QString(), styles)));
        crunch::EditSession *s = e.win.w.session();
        LinkSpec link;
        link.target = QStringLiteral("https://example.org/");
        auto range = [](const char *ref) {
            jp::office::sheet::CellRange r;
            jp::office::sheet::parseRangeRef(QString::fromLatin1(ref), &r);
            return r;
        };
        QVERIFY(s->setLink(0, range("A1:A2"), link, QStringLiteral("Link")).ok);        // (both unlocked)
        QVERIFY(!s->setLink(0, range("A1:B1"), link, QStringLiteral("Link")).ok);       // (B1 is locked)
        QVERIFY(!s->setLink(0, range("A3:A9"), link, QStringLiteral("Link")).ok);       // (A3 to A9 hold nothing, and are locked)
        QVERIFY(s->setLink(0, range("D1:D40"), link, QStringLiteral("Link")).ok);        // (column D is unlocked, and holds nothing)
        const auto big = s->setLink(0, range("D1:D60000"), link, QStringLiteral("Link"));
        QVERIFY(!big.ok);
        QVERIFY2(big.problem.contains(QStringLiteral("more cells")), qPrintable(big.problem));
        // taking links off is the same
        QVERIFY(!s->removeLinks(0, range("A1:B2"), QStringLiteral("Remove")).ok);
        QVERIFY(s->removeLinks(0, range("A1:A2"), QStringLiteral("Remove")).ok);
        QVERIFY(!s->anchors().linkAt(0, 0, 0));
    }

    void aLinkCanBeChangedAndRemovedAndEachIsAStepOfUndo()
    {
        EditWindow e;
        QVERIFY(e.open(linksBook()));
        crunch::EditSession *s = e.win.w.session();
        LinkRequest seen;
        e.win.w.anchorHooks().linkAsker = [&seen](LinkRequest *r) {
            seen = *r;
            r->link.target = QStringLiteral("https://example.org/changed");
            return true;
        };
        e.select(QStringLiteral("A1"));
        QCOMPARE(e.win.w.act(QStringLiteral("insert.link"))->text(), QStringLiteral("Edit Link"));
        e.win.w.act(QStringLiteral("insert.link"))->trigger();
        QVERIFY(seen.editing);
        QCOMPARE(seen.link.target, QStringLiteral("https://example.org/first"));
        QCOMPARE(s->anchors().linkAt(0, 0, 0)->target, QStringLiteral("https://example.org/changed"));
        QCOMPARE(s->anchors().links(0).size(), size_t(5));   // (changed, not added)
        e.win.w.undoStack()->undo();
        QCOMPARE(s->anchors().linkAt(0, 0, 0)->target, QStringLiteral("https://example.org/first"));
        QVERIFY(e.win.w.act(QStringLiteral("link.remove"))->isEnabled());
        e.win.w.act(QStringLiteral("link.remove"))->trigger();
        QVERIFY(!s->anchors().linkAt(0, 0, 0));
        QCOMPARE(s->anchors().links(0).size(), size_t(4));
        QVERIFY(!e.win.w.act(QStringLiteral("link.remove"))->isEnabled());
        e.win.w.undoStack()->undo();
        QVERIFY(s->anchors().linkAt(0, 0, 0));
    }

    void linksGoWithTheirCellsAndTheFileKeepsEveryChange()
    {
        EditWindow e;
        QVERIFY(e.open(linksBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(0, 0, 2).ok);
        QVERIFY(s->anchors().linkAt(0, 2, 0));
        QVERIFY(s->insertColumns(0, 1, 1).ok);
        QCOMPARE(s->anchors().linkAt(0, 2, 2)->location, QStringLiteral("Sheet2!C2"));   // (B1 -> C3; the place is on the other sheet, so it stays)
        // one removed, one changed, one made
        e.win.w.anchorHooks().linkAsker = [](LinkRequest *r) {
            r->link.target = QStringLiteral("https://example.org/changed");
            return true;
        };
        e.select(QStringLiteral("A3"));
        e.win.w.act(QStringLiteral("insert.link"))->trigger();
        e.select(QStringLiteral("E4"));   // (D2 -> E4: the e-mail address)
        e.win.w.act(QStringLiteral("link.remove"))->trigger();
        e.win.w.anchorHooks().linkAsker = [](LinkRequest *r) {
            r->link.location = QStringLiteral("Sheet1!A3");
            r->link.target.clear();
            return true;
        };
        e.select(QStringLiteral("G7"));
        e.win.w.act(QStringLiteral("insert.link"))->trigger();
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QStringList links = savedLinks(e.path);
        QStringList expected = {"A3->https://example.org/changed", "C3=>Sheet2!C2", "E3->data/report.pdf", "G7=>Sheet1!A3", "D3->javascript:alert(1)"};
        expected.sort();
        QCOMPARE(links, expected);
        // read again by a window: the same, and the file's javascript link is still never followed
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.session()->anchors().links(0).size(), size_t(5));
    }

    void theLinkDialogRefusesWhatCannotBeALink()
    {
        LinkRequest req;
        req.where = QStringLiteral("B2");
        req.sheetNames = {QStringLiteral("Sheet1"), QStringLiteral("My Sheet")};
        LinkDialog dialog(req);
        auto *address = dialog.findChild<QLineEdit *>(QStringLiteral("linkAddress"));
        QVERIFY(address);
        QString problem;
        LinkRequest out = req;
        address->setText(QStringLiteral("javascript:alert(1)"));
        QVERIFY(!dialog.result(&out, &problem));
        QVERIFY(!problem.isEmpty());
        address->setText(QString());
        QVERIFY(!dialog.result(&out, &problem));
        // a web address with no scheme gets one; spaces at its ends do not stay
        address->setText(QStringLiteral("  www.example.org/a b "));
        QVERIFY(dialog.result(&out, &problem));
        QCOMPARE(out.link.target, QStringLiteral("https://www.example.org/a b"));
        QVERIFY(out.link.location.isEmpty());
    }
};

JP_TEST_CLASS(CrunchLinksTests)
#include "tst_crunch_links.moc"

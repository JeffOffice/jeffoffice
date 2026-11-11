// Tests of Insert > Equation in JeffCrunch: the object over the cells (inserted, undone and redone, moved, edited in the editor the other programs have), kept in a workbook as math markup in a
// text box with a picture beside it for the programs that don't read math, read back from that and from a file another program wrote, saved as a picture in a copy, and printed as vector paths.
// Every workbook is built here.
#include "../sheet/drawinghelpers.h"
#include "../testrunner.h"
#include "../pdfmarkcheck.h"
#include "../withdialog.h"
#include "editwindow.h"
#include "savedok.h"

#include "core/fonts.h"
#include "core/printjob.h"
#include "core/sheetobjects.h"
#include "office/package.h"
#include "office/xml.h"
#include "sheet/oldsave.h"
#include "sheet/sheetreader.h"
#include "wml/wmlmathedit.h"
#include "wml/wmlmathpath.h"
#include "wml/wml.h"

#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>

using namespace crunchtest;
using namespace drawtest;

namespace {

const QString kMc = QStringLiteral("http://schemas.openxmlformats.org/markup-compatibility/2006");
const QString kA14 = QStringLiteral("http://schemas.microsoft.com/office/drawing/2010/main");
const QString kMath = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/math");

// The object a newer spreadsheet program writes for an equation: a pair of alternatives, a text box whose text is the math (`omml`, an m:oMath) and, for the programs that don't read it, a picture.
QString equationAnchors(const QString &omml)
{
    const QString sp = QStringLiteral("<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"2\" name=\"Equation 1\"/><xdr:cNvSpPr txBox=\"1\"/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/>"
                                      "<a:ext cx=\"1905000\" cy=\"476250\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:noFill/></xdr:spPr><xdr:txBody><a:bodyPr wrap=\"none\" anchor=\"ctr\"/><a:lstStyle/>"
                                      "<a:p><a14:m xmlns:a14=\"%1\"><m:oMathPara xmlns:m=\"%2\"><m:oMathParaPr><m:jc m:val=\"centerGroup\"/></m:oMathParaPr>%3</m:oMathPara></a14:m><a:endParaRPr lang=\"en-US\"/></a:p></xdr:txBody></xdr:sp>")
                           .arg(kA14, kMath, omml);
    return QStringLiteral("<mc:AlternateContent xmlns:mc=\"%1\"><mc:Choice xmlns:a14=\"%2\" Requires=\"a14\">%3</mc:Choice><mc:Fallback>%4</mc:Fallback></mc:AlternateContent>")
        .arg(kMc, kA14, twoCell(1, 5, 4, 8, sp), twoCell(1, 5, 4, 8, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Equation 1"))));
}

QByteArray equationBook(const QString &omml)
{
    return bookWithDrawing({worksheet(QString())}, drawingPart(equationAnchors(omml)), {imageTarget(QStringLiteral("rIdP"), 1, pngBytes(40, 12, Qt::darkGreen))});
}

QString mathOf(const QString &linear)
{
    bool ok = false;
    return QString::fromUtf8(jp::office::wml::equationMarkup(linear, &ok));
}

QMap<QString, QByteArray> savedParts(const QString &path)
{
    QMap<QString, QByteArray> parts;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
    return parts;
}

QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

bool saveWindow(EditWindow &e)
{
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    return e.win.w.save() && savedOk(saved, 20000);
}

} // namespace

class CrunchEquationTests : public QObject {
    Q_OBJECT

    static bool shown(EditWindow &e, int count)
    {
        crunch::SheetObjects *o = e.win.w.objects();
        return o && o->shownSheet() == 0 && o->layer()->count() == count;
    }
    // The ids of the objects of the first sheet that are there (not taken away), and the one equation among them.
    static std::vector<int> there(EditWindow &e)
    {
        std::vector<int> out;
        for (const int id : e.win.w.objects()->idsOf(0))
            if (!e.win.w.objects()->isDeleted(0, id)) out.push_back(id);
        return out;
    }

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // The gallery and the command are on the Insert tab; each design puts an equation on the sheet, and Undo and Redo take it away and bring it back.
    void insertingAnEquationUndoesAndRedoes()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.win.w.act(QStringLiteral("insert.equation")));
        QVERIFY(e.win.w.act(QStringLiteral("insert.equationDesign3")));
        QVERIFY(there(e).empty());
        e.win.w.act(QStringLiteral("insert.equationDesign3"))->trigger();
        QVERIFY(e.settle());
        const std::vector<int> ids = there(e);
        QCOMPARE(int(ids.size()), 1);
        QString linear;
        QByteArray markup;
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(o->equationOf(0, ids[0], &linear, &markup));
        // (the line is the one that makes the math again: not always the one typed)
        QCOMPARE(jp::office::wml::equationLine(markup), jp::office::wml::equationLine(jp::office::wml::equationMarkup(jp::office::wml::equationGallery().at(3).linear)));
        QCOMPARE(jp::office::wml::equationLine(jp::office::wml::equationMarkup(linear)), jp::office::wml::equationLine(markup));
        QVERIFY(markup.contains("<m:oMath"));
        QVERIFY(e.win.w.isModified());
        pressUndo(e.view());
        QVERIFY(there(e).empty());
        pressRedo(e.view());
        QCOMPARE(int(there(e).size()), 1);
        QVERIFY(o->equationOf(0, there(e)[0], &linear, nullptr));
    }

    // The same editor: "Equation" opens it, with the picked equation's line when one is picked; a change is one step that undoes, and the object keeps its name.
    void theDialogInsertsAndEditsAnEquation()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        withDialog([&] { e.win.w.act(QStringLiteral("insert.equation"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            auto *input = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            QVERIFY(input);
            QVERIFY(input->text().isEmpty());
            input->setText(QStringLiteral("a/b"));
            QCOMPARE(d->findChild<QLabel *>(QStringLiteral("equationPreview"))->text(), QStringLiteral("a/b"));
        });
        QVERIFY(e.settle());
        QCOMPARE(int(there(e).size()), 1);
        const int id = there(e)[0];
        crunch::SheetObjects *o = e.win.w.objects();
        const QString name = o->nameOf(0, id);
        QVERIFY(name.startsWith(QStringLiteral("Equation")));
        o->layer()->select(id);
        withDialog([&] { e.win.w.act(QStringLiteral("insert.equation"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            auto *input = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            QCOMPARE(jp::office::wml::equationLine(jp::office::wml::equationMarkup(input->text())), QStringLiteral("a/b"));
            input->setText(QStringLiteral("x^2+y^2"));
        });
        QVERIFY(e.settle());
        QCOMPARE(int(there(e).size()), 1);
        QString linear;
        QVERIFY(o->equationOf(0, there(e)[0], &linear, nullptr));
        QCOMPARE(jp::office::wml::equationLine(jp::office::wml::equationMarkup(linear)), QStringLiteral("x2 + y2"));
        QCOMPARE(o->nameOf(0, there(e)[0]), name);
        QCOMPARE(e.win.w.session()->undoStack()->undoText(), QStringLiteral("Edit Equation"));
        e.win.w.session()->undoStack()->undo();
        QCOMPARE(int(there(e).size()), 1);
        QVERIFY(o->equationOf(0, there(e)[0], &linear, nullptr));
        QCOMPARE(jp::office::wml::equationLine(jp::office::wml::equationMarkup(linear)), QStringLiteral("a/b"));
    }

    // A line that makes no equation puts nothing on the sheet.
    void aLineThatMakesNoEquationPutsNothingOnTheSheet()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(!e.win.w.insertEquation(QStringLiteral("   ")));
        QVERIFY(!e.win.w.insertEquation(QString(jp::office::wml::kMostEquationInput + 1, QLatin1Char('x'))));
        QVERIFY(there(e).empty());
        QVERIFY(!e.win.w.isModified());
        QCOMPARE(e.win.told.size(), 2);
    }

    // The workbook keeps an equation as one: math markup in a text box (as a pair of alternatives, with a picture for the programs that don't read math), which reads back as the same equation.
    void aSavedWorkbookKeepsTheEquationAndReadsItBack()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        const QString linear = QStringLiteral("\\sum_(k=0)^n x^k=(1-x^(n+1))/(1-x)");
        QVERIFY(e.win.w.insertEquation(linear));
        QVERIFY(e.settle());
        QVERIFY(saveWindow(e));
        const QMap<QString, QByteArray> parts = savedParts(e.path);
        const QString drawing = QString::fromUtf8(parts.value(QStringLiteral("xl/drawings/drawing1.xml")));
        QVERIFY2(drawing.contains(QStringLiteral("AlternateContent")) && drawing.contains(QStringLiteral("Requires=\"a14\"")) && drawing.contains(QStringLiteral("<a14:m")), qPrintable(drawing.left(1500)));
        QVERIFY2(drawing.contains(QStringLiteral("<m:nary")) && drawing.contains(QStringLiteral("<m:f")), qPrintable(drawing.mid(drawing.indexOf(QStringLiteral("<a14:m")), 900)));
        // the picture for the other programs: a PNG the drawing's relationships name
        QVERIFY2(drawing.contains(QStringLiteral("<xdr:pic>")) && drawing.contains(QStringLiteral("r:embed=")), "no picture beside the equation");
        int pngs = 0;
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().startsWith(QStringLiteral("xl/media/")) && it.value().startsWith("\x89PNG")) ++pngs;
        QCOMPARE(pngs, 1);
        // and read back: the same equation
        EditWindow again;
        QVERIFY(again.open(fileBytes(e.copyOfFile())));
        QTRY_VERIFY(shown(again, 1));
        QString back;
        QByteArray markup;
        QVERIFY(again.win.w.objects()->equationOf(0, again.win.w.objects()->idsOf(0).front(), &back, &markup));
        // (the line it was made again from makes the same math: the same text, and a layout of the same size)
        QCOMPARE(jp::office::wml::equationLine(jp::office::wml::equationMarkup(back)), jp::office::wml::equationLine(jp::office::wml::equationMarkup(linear)));
        QCOMPARE(jp::office::wml::equationLine(markup), jp::office::wml::equationLine(jp::office::wml::equationMarkup(linear)));
        const auto was = jp::office::wml::equationShapeCached(jp::office::wml::equationMarkup(linear)), now = jp::office::wml::equationShapeCached(markup);
        QVERIFY(was && now);
        QVERIFY(std::abs(was->box.width() - now->box.width()) < 1.0 && std::abs(was->box.height() - now->box.height()) < 1.0);
    }

    // A file another program wrote, with an equation of its own making (math no line makes): it is shown as an equation, drawn, and kept; the picture beside it is not shown.
    void aForeignEquationOpensAsAnEquation()
    {
        const QString omml = QStringLiteral("<m:oMath><m:r><m:t>E</m:t></m:r><m:r><m:t>=</m:t></m:r><m:r><m:t>m</m:t></m:r><m:sSup><m:e><m:r><m:t>c</m:t></m:r></m:e><m:sup><m:r><m:t>2</m:t></m:r></m:sup></m:sSup>"
                                            "<m:groupChr><m:e><m:r><m:t>q</m:t></m:r></m:e></m:groupChr></m:oMath>");
        EditWindow e;
        QVERIFY(e.open(equationBook(omml)));
        QTRY_VERIFY(shown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        QString linear = QStringLiteral("unset");
        QByteArray markup;
        QVERIFY2(o->equationOf(0, o->idsOf(0).front(), &linear, &markup), "the equation is not shown as an equation");
        QVERIFY2(markup.contains("<m:sSup>"), markup.constData());   // (and the namespace of the math is declared once)
        QCOMPARE(markup.count("xmlns:m="), 1);
        QVERIFY(linear.isEmpty());   // (no line makes it: it is changed by typing another)
        // drawn as an equation: ink in the box, not the green of the fallback picture
        o->layer()->prepareAll();
        const QRect box = e.view()->sheet().layout().rangeRect(5, 1, 7, 3);
        e.view()->viewport()->repaint();
        const QImage shot = e.view()->viewport()->grab().toImage();
        int dark = 0, green = 0;
        for (int y = box.top(); y < box.bottom(); ++y)
            for (int x = box.left(); x < box.right(); ++x) {
                const QColor c = shot.pixelColor(qRound(x * shot.devicePixelRatio()), qRound(y * shot.devicePixelRatio()));
                dark += c.value() < 90;
                green += c.green() > 90 && c.red() < 40 && c.blue() < 40;
            }
        QVERIFY2(dark > 10, qPrintable(QString::number(dark)));   // (ink, however the fonts of the machine set it)
        QCOMPARE(green, 0);
        // saving it unchanged leaves the pair as it was
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("1"));
        QVERIFY(e.settle());
        QVERIFY(saveWindow(e));
        QVERIFY(QString::fromUtf8(savedParts(e.path).value(QStringLiteral("xl/drawings/drawing1.xml"))).contains(QStringLiteral("<m:sSup")));
    }

    // Math that can't be read leaves the picture that stands for it, as a file with no math does.
    void mathThatCantBeReadShowsThePicture()
    {
        EditWindow e;
        QVERIFY(e.open(equationBook(QStringLiteral("<m:oMath></m:oMath>"))));
        QTRY_VERIFY(shown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(!o->equationOf(0, o->idsOf(0).front(), nullptr, nullptr));
        QCOMPARE(o->kindOf(0, o->idsOf(0).front()), jp::office::sheet::ObjectKind::Picture);
    }

    // Math nested past the limit, or with more text than an equation may hold, is read within the limits: the equation is shown (cut), and nothing hangs or grows.
    void hostileMathIsReadWithinTheLimits()
    {
        QString deep;
        for (int i = 0; i < 40; ++i) deep += QStringLiteral("<m:sSup><m:e>");
        deep += QStringLiteral("<m:r><m:t>x</m:t></m:r>");
        for (int i = 0; i < 40; ++i) deep += QStringLiteral("</m:e><m:sup><m:r><m:t>2</m:t></m:r></m:sup></m:sSup>");
        EditWindow e;
        QVERIFY(e.open(equationBook(QStringLiteral("<m:oMath>") + deep + QStringLiteral("</m:oMath>"))));
        QTRY_VERIFY(shown(e, 1));
        e.win.w.objects()->layer()->prepareAll();
        QVERIFY(e.win.w.objects()->idsOf(0).size() == 1);
        const auto shape = jp::office::wml::equationShape(QStringLiteral("<m:oMath xmlns:m=\"%1\">%2</m:oMath>").arg(kMath, deep).toUtf8(), 20);
        QVERIFY(shape.cut);
        const QString many = QStringLiteral("<m:r><m:t>%1</m:t></m:r>").arg(QString(jp::office::wml::kMaxEquationChars * 3, QLatin1Char('x')));
        const auto wide = jp::office::wml::equationShape(QStringLiteral("<m:oMath xmlns:m=\"%1\">%2</m:oMath>").arg(kMath, many).toUtf8(), 20);
        QVERIFY(wide.cut);
    }

    // A copy as .ods has no equation: it holds a picture of it, in the place the equation is, and the report says so.
    void anOdsCopySavesAPictureAndNamesIt()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.win.w.insertEquation(QStringLiteral("a^2+b^2=c^2")));
        QVERIFY(e.settle());
        QVERIFY(saveWindow(e));
        jp::office::Package package;
        QString why;
        QVERIFY2(package.readLazyFile(e.path, &why), qPrintable(why));
        jp::office::sheet::Workbook book;
        QVERIFY2(book.open(package, &why), qPrintable(why));
        const QStringList before = jp::office::sheet::featuresNotInCopy(book, jp::office::sheet::CopyKind::Ods);
        QVERIFY2(before.join(QLatin1Char('|')).contains(QStringLiteral("equation")), qPrintable(before.join(QLatin1Char('|'))));
        QByteArray ods;
        jp::office::sheet::CopyReport report;
        QVERIFY2(jp::office::sheet::saveOds(book, &ods, &report, &why), qPrintable(why));
        QVERIFY2(report.notKept.join(QLatin1Char('|')).contains(QStringLiteral("equation")), qPrintable(report.notKept.join(QLatin1Char('|'))));
        QVERIFY2(report.notKept.join(QLatin1Char('|')).contains(QStringLiteral("picture")), qPrintable(report.notKept.join(QLatin1Char('|'))));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(ods, parts));
        int pictures = 0;
        for (auto it = parts.begin(); it != parts.end(); ++it) pictures += it.key().startsWith(QStringLiteral("Pictures/")) && it.value().startsWith("\x89PNG");
        QCOMPARE(pictures, 1);
        const QString content = QString::fromUtf8(parts.value(QStringLiteral("content.xml")));
        QVERIFY(content.contains(QStringLiteral("<draw:frame")) && content.contains(QStringLiteral("<draw:image")));
        QVERIFY(QString::fromUtf8(parts.value(QStringLiteral("META-INF/manifest.xml"))).contains(QStringLiteral("Pictures/")));
    }

    // For a picture of the window with an equation in it (JP_SCREENSHOT_DIR names the folder; the test does nothing without it).
    void aWindowPictureOfAnEquation()
    {
        const QString dir = qEnvironmentVariable("JP_SCREENSHOT_DIR");
        if (dir.isEmpty()) QSKIP("JP_SCREENSHOT_DIR is not set");
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.resize(1100, 700);
        QVERIFY(e.win.w.insertEquation(QStringLiteral("x=(-b\\pm\\sqrt(b^2-4ac))/(2a)")));
        QVERIFY(e.settle());
        e.win.w.objects()->layer()->prepareAll();
        QTest::qWait(300);
        e.view()->viewport()->repaint();
        QVERIFY(e.win.w.grab().save(QDir(dir).filePath(QStringLiteral("crunch-equation.png"))));
    }

    // Saving 500 equations lays each distinct one out once: the layouts are kept by their markup and used for the picture beside each, for printing, and for the check on reading.
    void savingManyEquationsLaysEachDistinctOneOutOnce()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        const QStringList lines = {QStringLiteral("a/b"), QStringLiteral("x^2"), QStringLiteral("\\sqrt(x)"), QStringLiteral("a+b"), QStringLiteral("\\pi r^2")};
        for (int i = 0; i < 500; ++i) QVERIFY(e.win.w.insertEquation(lines[i % lines.size()]));
        QVERIFY(e.settle());
        jp::office::wml::clearEquationCacheForTest();
        const qint64 before = jp::office::wml::equationLayoutsMadeForTest();
        QVERIFY(saveWindow(e));
        const QString path = e.folder.dir.filePath(QStringLiteral("many.pdf"));
        QString why;
        QVERIFY2(e.win.w.exportPdfTo(path, crunch::PrintJobOptions(), QString(), &why), qPrintable(why));
        QCOMPARE(jp::office::wml::equationLayoutsMadeForTest() - before, qint64(lines.size()));
        int pngs = 0;
        for (const QByteArray &bytes : savedParts(e.path))
            if (bytes.startsWith("\x89PNG")) ++pngs;
        QCOMPARE(pngs, 500);
    }

    // The pictures that stand for equations are held to a total in one save: past it an equation is saved as its math alone, and the note says so.
    void thePicturesBesideEquationsAreHeldToATotalAndTheLimitIsNamed()
    {
        auto run = [&](qint64 limit, int *pngs, int *maths, QStringList *told) {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            e.win.w.objects()->setFallbackLimitForTest(limit);
            for (const char *line : {"a/b", "x^2", "a+b"}) QVERIFY(e.win.w.insertEquation(QString::fromLatin1(line)));
            QVERIFY(e.settle());
            QVERIFY(saveWindow(e));
            const QMap<QString, QByteArray> parts = savedParts(e.path);
            *pngs = *maths = 0;
            for (auto it = parts.begin(); it != parts.end(); ++it) *pngs += it.value().startsWith("\x89PNG");
            *maths = QString::fromUtf8(parts.value(QStringLiteral("xl/drawings/drawing1.xml"))).count(QStringLiteral("<a14:m"));
            *told = e.win.told;
        };
        int pngs = 0, maths = 0;
        QStringList told;
        run(qint64(1) << 20, &pngs, &maths, &told);
        QCOMPARE(pngs, 3);
        QCOMPARE(maths, 3);
        QVERIFY(told.join(QLatin1Char('|')).contains(QStringLiteral("saved as math alone")) == false);
        run(1, &pngs, &maths, &told);
        QCOMPARE(maths, 3);
        QVERIFY2(pngs < 3, qPrintable(QString::number(pngs)));
        QVERIFY2(told.join(QLatin1Char('|')).contains(QStringLiteral("saved as math alone")), qPrintable(told.join(QLatin1Char('|'))));
    }

    // The picture beside an equation, and what is printed, are in the color the letters have in the file (red here), not always black.
    void theFallbackPictureAndThePrintAreInTheEquationsColor()
    {
        const QString omml = QStringLiteral("<m:oMath><m:f><m:num><m:r><m:t>a</m:t></m:r></m:num><m:den><m:r><m:t>b</m:t></m:r></m:den></m:f></m:oMath>");
        QString book = equationAnchors(omml);
        book.replace(QStringLiteral("<a:endParaRPr lang=\"en-US\"/>"), QStringLiteral("<a:endParaRPr lang=\"en-US\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:endParaRPr>"));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({worksheet(QString())}, drawingPart(book), {imageTarget(QStringLiteral("rIdP"), 1, pngBytes(40, 12, Qt::darkGreen))})));
        QTRY_VERIFY(shown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        const int id = o->idsOf(0).front();
        QVERIFY(e.win.w.editEquation(id, QStringLiteral("a/b+c")));   // (the object is made again, so its picture is made at the save)
        QVERIFY(e.settle());
        QVERIFY(saveWindow(e));
        int red = 0, other = 0;
        for (const QByteArray &bytes : savedParts(e.path)) {
            if (!bytes.startsWith("\x89PNG")) continue;
            const QImage img = QImage::fromData(bytes, "PNG");
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x) {
                    const QColor c = img.pixelColor(x, y);
                    if (c.alpha() < 200) continue;
                    (c.red() > 150 && c.green() < 80 && c.blue() < 80 ? red : other)++;
                }
        }
        QVERIFY2(red > 30 && other <= red / 4, qPrintable(QStringLiteral("%1 red, %2 other").arg(red).arg(other)));
        const QString path = e.folder.dir.filePath(QStringLiteral("red.pdf"));
        QString why;
        QVERIFY2(e.win.w.exportPdfTo(path, crunch::PrintJobOptions(), QString(), &why), qPrintable(why));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QVector<QByteArray> pages = pdfcheck::pageContents(f.readAll());
        QVERIFY(!pages.isEmpty());
        QVERIFY2(QString::fromLatin1(pages.first()).contains(QRegularExpression(QStringLiteral("(?m)^1(\\.0+)? 0(\\.0+)? 0(\\.0+)? (rg|sc|scn)$"))), qPrintable(QStringLiteral("the print is not red: ") + QString::fromLatin1(pages.first()).split(QLatin1Char('\n')).filter(QRegularExpression(QStringLiteral(" (rg|sc|scn|g)$"))).join(QLatin1Char('|'))));
    }

    // Printing and PDF draw the equation as vector paths: filled outlines on the page, and no picture.
    void aPdfDrawsTheEquationAsPaths()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.win.w.insertEquation(QStringLiteral("x=(-b\\pm\\sqrt(b^2-4ac))/(2a)")));
        QVERIFY(e.settle());
        const QString path = e.folder.dir.filePath(QStringLiteral("eq.pdf"));
        QString why;
        QVERIFY2(e.win.w.exportPdfTo(path, crunch::PrintJobOptions(), QString(), &why), qPrintable(why));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        const QVector<QByteArray> pages = pdfcheck::pageContents(pdf);
        QVERIFY(!pages.isEmpty());
        const QString content = QString::fromLatin1(pages.first());
        QVERIFY2(content.count(QRegularExpression(QStringLiteral("(?m)^[-\\d. ]+ (m|l|c|re)$"))) > 10, "no path operators on the page");
        QVERIFY2(content.contains(QRegularExpression(QStringLiteral("(?m)^(f\\*?|B\\*?|S)$"))), "no painted path");
        QVERIFY(!pdf.contains("/Subtype /Image"));
    }
};

JP_TEST_CLASS(CrunchEquationTests)
#include "tst_crunch_equation.moc"

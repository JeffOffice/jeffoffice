// Form controls on a sheet in JeffCrunch's window: every kind (check box, option button, button, drop-down, list box, spin button, scroll bar, group box, label) is drawn where its anchor puts it,
// works without macros through the cell it is linked to (and follows that cell), is saved with its state and nothing else changed, and can be made and formatted. Every workbook is built here.
#include "editwindow.h"
#include "savedok.h"

#include "app/sheettabs.h"
#include "core/fonts.h"
#include "core/sheetobjects.h"
#include "io/zip.h"
#include "office/package.h"
#include "testrunner.h"

#include <QApplication>
#include <QMenu>
#include <QRegularExpression>
#include <QToolTip>

using namespace crunchtest;

namespace {

using Parts = QMap<QString, QByteArray>;

const QString kRel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
const QString kSheet = QStringLiteral("xl/worksheets/sheet1.xml");
const QString kSheetRels = QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels");

QByteArray repack(const QByteArray &zip, const std::function<void(Parts &)> &change)
{
    Parts parts;
    jp::readZip(zip, parts);
    change(parts);
    jp::ZipWriter z;
    for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

void relate(Parts &p, const QString &id, const QString &type, const QString &target)
{
    QByteArray &rels = p[kSheetRels];
    if (rels.isEmpty()) rels = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"></Relationships>";
    rels.replace("</Relationships>", QStringLiteral("<Relationship Id=\"%1\" Type=\"%2\" Target=\"%3\"/></Relationships>").arg(id, kRel + type, target).toUtf8());
}

void addOverride(Parts &p, const QString &part, const QString &type)
{
    p[QStringLiteral("[Content_Types].xml")].replace("</Types>", QStringLiteral("<Override PartName=\"/%1\" ContentType=\"%2\"/>").arg(part, type).toUtf8() + "</Types>");
}

// A shape of the legacy drawing for a control (as the 2007 file format writes one): its kind, the eight numbers of its anchor, its words, and what else its ClientData says.
QString shape(int id, const QString &type, const QString &anchor, const QString &words, const QString &more = QString())
{
    return QStringLiteral("<v:shape id=\"_x0000_s%1\" type=\"#_x0000_t201\" style='position:absolute;margin-left:0pt;margin-top:0pt;width:50pt;height:20pt;z-index:%1' filled=\"f\" stroked=\"f\" o:insetmode=\"auto\">"
                          "<v:textbox style='mso-direction-alt:auto'><div style='text-align:left'><font face=\"Segoe UI\" size=\"160\" color=\"auto\">%4</font></div></v:textbox>"
                          "<x:ClientData ObjectType=\"%2\"><x:SizeWithCells/><x:Anchor>%3</x:Anchor><x:AutoFill>False</x:AutoFill>%5</x:ClientData></v:shape>")
        .arg(id)
        .arg(type, anchor, words, more);
}

// Sheet "Sheet1" of editBook() (A1 = 1, A2 = 10, A3 = 0.25) with eight controls beside it: E1:E6 are the cells they are linked to.
QByteArray controlBook(const QString &extraSheetXml = QString(), const QByteArray &extraRows = QByteArray(), const QString &sheetViews = QString())
{
    return repack(editBook(extraRows), [&](Parts &p) {
        QByteArray &types = p[QStringLiteral("[Content_Types].xml")];
        if (!types.contains("Extension=\"vml\"")) types.insert(types.indexOf("<Override"), "<Default Extension=\"vml\" ContentType=\"application/vnd.openxmlformats-officedocument.vmlDrawing\"/>");
        addOverride(p, QStringLiteral("xl/ctrlProps/ctrlProp1.xml"), QStringLiteral("application/vnd.ms-excel.controlproperties+xml"));
        p[QStringLiteral("xl/ctrlProps/ctrlProp1.xml")] = "<formControlPr xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\" objectType=\"CheckBox\" fmlaLink=\"$E$1\" lockText=\"1\"/>";
        const QString shapes =
            shape(1025, QStringLiteral("Checkbox"), QStringLiteral("5, 2, 0, 2, 7, 0, 1, 0"), QStringLiteral("Check Box 1"), QStringLiteral("<x:FmlaLink>$E$1</x:FmlaLink>"))
            + shape(1026, QStringLiteral("Radio"), QStringLiteral("5, 2, 2, 2, 7, 0, 3, 0"), QStringLiteral("Option Button 2"), QStringLiteral("<x:FirstButton/><x:FmlaLink>$E$2</x:FmlaLink>"))
            + shape(1027, QStringLiteral("Radio"), QStringLiteral("5, 2, 3, 2, 7, 0, 4, 0"), QStringLiteral("Option Button 3"), QStringLiteral("<x:FmlaLink>$E$2</x:FmlaLink>"))
            + shape(1028, QStringLiteral("Drop"), QStringLiteral("9, 2, 0, 2, 11, 0, 1, 0"), QString(), QStringLiteral("<x:Sel>1</x:Sel><x:DropLines>8</x:DropLines><x:FmlaLink>$E$3</x:FmlaLink><x:FmlaRange>$A$1:$A$3</x:FmlaRange>"))
            + shape(1029, QStringLiteral("Spin"), QStringLiteral("9, 2, 2, 2, 9, 22, 4, 2"), QString(), QStringLiteral("<x:Val>1</x:Val><x:Min>0</x:Min><x:Max>5</x:Max><x:Inc>2</x:Inc><x:FmlaLink>$E$4</x:FmlaLink>"))
            + shape(1030, QStringLiteral("Button"), QStringLiteral("9, 2, 5, 2, 11, 0, 6, 2"), QStringLiteral("Run it"), QStringLiteral("<x:FmlaMacro>Sheet1.Run</x:FmlaMacro><x:PrintObject>False</x:PrintObject>"))
            + shape(1031, QStringLiteral("List"), QStringLiteral("12, 2, 0, 2, 14, 0, 3, 0"), QString(), QStringLiteral("<x:SelType>Single</x:SelType><x:FmlaLink>$E$5</x:FmlaLink><x:FmlaRange>$A$1:$A$3</x:FmlaRange>"))
            + shape(1032, QStringLiteral("Scroll"), QStringLiteral("12, 2, 4, 0, 12, 18, 8, 0"),
                    QString(), QStringLiteral("<x:Val>50</x:Val><x:Min>0</x:Min><x:Max>100</x:Max><x:Inc>1</x:Inc><x:Page>10</x:Page><x:FmlaLink>$E$6</x:FmlaLink>"))
            + shape(1033, QStringLiteral("GroupBox"), QStringLiteral("15, 2, 0, 2, 17, 0, 3, 0"), QStringLiteral("Group Box 9"))
            + shape(1034, QStringLiteral("Label"), QStringLiteral("15, 2, 4, 2, 17, 0, 5, 0"), QStringLiteral("Label 10"));
        p[QStringLiteral("xl/drawings/vmlDrawing1.vml")] =
            "<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\">"
            "<o:shapelayout v:ext=\"edit\"><o:idmap v:ext=\"edit\" data=\"1\"/></o:shapelayout>"
            "<v:shapetype id=\"_x0000_t201\" coordsize=\"21600,21600\" o:spt=\"201\" path=\"m,l,21600r21600,l21600,xe\"><v:stroke joinstyle=\"miter\"/></v:shapetype>"
                + shapes.toUtf8() + "</xml>";
        relate(p, QStringLiteral("rIdVml"), QStringLiteral("vmlDrawing"), QStringLiteral("../drawings/vmlDrawing1.vml"));
        relate(p, QStringLiteral("rIdCtl"), QStringLiteral("ctrlProp"), QStringLiteral("../ctrlProps/ctrlProp1.xml"));
        if (!sheetViews.isEmpty()) p[kSheet].replace("<sheetData>", sheetViews.toUtf8() + "<sheetData>");
        p[kSheet].replace("</worksheet>",
                          "<legacyDrawing r:id=\"rIdVml\"/><controls><mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"x14\">"
                          "<control shapeId=\"1025\" r:id=\"rIdCtl\" name=\"Check Box 1\"><controlPr defaultSize=\"0\" autoFill=\"0\" linked=\"1\"><anchor moveWithCells=\"1\"><from>"
                          "<xdr:col xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\">5</xdr:col></from></anchor></controlPr></control></mc:Choice></mc:AlternateContent></controls>"
                              + extraSheetXml.toUtf8() + "</worksheet>");
    });
}

QMap<QString, QByteArray> partsOf(const QString &path)
{
    QMap<QString, QByteArray> parts;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
    return parts;
}

} // namespace

class CrunchControlTests : public QObject {
    Q_OBJECT

    static QImage shot(EditWindow &e)
    {
        e.view()->viewport()->repaint();
        return e.view()->viewport()->grab().toImage();
    }
    static bool dark(const QImage &img, const QRect &r)
    {
        const qreal ratio = img.devicePixelRatio();
        for (int y = r.top(); y <= r.bottom(); ++y)
            for (int x = r.left(); x <= r.right(); ++x)
                if (qGray(img.pixel(qRound(x * ratio), qRound(y * ratio))) < 110) return true;
        return false;
    }
    // The layer's object of that name (the controls are named as the file names them), its id, and where it is on the screen.
    static int idOf(EditWindow &e, const QString &name)
    {
        jp::grid::ObjectLayer *layer = e.win.w.objects()->layer();
        for (int i = 0; i < layer->count(); ++i)
            if (layer->object(i).name == name) return layer->object(i).id;
        return -1;
    }
    static QRect rectOf(EditWindow &e, const QString &name) { return e.win.w.objects()->layer()->deviceRect(idOf(e, name), jp::grid::Pane::Body); }
    static void click(EditWindow &e, const QPoint &at) { QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, at); }
    // How many objects the layer has for the first sheet (-1 until it is shown).
    static int shownCount(EditWindow &e) { return e.win.w.objects() && e.win.w.objects()->shownSheet() == 0 ? e.win.w.objects()->layer()->count() : -1; }

private:
    // The shapes of the first drawing of a saved file: shape number to its bytes.
    static QMap<int, QByteArray> savedShapes(const QString &path, QByteArray *sheetXml = nullptr)
    {
        QMap<int, QByteArray> out;
        const QMap<QString, QByteArray> parts = partsOf(path);
        if (sheetXml) *sheetXml = parts.value(kSheet);
        for (auto it = parts.begin(); it != parts.end(); ++it) {
            if (!it.key().endsWith(QLatin1String(".vml"))) continue;
            static const QRegularExpression shapeRe(QStringLiteral("<v:shape id=\"_x0000_s(\\d+)\".*?</v:shape>"), QRegularExpression::DotMatchesEverythingOption);
            const QString text = QString::fromUtf8(it.value());
            for (auto m = shapeRe.globalMatch(text); m.hasNext();) {
                const QRegularExpressionMatch hit = m.next();
                out.insert(hit.captured(1).toInt(), hit.captured(0).toUtf8());
            }
        }
        return out;
    }
    static void letRun()
    {
        QCoreApplication::processEvents();
        QCoreApplication::sendPostedEvents(nullptr, 0);
        QCoreApplication::processEvents();
    }

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // Every kind is a place on the sheet: drawn (dark ink inside its anchor, none where it is not), named as the file names it.
    void everyKindIsDrawnWhereItsAnchorPutsIt()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        e.win.w.objects()->layer()->prepareAll();
        const QImage img = shot(e);
        const grid::GridLayout &L = e.view()->sheet().layout();
        QCOMPARE(idOf(e, QStringLiteral("Check Box 1")) >= 0, true);
        const QRect box = rectOf(e, QStringLiteral("Check Box 1"));
        QCOMPARE(box.left(), L.rangeRect(0, 5, 0, 6).left() + 2);
        QVERIFY(dark(img, box));
        // beside it (column E, where the cell it sets is) there is nothing dark
        QVERIFY(!dark(img, L.rangeRect(0, 4, 0, 4)));
        // the spin button and the scroll bar are as wide as their anchors
        QCOMPARE(rectOf(e, QStringLiteral("Spinner 5")).width(), 20);
        QCOMPARE(rectOf(e, QStringLiteral("Scroll Bar 8")).width(), 16);
    }

    // Zoom scales a control, and a control whose rows are hidden is not there.
    void controlsFollowHiddenRowsAndZoom()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook(QString(), "<row r=\"6\" hidden=\"1\"><c r=\"A6\"><v>1</v></c></row><row r=\"7\" hidden=\"1\"><c r=\"A7\"><v>1</v></c></row>")));
        QTRY_COMPARE(shownCount(e), 10);
        QVERIFY(rectOf(e, QStringLiteral("Button 6")).isEmpty());   // (from row 6 to row 7, both hidden)
        const QRect before = rectOf(e, QStringLiteral("Check Box 1"));
        QVERIFY(before.width() > 10);
        e.view()->setZoom(2.0);
        QTRY_VERIFY(rectOf(e, QStringLiteral("Check Box 1")).width() >= before.width() * 2 - 3);
    }

    // Frozen rows keep their controls in the frozen part.
    void controlsInFrozenRowsStayInTheFrozenPart()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook(QString(), QByteArray(), QStringLiteral("<sheetViews><sheetView workbookViewId=\"0\"><pane ySplit=\"2\" topLeftCell=\"A3\" activePane=\"bottomLeft\" state=\"frozen\"/></sheetView></sheetViews>"))));
        QTRY_COMPARE(shownCount(e), 10);
        const int id = idOf(e, QStringLiteral("Check Box 1"));
        QVERIFY(!e.win.w.objects()->layer()->deviceRect(id, jp::grid::Pane::Top).isEmpty());
        QVERIFY(e.win.w.objects()->layer()->deviceRect(id, jp::grid::Pane::Body).isEmpty());
    }

    // The printed page has the controls the file lets print (a button that says it does not is left off), each drawn from the control's state.
    void controlsPrintUnlessTheFileSaysNot()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        const std::vector<jp::grid::LayerObject> printed = e.win.w.objects()->printObjects(0);
        QCOMPARE(int(printed.size()), 9);
        for (const jp::grid::LayerObject &o : printed) {
            QVERIFY2(o.name != QStringLiteral("Button 6"), "the button that does not print is on the page");
            QVERIFY(bool(o.render));
        }
        jp::grid::RenderRequest q;
        q.pixels = QSize(120, 40);
        q.units = QSizeF(60, 20);
        q.zoom = 2;
        q.inner = QRectF(0, 0, 120, 40);
        const QImage picture = printed.front().render(q);
        QVERIFY(!picture.isNull());
        bool ink = false;
        for (int y = 0; y < picture.height() && !ink; ++y)
            for (int x = 0; x < picture.width() && !ink; ++x) ink = qAlpha(picture.pixel(x, y)) > 0;
        QVERIFY(ink);
    }

    // A check box writes TRUE or FALSE in its linked cell (one step of Undo), and the cell changed by hand changes the box.
    void aCheckBoxWritesItsLinkedCell()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        e.win.w.objects()->layer()->prepareAll();
        const QRect box = rectOf(e, QStringLiteral("Check Box 1"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("-"));
        click(e, box.center());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("b:1"));
        click(e, box.center());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("b:0"));
        pressUndo(e.view());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("b:1"));
        // the cell typed over: the box follows (its picture is drawn again with a check)
        e.win.w.objects()->layer()->prepareAll();
        const QImage checked = shot(e);
        e.select(QStringLiteral("E1"));
        e.enter(QStringLiteral("FALSE"));
        QVERIFY(e.settle());
        e.win.w.objects()->layer()->prepareAll();
        QVERIFY(shot(e) != checked);
    }

    // Option buttons that share a cell write their place in it.
    void optionButtonsWriteTheirPlace()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        click(e, rectOf(e, QStringLiteral("Option Button 3")).center());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E2")), QStringLiteral("n:2"));
        click(e, rectOf(e, QStringLiteral("Option Button 2")).center());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E2")), QStringLiteral("n:1"));
        pressUndo(e.view());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E2")), QStringLiteral("n:2"));
    }

    // A spin button steps its cell inside its limits, a scroll bar by its arrows and its pages.
    void spinButtonsAndScrollBarsStepTheirCell()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        const QRect spin = rectOf(e, QStringLiteral("Spinner 5"));
        click(e, QPoint(spin.center().x(), spin.top() + 3));   // the upper half: up by the step (1 and 2: 3)
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E4")), QStringLiteral("n:3"));
        click(e, QPoint(spin.center().x(), spin.top() + 3));
        QVERIFY(e.settle());
        click(e, QPoint(spin.center().x(), spin.top() + 3));   // (it stops at the largest: 5)
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E4")), QStringLiteral("n:5"));
        click(e, QPoint(spin.center().x(), spin.bottom() - 3));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E4")), QStringLiteral("n:3"));
        const QRect bar = rectOf(e, QStringLiteral("Scroll Bar 8"));
        click(e, QPoint(bar.center().x(), bar.top() + 3));   // the arrow at the top: down by 1
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E6")), QStringLiteral("n:49"));
        click(e, QPoint(bar.center().x(), bar.bottom() - 25));   // the track below the thumb: a page on
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E6")), QStringLiteral("n:59"));
    }

    // A list box writes the place of the item clicked; a drop-down opens its items and writes the one chosen.
    void listsWriteThePlaceChosen()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        const QRect list = rectOf(e, QStringLiteral("List Box 7"));
        click(e, QPoint(list.left() + 10, list.top() + 1 + 16 + 8));   // the second line
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("n:2"));
        const QRect drop = rectOf(e, QStringLiteral("Drop Down 4"));
        click(e, drop.center());
        QTRY_VERIFY(qobject_cast<QMenu *>(QApplication::activePopupWidget()));
        QMenu *menu = qobject_cast<QMenu *>(QApplication::activePopupWidget());
        QCOMPARE(menu->actions().size(), 3);
        QCOMPARE(menu->actions().at(1)->text(), QStringLiteral("10"));
        menu->actions().at(2)->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E3")), QStringLiteral("n:3"));
    }

    // A button runs no macro: a click changes nothing, and it says so.
    void aButtonDoesNothingAndSaysSo()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        const QRect button = rectOf(e, QStringLiteral("Button 6"));
        QVERIFY(button.isValid());
        QTest::mouseMove(e.view()->viewport(), button.center());
        QTRY_VERIFY(QToolTip::isVisible());
        QVERIFY2(QToolTip::text().contains(QStringLiteral("macro")), qPrintable(QToolTip::text()));
        click(e, button.center());
        QVERIFY(!e.win.w.isModified());
    }

    // The state is saved, and every other byte of the file is as it was.
    void theStateIsSavedAndNothingElseChanges()
    {
        EditWindow e;
        const QByteArray original = controlBook();
        QVERIFY(e.open(original));
        QTRY_COMPARE(shownCount(e), 10);
        click(e, rectOf(e, QStringLiteral("Check Box 1")).center());
        QVERIFY(e.settle());
        const QRect spin = rectOf(e, QStringLiteral("Spinner 5"));
        click(e, QPoint(spin.center().x(), spin.top() + 3));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        QVERIFY2(parts.value(QStringLiteral("xl/ctrlProps/ctrlProp1.xml")).contains("checked=\"Checked\""), parts.value(QStringLiteral("xl/ctrlProps/ctrlProp1.xml")).constData());
        const QByteArray vml = parts.value(QStringLiteral("xl/drawings/vmlDrawing1.vml"));
        QVERIFY(vml.contains("<x:Checked>1</x:Checked>"));
        QVERIFY(vml.contains("<x:Val>3</x:Val>"));
        // the shapes of the controls that did not change are the bytes they were
        const QMap<QString, QByteArray> before = [&] {
            QMap<QString, QByteArray> m;
            jp::readZip(original, m);
            return m;
        }();
        const QByteArray was = before.value(QStringLiteral("xl/drawings/vmlDrawing1.vml"));
        const qsizetype from = was.indexOf("<v:shape id=\"_x0000_s1026\"");
        QVERIFY(from > 0);
        QVERIFY(vml.contains(was.mid(from, was.indexOf("</v:shape>", from) - from)));
        QVERIFY(vml.startsWith(was.left(was.indexOf("<v:shape"))));
        // and the cell they set is in the file
        QVERIFY(QRegularExpression(QStringLiteral("<c r=\"E1\"[^>]*t=\"b\"[^>]*><v>1</v>")).match(QString::fromUtf8(parts.value(kSheet))).hasMatch());
        // reopened, the spin button's cell holds 3
        EditWindow again;
        QFile saved1(e.path);
        QVERIFY(saved1.open(QIODevice::ReadOnly));
        QVERIFY(again.open(saved1.readAll()));
        QTRY_COMPARE(shownCount(again), 10);
        QCOMPARE(again.valueAt(QStringLiteral("E4")), QStringLiteral("n:3"));
    }

    // Insert > Form Controls: a control of each kind goes on the sheet at the cell chosen, as one step of Undo; saved, it is a valid shape of the legacy drawing, and the file opens in
    // JeffCrunch with the control where it was.
    void formControlsCanBeInsertedAndReopen()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QTRY_VERIFY(e.win.w.objects() && e.win.w.objects()->shownSheet() == 0);
        const QStringList ids = {QStringLiteral("insert.control.checkBox"), QStringLiteral("insert.control.optionButton"), QStringLiteral("insert.control.dropDown"), QStringLiteral("insert.control.button"),
                                 QStringLiteral("insert.control.label"), QStringLiteral("insert.control.groupBox"), QStringLiteral("insert.control.spinButton"), QStringLiteral("insert.control.scrollBar"),
                                 QStringLiteral("insert.control.listBox")};
        int row = 5;
        for (const QString &id : ids) {
            QAction *a = e.win.w.act(id);
            QVERIFY2(a, qPrintable(id));
            e.select(QStringLiteral("G%1").arg(row));
            a->trigger();
            row += 3;
        }
        QTRY_COMPARE(e.win.w.objects()->layer()->count(), int(ids.size()));
        pressUndo(e.view());
        QCOMPARE(e.win.w.objects()->layer()->count() - 0, int(ids.size()));   // (the layer keeps the place of an object taken away; it is not drawn)
        QVERIFY(e.win.w.objects()->layer()->isDeleted(e.win.w.objects()->layer()->object(int(ids.size()) - 1).id));
        pressRedo(e.view());
        QVERIFY(!e.win.w.objects()->layer()->isDeleted(e.win.w.objects()->layer()->object(int(ids.size()) - 1).id));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        QString vmlName;
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().endsWith(QLatin1String(".vml"))) vmlName = it.key();
        QVERIFY2(!vmlName.isEmpty(), "the drawing is in the file");
        const QByteArray vml = parts.value(vmlName);
        QCOMPARE(vml.count("<x:ClientData ObjectType="), int(ids.size()));
        QVERIFY(vml.contains("ObjectType=\"Checkbox\"") && vml.contains("ObjectType=\"Scroll\"") && vml.contains("ObjectType=\"List\""));
        QDomDocument doc;
        QVERIFY2(doc.setContent(vml), "the drawing is well-formed XML");
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains(("/" + vmlName).toUtf8()) || parts.value(QStringLiteral("[Content_Types].xml")).contains("Extension=\"vml\""));
        QVERIFY(parts.value(kSheet).contains("<legacyDrawing"));
        EditWindow again;
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(again.open(f.readAll()));
        QTRY_COMPARE(again.win.w.objects()->layer()->count(), int(ids.size()));
    }

    // The acceptance set (not part of the suite: JO_CONTROL_ACCEPT names a text file with one workbook path to a line, kept outside the repository): each workbook opens, every sheet's controls are drawn
    // where their anchors put them (the picture over the control's place differs from the one without it), and a click on a control that sets a cell writes it. One line of totals is printed.
    void acceptanceSetFromTheEnvironment()
    {
        const QByteArray listPath = qgetenv("JO_CONTROL_ACCEPT");
        if (listPath.isEmpty()) QSKIP("JO_CONTROL_ACCEPT names a list of workbooks");
        QFile list(QString::fromLocal8Bit(listPath));
        QVERIFY(list.open(QIODevice::ReadOnly));
        int notLooked = 0, files = 0, controls = 0, drawn = 0, undrawn = 0, offscreen = 0, hiddenOnes = 0, clicked = 0, wrote = 0, refused = 0;
        QStringList problems;
        for (const QByteArray &line : list.readAll().split('\n')) {
            const QString path = QString::fromLocal8Bit(line).trimmed();
            if (path.isEmpty()) continue;
            QFile f(path);
            QVERIFY2(f.open(QIODevice::ReadOnly), qPrintable(path));
            EditWindow e;
            QVERIFY2(e.open(f.readAll()), qPrintable(path));
            ++files;
            crunch::SheetTabBar *tabs = e.win.w.sheetTabs();
            int inFile = 0;
            for (int tab = 0; tab < tabs->count(); ++tab) {
                QTest::mouseClick(tabs, Qt::LeftButton, Qt::NoModifier, tabs->tabRect(tab).center());
                crunch::SheetObjects *o = e.win.w.objects();
                QTRY_VERIFY(o->shownSheet() == e.win.w.activeSheet());
                const int sheet = e.win.w.activeSheet();
                const int here = o->controls()->countOf(sheet);
                if (here == 0) continue;
                inFile += here;
                controls += here;
                QTest::qWait(30);
                o->layer()->prepareAll();
                int looked = 0;
                for (int i = 0; i < o->layer()->count(); ++i) {
                    const jp::grid::LayerObject obj = o->layer()->object(i);
                    if (!crunch::isControlId(obj.id)) continue;
                    if (++looked > 12) {
                        ++notLooked;   // (the first twelve of a sheet are looked at: each takes two pictures of the window)
                        continue;
                    }
                    const jp::office::sheet::FormControl c = o->controls()->controlOf(sheet, obj.id);
                    const QRect view(QPoint(0, 0), e.view()->viewport()->size());
                    QRect r = o->layer()->deviceRect(obj.id, jp::grid::Pane::Body).intersected(view);
                    if (r.isEmpty()) {
                        // (scrolled out of sight: the sheet is scrolled to its cell)
                        e.view()->selectCell(c.anchor.from.row, c.anchor.from.col);
                        QCoreApplication::processEvents();
                        r = o->layer()->deviceRect(obj.id, jp::grid::Pane::Body).intersected(view);
                    }
                    if (r.width() < 8 || r.height() < 8) {   // (nothing of it, or a strip, is in view)
                        const jp::grid::ObjectBox b = o->layer()->boxNow(obj.id);
                        if (b.isEmpty()) ++hiddenOnes;
                        else ++offscreen;
                        continue;
                    }
                    o->layer()->prepareAll();
                    const QImage with = shot(e);
                    o->layer()->setDeleted(obj.id, true);
                    const QImage without = shot(e);
                    o->layer()->setDeleted(obj.id, false);
                    bool differs = false;
                    const qreal ratio = with.devicePixelRatio();
                    for (int y = r.top(); y <= r.bottom() && !differs; ++y)
                        for (int x = r.left(); x <= r.right() && !differs; ++x) differs = with.pixel(qRound(x * ratio), qRound(y * ratio)) != without.pixel(qRound(x * ratio), qRound(y * ratio));
                    if (differs) ++drawn;
                    else {
                        ++undrawn;
                        const jp::grid::ObjectBox b = o->layer()->boxNow(obj.id);
                        problems << QStringLiteral("%1: %2 (%3) is not drawn: box %4,%5 %6x%7 shown %8,%9 %10x%11 from %12,%13 to %14,%15")
                                        .arg(QFileInfo(path).fileName(), c.name)
                                        .arg(int(c.kind))
                                        .arg(b.x).arg(b.y).arg(b.w).arg(b.h).arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height())
                                        .arg(c.anchor.from.col).arg(c.anchor.from.row).arg(c.anchor.to.col).arg(c.anchor.to.row);
                    }
                    // a click that sets a cell writes it
                    int cs = 0;
                    quint32 row = 0, col = 0;
                    if (c.kind == jp::office::sheet::ControlKind::Button || !o->controls()->linkOf(sheet, obj.id, &cs, &row, &col) || cs != sheet) continue;
                    if (c.kind == jp::office::sheet::ControlKind::Drop) continue;
                    ++clicked;
                    const QString before = e.valueAt(cellName(row, col));
                    click(e, r.center());
                    e.settle();   // (a workbook that is still working out is looked at as it is: the cell is typed at once)
                    const QString after = e.valueAt(cellName(row, col));
                    if (after != before) ++wrote;
                    else ++refused;
                }
            }
            qInfo("accept %s: %d controls", qPrintable(QFileInfo(path).fileName().left(40)), inFile);
        }
        qInfo("ACCEPT files=%d controls=%d drawn=%d undrawn=%d hidden=%d offscreen=%d not-looked-at=%d clicked=%d wrote=%d unchanged=%d", files, controls, drawn, undrawn, hiddenOnes, offscreen, notLooked,
          clicked, wrote, refused);
        for (const QString &p : problems.mid(0, 20)) qInfo("  %s", qPrintable(p));
        QCOMPARE(undrawn, 0);
    }

    // A control with no cell changes its own state, as one step of Undo, and the state is saved.
    void aControlWithNoCellChangesItsOwnState()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QTRY_VERIFY(e.win.w.objects() && e.win.w.objects()->shownSheet() == 0);
        e.select(QStringLiteral("G5"));
        QAction *insert = e.win.w.act(QStringLiteral("insert.control.checkBox"));
        QVERIFY(insert);
        insert->trigger();
        QTRY_COMPARE(e.win.w.objects()->layer()->count(), 1);
        e.win.w.objects()->layer()->prepareAll();
        const int id = e.win.w.objects()->layer()->object(0).id;
        const QImage unchecked = shot(e);
        const QRect box = e.win.w.objects()->layer()->deviceRect(id, jp::grid::Pane::Body);
        QVERIFY(box.isValid());
        click(e, box.center());
        e.win.w.objects()->layer()->prepareAll();
        QVERIFY(shot(e) != unchecked);
        QVERIFY(e.win.w.isModified());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QByteArray vml;
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().endsWith(QLatin1String(".vml"))) vml = it.value();
        QVERIFY(vml.contains("<x:Checked>1</x:Checked>"));
        pressUndo(e.view());   // the click
        e.win.w.objects()->layer()->prepareAll();
        QCOMPARE(shot(e), unchecked);
        pressRedo(e.view());
        e.win.w.objects()->layer()->prepareAll();
        QVERIFY(shot(e) != unchecked);
    }

    // A note and a control made in one session take shapes of their own in the one drawing, also in the saves that follow (the control is found by the number the first save gave it).
    void aNoteAndAControlHaveShapesOfTheirOwn()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QTRY_VERIFY(e.win.w.objects() && e.win.w.objects()->shownSheet() == 0);
        e.select(QStringLiteral("G5"));
        e.win.w.act(QStringLiteral("insert.control.checkBox"))->trigger();
        QTRY_COMPARE(e.win.w.objects()->layer()->count(), 1);
        QVERIFY(e.win.w.session()->setNote(0, 0, 2, QStringLiteral("hello"), QStringLiteral("me"), false, QStringLiteral("Note")).ok);
        {
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY2(savedOk(saved, 20000), "the first save failed");
        }
        QMap<int, QByteArray> shapes = savedShapes(e.path);
        QCOMPARE(shapes.size(), 2);
        // the box is checked and saved again: its own shape, and not the note's, has the state
        e.win.w.objects()->layer()->prepareAll();
        const int id = e.win.w.objects()->layer()->object(0).id;
        click(e, e.win.w.objects()->layer()->deviceRect(id, jp::grid::Pane::Body).center());
        {
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY2(savedOk(saved, 20000), "the second save failed");
        }
        shapes = savedShapes(e.path);
        QCOMPARE(shapes.size(), 2);
        int boxes = 0, notes = 0;
        for (auto it = shapes.begin(); it != shapes.end(); ++it) {
            if (it.value().contains("ObjectType=\"Checkbox\"")) {
                ++boxes;
                QVERIFY(it.value().contains("<x:Checked>1</x:Checked>"));
            } else if (it.value().contains("ObjectType=\"Note\"")) {
                ++notes;
                QVERIFY(!it.value().contains("<x:Checked>"));
            }
        }
        QCOMPARE(boxes, 1);
        QCOMPARE(notes, 1);
    }

    // Rows inserted above a control move it, and the cell it is linked to; saved, its shape and its properties name them.
    void controlsFollowRowsInserted()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        const QRect before = rectOf(e, QStringLiteral("Spinner 5"));
        const grid::GridLayout &L = e.view()->sheet().layout();
        QVERIFY(e.win.w.session()->insertRows(0, 0, 1).ok);
        letRun();
        const int shift = L.rangeRect(1, 0, 1, 0).top() - L.rangeRect(0, 0, 0, 0).top();
        QTRY_COMPARE(rectOf(e, QStringLiteral("Spinner 5")).top(), before.top() + shift);
        // its cell was E4: it is E5 now
        const QRect spin = rectOf(e, QStringLiteral("Spinner 5"));
        click(e, QPoint(spin.center().x(), spin.top() + 3));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("n:3"));
        QCOMPARE(e.valueAt(QStringLiteral("E4")), QStringLiteral("-"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray shape = savedShapes(e.path).value(1029);
        QVERIFY2(shape.contains("<x:Anchor>9, 2, 3, 2, 9, 22, 5, 2</x:Anchor>"), shape.constData());
        QVERIFY2(QRegularExpression(QStringLiteral("<x:FmlaLink>[^<]*\\$?E\\$?5</x:FmlaLink>")).match(QString::fromUtf8(shape)).hasMatch(), shape.constData());
    }

    // A control whose cell is deleted is gone, saved without it, and Undo brings it back.
    void aControlWhoseRowIsDeletedIsDropped()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        const int box = idOf(e, QStringLiteral("Check Box 1"));
        QVERIFY(!rectOf(e, QStringLiteral("Check Box 1")).isEmpty());
        QVERIFY(e.win.w.session()->deleteRows(0, 0, 2).ok);   // (rows 1 and 2: the check box begins in the first)
        letRun();
        QTRY_VERIFY(e.win.w.objects()->layer()->isDeleted(box));
        QVERIFY(rectOf(e, QStringLiteral("Check Box 1")).isEmpty());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QByteArray sheetXml;
        const QMap<int, QByteArray> shapes = savedShapes(e.path, &sheetXml);
        QVERIFY(!shapes.contains(1025));
        QVERIFY(shapes.contains(1026));
        QVERIFY(!sheetXml.contains("shapeId=\"1025\""));
        QVERIFY2(shapes.value(1026).contains("<x:Anchor>5, 2, 0, 2, 7, 0, 1, 0</x:Anchor>"), shapes.value(1026).constData());
    }

    void deletedRowsUndoneBringTheControlBack()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        const int box = idOf(e, QStringLiteral("Check Box 1"));
        QVERIFY(e.win.w.session()->deleteRows(0, 0, 2).ok);
        letRun();
        QTRY_VERIFY(e.win.w.objects()->layer()->isDeleted(box));
        pressUndo(e.view());
        letRun();
        QTRY_VERIFY(!e.win.w.objects()->layer()->isDeleted(box));
        QVERIFY(!rectOf(e, QStringLiteral("Check Box 1")).isEmpty());
    }

    // A control picked (Ctrl and a click) and Delete: it goes, as one step of Undo, and the saved file lacks it.
    void aControlIsDeletedWithTheDeleteKey()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        const int box = idOf(e, QStringLiteral("Check Box 1"));
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, rectOf(e, QStringLiteral("Check Box 1")).center());
        QCOMPARE(e.win.w.objects()->layer()->selectedId(), box);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(e.win.w.objects()->layer()->isDeleted(box));
        QVERIFY(e.win.w.isModified());
        pressUndo(e.view());
        QVERIFY(!e.win.w.objects()->layer()->isDeleted(box));
        QVERIFY(!e.win.w.isModified());
        pressRedo(e.view());
        QVERIFY(e.win.w.objects()->layer()->isDeleted(box));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QByteArray sheetXml;
        const QMap<int, QByteArray> shapes = savedShapes(e.path, &sheetXml);
        QVERIFY(!shapes.contains(1025));
        QCOMPARE(shapes.size(), 9);
        QVERIFY(!sheetXml.contains("shapeId=\"1025\""));
        // and the file opens with nine
        EditWindow again;
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(again.open(f.readAll()));
        QTRY_COMPARE(shownCount(again), 9);
    }

    // Format Control: the cell, the range, and the limits of a control are changed (one step), and written.
    void formatControlChangesTheLinkAndTheLimits()
    {
        EditWindow e;
        QVERIFY(e.open(controlBook()));
        QTRY_COMPARE(shownCount(e), 10);
        const int id = idOf(e, QStringLiteral("Spinner 5"));
        QVERIFY(id >= 0);
        bool changed = false;
        QVERIFY(QMetaObject::invokeMethod(&e.win.w, "setControlProperties", Qt::DirectConnection, Q_RETURN_ARG(bool, changed), Q_ARG(int, id), Q_ARG(QString, QStringLiteral("$E$5")), Q_ARG(QString, QString()),
                                          Q_ARG(int, 0), Q_ARG(int, 9), Q_ARG(int, 3), Q_ARG(int, 10), Q_ARG(bool, false)));
        QVERIFY(changed);
        const QRect spin = rectOf(e, QStringLiteral("Spinner 5"));
        click(e, QPoint(spin.center().x(), spin.top() + 3));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("n:4"));   // (1 and the step 3, in the new cell)
        pressUndo(e.view());   // the cell
        pressUndo(e.view());   // the properties
        QVERIFY(e.settle());
        click(e, QPoint(spin.center().x(), spin.top() + 3));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E4")), QStringLiteral("n:3"));
    }
};

JP_TEST_CLASS(CrunchControlTests)
#include "tst_crunch_controls.moc"

// Tests of the form controls of a sheet (libs/office/sheet/formcontrol.h): what the legacy drawing and the control properties parts say of each kind is read, the changes a person makes are
// written into them and nothing else is, a control made here is a shape that reads back, and a save puts it all in the file (made, with its relationships, when the sheet had no drawing).
#include "sheethelpers.h"

#include "sheet/formcontrol.h"
#include "sheet/xlsxsave.h"

#include <QDomDocument>

namespace {

QString shape(int id, const QString &type, const QString &anchor, const QString &words, const QString &more = QString())
{
    return QStringLiteral("<v:shape id=\"_x0000_s%1\" type=\"#_x0000_t201\" style='position:absolute;margin-left:10pt;margin-top:20pt;width:50pt;height:20pt;z-index:%1%6' o:insetmode=\"auto\">"
                          "<v:textbox style='mso-direction-alt:auto'><div style='text-align:left'><font face=\"Segoe UI\" size=\"160\" color=\"#FF0000\">%4</font></div></v:textbox>"
                          "<x:ClientData ObjectType=\"%2\"><x:SizeWithCells/><x:Anchor>%3</x:Anchor><x:AutoFill>False</x:AutoFill>%5</x:ClientData></v:shape>")
        .arg(id)
        .arg(type, anchor, words, more, QString());
}

QByteArray vmlOf(const QString &shapes)
{
    return ("<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\">"
            "<o:shapelayout v:ext=\"edit\"><o:idmap v:ext=\"edit\" data=\"1\"/></o:shapelayout><v:shapetype id=\"_x0000_t201\" coordsize=\"21600,21600\" o:spt=\"201\"/>"
            + shapes + "</xml>")
        .toUtf8();
}

const QString kShapes = shape(1025, QStringLiteral("Checkbox"), QStringLiteral("1, 4, 2, 8, 3, 38, 5, 0"), QStringLiteral("Check\n   Box &amp; more"), QStringLiteral("<x:Checked>1</x:Checked><x:FmlaLink>$C$3</x:FmlaLink>"))
                        + shape(1026, QStringLiteral("Drop"), QStringLiteral("5, 0, 1, 0, 7, 0, 2, 0"), QString(), QStringLiteral("<x:Sel>2</x:Sel><x:DropLines>5</x:DropLines><x:FmlaLink>$C$4</x:FmlaLink><x:FmlaRange>Sheet1!$A$1:$A$4</x:FmlaRange>"))
                        + shape(1027, QStringLiteral("Spin"), QStringLiteral("5, 0, 3, 0, 5, 16, 5, 0"), QString(), QStringLiteral("<x:Val>3</x:Val><x:Min>1</x:Min><x:Max>9</x:Max><x:Inc>2</x:Inc><x:FmlaLink>$C$5</x:FmlaLink>"))
                        + shape(1028, QStringLiteral("Note"), QStringLiteral("5, 0, 3, 0, 5, 16, 5, 0"), QString(), QStringLiteral("<x:Row>1</x:Row><x:Column>1</x:Column>"));

QByteArray bookWith(const QByteArray &vml, const QByteArray &ctrlProp = QByteArray(), const QString &controlsXml = QString())
{
    QByteArray sheet = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"));
    if (!controlsXml.isEmpty()) sheet.replace("</worksheet>", controlsXml.toUtf8() + "</worksheet>");
    QVector<ExtraPart> parts;
    parts.append({QStringLiteral("xl/drawings/vmlDrawing1.vml"), vml, QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"), 0, QStringLiteral("rIdV"), QStringLiteral("vmlDrawing"), QStringLiteral("../drawings/vmlDrawing1.vml")});
    if (!ctrlProp.isEmpty())
        parts.append({QStringLiteral("xl/ctrlProps/ctrlProp1.xml"), ctrlProp, QStringLiteral("application/vnd.ms-excel.controlproperties+xml"), 0, QStringLiteral("rIdK"), QStringLiteral("ctrlProp"),
                      QStringLiteral("../ctrlProps/ctrlProp1.xml")});
    return withParts(buildXlsx({sheet}), parts);
}

struct Opened {
    Package package;
    Workbook book;
    QVector<FormControl> controls;
    bool ok = false;
};

bool open(const QByteArray &xlsx, Opened *o)
{
    QString why;
    if (!o->package.readLazy(xlsx, &why)) return false;
    if (!o->book.open(o->package, &why)) return false;
    o->controls = readFormControls(o->book.package(), o->book.info().sheets[0].part);
    return o->ok = true;
}

} // namespace

class SheetFormControlTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // Each kind is read from its shape: where it is (the cells and how far into them), its words, its state, its limits, and the cells it sets and lists; a note's shape is not a control.
    void shapesAreReadWithTheirProperties()
    {
        Opened o;
        QVERIFY(open(withParts(buildXlsx({worksheet(QString())}), {}), &o) || true);
        QVERIFY(open(bookWith(vmlOf(kShapes)), &o));
        QCOMPARE(o.controls.size(), 3);
        const FormControl &box = o.controls[0];
        QCOMPARE(box.kind, ControlKind::CheckBox);
        QCOMPARE(box.shapeId, qint64(1025));
        QCOMPARE(box.text, QStringLiteral("Check Box & more"));
        QCOMPARE(box.fontFace, QStringLiteral("Segoe UI"));
        QCOMPARE(box.fontPt, 8.0);
        QCOMPARE(box.color, 0xFFFF0000u);
        QCOMPARE(box.checked, 1);
        QCOMPARE(box.link, QStringLiteral("$C$3"));
        QCOMPARE(box.anchor.from.col, quint32(1));
        QCOMPARE(box.anchor.from.colOff, qint64(4 * 9525));
        QCOMPARE(box.anchor.from.row, quint32(2));
        QCOMPARE(box.anchor.to.col, quint32(3));
        QCOMPARE(box.anchor.to.rowOff, qint64(0));
        QCOMPARE(box.anchor.editAs, AnchorKind::TwoCell);
        QVERIFY(!box.hidden && box.prints);
        const FormControl &drop = o.controls[1];
        QCOMPARE(drop.kind, ControlKind::Drop);
        QCOMPARE(drop.sel, 2);
        QCOMPARE(drop.dropLines, 5);
        QCOMPARE(drop.range, QStringLiteral("Sheet1!$A$1:$A$4"));
        const FormControl &spin = o.controls[2];
        QCOMPARE(spin.kind, ControlKind::Spin);
        QCOMPARE(QVector<int>({spin.val, spin.min, spin.max, spin.inc}), (QVector<int>{3, 1, 9, 2}));
    }

    // The properties part and the sheet's own anchor are read too, and a part with no shape in the drawing is a control all the same.
    void propertiesPartsAndSheetAnchorsAreRead()
    {
        const QByteArray prop = "<formControlPr xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\" objectType=\"Scroll\" val=\"5\" min=\"0\" max=\"50\" inc=\"2\" page=\"7\" horiz=\"1\" fmlaLink=\"$B$2\"/>";
        const QString controls = QStringLiteral("<controls><mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"x14\">"
                                                "<control shapeId=\"2049\" r:id=\"rIdK\" name=\"Scroll Bar 4\"><controlPr defaultSize=\"0\" print=\"0\" macro=\"Sheet1.Go\"><anchor moveWithCells=\"1\">"
                                                "<from><xdr:col xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\">2</xdr:col><xdr:colOff>9525</xdr:colOff><xdr:row>3</xdr:row><xdr:rowOff>0</xdr:rowOff></from>"
                                                "<to><xdr:col>3</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>9</xdr:row><xdr:rowOff>19050</xdr:rowOff></to></anchor></controlPr></control></mc:Choice></mc:AlternateContent></controls>");
        Opened o;
        QVERIFY(open(bookWith(vmlOf(QString()), prop, controls), &o));
        QCOMPARE(o.controls.size(), 1);
        const FormControl &bar = o.controls[0];
        QCOMPARE(bar.kind, ControlKind::Scroll);
        QCOMPARE(bar.name, QStringLiteral("Scroll Bar 4"));
        QCOMPARE(bar.macro, QStringLiteral("Sheet1.Go"));
        QVERIFY(!bar.prints);
        QVERIFY(bar.horizontal);
        QCOMPARE(QVector<int>({bar.val, bar.min, bar.max, bar.inc, bar.page}), (QVector<int>{5, 0, 50, 2, 7}));
        QCOMPARE(bar.link, QStringLiteral("$B$2"));
        QCOMPARE(bar.anchor.from.col, quint32(2));
        QCOMPARE(bar.anchor.to.row, quint32(9));
        QCOMPARE(bar.anchor.to.rowOff, qint64(19050));
        QCOMPARE(bar.ctrlPart, QStringLiteral("/xl/ctrlProps/ctrlProp1.xml"));
    }

    // A change is written into its own shape and nothing else: every other byte of the drawing stays, and no change is no change.
    void aChangeTouchesOnlyItsShape()
    {
        Opened o;
        const QByteArray vml = vmlOf(kShapes);
        QVERIFY(open(bookWith(vml), &o));
        const std::vector<FormControl> was(o.controls.begin(), o.controls.end());
        QByteArray out;
        QVERIFY(patchControlsVml(vml, was, was, &out));
        QCOMPARE(out, vml);
        std::vector<FormControl> now = was;
        now[0].checked = 0;
        now[0].link = QStringLiteral("$D$1");
        now[2].val = 7;
        QVERIFY(patchControlsVml(vml, was, now, &out));
        QVERIFY(out != vml);
        QVERIFY(!out.contains("<x:Checked>"));
        QVERIFY(out.contains("<x:FmlaLink>$D$1</x:FmlaLink>"));
        QVERIFY(out.contains("<x:Val>7</x:Val>"));
        // the drop-down's shape, and the note's, are the bytes they were
        const qsizetype from = vml.indexOf("<v:shape id=\"_x0000_s1026\"");
        QVERIFY(from > 0);
        QVERIFY(out.contains(vml.mid(from, vml.indexOf("</v:shape>", from) - from)));
        QVERIFY(out.startsWith(vml.left(vml.indexOf("<v:shape"))));
    }

    // A control made here is a shape that reads back as it was made, with a shape number of its own.
    void aNewControlReadsBack()
    {
        FormControl c;
        c.kind = ControlKind::List;
        c.isNew = true;
        c.shapeId = 1025;   // (taken: the drawing has it already)
        c.range = QStringLiteral("$A$1:$A$3");
        c.link = QStringLiteral("$B$1");
        c.sel = 2;
        c.anchor.from = {1, 3, 2 * 9525, 4 * 9525};   // (column, row, then how far into them)
        c.anchor.to = {2, 7, 0, 0};
        c.place = QRectF(10, 20, 100, 80);
        QByteArray out;
        QVERIFY(patchControlsVml(vmlOf(kShapes), {}, {c}, &out));
        Opened o;
        QVERIFY(open(bookWith(out), &o));
        QCOMPARE(o.controls.size(), 4);
        const FormControl &made = o.controls[3];
        QCOMPARE(made.kind, ControlKind::List);
        QVERIFY(made.shapeId > 1028);
        QCOMPARE(made.range, QStringLiteral("$A$1:$A$3"));
        QCOMPARE(made.link, QStringLiteral("$B$1"));
        QCOMPARE(made.sel, 2);
        QCOMPARE(made.anchor.to.row, quint32(7));
        QCOMPARE(made.anchor.from.colOff, qint64(2 * 9525));
        QCOMPARE(made.anchor.from.rowOff, qint64(4 * 9525));
        QDomDocument doc;
        QVERIFY(doc.setContent(out));
        // each kind has a shape that reads back as its kind
        for (const ControlKind k : {ControlKind::CheckBox, ControlKind::Radio, ControlKind::Button, ControlKind::Drop, ControlKind::List, ControlKind::Spin, ControlKind::Scroll, ControlKind::GroupBox,
                                    ControlKind::Label}) {
            FormControl n;
            n.kind = k;
            n.isNew = true;
            n.text = QStringLiteral("Words");
            n.anchor.to = {1, 2, 0, 0};
            QByteArray v;
            QVERIFY(patchControlsVml(newControlsDrawing(), {}, {n}, &v));
            QVERIFY(doc.setContent(v));
            Opened p;
            QVERIFY(open(bookWith(v), &p));
            QCOMPARE(p.controls.size(), 1);
            QCOMPARE(p.controls[0].kind, k);
        }
    }

    // A save writes the properties part's state and the drawing's, and a control made on a sheet with no drawing makes the drawing (its part, its type, the sheet's link, and the element that
    // names it).
    void aSavePutsTheControlsInTheFile()
    {
        const QByteArray prop = "<formControlPr xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\" objectType=\"CheckBox\" fmlaLink=\"$C$3\"/>";
        Opened o;
        QVERIFY(open(bookWith(vmlOf(kShapes), prop, QStringLiteral("<controls><mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"x14\">"
                                                                    "<control shapeId=\"1025\" r:id=\"rIdK\" name=\"Check Box 1\"><controlPr defaultSize=\"0\"/></control></mc:Choice></mc:AlternateContent></controls>")),
                     &o));
        QCOMPARE(o.controls.size(), 3);
        SaveChanges changes;
        SheetChanges &sc = changes.sheets[0];
        sc.controlsWas = {o.controls[0]};
        FormControl now = o.controls[0];
        now.checked = 1;
        now.link = QStringLiteral("$D$3");
        sc.controlsNow = {now};
        QByteArray bytes;
        QString error;
        QVERIFY2(saveXlsx(o.book, changes, &bytes, &error), qPrintable(error));
        QMap<QString, QByteArray> parts;
        jp::readZip(bytes, parts);
        QVERIFY2(parts.value(QStringLiteral("xl/ctrlProps/ctrlProp1.xml")).contains("checked=\"Checked\""), parts.value(QStringLiteral("xl/ctrlProps/ctrlProp1.xml")).constData());
        QVERIFY(parts.value(QStringLiteral("xl/ctrlProps/ctrlProp1.xml")).contains("fmlaLink=\"$D$3\""));
        QVERIFY(parts.value(QStringLiteral("xl/drawings/vmlDrawing1.vml")).contains("<x:FmlaLink>$D$3</x:FmlaLink>"));

        // a sheet with no drawing
        Package package;
        QString why;
        QVERIFY(package.readLazy(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"))}), &why));
        Workbook book;
        QVERIFY(book.open(package, &why));
        SaveChanges made;
        FormControl c;
        c.kind = ControlKind::CheckBox;
        c.isNew = true;
        c.shapeId = 1025;
        c.text = QStringLiteral("Ready");
        c.link = QStringLiteral("$A$1");
        c.checked = 1;
        c.anchor.from = {1, 1, 0, 0};
        c.anchor.to = {3, 2, 0, 0};
        made.sheets[0].controlsNow = {c};
        QVERIFY2(saveXlsx(book, made, &bytes, &error), qPrintable(error));
        parts.clear();
        jp::readZip(bytes, parts);
        QString vmlName;
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().endsWith(QLatin1String(".vml"))) vmlName = it.key();
        QVERIFY2(!vmlName.isEmpty(), "the drawing is made");
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains(("/" + vmlName).toUtf8()));
        QVERIFY(parts.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")).contains("vmlDrawing"));
        QVERIFY(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<legacyDrawing"));
        Opened again;
        QVERIFY(open(bytes, &again));
        QCOMPARE(again.controls.size(), 1);
        QCOMPARE(again.controls[0].text, QStringLiteral("Ready"));
        QCOMPARE(again.controls[0].checked, 1);
    }
};

JP_TEST_CLASS(SheetFormControlTests)
#include "tst_sheet_formcontrol.moc"

// Tests of the embedded objects of a sheet (libs/office/sheet/drawing.h, saved by xlsxsave.h): an object of ours put in, saved, and read back with the same bytes and picture; an object of
// another program kept byte for byte (its element, its attributes, its part, its picture); a replaced object leaving no part behind; the caps; and a hostile file. Every workbook is built in
// memory from plain parts.
#include "drawinghelpers.h"

#include "ole/embedobj.h"
#include "sheet/drawing.h"
#include "sheet/notesave.h"
#include "sheet/xlsxsave.h"

using namespace drawtest;

namespace {

QByteArray objectPackage(const QByteArray &body)
{
    QByteArray end(22, '\0');
    end[0] = 'P';
    end[1] = 'K';
    end[2] = 5;
    end[3] = 6;
    return QByteArray("PK\x03\x04", 4) + body + end;
}

struct LimitsGuard {
    ~LimitsGuard() { ole::resetLimits(); }
};

// What one object of a built workbook is: the sheet it is on (from 0), its program, the relationship's type and target, the bytes of its part (not written if `write` is false), its picture, and
// where it is (two cells, from column 1 row 1 to column 4 row 6 unless said).
struct Spec {
    int sheet = 0;
    QString progId = QStringLiteral("Foo.Widget.1");
    int shapeId = 1025;
    QString relType = QStringLiteral("oleObject");
    QString target = QStringLiteral("/xl/embeddings/oleObject1.bin");
    QByteArray bytes;
    bool write = true;
    QByteArray png;
    QString extra;        // other attributes of the oleObject element, as the file writes them
    bool wrap = false;    // inside an AlternateContent, as another program writes it
    QString pictureType = QStringLiteral("image");
    int toCol = 4, toRow = 6;
};

QString childXml(const Spec &s, int n)
{
    const QString anchor = QStringLiteral("<anchor moveWithCells=\"1\"><xdr:from><xdr:col>1</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>1</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from><xdr:to><xdr:col>%1</xdr:col>"
                                          "<xdr:colOff>0</xdr:colOff><xdr:row>%2</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to></anchor>")
                               .arg(s.toCol).arg(s.toRow);
    const QString pic = s.png.isEmpty() ? QString() : QStringLiteral(" r:id=\"rIdP%1\"").arg(n);
    const QString object = QStringLiteral("<oleObject progId=\"%1\" shapeId=\"%2\" r:id=\"rIdO%3\" %4><objectPr defaultSize=\"0\"%5>%6</objectPr></oleObject>")
                               .arg(s.progId).arg(s.shapeId).arg(n).arg(s.extra, pic, anchor);
    if (!s.wrap) return QStringLiteral("<oleObject xmlns:xdr=\"%1\" ").arg(kXdr) + object.mid(11);
    return QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:xdr=\"%1\"><mc:Choice Requires=\"x14\">%2</mc:Choice><mc:Fallback>"
                          "<oleObject progId=\"%3\" shapeId=\"%4\" r:id=\"rIdO%5\"/></mc:Fallback></mc:AlternateContent>")
        .arg(kXdr, object, s.progId).arg(s.shapeId).arg(n);
}

// A workbook of `sheets` sheets with the objects of the specs, each with a package part and a picture part of its own unless `target` is shared.
QByteArray bookWith(const QVector<Spec> &specs, int sheets = 1)
{
    QVector<QByteArray> sheetXml;
    QVector<QString> children(sheets);
    QVector<ExtraPart> parts;
    QSet<QString> written;
    for (int i = 0; i < specs.size(); ++i) {
        const Spec &s = specs[i];
        const int n = i + 1;
        children[s.sheet] += childXml(s, n);
        const QString name = s.target.startsWith(QLatin1Char('/')) ? s.target.mid(1) : s.target;
        // (the part and the relationship to it are one entry; where the part is not to be, or is named again, a part of no use stands in for it)
        const bool real = s.write && !written.contains(name);
        written.insert(name);
        parts.append({real ? name : QStringLiteral("xl/unused/r%1.bin").arg(n), real ? s.bytes : QByteArray("x"), QString(), s.sheet, QStringLiteral("rIdO%1").arg(n), s.relType, s.target});
        if (!s.png.isEmpty())
            parts.append({QStringLiteral("xl/media/p%1.png").arg(n), s.png, QStringLiteral("image/png"), s.sheet, QStringLiteral("rIdP%1").arg(n), s.pictureType, QStringLiteral("/xl/media/p%1.png").arg(n)});
    }
    for (int i = 0; i < sheets; ++i) sheetXml.append(worksheet(QString(), children[i].isEmpty() ? QString() : QStringLiteral("<oleObjects>") + children[i] + QStringLiteral("</oleObjects>")));
    return withParts(buildXlsx(sheetXml), parts);
}

struct Opened {
    Package package;
    Workbook book;
};

bool open(Opened *o, const QByteArray &xlsx, QString *why)
{
    return o->package.readLazy(xlsx, why) && o->book.open(o->package, why);
}

QMap<QString, QByteArray> unzip(const QByteArray &zip)
{
    QMap<QString, QByteArray> parts;
    jp::readZip(zip, parts);
    return parts;
}

QStringList partsUnder(const QMap<QString, QByteArray> &parts, const QString &folder)
{
    QStringList out;
    for (auto it = parts.begin(); it != parts.end(); ++it)
        if (it.key().startsWith(folder)) out << it.key();
    return out;
}

ole::EmbeddedObject ours(const QByteArray &body, int side, const QColor &color, qint64 w = 1828800, qint64 h = 952500)
{
    ole::ObjectBudget budget;
    const auto object = ole::keepObject(ole::progIdOf(ole::Kind::Sheet), objectPackage(body), pngBytes(side, side, color), w, h, budget);
    return object ? *object : ole::EmbeddedObject();
}

DrawingAnchor cells(quint32 c1, quint32 r1, quint32 c2, quint32 r2)
{
    DrawingAnchor a;
    a.kind = AnchorKind::TwoCell;
    a.from.col = c1;
    a.from.row = r1;
    a.to.col = c2;
    a.to.row = r2;
    return a;
}

QVector<SheetOleObject> readAll(const QByteArray &xlsx, int sheet = 0, OleReadState *state = nullptr)
{
    Opened o;
    QString why;
    if (!open(&o, xlsx, &why)) return {};
    OleReadState own;
    OleReadState &st = state ? *state : own;
    std::vector<SheetOleObject> out;
    if (!readSheetOleObjects(o.package, o.book.info().sheets.value(sheet).part, &st, &out)) return {};
    return QVector<SheetOleObject>(out.begin(), out.end());
}

bool save(const QByteArray &xlsx, const SaveChanges &changes, QByteArray *out, QString *why)
{
    Opened o;
    if (!open(&o, xlsx, why)) return false;
    return saveXlsx(o.book, changes, out, why);
}


const QString kMain = QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main");

QByteArray vmlWith(const QByteArray &shapes)
{
    return "<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\">"
           "<o:shapelayout v:ext=\"edit\"><o:idmap v:ext=\"edit\" data=\"1\"/></o:shapelayout>"
           "<v:shapetype id=\"_x0000_t202\" coordsize=\"21600,21600\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\"/>" + shapes + "</xml>";
}

QByteArray noteShape(int id, int row)
{
    return "<v:shape id=\"_x0000_s" + QByteArray::number(id) + "\" type=\"#_x0000_t202\" style=\"position:absolute\"><x:ClientData ObjectType=\"Note\"><x:Anchor>1, 15, 0, 10, 3, 15, 3, 4</x:Anchor><x:Row>"
           + QByteArray::number(row) + "</x:Row><x:Column>0</x:Column></x:ClientData></v:shape>";
}

// A one-sheet workbook with a note on A1 (its comments part and its drawing, with a shape of the number given), whose sheet has `after` after its cells (the legacy drawing is named first).
QByteArray bookWithNote(int noteShapeId, const QString &after = QString(), const QByteArray &moreVml = QByteArray())
{
    const QByteArray sheet = worksheet(QString(), QStringLiteral("<legacyDrawing r:id=\"rIdV\"/>") + after);
    const QByteArray comments = ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<comments xmlns=\"" + kMain + "\"><authors><author>A</author></authors><commentList><comment ref=\"A1\" authorId=\"0\">"
                                 "<text><r><t>hello</t></r></text></comment></commentList></comments>").toUtf8();
    return withParts(buildXlsx({sheet}), {{QStringLiteral("xl/drawings/vmlDrawing1.vml"), vmlWith(noteShape(noteShapeId, 0) + moreVml), QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"), 0, QStringLiteral("rIdV"),
                                           QStringLiteral("vmlDrawing"), QStringLiteral("/xl/drawings/vmlDrawing1.vml")},
                                          {QStringLiteral("xl/comments1.xml"), comments, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml"), 0, QStringLiteral("rIdC"),
                                           QStringLiteral("comments"), QStringLiteral("/xl/comments1.xml")}});
}

QList<qint64> shapeNumbersIn(const QByteArray &xml, const QByteArray &attribute)
{
    QList<qint64> out;
    qsizetype at = 0;
    const QByteArray key = attribute + "=\"_x0000_s";
    while ((at = xml.indexOf(key, at)) >= 0) {
        at += key.size();
        out << xml.mid(at, xml.indexOf('"', at) - at).toLongLong();
    }
    return out;
}

QList<qint64> oleShapeIdsIn(const QByteArray &sheetXml)
{
    QList<qint64> out;
    qsizetype at = 0;
    while ((at = sheetXml.indexOf("<oleObject ", at)) >= 0) {
        at += 11;
        const qsizetype p = sheetXml.indexOf("shapeId=\"", at);
        if (p < 0) break;
        out << sheetXml.mid(p + 9, sheetXml.indexOf('"', p + 9) - p - 9).toLongLong();
    }
    return out;
}

} // namespace

class SheetOleObjectTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // An object of ours put on a sheet is in the saved file as the standard form, and reads back as the same package, the same picture, the same place.
    void ourObjectRoundTrips()
    {
        const ole::EmbeddedObject object = ours("first", 6, Qt::red);
        QVERIFY(object.isOurs);
        SaveChanges changes;
        changes.oleObjects[0].add.push_back({object, cells(1, 1, 4, 6)});
        QByteArray out;
        QString why;
        QVERIFY2(save(buildXlsx({dataSheet()}), changes, &out, &why), qPrintable(why));
        const auto parts = unzip(out);
        QVERIFY(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<oleObjects>"));
        QVERIFY(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("progId=\"Excel.Sheet.12\""));
        QCOMPARE(int(partsUnder(parts, QStringLiteral("xl/embeddings/")).size()), 1);
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("spreadsheetml.sheet"));
        const QVector<SheetOleObject> back = readAll(out);
        QCOMPARE(int(back.size()), 1);
        QVERIFY(back[0].object.isOurs);
        QCOMPARE(back[0].object.partBytes, object.partBytes);
        QCOMPARE(back[0].object.previewPng, object.previewPng);
        QCOMPARE(back[0].object.progId, object.progId);
        QCOMPARE(back[0].object.widthEmu, object.widthEmu);
        QCOMPARE(back[0].object.heightEmu, object.heightEmu);
        QCOMPARE(int(back[0].anchor.from.col), 1);
        QCOMPARE(int(back[0].anchor.to.row), 6);
        QVERIFY(back[0].shapeId > 1024);
        // The same file saved again with nothing changed is the same sheet.
        QByteArray again;
        QVERIFY2(save(out, SaveChanges(), &again, &why), qPrintable(why));
        QCOMPARE(unzip(again).value(QStringLiteral("xl/worksheets/sheet1.xml")), parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QCOMPARE(readAll(again).size(), 1);
        // A second and a third object on the same sheet get shapes and relationships of their own.
        SaveChanges more;
        more.oleObjects[0].add.push_back({ours("second", 7, Qt::green), cells(5, 1, 8, 6)});
        more.oleObjects[0].add.push_back({ours("third", 8, Qt::blue), cells(5, 8, 8, 12)});
        QByteArray three;
        QVERIFY2(save(out, more, &three, &why), qPrintable(why));
        const QVector<SheetOleObject> all = readAll(three);
        QCOMPARE(int(all.size()), 3);
        QSet<qint64> shapes;
        for (const SheetOleObject &o : all) shapes.insert(o.shapeId);
        QCOMPARE(int(shapes.size()), 3);
        QCOMPARE(all[0].object.partBytes, object.partBytes);
        QCOMPARE(int(partsUnder(unzip(three), QStringLiteral("xl/embeddings/")).size()), 3);
    }

    // An object of another program is kept as the file has it: its element with every attribute, its part, and its picture are the same bytes after a save that changes the sheet's objects.
    void foreignObjectIsKeptByteForByte()
    {
        Spec plain;
        plain.bytes = QByteArray("not ours, not a package");
        plain.png = pngBytes(9, 9, Qt::yellow);
        plain.extra = QStringLiteral("dvAspect=\"DVASPECT_ICON\" oleUpdate=\"OLEUPDATE_ALWAYS\" autoLoad=\"1\"");
        Spec wrapped;
        wrapped.target = QStringLiteral("/xl/embeddings/oleObject2.bin");
        wrapped.bytes = QByteArray("another, with a shape of its own", 31);
        wrapped.png = pngBytes(11, 5, Qt::cyan);
        wrapped.wrap = true;
        wrapped.shapeId = 1030;
        wrapped.progId = QStringLiteral("Foo.Other.2");
        wrapped.toCol = 6;
        const QByteArray xlsx = bookWith({plain, wrapped});
        const auto was = unzip(xlsx);
        const QVector<SheetOleObject> seen = readAll(xlsx);
        QCOMPARE(int(seen.size()), 2);
        QVERIFY(!seen[0].object.isOurs);
        QCOMPARE(seen[0].object.partBytes, plain.bytes);
        QCOMPARE(seen[0].object.previewPng, plain.png);
        QCOMPARE(seen[0].object.progId, plain.progId);
        // saved with an object of ours added beside them
        SaveChanges changes;
        changes.oleObjects[0].add.push_back({ours("mine", 6, Qt::red), cells(1, 10, 4, 14)});
        QByteArray out;
        QString why;
        QVERIFY2(save(xlsx, changes, &out, &why), qPrintable(why));
        const auto now = unzip(out);
        for (auto it = was.begin(); it != was.end(); ++it)
            if (it.key().startsWith(QStringLiteral("xl/embeddings/")) || it.key().startsWith(QStringLiteral("xl/media/"))) QCOMPARE(now.value(it.key()), it.value());
        // (each element is in the saved sheet as the file wrote it)
        const QByteArray before = was.value(QStringLiteral("xl/worksheets/sheet1.xml")), after = now.value(QStringLiteral("xl/worksheets/sheet1.xml"));
        const qsizetype first = before.indexOf("<oleObjects>") + 12, last = before.indexOf("</oleObjects>");
        QVERIFY(first > 12 && last > first);
        QVERIFY(after.contains(before.mid(first, last - first)));
        QVERIFY(after.contains("oleUpdate=\"OLEUPDATE_ALWAYS\""));
        QVERIFY(after.contains("mc:AlternateContent"));
        // and when nothing is asked of the objects, the sheet is not touched at all
        QByteArray plainSave;
        QVERIFY2(save(xlsx, SaveChanges(), &plainSave, &why), qPrintable(why));
        QCOMPARE(unzip(plainSave).value(QStringLiteral("xl/worksheets/sheet1.xml")), before);
        // Taking out one of them leaves the other as it was, and the part and picture of the one taken out go.
        SaveChanges take;
        take.oleObjects[0].remove.insert(0);
        QByteArray taken;
        QVERIFY2(save(xlsx, take, &taken, &why), qPrintable(why));
        const auto left = unzip(taken);
        QCOMPARE(int(partsUnder(left, QStringLiteral("xl/embeddings/")).size()), 1);
        QVERIFY(left.contains(QStringLiteral("xl/embeddings/oleObject2.bin")));
        QCOMPARE(left.value(QStringLiteral("xl/embeddings/oleObject2.bin")), wrapped.bytes);
        QCOMPARE(int(partsUnder(left, QStringLiteral("xl/media/")).size()), 1);
        QVERIFY(!left.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("Foo.Widget.1"));
        QVERIFY(left.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("Foo.Other.2"));
        // another program's object is never given new bytes
        SaveChanges replace;
        replace.oleObjects[0].replace[0] = ours("mine", 6, Qt::red);
        QByteArray refused;
        QVERIFY(!save(xlsx, replace, &refused, &why));
    }

    // A replaced object: the new package and picture are in the file, the old ones are not, and no relationship of the sheet names a part that is not there.
    void replacingAnObjectLeavesNoOrphan()
    {
        SaveChanges first;
        first.oleObjects[0].add.push_back({ours("old package", 6, Qt::red), cells(1, 1, 4, 6)});
        QByteArray one;
        QString why;
        QVERIFY2(save(buildXlsx({dataSheet()}), first, &one, &why), qPrintable(why));
        const ole::EmbeddedObject now = ours("new package, a little longer", 9, Qt::green);
        SaveChanges replace;
        replace.oleObjects[0].replace[0] = now;
        QByteArray two;
        QVERIFY2(save(one, replace, &two, &why), qPrintable(why));
        const auto parts = unzip(two);
        const QStringList embeddings = partsUnder(parts, QStringLiteral("xl/embeddings/"));
        QCOMPARE(int(embeddings.size()), 1);
        QCOMPARE(parts.value(embeddings[0]), now.partBytes);
        QCOMPARE(int(partsUnder(parts, QStringLiteral("xl/media/")).size()), 1);
        const QByteArray rels = parts.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels"));
        QCOMPARE(int(rels.count("<Relationship ")), 3);   // (the object, its picture, and the legacy drawing that holds its shape)
        const QVector<SheetOleObject> back = readAll(two);
        QCOMPARE(int(back.size()), 1);
        QCOMPARE(back[0].object.partBytes, now.partBytes);
        QCOMPARE(back[0].object.previewPng, now.previewPng);
        QCOMPARE(int(back[0].anchor.to.col), 4);   // (its place stays)
        QCOMPARE(back[0].shapeId, readAll(one)[0].shapeId);
        // Taken out, nothing of it is left: no part, no picture, no relationship, no element.
        SaveChanges take;
        take.oleObjects[0].remove.insert(0);
        QByteArray none;
        QVERIFY2(save(two, take, &none, &why), qPrintable(why));
        const auto empty = unzip(none);
        QCOMPARE(int(partsUnder(empty, QStringLiteral("xl/embeddings/")).size()), 0);
        QCOMPARE(int(partsUnder(empty, QStringLiteral("xl/media/")).size()), 0);
        QVERIFY(!empty.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("oleObject"));
        QVERIFY(!empty.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")).contains("embeddings"));
    }

    // The caps: the count and the bytes of the objects (packages and pictures) of all the sheets together, one object's size, and a refusal that counts nothing.
    void theCapsHoldAcrossSheets()
    {
        QVector<Spec> specs;
        for (int i = 0; i < 3; ++i) {
            Spec s;
            s.sheet = i;
            s.target = QStringLiteral("/xl/embeddings/o%1.bin").arg(i + 1);
            s.bytes = QByteArray(100, char('a' + i));
            s.png = pngBytes(8 + i, 8, Qt::red);
            specs.append(s);
        }
        const QByteArray xlsx = bookWith(specs, 3);
        {
            LimitsGuard guard;
            ole::Limits l;
            l.objects = 2;
            ole::setLimits(l);
            OleReadState state;
            int read = 0;
            for (int sheet = 0; sheet < 3; ++sheet) read += int(readAll(xlsx, sheet, &state).size());
            QCOMPARE(read, 2);   // exactly two of the three
            QCOMPARE(state.pastLimit, 1);
            QCOMPARE(state.budget.count(), 2);
        }
        {
            LimitsGuard guard;
            ole::Limits l;
            l.totalBytes = 2 * 100 + specs[0].png.size() + specs[1].png.size() + 10;   // the packages and pictures of two
            ole::setLimits(l);
            OleReadState state;
            int read = 0;
            for (int sheet = 0; sheet < 3; ++sheet) read += int(readAll(xlsx, sheet, &state).size());
            QCOMPARE(read, 2);
        }
        {
            LimitsGuard guard;
            ole::Limits l;
            l.objectBytes = 50;
            ole::setLimits(l);
            QCOMPARE(int(readAll(xlsx, 0).size()), 0);
        }
        // Saving: the objects the other sheets hold count, and a new one that does not fit makes the save fail with the file as it was.
        {
            LimitsGuard guard;
            ole::Limits l;
            l.objects = 3;
            ole::setLimits(l);
            SaveChanges changes;
            changes.oleObjects[0].add.push_back({ours("new", 6, Qt::red), cells(1, 1, 4, 6)});
            QByteArray out;
            QString why;
            QVERIFY(!save(xlsx, changes, &out, &why));
            QVERIFY(!why.isEmpty());
        }
        {
            LimitsGuard guard;
            ole::Limits l;
            l.objects = 4;
            ole::setLimits(l);
            SaveChanges changes;
            changes.oleObjects[0].add.push_back({ours("new", 6, Qt::red), cells(1, 1, 4, 6)});
            QByteArray out;
            QString why;
            QVERIFY2(save(xlsx, changes, &out, &why), qPrintable(why));
        }
        {
            // the bytes: the three file objects fill what a cap just over them leaves
            LimitsGuard guard;
            ole::Limits l;
            l.totalBytes = 300 + specs[0].png.size() + specs[1].png.size() + specs[2].png.size() + 20;
            ole::setLimits(l);
            SaveChanges changes;
            changes.oleObjects[0].add.push_back({ours(QByteArray(50, 'n'), 6, Qt::red), cells(1, 1, 4, 6)});
            QByteArray out;
            QString why;
            QVERIFY(!save(xlsx, changes, &out, &why));
        }
        // One object's package over the cap of one is refused.
        {
            LimitsGuard guard;
            const ole::EmbeddedObject big = ours(QByteArray(500, 'z'), 6, Qt::red);
            ole::Limits l;
            l.objectBytes = 200;
            ole::setLimits(l);
            SaveChanges changes;
            changes.oleObjects[0].add.push_back({big, cells(1, 1, 4, 6)});
            QByteArray out;
            QString why;
            QVERIFY(!save(buildXlsx({dataSheet()}), changes, &out, &why));
        }
    }

    // Copies of a sheet's objects are held to what is left of the budget, and a later one that fits is kept after one that does not.
    void copiesOfObjectsAreHeldToTheBudget()
    {
        std::vector<SheetOleObject> objects;
        for (const QByteArray &body : {QByteArray(300, 'b'), QByteArray(20, 's'), QByteArray(20, 't')}) {
            SheetOleObject o;
            o.object = ours(body, 6, Qt::red);
            o.fileIndex = 3;
            objects.push_back(o);
        }
        LimitsGuard guard;
        ole::Limits l;
        l.totalBytes = 2 * (objects[1].object.partBytes.size() + objects[1].object.previewPng.size()) + 10;
        ole::setLimits(l);
        ole::ObjectBudget budget;
        const std::vector<SheetOleObject> kept = oleObjectsThatFit(objects, &budget);
        QCOMPARE(int(kept.size()), 2);
        QCOMPARE(kept[0].object.partBytes, objects[1].object.partBytes);
        QCOMPARE(kept[1].object.partBytes, objects[2].object.partBytes);
        QCOMPARE(kept[0].fileIndex, -1);   // (a copy is a new object)
        QCOMPARE(budget.count(), 2);
    }

    // Hostile relationships: an object is named only by a relationship of the kind an object has, to a part under the embeddings; anything else is left as the file has it and read as nothing.
    void hostileRelationshipsAreNotObjects()
    {
        auto count = [](const Spec &s, QString *saved = nullptr) {
            const QByteArray xlsx = bookWith({s});
            if (saved) {
                SaveChanges changes;
                changes.oleObjects[0].add.push_back({ours("mine", 6, Qt::red), cells(1, 1, 4, 6)});
                QByteArray out;
                QString why;
                *saved = save(xlsx, changes, &out, &why) && unzip(out).value(QStringLiteral("xl/worksheets/sheet1.xml")).contains(s.progId.toUtf8()) ? QStringLiteral("kept") : QStringLiteral("lost");
            }
            return int(readAll(xlsx).size());
        };
        Spec good;
        good.bytes = QByteArray("some bytes");
        good.png = pngBytes(8, 8, Qt::red);
        QCOMPARE(count(good), 1);
        Spec hyperlink = good;
        hyperlink.relType = QStringLiteral("hyperlink");
        QCOMPARE(count(hyperlink), 0);
        Spec worksheet = good;
        worksheet.relType = QStringLiteral("worksheet");
        QCOMPARE(count(worksheet), 0);
        Spec image = good;
        image.relType = QStringLiteral("image");
        QCOMPARE(count(image), 0);
        Spec workbook = good;
        workbook.target = QStringLiteral("/xl/workbook.xml");
        workbook.write = false;
        QCOMPARE(count(workbook), 0);
        Spec core = good;
        core.target = QStringLiteral("/docProps/core.xml");
        core.write = false;
        QCOMPARE(count(core), 0);
        Spec dots = good;
        dots.target = QStringLiteral("../../outside.bin");
        dots.write = false;
        QCOMPARE(count(dots), 0);
        Spec elsewhere = good;
        elsewhere.target = QStringLiteral("/xl/media/other.bin");
        QCOMPARE(count(elsewhere), 0);
        Spec external = good;
        external.target = QStringLiteral("https://example.com/a.bin\" TargetMode=\"External");
        external.write = false;
        QCOMPARE(count(external), 0);
        Spec missing = good;
        missing.write = false;
        QCOMPARE(count(missing), 0);
        // the picture must be an image relationship to a PNG
        Spec wrongPicture = good;
        wrongPicture.pictureType = QStringLiteral("hyperlink");
        const QVector<SheetOleObject> noPicture = readAll(bookWith({wrongPicture}));
        QCOMPARE(int(noPicture.size()), 1);
        QVERIFY(noPicture[0].object.previewPng.isEmpty());
        Spec notPng = good;
        notPng.png = QByteArray("GIF89a not a png");
        QVERIFY(readAll(bookWith({notPng}))[0].object.previewPng.isEmpty());
        // What the file names in vain stays in the sheet as it was when the objects are changed.
        QString saved;
        count(hyperlink, &saved);
        QCOMPARE(saved, QStringLiteral("kept"));
        count(workbook, &saved);
        QCOMPARE(saved, QStringLiteral("kept"));
    }

    // A part is asked its size before it is inflated, and one that many objects name is inflated once.
    void partsAreSizedBeforeInflatingAndInflatedOnce()
    {
        // 5 MB of zeros under a cap of 1,000: refused, and not inflated.
        Spec big;
        big.bytes = QByteArray(5 << 20, '\0');
        big.png = pngBytes(8, 8, Qt::red);
        const QByteArray bigFile = bookWith({big});
        {
            LimitsGuard guard;
            ole::Limits l;
            l.objectBytes = 1000;
            ole::setLimits(l);
            Opened o;
            QString why;
            QVERIFY2(open(&o, bigFile, &why), qPrintable(why));
            const qint64 before = o.package.inflatedBytes();
            OleReadState state;
            std::vector<SheetOleObject> out;
            QVERIFY(readSheetOleObjects(o.package, o.book.info().sheets[0].part, &state, &out));
            QCOMPARE(int(out.size()), 0);
            QVERIFY(o.package.inflatedBytes() - before < (1 << 20));
        }
        // Three objects naming one 200,000-byte part: inflated once.
        QVector<Spec> three;
        for (int i = 0; i < 3; ++i) {
            Spec s;
            s.bytes = QByteArray(200000, 'q');
            s.png = pngBytes(8 + i, 8, Qt::red);
            s.shapeId = 1025 + i;
            three.append(s);
        }
        Opened o;
        QString why;
        QVERIFY2(open(&o, bookWith(three), &why), qPrintable(why));
        const qint64 before = o.package.inflatedBytes();
        OleReadState state;
        std::vector<SheetOleObject> out;
        QVERIFY(readSheetOleObjects(o.package, o.book.info().sheets[0].part, &state, &out));
        QCOMPARE(int(out.size()), 3);
        QVERIFY(o.package.inflatedBytes() - before < 350000);   // (twice the part would be 400,000)
    }

    // A new object's shape number is above every number the sheet's legacy drawings (the notes', the header pictures') and its form controls have, so that it names no shape of theirs.
    void newObjectShapeNumbersAvoidNotesControlsAndPictures()
    {
        // a note of shape 1025, a control (wrapped, as another program writes it) of 1030, and header pictures of 1040 in a drawing of their own
        const QString controls = QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"x14\"><controls><mc:AlternateContent>"
                                                "<mc:Choice Requires=\"x14\"><control shapeId=\"1030\" r:id=\"rIdK\" name=\"Check Box 1\"/></mc:Choice></mc:AlternateContent></controls></mc:Choice></mc:AlternateContent>");
        QByteArray xlsx = bookWithNote(1025, QStringLiteral("<legacyDrawingHF r:id=\"rIdH\"/>") + controls);
        const QByteArray hf = "<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\"><v:shape id=\"LH\" o:spid=\"_x0000_s1040\" type=\"#_x0000_t75\"/></xml>";
        xlsx = withParts(xlsx, {{QStringLiteral("xl/drawings/vmlDrawing2.vml"), hf, QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"), 0, QStringLiteral("rIdH"), QStringLiteral("vmlDrawing"),
                                 QStringLiteral("/xl/drawings/vmlDrawing2.vml")}});
        SaveChanges changes;
        changes.oleObjects[0].add.push_back({ours("first", 6, Qt::red), cells(1, 1, 4, 6)});
        changes.oleObjects[0].add.push_back({ours("second", 7, Qt::green), cells(1, 8, 4, 12)});
        QByteArray out;
        QString why;
        QVERIFY2(save(xlsx, changes, &out, &why), qPrintable(why));
        const auto parts = unzip(out);
        const QList<qint64> ids = oleShapeIdsIn(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QCOMPARE(int(ids.size()), 4);   // (each in the standard form is named twice: with its picture and without)
        for (qint64 id : ids) QVERIFY2(id > 1040, qPrintable(QString::number(id)));
        QVERIFY(ids[0] != ids[2]);
        // the note's drawing now has the note and one shape for each object, and no number is used twice
        const QByteArray vml = parts.value(QStringLiteral("xl/drawings/vmlDrawing1.vml"));
        QList<qint64> numbers = shapeNumbersIn(vml, "id");
        QCOMPARE(int(numbers.size()), 3);
        QCOMPARE(QSet<qint64>(numbers.begin(), numbers.end()).size(), 3);
        QVERIFY(numbers.contains(1025));
        QVERIFY(numbers.contains(ids[0]) && numbers.contains(ids[2]));
    }

    // A new object is written as another spreadsheet program expects: an element for a program that reads the extended form, a plain one for the others, and a shape in the legacy drawing (made, with
    // its relationship and type, for a sheet that has none) that has the same number and names the picture.
    void newObjectIsWrittenWithItsAlternateFormAndShape()
    {
        SaveChanges changes;
        const ole::EmbeddedObject object = ours("first", 6, Qt::red);
        changes.oleObjects[0].add.push_back({object, cells(1, 1, 4, 6)});
        QByteArray out;
        QString why;
        QVERIFY2(save(buildXlsx({dataSheet()}), changes, &out, &why), qPrintable(why));
        const auto parts = unzip(out);
        const QByteArray sheet = parts.value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(sheet.contains("<mc:AlternateContent"));
        QVERIFY(sheet.contains("<mc:Choice Requires=\"x14\">"));
        QVERIFY(sheet.contains("<mc:Fallback>"));
        QVERIFY(sheet.indexOf("<mc:Fallback>") > sheet.indexOf("<objectPr"));
        QVERIFY(sheet.contains("xmlns:x14="));
        // the plain element of the fallback has no picture or place
        const QByteArray fallback = sheet.mid(sheet.indexOf("<mc:Fallback>"));
        QVERIFY(!fallback.left(fallback.indexOf("</mc:Fallback>")).contains("objectPr"));
        const QList<qint64> ids = oleShapeIdsIn(sheet);
        QCOMPARE(int(ids.size()), 2);
        QCOMPARE(ids[0], ids[1]);
        // the legacy drawing is named by the sheet before its objects, and holds the shape
        QVERIFY(sheet.contains("<legacyDrawing "));
        QVERIFY(sheet.indexOf("<legacyDrawing ") < sheet.indexOf("<oleObjects"));
        const QStringList drawings = partsUnder(parts, QStringLiteral("xl/drawings/"));
        QStringList vmls;
        for (const QString &d : drawings)
            if (d.endsWith(QLatin1String(".vml"))) vmls << d;
        QCOMPARE(int(vmls.size()), 1);
        const QByteArray vml = parts.value(vmls[0]);
        QVERIFY(vml.contains("id=\"_x0000_s" + QByteArray::number(ids[0]) + "\""));
        QVERIFY(vml.contains("ObjectType=\"Pict\""));
        QVERIFY(vml.contains("_x0000_t75"));
        QVERIFY(vml.contains("<x:Anchor>1, 0, 1, 0, 4, 0, 6, 0</x:Anchor>"));
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("vmlDrawing1.vml"));
        QVERIFY(parts.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")).contains("vmlDrawing1.vml"));
        // its picture is named by a relationship of the drawing, to the picture the object has
        const qsizetype relid = vml.indexOf("o:relid=\"") + 9;
        const QByteArray relId = vml.mid(relid, vml.indexOf('"', relid) - relid);
        const QByteArray vmlRels = parts.value(QStringLiteral("xl/drawings/_rels/vmlDrawing1.vml.rels"));
        QVERIFY(vmlRels.contains("Id=\"" + relId + "\""));
        QVERIFY(vmlRels.contains("../media/oleimage1.png"));
        QCOMPARE(parts.value(QStringLiteral("xl/media/oleimage1.png")), object.previewPng);
        // it reads back, and read again after a save with nothing changed
        QCOMPARE(int(readAll(out).size()), 1);
        QByteArray again;
        QVERIFY2(save(out, SaveChanges(), &again, &why), qPrintable(why));
        QCOMPARE(unzip(again).value(vmls[0]), vml);
        // taken out, no shape is left, and no relationship or picture
        SaveChanges take;
        take.oleObjects[0].remove.insert(0);
        QByteArray none;
        QVERIFY2(save(out, take, &none, &why), qPrintable(why));
        const auto empty = unzip(none);
        QVERIFY(!empty.value(vmls[0]).contains("<v:shape "));
        QVERIFY(!empty.value(QStringLiteral("xl/drawings/_rels/vmlDrawing1.vml.rels")).contains("oleimage"));
        QCOMPARE(int(partsUnder(empty, QStringLiteral("xl/media/")).size()), 0);
        QCOMPARE(int(partsUnder(empty, QStringLiteral("xl/embeddings/")).size()), 0);
    }

    // On a sheet with a note the object's shape goes into the note's drawing (the note stays); taken out, only the object's shape goes.
    void newObjectJoinsTheNotesDrawing()
    {
        const QByteArray xlsx = bookWithNote(1025);
        const QByteArray noteBefore = noteShape(1025, 0);
        SaveChanges changes;
        changes.oleObjects[0].add.push_back({ours("first", 6, Qt::red), cells(1, 1, 4, 6)});
        QByteArray out;
        QString why;
        QVERIFY2(save(xlsx, changes, &out, &why), qPrintable(why));
        const auto parts = unzip(out);
        int vmlParts = 0;
        for (auto it = parts.begin(); it != parts.end(); ++it) vmlParts += it.key().endsWith(QLatin1String(".vml")) ? 1 : 0;
        QCOMPARE(vmlParts, 1);
        const QByteArray vml = parts.value(QStringLiteral("xl/drawings/vmlDrawing1.vml"));
        QVERIFY(vml.contains(noteBefore));
        const QList<qint64> ids = oleShapeIdsIn(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QCOMPARE(int(ids.size()), 2);
        QVERIFY(ids[0] > 1025);
        QVERIFY(vml.contains("id=\"_x0000_s" + QByteArray::number(ids[0]) + "\""));
        QCOMPARE(int(vml.count("<v:shape ")), 2);
        QCOMPARE(int(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).count("<legacyDrawing ")), 1);
        QVERIFY(parts.contains(QStringLiteral("xl/comments1.xml")));
        // taken out
        SaveChanges take;
        take.oleObjects[0].remove.insert(0);
        QByteArray none;
        QVERIFY2(save(out, take, &none, &why), qPrintable(why));
        const auto empty = unzip(none);
        const QByteArray left = empty.value(QStringLiteral("xl/drawings/vmlDrawing1.vml"));
        QVERIFY(left.contains(noteBefore));
        QCOMPARE(int(left.count("<v:shape ")), 1);
        QVERIFY(!left.contains("Pict"));
        QCOMPARE(int(partsUnder(empty, QStringLiteral("xl/media/")).size()), 0);
        QVERIFY(empty.contains(QStringLiteral("xl/comments1.xml")));
        // a note put in the same save as the object: both are in the drawing the notes have, with numbers of their own
        SaveChanges both;
        both.oleObjects[0].add.push_back({ours("again", 6, Qt::blue), cells(1, 1, 4, 6)});
        NoteOp note;
        note.row = 5;
        note.col = 2;
        note.text = QStringLiteral("second");
        note.author = QStringLiteral("A");
        both.sheets[0].noteOps.push_back(note);
        QByteArray three;
        QVERIFY2(save(xlsx, both, &three, &why), qPrintable(why));
        const QList<qint64> numbers = shapeNumbersIn(unzip(three).value(QStringLiteral("xl/drawings/vmlDrawing1.vml")), "id");
        QCOMPARE(int(numbers.size()), 3);
        QCOMPARE(QSet<qint64>(numbers.begin(), numbers.end()).size(), 3);
    }

    // An object the file made: its shape is taken out with it, its picture's relationship from the drawing too (and the picture, when nothing else names it), and a replaced object's shape names
    // the new picture. The shape of another object, and a picture a shape that stays names, are kept.
    void anObjectsShapeFromTheFileIsTakenOrChangedWithIt()
    {
        const ole::EmbeddedObject mine = ours("package", 6, Qt::red);
        Spec a, b;
        a.progId = ole::progIdOf(ole::Kind::Sheet);
        a.bytes = mine.partBytes;
        a.png = mine.previewPng;
        a.shapeId = 1026;
        b = a;
        b.target = QStringLiteral("/xl/embeddings/oleObject2.bin");
        b.shapeId = 1027;
        b.png = pngBytes(9, 9, Qt::blue);
        const QByteArray shapeA = "<v:shape id=\"_x0000_s1026\" type=\"#_x0000_t75\" o:ole=\"\"><v:imagedata o:relid=\"rIdX1\" o:title=\"\"/><x:ClientData ObjectType=\"Pict\"><x:Anchor>1, 0, 1, 0, 4, 0, 6, 0</x:Anchor></x:ClientData></v:shape>";
        const QByteArray shapeB = "<v:shape id=\"_x0000_s1027\" type=\"#_x0000_t75\" o:ole=\"\"><v:imagedata o:relid=\"rIdX2\" o:title=\"\"/><x:ClientData ObjectType=\"Pict\"><x:Anchor>1, 0, 1, 0, 4, 0, 6, 0</x:Anchor></x:ClientData></v:shape>";
        QByteArray xlsx = bookWith({a, b});
        // (the sheet names the drawing, and the drawing names the two pictures the objects have)
        QMap<QString, QByteArray> parts = unzip(xlsx);
        parts[QStringLiteral("xl/worksheets/sheet1.xml")].replace("<oleObjects>", "<legacyDrawing r:id=\"rIdV\"/><oleObjects>");
        xlsx = withParts([&] {
            jp::ZipWriter z;
            for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
            return z.finish();
        }(), {{QStringLiteral("xl/drawings/vmlDrawing1.vml"), vmlWith(shapeA + shapeB), QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"), 0, QStringLiteral("rIdV"), QStringLiteral("vmlDrawing"),
               QStringLiteral("/xl/drawings/vmlDrawing1.vml")},
              {QStringLiteral("xl/drawings/_rels/vmlDrawing1.vml.rels"),
               "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
               "<Relationship Id=\"rIdX1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"../media/p1.png\"/>"
               "<Relationship Id=\"rIdX2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"../media/p2.png\"/></Relationships>",
               QString(), -1, QString(), QString(), QString()}});
        QCOMPARE(int(readAll(xlsx).size()), 2);
        SaveChanges take;
        take.oleObjects[0].remove.insert(0);
        QByteArray out;
        QString why;
        QVERIFY2(save(xlsx, take, &out, &why), qPrintable(why));
        auto now = unzip(out);
        QVERIFY(!now.value(QStringLiteral("xl/drawings/vmlDrawing1.vml")).contains("_x0000_s1026"));
        QVERIFY(now.value(QStringLiteral("xl/drawings/vmlDrawing1.vml")).contains(shapeB));
        QVERIFY(!now.value(QStringLiteral("xl/drawings/_rels/vmlDrawing1.vml.rels")).contains("rIdX1"));
        QVERIFY(now.value(QStringLiteral("xl/drawings/_rels/vmlDrawing1.vml.rels")).contains("rIdX2"));
        QVERIFY(!now.contains(QStringLiteral("xl/media/p1.png")));   // (named by the sheet and the drawing, both let go of)
        QVERIFY(now.contains(QStringLiteral("xl/media/p2.png")));
        // replaced: the shape names the new picture, and the old one is gone
        SaveChanges replace;
        replace.oleObjects[0].replace[1] = ours("newer package", 8, Qt::green);
        QByteArray two;
        QVERIFY2(save(xlsx, replace, &two, &why), qPrintable(why));
        now = unzip(two);
        const QByteArray vml = now.value(QStringLiteral("xl/drawings/vmlDrawing1.vml"));
        const qsizetype at = vml.indexOf("_x0000_s1027");
        QVERIFY(at >= 0);
        QVERIFY(!vml.contains("rIdX2"));
        const qsizetype relid = vml.indexOf("o:relid=\"", at) + 9;
        const QByteArray fresh = vml.mid(relid, vml.indexOf('"', relid) - relid);
        QVERIFY(now.value(QStringLiteral("xl/drawings/_rels/vmlDrawing1.vml.rels")).contains("Id=\"" + fresh + "\""));
        QVERIFY(!now.value(QStringLiteral("xl/drawings/_rels/vmlDrawing1.vml.rels")).contains("rIdX2"));
        QVERIFY(!now.contains(QStringLiteral("xl/media/p2.png")));
        QVERIFY(now.contains(QStringLiteral("xl/media/p1.png")));
        QVERIFY(vml.contains("_x0000_s1026"));
        QCOMPARE(int(partsUnder(now, QStringLiteral("xl/media/")).size()), 2);
    }

    // Objects the file already has never make a save fail: at a cap lower than what the file holds, a save that changes no object, and one that only takes objects out, still works, and a new
    // object is refused with a message that says why. A part two sheets name counts once.
    void objectsTheFileHasDoNotFailASaveAndASharedPartCountsOnce()
    {
        QVector<Spec> specs;
        for (int i = 0; i < 3; ++i) {
            Spec s;
            s.sheet = i;
            s.target = QStringLiteral("/xl/embeddings/o%1.bin").arg(i + 1);
            s.bytes = QByteArray(100, char('a' + i));
            s.png = pngBytes(8 + i, 8, Qt::red);
            specs.append(s);
        }
        const QByteArray xlsx = bookWith(specs, 3);
        {
            LimitsGuard guard;
            ole::Limits l;
            l.objects = 2;   // (the reader keeps two of the three)
            ole::setLimits(l);
            QString why;
            QByteArray out;
            SaveChanges none;
            (void)none.oleObjects[0];   // (an entry with nothing in it)
            QVERIFY2(save(xlsx, none, &out, &why), qPrintable(why));
            SaveChanges take;
            take.oleObjects[1].remove.insert(0);
            QVERIFY2(save(xlsx, take, &out, &why), qPrintable(why));
            QCOMPARE(int(readAll(out, 1).size()), 0);
            SaveChanges add;
            add.oleObjects[0].add.push_back({ours("new", 6, Qt::red), cells(1, 1, 4, 6)});
            QVERIFY(!save(xlsx, add, &out, &why));
            QVERIFY2(why.contains(QLatin1String("more than a file can hold")), qPrintable(why));
        }
        // two sheets that name one part: counted once, so a third object fits a cap that two parts and their pictures leave room for
        Spec first, second;
        first.sheet = 0;
        first.bytes = QByteArray(100, 'q');
        first.png = pngBytes(8, 8, Qt::red);
        second = first;
        second.sheet = 1;
        second.png = pngBytes(9, 8, Qt::blue);
        const QByteArray shared = bookWith({first, second}, 2);
        const ole::EmbeddedObject added = ours("new", 6, Qt::green);
        {
            LimitsGuard guard;
            ole::Limits l;
            l.totalBytes = 100 + first.png.size() + second.png.size() + added.partBytes.size() + added.previewPng.size() + 5;
            ole::setLimits(l);
            SaveChanges add;
            add.oleObjects[0].add.push_back({added, cells(1, 1, 4, 6)});
            QByteArray out;
            QString why;
            QVERIFY2(save(shared, add, &out, &why), qPrintable(why));
        }
    }

    // A sheet whose tail is past the limit is neither read nor failed for an edit that does not touch it, and an edit that does is refused with a message that says so; the tail of a sheet that is
    // changed is read once for the save.
    void bigTailsAreOnlyReadForTheSheetsTheEditTouches()
    {
        QVector<Spec> specs;
        for (int i = 0; i < 2; ++i) {
            Spec s;
            s.sheet = i;
            s.target = QStringLiteral("/xl/embeddings/o%1.bin").arg(i + 1);
            s.bytes = QByteArray(100, char('a' + i));
            s.png = pngBytes(8 + i, 8, Qt::red);
            specs.append(s);
        }
        QMap<QString, QByteArray> parts = unzip(bookWith(specs, 2));
        parts[QStringLiteral("xl/worksheets/sheet1.xml")].replace("</worksheet>", "<!--" + QByteArray(20000, 'x') + "--></worksheet>");
        QByteArray xlsx;
        {
            jp::ZipWriter z;
            for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
            xlsx = z.finish();
        }
        struct TailGuard {
            ~TailGuard() { setSheetTailLimit(0); }
        } guard;
        setSheetTailLimit(10000);
        QString why;
        QByteArray out;
        // the other sheet is changed: the big one is not a reason to fail
        SaveChanges other;
        other.oleObjects[1].add.push_back({ours("new", 6, Qt::red), cells(1, 1, 4, 6)});
        const qint64 reads = sheetTailReads();
        QVERIFY2(save(xlsx, other, &out, &why), qPrintable(why));
        QVERIFY2(sheetTailReads() - reads <= 3, qPrintable(QString::number(sheetTailReads() - reads)));
        QCOMPARE(int(readAll(out, 1).size()), 2);
        // the big one is changed: refused, saying what is wrong
        SaveChanges own;
        own.oleObjects[0].add.push_back({ours("new", 6, Qt::red), cells(1, 1, 4, 6)});
        QVERIFY(!save(xlsx, own, &out, &why));
        QVERIFY2(why.contains(QLatin1String("after its cells")), qPrintable(why));
        QVERIFY(!why.contains(QLatin1String("can't put them on")));
        // edits that change nothing read nothing
        SaveChanges nothing;
        (void)nothing.oleObjects[0];
        const qint64 before = sheetTailReads();
        QVERIFY2(save(xlsx, nothing, &out, &why), qPrintable(why));
        QCOMPARE(sheetTailReads(), before);
        // a sheet that is changed has its tail read once, the other sheet's once for the count
        setSheetTailLimit(0);
        const qint64 start = sheetTailReads();
        QVERIFY2(save(xlsx, other, &out, &why), qPrintable(why));
        QCOMPARE(sheetTailReads() - start, qint64(2));
    }


    // Moving or sizing an object changes the place its element says and nothing else: its part, its picture (whatever kind), the relationships, and the types of the file are as they were, for the
    // element of a plain `oleObject` and for one that is wrapped, and the shape of the legacy drawing has the place too.
    void anObjectsPlaceChangesAndNothingElseDoes()
    {
        Spec plain;
        plain.bytes = QByteArray("not ours, not a package");
        plain.png = pngBytes(9, 9, Qt::yellow);
        Spec wrapped;
        wrapped.target = QStringLiteral("/xl/embeddings/oleObject2.bin");
        wrapped.bytes = QByteArray("another");
        wrapped.png = pngBytes(11, 5, Qt::cyan);
        wrapped.wrap = true;
        wrapped.shapeId = 1030;
        wrapped.toCol = 6;
        const QByteArray xlsx = bookWith({plain, wrapped});
        const auto was = unzip(xlsx);
        SaveChanges changes;
        changes.oleObjects[0].reanchor[0] = cells(2, 3, 5, 9);
        changes.oleObjects[0].reanchor[1] = cells(7, 8, 12, 20);
        QByteArray out;
        QString why;
        QVERIFY2(save(xlsx, changes, &out, &why), qPrintable(why));
        const auto now = unzip(out);
        for (auto it = was.begin(); it != was.end(); ++it)
            if (it.key() != QLatin1String("xl/worksheets/sheet1.xml")) QCOMPARE(now.value(it.key()), it.value());
        QCOMPARE(now.size(), was.size());
        const QVector<SheetOleObject> seen = readAll(out);
        QCOMPARE(int(seen.size()), 2);
        QCOMPARE(seen[0].anchor.from.col, quint32(2));
        QCOMPARE(seen[0].anchor.to.row, quint32(9));
        QCOMPARE(seen[1].anchor.from.row, quint32(8));
        QCOMPARE(seen[1].anchor.to.col, quint32(12));
        QCOMPARE(seen[0].object.partBytes, plain.bytes);
        QCOMPARE(seen[1].object.previewPng, wrapped.png);
        // (only the numbers of the places differ in the sheet)
        QByteArray was1 = was.value(QStringLiteral("xl/worksheets/sheet1.xml")), now1 = now.value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QCOMPARE(now1.size() - now1.count("<xdr:col>"), was1.size() - was1.count("<xdr:col>") + (now1.size() - was1.size()));
        QVERIFY(now1.contains("progId=\"Foo.Widget.1\""));
        // a place for a child that is not there is refused
        SaveChanges bad;
        bad.oleObjects[0].reanchor[5] = cells(1, 1, 2, 2);
        QVERIFY(!save(xlsx, bad, &out, &why));
    }

    // An object of another program that is put back (an Undo of a delete that was saved) is written as the parts it was read from, and a picture that is not a PNG is as the file had it; an
    // object with no picture at all is written with none; none of it fails the save.
    void aForeignObjectPutBackKeepsItsPartAndItsOwnKindOfPicture()
    {
        Spec s;
        s.bytes = QByteArray("not ours, not a package");
        s.png = QByteArray("METAFILE-BYTES");
        s.pictureType = QStringLiteral("image");
        QMap<QString, QByteArray> parts = unzip(bookWith({s}));
        // the picture is a metafile: its part, relationship, and type say so
        const QByteArray metafile = parts.take(QStringLiteral("xl/media/p1.png"));
        parts.insert(QStringLiteral("xl/media/p1.emf"), metafile);
        parts[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")].replace("/xl/media/p1.png", "/xl/media/p1.emf");
        parts[QStringLiteral("[Content_Types].xml")].replace("/xl/media/p1.png\" ContentType=\"image/png\"", "/xl/media/p1.emf\" ContentType=\"image/x-emf\"");
        QByteArray xlsx;
        {
            jp::ZipWriter z;
            for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
            xlsx = z.finish();
        }
        const QVector<SheetOleObject> seen = readAll(xlsx);
        QCOMPARE(int(seen.size()), 1);
        QVERIFY(seen[0].object.previewPng.isEmpty());
        QCOMPARE(seen[0].object.otherPicture, metafile);
        QCOMPARE(seen[0].object.otherPictureExt, QStringLiteral("emf"));
        QCOMPARE(seen[0].object.otherPictureType, QStringLiteral("image/x-emf"));
        // taken away and put back in a save of its own, beside another that is moved
        SaveChanges changes;
        changes.oleObjects[0].remove.insert(0);
        changes.oleObjects[0].add.push_back({seen[0].object, cells(3, 3, 6, 8)});
        ole::EmbeddedObject bare = seen[0].object;
        bare.otherPicture.clear();
        bare.partBytes = QByteArray("a second one");
        changes.oleObjects[0].add.push_back({bare, cells(8, 3, 11, 8)});
        QByteArray out;
        QString why;
        QVERIFY2(save(xlsx, changes, &out, &why), qPrintable(why));
        const auto now = unzip(out);
        const QStringList pictures = partsUnder(now, QStringLiteral("xl/media/"));
        QCOMPARE(int(pictures.size()), 1);
        QVERIFY(pictures.first().endsWith(QStringLiteral(".emf")));
        QCOMPARE(now.value(pictures.first()), metafile);
        QVERIFY(now.value(QStringLiteral("[Content_Types].xml")).contains("image/x-emf"));
        QCOMPARE(int(partsUnder(now, QStringLiteral("xl/embeddings/")).size()), 2);
        const QVector<SheetOleObject> again = readAll(out);
        QCOMPARE(int(again.size()), 2);
        QCOMPARE(again[0].object.partBytes, s.bytes);
        QCOMPARE(again[0].object.otherPicture, metafile);
        QVERIFY(again[1].object.otherPicture.isEmpty() && again[1].object.previewPng.isEmpty());
        QCOMPARE(again[1].anchor.from.col, quint32(8));
    }

    // A sheet with as many embedded objects as a save looks at is refused in plain words, not with the words for a sheet that can't hold objects at all.
    void aSheetWithTooManyObjectsIsRefusedPlainly()
    {
        QString children;
        for (int i = 0; i < kMaxOleChildren; ++i) children += QStringLiteral("<oleObject progId=\"Foo.Widget.1\" shapeId=\"%1\" r:id=\"rIdNone%1\"/>").arg(i + 1);
        const QByteArray xlsx = buildXlsx({worksheet(QString(), QStringLiteral("<oleObjects>") + children + QStringLiteral("</oleObjects>"))});
        SaveChanges changes;
        changes.oleObjects[0].add.push_back({ours("new", 6, Qt::red), cells(1, 1, 4, 6)});
        QByteArray out;
        QString why;
        QVERIFY(!save(xlsx, changes, &out, &why));
        QVERIFY2(why.contains(QStringLiteral("too many")), qPrintable(why));
        QVERIFY2(why.contains(QString::number(kMaxOleChildren)), qPrintable(why));
        QVERIFY(!why.contains(QStringLiteral("can't put them on")));
    }

    // Controls that are wrapped, as another program writes them, are where the objects go before.
    void objectsGoBeforeWrappedControls()
    {
        const QString controls = QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"x14\"><controls><mc:AlternateContent>"
                                                "<mc:Choice Requires=\"x14\"><control shapeId=\"1030\" r:id=\"rIdK\" name=\"Check Box 1\"/></mc:Choice></mc:AlternateContent></controls></mc:Choice></mc:AlternateContent>");
        const QByteArray xlsx = withParts(buildXlsx({worksheet(QString(), controls)}), {});
        SaveChanges changes;
        changes.oleObjects[0].add.push_back({ours("first", 6, Qt::red), cells(1, 1, 4, 6)});
        QByteArray out;
        QString why;
        QVERIFY2(save(xlsx, changes, &out, &why), qPrintable(why));
        const QByteArray sheet = unzip(out).value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(sheet.indexOf("<oleObjects>") > 0);
        QVERIFY2(sheet.indexOf("<oleObjects>") < sheet.indexOf("<controls>"), sheet.constData());
    }

    // An object that has no `objectPr` to name a picture is replaced without one: no picture part is written that nothing names.
    void replacingAnObjectWithNoPictureElementLeavesNoOrphan()
    {
        const ole::EmbeddedObject first = ours("package", 6, Qt::red);
        const QByteArray sheet = worksheet(QString(), QStringLiteral("<oleObjects><oleObject progId=\"%1\" shapeId=\"1025\" r:id=\"rIdO1\"/></oleObjects>").arg(first.progId));
        const QByteArray xlsx = withParts(buildXlsx({sheet}), {{QStringLiteral("xl/embeddings/oleObject1.bin"), first.partBytes, QString(), 0, QStringLiteral("rIdO1"), QStringLiteral("oleObject"),
                                                              QStringLiteral("/xl/embeddings/oleObject1.bin")}});
        SaveChanges replace;
        replace.oleObjects[0].replace[0] = ours("newer package", 9, Qt::green);
        QByteArray out;
        QString why;
        QVERIFY2(save(xlsx, replace, &out, &why), qPrintable(why));
        const auto parts = unzip(out);
        QCOMPARE(int(partsUnder(parts, QStringLiteral("xl/media/")).size()), 0);
        const QStringList embeddings = partsUnder(parts, QStringLiteral("xl/embeddings/"));
        QCOMPARE(int(embeddings.size()), 1);
        QCOMPARE(parts.value(embeddings[0]), ours("newer package", 9, Qt::green).partBytes);
        QCOMPARE(int(parts.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")).count("<Relationship ")), 1);
    }

    // One part that two sheets name stays while either does, and goes with the last; a sheet that is deleted takes its objects' parts with it.
    void aSharedTargetStaysWhileEitherSheetNamesItAndADeletedSheetTakesItsObjects()
    {
        Spec first, second;
        first.sheet = 0;
        first.bytes = QByteArray(50, 'q');
        first.png = pngBytes(8, 8, Qt::red);
        second = first;
        second.sheet = 1;
        second.png = pngBytes(9, 8, Qt::blue);
        const QByteArray xlsx = bookWith({first, second}, 2);
        QString why;
        SaveChanges take0;
        take0.oleObjects[0].remove.insert(0);
        QByteArray one;
        QVERIFY2(save(xlsx, take0, &one, &why), qPrintable(why));
        auto parts = unzip(one);
        QVERIFY(parts.contains(QStringLiteral("xl/embeddings/oleObject1.bin")));   // (the other sheet still names it)
        QCOMPARE(int(readAll(one, 1).size()), 1);
        QCOMPARE(int(readAll(one, 0).size()), 0);
        SaveChanges take1;
        take1.oleObjects[1].remove.insert(0);
        QByteArray two;
        QVERIFY2(save(one, take1, &two, &why), qPrintable(why));
        parts = unzip(two);
        QCOMPARE(int(partsUnder(parts, QStringLiteral("xl/embeddings/")).size()), 0);
        QCOMPARE(int(partsUnder(parts, QStringLiteral("xl/media/")).size()), 0);
        // the second sheet deleted: its object's part is named by the first sheet too and stays; its picture goes
        Opened o;
        QVERIFY2(open(&o, xlsx, &why), qPrintable(why));
        o.book.setSheetDeleted(1, true);
        SaveChanges del;
        SheetOp op;
        op.kind = SheetOp::Kind::Delete;
        op.name = QStringLiteral("Sheet2");
        del.sheetsChanged = true;
        del.sheetOps = {op};
        QByteArray out;
        QVERIFY2(saveXlsx(o.book, del, &out, &why), qPrintable(why));
        parts = unzip(out);
        QVERIFY(parts.contains(QStringLiteral("xl/embeddings/oleObject1.bin")));
        QVERIFY(!parts.contains(QStringLiteral("xl/media/p2.png")));
        QVERIFY(parts.contains(QStringLiteral("xl/media/p1.png")));
        // and with a part of its own, the part goes too
        Spec own = second;
        own.target = QStringLiteral("/xl/embeddings/oleObject2.bin");
        const QByteArray separate = bookWith({first, own}, 2);
        Opened o2;
        QVERIFY2(open(&o2, separate, &why), qPrintable(why));
        o2.book.setSheetDeleted(1, true);
        QByteArray out2;
        QVERIFY2(saveXlsx(o2.book, del, &out2, &why), qPrintable(why));
        parts = unzip(out2);
        QCOMPARE(int(partsUnder(parts, QStringLiteral("xl/embeddings/")).size()), 1);
        QCOMPARE(int(partsUnder(parts, QStringLiteral("xl/media/")).size()), 1);
    }
};

JP_TEST_CLASS(SheetOleObjectTests)

#include "tst_sheet_oleobjects.moc"

// Tests of the embedded objects of a sheet (libs/office/sheet/drawing.h, saved by xlsxsave.h): an object of ours put in, saved, and read back with the same bytes and picture; an object of
// another program kept byte for byte (its element, its attributes, its part, its picture); a replaced object leaving no part behind; the caps; and a hostile file. Every workbook is built in
// memory from plain parts.
#include "drawinghelpers.h"

#include "ole/embedobj.h"
#include "sheet/drawing.h"
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
        QCOMPARE(int(rels.count("<Relationship ")), 2);
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
        QVERIFY(o.package.inflatedBytes() - before < 300000);
    }
};

JP_TEST_CLASS(SheetOleObjectTests)

#include "tst_sheet_oleobjects.moc"

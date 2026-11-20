// Tests of the notes of an OpenDocument spreadsheet: read from an annotation in a cell (text, author, shown or not, and the box from its width, height, and position), written back as
// the standard's annotation, kept through .ods -> .ods and .xlsx -> .ods -> .xlsx, and bounded when the file is hostile. Every document is built in memory by odshelpers.
#include "odshelpers.h"

#include "io/zip.h"
#include "sheet/foreignbook.h"
#include "sheet/notes.h"
#include "sheet/oldsave.h"
#include "sheet/xlsxsave.h"

#include <QXmlStreamReader>

#include <cmath>

namespace {

const QString kAutomatic = QStringLiteral(
    "<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"2cm\"/></style:style>"
    "<style:style style:name=\"ro1\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"1cm\" style:use-optimal-row-height=\"false\"/></style:style>");

// A sheet of `cols` columns two centimeters wide and rows one centimeter high, with the rows given (each already a row of the sheet).
QByteArray gridBook(const QString &rows, int cols = 12)
{
    return buildOds(contentXml(table(QStringLiteral("S"), columns(cols, QStringLiteral("table:style-name=\"co1\"")) + rows), kAutomatic));
}

QString rowOf(const QString &cells, int repeat = 1)
{
    return row(cells, QStringLiteral("table:style-name=\"ro1\"") + (repeat > 1 ? QStringLiteral(" table:number-rows-repeated=\"%1\"").arg(repeat) : QString()));
}

QString annotation(const QString &inside, const QString &attrs = QString())
{
    return QStringLiteral("<office:annotation%1>%2</office:annotation>").arg(attrs.isEmpty() ? QString() : QLatin1Char(' ') + attrs, inside);
}

QString notedCell(const QString &note, const QString &attrs = QString()) { return cell(attrs, note); }

bool openIt(const QByteArray &bytes, Workbook *wb, QString *why, const Limits &limits = Limits()) { return openBytes(bytes, QStringLiteral("notes.ods"), wb, why, limits); }

std::map<quint64, NoteInfo> notesOf(const Workbook &wb, int sheet = 0, bool *truncated = nullptr) { return readSheetNoteInfos(wb, sheet, truncated); }

QByteArray odsOf(const Workbook &wb, CopyReport *report = nullptr, QString *why = nullptr)
{
    QByteArray ods;
    CopyReport local;
    QString message;
    if (!saveOds(wb, &ods, report ? report : &local, &message)) {
        if (why) *why = message;
        return QByteArray();
    }
    return ods;
}

QByteArray contentOf(const QByteArray &ods)
{
    QMap<QString, QByteArray> parts;
    jp::readZip(ods, parts);
    return parts.value(QStringLiteral("content.xml"));
}

const QString kNs = QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:office:1.0");

} // namespace

class SheetOdsNotesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The text (paragraphs, line breaks, spaces, and spans), the author, and whether it is shown are read from an annotation; one that is not said to be shown is hidden.
    void anAnnotationIsReadWithItsTextAuthorAndShownState()
    {
        const QString shown = annotation(QStringLiteral("<dc:creator>Ann</dc:creator><dc:date>2026-01-02T03:04:05</dc:date><text:p>one<text:span text:style-name=\"T1\">two</text:span></text:p>"
                                                        "<text:p>three<text:line-break/>four<text:s text:c=\"3\"/>x<text:tab/>y</text:p>"),
                                         QStringLiteral("office:display=\"true\""));
        const QString hidden = annotation(QStringLiteral("<dc:creator>Bob</dc:creator><text:p>quiet</text:p>"));
        const QString off = annotation(QStringLiteral("<text:p>off</text:p>"), QStringLiteral("office:display=\"false\""));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(gridBook(rowOf(notedCell(shown) + emptyCell() + notedCell(hidden)) + rowOf(emptyCell() + notedCell(off))), &wb, &why), qPrintable(why));
        const auto notes = notesOf(wb);
        QCOMPARE(notes.size(), size_t(3));
        const NoteInfo &a = notes.at(noteKey(0, 0));
        QCOMPARE(a.text, QStringLiteral("onetwo\nthree\nfour   x\ty"));
        QCOMPARE(a.author, QStringLiteral("Ann"));
        QVERIFY(a.shown);
        QVERIFY(!a.box.set);
        const NoteInfo &b = notes.at(noteKey(0, 2));
        QCOMPARE(b.text, QStringLiteral("quiet"));
        QCOMPARE(b.author, QStringLiteral("Bob"));
        QVERIFY(!b.shown);
        QVERIFY(!notes.at(noteKey(1, 1)).shown);
        QVERIFY(notes.at(noteKey(1, 1)).author.isEmpty());
        // the notes are no longer said to be left out
        QVERIFY(!wb.foreignBook()->skipped().contains(QStringLiteral("comments")));
    }

    // The box: the annotation's width, height, and position (from the sheet's top left) become the corners the .xlsx path has, counted from the note's cell: here columns 2 cm wide and rows
    // 1 cm high, a note on B2 at 2.5 cm, 1.25 cm, 3 cm wide and 1.5 cm high.
    void theBoxOfAnAnnotationIsCountedFromItsCell()
    {
        const QString note = annotation(QStringLiteral("<text:p>box</text:p>"), QStringLiteral("office:display=\"true\" svg:x=\"2.5cm\" svg:y=\"1.25cm\" svg:width=\"3cm\" svg:height=\"1.5cm\""));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(gridBook(rowOf(emptyCell()) + rowOf(emptyCell() + notedCell(note))), &wb, &why), qPrintable(why));
        const auto notes = notesOf(wb);
        QCOMPARE(notes.size(), size_t(1));
        const NoteBox box = notes.at(noteKey(1, 1)).box;
        QVERIFY(box.set);
        auto near = [](int got, int want) { return std::abs(got - want) <= 1; };
        // (the top left is 0.5 cm into column B and 0.25 cm into row 2; the bottom right 1.5 cm into column D and 0.75 cm into row 3; 1 cm is 37.8 pixels)
        QCOMPARE(box.col1, 0);
        QCOMPARE(box.row1, 0);
        QVERIFY2(near(box.x1, 19), qPrintable(QString::number(box.x1)));
        QVERIFY2(near(box.y1, 9), qPrintable(QString::number(box.y1)));
        QCOMPARE(box.col2, 1);
        QCOMPARE(box.row2, 1);
        QVERIFY2(near(box.x2, 57), qPrintable(QString::number(box.x2)));
        QVERIFY2(near(box.y2, 28), qPrintable(QString::number(box.y2)));
        // other units say the same place
        const QString inches = annotation(QStringLiteral("<text:p>in</text:p>"), QStringLiteral("svg:x=\"%1in\" svg:y=\"%2pt\" svg:width=\"%3mm\" svg:height=\"%4pc\"").arg(2.5 / 2.54, 0, 'f', 6).arg(1.25 / 2.54 * 72, 0, 'f', 4).arg(30).arg(1.5 / 2.54 * 6, 0, 'f', 5));
        Workbook wb2;
        QVERIFY2(openIt(gridBook(rowOf(emptyCell()) + rowOf(emptyCell() + notedCell(inches))), &wb2, &why), qPrintable(why));
        const NoteBox again = notesOf(wb2).at(noteKey(1, 1)).box;
        QVERIFY(again.set);
        QVERIFY(near(again.x1, box.x1) && near(again.y1, box.y1) && near(again.x2, box.x2) && near(again.y2, box.y2));
        QCOMPARE(again.col2, box.col2);
        QCOMPARE(again.row2, box.row2);
    }

    // A box that cannot be read (a negative place, a size of nothing or less, not a number, infinity, a unit that is not a length, no unit, a length too large to be a place on a sheet, one
    // missing) is no box, and the note is still read; a very large one that is a length is held to the bounds of a box.
    void aBoxThatCannotBeReadIsNoBoxAndTheNoteIsKept()
    {
        const QStringList bad = {
            QStringLiteral("svg:x=\"-1cm\" svg:y=\"1cm\" svg:width=\"3cm\" svg:height=\"1cm\""),   QStringLiteral("svg:x=\"1cm\" svg:y=\"-1cm\" svg:width=\"3cm\" svg:height=\"1cm\""),
            QStringLiteral("svg:x=\"1cm\" svg:y=\"1cm\" svg:width=\"0cm\" svg:height=\"1cm\""),    QStringLiteral("svg:x=\"1cm\" svg:y=\"1cm\" svg:width=\"-3cm\" svg:height=\"1cm\""),
            QStringLiteral("svg:x=\"nancm\" svg:y=\"1cm\" svg:width=\"3cm\" svg:height=\"1cm\""),  QStringLiteral("svg:x=\"infcm\" svg:y=\"1cm\" svg:width=\"3cm\" svg:height=\"1cm\""),
            QStringLiteral("svg:x=\"1cm\" svg:y=\"1cm\" svg:width=\"3em\" svg:height=\"1cm\""),    QStringLiteral("svg:x=\"1cm\" svg:y=\"1cm\" svg:width=\"30%\" svg:height=\"1cm\""),
            QStringLiteral("svg:x=\"1\" svg:y=\"1cm\" svg:width=\"3cm\" svg:height=\"1cm\""),      QStringLiteral("svg:x=\"1cm\" svg:y=\"1cm\" svg:width=\"1e400cm\" svg:height=\"1cm\""),
            QStringLiteral("svg:x=\"1e12cm\" svg:y=\"1cm\" svg:width=\"3cm\" svg:height=\"1cm\""), QStringLiteral("svg:x=\"1cm\" svg:y=\"1cm\" svg:width=\"3cm\""),
            QStringLiteral("svg:x=\"cm\" svg:y=\"1cm\" svg:width=\"3cm\" svg:height=\"1cm\""),     QStringLiteral("svg:x=\"1 xx\" svg:y=\"1cm\" svg:width=\"3cm\" svg:height=\"1cm\"")};
        QString cells;
        for (const QString &attrs : bad) cells += notedCell(annotation(QStringLiteral("<text:p>n</text:p>"), attrs));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(gridBook(rowOf(cells), int(bad.size())), &wb, &why), qPrintable(why));
        const auto notes = notesOf(wb);
        QCOMPARE(notes.size(), size_t(bad.size()));
        for (const auto &n : notes) {
            QCOMPARE(n.second.text, QStringLiteral("n"));
            QVERIFY2(!n.second.box.set, qPrintable(QString::number(n.first & 0xffffffffu)));
        }
        // huge but a length: kept to the bounds of a box
        const QString huge = annotation(QStringLiteral("<text:p>h</text:p>"), QStringLiteral("svg:x=\"1000000cm\" svg:y=\"1000000cm\" svg:width=\"1000000cm\" svg:height=\"900000cm\""));
        Workbook wb2;
        QVERIFY2(openIt(gridBook(rowOf(notedCell(huge))), &wb2, &why), qPrintable(why));
        const NoteBox box = notesOf(wb2).at(noteKey(0, 0)).box;
        QVERIFY(box.set);
        for (const int v : {box.col1, box.col2, box.row1, box.row2}) QVERIFY(std::abs(v) <= kMaxNoteBoxCells);
        for (const int v : {box.x1, box.x2, box.y1, box.y2}) QVERIFY(std::abs(v) <= kMaxNoteBoxPixels);
    }

    // An annotation in a cell that is repeated across a million columns and a million rows is one note, on the first cell of the repeat: a cell repeated is a copy of the first, and a note is
    // one thing with one place; a million of them would be a million notes, past what a sheet may have. Several annotations in one cell are one note too (the first).
    void anAnnotationInARepeatedCellIsOneNoteOnTheFirstCell()
    {
        const QString note = annotation(QStringLiteral("<text:p>again</text:p>"));
        const QString second = annotation(QStringLiteral("<text:p>second</text:p>"));
        const QString cells = emptyCell() + cell(QStringLiteral("table:number-columns-repeated=\"1000000\""), note) + notedCell(note + second);
        Workbook wb;
        QString why;
        QVERIFY2(openIt(gridBook(rowOf(emptyCell()) + row(cells, QStringLiteral("table:number-rows-repeated=\"1000000\""))), &wb, &why), qPrintable(why));
        const auto notes = notesOf(wb);
        QCOMPARE(notes.size(), size_t(1));
        QVERIFY(notes.count(noteKey(1, 1)));
        QCOMPARE(notes.at(noteKey(1, 1)).text, QStringLiteral("again"));
        // and a cell with two annotations has the first
        Workbook wb2;
        QVERIFY2(openIt(gridBook(rowOf(notedCell(note + second))), &wb2, &why), qPrintable(why));
        QCOMPARE(notesOf(wb2).size(), size_t(1));
        QCOMPARE(notesOf(wb2).at(noteKey(0, 0)).text, QStringLiteral("again"));
    }

    // At most kMaxNotes notes are read from a sheet, the text of a note is held to 100,000 characters, and what was left out is said.
    void theNotesOfASheetAndTheTextOfANoteAreCapped()
    {
        const QString note = annotation(QStringLiteral("<text:p>n</text:p>"));
        QString rows;
        for (qint64 i = 0; i < kMaxNotes + 5; ++i) rows += row(notedCell(note));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), columns(1) + rows))), &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(qint64(s.importedNotes.size()), kMaxNotes);
        QVERIFY2(s.notKept.join(QLatin1Char('|')).contains(QStringLiteral("notes past the")), qPrintable(s.notKept.join(QLatin1Char('|'))));
        bool truncated = false;
        QCOMPARE(qint64(notesOf(wb, 0, &truncated).size()), kMaxNotes);
        const QString longText = annotation(QStringLiteral("<text:p>") + QString(150000, QLatin1Char('x')) + QStringLiteral("</text:p>"));
        Workbook wb2;
        QVERIFY2(openIt(gridBook(rowOf(notedCell(longText))), &wb2, &why), qPrintable(why));
        QCOMPARE(notesOf(wb2).at(noteKey(0, 0)).text.size(), qsizetype(100000));
        Sheet s2;
        QVERIFY(wb2.readSheet(0, &s2));
        QVERIFY2(s2.notKept.join(QLatin1Char('|')).contains(QStringLiteral("notes cut")), qPrintable(s2.notKept.join(QLatin1Char('|'))));
        // spaces stand for characters too: a few bytes of `text:s` do not make a note of gigabytes
        const QString spaces = annotation(QStringLiteral("<text:p>a<text:s text:c=\"2000000000\"/>b</text:p>"));
        Workbook wb3;
        QVERIFY2(openIt(gridBook(rowOf(notedCell(spaces))), &wb3, &why), qPrintable(why));
        QVERIFY(notesOf(wb3).at(noteKey(0, 0)).text.size() <= 100000);
    }

    // The characters of all the notes of a sheet are bounded too (a note of 100,000 characters in each of many cells is not read past the budget).
    void theCharactersOfAllTheNotesOfASheetAreBounded()
    {
        const QString big = annotation(QStringLiteral("<text:p>") + QString(100000, QLatin1Char('x')) + QStringLiteral("</text:p>"));
        QString rows;
        const int count = int(kMaxNoteChars / 100000) + 20;
        for (int i = 0; i < count; ++i) rows += row(notedCell(big));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), columns(1) + rows))), &wb, &why), qPrintable(why));
        qint64 chars = 0;
        for (const auto &n : notesOf(wb)) chars += n.second.text.size();
        QVERIFY(chars <= kMaxNoteChars);
        QVERIFY(chars > kMaxNoteChars - 100000 - 1000);
    }

    // Items inside items are bounded as everywhere in the reader: an annotation nested more deeply than any real file has is refused, and a deep note that is allowed is read.
    void deeplyNestedElementsInAnAnnotationAreBounded()
    {
        auto nested = [](int depth) {
            QString inside;
            for (int i = 0; i < depth; ++i) inside += QStringLiteral("<text:span>");
            inside += QStringLiteral("deep");
            for (int i = 0; i < depth; ++i) inside += QStringLiteral("</text:span>");
            return annotation(QStringLiteral("<text:p>") + inside + QStringLiteral("</text:p>"));
        };
        Workbook ok;
        QString why;
        QVERIFY2(openIt(gridBook(rowOf(notedCell(nested(50)))), &ok, &why), qPrintable(why));
        QCOMPARE(notesOf(ok).at(noteKey(0, 0)).text, QStringLiteral("deep"));
        Workbook bad;
        QVERIFY2(openIt(gridBook(rowOf(notedCell(nested(5000)))), &bad, &why), qPrintable(why));
        Sheet s;
        QVERIFY(!bad.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("more deeply")), qPrintable(s.error));
        QVERIFY(notesOf(bad).empty());
    }

    // The standard's terms, in the written file: an annotation in the cell with display, the size and place as lengths, the author as dc:creator, and the text as paragraphs; nothing of
    // another vocabulary; and the note is on the right cell when its row and column are among empty ones (a cell with a note is not an empty cell).
    void aNoteIsWrittenAsTheStandardsAnnotation()
    {
        const QString shown = annotation(QStringLiteral("<dc:creator>Ann &amp; Co</dc:creator><text:p>line one</text:p><text:p>line  two</text:p>"),
                                         QStringLiteral("office:display=\"true\" svg:x=\"6.5cm\" svg:y=\"3.25cm\" svg:width=\"4cm\" svg:height=\"2cm\""));
        const QString hidden = annotation(QStringLiteral("<text:p>quiet</text:p>"));
        Workbook wb;
        QString why;
        // a note on H4 (row 3, column 7) among empty cells and rows, one at the end of a repeated empty row, and a value cell with a note
        const QString content = rowOf(emptyCell(), 3) + rowOf(cell(QStringLiteral("table:number-columns-repeated=\"7\""), QString()) + notedCell(shown)) + rowOf(floatCell(5) + notedCell(hidden));
        QVERIFY2(openIt(gridBook(content), &wb, &why), qPrintable(why));
        CopyReport report;
        const QByteArray ods = odsOf(wb, &report, &why);
        QVERIFY2(!ods.isEmpty(), qPrintable(why));
        // (the written XML read as the standard says)
        QXmlStreamReader xml(contentOf(ods));
        struct Found {
            quint32 row = 0, col = 0;
            QString display, x, y, w, h, creator, text;
            QStringList attrNamespaces;
            int paragraphs = 0;
        };
        std::vector<Found> found;
        quint32 row = 0, col = 0;
        int depthInCell = -1, depth = 0;
        Found *now = nullptr;
        bool inCreator = false, inAnnotation = false;
        quint32 rowRepeat = 1, cellRepeat = 1;
        while (!xml.atEnd()) {
            const auto t = xml.readNext();
            if (t == QXmlStreamReader::StartElement) {
                ++depth;
                const QString name = xml.qualifiedName().toString();
                if (name == QLatin1String("table:table-row")) {
                    rowRepeat = xml.attributes().value(QLatin1String("table:number-rows-repeated")).toUInt();
                    if (rowRepeat == 0) rowRepeat = 1;
                    col = 0;
                } else if (name == QLatin1String("table:table-cell") || name == QLatin1String("table:covered-table-cell")) {
                    cellRepeat = xml.attributes().value(QLatin1String("table:number-columns-repeated")).toUInt();
                    if (cellRepeat == 0) cellRepeat = 1;
                    depthInCell = depth;
                } else if (name == QLatin1String("office:annotation")) {
                    QVERIFY2(depth == depthInCell + 1, "the annotation is a child of its cell");
                    QCOMPARE(cellRepeat, quint32(1));   // (a cell with a note is never a repeated one)
                    found.push_back({});
                    now = &found.back();
                    now->row = row;
                    now->col = col;
                    inAnnotation = true;
                    now->display = xml.attributes().value(kNs, QLatin1String("display")).toString();
                    for (const auto &a : xml.attributes()) now->attrNamespaces << a.namespaceUri().toString();
                    now->x = xml.attributes().value(QLatin1String("svg:x")).toString();
                    now->y = xml.attributes().value(QLatin1String("svg:y")).toString();
                    now->w = xml.attributes().value(QLatin1String("svg:width")).toString();
                    now->h = xml.attributes().value(QLatin1String("svg:height")).toString();
                } else if (inAnnotation && name == QLatin1String("dc:creator")) {
                    inCreator = true;
                } else if (inAnnotation && name == QLatin1String("text:p")) {
                    ++now->paragraphs;
                    now->text += now->paragraphs > 1 ? QStringLiteral("\n") : QString();
                } else if (inAnnotation && name != QLatin1String("text:s")) {
                    QFAIL(qPrintable(QStringLiteral("element in an annotation that the standard does not have there: ") + name));
                }
            } else if (t == QXmlStreamReader::EndElement) {
                const QString name = xml.qualifiedName().toString();
                if (name == QLatin1String("table:table-row")) row += rowRepeat;
                else if (name == QLatin1String("table:table-cell") || name == QLatin1String("table:covered-table-cell")) col += cellRepeat;
                else if (name == QLatin1String("office:annotation")) inAnnotation = false;
                else if (name == QLatin1String("dc:creator")) inCreator = false;
                --depth;
            } else if (t == QXmlStreamReader::Characters && inAnnotation && !xml.isWhitespace()) {
                (inCreator ? now->creator : now->text) += xml.text().toString();
            }
        }
        QVERIFY2(!xml.hasError(), qPrintable(xml.errorString()));
        QCOMPARE(found.size(), size_t(2));
        // row 3 (after the three empty ones), column 7; the shown note first
        QCOMPARE(found[0].row, quint32(3));
        QCOMPARE(found[0].col, quint32(7));
        QCOMPARE(found[0].display, QStringLiteral("true"));
        QCOMPARE(found[0].creator, QStringLiteral("Ann & Co"));
        QCOMPARE(found[0].paragraphs, 2);
        QVERIFY(found[0].text.startsWith(QStringLiteral("line one\nline")));
        QCOMPARE(QSet<QString>(found[0].attrNamespaces.begin(), found[0].attrNamespaces.end()), QSet<QString>({kNs, QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0")}));
        for (const QString &len : {found[0].x, found[0].y, found[0].w, found[0].h}) QVERIFY2(len.endsWith(QLatin1String("pt")), qPrintable(len));
        auto points = [](const QString &len) { return len.chopped(2).toDouble(); };
        // (the box is where it was: 6.5 cm, 3.25 cm, 4 cm by 2 cm; the writer says it in points, within a pixel, 0.75 point)
        QVERIFY(std::abs(points(found[0].x) - 6.5 / 2.54 * 72) <= 0.8);
        QVERIFY(std::abs(points(found[0].y) - 3.25 / 2.54 * 72) <= 0.8);
        QVERIFY(std::abs(points(found[0].w) - 4 / 2.54 * 72) <= 1.6);
        QVERIFY(std::abs(points(found[0].h) - 2 / 2.54 * 72) <= 1.6);
        // the hidden note: not shown, no box
        QCOMPARE(found[1].display, QStringLiteral("false"));
        QVERIFY(found[1].x.isEmpty() && found[1].w.isEmpty());
        QCOMPARE(found[1].row, quint32(4));
        QCOMPARE(found[1].col, quint32(1));
        QVERIFY(found[1].creator.isEmpty());
    }

    // .ods -> open -> save .ods -> open: the same notes, text, authors, shown states, and boxes within a pixel (a note with no box is still one with none).
    void notesSurviveAnOdsToOdsRoundTrip()
    {
        const QString a = annotation(QStringLiteral("<dc:creator>Ann</dc:creator><text:p>first</text:p><text:p>second</text:p>"), QStringLiteral("office:display=\"true\" svg:x=\"4.5cm\" svg:y=\"1.2cm\" svg:width=\"5cm\" svg:height=\"2.5cm\""));
        const QString b = annotation(QStringLiteral("<text:p>no box &lt;here&gt;</text:p>"));
        const QString c = annotation(QStringLiteral("<dc:creator>Cy</dc:creator><text:p>tall</text:p>"), QStringLiteral("svg:x=\"11cm\" svg:y=\"6.1cm\" svg:width=\"2cm\" svg:height=\"7cm\""));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(gridBook(rowOf(emptyCell() + notedCell(a, QStringLiteral("office:value-type=\"float\" office:value=\"1\""))) + rowOf(notedCell(b)) + rowOf(emptyCell(), 4) + rowOf(cell(QStringLiteral("table:number-columns-repeated=\"3\""), QString()) + notedCell(c))), &wb, &why),
                 qPrintable(why));
        const auto before = notesOf(wb);
        QCOMPARE(before.size(), size_t(3));
        const QByteArray ods = odsOf(wb, nullptr, &why);
        QVERIFY2(!ods.isEmpty(), qPrintable(why));
        Workbook back;
        QVERIFY2(openIt(ods, &back, &why), qPrintable(why));
        const auto after = notesOf(back);
        QCOMPARE(after.size(), before.size());
        for (const auto &n : before) {
            QVERIFY2(after.count(n.first), "note on the same cell");
            const NoteInfo &x = n.second, &y = after.at(n.first);
            QCOMPARE(y.text, x.text);
            QCOMPARE(y.author, x.author);
            QCOMPARE(y.shown, x.shown);
            QCOMPARE(y.box.set, x.box.set);
            if (!x.box.set) continue;
            auto within = [](int p, int q) { return std::abs(p - q) <= 1; };
            QCOMPARE(y.box.col1, x.box.col1);
            QCOMPARE(y.box.row1, x.box.row1);
            QCOMPARE(y.box.col2, x.box.col2);
            QCOMPARE(y.box.row2, x.box.row2);
            QVERIFY(within(y.box.x1, x.box.x1) && within(y.box.y1, x.box.y1) && within(y.box.x2, x.box.x2) && within(y.box.y2, x.box.y2));
        }
        // the cell's value is still there
        Sheet s;
        QVERIFY(back.readSheet(0, &s));
        QCOMPARE(shown(back, s, QStringLiteral("B1")), QStringLiteral("n:1"));
    }

    // .ods -> .xlsx (a save as the window makes it: the comments and the drawing of the notes) -> .ods, and .xlsx -> .ods -> .xlsx: the notes stay, with their text, authors, shown states, and boxes.
    void notesSurviveThroughTheXlsxForm()
    {
        const QString a = annotation(QStringLiteral("<dc:creator>Ann</dc:creator><text:p>first</text:p>"), QStringLiteral("office:display=\"true\" svg:x=\"4.5cm\" svg:y=\"1.2cm\" svg:width=\"5cm\" svg:height=\"2.5cm\""));
        const QString b = annotation(QStringLiteral("<dc:creator>Bob</dc:creator><text:p>second</text:p>"));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(gridBook(rowOf(emptyCell() + notedCell(a)) + rowOf(emptyCell(), 2) + rowOf(emptyCell() + emptyCell() + notedCell(b))), &wb, &why), qPrintable(why));
        const auto original = notesOf(wb);
        QCOMPARE(original.size(), size_t(2));
        QByteArray xlsx;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &xlsx, &why), qPrintable(why));
        // (the .xlsx's own parts: a comments part with the texts and authors, and a drawing with a shape for each, visible for the one shown)
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(xlsx, parts));
        const QByteArray comments = parts.value(QStringLiteral("xl/comments1.xml"));
        QVERIFY2(comments.contains("<author>Ann</author>") && comments.contains(">first<") && comments.contains(">second<") && comments.contains("ref=\"B1\"") && comments.contains("ref=\"C4\""), comments.constData());
        const QByteArray vml = parts.value(QStringLiteral("xl/drawings/vmlDrawing1.vml"));
        QCOMPARE(vml.count("ObjectType=\"Note\""), 2);
        QCOMPARE(vml.count("<x:Visible/>"), 1);
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("vmlDrawing"));
        QVERIFY(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<legacyDrawing r:id=\"rId3\"/>"));
        // the .xlsx read as one: the notes are those of the package
        jp::office::Package package;
        Workbook fromXlsx;
        QVERIFY2(package.readLazy(xlsx, &why) && fromXlsx.open(package, &why), qPrintable(why));
        const auto viaXlsx = notesOf(fromXlsx);
        QCOMPARE(viaXlsx.size(), size_t(2));
        QCOMPARE(viaXlsx.at(noteKey(0, 1)).author, QStringLiteral("Ann"));
        QVERIFY(viaXlsx.at(noteKey(0, 1)).shown);
        QVERIFY(viaXlsx.at(noteKey(0, 1)).box == original.at(noteKey(0, 1)).box);
        // .xlsx -> .ods
        const QByteArray ods = odsOf(fromXlsx, nullptr, &why);
        QVERIFY2(!ods.isEmpty(), qPrintable(why));
        Workbook viaOds;
        QVERIFY2(openIt(ods, &viaOds, &why), qPrintable(why));
        const auto second = notesOf(viaOds);
        QCOMPARE(second.size(), size_t(2));
        for (const auto &n : original) {
            QVERIFY(second.count(n.first));
            QCOMPARE(second.at(n.first).text, n.second.text);
            QCOMPARE(second.at(n.first).author, n.second.author);
            QCOMPARE(second.at(n.first).shown, n.second.shown);
            // (a note with no box is given the usual one when it is written in the .xlsx form, which every shape has)
            if (n.second.box.set) QCOMPARE(second.at(n.first).box.set, true);
        }
        QVERIFY(std::abs(second.at(noteKey(0, 1)).box.x1 - original.at(noteKey(0, 1)).box.x1) <= 1);
        // .xlsx -> .ods -> .xlsx
        QByteArray again;
        QVERIFY2(saveXlsx(viaOds, SaveChanges(), &again, &why), qPrintable(why));
        jp::office::Package package2;
        Workbook last;
        QVERIFY2(package2.readLazy(again, &why) && last.open(package2, &why), qPrintable(why));
        const auto third = notesOf(last);
        QCOMPARE(third.size(), size_t(2));
        QCOMPARE(third.at(noteKey(3, 2)).text, QStringLiteral("second"));
        QCOMPARE(third.at(noteKey(3, 2)).author, QStringLiteral("Bob"));
        QVERIFY(!third.at(noteKey(3, 2)).shown);
    }

    // Many notes on a sheet with many other rows: each on its cell, the empty rows between them written as repeats and not as rows, and the count kept.
    void manyNotesAreWrittenOnTheirCellsWithTheEmptyRowsRepeated()
    {
        QString rows;
        const int count = 3000;
        for (int i = 0; i < count; ++i) {
            rows += rowOf(emptyCell(), 100);   // (a hundred empty rows)
            rows += rowOf((i % 40 ? cell(QStringLiteral("table:number-columns-repeated=\"%1\"").arg(i % 40), QString()) : QString()) + notedCell(annotation(QStringLiteral("<text:p>n%1</text:p>").arg(i))));
        }
        Workbook wb;
        QString why;
        QVERIFY2(openIt(gridBook(rows, 60), &wb, &why), qPrintable(why));
        QCOMPARE(notesOf(wb).size(), size_t(count));
        const QByteArray ods = odsOf(wb, nullptr, &why);
        QVERIFY2(!ods.isEmpty(), qPrintable(why));
        const QByteArray content = contentOf(ods);
        // (the empty rows are runs: no row is written for each of them)
        QVERIFY2(content.count("<table:table-row") <= 2 * count + 2, qPrintable(QString::number(content.count("<table:table-row"))));
        Workbook back;
        QVERIFY2(openIt(ods, &back, &why), qPrintable(why));
        const auto notes = notesOf(back);
        QCOMPARE(notes.size(), size_t(count));
        for (int i = 0; i < count; i += 97) QCOMPARE(notes.at(noteKey(quint32(i * 101 + 100), quint32(i % 40))).text, QStringLiteral("n%1").arg(i));
    }

    // A note on a cell that a merged range covers is not written (the cell is not one in the file) and is said; the note on the range's first cell is.
    void aNoteOnACoveredCellIsLeftOutAndSaid()
    {
        const QString note = annotation(QStringLiteral("<text:p>n</text:p>"));
        Workbook wb;
        QString why;
        const QString cells = cell(QStringLiteral("table:number-columns-spanned=\"2\" table:number-rows-spanned=\"1\""), note) + QStringLiteral("<table:covered-table-cell>") + note + QStringLiteral("</table:covered-table-cell>");
        QVERIFY2(openIt(gridBook(rowOf(cells)), &wb, &why), qPrintable(why));
        QCOMPARE(notesOf(wb).size(), size_t(2));
        CopyReport report;
        const QByteArray ods = odsOf(wb, &report, &why);
        QVERIFY2(!ods.isEmpty(), qPrintable(why));
        QVERIFY2(report.trimmed.join(QLatin1Char('|')).contains(QStringLiteral("note")), qPrintable(report.trimmed.join(QLatin1Char('|'))));
        Workbook back;
        QVERIFY2(openIt(ods, &back, &why), qPrintable(why));
        const auto notes = notesOf(back);
        QCOMPARE(notes.size(), size_t(1));
        QVERIFY(notes.count(noteKey(0, 0)));
    }
};

JP_TEST_CLASS(SheetOdsNotesTests)
#include "tst_sheet_odsnotes.moc"

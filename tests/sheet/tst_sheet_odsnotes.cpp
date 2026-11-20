// Tests of the notes of an OpenDocument spreadsheet: read from an annotation in a cell (text, author, shown or not, and the box from its width, height, and position), written back as
// the standard's annotation, kept through .ods -> .ods and .xlsx -> .ods -> .xlsx, and bounded when the file is hostile. Every document is built in memory by odshelpers.
#include "odshelpers.h"

#include "io/zip.h"
#include "sheet/foreignbook.h"
#include "sheet/notes.h"
#include "sheet/notesave.h"
#include "sheet/odsdraw.h"
#include "sheet/odsnotes.h"
#include "sheet/oldsave.h"
#include "sheet/xlsxsave.h"

#include <QXmlStreamReader>

#include <QRegularExpression>

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

// The svg lengths (in points) of the first annotation of a saved document's content, and its date; false when it has no annotation.
struct WrittenBox {
    double x = 0, y = 0, w = 0, h = 0;
    QString date;
    bool lengths = false;
};
WrittenBox firstAnnotation(const QByteArray &content)
{
    WrittenBox out;
    const QString text = QString::fromUtf8(content);
    const QRegularExpression tag(QStringLiteral("<office:annotation ([^>]*)>"));
    const auto m = tag.match(text);
    if (!m.hasMatch()) return out;
    auto length = [&](const char *name, double *v) {
        const auto a = QRegularExpression(QStringLiteral("svg:%1=\"([0-9.eE+-]+)pt\"").arg(QLatin1String(name))).match(m.captured(1));
        if (!a.hasMatch()) return false;
        *v = a.captured(1).toDouble();
        return true;
    };
    out.lengths = length("x", &out.x) && length("y", &out.y) && length("width", &out.w) && length("height", &out.h);
    const auto d = QRegularExpression(QStringLiteral("<dc:date>([^<]*)</dc:date>")).match(text, m.capturedEnd());
    if (d.hasMatch()) out.date = d.captured(1);
    return out;
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
        QVERIFY2(s.notKept.join(QLatin1Char('|')).contains(QStringLiteral("Only the first")), qPrintable(s.notKept.join(QLatin1Char('|'))));
        bool truncated = false;
        QCOMPARE(qint64(notesOf(wb, 0, &truncated).size()), kMaxNotes);
        const QString longText = annotation(QStringLiteral("<text:p>") + QString(150000, QLatin1Char('x')) + QStringLiteral("</text:p>"));
        Workbook wb2;
        QVERIFY2(openIt(gridBook(rowOf(notedCell(longText))), &wb2, &why), qPrintable(why));
        QCOMPARE(notesOf(wb2).at(noteKey(0, 0)).text.size(), qsizetype(100000));
        Sheet s2;
        QVERIFY(wb2.readSheet(0, &s2));
        QVERIFY2(s2.notKept.join(QLatin1Char('|')).contains(QStringLiteral("cut to")), qPrintable(s2.notKept.join(QLatin1Char('|'))));
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

    // A note on a cell that a merged range covers is written on that cell (the format lets a covered cell hold content) and read back; the range and the note on its first cell stay.
    void aNoteOnACoveredCellIsWrittenOnIt()
    {
        const QString first = annotation(QStringLiteral("<text:p>first</text:p>")), middle = annotation(QStringLiteral("<text:p>middle</text:p>"));
        Workbook wb;
        QString why;
        const QString cells = cell(QStringLiteral("table:number-columns-spanned=\"3\" table:number-rows-spanned=\"1\""), first) + QStringLiteral("<table:covered-table-cell>") + middle + QStringLiteral("</table:covered-table-cell>")
                              + QStringLiteral("<table:covered-table-cell/>");
        QVERIFY2(openIt(gridBook(rowOf(cells)), &wb, &why), qPrintable(why));
        QCOMPARE(notesOf(wb).size(), size_t(2));
        CopyReport report;
        const QByteArray ods = odsOf(wb, &report, &why);
        QVERIFY2(!ods.isEmpty(), qPrintable(why));
        QVERIFY2(!report.trimmed.join(QLatin1Char('|')).contains(QStringLiteral("note")), qPrintable(report.trimmed.join(QLatin1Char('|'))));
        const QByteArray content = contentOf(ods);
        QVERIFY2(content.contains("<table:covered-table-cell><office:annotation"), "the note is inside the covered cell");
        QCOMPARE(content.count("<office:annotation "), 2);
        QCOMPARE(content.count("</table:covered-table-cell>"), 1);
        Workbook back;
        QVERIFY2(openIt(ods, &back, &why), qPrintable(why));
        const auto notes = notesOf(back);
        QCOMPARE(notes.size(), size_t(2));
        QCOMPARE(notes.at(noteKey(0, 0)).text, QStringLiteral("first"));
        QCOMPARE(notes.at(noteKey(0, 1)).text, QStringLiteral("middle"));
        Sheet s;
        QVERIFY(back.readSheet(0, &s));
        QCOMPARE(s.merges.size(), qsizetype(1));
        QCOMPARE(quint32(s.merges[0].col2), quint32(2));
    }

    // The notes of all a workbook's sheets together are bounded as those of one workbook are, in a read and in both writers: each sheet is under its own bound (8.3 million characters), and the five
    // together are over the workbook's.
    void theNotesOfAllTheSheetsAreBoundedForTheWorkbook()
    {
        const QString big = annotation(QStringLiteral("<text:p><text:s text:c=\"100000\"/></text:p>"));
        QString rows;
        for (int i = 0; i < 83; ++i) rows += row(notedCell(big));
        QString tables;
        for (int k = 1; k <= 5; ++k) tables += table(QStringLiteral("S%1").arg(k), columns(1) + rows);
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(contentXml(tables)), &wb, &why), qPrintable(why));
        // read the way the loader gathers them: one context for the workbook
        NoteReadContext context;
        qint64 total = 0;
        int cutSheets = 0;
        for (int i = 0; i < 5; ++i) {
            bool truncated = false;
            const auto notes = readSheetNoteInfos(wb, i, &truncated, kMaxNotes, &context);
            for (const auto &n : notes) total += n.second.text.size() + n.second.author.size();
            if (truncated) ++cutSheets;
        }
        QVERIFY2(total <= kMaxNoteCharsPerBook, qPrintable(QString::number(total)));
        QVERIFY(total >= 4 * 8300000);
        QVERIFY(cutSheets >= 1);
        // written as .ods: whole sheets that fit, and the rest left out and said
        CopyReport report;
        const QByteArray ods = odsOf(wb, &report, &why);
        QVERIFY2(!ods.isEmpty(), qPrintable(why));
        QVERIFY2(report.notKept.join(QLatin1Char('|')).contains(QStringLiteral("more note text")), qPrintable(report.notKept.join(QLatin1Char('|'))));
        Workbook back;
        QVERIFY2(openIt(ods, &back, &why), qPrintable(why));
        qint64 written = 0;
        int withNotes = 0;
        for (int i = 0; i < 5; ++i) {
            const auto notes = notesOf(back, i);
            if (!notes.empty()) ++withNotes;
            for (const auto &n : notes) written += n.second.text.size() + n.second.author.size();
        }
        QCOMPARE(withNotes, 4);
        QVERIFY2(written <= kMaxNoteCharsPerBook, qPrintable(QString::number(written)));
        // written as .xlsx: the same four sheets
        QByteArray xlsx;
        SaveReport saveReport;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &xlsx, &why, &saveReport), qPrintable(why));
        QVERIFY2(saveReport.notes.join(QLatin1Char('|')).contains(QStringLiteral("more note text")), qPrintable(saveReport.notes.join(QLatin1Char('|'))));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(xlsx, parts));
        int commentParts = 0;
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().startsWith(QLatin1String("xl/comments"))) ++commentParts;
        QCOMPARE(commentParts, 5);   // (the fifth sheet keeps the notes that fit: see theNotesOfTheSheetThatReachesTheWorkbooksLimitAreKeptWholeUpToIt)
    }

    // Past the workbook's limit on note text, the notes of the sheet that reaches it are kept whole up to the limit (not the whole sheet's dropped), and the count left out is said.
    void theNotesOfTheSheetThatReachesTheWorkbooksLimitAreKeptWholeUpToIt()
    {
        const QString big = annotation(QStringLiteral("<text:p><text:s text:c=\"100000\"/></text:p>"));
        QString rows;
        for (int i = 0; i < 83; ++i) rows += row(notedCell(big));
        QString tables;
        for (int k = 1; k <= 5; ++k) tables += table(QStringLiteral("S%1").arg(k), columns(1) + rows);
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(contentXml(tables)), &wb, &why), qPrintable(why));
        QByteArray xlsx;
        SaveReport report;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &xlsx, &why, &report), qPrintable(why));
        Workbook back;
        jp::office::Package package;
        QVERIFY2(package.readLazy(xlsx, &why) && back.open(package, &why), qPrintable(why));
        qint64 written = 0, kept = 0;
        for (int i = 0; i < 5; ++i) {
            const auto notes = notesOf(back, i);
            kept += qint64(notes.size());
            for (const auto &n : notes) {
                written += n.second.text.size() + n.second.author.size();
                QCOMPARE(n.second.text.size(), qsizetype(100000));
            }
            if (i < 4) QCOMPARE(notes.size(), size_t(83));
        }
        QVERIFY2(written <= kMaxNoteCharsPerBook, qPrintable(QString::number(written)));
        QVERIFY(notesOf(back, 4).size() > 0 && notesOf(back, 4).size() < 83);   // (the fifth sheet keeps what fits)
        const qint64 left = 5 * 83 - kept;
        QVERIFY(left > 0);
        const QString said = report.notes.join(QLatin1Char('|'));
        QVERIFY2(said.contains(QStringLiteral("more note text")) && said.contains(QString::number(left) + QStringLiteral(" note")), qPrintable(said));
    }

    // A sheet whose cell text says "annotation" but has no note is not read again to look for notes (the real annotation elements are found in the first read), whether the sheet is saved as .xlsx or
    // as .ods.
    void aSheetThatOnlySaysAnnotationIsNotReadAgain()
    {
        auto readsToSave = [&](const QString &word, bool xlsx) {
            Workbook wb;
            QString why;
            if (!openIt(gridBook(rowOf(stringCell(word))), &wb, &why)) return qint64(-1);
            Sheet s;
            if (!wb.readSheet(0, &s)) return qint64(-1);
            const qint64 before = odsdraw::testHooks().sheetReads.load();
            QByteArray out;
            CopyReport report;
            const bool ok = xlsx ? saveXlsx(wb, SaveChanges(), &out, &why) : saveOds(wb, &out, &report, &why);
            return ok ? odsdraw::testHooks().sheetReads.load() - before : qint64(-1);
        };
        for (bool xlsx : {true, false}) {
            const qint64 plain = readsToSave(QStringLiteral("a plain word"), xlsx);
            QVERIFY(plain >= 0);
            QCOMPARE(readsToSave(QStringLiteral("the annotation of it"), xlsx), plain);
        }
    }

    // The note's date is kept: ods -> ods writes the standard's date element after the author, and a note with no date writes none. Through the .xlsx form (which has no date for a note) it is gone.
    void theDateOfANoteIsKeptInThisFormat()
    {
        const QString dated = annotation(QStringLiteral("<dc:creator>Ann</dc:creator><dc:date>2010-02-07T00:00:00</dc:date><text:p>x</text:p>"));
        const QString undated = annotation(QStringLiteral("<dc:creator>Bob</dc:creator><text:p>y</text:p>"));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(gridBook(rowOf(notedCell(dated) + notedCell(undated))), &wb, &why), qPrintable(why));
        const QByteArray ods = odsOf(wb, nullptr, &why);
        QVERIFY2(!ods.isEmpty(), qPrintable(why));
        const QByteArray content = contentOf(ods);
        QVERIFY2(content.contains("<dc:creator>Ann</dc:creator><dc:date>2010-02-07T00:00:00</dc:date><text:p>x</text:p>"), content.constData());
        QCOMPARE(content.count("<dc:date>"), 1);
        // (and again: the date of the saved file is read, and written, as it was)
        Workbook back;
        QVERIFY2(openIt(ods, &back, &why), qPrintable(why));
        const QByteArray again = odsOf(back, nullptr, &why);
        QVERIFY2(contentOf(again).contains("<dc:date>2010-02-07T00:00:00</dc:date>"), "the date survives a second save");
        // through .xlsx
        QByteArray xlsx;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &xlsx, &why), qPrintable(why));
        jp::office::Package package;
        Workbook fromXlsx;
        QVERIFY2(package.readLazy(xlsx, &why) && fromXlsx.open(package, &why), qPrintable(why));
        QVERIFY(!contentOf(odsOf(fromXlsx, nullptr, &why)).contains("<dc:date>"));
    }

    // A file made to be saved on as an OpenDocument one carries what an .xlsx cannot hold of a note (its date, and that it has no box of its own), in the comments and the drawing, and a change that puts a
    // note carries them too; a plain .xlsx holds neither.
    void anXlsxMadeToBeSavedOnAsOdsCarriesTheDateAndTheNoBox()
    {
        const QString dated = annotation(QStringLiteral("<dc:creator>Ann</dc:creator><dc:date>2010-02-07T00:00:00</dc:date><text:p>x</text:p>"), QStringLiteral("office:display=\"true\" svg:x=\"4.5cm\" svg:y=\"1.2cm\" svg:width=\"5cm\" svg:height=\"2.5cm\""));
        const QString plain = annotation(QStringLiteral("<dc:creator>Bob</dc:creator><text:p>y</text:p>"));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(gridBook(rowOf(notedCell(dated) + notedCell(plain))), &wb, &why), qPrintable(why));
        const auto was = notesOf(wb);
        SaveChanges keep;
        keep.keepOdfNotes = true;
        QByteArray xlsx;
        QVERIFY2(saveXlsx(wb, keep, &xlsx, &why), qPrintable(why));
        jp::office::Package package;
        Workbook made;
        QVERIFY2(package.readLazy(xlsx, &why) && made.open(package, &why), qPrintable(why));
        const auto now = notesOf(made);
        QCOMPARE(now.at(noteKey(0, 0)).date, QStringLiteral("2010-02-07T00:00:00"));
        QVERIFY(now.at(noteKey(0, 0)).box == was.at(noteKey(0, 0)).box);
        QVERIFY(now.at(noteKey(0, 1)).date.isEmpty());
        QVERIFY(!now.at(noteKey(0, 1)).box.set);
        const QByteArray ods = contentOf(odsOf(made, nullptr, &why));
        QVERIFY2(ods.contains("<dc:date>2010-02-07T00:00:00</dc:date>"), "the date is written");
        // a change that puts notes: one with a date and no box, one that gives the note without a box one
        SaveChanges edit = keep;
        NoteOp put;
        put.row = 0;
        put.col = 3;
        put.author = QStringLiteral("Cy");
        put.text = QStringLiteral("new");
        put.date = QStringLiteral("2020-01-02T03:04:05");
        NoteOp move;
        move.row = 0;
        move.col = 1;
        move.author = QStringLiteral("Bob");
        move.text = QStringLiteral("y");
        move.box = {true, 0, 1, 7, 8, 1, 3, 9, 10};
        edit.sheets[0].noteOps = {put, move};
        QByteArray changed;
        QVERIFY2(saveXlsx(wb, edit, &changed, &why), qPrintable(why));
        Workbook again;
        jp::office::Package package2;
        QVERIFY2(package2.readLazy(changed, &why) && again.open(package2, &why), qPrintable(why));
        const auto after = notesOf(again);
        QCOMPARE(after.at(noteKey(0, 3)).date, QStringLiteral("2020-01-02T03:04:05"));
        QVERIFY(!after.at(noteKey(0, 3)).box.set);
        QVERIFY(after.at(noteKey(0, 1)).box == move.box);
        QCOMPARE(after.at(noteKey(0, 0)).date, QStringLiteral("2010-02-07T00:00:00"));
        // a plain .xlsx: no date, and every note has a box
        SaveChanges edit2;
        edit2.sheets[0].noteOps = {put};
        QByteArray plainXlsx;
        QVERIFY2(saveXlsx(wb, edit2, &plainXlsx, &why), qPrintable(why));
        Workbook third;
        jp::office::Package package3;
        QVERIFY2(package3.readLazy(plainXlsx, &why) && third.open(package3, &why), qPrintable(why));
        const auto last = notesOf(third);
        QVERIFY(last.at(noteKey(0, 3)).date.isEmpty() && last.at(noteKey(0, 3)).box.set);
        QVERIFY(last.at(noteKey(0, 0)).date.isEmpty() && last.at(noteKey(0, 1)).box.set);
    }

    // The notes on rows 1 to 3 of a real older file (a box above its cell: every one has y 0 in): numbers copied from it into a file built here. Read, saved as .ods (the box is where it was, none of
    // its lengths negative, the round trip exact), and saved as .xlsx (no anchor number below zero, the box read back as it was).
    void notesWithABoxAboveTheirCellSurviveBothForms()
    {
        const QString styles = QStringLiteral(
            "<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"0.889in\"/></style:style>"
            "<style:style style:name=\"ro1\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"0.178in\" style:use-optimal-row-height=\"true\"/></style:style>");
        QString rows;
        for (int i = 1; i <= 3; ++i)
            rows += row(notedCell(annotation(QStringLiteral("<dc:date>2010-02-07T00:00:00</dc:date><text:p>Note %1</text:p>").arg(i),
                                             QStringLiteral("draw:style-name=\"gr1\" draw:text-style-name=\"P1\" svg:width=\"1.1413in\" svg:height=\"0.2346in\" svg:x=\"1.1248in\" svg:y=\"0in\" "
                                                            "draw:caption-point-x=\"-0.2402in\" draw:caption-point-y=\"0.%1in\"").arg(i == 1 ? QStringLiteral("0039") : i == 2 ? QStringLiteral("1815") : QStringLiteral("3594")))),
                        QStringLiteral("table:style-name=\"ro1\""));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("Sheet1"), columns(3, QStringLiteral("table:style-name=\"co1\"")) + rows), styles)), &wb, &why), qPrintable(why));
        const auto before = notesOf(wb);
        QCOMPARE(before.size(), size_t(3));
        for (quint32 r = 0; r < 3; ++r) {
            const NoteBox &b = before.at(noteKey(r, 0)).box;
            QVERIFY(b.set);
            QCOMPARE(b.row1, -int(r));
            QCOMPARE(b.row2, 1 - int(r));
            QCOMPARE(b.y1, 0);
            QVERIFY(std::abs(b.y2 - 5) <= 1);   // (0.2346 in is 0.0566 in into the second row: 5.4 pixels)
            QCOMPARE(b.col1, 1);
            QCOMPARE(b.col2, 2);
        }
        // saved as .ods
        const QByteArray ods = odsOf(wb, nullptr, &why);
        QVERIFY2(!ods.isEmpty(), qPrintable(why));
        const QString text = QString::fromUtf8(contentOf(ods));
        QRegularExpressionMatchIterator it = QRegularExpression(QStringLiteral("svg:y=\"([^\"]*)pt\"")).globalMatch(text);
        int found = 0;
        while (it.hasNext()) {
            const double y = it.next().captured(1).toDouble();
            QVERIFY2(y >= 0 && y < 1, qPrintable(QString::number(y)));   // (at the sheet's top, as the file had it)
            ++found;
        }
        QCOMPARE(found, 3);
        const WrittenBox box = firstAnnotation(contentOf(ods));
        QVERIFY(box.lengths);
        QVERIFY(std::abs(box.x - 1.1248 * 72) <= 0.8);
        QVERIFY(std::abs(box.w - 1.1413 * 72) <= 1.6);
        QVERIFY(std::abs(box.h - 0.2346 * 72) <= 1.6);
        Workbook back;
        QVERIFY2(openIt(ods, &back, &why), qPrintable(why));
        const auto after = notesOf(back);
        QCOMPARE(after.size(), size_t(3));
        for (quint32 r = 0; r < 3; ++r) QVERIFY2(after.at(noteKey(r, 0)).box == before.at(noteKey(r, 0)).box, "the box is the same after .ods -> .ods");
        // saved as .xlsx
        QByteArray xlsx;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &xlsx, &why), qPrintable(why));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(xlsx, parts));
        const QByteArray vml = parts.value(QStringLiteral("xl/drawings/vmlDrawing1.vml"));
        QCOMPARE(vml.count("<x:Anchor>"), 3);
        QRegularExpressionMatchIterator anchors = QRegularExpression(QStringLiteral("<x:Anchor>([^<]*)</x:Anchor>")).globalMatch(QString::fromLatin1(vml));
        while (anchors.hasNext()) QVERIFY2(!anchors.next().captured(1).contains(QLatin1Char('-')), "an anchor number below zero");
        jp::office::Package package;
        Workbook fromXlsx;
        QVERIFY2(package.readLazy(xlsx, &why) && fromXlsx.open(package, &why), qPrintable(why));
        const auto viaXlsx = notesOf(fromXlsx);
        for (quint32 r = 0; r < 3; ++r) QVERIFY(viaXlsx.at(noteKey(r, 0)).box == before.at(noteKey(r, 0)).box);
    }

    // A box that reaches above the sheet's top (cells counted from its note) is put at the top with its size kept, in the .xlsx form's anchor and in the .ods lengths.
    void aBoxAboveTheSheetIsMovedDownWithItsSizeKept()
    {
        NoteOp op;
        op.row = 1;
        op.col = 1;
        op.author = QStringLiteral("Ann");
        op.text = QStringLiteral("x");
        op.box.set = true;
        op.box.col1 = -2;
        op.box.x1 = 3;
        op.box.row1 = -3;
        op.box.y1 = 5;
        op.box.col2 = 0;
        op.box.x2 = 4;
        op.box.row2 = -1;
        op.box.y2 = 7;
        const QByteArray vml = newNotesDrawing({op});
        const qsizetype at = vml.indexOf("<x:Anchor>");
        QVERIFY(at >= 0);
        const QByteArray numbers = vml.mid(at + 10, vml.indexOf("</x:Anchor>", at) - at - 10);
        QStringList v = QString::fromLatin1(numbers).split(QLatin1Char(','));
        QCOMPARE(v.size(), 8);
        for (QString &x : v) x = x.trimmed();
        // (two cells down and one across: the cells between the corners, and the distances into them, are what they were)
        QCOMPARE(v[2].toInt(), 0);
        QCOMPARE(v[6].toInt() - v[2].toInt(), 2);
        QCOMPARE(v[0].toInt(), 0);
        QCOMPARE(v[4].toInt() - v[0].toInt(), 2);
        QCOMPARE(v[1].toInt(), 3);
        QCOMPARE(v[3].toInt(), 5);
        QCOMPARE(v[5].toInt(), 4);
        QCOMPARE(v[7].toInt(), 7);
        // the .ods lengths of the same box: a box with a size
        odsdraw::Grid grid(QVector<odsdraw::Grid::Run>(), 48, QVector<odsdraw::Grid::Run>(), 15);
        double x = 0, y = 0, w = 0, h = 0;
        QVERIFY(noteBoxToOds(op.box, op.row, op.col, grid, &x, &y, &w, &h));
        QVERIFY(x >= 0 && y >= 0);
        QVERIFY2(std::abs(h - (2 * 15 + (7 - 5) * 0.75)) <= 0.01, qPrintable(QString::number(h)));
        QVERIFY2(std::abs(w - (2 * 48 + (4 - 3) * 0.75)) <= 0.01, qPrintable(QString::number(w)));
    }

    // A sheet read only as far as a row has no boxes computed (its row heights are not all known), as it has no drawing; the notes are there.
    void aSheetReadPartWayHasNotesButNoBoxes()
    {
        const QString note = annotation(QStringLiteral("<text:p>n</text:p>"), QStringLiteral("svg:x=\"4.5cm\" svg:y=\"1.2cm\" svg:width=\"5cm\" svg:height=\"2.5cm\""));
        QString rows = rowOf(notedCell(note));
        for (int i = 0; i < 30; ++i) rows += rowOf(emptyCell() + stringCell(QStringLiteral("t")));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(gridBook(rows), &wb, &why), qPrintable(why));
        Sheet part;
        QVERIFY(wb.readSheet(0, &part, nullptr, 9));
        QVERIFY(part.partial);
        QCOMPARE(part.importedNotes.size(), qsizetype(1));
        QVERIFY(!part.importedNotes[0].box.set);
        Sheet whole;
        QVERIFY(wb.readSheet(0, &whole));
        QVERIFY(whole.importedNotes[0].box.set);
    }

    // Three annotations as real files have them (lengths in points, inches, and centimeters; a style name and a caption point on each; an author and a date; the shown state): values copied from
    // them into files built here. Read as the box in them, saved with the box where it was.
    void annotationsAsRealFilesHaveThemAreReadAndWritten_data()
    {
        QTest::addColumn<QByteArray>("file");
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("y");
        QTest::addColumn<double>("w");
        QTest::addColumn<double>("h");
        QTest::addColumn<QString>("author");
        QTest::addColumn<QString>("date");
        QTest::addColumn<bool>("shown");
        QTest::addColumn<int>("row");
        QTest::addColumn<int>("col");
        QTest::addColumn<QString>("text");
        auto one = [](const QString &colWidth, const QString &rowHeight, int cols, const QString &before, const QString &cellAttrs, const QString &note) {
            const QString styles = QStringLiteral("<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"%1\"/></style:style>"
                                                  "<style:style style:name=\"ro1\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"%2\" style:use-optimal-row-height=\"true\"/></style:style>")
                                       .arg(colWidth, rowHeight);
            return buildOds(contentXml(table(QStringLiteral("Sheet1"), columns(cols, QStringLiteral("table:style-name=\"co1\"")) + before + row(cellAttrs + note, QStringLiteral("table:style-name=\"ro1\""))), styles));
        };
        // points: one cell in the first row, the note's box to its right
        QTest::newRow("points") << one(QStringLiteral("64.01pt"), QStringLiteral("12.81pt"), 4, QString(), QString(),
                                       cell(QString(), annotation(QStringLiteral("<dc:creator>BAKO</dc:creator><dc:date>2016-12-17T00:00:00</dc:date><text:p text:style-name=\"P1\">Komentarz</text:p>"),
                                                                   QStringLiteral("office:display=\"true\" draw:style-name=\"gr1\" draw:text-style-name=\"P2\" svg:width=\"82.18pt\" svg:height=\"26.62pt\" svg:x=\"80.99pt\" svg:y=\"0pt\" "
                                                                                  "draw:caption-point-x=\"-17.29pt\" draw:caption-point-y=\"0.28pt\""))))
                               << 80.99 << 0.0 << 82.18 << 26.62 << QStringLiteral("BAKO") << QStringLiteral("2016-12-17T00:00:00") << true << 0 << 0 << QStringLiteral("Komentarz");
        // inches: the cell G3 (the file's own), two rows above it and six cells before it
        QTest::newRow("inches") << one(QStringLiteral("0.8925in"), QStringLiteral("0.1465in"), 9, rowOf(emptyCell()) + rowOf(emptyCell()),
                                       QStringLiteral("<table:table-cell table:number-columns-repeated=\"6\"/>"),
                                       cell(QString(), annotation(QStringLiteral("<dc:date>2011-09-06T00:00:00</dc:date><text:p text:style-name=\"P1\">Test</text:p>"),
                                                                   QStringLiteral("draw:style-name=\"gr1\" draw:text-style-name=\"P1\" svg:width=\"1.1413in\" svg:height=\"0.2094in\" svg:x=\"7.3484in\" svg:y=\"0in\" "
                                                                                  "draw:caption-point-x=\"-0.2402in\" draw:caption-point-y=\"0.2957in\""))))
                                << 7.3484 * 72 << 0.0 << 1.1413 * 72 << 0.2094 * 72 << QString() << QStringLiteral("2011-09-06T00:00:00") << false << 2 << 6 << QStringLiteral("Test");
        // centimeters: B2
        QTest::newRow("centimeters") << one(QStringLiteral("2.258cm"), QStringLiteral("0.452cm"), 3, rowOf(emptyCell()), emptyCell(),
                                            cell(QStringLiteral("office:value-type=\"string\" calcext:value-type=\"string\""),
                                                 annotation(QStringLiteral("<dc:creator>dd</dc:creator><dc:date>2022-11-25T00:00:00</dc:date><text:p text:style-name=\"P1\"><text:span text:style-name=\"T1\">Test row hide with notes</text:span></text:p>"),
                                                            QStringLiteral("office:display=\"true\" draw:style-name=\"gr1\" draw:text-style-name=\"P2\" svg:width=\"2.899cm\" svg:height=\"1.799cm\" svg:x=\"5.567cm\" svg:y=\"0.357cm\" "
                                                                           "draw:caption-point-x=\"-0.61cm\" draw:caption-point-y=\"0.105cm\"")) + QStringLiteral("<text:p>Cell with a note</text:p>")))
                                    << 5.567 / 2.54 * 72 << 0.357 / 2.54 * 72 << 2.899 / 2.54 * 72 << 1.799 / 2.54 * 72 << QStringLiteral("dd") << QStringLiteral("2022-11-25T00:00:00") << true << 1 << 1 << QStringLiteral("Test row hide with notes");
    }
    void annotationsAsRealFilesHaveThemAreReadAndWritten()
    {
        QFETCH(QByteArray, file);
        QFETCH(double, x);
        QFETCH(double, y);
        QFETCH(double, w);
        QFETCH(double, h);
        QFETCH(QString, author);
        QFETCH(QString, date);
        QFETCH(bool, shown);
        QFETCH(int, row);
        QFETCH(int, col);
        QFETCH(QString, text);
        Workbook wb;
        QString why;
        QVERIFY2(openIt(file, &wb, &why), qPrintable(why));
        const auto notes = notesOf(wb);
        QCOMPARE(notes.size(), size_t(1));
        QVERIFY2(notes.count(noteKey(quint32(row), quint32(col))), "the note is on its cell");
        const NoteInfo &n = notes.at(noteKey(quint32(row), quint32(col)));
        QCOMPARE(n.text, text);
        QCOMPARE(n.author, author);
        QCOMPARE(n.shown, shown);
        QVERIFY(n.box.set);
        const QByteArray ods = odsOf(wb, nullptr, &why);
        QVERIFY2(!ods.isEmpty(), qPrintable(why));
        const WrittenBox box = firstAnnotation(contentOf(ods));
        QVERIFY2(box.lengths, "the box is written as four lengths in points");
        // (measured from the sheet's top left, as the file had them: within a pixel of each)
        QVERIFY2(std::abs(box.x - x) <= 1.6, qPrintable(QString::number(box.x) + " vs " + QString::number(x)));
        QVERIFY2(std::abs(box.y - y) <= 1.6, qPrintable(QString::number(box.y) + " vs " + QString::number(y)));
        QVERIFY2(std::abs(box.w - w) <= 2.4, qPrintable(QString::number(box.w) + " vs " + QString::number(w)));
        QVERIFY2(std::abs(box.h - h) <= 2.4, qPrintable(QString::number(box.h) + " vs " + QString::number(h)));
        QCOMPARE(box.date, date);
        Workbook back;
        QVERIFY2(openIt(ods, &back, &why), qPrintable(why));
        QVERIFY(notesOf(back).at(noteKey(quint32(row), quint32(col))).box == n.box);
    }
};

JP_TEST_CLASS(SheetOdsNotesTests)
#include "tst_sheet_odsnotes.moc"

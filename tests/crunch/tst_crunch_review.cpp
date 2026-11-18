// Tests that came of a review of JeffCrunch's formatting and sheets: the sheets of a workbook that the reader takes and no more, parts of a workbook whose elements have a prefix, a list of
// sheets in a form of its own, the tab that is selected, the formulas inside the rules of conditional formatting and pivot tables that follow a sheet, whole columns of many cells given
// a format, and a workbook that is full of formats. Every workbook is built here.
#include "app/crunchwindow.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "app/widgets.h"
#include "io/zip.h"
#include "savedok.h"
#include "core/editcomments.h"
#include "sheet/threads.h"
#include "../pub/threadscript.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QRegularExpression>
#include <QTimer>
#include <QSignalSpy>
#include <QUndoStack>
#include <QXmlStreamReader>

#include <functional>

#include "../localepin.h"
#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
using namespace jp::office::sheet;

namespace {

QString cellXml(const QString &ref, const QString &value) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value); }
QString formulaXml(const QString &ref, const QString &text) { return QStringLiteral("<c r=\"%1\"><f>%2</f><v>0</v></c>").arg(ref, text.toHtmlEscaped()); }
QString rowXml(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

// The styles of a small workbook with room for formats: a font, two fills, a border, and a first cell format.
QByteArray reviewStyles()
{
    const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/></font></fonts>"
                                         "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
    return stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"), lists);
}

//   Data:  A1 = 1, A2 = 2, B1 = "hello", C1 = Other!A1 + 1       Other: A1 = 10
QByteArray reviewBook(const QString &dataAfter = QString())
{
    const QByteArray data = worksheet(rowXml(1, cellXml(QStringLiteral("A1"), QStringLiteral("1")) + QStringLiteral("<c r=\"B1\" t=\"s\"><v>0</v></c>") + formulaXml(QStringLiteral("C1"), QStringLiteral("Other!A1+1")))
                                          + rowXml(2, cellXml(QStringLiteral("A2"), QStringLiteral("2"))),
                                      dataAfter);
    const QByteArray other = worksheet(rowXml(1, cellXml(QStringLiteral("A1"), QStringLiteral("10"))));
    return buildXlsx({data, other}, sharedStringsXml({QStringLiteral("<t>hello</t>")}), QString(), QString(), reviewStyles(), {QStringLiteral("Data"), QStringLiteral("Other")});
}

// The workbook with its parts changed by `fn` (the parts are named as the package names them: "xl/workbook.xml").
QByteArray edited(const QByteArray &xlsx, const std::function<void(QMap<QString, QByteArray> &)> &fn)
{
    QMap<QString, QByteArray> parts;
    jp::readZip(xlsx, parts);
    fn(parts);
    jp::ZipWriter z;
    for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

// The XML with a prefix on every element of the main namespace (and the namespace declared for the prefix): what a file written by a program that does that looks like.
QByteArray withPrefix(const QByteArray &xml, const QString &prefix)
{
    QString text = QString::fromUtf8(xml);
    static const QRegularExpression open(QStringLiteral("<(/?)([A-Za-z])"));
    text.replace(open, QStringLiteral("<\\1") + prefix + QStringLiteral(":\\2"));
    text.replace(QStringLiteral(" xmlns=\""), QStringLiteral(" xmlns:") + prefix + QStringLiteral("=\""));
    return text.toUtf8();
}

bool wellFormed(const QByteArray &xml, QString *why = nullptr)
{
    QXmlStreamReader r(xml);
    while (!r.atEnd()) r.readNext();
    if (r.hasError() && why) *why = r.errorString();
    return !r.hasError();
}

// A sheet that has what the program keeps no model of: it is protected, filtered, outlined (rows and a column), has links (one to a web page, one to a cell), is set up for printing, has
// a break between pages, and a data bar that is filled flat, bordered, and has its axis in the middle.
//   A: header, B: numbers (data bar), C: text; rows 2 to 4 are in groups, the third hidden and the fourth collapsed
const char *kX14 = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main";
QByteArray richSheet(bool protectedToo = true)
{
    const QString head = QStringLiteral("<sheetPr><outlinePr summaryBelow=\"0\"/><pageSetUpPr fitToPage=\"1\"/></sheetPr><sheetFormatPr defaultRowHeight=\"15\" outlineLevelRow=\"2\" outlineLevelCol=\"1\"/>"
                                        "<cols><col min=\"2\" max=\"3\" width=\"10\" customWidth=\"1\" outlineLevel=\"1\" collapsed=\"1\"/></cols>");
    QString rows = rowXml(1, cellXml(QStringLiteral("A1"), QStringLiteral("1")) + cellXml(QStringLiteral("B1"), QStringLiteral("2")) + cellXml(QStringLiteral("C1"), QStringLiteral("3")));
    rows += QStringLiteral("<row r=\"2\" outlineLevel=\"1\"><c r=\"A2\"><v>5</v></c><c r=\"B2\"><v>10</v></c></row>");
    rows += QStringLiteral("<row r=\"3\" outlineLevel=\"2\" hidden=\"1\"><c r=\"A3\"><v>6</v></c><c r=\"B3\"><v>20</v></c></row>");
    rows += QStringLiteral("<row r=\"4\" collapsed=\"1\"><c r=\"A4\"><v>7</v></c><c r=\"B4\"><v>30</v></c></row>");
    rows += rowXml(5, cellXml(QStringLiteral("B5"), QStringLiteral("40"))) + rowXml(6, cellXml(QStringLiteral("B6"), QStringLiteral("50")));
    const QString after = QStringLiteral(
        "%2<autoFilter ref=\"A1:C6\"/>"
        "<conditionalFormatting sqref=\"B2:B6\"><cfRule type=\"dataBar\" priority=\"1\"><dataBar><cfvo type=\"min\"/><cfvo type=\"max\"/><color rgb=\"FF638EC6\"/></dataBar>"
        "<extLst><ext xmlns:x14=\"%1\" uri=\"{B025F937-C7B1-47D3-B67F-A62EFF666E3E}\"><x14:id>{AAAAAAAA-0000-4000-8000-000000000001}</x14:id></ext></extLst></cfRule></conditionalFormatting>"
        "<hyperlinks><hyperlink ref=\"A2\" r:id=\"rIdH1\" display=\"site\"/><hyperlink ref=\"B2\" location=\"Other!A1\" display=\"to Other\"/></hyperlinks>"
        "<printOptions gridLines=\"1\"/>"
        "<pageMargins left=\"1\" right=\"1\" top=\"1\" bottom=\"1\" header=\"0.5\" footer=\"0.5\"/>"
        "<pageSetup orientation=\"landscape\" paperSize=\"9\" r:id=\"rIdPS\"/>"
        "<headerFooter><oddHeader>&amp;CReport</oddHeader></headerFooter>"
        "<rowBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"3\" max=\"16383\" man=\"1\"/></rowBreaks>"
        "<extLst><ext xmlns:x14=\"%1\" uri=\"{78C0D931-6437-407d-A8EE-F0AAD7539E65}\"><x14:conditionalFormattings><x14:conditionalFormatting xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\">"
        "<x14:cfRule type=\"dataBar\" id=\"{AAAAAAAA-0000-4000-8000-000000000001}\"><x14:dataBar minLength=\"0\" maxLength=\"100\" gradient=\"0\" border=\"1\" axisPosition=\"middle\">"
        "<x14:cfvo type=\"autoMin\"/><x14:cfvo type=\"autoMax\"/><x14:borderColor rgb=\"FF638EC6\"/><x14:axisColor rgb=\"FF000000\"/></x14:dataBar></x14:cfRule><xm:sqref>B2:B6</xm:sqref>"
        "</x14:conditionalFormatting></x14:conditionalFormattings></ext></extLst>")
                              .arg(QString::fromLatin1(kX14), protectedToo ? QStringLiteral("<sheetProtection sheet=\"1\" objects=\"1\" scenarios=\"1\" password=\"CC1A\"/>") : QString());
    return worksheet(rows, after, head);
}

//   Rich: the sheet above     Other: A1 = 10
QByteArray richBook(bool protectedToo = true)
{
    const QByteArray other = worksheet(rowXml(1, cellXml(QStringLiteral("A1"), QStringLiteral("10"))));
    return edited(buildXlsx({richSheet(protectedToo), other}, QByteArray(), QString(), QString(), reviewStyles(), {QStringLiteral("Rich"), QStringLiteral("Other")}), [](QMap<QString, QByteArray> &parts) {
        parts[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")] =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
            "<Relationship Id=\"rIdH1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink\" Target=\"https://example.org/data\" TargetMode=\"External\"/>"
            "<Relationship Id=\"rIdPS\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/printerSettings\" Target=\"../printerSettings/printerSettings1.bin\"/></Relationships>";
        parts[QStringLiteral("xl/printerSettings/printerSettings1.bin")] = QByteArray(64, '\1');
        QString types = QString::fromUtf8(parts[QStringLiteral("[Content_Types].xml")]);
        if (!types.contains(QLatin1String("Extension=\"bin\"")))
            types.replace(QStringLiteral("<Override"), QStringLiteral("<Default Extension=\"bin\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.printerSettings\"/><Override"));
        parts[QStringLiteral("[Content_Types].xml")] = types.toUtf8();
    });
}

// A saved workbook, read again.
struct Saved {
    Package package;
    Workbook book;
    bool open(const QString &path)
    {
        QString why;
        return package.readLazyFile(path, &why) && book.open(package, &why);
    }
    QStringList names() const
    {
        QStringList out;
        for (const SheetEntry &e : book.info().sheets) out << e.name;
        return out;
    }
    QString part(const QString &name) const { return QString::fromUtf8(package.part(name.startsWith(QLatin1Char('/')) ? name : QStringLiteral("/") + name)); }
    QString sheetPart(int place) const { return part(book.info().sheets[place].part); }
};

bool saveNow(EditWindow &e)
{
    if (!e.settle()) return false;
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    if (!e.win.w.save()) return false;
    return saved.wait(60000) && saved.last().value(1).toBool();
}

// The format of a cell of the grid, as the grid draws it, and the cell's format.
const grid::CellLook &lookAt(EditWindow &e, quint32 r, quint32 c)
{
    const grid::GridSheet &g = e.view()->sheet();
    return g.looks().look(g.styleOf(r, c, g.cellAt(r, c)));
}
quint32 styleAt(EditWindow &e, quint32 r, quint32 c)
{
    const grid::GridSheet &g = e.view()->sheet();
    return g.styleOf(r, c, g.cellAt(r, c));
}

// The threads of the first sheet as the shared thread script (tests/pub/threadscript.h) sees them: the n-th thread is the n-th cell that holds one, in the order of the sheet.
struct CrunchThreads {
    static constexpr bool sharedThreadCap = false;   // (its own limit on a workbook's comments instead)
    crunch::EditSession *s;
    explicit CrunchThreads(crunch::EditSession *session) : s(session) {}
    QString who = QStringLiteral("Zed Ypsilon");
    std::vector<std::pair<quint32, quint32>> cells() const
    {
        std::vector<std::pair<quint32, quint32>> out;
        for (const auto &n : s->anchors().notes(0))
            if (n.second.thread && !n.second.thread->empty()) out.push_back({quint32(n.first >> 32), quint32(n.first & 0xffffffffu)});
        return out;
    }
    const NoteInfo *at(int t) const
    {
        const auto c = cells();
        return t >= 0 && size_t(t) < c.size() ? s->anchors().noteAt(0, c[size_t(t)].first, c[size_t(t)].second) : nullptr;
    }
    bool start(const QString &text) { return s->newComment(0, quint32(10 + 2 * cells().size()), 0, text, who, QStringLiteral("New Comment")).ok; }
    // A reply to comment e goes into the thread, last, and is a reply of the first comment (a reply to a reply is not nested).
    bool reply(int t, int e, const QString &text)
    {
        const auto c = cells();
        if (t < 0 || size_t(t) >= c.size() || e < 0 || e >= entries(t)) return false;
        const size_t before = at(t)->thread->size();
        if (!s->replyToComment(0, c[size_t(t)].first, c[size_t(t)].second, text, who, QStringLiteral("Reply")).ok) return false;
        const jp::office::sheet::ThreadList &list = *at(t)->thread;
        return list.size() == before + 1 && list.back().text == text && list.back().parentId == list.front().id;
    }
    bool edit(int t, int e, const QString &text)
    {
        const auto c = cells();
        return s->editComment(0, c[size_t(t)].first, c[size_t(t)].second, e, text, QStringLiteral("Edit Comment")).ok;
    }
    bool resolve(int t, bool done)
    {
        const auto c = cells();
        return s->resolveThread(0, c[size_t(t)].first, c[size_t(t)].second, done, QStringLiteral("Resolve Thread")).ok;
    }
    bool remove(int t, int e)
    {
        const auto c = cells();
        return s->deleteComment(0, c[size_t(t)].first, c[size_t(t)].second, e, QStringLiteral("Delete Comment")).ok;
    }
    void removeAll() { s->deleteThreads(0, cells(), QStringLiteral("Delete Comments")); }
    QStringList state() const
    {
        QStringList out;
        for (size_t t = 0; t < cells().size(); ++t) {
            QStringList words;
            for (const ThreadComment &c : *at(int(t))->thread) words << c.text;
            out << words.join(QLatin1String(" | ")) + (jp::office::sheet::threadResolved(*at(int(t))->thread) ? QStringLiteral(" [resolved]") : QString());
        }
        return out;
    }
    QString author(int t, int e) const { return at(t)->thread->at(size_t(e)).author; }
    int step(int t, bool forward) const
    {
        const auto c = cells();
        int sheet = 0;
        quint32 row = 0, col = 0;
        if (!crunch::stepThread(s->anchors(), 0, c[size_t(t)].first, c[size_t(t)].second, forward, &sheet, &row, &col)) return -1;
        for (size_t i = 0; i < c.size(); ++i)
            if (c[i].first == row && c[i].second == col) return int(i);
        return -1;
    }
    int entries(int t) const { return int(at(t)->thread->size()); }
    int threadCount() const { return int(cells().size()); }
};

// The bytes of the parts of a saved workbook whose names hold `word`, by name.
QMap<QString, QByteArray> partsNamed(const QString &path, const QString &word)
{
    QMap<QString, QByteArray> all, out;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly) || !jp::readZip(f.readAll(), all)) return out;
    for (auto it = all.begin(); it != all.end(); ++it)
        if (it.key().contains(word)) out.insert(it.key(), it.value());
    return out;
}

// A workbook as another program writes it: legacy notes (two of its own, and the stand-ins of the two threads), the threads (one whose last reply answers a reply), the persons part, and
// an extension list on the sheet. `moreReplies` makes the first thread that long.
QByteArray foreignBook(int moreReplies = 0)
{
    const char *kLegacy =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>Ann</author><author>Bob</author>"
        "<author>tc={11111111-2222-3333-4444-555555555555}</author><author>tc={66666666-2222-3333-4444-555555555555}</author></authors><commentList>"
        "<comment ref=\"A1\" authorId=\"0\" shapeId=\"0\"><text><t>Ann says check the total</t></text></comment>"
        "<comment ref=\"C3\" authorId=\"1\" shapeId=\"0\"><text><t>Bob says hi</t></text></comment>"
        "<comment ref=\"D2\" authorId=\"2\" shapeId=\"0\"><text><t>[Threaded comment] fallback</t></text></comment>"
        "<comment ref=\"B3\" authorId=\"3\" shapeId=\"0\"><text><t>[Threaded comment] fallback</t></text></comment></commentList></comments>";
    const char *kVmlOf =
        "<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\">"
        "<v:shapetype id=\"_x0000_t202\" coordsize=\"21600,21600\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\"><v:stroke joinstyle=\"miter\"/><v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/></v:shapetype>"
        "<v:shape id=\"_x0000_s1025\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:1;visibility:hidden\"><x:ClientData ObjectType=\"Note\"><x:Anchor>1, 15, 0, 2, 3, 15, 3, 16</x:Anchor><x:Row>0</x:Row><x:Column>0</x:Column></x:ClientData></v:shape>"
        "<v:shape id=\"_x0000_s1026\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:2;visibility:hidden\"><x:ClientData ObjectType=\"Note\"><x:Anchor>3, 15, 1, 2, 5, 15, 4, 16</x:Anchor><x:Row>2</x:Row><x:Column>2</x:Column></x:ClientData></v:shape>"
        "<v:shape id=\"_x0000_s1027\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:3;visibility:hidden\"><x:ClientData ObjectType=\"Note\"><x:Anchor>2, 15, 3, 2, 4, 15, 6, 16</x:Anchor><x:Row>1</x:Row><x:Column>3</x:Column></x:ClientData></v:shape>"
        "<v:shape id=\"_x0000_s1028\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:4;visibility:hidden\"><x:ClientData ObjectType=\"Note\"><x:Anchor>2, 15, 3, 2, 4, 15, 6, 16</x:Anchor><x:Row>2</x:Row><x:Column>1</x:Column></x:ClientData></v:shape></xml>";
    QByteArray threads = "<ThreadedComments xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\">"
                         "<threadedComment ref=\"D2\" dT=\"2024-01-01T10:00:00.00\" personId=\"{AAAA}\" id=\"{11111111-2222-3333-4444-555555555555}\"><text>Q1</text></threadedComment>"
                         "<threadedComment ref=\"D2\" dT=\"2024-01-01T10:01:00.00\" personId=\"{AAAA}\" id=\"{R1}\" parentId=\"{11111111-2222-3333-4444-555555555555}\"><text>R1</text></threadedComment>"
                         "<threadedComment ref=\"D2\" dT=\"2024-01-01T10:02:00.00\" personId=\"{AAAA}\" id=\"{R2}\" parentId=\"{R1}\"><text>R2</text></threadedComment>";
    for (int i = 0; i < moreReplies; ++i)
        threads += "<threadedComment ref=\"D2\" dT=\"2024-01-01T10:03:00.00\" personId=\"{AAAA}\" id=\"{M" + QByteArray::number(i) + "}\" parentId=\"{11111111-2222-3333-4444-555555555555}\"><text>more " + QByteArray::number(i) + "</text></threadedComment>";
    threads += "<threadedComment ref=\"B3\" dT=\"2024-01-02T10:00:00.00\" personId=\"{AAAA}\" id=\"{66666666-2222-3333-4444-555555555555}\"><text>Q2</text></threadedComment></ThreadedComments>";
    const char *kPersons = "<personList xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\" xmlns:x=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
                           "<person displayName=\"Ann Lee\" id=\"{AAAA}\" userId=\"ann@example.test\" providerId=\"None\"/></personList>";
    QVector<ExtraPart> parts;
    parts.push_back({QStringLiteral("xl/comments1.xml"), QByteArray(kLegacy), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml"), 0, QStringLiteral("rId11"), QStringLiteral("comments"), QStringLiteral("../comments1.xml")});
    parts.push_back({QStringLiteral("xl/drawings/vmlDrawing1.vml"), QByteArray(kVmlOf), QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"), 0, QStringLiteral("rId12"), QStringLiteral("vmlDrawing"), QStringLiteral("../drawings/vmlDrawing1.vml")});
    parts.push_back({QStringLiteral("xl/threadedComments/threadedComment1.xml"), threads, QStringLiteral("application/vnd.ms-excel.threadedcomments+xml"), 0, QStringLiteral("rId13"), QStringLiteral("threadedComment"), QStringLiteral("../threadedComments/threadedComment1.xml")});
    parts.push_back({QStringLiteral("xl/persons/person.xml"), QByteArray(kPersons), QStringLiteral("application/vnd.ms-excel.person+xml"), -1, QString(), QString(), QString()});
    QMap<QString, QByteArray> all;
    jp::readZip(withParts(editBook(), parts), all);
    QByteArray sheetRels = all.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels"));
    sheetRels.replace("http://schemas.openxmlformats.org/officeDocument/2006/relationships/threadedComment", "http://schemas.microsoft.com/office/2017/10/relationships/threadedComment");
    all[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")] = sheetRels;
    QByteArray bookRels = all.value(QStringLiteral("xl/_rels/workbook.xml.rels"));
    bookRels.replace("</Relationships>", "<Relationship Id=\"rIdPerson\" Type=\"http://schemas.microsoft.com/office/2017/10/relationships/person\" Target=\"persons/person.xml\"/></Relationships>");
    all[QStringLiteral("xl/_rels/workbook.xml.rels")] = bookRels;
    QByteArray sheet = all.value(QStringLiteral("xl/worksheets/sheet1.xml"));
    sheet.replace("</worksheet>", "<legacyDrawing r:id=\"rId12\"/><extLst><ext uri=\"{05C60535-1F16-4fd2-B633-F4F36F0B64E0}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\"><x14:sparklineGroups/></ext></extLst></worksheet>");
    all[QStringLiteral("xl/worksheets/sheet1.xml")] = sheet;
    jp::ZipWriter z;
    for (auto it = all.begin(); it != all.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

} // namespace

class CrunchReviewTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // The program does not write a file it would not read: a workbook of as many sheets as the reader takes (1,000) is given no more by Insert or by Copy, with a sentence that says why;
    // one sheet deleted makes room for one; and the workbook with them all is saved, and read again.
    void aWorkbookIsGivenNoMoreSheetsThanTheReaderTakes()
    {
        const PinnedLocale english;
        QVector<QByteArray> sheets;
        for (int i = 0; i < 999; ++i) sheets.append(worksheet(rowXml(1, cellXml(QStringLiteral("A1"), QString::number(i)))));
        EditWindow e;
        QVERIFY(e.open(buildXlsx(sheets, QByteArray(), QString(), QString(), reviewStyles())));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(e.win.w.calculation()->sheetCount(), 999);
        QVERIFY(s->insertSheet(-1, QString()).ok);
        QCOMPARE(e.win.w.calculation()->sheetCount(), 1000);
        const crunch::EditSession::Outcome more = s->insertSheet(-1, QString());
        QVERIFY(!more.ok);
        QVERIFY2(more.problem.contains(QStringLiteral("1000")), qPrintable(more.problem));
        const crunch::EditSession::Outcome copy = s->copySheet(0, -1, QString());
        QVERIFY(!copy.ok);
        QCOMPARE(copy.problem, more.problem);
        QCOMPARE(e.win.w.calculation()->sheetCount(), 1000);
        // one deleted: room for one
        QVERIFY(s->deleteSheet(5).ok);
        QVERIFY(s->insertSheet(-1, QString()).ok);
        QVERIFY(!s->insertSheet(-1, QString()).ok);
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        QCOMPARE(f.book.info().sheets.size(), 1000);
    }

    // A part whose elements all have a prefix ("x:worksheet", "x:styleSheet") is changed in the form it has: a format, a font, a fill, a border, and a number format added, a text typed, a
    // row inserted, and a sheet added, renamed, colored, and hidden leave a saved file whose every part is well formed, and has what was done.
    void everyPartThatTheSaverChangesMayHaveAPrefix()
    {
        const QByteArray book = edited(reviewBook(), [](QMap<QString, QByteArray> &parts) {
            for (const char *name : {"xl/workbook.xml", "xl/styles.xml", "xl/sharedStrings.xml", "xl/worksheets/sheet1.xml", "xl/worksheets/sheet2.xml"})
                parts[QString::fromLatin1(name)] = withPrefix(parts[QString::fromLatin1(name)], QStringLiteral("x"));
        });
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QVERIFY(s->type(0, 1, 1, QStringLiteral("a new text")).ok);
        FormatChange change;
        change.fontName = QStringLiteral("Georgia");
        change.fontSize = 14;
        change.bold = true;
        FillSpec fill;
        fill.pattern = PatternKind::Solid;
        fill.fg = ColorSpec::fromRgb(0xFFEB9C);
        change.fill = fill;
        BorderSide line;
        line.style = BorderStyle::Thin;
        line.color = ColorSpec::fromRgb(0x0000FF);
        change.left = line;
        change.numberFormat = QStringLiteral("0.000");
        QVERIFY(s->formatCells(0, rangeOf(QStringLiteral("A1:A2")), change, QStringLiteral("Formatting")).ok);
        QVERIFY(s->insertRows(0, 0, 1).ok);
        QVERIFY(s->insertSheet(-1, QStringLiteral("Added")).ok);
        QVERIFY(s->renameSheet(1, QStringLiteral("Renamed")).ok);
        QVERIFY(s->setTabColor(0, ColorSpec::fromRgb(0x00B050)).ok);
        QVERIFY(s->setSheetHidden(1, true).ok);
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        QCOMPARE(f.names(), QStringList({QStringLiteral("Data"), QStringLiteral("Renamed"), QStringLiteral("Added")}));
        QCOMPARE(f.book.info().sheets[1].state, SheetState::Hidden);
        // every part of the file is well formed, and none has a doubled colon
        QFile file(e.path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(file.readAll(), parts));
        int xmlParts = 0;
        for (auto it = parts.begin(); it != parts.end(); ++it) {
            if (!it.key().endsWith(QLatin1String(".xml")) && !it.key().endsWith(QLatin1String(".rels"))) continue;
            ++xmlParts;
            QString why;
            QVERIFY2(wellFormed(it.value(), &why), qPrintable(it.key() + QStringLiteral(": ") + why));
            QVERIFY2(!it.value().contains("::"), qPrintable(it.key()));
        }
        QVERIFY(xmlParts >= 7);
        // what was done is there: the formats, the text, the rows, the tab color
        const QString styles = f.part(QStringLiteral("xl/styles.xml"));
        QVERIFY2(styles.contains(QStringLiteral("Georgia")) && styles.contains(QStringLiteral("0.000")) && styles.contains(QStringLiteral("FFEB9C")), qPrintable(styles));
        Sheet data;
        QVERIFY(f.book.readSheet(0, &data));
        const Cell *a2 = at(data, QStringLiteral("A2"));
        QVERIFY(a2 && a2->style != 0);
        QVERIFY(f.sheetPart(0).contains(QStringLiteral("00B050")));
        QVERIFY(f.sheetPart(0).contains(QStringLiteral("a new text")) || f.part(QStringLiteral("xl/sharedStrings.xml")).contains(QStringLiteral("a new text")));
    }

    // A list of sheets in a form of its own is saved: a sheet element with no relationship (a sheet of macros, say), the relationships' attribute written with a prefix that is not "r", and
    // another attribute that ends in ":id" in another namespace. A sheet command (a tab color is enough) did not save such a workbook; the elements are kept as they were, and the sheet
    // that is added has an element made in the way the part writes them.
    void aListOfSheetsInAFormOfItsOwnIsSaved()
    {
        const QByteArray book = edited(reviewBook(), [](QMap<QString, QByteArray> &parts) {
            QString wb = QString::fromUtf8(parts[QStringLiteral("xl/workbook.xml")]);
            wb.replace(QStringLiteral("xmlns:r=\""), QStringLiteral("xmlns:q=\""));
            wb.replace(QStringLiteral("r:id="), QStringLiteral("q:id="));
            wb.replace(QStringLiteral("<sheet name=\"Data\""), QStringLiteral("<sheet xmlns:z=\"urn:other\" z:id=\"decoy\" name=\"Data\""));
            wb.replace(QStringLiteral("</sheets>"), QStringLiteral("<sheet name=\"Macros\" sheetId=\"9\" state=\"veryHidden\"/></sheets>"));
            parts[QStringLiteral("xl/workbook.xml")] = wb.toUtf8();
        });
        EditWindow e;
        QVERIFY(e.open(book));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.win.w.calculation()->sheetCount() >= 2);
        QVERIFY(s->setTabColor(1, ColorSpec::fromRgb(0xC00000)).ok);
        QVERIFY(s->insertSheet(-1, QStringLiteral("Added")).ok);
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        const QString wb = f.part(QStringLiteral("xl/workbook.xml"));
        QVERIFY2(wb.contains(QStringLiteral("name=\"Macros\"")) && wb.contains(QStringLiteral("veryHidden")), qPrintable(wb));
        QVERIFY2(wb.contains(QStringLiteral("z:id=\"decoy\"")), qPrintable(wb));
        int added = -1, other = -1;
        for (int i = 0; i < f.book.info().sheets.size(); ++i) {
            if (f.book.info().sheets[i].name == QLatin1String("Added")) added = i;
            if (f.book.info().sheets[i].name == QLatin1String("Other")) other = i;
        }
        QVERIFY(added >= 0 && other >= 0);
        QVERIFY2(!f.book.info().sheets[added].part.isEmpty(), "the added sheet has a part");
        // the new sheet's element uses the prefix the part declares for the relationships
        QVERIFY2(QRegularExpression(QStringLiteral("<sheet name=\"Added\"[^>]*\\bq:id=\"rId")).match(wb).hasMatch(), qPrintable(wb));
        QVERIFY(f.sheetPart(other).contains(QStringLiteral("C00000")));
    }

    // The sheet that is open: when it is hidden the workbook's open tab moves to a sheet that is shown, and the tab selected moves with it (a hidden sheet is not selected, and the one that is
    // open is).
    void theSelectedTabFollowsTheOpenSheet()
    {
        const QByteArray one = worksheet(rowXml(1, cellXml(QStringLiteral("A1"), QStringLiteral("1"))), QString(), QStringLiteral("<sheetViews><sheetView tabSelected=\"1\" workbookViewId=\"0\"/></sheetViews>"));
        const QByteArray two = worksheet(rowXml(1, cellXml(QStringLiteral("A1"), QStringLiteral("10"))));
        EditWindow e;
        QVERIFY(e.open(buildXlsx({one, two}, QByteArray(), QString(), QString(), reviewStyles(), QStringList(), QString(), QByteArray(), QStringLiteral("<workbookView activeTab=\"0\"/>"))));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.session()->setSheetHidden(0, true).ok);
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        const QString wb = f.part(QStringLiteral("xl/workbook.xml"));
        QVERIFY2(wb.contains(QStringLiteral("activeTab=\"1\"")), qPrintable(wb));
        QVERIFY2(!f.sheetPart(0).contains(QStringLiteral("tabSelected=\"1\"")), qPrintable(f.sheetPart(0).left(500)));
        QVERIFY2(f.sheetPart(1).contains(QStringLiteral("tabSelected=\"1\"")), qPrintable(f.sheetPart(1).left(500)));
        // shown again: the open tab and the selected one stay where they are
        QVERIFY(e.win.w.session()->setSheetHidden(0, false).ok);
        QVERIFY(saveNow(e));
        Saved g;
        QVERIFY(g.open(e.path));
        QVERIFY(!g.sheetPart(0).contains(QStringLiteral("tabSelected=\"1\"")));
        QVERIFY(g.sheetPart(1).contains(QStringLiteral("tabSelected=\"1\"")));
    }

    // The value of a color scale or a data bar that is a formula follows the cells it names when rows are inserted and a sheet it names is renamed, in the file saved, as the model has it.
    void theValuesOfARuleThatAreFormulasFollowTheCells()
    {
        const QString rules = QStringLiteral(
            "<conditionalFormatting sqref=\"A1:A2\"><cfRule type=\"colorScale\" priority=\"1\"><colorScale><cfvo type=\"formula\" val=\"$H$1\"/><cfvo type=\"formula\" val=\"Other!$A$1\"/><color rgb=\"FFFF0000\"/><color rgb=\"FF00FF00\"/></colorScale></cfRule></conditionalFormatting>"
            "<conditionalFormatting sqref=\"B1:B2\"><cfRule type=\"dataBar\" priority=\"2\"><dataBar><cfvo type=\"num\" val=\"3\"/><cfvo type=\"formula\" val=\"$H$4\"/><color rgb=\"FF638EC6\"/></dataBar></cfRule></conditionalFormatting>");
        EditWindow e;
        QVERIFY(e.open(reviewBook(rules)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(0, 0, 2).ok);
        QVERIFY(s->renameSheet(1, QStringLiteral("Q 1")).ok);
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        const QString part = f.sheetPart(0);
        QVERIFY2(part.contains(QStringLiteral("val=\"$H$3\"")) && part.contains(QStringLiteral("val=\"$H$6\"")), qPrintable(part));
        QVERIFY2(part.contains(QStringLiteral("val=\"'Q 1'!$A$1\"")), qPrintable(part));
        QVERIFY2(part.contains(QStringLiteral("val=\"3\"")), qPrintable(part));   // (a number is a number)
        QVERIFY2(part.contains(QStringLiteral("sqref=\"A3:A4\"")), qPrintable(part));
        // the model reads the same
        Sheet data;
        QVERIFY(f.book.readSheet(0, &data));
        QVERIFY(data.conditionalFormats.size() >= 2);
        QCOMPARE(data.conditionalFormats[0].values[0].value, QStringLiteral("$H$3"));
        QCOMPARE(data.conditionalFormats[0].values[1].value, QStringLiteral("'Q 1'!$A$1"));
    }

    // A pivot table's cache names the sheet it was made from: renamed, the cache names it by its new name; cells inserted above the source move its range; a source sheet that is deleted is
    // left as it is (the cache keeps its records and the name of the sheet it had, as the programs people know leave it, and nothing else of the file changes).
    void aPivotCacheFollowsItsSourceSheet()
    {
        const QString cache = QStringLiteral("<?xml version=\"1.0\"?><pivotCacheDefinition xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" refreshOnLoad=\"0\">"
                                             "<cacheSource type=\"worksheet\"><worksheetSource ref=\"A1:C5\" sheet=\"Data\"/></cacheSource><cacheFields count=\"0\"/></pivotCacheDefinition>");
        const QString partName = QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml");
        auto make = [&] {
            return edited(reviewBook(), [&](QMap<QString, QByteArray> &parts) {
                parts[partName] = cache.toUtf8();
                QString types = QString::fromUtf8(parts[QStringLiteral("[Content_Types].xml")]);
                types.replace(QStringLiteral("</Types>"), QStringLiteral("<Override PartName=\"/xl/pivotCache/pivotCacheDefinition1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheDefinition+xml\"/></Types>"));
                parts[QStringLiteral("[Content_Types].xml")] = types.toUtf8();
                QString rels = QString::fromUtf8(parts[QStringLiteral("xl/_rels/workbook.xml.rels")]);
                rels.replace(QStringLiteral("</Relationships>"), QStringLiteral("<Relationship Id=\"rIdP1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition\" Target=\"pivotCache/pivotCacheDefinition1.xml\"/></Relationships>"));
                parts[QStringLiteral("xl/_rels/workbook.xml.rels")] = rels.toUtf8();
                QString wb = QString::fromUtf8(parts[QStringLiteral("xl/workbook.xml")]);
                wb.replace(QStringLiteral("</sheets>"), QStringLiteral("</sheets><pivotCaches><pivotCache cacheId=\"1\" r:id=\"rIdP1\"/></pivotCaches>"));
                parts[QStringLiteral("xl/workbook.xml")] = wb.toUtf8();
            });
        };
        {
            EditWindow e;
            QVERIFY(e.open(make()));
            QVERIFY(e.settle());
            QVERIFY(e.win.w.session()->renameSheet(0, QStringLiteral("Sales 2026")).ok);
            QVERIFY(e.win.w.session()->insertRows(0, 0, 2).ok);
            QVERIFY(saveNow(e));
            Saved f;
            QVERIFY(f.open(e.path));
            const QString got = f.part(partName);
            QVERIFY2(got.contains(QStringLiteral("sheet=\"Sales 2026\"")) && got.contains(QStringLiteral("ref=\"A3:C7\"")), qPrintable(got));
        }
        {
            EditWindow e;
            QVERIFY(e.open(make()));
            QVERIFY(e.settle());
            QVERIFY(e.win.w.session()->deleteSheet(0).ok);
            QVERIFY(saveNow(e));
            Saved f;
            QVERIFY(f.open(e.path));
            QCOMPARE(f.part(partName), cache);   // (as it was, byte for byte)
            QCOMPARE(f.names(), QStringList({QStringLiteral("Other")}));
        }
        {
            // the source deleted, and another sheet given its name, and then another: the cache is not made to follow either
            EditWindow e;
            QVERIFY(e.open(make()));
            QVERIFY(e.settle());
            QVERIFY(e.win.w.session()->deleteSheet(0).ok);
            QVERIFY(e.win.w.session()->renameSheet(1, QStringLiteral("Data")).ok);
            QVERIFY(e.win.w.session()->renameSheet(1, QStringLiteral("Elsewhere")).ok);
            QVERIFY(saveNow(e));
            Saved f;
            QVERIFY(f.open(e.path));
            QCOMPARE(f.part(partName), cache);
        }
    }

    // A whole column is given a format whatever it holds: its cells that are there (a hundred and fifty thousand of them) take it, the column's own format is set for the cells that are not,
    // and it is one step. Clearing the formats of more than a hundred thousand cells clears them all (it did the first hundred thousand, and said nothing).
    void wholeColumnsAndLongBlocksTakeAFormatOrClearIt()
    {
        QString rows;
        for (int i = 1; i <= 150000; ++i) rows += rowXml(i, cellXml(QStringLiteral("A%1").arg(i), QString::number(i)));
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), reviewStyles())));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        FormatChange bold;
        bold.bold = true;
        const int steps = e.win.w.undoStack()->count();
        const crunch::EditSession::Outcome o = s->formatCells(0, CellRange{0, 0, 1048575, 0, true}, bold, QStringLiteral("Formatting"));
        QVERIFY2(o.ok, qPrintable(o.problem));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        const quint32 style = styleAt(e, 0, 0);
        QVERIFY(style != 0);
        for (const quint32 r : {quint32(0), quint32(99999), quint32(100000), quint32(149999)}) {
            QCOMPARE(styleAt(e, r, 0), style);
            QVERIFY(lookAt(e, r, 0).bold);
        }
        QVERIFY(lookAt(e, 200000, 0).bold);   // (a cell that is not there takes the column's)
        QVERIFY(!lookAt(e, 5, 1).bold);
        // saved: the cells are in the file with the format
        QVERIFY(saveNow(e));
        {
            Saved f;
            QVERIFY(f.open(e.path));
            Sheet sheet;
            QVERIFY(f.book.readSheet(0, &sheet));
            const Cell *last = at(sheet, QStringLiteral("A149999"));
            QVERIFY(last && last->style != 0);
            QVERIFY(f.sheetPart(0).contains(QStringLiteral("<col ")));
        }
        // one Undo takes it from all of them
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        for (const quint32 r : {quint32(0), quint32(120000), quint32(149999)}) QCOMPARE(styleAt(e, r, 0), 0u);
        QVERIFY(!lookAt(e, 200000, 0).bold);
        e.win.w.undoStack()->redo();
        QCOMPARE(styleAt(e, 149999, 0), style);
        // Clear Formats over a block of a hundred and fifty thousand cells clears each of them
        const crunch::EditSession::Outcome cleared = s->clearFormats(0, rangeOf(QStringLiteral("A1:A150000")));
        QVERIFY2(cleared.ok, qPrintable(cleared.problem));
        for (const quint32 r : {quint32(0), quint32(100001), quint32(149999)}) QCOMPARE(styleAt(e, r, 0), 0u);
    }

    // The Move or Copy window says what it asks in its own words: none of its labels, its choice of where the sheets go, or its check box is worded as another program's is.
    void theMoveOrCopyWindowIsWordedInOurOwnWords()
    {
        EditWindow e;
        QVERIFY(e.open(reviewBook()));
        QVERIFY(e.settle());
        QStringList seen;
        QTimer::singleShot(0, [&] {
            auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dialog) return;
            for (QLabel *l : dialog->findChildren<QLabel *>()) seen << l->text();
            for (QCheckBox *c : dialog->findChildren<QCheckBox *>()) seen << c->text();
            for (QListWidget *l : dialog->findChildren<QListWidget *>())
                for (int i = 0; i < l->count(); ++i) seen << l->item(i)->text();
            dialog->reject();
        });
        e.win.w.act(QStringLiteral("sheet.moveCopy"))->trigger();
        QVERIFY(seen.size() >= 4);
        for (const QString &text : seen) {
            const QString lower = text.toLower();
            QVERIFY2(!lower.contains(QStringLiteral("before sheet")) && !lower.contains(QStringLiteral("create a copy")) && !lower.contains(QStringLiteral("move to end")), qPrintable(text));
        }
        QVERIFY(seen.contains(QStringLiteral("(after the last sheet)")));
    }

    // A copy of a sheet is the sheet: its protection (the model says protected, and so does the file), filter, links (the one to a web page with its own relationship), page setup (without the
    // printer's settings, which are not copied), print options, margins, headers, page breaks, how it is outlined (rows, columns, and how deep), and how its data bars look (flat, bordered, with
    // the axis in the middle). Nothing is left out that the outcome does not say.
    void aCopyOfASheetHasWhatTheFileOfTheSheetHas()
    {
        EditWindow e;
        QVERIFY(e.open(richBook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const crunch::EditSession::Outcome copied = s->copySheet(0, -1, QStringLiteral("Copy"));
        QVERIFY(copied.ok);
        QVERIFY2(copied.problem.isEmpty(), qPrintable(copied.problem));   // (all of it is copied: nothing to say)
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        QCOMPARE(f.names(), QStringList({QStringLiteral("Rich"), QStringLiteral("Other"), QStringLiteral("Copy")}));
        const QString part = f.sheetPart(2);
        QVERIFY2(part.contains(QStringLiteral("<sheetProtection")) && part.contains(QStringLiteral("password=\"CC1A\"")), qPrintable(part));
        QVERIFY2(part.contains(QStringLiteral("<autoFilter ref=\"A1:C6\"")), qPrintable(part));
        QVERIFY2(part.contains(QStringLiteral("<hyperlink ref=\"A2\" r:id=\"rIdH1\"")) && part.contains(QStringLiteral("location=\"Other!A1\"")), qPrintable(part));
        QVERIFY2(part.contains(QStringLiteral("<printOptions gridLines=\"1\"")) && part.contains(QStringLiteral("<pageMargins left=\"1\"")), qPrintable(part));
        QVERIFY2(part.contains(QStringLiteral("orientation=\"landscape\"")) && part.contains(QStringLiteral("paperSize=\"9\"")), qPrintable(part));
        QVERIFY2(!QRegularExpression(QStringLiteral("<pageSetup[^>]*:id=")).match(part).hasMatch(), qPrintable(part));   // (no printer settings: the copy has no such part)
        QVERIFY2(part.contains(QStringLiteral("&amp;CReport")) && part.contains(QStringLiteral("<brk id=\"3\"")), qPrintable(part));
        QVERIFY2(part.contains(QStringLiteral("outlinePr summaryBelow=\"0\"")) && part.contains(QStringLiteral("fitToPage=\"1\"")), qPrintable(part));
        // the order the format wants: protection and filter after the data, then the rules, the links, the page setup, and the extensions last
        auto before = [&](const char *a, const char *b) { return part.indexOf(QLatin1String(a)) >= 0 && part.indexOf(QLatin1String(a)) < part.indexOf(QLatin1String(b)); };
        QVERIFY(before("</sheetData>", "<sheetProtection") && before("<sheetProtection", "<autoFilter") && before("<autoFilter", "<conditionalFormatting") && before("<conditionalFormatting", "<hyperlinks"));
        QVERIFY(before("<hyperlinks", "<printOptions") && before("<printOptions", "<pageMargins") && before("<pageMargins", "<pageSetup") && before("<pageSetup", "<headerFooter") && before("<headerFooter", "<rowBreaks"));
        QVERIFY(part.lastIndexOf(QLatin1String("<extLst")) > part.indexOf(QLatin1String("<rowBreaks")));   // (the list of extensions of the sheet ends it)
        // the link to the web page has its relationship in the copy's own relationships part
        QFile file(e.path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(file.readAll(), entries));
        const QString rels = QString::fromUtf8(entries.value(QStringLiteral("xl/worksheets/_rels/") + f.book.info().sheets[2].part.section(QLatin1Char('/'), -1) + QStringLiteral(".rels")));
        QVERIFY2(rels.contains(QStringLiteral("Id=\"rIdH1\"")) && rels.contains(QStringLiteral("https://example.org/data")) && rels.contains(QStringLiteral("TargetMode=\"External\"")), qPrintable(rels));
        QVERIFY2(!rels.contains(QStringLiteral("printerSettings")), qPrintable(rels));
        // the outline: the depth, the columns' level, the rows' levels and groups
        QVERIFY2(part.contains(QStringLiteral("outlineLevelRow=\"2\"")) && part.contains(QStringLiteral("outlineLevelCol=\"1\"")), qPrintable(part));
        QVERIFY2(QRegularExpression(QStringLiteral("<col [^>]*outlineLevel=\"1\"")).match(part).hasMatch(), qPrintable(part));
        QVERIFY2(QRegularExpression(QStringLiteral("<row r=\"2\"[^>]*outlineLevel=\"1\"")).match(part).hasMatch(), qPrintable(part));
        QVERIFY2(QRegularExpression(QStringLiteral("<row r=\"3\"[^>]*outlineLevel=\"2\"")).match(part).hasMatch() && QRegularExpression(QStringLiteral("<row r=\"3\"[^>]*hidden=\"1\"")).match(part).hasMatch(), qPrintable(part));
        QVERIFY2(QRegularExpression(QStringLiteral("<row r=\"4\"[^>]*collapsed=\"1\"")).match(part).hasMatch(), qPrintable(part));
        // the data bar's look: the file says it as the sheet it came from did, and the program reads it as it read the sheet
        QVERIFY2(part.contains(QStringLiteral("x14:dataBar")) && part.contains(QStringLiteral("gradient=\"0\"")) && part.contains(QStringLiteral("border=\"1\"")) && part.contains(QStringLiteral("axisPosition=\"middle\"")),
                 qPrintable(part));
        Sheet copy, original;
        QVERIFY(f.book.readSheet(2, &copy));
        QVERIFY(f.book.readSheet(0, &original));
        QVERIFY(copy.isProtected && original.isProtected);
        QCOMPARE(copy.conditionalFormats.size(), 1);
        QVERIFY(!copy.conditionalFormats[0].gradient && copy.conditionalFormats[0].border);
        QVERIFY(copy.conditionalFormats[0].axis == CondRule::Axis::Middle);
        QCOMPARE(copy.conditionalFormats[0].minLength, 0);
        QCOMPARE(copy.conditionalFormats[0].maxLength, 100);
        QCOMPARE(copy.format.outlineLevelRow, quint8(2));
        QCOMPARE(copy.format.outlineLevelCol, quint8(1));
        QCOMPARE(copy.rowSpecs.size(), original.rowSpecs.size());
        for (int i = 0; i < copy.rowSpecs.size(); ++i) {
            QCOMPARE(copy.rowSpecs[i].row, original.rowSpecs[i].row);
            QCOMPARE(copy.rowSpecs[i].outline, original.rowSpecs[i].outline);
            QCOMPARE(copy.rowSpecs[i].collapsed, original.rowSpecs[i].collapsed);
            QCOMPARE(copy.rowSpecs[i].hidden, original.rowSpecs[i].hidden);
        }
        // the sheet it was copied from is as it was
        const QString first = f.sheetPart(0);
        QVERIFY(first.contains(QStringLiteral("rIdPS")) && first.contains(QStringLiteral("gradient=\"0\"")));
    }

    // Rows inserted before a sheet is copied move what the copy takes (the filter, links, and page breaks follow the cells they name), rows inserted in the sheet after the copy are not the copy's,
    // and rows inserted in the copy move its own: each sheet has its places as its own cells have theirs.
    void whatACopyTakesFollowsTheRowsInsertedBeforeAndAfter()
    {
        EditWindow e;
        QVERIFY(e.open(richBook(false)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(0, 0, 2).ok);                                    // (before the copy: the copy has it)
        int made = -1;
        QVERIFY(s->copySheet(0, -1, QStringLiteral("Copy"), &made).ok);
        QCOMPARE(made, 2);
        QVERIFY(s->insertRows(0, 0, 1).ok);                                    // (after the copy, in the sheet it was copied from: not the copy's)
        QVERIFY(s->insertRows(made, 0, 3).ok);                                 // (in the copy: its own)
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        const QString original = f.sheetPart(0), copy = f.sheetPart(2);
        QVERIFY2(original.contains(QStringLiteral("<autoFilter ref=\"A4:C9\"")), qPrintable(original));   // the file's A1:C6, 2 rows down and then 1 more
        QVERIFY2(copy.contains(QStringLiteral("<autoFilter ref=\"A6:C11\"")), qPrintable(copy));          // the file's A1:C6, 2 rows down and then 3 more
        QVERIFY2(original.contains(QStringLiteral("<hyperlink ref=\"A5\"")) && copy.contains(QStringLiteral("<hyperlink ref=\"A7\"")), qPrintable(copy));
        QVERIFY2(original.contains(QStringLiteral("<brk id=\"6\"")) && copy.contains(QStringLiteral("<brk id=\"8\"")), qPrintable(copy));
        // the cells are where the model has them
        Sheet data;
        QVERIFY(f.book.readSheet(2, &data));
        QCOMPARE(show(data, f.book.sharedStrings(), QStringLiteral("B7")), QStringLiteral("n:10"));
    }

    // A copy of a sheet whose file has what the program neither shows nor copies says so, in words, in the outcome (the sheet is copied all the same).
    void aCopySaysWhatItLeavesOut()
    {
        const QByteArray book = edited(richBook(), [](QMap<QString, QByteArray> &parts) {
            parts[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")].replace("</Relationships>",
                "<Relationship Id=\"rIdD1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing\" Target=\"../drawings/drawing1.xml\"/></Relationships>");
            QString sheet = QString::fromUtf8(parts[QStringLiteral("xl/worksheets/sheet1.xml")]);
            sheet.replace(QStringLiteral("</extLst></worksheet>"),
                          QStringLiteral("<ext xmlns:x14=\"%1\" uri=\"{05C60535-1F16-4fd2-B633-F4F36F0B64E0}\"><x14:sparklineGroups xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\"/></ext></extLst></worksheet>")
                              .arg(QString::fromLatin1(kX14)));
            parts[QStringLiteral("xl/worksheets/sheet1.xml")] = sheet.toUtf8();
            parts[QStringLiteral("xl/drawings/drawing1.xml")] = "<xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\"/>";
        });
        EditWindow e;
        QVERIFY(e.open(book));
        QVERIFY(e.settle());
        const crunch::EditSession::Outcome copied = e.win.w.session()->copySheet(0, -1, QStringLiteral("Copy"));
        QVERIFY(copied.ok);
        QVERIFY2(copied.problem.contains(QStringLiteral("not copied")) && copied.problem.contains(QStringLiteral("pictures and charts")) && copied.problem.contains(QStringLiteral("sparklines")),
                 qPrintable(copied.problem));
    }

    // The font box shows the typeface of the cell as the workbook has it: a font that gives no name and says it is the theme's body face is the face the theme names, and a workbook with no
    // theme and no name shows the computer's own usual face (the program names no typeface of its own).
    void theFontBoxShowsTheTypefaceTheWorkbookHas()
    {
        const QString nameless = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><family val=\"2\"/><scheme val=\"minor\"/></font></fonts>"
                                                "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                                "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                                "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
        const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"), nameless);
        const QByteArray theme = QStringLiteral("<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"><a:themeElements><a:clrScheme name=\"Test\"/><a:fontScheme name=\"Test\">"
                                                "<a:majorFont><a:latin typeface=\"Cambria\"/></a:majorFont><a:minorFont><a:latin typeface=\"Candara\"/></a:minorFont></a:fontScheme></a:themeElements></a:theme>")
                                     .toUtf8();
        const QByteArray sheet = worksheet(rowXml(1, cellXml(QStringLiteral("A1"), QStringLiteral("1"))));
        {
            EditWindow e;
            QVERIFY(e.open(buildXlsx({sheet}, QByteArray(), QString(), QString(), styles, QStringList(), QString(), theme)));
            e.select(QStringLiteral("A1"));
            auto *font = e.win.w.findChild<FontCombo *>(QStringLiteral("fmt.fontName"));
            QVERIFY(font);
            QTRY_COMPARE(font->currentText(), QStringLiteral("Candara"));
        }
        {
            EditWindow e;
            QVERIFY(e.open(buildXlsx({sheet}, QByteArray(), QString(), QString(), styles)));
            e.select(QStringLiteral("A1"));
            auto *font = e.win.w.findChild<FontCombo *>(QStringLiteral("fmt.fontName"));
            QVERIFY(font);
            QTRY_VERIFY(!font->currentText().isEmpty());
            QVERIFY(font->currentText() != QLatin1String("Candara"));
        }
    }

    // A workbook takes no more cell formats than a file holds well (64,000), and says so when a format is not made, in place of leaving the cells as they were; the fonts, fills, and
    // borders that are looked for are found by a hash, a few comparisons each, not by going through all the workbook has.
    void aWorkbookTakesNoMoreFormatsThanAFileHoldsAndFindsThemByAHash()
    {
        const PinnedLocale english;
        EditWindow e;
        QVERIFY(e.open(reviewBook()));
        QVERIFY(e.settle());
        Workbook &book = *e.win.w.loaded()->book;
        const qint64 before = book.placeComparisons();
        for (int i = 0; i < 20000; ++i) {
            FormatChange c;
            c.fontName = QStringLiteral("Face%1").arg(i);
            book.formatWith(0, c);
        }
        // (a place for each of twenty thousand fonts, none of them the same as another: going through the fonts for each would be two hundred million comparisons)
        const qint64 compared = book.placeComparisons() - before;
        QVERIFY2(compared < 100000, qPrintable(QString::number(compared)));
        // the same format again is the one that is there
        FormatChange again;
        again.fontName = QStringLiteral("Face77");
        const qsizetype count = book.styles().formats.size();
        const quint32 face77 = book.formatWith(0, again);
        QCOMPARE(book.formatWith(0, again), face77);
        QCOMPARE(book.styles().formats.size(), count);
        // to the limit
        for (int i = 20000; book.styles().formats.size() < kMaxCellFormatsWritten; ++i) {
            FormatChange c;
            c.fontName = QStringLiteral("Face%1").arg(i);
            book.formatWith(0, c);
        }
        QCOMPARE(qint64(book.styles().formats.size()), kMaxCellFormatsWritten);
        book.clearFormatLimit();
        const int steps = e.win.w.undoStack()->count();
        FormatChange fresh;
        FillSpec fill;
        fill.pattern = PatternKind::Solid;
        fill.fg = ColorSpec::fromRgb(0x123456);
        fresh.fill = fill;
        const crunch::EditSession::Outcome o = e.win.w.session()->formatCells(0, rangeOf(QStringLiteral("A1")), fresh, QStringLiteral("Formatting"));
        QVERIFY(!o.ok);
        QVERIFY2(o.problem.contains(QStringLiteral("64,000")), qPrintable(o.problem));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(styleAt(e, 0, 0), 0u);
        // a format the workbook has already is still given
        QVERIFY(e.win.w.session()->formatCells(0, rangeOf(QStringLiteral("A1")), again, QStringLiteral("Formatting")).ok);
        QCOMPARE(styleAt(e, 0, 0), face77);
    }
    // The twelve-step script every app's threads are run through (tests/pub/threadscript.h): the same observable result as JeffPub's and JeffWrite's.
    void commentThreadsRunTheSharedScript()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchThreads t(e.win.w.session());
        threadscript::run(t, QStringLiteral("Zed Ypsilon"));
    }

    // The file keeps a thread with its replies both ways; our own file, with a note added somewhere else, is written with the same thread parts; the Reviewing Pane's list has the thread as a Commented change.
    void threadsAreKeptBothWaysAndOurOwnFileKeepsItsParts()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchThreads t(e.win.w.session());
        QVERIFY(t.start(QStringLiteral("Q")));
        QVERIFY(t.start(QStringLiteral("Second")));
        QVERIFY(t.reply(0, 0, QStringLiteral("R1")));
        QVERIFY(t.reply(0, 1, QStringLiteral("R2")));
        QVERIFY(t.resolve(0, true));
        const jp::review::ChangeList changes = crunch::threadChanges(e.win.w.session()->anchors(), [](int) { return QStringLiteral("Sheet1"); });
        QCOMPARE(changes.size(), 2);
        QCOMPARE(changes.changes[0].kind, jp::review::Kind::Commented);
        QCOMPARE(changes.changes[0].state, jp::review::State::Accepted);
        QCOMPARE(changes.changes[0].target.object, QStringLiteral("A11"));
        QCOMPARE(changes.changes[0].replies.size(), 2);
        QCOMPARE(changes.changes[0].replies[1].text, QStringLiteral("R2"));
        QCOMPARE(changes.changes[1].state, jp::review::State::Pending);
        QVERIFY(changes.changes[0].position < changes.changes[1].position);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const QString copy = e.copyOfFile();
        Window second;
        QVERIFY(second.open(copy));
        CrunchThreads r(second.w.session());
        QCOMPARE(r.state(), (QStringList{QStringLiteral("Q | R1 | R2 [resolved]"), QStringLiteral("Second")}));
        QCOMPARE(r.author(0, 2), QStringLiteral("Zed Ypsilon"));
        // saved again with a note added somewhere else: the thread parts and the people are the same bytes
        const QMap<QString, QByteArray> before = partsNamed(copy, QStringLiteral("hreaded"));
        QVERIFY(!before.isEmpty());
        QVERIFY(second.w.session()->setNote(0, 0, 0, QStringLiteral("elsewhere"), QStringLiteral("Zed"), false, QStringLiteral("Edit Note")).ok);
        QSignalSpy again(&second.w, &CrunchWindow::saved);
        QVERIFY(second.w.save());
        QVERIFY2(savedOk(again), "the second save failed");
        QCOMPARE(partsNamed(copy, QStringLiteral("hreaded")), before);
    }

    // A file another program made: every note and every thread is there after it is opened, changed somewhere else, and saved, and the persons part and the thread parts are still in it.
    void aForeignWorkbookKeepsEveryNoteAndThreadThroughOpenAndSave()
    {
        EditWindow e;
        QVERIFY(e.open(foreignBook()));
        crunch::EditSession *s = e.win.w.session();
        CrunchThreads t(s);
        QCOMPARE(t.state(), (QStringList{QStringLiteral("Q1 | R1 | R2"), QStringLiteral("Q2")}));   // (by place in the sheet: D2 is row 2)
        QCOMPARE(s->anchors().noteAt(0, 0, 0)->text, QStringLiteral("Ann says check the total"));
        QCOMPARE(s->anchors().noteAt(0, 2, 2)->text, QStringLiteral("Bob says hi"));
        QVERIFY(s->setNote(0, 4, 4, QStringLiteral("elsewhere"), QStringLiteral("Zed"), false, QStringLiteral("Edit Note")).ok);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const QString copy = e.copyOfFile();
        Window second;
        QVERIFY(second.open(copy));
        CrunchThreads r(second.w.session());
        QCOMPARE(r.state(), t.state());
        QCOMPARE(second.w.session()->anchors().noteAt(0, 0, 0)->text, QStringLiteral("Ann says check the total"));
        QCOMPARE(second.w.session()->anchors().noteAt(0, 2, 2)->text, QStringLiteral("Bob says hi"));
        QCOMPARE(second.w.session()->anchors().noteAt(0, 4, 4)->text, QStringLiteral("elsewhere"));
        QCOMPARE(int(second.w.session()->anchors().notes(0).size()), 5);
        QCOMPARE(r.author(0, 2), QStringLiteral("Ann Lee"));
        QVERIFY(!partsNamed(copy, QStringLiteral("person")).isEmpty());
        const QString threadPart = QString::fromUtf8(partsNamed(copy, QStringLiteral("hreadedComment")).value(QStringLiteral("xl/threadedComments/threadedComment1.xml")));
        QCOMPARE(int(threadPart.count(QStringLiteral("<threadedComment "))), 4);
    }

    // Past the thread limits a person may add, a thread from a file is still theirs to edit, resolve, and delete.
    void aThreadOfAFileLongerThanReplyingAllowsCanStillBeEditedResolvedAndDeleted()
    {
        EditWindow e;
        QVERIFY(e.open(foreignBook(300)));
        crunch::EditSession *s = e.win.w.session();
        CrunchThreads t(s);
        QCOMPARE(t.entries(0), 303);
        QVERIFY(!t.reply(0, 0, QStringLiteral("one more")));
        QCOMPARE(t.entries(0), 303);
        QVERIFY(t.edit(0, 250, QStringLiteral("edited")));
        QVERIFY(t.resolve(0, true));
        QVERIFY(t.remove(0, 250));
        QCOMPARE(t.entries(0), 302);
        QVERIFY(t.remove(0, 0));
        QCOMPARE(t.threadCount(), 1);
    }

    // The limit on the comments of a workbook is the reader's, not the 2000 threads of the other apps.
    void newCommentsStopAtTheWorkbookLimitAndNotAtTheOtherAppsThreadLimit()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        crunch::EditSession *s = e.win.w.session();
        CrunchThreads t(s);
        jp::threads::setCapsForTest(2, 0);
        QVERIFY(t.start(QStringLiteral("one")));
        QVERIFY(t.start(QStringLiteral("two")));
        QVERIFY(t.start(QStringLiteral("three")));
        jp::threads::setCapsForTest(0, 0);
        QCOMPARE(t.threadCount(), 3);
        crunch::setBookCommentCapForTest(3);
        const auto refused = s->newComment(0, 40, 0, QStringLiteral("four"), t.who, QStringLiteral("New Comment"));
        crunch::setBookCommentCapForTest(0);
        QVERIFY(!refused.ok);
        QVERIFY2(refused.problem.contains(QStringLiteral("workbook")), qPrintable(refused.problem));
        QCOMPARE(t.threadCount(), 3);
    }

    void convertingNotesStopsAtTheWorkbookLimitAndSaysSo()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        crunch::EditSession *s = e.win.w.session();
        CrunchThreads t(s);
        QVERIFY(t.start(QStringLiteral("a thread")));
        std::vector<std::pair<quint32, quint32>> cells;
        for (quint32 i = 0; i < 4; ++i) {
            QVERIFY(s->setNote(0, 30 + i, 2, QStringLiteral("note %1").arg(i), QStringLiteral("Zed"), false, QStringLiteral("Edit Note")).ok);
            cells.emplace_back(30 + i, 2);
        }
        crunch::setBookCommentCapForTest(3);
        const auto done = s->convertNotesToComments(0, cells, t.who, QStringLiteral("Convert Notes to Comments"));
        crunch::setBookCommentCapForTest(0);
        QVERIFY(done.ok);
        QVERIFY2(done.problem.contains(QStringLiteral("workbook")), qPrintable(done.problem));
        QCOMPARE(t.threadCount(), 3);   // (the first thread, and two of the four notes)
        int notes = 0;
        for (const auto &c : cells) notes += !s->anchors().noteAt(0, c.first, c.second)->thread;
        QCOMPARE(notes, 2);
    }

    void pastingThreadsStopsAtTheWorkbookLimitAndSaysSo()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        crunch::EditSession *s = e.win.w.session();
        CrunchThreads t(s);
        QVERIFY(t.start(QStringLiteral("a thread")));
        jp::office::sheet::CellRange src, target;
        jp::office::sheet::parseRangeRef(QStringLiteral("A11"), &src);
        jp::office::sheet::parseRangeRef(QStringLiteral("F20:F24"), &target);
        crunch::ClipData copied = s->copyCells(0, src, false), clip;
        QVERIFY(crunch::ClipData::fromBytes(copied.toBytes(), &clip));
        QVERIFY(!clip.threads.empty());
        crunch::setBookCommentCapForTest(3);
        const auto pasted = s->paste(0, target, clip, crunch::EditSession::PasteOptions());
        crunch::setBookCommentCapForTest(0);
        QVERIFY2(pasted.ok, qPrintable(pasted.problem));
        QVERIFY2(pasted.problem.contains(QStringLiteral("workbook")), qPrintable(pasted.problem));
        QCOMPARE(t.threadCount(), 3);   // (the thread and two of its five copies)
    }
};

JP_TEST_CLASS(CrunchReviewTests)
#include "tst_crunch_review.moc"

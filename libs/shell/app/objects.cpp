#include "app/objects.h"

#include "io/zip.h"
#include "office/chart.h"
#include "office/package.h"
#include "office/theme.h"
#include "sheet/cellref.h"
#include "sheet/sheetmodel.h"
#include "sheet/sheetreader.h"
#include "wml/wml.h"
#include "wml/wmlhtml.h"

#include <QAbstractTextDocumentLayout>
#include <QBuffer>
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QUuid>
#include <QFile>
#include <QLocale>
#include <QFileInfo>
#include <QFontMetricsF>
#include <QProcess>
#include <QPainter>
#include <QPalette>
#include <QSvgRenderer>
#include <QTextDocument>

#include <algorithm>
#include <cmath>

namespace jp::objects {

using namespace office::ole;

namespace {

constexpr double kRowHeight = 18;       // points
constexpr double kPageWidth = 612, kPageHeight = 792;
constexpr double kChartWidth = 360, kChartHeight = 216;
constexpr char kPageNote[] = "jeffoffice-page.txt";
constexpr qint64 kMostInflated = qint64(512) << 20;

void say(QString *why, const QString &text)
{
    if (why) *why = text;
}

QFont gridFont()
{
    QFont f;
    f.setPixelSize(11);
    return f;
}

QVector<double> columnWidths(const Grid &grid)
{
    const QFontMetricsF fm(gridFont());
    int columns = 0;
    for (const auto &row : grid) columns = std::max<int>(columns, int(row.size()));
    QVector<double> widths(columns, 48.0);
    for (const auto &row : grid)
        for (int c = 0; c < row.size(); ++c) widths[c] = std::clamp(std::max(widths[c], fm.horizontalAdvance(row[c].text) + 10), 48.0, 220.0);
    return widths;
}

QString xmlEscape(const QString &text)
{
    QString out = text;
    out.replace(QLatin1Char('&'), QLatin1String("&amp;")).replace(QLatin1Char('<'), QLatin1String("&lt;")).replace(QLatin1Char('>'), QLatin1String("&gt;"));
    // (A character XML does not allow is left out.)
    out.removeIf([](QChar c) { return c.unicode() < 0x20 && c != QLatin1Char('\t') && c != QLatin1Char('\n') && c != QLatin1Char('\r'); });
    return out;
}

QImage blank(qint64 widthEmu, qint64 heightEmu, int scale, QSizeF *points)
{
    const double w = double(widthEmu) / kEmuPerPoint, h = double(heightEmu) / kEmuPerPoint;
    *points = QSizeF(w, h);
    if (!pictureSizeIsSane(widthEmu, heightEmu) || scale < 1 || w < 1 || h < 1) return QImage();
    QImage image(int(std::ceil(w * scale)), int(std::ceil(h * scale)), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    return image;
}

QImage chartPicture(const QByteArray &package, qint64 widthEmu, qint64 heightEmu, int scale)
{
    office::Package pkg;
    if (!pkg.read(package, nullptr, kMostInflated)) return QImage();
    QString part;
    for (const QString &name : pkg.partNames()) {
        const QString lower = name.toLower();
        if (lower.contains(QLatin1String("xl/charts/chart")) && lower.endsWith(QLatin1String(".xml")) && (part.isEmpty() || lower < part.toLower())) part = name;
    }
    if (part.isEmpty()) return QImage();
    office::Chart chart;
    if (!office::readChart(pkg.part(part), office::defaultTheme(), office::ColorMap(), &chart)) return QImage();
    QSizeF points;
    QImage image = blank(widthEmu, heightEmu, scale, &points);
    if (image.isNull()) return image;
    QSvgRenderer svg(office::chartSvg(chart, points));
    if (!svg.isValid()) return QImage();
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    svg.render(&p, QRectF(0, 0, image.width(), image.height()));
    return image;
}

QImage gridPicture(const Grid &grid, qint64 widthEmu, qint64 heightEmu, int scale)
{
    QSizeF points;
    QImage image = blank(widthEmu, heightEmu, scale, &points);
    if (image.isNull()) return image;
    const QVector<double> widths = columnWidths(grid);
    double total = 0;
    for (double w : widths) total += w;
    const double height = kRowHeight * grid.size();
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.scale(image.width() / std::max(total, 1.0), image.height() / std::max(height, 1.0));
    p.setFont(gridFont());
    p.setPen(QColor(0xD0, 0xD0, 0xD0));
    double y = 0;
    for (int r = 0; r < grid.size(); ++r, y += kRowHeight) {
        double x = 0;
        for (int c = 0; c < widths.size(); ++c) {
            const QRectF cell(x, y, widths[c], kRowHeight);
            p.setPen(QColor(0xD0, 0xD0, 0xD0));
            p.drawRect(cell);
            if (c < grid[r].size() && !grid[r][c].text.isEmpty()) {
                p.setPen(Qt::black);
                p.drawText(cell.adjusted(4, 0, -4, 0), (grid[r][c].number ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter, grid[r][c].text);
            }
            x += widths[c];
        }
    }
    return image;
}

QByteArray withChartPart(const QByteArray &zip, const QByteArray &chartXml)
{
    QMap<QString, QByteArray> parts;
    if (!readZip(zip, parts, nullptr, kMostInflated)) return QByteArray();
    parts[QStringLiteral("xl/charts/chart1.xml")] = chartXml;
    QByteArray types = parts.value(QStringLiteral("[Content_Types].xml"));
    types.replace("</Types>", "<Override PartName=\"/xl/charts/chart1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawingml.chart+xml\"/></Types>");
    parts[QStringLiteral("[Content_Types].xml")] = types;
    ZipWriter zw;
    zw.add(QStringLiteral("[Content_Types].xml"), parts.value(QStringLiteral("[Content_Types].xml")), true);
    for (auto it = parts.cbegin(); it != parts.cend(); ++it)
        if (it.key() != QLatin1String("[Content_Types].xml")) zw.add(it.key(), it.value(), true);
    return zw.finish();
}

QByteArray withNote(const QByteArray &zip, const QByteArray &note)
{
    QMap<QString, QByteArray> parts;
    if (!readZip(zip, parts, nullptr, kMostInflated)) return QByteArray();
    parts[QString::fromLatin1(kPageNote)] = note;
    QByteArray types = parts.value(QStringLiteral("[Content_Types].xml"));
    if (!types.isEmpty() && !types.contains("Extension=\"txt\"")) {
        const qsizetype at = types.indexOf("<Override");
        const QByteArray add = "<Default Extension=\"txt\" ContentType=\"text/plain\"/>";
        types.insert(at >= 0 ? at : types.indexOf('>', types.indexOf("<Types")) + 1, add);
        parts[QStringLiteral("[Content_Types].xml")] = types;
    }
    ZipWriter zw;
    if (parts.contains(QStringLiteral("[Content_Types].xml"))) zw.add(QStringLiteral("[Content_Types].xml"), parts.value(QStringLiteral("[Content_Types].xml")), true);
    for (auto it = parts.cbegin(); it != parts.cend(); ++it)
        if (it.key() != QLatin1String("[Content_Types].xml")) zw.add(it.key(), it.value(), true);
    return zw.finish();
}

int pageNoteOf(const QByteArray &zip)
{
    QMap<QString, QByteArray> parts;
    if (!readZip(zip, parts, nullptr, kMostInflated)) return 1;
    bool ok = false;
    const int page = parts.value(QString::fromLatin1(kPageNote)).trimmed().toInt(&ok);
    return ok && page >= 1 ? std::min(page, kMostPages) : 1;
}

bool layOutPages(const QByteArray &docx, QTextDocument *doc)
{
    office::wml::Document words;
    QString error;
    if (!office::wml::readDocx(docx, &words, &error)) return false;
    office::wml::HtmlOptions options;
    options.singleFile = true;
    doc->setHtml(QString::fromUtf8(office::wml::documentToHtml(words, options)));
    doc->setDocumentMargin(54);
    doc->setPageSize(QSizeF(kPageWidth, kPageHeight));
    return true;
}

QByteArray fileBytes(const QString &path, QString *why)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        say(why, QCoreApplication::translate("jp::Objects", "The file can't be opened."));
        return QByteArray();
    }
    if (f.size() > limits().objectBytes) {
        say(why, QCoreApplication::translate("jp::Objects", "The file is too large to put in as an object."));
        return QByteArray();
    }
    return f.readAll();
}

std::optional<EmbeddedObject> make(Kind kind, const QByteArray &package, const QSizeF &points, QString *why)
{
    ObjectBudget budget;
    auto object = makeObject(kind, package, qint64(points.width() * kEmuPerPoint), qint64(points.height() * kEmuPerPoint), pictureFor(kind), budget);
    if (!object) say(why, QCoreApplication::translate("jp::Objects", "This can't be put in as an object."));
    return object;
}

// A range's cells in the order the workbook has them, as the window shows them.
bool gridOf(const office::sheet::Workbook &book, const office::sheet::Sheet &sheet, const office::sheet::CellRange &range, Grid *grid)
{
    using namespace office::sheet;
    const quint32 rows = std::min<quint32>(range.row2 - range.row1 + 1, kMostRows), cols = std::min<quint32>(range.col2 - range.col1 + 1, kMostColumns);
    grid->clear();
    for (quint32 r = 0; r < rows; ++r) {
        QVector<CellValue> line;
        for (quint32 c = 0; c < cols; ++c) {
            CellValue v;
            if (const Cell *cell = sheet.cells.find(range.row1 + r, range.col1 + c)) {
                switch (cell->type) {
                case CellType::Number:
                case CellType::Date:
                    v.number = true;
                    v.value = cell->number;
                    v.text = QString::number(cell->number, 'g', 10);
                    break;
                case CellType::Bool: v.text = cell->number != 0 ? QStringLiteral("TRUE") : QStringLiteral("FALSE"); break;
                case CellType::Error: v.text = errorText(CellError(int(cell->number))); break;
                case CellType::Empty: break;
                default: v.text = sheet.text(*cell, book.sharedStrings()); break;
                }
            }
            if (v.text.size() > kMostCellText) v.text.truncate(kMostCellText);
            line.push_back(v);
        }
        grid->push_back(line);
    }
    return !grid->isEmpty();
}

} // namespace

QString kindWords(const EmbeddedObject &object)
{
    switch (object.kind()) {
    case Kind::Sheet: return packageHasChart(object.partBytes) ? QCoreApplication::translate("jp::Objects", "Chart") : QCoreApplication::translate("jp::Objects", "Spreadsheet range");
    case Kind::Page: return QCoreApplication::translate("jp::Objects", "Document page");
    default: return QCoreApplication::translate("jp::Objects", "Other object");
    }
}

bool packageHasChart(const QByteArray &package)
{
    office::Package pkg;
    if (!pkg.readLazy(package)) return false;
    for (const QString &name : pkg.partNames())
        if (name.contains(QLatin1String("xl/charts/chart")) && name.endsWith(QLatin1String(".xml"))) return true;
    return false;
}

QImage sheetPicture(const QByteArray &package, qint64 widthEmu, qint64 heightEmu, int scale)
{
    if (packageHasChart(package)) return chartPicture(package, widthEmu, heightEmu, scale);
    Grid grid;
    if (!readRange(package, QString(), &grid, nullptr)) return QImage();
    return gridPicture(grid, widthEmu, heightEmu, scale);
}

QImage pagePicture(const QByteArray &package, qint64 widthEmu, qint64 heightEmu, int scale)
{
    QSizeF points;
    QImage image = blank(widthEmu, heightEmu, scale, &points);
    if (image.isNull()) return image;
    QTextDocument doc;
    if (!layOutPages(package, &doc)) return QImage();
    const int page = std::min(pageNoteOf(package), std::max(1, doc.pageCount())) - 1;
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.scale(image.width() / kPageWidth, image.height() / kPageHeight);
    p.translate(0, -page * kPageHeight);
    const QRectF area(0, page * kPageHeight, kPageWidth, kPageHeight);
    p.setClipRect(area);
    QAbstractTextDocumentLayout::PaintContext context;
    context.palette.setColor(QPalette::Text, Qt::black);
    context.clip = area;
    doc.documentLayout()->draw(&p, context);
    p.resetTransform();
    p.setPen(QColor(0xC0, 0xC0, 0xC0));
    p.drawRect(0, 0, image.width() - 1, image.height() - 1);
    return image;
}

PictureFn pictureFor(Kind kind)
{
    if (kind == Kind::Sheet) return sheetPicture;
    if (kind == Kind::Page) return pagePicture;
    return PictureFn();
}

void usePictures(EmbeddedEdit *edit)
{
    edit->setPicture(Kind::Sheet, sheetPicture);
    edit->setPicture(Kind::Page, pagePicture);
}

bool openInOwnApp(const QString &path, Kind kind)
{
    const QString name = kind == Kind::Sheet ? QStringLiteral("JeffCrunch") : kind == Kind::Page ? QStringLiteral("JeffWrite") : QString();
    if (name.isEmpty()) return false;
    const QDir folder(QCoreApplication::applicationDirPath());
    for (const QString &file : {name, name + QStringLiteral(".exe"), name.toLower()}) {
        const QFileInfo exe(folder.filePath(file));
        if (exe.isFile() && exe.isExecutable()) return QProcess::startDetached(exe.absoluteFilePath(), {path});
    }
    return false;
}

bool readRange(const QByteArray &workbook, const QString &range, Grid *grid, QString *why)
{
    using namespace office::sheet;
    office::Package pkg;
    QString error;
    Workbook book;
    if (!pkg.read(workbook, &error, kMostInflated) || !book.open(pkg, &error)) {
        say(why, QCoreApplication::translate("jp::Objects", "This is not a workbook that can be read."));
        return false;
    }
    Sheet sheet;
    if (!book.readSheet(0, &sheet)) {
        say(why, QCoreApplication::translate("jp::Objects", "The workbook's first sheet can't be read."));
        return false;
    }
    CellRange cr;
    if (range.trimmed().isEmpty()) {
        cr.row1 = cr.col1 = 0;
        cr.row2 = std::max<quint32>(sheet.extentRows, 1) - 1;
        cr.col2 = std::max<quint32>(sheet.extentCols, 1) - 1;
        cr.valid = true;
    } else if (!parseRangeRef(range.trimmed(), &cr)) {
        say(why, QCoreApplication::translate("jp::Objects", "That is not a range of cells. Write it like A1:C5."));
        return false;
    }
    if (!gridOf(book, sheet, cr, grid)) {
        say(why, QCoreApplication::translate("jp::Objects", "The range holds nothing."));
        return false;
    }
    return true;
}

bool readRangeFile(const QString &path, const QString &range, Grid *grid, QString *why)
{
    const QByteArray bytes = fileBytes(path, why);
    return !bytes.isEmpty() && readRange(bytes, range, grid, why);
}

Grid gridFromText(const QString &text)
{
    Grid grid;
    const QStringList lines = text.left(1 << 20).split(QLatin1Char('\n'));
    for (int r = 0; r < lines.size() && grid.size() < kMostRows; ++r) {
        if (r == lines.size() - 1 && lines[r].isEmpty()) break;
        QVector<CellValue> row;
        const QStringList cells = lines[r].split(QLatin1Char('\t'));
        for (int c = 0; c < cells.size() && c < kMostColumns; ++c) {
            CellValue v;
            v.text = cells[c];
            if (v.text.size() >= 2 && v.text.startsWith(QLatin1Char('"')) && v.text.endsWith(QLatin1Char('"'))) v.text = v.text.mid(1, v.text.size() - 2).replace(QLatin1String("\"\""), QLatin1String("\""));
            if (v.text.endsWith(QLatin1Char('\r'))) v.text.chop(1);
            if (v.text.size() > kMostCellText) v.text.truncate(kMostCellText);
            bool number = false;
            v.value = QLocale::c().toDouble(v.text, &number);
            v.number = number;
            row.push_back(v);
        }
        grid.push_back(row);
    }
    return grid;
}

QSizeF rangeSize(const Grid &grid)
{
    double total = 0;
    for (double w : columnWidths(grid)) total += w;
    return QSizeF(std::max(total, 48.0), std::max<double>(kRowHeight, kRowHeight * grid.size()));
}

QByteArray rangePackage(const Grid &grid)
{
    QString rows;
    for (int r = 0; r < grid.size(); ++r) {
        rows += QStringLiteral("<row r=\"%1\">").arg(r + 1);
        for (int c = 0; c < grid[r].size(); ++c) {
            const CellValue &v = grid[r][c];
            if (v.text.isEmpty()) continue;
            const QString ref = office::sheet::cellName(quint32(r), quint32(c));
            if (v.number) rows += QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, QString::number(v.value, 'g', 17));
            else rows += QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t xml:space=\"preserve\">%2</t></is></c>").arg(ref, xmlEscape(v.text));
        }
        rows += QStringLiteral("</row>");
    }
    const QString main = QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main");
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships");
    ZipWriter zw;
    zw.add(QStringLiteral("[Content_Types].xml"),
           QByteArrayLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                             "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/><Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                             "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
                             "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/></Types>"),
           true);
    zw.add(QStringLiteral("_rels/.rels"),
           QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                          "<Relationship Id=\"rId1\" Type=\"%1/officeDocument\" Target=\"xl/workbook.xml\"/></Relationships>").arg(rel).toUtf8(), true);
    zw.add(QStringLiteral("xl/workbook.xml"),
           QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><workbook xmlns=\"%1\" xmlns:r=\"%2\"><sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>")
               .arg(main, rel).toUtf8(), true);
    zw.add(QStringLiteral("xl/_rels/workbook.xml.rels"),
           QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                          "<Relationship Id=\"rId1\" Type=\"%1/worksheet\" Target=\"worksheets/sheet1.xml\"/></Relationships>").arg(rel).toUtf8(), true);
    zw.add(QStringLiteral("xl/worksheets/sheet1.xml"),
           QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><worksheet xmlns=\"%1\"><sheetData>%2</sheetData></worksheet>").arg(main, rows).toUtf8(), true);
    return zw.finish();
}

std::optional<EmbeddedObject> rangeObject(const Grid &grid, QString *why)
{
    if (grid.isEmpty()) {
        say(why, QCoreApplication::translate("jp::Objects", "The range holds nothing."));
        return std::nullopt;
    }
    return make(Kind::Sheet, rangePackage(grid), rangeSize(grid), why);
}

std::optional<EmbeddedObject> rangeObjectFromFile(const QString &path, const QString &range, QString *why)
{
    Grid grid;
    if (!readRangeFile(path, range, &grid, why)) return std::nullopt;
    return rangeObject(grid, why);
}

std::optional<EmbeddedObject> chartObject(const QByteArray &workbook, QString *why)
{
    if (!packageHasChart(workbook)) {
        say(why, QCoreApplication::translate("jp::Objects", "The workbook has no chart."));
        return std::nullopt;
    }
    return make(Kind::Sheet, workbook, QSizeF(kChartWidth, kChartHeight), why);
}

std::optional<EmbeddedObject> chartObjectFromFile(const QString &path, QString *why)
{
    const QByteArray bytes = fileBytes(path, why);
    if (bytes.isEmpty()) return std::nullopt;
    return chartObject(bytes, why);
}

std::optional<EmbeddedObject> chartObjectFromPart(const QByteArray &chartPartXml, QString *why)
{
    Grid none;
    none.push_back({CellValue()});
    const QByteArray book = withChartPart(rangePackage(none), chartPartXml);
    if (book.isEmpty()) {
        say(why, QCoreApplication::translate("jp::Objects", "This can't be put in as an object."));
        return std::nullopt;
    }
    return chartObject(book, why);
}

int pageCountOf(const QByteArray &docx)
{
    QTextDocument doc;
    if (!layOutPages(docx, &doc)) return 0;
    return std::min(std::max(1, doc.pageCount()), kMostPages);
}

std::optional<EmbeddedObject> pageObject(const QByteArray &docx, int page, QString *why)
{
    const int pages = pageCountOf(docx);
    if (pages == 0) {
        say(why, QCoreApplication::translate("jp::Objects", "This is not a document that can be read."));
        return std::nullopt;
    }
    if (page < 1 || page > pages) {
        say(why, QCoreApplication::translate("jp::Objects", "The document ends at page %1.").arg(pages));
        return std::nullopt;
    }
    const QByteArray noted = page == 1 ? docx : withNote(docx, QByteArray::number(page));
    if (noted.isEmpty()) {
        say(why, QCoreApplication::translate("jp::Objects", "This can't be put in as an object."));
        return std::nullopt;
    }
    return make(Kind::Page, noted, QSizeF(kPageWidth, kPageHeight), why);
}

std::optional<EmbeddedObject> pageObjectFromFile(const QString &path, int page, QString *why)
{
    const QByteArray bytes = fileBytes(path, why);
    if (bytes.isEmpty()) return std::nullopt;
    return pageObject(bytes, page, why);
}

namespace {
constexpr char kChunk[] = "joBj";   // (not needed to show the picture, private, and safe to copy)
constexpr int kMostChunks = 4096;

void put32(QByteArray *out, quint32 v)
{
    for (int shift = 24; shift >= 0; shift -= 8) out->append(char((v >> shift) & 0xFF));
}

quint32 get32(const QByteArray &b, qsizetype at)
{
    return (quint32(quint8(b[at])) << 24) | (quint32(quint8(b[at + 1])) << 16) | (quint32(quint8(b[at + 2])) << 8) | quint32(quint8(b[at + 3]));
}
} // namespace

QByteArray carryObject(const EmbeddedObject &object, const QString &token)
{
    const QByteArray &png = object.previewPng;
    // (an IHDR, which is 25 bytes after the 8 of the signature, is what the chunk goes after)
    if (png.size() < 33 || !png.startsWith("\x89PNG\r\n\x1a\n") || png.mid(12, 4) != "IHDR") return QByteArray();
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_6_0);
    s << QByteArray("JOOBJ1") << (token.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : token) << object.progId << qint64(object.widthEmu) << qint64(object.heightEmu) << object.partBytes;
    QByteArray chunk;
    put32(&chunk, quint32(data.size()));
    const QByteArray typed = QByteArray(kChunk, 4) + data;
    chunk += typed;
    put32(&chunk, crc32(typed));
    QByteArray out = png.left(33);
    out += chunk;
    out += png.mid(33);
    return out;
}

std::optional<Carried> carriedObject(const QByteArray &png)
{
    if (png.size() < 45 || !png.startsWith("\x89PNG\r\n\x1a\n")) return std::nullopt;
    qsizetype at = 8;
    for (int n = 0; n < kMostChunks && at + 12 <= png.size(); ++n) {
        const quint32 length = get32(png, at);
        const QByteArray type = png.mid(at + 4, 4);
        if (qint64(length) > qint64(png.size()) || at + 12 + qsizetype(length) > png.size()) return std::nullopt;
        if (type == "IDAT" || type == "IEND") return std::nullopt;   // (the chunk is before the picture's data)
        if (type == kChunk) {
            const QByteArray data = png.mid(at + 8, qsizetype(length));
            if (crc32(png.mid(at + 4, 4 + qsizetype(length))) != get32(png, at + 8 + qsizetype(length))) return std::nullopt;
            QDataStream s(data);
            s.setVersion(QDataStream::Qt_6_0);
            QByteArray magic, part;
            QString token, progId;
            qint64 w = 0, h = 0;
            s >> magic >> token >> progId >> w >> h >> part;
            if (s.status() != QDataStream::Ok || magic != "JOOBJ1") return std::nullopt;
            // (what the object shows is the picture that carries it, without the chunk)
            QByteArray plain = png.left(at) + png.mid(at + 12 + qsizetype(length));
            ObjectBudget budget;
            auto object = keepObject(progId, part, plain, std::clamp<qint64>(w, 0, kMaxEmu), std::clamp<qint64>(h, 0, kMaxEmu), budget);
            if (!object) return std::nullopt;
            return Carried{*object, token};
        }
        at += 12 + qsizetype(length);
    }
    return std::nullopt;
}

QByteArray encode(const EmbeddedObject &object)
{
    QByteArray out;
    QDataStream s(&out, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_6_0);
    s << QByteArray("JOOBJ1") << object.progId << qint64(object.widthEmu) << qint64(object.heightEmu) << object.partBytes << object.previewPng;
    return out;
}

std::optional<EmbeddedObject> decode(const QByteArray &bytes)
{
    if (bytes.size() > limits().objectBytes + limits().pictureBytes + 4096) return std::nullopt;
    QDataStream s(bytes);
    s.setVersion(QDataStream::Qt_6_0);
    QByteArray magic, part, png;
    QString progId;
    qint64 w = 0, h = 0;
    s >> magic >> progId >> w >> h >> part >> png;
    if (s.status() != QDataStream::Ok || magic != "JOOBJ1") return std::nullopt;
    ObjectBudget budget;
    return keepObject(progId, part, png, std::clamp<qint64>(w, 0, kMaxEmu), std::clamp<qint64>(h, 0, kMaxEmu), budget);
}

namespace {

class ObjectMime : public QMimeData {
public:
    explicit ObjectMime(std::function<std::optional<EmbeddedObject>()> make) : m_make(std::move(make)) {}
    QStringList formats() const override { return QMimeData::formats() << QString::fromLatin1(kMime); }
    bool hasFormat(const QString &mimeType) const override { return mimeType == QLatin1String(kMime) || QMimeData::hasFormat(mimeType); }

protected:
    QVariant retrieveData(const QString &mimeType, QMetaType type) const override
    {
        if (mimeType != QLatin1String(kMime)) return QMimeData::retrieveData(mimeType, type);
        if (!m_made) {
            m_made = true;
            if (const auto object = m_make()) m_bytes = encode(*object);
        }
        return m_bytes;
    }

private:
    std::function<std::optional<EmbeddedObject>()> m_make;
    mutable bool m_made = false;
    mutable QByteArray m_bytes;
};

} // namespace

QMimeData *withObject(QMimeData *mime, std::function<std::optional<EmbeddedObject>()> make)
{
    auto *wrapped = new ObjectMime(std::move(make));
    for (const QString &format : mime->formats()) wrapped->setData(format, mime->data(format));
    if (mime->hasImage()) wrapped->setImageData(mime->imageData());
    delete mime;
    return wrapped;
}

bool hasObject(const QMimeData *mime) { return mime && mime->hasFormat(QString::fromLatin1(kMime)); }

std::optional<EmbeddedObject> objectOf(const QMimeData *mime)
{
    if (!hasObject(mime)) return std::nullopt;
    return decode(mime->data(QString::fromLatin1(kMime)));
}

} // namespace jp::objects

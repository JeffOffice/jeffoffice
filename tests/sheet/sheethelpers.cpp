#include "sheethelpers.h"

namespace sheettest {

const QString kMainNs = QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main");
const QString kRelNs = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships");

QByteArray worksheet(const QString &rows, const QString &after, const QString &head)
{
    return ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<worksheet xmlns=\"" + kMainNs + "\" xmlns:r=\"" + kRelNs + "\">" + head
            + "<sheetData>" + rows + "</sheetData>" + after + "</worksheet>").toUtf8();
}

QByteArray sharedStringsXml(const QStringList &items)
{
    QString body;
    for (const QString &item : items) body += "<si>" + item + "</si>";
    return ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<sst xmlns=\"" + kMainNs + "\" count=\"" + QString::number(items.size())
            + "\" uniqueCount=\"" + QString::number(items.size()) + "\">" + body + "</sst>").toUtf8();
}

QByteArray stylesXml(const QString &numFmts, const QString &cellXfs, const QString &after)
{
    return ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<styleSheet xmlns=\"" + kMainNs + "\">"
            + (numFmts.isEmpty() ? QString() : "<numFmts count=\"" + QString::number(numFmts.count(QLatin1String("<numFmt"))) + "\">" + numFmts + "</numFmts>")
            + "<cellStyleXfs count=\"1\"><xf numFmtId=\"9\"/></cellStyleXfs><cellXfs count=\"" + QString::number(cellXfs.count(QLatin1String("<xf"))) + "\">" + cellXfs
            + "</cellXfs>" + after + "</styleSheet>").toUtf8();
}

QByteArray buildXlsx(const QVector<QByteArray> &sheets, const QByteArray &sharedStrings, const QString &workbookPr, const QString &calcPr, const QByteArray &styles,
                     const QStringList &names, const QString &definedNames)
{
    PackageWriter w;
    const QString relBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    QString sheetList;
    w.addPart(QStringLiteral("/xl/workbook.xml"), QByteArray(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    w.addRelationship(QStringLiteral("/"), relBase + "officeDocument", QStringLiteral("/xl/workbook.xml"));
    for (int i = 0; i < sheets.size(); ++i) {
        const QString part = QStringLiteral("/xl/worksheets/sheet%1.xml").arg(i + 1);
        w.addPart(part, sheets[i], QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
        const QString id = w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "worksheet", part);
        const QString name = i < names.size() ? names[i] : QStringLiteral("Sheet%1").arg(i + 1);
        sheetList += QStringLiteral("<sheet name=\"%3\" sheetId=\"%1\" r:id=\"%2\"/>").arg(i + 1).arg(id, name.toHtmlEscaped().replace(QLatin1Char('"'), QLatin1String("&quot;")));
    }
    if (!sharedStrings.isEmpty()) {
        w.addPart(QStringLiteral("/xl/sharedStrings.xml"), sharedStrings, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"));
        w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "sharedStrings", QStringLiteral("/xl/sharedStrings.xml"));
    }
    if (!styles.isEmpty()) {
        w.addPart(QStringLiteral("/xl/styles.xml"), styles, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"));
        w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "styles", QStringLiteral("/xl/styles.xml"));
    }
    const QByteArray workbook = ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<workbook xmlns=\"" + kMainNs + "\" xmlns:r=\"" + kRelNs + "\">"
                                 + (workbookPr.isEmpty() ? QString() : "<workbookPr " + workbookPr + "/>") + "<sheets>" + sheetList + "</sheets>" + (definedNames.isEmpty() ? QString() : "<definedNames>" + definedNames + "</definedNames>")
                                 + (calcPr.isEmpty() ? QString() : "<calcPr " + calcPr + "/>") + "</workbook>").toUtf8();
    w.addPart(QStringLiteral("/xl/workbook.xml"), workbook, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    return w.finish();
}

bool readSheetXml(const QByteArray &xml, Sheet *out, const ReadOptions &options)
{
    QByteArray copy = xml;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    return readSheetPart(&buffer, options, out);
}

const Cell *at(const Sheet &sheet, const QString &ref)
{
    quint32 row = 0, col = 0;
    if (!parseCellRef(ref, &row, &col)) return nullptr;
    return sheet.cells.find(row, col);
}

QString show(const Sheet &sheet, const SharedStrings &shared, const QString &ref)
{
    const Cell *c = at(sheet, ref);
    if (!c) return QStringLiteral("none");
    switch (c->type) {
    case CellType::Empty: return QStringLiteral("-");
    case CellType::Number: return QStringLiteral("n:") + QString::number(c->number, 'g', 17);
    case CellType::SharedString: return QStringLiteral("s:") + sheet.text(*c, shared);
    case CellType::InlineString: return QStringLiteral("i:") + sheet.text(*c, shared);
    case CellType::FormulaString: return QStringLiteral("t:") + sheet.text(*c, shared);
    case CellType::Bool: return QStringLiteral("b:") + QString::number(int(c->number));
    case CellType::Error: return QStringLiteral("e:") + errorText(CellError(int(c->number)));
    case CellType::Date: return QStringLiteral("d:") + QString::number(c->number, 'g', 17);
    }
    return {};
}

} // namespace sheettest

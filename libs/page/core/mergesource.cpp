#include "core/mergesource.h"

#include "calc/numfmt.h"
#include "office/package.h"
#include "sheet/recalc.h"
#include "sheet/sheetreader.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QXmlStreamReader>

namespace jp {

QVector<QStringList> parseDelimited(const QString &text, QChar sep)
{
    QVector<QStringList> rows;
    QStringList row;
    QString cell;
    bool quoted = false;
    for (int i = 0; i < text.size(); ++i) {
        const QChar ch = text[i];
        if (quoted) {
            if (ch == '"') {
                if (i + 1 < text.size() && text[i + 1] == '"') { cell += '"'; ++i; }
                else quoted = false;
            } else cell += ch;
            continue;
        }
        if (ch == '"') quoted = true;
        else if (ch == sep) { row << cell; cell.clear(); }
        else if (ch == '\n' || ch == '\r') {
            if (ch == '\r' && i + 1 < text.size() && text[i + 1] == '\n') ++i;
            row << cell;
            cell.clear();
            if (!(row.size() == 1 && row[0].isEmpty())) rows << row;
            row.clear();
        } else cell += ch;
    }
    if (!cell.isEmpty() || !row.isEmpty()) { row << cell; rows << row; }
    return rows;
}

// A spreadsheet the workbook reader can't take (a package with no workbook part, say) is read for its first sheet's cells alone.
static QVector<QStringList> readXlsx(const QByteArray &data)
{
    office::Package package;
    package.read(data);
    struct {
        QByteArray value(const char *name) const { return package->part(QStringLiteral("/") + QLatin1String(name)); }
        const office::Package *package;
    } files{&package};
    QStringList shared;
    {
        QXmlStreamReader r(files.value("xl/sharedStrings.xml"));
        QString cur;
        bool inSi = false;
        while (!r.atEnd()) {
            r.readNext();
            if (r.isStartElement() && r.name() == u"si") { inSi = true; cur.clear(); }
            else if (r.isStartElement() && r.name() == u"t" && inSi) cur += r.readElementText();
            else if (r.isEndElement() && r.name() == u"si") { shared << cur; inSi = false; }
        }
    }
    QVector<QStringList> rows;
    QXmlStreamReader r(files.value("xl/worksheets/sheet1.xml"));
    QStringList row;
    int rowIndex = -1;
    QString cellRef, cellType;
    // A column from its letters ("C" is 2), or -1 past the 16,384 columns a
    // spreadsheet has (the row is padded up to the column).
    auto colOf = [](const QString &ref) {
        int c = 0;
        for (QChar ch : ref) {
            const QChar u = ch.toUpper();
            if (u < QLatin1Char('A') || u > QLatin1Char('Z')) break;
            c = c * 26 + (u.unicode() - 'A' + 1);
            if (c > 16384) return -1;
        }
        return c - 1;
    };
    while (!r.atEnd()) {
        r.readNext();
        if (r.isStartElement() && r.name() == u"row") { row.clear(); ++rowIndex; }
        else if (r.isStartElement() && r.name() == u"c") { cellRef = r.attributes().value("r").toString(); cellType = r.attributes().value("t").toString(); }
        else if (r.isStartElement() && (r.name() == u"v" || r.name() == u"t")) {
            QString v = r.readElementText();
            if (cellType == "s") v = shared.value(v.toInt());
            const int col = colOf(cellRef);
            while (row.size() < col) row << QString();
            if (col >= 0 && col == row.size()) row << v; else if (col >= 0 && col < row.size()) row[col] = v;
        } else if (r.isEndElement() && r.name() == u"row") rows << row;
    }
    return rows;
}

bool readSpreadsheetRows(const QString &path, QVector<QStringList> *rows, QString *error)
{
    namespace sh = jp::office::sheet;
    sh::Workbook book;
    QString why;
    if (!book.openFile(path, &why)) {
        if (path.endsWith(QLatin1String(".xlsx"), Qt::CaseInsensitive)) {
            QFile f(path);
            if (f.open(QIODevice::ReadOnly)) *rows = readXlsx(f.readAll());
            if (!rows->isEmpty()) return true;
        }
        if (error) *error = why;
        return false;
    }
    sh::WorkbookSource src(book);
    src.loadAll();
    if (src.sheetCount() < 1) {
        if (error) *error = QCoreApplication::translate("Import", "The file has no sheets.");
        return false;
    }
    quint32 nrows = 0, ncols = 0;
    src.usedExtent(0, &nrows, &ncols);
    // (A list is a few thousand rows of a few dozen columns; a sheet with more is read to these bounds.)
    nrows = std::min<quint32>(nrows, 1000000);
    ncols = std::min<quint32>(ncols, 1000);
    const jp::calc::NumberSymbols symbols = jp::calc::NumberSymbols::of(QLocale());
    QHash<quint32, jp::calc::NumberFormat> formats;
    for (quint32 r = 0; r < nrows; ++r) {
        QStringList row;
        for (quint32 c = 0; c < ncols; ++c) {
            const jp::calc::Value v = src.cell(0, r, c);
            QString text;
            switch (v.kind()) {
            case jp::calc::Value::Kind::Text: text = v.text(); break;
            case jp::calc::Value::Kind::Bool: text = v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE"); break;
            case jp::calc::Value::Kind::Error: text = jp::calc::errorKindText(v.errorKind()); break;
            case jp::calc::Value::Kind::Number: {
                const sh::Cell *cell = src.sheet(0).cells.find(r, c);
                const quint32 style = cell ? cell->style : 0;
                auto it = formats.find(style);
                if (it == formats.end()) it = formats.insert(style, jp::calc::NumberFormat(book.styles().formatCode(style, QLocale())));
                const jp::calc::Formatted f = it->format(v.number(), book.info().dateSystem, 0, &symbols);
                text = f.failed ? jp::calc::numberToText(v.number()) : f.text;
                break;
            }
            default: break;
            }
            row << text;
        }
        while (!row.isEmpty() && row.last().isEmpty()) row.removeLast();
        rows->push_back(row);
    }
    return true;
}

static QVector<QStringList> readVcf(const QString &text, QStringList *fields)
{
    *fields = {"First Name", "Last Name", "Company", "Address Line 1", "City", "State", "ZIP Code", "Country", "Email", "Phone"};
    QVector<QStringList> rows;
    QStringList cur;   // the card being read; empty outside one
    for (QString line : text.split('\n')) {
        line = line.trimmed();
        if (line.startsWith("BEGIN:VCARD", Qt::CaseInsensitive)) cur = QStringList(fields->size());
        else if (cur.isEmpty()) continue;   // anything outside BEGIN/END belongs to no card
        else if (line.startsWith("END:VCARD", Qt::CaseInsensitive)) { rows << cur; cur.clear(); }
        else if (line.startsWith("N:") || line.startsWith("N;")) {
            const QStringList p = line.section(':', 1).split(';');
            cur[1] = p.value(0); cur[0] = p.value(1);
        } else if (line.startsWith("ORG")) cur[2] = line.section(':', 1).section(';', 0, 0);
        else if (line.startsWith("ADR")) {
            const QStringList p = line.section(':', 1).split(';');
            cur[3] = p.value(2); cur[4] = p.value(3); cur[5] = p.value(4); cur[6] = p.value(5); cur[7] = p.value(6);
        } else if (line.startsWith("EMAIL")) cur[8] = line.section(':', 1);
        else if (line.startsWith("TEL") && cur[9].isEmpty()) cur[9] = line.section(':', 1);
    }
    return rows;
}

bool loadMergeSource(const QString &path, MergeSource *out, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error) *error = f.errorString();
        return false;
    }
    const QByteArray data = f.readAll();
    const QString suffix = QFileInfo(path).suffix().toLower();
    QVector<QStringList> rows;
    MergeSource m;
    m.path = path;
    if (suffix == "xlsx" || suffix == "xlsm" || suffix == "xls" || suffix == "ods" || suffix == "fods") {
        if (!readSpreadsheetRows(path, &rows, error)) return false;
    }
    else if (suffix == "vcf" || suffix == "vcard") rows = readVcf(QString::fromUtf8(data), &m.fields);
    else {
        QString text = QString::fromUtf8(data);
        if (text.startsWith(QChar(0xFEFF))) text.remove(0, 1);
        const QString first = text.section('\n', 0, 0);
        const QChar sep = suffix == "tsv" || first.count('\t') > first.count(',') ? QChar('\t') : first.count(';') > first.count(',') ? QChar(';') : QChar(',');
        rows = parseDelimited(text, sep);
    }
    if (m.fields.isEmpty()) {
        if (rows.isEmpty()) {
            if (error) *error = QCoreApplication::translate("Import", "The file has no rows.");
            return false;
        }
        m.fields = rows.takeFirst();
        for (QString &fl : m.fields) fl = fl.trimmed();
    }
    for (QStringList &r : rows) {
        if (r.join(QString()).isEmpty()) continue;   // (a blank line of a sheet)
        while (r.size() < m.fields.size()) r << QString();
        m.rows << r;
        m.include << true;
    }
    *out = m;
    return true;
}

bool saveMergeCsv(const MergeSource &m, const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;
    auto esc = [](QString s) {
        if (s.contains(',') || s.contains('"') || s.contains('\n')) s = '"' + s.replace("\"", "\"\"") + '"';
        return s;
    };
    QStringList lines;
    QStringList head;
    for (const auto &h : m.fields) head << esc(h);
    lines << head.join(',');
    for (const auto &r : m.rows) {
        QStringList cells;
        for (const auto &c : r) cells << esc(c);
        lines << cells.join(',');
    }
    f.write(lines.join("\r\n").toUtf8());
    return true;
}

} // namespace jp

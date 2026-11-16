#include "sheet/pivotexternal.h"

#include "calc/numfmt.h"
#include "calc/value.h"
#include "sheet/pivotbook.h"
#include "sheet/sheetreader.h"
#include "sheet/styles.h"

#include <QDateTime>
#include <QFileInfo>
#include <QHash>
#include <QLocale>
#include <QUrl>

#include <memory>

namespace jp::office::sheet {

namespace {

constexpr qint64 kReadMillis = 30000;

struct Held {
    Workbook book;
    Sheet sheet;
    QHash<quint32, bool> dateStyle;
};

QString unquoted(QString name)
{
    if (name.size() > 1 && name.startsWith(QLatin1Char('\'')) && name.endsWith(QLatin1Char('\''))) name = name.mid(1, name.size() - 2).replace(QLatin1String("''"), QLatin1String("'"));
    return name;
}

} // namespace

QString pivotExternalAddress(const QString &path)
{
    return QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath()).toString(QUrl::FullyEncoded);
}

bool readPivotExternalSource(const QString &path, const QString &where, PivotSource *source, QString *sheetOut, QString *refOut, QString *nameOut, QString *why)
{
    auto fail = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    const QFileInfo info(path);
    if (!info.isFile()) return fail(QStringLiteral("the file is not there"));
    auto held = std::make_shared<Held>();
    const qint64 deadline = QDateTime::currentMSecsSinceEpoch() + kReadMillis;
    Limits limits;
    limits.cancelled = [deadline] { return QDateTime::currentMSecsSinceEpoch() > deadline; };
    QString error;
    if (!held->book.openFile(path, &error, limits)) return fail(error.isEmpty() ? QStringLiteral("the file can't be opened") : error);
    const auto &sheets = held->book.info().sheets;
    auto usable = [&](int s) { return s >= 0 && s < sheets.size() && !sheets[s].deleted && sheets[s].kind == SheetKind::Worksheet; };
    const QString text = where.trimmed();
    if (text.isEmpty()) return fail(QStringLiteral("no range"));
    int sheet = -1;
    CellRange range;
    QString name;
    const int bang = text.lastIndexOf(QLatin1Char('!'));
    QString refPart = text;
    if (bang > 0) {
        const QString sheetName = unquoted(text.left(bang));
        for (int s = 0; s < sheets.size(); ++s)
            if (usable(s) && sheets[s].name.compare(sheetName, Qt::CaseInsensitive) == 0) sheet = s;
        if (sheet < 0) return fail(QStringLiteral("the file has no sheet called \"%1\"").arg(sheetName));
        refPart = text.mid(bang + 1);
    } else {
        for (int s = 0; s < sheets.size() && sheet < 0; ++s)
            if (usable(s)) sheet = s;
    }
    refPart.remove(QLatin1Char('$'));
    if (parseRangeRef(refPart, &range)) {
        if (sheet < 0) return fail(QStringLiteral("the file has no sheet of cells"));
    } else {
        if (bang > 0 || !pivotNameSource(held->book, text, &sheet, &range)) return fail(QStringLiteral("the file has no range, table, or name \"%1\"").arg(text));
        name = text;
    }
    if (range.row2 <= range.row1) return fail(QStringLiteral("the data needs a row of names and at least one row of data"));
    if (range.col2 - range.col1 + 1 > quint32(kPivotMaxFields) || range.row2 - range.row1 > quint32(kPivotMaxRecords)) return fail(QStringLiteral("there is too much data"));
    if (qint64(range.row2 - range.row1) * qint64(range.col2 - range.col1 + 1) > kPivotMaxCreateCells) return fail(QStringLiteral("there is too much data"));
    const auto progress = [deadline](qint64) { return QDateTime::currentMSecsSinceEpoch() <= deadline; };
    if (!held->book.readSheet(sheet, &held->sheet, progress)) return fail(QStringLiteral("the sheet can't be read"));
    source->row1 = range.row1;
    source->col1 = range.col1;
    source->row2 = range.row2;
    source->col2 = range.col2;
    source->dates = held->book.info().dateSystem;
    source->numFmtIds.clear();
    for (quint32 c = range.col1; c <= range.col2; ++c) {
        const Cell *first = held->sheet.cells.find(range.row1 + 1, c);
        source->numFmtIds.append(first ? int(held->book.styles().numFmtId(first->style)) : 0);
    }
    source->cell = [held](quint32 r, quint32 c) {
        PivotValue pv;
        const Cell *cell = held->sheet.cells.find(r, c);
        if (!cell) return pv;
        switch (cell->type) {
        case CellType::Empty: break;
        case CellType::Number:
        case CellType::Date: {
            bool date = cell->type == CellType::Date;
            if (!date) {
                auto it = held->dateStyle.find(cell->style);
                if (it == held->dateStyle.end()) it = held->dateStyle.insert(cell->style, jp::calc::NumberFormat(held->book.styles().formatCode(cell->style, QLocale::c())).kind() == jp::calc::FormatKind::Date);
                date = it.value();
            }
            pv.kind = date ? PivotValue::Date : PivotValue::Number;
            pv.number = cell->number;
            break;
        }
        case CellType::SharedString:
            pv.kind = PivotValue::Text;
            pv.text = held->book.sharedStrings().at(cell->textIndex());
            break;
        case CellType::InlineString:
        case CellType::FormulaString:
            pv.kind = PivotValue::Text;
            pv.text = held->sheet.texts.value(cell->textIndex());
            break;
        case CellType::Bool: pv.kind = PivotValue::Bool; pv.number = cell->number != 0 ? 1 : 0; break;
        case CellType::Error: pv.kind = PivotValue::Error; pv.text = jp::calc::errorKindText(jp::calc::ErrorKind(std::max(0, std::min(int(jp::calc::ErrorKind::Unknown), int(cell->number))))); break;
        }
        return pv;
    };
    if (sheetOut) *sheetOut = name.isEmpty() ? sheets[sheet].name : QString();
    if (refOut) *refOut = name.isEmpty() ? pivotRangeRef(range.row1, range.col1, range.row2, range.col2) : QString();
    if (nameOut) *nameOut = name;
    return true;
}

} // namespace jp::office::sheet

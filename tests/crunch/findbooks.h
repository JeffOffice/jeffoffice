// What the tests of Find, Sort, Filter, and Go To share: workbooks of several sheets built from cell texts (as sheetOf does for one), with styles for a date (1), a percent (2),
// a fill in red (3), a font in blue (4), and a number with a comma (5).
#pragma once

#include "editwindow.h"

#include <QDir>
#include <QWidget>

#include <climits>

namespace crunchtest {

// With the environment variable JC_SHOT_DIR set, a picture of the widget is saved there as <name>.png (for looking at the windows; a test never depends on it).
inline void saveShot(QWidget *w, const QString &name)
{
    const QString dir = QString::fromLocal8Bit(qgetenv("JC_SHOT_DIR"));
    if (dir.isEmpty() || !w) return;
    QDir().mkpath(dir);
    QCoreApplication::processEvents();
    w->grab().save(dir + QLatin1Char('/') + name + QStringLiteral(".png"));
}

// The rows of a sheet from the cells' texts (see sheetOf); a spec may end with "~n" to give the cell the style n.
inline QString rowsOfSpecs(const QStringList &specs, const QMap<int, QString> &rowAttrs = QMap<int, QString>())
{
    struct Item {
        quint32 row, col;
        QString xml;
    };
    QVector<Item> items;
    for (QString spec : specs) {
        int style = -1;
        const int tilde = int(spec.lastIndexOf(QLatin1Char('~')));
        if (tilde > 0 && spec.mid(tilde + 1).size() == 1 && spec.at(tilde + 1).isDigit()) {
            style = spec.mid(tilde + 1).toInt();
            spec.truncate(tilde);
        }
        const int eq = int(spec.indexOf(QLatin1Char('=')));
        const QString ref = spec.left(eq), value = spec.mid(eq + 1);
        quint32 row = 0, col = 0;
        parseCellRef(ref, &row, &col);
        QString s = style >= 0 ? QStringLiteral(" s=\"%1\"").arg(style) : QString();
        QString xml;
        if (value.startsWith(QLatin1Char('='))) {
            xml = QStringLiteral("<c r=\"%1\"%3><f>%2</f><v>0</v></c>").arg(ref, value.mid(1).toHtmlEscaped(), s);
        } else if (value.startsWith(QLatin1Char('@'))) {
            const QDate d = QDate::fromString(value.mid(1), Qt::ISODate);
            double serial = 0;
            jp::calc::dateToSerial(d.year(), d.month(), d.day(), jp::calc::DateSystem::Date1900, &serial);
            xml = QStringLiteral("<c r=\"%1\" s=\"%3\"><v>%2</v></c>").arg(ref).arg(serial, 0, 'f', 0).arg(style >= 0 ? style : 1);
        } else if (value.startsWith(QLatin1Char('%'))) {
            xml = QStringLiteral("<c r=\"%1\" s=\"%3\"><v>%2</v></c>").arg(ref, value.mid(1)).arg(style >= 0 ? style : 2);
        } else if (value.isEmpty()) {
            xml = QStringLiteral("<c r=\"%1\"%2/>").arg(ref, s);
        } else {
            bool number = false;
            value.toDouble(&number);
            xml = number ? QStringLiteral("<c r=\"%1\"%3><v>%2</v></c>").arg(ref, value, s)
                         : QStringLiteral("<c r=\"%1\"%3 t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, value.toHtmlEscaped(), s);
        }
        items.append({row, col, xml});
    }
    std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
    QString rows;
    int i = 0;
    QMap<int, QString> pendingRows = rowAttrs;
    while (i < items.size() || !pendingRows.isEmpty()) {
        const int nextCell = i < items.size() ? int(items[i].row) + 1 : INT_MAX;
        const int nextAttr = pendingRows.isEmpty() ? INT_MAX : pendingRows.firstKey();
        const int row = std::min(nextCell, nextAttr);
        rows += QStringLiteral("<row r=\"%1\" %2>").arg(row).arg(pendingRows.take(row));
        for (; i < items.size() && int(items[i].row) + 1 == row; ++i) rows += items[i].xml;
        rows += QStringLiteral("</row>");
    }
    return rows;
}

inline QByteArray findStyles()
{
    return stylesXml(QString(),
                     QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"14\" applyNumberFormat=\"1\"/><xf numFmtId=\"9\" applyNumberFormat=\"1\"/><xf numFmtId=\"0\" fillId=\"2\" applyFill=\"1\"/>"
                                   "<xf numFmtId=\"0\" fontId=\"1\" applyFont=\"1\"/><xf numFmtId=\"4\" applyNumberFormat=\"1\"/>"),
                     QStringLiteral("<fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font><font><sz val=\"11\"/><color rgb=\"FF0000FF\"/><name val=\"Calibri\"/></font></fonts>"
                                    "<fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"
                                    "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFFF0000\"/><bgColor indexed=\"64\"/></patternFill></fill></fills>"));
}

// A workbook of sheets, each a list of cell texts; `after` is what follows the cells of the sheet (its protection, filter, ...) by the sheet's place.
// `rowAttrs` gives attributes to rows of the first sheet by their number from 1 ("hidden=\"1\""); `names` the inside of definedNames.
inline QByteArray bookOfSheets(const QVector<QStringList> &sheets, const QMap<int, QString> &after = QMap<int, QString>(), const QMap<int, QString> &rowAttrs = QMap<int, QString>(),
                               const QString &names = QString())
{
    QVector<QByteArray> parts;
    for (int i = 0; i < sheets.size(); ++i) parts << worksheet(rowsOfSpecs(sheets[i], i == 0 ? rowAttrs : QMap<int, QString>()), after.value(i));
    return buildXlsx(parts, QByteArray(), QString(), QString(), findStyles(), QStringList(), names);
}

} // namespace crunchtest

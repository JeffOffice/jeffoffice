// Spreadsheets in the OpenDocument form, built in memory for the tests of its reader: the XML of content.xml, styles.xml, and settings.xml put together by small functions, in a ZIP
// (or in one flat XML document). Nothing here reads a file made by another program.
#pragma once

#include "sheethelpers.h"

namespace odstest {

// The namespaces a document declares (the standard's own prefixes).
QString namespaces();

// <table:table> with a name and the inside of the table (its columns and rows); `attrs` are more attributes of the element.
QString table(const QString &name, const QString &inner, const QString &attrs = QString());
// Rows and cells. `attrs` are more attributes of the element.
QString row(const QString &cells, const QString &attrs = QString());
QString columns(int count, const QString &attrs = QString());   // <table:table-column table:number-columns-repeated=count/>
QString floatCell(double v, const QString &attrs = QString());
QString stringCell(const QString &text, const QString &attrs = QString());
QString emptyCell(const QString &attrs = QString());
// A cell with any attributes and any inside.
QString cell(const QString &attrs, const QString &inside = QString());

// The content.xml: the document root with its namespaces, `automaticStyles` the inside of office:automatic-styles, `fontFaces` the inside of office:font-face-decls, `tables` the
// inside of office:spreadsheet, and `before` what goes in office:spreadsheet before the tables (the calculation settings).
QByteArray contentXml(const QString &tables, const QString &automaticStyles = QString(), const QString &before = QString(), const QString &fontFaces = QString());
// settings.xml with the view settings of the sheets: each entry of `sheets` is the sheet's name and the items of its view (name -> value, all as integers or text); `active` is the sheet open.
QByteArray settingsXml(const QMap<QString, QMap<QString, QString>> &sheets, const QString &active = QString(), const QMap<QString, QString> &view = QMap<QString, QString>());

// A package: mimetype first and stored, content.xml, and the parts given (styles.xml with `commonStyles` as the inside of office:styles, settings.xml, other parts by name).
QByteArray buildOds(const QByteArray &content, const QString &commonStyles = QString(), const QByteArray &settings = QByteArray(), const QMap<QString, QByteArray> &extra = QMap<QString, QByteArray>(),
                    const QString &mimetype = QStringLiteral("application/vnd.oasis.opendocument.spreadsheet"), const QByteArray &manifest = QByteArray());
// The flat XML form: one document with the styles, the content, and the settings in it.
QByteArray buildFods(const QString &tables, const QString &automaticStyles = QString(), const QString &commonStyles = QString(), const QString &before = QString());

// Opens the bytes as a workbook through a file (as the apps do); `name` ends as the kind of file does.
bool openBytes(const QByteArray &bytes, const QString &name, Workbook *out, QString *why, const Limits &limits = Limits());

// The cell's text in the model's own words: "n:1.5", "s:text" for a text (inline or shared), "t:text" for a formula's text, "b:1", "e:#DIV/0!", "-" empty, "none" no cell.
QString shown(const Workbook &wb, const Sheet &s, const QString &ref);
// The formula of the cell as the model keeps it (empty for none).
QString formulaOf(const Sheet &s, const QString &ref);

} // namespace odstest

using namespace odstest;

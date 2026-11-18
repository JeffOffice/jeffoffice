#pragma once
// Objects placed between apps, as a person sees them: where one comes from (a range or chart of a workbook, a page of a document), the picture each kind shows, the words that name a
// kind, and the clipboard form that carries one. The object itself and its caps are libs/office/ole/embedobj.h; editing one in its own program is libs/shell/app/embeddededit.h. The
// pictures are drawn here from the packages themselves (nothing of an app is included), in the shape the object core gives a callback: the bytes, the size, and the scale.
//
// A range is kept as a small workbook of the values the range shows; a chart as the workbook it is in; a page as the document it is in, with a note of which page (a small part
// the packages of other programs do not read: a program that saves the document again drops it, and the picture then shows the first page).

#include "app/embeddededit.h"
#include "core/export.h"
#include "ole/embedobj.h"

#include <QByteArray>
#include <QImage>
#include <QMimeData>
#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>
#include <optional>

namespace jp::objects {

// What a range picks up: no more rows and columns than this, and no more text in a cell.
constexpr int kMostRows = 100;
constexpr int kMostColumns = 26;
constexpr int kMostCellText = 200;
// The pages a document is looked at for (a document of more is shown up to this).
constexpr int kMostPages = 500;

struct CellValue {
    QString text;           // what the cell shows
    bool number = false;    // a number (shown to the right)
    double value = 0;
};
using Grid = QVector<QVector<CellValue>>;

// ---- what a kind is called (never a program's name)
JO_COMMON_EXPORT QString kindWords(const office::ole::EmbeddedObject &object);
// A workbook that holds a chart in it (the object of a chart is the workbook).
JO_COMMON_EXPORT bool packageHasChart(const QByteArray &package);

// ---- pictures: what the object core's callback is, one for each kind
JO_COMMON_EXPORT QImage sheetPicture(const QByteArray &package, qint64 widthEmu, qint64 heightEmu, int scale);
JO_COMMON_EXPORT QImage pagePicture(const QByteArray &package, qint64 widthEmu, qint64 heightEmu, int scale);
JO_COMMON_EXPORT office::ole::PictureFn pictureFor(office::ole::Kind kind);
// Opens the package in the app that made that kind (the program of the same folder as this one); false when that program is not there.
JO_COMMON_EXPORT bool openInOwnApp(const QString &path, office::ole::Kind kind);
// Gives an editor these pictures for both kinds, to draw an object again after a save.
JO_COMMON_EXPORT void usePictures(EmbeddedEdit *edit);

// ---- where objects come from; each gives nothing, with `why` saying so in a sentence, when the source is not usable
// The cells of a sheet of a workbook as they show: the range (A1:C5; empty: the cells the sheet uses, up to the most). The cells are in `grid`.
JO_COMMON_EXPORT bool readRange(const QByteArray &workbook, const QString &range, Grid *grid, QString *why);
JO_COMMON_EXPORT bool readRangeFile(const QString &path, const QString &range, Grid *grid, QString *why);
JO_COMMON_EXPORT std::optional<office::ole::EmbeddedObject> rangeObject(const Grid &grid, QString *why = nullptr);
JO_COMMON_EXPORT std::optional<office::ole::EmbeddedObject> rangeObjectFromFile(const QString &path, const QString &range, QString *why = nullptr);
// The first chart of a workbook, with the workbook that holds it.
JO_COMMON_EXPORT std::optional<office::ole::EmbeddedObject> chartObjectFromFile(const QString &path, QString *why = nullptr);
JO_COMMON_EXPORT std::optional<office::ole::EmbeddedObject> chartObject(const QByteArray &workbook, QString *why = nullptr);
// A chart of a chart part (the XML), with a workbook made to hold it: the object a chart copied in the spreadsheet program is.
JO_COMMON_EXPORT std::optional<office::ole::EmbeddedObject> chartObjectFromPart(const QByteArray &chartPartXml, QString *why = nullptr);
// A page (from 1) of a document: the file, or its bytes (a .docx).
JO_COMMON_EXPORT std::optional<office::ole::EmbeddedObject> pageObjectFromFile(const QString &path, int page, QString *why = nullptr);
JO_COMMON_EXPORT std::optional<office::ole::EmbeddedObject> pageObject(const QByteArray &docx, int page, QString *why = nullptr);
// How many pages the document has when laid out for a picture, 0 if it can't be read.
JO_COMMON_EXPORT int pageCountOf(const QByteArray &docx);
// The cells of the text a copy of cells puts on the clipboard (a tab between cells, a line for each row; a cell in quotes is as it is inside them), no more than the most a range picks up.
JO_COMMON_EXPORT Grid gridFromText(const QString &text);
// The values of a range written as a workbook (the package a range object is), and the size in points the range's picture is made at.
JO_COMMON_EXPORT QByteArray rangePackage(const Grid &grid);
JO_COMMON_EXPORT QSizeF rangeSize(const Grid &grid);

// ---- an object kept in a picture: for a program whose pictures are the objects it can place (a sheet's drawing), the object is its picture with a private chunk of the PNG that holds the
// package, which a program that does not know the object shows as the picture and which every save of the picture keeps byte for byte. `token` names the object across saves (made when
// empty); the picture is the object's own, so the result is the PNG a window gives its sheet.
struct Carried {
    office::ole::EmbeddedObject object;
    QString token;
};
JO_COMMON_EXPORT QByteArray carryObject(const office::ole::EmbeddedObject &object, const QString &token = QString());
JO_COMMON_EXPORT std::optional<Carried> carriedObject(const QByteArray &png);

// ---- the clipboard
constexpr char kMime[] = "application/x-jeffoffice-object";
JO_COMMON_EXPORT QByteArray encode(const office::ole::EmbeddedObject &object);
JO_COMMON_EXPORT std::optional<office::ole::EmbeddedObject> decode(const QByteArray &bytes);
// Adds the object to a copy: it is made when something asks for it (a paste as an object), not at every copy.
JO_COMMON_EXPORT QMimeData *withObject(QMimeData *mime, std::function<std::optional<office::ole::EmbeddedObject>()> make);
JO_COMMON_EXPORT bool hasObject(const QMimeData *mime);
JO_COMMON_EXPORT std::optional<office::ole::EmbeddedObject> objectOf(const QMimeData *mime);

} // namespace jp::objects

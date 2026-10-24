#pragma once
// The text of a header or a footer as a sheet keeps it, with the & codes of the format, turned into what is drawn: three parts (left, center, and right), each some lines, each line
// some runs of text in one font and style, or one of the fields that are worked out for each page. No drawing: the page painter lays the runs out.
//
//   &L &C &R   the part the text that follows goes in        &P   the page's number              &N   the number of pages
//   &D         the date                                       &T   the time                       &F   the file's name
//   &Z         the folder the file is in                      &A   the sheet's name               &G   a picture (not drawn yet)
//   &B &I &U   bold, italic, underline, each on and off       &E   double underline               &S   strikethrough
//   &X &Y      superscript and subscript                      &"font,style"  the font and its style (a "-" for the font keeps the default)
//   &12        the size in points                             &KRRGGBB       the color         &&   an ampersand
//
// A code the format has no meaning for is passed over, as is an & at the end. Every length is bounded: the string, the codes in it, the runs, and the lines of a part.

#include "sheet/pagesetup.h"

#include <QColor>
#include <QDateTime>
#include <QLocale>
#include <QString>
#include <QVector>

namespace jp::office::sheet {

constexpr int kMaxHeaderFooterCodes = 512;
constexpr int kMaxHeaderFooterRuns = 256;
constexpr int kMaxHeaderFooterLines = 32;

enum class HfField : quint8 { None, Page, Pages, Date, Time, File, Path, Sheet, Picture };
enum class HfUnderline : quint8 { None, Single, Double };

struct HfRun {
    QString text;                 // for a run of text; empty for a field
    HfField field = HfField::None;
    QString font;                 // the family from &"font,style"; empty for the default
    double size = 0;              // points; 0 for the default
    bool bold = false, italic = false, strike = false, superscript = false, subscript = false;
    HfUnderline underline = HfUnderline::None;
    bool hasColor = false;
    QColor color;
    bool operator==(const HfRun &o) const
    {
        return text == o.text && field == o.field && font == o.font && size == o.size && bold == o.bold && italic == o.italic && strike == o.strike && superscript == o.superscript
               && subscript == o.subscript && underline == o.underline && hasColor == o.hasColor && (!hasColor || color == o.color);
    }
};

using HfLine = QVector<HfRun>;

struct HfPart {
    QVector<HfLine> lines;
    bool isEmpty() const { return lines.isEmpty(); }
};

struct HfText {
    HfPart left, center, right;
    bool hasPicture = false;      // a picture was asked for (&G): nothing is drawn for it yet
    bool isEmpty() const { return left.isEmpty() && center.isEmpty() && right.isEmpty(); }
};

// The runs of a header or footer string.
HfText parseHeaderFooter(const QString &code);

// What the fields are worked out from on a page.
struct HfContext {
    int page = 1;
    int pages = 1;
    QDateTime now;
    QString file;                 // the file's name, without the folder
    QString path;                 // the folder, ending in a separator
    QString sheet;
    QLocale locale;
};
// The text of a field on a page; empty for a picture and for no field.
QString hfFieldText(HfField field, const HfContext &context);

// A text typed by a person as it goes in a header or footer string: each & doubled.
QString escapeHeaderFooterText(const QString &plain);
// The three parts joined as the string the file keeps: "&Lleft&Ccenter&Rright" (a part that is empty is left out).
QString composeHeaderFooter(const QString &left, const QString &center, const QString &right);
// The string split into its three parts, each as the codes it holds (the opposite of composeHeaderFooter). A string with no part code is all center text, as the format reads it.
void splitHeaderFooter(const QString &code, QString *left, QString *center, QString *right);
// The words of a string as a person reads them in the sheet: its text with each field shown as it comes out for the page (a preview line of the dialog).
QString headerFooterPreview(const QString &code, const HfContext &context);

} // namespace jp::office::sheet

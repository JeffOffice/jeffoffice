#pragma once
// The styles of an OpenDocument spreadsheet, as the reader collects them from the XML (ods.cpp) and turns them into the model's: a cell style (with the named styles it follows) into a
// cell format with its font, fill, border, and alignment; a data style (number, percentage, currency, date, time, text) into a number format code; a column's or row's style into a
// size. Nothing here reads XML. An attribute is kept under a key made of the kind of properties it came from and its qualified name in the standard's own prefixes
// ("cell:fo:border-left", "text:fo:font-weight", "para:fo:text-align", "col:style:column-width", "row:style:row-height", "table:table:display"), whatever prefix the file chose.

#include "sheet/sheetreader.h"
#include "sheet/styles.h"

#include <QHash>
#include <QString>
#include <QVector>

#include <memory>

namespace jp::office::sheet::ods {

// A length ("2.5cm", "10pt", "0.5in", "12px", "1pc", "3mm") in points; false if it is none.
bool lengthPoints(const QString &text, double *points);
// "#rrggbb" as 0xRRGGBB; false for anything else (and for "transparent").
bool colorOf(const QString &text, quint32 *rgb);
// The short prefix the standard gives a namespace ("fo", "style", "svg", "number", "table", "text", "office", "draw", "config", "calcext"...); the local name alone for another.
QString canonicalName(const QString &namespaceUri, const QString &localName);

using Props = QHash<QString, QString>;

struct StyleDef {
    QString family;       // "table-cell", "table-column", "table-row", "table", "text", "paragraph"
    QString name;
    QString parent;
    QString dataStyle;    // a cell style's number format
    Props props;
};

// One thing a data style shows, in order.
struct DataPart {
    enum Kind : quint8 { Number, Scientific, Fraction, Text, CurrencySymbol, Day, Month, Year, Era, DayOfWeek, WeekOfYear, Quarter, Hours, Minutes, Seconds, AmPm, TextContent, Boolean } kind = Text;
    QString text;                 // for Text and CurrencySymbol
    int decimals = 0;             // Number, Scientific; the digits of seconds
    int minDecimals = -1;         // Number: the decimals that are always shown (else all of `decimals`)
    int minInteger = 1;           // Number, Scientific, Fraction
    int minExponent = 1;          // Scientific
    int minNumerator = 1, minDenominator = 1, denominatorValue = 0;   // Fraction
    bool grouping = false;        // Number
    int displayFactor = 1;        // Number: shown divided by this
    bool longStyle = false;       // Day, Month, Year, DayOfWeek, Hours, Minutes, Seconds: two digits, or the full name
    bool textual = false;         // Month: the name
    struct Embedded {
        int position = 0;
        QString text;
    };
    QVector<Embedded> embedded;   // Number: text among the digits
};

struct DataStyle {
    QString name;
    QString kind;                 // "number", "percentage", "currency", "date", "time", "boolean", "text"
    QVector<DataPart> parts;
    quint32 color = 0xFFFFFFFF;   // the text color the style asks for (0xRRGGBB), else none
    bool truncateOnOverflow = true;   // a time style: false shows the hours (and minutes) as elapsed, past 24 and 60
    struct Map {
        QString condition;        // "value()>=0"
        QString applyStyle;
    };
    QVector<Map> maps;
};

class StyleStore {
public:
    void addFont(const QString &name, const QString &family) { m_fonts.insert(name, family); }
    void setDefault(const QString &family, const Props &props) { m_defaults.insert(family, props); }
    // A later definition of a name replaces an earlier (the automatic styles of content.xml come after the common styles of styles.xml).
    void addStyle(const StyleDef &def) { m_styles.insert(def.family + QLatin1Char('\n') + def.name, def); }
    void addDataStyle(const DataStyle &style) { m_data.insert(style.name, style); }
    qint64 styleCount() const { return m_styles.size(); }

    // All the properties a style has, its parents' under its own (a parent that is not there, or a chain that loops or is longer than 32, ends it). For the family "table-cell" the
    // document's default is under all of them. An empty name is the default alone.
    Props resolve(const QString &family, const QString &name) const;
    bool hasStyle(const QString &family, const QString &name) const { return m_styles.contains(family + QLatin1Char('\n') + name); }
    // The data style of a cell style (its own or a parent's), by name; empty for none.
    QString dataStyleOf(const QString &family, const QString &name) const;
    // The number format code a data style stands for ("General" for one that is not there).
    QString dataStyleCode(const QString &name) const;

    // The font of a text's properties (cell or span).
    FontSpec fontOf(const Props &props) const;

    // Makes the model's cell formats: the first is the document's default look, then one for each cell style in the document (the same look is one format). `xfOf` is told the format
    // of each style by name ("" for the default). False, with a message, if there are more formats than the limit.
    bool buildCellFormats(const Limits &limits, Styles *styles, QHash<QString, quint32> *xfOf, QString *error) const;

private:
    QString dataStyleCodeDepth(const QString &name, int depth) const;
    QHash<QString, QString> m_fonts;
    QHash<QString, Props> m_defaults;
    QHash<QString, StyleDef> m_styles;
    QHash<QString, DataStyle> m_data;
    mutable QHash<QString, QString> m_codes;
};

} // namespace jp::office::sheet::ods

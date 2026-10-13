#pragma once
// The fonts a presentation file embeds (ECMA-376 Part 1, 19.2.1.9
// p:embeddedFontLst), as the presentation program does with "Embed fonts in
// the file".
//
// The list is in ppt/presentation.xml, after p:notesSz and before the custom
// shows: a p:embeddedFont for each family, with a p:font naming it (typeface,
// panose, pitchFamily, charset) and a p:regular, p:bold, p:italic, and
// p:boldItalic that each point, by r:id, at a font part (ppt/fonts/fontN.fntdata,
// content type application/x-fontdata, relationship type ".../font"). The
// presentation says it embeds with embedTrueTypeFonts="1": without that the
// program does not look at the list. A part is Embedded OpenType
// (libs/office/office/eot.h).
//
// Writing embeds the fonts the program ships (those other computers lack) in
// the styles the presentation's text uses, each with all its characters, so
// others can edit the text. Fonts everyone has (Calibri, Arial) are never
// embedded, nor is a font whose license doesn't allow it.

#include "core/embeddedfonts.h"
#include "office/package.h"

#include <QByteArray>
#include <QDomElement>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::deck {

// The XML of a part with every typeface it names under the name other programs know it by. The open fonts that are drawn to a standard
// font's character widths (Carlito for Calibri, Arimo for Arial, and the others interchangeFontName lists) are written as the standard
// font, wherever a font is named: a computer that has the standard font draws with it, and JeffOffice draws its look-alike for it, laid
// out the same. The same names are what the lists of fonts used and of embedded fonts hold, and those fonts are never embedded.
QString withStandardFontNames(const QString &xml);
QByteArray withStandardFontNames(const QByteArray &xml);

// The families and styles a presentation's text uses, found in the XML of the
// parts that are written: every run's, paragraph end's, and list level's text
// properties (a:rPr, a:endParaRPr, a:defRPr), and bullet fonts. A typeface
// that is the theme's ("+mj-lt", "+mn-lt") is the font the part's theme gives.
// A text property that names no font uses what its place gives it, so its
// style counts for the theme's two fonts.
class FontUse {
public:
    struct Face {
        QString typeface;
        bool bold = false, italic = false;
    };
    // A font scheme's fonts (heading and body): they are used, regular, by any text that follows the theme.
    void addThemeFonts(const QString &major, const QString &minor);
    // A part of DrawingML text (a slide, master, layout, notes page, or chart) and the theme fonts it follows.
    void scan(const QByteArray &xml, const QString &major, const QString &minor);
    // Every face found, by typeface and then style.
    QVector<Face> faces() const;
    bool isEmpty() const { return m_faces.isEmpty(); }

private:
    void add(const QString &typeface, bool bold, bool italic);
    QHash<QString, QString> m_names;   // lower-cased typeface -> as first written
    QSet<QString> m_faces;             // lower-cased typeface, then 0 to 3 for the style
};

struct FontsWritten {
    QString listXml;           // the p:embeddedFontLst, or empty when no font is embedded
    int faces = 0;             // font parts written
    qint64 bytes = 0;          // their size
};

// Writes the font parts for the faces in use and their relationships to the
// presentation part, and gives back the list that names them. A face is the
// program's own bundled font when `embedBundled` (as an uncompressed EOT of
// the whole font), else the part a file carried for it (`kept`), unchanged.
FontsWritten writeEmbeddedFonts(office::PackageWriter &pkg, const QString &presentationPart, const FontUse &use, const QVector<EmbeddedFace> &kept, bool embedBundled);

// The faces a presentation lists, read from its package: each with the part's
// bytes and, when the part holds a font that can be used, that font. At most
// kMaxEmbeddedFonts faces and kMaxEmbeddedFontsTotal bytes are read. `notes`
// gets what couldn't be used (a font the computer has already is not a loss).
QVector<EmbeddedFace> readEmbeddedFonts(const office::Package &pkg, const QString &presentationPart, const QDomElement &presentation, QStringList *notes);

// The fonts an open document presentation carries: its package holds font files (TrueType or OpenType, not wrapped)
// in the folder "Fonts", which the styles' font faces point at (svg:font-face-uri). Each is a face of the family
// and style its own tables say. The same limits as above.
QVector<EmbeddedFace> readEmbeddedFontFiles(const office::Package &pkg, const QString &folder, QStringList *notes);

} // namespace jp::deck

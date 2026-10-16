#pragma once
// Bundled open fonts and substitutions for common proprietary fonts that
// are missing on Linux. A substitute is used only when the named font is absent.

#include <QByteArray>
#include <QRectF>
#include <QString>
#include <QStringList>

namespace jp {

QStringList bundledFontDirs();                // where the fonts that ship with JeffPub are
int loadBundledFonts();                      // returns number of font files loaded
// A number that changes whenever fonts are added to or taken from the application's font database, for what is worked out from which families there are (the answer
// to "is this family here" is asked for every run of text a document has, and is not cheap).
int fontGeneration();
void fontsChanged();
void installFontSubstitutions();
QString substituteFor(const QString &family); // empty if none / font available
int substituteStretch(const QString &family, bool bold = false, bool italic = false);
// The stretch (percent) to give Qt so text is drawn that wide. Qt's macOS text
// engine applies a font's stretch twice (to the advances and again to the
// letters), so there it is the whole number whose square is nearest.
#ifdef Q_OS_MACOS
constexpr bool kStretchAppliedTwice = true;
#else
constexpr bool kStretchAppliedTwice = false;
#endif
int stretchToAsk(int percent, bool appliedTwice = kStretchAppliedTwice);
// A stand-in's name as this platform has it: the name itself, or the family it
// is a style of ("Selawik Light" is the style Light of Selawik on Windows and
// the Mac, a family of its own on Linux).
QString standInOnThisPlatform(const QString &standIn);
// The style to ask for in that family ("Light"), empty when the stand-in is a
// family of its own or the font is installed. A face asked for by weight is
// named too ("ExtraBold"), so font matching can't take the bold beside it.
QString substituteStyleName(const QString &family, bool bold = false, bool italic = false);
// A font file with its italic flags set when its letters slant but its flags
// say upright (Libre Franklin ExtraBold Italic: DirectWrite can't tell it from
// ExtraBold); empty when the file needs no change.
QByteArray fontWithItalicFlags(const QByteArray &font);
// The stretches (percent; regular, bold, italic, bold italic; 0 = none) the
// table of measured stand-in widths holds, whether or not this platform uses
// them, for the test that shows how the platform draws them.
struct StandInStretch { QString family, standIn; int stretch[4]; };
QList<StandInStretch> standInStretchesForTest();
// Every family the table of stand-ins names, sorted, for the same test.
QStringList standInFamiliesForTest();
// The name to write in files other programs read: a bundled font that is
// metric-compatible with a standard one (Arimo, Carlito...) is written as
// that standard font, which every Windows computer has.
QString interchangeFontName(const QString &family);  // horizontal scale (percent) to imitate a condensed original, 100 if none
// Weight a missing font's name implies ("Franklin Gothic Medium" -> 500), so its
// substitute is set in that weight; 0 when the font is installed or no hint.
// For a bold run, the weight its stand-in's bold is replaced by (Roboto Slab
// ExtraBold for Rockwell Condensed), 0 where the bold face stays.
int substituteWeight(const QString &family, bool bold = false);
// The width (ems) of a space in a missing font whose stand-in's spaces are
// narrower or wider; 0 when the font is installed or the stand-in matches.
double substituteSpaceEm(const QString &family, bool bold = false, bool italic = false);
// How much taller (over 1) or shorter to draw a missing font's stand-in so
// its letters stand as high as the real font's; 1 when installed or alike.
double substituteHeightScale(const QString &family, bool bold = false);
// For an italic run of a missing font that has no italic face of its own for
// the weight: how far to slant its upright stand-in (the shear, as Publisher
// slants the real font's upright face); 0 when installed or it has the italic.
double substituteItalicShear(const QString &family, bool bold);
// A stock font's measured advance width for each character it has (em), from
// resources/fontwidths.txt: the face asked for, else the nearest the family
// has. Null when JeffPub has no measurements of the family.
const QHash<char16_t, double> *originalLetterWidths(const QString &family, bool bold, bool italic);
// The same fonts' pair kerning (em), keyed first << 16 | second, from
// resources/fontkerning.txt. Null when JeffPub has none for the family.
const QHash<quint32, double> *originalKerning(const QString &family, bool bold, bool italic);
// Whether Publisher draws a font's text bold by hand (it does so for bold set
// on a measured font with no bold face, such as Franklin Gothic Demi or
// Forte, and for Arial Black and Rockwell Extra Bold whether or not bold is
// set). How wide the letters' extra stroke is depends on where they are drawn
// (see StoryLayout::paint).
bool originalBoldByHand(const QString &family, bool boldSet);
// The fonts drawn by hand whether or not bold is set: Arial Black and Rockwell Extra Bold.
bool originalBoldAlwaysByHand(const QString &family);
QStringList bundledFamilies();

// Symbol fonts (Symbol, Wingdings) give their pictures their own character
// codes, either 0x20-0xFF or 0xF020-0xF0FF. When such a font is missing,
// this is the same picture as a Unicode character, or an empty string.
bool isSymbolFont(const QString &family);
QString symbolToUnicode(const QString &family, uint code);
// A character of running text in a symbol font (a Greek letter or a sign set in Symbol, a hand or a number in a circle set in Wingdings) as
// the Unicode character that looks like it, or an empty string: what symbolToUnicode gives for a bullet, and the rest of the letters and
// signs the fonts hold that it leaves out. The code may be 0x20-0xFF or 0xF020-0xF0FF.
QString symbolTextToUnicode(const QString &family, uint code);
// The other way: the symbol font and code (0xF020-0xF0FF) for a Unicode
// picture, or 0 when no symbol font has it.
uint unicodeToSymbol(QChar c, QString *family);

// A bullet from a symbol font, as Publisher draws it. Measured in Publisher's
// PDFs, which hold the real glyphs (SymbolMT, Wingdings-Regular): the glyph's
// advance and the box its ink fills, in ems, from the pen's origin with y up
// from the baseline. When the font is missing the bullet is drawn from a
// stand-in glyph fitted to that box (an empty stand-in is a circle), so its
// size and place don't depend on which font the stand-in comes from.
// `standIn` alone, the other two empty: the glyph as it comes, no box known.
struct SymbolBullet {
    QString standIn;
    double advance = 0;
    QRectF ink;
};
bool symbolBullet(const QString &family, uint code, SymbolBullet *out);

// Qt measures a QFont's point size at the platform's logical DPI (96 on most
// systems), but publication geometry is in points. Multiply a document point
// size by this before handing it to Qt so 1 layout unit = 1 point everywhere.
double fontPointFactor();

// One-time setup shared by the app, tools and tests: fonts, substitutions,
// and room for the very large pictures print publications use.
void initCore();

} // namespace jp

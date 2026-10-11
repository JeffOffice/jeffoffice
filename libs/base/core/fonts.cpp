#include "core/fonts.h"

#include "core/appinfo.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QFontInfo>
#include <QGuiApplication>
#include <QHash>
#include <QImageReader>
#include <algorithm>
#include <cmath>

Q_GUI_EXPORT int qt_defaultDpiY();

// The built-in resources (the measured letter widths among them), for
// programs that use the core without registering them first (jpubtool).
static void initCoreResources() { Q_INIT_RESOURCE(resources); }

namespace jp {

double fontPointFactor()
{
    const int dpi = qt_defaultDpiY();
    return dpi > 0 ? 72.0 / dpi : 1.0;
}


static QStringList g_families;

QStringList bundledFontDirs()
{
    const QString app = QCoreApplication::applicationDirPath();
    // Beside the program (Windows), the Linux packages' share folder, and a
    // macOS app's Resources folder.
    QStringList dirs{app + "/fonts", app + "/../share/" + appInfo().fileName + "/fonts", app + "/../fonts", app + "/../Resources/fonts"};
#ifdef JP_SOURCE_DIR
    dirs << QStringLiteral(JP_SOURCE_DIR) + "/resources/fonts";
#endif
    return dirs;
}

// Adds a font file; an italic face whose flags say upright is added from a
// copy with the flags right (only italic-named files are looked at, and
// mapped, so the fonts that are right are not read in full).
static int addFontFile(const QDir &dir, const QString &name)
{
    if (name.contains(QLatin1String("Italic"), Qt::CaseInsensitive) || name.contains(QLatin1String("Oblique"), Qt::CaseInsensitive)) {
        QByteArray fixed;
        {
            QFile file(dir.filePath(name));
            if (file.open(QIODevice::ReadOnly))
                if (uchar *map = file.map(0, file.size())) {
                    fixed = fontWithItalicFlags(QByteArray::fromRawData(reinterpret_cast<const char *>(map), file.size()));
                    file.unmap(map);
                }
        }
        if (!fixed.isEmpty()) return QFontDatabase::addApplicationFontFromData(fixed);
    }
    return QFontDatabase::addApplicationFont(dir.filePath(name));
}

int loadBundledFonts()
{
    int n = 0;
    for (const QString &d : bundledFontDirs()) {
        QDir dir(d);
        if (!dir.exists()) continue;
        for (const QString &f : dir.entryList({"*.ttf", "*.otf"}, QDir::Files)) {
            const int id = addFontFile(dir, f);
            if (id < 0) continue;
            ++n;
            for (const QString &fam : QFontDatabase::applicationFontFamilies(id))
                if (!g_families.contains(fam)) g_families << fam;
        }
        if (n) break;
    }
    g_families.sort();
    return n;
}

QStringList bundledFamilies() { return g_families; }

// Metric-compatible replacements first; otherwise the closest open design.
static const QHash<QString, QString> &table()
{
    static const QHash<QString, QString> t{
        {"Arial", "Arimo"}, {"Helvetica", "Arimo"}, {"Arial Unicode MS", "Arimo"}, {"Liberation Sans", "Arimo"},
        {"Times New Roman", "Tinos"}, {"Times", "Tinos"}, {"Liberation Serif", "Tinos"},
        {"Courier New", "Cousine"}, {"Courier", "Cousine"}, {"Liberation Mono", "Cousine"}, {"Lucida Console", "Cousine"},
        {"Calibri", "Carlito"}, {"Calibri Light", "Carlito"}, {"Cambria", "Caladea"}, {"Georgia", "Gelasio"},
        {"Segoe UI", "Selawik"}, {"Segoe UI Light", "Selawik Light"}, {"Segoe UI Semilight", "Selawik Semilight"}, {"Segoe UI Semibold", "Selawik Semibold"}, {"Comic Sans MS", "Comic Relief"}, {"Garamond", "EB Garamond"},
        {"Book Antiqua", "TeX Gyre Pagella"}, {"Palatino Linotype", "TeX Gyre Pagella"}, {"Palatino", "TeX Gyre Pagella"},
        {"Bookman Old Style", "TeX Gyre Bonum"}, {"Century Schoolbook", "TeX Gyre Schola"}, {"Century Gothic", "TeX Gyre Adventor"},
        {"Gill Sans MT", "Cabin"}, {"Gill Sans", "Cabin"}, {"Abadi", "Cabin"}, {"Abadi MT Condensed Light", "Archivo Narrow"},
        {"Tw Cen MT", "Jost"}, {"Futura", "Jost"}, {"AG_Futura", "Jost"}, {"Futura Md BT", "Jost"}, {"Futura Bk BT", "Jost"},
        {"Franklin Gothic Book", "Libre Franklin"}, {"Franklin Gothic Medium", "Libre Franklin"}, {"Franklin Gothic Demi", "Libre Franklin"},
        {"Franklin Gothic Demi Cond", "Libre Franklin"}, {"Franklin Gothic Medium Cond", "Libre Franklin"}, {"Franklin Gothic Heavy", "Libre Franklin"},
        {"Arial Narrow", "Liberation Sans Narrow"}, {"Arial Black", "Archivo Black"}, {"Impact", "Anton"},
        {"Verdana", "DejaVu Sans"}, {"Tahoma", "DejaVu Sans"}, {"Trebuchet MS", "Open Sans"}, {"Corbel", "Carlito"}, {"Candara", "Cabin"},
        {"Constantia", "Gelasio"}, {"Sylfaen", "Gelasio"}, {"Consolas", "IBM Plex Mono"},
        {"Rockwell", "Arvo"}, {"Rockwell Condensed", "Roboto Slab"}, {"Rockwell Extra Bold", "Alfa Slab One"},
        {"Agency FB", "Archivo Narrow"}, {"Castellar", "Cinzel"}, {"Copperplate Gothic Bold", "Cinzel"}, {"Copperplate Gothic Light", "Cinzel"},
        {"Old English Text MT", "UnifrakturMaguntia"}, {"CloisterBlack BT", "UnifrakturMaguntia"}, {"Brush Script MT", "Kaushan Script"},
        {"Lucida Handwriting", "Merienda"}, {"Juice ITC", "Mountains of Christmas"}, {"Monotype Corsiva", "Dancing Script"}, {"Edwardian Script ITC", "Great Vibes"},
        {"Bodoni MT", "Playfair Display"}, {"Baskerville Old Face", "Tinos"}, {"Elephant", "Abril Fatface"},
        {"Bernard MT Condensed", "Abril Fatface"}, {"Haettenschweiler", "Anton"}, {"Gill Sans Ultra Bold", "Archivo Black"},
        {"Perpetua", "Crimson Text"}, {"Goudy Old Style", "Crimson Text"}, {"Lucida Sans", "DejaVu Sans"}, {"Lucida Sans Unicode", "DejaVu Sans"},
        {"Microsoft Sans Serif", "Arimo"}, {"MS Sans Serif", "Arimo"}, {"MS Serif", "Tinos"}, {"Script MT Bold", "Pacifico"},
        {"Imprint MT Shadow", "EB Garamond"}, {"OCR A Extended", "IBM Plex Mono"}, {"Kristen ITC", "Comic Relief"}, {"Papyrus", "Mountains of Christmas"},
        // Measured against each Windows font and Publisher's own test pages (Oct 8):
        // the nearest bundled face of the same kind.
        {"Algerian", "Cinzel"},
        {"Arial Rounded MT Bold", "Nunito"},
        {"Bahnschrift", "IBM Plex Sans"},
        {"Bahnschrift Condensed", "Archivo Narrow"},
        {"Bahnschrift Light", "IBM Plex Sans"},
        {"Bahnschrift Light Condensed", "Archivo Narrow"},
        {"Bahnschrift Light SemiCondensed", "Archivo Narrow"},
        {"Bahnschrift SemiBold", "IBM Plex Sans"},
        {"Bahnschrift SemiBold Condensed", "Archivo Narrow"},
        {"Bahnschrift SemiBold SemiCondensed", "Archivo Narrow"},
        {"Bahnschrift SemiCondensed", "Archivo Narrow"},
        {"Bahnschrift SemiLight", "IBM Plex Sans"},
        {"Bahnschrift SemiLight Condensed", "Archivo Narrow"},
        {"Bahnschrift SemiLight SemiCondensed", "Archivo Narrow"},
        {"Bauhaus 93", "Jost"},
        {"Bell MT", "Tinos"},
        {"Berlin Sans FB", "Jost"},
        {"Berlin Sans FB Demi", "Jost"},
        {"Blackadder ITC", "Great Vibes"},
        {"Bodoni MT Black", "Abril Fatface"},
        {"Bodoni MT Condensed", "Playfair Display"},
        {"Bodoni MT Poster Compressed", "Anton"},
        {"Bradley Hand ITC", "Comic Neue"},
        {"Britannic Bold", "Lato"},
        {"Broadway", "Abril Fatface"},
        {"Californian FB", "Crimson Text"},
        {"Calisto MT", "Tinos"},
        {"Cambria Math", "Caladea"},
        {"Candara Light", "Cabin"},
        {"Cascadia Code", "IBM Plex Mono"},
        {"Cascadia Code ExtraLight", "IBM Plex Mono"},
        {"Cascadia Code Light", "IBM Plex Mono"},
        {"Cascadia Code SemiBold", "IBM Plex Mono"},
        {"Cascadia Code SemiLight", "IBM Plex Mono"},
        {"Cascadia Mono", "IBM Plex Mono"},
        {"Cascadia Mono ExtraLight", "IBM Plex Mono"},
        {"Cascadia Mono Light", "IBM Plex Mono"},
        {"Cascadia Mono SemiBold", "IBM Plex Mono"},
        {"Cascadia Mono SemiLight", "IBM Plex Mono"},
        {"Centaur", "EB Garamond"},
        {"Century", "TeX Gyre Schola"},
        {"Chiller", "Mountains of Christmas"},
        {"Colonna MT", "Cormorant Garamond"},
        {"Cooper Black", "Fraunces"},
        {"Corbel Light", "Carlito"},
        {"Curlz MT", "Mountains of Christmas"},
        {"Dubai", "Carlito"},
        {"Dubai Light", "Carlito"},
        {"Dubai Medium", "Carlito"},
        {"Engravers MT", "Cinzel"},
        {"Eras Bold ITC", "Selawik"},
        {"Eras Demi ITC", "Selawik Semibold"},
        {"Eras Light ITC", "Selawik Light"},
        {"Eras Medium ITC", "Selawik"},
        {"Felix Titling", "Cinzel"},
        {"Footlight MT Light", "Cormorant Garamond"},
        {"Forte", "Lobster"},
        {"Freestyle Script", "Great Vibes"},
        {"French Script MT", "Great Vibes"},
        {"Gabriola", "Great Vibes"},
        {"Gigi", "Lobster"},
        {"Gill Sans MT Condensed", "Archivo Narrow"},
        {"Gill Sans MT Ext Condensed Bold", "Oswald"},
        {"Gill Sans Ultra Bold Condensed", "Anton"},
        {"Gloucester MT Extra Condensed", "EB Garamond"},
        {"Goudy Stout", "Alfa Slab One"},
        {"Harlow Solid Italic", "Lobster"},
        {"Harrington", "Playfair Display"},
        {"High Tower Text", "Crimson Text"},
        {"Informal Roman", "Dancing Script"},
        {"Ink Free", "Comic Neue"},
        {"Jokerman", "Permanent Marker"},
        {"Kunstler Script", "Great Vibes"},
        {"Lucida Bright", "Merriweather"},
        {"Lucida Calligraphy", "Merienda"},
        {"Lucida Fax", "Arvo"},
        {"Lucida Sans Typewriter", "Cousine"},
        {"Magneto", "Archivo Black"},
        {"Maiandra GD", "Selawik"},
        {"Matura MT Script Capitals", "Merienda"},
        {"Mistral", "Caveat"},
        {"Modern No. 20", "Tinos"},
        {"MS Reference Sans Serif", "DejaVu Sans"},
        {"MV Boli", "Comic Relief"},
        {"Niagara Engraved", "Anton"},
        {"Niagara Solid", "Anton"},
        {"Onyx", "Cormorant Garamond"},
        {"Palace Script MT", "Great Vibes"},
        {"Parchment", "Great Vibes"},
        {"Perpetua Titling MT", "Cinzel"},
        {"Playbill", "Anton"},
        {"Poor Richard", "EB Garamond"},
        {"Pristina", "Caveat"},
        {"Rage Italic", "Kaushan Script"},
        {"Ravie", "Bungee"},
        {"Segoe Print", "Comic Relief"},
        {"Segoe Script", "Merienda"},
        {"Segoe UI Black", "Libre Franklin"},
        {"Segoe UI Variable Display", "Selawik"},
        {"Segoe UI Variable Display Light", "Selawik Light"},
        {"Segoe UI Variable Display Semibold", "Selawik Semibold"},
        {"Segoe UI Variable Display Semilight", "Selawik Semilight"},
        {"Segoe UI Variable Small", "Selawik"},
        {"Segoe UI Variable Small Light", "Selawik Light"},
        {"Segoe UI Variable Small Semibold", "Selawik Semibold"},
        {"Segoe UI Variable Small Semilight", "Selawik Semilight"},
        {"Segoe UI Variable Text", "Selawik"},
        {"Segoe UI Variable Text Light", "Selawik Light"},
        {"Segoe UI Variable Text Semibold", "Selawik Semibold"},
        {"Segoe UI Variable Text Semilight", "Selawik Semilight"},
        {"Showcard Gothic", "Archivo Black"},
        {"Snap ITC", "Bungee"},
        {"Stencil", "Archivo Black"},
        {"Tempus Sans ITC", "Comic Neue"},
        {"Tw Cen MT Condensed", "Archivo Narrow"},
        {"Tw Cen MT Condensed Extra Bold", "Oswald"},
        {"Viner Hand ITC", "Kaushan Script"},
        {"Vivaldi", "Great Vibes"},
        {"Vladimir Script", "Great Vibes"},
        {"Wide Latin", "Alfa Slab One"},
    };
    return t;
}

// Condensed originals keep their width by scaling the substitute horizontally.
static const QHash<QString, int> &stretches()
{
    static const QHash<QString, int> t{
        {"Lucida Handwriting", 114}, {"Franklin Gothic Medium Cond", 75}, 
        {"Abadi MT Condensed Light", 85}, 
    };
    return t;
}

QString interchangeFontName(const QString &family)
{
    // Only the open fonts drawn to the same character widths as a standard
    // font; text laid out with one lays out the same with the other.
    static const QHash<QString, QString> t{
        {"Arimo", "Arial"}, {"Tinos", "Times New Roman"}, {"Cousine", "Courier New"},
        {"Carlito", "Calibri"}, {"Caladea", "Cambria"}, {"Gelasio", "Georgia"},
        {"Comic Relief", "Comic Sans MS"}, {"Selawik", "Segoe UI"}, {"Liberation Sans Narrow", "Arial Narrow"},
    };
    return t.value(family, family);
}

// Stand-ins drawn to the originals' widths, for each style (regular, bold,
// italic, bold italic; 0 where nothing was measured, which takes the
// regular or bold one): the letters narrowed or widened (percent) and the
// word space (ems). Measured from the character widths of the fonts embedded
// in Publisher's PDFs of 284 publications, weighted by how often each
// character appears in them, against the stand-ins (Oct 7): Cabin's letters
// run 5% wider than Gill Sans MT's and 10% narrower than its bold, and its
// spaces 24% narrower.
// They hold only for the stand-in they were measured with (macOS draws Gill
// Sans MT with its own Gill Sans, which needs none).
struct StandInWidths { const char *standIn; int stretch[4]; double space[4]; };
static const QHash<QString, StandInWidths> &standInWidths()
{
    static const QHash<QString, StandInWidths> t{
        {"Gill Sans MT", {"Cabin", {95, 109, 92, 103}, {0.278, 0.278, 0.278, 0.278}}},
        {"Franklin Gothic Book", {"Libre Franklin", {89, 0, 85, 0}, {0.25, 0, 0.25, 0}}},
        {"Franklin Gothic Demi", {"Libre Franklin", {89, 0, 88, 0}, {0.25, 0, 0.25, 0}}},
        // Heavy against Libre Franklin ExtraBold: 3.4% wider over a sample
        // of ordinary text (Oct 8, the Windows font's advances).
        {"Franklin Gothic Heavy", {"Libre Franklin", {97, 0, 0, 0}, {0, 0, 0, 0}}},
        // (Agency FB's letters stay: asked to narrow, its stand-in Saira
        // Condensed switches to a narrower face of its own instead.)
        {"Agency FB", {"Archivo Narrow", {85, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Garamond", {"EB Garamond", {0, 101, 88, 0}, {0.25, 0.25, 0.25, 0}}},
        {"Imprint MT Shadow", {"EB Garamond", {110, 0, 0, 0}, {0.25, 0, 0, 0}}},
        {"Castellar", {"Cinzel", {116, 0, 0, 0}, {0.36, 0, 0, 0}}},
        {"Arial Rounded MT Bold", {"Nunito", {103, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Abadi", {"Cabin", {0, 0, 0, 0}, {0.302, 0, 0, 0}}},
        // Measured with Qt against each Windows font's advances over a sample of
        // ordinary text (Oct 8).
        {"Algerian", {"Cinzel", {97, 95, 0, 0}, {0, 0, 0, 0}}},
        {"Bahnschrift", {"IBM Plex Sans", {0, 95, 0, 0}, {0, 0, 0, 0}}},
        {"Bahnschrift Condensed", {"Archivo Narrow", {90, 84, 0, 0}, {0, 0, 0, 0}}},
        {"Bahnschrift Light", {"IBM Plex Sans", {0, 95, 0, 0}, {0, 0, 0, 0}}},
        {"Bahnschrift Light Condensed", {"Archivo Narrow", {90, 84, 0, 0}, {0, 0, 0, 0}}},
        {"Bahnschrift Light SemiCondensed", {"Archivo Narrow", {106, 99, 0, 0}, {0, 0, 0, 0}}},
        {"Bahnschrift SemiBold", {"IBM Plex Sans", {95, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Bahnschrift SemiBold Condensed", {"Archivo Narrow", {84, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Bahnschrift SemiCondensed", {"Archivo Narrow", {106, 99, 0, 0}, {0, 0, 0, 0}}},
        {"Bahnschrift SemiLight", {"IBM Plex Sans", {0, 95, 0, 0}, {0, 0, 0, 0}}},
        {"Bahnschrift SemiLight Condensed", {"Archivo Narrow", {90, 84, 0, 0}, {0, 0, 0, 0}}},
        {"Bahnschrift SemiLight SemiCondensed", {"Archivo Narrow", {106, 99, 0, 0}, {0, 0, 0, 0}}},
        {"Baskerville Old Face", {"Tinos", {97, 92, 0, 0}, {0, 0, 0, 0}}},
        {"Bauhaus 93", {"Jost", {94, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Bell MT", {"Tinos", {0, 102, 94, 0}, {0, 0, 0, 0}}},
        {"Berlin Sans FB", {"Jost", {97, 101, 0, 0}, {0, 0, 0, 0}}},
        {"Berlin Sans FB Demi", {"Jost", {94, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Bernard MT Condensed", {"Abril Fatface", {84, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Blackadder ITC", {"Great Vibes", {107, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Bodoni MT", {"Playfair Display", {95, 90, 0, 102}, {0, 0, 0, 0}}},
        {"Bodoni MT Black", {"Abril Fatface", {112, 0, 119, 0}, {0, 0, 0, 0}}},
        {"Bodoni MT Condensed", {"Playfair Display", {64, 73, 67, 74}, {0, 0, 0, 0}}},
        {"Bodoni MT Poster Compressed", {"Anton", {60, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Broadway", {"Abril Fatface", {119, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Brush Script MT", {"Kaushan Script", {84, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Calibri Light", {"Carlito", {0, 96, 99, 0}, {0, 0, 0, 0}}},
        {"Californian FB", {"Crimson Text", {103, 99, 94, 0}, {0, 0, 0, 0}}},
        {"Calisto MT", {"Tinos", {105, 103, 94, 96}, {0, 0, 0, 0}}},
        {"Cambria", {"Caladea", {104, 102, 0, 0}, {0, 0, 0, 0}}},
        {"Cambria Math", {"Caladea", {104, 95, 0, 0}, {0, 0, 0, 0}}},
        {"Candara", {"Cabin", {0, 0, 104, 98}, {0, 0, 0, 0}}},
        {"Candara Light", {"Cabin", {0, 96, 103, 0}, {0, 0, 0, 0}}},
        {"Cascadia Code", {"IBM Plex Mono", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Cascadia Code ExtraLight", {"IBM Plex Mono", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Cascadia Code Light", {"IBM Plex Mono", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Cascadia Code SemiBold", {"IBM Plex Mono", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Cascadia Code SemiLight", {"IBM Plex Mono", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Cascadia Mono", {"IBM Plex Mono", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Cascadia Mono ExtraLight", {"IBM Plex Mono", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Cascadia Mono Light", {"IBM Plex Mono", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Cascadia Mono SemiBold", {"IBM Plex Mono", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Cascadia Mono SemiLight", {"IBM Plex Mono", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Centaur", {"EB Garamond", {96, 87, 0, 0}, {0, 0, 0, 0}}},
        {"Century", {"TeX Gyre Schola", {0, 89, 0, 0}, {0, 0, 0, 0}}},
        {"Chiller", {"Mountains of Christmas", {82, 79, 0, 0}, {0, 0, 0, 0}}},
        {"Colonna MT", {"Cormorant Garamond", {107, 105, 0, 0}, {0, 0, 0, 0}}},
        {"Consolas", {"IBM Plex Mono", {92, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Constantia", {"Gelasio", {98, 92, 93, 89}, {0, 0, 0, 0}}},
        {"Cooper Black", {"Fraunces", {104, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Copperplate Gothic Bold", {"Cinzel", {102, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Copperplate Gothic Light", {"Cinzel", {0, 98, 0, 0}, {0, 0, 0, 0}}},
        {"Corbel", {"Carlito", {0, 104, 98, 101}, {0, 0, 0, 0}}},
        {"Corbel Light", {"Carlito", {97, 95, 93, 0}, {0, 0, 0, 0}}},
        {"Curlz MT", {"Mountains of Christmas", {107, 102, 0, 0}, {0, 0, 0, 0}}},
        {"Dubai", {"Carlito", {0, 102, 0, 0}, {0, 0, 0, 0}}},
        {"Dubai Light", {"Carlito", {98, 95, 0, 0}, {0, 0, 0, 0}}},
        {"Dubai Medium", {"Carlito", {102, 99, 0, 0}, {0, 0, 0, 0}}},
        {"Edwardian Script ITC", {"Great Vibes", {91, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Elephant", {"Abril Fatface", {108, 0, 110, 0}, {0, 0, 0, 0}}},
        {"Engravers MT", {"Cinzel", {141, 138, 0, 0}, {0, 0, 0, 0}}},
        {"Eras Bold ITC", {"Selawik", {107, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Eras Demi ITC", {"Selawik Semibold", {102, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Eras Medium ITC", {"Selawik", {0, 93, 0, 0}, {0, 0, 0, 0}}},
        {"Felix Titling", {"Cinzel", {105, 103, 0, 0}, {0, 0, 0, 0}}},
        {"Footlight MT Light", {"Cormorant Garamond", {106, 104, 0, 0}, {0, 0, 0, 0}}},
        {"Forte", {"Lobster", {80, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Franklin Gothic Demi Cond", {"Libre Franklin", {78, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Franklin Gothic Medium", {"Libre Franklin", {90, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Freestyle Script", {"Great Vibes", {85, 0, 0, 0}, {0, 0, 0, 0}}},
        {"French Script MT", {"Great Vibes", {96, 0, 0, 0}, {0, 0, 0, 0}}},
        // Regular Gill Sans MT Condensed: its letters keep the font's own
        // advances (fontwidths.txt), so the stand-in is narrowed to the
        // letters' ink: a U 75 points wide at 233 pt in Publisher's picture
        // (the squeeze to the advances, 83%, drew it 90).
        {"Gill Sans MT Condensed", {"Archivo Narrow", {66, 78, 0, 0}, {0, 0, 0, 0}}},
        {"Gill Sans MT Ext Condensed Bold", {"Oswald", {60, 60, 0, 0}, {0, 0, 0, 0}}},
        {"Gill Sans Ultra Bold", {"Archivo Black", {115, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Gill Sans Ultra Bold Condensed", {"Anton", {112, 0, 0, 0}, {0, 0, 0, 0}}},
        // Drawn in the stand-in's ExtraBold face (substituteWeight), one
        // weight for both: its regular face at 72% carried 0.6 of the
        // original's ink, the bold at 62% 0.85, the ExtraBold at 62% 0.91
        // (line by line against Publisher's PDFs, Oct 10; capitals alone want
        // 60%, small letters 64%; wider carries more ink but scores worse).
        {"Gloucester MT Extra Condensed", {"EB Garamond", {62, 62, 0, 0}, {0, 0, 0, 0}}},
        {"Goudy Old Style", {"Crimson Text", {0, 95, 0, 0}, {0, 0, 0, 0}}},
        {"Goudy Stout", {"Alfa Slab One", {160, 160, 0, 0}, {0, 0, 0, 0}}},
        {"Haettenschweiler", {"Anton", {77, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Harrington", {"Playfair Display", {96, 94, 0, 0}, {0, 0, 0, 0}}},
        {"High Tower Text", {"Crimson Text", {107, 98, 0, 0}, {0, 0, 0, 0}}},
        {"Informal Roman", {"Dancing Script", {103, 100, 0, 0}, {0, 0, 0, 0}}},
        {"Ink Free", {"Comic Neue", {0, 97, 0, 0}, {0, 0, 0, 0}}},
        {"Jokerman", {"Permanent Marker", {97, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Juice ITC", {"Mountains of Christmas", {80, 76, 0, 0}, {0, 0, 0, 0}}},
        {"Kristen ITC", {"Comic Relief", {105, 99, 0, 0}, {0, 0, 0, 0}}},
        {"Kunstler Script", {"Great Vibes", {88, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Lucida Bright", {"Merriweather", {102, 0, 107, 110}, {0, 0, 0, 0}}},
        {"Lucida Calligraphy", {"Merienda", {109, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Lucida Fax", {"Arvo", {107, 105, 102, 103}, {0, 0, 0, 0}}},
        {"Lucida Sans", {"DejaVu Sans", {97, 88, 0, 0}, {0, 0, 0, 0}}},
        {"Lucida Sans Unicode", {"DejaVu Sans", {97, 83, 0, 0}, {0, 0, 0, 0}}},
        {"Maiandra GD", {"Selawik", {0, 94, 0, 0}, {0, 0, 0, 0}}},
        {"Matura MT Script Capitals", {"Merienda", {90, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Microsoft Sans Serif", {"Arimo", {0, 92, 0, 0}, {0, 0, 0, 0}}},
        {"Mistral", {"Caveat", {98, 96, 0, 0}, {0, 0, 0, 0}}},
        {"Modern No. 20", {"Tinos", {0, 93, 0, 0}, {0, 0, 0, 0}}},
        {"Monotype Corsiva", {"Dancing Script", {0, 97, 0, 0}, {0, 0, 0, 0}}},
        {"MS Reference Sans Serif", {"DejaVu Sans", {0, 87, 0, 0}, {0, 0, 0, 0}}},
        {"MV Boli", {"Comic Relief", {104, 98, 0, 0}, {0, 0, 0, 0}}},
        {"Niagara Engraved", {"Anton", {60, 60, 0, 0}, {0, 0, 0, 0}}},
        {"Niagara Solid", {"Anton", {60, 60, 0, 0}, {0, 0, 0, 0}}},
        {"Old English Text MT", {"UnifrakturMaguntia", {103, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Onyx", {"Cormorant Garamond", {60, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Palace Script MT", {"Great Vibes", {78, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Papyrus", {"Mountains of Christmas", {117, 111, 0, 0}, {0, 0, 0, 0}}},
        {"Parchment", {"Great Vibes", {63, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Perpetua", {"Crimson Text", {93, 99, 91, 104}, {0, 0, 0, 0}}},
        {"Perpetua Titling MT", {"Cinzel", {106, 114, 0, 0}, {0, 0, 0, 0}}},
        {"Playbill", {"Anton", {63, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Poor Richard", {"EB Garamond", {95, 86, 0, 0}, {0, 0, 0, 0}}},
        {"Pristina", {"Caveat", {96, 94, 0, 0}, {0, 0, 0, 0}}},
        {"Rage Italic", {"Kaushan Script", {91, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Ravie", {"Bungee", {113, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Rockwell", {"Arvo", {96, 0, 90, 92}, {0, 0, 0, 0}}},
        // Regular Rockwell Condensed is drawn in Roboto Slab Bold (its own
        // bold is wider, 86%, and drawn in the ExtraBold): Roboto Slab Regular
        // at 71% carried 0.85 of the original's ink, the bold at 60% 1.0; its
        // bold in the Bold at 86% 0.94, the ExtraBold 1.01 (Oct 10, as
        // Gloucester).
        {"Rockwell Condensed", {"Roboto Slab", {60, 86, 0, 0}, {0, 0, 0, 0}}},
        {"Rockwell Extra Bold", {"Alfa Slab One", {111, 0, 0, 0}, {0, 0, 0, 0}}},
        // Narrower than its advances ask (98): Pacifico's letters are wider and
        // heavier than the original's, and run into each other at 98.
        {"Script MT Bold", {"Pacifico", {86, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe Print", {"Comic Relief", {110, 103, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe Script", {"Merienda", {112, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI", {"Selawik", {0, 0, 98, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Black", {"Libre Franklin", {103, 105, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Light", {"Selawik Light", {0, 0, 98, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Semibold", {"Selawik Semibold", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Semilight", {"Selawik Semilight", {0, 0, 98, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Variable Display", {"Selawik", {98, 91, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Variable Display Light", {"Selawik Light", {96, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Variable Display Semibold", {"Selawik Semibold", {97, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Variable Display Semilight", {"Selawik Semilight", {96, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Variable Small", {"Selawik", {104, 97, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Variable Small Light", {"Selawik Light", {105, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Variable Small Semibold", {"Selawik Semibold", {102, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Variable Small Semilight", {"Selawik Semilight", {105, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Variable Text", {"Selawik", {0, 93, 0, 0}, {0, 0, 0, 0}}},
        {"Segoe UI Variable Text Semibold", {"Selawik Semibold", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Snap ITC", {"Bungee", {97, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Stencil", {"Archivo Black", {98, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Sylfaen", {"Gelasio", {96, 82, 0, 0}, {0, 0, 0, 0}}},
        {"Tahoma", {"DejaVu Sans", {85, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Tempus Sans ITC", {"Comic Neue", {0, 98, 0, 0}, {0, 0, 0, 0}}},
        {"Trebuchet MS", {"Open Sans", {98, 96, 105, 103}, {0, 0, 0, 0}}},
        // Regular Tw Cen MT is drawn in Jost Medium (substituteWeight): Jost
        // Regular at 94% carried 0.88 of the original's ink, the Medium at 85%
        // 1.0 (at 90%, its letters' own width, 1.05, and scored worse against
        // Publisher's pictures); italic keeps the Regular's 94% (Oct 10).
        {"Tw Cen MT", {"Jost", {85, 89, 94, 83}, {0, 0, 0, 0}}},
        {"Tw Cen MT Condensed", {"Archivo Narrow", {82, 90, 0, 0}, {0, 0, 0, 0}}},
        {"Tw Cen MT Condensed Extra Bold", {"Oswald", {94, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Viner Hand ITC", {"Kaushan Script", {117, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Vivaldi", {"Great Vibes", {108, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Vladimir Script", {"Great Vibes", {110, 0, 0, 0}, {0, 0, 0, 0}}},
        {"Wide Latin", {"Alfa Slab One", {157, 0, 0, 0}, {0, 0, 0, 0}}},
    };
    return t;
}

// The table's entry for a missing font when its stand-in is the one drawn.
static const StandInWidths *measuredStandIn(const QString &family)
{
    const auto w = standInWidths().constFind(family);
    if (w == standInWidths().constEnd()) return nullptr;
    static QHash<QString, bool> drawn;   // main thread only, like all layout
    auto it = drawn.constFind(family);
    if (it == drawn.constEnd()) {
        QFont f(family);
        f.setFamilies({family});
        it = drawn.insert(family, QFontInfo(f).family().compare(standInOnThisPlatform(QLatin1String(w->standIn)), Qt::CaseInsensitive) == 0);
    }
    return *it ? &*w : nullptr;
}

// Whether a family is installed or an application font. The font database
// takes about 0.2 ms to say, and layout asks it of every run: its answers are
// kept until fonts are added or removed.
static bool haveFamily(const QString &family)
{
    static QHash<QString, bool> cache;   // main thread only, like all layout
    static bool watching = false;
    if (!qGuiApp) return QFontDatabase::hasFamily(family);
    if (!watching) {
        QObject::connect(qGuiApp, &QGuiApplication::fontDatabaseChanged, qGuiApp, [] { cache.clear(); });
        watching = true;
    }
    auto it = cache.constFind(family);
    if (it == cache.constEnd()) it = cache.insert(family, QFontDatabase::hasFamily(family));
    return *it;
}

// A style's measured value: bold italic falls back to bold, then italic or
// bold to regular.
template <class T>
static T byStyle(const T (&v)[4], bool bold, bool italic)
{
    const int i = (bold ? 1 : 0) + (italic ? 2 : 0);
    if (v[i]) return v[i];
    if (i == 3 && v[1]) return v[1];
    return v[0];
}

int substituteStretch(const QString &family, bool bold, bool italic)
{
    if (haveFamily(family)) return 100;
    if (const StandInWidths *w = measuredStandIn(family))
        if (const int s = byStyle(w->stretch, bold, italic)) return s;
    return stretches().value(family, 100);
}

QList<StandInStretch> standInStretchesForTest()
{
    QList<StandInStretch> out;
    for (auto it = standInWidths().cbegin(); it != standInWidths().cend(); ++it) {
        StandInStretch s{it.key(), QString::fromLatin1(it->standIn), {}};
        for (int i = 0; i < 4; ++i) s.stretch[i] = it->stretch[i];
        out << s;
    }
    std::sort(out.begin(), out.end(), [](const StandInStretch &a, const StandInStretch &b) { return a.family < b.family; });
    return out;
}

double substituteSpaceEm(const QString &family, bool bold, bool italic)
{
    if (haveFamily(family)) return 0;
    if (const StandInWidths *w = measuredStandIn(family))
        if (const double em = byStyle(w->space, bold, italic); em > 0) return em;
    // AG_Futura's spaces are half an em (word gaps in reference PDFs of
    // book covers at 11, 16 and 36 pt); its stand-in Jost's are 0.3 em.
    if (family.compare(QLatin1String("AG_Futura"), Qt::CaseInsensitive) == 0) return 0.5;
    return 0;
}

// How much taller or shorter to draw a missing font's stand-in so its
// letters stand as high as the real font's: the real font's capital H,
// small x and ascender d (ink heights in the Windows fonts' outlines, read
// in place) over the stand-in's, their geometric mean, where all three
// differ the same way, within 10% of each other (one scale fits them all;
// Perpetua's ascenders differ from its capitals, so its stand-in is left
// alone), by 4% or more, kept within 0.8-1.25. Checked against
// Publisher's PDFs (its screen pictures place text differently). Only the
// drawing changes; widths, line breaks and spacing stay.
double substituteHeightScale(const QString &family, bool bold)
{
    struct Heights { const char *standIn; double regular, bold; };
    static const QHash<QString, Heights> t = {
        {"Arial Rounded MT Bold", {"Nunito", 1.042, 0}},
        {"Bell MT", {"Tinos", 0.95, 0}},
        {"Blackadder ITC", {"Great Vibes", 0.848, 0}},
        {"Bodoni MT Black", {"Abril Fatface", 0.892, 0}},
        {"Britannic Bold", {"Lato", 0.93, 0}},
        {"Californian FB", {"Crimson Text", 1.054, 1.045}},
        {"Candara", {"Cabin", 0.925, 0.919}},
        {"Candara Light", {"Cabin", 0.925, 0}},
        {"Centaur", {"EB Garamond", 0.944, 0}},
        {"Consolas", {"IBM Plex Mono", 0.932, 0.936}},
        {"Copperplate Gothic Bold", {"Cinzel", 0.924, 0}},
        {"Copperplate Gothic Light", {"Cinzel", 0.925, 0}},
        {"Courier New", {"Cousine", 0.837, 0.87}},
        // Capitals, small letters, and ascenders differ in Forte and Tw Cen MT
        // Condensed (read from Publisher's PDFs against the stand-ins: Lobster
        // 0.89, 0.94, 1.0; Archivo Narrow 0.89, 0.79, 0.90): the scale near their
        // geometric means that matched its pictures best.
        {"Forte", {"Lobster", 0.9, 0}},
        {"Franklin Gothic Book", {"Libre Franklin", 0.909, 0}},
        {"Franklin Gothic Demi", {"Libre Franklin", 0.909, 0}},
        {"Franklin Gothic Demi Cond", {"Libre Franklin", 0.909, 0}},
        {"Franklin Gothic Heavy", {"Libre Franklin", 0.911, 0}},
        {"Franklin Gothic Medium", {"Libre Franklin", 0.909, 0}},
        {"Franklin Gothic Medium Cond", {"Libre Franklin", 0.909, 0}},
        {"Garamond", {"EB Garamond", 0.95, 0.94}},
        {"Gill Sans MT", {"Cabin", 0.933, 0.935}},
        {"Gill Sans Ultra Bold", {"Archivo Black", 1.074, 0}},
        {"Gill Sans Ultra Bold Condensed", {"Anton", 0.844, 0}},
        {"Gloucester MT Extra Condensed", {"EB Garamond", 1.119, 0}},
        {"Haettenschweiler", {"Anton", 0.801, 0}},
        {"Impact", {"Anton", 0.908, 0}},
        {"Lucida Bright", {"Merriweather", 0.956, 0.959}},
        {"Lucida Sans Typewriter", {"Cousine", 1.054, 1.054}},
        {"Modern No. 20", {"Tinos", 0.952, 0}},
        {"OCR A Extended", {"IBM Plex Mono", 0.906, 0}},
        {"Perpetua", {"Crimson Text", 0, 0.895}},
        {"Rockwell", {"Arvo", 0.915, 0.915}},
        {"Rockwell Condensed", {"Roboto Slab", 0, 0.945}},
        {"Rockwell Extra Bold", {"Alfa Slab One", 0.866, 0}},
        // Pacifico's letters stand a quarter above Script MT Bold's (a W 220
        // points tall at 233 pt in Publisher's picture, 165 in the original).
        {"Script MT Bold", {"Pacifico", 0.8, 0}},
        {"Segoe Print", {"Comic Relief", 1.071, 0}},
        {"Segoe UI Black", {"Libre Franklin", 0.961, 0}},
        {"Sylfaen", {"Gelasio", 0.931, 0}},
        {"Tw Cen MT", {"Jost", 0.858, 0.887}},
        {"Tw Cen MT Condensed", {"Archivo Narrow", 0.857, 0.857}},
        {"Tw Cen MT Condensed Extra Bold", {"Oswald", 0.81, 0}},
        {"Wide Latin", {"Alfa Slab One", 0.872, 0}},
    };
    if (haveFamily(family)) return 1;
    const auto h = t.constFind(family);
    if (h == t.constEnd()) return 1;
    static QHash<QString, bool> drawn;   // main thread only, like all layout
    auto it = drawn.constFind(family);
    if (it == drawn.constEnd()) {
        QFont f(family);
        f.setFamilies({family});
        it = drawn.insert(family, QFontInfo(f).family().compare(QLatin1String(h->standIn), Qt::CaseInsensitive) == 0);
    }
    if (!*it) return 1;
    const double s = bold && h->bold > 0 ? h->bold : h->regular;
    return s > 0 ? s : 1;
}

// Where the real font has no italic face for the weight a run asks, Publisher
// slants the font's upright face a third of a letter's height: every such run
// in its PDFs (Impact, Tw Cen MT Condensed, Franklin Gothic Medium Cond and
// Demi Cond, Eras ITC) has a text matrix with that shear. Garamond has an
// italic and a bold but no bold italic: its bold is slanted, where the
// stand-in's own bold italic has curled letters (EB Garamond's m and n).
double substituteItalicShear(const QString &family, bool bold)
{
    struct Lacks { bool regular, bold; };   // the italic of the family's regular face, of its bold one
    static const QHash<QString, Lacks> t = {
        {"Eras Bold ITC", {true, true}},
        {"Eras Demi ITC", {true, true}},
        {"Eras Light ITC", {true, true}},
        {"Eras Medium ITC", {true, true}},
        {"Franklin Gothic Demi Cond", {true, true}},
        {"Franklin Gothic Medium Cond", {true, true}},
        {"Garamond", {false, true}},
        {"Impact", {true, true}},
        {"Script MT Bold", {true, true}},
        {"Tw Cen MT Condensed", {true, true}},
        {"Tw Cen MT Condensed Extra Bold", {true, true}},
    };
    if (haveFamily(family)) return 0;
    const auto l = t.constFind(family);
    if (l == t.constEnd()) return 0;
    return (bold ? l->bold : l->regular) ? 1.0 / 3 : 0;
}

// Fonts whose names say nothing of the weight their stand-in is drawn at: the
// weight for a run that isn't bold and for one that is (0: the stand-in's own
// bold stays).
struct NamedWeights { int regular, bold; };
static const QHash<QString, NamedWeights> &weightsByName()
{
    static const QHash<QString, NamedWeights> t{
        {"arial rounded mt bold", {700, 0}},
        {"bauhaus 93", {700, 0}},
        {"britannic bold", {700, 0}},
        {"copperplate gothic bold", {700, 0}},
        {"eras bold itc", {700, 0}},
        {"gill sans mt ext condensed bold", {700, 0}},
        {"gloucester mt extra condensed", {800, 800}},
        {"rockwell condensed", {700, 800}},
        {"tw cen mt", {500, 0}},
        {"tw cen mt condensed extra bold", {700, 0}},
    };
    return t;
}

int substituteWeight(const QString &family, bool bold)
{
    static QHash<QString, int> cache;
    const QString key = family + (bold ? QStringLiteral("|1") : QStringLiteral("|0"));
    auto it = cache.constFind(key);
    if (it != cache.constEnd()) return *it;
    int w = 0;
    if (!QFontDatabase::hasFamily(family)) {
        const QString f = family.toLower();
        const auto o = weightsByName().constFind(f);
        // A bold run keeps its stand-in's bold unless the table names another.
        if (bold) w = o != weightsByName().constEnd() ? o->bold : 0;
        else if (o != weightsByName().constEnd()) w = o->regular;
        // Franklin Gothic's weights run heavy: its Medium looks like a SemiBold.
        else if (f.startsWith("franklin gothic") && f.contains("medium")) w = 600;
        else if (f.startsWith("franklin gothic") && f.contains("demi")) w = 700;
        else if (f.startsWith("franklin gothic") && f.contains("heavy")) w = 800;   // Libre Franklin ExtraBold
        else if (f.contains("black") || f.contains("heavy")) w = 900;
        else if (f.contains("extrabold") || f.contains("extra bold") || f.contains("ultra bold")) w = 800;
        else if (f.contains("demi") || f.contains("semibold") || f.contains("semi bold")) w = 600;
        else if (f.contains("medium")) w = 500;
        else if (f.contains("light")) w = 300;
        else if (f.contains("thin")) w = 100;
    }
    cache.insert(key, w);
    return w;
}

bool isSymbolFont(const QString &family)
{
    static const QStringList fonts = {QStringLiteral("Symbol"), QStringLiteral("Wingdings"), QStringLiteral("Wingdings 2"), QStringLiteral("Wingdings 3"),
                                      QStringLiteral("Webdings")};
    return fonts.contains(family, Qt::CaseInsensitive);
}

QString symbolToUnicode(const QString &family, uint code)
{
    if (code >= 0xF000 && code <= 0xF0FF) code -= 0xF000;
    // The pictures most used as bullets and check boxes.
    static const QHash<uint, uint> symbol = {
        {0xB7, 0x2022}, {0xA7, 0x2663}, {0xA8, 0x2666}, {0xA9, 0x2665}, {0xAA, 0x2660}, {0xAE, 0x2192}, {0xDE, 0x21D2},
        {0xB0, 0x00B0}, {0xD7, 0x22C5}, {0xE0, 0x25CA}, {0x2A, 0x2217}, {0xC4, 0x2297}, {0xC5, 0x2295}, {0x6F, 0x03BF}};
    static const QHash<uint, uint> wingdings = {
        {0x6C, 0x25CF}, {0x6E, 0x25A0}, {0x6F, 0x25A1}, {0x71, 0x2751}, {0x72, 0x2752}, {0x75, 0x25C6}, {0x76, 0x2756},
        {0x77, 0x2B25}, {0x9F, 0x2022}, {0xA1, 0x25CB}, {0xA7, 0x25AA}, {0xA8, 0x25FB}, {0xAB, 0x2605}, {0xD8, 0x27A2},
        {0xE8, 0x2794}, {0xF0, 0x21E8}, {0xFB, 0x2718}, {0xFC, 0x2714}, {0xFD, 0x2612}, {0xFE, 0x2611}};
    const QString f = family.toLower();
    const QHash<uint, uint> *map = f == QLatin1String("symbol") ? &symbol : f == QLatin1String("wingdings") ? &wingdings : nullptr;
    if (!map || !map->contains(code)) return {};
    return QString(QChar(char16_t(map->value(code))));
}

uint unicodeToSymbol(QChar c, QString *family)
{
    for (const QString &f : {QStringLiteral("Symbol"), QStringLiteral("Wingdings")})
        for (uint code = 0x20; code <= 0xFF; ++code)
            if (symbolToUnicode(f, code) == QString(c)) {
                if (family) *family = f;
                return 0xF000 + code;
            }
    return 0;
}

bool symbolBullet(const QString &family, uint code, SymbolBullet *out)
{
    if (code >= 0xF000 && code <= 0xF0FF) code -= 0xF000;
    const QString f = family.toLower();
    if (f == QLatin1String("symbol")) {
        if (code == 0xB7) *out = {QString(), 0.458, QRectF(0.052, 0.103, 0.355, 0.355)};                     // the round bullet
        else if (code == 0x2A) *out = {QStringLiteral("*"), 0.5, QRectF(0.070, 0.129, 0.360, 0.405)};        // an asterisk
        else if (code == 0x3E) *out = {QStringLiteral(">"), 0, QRectF(0.090, 0.030, 0.410, 0.440)};          // a greater-than sign
        else return false;
        return true;
    }
    if (f == QLatin1String("wingdings")) {
        if (code == 0xE8) *out = {QString(QChar(0x2794)), 1.067, QRectF(0.085, 0, 0.899, 0.723)};            // a heavy arrow
        else if (code == 0xD8) *out = {QString(QChar(0x27A2)), 0.795, QRectF(0.084, 0, 0.627, 0.724)};      // an arrowhead
        else return false;
        return true;
    }
    return false;
}

QString substituteFor(const QString &family)
{
    if (haveFamily(family)) return {};
    return standInOnThisPlatform(table().value(family));
}

int stretchToAsk(int percent, bool appliedTwice)
{
    if (!appliedTwice) return percent;
    // The whole number s whose square, (s/100)^2, is nearest percent/100.
    const int want = 100 * std::max(percent, 1);
    int s = std::clamp(int(std::lround(std::sqrt(double(want)))), 1, 4000);
    for (const int c : {s - 1, s + 1})
        if (c >= 1 && c <= 4000 && std::abs(c * c - want) < std::abs(s * s - want)) s = c;
    return s;
}

QString standInOnThisPlatform(const QString &standIn)
{
    if (haveFamily(standIn)) return standIn;
    for (QString base = standIn; base.contains(QLatin1Char(' '));) {
        base.truncate(base.lastIndexOf(QLatin1Char(' ')));
        if (haveFamily(base)) return base;
    }
    return standIn;
}

QString substituteStyleName(const QString &family, bool bold, bool italic)
{
    if (haveFamily(family)) return {};
    const QString standIn = table().value(family), here = standInOnThisPlatform(standIn);
    if (here.size() < standIn.size()) return standIn.mid(here.size()).trimmed();
    // A face of the stand-in's family drawn by weight (Jost Medium, Roboto
    // Slab ExtraBold) is named too, where the family lists it. An italic run
    // isn't: that name is the upright face.
    if (italic || !weightsByName().contains(family.toLower())) return {};
    static QHash<QString, QString> named;   // main thread only, like all layout
    const QString key = family + (bold ? QStringLiteral("|1") : QStringLiteral("|0"));
    auto it = named.constFind(key);
    if (it == named.constEnd()) {
        const int w = substituteWeight(family, bold);
        const QString style = w == 500 ? QStringLiteral("Medium") : w == 800 ? QStringLiteral("ExtraBold") : QString();
        it = named.insert(key, !style.isEmpty() && QFontDatabase::styles(here).contains(style, Qt::CaseInsensitive) ? style : QString());
    }
    return *it;
}

QByteArray fontWithItalicFlags(const QByteArray &font)
{
    const auto u16 = [&](qsizetype at) { return at >= 0 && at + 2 <= font.size() ? quint16(uchar(font[at]) << 8 | uchar(font[at + 1])) : quint16(0); };
    const auto u32 = [&](qsizetype at) { return quint32(u16(at)) << 16 | u16(at + 2); };
    // A TrueType or OpenType file: version, then a directory of 16-byte table records from byte 12.
    if (const quint32 version = u32(0); version != 0x00010000 && version != 0x74727565 && version != 0x4F54544F) return {};
    qsizetype os2 = -1, head = -1, post = -1;
    for (qsizetype i = 0, n = u16(4); i < n && 12 + 16 * (i + 1) <= font.size(); ++i) {
        const qsizetype record = 12 + 16 * i;
        const QByteArray tag = font.mid(record, 4);
        if (tag == "OS/2") os2 = record;
        else if (tag == "head") head = record;
        else if (tag == "post") post = record;
    }
    if (os2 < 0 || head < 0 || post < 0) return {};
    // A table is inside the file, all of the length its record claims (the checksums are summed over it), and holds the bytes read from it.
    const auto inside = [&](qsizetype record, qsizetype size) { return u32(record + 12) >= size && qsizetype(u32(record + 8)) + qsizetype(u32(record + 12)) <= font.size(); };
    if (!inside(os2, 64) || !inside(head, 54) || !inside(post, 8)) return {};
    const qsizetype os2At = u32(os2 + 8), headAt = u32(head + 8), postAt = u32(post + 8);
    // Flagged italic already, or its letters are upright (the post table's italic angle).
    const quint16 selection = u16(os2At + 62);
    if ((selection & 1) || u32(postAt + 4) == 0) return {};
    QByteArray out(font.constData(), font.size());
    const auto set16 = [&](qsizetype at, quint16 v) { out[at] = char(v >> 8); out[at + 1] = char(v & 0xFF); };
    const auto set32 = [&](qsizetype at, quint32 v) { set16(at, quint16(v >> 16)); set16(at + 2, quint16(v & 0xFFFF)); };
    // The sum of a table's 32-bit words, padded with zeros.
    const auto sum = [&](qsizetype at, qsizetype length) {
        quint32 total = 0;
        for (qsizetype i = 0; i < length; i += 4) {
            quint32 word = 0;
            for (qsizetype k = 0; k < 4; ++k) word = word << 8 | (i + k < length ? uchar(out[at + i + k]) : 0);
            total += word;
        }
        return total;
    };
    set16(os2At + 62, quint16((selection | 1) & ~64));   // italic, not regular
    set16(headAt + 44, quint16(u16(headAt + 44) | 2));   // macStyle: italic
    set32(headAt + 8, 0);
    set32(os2 + 4, sum(os2At, u32(os2 + 12)));
    set32(head + 4, sum(headAt, u32(head + 12)));
    set32(headAt + 8, 0xB1B0AFBAu - sum(0, out.size()));
    return out;
}

QStringList standInFamiliesForTest()
{
    QStringList out = table().values();
    out.removeDuplicates();
    out.sort();
    return out;
}

void installFontSubstitutions()
{
    const auto &t = table();
    for (auto it = t.cbegin(); it != t.cend(); ++it) {
        if (QFontDatabase::hasFamily(it.key())) continue;
        QStringList subs{standInOnThisPlatform(it.value())};
        // Sensible generic fallbacks after the named substitute.
        subs << (it.value().contains("Mono") || it.value() == "Cousine" ? QStringLiteral("Cousine") : QStringLiteral("Arimo"));
        QFont::insertSubstitutions(it.key(), subs);
    }
}

} // namespace jp

namespace jp {

void initCore()
{
    static bool done = false;
    if (done) return;
    done = true;
    QImageReader::setAllocationLimit(2048);   // megabytes; print covers carry 600 dpi photos
    loadBundledFonts();
    installFontSubstitutions();
}

const QHash<char16_t, double> *originalLetterWidths(const QString &family, bool bold, bool italic)
{
    static const QHash<QString, QHash<char16_t, double>> table = [] {
        QHash<QString, QHash<char16_t, double>> t;
        if (!QFile::exists(QStringLiteral(":/fontwidths.txt"))) initCoreResources();
        QFile f(QStringLiteral(":/fontwidths.txt"));
        if (!f.open(QIODevice::ReadOnly)) return t;
        QVector<char16_t> chars;
        const QList<QByteArray> lines = f.readAll().split('\n');
        for (const QByteArray &line : lines) {
            if (line.isEmpty() || line.startsWith('#')) continue;
            const QList<QByteArray> p = line.split('\t');
            if (chars.isEmpty()) {
                for (const QByteArray &c : line.split(',')) chars << char16_t(c.toUInt());
                continue;
            }
            if (p.size() < 3) continue;
            QHash<char16_t, double> widths;
            const QList<QByteArray> ws = p[2].split(',');
            for (int i = 0; i < std::min(ws.size(), qsizetype(chars.size())); ++i)
                if (const int w = ws[i].toInt(); w >= 0) widths.insert(chars[i], w / 2048.0);
            t.insert(QString::fromUtf8(p[0]).toLower() + QLatin1Char('|') + QString::fromLatin1(p[1]), widths);
        }
        return t;
    }();
    const QString fam = family.toLower() + QLatin1Char('|');
    const int style = (bold ? 1 : 0) + (italic ? 2 : 0);
    for (int s : {style, style & 1, style & 2, 0})
        if (auto it = table.constFind(fam + QString::number(s)); it != table.constEnd()) return &*it;
    return nullptr;
}


const QHash<quint32, double> *originalKerning(const QString &family, bool bold, bool italic)
{
    static const QHash<QString, QHash<quint32, double>> table = [] {
        QHash<QString, QHash<quint32, double>> t;
        if (!QFile::exists(QStringLiteral(":/fontkerning.txt"))) initCoreResources();
        QFile f(QStringLiteral(":/fontkerning.txt"));
        if (!f.open(QIODevice::ReadOnly)) return t;
        const QList<QByteArray> lines = f.readAll().split('\n');
        for (const QByteArray &line : lines) {
            if (line.isEmpty() || line.startsWith('#')) continue;
            const QList<QByteArray> p = line.split('\t');
            if (p.size() < 3) continue;
            QHash<quint32, double> pairs;
            for (const QByteArray &e : p[2].split(',')) {
                const QList<QByteArray> v = e.split(':');
                if (v.size() == 3) pairs.insert(v[0].toUInt() << 16 | v[1].toUInt(), v[2].toInt() / 2048.0);
            }
            t.insert(QString::fromUtf8(p[0]).toLower() + QLatin1Char('|') + QString::fromLatin1(p[1]), pairs);
        }
        return t;
    }();
    const QString fam = family.toLower() + QLatin1Char('|');
    const int style = (bold ? 1 : 0) + (italic ? 2 : 0);
    for (int s : {style, style & 1, style & 2, 0})
        if (auto it = table.constFind(fam + QString::number(s)); it != table.constEnd()) return &*it;
    return nullptr;
}

bool originalBoldAlwaysByHand(const QString &family)
{
    const QString f = family.toLower();
    return f == QLatin1String("arial black") || f == QLatin1String("rockwell extra bold");
}

bool originalBoldByHand(const QString &family, bool boldSet)
{
    const QString f = family.toLower();
    // These two are stroked whether or not bold is set (in all 85 runs of
    // them in Publisher's PDFs); a "Bold" font is already bold, and Lucida's
    // bold is its Demibold face.
    if (originalBoldAlwaysByHand(family)) return true;
    if (!boldSet || f == QLatin1String("script mt bold") || f.startsWith(QLatin1String("lucida"))) return false;
    // From the faces fontwidths.txt measures: a family listed with no bold
    // (or bold italic) one has none.
    static const QHash<QString, bool> faces = [] {
        QHash<QString, bool> t;
        if (!QFile::exists(QStringLiteral(":/fontwidths.txt"))) initCoreResources();
        QFile file(QStringLiteral(":/fontwidths.txt"));
        if (!file.open(QIODevice::ReadOnly)) return t;
        for (const QByteArray &line : file.readAll().split('\n')) {
            const QList<QByteArray> p = line.split('\t');
            if (line.startsWith('#') || p.size() < 3) continue;
            bool &bold = t[QString::fromUtf8(p[0]).toLower()];
            bold |= (p[1].toInt() & 1) != 0;
        }
        return t;
    }();
    const auto it = faces.constFind(f);
    return it != faces.constEnd() && !*it;
}

} // namespace jp

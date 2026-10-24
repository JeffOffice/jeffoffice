#pragma once
// Reading a workbook's XML parts with a streaming reader (never a tree): the
// workbook part (the sheets, the date system, how it calculates), the shared
// strings, and each worksheet. A worksheet is read as it is inflated, in
// pieces, into the sheet's blocks of cells, so a sheet far larger than
// the memory it takes to hold its XML is read in the memory its cells take.

#include "office/package.h"
#include "calc/dates.h"
#include "sheet/sheetmodel.h"
#include "sheet/styles.h"

#include <QIODevice>
#include <QString>
#include <QVector>

#include <functional>
#include <memory>

namespace jp::office::sheet {

using jp::calc::DateSystem;

class XlsBook;

// What the files control, bounded. Each is at least ten times what any real
// workbook has; the most found in the 8,371 public workbooks used for tuning
// (its largest sheet has 151 MB of XML) is with each. A file past one is
// refused, with what was read before it kept. They are read from the
// workbook's own part, and the sheet's, and a Limits can say less (the tests do).
//
// Cells in one sheet (6,486,777 found; cells with a style and no value count).
constexpr qint64 kMaxCellsPerSheet = 100000000;
// What one sheet takes to hold, in bytes, as the reader counts it (a cell 32, a string or formula its slot and its block, a waiting shared formula its entry:
// each measured, rounded up, against the bytes a process held at its peak). Together with the bytes of the file (which a lazy package holds, at most 1 GB),
// the shared strings (about 260 MB at their limits), and the parser's own, it keeps a read within the 4 GB that a read may use whatever the other limits say.
// 207 MB is the most counted for a sheet found (its 6.5 million cells).
constexpr qint64 kMaxSheetBytes = 2200000000;
// Characters of one string, a cell's or a shared string (1,380 found; a cell holds 32,767 at most).
constexpr qint64 kMaxStringChars = 400000;
// Bytes of XML in one piece the parser is given: a text, a tag with its attributes, a comment, a CDATA section, or a processing instruction.
constexpr qint64 kMaxXmlTokenBytes = 4 * kMaxStringChars;
// Characters of one formula (943 found; the format allows 8,192).
constexpr qint64 kMaxFormulaChars = 100000;
// Distinct shared formulas in one sheet (2,131 found).
constexpr qint64 kMaxSharedFormulas = 1000000;
// Cells a shared formula says it covers, by its ref (407 found; a formula down a whole column of a sheet is 1,048,576).
constexpr qint64 kMaxSharedRangeCells = 100000000;
// All the formulas' characters in one sheet, a shared formula counted once for each cell it is in: what asking for every formula's text takes (8,054,139 found).
constexpr qint64 kMaxExpandedFormulaChars = 100000000;
// Sheets in one workbook (89 found).
constexpr int kMaxSheets = 1000;
// Characters of a sheet's name (the format allows 31).
constexpr int kMaxSheetNameChars = 1000;
// Shared strings (66,608 found), and all their characters together (547,609 found).
constexpr qint64 kMaxSharedStrings = 2000000;
constexpr qint64 kMaxSharedStringChars = 50000000;
// Number formats a workbook defines (1,736 found), cell formats (1,887 found; the format allows 64,000), and the characters of all the number formats' codes
// (a code is 233 characters at the most, and 4,096 are read).
constexpr qint64 kMaxNumFmts = 100000;
constexpr qint64 kMaxCellXfs = 1000000;
constexpr qint64 kMaxNumFmtChars = 4000000;
// Fonts, fills, and borders in the styles (cell formats have their own limit above). A font's name is 31 characters at the most in the format; 255 are kept, so that a styles part
// of a hundred thousand fonts each with a name of a million characters holds no more than the 4 GB budget says (the fonts take 50 MB at the most).
constexpr qint64 kMaxFonts = 100000;
constexpr int kMaxFontNameChars = 255;
constexpr qint64 kMaxFills = 100000;
constexpr qint64 kMaxBorders = 100000;
// `col` elements in one sheet (the columns that hold one each are 16,384 at the most), rows kept as not the default (a sheet has 1,048,576 rows; one a file names more
// than once counts each time), and merged ranges.
constexpr qint64 kMaxColumnSpans = 163840;
constexpr qint64 kMaxRowSpecs = 2097152;
constexpr qint64 kMaxMerges = 1000000;
// Defined names in a workbook (3,295 found), and the characters of all their names and what they stand for (a name stands for 4,553 characters at the most).
constexpr qint64 kMaxDefinedNames = 100000;
constexpr qint64 kMaxDefinedNameChars = 20000000;

struct Limits {
    qint64 cellsPerSheet = kMaxCellsPerSheet;
    qint64 sheetBytes = kMaxSheetBytes;
    qint64 stringChars = kMaxStringChars;
    qint64 formulaChars = kMaxFormulaChars;
    qint64 sharedFormulas = kMaxSharedFormulas;
    qint64 sharedRangeCells = kMaxSharedRangeCells;
    qint64 expandedFormulaChars = kMaxExpandedFormulaChars;
    int sheets = kMaxSheets;
    qint64 sharedStrings = kMaxSharedStrings;
    qint64 sharedStringChars = kMaxSharedStringChars;
    qint64 numFmts = kMaxNumFmts;
    qint64 cellXfs = kMaxCellXfs;
    qint64 numFmtChars = kMaxNumFmtChars;
    qint64 fonts = kMaxFonts;
    qint64 fills = kMaxFills;
    qint64 borders = kMaxBorders;
    qint64 columnSpans = kMaxColumnSpans;
    qint64 rowSpecs = kMaxRowSpecs;
    qint64 merges = kMaxMerges;
    qint64 definedNames = kMaxDefinedNames;
    qint64 definedNameChars = kMaxDefinedNameChars;
};

enum class CalcMode { Auto, AutoNoTable, Manual };
enum class SheetState { Visible, Hidden, VeryHidden };
enum class SheetKind { Worksheet, Chartsheet, Dialogsheet, Macrosheet };

struct SheetEntry {
    QString name;
    int sheetId = 0;
    SheetState state = SheetState::Visible;
    QString relId;        // the workbook's relationship to the sheet's part
    QString part;         // the part that holds it (when the package has one)
    SheetKind kind = SheetKind::Worksheet;
};

struct DefinedName {
    QString name;
    QString ref;            // what it stands for, as written: "Sheet1!$A$1", "'My Sheet'!$B$2:$C$9", or a formula
    int localSheet = -1;    // the place (counting from 0) of the sheet it belongs to; -1 for the whole workbook
    bool hidden = false;
};

struct WorkbookInfo {
    QVector<SheetEntry> sheets;   // in the order the workbook has them
    QVector<DefinedName> definedNames;
    DateSystem dateSystem = DateSystem::Date1900;
    CalcMode calcMode = CalcMode::Auto;
    bool fullCalcOnLoad = false;
    bool iterate = false;   // the workbook asks for iterative calculation (circular references are worked out by going round them)
    int activeTab = 0;      // the sheet (from 0, in the order the workbook has them) that was open when it was saved
    int firstSheet = 0;     // the first tab of the row of tabs that shows
};

// Reads the workbook part. Each sheet has its name, id, state, and the Id of
// its relationship (which Workbook::open turns into a part). False, with a
// message, if the XML is damaged, has a DOCTYPE, is nested too deeply, or has
// more sheets than the limit.
bool readWorkbookPart(QIODevice *in, const Limits &limits, WorkbookInfo *out, QString *error = nullptr);

// Reads the shared strings part. False, with a message, if the XML is
// damaged or something is past a limit; the strings read before it stay.
bool readSharedStrings(QIODevice *in, const Limits &limits, SharedStrings *out, QString *error = nullptr);

// Reads the styles part: the number formats it defines and the format of each cell format. False, with a message, if the XML is damaged or something is
// past a limit; what was read before it stays.
bool readStyles(QIODevice *in, const Limits &limits, Styles *out, QString *error = nullptr);

struct ReadOptions {
    Limits limits;
    // Stops, with no error, at the first row after this one (counted from 0): the sheet has the cells read so far, and `partial` says so. -1 reads it all. The
    // first screen of a large sheet is the rows it shows, read in a moment, and the rest is read afterwards.
    qint64 stopAfterRow = -1;
    // If above 0: reads what the sheet's view shows, and stops (with `partial`): the view's frozen rows, and this many rows from where the part that scrolls begins (not the first rows
    // of the sheet, which a view that was scrolled far has left behind; the cells of the rows between are not kept, but their heights are). A sheet whose view comes after its cells,
    // or says nothing, begins at its first row.
    qint64 screenRows = 0;
    DateSystem dateSystem = DateSystem::Date1900;   // for the cells of type "d", written as dates
    // Called after every 4,096 cells, with how many have been read; false
    // stops the read (the sheet's error says it was stopped).
    std::function<bool(qint64 cells)> progress;
};

// Reads a worksheet part into `out` (which is cleared first). True if the
// whole part was read; else `out` holds what came before the problem and
// out->error says what it was. Every sheet is finished: its blocks are in order.
bool readSheetPart(QIODevice *in, const ReadOptions &options, Sheet *out);

// A workbook: what its package says about it, with the sheets still in the
// package, to be read one at a time.
class Workbook {
public:
    // Reads the workbook part and the shared strings, and finds each sheet's
    // part by the workbook's relationships. The package is kept (its parts
    // are shared, not copied; for a lazy package none is inflated beyond
    // these two). False, with a message, if the package has no workbook part
    // or it can't be read. A shared strings part that is damaged is not
    // fatal: sharedStringsError() says, and the strings before the damage stay.
    bool open(const Package &package, QString *error = nullptr, const Limits &limits = Limits());
    // Opens a workbook file of either kind, told by its first bytes: the package of XML parts (.xlsx, .xlsm), or the older binary form (a compound file holding a
    // workbook stream: BIFF5 to BIFF8). A package is read lazily, as readLazyFile does. False, with a message, if the file is neither, is damaged, or is encrypted.
    bool openFile(const QString &path, QString *error = nullptr, const Limits &limits = Limits());
    // The same for the bytes of a whole binary workbook file.
    bool openBinary(const QByteArray &file, QString *error = nullptr, const Limits &limits = Limits());
    // Whether the workbook was read from the older binary form, and what such a file holds that is not read (charts, pictures, ...), in words.
    bool isBinary() const { return bool(m_xls); }
    QStringList skippedFeatures() const;
    const WorkbookInfo &info() const { return m_info; }
    const SharedStrings &sharedStrings() const { return m_shared; }
    QString sharedStringsError() const { return m_sharedError; }
    // The number formats: none (everything General) if the workbook has no styles part. A part that is damaged or past a limit keeps what was read before the
    // problem, and stylesError() says what it was.
    const Styles &styles() const { return m_styles; }
    QString stylesError() const { return m_stylesError; }
    // Reads sheet `index`. A sheet of a chart, and one with no relationship to a part, is read as an empty sheet; false, with the sheet's error saying why, if
    // there is no such sheet or its part is missing, or if the part could not be read whole.
    // `stopAfterRow` (from 0) reads only the rows up to it, and the sheet says it is `partial` (ReadOptions::stopAfterRow); -1 reads it all.
    // `screenRows` reads what the view shows, from where it begins (ReadOptions::screenRows).
    bool readSheet(int index, Sheet *out, const std::function<bool(qint64)> &progress = nullptr, qint64 stopAfterRow = -1, qint64 screenRows = 0) const;
    const Package &package() const { return m_package; }

private:
    bool openStream(const QByteArray &stream, bool hasMacros, QString *error, const Limits &limits);
    Package m_package;
    std::shared_ptr<const XlsBook> m_xls;
    WorkbookInfo m_info;
    SharedStrings m_shared;
    QString m_sharedError;
    Styles m_styles;
    QString m_stylesError;
    Limits m_limits;
};

} // namespace jp::office::sheet

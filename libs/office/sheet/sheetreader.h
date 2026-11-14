#pragma once
// Reading a workbook's XML parts with a streaming reader (never a tree): the
// workbook part (the sheets, the date system, how it calculates), the shared
// strings, and each worksheet. A worksheet is read as it is inflated, in
// pieces, into the sheet's blocks of cells, so a sheet far larger than
// the memory it takes to hold its XML is read in the memory its cells take.

#include "office/package.h"
#include "calc/dates.h"
#include "sheet/csvimport.h"
#include "sheet/pivotbook.h"
#include "sheet/slicer.h"
#include "sheet/customviews.h"
#include "sheet/protection.h"
#include "sheet/sheetmodel.h"
#include "sheet/tables.h"
#include "sheet/styles.h"

#include <QHash>
#include <QIODevice>
#include <QMutex>
#include <QSet>
#include <QString>
#include <QVector>

#include <functional>
#include <memory>
#include <map>
#include <unordered_map>
#include <vector>

namespace jp::office::sheet {

using jp::calc::DateSystem;

class XlsBook;
class ForeignBook;

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
// The most cell formats the program adds to a workbook (a person's formatting makes them): the format's own practical limit, which the programs that open a workbook keep to.
constexpr qint64 kMaxCellFormatsWritten = 64000;
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
// The work reading formulas takes in one sheet of a workbook of the old binary form: the bytes of tokens read, the pieces of text made of them, and the pieces written for each cell
// of a formula that cannot be shared (a unit for each; 3,000,000 found in the largest of the practice workbooks, its 190,000 formulas).
constexpr qint64 kMaxFormulaWork = 100000000;
// Defined names in a workbook (3,295 found), and the characters of all their names and what they stand for (a name stands for 4,553 characters at the most).
constexpr qint64 kMaxDefinedNames = 100000;
constexpr qint64 kMaxDefinedNameChars = 20000000;
// The bytes of the content of an OpenDocument spreadsheet (its sheets are in one part, which is held whole once it is inflated, so that each sheet is read from its own piece of it).
// No real file comes near it; 768 MB is ten times what the largest public sheet's XML comes to.
constexpr qint64 kMaxOdsContentBytes = qint64(768) << 20;
// Styles an OpenDocument spreadsheet defines (cell styles, and data styles for number formats), and the properties of all of them together. Real documents have thousands of styles
// (the most found in the test files of the other program is under 1,000) and some tens of thousands of properties; a style costs about 4 KB while the formats are made, so 100,000
// of them stay well inside the 4 GB a read may use.
constexpr qint64 kMaxOdsStyles = 100000;
constexpr qint64 kMaxOdsProperties = 1000000;
constexpr qint64 kMaxOdsFonts = 10000;          // the fonts a document names
constexpr qint64 kMaxOdsDataParts = 1000000;    // the parts of all the number formats of a document, together
// Runs of formatting inside the strings of a workbook, all together (the runs of the strings are kept only when asked for, or for a workbook of the older binary form; past this the strings
// are kept without their runs).
constexpr qint64 kMaxRichRuns = 5000000;

// Why a file was not opened for want of a password: it needs one and none was given, or the one given is not it.
enum class PasswordNeed { None, Needed, Wrong };

struct Limits {
    // The bytes of a workbook file (the most the apps open for every kind: office/package.h); a larger file is refused before any of it is read.
    qint64 fileBytes = kMaxOpenFileBytes;
    qint64 formulaWork = kMaxFormulaWork;
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
    // Keep the runs of formatting in the shared strings of a workbook of the XML form (SharedStrings::rich); off, they are read for their text only, which is all a program that
    // does not draw them needs.
    bool richText = false;
    qint64 richRuns = kMaxRichRuns;
    qint64 odsContentBytes = kMaxOdsContentBytes;
    qint64 odsStyles = kMaxOdsStyles;
    qint64 odsProperties = kMaxOdsProperties;
    qint64 odsFonts = kMaxOdsFonts;
    qint64 odsDataParts = kMaxOdsDataParts;
    // Asked now and then while a file is read (a worker looks at its stop flag here): true stops the read, with the message that it was stopped. Optional.
    std::function<bool()> cancelled;
    // For a text file of delimited values: what a person chose for it when it was opened (none: detected).
    std::shared_ptr<const CsvImport> csv;
    // The most bytes the links of all the sheets hold (see kMaxLinkBytesPerBook).
    qint64 linkBytes = kMaxLinkBytesPerBook;
    // The password of a file that is protected with one (none: the password the original program gives every file it protects only against changes is tried). The workbook does not keep it:
    // openFile and openBinary clear their copy when they end. `passwordNeed`, when given, is told why a protected file was refused.
    QString password;
    PasswordNeed *passwordNeed = nullptr;
    bool *usedDefaultPassword = nullptr;   // told true when the file opened with the password every program knows
};

enum class CalcMode { Auto, AutoNoTable, Manual };
enum class SheetState { Visible, Hidden, VeryHidden };
enum class SheetKind { Worksheet, Chartsheet, Dialogsheet, Macrosheet };

struct SheetEntry {
    QString name;                 // the name now (a person may have renamed the sheet; empty for a sheet that was deleted)
    QString fileName;             // the name the package's workbook part has for it (what the files' formulas say, until the next save)
    bool deleted = false;         // a sheet a person deleted: it keeps its place, and is not in the order of the tabs
    bool isNew = false;           // a sheet a person added: it has no part in the package yet
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
    QString comment;        // what the file says it is for
};

struct WorkbookInfo {
    QVector<SheetEntry> sheets;   // by place: the order the file has them in, then those a person added (a sheet's place never changes)
    QVector<int> order;           // the places of the sheets in the order of the tabs (those deleted are not in it)
    QVector<int> fileOrder;       // the places of the sheets in the order the package's workbook part has them (what `activeTab` and the `localSheet` of the file's names count by, until a save)
    QVector<DefinedName> definedNames;
    DateSystem dateSystem = DateSystem::Date1900;
    CalcMode calcMode = CalcMode::Auto;
    bool calcOnSave = true;         // by hand, the formulas are worked out when the workbook is saved (calcPr calcOnSave: true when not written)
    bool fullCalcOnLoad = false;
    bool calcUnfinished = false;   // saved before its calculation finished (calcCompleted="0", or the old form's UNCALCED record): the results it holds are not the program's own answers
    bool iterate = false;   // the workbook asks for iterative calculation (circular references are worked out by going round them)
    int iterateCount = 100;         // the most rounds it goes
    double iterateDelta = 0.001;    // and it stops when no value changes by as much as this
    int activeTab = 0;      // the sheet (from 0, in the order the workbook has them) that was open when it was saved
    int firstSheet = 0;     // the first tab of the row of tabs that shows
    WorkbookProtection protection;   // the protection of the workbook's structure (workbookProtection)
    QVector<CustomView> customViews; // the named views (customWorkbookViews)
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
    // With screenRows: the rows above the view are cut down before the parser sees them (see rowskip.h), which is far faster for a sheet saved scrolled far down; false gives every row to the
    // parser (what a test compares the result with).
    bool skipRows = true;
    // The rows of plain cells (numbers, shared strings, formulas with their results) are read by a scanner of their own, many times faster than the XML parser, and every other row is given to the
    // parser as it was; false gives every row to the parser (what a test compares the result with).
    bool fastRows = true;
    DateSystem dateSystem = DateSystem::Date1900;   // for the cells of type "d", written as dates
    // The numbers (from 1, as a cell's `cm` says them) of the cell metadata that mark a dynamic array, from the workbook's metadata part: a cell with an array formula and one of these is a formula that
    // spills (Cell::SpillFormula), and any other array formula is the older kind, written with Ctrl+Shift+Enter.
    QSet<int> dynamicCells;
    // The workbook's allowance for sparklines (all its sheets together), and which sheet this is.
    std::shared_ptr<SparklineBudget> sparklineBudget;
    int sparklineSheet = -1;
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
    // workbook stream: BIFF5 to BIFF8). A package is read lazily, as readLazyFile does. False, with a message, if the file is neither, is damaged, or is protected with a password that was not given (limits.password) or is not the one.
    bool openFile(const QString &path, QString *error = nullptr, const Limits &limits = Limits());
    // The same for the bytes of a whole binary workbook file.
    bool openBinary(const QByteArray &file, QString *error = nullptr, const Limits &limits = Limits());
    // Why the last openFile or openBinary that failed did, if it was for a password (None for any other failure, and after one that worked).
    PasswordNeed passwordNeed() const { return m_passwordNeed; }
    // Whether the file the workbook was read from was protected with a password (the default one included): a save writes it protected again.
    bool wasProtected() const { return m_protected; }
    // ... and whether that was only the password every program knows (a file protected only against changes by accident, which opens with no question).
    bool openedWithDefaultPassword() const { return m_defaultPassword; }
    // The SHA-256 of the bytes of a protected file as they were read (empty for any other file).
    QByteArray fileDigest() const { return m_fileDigest; }
    // Whether the workbook was read from the older binary form, and what such a file holds that is not read (charts, pictures, ...), in words.
    bool isBinary() const { return bool(m_xls); }
    // Whether it was read from a file that has no package of XML parts to put changes in, and so is saved as an .xlsx written from what was read: the older binary form, an
    // OpenDocument spreadsheet (.ods, .fods), or a file of delimited values (.csv). convertedKind() says which (".xls", ".ods", ".csv"); empty for any other.
    bool isConverted() const { return m_xls || m_foreign; }
    // How many charts a sheet of a workbook of the older binary form has (0 for any other): known before the sheet is read, so that a file can be laid out first.
    int importedChartCount(int sheet) const;
    QString convertedKind() const;
    // The reader of a file that is neither of the XML form nor the older binary form (an OpenDocument spreadsheet, a file of delimited values), or null.
    const ForeignBook *foreignBook() const { return m_foreign.get(); }
    QStringList skippedFeatures() const;
    QStringList changedFeatures() const;
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
    // The package the window's objects are shown from: for a workbook read from an OpenDocument spreadsheet that has drawing objects, a package made of them (makeObjectPackage, set by whoever opens
    // the file), named as the .xlsx a save writes names them, so that what the window says of an object is true of that file; for any other workbook, its own.
    const Package &objectPackage() const { return m_objectPackage ? *m_objectPackage : m_package; }
    void setObjectPackage(std::shared_ptr<const Package> package) { m_objectPackage = std::move(package); }
    // The pictures of a sheet's headers and footers, read from the legacy drawing its `legacyDrawingHF` names (readSheet does this for a sheet that has one). Bounded: the shapes looked at, the
    // pictures kept, the bytes of one and of all those a workbook reads (an image shared by several sheets is read once).
    void readHeaderPictures(const QString &sheetPart, HeaderFooter *out) const;
    // How many drawings of header pictures have been read (parsed) since the workbook was opened: a drawing that sheets share is read once.
    int headerDrawingsRead() const { return m_headerImages->parses; }
    // The parts the workbook is made of (empty for one it has not): its own, the shared strings, and the styles.
    QString mainPart() const { return m_mainPart; }
    // The cell metadata (a cell's `cm`, from 1) that mark a dynamic array in the workbook's metadata part; and whether the workbook has a metadata part at all.
    QSet<int> dynamicCellMetadata() const;
    bool hasMetadataPart() const;
    QString sharedStringsPart() const { return m_sharedPart; }
    QString stylesPart() const { return m_stylesPart; }
    // The number of strings in the shared strings part and of cell formats in the styles part when the workbook was opened (or last saved): what is added to either afterwards goes at the end.
    qint64 sharedStringsInFile() const { return m_sharedInFile; }
    qint64 cellFormatsInFile() const { return m_formatsInFile; }

    // ---- changes (the window's, once a person edits). They only add: a place already given is never moved.
    // The place of a shared string that holds the text, added at the end if it is not among those added since the file was read.
    qint32 addSharedString(const QString &text);
    // The id of a number format with the code (the built-in one, or one the file has, or a new one), and the place of the cell format that is the cell format `base` with that number
    // format (the one it is already, or one made).
    int numberFormatId(const QString &code);
    quint32 formatWithNumberFormat(quint32 base, int numFmtId);
    // The place of the cell format that is `base` with the change: one of the workbook's that is exactly that (a cell format is never made twice), else a new one at the end, with the
    // font, fill, and border it needs found among the workbook's or added. `base` itself when the change changes nothing (or the workbook can take no more cell formats).
    quint32 formatWith(quint32 base, const FormatChange &change);
    // Whether a format (a cell format, a font, a fill, or a border) was wanted since `clearFormatLimit` that the workbook could take no more of: the caller then says so, and does not go
    // on with what it was doing with `base`.
    bool formatLimitHit() const { return m_formatLimitHit; }
    void clearFormatLimit() { m_formatLimitHit = false; }
    // How many fonts, fills, and borders were compared with another to find a place for one (found by a hash, it is a few for each, not as many as the workbook has).
    qint64 placeComparisons() const { return m_placeComparisons; }
    // The place of the style format a named style has (by its name), made when the workbook does not have it and it is one of the built-in styles (see builtinStyleNames); -1 if there is none.
    qint32 styleNamed(const QString &name);
    // A style of a person's own, named `name` (made from the cell format at `format`): its place among the style formats; -1 if the name is taken (a style with that name, whatever the capitals),
    // is empty or too long, or the workbook has no place for styles or no room.
    qint32 addNamedStyle(const QString &name, quint32 format);
    // Takes a style made by addNamedStyle back out (Undo), with the cell formats made since that follow it: only while the file has not been saved with it, and only when it is the last
    // style there is. False (and nothing changed) otherwise.
    bool removeAddedStyle(const QString &name);
    // Hides a named style from the lists of styles (the file keeps it, marked hidden, because cells may still name it), or shows it again; false if the workbook has none by that name. What
    // was changed is kept, as the name and whether it is hidden now, for the saver to write.
    bool setStyleHidden(const QString &name, bool hidden);
    const QVector<QPair<QString, bool>> &styleVisibility() const { return m_stylesVisibility; }
    // The additions since the file was read or saved, for saving them: the strings, the number formats (id and code), and the cell formats (the one each is a copy of, and its number format).
    QStringList addedSharedStrings() const { return m_sharedAdded; }
    struct AddedNumberFormat {
        int id;
        QString code;
    };
    struct AddedCellFormat {
        quint32 base;        // the cell format it is a copy of (in the file, or added before)
        CellFormat format;   // and what it is: the places of its number format, font, fill, and border are the ones in the file as it will be written
    };
    QVector<AddedNumberFormat> addedNumberFormats() const { return m_numFmtsAdded; }
    QVector<AddedCellFormat> addedCellFormats() const { return m_formatsAdded; }
    QVector<FontSpec> addedFonts() const { return m_fontsAdded; }
    // The place of the differential format that is only a fill of this color (or, with `font`, only a font of it): one the workbook has, else one added at the end (the file's `dxfs`).
    // -1 if the workbook has no styles part to put one in, or has as many as it may.
    qint32 dxfForColor(const ColorSpec &color, bool font);
    // The place of a differential format with a font color, a fill, bold, or italic (not a border or a number format): one the workbook has that is exactly that, else one added at the end. -1 if
    // the workbook has no styles part to put one in, or has as many as it may.
    qint32 dxfFor(const DifferentialFormat &format);
    // Takes the differential format at `place` back out when it is the last, was added since the file was read or saved, and `place` is where it was put (Undo). False otherwise.
    bool removeAddedDxf(qint32 place);
    // Defines a table style of the workbook's own: the differential format of each part (the formats are put among the file's `dxfs`, as dxfFor does). False, with the reason, if the name is empty,
    // is taken (by a built-in style or one the workbook has), or the workbook has no styles part to put it in or has as many as it may.
    struct TableStylePart {
        TablePart part = TablePart::WholeTable;
        DifferentialFormat format;
    };
    bool defineTableStyle(const QString &name, const QVector<TableStylePart> &parts, QString *why = nullptr);
    // Takes a style defined since the file was read or saved back out, with the formats it added (Undo). False if the file has it.
    bool removeAddedTableStyle(const QString &name);
    // A number that changes whenever a table style is made or taken out (a window keeps what it drew from the styles as long as it is the same).
    quint64 tableStyleStamp() const { return m_tableStyleStamp; }
    QVector<CustomTableStyle> addedTableStyles() const { return m_tableStylesAdded; }
    // Whether the rules of conditional formatting the sheet's file has are all ones this program holds whole: not a rule of a kind it does not know, one with an extension list (the 2010 forms
    // of icon sets and data bars link to one), a table's `pivot` mark, the 2010 extension block, or more ranges or rules than are read. Only then may the rules be written anew.
    bool conditionalFormatsAreWhole(int sheet) const;
    QVector<DifferentialFormat> addedDxfs() const { return m_dxfsAdded; }
    qint64 dxfsInFile() const { return m_dxfsInFile; }
    QVector<FillSpec> addedFills() const { return m_fillsAdded; }
    QVector<BorderSpec> addedBorders() const { return m_bordersAdded; }
    // The named styles added (built-in ones the file did not have): the style format of each, and the name.
    struct AddedStyle {
        CellFormat format;
        CellStyleName name;
    };
    QVector<AddedStyle> addedStyles() const { return m_stylesAdded; }
    qint64 fontsInFile() const { return m_fontsInFile; }
    qint64 fillsInFile() const { return m_fillsInFile; }
    qint64 bordersInFile() const { return m_bordersInFile; }
    qint64 styleXfsInFile() const { return m_styleXfsInFile; }
    // The file has been written with the first `strings`, `numberFormats`, and `cellFormats` (and the fonts, fills, borders, and styles) of what was added: the package is the new file's, and
    // those are in it.
    struct SavedCounts {
        int strings = 0, numberFormats = 0, cellFormats = 0, fonts = 0, fills = 0, borders = 0, styles = 0, dxfs = 0, tableStyles = 0;
    };
    void savedTo(const Package &package, const SavedCounts &counts);
    void savedTo(const Package &package, int strings, int numberFormats, int cellFormats)
    {
        SavedCounts c;
        c.strings = strings;
        c.numberFormats = numberFormats;
        c.cellFormats = cellFormats;
        savedTo(package, c);
    }

    // ---- the sheets as a person changes them (see apps/crunch/core/editsession.h). A sheet's place never changes: one added is given the next place, and one deleted keeps its own, marked.
    // What the file has stays in `fileName` and `part` until a save. The window's thread only, with the worker held back.
    int addSheetEntry(const QString &name, SheetState state = SheetState::Visible, SheetKind kind = SheetKind::Worksheet);
    void removeLastSheetEntry();   // (Undo of adding one)
    void setSheetName(int sheet, const QString &name);
    void setSheetState(int sheet, SheetState state);
    void setSheetDeleted(int sheet, bool deleted);
    // The sheets, names, and the rest of the workbook's list as they are now (a copy of another reading of the same file takes the list of the window's: a save made on a thread of its own).
    void setInfo(const WorkbookInfo &info) { m_info = info; }
    void setSheetOrder(const QVector<int> &order) { m_info.order = order; }
    void setWorkbookProtection(const WorkbookProtection &protection) { m_info.protection = protection; }
    void setCustomViews(const QVector<CustomView> &views) { m_info.customViews = views; }
    // The tab that is open in the file, by its place in the order the tabs are in now: where the sheet the file says is open is (or, if it was deleted or is hidden, the first that is shown).
    int activeTabNow() const;
    // The part, relationship, and sheet number the package would have for a new sheet (names none that the package, or another new sheet, has), chosen once and kept.
    void planSheetPart(int sheet);

    // ---- tables (see sheet/tables.h): read from the package when first asked for
    const QVector<Table> &tables() const;
    // The table whose header row holds the cell, and the place of its column in the table; -1 if the cell is no table's header.
    int tableHeaderAt(int sheet, quint32 row, quint32 col, int *column) const;
    // Names a column (an edit, or its undoing); the table's part is written with the names when the workbook is saved.
    void setTableColumnName(int table, int column, const QString &name);
    // The cells the pivot tables of a sheet cover, as the file says (a pivot table that can't be read covers the whole sheet, so that nothing is done to it).
    QVector<CellRange> pivotTablesOn(int sheet) const;
    // The pivot tables and caches, as the file has them and as a person has changed them (made when first asked for).
    PivotBook &pivots() const;
    // The slicers' and timelines' caches the workbook names (as the file has them), and the slicers or timelines a sheet has (by the sheet's relationships), each part read once.
    QVector<SlicerCache> slicerCaches() const;
    // An item of a pivot cache's field was taken away (a calculated item): the slicers that read the field count their items again (those after it are one place nearer the first).
    void slicerItemRemoved(int pivotCache, int field, qint32 item);
    QVector<SlicerView> slicerViews(int sheet) const;
    // The slicer parts this session has written with a choice of their own: they are looked at again at each save (Undo may bring the table back to what the file had before).
    QSet<QString> &slicerPartsWritten() const { return m_slicerPartsWritten; }
    std::map<QString, std::function<QByteArray()>> changedPivotPartProducers() const;
    // Whether the tables allow the edit (not when it would cut through one, or remove one or its header row); if not, why.
    bool tablesAllow(const StructEdit &edit, QString *why) const;
    // Moves the tables for the edit (those it changes), and says where each was; putTablesBack puts them there again.
    std::vector<TablePlace> moveTables(const StructEdit &edit);
    void putTablesBack(const std::vector<TablePlace> &places);
    // The table parts of the file that are not as the file has them now (renamed, moved, restyled, ...), as they would be written, by the part's name.
    std::map<QString, QByteArray> changedTableParts() const;
    // ---- tables a person makes and changes (the window's thread, the worker held back). A table is put in the list, replaced, or taken out whole, so that a step can be undone; the list of a
    // workbook that is saved keeps its order (the places in it are what the steps name).
    void setTableAt(int index, const Table &table);
    int insertTable(int index, const Table &table);   // at `index` (-1 or past the end: last); the place it has
    void eraseTable(int index);
    // A part name no part of the package, and no table, has; an id no table has.
    QString freshTablePart() const;
    int freshTableId() const;
    // For saving: the tables made here (the file has no part for them) with their parts as they will be written, and the parts of the file whose tables were taken out (with the sheet
    // that had them).
    struct TableAdd {
        int sheet = 0;
        QString part;
        QByteArray xml;
    };
    std::vector<TableAdd> addedTables() const;
    struct TableGone {
        int sheet = 0;
        QString part;
    };
    std::vector<TableGone> removedTables() const;

private:
    bool openStream(const QByteArray &stream, bool hasMacros, QString *error, const Limits &limits);
    bool openFileWith(const QString &path, QString *error, const Limits &limits);
    bool openProtectedFile(const QString &path, QString *error, const Limits &limits);
    bool openBinaryWith(const QByteArray &file, QString *error, const Limits &limits);
    bool openProtectedPackage(const QByteArray &info, const QByteArray &package, QString *error, const Limits &limits);
    bool openPackageBytes(const QByteArray &bytes, QString *error, const Limits &limits);
    void prepareSheetList();
    bool openOpenDocument(const QByteArray &file, bool flat, QString *error, const Limits &limits);
    bool openDelimited(const QByteArray &file, const QString &path, QString *error, const Limits &limits);
    bool openHtml(const QByteArray &file, const QString &path, QString *error, const Limits &limits);
    void nameBaseParts();
    Package m_package;
    std::shared_ptr<const XlsBook> m_xls;
    std::shared_ptr<const ForeignBook> m_foreign;
    std::shared_ptr<const Package> m_objectPackage;
    QString m_mainPart, m_sharedPart, m_stylesPart;
    qint64 m_sharedInFile = 0, m_formatsInFile = 0;
    QStringList m_sharedAdded;
    QHash<QString, qint32> m_sharedAddedAt;
    QVector<AddedNumberFormat> m_numFmtsAdded;
    QVector<AddedCellFormat> m_formatsAdded;
    QVector<FontSpec> m_fontsAdded;
    QVector<FillSpec> m_fillsAdded;
    QVector<BorderSpec> m_bordersAdded;
    QVector<AddedStyle> m_stylesAdded;
    QVector<QPair<QString, bool>> m_stylesVisibility;
    QVector<DifferentialFormat> m_dxfsAdded;
    qint64 m_dxfsInFile = 0;
    quint64 m_tableStyleStamp = 1;
    QVector<CustomTableStyle> m_tableStylesAdded;
    QVector<int> m_tableStyleDxfsBefore;   // for each style added, how many differential formats there were before it
    qint64 m_fontsInFile = 0, m_fillsInFile = 0, m_bordersInFile = 0, m_styleXfsInFile = 0;
    std::unordered_multimap<quint64, quint32> m_formatIndex;   // the cell formats by a hash of what they are, to find an equal one (made when first needed)
    bool m_formatIndexMade = false;
    quint32 fontPlace(const FontSpec &font);
    quint32 fillPlace(const FillSpec &fill);
    quint32 borderPlace(const BorderSpec &border);
    quint32 findOrAddFormat(quint32 base, const CellFormat &format);
    std::unordered_multimap<quint64, quint32> m_fontIndex, m_fillIndex, m_borderIndex;   // the fonts, fills, and borders by a hash of what they are (made when first needed)
    bool m_fontIndexMade = false, m_fillIndexMade = false, m_borderIndexMade = false;
    struct HeaderImages {
        QMutex mutex;
        QHash<QString, QByteArray> images;
        qint64 bytes = 0;
        QHash<QString, QVector<HeaderPicture>> drawings;   // by the drawing's part (lower case): what it was read as, once for the workbook
        int parses = 0;
    };
    mutable std::shared_ptr<HeaderImages> m_headerImages = std::make_shared<HeaderImages>();
    // What the links of each sheet read hold, so that the links of all the sheets can be kept within one budget (a sheet read again replaces what it was counted as).
    struct LinkBudget {
        QMutex lock;
        std::map<int, qint64> perSheet;
    };
    mutable std::shared_ptr<LinkBudget> m_linkBudget = std::make_shared<LinkBudget>();
    bool m_formatLimitHit = false;
    qint64 m_placeComparisons = 0;
    WorkbookInfo m_info;
    mutable QSet<QString> m_slicerPartsWritten;
    mutable bool m_slicerCachesRead = false;           // (the package changes only when a save makes a new one: they are read again then)
    mutable QVector<SlicerCache> m_slicerCaches;
    mutable QHash<int, QVector<SlicerView>> m_slicerViews;
    SharedStrings m_shared;
    QString m_sharedError;
    Styles m_styles;
    QString m_stylesError;
    Limits m_limits;
    std::shared_ptr<SparklineBudget> m_sparklineBudget = std::make_shared<SparklineBudget>();
    PasswordNeed m_passwordNeed = PasswordNeed::None;
    bool m_protected = false;
    bool m_defaultPassword = false;
    QByteArray m_fileDigest;
    mutable QVector<Table> m_tables;
    mutable bool m_tablesRead = false;
    mutable QStringList m_tableNotes;   // what was left out of the tables when they were read (see readTables)
    mutable QSet<QString> m_tablePartSet;   // the parts of the file that are tables' (by the part's name), for looking one up
    mutable std::vector<std::pair<QString, int>> m_tablesInFile;   // the parts of the file that are tables' (by the part's name), and the sheet each is related to
    mutable std::shared_ptr<PivotBook> m_pivotBook;
};

} // namespace jp::office::sheet

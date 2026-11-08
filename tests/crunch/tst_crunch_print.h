#pragma once
// Tests of printing in JeffCrunch's window: how a sheet prints as a step of Undo, the print area, titles, and breaks following rows and columns inserted and deleted (and coming back with
// Undo), the page setup saved in the file and nothing else of it touched, and the pages the window prints. Every workbook is built here, in a temporary folder.
#include "../sheet/drawinghelpers.h"
#include "../sheet/hfpicturehelpers.h"
#include "app/crunchfilescreen.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "savedok.h"
#include "app/pagesetupdialog.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "app/printsides.h"
#include "app/sheettabs.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "core/printjob.h"
#include "grid/gridview.h"
#include "grid/hfpicturecache.h"
#include "grid/pagelayoutview.h"
#include "io/zipindex.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPaintDevice>
#include <QPainter>
#include <QPaintEngine>
#include <QPrinter>
#include <QScrollArea>
#include <QPushButton>
#include <QRadioButton>
#include <QScopeGuard>
#include <QSpinBox>
#include <QTabWidget>
#include <QSettings>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QStatusBar>
#include <QToolButton>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;
using jp::office::sheet::PageOrientation;
using jp::office::sheet::PageSetup;

namespace detail_tst_crunch_print {

// Two sheets with a few numbers; the second has a note of its own so that its part can be told from the first's.
inline QByteArray printBook()
{
    QString rows;
    for (int r = 1; r <= 12; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>%2</v></c></row>").arg(r).arg(r * 2);
    const QByteArray first = worksheet(rows, QStringLiteral("<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>"));
    const QByteArray second = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>second</t></is></c></row>"));
    return buildXlsx({first, second}, QByteArray(), QString(), QString(), QByteArray(), QStringList({"Data", "Notes"}));
}

inline QString areaOf(const PageSetup &p)
{
    QStringList l;
    for (const jp::office::sheet::CellRange &r : p.printArea) l << rangeName(r);
    return l.join(QLatin1Char(' '));
}

inline PageSetup withArea(PageSetup p, const QString &ref)
{
    jp::office::sheet::CellRange r;
    parseRangeRef(ref, &r);
    p.printArea = {r};
    return p;
}

// A saved file's page setup for a sheet.
inline PageSetup savedPage(const QString &path, int sheet = 0)
{
    Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return PageSetup();
    Workbook b;
    if (!b.open(p, &why)) return PageSetup();
    Sheet s;
    if (!b.readSheet(sheet, &s)) return PageSetup();
    return s.page;
}

// A paint device that draws nothing and writes down the texts it is asked to draw, in order.
class TextEngine : public QPaintEngine {
public:
    TextEngine() : QPaintEngine(QPaintEngine::PaintEngineFeatures()) {}
    bool begin(QPaintDevice *) override { return true; }
    bool end() override { return true; }
    void updateState(const QPaintEngineState &) override {}
    void drawPixmap(const QRectF &, const QPixmap &, const QRectF &) override {}
    void drawImage(const QRectF &, const QImage &, const QRectF &, Qt::ImageConversionFlags) override {}
    void drawPath(const QPainterPath &) override {}
    void drawPolygon(const QPointF *, int, PolygonDrawMode) override {}
    void drawPolygon(const QPoint *, int, PolygonDrawMode) override {}
    void drawRects(const QRectF *, int) override {}
    void drawRects(const QRect *, int) override {}
    void drawLines(const QLineF *, int) override {}
    void drawLines(const QLine *, int) override {}
    void drawTextItem(const QPointF &, const QTextItem &item) override { texts << item.text(); }
    Type type() const override { return User; }
    QStringList texts;
};

class TextDevice : public QPaintDevice {
public:
    QPaintEngine *paintEngine() const override { return &m_engine; }
    QStringList texts() const { return m_engine.texts; }

protected:
    int metric(PaintDeviceMetric m) const override
    {
        switch (m) {
        case PdmWidth: return 5100;
        case PdmHeight: return 6600;
        case PdmWidthMM: return 216;
        case PdmHeightMM: return 279;
        case PdmNumColors: return 16777216;
        case PdmDepth: return 32;
        case PdmDpiX:
        case PdmDpiY:
        case PdmPhysicalDpiX:
        case PdmPhysicalDpiY: return 600;
        case PdmDevicePixelRatio: return 1;
        case PdmDevicePixelRatioScaled: return int(devicePixelRatioFScale());
        }
        return 0;
    }

private:
    mutable TextEngine m_engine;
};

// The number of pages a PDF holds, and the sizes of its pages (the boxes) in the order they come.
inline int pdfPages(const QByteArray &pdf)
{
    int pages = 0;
    for (qsizetype at = pdf.indexOf("/Type /Page"); at >= 0; at = pdf.indexOf("/Type /Page", at + 1))
        if (at + 11 >= pdf.size() || pdf[at + 11] != 's') ++pages;
    return pages;
}

inline QStringList pdfBoxes(const QByteArray &pdf)
{
    QStringList out;
    for (qsizetype at = pdf.indexOf("/MediaBox"); at >= 0; at = pdf.indexOf("/MediaBox", at + 1)) {
        const qsizetype open = pdf.indexOf('[', at), close = pdf.indexOf(']', at);
        if (open > 0 && close > open) out << QString::fromLatin1(pdf.mid(open + 1, close - open - 1).simplified());
    }
    return out;
}

// Three sheets of one row each on three different papers (A4, letter on its side, legal), so that a page can be told from the size of its paper.
inline QByteArray threePapers()
{
    auto sheet = [](const char *setup) { return worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), QString::fromLatin1(setup)); };
    return buildXlsx({sheet("<pageSetup paperSize=\"9\"/>"), sheet("<pageSetup paperSize=\"1\" orientation=\"landscape\"/>"), sheet("<pageSetup paperSize=\"5\"/>")}, QByteArray(),
                     QString(), QString(), QByteArray(), QStringList({"A", "B", "C"}));
}

} // namespace
using namespace detail_tst_crunch_print;


class CrunchPrintTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void aBreakBeforeTheFirstRowOrColumnIsRefused();
    void aPageChangeIsOneStepOfUndo();
    void aHeaderTooLongForTheFileIsRefused();
    void moreBreaksThanTheFileMayHoldAreRefused();
    void theSessionTellsWhenASheetPrintsDifferently();
    void thePrintSettingsFollowInsertedAndDeletedRows();
    void aSavedFileHasTheSetupAndNothingElseChanges();
    void aJobCutsTheSheetsIntoPages();
    void theWholeWorkbookPrintsTheShownSheets();
    void theSelectedSheetsPrintTogether();
    void aSelectionPrintsAlone();
    void aPdfCanBeExportedAsPdfA();
    void thePagesGoToAPdfFile();
    void pageSetupFollowsTheSheetOperations();
    void anExportShowsProgressAndCanBeCancelled();
    void printingSendsTheCopiesAndCanBeStopped();
    void theMomentOfTheFieldsIsTakenOncePerJob();
    void aPageCanBeSeenAsAPicture();
    void theDialogShowsTheSetupAndGivesItBackUnchanged();
    void thePageTabHasThePrintQuality();
    void changesInTheDialogGoBack();
    void marginsCanBeInCentimeters();
    void aPrintAreaThatIsNotCellsIsRefused();
    void anAreaTypedWithTheSheetsNameIsTaken();
    void theHeaderAndFooterPresetsAreMadeFromTheSheet();
    void thePageLayoutTabAndItsCommandsAreThere();
    void marginsOrientationAndPaperChangeTheSheetAndShowWhatItHas();
    void thePrintAreaCommandsUseTheSelection();
    void pageBreaksAreInsertedAndRemovedAtTheSelection();
    void theScaleToFitBoxesShowAndChangeTheSheet();
    void gridlinesAndHeadingsCanBePrinted();
    void thePageSetupCommandChangesTheSheetThroughTheDialog();
    void thePrintScreenShowsThePagesAndFollowsTheChoices();
    void theOddOrTheEvenPagesCanBePrintedAlone();
    void printToAFileMakesThePdfAndClosesTheScreen();
    void severalPagesCanShareASheet();
    void pagesGoToAPrinterObject();
    void theSidesChoiceSetsTheDuplexModeOfThePrinter();
    void theSidesChoiceFollowsThePrinterAndTheFileTarget();
    void theSidesChoiceByHandPrintsTheOddPagesThenTheEvenPages();
    void theSidesChoiceIsRememberedAcrossRestarts();
    void theHandRunsAreMadeOfPhysicalSheets();
    void exportToPdfWritesTheSheetOrTheWorkbook();
    void exportToPdfCanBeProtectedWithAPassword();
    void theExportPageOffersAPasswordAndTurnsOffPdfA();
    void thePageBreaksShowOnTheSheetAfterAPreview();
    void undoAndRedoOfTypingMoveThePagesAtOnce();
    void aDroppedLineJobNeverLandsItsLines();
    void thePageBreakLinesOfAMillionRowsAreWorkedOutInSlices();
    void pageLinesComeFromTheCutsAndTheBreaks();
    void objectsPrintInPlaceUnlessTheyAreNotToBe();
    void aSheetOfOnlyDrawingsPrintsThem();
    void aJobThatNeedsTooManyPagesIsCutShort();
    void hostilePageSettingsAreBounded();
    void insertedRowsAndAPrintAreaAreSavedTogether();
    void thePageBreaksViewNumbersThePagesOverThePrintedCells();
    void draggingALineMovesABreakAsOneStep();
    void draggingALineOffTheCellsTakesItAway();
    void draggingAnEdgeOfThePrintedCellsSetsThePrintArea();
    void aFittedSheetCannotHaveItsLinesDragged();
    void theViewIsSavedAndComesBack();
    void frozenPanesAreSavedAndComeBack();
    void splitPanesAreSavedAndComeBack();
    void theTopPanesScrollAndRowsInsertedAboveTheBarAreKept();
    void sheetRightToLeftIsSavedAndComesBack();
    void notesPrintAtTheEndOfTheSheet();
    void theScrollAndTheCursorAreTakenAtTheSave();
    void aPictureInTheHeaderIsDrawnOnThePage();
    void aPictureInTheFooterAndInTheFirstPageForm();
    void aHeaderPictureSurvivesASaveThatChangesOtherThings();
    void thePictureButtonsOfTheHeaderEditor();
    void aHeaderPictureAddedThroughPageSetupSavesAndComesOut();
    void aHeaderPictureSurvivesRowsBeingInserted();
    void aCopyOfASheetWithHeaderPicturesHasThemToo();
    void aJobSaysWhereTheCellsOfAPageAre();
    void thePageLayoutViewShowsThePages();
    void cellsCanBeChosenAndMovedOnThePages();
    void cellsAreEditedOnThePages();
    void aHeaderIsTypedInPlace();
    void insertHeaderFooterOpensTheHeader();
    void theGalleriesWriteReadyFormsAndGoToOpensThePart();
    void theFirstPageHeaderIsTypedInPlace();
    void theTabPutsAPictureInTheHeader();
    void thePageLayoutViewIsSavedAndComesBack();
    void aSheetOfAMillionRowsIsShownAsPagesInSlices();
    void aZoomChangeAloneIsSaved();
    void leavingAViewPutsTheCellsOwnZoomBack();
    void eachSheetKeepsItsOwnZoomOfTheCells();
    void aFullListOfBreaksKeepsTheOldBreak();
    void aHeaderPictureIsMadeOffTheWindowThread();
    void anOversizedPageIsDrawnOnlyWhereItIsExposed();
    void thePagePainterDrawsOnlyWhatTheClipLeavesInView();
    void pagesStayOnTheScreenUntilTheNewOnesAreReady();
    void thePlaceOfAPagesCellsIsWorkedOutOncePerPage();
    void aNewPictureFitsTheHeaderMargin();
    void thePagesShowGridlinesAndHeadingsButTheyAreNotPrinted();
    void thePrintPreviewMakesHeaderPicturesOffTheWindowThread();
    void aPageDrawnWithoutAPictureIsDrawnAgainEvenIfTheWorkerWasQuick();
    void theCacheCountsTheFilesItHoldsOnTo();
    void firstAndEvenPagePicturesAreWrittenWithTheLongIds();
    void thePrintScreensPaintClosureIsFreedWithTheScreen();
};

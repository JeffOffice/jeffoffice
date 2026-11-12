#pragma once
// Tests of editing in JeffCrunch's window: typing over a cell, in it (F2, a double click), and in the formula bar; the keys that finish or give up an entry; what typed numbers,
// percents, money, dates, and times are given as values and formats; a formula that is not one; Delete; what depends on a cell worked out again by the worker; Undo and Redo; and saving
// (Save, Save As, a save that fails, the question when a window with changes is closed). Every workbook is built here, in a temporary folder.
#include "../localepin.h"
#include "../settingguard.h"
#include "../withdialog.h"
#include "app/crunchinfo.h"
#include "app/crunchfilescreen.h"
#include "app/crunchwindow.h"
#include "app/filescreenparts.h"
#include "savedok.h"
#include "app/help.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "app/filetasks.h"
#include "app/finddialog.h"
#include "app/pagesetupdialog.h"
#include "office/package.h"
#include <QLineEdit>
#include <QPushButton>
#include "app/otherformats.h"
#include "io/zip.h"
#include <QStandardPaths>
#include "core/appinfo.h"
#include "core/fixeddecimals.h"
#include "core/fonts.h"
#include "core/loader.h"
#include "core/resultmap.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "grid/valuestore.h"
#include "io/zipindex.h"

#include <QAction>
#include <QComboBox>
#include <QKeySequenceEdit>
#include <QTimer>
#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QFileInfo>
#include <QLineEdit>
#include <QMenu>
#include <QPointer>
#include <QRegularExpression>
#include <QSettings>
#include <QSignalSpy>
#include <QTableWidget>
#include <QThread>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;


class CrunchEditTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void helpShowsThePlatformsKeysAndSaysWhatTypingDoes();
    void theHelpTabHasHelpShortcutsWhatsNewAndFeedback();
    void typingOverACellPutsWhatWasTypedInIt();
    void theSymbolDialogPutsACharacterInTheEntry();
    void aSymbolFromAChosenFontIsARunInThatFont();
    void theSymbolDialogHasSpecialCharacters();
    void aSymbolCanHaveAShortcutKey();
    void theKeysThatFinishAnEntry();
    void enterGoesWhereTheOptionSays();
    void autoCompleteOffersTheRestOfAnEntryInTheColumn();
    void theFillHandleCanBeTurnedOff();
    void automaticDecimalPlacesPutThePointIn();
    void zerosCanBeHiddenOnASheetAndAreSavedThatWay();
    void ctrlEnterFillsTheSelection();
    void ctrlEnterKeepsToTheTopLeftCellOfAMergedArea();
    void todaysDateAndTheTimeByKeys();
    void altDownListsTheColumnsValues();
    void enterAfterTabsGoesBackToTheStartColumn();
    void enterAfterMovingByTabsGoesBackToTheStartColumn();
    void f2AndADoubleClickEditWhatTheCellHolds();
    void theFormulaBarEdits();
    void anEntryThatWasNotChangedCommitsNothing();
    void aFormulaTypedWithAPrefixedFunctionKeepsItsPrefixInTheFile();
    void clearAllOnEmptyCellsLeavesNothingForTheWorker();
    void aSaveThatDoesNothingWhenResumedDoesNotLeaveTheWindowToClose();
    void saveACopyKeepsWorkingInTheOriginal();
    void aBackupCopyIsKeptOnSave();
    void aWorkbookOpenedReadOnlyCannotBeChanged();
    void aReadOnlyWorkbookIsNotChangedByReplace();
    void theRepairFileGoesWithTheWindowOnEveryPath();
    void aRepairOfAFileLargerThanTheMostIsRefused();
    void aTemplateMadeFromAMacroWorkbookHasNoMacros();
    void aBackupThatFailsAsksBeforeTheWorkbookIsSaved();
    void aWorkbookOpenedAsACopyHasNoFile();
    void aWorkbookCanBeOpenedInANewWindow();
    void saveAsTemplatePutsAWorkbookInMyTemplates();
    void aTemplateOfAProtectedWorkbookIsProtectedToo();
    void aWorkbookCanBeSavedAsTemplatesInBothKinds();
    void aWorkbookCutShortOpensRepaired();
    void backspaceStartsAnEmptyEntry();
    void formulasAreTypedAndShownWithTheLocalesSeparators();
    void aTypedDateIsShownInTheOrderItWasTypedIn();
    void typedNumbersGetTheFormatThatShowsThemAsTyped();
    void aFormulaIsParsedAndAFaultyOneKeepsWhatWasTyped();
    void deleteClearsContentsAsOneStep();
    void aProtectedSheetsLockedCellsRefuseEdits();
    void typingInATableHeaderRenamesTheColumn();
    void whatDependsOnACellIsWorkedOutAgain();
    void anEditDuringTheCalculationIsAbsorbed();
    void anEditCostsTheWindowOnlyWhatPuttingTheCellInCosts();
    void theWorkerStaysAndGoesWithTheWindow();
    void anEditAfterEscResumesTheCalculation();
    void editsOneAfterTheOtherAreAllWorkedOut();
    void anEditIsRefusedWhileTheWorkbookIsBeingOpened();
    void aCellOfAnArrayIsNotChanged();
    void undoPutsBackValuesFormulasFormatsAndDependents();
    void undoPutsBackTheStoredResultOfAFormulaWithAFunctionTheProgramLacks();
    void undoHoldsAtMostItsBytesAndForgetsTheOldestSteps();
    void undoHoldsWhatChangedAndNoMore();
    void anUndoAndARedoStillSaveTheResultsOfTheRedo();
    void aCellBeingEditedShowsItsFormulaInTheCellAndTheBar();
    void saveWritesWhatChangedAndKeepsTheRest();
    void aSecondSaveWritesWhatChangedSinceTheFirst();
    void saveAsWritesAnotherFileAndTheWindowIsThatFile();
    void editingACellASpillFillsAndLeavingItAsItWasChangesNothing();
    void aSpillTypedInTheWindowIsSavedAndOpenedAgainAsASpill();
    void aFilledCellOnAProtectedSheetHidesTheFormulaThatFillsIt();
    void theFormulaBarCommittingAFilledCellsFormulaUnchangedChangesNothing();
    void aLetThatSpillsTypedInTheWindowIsSavedAndOpenedAgain();
    void aSaveThatFailsLeavesTheOldFile();
    void closingAsksAboutChangesThatAreNotSaved();
    void saveWaitsForTheCalculation();
    void savingOverAFileThatChangedAsks();
    void savingOverAFileThatIsAsItWasReadAsksNothingAndOverwriteWrites();
    void aFileThatIsOpenIsBroughtForwardNotOpenedAgain();
    void aNewWorkbookTakesTypingAndFormulas();
    void undoWorksOnANewWorkbook();
    void aNewWindowTakesTypingWithoutAClick();
    void saveOnANewWorkbookGoesToSaveAs();
    void aNewWorkbookStartsWithTheChosenFontSizeAndSheets();
    void aNewWorkbooksPaperAndUnitsFollowTheRegion();
    void theWindowMakesNewWorkbooksFromTheSettings();
    void fileNewMakesAnotherWindowWithABlankWorkbook();
    void aFileOpenedInAnUnchangedNewWorkbookReplacesIt();
    static QList<CrunchWindow *> crunchWindows();
    static CrunchWindow * windowSince(const QList<CrunchWindow *> &before);
    static void closeSince(const QList<CrunchWindow *> &before);
    static CrunchWindow * readyNewWindow(QStringList *told);
    static void typeInto(CrunchWindow *w, const QString &ref, char key);
    void aFileThatCannotBeReadLeavesNoDeadWindow();
    void bookNumbersAreForNewWorkbooksOnly();
    void aWaitingSaveIsNotDoneForTheFileThatReplacesTheBlank();
    void aNewWorkbookBeingSavedKeepsItsWindow();
    void aFileOpenedWhileTheNewWorkbookIsLoadingTakesItsPlace();
    void closingAChangedNewWorkbookThroughSave();
    void theNewShortcutMakesAWindow();
    void theFileScreensNewPageMakesABlankWorkbook();
    void aFileThatOpensElsewhereClosesTheFileScreen();
    void escCancelsASaveThatWaitsForTheCalculation();
    void aSharedFormulaSurvivesItsFirstCellBeingTypedOver();
    void onlyTheResultsTheEditChangedAreWritten();
    void theBytesOfAStructuralStepCountTheFormulasHeldInIds();
    void theBytesOfACellsContentCountTheFormulaAsItIsHeld();

    // ---- the save of a workbook whose formulas are held in ids, after rows were put in (the saver is round 7's: until then a formula is written whole only when its text is no longer the file's)
private:
    static bool saveParts(EditWindow &e, QMap<QString, QByteArray> *parts);
    static QByteArray stopgapBook();
private Q_SLOTS:
    void aSaveAfterARowWasPutInKeepsWhatDidNotMoveAndWritesWhatDid();
    void aFormulaWrittenWholeWithNoKnownResultAsksForAFullCalculationOnLoad();
    void aNameWithDeadLinesIsTheSameTextAfterASaveAndReopeningAsBefore();
    void aSaveWritesNoFormulaOfASheetThatTheEditDidNotMoveNorOfOnePutBack();

    // ---- round 7: the saver writes the formulas that read what moved
private:
    struct Dump {
        QMap<QString, QString> formulas;   // "sheet row col" to the text of the formula
        QMap<QString, double> numbers;     // and the numbers of the cells that are not formulas
    };
    template <class Cells>
    static void dumpSheet(Dump *out, int sheetIndex, const Sheet &sheet, const Cells &cells)
    {
        cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            const QString key = QStringLiteral("%1 %2 %3").arg(sheetIndex).arg(row).arg(col);
            if (c.hasFormula() && !(c.flags & (ArrayMember | DataTable))) out->formulas[key] = sheet.formulaText(c);
            else if (!c.hasFormula() && c.type == CellType::Number) out->numbers[key] = c.number;
        });
    }
    static Dump dumpOfWindow(EditWindow &e);
    static Dump dumpOfFile(const QString &path);
    static QByteArray editsBook();
    void randomEditsThenSave(quint32 seed, int count, int saveEvery);
private Q_SLOTS:
    void fiveHundredRandomEditsSavedAndReadAgainAreTheWindowsState();
    void editsSavedInStagesGiveTheWindowsStateEachTime();
    void aRowPutInAndSavedThenUndoneWritesTheFormulasAsTheyWere();
    void twoSavesOfRowsPutInGiveWhatOneSaveGives();
    static QString rawValue(const QMap<QString, QByteArray> &parts, const QString &ref, int sheet = 1);
    static QByteArray sumBook();
private Q_SLOTS:
    void aReferenceThroughSeveralSheetsFollowsTheEditsInTheSavedFile();
    void theCachedValueOfAMovedFormulaIsTheWindowsAfterATypedRowAndADeletedRow();
    void aFormulaNotWorkedOutAfterAMoveHasTheRightValueOrAsksForACalculation();

private Q_SLOTS:
    void theResultTableReusesThePlaceOfATextThatIsOverwrittenOrTakenOut();
    void theResultTableKeepsValuesAndRefusesItsOwnKeys();
    void theResultTableCostsUnderHalfWhatATreeOfValuesDoes();
    void anEditOfWhatACircleReadsAndItsUndoAndRedoGoRoundFromTheValuesItHad();
    void aWorkbookWithMacrosKeepsThem();

private:
    static QByteArray editBookWithParts();
    static QMap<QString, QByteArray> entriesOf(const QByteArray &zip);
    static QMap<QString, QByteArray> rawParts(const QByteArray &zip);
};

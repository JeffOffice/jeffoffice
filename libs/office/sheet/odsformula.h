#pragma once
// A formula as an OpenDocument spreadsheet writes it ("of:=SUM([.A1:.B2];[$Sheet2.C3])"), turned into the text the model keeps (SUM(A1:B2,Sheet2!C3), no "=" in front, the
// names of functions as the file format of the .xlsx writes them, with the prefix _xlfn. on those added after its first edition), so that the engine works it out.
//  - A reference is in square brackets, its sheet before a dot ([$'My Sheet'.A1]); the range of two sheets is a 3-D range; a reference to another file is not converted.
//  - Arguments are separated by a semicolon, the columns of an array constant by a semicolon and its rows by a vertical bar ({1;2|3;4}); the union of references is a tilde and
//    their intersection an exclamation mark.
//  - Names of functions the format spells with a prefix (COM.MICROSOFT.CEILING.MATH, ORG.OPENOFFICE.EASTERSUNDAY, LEGACY.NORMSDIST) lose it.
// A formula that cannot be converted (another file's reference, text that is not a formula) gives false: the cell keeps the value the file stored, as a constant.

#include <QHash>
#include <QString>

#include <QStringList>

#include <atomic>
#include <functional>

namespace jp::office::sheet {

// `formula` as the attribute holds it (with its "of:" or "oooc:" namespace prefix and "=", or without either). The converted text, or an empty one and `ok` false.
// `sheetNames`, if given, is the sheets whose names were changed (the document's name to the new one): a reference to one is written with the new name.
// `links`, if given, is asked for the number (from 1) of the workbook a reference to another file's cells names by its address ("file:///x.xlsx"): such a reference is written
// `[2]Sheet1!A1` (`'[2]My Sheet'!A1`), as the .xlsx form writes it. Without it, a formula that reads another file's cells is not converted.
QString odfFormulaToText(const QString &formula, bool *ok = nullptr, const QHash<QString, QString> *sheetNames = nullptr, const std::function<int(const QString &)> *links = nullptr);

// A cell range as the attributes of a named range write it: "$Sheet1.$A$1:.$B$2" or "$'My Sheet'.A1" gives "Sheet1!$A$1:$B$2" or "'My Sheet'!A1". `ok` false if it is no range.
QString odfRangeToText(const QString &address, bool *ok = nullptr, const QHash<QString, QString> *sheetNames = nullptr);

// The address a workbook's file is named by in a formula, in the standard's file address form: a path with backslashes or spaces ("C:\\my dir\\b.xlsx") is "file:///C:/my%20dir/b.xlsx",
// and a character an address may not hold ("]", "#", "%" of a path) is percent-encoded. (Apostrophes stay: the formula doubles them where it quotes the address.) The same text gives the same address.
QString odfLinkAddress(const QString &target);

// The workbooks formulas read, numbered from 1 (the numbers the .xlsx form gives its links): each is kept as its file named it (`target`) and found by the address formulas name it by.
// Filled before any sheet is read and not changed after, so that the numbers do not depend on which sheet is read first, and asked from any thread.
class OdsLinkTable {
public:
    static constexpr int kMost = 4000;
    // The number of the workbook at `address` (the next number if it is new, with `target` as its file); 0 if there are already the most.
    int add(const QString &target, const QString &address);
    // The workbook the file itself numbers `number` (1 to the most): the numbers it leaves out are kept as workbooks of no address, so that the others keep theirs.
    void addNumbered(int number, const QString &target, const QString &address);
    int numberOf(const QString &address) const;   // 0 if none
    QStringList targets() const { return m_targets; }
    // The entries looked at by all the asks of numberOf: one for each (a test counts it: it must not grow with the number of workbooks).
    qint64 entriesExamined() const { return m_examined.load(); }

private:
    QHash<QString, int> m_byAddress;
    QStringList m_targets;
    mutable std::atomic<qint64> m_examined{0};
};

// A cell address without sheet ("B3", "$B$3") as a row and column (from 0); false if it is none.
bool odfCellOf(const QString &text, quint32 *row, quint32 *col);

// The name the standard gives a function that the model names `modelName` (ERROR.TYPE, FORMULATEXT as FORMULA, CEILING as COM.MICROSOFT.CEILING.XCL, a function the .xlsx form added
// later with its COM.MICROSOFT. prefix), and the model's name for one the standard names: the two tables that odfFormulaToText reads function names by, one each way.
QString odfFunctionName(const QString &modelName);
QString modelFunctionName(const QString &odfName);

} // namespace jp::office::sheet

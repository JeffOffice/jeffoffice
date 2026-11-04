#pragma once
// The field engine, the part that needs no document: a field's code is read into its name, arguments, and switches; its result is worked out from what the document tells (the
// Env) and shown in the formats the switches ask for (\* roman, \# "0.00", \@ "MMMM d, yyyy"). Nothing here touches the story; core/fieldops.h walks the story and asks.
// A field this engine does not know, or one it cannot answer (a bookmark it has not been told of), has no result here, and the field keeps the result it has.

#include <QDateTime>
#include <QLocale>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>
#include <optional>

namespace jp::write::fields {

// The most characters of a code that are read, the most words in it, and the deepest nesting the formulas and the conditions are followed (a hostile file can say more).
constexpr int kMostCodeChars = 4096;
constexpr int kMostCodeWords = 256;
constexpr int kMostFormulaDepth = 48;
constexpr int kMostPictureChars = 255;

struct Switch {
    QChar letter;        // as written, except that the letter of a switch is compared without regard to case
    QString argument;    // the word after it that it takes (a quoted one without its quotes), else empty
};

struct Code {
    QString name;                // upper case: PAGE, REF, TOC...
    QStringList arguments;       // the words before the first switch, a quoted one without its quotes (an empty quoted one is kept)
    QString expression;          // the text after the name up to the first switch, as written (a formula's)
    QVector<Switch> switches;
    QStringList general;         // the arguments of \* (MERGEFORMAT, roman, Upper...)
    QString numberPicture, datePicture;   // the arguments of \# and \@
    bool hasNumberPicture = false, hasDatePicture = false;
    bool has(QChar letter) const;
    QString argumentOf(QChar letter) const;   // the argument of the first such switch
    bool keepsFormat() const;                 // \* MERGEFORMAT or \* CHARFORMAT: the result takes the look of the result it replaces
    bool mergesFormat() const;                // \* MERGEFORMAT: each place of the result keeps the look the old result had there
    bool usesCodeFormat() const;              // \* CHARFORMAT: the whole result has the look of the first character of the code
};

// The code of an instruction (leading and trailing blanks do not matter). Never fails: what it cannot read is left out.
Code parseCode(const QString &instruction);
// How many instructions parseCode has read since the counts were reset (a bound on what a pass over hostile fields costs).
qint64 parseCodeCallsForTest();
void resetParseCodeCallsForTest();

// ---- formats
// The number as the general-format switches show it (roman, ROMAN, alphabetic, ALPHABETIC, arabic, Ordinal, CardText, OrdText, Hex, ArabicDash, then Upper, Lower, FirstCap, Caps
// on the text); the plain digits where none asks for a number format.
QString formatNumber(qint64 n, const QStringList &general);
// Text in the case the switches ask for (Upper, Lower, FirstCap, Caps).
QString formatText(const QString &text, const QStringList &general);
// A number in a picture: 0 and # for digits, x to drop digits, . and , for the point and the groups, ; between the pictures for positive, negative, and zero, 'text' and other
// characters as they are, - and + for the sign.
QString numberInPicture(double value, const QString &picture);
// A date and time in a picture: M, d, y, h, H, m, s, am/pm, 'text', \ before a character. The names of months and days are in the locale's language (English by default).
QString dateInPicture(const QDateTime &when, const QString &picture, const QLocale &locale = QLocale::c());
// A number as a formula without a picture shows it (up to six places, no trailing zeros).
QString plainNumber(double value);

// ---- formulas and conditions
// The value of an expression of a formula field (= 2 + 3 * 4, SUM(1,2,3), (7 - 2) ^ 2, 50% ...). Names that are not functions are asked of `name` (a bookmark's number).
// Nothing for text that is no formula, or one deeper or longer than the most.
// `budget`, if given, is the steps that all the formulas of a pass may take together: each step and each name asked takes from it, and a formula is left unworked when it is spent.
std::optional<double> evaluateFormula(const QString &expression, const std::function<std::optional<double>(const QString &)> &name = nullptr, qint64 *budget = nullptr,
                                      const std::function<std::optional<QVector<double>>(const QString &)> &cells = nullptr);
// The steps all the formulas of a pass may take together.
constexpr qint64 kMostFormulaSteps = 200000;

// The pieces of the document a field's result may be made of. Each function says nothing (null) where the document cannot answer, and the field keeps its result.
struct BookmarkInfo {
    QString text;          // what the bookmark holds
    QString page;          // the page it is on, as a page number shows it (empty where the pages are not laid out)
    QString number;        // the number of its paragraph without trailing periods (REF \n), the one with the numbers of the levels above (\w), and the one relative to the field (\r)
    QString fullNumber, relativeNumber;
    int position = 0;      // where it begins in the text: whether it is above or below the field
    int pageIndex = 0;     // the page, from 1, for "on page N" (0 where unknown)
};

struct Properties {
    QString author, title, subject, keywords, comments, lastSavedBy, template_, company, manager, category, fileName, filePath, revision;
    QDateTime created, saved, printed;
    qint64 fileSize = -1;
    qint64 totalMinutes = 0;
    int pages = 0, words = 0, characters = 0, charactersWithSpaces = 0, lines = 0, paragraphs = 0;
    bool statistics = false;   // whether the counts are known
};

struct Env {
    QDateTime now;
    Properties properties;
    // Where the field is: its page (from 1) as a number and as the page number shows it, the pages in all and in its section, its section's number, and the place it stands at.
    int page = 0, pages = 0, sectionPages = 0, section = 0, position = 0;
    QString pageText;
    QLocale locale;   // the language of the names of months and days a date shows: the field's own where the document says it, else the program's
    std::function<std::optional<BookmarkInfo>(const QString &)> bookmark;
    std::function<void(const QString &name, BookmarkInfo *)> bookmarkPage;   // fills `page` and `pageIndex` of a bookmark's info (asked only when a page number is shown)
    qint64 *formulaBudget = nullptr;                                         // the steps the formulas of the pass have left (see evaluateFormula)
    std::function<std::optional<QString>(const QString &name, const Code &code)> noteNumber;    // NOTEREF: the number of the note the bookmark holds the reference of
    std::function<std::optional<QString>(const QString &style, const Code &code)> styleRef;     // STYLEREF
    std::function<std::optional<QString>(const QString &tag, const Code &code)> citation;       // CITATION: the text a source's tag gives, in the style the document's sources have
    // The counters of SEQ fields, by name (upper case): the pass that walks the document keeps them and resets those that count by chapter.
    QHash<QString, qint64> *sequences = nullptr;
    // Used with a bookmark name read by a formula, and by a condition of IF.
    std::function<std::optional<double>(const QString &)> number;
    // The numbers in the cells of the table the field is in that a cell name or a direction (A1, A1:B3, ABOVE, LEFT, ...) stands for; none when the field is not in a table.
    std::function<std::optional<QVector<double>>(const QString &)> cells;
    // The merge of a mail merge: asked of every field this engine has no result for (MERGEFIELD, ADDRESSBLOCK, GREETINGLINE, SET, ASK, SKIPIF...). Null where it has none either.
    std::function<std::optional<QString>(const Code &code)> merge;
};

// The result of a field with this code, or null where the engine leaves it as it is (a field it does not know, a document that cannot say). TOC is built by the caller.
std::optional<QString> compute(const Code &code, Env &env);

// The names of the fields that depend on the machine or the moment, not on the document (a yardstick leaves them out).
bool dependsOnTheMachine(const QString &name);
// The fields this engine works out.
bool isKnown(const QString &name);
// Counted work, for tests: the steps the glob matcher of IF has taken since the count was reset.
qint64 globStepsForTest();
void resetGlobStepsForTest();
// The text a reference shows when what it refers to is missing.
QString referenceSourceNotFound();
QString bookmarkNotDefined();
// The text a style reference shows when no paragraph has the style.
QString styleTextNotFound();

} // namespace jp::write::fields

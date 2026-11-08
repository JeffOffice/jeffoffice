#pragma once
// Spelling in a workbook: the text cells that the Spelling command goes through (a formula, a number, a date, and the rest are not looked at), the words in a text that are not in the
// dictionary, and the language the dictionary is chosen by. The words, the user's dictionary, and the options that let words go unchecked are the programs' shared ones (libs/shell spelling).

#include "core/editsession.h"
#include "sheet/cellref.h"

#include <QHash>
#include <QPair>
#include <QString>
#include <QVector>

#include <optional>
#include <vector>

namespace jp::crunch {

// The language words are checked in: Options > Proofing's, else the language the program is shown in, else US English (a language with no dictionary is US English).
QString proofingLanguage();
// The [start, end) places of the misspelled words in a text. Empty when the text is a formula, or the dictionaries are not there.
QVector<QPair<int, int>> misspelledRanges(const QString &text, const QString &language = QString());

// The text a cell holds when it is a text that was typed or read (not the result of a formula, a number, a date, a truth value, or an error).
bool spellableText(const EditSession &session, int sheet, quint32 row, quint32 col, QString *text);

struct SpellHit {
    quint32 row = 0, col = 0;
    int from = 0, to = 0;   // the word's places in the cell's text
    QString word;
    QString text;           // the cell's whole text
};

// The walk of the Spelling command: the text cells of a range in reading order (row by row), one misspelled word at a time. The workbook must not change between two calls except by the
// word it gave (the walk looks again at the cell it changed). A step looks at cells for a limited time only, so that the window can go on (a sheet may have a million text cells).
class SpellWalk {
public:
    SpellWalk(const EditSession *session, int sheet, const jp::office::sheet::CellRange &range);
    enum class Result { Found, Done, More };
    // Looks at cells for about `budgetMs` milliseconds (0: until a word or the end). Found: `hit` has the next misspelled word after the last one given; Done: there is none; More: the time
    // was up, call again.
    Result step(qint64 budgetMs, SpellHit *hit);
    // The same without a limit.
    std::optional<SpellHit> next();
    // The word given last is gone from the cell (it was changed): look at the cell again from where the word began, with `replacementEnd` the place after what replaced it.
    void changed(int replacementEnd);
    // Every cell the walk goes through, for Change All.
    const std::vector<std::pair<quint32, quint32>> &cells() const { return m_cells; }
    // How far the walk has come, from 0 to 1.
    double progress() const { return m_cells.empty() ? 1.0 : double(m_at) / double(m_cells.size()); }

private:
    const EditSession *m_session;
    int m_sheet;
    std::vector<std::pair<quint32, quint32>> m_cells;
    size_t m_at = 0;     // the cell being looked at
    int m_offset = 0;    // the place in its text the next word is looked for from
    // The misspelled places of the texts looked at lately, by text (many cells hold the same words); forgotten when the user's dictionary or ignored words change.
    QHash<QString, QVector<QPair<int, int>>> m_cache;
    int m_cacheRevision = -1;
    const QVector<QPair<int, int>> &rangesOf(const QString &text);
};

} // namespace jp::crunch

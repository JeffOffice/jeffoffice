#include "core/spellwalk.h"

#include "app/settings.h"
#include "app/spelling.h"
#include "text/dictionaries.h"

#include <QLocale>
#include <QTextBlock>
#include <QTextDocument>

#include <algorithm>

namespace jp::crunch {

using jp::office::sheet::CellType;

QString proofingLanguage()
{
    Settings &st = Settings::get();
    QString tag = st.value(QStringLiteral("proof/language")).toString();
    if (tag.isEmpty()) tag = st.value(QStringLiteral("ui/language")).toString();
    if (tag.isEmpty()) tag = QLocale().bcp47Name();
    return dict::match(tag);
}

QVector<QPair<int, int>> misspelledRanges(const QString &text, const QString &language)
{
    if (text.size() < 2 || text.startsWith(QLatin1Char('='))) return {};
    QTextDocument doc;
    doc.setPlainText(text);
    const QString code = language.isEmpty() ? proofingLanguage() : language;
    QVector<QPair<int, int>> out;
    // (a text of several lines is several blocks, and a place in the document is a place in the text)
    for (QTextBlock b = doc.begin(); b.isValid(); b = b.next()) out += misspellingsIn(b, code);
    return out;
}

bool spellableText(const EditSession &session, int sheet, quint32 row, quint32 col, QString *text)
{
    const jp::office::sheet::Calculation *calc = session.calculation();
    if (!calc || sheet < 0 || sheet >= calc->sheetCount()) return false;
    const jp::office::sheet::Calculation::CellContent c = calc->content(sheet, row, col);
    if (!c.exists || c.isFormula || !c.value.isText() || c.value.text().isEmpty()) return false;
    if (text) *text = c.value.text();
    return true;
}

SpellWalk::SpellWalk(const EditSession *session, int sheet, const jp::office::sheet::CellRange &range) : m_session(session), m_sheet(sheet)
{
    const jp::office::sheet::Calculation *calc = session ? session->calculation() : nullptr;
    if (!calc || sheet < 0 || sheet >= calc->sheetCount()) return;
    const auto &s = calc->sheet(sheet);
    if (s.extentRows == 0 || s.extentCols == 0) return;
    const quint32 r2 = std::min(range.row2, s.extentRows - 1), c2 = std::min(range.col2, s.extentCols - 1);
    if (range.row1 > r2 || range.col1 > c2) return;
    s.cells.forEachInRange(range.row1, range.col1, r2, c2, [&](quint32 row, quint32 col, const jp::office::sheet::Cell &c) {
        if ((c.type == CellType::SharedString || c.type == CellType::InlineString) && !c.hasFormula()) m_cells.emplace_back(row, col);
        return true;
    });
    std::sort(m_cells.begin(), m_cells.end());
}

std::optional<SpellHit> SpellWalk::next()
{
    for (; m_at < m_cells.size(); ++m_at, m_offset = 0) {
        QString text;
        if (!spellableText(*m_session, m_sheet, m_cells[m_at].first, m_cells[m_at].second, &text)) continue;
        for (const auto &range : misspelledRanges(text)) {
            if (range.first < m_offset) continue;
            SpellHit hit;
            hit.row = m_cells[m_at].first;
            hit.col = m_cells[m_at].second;
            hit.from = range.first;
            hit.to = range.second;
            hit.word = text.mid(range.first, range.second - range.first);
            hit.text = text;
            m_offset = range.second;
            return hit;
        }
    }
    return std::nullopt;
}

void SpellWalk::changed(int replacementEnd) { m_offset = replacementEnd; }

} // namespace jp::crunch

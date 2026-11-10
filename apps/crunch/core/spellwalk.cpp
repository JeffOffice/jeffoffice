#include "core/spellwalk.h"

#include "app/settings.h"
#include "app/spelling.h"
#include "text/dictionaries.h"

#include <QElapsedTimer>
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

bool spellableText(const EditSession &session, int sheet, quint32 row, quint32 col, QString *text, QString *language, bool *noProof)
{
    const jp::office::sheet::Calculation *calc = session.calculation();
    if (!calc || sheet < 0 || sheet >= calc->sheetCount()) return false;
    const jp::office::sheet::Calculation::CellContent c = calc->content(sheet, row, col);
    if (!c.exists || c.isFormula || !c.value.isText() || c.value.text().isEmpty()) return false;
    if (text) *text = c.value.text();
    if (language || noProof) {
        bool none = false;
        const QString tag = session.proofingLanguageOfStyle(c.style, &none);
        if (language) *language = tag;
        if (noProof) *noProof = none;
    }
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

const QVector<QPair<int, int>> &SpellWalk::rangesOf(const QString &text, const QString &language)
{
    if (m_cacheRevision != spellingRevision()) {
        m_cache.clear();
        m_cacheRevision = spellingRevision();
    }
    const QString key = language.isEmpty() ? text : language + QLatin1Char('\n') + text;
    auto it = m_cache.constFind(key);
    if (it == m_cache.constEnd()) {
        if (m_cache.size() > 4000) m_cache.clear();
        it = m_cache.insert(key, misspelledRanges(text, language));
    }
    return *it;
}

SpellWalk::Result SpellWalk::step(qint64 budgetMs, SpellHit *hit)
{
    QElapsedTimer clock;
    clock.start();
    int looked = 0;
    for (; m_at < m_cells.size(); ++m_at, m_offset = 0) {
        // (the clock is read now and then, not at every cell)
        if (budgetMs > 0 && (++looked & 31) == 0 && clock.elapsed() >= budgetMs) return Result::More;
        QString text, language;
        bool noProof = false;
        if (!spellableText(*m_session, m_sheet, m_cells[m_at].first, m_cells[m_at].second, &text, &language, &noProof) || noProof) continue;
        for (const auto &range : rangesOf(text, language)) {
            if (range.first < m_offset) continue;
            hit->row = m_cells[m_at].first;
            hit->col = m_cells[m_at].second;
            hit->from = range.first;
            hit->to = range.second;
            hit->word = text.mid(range.first, range.second - range.first);
            hit->text = text;
            hit->language = language;
            m_offset = range.second;
            return Result::Found;
        }
    }
    return Result::Done;
}

std::optional<SpellHit> SpellWalk::next()
{
    SpellHit hit;
    return step(0, &hit) == Result::Found ? std::optional<SpellHit>(hit) : std::nullopt;
}

void SpellWalk::changed(int replacementEnd) { m_offset = replacementEnd; }

} // namespace jp::crunch

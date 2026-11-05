#include "core/formulasteps.h"

#include <QRegularExpression>

namespace jp::crunch {

namespace {

// The places in `t` that are inside a text in quotes.
QVector<bool> quoted(const QString &t)
{
    QVector<bool> in(t.size(), false);
    bool inside = false;
    for (int i = 0; i < t.size(); ++i) {
        if (t[i] == QLatin1Char('"')) {
            in[i] = true;
            inside = !inside;
        } else {
            in[i] = inside;
        }
    }
    return in;
}

bool isNameChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.'); }

} // namespace

FormulaSteps::FormulaSteps(const QString &expression, Eval eval) : m_original(expression), m_text(expression), m_eval(std::move(eval))
{
    findNext();
}

void FormulaSteps::restart()
{
    m_text = m_original;
    m_problem.clear();
    m_finished = false;
    findNext();
}

void FormulaSteps::findNext()
{
    m_from = m_length = 0;
    // a formula that is one value has nothing more to work out
    bool ok = false;
    QString t = m_text.trimmed();
    if (t.isEmpty()) {
        m_finished = true;
        return;
    }
    static const QRegularExpression plainValue(QStringLiteral(R"(^(-?[0-9][0-9.,eE+-]*|"([^"]|"")*"|TRUE|FALSE|#[A-Z/0!?]+)$)"));
    if (plainValue.match(t).hasMatch()) {
        m_finished = true;
        return;
    }
    const QVector<bool> in = quoted(m_text);
    // 1. the leftmost cell (not a range, not a function's name)
    static const QRegularExpression cell(QStringLiteral("(?:(?:'[^']+'|[A-Za-z_][A-Za-z0-9_.]*)!)?\\$?[A-Za-z]{1,3}\\$?[0-9]+(?::\\$?[A-Za-z]{1,3}\\$?[0-9]+)?"));
    QRegularExpressionMatchIterator it = cell.globalMatch(m_text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const int s = int(m.capturedStart()), e = s + int(m.capturedLength());
        if (in[s]) continue;
        if (s > 0 && isNameChar(m_text[s - 1]) && m_text[s - 1] != QLatin1Char('!')) continue;
        int n = e;
        while (n < m_text.size() && m_text[n] == QLatin1Char(' ')) ++n;
        if (n < m_text.size() && (m_text[n] == QLatin1Char('(') || isNameChar(m_text[n]))) continue;
        if (m.captured().contains(QLatin1Char(':'))) continue;   // (a block of cells waits for the function that reads it)
        m_from = s;
        m_length = e - s;
        return;
    }
    // 2 and 3. the innermost brackets: a call (its name goes with it) or a group
    int open = -1;
    for (int i = 0; i < m_text.size(); ++i) {
        if (in[i]) continue;
        if (m_text[i] == QLatin1Char('(')) open = i;
        else if (m_text[i] == QLatin1Char(')') && open >= 0) {
            int s = open;
            while (s > 0 && isNameChar(m_text[s - 1])) --s;
            m_from = s;
            m_length = i + 1 - s;
            return;
        }
    }
    (void)ok;
    // 4. what is left
    m_from = 0;
    m_length = int(m_text.size());
}

bool FormulaSteps::step()
{
    m_problem.clear();
    if (m_finished || m_length <= 0) return false;
    const QString part = m_text.mid(m_from, m_length);
    QString problem;
    const std::optional<QString> value = m_eval(part, &problem);
    if (!value) {
        m_problem = problem;
        return false;
    }
    const QString before = m_text;
    m_text.replace(m_from, m_length, *value);
    findNext();
    if (m_text == before) {   // (a value that is not one the steps can use again, such as a block of cells: nothing more is left to work out)
        m_finished = true;
        m_from = m_length = 0;
    }
    return true;
}

} // namespace jp::crunch

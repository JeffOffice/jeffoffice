#include "review/textdiff.h"

#include <QRegularExpression>
#include <QStringList>
#include <algorithm>

namespace jp::textdiff {

namespace {

QStringList tokensOf(const QString &text, const DiffOptions &o)
{
    QStringList out;
    if (o.byCharacter) {
        for (const QChar ch : text) out << QString(ch);
        return out;
    }
    static const QRegularExpression re(QStringLiteral("\\w+|\\s+|[^\\w\\s]"));
    auto it = re.globalMatch(text);
    while (it.hasNext()) out << it.next().captured(0);
    return out;
}

QString keyOf(const QString &token, const DiffOptions &o)
{
    QString k = token;
    if (o.ignoreCase) k = k.toLower();
    if (o.ignoreWhiteSpace && !k.isEmpty() && k.at(0).isSpace()) k = QStringLiteral(" ");
    return k;
}

} // namespace

// The longest common run of two lists, as which of each are kept (a table of lengths, walked back).
void commonRun(const QStringList &a, const QStringList &b, QVector<int> *keepA, QVector<int> *keepB)
{
    const int n = int(a.size()), m = int(b.size());
    QVector<quint32> table(size_t(n + 1) * size_t(m + 1), 0);
    auto at = [&](int i, int j) -> quint32 & { return table[size_t(i) * size_t(m + 1) + size_t(j)]; };
    for (int i = n - 1; i >= 0; --i)
        for (int j = m - 1; j >= 0; --j) at(i, j) = a.at(i) == b.at(j) ? at(i + 1, j + 1) + 1 : std::max(at(i + 1, j), at(i, j + 1));
    int i = 0, j = 0;
    while (i < n && j < m) {
        if (a.at(i) == b.at(j)) {
            keepA->push_back(i++);
            keepB->push_back(j++);
        } else if (at(i + 1, j) >= at(i, j + 1)) {
            ++i;
        } else {
            ++j;
        }
    }
}

QVector<TextEdit> diffText(const QString &from, const QString &to, const DiffOptions &options)
{
    QVector<TextEdit> edits;
    if (from == to) return edits;
    const QStringList ta = tokensOf(from, options), tb = tokensOf(to, options);
    if (ta.size() > kMostTokens || tb.size() > kMostTokens) {
        edits.push_back({0, int(from.size()), to});
        return edits;
    }
    QStringList ka, kb;
    for (const QString &t : ta) ka << keyOf(t, options);
    for (const QString &t : tb) kb << keyOf(t, options);
    QVector<int> keepA, keepB;
    commonRun(ka, kb, &keepA, &keepB);
    // Where each token of `from` begins, in its characters.
    QVector<int> startA(ta.size() + 1, 0);
    for (int i = 0; i < ta.size(); ++i) startA[i + 1] = startA[i] + int(ta.at(i).size());
    int i = 0, j = 0;
    for (int k = 0; k <= keepA.size(); ++k) {
        const int ei = k < keepA.size() ? keepA.at(k) : int(ta.size()), ej = k < keepB.size() ? keepB.at(k) : int(tb.size());
        if (i < ei || j < ej) {
            TextEdit e;
            e.deleteFrom = startA.at(i);
            e.deleteTo = startA.at(ei);
            for (int t = j; t < ej; ++t) e.insertText += tb.at(t);
            if (e.deleteTo > e.deleteFrom || !e.insertText.isEmpty()) {
                // (With no deletion the insert goes at the start of the next kept word.)
                edits.push_back(e);
            }
        }
        i = ei + 1;
        j = ej + 1;
    }
    return edits;
}

qint64 diffCells(const QString &from, const QString &to, const DiffOptions &options)
{
    if (from == to) return 0;
    const qint64 n = tokensOf(from, options).size(), m = tokensOf(to, options).size();
    if (n > kMostTokens || m > kMostTokens) return 0;
    return (n + 1) * (m + 1);
}

} // namespace jp::textdiff

#include "core/formulaspot.h"

#include "core/nametable.h"

#include <QCoreApplication>
#include <QLocale>
#include <QSet>

#include <algorithm>

namespace jp::crunch {

namespace {

bool wordChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char('\\'); }

}

QChar argumentSeparator(const QLocale &locale) { return locale.decimalPoint() == QLatin1Char(',') ? QLatin1Char(';') : QLatin1Char(','); }

FormulaSpot spotAt(const QString &text, int cursor, QChar separator)
{
    FormulaSpot spot;
    if (!text.startsWith(QLatin1Char('='))) return spot;
    spot.formula = true;
    const int n = std::clamp(cursor, 0, int(text.size()));
    struct Frame {
        QString function;
        int argument = 0;
    };
    std::vector<Frame> frames;
    bool inString = false, inQuote = false;
    int brackets = 0, braces = 0;
    for (int i = 1; i < n; ++i) {
        const QChar c = text[i];
        if (inString || inQuote) {
            const QChar q = inString ? QLatin1Char('"') : QLatin1Char('\'');
            if (c == q) {
                if (i + 1 < n && text[i + 1] == q) ++i;   // (doubled: it stands for itself)
                else inString = inQuote = false;
            }
            continue;
        }
        if (c == QLatin1Char('"')) {
            inString = true;
        } else if (c == QLatin1Char('\'')) {
            inQuote = true;
        } else if (c == QLatin1Char('[')) {
            ++brackets;
        } else if (c == QLatin1Char(']')) {
            if (brackets > 0) --brackets;
        } else if (brackets > 0) {
            continue;
        } else if (c == QLatin1Char('{')) {
            ++braces;
        } else if (c == QLatin1Char('}')) {
            if (braces > 0) --braces;
        } else if (braces > 0) {
            continue;
        } else if (c == QLatin1Char('(')) {
            int s = i;
            while (s > 0 && wordChar(text[s - 1])) --s;
            frames.push_back({text.mid(s, i - s), 0});
        } else if (c == QLatin1Char(')')) {
            if (!frames.empty()) frames.pop_back();
        } else if (c == separator) {
            if (!frames.empty()) ++frames.back().argument;
        }
    }
    if (inString || inQuote || brackets > 0) {
        spot.inText = true;
        spot.wordStart = n;
        return spot;
    }
    // the nearest function (a bracket that only groups is passed over: the arguments counted are the function's own)
    for (size_t i = frames.size(); i-- > 0;)
        if (!frames[i].function.isEmpty()) {
            spot.function = frames[i].function;
            spot.argument = frames[i].argument;
            break;
        }
    int start = n;
    while (start > 1 && wordChar(text[start - 1])) --start;
    spot.wordStart = start;
    spot.word = text.mid(start, n - start);
    int back = start - 1;
    while (back > 0 && text[back] == QLatin1Char(' ')) --back;
    const QChar before = text[back];
    static const QString operators = QStringLiteral("=(+-*/^&<>@{~");
    const bool placeForWord = back == 0 || operators.contains(before) || before == separator || before == QLatin1Char(',') || before == QLatin1Char(';');
    spot.canComplete = braces == 0 && placeForWord && !spot.word.isEmpty() && !spot.word[0].isDigit() && spot.word[0] != QLatin1Char('.');
    return spot;
}

// ---- what is offered

void CompletionIndex::build(const NameTable &names, int sheet, const QStringList &tables)
{
    m_names.clear();
    int taken = 0;
    // (the tables first: there are few, and the names past the bound are the ones left out)
    for (const QString &t : tables)
        if (!t.isEmpty() && taken < kMaxNamesIndexed) {
            m_names.push_back({t.toUpper(), t, -1, true});
            ++taken;
        }
    // (a name of this sheet hides the workbook's of the same text here: that one is offered, once)
    QSet<QString> own;
    for (int id = 0; id < names.slots(); ++id) {
        const NameEntry &e = names.at(id);
        if (e.alive && !e.hidden && e.scope == sheet && sheet >= 0) own.insert(e.name.toUpper());
    }
    for (int id = 0; id < names.slots() && taken < kMaxNamesIndexed; ++id) {
        const NameEntry &e = names.at(id);
        if (e.scope < 0 && own.contains(e.name.toUpper())) continue;
        if (!e.alive || e.hidden || (e.scope >= 0 && e.scope != sheet)) continue;
        m_names.push_back({e.name.toUpper(), e.name, id, false});
        ++taken;
    }
    std::sort(m_names.begin(), m_names.end(), [](const Entry &a, const Entry &b) { return a.upper < b.upper; });
}

std::vector<Completion> CompletionIndex::matches(const QString &word, int cap) const
{
    std::vector<Completion> out;
    if (word.isEmpty() || cap <= 0) return out;
    const QString upper = word.toUpper();
    // the functions (the engine has a couple of hundred)
    int count = 0;
    const FunctionDoc *docs = functionDocs(&count);
    std::vector<Completion> found;
    for (int i = 0; i < count; ++i)
        if (QLatin1String(docs[i].name).startsWith(upper)) found.push_back({Completion::Kind::Function, QString::fromLatin1(docs[i].name), -1});
    // the names that begin with the word: a place found by halving, then those that follow it (a name that belongs to another sheet is not in the index)
    auto at = std::lower_bound(m_names.begin(), m_names.end(), upper, [](const Entry &e, const QString &w) { return e.upper < w; });
    for (int taken = 0; at != m_names.end() && at->upper.startsWith(upper) && taken < cap; ++at, ++taken) found.push_back({at->table ? Completion::Kind::Table : Completion::Kind::Name, at->shown, at->id});
    std::sort(found.begin(), found.end(), [](const Completion &a, const Completion &b) {
        const int c = QString::compare(a.text, b.text, Qt::CaseInsensitive);
        return c != 0 ? c < 0 : a.kind < b.kind;
    });
    if (int(found.size()) > cap) found.resize(size_t(cap));
    return found;
}

// ---- the argument list

namespace {

// The pieces of the list, and which is the one the cursor is at.
QStringList pieces(const FunctionDoc &doc, int argument, int *marked)
{
    const QStringList args = argumentsOf(doc);
    *marked = -1;
    if (args.isEmpty()) return args;
    const int n = int(args.size());
    if (args.last() == QLatin1String("...")) {
        // (any number of the last kind: past the named ones, the dots are what is being typed)
        *marked = argument < n - 1 ? argument : n - 1;
    } else if (argument < n) {
        *marked = argument;
    }
    return args;
}

}

QString hintPlain(const FunctionDoc &doc, int argument, QChar separator, int *marked)
{
    int m = -1;
    const QStringList args = pieces(doc, argument, &m);
    if (marked) *marked = m;
    return QString::fromLatin1(doc.name) + QLatin1Char('(') + args.join(QString(separator) + QLatin1Char(' ')) + QLatin1Char(')');
}

QString hintHtml(const FunctionDoc &doc, int argument, QChar separator)
{
    int m = -1;
    const QStringList args = pieces(doc, argument, &m);
    QString html = QString::fromLatin1(doc.name).toHtmlEscaped() + QStringLiteral("(");
    for (int i = 0; i < args.size(); ++i) {
        if (i > 0) html += QString(separator).toHtmlEscaped() + QLatin1Char(' ');
        html += i == m ? QStringLiteral("<b>") + args[i].toHtmlEscaped() + QStringLiteral("</b>") : args[i].toHtmlEscaped();
    }
    return html + QStringLiteral(")");
}

} // namespace jp::crunch

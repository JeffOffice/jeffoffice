#include "core/findops.h"

#include <QCoreApplication>

namespace jp::write {

namespace {

QString problem(const char *text) { return QCoreApplication::translate("jp::write::Find", text); }

}

FindPattern wildcardPattern(const QString &pattern, bool matchCase)
{
    FindPattern out;
    if (pattern.isEmpty()) {
        out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "Type what to find."));
        return out;
    }
    if (pattern.size() > kMostPatternLength) {
        out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "The pattern is too long."));
        return out;
    }
    QString re;
    int groups = 0;
    for (int i = 0; i < pattern.size(); ++i) {
        const QChar c = pattern[i];
        const QChar next = i + 1 < pattern.size() ? pattern[i + 1] : QChar();
        if (c == QLatin1Char('\\') && !next.isNull()) {
            re += QRegularExpression::escape(QString(next));
            ++i;
        } else if (c == QLatin1Char('^') && !next.isNull()) {
            ++i;
            switch (next.unicode()) {
            case 'p': re += QStringLiteral("\n"); break;
            case 't': re += QStringLiteral("\t"); break;
            case '#': re += QStringLiteral("\\d"); break;
            case '$': re += QStringLiteral("[A-Za-z]"); break;
            case 'w': re += QStringLiteral("[ \\t]+"); break;
            case '^': re += QStringLiteral("\\^"); break;
            default: re += QRegularExpression::escape(QString(c) + next); break;
            }
        } else if (c == QLatin1Char('?')) {
            re += QStringLiteral("[^\n]");
        } else if (c == QLatin1Char('*')) {
            re += QStringLiteral("[^\n]*?");
        } else if (c == QLatin1Char('<')) {
            re += QStringLiteral("\\b(?=\\w)");
        } else if (c == QLatin1Char('>')) {
            re += QStringLiteral("(?<=\\w)\\b");
        } else if (c == QLatin1Char('@')) {
            re += QLatin1Char('+');
        } else if (c == QLatin1Char('[')) {
            const int close = int(pattern.indexOf(QLatin1Char(']'), i + 2));
            if (close < 0) {
                out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "A [ has no ] to close it."));
                return out;
            }
            QString set = pattern.mid(i + 1, close - i - 1);
            const bool negate = set.startsWith(QLatin1Char('!'));
            if (negate) set.remove(0, 1);
            set.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
            set.replace(QLatin1Char('^'), QStringLiteral("\\^"));
            set.replace(QLatin1Char('['), QStringLiteral("\\["));
            re += QLatin1Char('[') + (negate ? QStringLiteral("^") : QString()) + set + QLatin1Char(']');
            i = close;
        } else if (c == QLatin1Char('{')) {
            const int close = int(pattern.indexOf(QLatin1Char('}'), i + 1));
            static const QRegularExpression counts(QStringLiteral("^\\d{1,3}(,\\d{0,3})?$"));
            if (close < 0 || !counts.match(pattern.mid(i + 1, close - i - 1)).hasMatch()) {
                out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "A { needs a count and a } to close it, such as {2} or {1,3}."));
                return out;
            }
            re += pattern.mid(i, close - i + 1);
            i = close;
        } else if (c == QLatin1Char('(')) {
            if (++groups > 9) {
                out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "A pattern can have at most nine groups."));
                return out;
            }
            re += c;
        } else if (c == QLatin1Char(')')) {
            if (--groups < 0) break;
            re += c;
        } else {
            re += QRegularExpression::escape(QString(c));
        }
    }
    if (groups != 0) {
        out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "The ( and ) in the pattern do not match."));
        return out;
    }
    out.regex = QRegularExpression(re, matchCase ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
    if (!out.regex.isValid()) {
        out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "That is not a pattern JeffWrite can use."));
        out.regex = QRegularExpression();
    }
    return out;
}

QString expandReplacement(const QString &with, const QRegularExpressionMatch &match)
{
    QString out;
    for (int i = 0; i < with.size(); ++i) {
        const QChar c = with[i];
        const QChar next = i + 1 < with.size() ? with[i + 1] : QChar();
        if (c == QLatin1Char('\\') && next.isDigit() && next != QLatin1Char('0')) {
            out += match.captured(next.digitValue());
            ++i;
        } else if (c == QLatin1Char('\\') && next == QLatin1Char('\\')) {
            out += QLatin1Char('\\');
            ++i;
        } else if (c == QLatin1Char('^') && next == QLatin1Char('&')) {
            out += match.captured(0);
            ++i;
        } else if (c == QLatin1Char('^') && next == QLatin1Char('t')) {
            out += QLatin1Char('\t');
            ++i;
        } else if (c == QLatin1Char('^') && next == QLatin1Char('^')) {
            out += QLatin1Char('^');
            ++i;
        } else {
            out += c;
        }
    }
    return out;
}

} // namespace jp::write

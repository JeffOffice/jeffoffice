#include "sheet/fnnames.h"

#include <QFile>
#include <QHash>
#include <QSet>
#include <QStringList>

#include <vector>

namespace jp::office::sheet {

namespace {

struct Column {
    QString language;
    QHash<QString, QString> toLocal;     // English (capitals) to local
    QHash<QString, QString> toEnglish;   // local (capitals) to English
};

struct Table {
    QStringList languages;
    std::vector<Column> columns;
    QStringList rows;
    QHash<QString, bool> kept;
    QHash<QString, QStringList> also;    // "language english" to the other names accepted on input
    QStringList problems;                // names that mean two things
    int index(const QString &language) const { return languages.indexOf(language); }
};

Table parseTable(const QString &text)
{
    Table out;
    const QStringList lines = text.split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        if (line.startsWith(QStringLiteral("#columns "))) {
            out.languages = line.mid(9).split(QLatin1Char(' '), Qt::SkipEmptyParts);
            out.columns.resize(size_t(out.languages.size()));
            for (int i = 0; i < out.languages.size(); ++i) out.columns[size_t(i)].language = out.languages[i];
            continue;
        }
        // (an error value begins with "#", a comment with "# ")
        if (line.isEmpty() || line.startsWith(QStringLiteral("# "))) continue;
        const QStringList p = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (p.isEmpty()) continue;
        const QString english = p[0].toUpper();
        out.rows << english;
        bool any = false;
        QString ptBR;
        for (int i = 0; i < out.languages.size() && i + 1 < p.size(); ++i) {
            QString cell = p[i + 1];
            if (cell == QLatin1String("=")) cell = ptBR;
            if (out.languages[i] == QLatin1String("pt_BR")) ptBR = cell;
            if (cell == QLatin1String("-") || cell.isEmpty()) continue;
            // (the first name is the one shown, the others after "|" are read too)
            const QStringList names = cell.split(QLatin1Char('|'), Qt::SkipEmptyParts);
            Column &c = out.columns[size_t(i)];
            for (int k = 0; k < names.size(); ++k) {
                const QString upper = names[k].toUpper();
                if (upper != english) any = true;
                if (k == 0) c.toLocal.insert(english, names[k]);
                else out.also[out.languages[i] + QLatin1Char(' ') + english] << names[k];
                const auto have = c.toEnglish.constFind(upper);
                if (have != c.toEnglish.constEnd() && *have != english)
                    out.problems << out.languages[i] + QLatin1Char(' ') + upper + QStringLiteral(" is both ") + *have + QStringLiteral(" and ") + english;
                c.toEnglish.insert(upper, english);
            }
        }
        out.kept.insert(english, !any);
    }
    // (a local name that is another function's English name would be read as that function)
    const QSet<QString> have(out.rows.begin(), out.rows.end());
    for (const Column &c : out.columns)
        for (auto it = c.toEnglish.constBegin(); it != c.toEnglish.constEnd(); ++it)
            if (it.key() != it.value() && have.contains(it.key()))
                out.problems << c.language + QLatin1Char(' ') + it.key() + QStringLiteral(" is ") + it.value() + QStringLiteral(" here and a function's English name");
    out.problems.sort();
    return out;
}

const Table &table()
{
    static const Table t = [] {
        QFile f(QStringLiteral(":/functionnames.txt"));
        if (!f.open(QIODevice::ReadOnly)) return Table();
        return parseTable(QString::fromUtf8(f.readAll()));
    }();
    return t;
}

}

QString formulaLanguageOf(const QString &uiLanguage)
{
    QString l = uiLanguage;
    l.replace(QLatin1Char('-'), QLatin1Char('_'));
    if (l.isEmpty() || l.startsWith(QLatin1String("en"))) return QString();
    const Table &t = table();
    if (t.index(l) >= 0) return l;
    if (l.startsWith(QLatin1String("es"))) return t.index(QStringLiteral("es")) >= 0 ? QStringLiteral("es") : QString();
    const QString base = l.section(QLatin1Char('_'), 0, 0);
    if (base == QLatin1String("pt")) return QStringLiteral("pt_BR");
    return t.index(base) >= 0 ? base : QString();
}

QStringList formulaNameLanguages() { return table().languages; }

QString localFormulaName(const QString &english, const QString &language)
{
    const Table &t = table();
    const int i = t.index(language);
    if (i < 0) return english;
    const auto at = t.columns[size_t(i)].toLocal.constFind(english.toUpper());
    return at == t.columns[size_t(i)].toLocal.constEnd() ? english : *at;
}

QString englishFormulaName(const QString &local, const QString &language)
{
    const Table &t = table();
    const int i = t.index(language);
    if (i < 0 || local.size() > kMaxFormulaNameChars) return QString();
    const auto at = t.columns[size_t(i)].toEnglish.constFind(local.toUpper());
    return at == t.columns[size_t(i)].toEnglish.constEnd() ? QString() : *at;
}

QStringList formulaNameRows() { return table().rows; }

int localFormulaNameCount(const QString &language)
{
    const Table &t = table();
    const int i = t.index(language);
    if (i < 0) return 0;
    int n = 0;
    for (auto it = t.columns[size_t(i)].toLocal.constBegin(); it != t.columns[size_t(i)].toLocal.constEnd(); ++it)
        if (!it.key().startsWith(QLatin1Char('#')) && it.value().toUpper() != it.key()) ++n;
    return n;
}

QStringList formulaNameAlternatives(const QString &english, const QString &language)
{
    return table().also.value(language + QLatin1Char(' ') + english.toUpper());
}

QStringList formulaNameProblems(const QString &tableText) { return parseTable(tableText).problems; }

QStringList formulaNameProblems() { return table().problems; }

bool formulaNameKeptEnglish(const QString &english) { return table().kept.value(english.toUpper(), false); }

} // namespace jp::office::sheet

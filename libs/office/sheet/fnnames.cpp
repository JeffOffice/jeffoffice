#include "sheet/fnnames.h"

#include <QFile>
#include <QHash>
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
    int index(const QString &language) const { return languages.indexOf(language); }
};

const Table &table()
{
    static const Table t = [] {
        Table out;
        QFile f(QStringLiteral(":/functionnames.txt"));
        if (!f.open(QIODevice::ReadOnly)) return out;
        const QStringList lines = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'));
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
                QString name = p[i + 1];
                if (name == QLatin1String("=")) name = ptBR;
                if (out.languages[i] == QLatin1String("pt_BR")) ptBR = name;
                if (name == QLatin1String("-") || name.isEmpty()) continue;
                const QString upper = name.toUpper();
                if (upper != english) any = true;
                Column &c = out.columns[size_t(i)];
                c.toLocal.insert(english, name);
                c.toEnglish.insert(upper, english);
            }
            out.kept.insert(english, !any);
        }
        return out;
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

bool formulaNameKeptEnglish(const QString &english) { return table().kept.value(english.toUpper(), false); }

} // namespace jp::office::sheet

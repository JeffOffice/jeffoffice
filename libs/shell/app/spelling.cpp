#include "app/spelling.h"

#include "app/settings.h"
#include "text/dictionaries.h"
#include "text/textprops.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

namespace jp {

Speller &Speller::get()
{
    static Speller s;
    return s;
}

bool Speller::check(const QString &code, const QString &w)
{
    if (w.isEmpty()) return true;
    if (m_ignore.contains(w.toLower()) || m_custom.contains(w) || m_custom.contains(w.toLower())) return true;
    Settings &st = Settings::get();
    if (st.value("proof/ignoreUpper", true).toBool() && w == w.toUpper() && w.size() > 1) return true;
    if (st.value("proof/ignoreNumbers", true).toBool() && w.contains(QRegularExpression("\\d"))) return true;
    return dict::spell(code, w);
}

QStringList Speller::suggest(const QString &code, const QString &w) { return dict::suggest(code, w); }

void Speller::ignore(const QString &w) { m_ignore.insert(w.toLower()); }

void Speller::add(const QString &w)
{
    m_custom.insert(w);
    QFile f(userDict());
    if (f.open(QIODevice::Append)) f.write((w + "\n").toUtf8());
}

Speller::Speller()
{
    QFile f(userDict());
    if (f.open(QIODevice::ReadOnly))
        for (const QByteArray &line : f.readAll().split('\n'))
            if (!line.trimmed().isEmpty()) m_custom.insert(QString::fromUtf8(line.trimmed()));
}

QString Speller::userDict()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/custom.dic";
}

const QRegularExpression &spellingWordPattern()
{
    static const QRegularExpression re(QStringLiteral("[\\p{L}][\\p{L}\\p{M}'’]*"));
    return re;
}

QVector<QPair<int, int>> misspellings(QTextDocument *doc)
{
    QVector<QPair<int, int>> out;
    if (dict::root().isEmpty()) return out;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        const QString text = b.text();
        auto it = spellingWordPattern().globalMatch(text);
        while (it.hasNext()) {
            const auto m = it.next();
            QString w = m.captured();
            while (w.endsWith('\'') || w.endsWith(QChar(0x2019))) w.chop(1);
            if (w.size() < 2) continue;
            QTextCursor c(doc);
            c.setPosition(b.position() + m.capturedStart() + 1);
            if (c.charFormat().boolProperty(tp::NoProof)) continue;
            // Text in a language without a dictionary isn't checked.
            const QString code = dict::match(c.charFormat().stringProperty(tp::Language));
            if (code.isEmpty() || !dict::hasSpelling(code)) continue;
            if (!Speller::get().check(code, w)) out << qMakePair(b.position() + int(m.capturedStart()), b.position() + int(m.capturedStart() + w.size()));
        }
    }
    return out;
}

QStringList spellingSuggestions(const QString &w, const QString &language)
{
    const QString code = dict::match(language);
    return code.isEmpty() ? QStringList() : Speller::get().suggest(code, w);
}
void spellingAdd(const QString &w) { Speller::get().add(w); }
void spellingIgnore(const QString &w) { Speller::get().ignore(w); }

} // namespace jp

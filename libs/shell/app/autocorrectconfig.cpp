#include "app/autocorrectconfig.h"

#include "app/settings.h"

namespace jp {

namespace {

// (A list the person emptied is kept as one entry that has no word, so that it is told from a list that was never changed.)
const QString kEmpty = QStringLiteral("\t");

QStringList readList(const QString &key)
{
    const QVariant v = Settings::get().value(key);
    QStringList list = v.toStringList();
    if (!v.isValid() || list.isEmpty()) return {};
    list.removeAll(kEmpty);
    list.removeAll(QString());
    return list.isEmpty() ? QStringList{kEmpty} : list;
}

void writeList(const QString &key, const QStringList &list, const QStringList &program)
{
    if (list == program) Settings::get().setValue(key, QVariant());
    else Settings::get().setValue(key, list.isEmpty() ? QStringList{kEmpty} : list);
}

} // namespace

autotext::Config autoCorrectConfig(const QString &language)
{
    autotext::Config c = autotext::defaultConfig(language);
    Settings &st = Settings::get();
    const QString base = autotext::baseLanguage(language);
    c.replace = st.value(QStringLiteral("proof/ac/replaceText"), true).toBool();
    c.sentences = st.value(QStringLiteral("proof/ac/sentences"), true).toBool();
    c.cells = st.value(QStringLiteral("proof/ac/cells"), true).toBool();
    c.days = st.value(QStringLiteral("proof/ac/days"), true).toBool();
    c.twoInitials = st.value(QStringLiteral("proof/ac/twoInitials"), true).toBool();
    c.capsLock = st.value(QStringLiteral("proof/ac/capsLock"), true).toBool();
    if (const QStringList own = readList(QStringLiteral("proof/ac/replace/") + base); !own.isEmpty()) {
        c.replacements.clear();
        for (const QString &line : own) {
            const int tab = int(line.indexOf(QLatin1Char('\t')));
            if (tab > 0) c.replacements.append({line.left(tab), line.mid(tab + 1)});
        }
    }
    if (const QStringList own = readList(QStringLiteral("proof/ac/exceptions/") + base); !own.isEmpty()) {
        c.exceptions = own;
        c.exceptions.removeAll(kEmpty);
    }
    if (const QStringList own = readList(QStringLiteral("proof/ac/initialExceptions/") + base); !own.isEmpty()) {
        c.initialExceptions = own;
        c.initialExceptions.removeAll(kEmpty);
    }
    return c;
}

void saveAutoCorrectConfig(const autotext::Config &config)
{
    Settings &st = Settings::get();
    const QString base = autotext::baseLanguage(config.language);
    st.setValue(QStringLiteral("proof/ac/replaceText"), config.replace);
    st.setValue(QStringLiteral("proof/ac/sentences"), config.sentences);
    st.setValue(QStringLiteral("proof/ac/cells"), config.cells);
    st.setValue(QStringLiteral("proof/ac/days"), config.days);
    st.setValue(QStringLiteral("proof/ac/twoInitials"), config.twoInitials);
    st.setValue(QStringLiteral("proof/ac/capsLock"), config.capsLock);
    QStringList replacements, program;
    for (const auto &entry : config.replacements) replacements << entry.first + QLatin1Char('\t') + entry.second;
    for (const auto &entry : autotext::defaultReplacements(config.language)) program << entry.first + QLatin1Char('\t') + entry.second;
    writeList(QStringLiteral("proof/ac/replace/") + base, replacements, program);
    writeList(QStringLiteral("proof/ac/exceptions/") + base, config.exceptions, autotext::defaultExceptions(config.language));
    writeList(QStringLiteral("proof/ac/initialExceptions/") + base, config.initialExceptions, autotext::defaultInitialExceptions(config.language));
}

} // namespace jp

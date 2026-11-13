#include "text/dictionaries.h"

#include "core/appinfo.h"
#include "core/datafolder.h"
#include "text/dictpack.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QMutex>

#include <hunspell/hunspell.hxx>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifdef Q_OS_WIN
#include <windows.h>   // GetShortPathNameW
#endif

// Our patch to Hunspell (third_party/CMakeLists.txt): names this function says it has in memory are read from there, not from disk.
extern bool (*jeffoffice_memory_file)(const char *name, const char **data, size_t *size);

namespace jp::dict {

namespace {

// The dictionary files Hunspell is loading, by the names given to it (set only while the checker is built, under the mutex).
const std::map<std::string, QByteArray> *s_memoryFiles = nullptr;

bool memoryFile(const char *name, const char **data, size_t *size)
{
    if (!s_memoryFiles) return false;
    const auto it = s_memoryFiles->find(name);
    if (it == s_memoryFiles->end()) return false;
    *data = it->second.constData();
    *size = size_t(it->second.size());
    return true;
}

struct Entry {
    const char *code, *name;
    const char *spelling;      // .aff/.dic without the extension, under root()
    const char *hyphenation;   // under root()
    const char *common = nullptr;   // the words every region shares (Spanish), loaded first, with `spelling`'s .dic added on top
    const char *affix = nullptr;    // the .aff, when it is another entry's (several regions share one)
};

const Entry kEntries[] = {
    {"en-US", QT_TRANSLATE_NOOP("Text", "English (United States)"), "en/en_US", "en/hyph_en_US.dic"},
    {"en-GB", QT_TRANSLATE_NOOP("Text", "English (United Kingdom)"), "en/en_GB", "en/hyph_en_GB.dic"},
    {"en-CA", QT_TRANSLATE_NOOP("Text", "English (Canada)"), "en/en_CA", "en/hyph_en_US.dic"},
    {"en-AU", QT_TRANSLATE_NOOP("Text", "English (Australia)"), "en/en_AU", "en/hyph_en_GB.dic"},
    {"es-MX", QT_TRANSLATE_NOOP("Text", "Spanish (Mexico)"), "es/es_MX", "es/hyph_es.dic", "es/es_common", "es/es_MX"},
    {"es-ES", QT_TRANSLATE_NOOP("Text", "Spanish (Spain)"), "es/es_ES", "es/hyph_es.dic", "es/es_common", "es/es_ES"},
    {"es-AR", QT_TRANSLATE_NOOP("Text", "Spanish (Argentina)"), "es/es_AR", "es/hyph_es.dic", "es/es_common", "es/es_AR"},
    {"es-BO", QT_TRANSLATE_NOOP("Text", "Spanish (Bolivia)"), "es/es_BO", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-CL", QT_TRANSLATE_NOOP("Text", "Spanish (Chile)"), "es/es_CL", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-CO", QT_TRANSLATE_NOOP("Text", "Spanish (Colombia)"), "es/es_CO", "es/hyph_es.dic", "es/es_common", "es/es_CO"},
    {"es-CR", QT_TRANSLATE_NOOP("Text", "Spanish (Costa Rica)"), "es/es_CR", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-CU", QT_TRANSLATE_NOOP("Text", "Spanish (Cuba)"), "es/es_CU", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-DO", QT_TRANSLATE_NOOP("Text", "Spanish (Dominican Republic)"), "es/es_DO", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-EC", QT_TRANSLATE_NOOP("Text", "Spanish (Ecuador)"), "es/es_EC", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-SV", QT_TRANSLATE_NOOP("Text", "Spanish (El Salvador)"), "es/es_SV", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-GT", QT_TRANSLATE_NOOP("Text", "Spanish (Guatemala)"), "es/es_GT", "es/hyph_es.dic", "es/es_common", "es/es_GT"},
    {"es-HN", QT_TRANSLATE_NOOP("Text", "Spanish (Honduras)"), "es/es_HN", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-NI", QT_TRANSLATE_NOOP("Text", "Spanish (Nicaragua)"), "es/es_NI", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-PA", QT_TRANSLATE_NOOP("Text", "Spanish (Panama)"), "es/es_PA", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-PY", QT_TRANSLATE_NOOP("Text", "Spanish (Paraguay)"), "es/es_PY", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-PE", QT_TRANSLATE_NOOP("Text", "Spanish (Peru)"), "es/es_PE", "es/hyph_es.dic", "es/es_common", "es/es_MX"},
    {"es-PR", QT_TRANSLATE_NOOP("Text", "Spanish (Puerto Rico)"), "es/es_PR", "es/hyph_es.dic", "es/es_common", "es/es_BO"},
    {"es-US", QT_TRANSLATE_NOOP("Text", "Spanish (United States)"), "es/es_US", "es/hyph_es.dic", "es/es_common", "es/es_US"},
    {"es-UY", QT_TRANSLATE_NOOP("Text", "Spanish (Uruguay)"), "es/es_UY", "es/hyph_es.dic", "es/es_common", "es/es_UY"},
    {"es-VE", QT_TRANSLATE_NOOP("Text", "Spanish (Venezuela)"), "es/es_VE", "es/hyph_es.dic", "es/es_common", "es/es_MX"},
    {"fr-FR", QT_TRANSLATE_NOOP("Text", "French"), "fr_FR/fr", "fr_FR/hyph_fr.dic"},
    {"de-DE", QT_TRANSLATE_NOOP("Text", "German"), "de/de_DE_frami", "de/hyph_de_DE.dic"},
    {"it-IT", QT_TRANSLATE_NOOP("Text", "Italian"), "it_IT/it_IT", "it_IT/hyph_it_IT.dic"},
    {"nl-NL", QT_TRANSLATE_NOOP("Text", "Dutch"), "nl_NL/nl_NL", "nl_NL/hyph_nl_NL.dic"},
    {"pt-BR", QT_TRANSLATE_NOOP("Text", "Portuguese (Brazil)"), "pt_BR/pt_BR", "pt_BR/hyph_pt_BR.dic"},
    {"pt-PT", QT_TRANSLATE_NOOP("Text", "Portuguese (Portugal)"), "pt_PT/pt_PT", "pt_PT/hyph_pt_PT.dic"},
    {"ru-RU", QT_TRANSLATE_NOOP("Text", "Russian"), "ru_RU/ru_RU", "ru_RU/hyph_ru_RU.dic"},
    {"id-ID", QT_TRANSLATE_NOOP("Text", "Indonesian"), "id/id_ID", "id/hyph_id_ID.dic"},
    {"uk-UA", QT_TRANSLATE_NOOP("Text", "Ukrainian"), "uk_UA/uk_UA", "uk_UA/hyph_uk_UA.dic"},
};

const Entry *entry(const QString &code)
{
    for (const Entry &e : kEntries)
        if (code == QLatin1String(e.code)) return &e;
    return nullptr;
}

// A Hunspell checker and the encoding its files use: UTF-8, or ISO 8859-1
// (the German dictionary).
struct Checker {
    std::unique_ptr<Hunspell> h;
    bool utf8 = true;
    std::string encode(QString w) const
    {
        if (utf8) return w.toStdString();
        w.replace(QChar(0x2019), QLatin1Char('\''));
        return w.toLatin1().toStdString();
    }
    QString decode(const std::string &s) const
    {
        return utf8 ? QString::fromStdString(s) : QString::fromLatin1(s.data(), qsizetype(s.size()));
    }
};

QMutex &mutex()
{
    static QMutex m;
    return m;
}

// Loaded on first use: the big dictionaries take a moment.
QHash<QString, std::shared_ptr<Checker>> &loadedTable()
{
    static QHash<QString, std::shared_ptr<Checker>> loaded;
    return loaded;
}

bool s_thesaurusTried = false;
QString s_rootOverride;

QMutex &thesaurusMutex()
{
    static QMutex m;
    return m;
}

dictpack::Thesaurus &thesaurusFile()
{
    static dictpack::Thesaurus t;
    return t;
}

// Opened on first use (the data folder's compressed file, else the plain file of the source tree or an older install).
dictpack::Thesaurus &thesaurus()
{
    dictpack::Thesaurus &t = thesaurusFile();
    QMutexLocker lock(&thesaurusMutex());
    if (!s_thesaurusTried) {
        s_thesaurusTried = true;
        const QString path = root().isEmpty() ? QString() : root() + QLatin1String("/en/th_en_US_v2.dat");
        if (!path.isEmpty() && QFile::exists(path + QLatin1String(dictpack::kSuffix))) t.open(path + QLatin1String(dictpack::kSuffix));
        else if (!path.isEmpty() && QFile::exists(path)) {
            // The plain file (the source tree, or an older install): packed in memory (fast, not small), so one way of looking words up.
            QFile f(path);
            if (f.open(QIODevice::ReadOnly)) t.openMemory(dictpack::packThesaurus(f.readAll(), 1));
        }
    }
    return t;
}

Checker *checker(const QString &code)
{
    QHash<QString, std::shared_ptr<Checker>> &loaded = loadedTable();
    if (!entry(code)) return nullptr;   // (only a language of the list is kept: a file's odd tags cannot grow the table)
    auto it = loaded.constFind(code);
    if (it != loaded.constEnd()) return it->get();
    std::shared_ptr<Checker> c;
    const Entry *e = entry(code);
    const QString base = e && !root().isEmpty() ? root() + '/' + QLatin1String(e->spelling) : QString();
    const QString affix = e && e->affix ? root() + '/' + QLatin1String(e->affix) : base;
    const QString common = e && e->common ? root() + '/' + QLatin1String(e->common) : QString();
    if (!base.isEmpty() && fileExists(base + ".dic") && fileExists(affix + ".aff") && (common.isEmpty() || fileExists(common + ".dic"))) {
        // The files are read (and inflated, when compressed) into memory and handed to Hunspell there, then dropped: only what Hunspell
        // builds from them stays. A file that cannot be read leaves the language with no checker (every word is accepted).
        std::map<std::string, QByteArray> files;
        files["jeffoffice-memory:aff"] = readFile(affix + ".aff");
        files["jeffoffice-memory:dic"] = readFile((common.isEmpty() ? base : common) + ".dic");
        if (!common.isEmpty()) files["jeffoffice-memory:region"] = readFile(base + ".dic");
        bool whole = true;
        for (const auto &f : files) whole = whole && !f.second.isEmpty();
        if (whole) {
            c = std::make_shared<Checker>();
            s_memoryFiles = &files;
            jeffoffice_memory_file = memoryFile;
            // (a region's own words are added to the common ones: the same words as one file of both)
            c->h = std::make_unique<Hunspell>("jeffoffice-memory:aff", "jeffoffice-memory:dic");
            if (!common.isEmpty()) c->h->add_dic("jeffoffice-memory:region");
            jeffoffice_memory_file = nullptr;
            s_memoryFiles = nullptr;
            c->utf8 = QString::fromStdString(c->h->get_dict_encoding()).compare(QLatin1String("UTF-8"), Qt::CaseInsensitive) == 0;
        }
    }
    loaded.insert(code, c);
    return c.get();
}

} // namespace

const QVector<Language> &languages()
{
    static const QVector<Language> list = [] {
        QVector<Language> l;
        for (const Entry &e : kEntries) l << Language{QLatin1String(e.code), QCoreApplication::translate("Text", e.name)};
        return l;
    }();
    return list;
}

QString languageName(const QString &tag)
{
    if (const Entry *e = entry(tag)) return QCoreApplication::translate("Text", e->name);
    return tag;
}

QString match(const QString &tag)
{
    if (tag.trimmed().isEmpty()) return QStringLiteral("en-US");
    const QStringList parts = QString(tag).replace('_', '-').split('-', Qt::SkipEmptyParts);
    const QString lang = parts.value(0).toLower();
    const QString region = parts.value(1).toUpper();
    const QString exact = lang + '-' + region;
    if (entry(exact)) return exact;
    if (lang == QLatin1String("en"))
        return QStringList{"IE", "NZ", "ZA", "IN"}.contains(region) ? QStringLiteral("en-GB") : QStringLiteral("en-US");
    if (lang == QLatin1String("pt"))
        return QStringList{"AO", "MZ", "CV", "GW", "ST", "TL"}.contains(region) ? QStringLiteral("pt-PT") : QStringLiteral("pt-BR");
    if (lang == QLatin1String("es")) return QStringLiteral("es-MX");
    for (const Entry &e : kEntries)
        if (QLatin1String(e.code).startsWith(lang + '-')) return QLatin1String(e.code);
    return {};
}

int loadedCount() { return int(loadedTable().size()); }

void setRootOverride(const QString &dir)
{
    QMutexLocker lock(&mutex());
    s_rootOverride = dir;
    loadedTable().clear();
    QMutexLocker thesaurusLock(&thesaurusMutex());
    thesaurusFile().close();
    s_thesaurusTried = false;
}

QString root()
{
    if (!s_rootOverride.isEmpty()) return s_rootOverride;
    static const QString dir = [] {
        const QString app = QCoreApplication::applicationDirPath();
        QStringList dirs;
        if (const QString data = dataFolder(QStringLiteral("dict")); !data.isEmpty()) dirs << data;
        dirs << app + "/dict" << app + "/../share/jeffoffice/dict" << app + "/../share/" + appInfo().fileName + "/dict" << app + "/../dict"
             << app + "/../Resources/dict";
#ifdef JP_SOURCE_DIR
        dirs << QStringLiteral(JP_SOURCE_DIR) + "/resources/dict";
#endif
        for (const QString &d : dirs)
            if (fileExists(d + "/en/en_US.dic")) return d;
        return QString();
    }();
    return dir;
}

bool hasSpelling(const QString &code)
{
    QMutexLocker lock(&mutex());
    return checker(code) != nullptr;
}

bool spell(const QString &code, const QString &word)
{
    QMutexLocker lock(&mutex());
    Checker *c = checker(code);
    if (!c || word.isEmpty()) return true;
    if (c->h->spell(c->encode(word))) return true;
    // Dictionaries list words with one apostrophe or the other.
    const QChar curly(0x2019);
    if (word.contains(curly)) return c->h->spell(c->encode(QString(word).replace(curly, QLatin1Char('\''))));
    if (word.contains(QLatin1Char('\''))) return c->h->spell(c->encode(QString(word).replace(QLatin1Char('\''), curly)));
    return false;
}

QStringList suggest(const QString &code, const QString &word)
{
    QMutexLocker lock(&mutex());
    QStringList out;
    Checker *c = checker(code);
    if (!c) return out;
    for (const std::string &s : c->h->suggest(c->encode(word))) out << c->decode(s);
    return out;
}

QByteArray hunspellFileName(const QString &path)
{
    const QByteArray local = QFile::encodeName(path);
#ifdef Q_OS_WIN
    if (QFile::decodeName(local) != path) {
        const std::wstring wide = QDir::toNativeSeparators(path).toStdWString();
        std::vector<wchar_t> shortName(32768);
        const DWORD n = GetShortPathNameW(wide.c_str(), shortName.data(), DWORD(shortName.size()));
        if (n > 0 && n < shortName.size()) return QFile::encodeName(QString::fromWCharArray(shortName.data(), int(n)));
    }
#endif
    return local;
}

QString hyphenationFile(const QString &code)
{
    const Entry *e = entry(code);
    if (!e || root().isEmpty() || !*e->hyphenation) return {};
    const QString f = root() + '/' + QLatin1String(e->hyphenation);
    return fileExists(f) ? f : QString();
}

bool fileExists(const QString &path)
{
    return QFile::exists(path + QLatin1String(dictpack::kSuffix)) || QFile::exists(path);
}

QByteArray readFile(const QString &path)
{
    QFile packed(path + QLatin1String(dictpack::kSuffix));
    if (packed.exists()) {
        // (a compressed file that cannot be read is not made up for by a plain one beside it: the two would disagree)
        QByteArray out;
        if (!packed.open(QIODevice::ReadOnly) || packed.size() > dictpack::kMaxFile || !dictpack::unpack(packed.readAll(), out)) return {};
        return out;
    }
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}


QByteArray thesaurusEntry(const QByteArray &lowerWord) { return thesaurus().entry(lowerWord); }

bool hasThesaurus() { return thesaurus().isOpen(); }

} // namespace jp::dict

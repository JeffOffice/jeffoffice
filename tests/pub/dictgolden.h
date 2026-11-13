#pragma once
// A word list per language, drawn from the plain dictionaries of resources/dict, and what the checker says of each word (and its
// suggestions for a few misspelled ones), as one SHA-1 per language. tests/data/dict_golden.txt was made (tools/dictbench.cpp --golden) when
// the dictionaries were read as plain files; the test (tst_pub_dict.cpp) checks the compressed ones in the install give the same.

#include "app/proofingdialogs.h"
#include "text/dictionaries.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>
#include <QStringList>

namespace jp::dictgolden {

struct Source {
    const char *code;
    const char *files;   // the plain .dic files under resources/dict whose words are used, separated by spaces
};

inline const Source kSources[] = {
    {"en-US", "en/en_US.dic"}, {"en-GB", "en/en_GB.dic"}, {"en-CA", "en/en_CA.dic"}, {"en-AU", "en/en_AU.dic"},
    {"es-MX", "es/es_common.dic es/es_MX.dic"}, {"es-ES", "es/es_ES.dic"}, {"es-AR", "es/es_AR.dic"}, {"es-BO", "es/es_BO.dic"},
    {"es-CL", "es/es_CL.dic"}, {"es-CO", "es/es_CO.dic"}, {"es-CR", "es/es_CR.dic"}, {"es-CU", "es/es_CU.dic"},
    {"es-DO", "es/es_DO.dic"}, {"es-EC", "es/es_EC.dic"}, {"es-SV", "es/es_SV.dic"}, {"es-GT", "es/es_GT.dic"},
    {"es-HN", "es/es_HN.dic"}, {"es-NI", "es/es_NI.dic"}, {"es-PA", "es/es_PA.dic"}, {"es-PY", "es/es_PY.dic"},
    {"es-PE", "es/es_PE.dic"}, {"es-PR", "es/es_PR.dic"}, {"es-US", "es/es_US.dic"}, {"es-UY", "es/es_UY.dic"},
    {"es-VE", "es/es_VE.dic"}, {"fr-FR", "fr_FR/fr.dic"}, {"de-DE", "de/de_DE_frami.dic"}, {"it-IT", "it_IT/it_IT.dic"},
    {"nl-NL", "nl_NL/nl_NL.dic"}, {"pt-BR", "pt_BR/pt_BR.dic"}, {"pt-PT", "pt_PT/pt_PT.dic"}, {"ru-RU", "ru_RU/ru_RU.dic"},
    {"id-ID", "id/id_ID.dic"}, {"uk-UA", "uk_UA/uk_UA.dic"},
};

// Up to about `want` words of the files, evenly spaced (the entry's flags cut off), and for each a misspelled form (two letters
// swapped, or a letter added) and an all-capitals form.
inline QStringList words(const QString &plainRoot, const Source &s, int want = 120)
{
    QStringList all;
    const bool latin1 = QLatin1String(s.code) == QLatin1String("de-DE");   // (the German file is ISO 8859-1)
    for (const QString &rel : QString::fromLatin1(s.files).split(QLatin1Char(' '))) {
        QFile f(plainRoot + QLatin1Char('/') + rel);
        if (!f.open(QIODevice::ReadOnly)) return {};
        const QList<QByteArray> lines = f.readAll().split('\n');
        for (qsizetype i = 1; i < lines.size(); ++i) {
            QByteArray w = lines[i].trimmed();
            const qsizetype cut = w.indexOf('/');
            if (cut >= 0) w.truncate(cut);
            const qsizetype tab = w.indexOf('\t');
            if (tab >= 0) w.truncate(tab);
            if (w.isEmpty()) continue;
            all << (latin1 ? QString::fromLatin1(w) : QString::fromUtf8(w));
        }
    }
    QStringList out;
    const qsizetype step = std::max<qsizetype>(1, all.size() / want);
    for (qsizetype i = 0; i < all.size(); i += step) {
        const QString w = all[i];
        out << w << w.toUpper();
        out << (w.size() >= 4 ? w.left(1) + w.at(2) + w.at(1) + w.mid(3) : w + QLatin1Char('x'));
    }
    return out;
}

// "code count valid hash": how many words, how many are right, and the SHA-1 of every word's answer and of the suggestions for the
// first six words the language rejects.
inline QString digest(const QString &plainRoot, const Source &s)
{
    const QString code = QLatin1String(s.code);
    const QStringList list = words(plainRoot, s);
    if (list.isEmpty()) return code + QStringLiteral(" no words");
    QCryptographicHash h(QCryptographicHash::Sha1);
    int valid = 0, sug = 0;
    for (const QString &w : list) {
        const bool ok = jp::dict::spell(code, w);
        valid += ok;
        h.addData((w + QLatin1Char('\t') + (ok ? QLatin1Char('1') : QLatin1Char('0')) + QLatin1Char('\n')).toUtf8());
        if (!ok && sug < 6) {
            ++sug;
            h.addData((w + QLatin1Char('\t') + jp::dict::suggest(code, w).join(QLatin1Char('|')) + QLatin1Char('\n')).toUtf8());
        }
    }
    return QStringLiteral("%1 %2 %3 %4").arg(code).arg(list.size()).arg(valid).arg(QString::fromLatin1(h.result().toHex()));
}

// 1,000 words of the thesaurus (every few headwords of the plain file, a third of them with an "s" or "ing" added so the base-form
// search is used), and the SHA-1 of the lookup's answer for each, as "thesaurus 1000 hash".
inline QStringList thesaurusWords(const QString &plainRoot)
{
    QFile f(plainRoot + QStringLiteral("/en/th_en_US_v2.dat"));
    if (!f.open(QIODevice::ReadOnly)) return {};
    QStringList heads;
    for (const QByteArray &line : f.readAll().split('\n')) {
        const qsizetype bar = line.indexOf('|');
        if (bar > 0 && !line.startsWith('(') && line.mid(bar + 1).trimmed().toInt() > 0) heads << QString::fromUtf8(line.left(bar));
    }
    QStringList out;
    const qsizetype step = std::max<qsizetype>(1, heads.size() / 1000);
    for (qsizetype i = 0; i < heads.size() && out.size() < 1000; i += step) {
        const QString w = heads[i];
        out << (out.size() % 3 == 1 ? w + QStringLiteral("s") : out.size() % 3 == 2 ? w + QStringLiteral("ing") : w);
    }
    return out;
}

inline QString thesaurusDigest(const QString &plainRoot)
{
    const QStringList list = thesaurusWords(plainRoot);
    if (list.isEmpty()) return QStringLiteral("thesaurus no words");
    QCryptographicHash h(QCryptographicHash::Sha1);
    int found = 0;
    for (const QString &w : list) {
        const QStringList r = jp::thesaurusLookup(w);
        found += !r.isEmpty();
        h.addData((w + QLatin1Char('\t') + r.join(QLatin1Char('|')) + QLatin1Char('\n')).toUtf8());
    }
    return QStringLiteral("thesaurus %1 %2 %3").arg(list.size()).arg(found).arg(QString::fromLatin1(h.result().toHex()));
}

} // namespace jp::dictgolden

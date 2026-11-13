#pragma once
// The list of translation models the program can download: for each language pair, its files (each with its address, size, and SHA-256). One JSON file lists them all
// (resources/translate/models.json); the addresses in it are relative to its "baseUrl", so moving the models to another server is a change of that one line.

#include <QMap>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVector>

namespace jp {
namespace translate {

struct ModelFile {
    QString role;      // "model", "shortlist", "vocab" (one for both languages), or "srcvocab" and "trgvocab"
    QUrl url;
    qint64 size = 0;
    QString sha256;    // lowercase hex
    QString fileName;  // what it is called in the folder of the pair
};

struct ModelPair {
    QString from, to;
    QVector<ModelFile> files;
    qint64 totalBytes() const;
    QString key() const { return from + QLatin1Char('-') + to; }
};

class Catalog {
public:
    // The catalog built in the program.
    static const Catalog &builtIn();
    // A catalog from its JSON; an invalid one (a file with no sha256, a size that is not a number, a plain http address, or, unless `allowLocal` (for tests: it lets file addresses and http ones for this computer through), a file address) is empty, with `error` saying what is wrong.
    static Catalog fromJson(const QByteArray &json, QString *error = nullptr, bool allowLocal = false);

    bool isEmpty() const { return m_pairs.isEmpty(); }
    bool hasPair(const QString &from, const QString &to) const { return m_pairs.contains(from + QLatin1Char('-') + to); }
    ModelPair pair(const QString &from, const QString &to) const { return m_pairs.value(from + QLatin1Char('-') + to); }
    // The directions a text can be translated in, for the languages named: through English where there is no pair of its own.
    bool canTranslate(const QString &from, const QString &to) const;
    // The pairs a translation from `from` to `to` uses: one, or two through English; empty if there is no way.
    QVector<ModelPair> route(const QString &from, const QString &to) const;
    QStringList languages() const;   // every code that has a pair with English

private:
    QMap<QString, ModelPair> m_pairs;
};

} // namespace translate
} // namespace jp

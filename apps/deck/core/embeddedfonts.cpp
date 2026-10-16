#include "core/embeddedfonts.h"

#include "core/fonts.h"

#include "core/fontfiles.h"
#include "core/sfnt.h"

#include <QCryptographicHash>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>

namespace jp::deck {

namespace {

struct Added {
    int id = -1;
    int users = 0;
    QStringList families;
};

struct Carried {
    QVector<EmbeddedFace> faces;
    QVector<QByteArray> fonts;   // the hashes of the added fonts this document uses
};

QMutex g_mutex;
QHash<const Document *, Carried> g_documents;
QHash<QByteArray, Added> g_added;

// The families the computer has: its own, not the ones this program added for presentations (the family of a font another presentation carries is
// still the computer's to want).
QSet<QString> computerFamilies()
{
    QSet<QString> added;
    for (const Added &a : std::as_const(g_added))
        for (const QString &family : a.families) added.insert(family.toLower());
    QSet<QString> out;
    for (const QString &family : QFontDatabase::families())
        if (!added.contains(family.toLower())) out.insert(family.toLower());
    return out;
}

} // namespace

void EmbeddedFonts::use(Document *doc, QVector<EmbeddedFace> faces)
{
    release(doc);   // whatever a document that stood at this address carried
    doc->fontAliases.clear();
    Carried carried;
    {
        QMutexLocker lock(&g_mutex);
        // Looked at once, before any face is added: the faces of one family are all added or none.
        const QSet<QString> have = computerFamilies();
        // The families the faces are of, in the order they come (a family is the typeface the file names).
        QStringList order;
        QHash<QString, QVector<int>> byFamily;
        for (int i = 0; i < faces.size(); ++i) {
            const QString key = faces[i].typeface.toLower();
            if (!byFamily.contains(key)) order << key;
            byFamily[key] << i;
        }
        for (const QString &key : std::as_const(order)) {
            const QVector<int> &members = byFamily[key];
            bool computerHasIt = have.contains(key);
            for (int i : members) computerHasIt = computerHasIt || have.contains(faces[i].info.family.toLower()) || have.contains(faces[i].info.typographicFamily.toLower());
            // The private name: one for the family (so its faces are styles of one), made from the fonts themselves (so the same fonts in two
            // presentations are one font to the database).
            QByteArray all;
            for (int slot = 0; slot < 4; ++slot)
                for (int i : members)
                    if (int(faces[i].slot) == slot) all += faces[i].font;
            if (computerHasIt || !qGuiApp || all.isEmpty()) continue;
            const QString original = faces[members.first()].info.family.isEmpty() ? faces[members.first()].typeface : faces[members.first()].info.family;
            const QString privateFamily = QString(kEmbeddedFamilyPrefix) + QString::fromLatin1(QCryptographicHash::hash(all, QCryptographicHash::Sha1).toHex().left(8)) + QLatin1Char('-') + original;
            bool any = false;
            for (int i : members) {
                const QByteArray renamed = faces[i].font.isEmpty() ? QByteArray() : fontWithFamilyName(faces[i].font, privateFamily);
                if (renamed.isEmpty()) continue;
                const QByteArray hash = QCryptographicHash::hash(renamed, QCryptographicHash::Sha1);
                if (auto known = g_added.find(hash); known != g_added.end()) {
                    ++known->users;
                    carried.fonts << hash;
                    any = true;
                    continue;
                }
                const int id = QFontDatabase::addApplicationFontFromData(renamed);
                jp::fontsChanged();
                if (id < 0) continue;
                Added added;
                added.id = id;
                added.users = 1;
                added.families = QFontDatabase::applicationFontFamilies(id);
                g_added.insert(hash, added);
                carried.fonts << hash;
                any = true;
            }
            if (any) doc->fontAliases.insert(key, privateFamily);
        }
        for (EmbeddedFace &face : faces) {
            face.font.clear();
            carried.faces << face;
        }
        g_documents.insert(doc, carried);
    }
}

void EmbeddedFonts::release(const Document *doc)
{
    QMutexLocker lock(&g_mutex);
    const auto it = g_documents.find(doc);
    if (it == g_documents.end()) return;
    for (const QByteArray &hash : std::as_const(it->fonts)) {
        const auto added = g_added.find(hash);
        if (added == g_added.end()) continue;
        if (--added->users <= 0) {
            QFontDatabase::removeApplicationFont(added->id);
            jp::fontsChanged();
            g_added.erase(added);
        }
    }
    g_documents.erase(it);
}

QVector<EmbeddedFace> EmbeddedFonts::facesOf(const Document *doc)
{
    QMutexLocker lock(&g_mutex);
    return g_documents.value(doc).faces;
}

int EmbeddedFonts::addedFontCount()
{
    QMutexLocker lock(&g_mutex);
    return int(g_added.size());
}

QStringList EmbeddedFonts::addedFamilies()
{
    QMutexLocker lock(&g_mutex);
    QStringList out;
    for (const Added &a : std::as_const(g_added)) out << a.families;
    out.removeDuplicates();
    out.sort();
    return out;
}

} // namespace jp::deck

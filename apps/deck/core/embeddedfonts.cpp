#include "core/embeddedfonts.h"

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

void EmbeddedFonts::use(const Document *doc, QVector<EmbeddedFace> faces)
{
    release(doc);   // whatever a document that stood at this address carried
    Carried carried;
    {
        QMutexLocker lock(&g_mutex);
        // Looked at once, before any face is added: the faces of one family are all added or none.
        const QSet<QString> have = computerFamilies();
        for (EmbeddedFace &face : faces) {
            if (!face.font.isEmpty() && qGuiApp) {
                const QByteArray hash = QCryptographicHash::hash(face.font, QCryptographicHash::Sha1);
                if (auto known = g_added.find(hash); known != g_added.end()) {
                    ++known->users;
                    carried.fonts << hash;
                } else if (!have.contains(face.info.family.toLower()) && !have.contains(face.info.typographicFamily.toLower())) {
                    const int id = QFontDatabase::addApplicationFontFromData(face.font);
                    if (id >= 0) {
                        Added added;
                        added.id = id;
                        added.users = 1;
                        added.families = QFontDatabase::applicationFontFamilies(id);
                        g_added.insert(hash, added);
                        carried.fonts << hash;
                    }
                }
            }
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

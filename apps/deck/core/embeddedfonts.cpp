#include "core/embeddedfonts.h"

#include <QCryptographicHash>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>

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

bool computerHasFamily(const QString &family)
{
    return !family.isEmpty() && QFontDatabase::families().contains(family, Qt::CaseInsensitive);
}

} // namespace

void EmbeddedFonts::use(const Document *doc, QVector<EmbeddedFace> faces)
{
    release(doc);   // whatever a document that stood at this address carried
    Carried carried;
    {
        QMutexLocker lock(&g_mutex);
        for (EmbeddedFace &face : faces) {
            if (!face.font.isEmpty() && qGuiApp) {
                const QByteArray hash = QCryptographicHash::hash(face.font, QCryptographicHash::Sha1);
                if (auto known = g_added.find(hash); known != g_added.end()) {
                    ++known->users;
                    carried.fonts << hash;
                } else if (!computerHasFamily(face.info.family) && !computerHasFamily(face.info.typographicFamily)) {
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

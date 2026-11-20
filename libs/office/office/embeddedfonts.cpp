#include "office/embeddedfonts.h"

#include "core/fonts.h"
#include "office/eot.h"

#include "core/fontfiles.h"
#include "core/sfnt.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <algorithm>

namespace jp::office {

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
qint64 s_mostHeld = 0;
QHash<const void *, Carried> g_documents;
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

void EmbeddedFaceIntake::add(const QString &typeface, int slot, QByteArrayView part)
{
    if (m_faces.size() >= m_limits.faces) {
        m_tooMany = true;
        return;
    }
    if (part.size() > m_limits.partBytes || m_total + part.size() > m_limits.totalBytes) {
        m_tooLarge = true;
        return;
    }
    m_total += part.size();
    ++m_decoded;
    EmbeddedFace face;
    QString why;
    if (!readEmbeddedFont(QByteArray(part.data(), part.size()), &face.font, &face.info, &why)) {
        ++m_unusable;
        if (m_firstReason.isEmpty()) m_firstReason = why;
        return;
    }
    if (!fontMayBeShown(face.info)) {
        ++m_forbidden;
        return;
    }
    if (m_whole + face.font.size() > m_limits.totalBytes) {   // (a compressed font is several times its part)
        m_tooLarge = true;
        return;
    }
    m_whole += face.font.size();
    face.typeface = typeface.left(m_limits.nameChars).trimmed();
    if (face.typeface.isEmpty()) face.typeface = face.info.family;
    face.slot = slot >= 0 && slot < 4 ? FontSlot(slot) : fontSlot(face.info.bold, face.info.italic);
    face.part = QByteArray(part.data(), part.size());
    face.pitchFamily = face.info.pitchFamily();
    face.charset = face.info.charset();
    m_faces << std::move(face);
}

QVector<EmbeddedFace> EmbeddedFaceIntake::take(QStringList *notes)
{
    if (notes) {
        if (m_unusable == 1) *notes << QCoreApplication::translate("jp::office::wml", "An embedded font in the file can't be used: %1").arg(m_firstReason);
        else if (m_unusable > 1) *notes << QCoreApplication::translate("jp::office::wml", "%1 embedded fonts in the file can't be used: %2").arg(m_unusable).arg(m_firstReason);
        if (m_forbidden > 0) *notes << QCoreApplication::translate("jp::office::wml", "Some embedded fonts in the file were not used, because their licenses do not allow it.");
        if (m_tooMany) *notes << QCoreApplication::translate("jp::office::wml", "Only the first %1 embedded fonts are read.").arg(m_limits.faces);
        if (m_tooLarge) *notes << QCoreApplication::translate("jp::office::wml", "Some embedded fonts are left out: they are too large.");
    }
    QVector<EmbeddedFace> out = std::move(m_faces);
    m_faces.clear();
    return out;
}

QHash<QString, QString> EmbeddedFonts::use(const void *doc, QVector<EmbeddedFace> faces)
{
    release(doc);   // whatever a document that stood at this address carried
    QHash<QString, QString> aliases;
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
            // presentations are one font to the database). Only the first face of each style counts, so a family a file lists many times is held
            // as at most four fonts; the faces are hashed one by one, never joined.
            QVector<int> chosen;
            bool haveSlot[4] = {false, false, false, false};
            for (int slot = 0; slot < 4; ++slot)
                for (int i : members)
                    if (int(faces[i].slot) == slot && !faces[i].font.isEmpty() && !haveSlot[slot]) {
                        haveSlot[slot] = true;
                        chosen << i;
                    }
            QCryptographicHash all(QCryptographicHash::Sha1);
            qint64 held = 0;
            for (int i : std::as_const(chosen)) {
                all.addData(faces[i].font);
                held += faces[i].font.size();
            }
            s_mostHeld = std::max(s_mostHeld, held);
            if (computerHasIt || !qGuiApp || chosen.isEmpty()) continue;
            const QString original = faces[members.first()].info.family.isEmpty() ? faces[members.first()].typeface : faces[members.first()].info.family;
            // (A family name is kept short: the font's name table takes at most 120 characters, and the hash makes the name its own.)
            const QString privateFamily = QString(kEmbeddedFamilyPrefix) + QString::fromLatin1(all.result().toHex().left(8)) + QLatin1Char('-') + original.left(60);
            bool any = false;
            for (int i : std::as_const(chosen)) {
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
            if (any) aliases.insert(key, privateFamily);
        }
        for (EmbeddedFace &face : faces) {
            face.font.clear();
            carried.faces << face;
        }
        g_documents.insert(doc, carried);
    }
    return aliases;
}

void EmbeddedFonts::release(const void *doc)
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

QVector<EmbeddedFace> EmbeddedFonts::facesOf(const void *doc)
{
    QMutexLocker lock(&g_mutex);
    return g_documents.value(doc).faces;
}

qint64 EmbeddedFonts::mostBytesHeldForTest()
{
    QMutexLocker lock(&g_mutex);
    const qint64 n = s_mostHeld;
    s_mostHeld = 0;
    return n;
}

int EmbeddedFonts::addedFontCount()
{
    QMutexLocker lock(&g_mutex);
    return int(g_added.size());
}

} // namespace jp::office

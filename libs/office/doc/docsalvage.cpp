#include "doc/docsalvage.h"

#include "doc/doc.h"
#include "doc/docformat.h"
#include "office/cfbsalvage.h"

#include <QCoreApplication>
#include <QtEndian>

#include <algorithm>

namespace jp::office::doc {

namespace {

void put32(QByteArray &b, qint64 at, quint32 v)
{
    if (at >= 0 && at + 4 <= b.size()) qToLittleEndian(v, b.data() + at);
}

// Where the header keeps its numbers of characters (the first of them) and its table of places, as readFib reads them (0 if the header is not one).
struct FibPlaces {
    qint64 ccp = 0, pairs = 0, pairsEnd = 0;
};
FibPlaces fibPlaces(const Span &wd)
{
    FibPlaces p;
    qint64 o = 32;
    const int csw = wd.u16(o);
    o += 2 + qint64(csw) * 2;
    const int cslw = wd.u16(o);
    o += 2;
    if (cslw < 11 || !wd.fits(o, qint64(cslw) * 4)) return p;
    p.ccp = o;
    o += qint64(cslw) * 4;
    const int pairs = std::min<int>(wd.u16(o), 256);
    o += 2;
    p.pairs = o;
    p.pairsEnd = o + qint64(pairs) * 8;
    return p;
}

// Whether a place of the table stream holds the table of the text's pieces the header names: the first bytes only.
bool clxLooksRight(QByteArrayView t, const Fib &fib)
{
    const Fib::Fc c = fib.at(Fib::Clx);
    if (c.lcb < 16 || qint64(c.fc) + c.lcb > t.size()) return false;
    const Span s(reinterpret_cast<const uchar *>(t.constData()), t.size());
    qint64 o = c.fc;
    while (s.u8(o) == 1) o += 3 + s.u16(o + 1);   // (property modifiers come first)
    if (o >= qint64(c.fc) + c.lcb || s.u8(o) != 2) return false;
    const qint64 lcb = s.u32(o + 1);
    if (lcb < 16 || (lcb - 4) % 12 != 0 || o + 5 + lcb > t.size()) return false;
    return s.u32(o + 5) == 0 && s.u32(o + 9) > 0;
}

// The same, whole: the positions climb from the first character, to the end of the text.
qint64 clxValid(QByteArrayView t, const Fib &fib)
{
    if (!clxLooksRight(t, fib)) return 0;
    const Span s(reinterpret_cast<const uchar *>(t.constData()), t.size());
    qint64 o = fib.at(Fib::Clx).fc;
    while (s.u8(o) == 1) o += 3 + s.u16(o + 1);
    const qint64 n = (qint64(s.u32(o + 1)) - 4) / 12;
    if (n > kMaxPieces) return 0;
    qint64 prev = -1;
    for (qint64 i = 0; i <= n; ++i) {
        const qint64 cp = s.u32(o + 5 + i * 4);
        if (cp <= prev) return 0;
        prev = cp;
    }
    return t.size();
}

bool textual(quint8 c) { return c == 9 || c == 10 || c == 13 || (c >= 0x20 && c != 0x7F && c != 0x81 && c != 0x8D && c != 0x8F && c != 0x90 && c != 0x9D); }

// Where the main text starts in the document stream, and in which coding: the first place at which as many letters as there are characters (at most 64 of them are looked at) follow, in
// one byte or two to a letter. False if there is none in the part looked at.
bool findText(const Span &wd, qint64 chars, qint64 from, qint64 *at, bool *unicode)
{
    const qint64 k = std::min<qint64>(chars, 64);
    if (k < 1) return false;
    const qint64 last = std::min<qint64>(wd.size(), kSalvageTextSearchBytes + from);
    for (qint64 o = from; o + k <= last; ++o) {
        bool eight = true;
        for (qint64 i = 0; i < k && eight; ++i) eight = textual(wd.u8(o + i));
        if (eight) {
            *at = o;
            *unicode = false;
            return true;
        }
        if (o + 2 * k > wd.size()) continue;
        bool wide = true;
        for (qint64 i = 0; i < k && wide; ++i) {
            const quint16 u = wd.u16(o + 2 * i);
            wide = (u < 0x100 && textual(quint8(u))) || (u >= 0x2000 && u < 0x2070) || (u >= 0x4E00 && u < 0xA000);
        }
        if (wide) {
            *at = o;
            *unicode = true;
            return true;
        }
    }
    return false;
}

} // namespace

QByteArray salvageDoc(const QByteArray &file, QStringList *notes, QString *error)
{
    // the document stream, by its header; each table stream, by the piece table the header names, for the one the header says is in use
    auto wordLooks = [](QByteArrayView from, const QMap<QString, QByteArray> &) {
        const Span s(reinterpret_cast<const uchar *>(from.constData()), from.size());
        return s.u16(0) == 0xA5EC && s.u16(2) >= 0xC1 && s.u16(32) == 14 && s.u16(62) >= 11;
    };
    auto wordValid = [](QByteArrayView bytes, const QMap<QString, QByteArray> &) -> qint64 {
        Fib fib;
        QString why;
        return readFib(Span(reinterpret_cast<const uchar *>(bytes.constData()), bytes.size()), &fib, &why) ? bytes.size() : 0;
    };
    auto fibOf = [](const QMap<QString, QByteArray> &found, Fib *fib) {
        const QByteArray wd = found.value(QStringLiteral("WordDocument"));
        QString why;
        return !wd.isEmpty() && readFib(Span(wd), fib, &why);
    };
    auto tableWish = [&](const char *name, bool second) {
        CfbWish w;
        w.name = QString::fromLatin1(name);
        w.required = false;
        w.looksLike = [=](QByteArrayView from, const QMap<QString, QByteArray> &found) {
            Fib fib;
            return fibOf(found, &fib) && fib.whichTable1 == second && clxLooksRight(from, fib);
        };
        w.validBytes = [=](QByteArrayView bytes, const QMap<QString, QByteArray> &found) -> qint64 {
            Fib fib;
            return fibOf(found, &fib) && fib.whichTable1 == second ? clxValid(bytes, fib) : 0;
        };
        return w;
    };
    CfbWish word;
    word.name = QStringLiteral("WordDocument");
    word.looksLike = wordLooks;
    word.validBytes = wordValid;
    CfbWish data;
    data.name = QStringLiteral("Data");
    data.required = false;
    QVector<CfbWish> wishes{word, tableWish("0Table", false), tableWish("1Table", true), data};

    CfbSalvage found;
    QString why;
    if (!salvageCompoundFile(file, wishes, &found, &why)) {
        if (error) *error = found.streams.contains(QStringLiteral("WordDocument")) ? why : QCoreApplication::translate("jp::office::doc::Salvage", "No document could be found in the file.");
        return QByteArray();
    }
    if (found.directoryNames.contains(QStringLiteral("EncryptedPackage"))) {
        if (error) *error = QCoreApplication::translate("jp::office::doc::Salvage", "The document is protected with a password, and can't be repaired.");
        return QByteArray();
    }
    Fib fib;
    fibOf(found.streams, &fib);
    if (fib.encrypted) {
        if (error) *error = QCoreApplication::translate("jp::office::doc::Salvage", "The document is protected with a password, and can't be repaired.");
        return QByteArray();
    }
    QStringList said = found.notes;
    const QString tableName = fib.whichTable1 ? QStringLiteral("1Table") : QStringLiteral("0Table");
    const QString other = fib.whichTable1 ? QStringLiteral("0Table") : QStringLiteral("1Table");
    found.streams.remove(other);   // (the stream the header does not name is not used)
    found.how.remove(other);

    // the document as it reads; if it does not, or the tables are gone, the text alone
    auto reads = [&](const CfbSalvage &s) {
        QString ignored;
        return !toDocxPackage(rebuildCompoundFile(s), &ignored, nullptr).isEmpty();
    };
    bool plain = !found.streams.contains(tableName) || !reads(found);
    if (plain) {
        const QByteArray wdBytes = found.streams.value(QStringLiteral("WordDocument"));
        const Span wd(wdBytes);
        const FibPlaces at = fibPlaces(wd);
        qint64 start = 0;
        bool wide = false;
        if (!at.ccp || at.pairs + (Fib::Clx + 1) * 8 > at.pairsEnd || fib.ccpText < 1 || !findText(wd, fib.ccpText, at.pairsEnd, &start, &wide)) {
            if (error) *error = QCoreApplication::translate("jp::office::doc::Salvage", "The document's tables are lost, and its text could not be found without them.");
            return QByteArray();
        }
        // one piece holding all the text, in the coding found, and a header that names it and nothing else
        QByteArray clx;
        clx.append(char(2));
        clx.append(QByteArray(4, '\0'));
        clx.append(QByteArray(4, '\0'));
        clx.append(QByteArray(4, '\0'));
        clx.append(QByteArray(8, '\0'));
        const qint64 length = std::min<qint64>(fib.ccpText, wide ? (wd.size() - start) / 2 : wd.size() - start);
        qToLittleEndian(quint32(length), clx.data() + 9);
        qToLittleEndian(quint32(wide ? start : (start * 2) | 0x40000000u), clx.data() + 15);
        qToLittleEndian(quint32(clx.size() - 5), clx.data() + 1);
        QByteArray header = wdBytes;
        for (qint64 slot = 0; slot < 93 && at.pairs + slot * 8 + 8 <= at.pairsEnd; ++slot) {
            put32(header, at.pairs + slot * 8, 0);
            put32(header, at.pairs + slot * 8 + 4, 0);
        }
        put32(header, at.pairs + Fib::Clx * 8, 0);
        put32(header, at.pairs + Fib::Clx * 8 + 4, quint32(clx.size()));
        for (qint64 k : {16, 20, 28, 32, 36, 40}) put32(header, at.ccp + k, 0);   // (the stories after the main text)
        put32(header, at.ccp + 12, quint32(length));
        found.streams.insert(QStringLiteral("WordDocument"), header);
        found.streams.insert(tableName, clx);
        found.streams.remove(QStringLiteral("Data"));
        if (!reads(found)) {
            if (error) *error = QCoreApplication::translate("jp::office::doc::Salvage", "The document's tables are lost, and its text could not be read without them.");
            return QByteArray();
        }
        said << QCoreApplication::translate("jp::office::doc::Salvage", "The tables that say how the text is formatted were lost, so the text is kept as plain paragraphs, without its styles, lists, tables, or pictures.");
    } else if (!found.streams.contains(QStringLiteral("Data")) && (!found.directoryRead || ((found.cutShort || found.tablesGuessed) && found.directoryNames.contains(QStringLiteral("Data"), Qt::CaseInsensitive)))) {
        said << QCoreApplication::translate("jp::office::doc::Salvage", "The pictures of the document, if it had any, could not be found.");
    }
    if (notes) *notes = said;
    return rebuildCompoundFile(found);
}

} // namespace jp::office::doc

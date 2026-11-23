#include "doc/docsalvage.h"

#include "doc/doc.h"
#include "doc/docformat.h"
#include "office/cfbsalvage.h"

#include <QCoreApplication>
#include <QtEndian>

#include <algorithm>
#include <array>
#include <memory>

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

// The property modifiers that may come before the table of pieces in the table of the text (real files have one or two), looked at at most this many.
constexpr int kMaxModifiers = 16;
thread_local SalvageWork g_work;

// Where the table of the text's pieces starts in a table stream, after the modifiers that come first; false if the place the header names does not hold one.
bool clxStart(const Span &s, const Fib &fib, qint64 *at)
{
    const Fib::Fc c = fib.at(Fib::Clx);
    const qint64 stop = qint64(c.fc) + c.lcb;
    if (c.lcb < 16 || stop > s.size()) return false;
    qint64 o = c.fc;
    int steps = 0;
    while (s.u8(o) == 1) {
        if (steps >= kMaxModifiers || o >= stop) {
            g_work.maxModifierSteps = std::max(g_work.maxModifierSteps, steps);
            return false;
        }
        ++steps;
        o += 3 + s.u16(o + 1);
    }
    g_work.maxModifierSteps = std::max(g_work.maxModifierSteps, steps);
    *at = o;
    return o < stop && s.u8(o) == 2;
}

// Whether a place of the table stream holds the table of the text's pieces the header names: the first bytes only.
bool clxLooksRight(QByteArrayView t, const Fib &fib)
{
    const Span s(reinterpret_cast<const uchar *>(t.constData()), t.size());
    qint64 o = 0;
    if (!clxStart(s, fib, &o)) return false;
    const qint64 lcb = s.u32(o + 1);
    if (lcb < 16 || (lcb - 4) % 12 != 0 || o + 5 + lcb > t.size()) return false;
    return s.u32(o + 5) == 0 && s.u32(o + 9) > 0;
}

// The same, whole: the positions climb from the first character to at least the end of the main text, and the first piece's text starts inside the document stream.
qint64 clxValid(QByteArrayView t, const Fib &fib, qint64 documentBytes)
{
    if (!clxLooksRight(t, fib)) return 0;
    const Span s(reinterpret_cast<const uchar *>(t.constData()), t.size());
    qint64 o = 0;
    clxStart(s, fib, &o);
    const qint64 n = (qint64(s.u32(o + 1)) - 4) / 12;
    if (n > kMaxPieces) return 0;
    qint64 prev = -1;
    for (qint64 i = 0; i <= n; ++i) {
        const qint64 cp = s.u32(o + 5 + i * 4);
        if (cp <= prev) return 0;
        prev = cp;
    }
    if (prev < fib.ccpText) return 0;
    const quint32 raw = s.u32(o + 5 + (n + 1) * 4 + 2);
    const qint64 first = (raw & 0x40000000) ? qint64(raw & 0x3FFFFFFF) / 2 : qint64(raw & 0x3FFFFFFF);
    if (documentBytes > 0 && first >= documentBytes) return 0;
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

// Where a byte of the stream falls in the text: before it (in the header), in a paragraph (counted from 1, by the paragraph marks of the stretches that hold text in the order of the text), or
// after it (where the formatting is kept). `ranges` are the stretches of the stream that hold text (first byte, end, and whether two bytes make a character). One pass over the bytes before
// the place, once each (`*steps` counts them).
struct TextPlace {
    enum Kind { Before, Paragraph, After } kind = After;
    qint64 paragraph = 0;
};
TextPlace placeInText(const QByteArray &wd, qint64 j, const QVector<std::array<qint64, 3>> &ranges, qint64 *steps)
{
    TextPlace out;
    if (ranges.isEmpty()) return out;
    qint64 first = ranges.first()[0], last = 0;
    for (const auto &r : ranges) {
        first = std::min(first, r[0]);
        last = std::max(last, std::min<qint64>(r[1], wd.size()));
    }
    if (j < first) {
        out.kind = TextPlace::Before;
        return out;
    }
    if (j >= last) return out;
    out.kind = TextPlace::Paragraph;
    qint64 n = 1;
    for (const auto &r : ranges) {
        const qint64 from = r[0], to = std::min<qint64>(r[1], wd.size()), unit = r[2] ? 2 : 1;
        // (a join in a stretch counts the marks before it; a join before the stretch, or after it, counts all of it or none: the stretches are in the order of the text)
        const qint64 stop = std::clamp<qint64>(j, from, to);
        for (qint64 o = from; o + unit <= stop; o += unit) {
            ++*steps;
            if (quint8(wd[qsizetype(o)]) == 13 && (unit == 1 || wd[qsizetype(o) + 1] == 0)) ++n;
        }
        if (j >= from && j < to) break;
    }
    out.paragraph = n;
    return out;
}

} // namespace

SalvageWork salvageWorkForTest() { return g_work; }

QByteArray salvageDoc(const QByteArray &file, QStringList *notes, QString *error, qint64 *joinSteps)
{
    g_work = SalvageWork();
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
    // (the header is read once for a document stream, not at each place a scan looks at)
    struct FibCache {
        const char *at = nullptr;
        qsizetype size = 0;
        Fib fib;
        bool ok = false;
    };
    auto cache = std::make_shared<FibCache>();
    auto fibOf = [cache](const QMap<QString, QByteArray> &found, Fib *fib) {
        const QByteArray wd = found.value(QStringLiteral("WordDocument"));
        if (wd.isEmpty()) return false;
        if (cache->at != wd.constData() || cache->size != wd.size()) {
            QString why;
            cache->at = wd.constData();
            cache->size = wd.size();
            cache->ok = readFib(Span(wd), &cache->fib, &why);
            ++g_work.fibReads;
        }
        *fib = cache->fib;
        return cache->ok;
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
            return fibOf(found, &fib) && fib.whichTable1 == second ? clxValid(bytes, fib, found.value(QStringLiteral("WordDocument")).size()) : 0;
        };
        // a table stream ends well when every range the header names in it lies inside it
        w.endsWell = [=](QByteArrayView bytes, const QMap<QString, QByteArray> &found) {
            CfbWish::End end;
            Fib fib;
            if (!fibOf(found, &fib) || fib.whichTable1 != second) return end;
            end.prefix = bytes.size();
            qint64 reach = 0;
            for (const Fib::Fc &c : std::as_const(fib.fcs))
                if (c.lcb > 0) reach = std::max<qint64>(reach, qint64(c.fc) + c.lcb);
            if (reach > 0 && reach <= bytes.size()) end.whole = reach;   // (it ends where the last range the header names ends)
            return end;
        };
        return w;
    };
    CfbWish word;
    word.name = QStringLiteral("WordDocument");
    word.looksLike = wordLooks;
    word.validBytes = wordValid;
    // The document stream ends well when its header is one and the stream holds every byte the header says was written (the format says the bytes from there on mean nothing). A header
    // that says nothing leaves the end unconfirmed; the stream is then whole as far as it goes.
    word.endsWell = [](QByteArrayView bytes, const QMap<QString, QByteArray> &) {
        CfbWish::End end;
        const Span wd(reinterpret_cast<const uchar *>(bytes.constData()), bytes.size());
        Fib fib;
        QString why;
        if (!readFib(wd, &fib, &why)) return end;
        end.prefix = bytes.size();
        const FibPlaces at = fibPlaces(wd);
        const qint64 written = at.ccp ? qint64(wd.u32(at.ccp)) : 0;
        if (written > 0 && written <= bytes.size()) end.whole = written;   // (it ends where the header says the last byte was written)
        return end;
    };
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
    // (where pieces were joined by guess in the text, what lies across and after the first join is shown as it is read, and the notes say where it begins)
    qint64 steps = 0;
    TextPlace joinPlace;   // where the first guessed join falls: before the text, in a paragraph, or after the text
    const QVector<qint64> textJoins = found.joins.value(QStringLiteral("WordDocument"));
    const qint64 firstJoin = textJoins.isEmpty() ? -1 : *std::min_element(textJoins.cbegin(), textJoins.cend());
    if (firstJoin >= 0 && firstJoin < found.streams.value(QStringLiteral("WordDocument")).size() && found.streams.contains(tableName)) {
        const QByteArray wdNow = found.streams.value(QStringLiteral("WordDocument"));
        const Span wdSpan(wdNow);
        const QByteArray tbl = found.streams.value(tableName);
        const Span tableSpan(tbl);
        const Fib::Fc clx = fib.at(Fib::Clx);
        PieceTable table;
        bool truncated = false;
        QVector<std::array<qint64, 3>> ranges;
        if (clx.lcb > 0 && qint64(clx.fc) + clx.lcb <= tbl.size() && table.read(tableSpan.sub(clx.fc, clx.lcb), wdSpan, fib.ccpText, &truncated))
            for (const Piece &p : table.pieces()) ranges.push_back({p.fc, p.fc + (p.cp1 - p.cp0) * (p.unicode ? 2 : 1), p.unicode ? 1 : 0});
        joinPlace = placeInText(wdNow, firstJoin, ranges, &steps);
        if (ranges.isEmpty()) joinPlace.kind = TextPlace::Before;   // (the text can't be told: the whole document is in doubt)
    }
    // (the data stream holds the pictures: they are read as far as they go, and the notes say that some may not be whole)
    bool picturesDamaged = found.partial.contains(QStringLiteral("Data")) || found.lost.contains(QStringLiteral("Data"));
    {
        const QVector<qint64> dataJoins = found.joins.value(QStringLiteral("Data"));
        if (!dataJoins.isEmpty() && dataJoins.first() < found.streams.value(QStringLiteral("Data")).size()) picturesDamaged = true;
    }
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
        const QVector<std::array<qint64, 3>> one{{start, start + length * (wide ? 2 : 1), wide ? 1 : 0}};
        if (firstJoin >= 0) joinPlace = placeInText(header, firstJoin, one, &steps);
        found.streams.insert(QStringLiteral("WordDocument"), header);
        found.streams.insert(tableName, clx);
        found.streams.remove(QStringLiteral("Data"));
        if (!reads(found)) {
            if (error) *error = QCoreApplication::translate("jp::office::doc::Salvage", "The document's tables are lost, and its text could not be read without them.");
            return QByteArray();
        }
        said << QCoreApplication::translate("jp::office::doc::Salvage", "The tables that say how the text is formatted were lost, so the text is kept as plain paragraphs, without its styles, lists, tables, pictures, headers, footers, or footnotes.");
    } else if (picturesDamaged) {
        said << QCoreApplication::translate("jp::office::doc::Salvage", "Some pictures could not be fully recovered.");
    } else if (!found.streams.contains(QStringLiteral("Data")) && (!found.directoryRead || ((found.cutShort || found.tablesGuessed) && found.directoryNames.contains(QStringLiteral("Data"), Qt::CaseInsensitive)))) {
        said << QCoreApplication::translate("jp::office::doc::Salvage", "The pictures of the document, if it had any, could not be found.");
    }
    // (what lies across or after a guessed join is shown as it is read; the notes say where the part that was not checked begins)
    if (firstJoin >= 0 && firstJoin < found.streams.value(QStringLiteral("WordDocument")).size()) {
        if (joinPlace.kind == TextPlace::Paragraph)
            said << QCoreApplication::translate("jp::office::doc::Salvage", "From paragraph %1 on, this text was pieced together from a damaged file. It may be an older version or come from another part of the file. Compare it with a backup if you have one.").arg(joinPlace.paragraph);
        else if (joinPlace.kind == TextPlace::Before)
            said << QCoreApplication::translate("jp::office::doc::Salvage", "This whole document was pieced together from a damaged file. It may be an older version or come from another part of the file. Compare it with a backup if you have one.");
        else
            said << QCoreApplication::translate("jp::office::doc::Salvage", "The formatting of this document was pieced together from a damaged file, so some of it may be wrong. Compare the document with a backup if you have one.");
    }
    // (the tables that say how the text is formatted, read across a guessed join, are shown as they are, and the notes say they were checked only for their form)
    {
        const QVector<qint64> tableJoins = found.joins.value(tableName);
        if (!plain && !tableJoins.isEmpty() && tableJoins.first() < found.streams.value(tableName).size())
            said << QCoreApplication::translate("jp::office::doc::Salvage", "Part of what says how the text is formatted was pieced together from a damaged file, so some of the formatting may be wrong.");
    }
    if (joinSteps) *joinSteps = steps + textJoins.size();
    if (notes) *notes = said;
    return rebuildCompoundFile(found);
}

} // namespace jp::office::doc

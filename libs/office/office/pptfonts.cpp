#include "office/pptfonts.h"

#include <QCoreApplication>
#include <QSet>
#include <QtEndian>

namespace jp::office {

namespace {

constexpr quint16 kFontEntityAtom = 4023, kFontEmbedDataBlob = 4024;

QString faceName(const char *p, qint64 len)
{
    QString name;
    for (qint64 i = 0; i < 64 && i + 2 <= len; i += 2) {
        const QChar ch(qFromLittleEndian<quint16>(p + i));
        if (ch.isNull()) break;
        name += ch;
    }
    return name.trimmed();
}

} // namespace

QVector<EmbeddedFace> readPptFonts(const QByteArray &stream, qint64 begin, qint64 end, QStringList *notes)
{
    return readPptFonts(stream, QList<QPair<qint64, qint64>>{{begin, end}}, notes);
}

QVector<EmbeddedFace> readPptFonts(const QByteArray &stream, const QList<QPair<qint64, qint64>> &lists, QStringList *notes)
{
    EmbeddedFaceIntake intake;
    bool tooMany = false;
    int records = 0, orphans = 0;
    QSet<QString> taken;   // (the faces of the lists before: a face in two lists is taken from the first)
    for (const auto &list : lists) {
        const qint64 end = std::min<qint64>(list.second, stream.size());
        QString name;
        bool haveName = false;
        QSet<QString> here;
        for (qint64 at = list.first; at >= 0 && at + 8 <= end && !tooMany;) {
            if (++records > kMaxPptFontRecords) {
                tooMany = true;
                break;
            }
            const quint16 verInst = qFromLittleEndian<quint16>(stream.constData() + at), type = qFromLittleEndian<quint16>(stream.constData() + at + 2);
            const quint32 len = qFromLittleEndian<quint32>(stream.constData() + at + 4);
            const qint64 data = at + 8;
            if (qint64(len) > end - data) break;   // a length that runs past the end ends the list
            if (type == kFontEntityAtom) {
                name = faceName(stream.constData() + data, len);
                haveName = true;
            } else if (type == kFontEmbedDataBlob) {
                const int slot = verInst >> 4;
                if (!haveName || slot > 3) {
                    ++orphans;
                } else {
                    const QString key = name.toLower() + QLatin1Char('|') + QString::number(slot);
                    if (!taken.contains(key)) intake.add(name, slot, QByteArrayView(stream.constData() + data, qsizetype(len)));
                    here.insert(key);
                }
            }
            at = data + len;
        }
        taken |= here;
    }
    QVector<EmbeddedFace> faces = intake.take(notes);
    if (notes) {
        if (orphans > 0) *notes << QCoreApplication::translate("jp::office::wml", "%n embedded font(s) in the file can't be used: %1", nullptr, orphans).arg(QCoreApplication::translate("jp::office", "its record is not where a font belongs."));
        if (tooMany) *notes << QCoreApplication::translate("jp::office", "Only the first %n record(s) of the font list are read.", nullptr, kMaxPptFontRecords);
    }
    return faces;
}

} // namespace jp::office

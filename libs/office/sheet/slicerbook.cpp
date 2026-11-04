// The slicers and timelines of a workbook: the parts its relationships name, read when asked for.
#include "sheet/sheetreader.h"

namespace jp::office::sheet {

namespace {

// The parts one workbook is let list: far more than real files have (a workbook has a cache for each slicer, a few dozen at most).
constexpr int kMaxSlicerParts = 20000;
constexpr qint64 kMaxSlicerPartBytes = qint64(8) << 20;

} // namespace

QVector<SlicerCache> Workbook::slicerCaches() const
{
    QVector<SlicerCache> out;
    if (m_info.sheets.isEmpty() || mainPart().isEmpty()) return out;
    for (const char *type : {"/slicerCache", "/timelineCache"}) {
        const QVector<Relationship> rels = package().relationshipsOfType(mainPart(), QString::fromLatin1(type));
        for (const Relationship &r : rels) {
            if (r.external || out.size() >= kMaxSlicerParts) continue;
            const QByteArray xml = package().part(r.target);
            if (xml.isEmpty() || xml.size() > kMaxSlicerPartBytes) continue;
            SlicerCache c;
            if (readSlicerCache(xml, &c)) out.append(c);
        }
    }
    return out;
}

QVector<SlicerView> Workbook::slicerViews(int sheet) const
{
    QVector<SlicerView> out;
    if (sheet < 0 || sheet >= m_info.sheets.size() || m_info.sheets[sheet].part.isEmpty()) return out;
    for (const char *type : {"/slicer", "/timeline"}) {
        const QVector<Relationship> rels = package().relationshipsOfType(m_info.sheets[sheet].part, QString::fromLatin1(type));
        for (const Relationship &r : rels) {
            if (r.external || out.size() >= kMaxSlicerParts) continue;
            const QByteArray xml = package().part(r.target);
            if (xml.isEmpty() || xml.size() > kMaxSlicerPartBytes) continue;
            QVector<SlicerView> v;
            if (readSlicerViews(xml, &v)) out += v;
        }
    }
    return out;
}

} // namespace jp::office::sheet

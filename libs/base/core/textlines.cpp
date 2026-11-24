#include "core/textlines.h"

namespace jp {

QList<QByteArray> textResourceLines(const QByteArray &data, bool keepTabs)
{
    QList<QByteArray> lines;
    qsizetype at = data.startsWith("\xEF\xBB\xBF") ? 3 : 0;
    const qsizetype size = data.size();
    while (at < size) {
        qsizetype end = at;
        while (end < size && data[end] != '\n' && data[end] != '\r') ++end;
        qsizetype stop = end;
        while (stop > at && (data[stop - 1] == ' ' || (!keepTabs && data[stop - 1] == '\t'))) --stop;
        lines << data.mid(at, stop - at);
        if (end < size && data[end] == '\r' && end + 1 < size && data[end + 1] == '\n') ++end;
        at = end + 1;
    }
    return lines;
}

}

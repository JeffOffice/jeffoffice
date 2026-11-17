#pragma once
// For the tests of tagged PDFs (JeffPub's and JeffDeck's): what a page's content holds, read from the file as Qt wrote it.
#include "io/qtpdf.h"

#include <QHash>
#include <QRegularExpression>
#include <QString>
#include <QVector>

namespace pdfcheck {

// The content of each page, in the file's order.
inline QVector<QByteArray> pageContents(const QByteArray &pdf)
{
    QVector<QByteArray> out;
    jp::QtPdf q;
    if (!q.loadBytes(pdf)) return out;
    for (const jp::QtPdf::Obj &o : q.objects) {
        if (!o.body.contains("/Type /Page\n")) continue;
        const int contents = jp::QtPdf::ref(o.body, "/Contents");
        const jp::QtPdf::Obj *co = contents ? q.object(contents) : nullptr;
        bool ok = false;
        const QByteArray data = co ? q.streamData(*co, &ok) : QByteArray();
        out << (ok ? data : QByteArray());
    }
    return out;
}

// How many operators that draw (text, a painted path, a picture, a shading) a page's content has outside any marked content: not in a tag, and not in an artifact.
inline int drawnOutsideMarkedContent(const QByteArray &content)
{
    static const QRegularExpression paints(QStringLiteral("(^|[\\s\\]>)])(Tj|TJ|f|f\\*|F|S|s|B|B\\*|b|b\\*|Do|sh|BI)$"));
    int depth = 0, count = 0;
    for (const QByteArray &raw : content.split('\n')) {
        const QByteArray line = raw.trimmed();
        if (line.endsWith("BDC") || line.endsWith("BMC")) ++depth;
        else if (line == "EMC") --depth;
        else if (depth <= 0 && paints.match(QString::fromLatin1(line)).hasMatch()) ++count;
    }
    return count;
}

// How many of the squares tagging draws off the page as marks are in a PDF's pages (any size: where a mark goes).
inline int marksLeft(const QByteArray &pdf)
{
    static const QRegularExpression mark(QStringLiteral("(?m)^-\\d{4,}(?:\\.\\d+)? -(?:4000|4100)(?:\\.\\d+)? [\\d.]+ [\\d.]+ re$"));
    int n = 0;
    for (const QByteArray &c : pageContents(pdf)) for (auto it = mark.globalMatch(QString::fromLatin1(c)); it.hasNext(); it.next()) ++n;
    return n;
}

// What is wrong with the structure of a tagged PDF (empty when nothing is): on each page the marked content is balanced (BDC and BMC against EMC), the MCIDs are 0, 1, 2... once each,
// and the page's entry of the ParentTree lists, for each MCID, the structure element whose content it is (its /Pg the page, its /K the MCID).
inline QString structureProblems(const QByteArray &pdf)
{
    jp::QtPdf q;
    if (!q.loadBytes(pdf)) return QStringLiteral("not readable");
    QHash<int, QByteArray> body;
    for (const jp::QtPdf::Obj &o : q.objects) body.insert(o.id, o.body);
    const QRegularExpression ref(QStringLiteral("(\\d+) 0 R"));
    // the ParentTree: page key -> the elements
    QHash<int, QVector<int>> tree;
    for (const jp::QtPdf::Obj &o : q.objects) {
        if (!o.body.contains("/Type /StructTreeRoot")) continue;
        const qsizetype nums = o.body.indexOf("/Nums [");
        const QString list = QString::fromLatin1(o.body.mid(nums + 7, o.body.indexOf("] >>", nums) - nums - 7));
        const QRegularExpression entry(QStringLiteral("(\\d+) \\[([^\\]]*)\\]"));
        for (auto it = entry.globalMatch(list); it.hasNext();) {
            const auto m = it.next();
            QVector<int> ids;
            for (auto r = ref.globalMatch(m.captured(2)); r.hasNext();) ids << r.next().captured(1).toInt();
            tree.insert(m.captured(1).toInt(), ids);
        }
    }
    for (const jp::QtPdf::Obj &o : q.objects) {
        if (!o.body.contains("/Type /Page\n")) continue;
        const int contents = jp::QtPdf::ref(o.body, "/Contents");
        bool ok = false;
        const QByteArray data = contents ? q.streamData(*q.object(contents), &ok) : QByteArray();
        if (!ok) continue;
        int depth = 0;
        for (const QByteArray &raw : data.split('\n')) {
            const QByteArray line = raw.trimmed();
            if (line.endsWith("BDC") || line.endsWith("BMC")) ++depth;
            else if (line == "EMC" && --depth < 0) return QStringLiteral("an EMC with no BDC on page %1").arg(o.id);
        }
        if (depth != 0) return QStringLiteral("marked content not closed on page %1").arg(o.id);
        QVector<int> mcids;
        for (auto it = QRegularExpression(QStringLiteral("/MCID (\\d+)")).globalMatch(QString::fromLatin1(data)); it.hasNext();) mcids << it.next().captured(1).toInt();
        for (int i = 0; i < mcids.size(); ++i)
            if (mcids[i] != i) return QStringLiteral("MCIDs on page %1 are not 0, 1, 2... (%2 at %3)").arg(o.id).arg(mcids[i]).arg(i);
        if (mcids.isEmpty()) continue;
        const auto key = QRegularExpression(QStringLiteral("/StructParents (\\d+)")).match(QString::fromLatin1(o.body));
        if (!key.hasMatch() || !tree.contains(key.captured(1).toInt())) return QStringLiteral("page %1 has no ParentTree entry").arg(o.id);
        const QVector<int> &ids = tree[key.captured(1).toInt()];
        if (ids.size() != mcids.size()) return QStringLiteral("page %1: %2 MCIDs, %3 in the ParentTree").arg(o.id).arg(mcids.size()).arg(ids.size());
        for (int i = 0; i < ids.size(); ++i) {
            const QByteArray &e = body.value(ids[i]);
            const auto pg = QRegularExpression(QStringLiteral("/Pg (\\d+) 0 R")).match(QString::fromLatin1(e));
            const bool listed = QRegularExpression(QStringLiteral("/K (?:%1\\b|\\[[^\\]]*\\b%1\\b)").arg(i)).match(QString::fromLatin1(e)).hasMatch();
            if (!pg.hasMatch() || pg.captured(1).toInt() != o.id || !listed) return QStringLiteral("page %1: MCID %2 is not the content of its element").arg(o.id).arg(i);
        }
    }
    return QString();
}

} // namespace pdfcheck

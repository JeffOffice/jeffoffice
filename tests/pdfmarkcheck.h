#pragma once
// For the tests of tagged PDFs (JeffPub's and JeffDeck's): what a page's content holds, read from the file as Qt wrote it.
#include "io/qtpdf.h"

#include <QRegularExpression>
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

} // namespace pdfcheck

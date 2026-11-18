#pragma once
// Small PDFs in the layout Qt writes (the one io/qtpdf.h reads), made by hand, for the tests of the transparency check.
#include <QByteArray>
#include <QList>

namespace jptest {

// `objects` are the bodies of objects 1, 2, 3 and on (object 1 is the catalog); a stream's is "<< ... >>\nstream\n...\nendstream" with its /Length as it is.
inline QByteArray handPdf(const QList<QByteArray> &objects)
{
    QByteArray out = "%PDF-1.4\n%\xe2\xe3\xcf\xd3\n\n";
    out.chop(1);
    for (int i = 0; i < objects.size(); ++i) out += QByteArray::number(i + 1) + " 0 obj\n" + objects[i] + "\nendobj\n";
    out += "xref\n0 1\n0000000000 65535 f \ntrailer\n<< /Size " + QByteArray::number(objects.size() + 1) + " /Root 1 0 R >>\nstartxref\n0\n%%EOF\n";
    return out;
}

inline QByteArray handStream(const QByteArray &extra, const QByteArray &data)
{
    return "<< " + extra + " /Length " + QByteArray::number(data.size()) + " >>\nstream\n" + data + "\nendstream";
}

// One page that draws `content` with the resources `res` (the text between "/Resources << " and " >>"), and the objects after the page's (from 5 on).
inline QByteArray onePagePdf(const QByteArray &res, const QByteArray &content, const QList<QByteArray> &more = {}, const QByteArray &pageExtra = {})
{
    QList<QByteArray> objects{"<< /Type /Catalog /Pages 2 0 R >>", "<< /Type /Pages /Kids [3 0 R] /Count 1 >>",
                              "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 100 100] /Resources << " + res + " >> /Contents 4 0 R " + pageExtra + " >>", handStream({}, content)};
    objects += more;
    return handPdf(objects);
}

} // namespace jptest

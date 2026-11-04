// Labels and envelopes (core/labelops.h).

#include "core/labelops.h"

#include "wml/wml.h"

#include <QCoreApplication>
#include <cmath>

namespace jp::write::mail {

namespace {

constexpr double kInch = 72.0, kMm = 72.0 / 25.4;

QString fraction(double inches)
{
    // 2.625 -> "2 5/8", 1.3333 -> "1 1/3", 4 -> "4"
    const int whole = int(std::floor(inches + 1e-6));
    const double rest = inches - whole;
    QString f;
    static const struct {
        int n, d;
    } fracs[] = {{1, 8}, {1, 4}, {1, 3}, {3, 8}, {1, 2}, {5, 8}, {2, 3}, {3, 4}, {7, 8}};
    for (const auto &c : fracs)
        if (std::fabs(rest - double(c.n) / c.d) < 0.01) f = QStringLiteral("%1/%2").arg(c.n).arg(c.d);
    if (rest < 0.01) return QString::number(whole);
    if (f.isEmpty()) return QString::number(inches, 'f', 2);
    return whole > 0 ? QStringLiteral("%1 %2").arg(whole).arg(f) : f;
}

LabelSheet inchSheet(int columns, int rows, double w, double h, double top, double left, double hPitch, double vPitch)
{
    LabelSheet s;
    s.pageW = 8.5 * kInch;
    s.pageH = 11 * kInch;
    s.columns = columns;
    s.rows = rows;
    s.labelW = w * kInch;
    s.labelH = h * kInch;
    s.top = top * kInch;
    s.left = left * kInch;
    s.hPitch = hPitch * kInch;
    s.vPitch = vPitch * kInch;
    s.name = QCoreApplication::translate("jp::write::Mail", "%1 per sheet, %2 x %3 in").arg(columns * rows).arg(fraction(h), fraction(w));
    return s;
}

LabelSheet mmSheet(int columns, int rows, double w, double h, double top, double left, double hPitch, double vPitch)
{
    LabelSheet s;
    s.pageW = 210 * kMm;
    s.pageH = 297 * kMm;
    s.columns = columns;
    s.rows = rows;
    s.labelW = w * kMm;
    s.labelH = h * kMm;
    s.top = top * kMm;
    s.left = left * kMm;
    s.hPitch = hPitch * kMm;
    s.vPitch = vPitch * kMm;
    s.name = QCoreApplication::translate("jp::write::Mail", "%1 per sheet, %2 x %3 mm, A4").arg(columns * rows).arg(QString::number(h, 'f', 1), QString::number(w, 'f', 1));
    return s;
}

QByteArray esc(const QString &s)
{
    QString t = s;
    t.replace(QLatin1Char('&'), QLatin1String("&amp;")).replace(QLatin1Char('<'), QLatin1String("&lt;")).replace(QLatin1Char('>'), QLatin1String("&gt;")).replace(QLatin1Char('"'), QLatin1String("&quot;"));
    return t.toUtf8();
}

int twips(double points) { return int(std::lround(points * 20.0)); }

const char kRootStart[] = "<w:hdr xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";

// A document made from the XML of a body and the properties of its section: the program's blank document, with the body and the section replaced.
std::unique_ptr<WriteDoc> fromBody(const QByteArray &bodyXml, const std::function<void(office::wml::SectionProps *)> &section)
{
    QString error;
    const std::unique_ptr<WriteDoc> blank = WriteDoc::blank();
    const QByteArray bytes = blank->toDocx(&error);
    office::wml::Document d;
    if (bytes.isEmpty() || !office::wml::readDocx(bytes, &d, &error)) return nullptr;
    QVector<office::wml::Block> blocks;
    if (!office::wml::readBlocks(QByteArray(kRootStart) + bodyXml + "</w:hdr>", &blocks, true)) return nullptr;
    d.body = blocks;
    section(&d.lastSection);
    d.lastSection.hasPgSz = d.lastSection.hasPgMar = true;
    d.lastSectionStated = true;
    const QByteArray written = office::wml::writeDocx(d, &error);
    if (written.isEmpty()) return nullptr;
    return WriteDoc::fromDocx(written, &error);
}

} // namespace

QVector<LabelSheet> labelSheets()
{
    QVector<LabelSheet> out;
    out << inchSheet(3, 10, 2.625, 1, 0.5, 0.1875, 2.75, 1);          // 30 per sheet
    out << inchSheet(2, 10, 4, 1, 0.5, 0.15625, 4.1875, 1);            // 20 per sheet
    out << inchSheet(2, 7, 4, 1.0 + 1.0 / 3.0, 0.8333, 0.15625, 4.1875, 1.0 + 1.0 / 3.0);   // 14 per sheet
    out << inchSheet(2, 5, 4, 2, 0.5, 0.15625, 4.1875, 2);             // 10 per sheet
    out << inchSheet(2, 3, 4, 3.0 + 1.0 / 3.0, 0.5, 0.15625, 4.1875, 3.0 + 1.0 / 3.0);   // 6 per sheet
    out << inchSheet(4, 20, 1.75, 0.5, 0.5, 0.3, 2.0625, 0.5);         // 80 per sheet (return address labels)
    out << inchSheet(1, 2, 8.5, 5.5, 0, 0, 8.5, 5.5);                  // 2 per sheet
    out << inchSheet(1, 1, 8.5, 11, 0, 0, 8.5, 11);                    // one to a sheet
    out << mmSheet(3, 7, 63.5, 38.1, 15.15, 7.2, 66.0, 38.1);          // 21 per sheet
    out << mmSheet(3, 8, 63.5, 33.9, 13.0, 7.2, 66.0, 33.9);           // 24 per sheet
    out << mmSheet(2, 7, 99.1, 38.1, 15.15, 4.65, 101.6, 38.1);        // 14 per sheet
    out << mmSheet(2, 4, 99.1, 67.7, 13.0, 4.65, 101.6, 67.7);         // 8 per sheet
    return out;
}

QVector<EnvelopeSize> envelopeSizes()
{
    auto in = [](const QString &name, double w, double h) { return EnvelopeSize{name, w * kInch, h * kInch}; };
    auto mm = [](const QString &name, double w, double h) { return EnvelopeSize{name, w * kMm, h * kMm}; };
    QVector<EnvelopeSize> out;
    out << in(QCoreApplication::translate("jp::write::Mail", "4 1/8 x 9 1/2 in"), 9.5, 4.125);
    out << in(QCoreApplication::translate("jp::write::Mail", "3 7/8 x 8 7/8 in"), 8.875, 3.875);
    out << in(QCoreApplication::translate("jp::write::Mail", "3 5/8 x 6 1/2 in"), 6.5, 3.625);
    out << in(QCoreApplication::translate("jp::write::Mail", "3 7/8 x 7 1/2 in"), 7.5, 3.875);
    out << in(QCoreApplication::translate("jp::write::Mail", "5 x 7 1/4 in"), 7.25, 5);
    out << mm(QCoreApplication::translate("jp::write::Mail", "110 x 220 mm"), 220, 110);
    out << mm(QCoreApplication::translate("jp::write::Mail", "114 x 162 mm"), 162, 114);
    out << mm(QCoreApplication::translate("jp::write::Mail", "162 x 229 mm"), 229, 162);
    out << mm(QCoreApplication::translate("jp::write::Mail", "176 x 250 mm"), 250, 176);
    out << mm(QCoreApplication::translate("jp::write::Mail", "229 x 324 mm"), 324, 229);
    return out;
}

QByteArray textRuns(const QString &text)
{
    QByteArray out;
    const QStringList lines = QString(text).replace(QChar::LineSeparator, QLatin1Char('\n')).split(QLatin1Char('\n'));
    for (int i = 0; i < lines.size(); ++i) {
        if (i > 0) out += "<w:r><w:br/></w:r>";
        if (!lines[i].isEmpty()) out += "<w:r><w:t xml:space=\"preserve\">" + esc(lines[i]) + "</w:t></w:r>";
    }
    return out;
}

QByteArray fieldRuns(const QString &code, const QString &result)
{
    return "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> " + esc(code) + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + textRuns(result) +
           "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
}

QByteArray tightParagraph(const QByteArray &runs) { return "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr>" + runs + "</w:p>"; }

std::unique_ptr<WriteDoc> labelDocument(const LabelSheet &s, const std::function<QByteArray(int index)> &cell)
{
    const double gap = s.hPitch - s.labelW;
    QByteArray grid, rows;
    int total = 0;
    for (int c = 0; c < s.columns; ++c) {
        grid += "<w:gridCol w:w=\"" + QByteArray::number(twips(s.labelW)) + "\"/>";
        total += twips(s.labelW);
        if (c + 1 < s.columns && gap > 0.5) {
            grid += "<w:gridCol w:w=\"" + QByteArray::number(twips(gap)) + "\"/>";
            total += twips(gap);
        }
    }
    // (The paragraph that must follow a table needs a little room of its own, or it makes a page of its own: the last row gives some up where the sheet has none to spare.)
    const double room = s.pageH - s.top - s.vPitch * s.rows;
    const double after = 4.0;
    int index = 0;
    for (int r = 0; r < s.rows; ++r) {
        const double height = r + 1 == s.rows ? s.vPitch - std::max(0.0, after - room) : s.vPitch;
        rows += "<w:tr><w:trPr><w:cantSplit/><w:trHeight w:val=\"" + QByteArray::number(twips(height)) + "\" w:hRule=\"exact\"/></w:trPr>";
        for (int c = 0; c < s.columns; ++c) {
            QByteArray content = cell(index++);
            if (content.isEmpty()) content = tightParagraph(QByteArray());
            rows += "<w:tc><w:tcPr><w:tcW w:w=\"" + QByteArray::number(twips(s.labelW)) + "\" w:type=\"dxa\"/></w:tcPr>" + content + "</w:tc>";
            if (c + 1 < s.columns && gap > 0.5) rows += "<w:tc><w:tcPr><w:tcW w:w=\"" + QByteArray::number(twips(gap)) + "\" w:type=\"dxa\"/></w:tcPr>" + tightParagraph(QByteArray()) + "</w:tc>";
        }
        rows += "</w:tr>";
    }
    QByteArray body = "<w:tbl><w:tblPr><w:tblW w:w=\"" + QByteArray::number(total) + "\" w:type=\"dxa\"/><w:tblInd w:w=\"0\" w:type=\"dxa\"/><w:tblLayout w:type=\"fixed\"/>"
                      "<w:tblCellMar><w:top w:w=\"0\" w:type=\"dxa\"/><w:left w:w=\"115\" w:type=\"dxa\"/><w:bottom w:w=\"0\" w:type=\"dxa\"/><w:right w:w=\"115\" w:type=\"dxa\"/></w:tblCellMar></w:tblPr>"
                      "<w:tblGrid>" + grid + "</w:tblGrid>" + rows + "</w:tbl>"
                      "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"20\" w:lineRule=\"exact\"/><w:rPr><w:sz w:val=\"2\"/></w:rPr></w:pPr></w:p>";
    return fromBody(body, [&](office::wml::SectionProps *p) {
        p->pageW = twips(s.pageW);
        p->pageH = twips(s.pageH);
        p->orient = QStringLiteral("portrait");
        p->top = twips(s.top);
        p->left = twips(s.left);
        p->right = std::max(0, twips(s.pageW) - twips(s.left) - total);
        p->bottom = std::max(0, twips(s.pageH) - twips(s.top) - twips(s.vPitch) * s.rows - twips(after));
        p->header = p->footer = 0;
        p->gutter = 0;
    });
}

std::unique_ptr<WriteDoc> envelopeDocument(const EnvelopeSize &size, const QString &returnAddress, const QByteArray &deliveryRuns)
{
    QByteArray body;
    const double leftOfDelivery = size.w * 0.42;
    const double aboveDelivery = std::max(0.0, size.h * 0.42 - (returnAddress.isEmpty() ? 0.0 : (returnAddress.count(QLatin1Char('\n')) + 1) * 13.0));
    if (!returnAddress.isEmpty()) body += tightParagraph(textRuns(returnAddress));
    // The delivery address: in the lower right part of the envelope, set in from the left, with space above it to bring it down.
    body += "<w:p><w:pPr><w:spacing w:before=\"" + QByteArray::number(twips(aboveDelivery)) + "\" w:after=\"0\"/><w:ind w:left=\"" + QByteArray::number(twips(leftOfDelivery)) + "\"/></w:pPr>" + deliveryRuns + "</w:p>";
    return fromBody(body, [&](office::wml::SectionProps *p) {
        p->pageW = twips(size.w);
        p->pageH = twips(size.h);
        p->orient = QStringLiteral("landscape");
        p->top = p->bottom = twips(0.35 * kInch);
        p->left = p->right = twips(0.5 * kInch);
        p->header = p->footer = 0;
    });
}

} // namespace jp::write::mail

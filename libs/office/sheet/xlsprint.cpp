#include "sheet/xlsprint.h"

#include "sheet/xlsbytes.h"

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

namespace {

enum : quint16 {
    kVerticalPageBreaks = 0x001A,
    kHorizontalPageBreaks = 0x001B,
    kHeader = 0x0014,
    kFooter = 0x0015,
    kLeftMargin = 0x0026,
    kRightMargin = 0x0027,
    kTopMargin = 0x0028,
    kBottomMargin = 0x0029,
    kPrintHeaders = 0x002A,
    kPrintGridlines = 0x002B,
    kWsBool = 0x0081,
    kHCenter = 0x0083,
    kVCenter = 0x0084,
    kSetup = 0x00A1,
    kHeaderFooter = 0x089C,
};

// A text of the form the header and footer records and the extra record hold, with its count read already: the characters of BIFF8 begin with a byte that says whether they take one
// byte or two.
QString unicodeChars(Bytes &b, qint64 count)
{
    if (count <= 0) return QString();
    const bool high = b.u8() & 1;
    const qint64 per = high ? 2 : 1;
    const qint64 n = std::min<qint64>(count, b.left() / per);
    if (n <= 0) return QString();
    const char *p = b.take(n * per);
    if (!p) return QString();
    QString s = high ? QString::fromUtf16(reinterpret_cast<const char16_t *>(p), int(n)) : QString::fromLatin1(p, int(n));
    return s.left(kMaxHeaderFooterChars);
}

void margin(Bytes &b, double *into)
{
    const double v = b.f64();
    if (!b.bad() && std::isfinite(v)) *into = std::clamp(v, 0.0, kMaxMarginInches);
}

} // namespace

bool readXlsPrintRecord(quint16 id, const QByteArray &data, qsizetype pos, qsizetype len, bool biff8, const std::function<QString(const char *, int)> &bytesToText, PageSetup *out)
{
    Bytes b(data, pos, pos + len);
    switch (id) {
    case kSetup: {
        const quint16 paper = b.u16(), scale = b.u16(), start = b.u16(), fitW = b.u16(), fitH = b.u16(), grbit = b.u16();
        b.skip(4);
        const double header = b.f64(), footer = b.f64();
        if (b.bad()) return true;
        const bool noPrinter = grbit & 0x0004;
        if (!noPrinter) {
            out->paperSize = paper <= 32767 ? int(paper) : 1;
            out->scale = std::clamp<int>(scale, kMinPageScale, kMaxPageScale);
        }
        if (scale == 0 && !noPrinter) out->scale = 100;
        out->firstPageNumber = std::min<int>(start, 32767);
        out->fitToWidth = std::min<int>(fitW, kMaxFitPages);
        out->fitToHeight = std::min<int>(fitH, kMaxFitPages);
        out->pageOrder = (grbit & 0x0001) ? PageOrder::OverThenDown : PageOrder::DownThenOver;
        out->orientation = (grbit & 0x0040) ? PageOrientation::Default : (grbit & 0x0002) ? PageOrientation::Portrait : PageOrientation::Landscape;
        out->blackAndWhite = grbit & 0x0008;
        out->draft = grbit & 0x0010;
        out->comments = (grbit & 0x0020) ? ((grbit & 0x0200) ? PrintComments::AtEnd : PrintComments::AsDisplayed) : PrintComments::None;
        out->useFirstPageNumber = grbit & 0x0080;
        switch ((grbit >> 10) & 3) {
        case 1: out->errors = PrintErrors::Blank; break;
        case 2: out->errors = PrintErrors::Dash; break;
        case 3: out->errors = PrintErrors::NA; break;
        default: out->errors = PrintErrors::Displayed; break;
        }
        if (std::isfinite(header)) out->margins.header = std::clamp(header, 0.0, kMaxMarginInches);
        if (std::isfinite(footer)) out->margins.footer = std::clamp(footer, 0.0, kMaxMarginInches);
        return true;
    }
    case kHeader:
    case kFooter: {
        QString s;
        if (len > 0) {
            if (biff8) {
                const quint16 cch = b.u16();
                s = unicodeChars(b, cch);
            } else {
                const int n = b.u8();
                const char *p = b.take(std::min<qint64>(n, b.left()));
                if (p) s = bytesToText(p, int(std::min<qint64>(n, len - 1))).left(kMaxHeaderFooterChars);
            }
        }
        (id == kHeader ? out->headerFooter.oddHeader : out->headerFooter.oddFooter) = s;
        return true;
    }
    case kLeftMargin: margin(b, &out->margins.left); return true;
    case kRightMargin: margin(b, &out->margins.right); return true;
    case kTopMargin: margin(b, &out->margins.top); return true;
    case kBottomMargin: margin(b, &out->margins.bottom); return true;
    case kPrintHeaders: {
        const quint16 v = b.u16();
        if (!b.bad()) out->headings = v != 0;
        return true;
    }
    case kPrintGridlines: {
        const quint16 v = b.u16();
        if (!b.bad()) out->gridLines = v != 0;
        return true;
    }
    case kHCenter: {
        const quint16 v = b.u16();
        if (!b.bad()) out->centerHorizontally = v != 0;
        return true;
    }
    case kVCenter: {
        const quint16 v = b.u16();
        if (!b.bad()) out->centerVertically = v != 0;
        return true;
    }
    case kWsBool: {
        const quint16 v = b.u16();
        if (!b.bad()) out->fitToPage = v & 0x0100;
        return true;
    }
    case kHorizontalPageBreaks:
    case kVerticalPageBreaks: {
        const bool rows = id == kHorizontalPageBreaks;
        QVector<PageBreak> &list = rows ? out->rowBreaks : out->colBreaks;
        const quint16 count = b.u16();
        const qsizetype each = biff8 ? 6 : 2;
        for (quint32 i = 0; i < count && b.left() >= each && list.size() < kMaxPageBreaks; ++i) {
            const quint16 at = b.u16();
            quint16 lo = 0, hi = 0;
            if (biff8) {
                lo = b.u16();
                hi = b.u16();
            }
            if (at < 1 || at >= (rows ? kMaxRows : kMaxColumns)) continue;
            bool seen = false;
            for (const PageBreak &e : std::as_const(list)) seen = seen || e.before == at;
            if (seen) continue;
            PageBreak br;
            br.before = at;
            br.min = lo;
            br.max = hi;
            br.manual = true;
            list.append(br);
        }
        return true;
    }
    case kHeaderFooter: {
        if (!biff8) return true;
        const quint16 rt = b.u16();
        b.skip(10 + 16);
        const quint16 grbit = b.u16();
        const quint16 cHeaderEven = b.u16(), cFooterEven = b.u16(), cHeaderFirst = b.u16(), cFooterFirst = b.u16();
        if (b.bad() || rt != kHeaderFooter) return true;
        HeaderFooter &h = out->headerFooter;
        h.differentOddEven = grbit & 0x0001;
        h.differentFirst = grbit & 0x0002;
        h.scaleWithDoc = grbit & 0x0004;
        h.alignWithMargins = grbit & 0x0008;
        h.evenHeader = unicodeChars(b, cHeaderEven);
        h.evenFooter = unicodeChars(b, cFooterEven);
        h.firstHeader = unicodeChars(b, cHeaderFirst);
        h.firstFooter = unicodeChars(b, cFooterFirst);
        return true;
    }
    default: return false;
    }
}

} // namespace jp::office::sheet

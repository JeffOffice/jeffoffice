// An old binary document: the compound file, its streams, and the package it is turned into.

#include "doc/doc.h"
#include "doc/docconvert.h"

#include "io/cfb.h"
#include "office/theme.h"

#include <QCoreApplication>
#include <algorithm>

namespace jp::office::doc {

namespace {

const char kNamespaces[] =
    "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
    "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
    "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";
const char kXmlHeader[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
const char kRel[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";
const char kWml[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.";

QString utf16At(const Span &s, qint64 at, qint64 limitChars)
{
    QString out;
    for (qint64 i = 0; i < limitChars; ++i) {
        const ushort u = s.u16(at + 2 * i);
        if (u == 0) break;
        out += QChar(u);
    }
    return out;
}

} // namespace

bool looksLikeCompoundFile(const QByteArray &bytes) { return bytes.size() >= 512 && bytes.startsWith(QByteArray::fromHex("D0CF11E0A1B11AE1")); }

QStringList readStringTable(const Span &s, int limit)
{
    QStringList out;
    if (s.size() < 4) return out;
    const bool extended = s.u16(0) == 0xFFFF;
    const int count = extended ? s.u16(2) : s.u16(0);
    const int extra = extended ? s.u16(4) : s.u16(2);
    qint64 o = extended ? 6 : 4;
    for (int i = 0; i < count && i < limit && o < s.size(); ++i) {
        if (extended) {
            const int cch = s.u16(o);
            out << utf16At(s, o + 2, std::min(cch, 1024));
            o += 2 + 2 * qint64(cch) + extra;
        } else {
            const int cch = s.u8(o);
            QString t;
            for (int k = 0; k < cch; ++k) t += QChar(s.u8(o + 1 + k));
            out << t;
            o += 1 + cch + extra;
        }
    }
    return out;
}

void Converter::left(const QString &key, const QString &text)
{
    if (m_leftKeys.contains(key)) return;
    m_leftKeys.insert(key);
    if (!notes) return;
    notes->leftOut = true;
    notes->notes << text;
}

Span Converter::tableRange(int slot) const
{
    const Fib::Fc f = fib.at(slot);
    return f.lcb ? tbl.sub(f.fc, f.lcb) : Span();
}

bool Converter::open(const QByteArray &file, QString *error)
{
    auto fail = [&](const QString &m) {
        if (error) *error = m;
        return false;
    };
    if (file.size() > kMaxFileBytes) return fail(QCoreApplication::translate("jp::office::doc", "The file is larger than this program can open."));
    cfb::File cf;
    QString why;
    if (!cfb::read(file, &cf, &why)) return fail(QCoreApplication::translate("jp::office::doc", "This is not a document this program can open: %1").arg(why));
    fileWd = cf.stream(QStringLiteral("WordDocument"));
    if (fileWd.isEmpty()) return fail(QCoreApplication::translate("jp::office::doc", "The file holds no word processing document."));
    wd = Span(fileWd);
    if (!readFib(wd, &fib, &why)) return fail(why);
    if (fib.encrypted) return fail(QCoreApplication::translate("jp::office::doc", "The document is protected with a password, which this program can't open yet."));
    fileTable = cf.stream(fib.whichTable1 ? QStringLiteral("1Table") : QStringLiteral("0Table"));
    if (fileTable.isEmpty()) return fail(QCoreApplication::translate("jp::office::doc", "The document's tables are missing, so its text can't be read."));
    tbl = Span(fileTable);
    fileData = cf.stream(QStringLiteral("Data"));
    data = Span(fileData);
    if (notes && cf.find(QStringLiteral("Macros")) >= 0) notes->hadMacros = true;

    bool cut = false;
    if (!pieces.read(tableRange(Fib::Clx), wd, fib.total() + 2, &cut)) return fail(QCoreApplication::translate("jp::office::doc", "The document's text can't be found in the file."));
    if (cut) left(QStringLiteral("text"), QCoreApplication::translate("jp::office::doc", "Part of the text could not be read, and was left out."));
    // The stories can't hold more characters than the text has.
    {
        qint64 room = pieces.length();
        for (qint64 *cc : {&fib.ccpText, &fib.ccpFtn, &fib.ccpHdd, &fib.ccpAtn, &fib.ccpEdn, &fib.ccpTxbx, &fib.ccpHdrTxbx}) {
            if (*cc > room) {
                *cc = room;
                left(QStringLiteral("text"), QCoreApplication::translate("jp::office::doc", "Part of the text could not be read, and was left out."));
            }
            room -= *cc;
        }
    }
    qint64 budget = kMaxBinRuns;
    chpBin.read(tableRange(Fib::PlcfBteChpx), wd, false, &budget);
    papBin.read(tableRange(Fib::PlcfBtePapx), wd, true, &budget);
    readFontTable();
    styles.read(tableRange(Fib::Stshf));
    // (The levels of the lists follow the list definitions in the stream, past the range the header gives for them.)
    lists = readLists(fib.at(Fib::PlfLst).lcb ? tbl.from(fib.at(Fib::PlfLst).fc) : Span(), fib.at(Fib::PlfLfo).lcb ? tbl.from(fib.at(Fib::PlfLfo).fc) : Span(), fonts, styles);
    authors = readStringTable(tableRange(Fib::SttbfRMark), 1000);
    readSections();
    readNotePlcs();
    if (fib.ccpAtn > 0) left(QStringLiteral("comments"), QCoreApplication::translate("jp::office::doc", "Comments in the document were not brought in."));
    if (fib.ccpTxbx > 0 || fib.ccpHdrTxbx > 0) left(QStringLiteral("shapes"), QCoreApplication::translate("jp::office::doc", "Drawings and text boxes in the document were not brought in."));
    return true;
}

void Converter::readFontTable()
{
    const Span s = tableRange(Fib::SttbfFfn);
    const int count = std::min<int>(s.u16(0), kMaxFonts);
    qint64 o = 4;
    for (int i = 0; i < count; ++i) {
        const int cb = s.u8(o);
        if (cb == 0 || !s.fits(o, cb + 1)) break;
        const Span f = s.sub(o, cb + 1);
        FontNames::Entry e;
        e.prq = f.u8(1) & 3;
        e.family = (f.u8(1) >> 4) & 7;
        e.charset = f.u8(4);
        const int altAt = f.u8(5);
        e.name = utf16At(f, 40, 64);
        if (altAt > 0 && altAt < 64) e.alt = utf16At(f, 40 + 2 * qint64(altAt), 64);
        fonts.entries.push_back(e);
        o += cb + 1;
    }
}

void Converter::readSections()
{
    const Span s = tableRange(Fib::PlcfSed);
    Plc sed(s, 12);
    QVector<Section> all;
    qint64 prev = 0;
    for (int i = 0; i < sed.n && i < kMaxSections; ++i) {
        Section sec;
        sec.index = i;
        sec.end = std::min(sed.cp(i + 1), fib.ccpText);
        if (sec.end <= prev) continue;
        const quint32 fcSepx = sed.record(i).u32(2);
        if (fcSepx != 0xFFFFFFFFu && wd.fits(fcSepx, 2)) sec.sep.apply(wd.sub(fcSepx + 2, wd.u16(fcSepx)));
        all.push_back(sec);
        sectionEnds.push_back(sec.end);
        prev = sec.end;
    }
    // A section ends in a form feed that is the end of a paragraph; where it doesn't (the form feed is a page break inside a paragraph, or the
    // section ends in something else), the break isn't one, and the text goes on in the next section, which has its properties.
    const QString &T = pieces.text();
    for (int i = 0; i < all.size(); ++i) {
        const bool last = i == all.size() - 1;
        bool valid = last;
        if (!last) {
            const qint64 at = all[i].end - 1;
            const Piece *pc = at >= 0 && at < T.size() ? pieces.at(at) : nullptr;
            if (pc && T[at] == QChar(0x0C)) {
                const qint64 fc = PieceTable::fcOf(*pc, at);
                const BinRun *run = papBin.find(quint32(fc));
                valid = run && qint64(run->fc1) == fc + (pc->unicode ? 2 : 1);
            }
        }
        if (valid) sections.push_back(all[i]);
    }
    if (sections.isEmpty()) {
        Section sec;
        sec.end = fib.ccpText;
        sections.push_back(sec);
    }
    // The last section runs to the end of the text.
    sections.last().end = fib.ccpText;
}

void Converter::readNotePlcs()
{
    auto load = [&](int refSlot, int txtSlot, QVector<qint64> *refs, QVector<qint64> *txt) {
        const Plc r(tableRange(refSlot), 2), t(tableRange(txtSlot), 0);
        const int n = std::min({r.n, t.n - 1, kMaxNotes});
        for (int i = 0; i < n; ++i) {
            refs->push_back(r.cp(i));
            txt->push_back(t.cp(i));
        }
        if (n > 0) txt->push_back(t.cp(n));
    };
    load(Fib::PlcffndRef, Fib::PlcffndTxt, &footnoteRefs, &footnoteText);
    load(Fib::PlcfendRef, Fib::PlcfendTxt, &endnoteRefs, &endnoteText);
    // (Looked up by position: the references are in order in a sound file.)
    if (!std::is_sorted(footnoteRefs.begin(), footnoteRefs.end())) {
        footnoteRefs.clear();
        footnoteText.clear();
    }
    if (!std::is_sorted(endnoteRefs.begin(), endnoteRefs.end())) {
        endnoteRefs.clear();
        endnoteText.clear();
    }
}

QByteArray Converter::fontTableXml() const
{
    QByteArray x = QByteArray(kXmlHeader) + "<w:fonts " + kNamespaces + ">";
    static const char *const families[] = {"auto", "roman", "swiss", "modern", "script", "decorative", "auto", "auto"};
    for (const FontNames::Entry &e : fonts.entries) {
        if (e.name.isEmpty()) continue;
        x += "<w:font w:name=\"" + xmlEscape(e.name) + "\">";
        if (!e.alt.isEmpty()) x += "<w:altName w:val=\"" + xmlEscape(e.alt) + "\"/>";
        x += QByteArray("<w:charset w:val=\"") + QByteArray::number(e.charset, 16).rightJustified(2, '0') + "\"/>";
        x += QByteArray("<w:family w:val=\"") + families[e.family & 7] + "\"/>";
        x += e.prq == 1 ? "<w:pitch w:val=\"fixed\"/>" : e.prq == 2 ? "<w:pitch w:val=\"variable\"/>" : "";
        x += "</w:font>";
    }
    x += "</w:fonts>";
    return x;
}

QByteArray Converter::settingsXml() const
{
    const Span dop = tableRange(Fib::Dop);
    QByteArray x = QByteArray(kXmlHeader) + "<w:settings " + kNamespaces + ">";
    if (dop.u8(0) & 1) x += "<w:evenAndOddHeaders/>";
    const int tab = dop.size() >= 12 ? dop.u16(10) : 720;
    x += "<w:defaultTabStop w:val=\"" + QByteArray::number(tab > 0 ? tab : 720) + "\"/>";
    // How the notes are numbered: where the count begins again, and the number it starts at.
    auto noteProps = [&](const char *tag, int bits) {
        const int restart = bits & 3, start = bits >> 2;
        if (restart == 0 && start <= 1) return QByteArray();
        QByteArray p = QByteArray("<w:") + tag + ">";
        if (start > 1) p += "<w:numStart w:val=\"" + QByteArray::number(start) + "\"/>";
        if (restart == 1) p += "<w:numRestart w:val=\"eachSect\"/>";
        else if (restart == 2) p += "<w:numRestart w:val=\"eachPage\"/>";
        p += QByteArray("</w:") + tag + ">";
        return p;
    };
    if (dop.size() >= 54) {
        x += noteProps("footnotePr", dop.u16(2));
        x += noteProps("endnotePr", dop.u16(52));
    }
    // The file says no other mode: that of the 2003 programs.
    x += "<w:compat><w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"11\"/></w:compat></w:settings>";
    return x;
}

QByteArray Converter::build()
{
    const QString mainName = QStringLiteral("/word/document.xml");
    const QString rel = QString::fromLatin1(kRel);
    const QString wml = QString::fromLatin1(kWml);
    pkg.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), mainName);

    // The parts the document reads from.
    pkg.addPart(QStringLiteral("/word/styles.xml"), styles.xml(fonts, lists.instances), wml + QStringLiteral("styles+xml"));
    pkg.addRelationship(mainName, rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    if (!lists.xml.isEmpty()) {
        pkg.addPart(QStringLiteral("/word/numbering.xml"), lists.xml, wml + QStringLiteral("numbering+xml"));
        pkg.addRelationship(mainName, rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
    }
    // (A theme, as the file has none: black text on white, for text with no color of its own.)
    Theme theme = defaultTheme();
    theme.colors[Theme::Dark1] = QColor(0, 0, 0);
    theme.colors[Theme::Light1] = QColor(255, 255, 255);
    theme.colors[Theme::Hyperlink] = QColor(0, 0, 255);
    theme.colors[Theme::FollowedHyperlink] = QColor(128, 0, 128);
    pkg.addPart(QStringLiteral("/word/theme/theme1.xml"), writeTheme(theme), QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));
    pkg.addRelationship(mainName, rel + QStringLiteral("theme"), QStringLiteral("/word/theme/theme1.xml"));
    pkg.addPart(QStringLiteral("/word/settings.xml"), settingsXml(), wml + QStringLiteral("settings+xml"));
    pkg.addRelationship(mainName, rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
    if (!fonts.entries.isEmpty()) {
        pkg.addPart(QStringLiteral("/word/fontTable.xml"), fontTableXml(), wml + QStringLiteral("fontTable+xml"));
        pkg.addRelationship(mainName, rel + QStringLiteral("fontTable"), QStringLiteral("/word/fontTable.xml"));
    }

    // Headers and footers: the six stories of each section, after six for the notes' separators.
    {
        const Plc hdd(tableRange(Fib::PlcfHdd), 0);
        int made = 0;
        for (int si = 0; si < sections.size(); ++si) {
            QByteArray refs;
            const int fileIndex = sections[si].index;
            static const struct { int slot; bool header; const char *type; } kinds[] = {{0, true, "even"}, {1, true, "default"}, {2, false, "even"}, {3, false, "default"}, {4, true, "first"}, {5, false, "first"}};
            for (const auto &k : kinds) {
                const int j = 6 + 6 * fileIndex + k.slot;
                if (j >= hdd.n || made >= kMaxHeaderStories) continue;
                const qint64 a = hdd.cp(j), b = hdd.cp(j + 1);
                if (b <= a || a < 0 || a >= fib.ccpHdd) continue;
                ++made;
                const QString name = QStringLiteral("/word/%1%2.xml").arg(k.header ? "header" : "footer").arg(made);
                // (A header's or footer's text ends with one paragraph mark more than it has paragraphs: the mark that ends the story.)
                qint64 stop = fib.headerStart() + std::min<qint64>(b, fib.ccpHdd);
                if (stop > fib.headerStart() + a && pieces.text().size() >= stop && pieces.text()[stop - 1] == QChar(0x0D)) --stop;
                QByteArray body = story(fib.headerStart() + a, stop, Kind::HeaderFooter, name);
                if (body.isEmpty()) body = "<w:p/>";
                const QByteArray tag = k.header ? "w:hdr" : "w:ftr";
                pkg.addPart(name, QByteArray(kXmlHeader) + "<" + tag + " " + kNamespaces + ">" + body + "</" + tag + ">", wml + (k.header ? QStringLiteral("header+xml") : QStringLiteral("footer+xml")));
                const QString rid = pkg.addRelationship(mainName, rel + (k.header ? QStringLiteral("header") : QStringLiteral("footer")), name);
                refs += QByteArray(k.header ? "<w:headerReference w:type=\"" : "<w:footerReference w:type=\"") + k.type + "\" r:id=\"" + rid.toUtf8() + "\"/>";
            }
            sections[si].xml = "<w:sectPr>" + sectionPropertiesXml(sections[si].sep, refs) + "</w:sectPr>";
        }
    }

    // Notes.
    for (int pass = 0; pass < 2; ++pass) {
        const bool endnotes = pass == 1;
        const QVector<qint64> &refs = endnotes ? endnoteRefs : footnoteRefs, &txt = endnotes ? endnoteText : footnoteText;
        if (refs.isEmpty()) continue;
        const qint64 base = endnotes ? fib.endnoteStart() : fib.footnoteStart(), size = endnotes ? fib.ccpEdn : fib.ccpFtn;
        const QString name = endnotes ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml");
        const QByteArray tag = endnotes ? "w:endnote" : "w:footnote";
        QByteArray x = QByteArray(kXmlHeader) + "<" + tag + "s " + kNamespaces + ">";
        x += "<" + tag + " w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></" + tag + ">";
        x += "<" + tag + " w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></" + tag + ">";
        for (int i = 0; i < refs.size(); ++i) {
            const qint64 a = std::min(txt[i], size), b = std::min(txt[i + 1], size);
            QByteArray body = b > a ? story(base + a, base + b, endnotes ? Kind::Endnote : Kind::Footnote, name) : QByteArray();
            if (body.isEmpty()) body = "<w:p/>";
            x += "<" + tag + " w:id=\"" + QByteArray::number(i + 1) + "\">" + body + "</" + tag + ">";
        }
        x += "</" + tag + "s>";
        pkg.addPart(name, x, wml + (endnotes ? QStringLiteral("endnotes+xml") : QStringLiteral("footnotes+xml")));
        pkg.addRelationship(mainName, rel + (endnotes ? QStringLiteral("endnotes") : QStringLiteral("footnotes")), name);
    }

    // The main text.
    const QByteArray body = story(0, fib.ccpText, Kind::Main, mainName);
    if (sections.last().xml.isEmpty()) sections.last().xml = "<w:sectPr>" + sectionPropertiesXml(sections.last().sep, QByteArray()) + "</w:sectPr>";
    pkg.addPart(mainName, QByteArray(kXmlHeader) + "<w:document " + kNamespaces + "><w:body>" + (body.isEmpty() ? QByteArray("<w:p/>") : body) + sections.last().xml + "</w:body></w:document>",
                wml + QStringLiteral("document.main+xml"));
    return pkg.finish();
}

QByteArray toDocxPackage(const QByteArray &bytes, QString *error, wml::ReadNotes *notes)
{
    Converter c(notes);
    if (!c.open(bytes, error)) return QByteArray();
    return c.build();
}

bool readDoc(const QByteArray &bytes, wml::Document *doc, QString *error, wml::ReadNotes *notes)
{
    wml::ReadNotes local;
    wml::ReadNotes *use = notes ? notes : &local;
    const QByteArray zip = toDocxPackage(bytes, error, use);
    if (zip.isEmpty()) return false;
    const bool macros = use->hadMacros;
    if (!wml::readDocx(zip, doc, error, use)) return false;
    use->hadMacros = use->hadMacros || macros;
    return true;
}

} // namespace jp::office::doc

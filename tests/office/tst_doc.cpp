// Tests of libs/office/doc: reading an old binary word processing file into the document the .docx reader makes. Every file is built in
// memory by dochelpers.h; the hostile ones are that file changed.
#include "dochelpers.h"

#include "doc/doc.h"
#include "io/cfb.h"
#include "wml/wml.h"

#include <QBuffer>
#include <QDir>
#include <QDirIterator>
#include <QImage>
#include <QImageReader>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QXmlStreamReader>
#include <QtTest>

#include "../testrunner.h"

using namespace dochelp;
using namespace jp::office;
using jp::office::wml::Block;
using jp::office::wml::Document;
using jp::office::wml::Inline;
using jp::office::wml::ReadNotes;
using jp::office::Package;

namespace {

// How long a file made to cost a great deal may take to read: two seconds (ten times that where the build is slow, with no optimization or with a sanitizer).
#ifdef NDEBUG
constexpr qint64 kFastMs = 2000;
#else
constexpr qint64 kFastMs = 20000;
#endif

QString paragraphText(const Block &b)
{
    QString t;
    for (const Inline &in : b.para.content)
        if (in.kind == Inline::Text) t += in.text;
    return t;
}

// The text of each paragraph of the body, in order (a table's cells included).
QStringList bodyTexts(const Document &d)
{
    QStringList out;
    for (const Block &b : d.body)
        if (!b.isRaw) out << paragraphText(b);
    return out;
}

bool read(const QByteArray &file, Document *d, ReadNotes *notes = nullptr, QString *error = nullptr)
{
    return doc::readDoc(file, d, error, notes);
}

const wml::KeptPart *part(const Document &d, const QString &name)
{
    for (const wml::KeptPart &p : d.parts)
        if (p.name == name) return &p;
    return nullptr;
}

// The XML of the body of the package a file comes to.
QByteArray bodyXml(const DocWriter &w)
{
    QString why;
    const QByteArray zip = doc::toDocxPackage(w.build(), &why);
    Package pkg;
    if (zip.isEmpty() || !pkg.read(zip)) return QByteArray("no package: ") + why.toUtf8();
    return pkg.part(QStringLiteral("/word/document.xml"));
}

// The sprms of a tracked insertion or deletion by the author at `by` of the file's list, on the date.
QByteArray insertion(int by, quint32 date = 0) { return sprm1(sprm::inserted, 0x81) + sprm2(sprm::insertedBy, by) + (date ? sprm4(sprm::insertedAt, date) : QByteArray()); }
QByteArray deletion(int by, quint32 date = 0) { return sprm1(sprm::deleted, 0x81) + sprm2(sprm::deletedBy, by) + (date ? sprm4(sprm::deletedAt, date) : QByteArray()); }

// Where in a stream a change reaches something the reader reads: the bytes that are not zero (most of a stream's padding and unused places are zeros,
// where a change would do nothing).
QVector<int> liveBytes(const QByteArray &s)
{
    QVector<int> out;
    for (int i = 0; i < s.size(); ++i)
        if (s[i] != 0) out.push_back(i);
    return out;
}

// The bytes of a stream's header that say how long things are and where: the lengths of the stories, and the place and length of every table the header names.
// A change to one of these (to 0xFF in its high byte, say) is a count or a length the reader must not believe.
QVector<int> fibNumbers(const QByteArray &wd)
{
    QVector<int> out;
    for (int at = 64 + 12; at <= 64 + 40; at += 4)   // (the stories' lengths)
        for (int k = 0; k < 4; ++k) out.push_back(at + k);
    for (int slot = 0; slot < 93; ++slot) {
        const int at = 154 + 8 * slot;
        if (at + 8 > wd.size()) break;
        const quint32 lcb = quint32(quint8(wd[at + 4])) | (quint32(quint8(wd[at + 5])) << 8) | (quint32(quint8(wd[at + 6])) << 16) | (quint32(quint8(wd[at + 7])) << 24);
        if (lcb == 0) continue;
        for (int k = 0; k < 8; ++k) out.push_back(at + k);   // (where the table is, and how long)
    }
    return out;
}

// The first and last bytes of every table of the table stream that the header names: a table's count of records, or the first and last of its places.
QVector<int> tableEdges(const QByteArray &wd, const QByteArray &table)
{
    QVector<int> out;
    for (int slot = 0; slot < 93; ++slot) {
        const int at = 154 + 8 * slot;
        if (at + 8 > wd.size()) break;
        auto u32 = [&](int o) { return quint32(quint8(wd[o])) | (quint32(quint8(wd[o + 1])) << 8) | (quint32(quint8(wd[o + 2])) << 16) | (quint32(quint8(wd[o + 3])) << 24); };
        const quint32 fc = u32(at), lcb = u32(at + 4);
        if (lcb == 0 || fc >= quint32(table.size())) continue;
        for (quint32 k = 0; k < 12 && k < lcb && fc + k < quint32(table.size()); ++k) out.push_back(int(fc + k));
        for (quint32 k = 1; k <= 8 && k <= lcb && fc + lcb - k < quint32(table.size()); ++k) out.push_back(int(fc + lcb - k));
    }
    return out;
}

DocWriter simple(const QString &text)
{
    DocWriter w;
    w.text = text;
    return w;
}

} // namespace

class OfficeDocTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ---- the text

    void textOfOnePiece()
    {
        Document d;
        QVERIFY(read(simple(QStringLiteral("Hello world\rSecond paragraph\r")).build(), &d));
        QCOMPARE(bodyTexts(d), (QStringList{"Hello world", "Second paragraph"}));
    }

    void eightBitTextIsTheWesternCodePage()
    {
        // The curly quotes, the dashes, the euro sign, and the rest are bytes 0x80 to 0x9F of the stored text, not their Latin-1 meanings.
        const QString text = QStringLiteral("“quoted” – €5 … ™ šž é ‘x’\r");
        DocWriter w = simple(text);
        const QByteArray file = w.build();
        QVERIFY(file.contains('\x93') && file.contains('\x94') && file.contains('\x96') && file.contains('\x80') && file.contains('\x85'));
        Document d;
        QVERIFY(read(file, &d));
        QCOMPARE(bodyTexts(d), QStringList{text.chopped(1)});
        // (And the same text in two pieces, the first 8-bit and the second 16-bit.)
        DocWriter v = simple(text);
        v.splitAt = 12;
        Document e;
        QVERIFY(read(v.build(), &e));
        QCOMPARE(bodyTexts(e), QStringList{text.chopped(1)});
    }

    void textInTwoPiecesOfDifferentCodings()
    {
        DocWriter w = simple(QStringLiteral("café first\r€ and Ω second\r"));
        w.splitAt = 11;   // after "café first\r"
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), (QStringList{"café first", "€ and Ω second"}));
        // (The same text kept as 16-bit throughout reads the same.)
        DocWriter u = simple(w.text);
        u.unicode = true;
        Document e;
        QVERIFY(read(u.build(), &e));
        QCOMPARE(bodyTexts(e), bodyTexts(d));
    }

    void controlCharactersInText()
    {
        Document d;
        QVERIFY(read(simple(QStringLiteral("a\tb\x0Bz\x0C""c\x1E""d\r")).build(), &d));
        // (A tab, a line break, and a page break stay in the text; a no-break hyphen is its own character.)
        QCOMPARE(bodyTexts(d), (QStringList{QStringLiteral("a\tb z\fc‑d")}));
    }

    void aParagraphMarkInsideAParagraphIsALineBreak()
    {
        // One stretch of paragraph properties covers both marks: the first is a line break, as the original program reads it.
        DocWriter w = simple(QStringLiteral("one\rtwo\rthree\r"));
        w.paras = {{0, 8, 0, QByteArray()}, {8, 14, 0, QByteArray()}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), (QStringList{QStringLiteral("one\u2028two"), QStringLiteral("three")}));
    }

    void aParagraphWithMarksInsideHasThePropertiesItBeginsWith()
    {
        // The stretches of properties don't line up with the marks: one covers "one", the next the mark and a space, the last "two" and its mark.
        DocWriter w = simple(QStringLiteral("one\r two\r"));
        w.paras = {{0, 3, 0, sprm1(sprm::align, 1)}, {3, 5, 0, QByteArray()}, {5, 9, 0, sprm1(sprm::align, 2)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("one\u2028 two")});
        QCOMPARE(d.body[0].para.props.jc, QStringLiteral("center"));
    }

    void everyPartOfThePackageIsWellFormedXml()
    {
        // Names and texts out of a file may hold anything: control characters, quotes, and angle brackets.
        DocWriter w = simple(QStringLiteral("a<b & \"c\" \x01\x02 d\r"));
        w.fonts = {QStringLiteral("Bad \"Font\" <&>\x03"), QStringLiteral("Times New Roman")};
        StyleSpec normal;
        normal.name = QStringLiteral("Normal");
        normal.sti = 0;
        StyleSpec odd;
        odd.name = QStringLiteral("odd <style> \"x\" & \x04\x05");
        w.styles = {normal, odd};
        ListLevelSpec lvl;
        lvl.text = QString(QChar(0)) + QStringLiteral("\x0B\x0C<\"&");
        w.lists = {{lvl}};
        w.paras = {{0, 20, 1, sprm2(sprm::listInstance, 1)}};
        w.chars = {{0, 2, sprm2(sprm::fontAscii, 0)}};
        QString why;
        const QByteArray zip = doc::toDocxPackage(w.build(), &why);
        QVERIFY2(!zip.isEmpty(), qPrintable(why));
        Package pkg;
        QVERIFY(pkg.read(zip));
        int parts = 0;
        for (const QString &name : pkg.partNames()) {
            if (!(name.endsWith(QLatin1String(".xml")) || name.endsWith(QLatin1String(".rels")))) continue;
            QXmlStreamReader r(pkg.part(name));
            while (!r.atEnd()) r.readNext();
            QVERIFY2(!r.hasError(), qPrintable(name + QLatin1String(": ") + r.errorString()));
            ++parts;
        }
        QVERIFY(parts >= 5);
        Document d;
        QVERIFY(read(w.build(), &d));
    }

    void compatibilityModeIsThatOfTheOlderPrograms()
    {
        Document d;
        QVERIFY(read(simple(QStringLiteral("x\r")).build(), &d));
        QCOMPARE(d.compatMode, 11);
    }

    // ---- properties

    void characterProperties()
    {
        DocWriter w = simple(QStringLiteral("plain bold both\r"));
        w.fonts = {QStringLiteral("Times New Roman"), QStringLiteral("Arial")};
        w.chars = {{6, 10, sprm1(sprm::bold, 1)}, {11, 15, sprm1(sprm::bold, 1) + sprm1(sprm::italic, 1) + sprm2(sprm::size, 28) + sprm2(sprm::fontAscii, 1) + sprm2(sprm::fontOther, 1)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        const Block &p = d.body.first();
        QCOMPARE(paragraphText(p), QStringLiteral("plain bold both"));
        QVector<Inline> runs;
        for (const Inline &in : p.para.content)
            if (in.kind == Inline::Text) runs << in;
        QCOMPARE(runs.size(), 4);
        QVERIFY(!runs[0].props.bold.value_or(false));
        QCOMPARE(runs[1].text, QStringLiteral("bold"));
        QVERIFY(runs[1].props.bold.value_or(false));
        QVERIFY(!runs[2].props.bold.value_or(false));
        QCOMPARE(runs[3].text, QStringLiteral("both"));
        QVERIFY(runs[3].props.bold.value_or(false) && runs[3].props.italic.value_or(false));
        QCOMPARE(runs[3].props.size.value_or(0), 28);
        QVERIFY(runs[3].props.fonts && runs[3].props.fonts->ascii == QLatin1String("Arial"));
    }

    void trackedChangesHaveTheirAuthorsAndDates()
    {
        DocWriter w = simple(QStringLiteral("a new b old c\r"));
        w.revisionAuthors = {QStringLiteral("Ann"), QStringLiteral("Bob")};
        w.chars = {{2, 5, insertion(0, dttm(2003, 4, 7, 9, 30))}, {8, 11, deletion(1, dttm(2004, 11, 23, 16, 5))}};
        const QByteArray x = bodyXml(w);
        QVERIFY2(x.contains("<w:ins w:id=\"1\" w:author=\"Ann\" w:date=\"2003-04-07T09:30:00Z\"><w:r><w:t xml:space=\"preserve\">new</w:t></w:r></w:ins>"), x.constData());
        QVERIFY2(x.contains("<w:del w:id=\"2\" w:author=\"Bob\" w:date=\"2004-11-23T16:05:00Z\"><w:r><w:delText xml:space=\"preserve\">old</w:delText></w:r></w:del>"), x.constData());
        // A date the file leaves out, or one that is no date (month 0), is left out of the mark.
        DocWriter v = simple(QStringLiteral("a new b old c\r"));
        v.revisionAuthors = {QStringLiteral("Ann")};
        v.chars = {{2, 5, insertion(0)}, {8, 11, deletion(0, dttm(2003, 0, 7, 9, 30))}};
        const QByteArray y = bodyXml(v);
        QVERIFY2(y.contains("<w:ins w:id=\"1\" w:author=\"Ann\"><w:r>"), y.constData());
        QVERIFY2(y.contains("<w:del w:id=\"2\" w:author=\"Ann\"><w:r>"), y.constData());
    }

    void aParagraphMarkThatWasInsertedOrDeletedSaysSo()
    {
        DocWriter w = simple(QStringLiteral("first\rsecond\r"));
        w.revisionAuthors = {QStringLiteral("Ann")};
        w.chars = {{5, 6, insertion(0, dttm(2003, 4, 7, 9, 30))}, {12, 13, deletion(0, dttm(2004, 11, 23, 16, 5))}};
        const QByteArray x = bodyXml(w);
        QVERIFY2(x.contains("<w:pPr><w:rPr><w:ins w:id=\"1\" w:author=\"Ann\" w:date=\"2003-04-07T09:30:00Z\"/></w:rPr></w:pPr>"), x.constData());
        QVERIFY2(x.contains("<w:pPr><w:rPr><w:del w:id=\"2\" w:author=\"Ann\" w:date=\"2004-11-23T16:05:00Z\"/></w:rPr></w:pPr>"), x.constData());
        // (The words of the paragraphs are not marked: only their marks are.)
        QVERIFY(!x.contains("<w:ins w:id=\"1\" w:author=\"Ann\" w:date=\"2003-04-07T09:30:00Z\"><w:r>"));
        // And the document reads them: the mark's properties are kept with the paragraph.
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), (QStringList{QStringLiteral("first"), QStringLiteral("second")}));
        QVERIFY(d.body[0].para.props.markRun.has_value());
    }

    void plainCharactersKeepTheirTrackedChange()
    {
        // "Plain" takes the formatting back to the style's; it is no reason to forget who typed the words.
        DocWriter w = simple(QStringLiteral("a new b\r"));
        w.revisionAuthors = {QStringLiteral("Ann")};
        w.chars = {{2, 5, insertion(0, dttm(2003, 4, 7, 9, 30)) + sprm1(0x2A33, 1) + sprm1(sprm::bold, 1)}};
        const QByteArray x = bodyXml(w);
        QVERIFY2(x.contains("<w:ins w:id=\"1\" w:author=\"Ann\" w:date=\"2003-04-07T09:30:00Z\"><w:r><w:rPr><w:b/></w:rPr><w:t xml:space=\"preserve\">new</w:t></w:r></w:ins>"), x.constData());
    }

    void textInsertedAndThenDeletedKeepsBothMarks()
    {
        DocWriter w = simple(QStringLiteral("a new b\r"));
        w.revisionAuthors = {QStringLiteral("Ann"), QStringLiteral("Bob")};
        w.chars = {{2, 5, insertion(0, dttm(2003, 4, 7, 9, 30)) + deletion(1, dttm(2004, 11, 23, 16, 5))}};
        const QByteArray x = bodyXml(w);
        QVERIFY2(x.contains("<w:ins w:id=\"1\" w:author=\"Ann\" w:date=\"2003-04-07T09:30:00Z\"><w:del w:id=\"2\" w:author=\"Bob\" w:date=\"2004-11-23T16:05:00Z\">"
                            "<w:r><w:delText xml:space=\"preserve\">new</w:delText></w:r></w:del></w:ins>"),
                 x.constData());
    }

    void toggleRelativeToTheStyle()
    {
        // A style that is bold; a run stated "the opposite of the style" is not bold.
        DocWriter w = simple(QStringLiteral("abc\r"));
        StyleSpec normal;
        normal.name = QStringLiteral("Normal");
        normal.sti = 0;
        StyleSpec head;
        head.name = QStringLiteral("heading 1");
        head.sti = 1;
        head.base = 0;
        head.character = sprm1(sprm::bold, 1);
        w.styles = {normal, head};
        w.paras = {{0, 4, 1, QByteArray()}};
        w.chars = {{1, 2, sprm1(sprm::bold, 0x81)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        const Block &p = d.body.first();
        QCOMPARE(p.para.props.style, QStringLiteral("Heading1"));
        QVector<Inline> runs;
        for (const Inline &in : p.para.content)
            if (in.kind == Inline::Text) runs << in;
        QCOMPARE(runs.size(), 3);
        QVERIFY(!runs[0].props.bold.has_value());          // (as the style has it)
        QCOMPARE(runs[1].props.bold.value_or(true), false);   // (the opposite)
    }

    void paragraphProperties()
    {
        DocWriter w = simple(QStringLiteral("centered\rindented\r"));
        w.paras = {{0, 9, 0, sprm1(sprm::align, 1) + sprm2(sprm::before, 120) + sprm2(sprm::after, 240)},
                   {9, 18, 0, sprm2(sprm::left, 720) + sprm2(sprm::firstLine, 0xFE98 /* -360 */) + sprm1(sprm::keepNext, 1)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        const wml::ParaProps &a = d.body[0].para.props, &b = d.body[1].para.props;
        QCOMPARE(a.jc, QStringLiteral("center"));
        QCOMPARE(a.before.value_or(-1), 120);
        QCOMPARE(a.after.value_or(-1), 240);
        QCOMPARE(b.indLeft.value_or(-1), 720);
        QCOMPARE(b.indFirst.value_or(0), -360);
        QVERIFY(b.keepNext.value_or(false));
    }

    void stylesInheritFromTheirBase()
    {
        DocWriter w = simple(QStringLiteral("Title text\r"));
        StyleSpec normal;
        normal.name = QStringLiteral("Normal");
        normal.sti = 0;
        normal.character = sprm2(sprm::size, 24);
        StyleSpec head;
        head.name = QStringLiteral("heading 1");
        head.sti = 1;
        head.base = 0;
        head.paragraph = sprm1(sprm::align, 1);
        head.character = sprm1(sprm::bold, 1);
        StyleSpec mine;
        mine.name = QStringLiteral("My Style");
        mine.base = 1;
        mine.paragraph = sprm2(sprm::left, 360);
        w.styles = {normal, head, mine};
        w.paras = {{0, 11, 2, QByteArray()}};
        Document d;
        QVERIFY(read(w.build(), &d));
        const wml::Style *s = d.styles.find(QStringLiteral("MyStyle"));
        QVERIFY(s);
        QCOMPARE(s->basedOn, QStringLiteral("Heading1"));
        const wml::ParaProps resolved = d.styles.paragraph(QStringLiteral("MyStyle"));
        QCOMPARE(resolved.jc, QStringLiteral("center"));   // from the style it is based on
        QCOMPARE(resolved.indLeft.value_or(-1), 360);      // its own
        QCOMPARE(d.styles.run(QStringLiteral("MyStyle"), QString()).bold.value_or(false), true);
        QCOMPARE(d.styles.run(QStringLiteral("MyStyle"), QString()).size.value_or(0), 24);   // from Normal, two styles up
        QCOMPARE(d.body.first().para.props.style, QStringLiteral("MyStyle"));
    }

    void aStyleThatIsItsOwnAncestorIsRead()
    {
        DocWriter w = simple(QStringLiteral("x\r"));
        StyleSpec a;
        a.name = QStringLiteral("Normal");
        a.sti = 0;
        a.base = 1;
        StyleSpec b;
        b.name = QStringLiteral("Other");
        b.base = 0;
        b.character = sprm1(sprm::bold, 0x81);
        w.styles = {a, b};
        w.paras = {{0, 2, 1, QByteArray()}};
        Document d;
        QElapsedTimer t;
        t.start();
        QVERIFY(read(w.build(), &d));
        QVERIFY(t.elapsed() < 5000);
        QCOMPARE(bodyTexts(d), QStringList{"x"});
    }

    // ---- lists

    void numberedAndBulletedLists()
    {
        DocWriter w = simple(QStringLiteral("one\rtwo\rbullet\r"));
        ListLevelSpec dec;
        dec.format = 0;
        dec.text = QString(QChar(0)) + QLatin1Char('.');
        dec.paragraph = sprm2(sprm::left, 720) + sprm2(sprm::firstLine, 0xFE98);
        ListLevelSpec bul;
        bul.format = 23;
        bul.text = QString(QChar(0xF0B7));
        w.lists = {{dec}, {bul}};
        w.paras = {{0, 4, 0, sprm2(sprm::listInstance, 1) + sprm1(sprm::listLevel, 0)}, {4, 8, 0, sprm2(sprm::listInstance, 1)}, {8, 15, 0, sprm2(sprm::listInstance, 2)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(d.body[0].para.props.numId.value_or(-1), 1);
        QCOMPARE(d.body[1].para.props.numId.value_or(-1), 1);
        QCOMPARE(d.body[2].para.props.numId.value_or(-1), 2);
        const wml::AbstractNum *first = d.numbering.abstractOf(1), *second = d.numbering.abstractOf(2);
        QVERIFY(first && second);
        QCOMPARE(first->levels[0].numFmt, QStringLiteral("decimal"));
        QCOMPARE(first->levels[0].text, QStringLiteral("%1."));
        QCOMPARE(first->levels[0].pPr.indLeft.value_or(-1), 720);
        QCOMPARE(first->levels[0].pPr.indFirst.value_or(0), -360);
        QCOMPARE(second->levels[0].numFmt, QStringLiteral("bullet"));
        QCOMPARE(second->levels[0].text, QString(QChar(0xF0B7)));
    }

    void aParagraphInAListThatIsNotThereIsNotInOne()
    {
        DocWriter w = simple(QStringLiteral("one\r"));
        w.paras = {{0, 4, 0, sprm2(sprm::listInstance, 9)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(!d.body[0].para.props.numId.has_value() || *d.body[0].para.props.numId == 0);
    }

    // ---- sections, headers, and footers

    void sectionsAndPageSetup()
    {
        // Two sections: the first ends in a form feed of its own paragraph; the second is landscape.
        DocWriter w = simple(QStringLiteral("first\r\fsecond\r"));
        const QByteArray one = sprm2(sprm::pageWidth, 12240) + sprm2(sprm::pageHeight, 15840) + sprm2(sprm::marginLeft, 1800) + sprm2(sprm::marginRight, 1800) + sprm2(sprm::marginTop, 1440) + sprm2(sprm::marginBottom, 1440);
        const QByteArray two = sprm1(sprm::landscape, 2) + sprm2(sprm::pageWidth, 15840) + sprm2(sprm::pageHeight, 12240) + sprm2(sprm::columns, 1) + sprm2(sprm::columnSpace, 360) + sprm1(sprm::sectionType, 0);
        w.sections = {{7, one}, {int(w.text.size()) + 1, two}};
        w.paras = {{0, 6, 0, QByteArray()}, {6, 7, 0, QByteArray()}, {7, int(w.text.size()), 0, QByteArray()}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(d.body[1].para.props.section);   // the form feed's paragraph ends the first section
        QCOMPARE(d.body[1].para.props.section->pageW, 12240);
        QCOMPARE(d.body[1].para.props.section->left, 1800);
        QVERIFY(d.lastSectionStated);
        QCOMPARE(d.lastSection.pageW, 15840);
        QCOMPARE(d.lastSection.pageH, 12240);
        QCOMPARE(d.lastSection.orient, QStringLiteral("landscape"));
        QCOMPARE(d.lastSection.cols, 2);
        QCOMPARE(d.lastSection.colSpace, 360);
        QCOMPARE(d.lastSection.type, QStringLiteral("continuous"));
    }

    void aFormFeedInAParagraphIsAPageBreak()
    {
        Document d;
        QVERIFY(read(simple(QStringLiteral("before\fafter\r")).build(), &d));
        QCOMPARE(d.body.size(), qsizetype(1));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("before\fafter")});
        QVERIFY(!d.body[0].para.props.section);
    }

    void headersAndFootersOfASection()
    {
        // Six stories for the notes' separators, then six for the section: even header, odd header, even footer, odd footer, first header, first footer.
        DocWriter w = simple(QStringLiteral("body\r"));
        w.headerStories = {QString(), QString(), QString(), QString(), QString(), QString(),
                           QString(), QStringLiteral("Header text\r"), QString(), QStringLiteral("Footer text\r"), QStringLiteral("First page\r"), QString()};
        w.sections = {{5, sprm1(sprm::titlePage, 1)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(d.lastSectionStated);
        QVERIFY(d.lastSection.titlePg);
        const wml::HeaderRefs refs = wml::headerRefsOf(d.lastSection);
        QVERIFY(refs.any());
        QVERIFY(refs.header[0].isEmpty() == false);
        QVERIFY(refs.footer[0].isEmpty() == false);
        QVERIFY(refs.header[1].isEmpty() == false);
        QVERIFY(refs.footer[1].isEmpty());       // (no first-page footer)
        QVERIFY(refs.header[2].isEmpty());       // (no even header)
        auto text = [&](const QString &rid) {
            for (const auto &r : d.mainRelationships)
                if (r.id == rid)
                    if (const wml::KeptPart *p = part(d, r.target)) {
                        QVector<Block> blocks;
                        wml::readBlocks(p->data, &blocks);
                        QStringList t;
                        for (const Block &b : blocks) t << paragraphText(b);
                        return t.join(QLatin1Char('|'));
                    }
            return QString();
        };
        // (Each story ends in one mark more than it has paragraphs: that one is not a paragraph.)
        QCOMPARE(text(refs.header[0]), QStringLiteral("Header text"));
        QCOMPARE(text(refs.footer[0]), QStringLiteral("Footer text"));
        QCOMPARE(text(refs.header[1]), QStringLiteral("First page"));
    }

    void evenAndOddHeadersAreSaid()
    {
        DocWriter w = simple(QStringLiteral("x\r"));
        w.evenAndOddHeaders = true;
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(d.evenAndOddHeaders);
        QCOMPARE(d.defaultTabStop, 720);
        DocWriter v = simple(QStringLiteral("x\r"));
        v.defaultTab = 360;
        Document e;
        QVERIFY(read(v.build(), &e));
        QVERIFY(!e.evenAndOddHeaders);
        QCOMPARE(e.defaultTabStop, 360);
    }

    // ---- notes

    void footnotes()
    {
        DocWriter w = simple(QStringLiteral("Text with a note.\r"));
        w.chars = {{16, 17, sprm1(sprm::special, 1)}};
        w.text = QStringLiteral("Text with a note\x02.\r");
        w.chars = {{16, 17, sprm1(sprm::special, 1)}};
        w.footnotes = {QStringLiteral("\x02 The note itself.\r")};
        w.footnoteRefs = {16};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("Text with a note.")});
        bool cited = false;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("footnoteReference")) cited = true;
        QVERIFY(cited);
        const wml::KeptPart *notes = part(d, QStringLiteral("/word/footnotes.xml"));
        QVERIFY(notes);
        QVector<wml::NotePart> list;
        QVERIFY(wml::readNotes(notes->data, &list));
        QString found;
        for (const wml::NotePart &n : list)
            if (n.type.isEmpty())
                for (const Block &b : n.blocks) found += paragraphText(b);
        QCOMPARE(found.trimmed(), QStringLiteral("The note itself."));
    }

    // ---- bookmarks

    void bookmarksAreWhereTheFileSays()
    {
        DocWriter w = simple(QStringLiteral("Hello world\rSecond\r"));
        w.bookmarks = {{QStringLiteral("Greeting"), 0, 5}, {QStringLiteral("Second_one"), 12, 18}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), (QStringList{"Hello world", "Second"}));
        auto marks = [&](int paragraph) {
            QStringList out;
            QString text;
            for (const Inline &in : d.body[paragraph].para.content) {
                if (in.kind == Inline::Text) text += in.text;
                else if (in.kind == Inline::ParaAtom && (in.tag == QLatin1String("bookmarkStart") || in.tag == QLatin1String("bookmarkEnd")))
                    out << QStringLiteral("%1@%2").arg(in.tag.mid(8), QString::number(text.size()));
            }
            return out;
        };
        QCOMPARE(marks(0), (QStringList{"Start@0", "End@5"}));
        QCOMPARE(marks(1), (QStringList{"Start@0", "End@6"}));
        QVERIFY(QString::fromUtf8(d.body[0].para.content.first().raw).contains(QLatin1String("Greeting")));
    }

    // ---- fields

    void aFieldKeepsItsInstructionAndShowsItsResult()
    {
        const QString text = QStringLiteral("Page \x13 PAGE \x14""7\x15 of \x13 NUMPAGES \x14""9\x15\r");
        Document d;
        QVERIFY(read(simple(text).build(), &d));
        // The result is text; the field's marks and instruction are kept (so PAGE and NUMPAGES stay live).
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("Page 7 of 9")});
        QStringList instructions;
        int marks = 0;
        for (const Inline &in : d.body[0].para.content) {
            if (in.kind != Inline::RunAtom) continue;
            if (in.tag == QLatin1String("fldChar")) ++marks;
            if (in.tag == QLatin1String("instrText")) instructions << QString::fromUtf8(in.raw);
        }
        QCOMPARE(marks, 6);
        QCOMPARE(instructions.size(), 2);
        QVERIFY(instructions[0].contains(QLatin1String("PAGE")));
        QVERIFY(instructions[1].contains(QLatin1String("NUMPAGES")));
    }

    void fieldsThatNestTooDeepAreStillBalanced()
    {
        QString text = QStringLiteral("a");
        const int depth = doc::kMaxFieldDepth + 30;
        for (int i = 0; i < depth; ++i) text += QStringLiteral(" \x13 X \x14");
        text += QStringLiteral("deep");
        for (int i = 0; i < depth; ++i) text += QLatin1Char(0x15);
        text += QStringLiteral("\r");
        Document d;
        QVERIFY(read(simple(text).build(), &d));
        int begins = 0, ends = 0;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("fldChar")) {
                if (in.key == QLatin1String("begin")) ++begins;
                if (in.key == QLatin1String("end")) ++ends;
            }
        QCOMPARE(begins, ends);
        QVERIFY(begins <= doc::kMaxFieldDepth);
        QVERIFY(paragraphText(d.body[0]).contains(QLatin1String("deep")));
    }

    void fieldsOpenedByTheHundredThousandAreBalancedAndCostLittle()
    {
        // One mark opens a field, and the fields of a file can be nested in as many as it has marks (a state kept for each was a state kept for 40 million).
        const int depth = 200000;
        QString text = QStringLiteral("start");
        text += QString(depth, QChar(0x13));
        text += QString(depth, QChar(0x15));
        text += QStringLiteral(" finish\r");
        QElapsedTimer t;
        t.start();
        Document d;
        QVERIFY(read(simple(text).build(), &d));
        QVERIFY2(t.elapsed() < kFastMs, qPrintable(QStringLiteral("took %1 ms").arg(t.elapsed())));
        int begins = 0, ends = 0;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("fldChar")) {
                if (in.key == QLatin1String("begin")) ++begins;
                if (in.key == QLatin1String("end")) ++ends;
            }
        QCOMPARE(begins, ends);
        QVERIFY(begins <= doc::kMaxFieldDepth);
        // (Every field is closed again, so what follows them is text in the paragraph.)
        QCOMPARE(paragraphText(d.body[0]), QStringLiteral("start finish"));
    }

    void aFieldLeftOpenIsClosed()
    {
        Document d;
        QVERIFY(read(simple(QStringLiteral("open \x13 PAGE \x14""3\r")).build(), &d));
        int begins = 0, ends = 0;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("fldChar")) {
                if (in.key == QLatin1String("begin")) ++begins;
                if (in.key == QLatin1String("end")) ++ends;
            }
        QCOMPARE(begins, 1);
        QCOMPARE(ends, 1);
    }

    // ---- tables

    void aTableWithRowsAndCells()
    {
        // Two rows of two cells: a cell's last paragraph ends in \a, a row ends in a \a paragraph that says where the cells are.
        const QString text = QStringLiteral("a1\ab1\a\ra2\ab2\a\rafter\r");
        DocWriter w = simple(text);
        const QByteArray in = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, 1);
        const QByteArray row = in + sprm1(sprm::endsRow, 1) + tableRow({0, 2000, 5000});
        w.paras = {{0, 3, 0, in}, {3, 6, 0, in}, {6, 7, 0, row}, {7, 10, 0, in}, {10, 13, 0, in}, {13, 14, 0, row}, {14, 20, 0, QByteArray()}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QStringList cells;
        int rows = 0, tables = 0;
        QVector<int> grid;
        for (const Block &b : d.body) {
            if (b.isRaw && b.tag == QLatin1String("tbl") && b.role == Inline::Open) {
                ++tables;
                if (b.table) grid = b.table->grid;
            }
            if (b.isRaw && b.tag == QLatin1String("tr") && b.role == Inline::Open) ++rows;
            if (!b.isRaw) cells << paragraphText(b);
        }
        QCOMPARE(tables, 1);
        QCOMPARE(rows, 2);
        QCOMPARE(cells, (QStringList{"a1", "b1", "a2", "b2", "after"}));
        QCOMPARE(grid, (QVector<int>{2000, 3000}));
    }

    void tablesNestedTooDeepArePlainParagraphs()
    {
        // Nested tables to twice the limit: nothing is lost, and nothing nests past the limit.
        const int depth = doc::kMaxTableDepth * 2;
        QString text;
        DocWriter w;
        int at = 0;
        for (int level = 1; level <= depth; ++level) {
            const QByteArray in = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, quint32(level));
            text += QStringLiteral("c%1\a").arg(level);
            w.paras.push_back({at, int(text.size()), 0, in});
            at = int(text.size());
        }
        for (int level = depth; level >= 1; --level) {
            const QByteArray row = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, quint32(level)) + sprm1(sprm::endsRow, 1) + tableRow({0, 3000});
            text += QStringLiteral("\a");
            w.paras.push_back({at, int(text.size()), 0, row});
            at = int(text.size());
        }
        text += QStringLiteral("end\r");
        w.text = text;
        w.paras.push_back({at, int(text.size()), 0, QByteArray()});
        Document d;
        ReadNotes notes;
        QVERIFY(read(w.build(), &d, &notes));
        int open = 0, deepest = 0;
        for (const Block &b : d.body) {
            if (b.isRaw && b.tag == QLatin1String("tbl")) {
                open += b.role == Inline::Open ? 1 : -1;
                deepest = std::max(deepest, open);
            }
        }
        QVERIFY(deepest <= doc::kMaxTableDepth);
        QVERIFY(bodyTexts(d).join(QLatin1Char(' ')).contains(QLatin1String("end")));
        QVERIFY(bodyTexts(d).join(QLatin1Char(' ')).contains(QStringLiteral("c%1").arg(depth)));
    }

    // ---- pictures

    void aPictureInTheText()
    {
        DocWriter w = simple(QStringLiteral("a\x01""b\r"));
        const int at = w.addPicture(tinyPng(8, 6), 2880, 1440);   // two inches by one
        w.chars = {{1, 2, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(at))}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("ab")});
        const Inline *drawing = nullptr;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("drawing")) drawing = &in;
        QVERIFY(drawing);
        const QVector<wml::Shape> shapes = wml::readShapes(drawing->raw);
        QCOMPARE(shapes.size(), qsizetype(1));
        QCOMPARE(shapes[0].kind, wml::Shape::Picture);
        QVERIFY(!shapes[0].floating);
        QCOMPARE(shapes[0].width, 144.0);
        QCOMPARE(shapes[0].height, 72.0);
        QVERIFY(!shapes[0].embed.isEmpty());
        bool media = false;
        for (const wml::KeptPart &p : d.parts)
            if (p.name.startsWith(QLatin1String("/word/media/")) && p.data.startsWith(QByteArray("\x89PNG"))) media = true;
        QVERIFY(media);
    }

    void aPictureWithNoSizeTakesItsSizeFromItsHeader()
    {
        // A picture the file gives no size for is shown at its own. Its header says what that is; the whole image was decoded to find out, which for a small
        // file that makes a very large image (and a picture for each of thousands of places in it) is all the memory and time there are. With the allowance
        // for decoding turned down to one megabyte this 2000 by 2000 picture can't be decoded, and still has its size.
        QImage image(2000, 2000, QImage::Format_Grayscale8);
        image.fill(200);
        QByteArray png;
        {
            QBuffer buffer(&png);
            QVERIFY(buffer.open(QIODevice::WriteOnly));
            QVERIFY(image.save(&buffer, "PNG"));
        }
        struct Allowance {
            int before = QImageReader::allocationLimit();
            Allowance() { QImageReader::setAllocationLimit(1); }
            ~Allowance() { QImageReader::setAllocationLimit(before); }
        } allowance;
        DocWriter w = simple(QStringLiteral("a\x01""b\r"));
        const int at = w.addPicture(png, 0, 0);
        w.chars = {{1, 2, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(at))}};
        const QByteArray x = bodyXml(w);
        // (2000 pixels at 96 to the inch: 1500 points, in the 12,700ths of a point the package counts in.)
        QVERIFY2(x.contains("<wp:extent cx=\"19050000\" cy=\"19050000\"/>"), x.constData());
    }

    void aRuleInTheLineIsAShapeWithNoPicture()
    {
        DocWriter w = simple(QStringLiteral("a\x01""b\r"));
        const int at = w.addRule(30);   // one and a half points high
        w.chars = {{1, 2, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(at))}};
        Document d;
        ReadNotes notes;
        QVERIFY(read(w.build(), &d, &notes));
        QVERIFY(!notes.leftOut);
        const Inline *pict = nullptr;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("pict")) pict = &in;
        QVERIFY(pict);
        QVERIFY(QString::fromUtf8(pict->raw).contains(QLatin1String("o:hr=\"t\"")));
        const QVector<wml::Shape> shapes = wml::readShapes(pict->raw);
        QCOMPARE(shapes.size(), qsizetype(1));
        QCOMPARE(shapes[0].kind, wml::Shape::Rect);
        QVERIFY(!shapes[0].floating);
        QCOMPARE(shapes[0].height, 1.5);
        QVERIFY(shapes[0].filled);
        QCOMPARE(shapes[0].fillColor, QStringLiteral("808080"));
    }

    void thePageNumberRestartsOnlyWhereTheSectionSaysSo()
    {
        const QByteArray start = sprm2(0x501C, 5);   // (the number to start at, as a section states it with or without the restart)
        DocWriter w = simple(QStringLiteral("x\r"));
        w.sections = {{int(w.text.size()) + 1, start}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(!d.lastSection.pageStart.has_value());
        DocWriter r = simple(QStringLiteral("x\r"));
        r.sections = {{int(r.text.size()) + 1, start + sprm1(0x3011, 1)}};
        Document e;
        QVERIFY(read(r.build(), &e));
        QCOMPARE(e.lastSection.pageStart.value_or(-1), 5);
    }

    void notesAreNumberedAsTheDocumentSays()
    {
        DocWriter w = simple(QStringLiteral("x\r"));
        w.footnoteFormat = 4;
        Document d;
        QVERIFY(read(w.build(), &d));
        bool found = false;
        for (const wml::RawChild &c : d.lastSection.extra)
            if (c.xml.contains("footnotePr") && c.xml.contains("lowerLetter")) found = true;
        QVERIFY(found);
    }

    void aPictureThatIsNotThereIsLeftOut()
    {
        DocWriter w = simple(QStringLiteral("a\x01""b\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1) + sprm4(sprm::picture, 5000)}};
        Document d;
        ReadNotes notes;
        QVERIFY(read(w.build(), &d, &notes));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("ab")});
        QVERIFY(notes.leftOut);
        QVERIFY(!notes.notes.isEmpty());
    }

    // ---- drawings

    void aTextBoxIsSetWhereTheFileSaysAndHoldsItsText()
    {
        DocWriter w = simple(QStringLiteral("Before\x08 after\r"));
        w.chars = {{6, 7, sprm1(sprm::special, 1)}};
        ShapeSpec box;
        box.cp = 6;
        box.x0 = 720; box.y0 = 360; box.x1 = 4320; box.y1 = 1440;
        box.bx = 1; box.by = 0; box.wr = 2;
        box.text = QStringLiteral("Boxed words\r");
        w.shapes = {box};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("Before after")});
        const Inline *pict = nullptr;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("pict")) pict = &in;
        QVERIFY(pict);
        const QVector<wml::Shape> shapes = wml::readShapes(pict->raw);
        QCOMPARE(shapes.size(), qsizetype(1));
        const wml::Shape &s = shapes[0];
        QCOMPARE(s.kind, wml::Shape::TextBox);
        QVERIFY(s.floating);
        QCOMPARE(s.left, 36.0);
        QCOMPARE(s.top, 18.0);
        QCOMPARE(s.width, 180.0);
        QCOMPARE(s.height, 54.0);
        QCOMPARE(s.hRel, wml::Shape::Page);
        QCOMPARE(s.vRel, wml::Shape::Margin);
        QCOMPARE(s.wrap, wml::Shape::Square);
        QVERIFY(!s.behind);
        QVERIFY(QString::fromUtf8(s.textBox).contains(QLatin1String("Boxed words")));
        // (The mark that ends the box's text is not a paragraph of it.)
        QCOMPARE(QString::fromUtf8(s.textBox).count(QLatin1String("<w:p>")), 1);
    }

    void aPictureShapeBehindTheText()
    {
        DocWriter w = simple(QStringLiteral("x\x08y\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1)}};
        ShapeSpec pic;
        pic.spid = 1030;
        pic.type = 75;
        pic.cp = 1;
        pic.x0 = 0; pic.y0 = 0; pic.x1 = 2880; pic.y1 = 1440;
        pic.wr = 3;
        pic.below = true;
        pic.png = tinyPng(16, 8);
        w.shapes = {pic};
        Document d;
        QVERIFY(read(w.build(), &d));
        const Inline *pict = nullptr;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("pict")) pict = &in;
        QVERIFY(pict);
        const QVector<wml::Shape> shapes = wml::readShapes(pict->raw);
        QCOMPARE(shapes.size(), qsizetype(1));
        QCOMPARE(shapes[0].kind, wml::Shape::Picture);
        QVERIFY(shapes[0].floating);
        QVERIFY(shapes[0].behind);
        QCOMPARE(shapes[0].wrap, wml::Shape::WrapNone);
        QCOMPARE(shapes[0].width, 144.0);
        QCOMPARE(shapes[0].height, 72.0);
        QCOMPARE(shapes[0].hRel, wml::Shape::Column);
        QCOMPARE(shapes[0].vRel, wml::Shape::Paragraph);
        QVERIFY(!shapes[0].embed.isEmpty());
        bool media = false;
        for (const wml::KeptPart &p : d.parts)
            if (p.name.startsWith(QLatin1String("/word/media/")) && p.data.startsWith(QByteArray("\x89PNG"))) media = true;
        QVERIFY(media);
    }

    void shapesOfTheHeadersAreInTheHeaders()
    {
        DocWriter w = simple(QStringLiteral("body\r"));
        w.headerStories = {QString(), QString(), QString(), QString(), QString(), QString(), QString(), QStringLiteral("\x08\r")};
        const int headerStart = int(w.text.size());
        w.chars = {{headerStart, headerStart + 1, sprm1(sprm::special, 1)}};
        ShapeSpec box;
        box.header = true;
        box.cp = 0;
        box.x0 = -720; box.y0 = -240; box.x1 = 7200; box.y1 = 600;
        box.text = QStringLiteral("Letterhead\r");
        w.shapes = {box};
        Document d;
        QVERIFY(read(w.build(), &d));
        const wml::HeaderRefs refs = wml::headerRefsOf(d.lastSection);
        QVERIFY(!refs.header[0].isEmpty());
        QString headerPart;
        for (const auto &r : d.mainRelationships)
            if (r.id == refs.header[0]) headerPart = r.target;
        const wml::KeptPart *p = part(d, headerPart);
        QVERIFY(p);
        QVERIFY(QString::fromUtf8(p->data).contains(QLatin1String("Letterhead")));
        QVERIFY(QString::fromUtf8(p->data).contains(QLatin1String("margin-left:-36pt")));
    }

    void theMembersOfAGroupAreScaledIntoTheGroup()
    {
        DocWriter w = simple(QStringLiteral("g\x08\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1)}};
        ShapeSpec group;
        group.spid = 1040;
        group.type = 0;
        group.cp = 1;
        group.x0 = 1000; group.y0 = 2000; group.x1 = 5000; group.y1 = 4000;   // 4000 by 2000 twips among the text
        group.groupLeft = 0; group.groupTop = 0; group.groupRight = 1000; group.groupBottom = 1000;   // its members' coordinates
        ShapeSpec member;
        member.spid = 1041;
        member.type = 1;
        member.group = 0;
        member.memberLeft = 250; member.memberTop = 500; member.memberRight = 750; member.memberBottom = 1000;
        member.fill = 0x0000FF;
        member.line = 0x000000;
        w.shapes = {group, member};
        Document d;
        QVERIFY(read(w.build(), &d));
        const Inline *pict = nullptr;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("pict")) pict = &in;
        QVERIFY(pict);
        const QVector<wml::Shape> shapes = wml::readShapes(pict->raw);
        QCOMPARE(shapes.size(), qsizetype(1));
        QCOMPARE(shapes[0].kind, wml::Shape::Rect);
        // A quarter of the way across 4000 twips from 1000 (to 2000), half way down 2000 from 2000 (to 3000): 50 by 50 points to 100 wide... as twips/20.
        QCOMPARE(shapes[0].left, 100.0);
        QCOMPARE(shapes[0].top, 150.0);
        QCOMPARE(shapes[0].width, 100.0);
        QCOMPARE(shapes[0].height, 50.0);
        QVERIFY(shapes[0].filled);
        QCOMPARE(shapes[0].fillColor, QStringLiteral("FF0000"));
    }

    void aShapeThatTheDrawingDoesNotHoldIsLeftOut()
    {
        DocWriter w = simple(QStringLiteral("x\x08y\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1)}};
        ShapeSpec box;
        box.cp = 1;
        box.x1 = 1440; box.y1 = 720;
        box.text = QStringLiteral("t\r");
        w.shapes = {box};
        QByteArray file = w.build();
        // (The shape's record is made another shape's: the place the file gives has no shape in the drawing.)
        jp::cfb::File f;
        QVERIFY(jp::cfb::read(file, &f));
        QByteArray t = f.stream(QStringLiteral("1Table"));
        const int at = t.indexOf(QByteArray::fromHex("01040000"));   // (the box's id, 1025, in the table of places)
        QVERIFY(at >= 0);
        t[at] = char(0x77);
        QVERIFY(f.setStream(QStringLiteral("1Table"), t));
        Document d;
        ReadNotes notes;
        QVERIFY(read(jp::cfb::write(f), &d, &notes));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("xy")});
        QVERIFY(notes.leftOut);
    }

    // ---- files that are not read

    void refusals()
    {
        Document d;
        QString why;
        QVERIFY(!read(QByteArray("this is not a compound file at all"), &d, nullptr, &why));
        QVERIFY(!why.isEmpty());
        DocWriter pw = simple(QStringLiteral("secret\r"));
        pw.password = true;
        why.clear();
        QVERIFY(!read(pw.build(), &d, nullptr, &why));
        QVERIFY2(why.contains(QLatin1String("password")), qPrintable(why));
        DocWriter ow = simple(QStringLiteral("old\r"));
        ow.oldVersion = true;
        why.clear();
        QVERIFY(!read(ow.build(), &d, nullptr, &why));
        QVERIFY2(why.contains(QLatin1String("1997")), qPrintable(why));
        why.clear();
        QVERIFY(!read(QByteArray(), &d, nullptr, &why));
    }

    void looksLikeACompoundFile()
    {
        QVERIFY(doc::looksLikeCompoundFile(simple(QStringLiteral("x\r")).build()));
        QVERIFY(!doc::looksLikeCompoundFile(QByteArray("PK\x03\x04 and more bytes")));
        QVERIFY(!doc::looksLikeCompoundFile(QByteArray()));
    }

    // ---- hostile files

    void aTextLongerThanTheFileSaysIsCut()
    {
        QByteArray file = simple(QStringLiteral("short\r")).build();
        Document d;
        ReadNotes notes;
        // (Header changed by hand: the main text claims two billion characters.)
        jp::cfb::File f;
        QVERIFY(jp::cfb::read(file, &f));
        QByteArray wd = f.stream(QStringLiteral("WordDocument"));
        wd[76] = char(0xFF);
        wd[77] = char(0xFF);
        wd[78] = char(0xFF);
        wd[79] = char(0x7F);
        QVERIFY(f.setStream(QStringLiteral("WordDocument"), wd));
        QElapsedTimer t;
        t.start();
        QString error;
        QVERIFY2(read(jp::cfb::write(f), &d, &notes, &error), qPrintable(error));
        QVERIFY(t.elapsed() < 5000);
        // (What the file holds is read; the length it claims is not believed, and the cut is said.)
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("short")});
        QVERIFY(notes.leftOut);
    }

    void tooManyRowsAreCutAndSaid()
    {
        // More rows than the .docx reader lays out: the rest are left out, and the time stays short.
        const int rows = wml::kMaxTableRows + 300;
        QString text;
        DocWriter w;
        const QByteArray in = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, 1);
        const QByteArray row = in + sprm1(sprm::endsRow, 1) + tableRow({0, 1000});
        int at = 0;
        for (int i = 0; i < rows; ++i) {
            text += QStringLiteral("x\a\a");
            w.paras.push_back({at, at + 2, 0, in});
            w.paras.push_back({at + 2, at + 3, 0, row});
            at += 3;
        }
        text += QStringLiteral("end\r");
        w.text = text;
        QElapsedTimer t;
        t.start();
        Document d;
        ReadNotes notes;
        QVERIFY(read(w.build(), &d, &notes));
        QVERIFY(t.elapsed() < 20000);
        int count = 0;
        for (const Block &b : d.body)
            if (b.isRaw && b.tag == QLatin1String("tr") && b.role == Inline::Open) ++count;
        QVERIFY(count <= wml::kMaxTableRows);
        QVERIFY(notes.leftOut);
    }

    void manySectionsAreBounded()
    {
        const int sections = doc::kMaxSections + 500;
        QString text;
        DocWriter w;
        for (int i = 0; i < sections; ++i) {
            text += QLatin1Char('\f');
            w.paras.push_back({i, i + 1, 0, QByteArray()});
            w.sections.push_back({i + 1, QByteArray()});
        }
        w.text = text;
        QElapsedTimer t;
        t.start();
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(t.elapsed() < 20000);
        int breaks = 0;
        for (const Block &b : d.body)
            if (!b.isRaw && b.para.props.section) ++breaks;
        QVERIFY(breaks <= doc::kMaxSections);
    }

    void aPictureManyCharactersPointToIsReadOnce()
    {
        DocWriter w;
        const int at = w.addPicture(tinyPng(32, 32), 1440, 1440);
        QString text;
        const int refs = 3000;
        for (int i = 0; i < refs; ++i) {
            text += QLatin1Char('\x01');
            w.chars.push_back({i, i + 1, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(at))});
        }
        text += QLatin1Char('\r');
        w.text = text;
        QElapsedTimer t;
        t.start();
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(t.elapsed() < 20000);
        int media = 0;
        for (const wml::KeptPart &p : d.parts)
            if (p.name.startsWith(QLatin1String("/word/media/"))) ++media;
        QCOMPARE(media, 1);
    }

    void notesThatOverlapAreLeftOut()
    {
        // The table of the notes' text says the second begins at 255, past the end of the notes' text, and the first has to end there: the notes follow one
        // another, so this is not read, rather than read again for each.
        DocWriter w = simple(QStringLiteral("a\x02""b\x02""c\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1)}, {3, 4, sprm1(sprm::special, 1)}};
        w.footnotes = {QStringLiteral("\x02 one\r"), QStringLiteral("\x02 two\r")};
        w.footnoteRefs = {1, 3};
        // (As the file should have it, the notes are read.)
        {
            Document d;
            QVERIFY(read(w.build(), &d));
            QVERIFY(part(d, QStringLiteral("/word/footnotes.xml")) != nullptr);
        }
        w.footnoteStarts = {0, 255};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("abc")});
        QVERIFY(part(d, QStringLiteral("/word/footnotes.xml")) == nullptr || !QString::fromUtf8(part(d, QStringLiteral("/word/footnotes.xml"))->data).contains(QLatin1String("one")));
    }

    void aLongChainOfStylesIsCutShort()
    {
        DocWriter w = simple(QStringLiteral("x\r"));
        QVector<StyleSpec> st;
        for (int i = 0; i < 300; ++i) {
            StyleSpec s;
            s.name = QStringLiteral("S%1").arg(i);
            s.sti = i == 0 ? 0 : 0xFFE;
            s.base = i == 0 ? 0xFFF : i - 1;
            s.character = sprm1(sprm::bold, 0x81);
            st.push_back(s);
        }
        w.styles = st;
        w.paras = {{0, 2, 299, QByteArray()}};
        Document d;
        QElapsedTimer t;
        t.start();
        QVERIFY(read(w.build(), &d));
        QVERIFY(t.elapsed() < 5000);
        QCOMPARE(bodyTexts(d), QStringList{"x"});
        // (The style the paragraph has is still read, and its chain no longer than the limit.)
        int depth = 0;
        for (const wml::Style *s = d.styles.find(QStringLiteral("S299")); s && depth < 1000; s = s->basedOn.isEmpty() ? nullptr : d.styles.find(s->basedOn)) ++depth;
        QVERIFY(depth <= doc::kMaxStyleDepth + 2);
    }

    // ---- files made to cost a great deal (each must be read in a couple of seconds)

    void aGroupAnchoredOftenIsNotWalkedOverAndOver()
    {
        // 20,000 members the reader shows nothing of, in one group that 1,000 anchors point to: each anchor walked the group again (20 million visits).
        const int members = 20000, anchors = 1000;
        DocWriter w = simple(QString(anchors, QLatin1Char('\x08')) + QLatin1Char('\r'));
        w.chars = {{0, anchors, sprm1(sprm::special, 1)}};
        ShapeSpec group;
        group.spid = 2000;
        group.type = 0;
        group.cp = 0;
        for (int i = 1; i < anchors; ++i) group.moreAnchors.push_back(i);
        w.shapes.push_back(group);
        for (int i = 0; i < members; ++i) {
            ShapeSpec m;
            m.spid = 3000 + i;
            m.type = 0;
            m.group = 0;
            m.memberLeft = 0; m.memberTop = 0; m.memberRight = 10; m.memberBottom = 10;
            w.shapes.push_back(m);
        }
        const QByteArray file = w.build();
        QElapsedTimer t;
        t.start();
        Document d;
        ReadNotes notes;
        QVERIFY(read(file, &d, &notes));
        QVERIFY2(t.elapsed() < kFastMs, qPrintable(QStringLiteral("took %1 ms").arg(t.elapsed())));
        QVERIFY(notes.leftOut);
    }

    void aShapeNumberThatDoublesPastTheLimitDoesNotMeetAnother()
    {
        // The ids 0x40000000 and 0xC0000000 doubled (the way text boxes already shown are kept) are the same number in 32 bits: the second box lost its text.
        DocWriter w = simple(QStringLiteral("a\x08""b\x08\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1)}, {3, 4, sprm1(sprm::special, 1)}};
        ShapeSpec one, two;
        one.spid = 0x40000000;
        one.cp = 1;
        one.x1 = 1440; one.y1 = 720;
        one.text = QStringLiteral("first box\r");
        two.spid = qint32(0xC0000000u);
        two.cp = 3;
        two.x1 = 1440; two.y1 = 720;
        two.text = QStringLiteral("second box\r");
        w.shapes = {one, two};
        Document d;
        QVERIFY(read(w.build(), &d));
        QStringList boxes;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("pict"))
                for (const wml::Shape &s : wml::readShapes(in.raw)) boxes << QString::fromUtf8(s.textBox);
        QCOMPARE(boxes.size(), qsizetype(2));
        QVERIFY(boxes[0].contains(QLatin1String("first box")));
        QVERIFY(boxes[1].contains(QLatin1String("second box")));
    }

    void pieceTablesThatShareTextCannotMakeMoreXmlNodesThanTheReaderTakes()
    {
        // 90 characters of text, each its own run (bold, then not), listed 5,000 times over: 450,000 runs from a file of a few kilobytes. The XML of that
        // has more nodes than the .docx reader takes in a part, and the document was refused after all the work of making it.
        const QString line = QStringLiteral("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
        DocWriter w;
        w.text = line.left(89) + QLatin1Char('\r');
        for (int i = 0; i < 89; ++i) w.chars.push_back({i, i + 1, sprm1(sprm::bold, i & 1)});
        w.repeatText = 5000;
        const QByteArray file = w.build();
        QVERIFY(file.size() < 200000);
        QElapsedTimer t;
        t.start();
        Document d;
        ReadNotes notes;
        QString error;
        QVERIFY2(read(file, &d, &notes, &error), qPrintable(error));
        QVERIFY2(t.elapsed() < kFastMs, qPrintable(QStringLiteral("took %1 ms").arg(t.elapsed())));
        QVERIFY(notes.leftOut);
        QVERIFY(notes.notes.join(QLatin1Char(' ')).contains(QLatin1String("longer than this program can open")));
        // (What came before the cut is there, whole paragraphs of it.)
        const QStringList texts = bodyTexts(d);
        QVERIFY(texts.size() > 100 && texts.size() < 5000);
        QCOMPARE(texts[0], w.text.left(89));
    }

    void pieceTablesThatShareTextCannotMakeMoreXmlBytesThanTheReaderTakes()
    {
        // 1,000 ampersands (five bytes of XML each) listed 15,000 times over: 75 MB of XML from a file of a few kilobytes, past what the reader takes of a part.
        DocWriter w;
        w.text = QString(1000, QLatin1Char('&')) + QLatin1Char('\r');
        w.repeatText = 15000;
        const QByteArray file = w.build();
        QVERIFY(file.size() < 200000);
        QElapsedTimer t;
        t.start();
        Document d;
        ReadNotes notes;
        QString error;
        QVERIFY2(read(file, &d, &notes, &error), qPrintable(error));
        QVERIFY2(t.elapsed() < kFastMs, qPrintable(QStringLiteral("took %1 ms").arg(t.elapsed())));
        QVERIFY(notes.leftOut);
        QVERIFY(notes.notes.join(QLatin1Char(' ')).contains(QLatin1String("longer than this program can open")));
        const QStringList texts = bodyTexts(d);
        QVERIFY(texts.size() > 1000 && texts.size() < 15000);
        QCOMPARE(texts[0], QString(1000, QLatin1Char('&')));
    }

    void aSingleParagraphLongerThanTheOutputMayBeIsCutAndSaid()
    {
        // The same, as one paragraph (no mark ends it): it is cut in the middle, and the cut is said.
        DocWriter w;
        w.text = QString(1000, QLatin1Char('&'));
        w.repeatText = 15000;
        const QByteArray file = w.build();
        QElapsedTimer t;
        t.start();
        Document d;
        ReadNotes notes;
        QString error;
        QVERIFY2(read(file, &d, &notes, &error), qPrintable(error));
        QVERIFY2(t.elapsed() < kFastMs, qPrintable(QStringLiteral("took %1 ms").arg(t.elapsed())));
        QVERIFY(notes.leftOut);
        QVERIFY(notes.notes.join(QLatin1Char(' ')).contains(QLatin1String("longer than this program can open")));
        const QStringList texts = bodyTexts(d);
        QVERIFY(!texts.isEmpty());
        QVERIFY(texts[0].size() > 1000000 && texts[0].size() < 15000000);
    }

    void picturesTogetherAreHeldToALimit()
    {
        // Four metafiles that each make 30 MB once inflated, from a few kilobytes of file apiece: the pictures together could be 120 MB, or (the limit then)
        // 384 MB, held while the package was made, written, and read again.
        DocWriter w;
        QString text;
        const int pictures = 4;
        for (int i = 0; i < pictures; ++i) {
            const int at = w.addMetafile(30 << 20, 1440, 1440);
            w.chars.push_back({i, i + 1, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(at))});
            text += QLatin1Char('\x01');
        }
        w.text = text + QLatin1Char('\r');
        const QByteArray file = w.build();
        QVERIFY(file.size() < 1000000);
        QElapsedTimer t;
        t.start();
        Document d;
        ReadNotes notes;
        QString error;
        QVERIFY2(read(file, &d, &notes, &error), qPrintable(error));
        QVERIFY2(t.elapsed() < kFastMs, qPrintable(QStringLiteral("took %1 ms").arg(t.elapsed())));
        qint64 bytes = 0;
        int media = 0;
        for (const wml::KeptPart &p : d.parts)
            if (p.name.startsWith(QLatin1String("/word/media/"))) {
                ++media;
                bytes += p.data.size();
            }
        QVERIFY(media >= 1 && media < pictures);
        QVERIFY(bytes <= doc::kMaxPictureTotal);
        QVERIFY(notes.leftOut);
        QVERIFY(notes.notes.join(QLatin1Char(' ')).contains(QLatin1String("pictures could not be shown")));
    }

    void changedBytesNeverCrashOrHang()
    {
        // A document with most of what the reader reads, with bytes of it changed at random (the same changes each run), and cut short.
        DocWriter w = simple(QStringLiteral("Hello \x13 PAGE \x14""1\x15 world\a\a\rcell\a\a\r\x01 end\rnote\x02\r"));
        w.fonts = {QStringLiteral("Times New Roman"), QStringLiteral("Arial")};
        StyleSpec normal;
        normal.name = QStringLiteral("Normal");
        normal.sti = 0;
        StyleSpec head;
        head.name = QStringLiteral("heading 1");
        head.sti = 1;
        head.base = 0;
        head.character = sprm1(sprm::bold, 1);
        w.styles = {normal, head};
        ListLevelSpec dec;
        dec.text = QString(QChar(0)) + QLatin1Char('.');
        w.lists = {{dec}};
        w.footnotes = {QStringLiteral("\x02 note\r")};
        w.footnoteRefs = {10};
        w.headerStories = {QString(), QString(), QString(), QString(), QString(), QString(), QStringLiteral("head\r"), QString(), QString(), QString(), QString(), QString()};
        const int pic = w.addPicture(tinyPng(), 1440, 720);
        w.chars = {{0, 5, sprm1(sprm::bold, 1)}, {30, 31, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(pic))}};
        const QByteArray in = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, 1);
        w.paras = {{0, 24, 1, sprm2(sprm::listInstance, 1)}, {24, 26, 0, in + sprm1(sprm::endsRow, 1) + tableRow({0, 1000, 2000})}};
        w.sections = {{40, sprm2(sprm::pageWidth, 12240)}, {int(w.text.size()) + 1, QByteArray()}};
        const QByteArray good = w.build();
        Document ok;
        QVERIFY(read(good, &ok));
        QRandomGenerator rng(20261016);
        QElapsedTimer total;
        total.start();
        int read1 = 0;
        // (The changes are made where the file holds something: the zeros that fill out its sectors would take a change and do nothing.)
        const QVector<int> live = liveBytes(good);
        QVERIFY(live.size() > 1000);
        for (int round = 0; round < 400; ++round) {
            QByteArray bad = good;
            const int changes = 1 + int(rng.bounded(8));
            for (int i = 0; i < changes; ++i) {
                const int at = live[int(rng.bounded(quint32(live.size())))];
                switch (rng.bounded(3u)) {
                case 0: bad[at] = char(rng.bounded(256u)); break;
                case 1: bad[at] = char(0xFF); break;
                default: bad[at] = char(bad[at] ^ (1 << rng.bounded(8u))); break;
                }
            }
            if (round % 7 == 0) bad.truncate(int(rng.bounded(quint32(bad.size()))));
            Document d;
            QElapsedTimer one;
            one.start();
            if (read(bad, &d)) ++read1;
            QVERIFY2(one.elapsed() < 3000, qPrintable(QStringLiteral("round %1 took %2 ms").arg(round).arg(one.elapsed())));
        }
        QVERIFY(read1 > 0);
        QVERIFY(total.elapsed() < 120000);
    }

    // A developer's hook, not a test: JO_DOC_SWEEP_IN names a folder (searched through) of .doc files; each is read as it is, and again with bytes
    // of it changed, JO_DOC_SWEEP_ROUNDS times (20 if not said; the same changes each run unless JO_DOC_SWEEP_SEED says another), and what a sanitizer
    // build says is what it is for.
    void sweepOfRealFiles()
    {
        const QString in = qEnvironmentVariable("JO_DOC_SWEEP_IN");
        if (in.isEmpty()) QSKIP("JO_DOC_SWEEP_IN is not set");
        const int rounds = qEnvironmentVariableIsSet("JO_DOC_SWEEP_ROUNDS") ? qEnvironmentVariableIntValue("JO_DOC_SWEEP_ROUNDS") : 20;
        QRandomGenerator rng(qEnvironmentVariableIsSet("JO_DOC_SWEEP_SEED") ? quint32(qEnvironmentVariableIntValue("JO_DOC_SWEEP_SEED")) : 20261018u);
        int files = 0, readable = 0;
        QDirIterator it(in, {QStringLiteral("*.doc")}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QFile f(it.next());
            if (!f.open(QIODevice::ReadOnly)) continue;
            const QByteArray bytes = f.readAll();
            ++files;
            Document d;
            QElapsedTimer t;
            t.start();
            if (read(bytes, &d)) ++readable;
            QVERIFY2(t.elapsed() < 60000, qPrintable(f.fileName()));
            jp::cfb::File cf;
            const bool whole = jp::cfb::read(bytes, &cf);
            for (int round = 0; round < rounds; ++round) {
                QByteArray bad;
                if (whole && round % 2 == 0) {
                    jp::cfb::File g = cf;
                    const QStringList names = {QStringLiteral("WordDocument"), QStringLiteral("1Table"), QStringLiteral("0Table"), QStringLiteral("Data")};
                    const QString name = names[int(rng.bounded(quint32(names.size())))];
                    QByteArray st = g.stream(name);
                    if (st.isEmpty()) continue;
                    const int changes = 1 + int(rng.bounded(10));
                    for (int i = 0; i < changes; ++i) {
                        const int at = int(rng.bounded(quint32(st.size())));
                        switch (rng.bounded(3u)) {
                        case 0: st[at] = char(rng.bounded(256u)); break;
                        case 1: st[at] = char(0xFF); break;
                        default: st[at] = char(0); break;
                        }
                    }
                    g.setStream(name, st);
                    bad = jp::cfb::write(g);
                } else {
                    bad = bytes;
                    const int changes = 1 + int(rng.bounded(10));
                    for (int i = 0; i < changes; ++i) bad[int(rng.bounded(quint32(bad.size())))] = char(rng.bounded(256u));
                    if (round % 7 == 3) bad.truncate(int(rng.bounded(quint32(bad.size()))));
                }
                Document e;
                QElapsedTimer one;
                one.start();
                read(bad, &e);
                QVERIFY2(one.elapsed() < 60000, qPrintable(f.fileName()));
            }
        }
        qInfo("sweep: %d files, %d read whole, %d changed copies each", files, readable, rounds);
    }

    void changedStreamsNeverCrashOrHang()
    {
        // The same, with the changes made inside the streams (the compound file stays whole, so the damage reaches the reader's structures).
        DocWriter w = simple(QStringLiteral("Hello \x13 PAGE \x14""1\x15 world\a\a\rcell\a\a\r\x01 end\r"));
        w.styles = {};
        ListLevelSpec dec;
        dec.text = QString(QChar(0)) + QLatin1Char('.');
        w.lists = {{dec}};
        w.footnotes = {QStringLiteral("\x02 note\r")};
        w.footnoteRefs = {10};
        const int pic = w.addPicture(tinyPng(), 1440, 720);
        w.chars = {{0, 5, sprm1(sprm::bold, 1)}, {28, 29, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(pic))}};
        const QByteArray in = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, 1);
        w.paras = {{0, 24, 0, sprm2(sprm::listInstance, 1)}, {24, 26, 0, in + sprm1(sprm::endsRow, 1) + tableRow({0, 1000, 2000})}};
        jp::cfb::File f;
        QVERIFY(jp::cfb::read(w.build(), &f));
        QRandomGenerator rng(20261017);
        const QStringList names = {QStringLiteral("WordDocument"), QStringLiteral("1Table"), QStringLiteral("Data")};
        // The places a change is made: the header's numbers of lengths and places, the first and last bytes of the tables it names (their counts and their
        // last places), and any byte that holds something; never the padding of zeros.
        const QByteArray wdBytes = f.stream(QStringLiteral("WordDocument")), tableBytes = f.stream(QStringLiteral("1Table"));
        QHash<QString, QVector<int>> aims;
        aims[names[0]] = fibNumbers(wdBytes) + liveBytes(wdBytes);
        aims[names[1]] = tableEdges(wdBytes, tableBytes) + liveBytes(tableBytes);
        aims[names[2]] = liveBytes(f.stream(QStringLiteral("Data")));
        QVERIFY(aims[names[0]].size() > 100 && aims[names[1]].size() > 100 && aims[names[2]].size() > 20);
        for (int round = 0; round < 600; ++round) {
            jp::cfb::File g = f;
            const QString name = names[int(rng.bounded(quint32(names.size())))];
            QByteArray s = g.stream(name);
            if (s.isEmpty()) continue;
            const QVector<int> &aim = aims[name];
            const int changes = 1 + int(rng.bounded(6));
            for (int i = 0; i < changes; ++i) {
                // (The first half of the aim is the numbers that count and measure; those go to the largest value a byte has, or the smallest, as often as to any.)
                const int at = aim[int(rng.bounded(quint32(aim.size())))];
                switch (rng.bounded(4u)) {
                case 0: s[at] = char(0xFF); break;
                case 1: s[at] = char(0x7F); break;
                case 2: s[at] = char(0); break;
                default: s[at] = char(rng.bounded(256u)); break;
                }
            }
            QVERIFY(g.setStream(name, s));
            Document d;
            QElapsedTimer one;
            one.start();
            read(jp::cfb::write(g), &d);
            QVERIFY2(one.elapsed() < 3000, qPrintable(QStringLiteral("round %1 (%2) took %3 ms").arg(round).arg(name).arg(one.elapsed())));
        }
    }
};
JP_TEST_CLASS(OfficeDocTests)

#include "tst_doc.moc"

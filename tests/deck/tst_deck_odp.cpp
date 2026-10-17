// JeffDeck's tests of open document presentations: reading, writing, and saving from the window.
#include "deckhelpers.h"

namespace odptest {

// A chart object as the other office suite writes one: its numbers in a table of their own, and the picture it keeps of itself in its own metafile format.
QByteArray sampleChart()
{
    return (QStringLiteral("<?xml version=\"1.0\"?><office:document-content") + kNs + QStringLiteral("><office:automatic-styles>"
        "<style:style style:name=\"ch1\" style:family=\"chart\"><style:chart-properties chart:stacked=\"false\"/></style:style>"
        "<style:style style:name=\"ch2\" style:family=\"chart\"><style:graphic-properties draw:fill-color=\"#4f81bd\"/></style:style></office:automatic-styles>"
        "<office:body><office:chart><chart:chart chart:class=\"chart:bar\" svg:width=\"8cm\" svg:height=\"5cm\"><chart:title><text:p>Sales by quarter</text:p></chart:title>"
        "<chart:legend chart:legend-position=\"end\"/><chart:plot-area chart:style-name=\"ch1\"><chart:axis chart:dimension=\"x\" chart:name=\"primary-x\">"
        "<chart:categories table:cell-range-address=\"local-table.$A$2:.$A$4\"/></chart:axis><chart:axis chart:dimension=\"y\" chart:name=\"primary-y\"><chart:grid chart:class=\"major\"/></chart:axis>"
        "<chart:series chart:values-cell-range-address=\"local-table.$B$2:.$B$4\" chart:label-cell-address=\"local-table.$B$1\" chart:style-name=\"ch2\" chart:class=\"chart:bar\"/></chart:plot-area>"
        "<table:table table:name=\"local-table\"><table:table-header-columns><table:table-column/></table:table-header-columns><table:table-columns><table:table-column/></table:table-columns>"
        "<table:table-header-rows><table:table-row><table:table-cell/><table:table-cell office:value-type=\"string\"><text:p>Revenue</text:p></table:table-cell></table:table-row></table:table-header-rows>"
        "<table:table-rows><table:table-row><table:table-cell office:value-type=\"string\"><text:p>Q1</text:p></table:table-cell><table:table-cell office:value-type=\"float\" office:value=\"10\"/></table:table-row>"
        "<table:table-row><table:table-cell office:value-type=\"string\"><text:p>Q2</text:p></table:table-cell><table:table-cell office:value-type=\"float\" office:value=\"20\"/></table:table-row>"
        "<table:table-row><table:table-cell office:value-type=\"string\"><text:p>Q3</text:p></table:table-cell><table:table-cell office:value-type=\"float\" office:value=\"15\"/></table:table-row>"
        "</table:table-rows></table:table></chart:chart></office:chart></office:body></office:document-content>")).toUtf8();
}

// A chart object's content with a table of its own: the first column names the categories, each other column is a series (named S1, S2...),
// drawn as `cls`. `autoStyles` are the styles it uses, `plotXml` goes in the plot area before the series, `seriesXml` inside each series, and
// `categories` is the range the x axis names, or empty for the column of names itself.
QByteArray chartContent(const QString &cls, const QStringList &names, const QVector<QVector<double>> &columns, const QString &autoStyles = QString(), const QString &plotXml = QString(),
                        const QString &seriesAttrs = QString(), const QString &seriesXml = QString(), const QString &categories = QString(), const QString &axesXml = QString())
{
    const int n = int(names.size());
    QString header = QStringLiteral("<table:table-row><table:table-cell/>"), body;
    for (int c = 0; c < columns.size(); ++c) header += QStringLiteral("<table:table-cell office:value-type=\"string\"><text:p>S%1</text:p></table:table-cell>").arg(c + 1);
    header += QStringLiteral("</table:table-row>");
    for (int r = 0; r < n; ++r) {
        body += QStringLiteral("<table:table-row><table:table-cell office:value-type=\"string\"><text:p>%1</text:p></table:table-cell>").arg(names[r]);
        for (const QVector<double> &column : columns)
            body += QStringLiteral("<table:table-cell office:value-type=\"float\" office:value=\"%1\"><text:p>%1</text:p></table:table-cell>").arg(r < column.size() ? column[r] : 0);
        body += QStringLiteral("</table:table-row>");
    }
    QString series;
    for (int c = 0; c < columns.size(); ++c) {
        const QChar col = QChar('B' + c);
        series += QStringLiteral("<chart:series chart:values-cell-range-address=\"local-table.$%1$2:.$%1$%2\" chart:label-cell-address=\"local-table.$%1$1\" chart:class=\"%3\" %4>%5</chart:series>")
                      .arg(col).arg(n + 1).arg(cls, seriesAttrs, seriesXml);
    }
    const QString axes = axesXml.isEmpty() ? QStringLiteral("<chart:axis chart:dimension=\"x\" chart:name=\"primary-x\"><chart:categories table:cell-range-address=\"%1\"/></chart:axis>"
                                                           "<chart:axis chart:dimension=\"y\" chart:name=\"primary-y\"><chart:grid chart:class=\"major\"/></chart:axis>")
                                                 .arg(categories.isEmpty() ? QStringLiteral("local-table.$A$2:.$A$%1").arg(n + 1) : categories)
                                           : axesXml;
    return (QStringLiteral("<?xml version=\"1.0\"?><office:document-content") + kNs + QStringLiteral("><office:automatic-styles>") + autoStyles
            + QStringLiteral("</office:automatic-styles><office:body><office:chart><chart:chart chart:class=\"%1\"><chart:legend chart:legend-position=\"end\"/><chart:plot-area>").arg(cls)
            + plotXml + axes + series + QStringLiteral("</chart:plot-area><table:table table:name=\"local-table\"><table:table-header-rows>") + header
            + QStringLiteral("</table:table-header-rows><table:table-rows>") + body + QStringLiteral("</table:table-rows></table:table></chart:chart></office:chart></office:body></office:document-content>"))
        .toUtf8();
}

// A presentation of one slide with a frame for each of `sizes` (width and height in points), every one holding the chart `chart` (as another part
// of the file, with a picture of its own that can't be shown).
QByteArray chartDeck(const QByteArray &chart, const QVector<QSizeF> &sizes)
{
    QString frames;
    for (const QSizeF &s : sizes)
        frames += QStringLiteral("<draw:frame draw:style-name=\"gr5\" draw:layer=\"layout\" svg:width=\"%1pt\" svg:height=\"%2pt\" svg:x=\"1cm\" svg:y=\"1cm\"><draw:object xlink:href=\"./Object 1\"/>"
                                 "<draw:image xlink:href=\"./ObjectReplacements/Object 1\"/></draw:frame>").arg(s.width()).arg(s.height());
    return odpZip(odpContent(sampleAutomaticStyles(), QStringLiteral("<draw:page draw:name=\"p\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\">") + frames + QStringLiteral("</draw:page>")),
                  odpStyles(), {{QStringLiteral("Object 1/content.xml"), chart}, {QStringLiteral("ObjectReplacements/Object 1"), QByteArray("VCLMTF\1\0 a metafile of the producer's own")}});
}

QByteArray sampleOdp()
{
    return odpZip(sampleContent(), odpStyles(), {{QStringLiteral("Pictures/a.png"), png(200, 100, QColor(200, 30, 30))},
                                                 {QStringLiteral("ObjectReplacements/Object 1"), png(40, 30, QColor(30, 30, 200))},
                                                 {QStringLiteral("Object 2/content.xml"), sampleChart()},
                                                 {QStringLiteral("ObjectReplacements/Object 2"), QByteArray("VCLMTF\1\0 a metafile of the producer's own")}});
}

// What one object of a slide is, as the model keeps it: enough to tell that a presentation written and read again has the same slides.
QString describeItem(const Document &d, const Item &it)
{
    QStringList s;
    s << itemTypeName(it.type()) << QStringLiteral("%1,%2 %3x%4").arg(it.rect.x(), 0, 'f', 1).arg(it.rect.y(), 0, 'f', 1).arg(it.rect.width(), 0, 'f', 1).arg(it.rect.height(), 0, 'f', 1);
    s << QStringLiteral("rot %1").arg(std::lround(it.rotation));
    if (it.flipH || it.flipV) s << QStringLiteral("flip %1%2").arg(it.flipH).arg(it.flipV);
    s << (it.role.isEmpty() ? QString() : it.role.section(QLatin1Char(':'), 0, 1));
    if (it.type() != ItemType::Line && it.type() != ItemType::Table && it.type() != ItemType::Group) {
        s << QStringLiteral("fill %1").arg(it.fill.type == Fill::Solid ? it.fill.color.toString() + QStringLiteral("/") + QString::number(it.fill.transparency, 'f', 2) : QString::number(int(it.fill.type)));
    }
    if (!it.stroke.isNone()) s << QStringLiteral("line %1 %2 %3 %4 %5").arg(it.stroke.width, 0, 'f', 2).arg(it.stroke.color.toString()).arg(int(it.stroke.dash)).arg(int(it.stroke.startArrow)).arg(int(it.stroke.endArrow));
    if (it.fx.shadow.on) s << QStringLiteral("shadow");
    if (it.type() == ItemType::Text) {
        const auto &t = static_cast<const TextItem &>(it);
        s << d.storyDoc(t.storyId)->toPlainText() << QStringLiteral("valign %1 autofit %2").arg(int(t.valign)).arg(int(t.autofit));
    } else if (it.type() == ItemType::Shape) {
        const auto &sh = static_cast<const ShapeItem &>(it);
        s << sh.shape << (sh.storyId.isEmpty() ? QString() : d.storyDoc(sh.storyId)->toPlainText());
    } else if (it.type() == ItemType::Picture) {
        const auto &p = static_cast<const PictureItem &>(it);
        s << QStringLiteral("image %1").arg(p.imageId.isEmpty() ? 0 : qHash(d.images.value(p.imageId).bytes)) << QStringLiteral("crop %1,%2 %3x%4").arg(p.imgRect.x(), 0, 'f', 1).arg(p.imgRect.y(), 0, 'f', 1).arg(p.imgRect.width(), 0, 'f', 1).arg(p.imgRect.height(), 0, 'f', 1);
    } else if (it.type() == ItemType::Line) {
        const auto &l = static_cast<const LineItem &>(it);
        s << QStringLiteral("%1,%2 to %3,%4 route %5").arg(l.p1.x(), 0, 'f', 1).arg(l.p1.y(), 0, 'f', 1).arg(l.p2.x(), 0, 'f', 1).arg(l.p2.y(), 0, 'f', 1).arg(int(l.route));
    } else if (it.type() == ItemType::Table) {
        const auto &t = static_cast<const TableItem &>(it);
        s << QStringLiteral("%1x%2").arg(t.rows).arg(t.cols);
        for (int r = 0; r < t.rows; ++r)
            for (int c = 0; c < t.cols; ++c) {
                const TableCell &cell = t.cell(r, c);
                s << QStringLiteral("[%1 %2 %3 %4 %5]").arg(cell.covered).arg(cell.rowSpan).arg(cell.colSpan).arg(d.storyDoc(cell.storyId)->toPlainText(), cell.fill.type == Fill::Solid ? cell.fill.color.toString() : QString());
            }
    } else if (it.type() == ItemType::Group) {
        s << QStringLiteral("%1 children").arg(static_cast<const GroupItem &>(it).children.size());
    }
    return s.join(QLatin1Char('|'));
}

// The first difference between two presentations that should be alike (empty when there is none).
QString odpDifference(const Document &a, const Document &b)
{
    if (a.pages.size() != b.pages.size()) return QStringLiteral("slides %1 and %2").arg(a.pages.size()).arg(b.pages.size());
    if (a.pageSize() != b.pageSize()) return QStringLiteral("slide size");
    for (int i = 0; i < a.pages.size(); ++i) {
        const Page &x = *a.pages[i], &y = *b.pages[i];
        const QString at = QStringLiteral("slide %1: ").arg(i + 1);
        if (x.hidden != y.hidden) return at + QStringLiteral("hidden");
        if (x.showMasterItems != y.showMasterItems) return at + QStringLiteral("master items");
        if (x.title != y.title) return at + QStringLiteral("title \"%1\" and \"%2\"").arg(x.title, y.title);
        if (!(deck::transitionOf(x) == deck::transitionOf(y))) return at + QStringLiteral("transition");
        if (deck::notesText(a, x) != deck::notesText(b, y)) return at + QStringLiteral("notes");
        if (x.background.toJson() != y.background.toJson()) return at + QStringLiteral("background");
        QStringList da, db;
        walkItems(x.items, [&](const ItemPtr &it) { da << describeItem(a, *it); });
        walkItems(y.items, [&](const ItemPtr &it) { db << describeItem(b, *it); });
        if (da.size() != db.size()) return at + QStringLiteral("%1 objects and %2:\n%3\n%4").arg(da.size()).arg(db.size()).arg(da.join(QLatin1Char('\n')), db.join(QLatin1Char('\n')));
        for (int k = 0; k < da.size(); ++k)
            if (da[k] != db[k]) return at + QStringLiteral("object %1:\n%2\n%3").arg(k + 1).arg(da[k], db[k]);
    }
    return QString();
}

// An attribute of an element by the name it has there ("draw:name").
QString attrOf(const QDomElement &e, const char *qualified)
{
    static const QHash<QString, QString> spaces = {
        {QStringLiteral("draw"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0")}, {QStringLiteral("text"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:text:1.0")},
        {QStringLiteral("presentation"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:presentation:1.0")}, {QStringLiteral("smil"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0")},
        {QStringLiteral("office"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:office:1.0")}};
    const QString q = QString::fromLatin1(qualified);
    return e.attributeNS(spaces.value(q.section(QLatin1Char(':'), 0, 0)), q.section(QLatin1Char(':'), 1));
}

} // namespace odptest

class DeckOdpTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // Presentation files open through one function that looks at the file, not
    // its name: the old binary format by its compound file signature.
    void presentationFilesOpenByTheirFormat()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("talk.ppt"));
        QFile out(path);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write(buildPpt());
        out.close();
        QString err;
        auto d = deck::importAnyPresentationFile(path, &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);

        // The name doesn't decide it.
        const QString renamed = dir.filePath(QStringLiteral("talk.pptx"));
        QVERIFY(QFile::copy(path, renamed));
        auto again = deck::importAnyPresentationFile(renamed, &err);
        QVERIFY2(again, qPrintable(err));
        QCOMPARE(int(again->pages.size()), 2);

        // A newer presentation goes to the reader for its format; other files say they aren't presentations.
        err.clear();
        QFile junk(dir.filePath(QStringLiteral("junk.pptx")));
        QVERIFY(junk.open(QIODevice::WriteOnly));
        junk.write("PK nothing");
        junk.close();
        QVERIFY(!deck::importAnyPresentationFile(junk.fileName(), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::importAnyPresentationFile(dir.filePath(QStringLiteral("missing.ppt")), &err));
        QVERIFY(!err.isEmpty());
    }

    // Open document presentations open by what they are, not what they are called: the type the ZIP starts
    // with says so, a template is a presentation too, and another kind of open document says what it is.
    void openDocumentPresentationsOpenByTheirType()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto write = [&](const QString &name, const QByteArray &bytes) {
            QFile f(dir.filePath(name));
            if (!f.open(QIODevice::WriteOnly)) return QString();
            f.write(bytes);
            return f.fileName();
        };
        const QByteArray odp = odptest::sampleOdp();
        QString err;
        auto d = deck::importAnyPresentationFile(write(QStringLiteral("talk.odp"), odp), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 4);
        QVERIFY(d->fromFile);
        // The name doesn't decide it, and a template is opened as a new presentation.
        d = deck::importAnyPresentationFile(write(QStringLiteral("talk.pptx"), odp), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 4);
        const QByteArray template_ = odptest::odpZip(odptest::sampleContent(), odptest::odpStyles(), {}, "application/vnd.oasis.opendocument.presentation-template");
        d = deck::importAnyPresentationFile(write(QStringLiteral("design.otp"), template_), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 4);
        // One whose type is not the first entry of its ZIP (or is deflated) is found by its parts.
        const QByteArray late = odptest::odpZip(odptest::sampleContent(), odptest::odpStyles(), {{QStringLiteral("Pictures/a.png"), odptest::png(8, 8, Qt::red)}},
                                                 "application/vnd.oasis.opendocument.presentation", QString(), false);
        QVERIFY(!deck::looksLikeOpenDocument(late.left(128)));
        d = deck::importAnyPresentationFile(write(QStringLiteral("late.odp"), late), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 4);
        // What was left out is said, one note a line.
        QVERIFY2(err.contains(QStringLiteral("animations")), qPrintable(err));
        // Other kinds of open document, and the other formats' files, go where they go.
        const QByteArray text = odptest::odpZip(odptest::sampleContent(), odptest::odpStyles(), {}, "application/vnd.oasis.opendocument.text");
        err.clear();
        QVERIFY(!deck::importAnyPresentationFile(write(QStringLiteral("letter.odt"), text), &err));
        QVERIFY2(err.contains(QStringLiteral("text document")), qPrintable(err));
        QVERIFY(deck::looksLikeOpenDocument(odp.left(128)));
        QVERIFY(!deck::looksLikeOpenDocument(buildPpt().left(128)));
        QVERIFY(!deck::looksLikeOpenDocument(pack(samplePptx()).left(128)));
    }

    // The page size is the slide size; the master page is a slide master with its drawing, background,
    // and placeholders; its layouts are the ones the slides use and the four every presentation has.
    void odpReadsSizeMastersAndLayouts()
    {
        QString err;
        QStringList skipped;
        auto d = deck::importOdp(odptest::sampleOdp(), &err, &skipped);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(d->slideText);
        QVERIFY(std::abs(d->pageSize().width() - 28 / 2.54 * 72) < 0.01);
        QVERIFY(std::abs(d->pageSize().height() - 15.75 / 2.54 * 72) < 0.01);
        QCOMPARE(d->setup.sheet, d->pageSize());
        QCOMPARE(d->setup.sizeName, QStringLiteral("Custom"));
        QCOMPARE(d->props.title, QStringLiteral("Quarterly"));
        QCOMPARE(d->props.author, QStringLiteral("Pat"));

        QCOMPARE(int(d->masters.size()), 5);
        const MasterPage &master = *d->masters[0];
        QVERIFY(deck::isSlideMaster(master));
        QCOMPARE(master.name, QStringLiteral("Warm"));
        QCOMPARE(master.background.type, Fill::Solid);
        QCOMPARE(master.background.color.rgbValue(), QColor(0xFF, 0xF2, 0xCC));
        // Its band, then its prompts: the title and the outline (the slide master's body), the date, footer, and number.
        QStringList roles;
        for (const auto &it : master.items) roles << it->role;
        QCOMPARE(roles, (QStringList{QString(), "ph:title", "ph:body:1", "ph:dt", "ph:ftr", "ph:sldNum"}));
        QCOMPARE(master.items[0]->type(), ItemType::Shape);
        QCOMPARE(master.items[0]->fill.color.rgbValue(), QColor(0xC5, 0x5A, 0x11));
        QVERIFY(std::abs(master.items[0]->rect.y() - 15.25 / 2.54 * 72) < 0.01);
        QVERIFY(std::abs(master.items[1]->rect.x() - 1.4 / 2.54 * 72) < 0.01);
        QVERIFY(std::abs(master.items[1]->rect.width() - 25.199 / 2.54 * 72) < 0.01);
        QCOMPARE(textOf(*d, *master.items[1]), QStringLiteral("Click to edit Master title style"));
        QCOMPARE(static_cast<const TextItem *>(master.items[1].get())->valign, VAlign::Middle);

        QStringList types;
        for (MasterPage *l : deck::layouts(*d)) {
            QCOMPARE(l->baseId, master.id);
            types << deck::layoutType(*l);
        }
        QCOMPARE(types, (QStringList{"title", "obj", "titleOnly", "blank"}));
        // The slides stand on the layouts their placeholders make.
        QCOMPARE(deck::layoutType(*d->masterFor(*d->pages[0])), QStringLiteral("title"));
        QCOMPARE(deck::layoutType(*d->masterFor(*d->pages[1])), QStringLiteral("obj"));
        QCOMPARE(deck::layoutType(*d->masterFor(*d->pages[2])), QStringLiteral("blank"));
        QCOMPARE(deck::slideMasterOf(*d, 1), master.id);
        QCOMPARE(deck::layoutOfType(*d, QStringLiteral("obj"))->items[0]->role, QStringLiteral("ph:title"));
        QCOMPARE(deck::layoutOfType(*d, QStringLiteral("title"))->items[0]->role, QStringLiteral("ph:ctrTitle"));
        // A new slide on a layout has its placeholders, with the formatting of the master's.
        const int added = deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        QStringList made;
        for (const auto &it : d->pages[added]->items) made << deck::placeholderType(*it);
        QVERIFY(made.contains(QStringLiteral("title")) && made.contains(QStringLiteral("obj")));
        // The notes pages' size, and the notes.
        QVERIFY(std::abs(d->appData.value(QStringLiteral("notesSize")).toObject().value(QStringLiteral("w")).toDouble() - 21.59 / 2.54 * 72) < 0.01);
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("Say hello first\nThen the numbers"));
        // The theme's fonts are the title's and the outline's.
        QCOMPARE(deck::themeOf(*d).headingFont, QStringLiteral("Trebuchet MS"));
        // A slide of its own look: hidden, on a background of its own, without the master's drawing.
        QVERIFY(d->pages[3]->hidden);
        QVERIFY(!d->pages[3]->showMasterItems);
        QCOMPARE(d->pages[3]->background.color.rgbValue(), QColor(0x11, 0x22, 0x33));
        QVERIFY(!d->pages[1]->hidden && d->pages[1]->showMasterItems && d->pages[1]->background.isNone());
        QRegularExpressionMatch lost = QRegularExpression(QStringLiteral("animations of (\\d+) slides")).match(skipped.join(QLatin1Char('\n')));
        QVERIFY2(lost.hasMatch() && lost.captured(1) == QStringLiteral("1"), qPrintable(skipped.join(QLatin1Char('\n'))));
    }

    // A custom shape's equation nested thousands deep (a damaged or hostile
    // file) reads as 0 rather than running out of stack; the shape is kept.
    void odpFormulasNestOnlySoDeep()
    {
        for (const QString &formula : {QString(1000, QLatin1Char('(')) + QStringLiteral("1"), QString(100000, QLatin1Char('(')) + QStringLiteral("1"), QString(300000, QLatin1Char('-')) + QStringLiteral("1"),
                                       QStringLiteral("abs(").repeated(50000) + QStringLiteral("1")}) {
            const QString page = QStringLiteral("<draw:page draw:name=\"p1\" draw:master-page-name=\"Default\">"
                                                "<draw:custom-shape svg:width=\"4cm\" svg:height=\"4cm\" svg:x=\"1cm\" svg:y=\"1cm\">"
                                                "<draw:enhanced-geometry svg:viewBox=\"0 0 21600 21600\" draw:enhanced-path=\"M 0 0 L ?f0 0 21600 21600 Z N\">"
                                                "<draw:equation draw:name=\"f0\" draw:formula=\"")
                                 + formula + QStringLiteral("\"/></draw:enhanced-geometry></draw:custom-shape></draw:page>");
            QString err;
            auto d = deck::importOdp(odptest::odpZip(odptest::odpContent(QString(), page)), &err);
            QVERIFY2(d, qPrintable(err));
            QCOMPARE(int(d->pages.size()), 1);
            QCOMPARE(int(d->pages[0]->items.size()), 1);
        }
    }

    // Text takes what its styles give it: the placeholder's presentation style, the automatic styles of
    // paragraphs and spans, the sizes in percent, and the bullets and numbers of the list styles.
    void odpReadsTextAndLists()
    {
        QString err;
        auto d = deck::importOdp(odptest::sampleOdp(), &err);
        QVERIFY2(d, qPrintable(err));
        // The title slide: a title with a subtitle beside it is the centered one.
        const Page &first = *d->pages[0];
        QCOMPARE(int(first.items.size()), 2);
        QCOMPARE(first.items[0]->role, QStringLiteral("ph:ctrTitle"));
        QCOMPARE(first.items[1]->role, QStringLiteral("ph:subTitle:1"));
        QCOMPARE(first.title, QStringLiteral("Quarterly results"));
        QCOMPARE(textOf(*d, *first.items[0]), QStringLiteral("Quarterly results"));
        const QTextDocument *title = d->storyDoc(static_cast<const TextItem *>(first.items[0].get())->storyId);
        QTextBlock block = title->begin();
        QCOMPARE(block.blockFormat().alignment(), Qt::AlignHCenter);
        QVector<QTextFragment> fragments;
        for (auto it = block.begin(); !it.atEnd(); ++it) fragments << it.fragment();
        QCOMPARE(int(fragments.size()), 2);
        const QTextCharFormat a = fragments[0].charFormat(), b = fragments[1].charFormat();
        QCOMPARE(a.fontPointSize(), 44.0);
        QCOMPARE(a.fontWeight(), int(QFont::Bold));
        QCOMPARE(a.fontFamilies().toStringList(), QStringList{"Trebuchet MS"});
        QCOMPARE(a.foreground().color(), QColor(0x1F, 0x38, 0x64));
        QVERIFY(!a.fontItalic());
        QVERIFY(b.fontItalic());
        QCOMPARE(b.underlineStyle(), QTextCharFormat::SingleUnderline);
        QCOMPARE(b.foreground().color(), QColor(Qt::red));
        QCOMPARE(b.fontPointSize(), 44.0);
        QCOMPARE(static_cast<const TextItem *>(first.items[0].get())->valign, VAlign::Middle);
        // Text with no color of its own is black on a light page.
        const QTextDocument *sub = d->storyDoc(static_cast<const TextItem *>(first.items[1].get())->storyId);
        QCOMPARE(sub->begin().begin().fragment().charFormat().foreground().color(), QColor(Qt::black));
        QCOMPARE(sub->begin().begin().fragment().charFormat().fontPointSize(), 32.0);

        // The second slide: the outline's bullets, in levels, and what follows the list.
        const Page &second = *d->pages[1];
        QCOMPARE(second.items[1]->role, QStringLiteral("ph:obj:1"));
        const QTextDocument *outline = d->storyDoc(static_cast<const TextItem *>(second.items[1].get())->storyId);
        QCOMPARE(outline->blockCount(), 3);
        QTextBlock one = outline->begin(), two = one.next(), three = two.next();
        QCOMPARE(one.text(), QStringLiteral("First point"));
        QCOMPARE(two.text(), QStringLiteral("Second point"));
        QCOMPARE(three.text(), QStringLiteral("Plain third"));
        QVERIFY(one.textList());
        QVERIFY(two.textList());
        QVERIFY(!three.textList());
        // The big round bullet of the symbol font, drawn at 45%, is the usual bullet at full size.
        QCOMPARE(one.textList()->format().property(tp::BulletChar).toString(), QStringLiteral("•"));
        QCOMPARE(two.textList()->format().property(tp::BulletChar).toString(), QStringLiteral("–"));
        QCOMPARE(two.blockFormat().property(tp::ListLevel).toInt(), 1);
        // The label is at the space before; the text starts a label's width on.
        QVERIFY(std::abs(one.blockFormat().leftMargin() - 1.2 / 2.54 * 72) < 0.05);
        QVERIFY(std::abs(one.blockFormat().textIndent() + 0.9 / 2.54 * 72) < 0.05);
        QVERIFY(std::abs(two.blockFormat().leftMargin() - 2.4 / 2.54 * 72) < 0.05);
        // The outline's levels have their own sizes (32 and 28 points), the spans theirs (150%, a superscript), the paragraph's style its alignment.
        QCOMPARE(one.begin().fragment().charFormat().fontPointSize(), 32.0);
        QCOMPARE(two.begin().fragment().charFormat().fontPointSize(), 28.0);
        QCOMPARE(two.blockFormat().alignment(), Qt::AlignRight);
        QCOMPARE(two.begin().fragment().text(), QStringLiteral("Second "));
        auto lastFragment = [](const QTextBlock &blk) { QTextFragment f; for (auto it = blk.begin(); !it.atEnd(); ++it) f = it.fragment(); return f; };
        QCOMPARE(lastFragment(two).charFormat().fontPointSize(), 42.0);
        QCOMPARE(lastFragment(two).charFormat().verticalAlignment(), QTextCharFormat::AlignSuperScript);
        // Space above the paragraphs, from the outline's styles.
        QVERIFY(std::abs(one.blockFormat().topMargin() - 0.5 / 2.54 * 72) < 0.05);
        QVERIFY(std::abs(two.blockFormat().topMargin() - 0.4 / 2.54 * 72) < 0.05);

        // A text box of its own: numbers from the list style (starting at 3), and a link.
        const TextItem *box = nullptr, *link = nullptr;
        for (const auto &it : second.items)
            if (it->type() == ItemType::Text && it->role.isEmpty()) (box ? link : box) = static_cast<const TextItem *>(it.get());
        QVERIFY(box && link);
        const QTextDocument *numbers = d->storyDoc(box->storyId);
        QVERIFY(numbers->begin().textList());
        QCOMPARE(numbers->begin().textList()->format().style(), QTextListFormat::ListDecimal);
        QCOMPARE(numbers->begin().textList()->format().start(), 3);
        QCOMPARE(numbers->begin().textList()->format().property(tp::NumberFormat).toInt(), 1);
        QCOMPARE(numbers->blockCount(), 2);
        const QTextDocument *linked = d->storyDoc(link->storyId);
        // A link to a program or a file is not followed: its text stays text.
        QCOMPARE(linked->toPlainText(), QStringLiteral("Visit the site now run"));
        QStringList hrefs;
        for (auto it = linked->begin().begin(); !it.atEnd(); ++it) hrefs << it.fragment().charFormat().anchorHref();
        QCOMPARE(hrefs, (QStringList{QString(), "https://example.com/a?b=1&c=2", QString()}));
    }

    // Pictures with their crop, shapes by their type or their path, lines with arrows, groups, tables with
    // spans, turns, transitions, and the footers a slide shows.
    void odpReadsObjects()
    {
        QString err;
        auto d = deck::importOdp(odptest::sampleOdp(), &err);
        QVERIFY2(d, qPrintable(err));
        auto near = [](double a, double b) { return std::abs(a - b) < 0.05; };
        const double cm = 72 / 2.54;
        // The picture: a frame with a crop (7.5, 15, 3.75, and 30 points of a 150 by 75 point picture).
        const Page &second = *d->pages[1];
        const PictureItem *pic = nullptr;
        for (const auto &it : second.items)
            if (it->type() == ItemType::Picture) pic = static_cast<const PictureItem *>(it.get());
        QVERIFY(pic);
        QVERIFY(!pic->imageId.isEmpty());
        QCOMPARE(d->imageSize(pic->imageId), QSize(200, 100));
        QVERIFY(near(pic->rect.x(), 2 * cm) && near(pic->rect.y(), 10 * cm) && near(pic->rect.width(), 100) && near(pic->rect.height(), 50));
        QVERIFY2(near(pic->imgRect.x(), -0.2 * 100 / 0.7) && near(pic->imgRect.width(), 100 / 0.7), qPrintable(QString::number(pic->imgRect.x())));
        QVERIFY(near(pic->imgRect.y(), -0.1 * 50 / 0.85) && near(pic->imgRect.height(), 50 / 0.85));
        QVERIFY(pic->flipH);
        QVERIFY(pic->stroke.isNone());

        const Page &third = *d->pages[2];
        QVector<const Item *> items;
        for (const auto &it : third.items) items << it.get();
        // A custom shape by the type the standard names, with the fill and its opacity.
        QCOMPARE(items[0]->type(), ItemType::Shape);
        const auto *arrow = static_cast<const ShapeItem *>(items[0]);
        QCOMPARE(arrow->shape, QStringLiteral("rightArrow"));
        QCOMPARE(arrow->fill.color.rgbValue(), QColor(0x33, 0x66, 0xCC));
        QVERIFY(near(arrow->fill.transparency, 0.5));
        QVERIFY(arrow->stroke.isNone());
        QVERIFY(near(arrow->rect.x(), 1 * cm) && near(arrow->rect.width(), 4 * cm));
        QCOMPARE(static_cast<const ShapeItem *>(items[1])->shape, QStringLiteral("ellipse"));
        // Its text, which a shape keeps in a story of its own.
        QVERIFY(!static_cast<const ShapeItem *>(items[1])->storyId.isEmpty());
        QCOMPARE(d->storyDoc(static_cast<const ShapeItem *>(items[1])->storyId)->toPlainText(), QStringLiteral("Round"));
        // One whose type is no preset is drawn from its path.
        const auto *freeform = static_cast<const ShapeItem *>(items[2]);
        QVERIFY(!freeform->customPath.isEmpty());
        QVERIFY(near(freeform->customPath.boundingRect().width(), freeform->rect.width()) && near(freeform->customPath.boundingRect().height(), freeform->rect.height()));
        QVERIFY(near(freeform->rect.width(), 5 * cm) && near(freeform->rect.height(), 2.5 * cm));
        // A line with its width, color, and an arrow at its end.
        QCOMPARE(items[3]->type(), ItemType::Line);
        const auto *line = static_cast<const LineItem *>(items[3]);
        QVERIFY(near(line->p1.x(), 1 * cm) && near(line->p1.y(), 8 * cm) && near(line->p2.x(), 9 * cm) && near(line->p2.y(), 9 * cm));
        QVERIFY(near(line->stroke.width, 0.1 * cm));
        QCOMPARE(line->stroke.color.rgbValue(), QColor(0x00, 0x33, 0x00));
        QCOMPARE(line->stroke.endArrow, Arrow::Triangle);
        QCOMPARE(line->stroke.startArrow, Arrow::None);
        // A group has its objects, placed on the page.
        QCOMPARE(items[4]->type(), ItemType::Group);
        const auto *group = static_cast<const GroupItem *>(items[4]);
        QCOMPARE(int(group->children.size()), 2);
        QVERIFY(near(group->children[0]->rect.x(), 2 * cm) && near(group->children[1]->rect.x(), 5 * cm));
        QVERIFY(near(group->rect.width(), 5 * cm) && near(group->rect.height(), 3 * cm));
        // A table: its columns and rows, a merged cell, the cells' fills, borders, and margins.
        QCOMPARE(items[5]->type(), ItemType::Table);
        const auto *table = static_cast<const TableItem *>(items[5]);
        QCOMPARE(table->rows, 2);
        QCOMPARE(table->cols, 2);
        QVERIFY(near(table->colW[0], 3 * cm) && near(table->colW[1], 4 * cm) && near(table->rowH[0], 1 * cm));
        QVERIFY(near(table->rect.x(), 14 * cm) && near(table->rect.y(), 2 * cm));
        QCOMPARE(table->cell(0, 0).colSpan, 2);
        QVERIFY(table->cell(0, 1).covered);
        QVERIFY(!table->cell(1, 0).covered);
        QCOMPARE(d->storyDoc(table->cell(0, 0).storyId)->toPlainText(), QStringLiteral("Header"));
        QCOMPARE(d->storyDoc(table->cell(1, 1).storyId)->toPlainText(), QStringLiteral("b"));
        QCOMPARE(table->cell(1, 0).fill.color.rgbValue(), QColor(0xF9, 0xF9, 0xF9));
        QCOMPARE(table->cell(1, 0).border.left.color.rgbValue(), QColor(0xFF, 0x00, 0x00));
        QVERIFY(near(table->cell(1, 0).border.top.width, 0.5));
        QCOMPARE(table->cell(1, 0).valign, VAlign::Middle);
        QVERIFY(near(table->cell(1, 0).margins.left(), 0.1 * cm));
        // A rectangle turned a quarter counter-clockwise about its corner, then moved.
        QCOMPARE(items[6]->type(), ItemType::Shape);
        QVERIFY(near(items[6]->rotation, 270));
        QVERIFY(near(items[6]->rect.center().x(), 11 * cm) && near(items[6]->rect.center().y(), 3 * cm));
        QVERIFY(near(items[6]->rect.width(), 4 * cm) && near(items[6]->rect.height(), 2 * cm));

        // An embedded object is the picture the file keeps of it, with its description.
        QCOMPARE(items[7]->type(), ItemType::Picture);
        QVERIFY(!static_cast<const PictureItem *>(items[7])->imageId.isEmpty());
        QCOMPARE(items[7]->altText, QStringLiteral("A chart of sales"));
        QVERIFY(near(items[7]->rect.x(), 10 * cm) && near(items[7]->rect.width(), 4 * cm));

        // A chart the file keeps only a picture of in its own metafile format is drawn from its numbers.
        QCOMPARE(items[8]->type(), ItemType::Picture);
        const QString chartId = static_cast<const PictureItem *>(items[8])->imageId;
        QVERIFY(!chartId.isEmpty());
        QCOMPARE(d->images.value(chartId).format, QStringLiteral("svg"));
        const QString svg = QString::fromUtf8(d->images.value(chartId).bytes);
        // The picture's text, its pieces joined: macOS writes "Q2" as "Q" and "2".
        QStringList texts;
        static const QRegularExpression textEl(QStringLiteral("<text[^>]*>([^<]*)</text>"));
        for (auto m = textEl.globalMatch(svg); m.hasNext();) texts << m.next().captured(1).trimmed();
        const QString shown = texts.join(QString());
        QVERIFY2(shown.contains(QStringLiteral("Revenue")) && shown.contains(QStringLiteral("Q2")) && shown.contains(QStringLiteral("Sales by quarter")),
                 qPrintable(QStringLiteral("%1 bytes, text: %2").arg(svg.size()).arg(texts.join(QStringLiteral(" | ")))));

        // A picture fill that doesn't say how it fills is tiled.
        QCOMPARE(items[9]->fill.type, Fill::Texture);
        QVERIFY(items[9]->fill.tile && !items[9]->fill.imageId.isEmpty());

        // How the slide changes to the next, and when it moves on.
        const deck::Transition t = deck::transitionOf(third);
        QCOMPARE(t.type, QStringLiteral("push"));
        QCOMPARE(t.dir, QStringLiteral("r"));
        QVERIFY(near(t.duration, 1.0));
        QVERIFY(t.advanceAfter && near(*t.advanceAfter, 5));
        QVERIFY(!t.advanceOnClick);
        QCOMPARE(deck::transitionOf(*d->pages[1]), deck::Transition());
        // The footer and number the slide shows are in the places its layout has for them.
        QStringList placeholders;
        for (const auto &it : third.items) placeholders << deck::placeholderType(*it);
        QVERIFY(placeholders.contains(QStringLiteral("ftr")) && placeholders.contains(QStringLiteral("sldNum")) && !placeholders.contains(QStringLiteral("dt")));
        for (const auto &it : third.items)
            if (deck::placeholderType(*it) == QLatin1String("ftr")) QCOMPARE(textOf(*d, *it), QStringLiteral("Confidential"));
        QVERIFY(d->appData.value(QStringLiteral("footers")).toBool());
    }

    // A range of cells the file names past its table (a chart that names a million rows of a table of three) is read as far as the table goes.
    void odpChartCategoryRangeStaysInItsTable()
    {
        QString err;
        const QByteArray chart = odptest::chartContent(QStringLiteral("chart:bar"), {QStringLiteral("North"), QStringLiteral("South"), QStringLiteral("West")}, {{4, 8, 6}}, QString(), QString(),
                                                       QString(), QString(), QStringLiteral("local-table.$A$2:.$A$3000"));
        auto d = deck::importOdp(odptest::chartDeck(chart, {QSizeF(300, 200)}), &err);
        QVERIFY2(d, qPrintable(err));
        const QVector<QByteArray> svgs = chartPictures(*d);
        QCOMPARE(svgs.size(), 1);
        QVERIFY2(svgs[0].count('<') < 700, qPrintable(QString::number(svgs[0].count('<'))));
        const QStringList texts = textsOf(svgs[0]);
        QVERIFY2(texts.join(QString()).contains(QStringLiteral("North")), qPrintable(texts.join(QLatin1Char('|'))));
        // The values' own range, and the one that names categories for series with none, are bounded the same way.
        const QByteArray values = odptest::chartContent(QStringLiteral("chart:bar"), {QStringLiteral("North"), QStringLiteral("South")}, {{4, 8}}, QString(), QString(), QString(), QString(), QStringLiteral("local-table.$A$2:.$A$3"),
                                                        QStringLiteral("<chart:axis chart:dimension=\"x\" chart:name=\"primary-x\"/><chart:axis chart:dimension=\"y\" chart:name=\"primary-y\"/>"));
        QString widened = QString::fromUtf8(values);
        widened.replace(QStringLiteral("$B$3\""), QStringLiteral("$B$300000\""));
        d = deck::importOdp(odptest::chartDeck(widened.toUtf8(), {QSizeF(300, 200)}), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(chartPictures(*d).value(0).count('<') < 700, qPrintable(QString::number(chartPictures(*d).value(0).count('<'))));
    }

    // A scatter chart's points stand at the x values of the series' domain, not at 1, 2, 3.
    void odpChartScatterUsesItsDomain()
    {
        // The table as the other suite writes it: a column of y values (and the series' name above it), and the column of x values beside it.
        const QString content = QStringLiteral("<?xml version=\"1.0\"?><office:document-content") + odptest::kNs + QStringLiteral("><office:automatic-styles/><office:body><office:chart>"
            "<chart:chart chart:class=\"chart:scatter\"><chart:plot-area><chart:axis chart:dimension=\"x\" chart:name=\"primary-x\"/><chart:axis chart:dimension=\"y\" chart:name=\"primary-y\"/>"
            "<chart:series chart:values-cell-range-address=\"local-table.$B$2:.$B$5\" chart:label-cell-address=\"local-table.$B$1\" chart:class=\"chart:scatter\"><chart:domain table:cell-range-address=\"local-table.$C$2:.$C$5\"/></chart:series>"
            "</chart:plot-area><table:table table:name=\"local-table\"><table:table-header-rows><table:table-row><table:table-cell/><table:table-cell office:value-type=\"string\"><text:p>S</text:p></table:table-cell>"
            "<table:table-cell office:value-type=\"string\"><text:p>X</text:p></table:table-cell></table:table-row></table:table-header-rows><table:table-rows>"
            "<table:table-row><table:table-cell/><table:table-cell office:value-type=\"float\" office:value=\"1\"/><table:table-cell office:value-type=\"float\" office:value=\"10\"/></table:table-row>"
            "<table:table-row><table:table-cell/><table:table-cell office:value-type=\"float\" office:value=\"2\"/><table:table-cell office:value-type=\"float\" office:value=\"20\"/></table:table-row>"
            "<table:table-row><table:table-cell/><table:table-cell office:value-type=\"float\" office:value=\"3\"/><table:table-cell office:value-type=\"float\" office:value=\"40\"/></table:table-row>"
            "<table:table-row><table:table-cell/><table:table-cell office:value-type=\"float\" office:value=\"2.5\"/><table:table-cell office:value-type=\"float\" office:value=\"80\"/></table:table-row>"
            "</table:table-rows></table:table></chart:chart></office:chart></office:body></office:document-content>");
        QString err;
        auto d = deck::importOdp(odptest::chartDeck(content.toUtf8(), {QSizeF(300, 200)}), &err);
        QVERIFY2(d, qPrintable(err));
        const QVector<QByteArray> svgs = chartPictures(*d);
        QCOMPARE(svgs.size(), 1);
        const QStringList texts = textsOf(svgs[0]);
        // The horizontal axis runs to 90 for x values to 80, and to 4.5 for the numbers 1 to 4.
        QVERIFY2(texts.contains(QStringLiteral("80")) && texts.contains(QStringLiteral("90")) && !texts.contains(QStringLiteral("4.5")), qPrintable(texts.join(QLatin1Char('|'))));
    }

    // A series that has no color of its own takes the next of the palette, as a slice of a pie does; it isn't black.
    void odpChartSeriesWithoutColorTakeThePalette()
    {
        const QByteArray chart = odptest::chartContent(QStringLiteral("chart:bar"), {QStringLiteral("a"), QStringLiteral("b")}, {{1, 2}, {2, 3}, {3, 1}});
        QString err;
        auto d = deck::importOdp(odptest::chartDeck(chart, {QSizeF(300, 200)}), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray svg = chartPictures(*d).value(0);
        QVERIFY(!svg.isEmpty());
        QVERIFY2(svg.contains("fill=\"#004586\"") && svg.contains("fill=\"#ff420e\"") && svg.contains("fill=\"#ffd320\""), qPrintable(QString::fromUtf8(svg.left(1500))));
        QVERIFY2(!svg.contains("<g fill=\"#000000\""), "a series is drawn black");
        // A series with a color of its own keeps it.
        const QByteArray own = odptest::chartContent(QStringLiteral("chart:bar"), {QStringLiteral("a"), QStringLiteral("b")}, {{1, 2}, {2, 3}},
                                                    QStringLiteral("<style:style style:name=\"ch2\" style:family=\"chart\"><style:graphic-properties draw:fill-color=\"#123456\"/></style:style>"));
        QString withStyle = QString::fromUtf8(own);
        withStyle.replace(QStringLiteral("chart:class=\"chart:bar\" >"), QStringLiteral("chart:class=\"chart:bar\" chart:style-name=\"ch2\">"));
        d = deck::importOdp(odptest::chartDeck(withStyle.toUtf8(), {QSizeF(300, 200)}), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray two = chartPictures(*d).value(0);
        QVERIFY(two.contains("fill=\"#123456\""));
        QVERIFY(!two.contains("<g fill=\"#000000\""));
    }

    // A chart object that frames name again and again is read and drawn once for each size, and a presentation draws only so many.
    void odpChartFramesShareTheirDrawingAndAreLimited()
    {
        const QByteArray chart = odptest::chartContent(QStringLiteral("chart:bar"), {QStringLiteral("a"), QStringLiteral("b")}, {{1, 2}});
        QString err;
        QStringList notes;
        // The same frame 500 times: one drawing, and every frame has its picture.
        auto d = deck::importOdp(odptest::chartDeck(chart, QVector<QSizeF>(500, QSizeF(300, 8))), &err, &notes);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(chartPictures(*d).size(), 500);
        QCOMPARE(d->images.size(), 1);
        QVERIFY2(notes.join(QLatin1Char('|')).contains(QStringLiteral("300")) == false, qPrintable(notes.join(QLatin1Char('|'))));
        // A thousand sizes: the first 300 are drawn, the rest left out, with a note.
        QVector<QSizeF> sizes;
        for (int i = 0; i < 1000; ++i) sizes << QSizeF(300 + i, 8);
        notes.clear();
        d = deck::importOdp(odptest::chartDeck(chart, sizes), &err, &notes);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(chartPictures(*d).size(), 300);
        QVERIFY2(notes.join(QLatin1Char('|')).contains(QStringLiteral("300")), qPrintable(notes.join(QLatin1Char('|'))));
    }

    // A chart with more points than a picture can show is drawn from some of them, and the file's notes say so.
    void odpChartWithMorePointsThanAPictureCanShowSaysSo()
    {
        QStringList names;
        QVector<double> values;
        for (int i = 0; i < 6000; ++i) {
            names << QStringLiteral("r%1").arg(i);
            values << i % 70;
        }
        QString err;
        QStringList notes;
        auto d = deck::importOdp(odptest::chartDeck(odptest::chartContent(QStringLiteral("chart:line"), names, {values}), {QSizeF(400, 300)}), &err, &notes);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(notes.join(QLatin1Char('|')).contains(QStringLiteral("more points")));
        QCOMPARE(chartPictures(*d).size(), 1);
        notes.clear();
        d = deck::importOdp(odptest::chartDeck(odptest::chartContent(QStringLiteral("chart:line"), names.mid(0, 50), {values.mid(0, 50)}), {QSizeF(400, 300)}), &err, &notes);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(!notes.join(QLatin1Char('|')).contains(QStringLiteral("more points")), qPrintable(notes.join(QLatin1Char('|'))));
    }

    // A frame is drawn no larger than a slide can be, however large the file says it is.
    void odpChartFrameSizeIsClamped()
    {
        const QByteArray chart = odptest::chartContent(QStringLiteral("chart:bar"), {QStringLiteral("a"), QStringLiteral("b")}, {{1, 2}});
        QString err;
        auto d = deck::importOdp(odptest::chartDeck(chart, {QSizeF(900000, 900000)}), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray svg = chartPictures(*d).value(0);
        QVERIFY2(svg.contains("viewBox=\"0 0 4032 4032\""), qPrintable(QString::fromUtf8(svg.left(400))));
    }

    // A font name in a chart's style, with the characters an SVG attribute can't hold, doesn't make the chart vanish.
    void odpChartFontNameCannotBreakTheChart()
    {
        const QByteArray chart = odptest::chartContent(QStringLiteral("chart:bar"), {QStringLiteral("a"), QStringLiteral("b")}, {{1, 2}},
                                                       QStringLiteral("<style:style style:name=\"axs\" style:family=\"chart\"><style:text-properties fo:font-family=\"A&lt;B&amp;C&gt;D\" fo:font-size=\"10pt\"/></style:style>"),
                                                       QString(), QString(), QString(), QString(),
                                                       QStringLiteral("<chart:axis chart:dimension=\"x\" chart:name=\"primary-x\" chart:style-name=\"axs\"><chart:categories table:cell-range-address=\"local-table.$A$2:.$A$3\"/></chart:axis>"
                                                                      "<chart:axis chart:dimension=\"y\" chart:name=\"primary-y\"/>"));
        QString err;
        auto d = deck::importOdp(odptest::chartDeck(chart, {QSizeF(300, 200)}), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray svg = chartPictures(*d).value(0);
        QVERIFY(!svg.isEmpty());
        QXmlStreamReader xml(svg);
        while (!xml.atEnd()) xml.readNext();
        QVERIFY2(!xml.hasError(), qPrintable(xml.errorString()));
        QVERIFY(textsOf(svg).contains(QStringLiteral("a")));
    }

    // A chart with more points than a picture can show is drawn from some of them, and the file's notes say so.
    void pptxChartWithMorePointsThanAPictureCanShowSaysSo()
    {
        auto withPoints = [](int n) {
            QString points;
            for (int i = 0; i < n; ++i) points += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(i % 90);
            Parts p = chartPptx();
            p.data["/ppt/charts/chart1.xml"].xml = (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" "
                "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><c:chart><c:autoTitleDeleted val=\"1\"/>"
                "<c:plotArea><c:layout/><c:lineChart><c:grouping val=\"standard\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:val><c:numRef><c:f>Sheet1!$B$2</c:f><c:numCache><c:formatCode>General</c:formatCode>"
                "<c:ptCount val=\"%1\"/>%2</c:numCache></c:numRef></c:val></c:ser><c:marker val=\"0\"/><c:axId val=\"1\"/><c:axId val=\"2\"/></c:lineChart>"
                "<c:catAx><c:axId val=\"1\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:crossAx val=\"2\"/></c:catAx>"
                "<c:valAx><c:axId val=\"2\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:crossAx val=\"1\"/></c:valAx></c:plotArea></c:chart>"
                "<c:externalData r:id=\"rId1\"><c:autoUpdate val=\"0\"/></c:externalData><c:userShapes r:id=\"rId2\"/></c:chartSpace>")
                                                         .arg(n).arg(points)).toUtf8();
            return pack(p);
        };
        QString err;
        auto d = deck::importPresentation(withPoints(9000), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(chartPicture(*d, *d->pages[0]));
        QVERIFY2(err.contains(QStringLiteral("Chart 8")) && err.contains(QStringLiteral("more points")), qPrintable(err));
        err.clear();
        d = deck::importPresentation(withPoints(90), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(!err.contains(QStringLiteral("more points")), qPrintable(err));
    }

    // A chart frame of a slide is drawn no larger than a slide can be.
    void pptxChartFrameSizeIsClamped()
    {
        Parts p = chartPptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QVERIFY(xml.contains(QStringLiteral("<a:ext cx=\"3657600\" cy=\"2743200\"/>")));
        xml.replace(QStringLiteral("<a:ext cx=\"3657600\" cy=\"2743200\"/>"), QStringLiteral("<a:ext cx=\"9000000000000\" cy=\"700000000000\"/>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const auto *pic = chartPicture(*d, *d->pages[0]);
        QVERIFY(pic);
        QCOMPARE(pic->rect.size(), QSizeF(4032, 4032));
        QCOMPARE(pic->imgRect, QRectF(0, 0, 4032, 4032));
        const QByteArray svg = d->images.value(pic->imageId).bytes;
        QVERIFY2(svg.contains("viewBox=\"0 0 4032 4032\""), qPrintable(QString::fromUtf8(svg.left(400))));
    }

    // A presentation saved with no slides (a template can be, and so can a
    // script's empty one) opens with one blank Title Slide, as a new one
    // starts, rather than as a window with nothing to show.
    void emptyPresentationsOpenWithASlide()
    {
        QString err;
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(deck::exportPresentation(*deck::newPresentation()), zip, &err));
        QString pres = QString::fromUtf8(zip.value(QStringLiteral("ppt/presentation.xml")));
        pres.replace(QRegularExpression(QStringLiteral("<p:sldIdLst>.*</p:sldIdLst>")), QString());
        QVERIFY(!pres.contains(QLatin1String("sldId")));
        zip[QStringLiteral("ppt/presentation.xml")] = pres.toUtf8();
        ZipWriter z;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
        auto pptx = deck::importPresentation(z.finish(), &err);
        QVERIFY2(pptx, qPrintable(err));
        QCOMPARE(pptx->pages.size(), 1);
        QCOMPARE(deck::layoutType(*pptx->masterFor(*pptx->pages[0])), QStringLiteral("title"));
        // An .odp with no pages, the same.
        auto odp = deck::importOdp(odptest::odpZip(odptest::odpContent(QString(), QString())), &err);
        QVERIFY2(odp, qPrintable(err));
        QCOMPARE(odp->pages.size(), 1);
    }

    // Damaged and hostile files are refused, or read as far as they go, and say why.
    void odpSurvivesHostileFiles()
    {
        QString err;
        QStringList skipped;
        QVERIFY(!deck::importOdp(QByteArray("this is not a package"), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::importOdp(QByteArray(), &err));
        QVERIFY(!err.isEmpty());
        // A package with no presentation in it, with parts that aren't XML, and with the wrong root.
        err.clear();
        QVERIFY(!deck::importOdp(odptest::odpZip(odptest::sampleContent(), QString::fromLatin1("<<< not xml")), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::importOdp(odptest::odpZip(QString::fromLatin1("<<< not xml")), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::importOdp(odptest::odpZip(QStringLiteral("<a/>")), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::importOdp(odptest::odpZip(odptest::sampleContent(), odptest::odpStyles().replace(QStringLiteral("<style:master-page"), QStringLiteral("<style:no-page"))
                                                                       .replace(QStringLiteral("</style:master-page>"), QStringLiteral("</style:no-page>"))), &err));
        QVERIFY(!err.isEmpty());
        // A DOCTYPE (an entity it could define) is refused.
        err.clear();
        QString doctype = odptest::sampleContent();
        doctype.insert(doctype.indexOf(QLatin1String("<office:document-content")), QStringLiteral("<!DOCTYPE x [<!ENTITY e \"boom\">]>"));
        QVERIFY(!deck::importOdp(odptest::odpZip(doctype), &err));
        QVERIFY(!err.isEmpty());
        // A presentation protected with a password is told apart from damage.
        err.clear();
        const QString encrypted = QStringLiteral("<?xml version=\"1.0\"?><manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">"
                                                 "<manifest:file-entry manifest:full-path=\"content.xml\"><manifest:encryption-data manifest:checksum-type=\"x\"/></manifest:file-entry></manifest:manifest>");
        QVERIFY(!deck::importOdp(odptest::odpZip(odptest::sampleContent(), odptest::odpStyles(), {}, "application/vnd.oasis.opendocument.presentation", encrypted), &err));
        QVERIFY2(err.contains(QStringLiteral("password")), qPrintable(err));
        // A part named to climb out of the package is left out, and the rest opens.
        err.clear();
        QVERIFY2(deck::importOdp(odptest::odpZip(odptest::sampleContent(), odptest::odpStyles(), {{QStringLiteral("../evil.xml"), "x"}}), &err), qPrintable(err));
        // A slide whose master page isn't there stands on the first one.
        {
            QString content = odptest::sampleContent();
            content.replace(QStringLiteral("draw:name=\"page1\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\""), QStringLiteral("draw:name=\"page1\" draw:style-name=\"dp1\" draw:master-page-name=\"Gone\""));
            err.clear();
            auto d = deck::importOdp(odptest::odpZip(content), &err, &skipped);
            QVERIFY2(d, qPrintable(err));
            QCOMPARE(int(d->pages.size()), 4);
        }
        // Pictures that aren't in the package, or aren't pictures, leave their frames out, with a note.
        {
            err.clear();
            auto d = deck::importOdp(odptest::odpZip(odptest::sampleContent(), odptest::odpStyles(), {}), &err, &skipped);
            QVERIFY(d);
            QVERIFY(skipped.join(QLatin1Char('\n')).contains(QStringLiteral("Pictures")));
            auto garbage = deck::importOdp(odptest::odpZip(odptest::sampleContent(), odptest::odpStyles(), {{QStringLiteral("Pictures/a.png"), "not a picture at all"}}), &err, &skipped);
            QVERIFY(garbage);
            QVERIFY(skipped.join(QLatin1Char('\n')).contains(QStringLiteral("Pictures")));
        }
        // Huge counts: a table of a billion repeated columns and rows is cut to what a table is kept to, and so are the slides.
        {
            QString pages = QStringLiteral("<draw:page draw:name=\"p\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\"><draw:frame svg:width=\"10cm\" svg:height=\"5cm\" svg:x=\"1cm\" svg:y=\"1cm\">"
                                           "<table:table><table:table-column table:number-columns-repeated=\"1000000000\"/><table:table-row table:number-rows-repeated=\"1000000000\">"
                                           "<table:table-cell table:number-columns-repeated=\"1000000000\"><text:p>x</text:p></table:table-cell></table:table-row></table:table></draw:frame></draw:page>");
            err.clear();
            auto d = deck::importOdp(odptest::odpZip(odptest::odpContent(odptest::sampleAutomaticStyles(), pages)), &err, &skipped);
            // 256 columns and 256 rows are more than a table holds of cells here: its first 39 rows (of 9,984 cells) are kept, with the notes.
            QVERIFY2(d, qPrintable(err));
            QCOMPARE(int(d->pages.size()), 1);
            QCOMPARE(int(d->pages[0]->items.size()), 1);
            QCOMPARE(d->pages[0]->items[0]->type(), ItemType::Table);
            QCOMPARE(static_cast<const TableItem *>(d->pages[0]->items[0].get())->rows, 39);
            QCOMPARE(static_cast<const TableItem *>(d->pages[0]->items[0].get())->cols, 256);
            QVERIFY(skipped.join(QLatin1Char('\n')).contains(QStringLiteral("256 rows and columns")) && skipped.join(QLatin1Char('\n')).contains(QStringLiteral("10,000 cells")));
            QString many;
            for (int i = 0; i < 10001; ++i) many += QStringLiteral("<draw:page draw:name=\"p%1\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\"/>").arg(i);
            err.clear();
            d = deck::importOdp(odptest::odpZip(odptest::odpContent(odptest::sampleAutomaticStyles(), many)), &err, &skipped);
            QVERIFY2(d, qPrintable(err));
            QCOMPARE(int(d->pages.size()), 10000);
            QVERIFY(skipped.join(QLatin1Char('\n')).contains(QStringLiteral("10,000")));
        }
        // Deep nesting: groups inside groups, and lists inside lists, are cut at a depth, and XML nested too deeply is refused.
        {
            QString groups, lists;
            for (int i = 0; i < 200; ++i) groups += QStringLiteral("<draw:g>");
            groups += QStringLiteral("<draw:rect svg:width=\"1cm\" svg:height=\"1cm\" svg:x=\"0cm\" svg:y=\"0cm\"/>");
            for (int i = 0; i < 200; ++i) groups += QStringLiteral("</draw:g>");
            for (int i = 0; i < 100; ++i) lists += QStringLiteral("<text:list><text:list-item>");
            lists += QStringLiteral("<text:p>deep</text:p>");
            for (int i = 0; i < 100; ++i) lists += QStringLiteral("</text:list-item></text:list>");
            const QString page = QStringLiteral("<draw:page draw:name=\"p\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\">%1<draw:frame svg:width=\"10cm\" svg:height=\"5cm\" svg:x=\"1cm\" svg:y=\"1cm\">"
                                                "<draw:text-box>%2</draw:text-box></draw:frame></draw:page>");
            err.clear();
            QElapsedTimer timer;
            timer.start();
            auto d = deck::importOdp(odptest::odpZip(odptest::odpContent(odptest::sampleAutomaticStyles(), page.arg(groups, lists))), &err, &skipped);
            QVERIFY2(d, qPrintable(err));
            QCOMPARE(int(d->pages.size()), 1);
            QVERIFY(timer.elapsed() < 20000);
            QString deeper;
            for (int i = 0; i < 300; ++i) deeper += QStringLiteral("<draw:g>");
            for (int i = 0; i < 300; ++i) deeper += QStringLiteral("</draw:g>");
            err.clear();
            QVERIFY(!deck::importOdp(odptest::odpZip(odptest::odpContent(odptest::sampleAutomaticStyles(), page.arg(deeper, QString()))), &err));
            QVERIFY(!err.isEmpty());
        }
        // Equations that refer to themselves, huge numbers, and a transform that isn't one are harmless.
        {
            const QString shape = QStringLiteral("<draw:custom-shape draw:style-name=\"gr2\" svg:width=\"1e30cm\" svg:height=\"5cm\" svg:x=\"-1e30cm\" svg:y=\"1cm\" draw:transform=\"rotate (nan) translate (inf 1cm) matrix (1)\">"
                                                 "<draw:enhanced-geometry draw:type=\"nothing\" svg:viewBox=\"0 0 21600 21600\" draw:enhanced-path=\"M ?f0 ?f1 L 1e300 ?f0 ?f1 ?f1 Z N\">"
                                                 "<draw:equation draw:name=\"f0\" draw:formula=\"?f1+1\"/><draw:equation draw:name=\"f1\" draw:formula=\"?f0*?f0/0\"/></draw:enhanced-geometry></draw:custom-shape>");
            const QString page = QStringLiteral("<draw:page draw:name=\"p\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\">") + shape + QStringLiteral("</draw:page>");
            err.clear();
            auto d = deck::importOdp(odptest::odpZip(odptest::odpContent(odptest::sampleAutomaticStyles(), page)), &err, &skipped);
            QVERIFY2(d, qPrintable(err));
            QCOMPARE(int(d->pages.size()), 1);
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = d.get();
            ctx.cache = &cache;
            QVERIFY(!Renderer::renderToImage(ctx, 0, 0.1).isNull());
        }
        // Styles whose parents run in a circle, and a master page that names itself.
        {
            QString styles = odptest::odpStyles();
            styles.replace(QStringLiteral("<style:style style:name=\"Default-outline2\" style:family=\"presentation\" style:parent-style-name=\"Default-outline1\">"),
                           QStringLiteral("<style:style style:name=\"Default-outline2\" style:family=\"presentation\" style:parent-style-name=\"Default-outline2\">"));
            err.clear();
            auto d = deck::importOdp(odptest::odpZip(odptest::sampleContent(), styles), &err);
            QVERIFY2(d, qPrintable(err));
            QCOMPARE(int(d->pages.size()), 4);
        }
        // Damage anywhere in the package: refused, or read as far as it goes, with the reason; never a crash.
        {
            const QByteArray good = odptest::sampleOdp();
            quint32 seed = 12345;
            auto next = [&seed] { seed = seed * 1664525u + 1013904223u; return seed; };
            int opened = 0;
            for (int round = 0; round < 150; ++round) {
                QByteArray bad = good;
                const int hits = 1 + int(next() % 5);
                for (int i = 0; i < hits; ++i) bad[int((next() >> 8) % quint32(bad.size()))] = char(next() >> 16);
                if (round % 9 == 0) bad.truncate(int((next() >> 8) % quint32(bad.size())));
                err.clear();
                if (auto d = deck::importOdp(bad, &err)) ++opened;
                else QVERIFY(!err.isEmpty());
            }
            QVERIFY(opened > 0);
        }
    }

    // ---- Saving open document presentations (.odp, .otp) ----

    // What the writer makes is an open document package of the standard form: the type stored first, a manifest that lists every part with its
    // media type, parts that are well formed XML in the standard's namespaces, and the program's name and version in the metadata.
    void odpWriterMakesAPackageOfTheStandardForm()
    {
        auto d = deck::newPresentation();
        d->props.title = QStringLiteral("Quarterly <results> & plans");
        d->props.author = QStringLiteral("Pat Doe");
        d->props.keywords = QStringLiteral("sales, plan");
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportOdp(*d, false, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY(skipped.isEmpty());
        // The first entry is "mimetype", stored (method 0), and holds the type.
        QVERIFY(deck::looksLikeOpenDocument(bytes.left(128)));
        QCOMPARE(int(uchar(bytes[8])), 0);
        QMap<QString, QByteArray> zip;
        QVERIFY2(readZip(bytes, zip, &err), qPrintable(err));
        QCOMPARE(QString::fromUtf8(zip.value(QStringLiteral("mimetype"))), QStringLiteral("application/vnd.oasis.opendocument.presentation"));
        auto parse = [&](const QString &name, QDomDocument *dom) {
            QString why;
            const bool ok = office::parseXml(zip.value(name), dom, &why);
            if (!ok) qWarning("%s: %s", qPrintable(name), qPrintable(why));
            return ok;
        };
        const QString officeNs = QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:office:1.0");
        QDomDocument content, styles, meta, manifest;
        QVERIFY(parse(QStringLiteral("content.xml"), &content));
        QVERIFY(parse(QStringLiteral("styles.xml"), &styles));
        QVERIFY(parse(QStringLiteral("meta.xml"), &meta));
        QVERIFY(parse(QStringLiteral("META-INF/manifest.xml"), &manifest));
        QCOMPARE(content.documentElement().namespaceURI(), officeNs);
        QCOMPARE(content.documentElement().localName(), QStringLiteral("document-content"));
        QCOMPARE(content.documentElement().attributeNS(officeNs, QStringLiteral("version")), QStringLiteral("1.3"));
        QCOMPARE(styles.documentElement().localName(), QStringLiteral("document-styles"));
        QCOMPARE(styles.documentElement().attributeNS(officeNs, QStringLiteral("version")), QStringLiteral("1.3"));
        QCOMPARE(meta.documentElement().localName(), QStringLiteral("document-meta"));
        // The body is a presentation with a page for each slide; the styles have the master pages and the page size.
        QCOMPARE(int(odptest::byName(content, "draw:page").size()), 1);
        QVERIFY(odptest::byName(styles, "style:master-page").size() >= 1);
        QVERIFY(odptest::byName(styles, "style:page-layout-properties").size() >= 1);
        // The manifest lists every part but the type and itself, with a media type.
        const QString manifestNs = QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:manifest:1.0");
        QCOMPARE(manifest.documentElement().namespaceURI(), manifestNs);
        QHash<QString, QString> listed;
        const QDomNodeList entries = manifest.documentElement().elementsByTagNameNS(manifestNs, QStringLiteral("file-entry"));
        for (int i = 0; i < entries.size(); ++i) {
            const QDomElement e = entries.item(i).toElement();
            listed.insert(e.attributeNS(manifestNs, QStringLiteral("full-path")), e.attributeNS(manifestNs, QStringLiteral("media-type")));
        }
        QCOMPARE(listed.value(QStringLiteral("/")), QStringLiteral("application/vnd.oasis.opendocument.presentation"));
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) {
            if (it.key() == QLatin1String("mimetype") || it.key() == QLatin1String("META-INF/manifest.xml")) continue;
            QVERIFY2(listed.contains(it.key()), qPrintable(it.key()));
            QVERIFY2(!listed.value(it.key()).isEmpty(), qPrintable(it.key()));
        }
        QCOMPARE(listed.value(QStringLiteral("content.xml")), QStringLiteral("text/xml"));
        QCOMPARE(listed.value(QStringLiteral("styles.xml")), QStringLiteral("text/xml"));
        QCOMPARE(listed.value(QStringLiteral("meta.xml")), QStringLiteral("text/xml"));
        // The metadata names the program and its version, and holds what the presentation says of itself.
        const QString metaText = QString::fromUtf8(zip.value(QStringLiteral("meta.xml")));
        QVERIFY2(metaText.contains(QStringLiteral("<meta:generator>JeffDeck/") + QStringLiteral(JP_VERSION) + QStringLiteral("</meta:generator>")), qPrintable(metaText));
        QVERIFY(metaText.contains(QStringLiteral("<dc:title>Quarterly &lt;results&gt; &amp; plans</dc:title>")));
        QVERIFY(metaText.contains(QStringLiteral("<meta:initial-creator>Pat Doe</meta:initial-creator>")));
        QCOMPARE(int(odptest::byName(meta, "meta:keyword").size()), 2);
        // It reads back with what it said.
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->props.title, d->props.title);
        QCOMPARE(back->props.author, QStringLiteral("Pat Doe"));
        // A template differs in its type, and is read as a presentation.
        const QByteArray tmpl = deck::exportOdp(*d, true, &err);
        QVERIFY(!tmpl.isEmpty());
        QVERIFY(deck::looksLikeOpenDocument(tmpl.left(128)));
        QMap<QString, QByteArray> tz;
        QVERIFY(readZip(tmpl, tz, &err));
        QCOMPARE(QString::fromUtf8(tz.value(QStringLiteral("mimetype"))), QStringLiteral("application/vnd.oasis.opendocument.presentation-template"));
        QVERIFY(deck::importOdp(tmpl, &err));
        // A presentation with no slide can't be written.
        auto none = deck::newPresentation();
        none->pages.clear();
        QVERIFY(deck::exportOdp(*none, false, &err).isEmpty());
        QVERIFY(!err.isEmpty());
    }

    // The file's endings say how it is saved: .odp and .otp as open documents, the others as before; and the old binary endings are not written.
    void savingGoesByTheEndingOfTheFileName()
    {
        QCOMPARE(int(deck::formatOfFileName(QStringLiteral("a.odp"))), int(deck::FileFormat::Odp));
        QCOMPARE(int(deck::formatOfFileName(QStringLiteral("a.OTP"))), int(deck::FileFormat::Odp));
        QCOMPARE(int(deck::formatOfFileName(QStringLiteral("a.pptx"))), int(deck::FileFormat::Pptx));
        QCOMPARE(int(deck::formatOfFileName(QStringLiteral("a.ppt"))), int(deck::FileFormat::Pptx));
        QCOMPARE(int(deck::presentationKindOf(QStringLiteral("a.otp"))), int(deck::PresentationKind::Template));
        QCOMPARE(int(deck::presentationKindOf(QStringLiteral("a.potx"))), int(deck::PresentationKind::Template));
        QCOMPARE(int(deck::presentationKindOf(QStringLiteral("a.odp"))), int(deck::PresentationKind::Presentation));
        QCOMPARE(int(deck::presentationKindOf(QStringLiteral("a.ppsx"))), int(deck::PresentationKind::Show));
        for (const char *ending : {"pptx", "potx", "ppsx", "odp", "otp"}) QVERIFY(deck::isWrittenEnding(QStringLiteral("a.") + QLatin1String(ending)));
        for (const char *ending : {"ppt", "pps", "pot", "odt", "txt"}) QVERIFY(!deck::isWrittenEnding(QStringLiteral("a.") + QLatin1String(ending)));

        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto d = deck::newPresentation();
        QString err;
        const QString odp = dir.filePath(QStringLiteral("talk.odp")), otp = dir.filePath(QStringLiteral("design.otp")), pptx = dir.filePath(QStringLiteral("talk.pptx"));
        QVERIFY2(deck::saveAnyPresentationFile(*d, odp, &err), qPrintable(err));
        QVERIFY2(deck::saveAnyPresentationFile(*d, otp, &err), qPrintable(err));
        QVERIFY2(deck::saveAnyPresentationFile(*d, pptx, &err), qPrintable(err));
        QVERIFY(deck::looksLikeOpenDocument(fileBytes(odp).left(128)));
        QVERIFY(deck::looksLikeOpenDocument(fileBytes(otp).left(128)));
        QVERIFY(!deck::looksLikeOpenDocument(fileBytes(pptx).left(128)));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(fileBytes(otp), zip, &err));
        QVERIFY(zip.value(QStringLiteral("mimetype")).endsWith("presentation-template"));
        QVERIFY(deck::importAnyPresentationFile(odp, &err));
        // A save that can't be done leaves the old file whole.
        const QByteArray before = fileBytes(odp);
        QVERIFY(!deck::saveAnyPresentationFile(*d, dir.filePath(QStringLiteral("missing/talk.odp")), &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(!deck::saveAnyPresentationFile(*d, QString(), &err));
        QCOMPARE(fileBytes(odp), before);
    }

    // A presentation read from an .odp, written, and read again is the same slides: the sample presentation (a title with a link-less
    // run of its own, an outline of two levels, a numbered list, links, a picture cropped and mirrored, shapes with fills and arrows, a
    // line, a group, a table with merged cells, a turned rectangle, charts as pictures, a picture fill, a transition, the footer, and a
    // hidden slide on a background of its own) comes back with every object where it was, with its text, fill, line, and picture.
    void odpWrittenAndReadAgainIsTheSamePresentation()
    {
        QString err;
        auto first = deck::importOdp(odptest::sampleOdp(), &err);
        QVERIFY2(first, qPrintable(err));
        QStringList skipped;
        const QByteArray bytes = deck::exportOdp(*first, false, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto second = deck::importOdp(bytes, &err);
        QVERIFY2(second, qPrintable(err));
        const QString difference = odptest::odpDifference(*first, *second);
        QVERIFY2(difference.isEmpty(), qPrintable(difference));
        // It is the same the second time round too: nothing is lost by writing what was written.
        const QByteArray again = deck::exportOdp(*second, false, &err);
        auto third = deck::importOdp(again, &err);
        QVERIFY2(third, qPrintable(err));
        QVERIFY2(odptest::odpDifference(*second, *third).isEmpty(), qPrintable(odptest::odpDifference(*second, *third)));
        // The footer of the third slide (a footer text, the number) is the page's own setting and a declaration, not an object of the slide.
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        QVERIFY(content.contains(QStringLiteral("<presentation:footer-decl presentation:name=\"ftr1\">Confidential</presentation:footer-decl>")));
        QVERIFY(content.contains(QStringLiteral("presentation:use-footer-name=\"ftr1\"")));
        QVERIFY(content.contains(QStringLiteral("presentation:display-page-number=\"true\"")));
        // The pictures are in the file once each, under Pictures/, with their media type in the manifest.
        int pictureParts = 0;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) pictureParts += it.key().startsWith(QLatin1String("Pictures/"));
        QVERIFY(pictureParts >= 1);
        QVERIFY(zip.value(QStringLiteral("META-INF/manifest.xml")).contains("media-type=\"image/png\""));
    }

    // A presentation built here keeps what it says through an .odp: its text with the formatting of each run (a face, a size, bold,
    // italic, underline, a color, superscript), the paragraphs' alignment, bullets and numbers with their levels, the slide's
    // look (fill, outline, dash, arrows, shadow), and its pictures, and renders the same as it did.
    void odpKeepsTextAndLooksOfAPresentationBuiltHere()
    {
        auto d = deck::newPresentation();
        const int second = deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        Page &slide = *d->pages[second];
        TextItem *title = nullptr, *body = nullptr;
        for (const ItemPtr &it : slide.items) {
            if (deck::placeholderType(*it) == QLatin1String("title")) title = static_cast<TextItem *>(it.get());
            else if (deck::placeholderType(*it) == QLatin1String("obj")) body = static_cast<TextItem *>(it.get());
        }
        QVERIFY(title && body);
        d->storyDoc(title->storyId)->setPlainText(QStringLiteral("Plans & <numbers>"));
        QTextDocument *sd = d->storyDoc(body->storyId);
        sd->clear();
        QTextCursor c(sd);
        QTextCharFormat bold;
        bold.setFontFamilies({QStringLiteral("Arial")});
        bold.setFontPointSize(24);
        bold.setFontWeight(QFont::Bold);
        bold.setProperty(tp::ColorRefP, ColorRef::rgb(QColor(0x33, 0x66, 0x99)).toString());
        QTextCharFormat plain = bold;
        plain.setFontWeight(QFont::Normal);
        plain.setProperty(tp::ColorRefP, ColorRef::rgb(QColor(0, 0, 0)).toString());
        QTextCharFormat fancy = plain;
        fancy.setFontItalic(true);
        fancy.setFontUnderline(true);
        fancy.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
        QTextBlockFormat level0;
        level0.setLeftMargin(27);
        level0.setTextIndent(-27);
        level0.setProperty(tp::ListLevel, 0);
        QTextListFormat bullets;
        bullets.setStyle(QTextListFormat::ListDisc);
        bullets.setProperty(tp::BulletChar, QStringLiteral("•"));
        c.setBlockFormat(level0);
        c.createList(bullets);
        c.insertText(QStringLiteral("First  point"), bold);
        QTextBlockFormat level1 = level0;
        level1.setLeftMargin(54);
        level1.setProperty(tp::ListLevel, 1);
        level1.setAlignment(Qt::AlignRight);
        c.insertBlock(level1);
        c.createList(bullets);
        c.insertText(QStringLiteral("Second "), plain);
        c.insertText(QStringLiteral("fancy"), fancy);
        QTextBlockFormat numbered = level0;
        c.insertBlock(numbered);
        QTextListFormat numbers;
        numbers.setStyle(QTextListFormat::ListDecimal);
        numbers.setNumberSuffix(QStringLiteral("."));
        c.createList(numbers);
        c.insertText(QStringLiteral("Numbered"), plain);
        QTextBlockFormat none;
        none.setTopMargin(6);
        c.insertBlock(none, plain);
        c.insertText(QStringLiteral("Plain\tpart"), plain);
        // A shape with the look: fill, dashed line with an arrow, and a shadow.
        auto shape = std::make_shared<ShapeItem>();
        shape->shape = QStringLiteral("roundRect");
        shape->adj = {0.2};
        shape->rect = QRectF(100, 380, 200, 80);
        shape->fill = Fill::solid(ColorRef::rgb(QColor(0xcc, 0x66, 0x00)), 0.25);
        shape->stroke = Stroke::line(ColorRef::rgb(QColor(0, 0x33, 0)), 3);
        shape->stroke.dash = Stroke::DashLine;
        shape->stroke.endArrow = Arrow::Triangle;
        shape->fx.shadow.on = true;
        shape->storyId = d->createStory(QStringLiteral("Shape text"));
        slide.items.push_back(shape);
        auto bar = std::make_shared<ShapeItem>();
        bar->shape = QStringLiteral("rect");
        bar->rect = QRectF(320, 380, 100, 60);
        bar->fill = Fill::gradient(ColorRef::rgb(QColor(255, 0, 0)), ColorRef::rgb(QColor(0, 0, 255)), 0);
        bar->rotation = 30;
        slide.items.push_back(bar);
        // A picture, cropped, mirrored, and brighter.
        auto picture = std::make_shared<PictureItem>();
        picture->imageId = d->addImage(odptest::png(60, 40, QColor(10, 160, 60)), QStringLiteral("png"));
        picture->rect = QRectF(450, 380, 90, 60);
        picture->imgRect = QRectF(-15, -10, 120, 80);
        picture->flipH = true;
        picture->brightness = 20;
        slide.items.push_back(picture);
        d->pages[second]->hidden = false;

        QString err;
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 2);
        Page &page = *back->pages[1];
        QCOMPARE(textOf(*back, *page.items[0]), QStringLiteral("Plans & <numbers>"));
        const auto *bodyBack = static_cast<const TextItem *>(page.items[1].get());
        const QTextDocument *text = back->storyDoc(bodyBack->storyId);
        QCOMPARE(text->toPlainText(), sd->toPlainText());
        // The runs: bold blue Arial, then plain, then italic underlined superscript.
        QTextBlock b = text->begin();
        QCOMPARE(b.textList() != nullptr, true);
        QTextCharFormat cf = b.begin().fragment().charFormat();
        QCOMPARE(cf.fontPointSize(), 24.0);
        QVERIFY(cf.fontWeight() >= QFont::DemiBold);
        QCOMPARE(cf.fontFamilies().toStringList().value(0), QStringLiteral("Arial"));
        QCOMPARE(ColorRef::fromString(cf.stringProperty(tp::ColorRefP)).rgbValue().name(), QStringLiteral("#336699"));
        b = b.next();
        QVERIFY(b.textList() != nullptr);
        QCOMPARE(int(b.blockFormat().alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignRight));
        QCOMPARE(b.blockFormat().intProperty(tp::ListLevel), 1);
        auto it = b.begin();
        QVERIFY(it.fragment().charFormat().fontWeight() < QFont::DemiBold);
        ++it;
        QVERIFY(!it.atEnd());
        QVERIFY(it.fragment().charFormat().fontItalic());
        QVERIFY(it.fragment().charFormat().underlineStyle() != QTextCharFormat::NoUnderline);
        QCOMPARE(int(it.fragment().charFormat().verticalAlignment()), int(QTextCharFormat::AlignSuperScript));
        b = b.next();
        QVERIFY(b.textList() != nullptr);
        QCOMPARE(int(b.textList()->format().style()), int(QTextListFormat::ListDecimal));
        b = b.next();
        QVERIFY(b.textList() == nullptr);
        QCOMPARE(b.blockFormat().topMargin(), 6.0);
        QVERIFY(b.text().contains(QLatin1Char('\t')));
        // The shape's look.
        const auto *shapeBack = static_cast<const ShapeItem *>(page.items[2].get());
        QCOMPARE(shapeBack->shape, QStringLiteral("roundRect"));
        QVERIFY(std::abs(shapeBack->adj.value(0) - 0.2) < 0.01);
        QCOMPARE(shapeBack->fill.type, Fill::Solid);
        QCOMPARE(shapeBack->fill.color.rgbValue().name(), QStringLiteral("#cc6600"));
        QVERIFY(std::abs(shapeBack->fill.transparency - 0.25) < 0.01);
        QVERIFY(std::abs(shapeBack->stroke.width - 3) < 0.05);
        QCOMPARE(int(shapeBack->stroke.dash), int(Stroke::DashLine));
        QCOMPARE(int(shapeBack->stroke.endArrow), int(Arrow::Triangle));
        QVERIFY(shapeBack->fx.shadow.on);
        QCOMPARE(back->storyDoc(shapeBack->storyId)->toPlainText(), QStringLiteral("Shape text"));
        const auto *barBack = static_cast<const ShapeItem *>(page.items[3].get());
        QCOMPARE(barBack->fill.type, Fill::Gradient);
        QVERIFY(std::abs(barBack->rotation - 30) < 0.1);
        QVERIFY(barBack->rect.size().width() > 99.9 && barBack->rect.size().width() < 100.1);
        const auto *pictureBack = static_cast<const PictureItem *>(page.items[4].get());
        QCOMPARE(back->images.value(pictureBack->imageId).bytes, d->images.value(picture->imageId).bytes);
        QVERIFY(pictureBack->flipH);
        QVERIFY(std::abs(pictureBack->brightness - 20) < 1);
        QVERIFY(std::abs(pictureBack->imgRect.x() - (-15)) < 0.5 && std::abs(pictureBack->imgRect.width() - 120) < 0.5);
        // Drawn, the slide is the same: the mean difference of the two pictures is under a gray level.
        LayoutCache cacheA, cacheB;
        PaintContext ca, cb;
        ca.doc = d.get();
        ca.cache = &cacheA;
        ca.opt.output = true;
        cb.doc = back.get();
        cb.cache = &cacheB;
        cb.opt.output = true;
        const QImage ia = Renderer::renderToImage(ca, second, 1.0).convertToFormat(QImage::Format_Grayscale8), ib = Renderer::renderToImage(cb, 1, 1.0).convertToFormat(QImage::Format_Grayscale8);
        QCOMPARE(ia.size(), ib.size());
        qint64 total = 0;
        for (int y = 0; y < ia.height(); ++y)
            for (int x = 0; x < ia.width(); ++x) total += std::abs(int(ia.constScanLine(y)[x]) - int(ib.constScanLine(y)[x]));
        const double mean = double(total) / (ia.width() * ia.height());
        QVERIFY2(mean < 1.0, qPrintable(QString::number(mean)));
    }

    // Each transition JeffDeck has is written as the standard names it, and read back as the one it was (a cut, which the standard has
    // as a fade through black, and a reveal, which is an uncover, are read as those); how long it takes and when it moves on go with it.
    void odpTransitionsComeBackAsTheyWere()
    {
        auto d = deck::newPresentation();
        struct Case {
            const char *type, *dir;
            const char *readAs;
        };
        const Case cases[] = {{"fade", "l", "fade"},   {"push", "l", "push"},   {"push", "r", "push"},   {"push", "u", "push"},     {"push", "d", "push"},     {"wipe", "l", "wipe"},
                              {"wipe", "r", "wipe"},   {"wipe", "u", "wipe"},   {"wipe", "d", "wipe"},   {"split", "in", "split"},  {"split", "out", "split"}, {"cover", "u", "cover"},
                              {"cover", "r", "cover"}, {"uncover", "l", "uncover"}, {"uncover", "d", "uncover"}, {"zoom", "in", "zoom"}, {"zoom", "out", "zoom"}, {"cut", "l", "fade"},
                              {"reveal", "r", "uncover"}, {"none", "l", "none"}};
        // One slide for each case, after the first.
        for (size_t i = 0; i < std::size(cases); ++i) deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        for (size_t i = 0; i < std::size(cases); ++i) {
            deck::Transition t;
            t.type = QString::fromLatin1(cases[i].type);
            t.dir = QString::fromLatin1(cases[i].dir);
            t.duration = i % 3 == 0 ? 0.4 : i % 3 == 1 ? 0.7 : 1.0;
            if (i % 4 == 1) {
                t.advanceAfter = 3.0;
                t.advanceOnClick = i % 8 == 1;
            }
            deck::setTransition(*d->pages[i + 1], t);
        }
        QString err;
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), int(d->pages.size()));
        for (size_t i = 0; i < std::size(cases); ++i) {
            const deck::Transition was = deck::transitionOf(*d->pages[i + 1]), now = deck::transitionOf(*back->pages[i + 1]);
            const QString what = QStringLiteral("%1 %2").arg(QLatin1String(cases[i].type), QLatin1String(cases[i].dir));
            QCOMPARE(now.type, QString::fromLatin1(cases[i].readAs));
            if (was.hasDirection() || was.hasInOut()) QVERIFY2(now.dir == was.dir, qPrintable(what + QLatin1String(" went ") + now.dir));
            QVERIFY2(std::abs(now.duration - was.duration) < 0.31, qPrintable(what));
            QCOMPARE(now.advanceAfter.has_value(), was.advanceAfter.has_value());
            if (was.advanceAfter) {
                QCOMPARE(*now.advanceAfter, *was.advanceAfter);
                QCOMPARE(now.advanceOnClick, was.advanceOnClick);
            }
        }
    }

    // The animations are written as the other suite writes them: a timing root with the main sequence, a step for each click, the effects that
    // start with or after the one before in the groups of that step, each effect as the standard names it with the parts that do it (show,
    // fade, fly, wipe, zoom, spin, grow), aimed at objects by the ids they have, and at the paragraphs of a list by those they have.
    void odpWritesAnimationsAsTheOtherSuiteDoes()
    {
        auto d = deck::newPresentation();
        const int at = deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        Page &slide = *d->pages[at];
        auto shape = [&](const char *name, double x) {
            auto s = std::make_shared<ShapeItem>();
            s->name = QString::fromLatin1(name);
            s->rect = QRectF(x, 400, 60, 40);
            s->fill = Fill::solid(ColorRef::rgb(Qt::red));
            slide.items.push_back(s);
            return s->id;
        };
        const QString a = shape("A", 20), b = shape("B", 100), c = shape("C", 180), e = shape("E", 260), f = shape("F", 340), g = shape("G", 420);
        TextItem *body = nullptr;
        for (const ItemPtr &it : slide.items)
            if (deck::placeholderType(*it) == QLatin1String("obj")) body = static_cast<TextItem *>(it.get());
        QVERIFY(body);
        d->storyDoc(body->storyId)->setPlainText(QStringLiteral("one\ntwo\nthree"));
        deck::AnimSequence seq;
        auto add = [&](const char *kind, const QString &id, deck::EffectStart start, const QString &variant = QString()) {
            deck::Anim an = deck::makeAnim(QString::fromLatin1(kind), deck::shapeNumber(slide, id));
            an.start = start;
            if (!variant.isEmpty()) an.variant = variant;
            seq.add(an);
        };
        add("entr.fade", a, deck::EffectStart::OnClick);
        add("entr.fly", b, deck::EffectStart::WithPrevious, QStringLiteral("bottom"));
        add("entr.wipe", c, deck::EffectStart::AfterPrevious, QStringLiteral("left"));
        add("emph.spin", e, deck::EffectStart::OnClick);
        add("emph.growshrink", f, deck::EffectStart::OnClick);
        add("entr.zoom", g, deck::EffectStart::OnClick);
        add("exit.fade", a, deck::EffectStart::OnClick);
        // The text of the body, one click for each paragraph.
        deck::Anim paragraph = deck::makeAnim(QStringLiteral("entr.appear"), deck::shapeNumber(slide, body->id));
        seq.add(paragraph);
        deck::setByParagraph(seq, int(seq.effects.size()) - 1, true, *d, slide);
        deck::setAnimations(*d, slide, seq);
        QVERIFY(!deck::timelineOf(slide).isEmpty());

        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportOdp(*d, false, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join(QLatin1Char('\n'))));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        QDomDocument dom;
        QVERIFY(office::parseXml(zip.value(QStringLiteral("content.xml")), &dom, &err));
        const QDomElement page = odptest::byName(dom, "draw:page").value(1);
        QVERIFY(!page.isNull());
        const QList<QDomElement> pars = odptest::byName(page, "anim:par");
        QVERIFY(pars.size() > 5);
        // The root and the main sequence.
        QVERIFY(content.contains(QStringLiteral("presentation:node-type=\"timing-root\"")));
        QVERIFY(content.contains(QStringLiteral("<anim:seq smil:dur=\"indefinite\" presentation:node-type=\"main-sequence\">")));
        // A step for each click: fade A; spin E; grow F; zoom G; fade-out A; and three paragraphs.
        int steps = 0;
        QStringList types;
        for (const QDomElement &p : pars) {
            if (odptest::attrOf(p, "smil:begin") == QLatin1String("next")) ++steps;
            const QString type = odptest::attrOf(p, "presentation:node-type");
            if (type == QLatin1String("on-click") || type == QLatin1String("with-previous") || type == QLatin1String("after-previous"))
                types << type + QLatin1Char(' ') + odptest::attrOf(p, "presentation:preset-id");
        }
        QCOMPARE(steps, 8);
        QCOMPARE(types.mid(0, 3), (QStringList{QStringLiteral("on-click ooo-entrance-fade-in"), QStringLiteral("with-previous ooo-entrance-fly-in"), QStringLiteral("after-previous ooo-entrance-wipe")}));
        QVERIFY(types.contains(QStringLiteral("on-click ooo-emphasis-spin")));
        QVERIFY(types.contains(QStringLiteral("on-click ooo-emphasis-grow-and-shrink")));
        QVERIFY(types.contains(QStringLiteral("on-click ooo-entrance-fade-in-and-zoom")));
        QVERIFY(types.contains(QStringLiteral("on-click ooo-exit-fade-out")));
        // The parts that do it, as the other suite writes them.
        QVERIFY(content.contains(QStringLiteral("smil:attributeName=\"visibility\" smil:to=\"visible\"")));
        QVERIFY(content.contains(QStringLiteral("smil:type=\"fade\" smil:subtype=\"crossfade\"")));
        QVERIFY(content.contains(QStringLiteral("smil:attributeName=\"y\" smil:values=\"1+height/2;y\"")));
        QVERIFY(content.contains(QStringLiteral("smil:type=\"barWipe\" smil:subtype=\"leftToRight\"")));
        QVERIFY(content.contains(QStringLiteral("smil:attributeName=\"width\" smil:values=\"0;width\"")));
        QVERIFY(content.contains(QStringLiteral("smil:by=\"360\" smil:attributeName=\"transform\" svg:type=\"rotate\"")) || content.contains(QStringLiteral("svg:type=\"rotate\"")));
        QVERIFY(content.contains(QStringLiteral("svg:type=\"scale\"")));
        QVERIFY(content.contains(QStringLiteral("smil:mode=\"out\"")));
        // What the effects aim at is in the page, by its id: the six shapes, and the three paragraphs of the body.
        QSet<QString> ids;
        QList<QDomElement> all;
        for (QDomNode n = page.firstChild(); !n.isNull(); n = n.nextSibling()) all << n.toElement();
        // Every element of the page, at any depth.
        QList<QDomElement> queue = all;
        all.clear();
        while (!queue.isEmpty()) {
            const QDomElement el = queue.takeFirst();
            if (el.isNull()) continue;
            all << el;
            for (QDomNode n = el.firstChild(); !n.isNull(); n = n.nextSibling()) queue << n.toElement();
        }
        for (const QDomElement &el : std::as_const(all)) {
            if (const QString id = odptest::attrOf(el, "draw:id"); !id.isEmpty()) ids.insert(id);
            if (const QString tid = odptest::attrOf(el, "text:id"); !tid.isEmpty()) ids.insert(tid);
        }
        QVERIFY(ids.size() >= 9);
        for (const QDomElement &el : std::as_const(all)) {
            const QString target = odptest::attrOf(el, "smil:targetElement");
            if (!target.isEmpty()) QVERIFY2(ids.contains(target), qPrintable(target));
        }
        // The page still reads (its animations are left out of the model, as before).
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 2);
    }

    // What no animation form here has is left out and said so: effects that wait for a click on an object, and the colors that change.
    void odpSaysWhichAnimationsItLeftOut()
    {
        auto d = deck::newPresentation();
        Page &slide = *d->pages[0];
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(20, 20, 60, 40);
        s->fill = Fill::solid(ColorRef::rgb(Qt::red));
        slide.items.push_back(s);
        const int number = deck::shapeNumber(slide, s->id);
        // A hand-made timing with an effect that changes a color (no form here), beside one that fades.
        deck::AnimSequence seq;
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), number));
        deck::setAnimations(*d, slide, seq);
        QString timing = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(!timing.isEmpty());
        QString err;
        QStringList skipped;
        QVERIFY(!deck::exportOdp(*d, false, &err, &skipped).isEmpty());
        QVERIFY(skipped.isEmpty());
        // An effect whose parts are all colors can't be written.
        const QString colorsOnly = QStringLiteral("<p:timing xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"><p:childTnLst>"
            "<p:seq concurrent=\"1\" nextAc=\"seek\"><p:cTn id=\"2\" dur=\"indefinite\" nodeType=\"mainSeq\"><p:childTnLst><p:par><p:cTn id=\"3\" fill=\"hold\"><p:stCondLst><p:cond delay=\"indefinite\"/></p:stCondLst><p:childTnLst>"
            "<p:par><p:cTn id=\"4\" fill=\"hold\"><p:stCondLst><p:cond delay=\"0\"/></p:stCondLst><p:childTnLst><p:par><p:cTn id=\"5\" presetID=\"3\" presetClass=\"emph\" presetSubtype=\"0\" fill=\"hold\" nodeType=\"clickEffect\">"
            "<p:stCondLst><p:cond delay=\"0\"/></p:stCondLst><p:childTnLst><p:animClr clrSpc=\"rgb\" dir=\"cw\"><p:cBhvr><p:cTn id=\"6\" dur=\"500\" fill=\"hold\"/><p:tgtEl><p:spTgt spid=\"%1\"/></p:tgtEl>"
            "<p:attrNameLst><p:attrName>fillcolor</p:attrName></p:attrNameLst></p:cBhvr><p:to><a:srgbClr xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" val=\"0000FF\"/></p:to></p:animClr>"
            "</p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:seq></p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>").arg(number);
        slide.props[QStringLiteral("timingXml")] = colorsOnly;
        skipped.clear();
        const QByteArray bytes = deck::exportOdp(*d, false, &err, &skipped);
        QVERIFY(!bytes.isEmpty());
        QCOMPARE(int(skipped.size()), 1);
        QVERIFY2(skipped[0].contains(QStringLiteral("Slide 1")) && skipped[0].contains(QStringLiteral("animation")), qPrintable(skipped[0]));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        QVERIFY(!zip.value(QStringLiteral("content.xml")).contains("anim:par"));
    }

    // Comments are written as the other suite writes them (a note of the page for each, with its author, initials, date, and text; a reply
    // another note), the date and footer of a slide as the page's settings and the declarations, and how the show plays with the custom shows.
    void odpWritesCommentsFootersAndTheShow()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        deck::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")}, lee{QStringLiteral("Sam Lee"), QStringLiteral("SL")};
        const QDateTime noon(QDate(2026, 10, 1), QTime(12, 0), QTimeZone::utc());
        const QString first = deck::addComment(*d->pages[1], pat, QStringLiteral("Check\nthe numbers & <more>"), QPointF(100, 40), noon);
        deck::addReply(*d->pages[1], first, lee, QStringLiteral("Done"), noon.addSecs(3600));
        deck::addComment(*d->pages[1], lee, QStringLiteral("Second"), QPointF(300, 200), noon.addDays(1));
        // The footer: a date, the number, and a text.
        deck::FooterOptions footers;
        footers.date = footers.slideNumber = footers.footer = true;
        footers.footerText = QStringLiteral("Confidential <draft>");
        deck::applyFooters(*d, footers);
        // How the show plays, and a custom show of two slides.
        deck::ShowSettings settings;
        settings.type = deck::ShowSettings::Type::Individual;
        settings.loop = true;
        settings.animation = false;
        settings.useTimings = false;
        QVector<deck::CustomShow> shows;
        shows.append({0, QStringLiteral("Short <one>"), {d->pages[2]->id, d->pages[0]->id}});
        deck::setCustomShows(*d, shows);
        settings.slides = deck::ShowSettings::Slides::Custom;
        settings.customShow = 0;
        deck::setShowSettings(*d, settings);
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportOdp(*d, false, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        QDomDocument dom;
        QVERIFY(office::parseXml(zip.value(QStringLiteral("content.xml")), &dom, &err));
        // The comments, in the second page, after its notes.
        const QList<QDomElement> drawPages = odptest::byName(dom, "draw:page");
        const QDomElement page = drawPages.value(1);
        const QList<QDomElement> notes = odptest::byName(page, "officeooo:annotation");
        QCOMPARE(int(notes.size()), 3);
        QCOMPARE(odptest::byName(notes[0], "dc:creator").value(0).text(), QStringLiteral("Pat Doe"));
        QCOMPARE(odptest::byName(notes[0], "meta:creator-initials").value(0).text(), QStringLiteral("PD"));
        QCOMPARE(odptest::byName(notes[0], "dc:date").value(0).text(), QStringLiteral("2026-10-01T12:00:00"));
        const QList<QDomElement> lines = odptest::byName(notes[0], "text:p");
        QCOMPARE(int(lines.size()), 2);
        QCOMPARE(lines[1].text(), QStringLiteral("the numbers & <more>"));
        QCOMPARE(odptest::byName(notes[1], "dc:creator").value(0).text(), QStringLiteral("Sam Lee"));
        QCOMPARE(int(odptest::byName(drawPages[0], "officeooo:annotation").size()), 0);
        // The footer, the number, and the date: the page's settings, the footer text declared once.
        QCOMPARE(content.count(QStringLiteral("<presentation:footer-decl")), 1);
        QVERIFY(content.contains(QStringLiteral(">Confidential &lt;draft&gt;</presentation:footer-decl>")));
        QCOMPARE(content.count(QStringLiteral("presentation:use-footer-name=\"ftr1\"")), 3);
        // All three slides take the one page style that says so.
        QString style = odptest::attrOf(drawPages[0], "draw:style-name");
        QVERIFY(content.contains(QStringLiteral("presentation:display-footer=\"true\" presentation:display-page-number=\"true\" presentation:display-date-time=\"true\"")));
        QCOMPARE(odptest::attrOf(drawPages[1], "draw:style-name"), style);
        QCOMPARE(odptest::attrOf(drawPages[2], "draw:style-name"), style);
        QVERIFY(content.contains(QStringLiteral("<presentation:date-time-decl presentation:name=\"dtd1\" presentation:source=\"current-date\"")));
        // How the show plays.
        const QList<QDomElement> settingsList = odptest::byName(dom, "presentation:settings");
        QCOMPARE(int(settingsList.size()), 1);
        const QDomElement st = settingsList[0];
        auto presentationAttr = [&](const char *name) { return odptest::attrOf(st, (QByteArray("presentation:") + name).constData()); };
        QCOMPARE(presentationAttr("full-screen"), QStringLiteral("false"));
        QCOMPARE(presentationAttr("endless"), QStringLiteral("true"));
        QCOMPARE(presentationAttr("animations"), QStringLiteral("disabled"));
        QCOMPARE(presentationAttr("force-manual"), QStringLiteral("true"));
        QCOMPARE(presentationAttr("show"), QStringLiteral("Short <one>"));
        const QList<QDomElement> shownPages = odptest::byName(st, "presentation:show");
        QCOMPARE(int(shownPages.size()), 1);
        const QStringList names = odptest::attrOf(shownPages[0], "presentation:pages").split(QLatin1Char(','));
        QCOMPARE(int(names.size()), 2);
        // The names it lists are those of the slides it plays, the third and then the first.
        auto pageName = [&](int i) { return odptest::attrOf(drawPages[i], "draw:name"); };
        QCOMPARE(names[0], pageName(2));
        QCOMPARE(names[1], pageName(0));
        // Read back, the footer is on the slides again and the notes are left to the standard.
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 3);
        for (const auto &readPage : back->pages) {
            QSet<QString> kinds;
            for (const ItemPtr &it : readPage->items) {
                kinds.insert(deck::placeholderType(*it));
                if (deck::placeholderType(*it) == QLatin1String("ftr")) QCOMPARE(textOf(*back, *it), QStringLiteral("Confidential <draft>"));
            }
            QVERIFY(kinds.contains(QStringLiteral("ftr")) && kinds.contains(QStringLiteral("sldNum")) && kinds.contains(QStringLiteral("dt")));
        }
    }

    // What an .odp has no form for is said when the presentation is written: the sections, an inner shadow, and a reflection.
    void odpSaysWhatItLeavesOut()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        deck::addSection(*d, 0, QStringLiteral("Opening"));
        deck::addSection(*d, 1, QStringLiteral("Middle"));
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(20, 20, 60, 40);
        s->fill = Fill::solid(ColorRef::rgb(Qt::red));
        s->fx.shadow.on = true;
        s->fx.shadow.inner = true;
        s->fx.reflection.on = true;
        d->pages[1]->items.push_back(s);
        QString err;
        QStringList skipped;
        QVERIFY(!deck::exportOdp(*d, false, &err, &skipped).isEmpty());
        const QString all = skipped.join(QLatin1Char('\n'));
        QVERIFY2(all.contains(QStringLiteral("Sections")), qPrintable(all));
        QVERIFY2(all.contains(QStringLiteral("Inner shadows")), qPrintable(all));
        QVERIFY2(!all.contains(QStringLiteral("Reflections")), qPrintable(all));   // a reflection is kept (in an attribute of ours)
        // A presentation with nothing of that kind says nothing.
        skipped.clear();
        QVERIFY(!deck::exportOdp(*deck::newPresentation(), false, &err, &skipped).isEmpty());
        QVERIFY(skipped.isEmpty());
    }

    // A movie or sound is written the way the other suite writes one: a frame whose plug-in plays the file (kept in the package under
    // Media/, with its type in the manifest) and whose picture is the poster; read back, the poster is still there.
    void odpWritesMoviesAndSoundsAsPlugIns()
    {
        auto d = deck::newPresentation();
        Page &slide = *d->pages[0];
        QImage poster(320, 180, QImage::Format_RGB32);
        poster.fill(QColor(30, 90, 160));
        const QByteArray movie(2000, 'v'), sound(900, 's');
        auto video = deck::newMediaPicture(*d, slide, movie, QStringLiteral("Clip.mp4"), poster, 12345);
        QVERIFY(video);
        slide.items.push_back(video);
        auto audio = deck::newMediaPicture(*d, slide, sound, QStringLiteral("Tune.wav"), QImage(), 5000);
        QVERIFY(audio);
        slide.items.push_back(audio);
        QVERIFY(video->sourceId != audio->sourceId);
        // The movie plays when it is clicked on, the sound with the slide's clicks.
        deck::MediaPlay onClick;
        onClick.start = deck::MediaStart::WhenClickedOn;
        deck::setMediaPlay(slide, *video, onClick, 12345);
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportOdp(*d, false, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join(QLatin1Char('\n'))));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        QCOMPARE(zip.value(QStringLiteral("Media/media1.mp4")), movie);
        QCOMPARE(zip.value(QStringLiteral("Media/media2.wav")), sound);
        const QString manifest = QString::fromUtf8(zip.value(QStringLiteral("META-INF/manifest.xml")));
        QVERIFY(manifest.contains(QStringLiteral("manifest:full-path=\"Media/media1.mp4\" manifest:media-type=\"video/mp4\"")));
        QVERIFY(manifest.contains(QStringLiteral("manifest:full-path=\"Media/media2.wav\" manifest:media-type=\"audio/wav\"")));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        QVERIFY(content.contains(QStringLiteral("<draw:plugin xlink:href=\"Media/media1.mp4\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\" draw:mime-type=\"video/mp4\">")));
        QVERIFY(content.contains(QStringLiteral("<draw:param draw:name=\"Loop\" draw:value=\"false\"/>")));
        QVERIFY(content.contains(QStringLiteral("draw:mime-type=\"audio/wav\"")));
        // What starts them is written as the other suite writes it: the call to play, and a click on a movie that pauses it.
        QVERIFY(content.contains(QStringLiteral("presentation:preset-class=\"media-call\"")));
        QVERIFY(content.contains(QStringLiteral("anim:command=\"play\"")));
        QVERIFY(content.contains(QStringLiteral("presentation:node-type=\"interactive-sequence\"")));
        QVERIFY(content.contains(QStringLiteral("anim:command=\"toggle-pause\"")));
        QVERIFY(content.contains(QStringLiteral(".click+0s\"")));
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        int pictures = 0;
        for (const ItemPtr &it : back->pages[0]->items) pictures += it->type() == ItemType::Picture;
        QCOMPARE(pictures, 2);
    }

    // A bullet in front of nothing is drawn by the other suite, and not here: an empty paragraph in a list is written outside the list, so it
    // has no bullet there either, and the numbers of a numbered list go on after it.
    void odpWritesEmptyListParagraphsOutsideTheList()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(20, 20, 300, 200);
        t->storyId = d->createStory();
        QTextDocument *sd = d->storyDoc(t->storyId);
        QTextCursor c(sd);
        QTextBlockFormat level0;
        level0.setLeftMargin(27);
        level0.setTextIndent(-27);
        level0.setProperty(tp::ListLevel, 0);
        QTextListFormat numbers;
        numbers.setStyle(QTextListFormat::ListDecimal);
        c.setBlockFormat(level0);
        c.createList(numbers);
        c.insertBlock();   // the first paragraph stays empty
        c.insertText(QStringLiteral("One"));
        c.insertBlock();
        c.insertBlock();   // an empty one in the middle
        c.insertText(QStringLiteral("Two"));
        d->pages[1]->items.push_back(t);
        QString err;
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        const int at = content.indexOf(QStringLiteral("<draw:text-box"));
        const QString box = content.mid(at, content.indexOf(QStringLiteral("</draw:text-box>"), at) - at);
        QCOMPARE(box.count(QStringLiteral("<text:list-item>")), 2);
        QCOMPARE(box.count(QRegularExpression(QStringLiteral("<text:p text:style-name=\"[^\"]*\"/>"))), 2);
        // The empty paragraph in front of the first is not in a list; the second list carries on the numbers.
        QVERIFY(box.indexOf(QStringLiteral("<text:p ")) < box.indexOf(QStringLiteral("<text:list ")));
        QVERIFY(box.contains(QStringLiteral("text:continue-numbering=\"true\"")));
    }

    // A tab at the start of a paragraph that hangs (its first line stands out to the left of the rest) goes to where the rest begin. The
    // other suite puts it a whole step on, so the first line is written to begin there without the tab; drawn again, it is the same.
    void odpWritesAHangingParagraphThatStartsWithATabWithoutTheTab()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(20, 20, 300, 200);
        t->storyId = d->createStory();
        QTextDocument *sd = d->storyDoc(t->storyId);
        QTextCursor c(sd);
        QTextBlockFormat hanging;
        hanging.setLeftMargin(27);
        hanging.setTextIndent(-27);
        c.setBlockFormat(hanging);
        c.insertText(QStringLiteral("\t(1) Hangs from the tab, and wraps onto a second line that starts at the indent"));
        c.insertBlock(hanging);
        c.insertText(QStringLiteral("\t\tTwo tabs stay as they are"));
        c.insertBlock(hanging);
        c.insertText(QStringLiteral("(3) No tab here, so it hangs"));
        d->pages[1]->items.push_back(t);
        QString err;
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        // Only the paragraph with two tabs keeps any: its two.
        QCOMPARE(content.count(QStringLiteral("<text:tab/>")), 2);
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        LayoutCache cacheA, cacheB;
        PaintContext ca, cb;
        ca.doc = d.get();
        ca.cache = &cacheA;
        ca.opt.output = true;
        cb.doc = back.get();
        cb.cache = &cacheB;
        cb.opt.output = true;
        const QImage ia = Renderer::renderToImage(ca, 1, 1.0).convertToFormat(QImage::Format_Grayscale8), ib = Renderer::renderToImage(cb, 1, 1.0).convertToFormat(QImage::Format_Grayscale8);
        QCOMPARE(ia.size(), ib.size());
        qint64 total = 0;
        for (int y = 0; y < ia.height(); ++y)
            for (int x = 0; x < ia.width(); ++x) total += std::abs(int(ia.constScanLine(y)[x]) - int(ib.constScanLine(y)[x]));
        QVERIFY2(double(total) / (ia.width() * ia.height()) < 0.5, qPrintable(QString::number(double(total) / (ia.width() * ia.height()))));
    }

    // How far a tab goes where the paragraph has no stops is the presentation's own (a presentation made elsewhere often says an inch, not
    // half of one): it is written with each paragraph, and read back, a tab at the start of a line goes as far.
    void odpKeepsHowFarATabGoes()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        d->defaultTabStop = 72;
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(20, 20, 400, 100);
        t->storyId = d->createStory(QStringLiteral("\tTabbed in a whole inch, and then some more words to wrap onto a second line"));
        d->pages[1]->items.push_back(t);
        QString err;
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        QVERIFY(QString::fromUtf8(zip.value(QStringLiteral("content.xml"))).contains(QStringLiteral("loext:tab-stop-distance=\"2.54cm\"")));
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        LayoutCache cacheA, cacheB;
        PaintContext ca, cb;
        ca.doc = d.get();
        ca.cache = &cacheA;
        ca.opt.output = true;
        cb.doc = back.get();
        cb.cache = &cacheB;
        cb.opt.output = true;
        const QImage ia = Renderer::renderToImage(ca, 1, 1.0).convertToFormat(QImage::Format_Grayscale8), ib = Renderer::renderToImage(cb, 1, 1.0).convertToFormat(QImage::Format_Grayscale8);
        QCOMPARE(ia.size(), ib.size());
        qint64 total = 0;
        for (int y = 0; y < ia.height(); ++y)
            for (int x = 0; x < ia.width(); ++x) total += std::abs(int(ia.constScanLine(y)[x]) - int(ib.constScanLine(y)[x]));
        QVERIFY2(double(total) / (ia.width() * ia.height()) < 0.5, qPrintable(QString::number(double(total) / (ia.width() * ia.height()))));
    }

    // A bullet of a paragraph whose first line is set in from the rest (a positive first line indent) stands where that line starts: the
    // list's level says so, with no width for the label to hang in.
    void odpPutsABulletWhereAnIndentedFirstLineStarts()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(20, 20, 300, 100);
        t->storyId = d->createStory();
        QTextDocument *sd = d->storyDoc(t->storyId);
        QTextCursor c(sd);
        QTextBlockFormat bf;
        bf.setLeftMargin(10);
        bf.setTextIndent(20);
        bf.setProperty(tp::ListLevel, 0);
        QTextListFormat bullets;
        bullets.setStyle(QTextListFormat::ListDisc);
        bullets.setProperty(tp::BulletChar, QStringLiteral("•"));
        c.setBlockFormat(bf);
        c.createList(bullets);
        c.insertText(QStringLiteral("Set in"));
        d->pages[1]->items.push_back(t);
        QString err;
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        // 10 + 20 points in from the left, 1.0583 cm.
        QVERIFY2(content.contains(QStringLiteral("text:space-before=\"1.0583cm\" text:min-label-width=\"0cm\"")), qPrintable(content.mid(content.indexOf(QStringLiteral("text:list-level-style-bullet")), 400)));
    }

    // The other suite mirrors a picture side to side only (a mirror from top to bottom is ignored): a picture turned over is written as turned
    // half a turn and mirrored side to side, which is the same picture.
    void odpWritesAPictureTurnedOverAsAHalfTurnAndAMirror()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        // Red at the top left, blue below, green on the right: no way to mistake one flip for another.
        QImage image(60, 40, QImage::Format_RGB32);
        image.fill(Qt::green);
        for (int y = 0; y < 40; ++y)
            for (int x = 0; x < 30; ++x) image.setPixelColor(x, y, y < 20 ? QColor(Qt::red) : QColor(Qt::blue));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        auto make = [&](bool flipH, bool flipV, double turn, const QRectF &where) {
            auto pic = std::make_shared<PictureItem>();
            pic->imageId = d->addImage(png, QStringLiteral("png"));
            pic->rect = where;
            pic->imgRect = QRectF(QPointF(0, 0), where.size());
            pic->flipH = flipH;
            pic->flipV = flipV;
            pic->rotation = turn;
            d->pages[1]->items.push_back(pic);
        };
        make(false, true, 0, QRectF(20, 20, 120, 80));
        make(true, true, 0, QRectF(160, 20, 120, 80));
        make(false, true, 180, QRectF(300, 20, 120, 80));
        make(true, false, 30, QRectF(20, 140, 120, 80));
        QString err;
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        QVERIFY(!content.contains(QStringLiteral("style:mirror=\"vertical")));
        QVERIFY(!content.contains(QStringLiteral("horizontal vertical")));
        QVERIFY(content.contains(QStringLiteral("style:mirror=\"horizontal\"")));
        // Read back, and drawn, the slide is the same.
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        LayoutCache cacheA, cacheB;
        PaintContext ca, cb;
        ca.doc = d.get();
        ca.cache = &cacheA;
        ca.opt.output = true;
        cb.doc = back.get();
        cb.cache = &cacheB;
        cb.opt.output = true;
        const QImage ia = Renderer::renderToImage(ca, 1, 1.0).convertToFormat(QImage::Format_RGB32), ib = Renderer::renderToImage(cb, 1, 1.0).convertToFormat(QImage::Format_RGB32);
        QCOMPARE(ia.size(), ib.size());
        qint64 total = 0;
        for (int y = 0; y < ia.height(); ++y)
            for (int x = 0; x < ia.width(); ++x) total += std::abs(qRed(ia.pixel(x, y)) - qRed(ib.pixel(x, y))) + std::abs(qBlue(ia.pixel(x, y)) - qBlue(ib.pixel(x, y)));
        QVERIFY2(double(total) / (ia.width() * ia.height()) < 1.0, qPrintable(QString::number(double(total) / (ia.width() * ia.height()))));
    }

    // A presentation's own parts call a movie "video/unknown", which the other suite doesn't take for a movie it can play: the type written
    // is the one the file's ending gives. A movie that is linked, not kept in the presentation, keeps its address and its picture; one with
    // neither says it was left out.
    void odpWritesMoviesWhateverTheirTypeAndWherever()
    {
        auto d = deck::newPresentation();
        Page &slide = *d->pages[0];
        QImage poster(320, 180, QImage::Format_RGB32);
        poster.fill(QColor(0, 0, 0));
        auto kept = deck::newMediaPicture(*d, slide, QByteArray(1500, 'k'), QStringLiteral("Kept.mp4"), poster, 9000);
        QVERIFY(kept);
        slide.items.push_back(kept);
        d->media[kept->mediaId].contentType = QStringLiteral("video/unknown");
        auto linked = deck::newMediaPicture(*d, slide, QByteArray(10, 'l'), QStringLiteral("Linked.mp4"), poster, 9000);
        QVERIFY(linked);
        slide.items.push_back(linked);
        linked->mediaId.clear();
        linked->mediaLink = QStringLiteral("https://example.com/clip.mp4");
        auto lost = deck::newMediaPicture(*d, slide, QByteArray(10, 'x'), QStringLiteral("Lost.mp4"), poster, 9000);
        QVERIFY(lost);
        slide.items.push_back(lost);
        lost->mediaId.clear();
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportOdp(*d, false, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        QVERIFY2(content.contains(QStringLiteral("xlink:href=\"Media/media1.mp4\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\" draw:mime-type=\"video/mp4\"")), qPrintable(content.left(3000)));
        QVERIFY(QString::fromUtf8(zip.value(QStringLiteral("META-INF/manifest.xml"))).contains(QStringLiteral("manifest:full-path=\"Media/media1.mp4\" manifest:media-type=\"video/mp4\"")));
        QVERIFY(content.contains(QStringLiteral("<draw:plugin xlink:href=\"https://example.com/clip.mp4\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\" draw:mime-type=\"application/vnd.sun.star.media\">")));
        // All three keep their picture.
        QCOMPARE(content.count(QStringLiteral("<draw:image ")), 3);
        QCOMPARE(content.count(QStringLiteral("<draw:plugin ")), 2);
        QCOMPARE(int(skipped.size()), 1);
        QVERIFY2(skipped[0].contains(QStringLiteral("movie or sound")), qPrintable(skipped[0]));
    }

    // The other suite reads a link's words as plain text: they are written straight in the link, the span that styles them around it.
    void odpWritesLinkWordsInsideTheLink()
    {
        QString err;
        auto d = deck::importOdp(odptest::sampleOdp(), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        QVERIFY(content.contains(QStringLiteral("<text:a xlink:type=\"simple\" xlink:href=\"https://example.com/a?b=1&amp;c=2\">the site</text:a>")));
        QVERIFY(!content.contains(QStringLiteral("xlink:type=\"simple\" xlink:href=\"https://example.com/a?b=1&amp;c=2\"><text:span")));
    }

    // Text the presentation program shrank to fit (a:normAutofit's fontScale and lnSpcReduction) can't keep that scale in this kind of file:
    // it is written at the size it was shrunk to, with the margins and the line spacing so, and read back it looks as it did.
    void odpWritesShrunkTextAtTheSizeItWasShrunkTo()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QString lines;
        for (int i = 0; i < 12; ++i) lines += para(QStringLiteral("Line %1 of a long list").arg(i + 1));
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Shrunk\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>")
                                                       + xfrm(0, 0, 4572000, 2540000)
                                                       + QStringLiteral("</p:spPr><p:txBody><a:bodyPr><a:normAutofit fontScale=\"62500\" lnSpcReduction=\"20000\"/></a:bodyPr><a:lstStyle/>")
                                                       + lines + QStringLiteral("</p:txBody></p:sp></p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        auto find = [](const Document &doc, const QString &name) -> TextItem * {
            for (const auto &it : doc.pages[0]->items)
                if (it->name == name && it->type() == ItemType::Text) return static_cast<TextItem *>(it.get());
            return nullptr;
        };
        TextItem *t = find(*d, QStringLiteral("Shrunk"));
        QVERIFY(t && t->fitAsStored);
        LayoutCache cacheA, cacheB;
        const auto original = cacheA.textFrame(*d, *t, 1, RenderOptions());
        const auto before = original.layout->lineInfo(0);
        QCOMPARE(int(before.size()), 12);
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        // 18 points at 62.5% is 11.25, drawn in whole points as the program draws it (the lines 20 points closer is a share of 80% of the 100%).
        QVERIFY2(content.contains(QStringLiteral("fo:font-size=\"11pt\"")), qPrintable(content.left(3000)));
        QVERIFY(content.contains(QStringLiteral("fo:line-height=\"80%\"")));
        const int shrinking = content.count(QStringLiteral("style:shrink-to-fit=\"true\""));
        QVERIFY(shrinking >= 1);
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        TextItem *again = find(*back, QStringLiteral("Shrunk"));
        QVERIFY(again);
        const auto reread = cacheB.textFrame(*back, *again, 1, RenderOptions());
        const auto after = reread.layout->lineInfo(0);
        QCOMPARE(int(after.size()), 12);
        QVERIFY2(std::abs(after[1].baseline - after[0].baseline - (before[1].baseline - before[0].baseline)) < 0.1,
                 qPrintable(QStringLiteral("%1 vs %2").arg(after[1].baseline - after[0].baseline).arg(before[1].baseline - before[0].baseline)));
        // In a box too short for it at that size (as a design's sample text often is), the program left it as it was and shows it so: it isn't
        // set to shrink here, or the other suite would shrink it the moment the file is opened.
        t->rect.setHeight(60);
        const QByteArray shortBox = deck::exportOdp(*d, false, &err);
        QVERIFY2(!shortBox.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip2;
        QVERIFY(readZip(shortBox, zip2, &err));
        QCOMPARE(QString::fromUtf8(zip2.value(QStringLiteral("content.xml"))).count(QStringLiteral("style:shrink-to-fit=\"true\"")), shrinking - 1);
    }

    // A shape whose text shrinks to fit keeps that too: the text is written at the size it was shrunk to, with the shrink set, and read back
    // it is drawn as it was.
    void odpKeepsAShapesShrinkToFit()
    {
        auto d = deck::newPresentation();
        auto s = std::make_shared<ShapeItem>();
        s->name = QStringLiteral("Shrunk");
        s->shape = QStringLiteral("roundRect");
        s->rect = QRectF(100, 100, 150, 60);
        s->storyId = d->createStory();
        QTextDocument *story = d->storyDoc(s->storyId);
        story->setPlainText(QStringLiteral("alpha beta gamma delta epsilon zeta eta theta"));
        QTextCursor c(story);
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setFontPointSize(24);
        c.mergeCharFormat(f);
        s->shrinks = true;
        s->fitScale = 0.625;
        s->fitLineCut = 10;
        d->pages[0]->items.push_back(s);
        LayoutCache cacheA, cacheB;
        PaintContext ctxA;
        ctxA.doc = d.get();
        ctxA.cache = &cacheA;
        QPointF origin;
        const auto before = Renderer::shapeTextLayout(ctxA, *s, &origin)->lineInfo(0);
        QVERIFY(!before.isEmpty());
        QString err;
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const ShapeItem *again = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (it->name == QStringLiteral("Shrunk") && it->type() == ItemType::Shape) again = static_cast<const ShapeItem *>(it.get());
        QVERIFY(again);
        QVERIFY(again->shrinks);
        PaintContext ctxB;
        ctxB.doc = back.get();
        ctxB.cache = &cacheB;
        const auto after = Renderer::shapeTextLayout(ctxB, *again, &origin)->lineInfo(0);
        QCOMPARE(after.size(), before.size());
        QVERIFY2(qAbs(after[0].pointSize - before[0].pointSize) < 0.01, qPrintable(QStringLiteral("%1 vs %2").arg(after[0].pointSize).arg(before[0].pointSize)));
        QVERIFY2(std::abs(after[1].baseline - after[0].baseline - (before[1].baseline - before[0].baseline)) < 0.1,
                 qPrintable(QStringLiteral("%1 vs %2").arg(after[1].baseline - after[0].baseline).arg(before[1].baseline - before[0].baseline)));
    }

    // The space around shrunk paragraphs goes into the other kind of file as it is drawn here: the points as written, the share of a line
    // at the size the text was shrunk to; read back, the lines are where they were.
    void odpWritesShrunkTextsSpaceAsItIsDrawn()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QString paras;
        for (int i = 0; i < 5; ++i)
            paras += QStringLiteral("<a:p><a:pPr marL=\"228600\" indent=\"-228600\"><a:lnSpc><a:spcPct val=\"90000\"/></a:lnSpc><a:spcBef><a:spcPct val=\"20000\"/></a:spcBef><a:spcAft><a:spcPts val=\"600\"/></a:spcAft>"
                                    "<a:buFont typeface=\"Arial\"/><a:buChar char=\"&#8226;\"/></a:pPr><a:r><a:rPr lang=\"en-US\" sz=\"2800\"/><a:t>Paragraph %1</a:t></a:r></a:p>").arg(i + 1);
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Shrunk\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>")
                                                       + xfrm(0, 0, 4572000, 3200000)
                                                       + QStringLiteral("</p:spPr><p:txBody><a:bodyPr><a:normAutofit fontScale=\"85000\" lnSpcReduction=\"20000\"/></a:bodyPr><a:lstStyle/>")
                                                       + paras + QStringLiteral("</p:txBody></p:sp></p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        auto find = [](const Document &doc) -> TextItem * {
            for (const auto &it : doc.pages[0]->items)
                if (it->name == QLatin1String("Shrunk") && it->type() == ItemType::Text) return static_cast<TextItem *>(it.get());
            return nullptr;
        };
        TextItem *t = find(*d);
        QVERIFY(t && t->fitAsStored);
        LayoutCache cacheA, cacheB;
        const auto before = cacheA.textFrame(*d, *t, 1, RenderOptions()).layout->lineInfo(0);
        QCOMPARE(int(before.size()), 5);
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        TextItem *again = find(*back);
        QVERIFY(again);
        const auto after = cacheB.textFrame(*back, *again, 1, RenderOptions()).layout->lineInfo(0);
        QCOMPARE(int(after.size()), 5);
        QVERIFY2(std::abs(after[0].pointSize - 24) < 0.01, qPrintable(QString::number(after[0].pointSize)));
        for (int i = 1; i < 5; ++i)
            QVERIFY2(std::abs((after[i].baseline - after[0].baseline) - (before[i].baseline - before[0].baseline)) < 0.1,
                     qPrintable(QStringLiteral("%1: %2 vs %3").arg(i).arg(after[i].baseline - after[0].baseline).arg(before[i].baseline - before[0].baseline)));
    }

    // A run is kerned in the other kind of file by the size it is drawn at, as it is laid out here (the program: a 37 point run kerned from 14
    // is kerned at 20 points and not at 11 when shrunk), not by the size it was written at.
    void odpKernsShrunkTextByTheSizeItIsDrawnAt()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QString boxes;
        int id = 30;
        for (const char *scale : {"30000", "55000"}) {
            boxes += QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"Box%1\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>").arg(id) + xfrm(0, (id - 30) * 1500000, 4572000, 1000000)
                     + QStringLiteral("</p:spPr><p:txBody><a:bodyPr><a:normAutofit fontScale=\"%1\"/></a:bodyPr><a:lstStyle/>").arg(QLatin1String(scale))
                     + QStringLiteral("<a:p><a:r><a:rPr lang=\"en-US\" sz=\"3700\" kern=\"1400\"/><a:t>AVAVAV</a:t></a:r></a:p></p:txBody></p:sp>");
            ++id;
        }
        xml.replace(QStringLiteral("</p:spTree>"), boxes + QStringLiteral("</p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        // 37 points at 30 percent are 11 (below 14: not kerned); at 55 percent are 20 (kerned).
        QRegularExpression styleRe(QStringLiteral("<style:text-properties[^>]*fo:font-size=\"(\\d+)pt\"[^>]*>"));
        int small = 0, large = 0;
        for (auto it = styleRe.globalMatch(content); it.hasNext();) {
            const auto m = it.next();
            if (m.captured(1) == QLatin1String("11")) {
                ++small;
                QVERIFY2(m.captured(0).contains(QStringLiteral("style:letter-kerning=\"false\"")), qPrintable(m.captured(0)));
            } else if (m.captured(1) == QLatin1String("20")) {
                ++large;
                QVERIFY2(m.captured(0).contains(QStringLiteral("style:letter-kerning=\"true\"")), qPrintable(m.captured(0)));
            }
        }
        QVERIFY(small >= 1 && large >= 1);
    }

    // A gradient whose first color doesn't start at its beginning (the ends are padded with the colors) and one that fades in the middle keep
    // their stops: written with stops of their own, as the other suite writes them, beside the two ends a program without stops reads.
    void odpWritesGradientsWithStopsOfTheirOwn()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        auto make = [&](const QVector<GradientStop> &stops) {
            auto s = std::make_shared<ShapeItem>();
            s->rect = QRectF(20, 20, 200, 100);
            s->fill.type = Fill::Gradient;
            s->fill.stops = stops;
            s->fill.color = stops.first().color;
            s->fill.color2 = stops.last().color;
            s->fill.angle = 90;
            d->pages[1]->items.push_back(s);
        };
        make({{0.1, ColorRef::rgb(Qt::red), 0}, {1.0, ColorRef::rgb(Qt::blue), 0}});
        make({{0.0, ColorRef::rgb(Qt::red), 0}, {0.5, ColorRef::rgb(Qt::green), 0.8}, {1.0, ColorRef::rgb(Qt::blue), 0}});
        make({{0.0, ColorRef::rgb(Qt::red), 0}, {1.0, ColorRef::rgb(Qt::blue), 0}});
        QString err;
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml"))) + QString::fromUtf8(zip.value(QStringLiteral("styles.xml")));
        // The padded one has two stops, the fading one three, and the plain one none.
        QCOMPARE(content.count(QStringLiteral("<loext:gradient-stop svg:offset=\"0.1\"")), 1);
        QCOMPARE(content.count(QStringLiteral("<loext:gradient-stop")), 5);
        QCOMPARE(content.count(QStringLiteral("<loext:opacity-stop")), 3);
        QVERIFY(content.contains(QStringLiteral("<loext:opacity-stop svg:offset=\"0.5\" svg:stop-opacity=\"0.2\"/>")));
    }

    // Pairs of letters are kerned from the size the text says (14 points when it says none), as the text is set here, so another program sets the
    // lines the same way.
    void odpSaysWhichTextIsKerned()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(20, 20, 300, 100);
        t->storyId = d->createStory();
        QTextDocument *sd = d->storyDoc(t->storyId);
        QTextCursor c(sd);
        QTextCharFormat big, small, early;
        big.setFontPointSize(20);
        small.setFontPointSize(10);
        early.setFontPointSize(12);
        early.setProperty(tp::KernAbove, 12.0);
        c.insertText(QStringLiteral("Big"), big);
        c.insertText(QStringLiteral("Small"), small);
        c.insertText(QStringLiteral("Early"), early);
        d->pages[1]->items.push_back(t);
        QString err;
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        auto kerned = [&](const char *size) {
            const int at = content.indexOf(QStringLiteral("fo:font-size=\"%1pt\"").arg(QLatin1String(size)));
            return at < 0 ? QString() : content.mid(at, 1500).section(QStringLiteral("style:letter-kerning=\""), 1).section(QLatin1Char('"'), 0, 0);
        };
        QCOMPARE(kerned("20"), QStringLiteral("true"));
        QCOMPARE(kerned("10"), QStringLiteral("false"));
        QCOMPARE(kerned("12"), QStringLiteral("true"));
    }

    // A shadow on words is kept in an .odp (its offset); the effects that kind of file has no form for on words (a glow, a reflection, an inner
    // shadow) are listed after the save.
    void odpKeepsAShadowOnWordsAndListsTheRest()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(20, 20, 300, 100);
        t->storyId = d->createStory();
        QTextCursor c(d->storyDoc(t->storyId));
        QTextCharFormat shaded, glowing;
        shaded.setFontPointSize(20);
        shaded.setProperty(tp::TextFx, shadowOf(QColor(0, 0, 0), 3, 4, 90).toJson());
        TextEffects rest;
        rest.glow.on = true;
        rest.glow.radius = 6;
        rest.reflection.on = true;
        rest.inner.on = true;
        glowing.setFontPointSize(20);
        glowing.setProperty(tp::TextFx, rest.toJson());
        c.insertText(QStringLiteral("Shaded"), shaded);
        c.insertText(QStringLiteral("Other"), glowing);
        d->pages[1]->items.push_back(t);
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportOdp(*d, false, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        // 4 points down: no distance across, 4 along.
        QVERIFY2(content.contains(QStringLiteral("fo:text-shadow=\"0.00pt 4.00pt\"")), qPrintable(content.left(3000)));
        const QString notes = skipped.join(QLatin1Char('\n'));
        QVERIFY2(notes.contains(QStringLiteral("Reflections")) && notes.contains(QStringLiteral("Inner shadows")) && notes.contains(QStringLiteral("Glows")), qPrintable(notes));
    }

private:
    // The first table of the first slide of an open document presentation, in `d`.
    static const TableItem *odpFirstTable(const Document &d)
    {
        if (d.pages.isEmpty()) return nullptr;
        for (const auto &it : d.pages[0]->items)
            if (it->type() == ItemType::Table) return static_cast<const TableItem *>(it.get());
        return nullptr;
    }
    // A slide with a table: its columns and rows (as the table's XML), in a frame 20 by 10 centimeters.
    static QString odpTablePage(const QString &table)
    {
        return QStringLiteral("<draw:page draw:name=\"p\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\"><draw:frame svg:width=\"20cm\" svg:height=\"10cm\" svg:x=\"1cm\" svg:y=\"1cm\">"
                              "<table:table>%1</table:table></draw:frame></draw:page>").arg(table);
    }

private Q_SLOTS:
    // A table that repeats one cell down and across (a file of 1.6 KB: a hundred rows of a hundred cells of two thousand characters, in cells
    // a fifth of a centimeter wide) is made of one copy of the cell: it took seven minutes, parsing and laying out each of its 10,000
    // cells. Its text is kept to a budget, and the rows are fitted only to as much of it as can be laid out in seconds.
    void odpReadsATableOfRepeatedCellsToABudget()
    {
        const QString word(2000, QLatin1Char('a'));
        const QString table = QStringLiteral("<table:table-column table:number-columns-repeated=\"100\"/><table:table-row table:number-rows-repeated=\"100\">"
                                             "<table:table-cell table:number-columns-repeated=\"100\"><text:p>%1</text:p></table:table-cell></table:table-row>").arg(word);
        QString err;
        QStringList skipped;
        QElapsedTimer timer;
        timer.start();
        auto d = deck::importOdp(odptest::odpZip(odptest::odpContent(odptest::sampleAutomaticStyles(), odpTablePage(table))), &err, &skipped);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(timer.elapsed() < 20000, qPrintable(QString::number(timer.elapsed())));
        const TableItem *t = odpFirstTable(*d);
        QVERIFY(t);
        QCOMPARE(t->rows, 100);
        QCOMPARE(t->cols, 100);
        // A table keeps 100,000 characters of text: the first 50 cells have theirs, each its own, and the rest have none.
        int withText = 0;
        QSet<QString> stories;
        for (int i = 0; i < t->cells.size(); ++i) {
            stories.insert(t->cells[i].storyId);
            withText += d->storyDoc(t->cells[i].storyId)->toPlainText() == word;
        }
        QCOMPARE(withText, 50);
        QCOMPARE(int(stories.size()), 10000);
        QVERIFY(d->storyDoc(t->cell(0, 49).storyId)->toPlainText() == word);
        QVERIFY(d->storyDoc(t->cell(0, 50).storyId)->toPlainText().isEmpty());
        // A copy is independent of the cell it copies.
        QTextCursor cursor(d->storyDoc(t->cell(0, 1).storyId));
        cursor.insertText(QStringLiteral("z"));
        QCOMPARE(d->storyDoc(t->cell(0, 2).storyId)->toPlainText(), word);
        QVERIFY2(skipped.join(QLatin1Char('\n')).contains(QStringLiteral("more text")), qPrintable(skipped.join(QLatin1Char('\n'))));
        QVERIFY2(skipped.join(QLatin1Char('\n')).contains(QStringLiteral("great deal of text")), qPrintable(skipped.join(QLatin1Char('\n'))));
    }

    // A cell that a table repeats comes out as it would if each repeat were written out: its style (which a column may give), its
    // borders, its spans, and its text, across and down, and the picture of the whole is the same.
    void odpTableCellsThatRepeatAreTheCellsWrittenOut()
    {
        const QString cellStyles = QStringLiteral(
            "<style:style style:name=\"ce1\" style:family=\"table-cell\"><style:table-cell-properties fo:background-color=\"#ffcc00\" fo:padding=\"0.2cm\" fo:border=\"0.05pt solid #cc0000\""
            " style:vertical-align=\"middle\"/></style:style>"
            "<style:style style:name=\"ce2\" style:family=\"table-cell\"><style:table-cell-properties fo:background-color=\"#00ccff\" fo:padding=\"0.1cm\"/>"
            "<style:text-properties fo:font-weight=\"bold\" fo:font-size=\"20pt\"/></style:style>"
            "<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"4cm\"/></style:style>");
        const QString styles = odptest::sampleAutomaticStyles() + cellStyles;
        auto cell = [](const QString &text, const QString &attributes = QString()) { return QStringLiteral("<table:table-cell %1><text:p>%2</text:p></table:table-cell>").arg(attributes, text); };
        const QString columns = QStringLiteral("<table:table-column table:style-name=\"co1\" table:default-cell-style-name=\"ce1\" table:number-columns-repeated=\"2\"/>"
                                               "<table:table-column table:style-name=\"co1\" table:default-cell-style-name=\"ce2\"/>");
        // Three rows of a cell repeated three times, the third column in the other style; and a row whose first cell spans two columns and repeats.
        const QString repeated = columns + QStringLiteral("<table:table-row table:number-rows-repeated=\"3\">")
            + cell(QStringLiteral("x"), QStringLiteral("table:number-columns-repeated=\"3\"")) + QStringLiteral("</table:table-row>")
            + QStringLiteral("<table:table-row>") + cell(QStringLiteral("wide"), QStringLiteral("table:number-columns-spanned=\"2\" table:number-columns-repeated=\"2\""))
            + QStringLiteral("</table:table-row>");
        const QString oneRow = cell(QStringLiteral("x")) + cell(QStringLiteral("x")) + cell(QStringLiteral("x"));
        const QString written = columns + QStringLiteral("<table:table-row>") + oneRow + QStringLiteral("</table:table-row><table:table-row>") + oneRow
            + QStringLiteral("</table:table-row><table:table-row>") + oneRow + QStringLiteral("</table:table-row><table:table-row>")
            + cell(QStringLiteral("wide"), QStringLiteral("table:number-columns-spanned=\"2\"")) + cell(QStringLiteral("wide"), QStringLiteral("table:number-columns-spanned=\"2\""))
            + QStringLiteral("</table:table-row>");
        QString err;
        auto a = deck::importOdp(odptest::odpZip(odptest::odpContent(styles, odpTablePage(repeated))), &err);
        QVERIFY2(a, qPrintable(err));
        auto b = deck::importOdp(odptest::odpZip(odptest::odpContent(styles, odpTablePage(written))), &err);
        QVERIFY2(b, qPrintable(err));
        const TableItem *ta = odpFirstTable(*a), *tb = odpFirstTable(*b);
        QVERIFY(ta && tb);
        QCOMPARE(ta->rows, 4);
        QCOMPARE(ta->rows, tb->rows);
        QCOMPARE(ta->cols, tb->cols);
        QCOMPARE(ta->colW, tb->colW);
        QCOMPARE(ta->rowH, tb->rowH);
        for (int r = 0; r < ta->rows; ++r)
            for (int c = 0; c < ta->cols; ++c) {
                const TableCell &x = ta->cell(r, c), &y = tb->cell(r, c);
                const QString where = QStringLiteral("cell %1, %2").arg(r).arg(c);
                QVERIFY2(x.covered == y.covered && x.colSpan == y.colSpan && x.rowSpan == y.rowSpan, qPrintable(where));
                QVERIFY2(x.fill.type == y.fill.type && x.fill.color.rgbValue() == y.fill.color.rgbValue() && x.valign == y.valign && x.margins == y.margins, qPrintable(where));
                QVERIFY2(x.border.left.width == y.border.left.width && x.border.left.color.toString() == y.border.left.color.toString() && x.hyphenate == y.hyphenate, qPrintable(where));
                // (A place the row has no cell for has no story.)
                const QTextDocument *sa = a->storyDoc(x.storyId), *sb = b->storyDoc(y.storyId);
                QVERIFY2(bool(sa) == bool(sb), qPrintable(where));
                if (sa) {
                    QCOMPARE(sa->toPlainText(), sb->toPlainText());
                    QCOMPARE(sa->toHtml(), sb->toHtml());
                }
            }
        QCOMPARE(ta->cell(0, 0).fill.color.rgbValue(), QColor(0xFF, 0xCC, 0x00));
        QCOMPARE(ta->cell(2, 2).fill.color.rgbValue(), QColor(0x00, 0xCC, 0xFF));
        QVERIFY(ta->cell(3, 1).covered);
        QCOMPARE(ta->cell(3, 0).colSpan, 2);
        QCOMPARE(a->storyDoc(ta->cell(1, 1).storyId)->toPlainText(), QStringLiteral("x"));
        QVERIFY(ta->cell(0, 0).storyId != ta->cell(0, 1).storyId && ta->cell(0, 1).storyId != ta->cell(1, 1).storyId);
        // The same picture.
        LayoutCache cacheA, cacheB;
        PaintContext ca, cb;
        ca.doc = a.get();
        ca.cache = &cacheA;
        cb.doc = b.get();
        cb.cache = &cacheB;
        QVERIFY(Renderer::renderToImage(ca, 0, 0.5) == Renderer::renderToImage(cb, 0, 0.5));
    }

    // A table of more cells than 10,000 keeps its first rows (the ones that make up that many), with a note, where it was left out whole.
    void odpKeepsTheFirstRowsOfATableOfTooManyCells()
    {
        const QString table = QStringLiteral("<table:table-column table:number-columns-repeated=\"100\"/><table:table-row table:number-rows-repeated=\"101\">"
                                             "<table:table-cell table:number-columns-repeated=\"100\"><text:p>x</text:p></table:table-cell></table:table-row>");
        QString err;
        QStringList skipped;
        auto d = deck::importOdp(odptest::odpZip(odptest::odpContent(odptest::sampleAutomaticStyles(), odpTablePage(table))), &err, &skipped);
        QVERIFY2(d, qPrintable(err));
        const TableItem *t = odpFirstTable(*d);
        QVERIFY(t);
        QCOMPARE(t->rows, 100);
        QCOMPARE(t->cols, 100);
        QCOMPARE(d->storyDoc(t->cell(99, 99).storyId)->toPlainText(), QStringLiteral("x"));
        QVERIFY2(skipped.join(QLatin1Char('\n')).contains(QStringLiteral("10,000 cells")), qPrintable(skipped.join(QLatin1Char('\n'))));
        // A table with no cells at all is still left out.
        skipped.clear();
        d = deck::importOdp(odptest::odpZip(odptest::odpContent(odptest::sampleAutomaticStyles(), odpTablePage(QStringLiteral("<table:table-column/>")))), &err, &skipped);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(!odpFirstTable(*d));
        QVERIFY(!skipped.isEmpty());
    }

    // An open document presentation opens in a window and saves as itself, an .odp again: what it holds is written and read back. A
    // template of that kind (.otp) opened here is saved under a new name; one this window wrote is saved over.
    void windowSavesAnOpenDocumentPresentationAsItself()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("talk.odp"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(odptest::sampleOdp());
        f.close();
        const QByteArray before = fileBytes(path);
        DeckWindow w;
        // What was left out (the slide's animation) is said when it opens.
        QString shown;
        QTimer::singleShot(0, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                shown = box->text() + box->informativeText();
                box->close();
            }
        });
        QVERIFY(w.openFile(path));
        QVERIFY2(shown.contains(QStringLiteral("animations")), qPrintable(shown));
        QCOMPARE(int(w.editor()->doc()->pages.size()), 4);
        QCOMPARE(w.editor()->filePath(), path);
        QVERIFY(!w.editor()->isModified());
        w.newSlide();
        // Save writes to the file it came from without asking.
        bool asked = false;
        w.setSavePathChooser([&](const QString &, const QString &) {
            asked = true;
            return QString();
        });
        QVERIFY(w.save());
        QVERIFY(!asked);
        QVERIFY(!w.editor()->isModified());
        QCOMPARE(w.editor()->filePath(), path);
        QVERIFY(fileBytes(path) != before);
        QVERIFY(deck::looksLikeOpenDocument(fileBytes(path).left(128)));
        QString err;
        auto back = deck::importAnyPresentationFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 5);
        QCOMPARE(textOf(*back, *back->pages[0]->items[0]), QStringLiteral("Quarterly results"));
        QCOMPARE(deck::notesText(*back, *back->pages[0]), QStringLiteral("Say hello first\nThen the numbers"));
        // The shapes, the table, and the picture are in what was written.
        int tables = 0, pictures = 0, shapes = 0;
        for (const auto &page : back->pages)
            walkItems(page->items, [&](const ItemPtr &it) {
                tables += it->type() == ItemType::Table;
                pictures += it->type() == ItemType::Picture;
                shapes += it->type() == ItemType::Shape;
            });
        QCOMPARE(tables, 1);
        QCOMPARE(pictures, 3);
        QVERIFY(shapes >= 4);
        // Save As offers the open document types beside the others, and suggests an .odp for this file.
        QStringList suggestions, filters;
        w.setSavePathChooser([&](const QString &suggested, const QString &filter) {
            suggestions << suggested;
            filters << filter;
            return QString();
        });
        QVERIFY(!w.saveAs());
        QCOMPARE(suggestions, QStringList{path});
        QVERIFY2(filters[0].contains(QStringLiteral("*.odp")), qPrintable(filters[0]));
        // A name typed without an ending gets the one the chosen type has.
        w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("typed")); });
        QVERIFY(w.saveAs());
        QCOMPARE(w.editor()->filePath(), dir.filePath(QStringLiteral("typed.odp")));
        QVERIFY(deck::looksLikeOpenDocument(fileBytes(w.editor()->filePath()).left(128)));
        // A template this window wrote is saved over when saved again; the one it came from is not.
        const QString mine = dir.filePath(QStringLiteral("mine.otp"));
        QVERIFY(w.saveTo(mine));
        w.newSlide();
        QVERIFY(w.save());
        QVERIFY(!w.editor()->isModified());
        QCOMPARE(int(deck::importAnyPresentationFile(mine, &err)->pages.size()), 6);
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(fileBytes(mine), zip, &err));
        QVERIFY(zip.value(QStringLiteral("mimetype")).endsWith("presentation-template"));
        DeckWindow v;
        QVERIFY(v.openFile(mine));
        v.newSlide();
        const QByteArray templateBytes = fileBytes(mine);
        QStringList asks;
        v.setSavePathChooser([&](const QString &suggested, const QString &) {
            asks << suggested;
            return QString();
        });
        QVERIFY(!v.save());
        QCOMPARE(asks, QStringList{dir.filePath(QStringLiteral("mine.odp"))});
        QCOMPARE(fileBytes(mine), templateBytes);
        v.editor()->markSaved();
        w.editor()->markSaved();
        // The files are gone with the folder; the list of recent ones doesn't keep them for the tests after this.
        for (const QString &name : {QStringLiteral("talk.odp"), QStringLiteral("typed.odp"), QStringLiteral("mine.otp")}) Settings::get().removeRecentFile(dir.filePath(name));
    }

    // An old binary presentation (.ppt) is read, never written over: it is saved under a new name, a .pptx.
    void windowSavesAnOldBinaryPresentationAsANewOne()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("talk.ppt"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(buildPpt());
        f.close();
        const QByteArray before = fileBytes(path);
        DeckWindow w;
        QTimer::singleShot(0, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) box->close();
        });
        QVERIFY(w.openFile(path));
        w.newSlide();
        QStringList asked;
        w.setSavePathChooser([&](const QString &suggested, const QString &) {
            asked << suggested;
            return QString();
        });
        QVERIFY(!w.save());
        QCOMPARE(asked, QStringList{dir.filePath(QStringLiteral("talk.pptx"))});
        QCOMPARE(fileBytes(path), before);
        w.editor()->markSaved();
        Settings::get().removeRecentFile(path);
    }

    // A run with the older Shadow property and an effect list with a shadow has its .odp text shadow once: the file is well formed.
    void odpWritesATextShadowOnceAndIsWellFormed()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(20, 20, 300, 100);
        t->storyId = d->createStory();
        QTextCursor c(d->storyDoc(t->storyId));
        QTextCharFormat cf;
        cf.setFontPointSize(20);
        cf.setProperty(tp::Shadow, true);
        cf.setProperty(tp::TextFx, shadowOf(QColor(0, 0, 0), 3, 4, 90).toJson());
        c.insertText(QStringLiteral("Both"), cf);
        d->pages[1]->items.push_back(t);
        QString err;
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(bytes, zip, &err));
        QDomDocument dom;
        QString why;
        QVERIFY2(office::parseXml(zip.value(QStringLiteral("content.xml")), &dom, &why), qPrintable(why));
        QVERIFY2(QString::fromUtf8(zip.value(QStringLiteral("content.xml"))).count(QStringLiteral("fo:text-shadow=\"0.00pt 4.00pt\"")) >= 1, "the list's shadow");
        QVERIFY(!QString::fromUtf8(zip.value(QStringLiteral("content.xml"))).contains(QStringLiteral("fo:text-shadow=\"1pt 1pt\"")));
    }

private:
    // The pictures a presentation's charts are, in order, as the SVG each one is.
    static QVector<QByteArray> chartPictures(const Document &d)
    {
        QVector<QByteArray> out;
        for (const auto &page : d.pages)
            for (const auto &it : page->items)
                if (it->type() == ItemType::Picture && d.images.value(static_cast<const PictureItem *>(it.get())->imageId).format == QLatin1String("svg"))
                    out << d.images.value(static_cast<const PictureItem *>(it.get())->imageId).bytes;
        return out;
    }
    static QStringList textsOf(const QByteArray &svg)
    {
        QStringList texts;
        static const QRegularExpression textEl(QStringLiteral("<text[^>]*>([^<]*)</text>"));
        const QString s = QString::fromUtf8(svg);
        for (auto m = textEl.globalMatch(s); m.hasNext();) texts << m.next().captured(1).trimmed();
        return texts;
    }

private Q_SLOTS:

    // The other suite can't say a picture fill's changes of colors, so a fill with them is written as the picture those changes make (not as the
    // picture they were made to), and so is a picture with them: bytes, not words, are what it can keep. A fill with none is the picture as it is.
    void odpWritesWhatTheEffectsOfAPictureMake()
    {
        QString err;
        // A background with a duotone, and a picture with one.
        Parts p = samplePptx();
        const QString duotone = QStringLiteral("<a:duotone><a:srgbClr val=\"000080\"/><a:srgbClr val=\"FFFF00\"/></a:duotone>");
        p.data["/ppt/slides/slide2.xml"].xml.replace("<p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg>",
                                                     QStringLiteral("<p:bg><p:bgPr><a:blipFill><a:blip r:embed=\"rId2\">%1</a:blip><a:stretch><a:fillRect/></a:stretch></a:blipFill><a:effectLst/></p:bgPr></p:bg>").arg(duotone).toUtf8());
        p.data["/ppt/slides/slide1.xml"].xml.replace("<a:blip r:embed=\"rId2\"/>", QStringLiteral("<a:blip r:embed=\"rId2\"><a:tint hue=\"10800000\" amt=\"100000\"/></a:blip>").toUtf8());
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QStringList skipped;
        const QByteArray odp = deck::exportOdp(*d, false, &err, &skipped);
        QVERIFY2(!odp.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(odp, zip, &err));
        // The pictures the package holds: the background's after the duotone (97 of 255 in gray, between the two colors), and the picture's
        // (as a picture of what it looks like) after the tint, which turns every color to cyan: a picture with a change the other suite
        // can't say is a picture of itself, as one with a transparent color is.
        bool duotoned = false, tinted = false;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) {
            if (!it.key().startsWith(QStringLiteral("Pictures/"))) continue;
            QImage img;
            if (!img.loadFromData(it.value())) continue;
            const QColor c = img.pixelColor(img.width() / 2, img.height() / 2);
            if (it.key().startsWith(QStringLiteral("Pictures/object"))) tinted = tinted || (c.red() < 20 && c.blue() > 150 && c.green() > 150);
            else duotoned = duotoned || colorsNear(c, QColor(97, 97, 79), 2);
        }
        QVERIFY2(duotoned, "the background is the picture the duotone makes");
        QVERIFY2(tinted, "the picture is the picture the tint makes");
        // The same deck with no effects writes the picture it has, as it is.
        Parts plain = samplePptx();
        auto asIs = deck::importPresentation(pack(plain), &err);
        QVERIFY2(asIs, qPrintable(err));
        QMap<QString, QByteArray> plainZip;
        QVERIFY(readZip(deck::exportOdp(*asIs, false, &err), plainZip, &err));
        bool kept = false;
        for (auto it = plainZip.cbegin(); it != plainZip.cend(); ++it) kept = kept || (it.key().startsWith(QStringLiteral("Pictures/image")) && it.value() == tinyPng());
        QVERIFY(kept);
    }

    // The documents of the other suite are drawn as a presentation's are (brightness and contrast as the presentation program has them), so a
    // deck looks the same after it is saved as an .odp and opened again, and after it goes on to a .pptx.
    void odpImportDrawsPicturesAsAPresentationDoes()
    {
        QString err;
        auto d = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\"><a:lum bright=\"21000\" contrast=\"52000\"/></a:blip>"), &err);
        QVERIFY2(d, qPrintable(err));
        const QColor first = colorOfSamplePicture(*d);
        QVERIFY2(colorsNear(first, QColor(255, 82, 0)), qPrintable(first.name()));
        const QByteArray odp = deck::exportOdp(*d, false, &err);
        QVERIFY2(!odp.isEmpty(), qPrintable(err));
        auto again = deck::importOdp(odp, &err);
        QVERIFY2(again, qPrintable(err));
        QVERIFY(again->slidePictures);
        QVERIFY2(colorsNear(colorOfSamplePicture(*again), first, 2), qPrintable(colorOfSamplePicture(*again).name()));
        // And through a .pptx after it.
        auto third = deck::importPresentation(deck::exportPresentation(*again, &err), &err);
        QVERIFY2(third, qPrintable(err));
        QVERIFY2(colorsNear(colorOfSamplePicture(*third), first, 2), qPrintable(colorOfSamplePicture(*third).name()));
    }
};

JP_TEST_CLASS(DeckOdpTests)
#include "tst_deck_odp.moc"

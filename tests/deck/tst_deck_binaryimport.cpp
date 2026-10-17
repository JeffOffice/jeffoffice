// JeffDeck's tests of old binary presentations: the container, its limits, and damaged files.
#include "deckhelpers.h"
#include "../timing.h"

class DeckBinaryImportTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

private:
    static QByteArray withStream(const QByteArray &file, const QString &name, const QByteArray &data)
    {
        cfb::File f;
        cfb::read(file, &f);
        f.setStream(name, data);
        return cfb::write(f);
    }

private Q_SLOTS:
    // An old binary presentation opens as slides over a master and layouts:
    // the slide size, the slides in the order the file lists them, the newest
    // edit of each (the second slide is hidden by the later edit), the notes
    // of the one that has them, and the colors of the master's scheme.
    void binaryPresentationOpensWithMastersAndLayouts()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(d->pageSize(), QSizeF(720, 540));
        QCOMPARE(int(d->pages.size()), 2);
        QCOMPARE(placeholderText(*d, *d->pages[0], QStringLiteral("ph:title")), QStringLiteral("Quarterly Results"));
        QCOMPARE(placeholderText(*d, *d->pages[1], QStringLiteral("ph:title")), QStringLiteral("Plan"));
        QCOMPARE(d->pages[1]->title, QStringLiteral("Plan"));
        QVERIFY(d->pages[1]->hidden);
        QVERIFY(!d->pages[0]->hidden);
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("Remember the demo"));
        QVERIFY(deck::notesText(*d, *d->pages[1]).isEmpty());
        // Notes also find their slide by the slide they name.
        auto byAtom = deck::importBinaryPresentation(buildPpt(false, false, false, true), &err);
        QVERIFY2(byAtom, qPrintable(err));
        QCOMPARE(deck::notesText(*byAtom, *byAtom->pages[0]), QStringLiteral("Remember the demo"));
        QVERIFY(deck::notesText(*byAtom, *byAtom->pages[1]).isEmpty());

        int masters = 0;
        for (const auto &m : d->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 1);
        const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("obj"));
        QVERIFY(layout);
        QVERIFY(d->masterFor(*d->pages[0]) == layout);
        QVERIFY(d->masterFor(*d->pages[1]) == layout);
        QVERIFY(deck::isSlideMaster(*d->master(layout->baseId)));
        QStringList placeholders;
        for (const auto &it : layout->items) placeholders << deck::placeholderType(*it);
        QVERIFY(placeholders.contains(QStringLiteral("title")));
        QVERIFY(placeholders.contains(QStringLiteral("obj")));
        // Slides can also be added in the layouts every presentation has.
        for (const char *type : {"title", "titleOnly", "blank"}) QVERIFY2(deck::layoutOfType(*d, QString::fromLatin1(type)), type);

        // The scheme's eight colors: the presentation program turns them into a theme's dark 1 (text), light 1 (background),
        // dark 2 (title text), light 2 (shadow), accent 1 (fills), accent 2 (accent), hyperlink, and followed hyperlink.
        const deck::DeckTheme theme = deck::themeOf(*d);
        QCOMPARE(theme.colors[0], QColor(0x11, 0x22, 0x33));
        QCOMPARE(theme.colors[1], QColor(0xFA, 0xFA, 0xFA));
        QCOMPARE(theme.colors[2], QColor(0x44, 0x55, 0x66));
        QCOMPARE(theme.colors[3], QColor(0xDD, 0xDD, 0xCC));
        QCOMPARE(theme.colors[4], QColor(0x77, 0x88, 0x99));
        QCOMPARE(theme.colors[5], QColor(0xAA, 0xBB, 0xCC));
        QCOMPARE(theme.colors[10], QColor(1, 2, 3));
        QCOMPARE(theme.colors[11], QColor(4, 5, 6));
        // The fonts of the default title and body styles.
        QCOMPARE(theme.headingFont, QStringLiteral("Beta Serif"));
        QCOMPARE(theme.bodyFont, QStringLiteral("Alpha Sans"));
        QVERIFY(d->fromFile);
        // Tabs go to every inch when the styles don't say otherwise.
        QCOMPARE(d->defaultTabStop, 72.0);
    }

    // Text takes its look from the default styles of its kind and level, then
    // from its own paragraph and character runs.
    void binaryPresentationTextFormats()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(), &err);
        QVERIFY2(d, qPrintable(err));
        const Page &first = *d->pages[0];

        QTextDocument *title = placeholderStory(*d, first, QStringLiteral("ph:title"));
        QVERIFY(title);
        const QTextCharFormat tf = title->begin().begin().fragment().charFormat();
        QCOMPARE(tf.fontFamilies().toStringList(), QStringList{"Beta Serif"});
        QCOMPARE(tf.fontPointSize(), 44.0);
        QCOMPARE(ColorRef::fromString(tf.stringProperty(tp::ColorRefP)).rgbValue(), QColor(0x44, 0x55, 0x66));   // the scheme's title text
        QVERIFY((title->begin().blockFormat().alignment() & Qt::AlignHorizontal_Mask) == Qt::AlignHCenter);
        QVERIFY(!title->begin().textList());

        QTextDocument *body = placeholderStory(*d, first, QStringLiteral("ph:body:1"));
        QVERIFY(body);
        QCOMPARE(body->toPlainText(), QStringLiteral("First point\nSecond point\nThird point"));
        const QTextBlock one = body->begin(), two = one.next(), three = two.next();
        QVERIFY(one.isValid() && two.isValid() && three.isValid() && !three.next().isValid());
        // The first level: a round bullet hanging 45 points, text of 28 points in the first font and the text color.
        QVERIFY(one.textList());
        QCOMPARE(one.textList()->format().style(), QTextListFormat::ListDisc);
        QCOMPARE(one.textList()->format().intProperty(tp::ListLevel), 0);
        QCOMPARE(one.blockFormat().leftMargin(), 45.0);
        QCOMPARE(one.blockFormat().textIndent(), -45.0);
        const QTextCharFormat c1 = one.begin().fragment().charFormat();
        QCOMPARE(c1.fontFamilies().toStringList(), QStringList{"Alpha Sans"});
        QCOMPARE(c1.fontPointSize(), 28.0);
        QVERIFY(c1.fontWeight() != QFont::Bold);
        QCOMPARE(ColorRef::fromString(c1.stringProperty(tp::ColorRefP)).rgbValue(), QColor(0x11, 0x22, 0x33));
        // The second level: a dash, 24 points, 90 points in, and a bold first word.
        QVERIFY(two.textList());
        QCOMPARE(two.textList()->format().intProperty(tp::ListLevel), 1);
        QCOMPARE(two.textList()->format().stringProperty(tp::BulletChar), QString(QChar(0x2013)));
        QCOMPARE(two.blockFormat().leftMargin(), 90.0);
        QCOMPARE(two.blockFormat().textIndent(), -45.0);
        QStringList runs;
        for (auto f = two.begin(); !f.atEnd(); ++f) {
            const QTextCharFormat cf = f.fragment().charFormat();
            runs << QStringLiteral("%1|%2|%3").arg(f.fragment().text(), cf.fontWeight() >= QFont::Bold ? QStringLiteral("bold") : QStringLiteral("plain")).arg(cf.fontPointSize());
        }
        QCOMPARE(runs, (QStringList{"Second|bold|24", " point|plain|24"}));
        // Paragraphs of one level and mark go on in the one list; another level starts its own.
        QVERIFY(three.textList() == two.textList());
        QVERIFY(one.textList() != two.textList());

        // The second slide's body: a line break within a paragraph that has no bullet.
        QTextDocument *plan = placeholderStory(*d, *d->pages[1], QStringLiteral("ph:body:1"));
        QVERIFY(plan);
        QCOMPARE(plan->toPlainText(), QStringLiteral("Ship\nit"));
        QCOMPARE(plan->blockCount(), 1);
        QVERIFY(plan->toRawText().contains(QChar(QChar::LineSeparator)));
        QVERIFY(!plan->begin().textList());
    }

    // A title master is the base of the title layout's own objects; the
    // centered title and the subtitle are placeholders of that layout.
    void binaryPresentationTitleMasterMakesTheTitleLayout()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(true), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 3);
        int masters = 0;
        for (const auto &m : d->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 1);
        const Page &welcome = *d->pages[2];
        const MasterPage *layout = d->masterFor(welcome);
        QVERIFY(layout);
        QCOMPARE(deck::layoutType(*layout), QStringLiteral("title"));
        QVERIFY(deck::isSlideMaster(*d->master(layout->baseId)));
        QStringList types;
        for (const auto &it : layout->items) types << deck::placeholderType(*it);
        QVERIFY(types.contains(QStringLiteral("ctrTitle")));
        QVERIFY(types.contains(QStringLiteral("subTitle")));
        QCOMPARE(placeholderText(*d, welcome, QStringLiteral("ph:ctrTitle")), QStringLiteral("Welcome"));
        QCOMPARE(placeholderText(*d, welcome, QStringLiteral("ph:subTitle:1")), QStringLiteral("To the talk"));
        // The title master's own size for the centered title (54 points), centered.
        QTextDocument *ctr = placeholderStory(*d, welcome, QStringLiteral("ph:ctrTitle"));
        QVERIFY(ctr);
        QCOMPARE(ctr->begin().begin().fragment().charFormat().fontPointSize(), 54.0);
        QVERIFY((ctr->begin().blockFormat().alignment() & Qt::AlignHorizontal_Mask) == Qt::AlignHCenter);
    }

    // The presentation program keeps a title master in a slide record, not in a master record: it is found all the same, and its
    // own objects and sizes make the title layout.
    void binaryPresentationTitleMasterKeptAsASlideRecord()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(true, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), QByteArray(), QByteArray(), 1006), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(!err.contains(QStringLiteral("master was left out")), qPrintable(err));
        QCOMPARE(int(d->pages.size()), 3);
        const Page &welcome = *d->pages[2];
        const MasterPage *layout = d->masterFor(welcome);
        QVERIFY(layout);
        QCOMPARE(deck::layoutType(*layout), QStringLiteral("title"));
        QTextDocument *ctr = placeholderStory(*d, welcome, QStringLiteral("ph:ctrTitle"));
        QVERIFY(ctr);
        QCOMPARE(ctr->begin().begin().fragment().charFormat().fontPointSize(), 54.0);
    }

    // A text's fields, links, languages, and ruler come through, and so does how a
    // slide moves on (kept as the file has it).
    void binaryPresentationFieldsLinksRulerAndTransition()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, true), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 3);
        const Page &slide = *d->pages[2];
        QTextDocument *body = placeholderStory(*d, slide, QStringLiteral("ph:body:1"));
        QVERIFY(body);
        QCOMPARE(body->toPlainText(), QStringLiteral("Go to the site. Slide %1").arg(QChar(QChar::ObjectReplacementCharacter)));
        QStringList runs;
        for (auto f = body->begin().begin(); !f.atEnd(); ++f) {
            const QTextCharFormat cf = f.fragment().charFormat();
            runs << QStringLiteral("%1|%2|%3|%4").arg(f.fragment().text(), cf.isAnchor() ? cf.anchorHref() : QString(), cf.stringProperty(tp::Language), cf.stringProperty(tp::Field));
        }
        QCOMPARE(runs, (QStringList{QStringLiteral("Go to ||fr-FR|"), QStringLiteral("the site|https://example.org/site|fr-FR|"), QStringLiteral(".||fr-FR|"),
                                    QStringLiteral(" Slide |||"), QString(QChar(QChar::ObjectReplacementCharacter)) + QStringLiteral("|||page")}));
        // The ruler's first level: text at 90 points, its first line (and bullet) at 36.
        QCOMPARE(body->begin().blockFormat().leftMargin(), 90.0);
        QCOMPARE(body->begin().blockFormat().textIndent(), -54.0);

        // Effect 6 (a fade), medium speed, moving on by itself after 3 seconds and on a click.
        const deck::Transition fade = deck::transitionOf(slide);
        QCOMPARE(fade.type, QStringLiteral("fade"));
        QCOMPARE(fade.duration, 0.5);
        QVERIFY(fade.advanceAfter.has_value());
        QCOMPARE(*fade.advanceAfter, 3.0);
        QVERIFY(fade.advanceOnClick);
        // What the file said is kept too.
        const QJsonObject raw = slide.props.value(QStringLiteral("binaryTransition")).toObject();
        QCOMPARE(raw.value(QStringLiteral("effect")).toInt(), 6);
        QCOMPARE(raw.value(QStringLiteral("direction")).toInt(), 1);
        QCOMPARE(raw.value(QStringLiteral("speed")).toInt(), 2);
        QCOMPARE(raw.value(QStringLiteral("advanceMs")).toInt(), 3000);
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("binaryTransition")));
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("transition")));
    }

    // An old binary presentation (.ppt, a show .pps, a template .pot) is
    // read, never written over: Save asks for a new name, suggesting the
    // newer kind beside it, and the old file stays as it was.
    void oldBinaryFilesAreSavedUnderANewName()
    {
        QTemporaryDir dir;
        const QByteArray bytes = buildPpt();
        for (const auto &[ending, suggestedEnding] : {std::pair{"ppt", "pptx"}, std::pair{"pps", "ppsx"}, std::pair{"pot", "pptx"}}) {
            const QString path = dir.filePath(QStringLiteral("old.") + QLatin1String(ending));
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(bytes);
            f.close();
            DeckWindow w;
            QVERIFY(w.openFile(path));
            w.newSlide();
            QStringList asked;
            w.setSavePathChooser([&](const QString &suggested, const QString &) {
                asked << suggested;
                return QString();
            });
            QVERIFY(!w.save());
            QCOMPARE(asked, QStringList{dir.filePath(QStringLiteral("old.") + QLatin1String(suggestedEnding))});
            QCOMPARE(fileBytes(path), bytes);
            w.editor()->markSaved();
            Settings::get().removeRecentFile(path);
        }
    }

    // Files that can't be read say why: protected with a password, cut short,
    // edits that go round in a circle, no record of who edited last.
    void binaryPresentationRefusesDamagedFiles()
    {
        const QByteArray good = buildPpt();
        QString err;
        QVERIFY(deck::importBinaryPresentation(good, &err));

        cfb::File f;
        QVERIFY(cfb::read(good, &f));
        // A password: the header token of the current-user record says the presentation is encrypted.
        QByteArray user = f.stream(QStringLiteral("Current User"));
        user.replace(12, 4, le(0xF3D1C4DF, 4));
        QVERIFY(!deck::importBinaryPresentation(withStream(good, QStringLiteral("Current User"), user), &err));
        QVERIFY2(err.contains(QStringLiteral("password")), qPrintable(err));

        err.clear();
        const QByteArray stream = f.stream(pptDocumentStream());
        QVERIFY(!deck::importBinaryPresentation(withStream(good, pptDocumentStream(), stream.left(stream.size() / 2)), &err));
        QVERIFY(!err.isEmpty());

        err.clear();
        QVERIFY(!deck::importBinaryPresentation(buildPpt(false, true), &err));
        QVERIFY2(err.contains(QStringLiteral("circle")), qPrintable(err));

        err.clear();
        cfb::File without;
        QVERIFY(cfb::read(good, &without));
        without.entries[0].children.removeAll(without.find(QStringLiteral("Current User")));
        QVERIFY(!deck::importBinaryPresentation(cfb::write(without), &err));
        QVERIFY(!err.isEmpty());

        err.clear();
        QVERIFY(!deck::importBinaryPresentation(QByteArray("not a compound file at all"), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::importBinaryPresentation(QByteArray(), &err));
        QVERIFY(!err.isEmpty());
        // A compound file that holds no presentation.
        cfb::File other;
        QVERIFY(cfb::read(good, &other));
        other.entries[0].children.removeAll(other.find(pptDocumentStream()));
        err.clear();
        QVERIFY(!deck::importBinaryPresentation(cfb::write(other), &err));
        QVERIFY(!err.isEmpty());
    }

    // A text of thousands of paragraphs, a bullet on each, is read to a limit and
    // quickly (a list takes its items at a cost that grows with its length).
    void binaryPresentationLimitsHugeTexts()
    {
        QString text;
        for (int i = 0; i < 30000; ++i) text += QStringLiteral("a\r");
        const QByteArray big = pptRec(3999, le(1, 4)) + pptRec(4000, utf16(text)) + pptRec(4001, pfRun(text.size() + 1, 0, 0) + cfRun(text.size() + 1, 0));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, big), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(err.contains(QStringLiteral("paragraphs")), qPrintable(err));
        const QTextDocument *body = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(body);
        QCOMPARE(body->blockCount(), 10000);
        QVERIFY(body->begin().textList());
    }

private:
    // ---- Old binary presentations built to make a reader repeat its work (small in bytes)

    // The compound file around a presentation stream whose newest edit starts at `lastEdit`.
    static QByteArray pptFileOf(const QByteArray &stream, quint32 lastEdit, const QByteArray &pictures = QByteArray())
    {
        cfb::File f;
        cfb::Entry root;
        root.type = cfb::Entry::Root;
        root.name = QStringLiteral("Root Entry");
        f.entries << root;
        auto add = [&](const QString &name, const QByteArray &data) {
            cfb::Entry e;
            e.type = cfb::Entry::Stream;
            e.name = name;
            e.data = data;
            f.entries[0].children << int(f.entries.size());
            f.entries << e;
        };
        const QByteArray user("Tester");
        add(QStringLiteral("Current User"), pptRec(4086, le(20, 4) + le(0xE391C05F, 4) + le(lastEdit, 4) + le(user.size(), 2) + le(0x3F4, 2) + raw({3, 0, 0, 0}) + user + le(8, 4)));
        add(pptDocumentStream(), stream);
        add(QStringLiteral("Pictures"), pictures);
        return cfb::write(f);
    }
    // A document with the slide master list `masters` and the slide list `slides` (and a drawing group, if given).
    static QByteArray pptDocumentOf(const QByteArray &slides, const QByteArray &masters = slidePersist(3, 0, 0x80000001), const QByteArray &drawingGroup = QByteArray())
    {
        return pptBox(1000, pptRec(1001, le(5760, 4) + le(4320, 4) + le(4320, 4) + le(5760, 4) + le(5, 4) + le(10, 4) + le(0, 4) + le(0, 4) + le(1, 2) + le(0, 2) + le(0, 4))
                                + drawingGroup + pptBox(4080, masters, 1) + pptBox(4080, slides, 0));
    }
    static QByteArray pptMasterOf() { return pptBox(1016, slideAtom(1, QByteArray("\x01\x02", 2), 0, 0, 3) + colorScheme(1) + pptDrawing()); }
    static QByteArray pptSlideOf(const QByteArray &shapes) { return pptBox(1006, slideAtom(1, QByteArray(), 0x80000001, 0, 7) + pptDrawing(shapes)); }
    // A rectangle that is only a place: 40 bytes.
    static QByteArray pptTinyShape() { return pptBox(0xF004, pptRec(0xF00A, le(0x500, 4) + le(0xA00, 4), 1, 2) + pptRec(0xF010, le(100, 2) + le(200, 2) + le(300, 2) + le(400, 2))); }
    // A picture frame showing picture `index` of the store.
    static QByteArray pptPictureShape(int index)
    {
        return pptBox(0xF004, pptRec(0xF00A, le(0x500, 4) + le(0xA00, 4), 75, 2) + pptRec(0xF00B, le(0x4104, 2) + le(quint32(index), 4), 1, 3)
                                  + pptRec(0xF010, le(100, 2) + le(200, 2) + le(300, 2) + le(400, 2)));
    }
    // A presentation of the slide list `slides`, the records (persist id and record) after the master, and one edit.
    static QByteArray pptOf(const QByteArray &slides, const QVector<QPair<quint32, QByteArray>> &records, const QByteArray &drawingGroup = QByteArray(),
                            const QByteArray &pictures = QByteArray(), const QByteArray &masters = slidePersist(3, 0, 0x80000001))
    {
        QByteArray s = pptDocumentOf(slides, masters, drawingGroup);
        QVector<QPair<quint32, quint32>> at{{1, 0}, {3, quint32(s.size())}};
        s += pptMasterOf();
        for (const auto &r : records) {
            at << qMakePair(r.first, quint32(s.size()));
            s += r.second;
        }
        const quint32 directory = quint32(s.size());
        s += persistDirectory(at);
        const quint32 edit = quint32(s.size());
        s += userEdit(0, directory, 1);
        return pptFileOf(s, edit, pictures);
    }
    // A persist directory of `count` ids from `first` on, all at offset 0, in runs of 4095 (the most a run holds).
    static QByteArray pptDirectoryOf(quint32 first, quint32 count)
    {
        QByteArray d;
        for (quint32 done = 0; done < count; done += 4095) {
            const quint32 n = std::min<quint32>(4095, count - done);
            d += le((first + done) | (n << 20), 4) + QByteArray(int(n) * 4, '\0');
        }
        return pptRec(6002, d);
    }

private Q_SLOTS:
    // One slide that many entries of the slide list name is read once: the drawing of 20,000 shapes is not made again for each
    // of them, as the entries each had their own slide (a file of 1 MB then took half a minute and gigabytes).
    void binaryPresentationReadsASlideNamedManyTimesOnce()
    {
        QByteArray entries;
        for (int i = 0; i < 12; ++i) entries += slidePersist(5, 0, 0x100 + i);
        QString err;
        std::unique_ptr<Document> d;
        const qint64 ms = bestNanoseconds([&] {
            d = deck::importBinaryPresentation(pptOf(entries, {{5, pptSlideOf(pptTinyShape().repeated(20000))}}), &err);
        }, 5, [&] {
            d.reset();
            err.clear();
        }) / 1000000;
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 1);
        QCOMPARE(int(d->pages[0]->items.size()), 20000);
        QVERIFY2(err.contains(QStringLiteral("Slide 2 was left out")) && err.contains(QStringLiteral("Slide 12 was left out")), qPrintable(err));
        QVERIFY(ms < 20000);
        // Slides that are each their own are all read, with no note.
        entries.clear();
        for (int i = 0; i < 3; ++i) entries += slidePersist(5 + i, 0, 0x100 + i);
        err.clear();
        d = deck::importBinaryPresentation(pptOf(entries, {{5, pptSlideOf(pptTinyShape())}, {6, pptSlideOf(pptTinyShape().repeated(2))}, {7, pptSlideOf(pptTinyShape().repeated(3))}}), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 3);
        QCOMPARE(int(d->pages[2]->items.size()), 3);
        QVERIFY2(err.isEmpty(), qPrintable(err));
        // A slide master that two entries name is one master.
        auto masters = deck::importBinaryPresentation(pptOf(slidePersist(5, 0, 0x100), {{5, pptSlideOf(QByteArray())}}), &err);
        QVERIFY(masters);
        err.clear();
        auto twice = deck::importBinaryPresentation(pptOf(slidePersist(5, 0, 0x100), {{5, pptSlideOf(QByteArray())}}, QByteArray(), QByteArray(),
                                                          slidePersist(3, 0, 0x80000001) + slidePersist(3, 0, 0x80000002)), &err);
        QVERIFY2(twice, qPrintable(err));
        QCOMPARE(twice->masters.size(), masters->masters.size());
        QVERIFY2(err.contains(QStringLiteral("master")), qPrintable(err));
    }

    // All the slides and masters together keep to one budget of shapes: a file of slides that each hold a drawing as big as is read
    // (and a slide master or a slide for each) can't make memory grow with the count of slides. The budget is 300,000 shapes.
    void binaryPresentationKeepsOneShapeBudgetForAllSlides()
    {
        QByteArray entries;
        QVector<QPair<quint32, QByteArray>> slides;
        const QByteArray shapes = pptSlideOf(pptTinyShape().repeated(90000));
        for (int i = 0; i < 4; ++i) {
            entries += slidePersist(5 + i, 0, 0x100 + i);
            slides << qMakePair(quint32(5 + i), shapes);
        }
        QString err;
        auto d = deck::importBinaryPresentation(pptOf(entries, slides), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 4);
        int items = 0;
        for (const auto &p : d->pages) items += int(p->items.size());
        // (The budget pays for the records looked at to find the master's background too, a few.)
        QVERIFY2(items <= 300000 && items > 299900, qPrintable(QString::number(items)));
        QVERIFY(d->pages[3]->items.size() < 90000);
        QVERIFY2(err.contains(QStringLiteral("more shapes")), qPrintable(err));
    }

    // A chain of edits that each name the one persist directory (of a million ids) reads it once: each edit re-reading it took
    // a tenth of a second, and two thousand edits (a file of 4 MB) took minutes.
    void binaryPresentationReadsEachPersistDirectoryOnce()
    {
        QByteArray s = pptDocumentOf(slidePersist(5, 0, 0x100));
        const quint32 master = quint32(s.size());
        s += pptMasterOf();
        const quint32 slide = quint32(s.size());
        s += pptSlideOf(pptTinyShape());
        const quint32 big = quint32(s.size());
        s += pptDirectoryOf(8, 1000000);
        quint32 previous = 0;
        for (int i = 0; i < 300; ++i) {
            const quint32 at = quint32(s.size());
            s += userEdit(previous, big, 1);
            previous = at;
        }
        const quint32 directory = quint32(s.size());
        s += persistDirectory({{1, 0}, {3, master}, {5, slide}});
        const quint32 newest = quint32(s.size());
        s += userEdit(previous, directory, 1);
        QString err;
        std::unique_ptr<Document> d;
        const qint64 ms = bestNanoseconds([&] {
            d = deck::importBinaryPresentation(pptFileOf(s, newest), &err);
        }, 5, [&] {
            d.reset();
            err.clear();
        }) / 1000000;
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 1);
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QVERIFY2(ms < 8000, qPrintable(QString::number(ms)));
    }

    // Directories that lie inside one another (each edit's begins in the last one's ids) are read only up to as many ids as the
    // stream has bytes, which no file that doesn't overlap them comes near: the older edits left are left out, with a note, and the
    // newest edit's slides still open.
    void binaryPresentationCapsThePersistEntriesItReads()
    {
        // 700 runs of 375 ids, each run's first word a record header (a persist directory) that runs to the end of the whole.
        constexpr int kRuns = 700, kRunBytes = 4 + 375 * 4;
        QByteArray data;
        for (int i = 0; i < kRuns; ++i) data += le(6002u << 16, 4) + le(quint32((kRuns - i) * kRunBytes - 8), 4) + QByteArray(374 * 4, '\0');
        QByteArray s = pptDocumentOf(slidePersist(5, 0, 0x100));
        const quint32 master = quint32(s.size());
        s += pptMasterOf();
        const quint32 slide = quint32(s.size());
        s += pptSlideOf(pptTinyShape());
        const quint32 big = quint32(s.size());
        s += pptRec(6002, data);
        quint32 previous = 0;
        for (int i = 0; i < kRuns; ++i) {
            const quint32 at = quint32(s.size());
            s += userEdit(previous, big + 8 + quint32(i) * kRunBytes, 1);
            previous = at;
        }
        const quint32 directory = quint32(s.size());
        s += persistDirectory({{1, 0}, {3, master}, {5, slide}});
        const quint32 newest = quint32(s.size());
        s += userEdit(previous, directory, 1);
        QString err;
        std::unique_ptr<Document> d;
        const qint64 ms = bestNanoseconds([&] {
            d = deck::importBinaryPresentation(pptFileOf(s, newest), &err);
        }, 5, [&] {
            d.reset();
            err.clear();
        }) / 1000000;
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 1);
        QVERIFY2(err.contains(QStringLiteral("older edits")), qPrintable(err));
        QVERIFY(ms < 8000);
    }

    // A picture is found in the store by its number in one step: a store of 100,000 pictures and a slide of 100,000 frames, each
    // for another picture, took each frame a pass over the whole store (a file of 3 MB then took ten seconds and more).
    void binaryPresentationFindsPicturesWithoutScanningTheStore()
    {
        constexpr int kPictures = 100000;
        // Entries that name no picture anywhere: the lookup is the work.
        const QByteArray entry = pptRec(0xF007, QByteArray(1, '\x06') + QByteArray(1, '\0') + QByteArray(16, 's') + le(0xFF, 2) + le(0, 4) + le(1, 4) + le(0xFFFFFFFF, 4) + le(0, 4), 6, 2);
        const QByteArray group = pptBox(1035, pptBox(0xF000, pptRec(0xF006, QByteArray(16, '\0')) + pptBox(0xF001, entry.repeated(kPictures), 0xFFF)));
        QByteArray frames;
        for (int i = 1; i <= kPictures; ++i) frames += pptPictureShape(i);
        QString err;
        std::unique_ptr<Document> d;
        const qint64 ms = bestNanoseconds([&] {
            d = deck::importBinaryPresentation(pptOf(slidePersist(5, 0, 0x100), {{5, pptSlideOf(frames)}}, group), &err);
        }, 5, [&] {
            d.reset();
            err.clear();
        }) / 1000000;
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 1);
        QCOMPARE(int(d->pages[0]->items.size()), kPictures);
        QVERIFY2(ms < 10000, qPrintable(QString::number(ms)));
    }

    // The Macintosh pictures of a presentation are drawn from what they may do between them: pictures that are as slow as one may be, six of
    // them (a polygon of 16,001 points that crosses itself, which took 15 seconds to paint), use up the file's share by the fourth, and the
    // fifth and sixth are left out, with the note of pictures that were. Three of them are within it, and need no note.
    void binaryPresentationLeavesMacintoshPicturesOutWhenTheyHaveDoneTooMuch()
    {
        auto big = [](int value) { QByteArray b(2, '\0'); qToBigEndian<quint16>(quint16(value), b.data()); return b; };
        // A picture of 200 by 100 points that paints a star polygon: its size and frame, the version and header, and the polygon.
        QByteArray star = big(0) + big(0) + big(0) + big(100) + big(200);
        for (int i = 0; i < 16001; ++i) {
            const double angle = 2 * M_PI * ((i * 8000) % 16001) / 16001.0;
            star += big(int(50 + 49 * std::sin(angle))) + big(int(100 + 99 * std::cos(angle)));
        }
        qToBigEndian<quint16>(quint16(star.size()), star.data());
        const QByteArray pict = big(0) + big(0) + big(0) + big(100) + big(200) + big(0x0011) + big(0x02FF) + big(0x0C00) + big(0xFFFE) + big(0) + QByteArray(8, '\0') + big(0) + big(0) + big(100)
                                + big(200) + QByteArray(4, '\0') + big(0x0071) + star + big(0x00FF);
        // (the record: two uids' worth of one, then the 34 bytes that tell its size and that it is stored, not deflated)
        auto blip = [&] {
            return pptRec(0xF01C, QByteArray(16, 'u') + le(quint32(pict.size()), 4) + le(0, 4) + le(0, 4) + le(200, 4) + le(100, 4) + le(0, 4) + le(0, 4) + le(quint32(pict.size()), 4)
                                      + QByteArray("\xFE\xFE", 2) + pict, 0x542);
        };
        auto import = [&](int pictures) {
            QByteArray entries, frames;
            for (int i = 1; i <= pictures; ++i) {
                const QByteArray one = blip();
                entries += pptRec(0xF007, QByteArray(1, '\x04') + QByteArray(1, '\0') + QByteArray(16, char('a' + i)) + le(0xFF, 2) + le(quint32(one.size()), 4) + le(1, 4) + le(0xFFFFFFFF, 4) + le(0, 4) + one, 4, 2);
                frames += pptPictureShape(i);
            }
            const QByteArray group = pptBox(1035, pptBox(0xF000, pptRec(0xF006, QByteArray(16, '\0')) + pptBox(0xF001, entries, quint16(pictures))));
            QString err;
            auto d = deck::importBinaryPresentation(pptOf(slidePersist(5, 0, 0x100), {{5, pptSlideOf(frames)}}, group), &err);
            return std::make_pair(std::move(d), err);
        };
        decltype(import(6)) six, three;
        const qint64 ms = bestNanoseconds([&] {
            six = import(6);
            three = import(3);
        }, 3, [&] {
            six.first.reset();
            three.first.reset();
        }) / 1000000;
        QVERIFY2(six.first, qPrintable(six.second));
        QVERIFY2(six.second.contains(QStringLiteral("2 pictures were left out")), qPrintable(six.second));
        QVERIFY2(three.first, qPrintable(three.second));
        QVERIFY2(!three.second.contains(QStringLiteral("were left out")), qPrintable(three.second));
        QVERIFY2(ms < 20000, qPrintable(QString::number(ms)));
    }

    // The notes pages that slides name are found by their slide id in one step: 8,000 slides naming pages in a list of 500,000 each had
    // the list gone through for it (a file of 15 MB took fifteen seconds).
    void binaryPresentationFindsNotesWithoutScanningTheList()
    {
        constexpr int kSlides = 8000;
        QByteArray entries, notes;
        QVector<QPair<quint32, quint32>> at{{1, 0}};
        for (int i = 0; i < kSlides; ++i) entries += slidePersist(100 + i, 0, 0x100 + i);
        // Fifty lists of notes pages, which are not there.
        for (int list = 0; list < 50; ++list) {
            QByteArray page;
            for (int i = 0; i < 10000; ++i) page += slidePersist(2000000 + list * 10000 + i, 0, 0x50000 + list * 10000 + i);
            notes += pptBox(4080, page, 2);
        }
        QByteArray s = pptBox(1000, pptRec(1001, le(5760, 4) + le(4320, 4) + le(4320, 4) + le(5760, 4) + le(5, 4) + le(10, 4) + le(0, 4) + le(0, 4) + le(1, 2) + le(0, 2) + le(0, 4))
                                        + pptBox(4080, slidePersist(3, 0, 0x80000001), 1) + pptBox(4080, entries, 0) + notes);
        at << qMakePair(quint32(3), quint32(s.size()));
        s += pptMasterOf();
        // Slides that each name notes (0x1234) that aren't in the list.
        const QByteArray slide = pptBox(1006, slideAtom(1, QByteArray(), 0x80000001, 0x1234, 7) + pptDrawing());
        for (int i = 0; i < kSlides; ++i) {
            at << qMakePair(quint32(100 + i), quint32(s.size()));
            s += slide;
        }
        const quint32 directory = quint32(s.size());
        s += persistDirectory(at);
        const quint32 edit = quint32(s.size());
        s += userEdit(0, directory, 1);
        QString err;
        std::unique_ptr<Document> d;
        const qint64 ms = bestNanoseconds([&] {
            d = deck::importBinaryPresentation(pptFileOf(s, edit), &err);
        }, 5, [&] {
            d.reset();
            err.clear();
        }) / 1000000;
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), kSlides);
        QVERIFY2(ms < 5000, qPrintable(QString::number(ms)));
    }

    // The placeholders of a slide find their master's by type in one step: 10,000 of them on a master of 40,000 shapes each had the
    // master's shapes gone through for it (ten seconds), and take the master's place as before.
    void binaryPresentationFindsMasterPlaceholdersWithoutScanningTheShapes()
    {
        QByteArray s = pptDocumentOf(slidePersist(5, 0, 0x100));
        const quint32 master = quint32(s.size());
        s += pptBox(1016, slideAtom(1, QByteArray("\x01\x02", 2), 0, 0, 3) + colorScheme(1)
                              + pptDrawing(pptTinyShape().repeated(40000) + pptShape(0x402, 1, QByteArray(), 100, 200, 5000, 900)));
        // Placeholders with no place of their own.
        const QByteArray holder = pptBox(0xF004, pptRec(0xF00A, le(0x500, 4) + le(0xA00, 4), 1, 2) + pptBox(0xF011, pptRec(3011, le(0xFFFFFFFF, 4) + raw({1, 0, 0, 0}))));
        const quint32 slide = quint32(s.size());
        s += pptSlideOf(holder.repeated(10000));
        const quint32 directory = quint32(s.size());
        s += persistDirectory({{1, 0}, {3, master}, {5, slide}});
        const quint32 edit = quint32(s.size());
        s += userEdit(0, directory, 1);
        QString err;
        std::unique_ptr<Document> d;
        const qint64 ms = bestNanoseconds([&] {
            d = deck::importBinaryPresentation(pptFileOf(s, edit), &err);
        }, 5, [&] {
            d.reset();
            err.clear();
        }) / 1000000;
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages[0]->items.size()), 10000);
        for (const auto &it : d->pages[0]->items) QCOMPARE(it->rect, QRectF(25, 12.5, 600, 100));
        QVERIFY2(ms < 4000, qPrintable(QString::number(ms)));
    }

    // The copies of a title master's own shapes that each layout on it has are among the budget of shapes: twelve layouts of a master of
    // 60,000 shapes were 720,000 more.
    void binaryPresentationCountsTheLayoutsCopiesInTheShapeBudget()
    {
        const QByteArray masters = slidePersist(3, 0, 0x80000001) + slidePersist(4, 0, 0x80000002);
        const QByteArray titleMaster = pptBox(1016, slideAtom(3, QByteArray("\x03\x04\x07\x08\x09", 5), 0, 0, 1) + colorScheme(1) + pptDrawing(pptTinyShape().repeated(60000)));
        QVector<QPair<quint32, QByteArray>> records{{4, titleMaster}};
        QByteArray entries;
        int n = 0;
        for (int geom : {0, 1, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}) {
            entries += slidePersist(5 + n, 0, 0x100 + n);
            records << qMakePair(quint32(5 + n), pptBox(1006, slideAtom(geom, QByteArray(), 0x80000002, 0, 7) + pptDrawing()));
            ++n;
        }
        QString err;
        auto d = deck::importBinaryPresentation(pptOf(entries, records, QByteArray(), QByteArray(), masters), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 12);
        int items = 0;
        for (const auto &m : d->masters) items += int(m->items.size());
        QVERIFY2(items > 100000 && items <= 300000, qPrintable(QString::number(items)));
        QVERIFY2(err.contains(QStringLiteral("more shapes")), qPrintable(err));
    }

    // Damage to the bytes of the streams never crashes or hangs the reader: a
    // file is a presentation, or says it isn't.
    void binaryPresentationSurvivesRandomDamage()
    {
        const QByteArray good = buildPpt(true);
        cfb::File f;
        QVERIFY(cfb::read(good, &f));
        const QByteArray stream = f.stream(pptDocumentStream());
        quint32 seed = 12345;
        auto next = [&] { return seed = seed * 1664525u + 1013904223u; };
        int opened = 0;
        for (int round = 0; round < 300; ++round) {
            QByteArray bad = stream;
            const int hits = 1 + int(next() % 6);
            for (int i = 0; i < hits; ++i) bad[int((next() >> 8) % quint32(bad.size()))] = char(next() >> 16);
            if (round % 7 == 0) bad.truncate(int((next() >> 8) % quint32(bad.size())));
            QString err;
            auto d = deck::importBinaryPresentation(withStream(good, pptDocumentStream(), bad), &err);
            if (d) {
                ++opened;
                QVERIFY(d->pages.size() <= 3);
            } else {
                QVERIFY(!err.isEmpty());
            }
        }
        QVERIFY(opened > 0);
    }
};

JP_TEST_CLASS(DeckBinaryImportTests)
#include "tst_deck_binaryimport.moc"

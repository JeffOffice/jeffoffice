// JeffDeck's tests of old binary presentations: the container, its limits, and damaged files.
#include "deckhelpers.h"
#include "../timing.h"

#include "office/pptcrypt.h"
#include "io/pptsalvage.h"

class DeckBinaryImportTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

private:
    static QByteArray f_user(const QByteArray &file)
    {
        cfb::File f;
        cfb::read(file, &f);
        return f.stream(QStringLiteral("Current User"));
    }

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

    // A presentation protected with a password: each persist object is enciphered on its own with the key of its persist id, the pictures field by field. The right password gives the
    // slides and the picture as they were; the file without it says it is protected; a wrong password is refused, and nothing is read.
    void protectedBinaryPresentationOpensWithItsPassword()
    {
        const QByteArray blip = pptRec(0xF01E, QByteArray(16, 'u') + QByteArray(1, '\xFF') + QByteArray("png-bytes-of-a-picture"), 0x6E0);
        const QByteArray secondBlip = pptRec(0xF01A, QByteArray(32, 'v') + QByteArray(34, 'h') + QByteArray("metafile-bytes"), 0x3D5);
        const QByteArray plain = withStream(buildPpt(), QStringLiteral("Pictures"), blip + secondBlip);
        const QByteArray locked = encryptPpt(plain, QStringLiteral("secret"));
        QVERIFY(office::ppt::pptEncrypted(locked));
        QVERIFY(!office::ppt::pptEncrypted(plain));
        QString err;
        QVERIFY(!deck::importBinaryPresentation(locked, &err));
        QVERIFY2(err.contains(QStringLiteral("password")), qPrintable(err));

        QByteArray open;
        QCOMPARE(office::ppt::decryptPpt(locked, QStringLiteral("secret"), 1 << 28, false, &open), office::CryptResult::Ok);
        auto d = deck::importBinaryPresentation(open, &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);
        QCOMPARE(placeholderText(*d, *d->pages[0], QStringLiteral("ph:title")), QStringLiteral("Quarterly Results"));
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("Remember the demo"));
        cfb::File f;
        QVERIFY(cfb::read(open, &f));
        QCOMPARE(f.stream(QStringLiteral("Pictures")), blip + secondBlip);
        QCOMPARE(f.stream(pptDocumentStream()).size() > 0, true);

        // the other keys sizes the provider makes
        QByteArray small;
        QCOMPARE(office::ppt::decryptPpt(encryptPpt(plain, QStringLiteral("p\u00e4ss"), 40), QStringLiteral("p\u00e4ss"), 1 << 28, false, &small), office::CryptResult::Ok);
        QVERIFY(deck::importBinaryPresentation(small, &err));

        QByteArray none = QByteArray("kept");
        QCOMPARE(office::ppt::decryptPpt(locked, QStringLiteral("Secret"), 1 << 28, false, &none), office::CryptResult::WrongPassword);
        QCOMPARE(none, QByteArray("kept"));
        QCOMPARE(office::ppt::decryptPpt(locked, QString(), 1 << 28, true, &none), office::CryptResult::WrongPassword);
        QCOMPARE(office::ppt::decryptPpt(locked, QStringLiteral("secret"), 1 << 28, true, &none), office::CryptResult::Ok);
        QCOMPARE(none, QByteArray("kept"));
        QCOMPARE(office::ppt::decryptPpt(locked, QStringLiteral("secret"), 100, false, &none), office::CryptResult::TooLarge);
    }

    // A protection this program doesn't read says so, and a damaged one is refused: never a half-opened file.
    void protectedBinaryPresentationRefusesWhatItCannotRead()
    {
        const QByteArray plain = buildPpt();
        auto result = [](const QByteArray &file) {
            QByteArray out;
            return office::ppt::decryptPpt(file, QStringLiteral("secret"), 1 << 28, false, &out);
        };
        QCOMPARE(result(encryptPpt(plain, QStringLiteral("secret"))), office::CryptResult::Ok);
        // another cipher (AES): the header says so
        QCOMPARE(result(encryptPpt(plain, QStringLiteral("secret"), 128, 0x24, 0x660E)), office::CryptResult::Unsupported);
        QCOMPARE(result(encryptPpt(plain, QStringLiteral("secret"), 128, 0x04, 0x660E)), office::CryptResult::Unsupported);

        const QByteArray locked = encryptPpt(plain, QStringLiteral("secret"));
        cfb::File f;
        QVERIFY(cfb::read(locked, &f));
        const QByteArray stream = f.stream(pptDocumentStream());
        const qsizetype container = stream.indexOf(pptRec(12052, QByteArray(), 0, 0xF).left(4));
        QVERIFY(container > 0);
        // the description cut short (its record says it is shorter), or a header that claims more than there is
        QByteArray cut = stream;
        cut.replace(container + 4, 4, le(10, 4));
        QCOMPARE(result(withStream(locked, pptDocumentStream(), cut)), office::CryptResult::Damaged);
        QByteArray big = stream;
        big.replace(container + 8 + 8, 4, le(0x7FFFFFF0, 4));
        QCOMPARE(result(withStream(locked, pptDocumentStream(), big)), office::CryptResult::Damaged);
        // a record that is no description where the edit points; and a stream cut in half
        QByteArray notIt = stream;
        notIt.replace(container + 2, 2, le(1000, 2));
        QCOMPARE(result(withStream(locked, pptDocumentStream(), notIt)), office::CryptResult::Damaged);
        QCOMPARE(result(withStream(locked, pptDocumentStream(), stream.left(stream.size() / 2))), office::CryptResult::Damaged);
        QCOMPARE(result(QByteArray("not a compound file")), office::CryptResult::Damaged);
        // an edit that is not where the Current User stream says
        QByteArray user = f.stream(QStringLiteral("Current User"));
        QByteArray noEdit = user;
        noEdit.replace(16, 4, le(3, 4));
        QCOMPARE(result(withStream(locked, QStringLiteral("Current User"), noEdit)), office::CryptResult::Damaged);
        // through the reader: whatever the file is, it is refused with a reason and nothing is left half read
        QString err;
        QVERIFY(!deck::importBinaryPresentation(withStream(locked, pptDocumentStream(), cut), &err));
        QVERIFY(!err.isEmpty());
    }

    // The limits on what a protected file makes the reader do before a password is used (and after): each of these is Ok with its limit lifted, and refused with it.
    void protectedBinaryPresentationBoundsWhatItReads()
    {
        const QString pw = QStringLiteral("secret");
        const QByteArray plain = buildPpt();
        const QByteArray locked = encryptPpt(plain, pw);
        auto result = [&](const QByteArray &file, const std::function<bool()> &cancelled = nullptr) {
            QByteArray out;
            return office::ppt::decryptPpt(file, pw, 1 << 28, false, &out, cancelled);
        };
        auto streamOf = [](const QByteArray &file) {
            cfb::File f;
            cfb::read(file, &f);
            return f.stream(pptDocumentStream());
        };
        const QByteArray stream = streamOf(locked);
        QCOMPARE(result(locked), office::CryptResult::Ok);
        const QByteArray header = pptRec(1000, QByteArray());   // a record with nothing in it: where a persist object that is only a header goes

        // edits: 1,000 are followed (the file has three), the 1,001st is refused
        QList<QByteArray> empty;
        for (int i = 0; i < office::ppt::kMaxPptEdits - 3; ++i) empty << persistDirectory({});
        QCOMPARE(result(appendEdits(locked, QByteArray(), empty)), office::CryptResult::Ok);
        empty << persistDirectory({});
        QCOMPARE(result(appendEdits(locked, QByteArray(), empty)), office::CryptResult::Damaged);
        // edits that go round in a circle
        const quint32 newest = quint32(quint8(f_user(locked)[16])) | quint32(quint8(f_user(locked)[17])) << 8 | quint32(quint8(f_user(locked)[18])) << 16;
        QByteArray round = stream;
        round.replace(newest + 8 + 8, 4, le(newest, 4));
        QCOMPARE(result(withStream(locked, pptDocumentStream(), round)), office::CryptResult::Damaged);

        // the words of the directories, all together: a stream that is mostly one directory of empty entries is refused (one that is little is not)
        QCOMPARE(result(appendEdits(locked, QByteArray(), {pptRec(6002, QByteArray(40, 0))})), office::CryptResult::Ok);
        QCOMPARE(result(appendEdits(locked, QByteArray(), {pptRec(6002, QByteArray(20000, 0))})), office::CryptResult::Damaged);
        // directories that lie inside one another are refused
        {
            const QByteArray inner = pptRec(6002, QByteArray()), outer = pptRec(6002, inner);   // (the inner atom, read as entries, is one id at the start of the stream)
            const QByteArray two = appendEdits(locked, QByteArray(), {outer, persistDirectory({})});
            QCOMPARE(result(two), office::CryptResult::Ok);
            QByteArray s = streamOf(two);
            const qint64 outerAt = stream.size();
            const qint64 secondEdit = s.size() - 40;
            s.replace(secondEdit + 8 + 12, 4, le(quint32(outerAt + 8), 4));   // (the second edit names the atom inside the first directory)
            QCOMPARE(result(withStream(two, pptDocumentStream(), s)), office::CryptResult::Damaged);
        }

        // a record the length of the description limit and under it, and over it
        QCOMPARE(result(encryptPpt(plain, pw, 128, 0x04, 0x6801, int(office::ppt::kMaxDescription) - 200)), office::CryptResult::Ok);
        QCOMPARE(result(encryptPpt(plain, pw, 128, 0x04, 0x6801, int(office::ppt::kMaxDescription) + 1)), office::CryptResult::Damaged);

        // a persist object made here: its header only, where the stream ends, enciphered with the key of id 60
        const quint32 at = quint32(stream.size());
        const QByteArray object = pptCryptBlock(pw, 128, 60, header);
        QCOMPARE(result(appendEdits(locked, object, {persistDirectory({{60, at}})})), office::CryptResult::Ok);
        // an id listed twice in one directory is refused
        QCOMPARE(result(appendEdits(locked, object, {pptRec(6002, le(60 | (1u << 20), 4) + le(at, 4) + le(60 | (1u << 20), 4) + le(at, 4))})), office::CryptResult::Damaged);
        // a persist object that runs past the end of the stream (its length, once deciphered, is far more than the stream has)
        const QByteArray huge = pptCryptBlock(pw, 128, 60, pptRec(1000, QByteArray()).replace(4, 4, le(0x7FFFFFF0, 4)));
        QCOMPARE(result(appendEdits(locked, huge, {persistDirectory({{60, at}})})), office::CryptResult::Damaged);
        // two persist objects that overlap: a second one that starts inside the master's record (its bytes are made to give a header once the master is deciphered)
        {
            cfb::File pf;
            QVERIFY(cfb::read(plain, &pf));
            const QByteArray plainStream = pf.stream(pptDocumentStream());
            const quint32 masterAt = pptPersistOffsets(plainStream, quint32(quint8(f_user(plain)[16])) | quint32(quint8(f_user(plain)[17])) << 8 | quint32(quint8(f_user(plain)[18])) << 16).key(3);
            const qint64 masterLength = 8 + qint64(quint8(plainStream[masterAt + 4])) + (qint64(quint8(plainStream[masterAt + 5])) << 8);
            const quint32 inside = masterAt + 16;
            QByteArray s = stream;
            const QByteArray keystream = pptCryptBlock(pw, 128, 3, QByteArray(int(masterLength), 0)).mid(16, 8);
            QByteArray want = pptCryptBlock(pw, 128, 60, header);
            for (int i = 0; i < 8; ++i) want[i] = char(want[i] ^ keystream[i]);
            s.replace(inside, 8, want);
            QCOMPARE(result(appendEdits(withStream(locked, pptDocumentStream(), s), QByteArray(), {persistDirectory({{60, inside}})})), office::CryptResult::Damaged);
        }
        // many ids that name one offset: more than eight are refused
        {
            QByteArray many;
            for (int i = 0; i < 9; ++i) many += le((100 + i) | (1u << 20), 4) + le(0, 4);
            QCOMPARE(result(appendEdits(locked, QByteArray(), {pptRec(6002, many)})), office::CryptResult::Damaged);
        }
        // ids freed and left at the start of the stream (where the document is): the object is read with the id that fits
        {
            QByteArray out;
            QCOMPARE(office::ppt::decryptPpt(appendEdits(locked, QByteArray(), {persistDirectory({{61, 0}, {62, 0}})}), pw, 1 << 28, false, &out), office::CryptResult::Ok);
            QString err;
            QVERIFY2(deck::importBinaryPresentation(out, &err), qPrintable(err));
        }

        // cancelling: asked in every phase, and each place it is asked gives Cancelled
        int asked = 0;
        QCOMPARE(result(locked, [&] { ++asked; return false; }), office::CryptResult::Ok);
        QVERIFY(asked >= 6);
        for (int at2 : {1, 2, asked / 2, asked - 1, asked}) {
            int n = 0;
            QCOMPARE(result(locked, [&] { return ++n >= at2; }), office::CryptResult::Cancelled);
        }
    }

    // Every edit of the file that carries the name of the description has it cleared (the reader refuses an edit that has one), not only the newest.
    void protectedBinaryPresentationClearsEveryNameOfTheDescription()
    {
        const QString pw = QStringLiteral("secret");
        const QByteArray locked = appendEdits(encryptPpt(buildPpt(), pw), QByteArray(), {persistDirectory({})});
        QByteArray out;
        QCOMPARE(office::ppt::decryptPpt(locked, pw, 1 << 28, false, &out), office::CryptResult::Ok);
        QString err;
        auto d = deck::importBinaryPresentation(out, &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);
    }

    // The kind of protection is known before a password is: the first form of the 40-bit kind, and another cipher, are Unsupported with no password and with the right one; a
    // presentation is told from a document by its small Current User stream, and a file that is neither is left alone.
    void protectedBinaryPresentationKindIsReadFirst()
    {
        const QString pw = QStringLiteral("secret");
        const QByteArray first = encryptPpt(buildPpt(), pw, 128, 0x04, 0x6801, 0, 1, 1);
        QByteArray out;
        QCOMPARE(office::ppt::decryptPpt(first, QString(), 1 << 28, true, &out), office::CryptResult::Unsupported);
        QCOMPARE(office::ppt::decryptPpt(first, pw, 1 << 28, false, &out), office::CryptResult::Unsupported);
        QCOMPARE(office::ppt::decryptPpt(encryptPpt(buildPpt(), pw, 128, 0x24, 0x660E), QString(), 1 << 28, true, &out), office::CryptResult::Unsupported);
        QCOMPARE(office::ppt::decryptPpt(encryptPpt(buildPpt(), pw), QString(), 1 << 28, true, &out), office::CryptResult::WrongPassword);

        bool isPpt = false;
        QVERIFY(office::ppt::pptEncrypted(first, &isPpt));
        QVERIFY(isPpt);
        isPpt = false;
        QVERIFY(!office::ppt::pptEncrypted(buildPpt(), &isPpt));
        QVERIFY(isPpt);
        // a compound file with no Current User stream is not a presentation
        cfb::File f;
        QVERIFY(cfb::read(buildPpt(), &f));
        f.entries[0].children.removeAll(f.find(QStringLiteral("Current User")));
        QVERIFY(!office::ppt::pptEncrypted(cfb::write(f), &isPpt));
        QVERIFY(!isPpt);
    }

    // The test for a presentation reads its one small stream, not the whole file: the work of following sector chains for a presentation with a large Pictures stream is a small
    // part of what reading it whole takes.
    void aPresentationIsToldFromTheSmallStreamAlone()
    {
        const QByteArray big = withStream(buildPpt(), QStringLiteral("Pictures"), QByteArray(3 << 20, 'z'));
        const qint64 start = cfb::chainWorkForTest();
        QVERIFY(!office::ppt::pptEncrypted(big));
        const qint64 probe = cfb::chainWorkForTest() - start;
        cfb::File f;
        QVERIFY(cfb::read(big, &f));
        const qint64 whole = cfb::chainWorkForTest() - start - probe;
        QVERIFY2(probe * 10 < whole, qPrintable(QStringLiteral("%1 against %2").arg(probe).arg(whole)));
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
        d = deck::importBinaryPresentation(pptOf(entries, {{5, pptSlideOf(pptTinyShape().repeated(20000))}}), &err);
        { const auto w = jp::deck::lastPptImportWorkForTest();
          // (Counted: the drawing of 20,000 shapes is read once, though twelve entries name it; the clock said so before.)
          QVERIFY2(w.shapesTaken <= 20000 + 16, qPrintable(QString::number(w.shapesTaken))); }
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 1);
        QCOMPARE(int(d->pages[0]->items.size()), 20000);
        QVERIFY2(err.contains(QStringLiteral("Slide 2 was left out")) && err.contains(QStringLiteral("Slide 12 was left out")), qPrintable(err));
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
        d = deck::importBinaryPresentation(pptFileOf(s, newest), &err);
        { const auto w = jp::deck::lastPptImportWorkForTest();
          // (Each of the million ids of the directory the 300 edits share is listed once, not once for each edit.)
          QVERIFY2(w.persistRead <= 1000000 + 16, qPrintable(QString::number(w.persistRead))); }
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 1);
        QVERIFY2(err.isEmpty(), qPrintable(err));
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
        d = deck::importBinaryPresentation(pptFileOf(s, newest), &err);
        { const auto w = jp::deck::lastPptImportWorkForTest();
          // (The ids listed are no more than the stream has bytes.)
          QVERIFY2(w.persistRead <= pptFileOf(s, newest).size(), qPrintable(QString::number(w.persistRead))); }
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 1);
        QVERIFY2(err.contains(QStringLiteral("older edits")), qPrintable(err));
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
        d = deck::importBinaryPresentation(pptOf(slidePersist(5, 0, 0x100), {{5, pptSlideOf(frames)}}, group), &err);
        { const auto w = jp::deck::lastPptImportWorkForTest();
          // (The store is passed once to be indexed, and a frame's picture is found by its number.)
          QVERIFY2(w.storeEntriesVisited <= kPictures + 16 && w.shapesTaken >= kPictures, qPrintable(QString::number(w.storeEntriesVisited))); }
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 1);
        QCOMPARE(int(d->pages[0]->items.size()), kPictures);
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
        decltype(import(6)) six = import(6), three = import(3);
        QVERIFY2(six.first, qPrintable(six.second));
        QVERIFY2(six.second.contains(QStringLiteral("2 pictures were left out")), qPrintable(six.second));
        QVERIFY2(three.first, qPrintable(three.second));
        QVERIFY2(!three.second.contains(QStringLiteral("were left out")), qPrintable(three.second));
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
        d = deck::importBinaryPresentation(pptFileOf(s, edit), &err);
        { const auto w = jp::deck::lastPptImportWorkForTest();
          // (The list of 500,000 is passed to be indexed, twice at the most, and not for each of the 8,000 slides.)
          QVERIFY2(w.notesVisited <= 2 * 50 * 10000 + 16, qPrintable(QString::number(w.notesVisited))); }
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), kSlides);
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
        d = deck::importBinaryPresentation(pptFileOf(s, edit), &err);
        { const auto w = jp::deck::lastPptImportWorkForTest();
          // (The master's 40,001 shapes are passed once to be indexed, and not for each of the 10,000 placeholders.)
          QVERIFY2(w.placeholderItemsVisited <= 2 * 40001, qPrintable(QString::number(w.placeholderItemsVisited))); }
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages[0]->items.size()), 10000);
        for (const auto &it : d->pages[0]->items) QCOMPARE(it->rect, QRectF(25, 12.5, 600, 100));
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

private:
    // ---- Open and Repair of a damaged presentation

    // A presentation whose stream is long (the first slide holds a record of 9,000 bytes the reader does not use), so that it is kept in sectors of its own and not in the container's mini stream.
    static QByteArray bigDeck()
    {
        const QByteArray none;
        return buildPpt(false, false, false, false, none, none, none, none, none, none, 1016, none, pptRec(2999, QByteArray(9000, 'x')));
    }
    static quint32 rd32(const QByteArray &b, int at) { return at + 4 <= b.size() ? qFromLittleEndian<quint32>(b.constData() + at) : 0; }
    static void zeroSector(QByteArray &f, int n) { f.replace((n + 1) * 512, 512, QByteArray(512, '\0')); }
    // The offset in the stream of the first record of a kind at its top.
    static int topRecordOf(const QByteArray &s, int type)
    {
        for (int pos = 0; pos + 8 <= s.size();) {
            if (rd32(s, pos) >> 16 == quint32(type)) return pos;
            pos += 8 + int(rd32(s, pos + 4));
        }
        return -1;
    }
    static QStringList titles(const Document &d)
    {
        QStringList out = titlesOf(d);
        out.sort();
        return out;
    }
    // The file repaired and opened: the document, and what was said.
    static std::unique_ptr<Document> repaired(const QByteArray &file, QStringList *notes, QString *error = nullptr)
    {
        const QByteArray fixed = deck::salvagePresentation(file, notes, error);
        return fixed.isEmpty() ? nullptr : deck::importBinaryPresentation(fixed, error);
    }

private Q_SLOTS:
    void aSoundPresentationIsRepairedToItself()
    {
        QStringList notes;
        QString why;
        auto d = repaired(buildPpt(), &notes, &why);
        QVERIFY2(d, qPrintable(why));
        QCOMPARE(int(d->pages.size()), 2);
        QCOMPARE(titles(*d), (QStringList{"Plan", "Quarterly Results"}));
        QVERIFY2(notes.isEmpty(), qPrintable(notes.join(QLatin1Char('|'))));
    }

    void aPresentationWhoseLastEditIsCutOffOpensAsItWasAtTheEditBefore()
    {
        const QByteArray good = buildPpt();
        cfb::File f;
        QVERIFY(cfb::read(good, &f));
        QByteArray stream = f.stream(pptDocumentStream());
        stream.chop(20);   // (the newest edit and its list of places are the end of the stream)
        const QByteArray cut = withStream(good, pptDocumentStream(), stream);
        QVERIFY(!deck::importBinaryPresentation(cut));
        QStringList notes;
        QString why;
        auto d = repaired(cut, &notes, &why);
        QVERIFY2(d, qPrintable(why));
        QCOMPARE(int(d->pages.size()), 2);
        QVERIFY(!d->pages[1]->hidden);   // (the hiding is the newest edit's)
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("last save")));
    }

    void aPresentationCutInsideItsSlidesHasTheListOfPlacesMadeAgain()
    {
        const QByteArray good = bigDeck();
        cfb::File f;
        QVERIFY(cfb::read(good, &f));
        const QByteArray stream = f.stream(pptDocumentStream());
        // (the stream is cut in the notes, the last record before the first list of places)
        const int directory = topRecordOf(stream, 6002);
        QVERIFY(directory > 100);
        const QByteArray cut = withStream(good, pptDocumentStream(), stream.left(directory - 10));
        QVERIFY(!deck::importBinaryPresentation(cut));
        QStringList notes;
        QString why;
        auto d = repaired(cut, &notes, &why);
        QVERIFY2(d, qPrintable(why));
        QCOMPARE(int(d->pages.size()), 2);   // (the slides are in the order stored, which this file does not keep in its list's order)
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("order")));
    }

    void aPresentationFileCutShortKeepsTheSlidesBeforeTheCut()
    {
        const QByteArray good = bigDeck();
        int before = 1 << 20;
        for (int percent : {95, 70, 50}) {
            const QByteArray cut = good.left(good.size() * percent / 100);
            QVERIFY(!deck::importBinaryPresentation(cut));   // (the tables at the end that name the streams are gone)
            QStringList notes;
            QString why;
            auto d = repaired(cut, &notes, &why);
            QVERIFY2(d, qPrintable(QString::number(percent) + QLatin1Char(' ') + why));
            QVERIFY(int(d->pages.size()) <= before);
            before = int(d->pages.size());
            QVERIFY(d->pages.size() >= 1 && d->pages.size() <= 2);
            QVERIFY(!notes.isEmpty());
            if (percent == 50) {
                QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("cut short")));
                QCOMPARE(int(d->pages.size()), 1);   // (the first slide, which is the large one, is cut in the middle)
            }
        }
        // cut before the slides: nothing to keep
        QStringList notes;
        QString why;
        QVERIFY(deck::salvagePresentation(good.left(1000), &notes, &why).isEmpty());
        QVERIFY(!why.isEmpty());
    }

    void aPresentationWhoseTablesOfSectorsAreZeroedOrLoopIsRepaired()
    {
        const QByteArray good = bigDeck();
        // the table of sectors zeroed
        QByteArray bad = good;
        zeroSector(bad, int(rd32(bad, 76)));
        QStringList notes;
        QString why;
        auto d = repaired(bad, &notes, &why);
        QVERIFY2(d, qPrintable(why));
        QCOMPARE(int(d->pages.size()), 2);
        QVERIFY(!notes.isEmpty());
        // a chain that leads back to the second sector of the stream
        bad = good;
        cfb::File f;
        QVERIFY(cfb::read(good, &f));
        const int root = (int(rd32(bad, 48)) + 1) * 512;
        int start = -1;
        for (int o = root; o < root + 512; o += 128)
            if ((rd32(bad, o + 64) & 0xFFFF) == 2 * (QStringLiteral("PowerPoint Document").size() + 1)) start = int(rd32(bad, o + 116));
        QVERIFY(start >= 0);
        qToLittleEndian(quint32(start + 1), bad.data() + (int(rd32(bad, 76)) + 1) * 512 + (start + 4) * 4);
        d = repaired(bad, &notes, &why);
        QVERIFY2(d, qPrintable(why));
        QCOMPARE(int(d->pages.size()), 2);
    }

    void aPresentationWhoseDirectoryIsZeroedIsFoundByItsRecords()
    {
        QByteArray bad = bigDeck();
        zeroSector(bad, int(rd32(bad, 48)));
        QVERIFY(!deck::importBinaryPresentation(bad));
        QStringList notes;
        QString why;
        auto d = repaired(bad, &notes, &why);
        QVERIFY2(d, qPrintable(why));
        QCOMPARE(titles(*d), (QStringList{"Plan", "Quarterly Results"}));
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("directory")));
    }

    void aPresentationProtectedWithAPasswordIsNotRepaired()
    {
        const QByteArray locked = encryptPpt(buildPpt(), QStringLiteral("pw"));
        QStringList notes;
        QString why;
        QVERIFY(deck::salvagePresentation(locked, &notes, &why).isEmpty());
        QVERIFY2(why.contains(QLatin1String("password")), qPrintable(why));
    }

    void theRecordsOfAStreamAreWalkedOnlyToTheLimit()
    {
        // a document record, then more empty records than the limit
        cfb::File f;
        QVERIFY(cfb::read(buildPpt(), &f));
        QByteArray stream = f.stream(pptDocumentStream());
        const QByteArray empty = pptRec(1100, QByteArray());
        QByteArray many;
        many.reserve(empty.size() * (deck::kSalvageMaxRecords + 200000));
        for (int i = 0; i < deck::kSalvageMaxRecords + 200000; ++i) many += empty;
        stream += many;
        QStringList notes;
        QString why;
        deck::salvagePresentation(withStream(buildPpt(), pptDocumentStream(), stream), &notes, &why);
        QCOMPARE(deck::salvageRecordsWalkedForTest(), qint64(deck::kSalvageMaxRecords));
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

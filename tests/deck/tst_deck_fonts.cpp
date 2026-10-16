// JeffDeck's tests of fonts a file carries.
#include "deckhelpers.h"

class DeckFontsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // ----- Fonts in the file: embedded on save, and used while the presentation is open -----

    // A saved file with the program's own fonts embedded: the fonts its text uses, in the styles it uses, each as a part
    // of its own (an EOT of the whole font) that the presentation lists after its sizes, and says it embeds.
    void savedPresentationEmbedsTheBundledFontsItUses()
    {
        auto d = deck::newPresentation();
        deck::DeckTheme theme = deck::themeOf(*d);
        theme.headingFont = QStringLiteral("Fraunces");
        theme.bodyFont = QStringLiteral("Source Serif 4");
        deck::setTheme(*d, theme);
        auto *title = static_cast<TextItem *>(d->pages[0]->items[0].get());
        auto *subtitle = static_cast<TextItem *>(d->pages[0]->items[1].get());
        auto run = [&](TextItem *box, const QString &text, const QString &family, bool bold, bool italic) {
            QTextCursor c(d->storyDoc(box->storyId));
            c.movePosition(QTextCursor::End);
            QTextCharFormat f;
            f.setFontFamilies({family});
            f.setFontWeight(bold ? QFont::Bold : QFont::Normal);
            f.setFontItalic(italic);
            c.insertText(text, f);
        };
        run(title, QStringLiteral("Heading"), QStringLiteral("Fraunces"), false, false);
        run(subtitle, QStringLiteral("body "), QStringLiteral("Source Serif 4"), false, false);
        run(subtitle, QStringLiteral("bold "), QStringLiteral("Source Serif 4"), true, false);
        run(subtitle, QStringLiteral("italic "), QStringLiteral("Source Serif 4"), false, true);
        // Fonts every computer has are never embedded.
        run(subtitle, QStringLiteral("system "), QStringLiteral("Calibri"), true, false);
        run(subtitle, QStringLiteral("system "), QStringLiteral("Arial"), false, false);

        QString err;
        const QByteArray plain = deck::exportPresentation(*d, &err);
        const QByteArray bytes = deck::exportPresentation(*d, &err, nullptr, deck::PresentationKind::Presentation, true);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString pres = part(e, QStringLiteral("/ppt/presentation.xml"));
        // The file says it embeds (without that the program doesn't look), and the list is where the schema puts it.
        QVERIFY2(pres.contains(QStringLiteral(" embedTrueTypeFonts=\"1\"")), qPrintable(pres.left(500)));
        QVERIFY(!pres.contains(QStringLiteral("saveSubsetFonts")));   // all the characters are in
        const int sizes = pres.indexOf(QStringLiteral("<p:notesSz")), list = pres.indexOf(QStringLiteral("<p:embeddedFontLst>")), text = pres.indexOf(QStringLiteral("<p:defaultTextStyle>"));
        QVERIFY(sizes > 0 && list > sizes && text > list);
        const auto fonts = embeddedFontsOf(e);
        QVERIFY2(fonts.contains(QStringLiteral("Fraunces")) && fonts.contains(QStringLiteral("Source Serif 4")), qPrintable(fonts.keys().join(QStringLiteral(", "))));
        QVERIFY(!fonts.contains(QStringLiteral("Calibri")) && !fonts.contains(QStringLiteral("Arial")));
        // Only the styles the text uses: the heading font regular; the body font regular, bold, and italic, but not both.
        QCOMPARE(QStringList(fonts.value(QStringLiteral("Fraunces")).keys()), (QStringList{"regular"}));
        QCOMPARE(QStringList(fonts.value(QStringLiteral("Source Serif 4")).keys()), (QStringList{"bold", "italic", "regular"}));
        for (auto family = fonts.cbegin(); family != fonts.cend(); ++family)
            for (auto face = family->cbegin(); face != family->cend(); ++face) {
                QByteArray font;
                FontFileInfo info;
                QString why;
                QVERIFY2(office::readEmbeddedFont(face.value(), &font, &info, &why), qPrintable(family.key() + QLatin1Char(' ') + face.key() + QLatin1String(": ") + why));
                if (family.key() != QLatin1String("Fraunces") && family.key() != QLatin1String("Source Serif 4")) continue;
                QCOMPARE(info.family, family.key());
                QCOMPARE(info.bold, face.key() == QLatin1String("bold"));
                QCOMPARE(info.italic, face.key() == QLatin1String("italic"));
                // The font's own file, whole: the characters are all there so the text can be edited.
                QCOMPARE(font, fontFileOf(family.key(), info.bold, info.italic));
            }
        // The part's name, type, and link are the program's own.
        const QString rId = [&] {
            const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("<p:embeddedFont><p:font typeface=\"Fraunces\"[^>]*/><p:regular r:id=\"(rId\\d+)\"/>")).match(pres);
            return m.captured(1);
        }();
        QVERIFY2(!rId.isEmpty(), qPrintable(pres));
        const office::Relationship link = e.pkg.relationship(QStringLiteral("/ppt/presentation.xml"), rId);
        QVERIFY(link.type.endsWith(QLatin1String("/relationships/font")));
        QVERIFY2(link.target.startsWith(QLatin1String("/ppt/fonts/font")) && link.target.endsWith(QLatin1String(".fntdata")), qPrintable(link.target));
        QCOMPARE(e.pkg.contentType(link.target), QStringLiteral("application/x-fontdata"));
        // Saved without them, the file is as it was before there were any.
        QVERIFY(!QString::fromUtf8(plain).isEmpty());
        QVERIFY(deck::exportPresentation(*d, &err, nullptr, deck::PresentationKind::Presentation, false) == plain);
        Exported none;
        QVERIFY2(unpack(plain, &none, &err), qPrintable(err));
        QVERIFY(!part(none, QStringLiteral("/ppt/presentation.xml")).contains(QStringLiteral("embedded")));
        QVERIFY(!part(none, QStringLiteral("/ppt/presentation.xml")).contains(QStringLiteral("embedTrueTypeFonts")));
        for (const QString &name : none.pkg.partNames()) QVERIFY2(!name.startsWith(QLatin1String("/ppt/fonts/")), qPrintable(name));
    }

    // Wherever the text is: the theme's fonts (used by any text that follows it), a master's prompts, a notes page, and a table's cells.
    void fontsInMastersNotesAndTablesAreEmbeddedToo()
    {
        const QString id = QStringLiteral("{11111111-2222-3333-4444-555555555555}");
        Parts p = samplePptx();
        p.data["/ppt/tableStyles.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.tableStyles+xml"),
            QStringLiteral("<a:tblStyleLst xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" def=\"%1\"><a:tblStyle styleId=\"%1\" styleName=\"Test\"><a:wholeTbl><a:tcTxStyle><a:fontRef idx=\"minor\"/>"
                           "<a:srgbClr val=\"202020\"/></a:tcTxStyle><a:tcStyle><a:tcBdr/></a:tcStyle></a:wholeTbl></a:tblStyle></a:tblStyleLst>").arg(id).toUtf8()};
        p.rels << Rel{"/ppt/presentation.xml", "tableStyles", "/ppt/tableStyles.xml"};
        const QString cell = QStringLiteral("<a:tc><a:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\" b=\"1\"><a:latin typeface=\"Raleway\"/></a:rPr><a:t>Quarter</a:t></a:r></a:p></a:txBody><a:tcPr/></a:tc>");
        const QString table = QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"8\" name=\"Results\"/><p:cNvGraphicFramePr><a:graphicFrameLocks noGrp=\"1\"/></p:cNvGraphicFramePr><p:nvPr/></p:nvGraphicFramePr>"
                                             "<p:xfrm><a:off x=\"914400\" y=\"1828800\"/><a:ext cx=\"1828800\" cy=\"370840\"/></p:xfrm>"
                                             "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"><a:tbl><a:tblPr firstRow=\"1\"><a:tableStyleId>%1</a:tableStyleId></a:tblPr>"
                                             "<a:tblGrid><a:gridCol w=\"1828800\"/></a:tblGrid><a:tr h=\"370840\">%2</a:tr></a:tbl></a:graphicData></a:graphic></p:graphicFrame>")
                                     .arg(id, cell);
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QVERIFY(xml.contains(unknownFrame(8)));
        xml.replace(unknownFrame(8), table);
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        // A master's prompt, and the notes of a slide, set in other families.
        auto setFamily = [&](const QString &storyId, const QString &family, bool italic) {
            QTextCursor c(d->storyDoc(storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontFamilies({family});
            f.setFontItalic(italic);
            c.mergeCharFormat(f);
        };
        TextItem *prompt = nullptr;
        for (const auto &it : d->masters[0]->items)
            if (it->type() == ItemType::Text) {
                prompt = static_cast<TextItem *>(it.get());
                break;
            }
        QVERIFY(prompt);
        setFamily(prompt->storyId, QStringLiteral("Oswald"), false);
        const QString notes = d->createStory(QStringLiteral("Speaker notes"));
        d->pages[0]->notesStoryId = notes;
        setFamily(notes, QStringLiteral("Lato"), true);

        const QByteArray bytes = deck::exportPresentation(*d, &err, nullptr, deck::PresentationKind::Presentation, true);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const auto fonts = embeddedFontsOf(e);
        QVERIFY2(fonts.value(QStringLiteral("Raleway")).contains(QStringLiteral("bold")), qPrintable(fonts.keys().join(QStringLiteral(", "))));   // a table cell's
        QVERIFY2(fonts.contains(QStringLiteral("Oswald")), qPrintable(fonts.keys().join(QStringLiteral(", "))));                                      // a master's
        QVERIFY2(fonts.value(QStringLiteral("Lato")).contains(QStringLiteral("italic")), qPrintable(fonts.keys().join(QStringLiteral(", "))));       // a notes page's
        // The theme's own fonts: the sample's theme names the font scheme "Calibri", which no one needs from the file.
        QVERIFY(!fonts.contains(QStringLiteral("Calibri")));
    }

    // A font whose license says it may not travel, or that is not one the program ships, is never embedded.
    void fontsThatMayNotTravelOrAreNotTheProgramsAreLeftOut()
    {
        auto d = deck::newPresentation();
        auto *title = static_cast<TextItem *>(d->pages[0]->items[0].get());
        QTextCursor c(d->storyDoc(title->storyId));
        auto run = [&](const QString &family, bool bold, bool italic) {
            QTextCharFormat f;
            f.setFontFamilies({family});
            f.setFontWeight(bold ? QFont::Bold : QFont::Normal);
            f.setFontItalic(italic);
            c.insertText(QStringLiteral("x "), f);
        };
        // Zilla Slab Bold Italic says "preview and print only"; the family's other styles say editable.
        run(QStringLiteral("Zilla Slab"), false, false);
        run(QStringLiteral("Zilla Slab"), true, true);
        // The TeX Gyre fonts say "preview and print, and editable": the freest applies.
        run(QStringLiteral("TeX Gyre Bonum"), false, false);
        run(QStringLiteral("Not A Font Anyone Has"), false, false);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err, nullptr, deck::PresentationKind::Presentation, true);
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const auto fonts = embeddedFontsOf(e);
        QCOMPARE(QStringList(fonts.value(QStringLiteral("Zilla Slab")).keys()), (QStringList{"regular"}));
        QVERIFY(fonts.value(QStringLiteral("TeX Gyre Bonum")).contains(QStringLiteral("regular")));
        QVERIFY(!fonts.contains(QStringLiteral("Not A Font Anyone Has")));
    }

    // What a file's embedded fonts do on opening: a font the computer lacks is available while the presentation is open
    // (never installed, never written to disk) and goes with the last presentation that carries it; a font the computer
    // already has is not added.
    void embeddedFontsAreUsedWhileThePresentationIsOpen()
    {
        const QByteArray font = renamedFont(fontFileOf(QStringLiteral("Lobster"), false, false), QStringLiteral("Lobster"), QStringLiteral("Zobster"));
        QVERIFY(!font.isEmpty());
        const int before = deck::EmbeddedFonts::addedFontCount();
        const int generation = jp::fontGeneration();   // (what the text engine remembers of which families there are goes with it)
        QVERIFY(!QFontDatabase::families().contains(QStringLiteral("Zobster")));
        QString err;
        auto first = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), {{QStringLiteral("Zobster"), {{0, office::makeEot(font)}}}})), &err);
        QVERIFY2(first, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before + 1);
        QVERIFY(jp::fontGeneration() > generation);
        // Never under its own name: the presentation says which private family its text in Zobster is drawn with.
        QVERIFY(!QFontDatabase::families().contains(QStringLiteral("Zobster")));
        QVERIFY(first->fontAliases.contains(QStringLiteral("zobster")));
        const QString privateFamily = first->fontAliases.value(QStringLiteral("zobster"));
        QVERIFY2(privateFamily.startsWith(jp::kEmbeddedFamilyPrefix) && privateFamily.endsWith(QStringLiteral("-Zobster")), qPrintable(privateFamily));
        QVERIFY(QFontDatabase::families().contains(privateFamily));
        QVERIFY(!jp::visibleFontFamilies().contains(privateFamily));   // no menu lists it
        QCOMPARE(QFontInfo(QFont(privateFamily)).family(), privateFamily);
        // Another presentation with the same font shares it.
        auto second = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), {{QStringLiteral("Zobster"), {{0, office::makeEot(font)}}}})), &err);
        QVERIFY(second);
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before + 1);
        QCOMPARE(second->fontAliases.value(QStringLiteral("zobster")), privateFamily);
        deck::EmbeddedFonts::release(first.get());
        QVERIFY(QFontDatabase::families().contains(privateFamily));
        const int whileOpen = jp::fontGeneration();
        deck::EmbeddedFonts::release(second.get());
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before);
        QVERIFY(jp::fontGeneration() > whileOpen);
        QVERIFY(!QFontDatabase::families().contains(privateFamily));
        deck::EmbeddedFonts::release(second.get());   // twice is nothing
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before);

        // A font the computer has (Lato, which ships with the program) is not added again; the file's part is still kept for writing back.
        const int lato = deck::EmbeddedFonts::addedFontCount();
        auto have = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), {{QStringLiteral("Lato"), {{0, office::makeEot(fontFileOf(QStringLiteral("Lato"), false, false))}}}})), &err);
        QVERIFY2(have, qPrintable(err));
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), lato);
        QCOMPARE(int(deck::EmbeddedFonts::facesOf(have.get()).size()), 1);
        deck::EmbeddedFonts::release(have.get());
        QCOMPARE(int(deck::EmbeddedFonts::facesOf(have.get()).size()), 0);
    }

    // A family's styles are all available, not only the first: a font the computer lacks comes in a regular and a bold face, say.
    void allTheStylesOfAnEmbeddedFamilyAreAvailable()
    {
        QMap<int, QByteArray> faces;
        for (int slot = 0; slot < 4; ++slot) {
            const QByteArray font = renamedFont(fontFileOf(QStringLiteral("Lato"), slot & 1, slot & 2), QStringLiteral("Lato"), QStringLiteral("Zato"));
            QVERIFY(!font.isEmpty());
            faces[slot] = office::makeEot(font);
        }
        QVERIFY(!QFontDatabase::families().contains(QStringLiteral("Zato")));
        QString err;
        auto d = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), {{QStringLiteral("Zato"), faces}})), &err);
        QVERIFY2(d, qPrintable(err));
        const QString privateFamily = d->fontAliases.value(QStringLiteral("zato"));
        QVERIFY(!privateFamily.isEmpty());
        const QStringList styles = QFontDatabase::styles(privateFamily);
        QVERIFY2(styles.contains(QStringLiteral("Regular")) && styles.contains(QStringLiteral("Bold")) && styles.contains(QStringLiteral("Italic")) && styles.contains(QStringLiteral("Bold Italic")),
                 qPrintable(styles.join(QStringLiteral(", "))));
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), 4);
        // Bold asked for is the bold face's.
        QFont bold(privateFamily);
        bold.setWeight(QFont::Bold);
        QVERIFY(QFontInfo(bold).bold());
        deck::EmbeddedFonts::release(d.get());
        QVERIFY(!QFontDatabase::families().contains(privateFamily));
    }

    // An open document presentation carries its fonts as files in its Fonts folder: available while it is open, like a .pptx's.
    void anOpenDocumentPresentationsFontsAreUsedWhileItIsOpen()
    {
        const QByteArray font = renamedFont(fontFileOf(QStringLiteral("Lobster"), false, false), QStringLiteral("Lobster"), QStringLiteral("Zobster"));
        const int before = deck::EmbeddedFonts::addedFontCount();
        QString err;
        QStringList skipped;
        auto d = deck::importOdp(odptest::odpZip(odptest::sampleContent(), odptest::odpStyles(), {{QStringLiteral("Fonts/Zobster-Regular.ttf"), font}, {QStringLiteral("Fonts/notes.txt"), QByteArray("not a font")}}), &err, &skipped);
        QVERIFY2(d, qPrintable(err));
        const QString privateFamily = d->fontAliases.value(QStringLiteral("zobster"));
        QVERIFY(!privateFamily.isEmpty() && QFontDatabase::families().contains(privateFamily));
        QVERIFY(!QFontDatabase::families().contains(QStringLiteral("Zobster")));
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before + 1);
        // Kept for a .pptx to carry, in the form the presentation program reads.
        const QVector<deck::EmbeddedFace> faces = deck::EmbeddedFonts::facesOf(d.get());
        QCOMPARE(int(faces.size()), 1);
        QCOMPARE(faces[0].typeface, QStringLiteral("Zobster"));
        QByteArray inside;
        QVERIFY(office::readEmbeddedFont(faces[0].part, &inside));
        QCOMPARE(inside, font);
        QVERIFY(faces[0].part != font);   // wrapped, not the bare font
        deck::EmbeddedFonts::release(d.get());
        QVERIFY(!QFontDatabase::families().contains(privateFamily));
        // A part that isn't a font is said so.
        auto bad = deck::importOdp(odptest::odpZip(odptest::sampleContent(), odptest::odpStyles(), {{QStringLiteral("Fonts/Broken.ttf"), QByteArray("not a font at all, nor a header .............")}}), &err, &skipped);
        QVERIFY2(bad, qPrintable(err));
        QVERIFY2(skipped.join(QLatin1Char('\n')).contains(QStringLiteral("embedded font")), qPrintable(skipped.join(QLatin1Char('\n'))));
        deck::EmbeddedFonts::release(bad.get());
    }

    // A font whose license is the restricted one (no copy of it may be embedded) is not drawn with, though a file carries it; the license that
    // says preview and print only is no bar to drawing it. What the file carries is kept as it is.
    void aRestrictedFontInAFileIsNotUsed()
    {
        auto withLicense = [&](const QString &name, quint16 fsType) {
            QByteArray font = renamedFont(fontFileOf(QStringLiteral("Lobster"), false, false), QStringLiteral("Lobster"), name);
            for (int i = 0, n = qFromBigEndian<quint16>(font.constData() + 4); i < n; ++i)
                if (font.mid(12 + 16 * i, 4) == "OS/2") qToBigEndian<quint16>(fsType, font.data() + int(qFromBigEndian<quint32>(font.constData() + 12 + 16 * i + 8)) + 8);
            return font;
        };
        const int before = deck::EmbeddedFonts::addedFontCount();
        QString err;
        auto restricted = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), {{QStringLiteral("Robster"), {{0, office::makeEot(withLicense(QStringLiteral("Robster"), 0x0002))}}}})), &err);
        QVERIFY2(restricted, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before);
        QVERIFY(restricted->fontAliases.isEmpty());
        QCOMPARE(int(deck::EmbeddedFonts::facesOf(restricted.get()).size()), 1);
        deck::EmbeddedFonts::release(restricted.get());
        auto preview = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), {{QStringLiteral("Pobster"), {{0, office::makeEot(withLicense(QStringLiteral("Pobster"), 0x0004))}}}})), &err);
        QVERIFY2(preview, qPrintable(err));
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before + 1);
        QVERIFY(QFontDatabase::families().contains(preview->fontAliases.value(QStringLiteral("pobster"))));
        deck::EmbeddedFonts::release(preview.get());
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before);
    }

    // An embedded font is the font of its own presentation and nothing else's: where the computer has no Arial, a file's embedded "Arial" is
    // what that file's text in Arial is drawn with, but not what another presentation's, a menu's, or the program's own request for Arial gets.
    void anEmbeddedFontDoesNotTakeOverAFamilyNameForTheProgram()
    {
        if (QFontDatabase::families().contains(QStringLiteral("Arial"), Qt::CaseInsensitive)) QSKIP("this computer has Arial: the embedded one would not be used");
        auto plain = deck::importPresentation(pack(sampleWithRunIn(QStringLiteral("Arial"))));
        QVERIFY(plain);
        const QImage before = slideImage(*plain);
        const QString arialBefore = QFontInfo(QFont(QStringLiteral("Arial"))).family();
        const QStringList familiesBefore = QFontDatabase::families();
        const QByteArray cabin = renamedFont(fontFileOf(QStringLiteral("Cabin"), false, false), QStringLiteral("Cabin"), QStringLiteral("Arial"));
        QVERIFY(!cabin.isEmpty());
        QString err;
        auto carrier = deck::importPresentation(pack(withEmbeddedFonts(sampleWithRunIn(QStringLiteral("Arial")), {{QStringLiteral("Arial"), {{0, office::makeEot(cabin)}}}})), &err);
        QVERIFY2(carrier, qPrintable(err));
        QVERIFY(carrier->fontAliases.contains(QStringLiteral("arial")));
        // Its text is drawn with the embedded font: as the same text in Cabin is.
        auto cabinDeck = deck::importPresentation(pack(sampleWithRunIn(QStringLiteral("Cabin"))));
        QVERIFY(cabinDeck);
        const QImage embedded = slideImage(*carrier);
        QVERIFY(embedded == slideImage(*cabinDeck));
        QVERIFY(embedded != before);
        // Nothing else is changed: another presentation's Arial, the program's own, and the font lists.
        QVERIFY(slideImage(*plain) == before);
        QCOMPARE(QFontInfo(QFont(QStringLiteral("Arial"))).family(), arialBefore);
        QVERIFY(!QFontDatabase::families().contains(QStringLiteral("Arial"), Qt::CaseInsensitive));
        QStringList added = QFontDatabase::families();
        for (const QString &family : familiesBefore) added.removeAll(family);
        QCOMPARE(added.size(), 1);
        QVERIFY(added.first().startsWith(jp::kEmbeddedFamilyPrefix));
        QCOMPARE(jp::visibleFontFamilies(), familiesBefore);
        // And a presentation that has the embedded font is drawn with it only while it has it.
        deck::EmbeddedFonts::release(carrier.get());
        QVERIFY(slideImage(*plain) == before);
    }

    // Two open presentations that carry different fonts under one family name each draw with their own.
    void presentationsCarryingDifferentFontsUnderOneNameDoNotClash()
    {
        const QString name = QStringLiteral("Embtest");
        const QByteArray lobster = renamedFont(fontFileOf(QStringLiteral("Lobster"), false, false), QStringLiteral("Lobster"), name);
        const QByteArray satisfy = renamedFont(fontFileOf(QStringLiteral("Satisfy"), false, false), QStringLiteral("Satisfy"), name);
        QVERIFY(!lobster.isEmpty() && !satisfy.isEmpty());
        QString err;
        auto a = deck::importPresentation(pack(withEmbeddedFonts(sampleWithRunIn(name), {{name, {{0, office::makeEot(lobster)}}}})), &err);
        auto b = deck::importPresentation(pack(withEmbeddedFonts(sampleWithRunIn(name), {{name, {{0, office::makeEot(satisfy)}}}})), &err);
        QVERIFY2(a && b, qPrintable(err));
        QVERIFY(a->fontAliases.value(name.toLower()) != b->fontAliases.value(name.toLower()));
        auto lobsterDeck = deck::importPresentation(pack(sampleWithRunIn(QStringLiteral("Lobster"))));
        auto satisfyDeck = deck::importPresentation(pack(sampleWithRunIn(QStringLiteral("Satisfy"))));
        QVERIFY(lobsterDeck && satisfyDeck);
        const QImage imageA = slideImage(*a), imageB = slideImage(*b);
        QVERIFY(imageA == slideImage(*lobsterDeck));
        QVERIFY(imageB == slideImage(*satisfyDeck));
        QVERIFY(imageA != imageB);
        // Whichever is closed, the other keeps its own.
        deck::EmbeddedFonts::release(a.get());
        QVERIFY(slideImage(*b) == imageB);
        deck::EmbeddedFonts::release(b.get());
    }

    // The window gives a presentation's fonts back when the presentation is replaced and when the window closes.
    void aWindowReleasesItsPresentationsFonts()
    {
        const QByteArray font = renamedFont(fontFileOf(QStringLiteral("Lobster"), false, false), QStringLiteral("Lobster"), QStringLiteral("Zobster"));
        const int before = deck::EmbeddedFonts::addedFontCount();
        QString err;
        auto doc = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), {{QStringLiteral("Zobster"), {{0, office::makeEot(font)}}}})), &err);
        QVERIFY2(doc, qPrintable(err));
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before + 1);
        {
            DeckWindow w;
            w.editor()->setDocument(std::move(doc));
            QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before + 1);
            w.editor()->setDocument(deck::newPresentation());   // the presentation is replaced
            QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before);
            auto again = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), {{QStringLiteral("Zobster"), {{0, office::makeEot(font)}}}})), &err);
            QVERIFY(again);
            w.editor()->setDocument(std::move(again));
            QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before + 1);
        }
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before);   // the window closed
    }

    // Fonts a file carries are read under limits: a part that isn't a font, or is too big, costs that font and nothing else;
    // at most 64 faces are read.
    void embeddedFontsAreReadUnderLimits()
    {
        const QByteArray lobster = renamedFont(fontFileOf(QStringLiteral("Lobster"), false, false), QStringLiteral("Lobster"), QStringLiteral("Zobster"));
        const int before = deck::EmbeddedFonts::addedFontCount();
        QString err;
        // One part is damaged and its family is not one the computer has: said so; the font after it is read.
        QMap<int, QByteArray> broken{{0, QByteArray("not a font at all, and long enough to be read as a header ...............................................")}};
        auto d = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), {{QStringLiteral("Nofont Sans"), broken}, {QStringLiteral("Zobster"), {{0, office::makeEot(lobster)}}}})), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(err.contains(QStringLiteral("embedded font")), qPrintable(err));
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before + 1);
        deck::EmbeddedFonts::release(d.get());

        // A damaged part of a family the computer has is no loss to say anything about.
        auto have = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), {{QStringLiteral("Lato"), broken}})), &err);
        QVERIFY2(have, qPrintable(err));
        QVERIFY2(!err.contains(QStringLiteral("embedded font")), qPrintable(err));
        deck::EmbeddedFonts::release(have.get());

        // More than 64 faces: the first 64 are read.
        QVector<QPair<QString, QMap<int, QByteArray>>> many;
        for (int i = 0; i < 70; ++i) many.append({QStringLiteral("Family %1").arg(i), {{0, office::makeEot(lobster)}}});
        auto crowded = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), many)), &err);
        QVERIFY2(crowded, qPrintable(err));
        QVERIFY2(err.contains(QStringLiteral("first 64")), qPrintable(err));
        QCOMPARE(int(deck::EmbeddedFonts::facesOf(crowded.get()).size()), 64);
        deck::EmbeddedFonts::release(crowded.get());
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before);

        // A part past 32 MB is not read at all.
        QByteArray big = office::makeEot(lobster);
        big.append(QByteArray(jp::kMaxEmbeddedFontBytes + 100000, '\0'));
        auto large = deck::importPresentation(pack(withEmbeddedFonts(samplePptx(), {{QStringLiteral("Zobster"), {{0, big}}}})), &err);
        QVERIFY2(large, qPrintable(err));
        QVERIFY2(err.contains(QStringLiteral("too large")), qPrintable(err));
        QCOMPARE(deck::EmbeddedFonts::addedFontCount(), before);
        deck::EmbeddedFonts::release(large.get());
    }

    // A font a file carries that the program doesn't have is written back as the part was, while the text uses it; the file
    // that used it no more drops it. The setting that embeds the program's own fonts has no say over it.
    void fontsAFileCarriesAreWrittenBackWhileUsed()
    {
        const QByteArray lobster = renamedFont(fontFileOf(QStringLiteral("Lobster"), false, false), QStringLiteral("Lobster"), QStringLiteral("Zobster"));
        const QByteArray eot = office::makeEot(lobster);
        Parts p = samplePptx();
        QString slide = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        slide.replace(QStringLiteral("<a:t>Plain text</a:t>"), QStringLiteral("<a:t>Plain text</a:t>"));
        const QString plainRun = QStringLiteral("<a:r><a:rPr lang=\"en-US\"/><a:t>Plain text</a:t></a:r>");
        QVERIFY(slide.contains(plainRun));
        slide.replace(plainRun, QStringLiteral("<a:r><a:rPr lang=\"en-US\"><a:latin typeface=\"Zobster\"/></a:rPr><a:t>Plain text</a:t></a:r>"));
        p.data["/ppt/slides/slide1.xml"].xml = slide.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(withEmbeddedFonts(p, {{QStringLiteral("Zobster"), {{0, eot}}}})), &err);
        QVERIFY2(d, qPrintable(err));
        for (bool embed : {false, true}) {
            const QByteArray bytes = deck::exportPresentation(*d, &err, nullptr, deck::PresentationKind::Presentation, embed);
            Exported e;
            QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
            QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
            const auto fonts = embeddedFontsOf(e);
            QVERIFY2(fonts.contains(QStringLiteral("Zobster")), qPrintable(QString::number(embed)));
            QCOMPARE(fonts.value(QStringLiteral("Zobster")).value(QStringLiteral("regular")), eot);   // byte for byte
            QVERIFY(part(e, QStringLiteral("/ppt/presentation.xml")).contains(QStringLiteral(" embedTrueTypeFonts=\"1\"")));
            // And it opens again with the font.
            auto again = deck::importPresentation(bytes, &err);
            QVERIFY2(again, qPrintable(err));
            QCOMPARE(int(deck::EmbeddedFonts::facesOf(again.get()).size()), 1);
            deck::EmbeddedFonts::release(again.get());
        }
        // The text no longer uses it: it is not written.
        auto *box = static_cast<TextItem *>(d->pages[0]->items[3].get());
        QCOMPARE(box->name, QStringLiteral("TextBox 4"));
        QTextCursor c(d->storyDoc(box->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setFontFamilies({QStringLiteral("Lato")});
        c.mergeCharFormat(f);
        const QByteArray gone = deck::exportPresentation(*d, &err, nullptr, deck::PresentationKind::Presentation, false);
        Exported e;
        QVERIFY2(unpack(gone, &e, &err), qPrintable(err));
        QVERIFY(!embeddedFontsOf(e).contains(QStringLiteral("Zobster")));
        QVERIFY(!part(e, QStringLiteral("/ppt/presentation.xml")).contains(QStringLiteral("embedTrueTypeFonts")));
        deck::EmbeddedFonts::release(d.get());
    }

    // Options > Save has "Embed fonts in the file", on unless turned off, and Save follows it.
    void optionsSaveEmbedsFontsUnlessTurnedOff()
    {
        Settings::get().setValue(QStringLiteral("save/embedFonts"), true);
        DeckWindow w;
        QStringList tabs;
        bool checked = false, found = false;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            if (auto *t = d->findChild<QTabWidget *>())
                for (int i = 0; i < t->count(); ++i) tabs << t->tabText(i);
            if (auto *box = d->findChild<QCheckBox *>(QStringLiteral("embedFonts"))) {
                found = true;
                checked = box->isChecked();
                QCOMPARE(box->text(), QStringLiteral("Embed fonts in the file"));
            }
            d->reject();
        });
        w.act(QStringLiteral("file.options"))->trigger();
        QVERIFY(tabs.contains(QStringLiteral("Save")));
        QVERIFY(found);
        QVERIFY(checked);

        // Save puts the fonts in (a font that ships with the program and has no standard equivalent).
        {
            auto *title = static_cast<TextItem *>(w.editor()->doc()->pages[0]->items[0].get());
            QTextCharFormat f;
            f.setFontFamilies({QStringLiteral("Fraunces")});
            QTextCursor(w.editor()->doc()->storyDoc(title->storyId)).insertText(QStringLiteral("Set in Fraunces"), f);
        }
        QTemporaryDir dir;
        const QString on = dir.filePath(QStringLiteral("on.pptx")), off = dir.filePath(QStringLiteral("off.pptx"));
        QVERIFY(w.saveTo(on));
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            if (auto *box = d->findChild<QCheckBox *>(QStringLiteral("embedFonts"))) box->setChecked(false);
            d->accept();
        });
        w.act(QStringLiteral("file.options"))->trigger();
        QVERIFY(!Settings::get().value(QStringLiteral("save/embedFonts"), true).toBool());
        QVERIFY(w.saveTo(off));
        Settings::get().setValue(QStringLiteral("save/embedFonts"), true);
        QString err;
        Exported withFonts, without;
        QFile f1(on), f2(off);
        QVERIFY(f1.open(QIODevice::ReadOnly) && f2.open(QIODevice::ReadOnly));
        QVERIFY2(unpack(f1.readAll(), &withFonts, &err), qPrintable(err));
        QVERIFY2(unpack(f2.readAll(), &without, &err), qPrintable(err));
        QVERIFY(part(withFonts, QStringLiteral("/ppt/presentation.xml")).contains(QStringLiteral("<p:embeddedFontLst>")));
        QVERIFY(!part(without, QStringLiteral("/ppt/presentation.xml")).contains(QStringLiteral("<p:embeddedFontLst>")));
    }

    // The open fonts that are drawn to a standard font's character widths (Carlito for Calibri, Arimo for Arial...) are written under the
    // standard name wherever a font is named, and never embedded: a computer that has the standard font uses it, and one that has not
    // gets the look-alike drawn for it, laid out the same. A new presentation names Calibri, as it is drawn; one whose font is Carlito
    // (a theme set to it, or text in it) is written as Calibri too. Neither embeds anything.
    void aDefaultPresentationNamesCalibriAndEmbedsNothing()
    {
        for (bool carlito : {false, true}) {
            auto d = deck::newPresentation();
            QCOMPARE(deck::themeOf(*d).headingFont, QStringLiteral("Calibri"));
            if (carlito) {
                deck::DeckTheme theme = deck::themeOf(*d);
                theme.headingFont = theme.bodyFont = QStringLiteral("Carlito");
                deck::setTheme(*d, theme);
            }
            auto *title = static_cast<TextItem *>(d->pages[0]->items[0].get());
            auto *subtitle = static_cast<TextItem *>(d->pages[0]->items[1].get());
            QTextCharFormat family;
            if (carlito) family.setFontFamilies({QStringLiteral("Carlito")});
            QTextCursor(d->storyDoc(title->storyId)).insertText(QStringLiteral("Quarterly results"), family);
            QTextCursor c(d->storyDoc(subtitle->storyId));
            QTextCharFormat bold = family;
            bold.setFontWeight(QFont::Bold);
            c.insertText(QStringLiteral("Bold "), bold);
            c.insertText(QStringLiteral("and plain"), family);
            d->pages[0]->notesStoryId = d->createStory(QStringLiteral("Speaker notes"));
            QString err;
            const QByteArray bytes = deck::exportPresentation(*d, &err, nullptr, deck::PresentationKind::Presentation, true);
            QVERIFY2(!bytes.isEmpty(), qPrintable(err));
            Exported e;
            QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
            QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
            // No part names the open font, and the theme, a run, and the master's text style name the standard one.
            for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it)
                if (it.key().endsWith(QLatin1String(".xml")) || it.key().endsWith(QLatin1String(".rels")))
                    QVERIFY2(!QString::fromUtf8(it.value()).contains(QStringLiteral("Carlito")), qPrintable(it.key()));
            const QString theme = part(e, QStringLiteral("/ppt/theme/theme1.xml"));
            QVERIFY2(theme.contains(QStringLiteral("<a:majorFont><a:latin typeface=\"Calibri\"/>")) && theme.contains(QStringLiteral("<a:minorFont><a:latin typeface=\"Calibri\"/>")), qPrintable(theme.left(1500)));
            if (carlito) QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<a:latin typeface=\"Calibri\"/>")));
            // The layouts' prompts name it in their runs.
            QVERIFY(part(e, QStringLiteral("/ppt/slideLayouts/slideLayout1.xml")).contains(QStringLiteral("<a:latin typeface=\"Calibri\"/>")));
            QVERIFY(part(e, QStringLiteral("/docProps/app.xml")).contains(QStringLiteral("<vt:lpstr>Calibri</vt:lpstr>")));
            // Nothing is embedded: no list, no parts, and no claim to embed.
            const QString pres = part(e, QStringLiteral("/ppt/presentation.xml"));
            QVERIFY2(!pres.contains(QStringLiteral("embeddedFont")) && !pres.contains(QStringLiteral("embedTrueTypeFonts")), qPrintable(pres.left(600)));
            for (const QString &name : e.pkg.partNames()) QVERIFY2(!name.startsWith(QLatin1String("/ppt/fonts/")), qPrintable(name));
            QVERIFY(embeddedFontsOf(e).isEmpty());
            // Saved again after it is opened, it is the same.
            auto back = deck::importPresentation(bytes, &err);
            QVERIFY2(back, qPrintable(err));
            QCOMPARE(deck::themeOf(*back).headingFont, QStringLiteral("Calibri"));
            const QByteArray again = deck::exportPresentation(*back, &err, nullptr, deck::PresentationKind::Presentation, true);
            Exported f;
            QVERIFY2(unpack(again, &f, &err), qPrintable(err));
            QVERIFY(embeddedFontsOf(f).isEmpty());
            QVERIFY(part(f, QStringLiteral("/ppt/theme/theme1.xml")).contains(QStringLiteral("<a:minorFont><a:latin typeface=\"Calibri\"/>")));
            QVERIFY(!part(f, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("Carlito")));
        }
    }

    // Each of the nine (and no other) is written as the standard font, in runs, a master's text, a table's cells, and the list of fonts used;
    // a bundled font that has no standard equivalent is still named as it is, and embedded.
    void bundledFontsWithAStandardEquivalentAreWrittenAsTheStandardFont()
    {
        const QVector<QPair<QString, QString>> pairs{{"Arimo", "Arial"}, {"Tinos", "Times New Roman"}, {"Cousine", "Courier New"}, {"Carlito", "Calibri"}, {"Caladea", "Cambria"},
                                                    {"Gelasio", "Georgia"}, {"Comic Relief", "Comic Sans MS"}, {"Selawik", "Segoe UI"}, {"Liberation Sans Narrow", "Arial Narrow"}};
        for (const auto &pair : pairs) QCOMPARE(interchangeFontName(pair.first), pair.second);   // the table this follows (core/fonts.h)
        auto d = deck::newPresentation();
        auto *title = static_cast<TextItem *>(d->pages[0]->items[0].get());
        QTextCursor c(d->storyDoc(title->storyId));
        for (const auto &pair : pairs) {
            QTextCharFormat f;
            f.setFontFamilies({pair.first});
            c.insertText(pair.first + QLatin1Char(' '), f);
        }
        QTextCharFormat fraunces;
        fraunces.setFontFamilies({QStringLiteral("Fraunces")});
        c.insertText(QStringLiteral("Fraunces"), fraunces);
        // A master's prompt, and the notes.
        TextItem *prompt = nullptr;
        for (const auto &it : d->masters[0]->items)
            if (it->type() == ItemType::Text) {
                prompt = static_cast<TextItem *>(it.get());
                break;
            }
        QVERIFY(prompt);
        {
            QTextCursor all(d->storyDoc(prompt->storyId));
            all.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontFamilies({QStringLiteral("Arimo")});
            all.mergeCharFormat(f);
        }
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err, nullptr, deck::PresentationKind::Presentation, true);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        for (const auto &pair : pairs) {
            QVERIFY2(slide.contains(QStringLiteral("<a:latin typeface=\"%1\"/>").arg(pair.second)), qPrintable(pair.second));
            for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it)
                if (it.key().endsWith(QLatin1String(".xml")) || it.key().endsWith(QLatin1String(".rels")))
                    QVERIFY2(!QString::fromUtf8(it.value()).contains(QLatin1Char('"') + pair.first + QLatin1Char('"')), qPrintable(it.key() + QLatin1Char(' ') + pair.first));
        }
        QVERIFY(part(e, QStringLiteral("/ppt/slideLayouts/slideLayout1.xml")).contains(QStringLiteral("typeface=\"Arial\"")) ||
                part(e, QStringLiteral("/ppt/slideMasters/slideMaster1.xml")).contains(QStringLiteral("typeface=\"Arial\"")));
        const QString app = part(e, QStringLiteral("/docProps/app.xml"));
        QVERIFY(app.contains(QStringLiteral("<vt:lpstr>Calibri</vt:lpstr>")) && !app.contains(QStringLiteral("<vt:lpstr>Carlito</vt:lpstr>")));
        // Only the one without a standard equivalent is embedded.
        const auto fonts = embeddedFontsOf(e);
        QCOMPARE(fonts.keys(), (QStringList{"Fraunces"}));
        QVERIFY(slide.contains(QStringLiteral("<a:latin typeface=\"Fraunces\"/>")));
    }

    // A table's cells and a chart's text are written as the standard font too.
    void standardFontNamesReachTablesAndKeptParts()
    {
        const QString id = QStringLiteral("{11111111-2222-3333-4444-555555555555}");
        Parts p = samplePptx();
        const QString cell = QStringLiteral("<a:tc><a:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\"><a:latin typeface=\"Tinos\"/></a:rPr><a:t>Quarter</a:t></a:r></a:p></a:txBody><a:tcPr/></a:tc>");
        const QString table = QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"8\" name=\"Results\"/><p:cNvGraphicFramePr><a:graphicFrameLocks noGrp=\"1\"/></p:cNvGraphicFramePr><p:nvPr/></p:nvGraphicFramePr>"
                                             "<p:xfrm><a:off x=\"914400\" y=\"1828800\"/><a:ext cx=\"1828800\" cy=\"370840\"/></p:xfrm>"
                                             "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"><a:tbl><a:tblPr/>"
                                             "<a:tblGrid><a:gridCol w=\"1828800\"/></a:tblGrid><a:tr h=\"370840\">%1</a:tr></a:tbl></a:graphicData></a:graphic></p:graphicFrame>")
                                     .arg(cell);
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QVERIFY(xml.contains(unknownFrame(8)));
        xml.replace(unknownFrame(8), table);
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        // A theme that names one of the open fonts (as the files older versions wrote do).
        QString theme = QString::fromUtf8(p.data["/ppt/theme/theme1.xml"].xml);
        theme.replace(QRegularExpression(QStringLiteral("<a:latin typeface=\"[^\"]*\"/>")), QStringLiteral("<a:latin typeface=\"Carlito\"/>"));
        p.data["/ppt/theme/theme1.xml"].xml = theme.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray bytes = deck::exportPresentation(*d, &err, nullptr, deck::PresentationKind::Presentation, true);
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(slide.contains(QStringLiteral("<a:latin typeface=\"Times New Roman\"/>")) && !slide.contains(QStringLiteral("\"Tinos\"")), qPrintable(slide.left(2000)));
        QVERIFY(!part(e, QStringLiteral("/ppt/theme/theme1.xml")).contains(QStringLiteral("Carlito")));
        QVERIFY(embeddedFontsOf(e).isEmpty());
    }

    // A presentation made with the default theme, saved and opened again, draws the same.
    void aDefaultPresentationSavedAndOpenedAgainDrawsTheSame()
    {
        auto d = deck::newPresentation();
        auto *title = static_cast<TextItem *>(d->pages[0]->items[0].get());
        auto *subtitle = static_cast<TextItem *>(d->pages[0]->items[1].get());
        QTextCursor(d->storyDoc(title->storyId)).insertText(QStringLiteral("Quarterly results: Hamburgefonstiv"));
        QTextCursor c(d->storyDoc(subtitle->storyId));
        QTextCharFormat bold, italic;
        bold.setFontWeight(QFont::Bold);
        italic.setFontItalic(true);
        c.insertText(QStringLiteral("Bold, "), bold);
        c.insertText(QStringLiteral("italic, "), italic);
        c.insertText(QStringLiteral("and plain text that is long enough to be set on more than one line, so that its widths matter."));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err, nullptr, deck::PresentationKind::Presentation, true);
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        LayoutCache cacheA, cacheB;
        PaintContext a, b;
        a.doc = d.get();
        a.cache = &cacheA;
        b.doc = back.get();
        b.cache = &cacheB;
        // The text is set the same: the same lines in the same places (the baselines, which the standard font's own metrics decide, too).
        for (int i = 0; i < 2; ++i) {
            const auto &was = *static_cast<TextItem *>(d->pages[0]->items[i].get());
            const auto &now = *static_cast<TextItem *>(back->pages[0]->items[i].get());
            const auto linesWas = cacheA.textFrame(*d, was, 1, RenderOptions()).layout->lineInfo(0), linesNow = cacheB.textFrame(*back, now, 1, RenderOptions()).layout->lineInfo(0);
            QCOMPARE(int(linesNow.size()), int(linesWas.size()));
            for (size_t k = 0; k < linesWas.size(); ++k) {
                QVERIFY2(std::abs(linesNow[k].baseline - linesWas[k].baseline) < 0.01, qPrintable(QString::number(linesNow[k].baseline - linesWas[k].baseline)));
                QVERIFY(std::abs(linesNow[k].rect.left() - linesWas[k].rect.left()) < 0.01 && std::abs(linesNow[k].rect.right() - linesWas[k].rect.right()) < 0.01);
            }
        }
        // And the same picture: the shrink to fit of the long line is kept in thousandths of a percent, so a letter may stand a hair
        // differently, never more than a few levels of gray.
        const QImage was = Renderer::renderToImage(a, 0, 1.0), now = Renderer::renderToImage(b, 0, 1.0);
        QCOMPARE(now.size(), was.size());
        int worst = 0;
        for (int y = 0; y < was.height(); ++y)
            for (int x = 0; x < was.width(); ++x) {
                const QColor p = was.pixelColor(x, y), q = now.pixelColor(x, y);
                worst = std::max({worst, std::abs(p.red() - q.red()), std::abs(p.green() - q.green()), std::abs(p.blue() - q.blue())});
            }
        QVERIFY2(worst <= 8, qPrintable(QStringLiteral("the picture differs by %1 levels once it has been saved and opened").arg(worst)));
        // And it does draw something.
        bool inked = false;
        for (int y = 0; y < was.height() && !inked; y += 3)
            for (int x = 0; x < was.width() && !inked; x += 3) inked = was.pixelColor(x, y) != QColor(Qt::white);
        QVERIFY(inked);
    }

private:

    // The fonts an exported file embeds: the family, then each style ("regular", "bold", "italic", "boldItalic") and the part it points at.
    static QMap<QString, QMap<QString, QByteArray>> embeddedFontsOf(const Exported &e)
    {
        QMap<QString, QMap<QString, QByteArray>> out;
        QDomDocument dom;
        if (!office::parseXml(e.zip.value(QStringLiteral("ppt/presentation.xml")), &dom)) return out;
        const QDomElement list = office::child(dom.documentElement(), office::ns::P, QStringLiteral("embeddedFontLst"));
        for (const QDomElement &entry : office::children(list, office::ns::P, QStringLiteral("embeddedFont"))) {
            const QString typeface = office::attr(office::child(entry, office::ns::P, QStringLiteral("font")), QStringLiteral("typeface"));
            for (const char *style : {"regular", "bold", "italic", "boldItalic"}) {
                const QDomElement face = office::child(entry, office::ns::P, QLatin1String(style));
                if (face.isNull()) continue;
                const office::Relationship link = e.pkg.relationship(QStringLiteral("/ppt/presentation.xml"), office::attrNs(face, office::ns::R, QStringLiteral("id")));
                out[typeface][QLatin1String(style)] = e.pkg.part(link.target);
            }
        }
        return out;
    }
    // A bundled font's file.
    static QByteArray fontFileOf(const QString &family, bool bold, bool italic)
    {
        QFile f(bundledFontFile(family, bold, italic).path);
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
    }
    // The same font under another family name of the same length (in its name table, with the checksums made right), for a font
    // the computer doesn't have.
    static QByteArray renamedFont(QByteArray font, const QString &from, const QString &to)
    {
        if (from.size() != to.size() || font.size() < 12) return QByteArray();
        auto u16 = [&](int at) { return quint16(uchar(font[at]) << 8 | uchar(font[at + 1])); };
        auto u32 = [&](int at) { return quint32(u16(at)) << 16 | u16(at + 2); };
        auto set32 = [&](int at, quint32 v) {
            font[at] = char(v >> 24);
            font[at + 1] = char(v >> 16);
            font[at + 2] = char(v >> 8);
            font[at + 3] = char(v);
        };
        auto sum = [&](int at, int length) {
            quint32 total = 0;
            for (int i = 0; i < length; i += 4) {
                quint32 word = 0;
                for (int k = 0; k < 4; ++k) word = word << 8 | (i + k < length ? uchar(font[at + i + k]) : 0);
                total += word;
            }
            return total;
        };
        int nameRecord = -1, headRecord = -1;
        for (int i = 0, n = u16(4); i < n; ++i) {
            const QByteArray tag = font.mid(12 + 16 * i, 4);
            if (tag == "name") nameRecord = 12 + 16 * i;
            if (tag == "head") headRecord = 12 + 16 * i;
        }
        if (nameRecord < 0 || headRecord < 0) return QByteArray();
        const int nameAt = int(u32(nameRecord + 8)), nameLength = int(u32(nameRecord + 12)), headAt = int(u32(headRecord + 8));
        QByteArray names = font.mid(nameAt, nameLength);
        QByteArray wide;
        for (QChar c : from) wide.append(char(0)).append(char(c.unicode()));
        QByteArray wideTo;
        for (QChar c : to) wideTo.append(char(0)).append(char(c.unicode()));
        names.replace(wide, wideTo);
        names.replace(from.toLatin1(), to.toLatin1());
        font.replace(nameAt, nameLength, names);
        set32(nameRecord + 4, sum(nameAt, nameLength));
        set32(headAt + 8, 0);
        set32(headRecord + 4, sum(headAt, int(u32(headRecord + 12))));
        set32(headAt + 8, 0xB1B0AFBAu - sum(0, int(font.size())));
        return font;
    }
    // The sample presentation with its text box set in one family.
    static Parts sampleWithRunIn(const QString &family)
    {
        Parts p = samplePptx();
        QString slide = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        const QString plainRun = QStringLiteral("<a:r><a:rPr lang=\"en-US\"/><a:t>Plain text</a:t></a:r>");
        slide.replace(plainRun, QStringLiteral("<a:r><a:rPr lang=\"en-US\" sz=\"4800\"><a:latin typeface=\"%1\"/></a:rPr><a:t>Hamburgefonstiv Plain text</a:t></a:r>").arg(family));
        p.data["/ppt/slides/slide1.xml"].xml = slide.toUtf8();
        return p;
    }
    // A presentation file's parts with fonts embedded: for each family, the EOT (or other bytes) for each style (0 regular, 1 bold, 2 italic, 3 bold italic).
    static Parts withEmbeddedFonts(Parts p, const QVector<QPair<QString, QMap<int, QByteArray>>> &fonts)
    {
        QString list = QStringLiteral("<p:embeddedFontLst>");
        int number = 0;
        // The package writer numbers a part's links in the order they are added.
        int links = 0;
        for (const Rel &r : std::as_const(p.rels))
            if (r.source == QLatin1String("/ppt/presentation.xml")) ++links;
        for (const auto &family : fonts) {
            list += QStringLiteral("<p:embeddedFont><p:font typeface=\"%1\" pitchFamily=\"2\" charset=\"0\"/>").arg(family.first);
            for (auto face = family.second.cbegin(); face != family.second.cend(); ++face) {
                const QString name = QStringLiteral("/ppt/fonts/font%1.fntdata").arg(++number);
                const QString id = QStringLiteral("rId%1").arg(++links);
                p.data[name] = {QStringLiteral("application/x-fontdata"), face.value()};
                p.rels << Rel{QStringLiteral("/ppt/presentation.xml"), QStringLiteral("font"), name};
                list += QStringLiteral("<p:%1 r:id=\"%2\"/>").arg(QLatin1String(face.key() == 0 ? "regular" : face.key() == 1 ? "bold" : face.key() == 2 ? "italic" : "boldItalic"), id);
            }
            list += QStringLiteral("</p:embeddedFont>");
        }
        list += QStringLiteral("</p:embeddedFontLst>");
        QString xml = QString::fromUtf8(p.data["/ppt/presentation.xml"].xml);
        xml.replace(QStringLiteral("<p:presentation "), QStringLiteral("<p:presentation embedTrueTypeFonts=\"1\" "));
        xml.replace(QRegularExpression(QStringLiteral("(<p:notesSz [^>]*/>)")), QStringLiteral("\\1") + list);
        p.data["/ppt/presentation.xml"].xml = xml.toUtf8();
        return p;
    }
};

JP_TEST_CLASS(DeckFontsTests)
#include "tst_deck_fonts.moc"

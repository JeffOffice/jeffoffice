// JeffDeck's tests of writing a presentation file.
#include "deckhelpers.h"
#include "core/gridview.h"
#include "app/guidesdialog.h"
#include "core/notesmaster.h"
#include "core/printheaderfooter.h"
#include "io/zip.h"
#include "app/listmenus.h"
#include "core/picturestyles.h"

class DeckPptxExportTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A table's header row is written as firstRow on its properties, and comes back on or off as it was.
    void pptxKeepsAHeaderRowOfATable()
    {
        for (bool header : {true, false}) {
            auto d = deck::newPresentation();
            auto table = std::make_shared<jp::TableItem>();
            table->rect = QRectF(50, 100, 200, 60);
            table->rows = table->cols = 2;
            table->colW = {100, 100};
            table->rowH = {30, 30};
            table->cells.resize(4);
            for (jp::TableCell &cell : table->cells) cell.storyId = d->createStory(QStringLiteral("x"));
            table->header = header;
            d->pages[0]->items.push_back(table);
            QString err;
            const QByteArray bytes = deck::exportPresentation(*d, &err);
            QVERIFY2(!bytes.isEmpty(), qPrintable(err));
            auto back = deck::importPresentation(bytes, &err);
            QVERIFY2(back, qPrintable(err));
            const jp::TableItem *found = nullptr;
            for (const auto &it : back->pages[0]->items)
                if (it->type() == ItemType::Table) found = static_cast<const jp::TableItem *>(it.get());
            QVERIFY(found);
            QCOMPARE(found->header, header);
        }
    }

    // A macro-enabled presentation (.pptm, .ppsm, .potm) is opened with its macro project untouched and never run, and saved as the same kind it is written back to the
    // byte, with the types and links other programs need; saved as a plain kind the project is left out whole.
    void aMacroPresentationKeepsItsProjectInItsOwnKind()
    {
        // (the lengths from the literals themselves: they hold zero bytes, so strlen would stop short; a count typed by hand read past one's end)
        static const char kProject[] = "\xD0\xCF\x11\xE0 our own bytes, not a project \x00\x01\x02\xFF", kSignature[] = "\xD0\xCF\x11\xE0 signature bytes \x00\x10\x20";
        const QByteArray project(kProject, sizeof kProject - 1), signature(kSignature, sizeof kSignature - 1);
        struct Case { const char *ending, *mainType; } cases[] = {
            {"pptm", "application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml"},
            {"ppsm", "application/vnd.ms-powerpoint.slideshow.macroEnabled.main+xml"},
            {"potm", "application/vnd.ms-powerpoint.template.macroEnabled.main+xml"}};
        for (const Case &c : cases) {
            const QString ending = QString::fromLatin1(c.ending);
            QTemporaryDir dir;
            QVERIFY(dir.isValid());
            const QString source = dir.filePath(QStringLiteral("macros.") + ending);
            {
                QFile f(source);
                QVERIFY(f.open(QIODevice::WriteOnly));
                const QByteArray made = macroPptx(QString::fromLatin1(c.mainType), project, signature);
                QVERIFY(!made.isEmpty());
                f.write(made);
            }
            QString err;
            auto doc = deck::importAnyPresentationFile(source, &err);
            QVERIFY2(doc, qPrintable(err));
            QVERIFY(!deck::presentationMacros(*doc).isEmpty());
            QVERIFY(deck::isWrittenEnding(source));

            // the same kind: the project and its signature come back to the byte
            const QString same = dir.filePath(QStringLiteral("again.") + ending);
            QVERIFY2(deck::saveAnyPresentationFile(*doc, same, &err), qPrintable(err));
            QFile sameFile(same);
            QVERIFY(sameFile.open(QIODevice::ReadOnly));
            Exported kept;
            QString why;
            QVERIFY2(unpack(sameFile.readAll(), &kept, &why), qPrintable(why));
            QVERIFY2(packageProblem(kept).isEmpty(), qPrintable(packageProblem(kept)));
            QCOMPARE(kept.zip.value(QStringLiteral("ppt/vbaProject.bin")), project);
            QCOMPARE(kept.zip.value(QStringLiteral("ppt/vbaProjectSignature.bin")), signature);
            QCOMPARE(kept.pkg.contentType(QStringLiteral("/ppt/presentation.xml")), QString::fromLatin1(c.mainType));
            QCOMPARE(kept.pkg.contentType(QStringLiteral("/ppt/vbaProject.bin")), QStringLiteral("application/vnd.ms-office.vbaProject"));
            QCOMPARE(kept.pkg.contentType(QStringLiteral("/ppt/vbaProjectSignature.bin")), QStringLiteral("application/vnd.ms-office.vbaProjectSignature"));
            QCOMPARE(relationshipTargets(kept, QStringLiteral("/ppt/presentation.xml"), QStringLiteral("/vbaProject")), QStringList{QStringLiteral("/ppt/vbaProject.bin")});
            QCOMPARE(relationshipTargets(kept, QStringLiteral("/ppt/vbaProject.bin"), QStringLiteral("/vbaProjectSignature")), QStringList{QStringLiteral("/ppt/vbaProjectSignature.bin")});
            // (and it opens again, as the same kind, with the project still there)
            auto again = deck::importAnyPresentationFile(same, &err);
            QVERIFY2(again, qPrintable(err));
            QVERIFY(deck::presentationMacros(*again).parts.size() == 2);

            // a plain kind: no project, no signature, no link, no macro type
            const QString plainEnding = ending == QLatin1String("ppsm") ? QStringLiteral("ppsx") : ending == QLatin1String("potm") ? QStringLiteral("potx") : QStringLiteral("pptx");
            const QString plain = dir.filePath(QStringLiteral("plain.") + plainEnding);
            QVERIFY2(deck::saveAnyPresentationFile(*doc, plain, &err), qPrintable(err));
            QFile plainFile(plain);
            QVERIFY(plainFile.open(QIODevice::ReadOnly));
            Exported gone;
            QVERIFY2(unpack(plainFile.readAll(), &gone, &why), qPrintable(why));
            QVERIFY2(packageProblem(gone).isEmpty(), qPrintable(packageProblem(gone)));
            for (const QString &name : gone.zip.keys()) QVERIFY2(!name.contains(QLatin1String("vba")), qPrintable(name));
            const QString types = QString::fromUtf8(gone.zip.value(QStringLiteral("[Content_Types].xml")));
            QVERIFY2(!types.contains(QLatin1String("macroEnabled")) && !types.contains(QLatin1String("vba")), qPrintable(types));
            QVERIFY(!QString::fromUtf8(gone.zip.value(QStringLiteral("ppt/_rels/presentation.xml.rels"))).contains(QLatin1String("vba")));
            QVERIFY(!gone.pkg.contentType(QStringLiteral("/ppt/presentation.xml")).contains(QLatin1String("macroEnabled")));
        }
    }

    // A macro project over a bound (a part of 32 MiB, or more than 8 parts, or 64 MiB together) is not kept, the file is then the plain kind, and the person is told.
    void aMacroProjectOverTheBoundIsLeftOut()
    {
        QByteArray big(int(office::kMaxMacroPartBytes) + 1, 'x');
        big.replace(0, 4, QByteArray::fromHex("D0CF11E0"));
        QString err;
        const QByteArray made = macroPptx(QStringLiteral("application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml"), big, QByteArray());
        QVERIFY(!made.isEmpty());
        auto doc = deck::importPresentation(made, &err);
        QVERIFY2(doc, qPrintable(err));
        QVERIFY(deck::presentationMacros(*doc).isEmpty());
        QVERIFY2(err.contains(QLatin1String("too large")), qPrintable(err));
        // the bytes in a document's own data are held to the same bounds
        office::MacroProject p;
        p.parts.push_back(office::MacroPart{QStringLiteral("/ppt/vbaProject.bin"), QStringLiteral("application/vnd.ms-office.vbaProject"), big, -1, false});
        QVERIFY(office::macroProjectFromJson(office::macroProjectToJson(p)).isEmpty());
        office::MacroProject many;
        for (int i = 0; i <= office::kMaxMacroParts; ++i) many.parts.push_back(office::MacroPart{QStringLiteral("/ppt/p%1.bin").arg(i), QStringLiteral("application/vnd.ms-office.vbaProject"), QByteArray("x"), -1, false});
        QVERIFY(office::macroProjectFromJson(office::macroProjectToJson(many)).isEmpty());
    }

    // The macro link of a file names a project only if the part is one: of the project type and a compound file by its first bytes; a link to a slide, a picture, a part of
    // another type, a place outside the file, or an address is not taken.
    void aMacroLinkToAnythingButAProjectIsNotTaken()
    {
        const QByteArray project("\xD0\xCF\x11\xE0 our own bytes");   // (no length: 20 read past its 18 bytes, memory checker Nov 17)
        const QString macroType = QStringLiteral("application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml");
        struct Case { const char *target; bool external; } cases[] = {{"slides/slide1.xml", false}, {"../docProps/app.xml", false}, {"../../vbaProject.bin", false}, {"http://example.com/vbaProject.bin", true}};
        for (const Case &c : cases) {
            const QByteArray bytes = macroPptx(macroType, project, QByteArray(), c.target, c.external);
            QVERIFY(!bytes.isEmpty());
            QString err;
            auto doc = deck::importPresentation(bytes, &err);
            QVERIFY2(doc, qPrintable(err));
            QVERIFY2(deck::presentationMacros(*doc).isEmpty(), c.target);
        }
        // a project that is not a compound file is not one
        auto notOne = deck::importPresentation(macroPptx(macroType, QByteArray("<p:sld/> not a project"), QByteArray()));
        QVERIFY(notOne);
        QVERIFY(deck::presentationMacros(*notOne).isEmpty());
        // the right link is taken
        auto good = deck::importPresentation(macroPptx(macroType, project, QByteArray()));
        QVERIFY(good && deck::presentationMacros(*good).parts.size() == 1);
    }

    // A plain file with a link to a macro project does not carry the project into a macro-enabled kind when it is saved as one.
    void aPlainFileCarriesNoMacroProject()
    {
        const QByteArray project("\xD0\xCF\x11\xE0 our own bytes");   // (no length: 20 read past its 18 bytes, memory checker Nov 17)
        const QByteArray bytes = macroPptx(QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"), project, QByteArray());
        QVERIFY(!bytes.isEmpty());
        QString err;
        auto doc = deck::importPresentation(bytes, &err);
        QVERIFY2(doc, qPrintable(err));
        QVERIFY(deck::presentationMacros(*doc).isEmpty());
        Exported out;
        QString why;
        QVERIFY2(unpack(deck::exportPresentation(*doc, &err, nullptr, deck::PresentationKind::Presentation, false, true), &out, &why), qPrintable(why));
        for (const QString &name : out.zip.keys()) QVERIFY2(!name.contains(QLatin1String("vba")), qPrintable(name));
    }

    // The bounds hold on the package as well as on a document's data: more than 8 parts, or more than 64 MiB together, is left out whole; the size a part claims is read
    // from the package's directory, and a part that claims more than the bound is never inflated.
    void theMacroBoundsHoldOnAPackage()
    {
        auto packageOf = [&](int projects, qsizetype each) {
            office::PackageWriter w;
            w.addPart(QStringLiteral("/ppt/presentation.xml"), QByteArray("<x/>"), QStringLiteral("application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml"));
            w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/ppt/presentation.xml"));
            QByteArray bytes(each, '\0');
            bytes.replace(0, 4, QByteArray::fromHex("D0CF11E0"));
            for (int i = 0; i < projects; ++i) {
                const QString name = QStringLiteral("/ppt/vba%1.bin").arg(i);
                w.addPart(name, bytes, QStringLiteral("application/vnd.ms-office.vbaProject"));
                w.addRelationship(QStringLiteral("/ppt/presentation.xml"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProject"), name);
            }
            return w.finish();
        };
        office::Package p8, p9, big;
        QVERIFY(p8.read(packageOf(office::kMaxMacroParts, 100)));
        QVERIFY(p9.read(packageOf(office::kMaxMacroParts + 1, 100)));
        QVERIFY(big.read(packageOf(3, 24 << 20)));
        const office::MacroProject eight = office::readMacroProject(p8, QStringLiteral("/ppt/presentation.xml"));
        QCOMPARE(int(eight.parts.size()), office::kMaxMacroParts);
        QVERIFY(!eight.leftOut);
        QVERIFY(office::readMacroProject(p9, QStringLiteral("/ppt/presentation.xml")).leftOut);
        const office::MacroProject together = office::readMacroProject(big, QStringLiteral("/ppt/presentation.xml"));
        QVERIFY(together.leftOut && together.isEmpty());
        // a part whose entry claims 1.9 GiB (and holds four bytes): refused by its size, before anything is inflated
        QMap<QString, QByteArray> zipParts;
        const QByteArray good = packageOf(1, 100);
        QString why;
        QVERIFY2(readZip(good, zipParts, &why), qPrintable(why));
        ZipWriter zip;
        for (auto it = zipParts.cbegin(); it != zipParts.cend(); ++it) {
            if (it.key() == QLatin1String("ppt/vba0.bin")) zip.addRaw(it.key(), 8, 0, 4, quint64(1900) << 20, "abcd");
            else zip.add(it.key(), it.value(), true);
        }
        office::Package lazy;
        QVERIFY(lazy.readLazy(zip.finish()));
        QVERIFY(lazy.partSize(QStringLiteral("/ppt/vba0.bin")) > office::kMaxMacroPartBytes);
        QVERIFY(office::readMacroProject(lazy, QStringLiteral("/ppt/presentation.xml")).leftOut);
    }

    // The grid and the guides are in the file's view properties: the spacing, the snap and show attributes, and the guide list (positions in eighths of a point), and
    // they read back as they were. A presentation with none writes the part it always has.
    void theGridAndGuidesRoundTripInThePptx()
    {
        auto d = deck::newPresentation();
        QString err, why;
        Exported plain;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &plain, &why), qPrintable(why));
        QVERIFY(part(plain, QStringLiteral("/ppt/viewProps.xml")).contains(QStringLiteral("<p:guideLst/>")));
        QVERIFY(!part(plain, QStringLiteral("/ppt/viewProps.xml")).contains(QStringLiteral("gridSpacing")));
        deck::GridView g;
        g.spacing = 18;
        g.snapGrid = true;
        g.snapObjects = false;
        g.showGuides = true;
        deck::setGridView(*d, g);
        d->viewGuides.h = {270};
        d->viewGuides.v = {100.5, 480};
        Exported out;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &out, &why), qPrintable(why));
        const QString xml = part(out, QStringLiteral("/ppt/viewProps.xml"));
        QVERIFY(xml.contains(QStringLiteral("<p:guide orient=\"horz\" pos=\"2160\"/><p:guide pos=\"804\"/><p:guide pos=\"3840\"/>")));
        QVERIFY(xml.contains(QStringLiteral("<p:gridSpacing cx=\"228600\" cy=\"228600\"/>")));
        QVERIFY(xml.contains(QStringLiteral("showGuides=\"1\"")) && xml.contains(QStringLiteral("snapToGrid=\"1\"")) && xml.contains(QStringLiteral("snapToObjects=\"0\"")));
        auto back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(deck::gridViewOf(*back), g);
        QCOMPARE(back->viewGuides.h, QVector<double>({270}));
        QCOMPARE(back->viewGuides.v, QVector<double>({100.5, 480}));
        // (the package is still sound, and the part is in it once)
        QVERIFY2(packageProblem(out).isEmpty(), qPrintable(packageProblem(out)));
    }

    // A spacing of 6 points set on purpose is kept through the file (it is the program's own too, but a presentation that sets it is told apart from one that does not).
    void anExplicitSixPointGridSurvivesThePptx()
    {
        QString err;
        auto d = deck::newPresentation();
        deck::GridView g;
        g.spacing = 6;
        deck::setGridView(*d, g);
        auto back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(deck::gridViewOf(*back).spacing.has_value());
        QCOMPARE(deck::gridViewOf(*back).spacing.value_or(0), 6.0);
        // (and one with no spacing set has none after)
        auto none = deck::importPresentation(deck::exportPresentation(*deck::newPresentation(), &err), &err);
        QVERIFY2(none, qPrintable(err));
        QVERIFY(!deck::gridViewOf(*none).spacing.has_value());
    }

    // A guide may sit off the slide (on the pasteboard, within a slide's size of its edge): it is saved where it is and read back there.
    void guidesOffTheSlideSurviveThePptx()
    {
        QString err;
        auto d = deck::newPresentation();
        const QSizeF size = d->pageSize();
        d->viewGuides.v = {-100, size.width() * 1.5};
        d->viewGuides.h = {size.height() + 40.5, -size.height() / 2};
        auto back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->viewGuides.v, d->viewGuides.v);
        QCOMPARE(back->viewGuides.h, d->viewGuides.h);
    }

    // What a file says about its guides is read within bounds: at most kMaxGuidesPerAxis of a kind, each within a slide's size of the slide, a position that is not a
    // number left out, and only the first 4096 elements of the list looked at.
    void theGuidesOfAFileAreBounded()
    {
        auto d = deck::newPresentation();
        QString err, why;
        Exported base;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &base, &why), qPrintable(why));
        QString list = QStringLiteral("<p:guide pos=\"1e12\"/><p:guide pos=\"-5000\"/><p:guide pos=\"abc\"/>");
        list += QStringLiteral("<p:guide pos=\"-50000\"/>");
        for (int i = 0; i < 1200; ++i) list += QStringLiteral("<p:guide orient=\"horz\" pos=\"%1\"/>").arg(i * 4);   // (a half point apart: all on and near the slide)
        for (int i = 0; i < 4000; ++i) list += QStringLiteral("<p:guide pos=\"nope\"/>");
        list += QStringLiteral("<p:guide pos=\"1600\"/>");   // (past the 4096th element)
        ZipWriter zip;
        for (auto it = base.zip.cbegin(); it != base.zip.cend(); ++it) {
            QByteArray data = it.value();
            if (it.key() == QLatin1String("ppt/viewProps.xml")) {
                QVERIFY(data.contains("<p:guideLst/>"));
                data.replace("<p:guideLst/>", ("<p:guideLst>" + list + "</p:guideLst>").toUtf8());
                data.replace("<p:gridSpacing cx=\"76200\"", "<p:gridSpacing cx=\"9999999999\"");
            }
            zip.add(it.key(), data, true);
        }
        auto doc = deck::importPresentation(zip.finish(), &err);
        QVERIFY2(doc, qPrintable(err));
        const QSizeF size = doc->pageSize();
        QCOMPARE(doc->viewGuides.h.size(), 1000);                    // (1,200 in the file)
        QCOMPARE(doc->viewGuides.v.size(), 3);                       // (the far ones at the limits, the near one where it is; the word is not read)
        for (double v : doc->viewGuides.v) QVERIFY(v >= -size.width() && v <= 2 * size.width());
        for (double v : doc->viewGuides.h) QVERIFY(v >= -size.height() && v <= 2 * size.height());
        QVERIFY(doc->viewGuides.v.contains(2 * size.width()) && doc->viewGuides.v.contains(-size.width()) && doc->viewGuides.v.contains(-5000 / 8.0));
        // The spacing too is bounded.
        QVERIFY(deck::gridViewOf(*doc).spacing.value_or(0) <= 720);
    }

    // A project survives the path that settles live text fits first (a shape that shrinks its text), and a name the writer already holds is not written twice.
    void aMacroProjectSurvivesTheLiveFitPathAndTakenNames()
    {
        const QByteArray project("\xD0\xCF\x11\xE0 our own bytes");   // (no length: 20 read past its 18 bytes, memory checker Nov 17)
        auto doc = deck::importPresentation(macroPptx(QStringLiteral("application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml"), project, QByteArray()));
        QVERIFY(doc);
        auto s = std::make_shared<ShapeItem>();
        s->shape = QStringLiteral("roundRect");
        s->rect = QRectF(100, 100, 150, 60);
        s->storyId = doc->createStory(QStringLiteral("alpha beta gamma delta epsilon zeta eta theta"));
        s->shrinks = true;
        doc->pages[0]->items.push_back(s);
        QString err, why;
        Exported out;
        QVERIFY2(unpack(deck::exportPresentation(*doc, &err, nullptr, deck::PresentationKind::Presentation, false, true), &out, &why), qPrintable(why));
        QCOMPARE(out.zip.value(QStringLiteral("ppt/vbaProject.bin")), project);
        // a project whose name an empty part of the package already has goes under another, and nothing is written twice
        office::PackageWriter w;
        w.addPart(QStringLiteral("/ppt/vbaProject.bin"), QByteArray(), QStringLiteral("application/xml"));
        office::MacroProject mp;
        mp.parts.push_back(office::MacroPart{QStringLiteral("/ppt/vbaProject.bin"), QStringLiteral("application/vnd.ms-office.vbaProject"), project, -1, false});
        office::writeMacroProject(&w, QStringLiteral("/ppt/presentation.xml"), mp, QStringLiteral("/ppt/"));
        QCOMPARE(w.partData(QStringLiteral("/ppt/vbaProject.bin")), QByteArray());
        QCOMPARE(w.partData(QStringLiteral("/ppt/vbaProject2.bin")), project);
    }

    void initTestCase() { initDeckTestCase(); }

private:

    // A presentation made here with a macro project of our own bytes in it, as the file of that kind has one: the main part of the type given, the project and its
    // signature as parts of the types that name them, and the links to them.
    static QByteArray macroPptx(const QString &mainType, const QByteArray &project, const QByteArray &signature, const QByteArray &linkTarget = "vbaProject.bin", bool externalLink = false)
    {
        auto d = deck::newPresentation();
        QString err, why;
        Exported base;
        if (!unpack(deck::exportPresentation(*d, &err), &base, &why)) return QByteArray();
        ZipWriter zip;
        for (auto it = base.zip.cbegin(); it != base.zip.cend(); ++it) {
            QByteArray data = it.value();
            if (it.key() == QLatin1String("[Content_Types].xml")) {
                if (!data.contains("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml")) return QByteArray();   // (the caller checks: a test that changed nothing would test nothing)
                data.replace("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml", mainType.toUtf8());
                data.replace("</Types>", QByteArray("<Override PartName=\"/ppt/vbaProject.bin\" ContentType=\"application/vnd.ms-office.vbaProject\"/>") +
                                             (signature.isEmpty() ? QByteArray() : QByteArray("<Override PartName=\"/ppt/vbaProjectSignature.bin\" ContentType=\"application/vnd.ms-office.vbaProjectSignature\"/>")) + "</Types>");
            } else if (it.key() == QLatin1String("ppt/_rels/presentation.xml.rels")) {
                data.replace("</Relationships>", "<Relationship Id=\"rIdVba\" Type=\"http://schemas.microsoft.com/office/2006/relationships/vbaProject\" Target=\"" + linkTarget + (externalLink ? "\" TargetMode=\"External\"/></Relationships>" : "\"/></Relationships>"));
            }
            zip.add(it.key(), data, true);
        }
        zip.add(QStringLiteral("ppt/vbaProject.bin"), project, true);
        if (!signature.isEmpty()) {
            zip.add(QStringLiteral("ppt/vbaProjectSignature.bin"), signature, true);
            zip.add(QStringLiteral("ppt/_rels/vbaProject.bin.rels"),
                    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                    "<Relationship Id=\"rId1\" Type=\"http://schemas.microsoft.com/office/2006/relationships/vbaProjectSignature\" Target=\"vbaProjectSignature.bin\"/></Relationships>", true);
        }
        return zip.finish();
    }

    // The local names of an element's children, in order.
    static QStringList childNames(const QByteArray &xml)
    {
        QDomDocument dom;
        QString why;
        if (!office::parseXml(xml, &dom, &why)) return {QStringLiteral("unreadable: ") + why};
        QStringList out;
        for (const QDomElement &c : office::children(dom.documentElement())) out << c.localName();
        return out;
    }
    // Whether names come in the order of the schema's sequence (each at or after the one before).
    static bool inSequence(const QStringList &names, const QStringList &sequence)
    {
        int at = 0;
        for (const QString &n : names) {
            const int i = int(sequence.indexOf(n));
            if (i < at) return false;
            at = i;
        }
        return true;
    }

private Q_SLOTS:
    // The arc-based presets, with adjustments of their own, are saved as their presets and opened as the same presets, with
    // the same places and adjustments (an arc is its curve, named for it, and its sector).
    void arcPresetsSaveAndOpenAsThemselves()
    {
        const QStringList names = {QStringLiteral("arc"), QStringLiteral("chord"), QStringLiteral("pie"), QStringLiteral("blockArc")};
        const QVector<QVector<double>> adj = {{0.25, 0.5}, {0.0625, 0.25}, {0.25, 0.5}, {0.25, 0.75, 0.1}};
        auto d = deck::newPresentation();
        for (int i = 0; i < names.size(); ++i) {
            auto s = std::make_shared<jp::ShapeItem>();
            s->rect = QRectF(20 + 150 * i, 30, 136.6, 90);
            s->shape = names[i];
            s->adj = adj[i];
            s->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(0x20, 0x40, 0x80)));
            s->stroke = jp::Stroke::line(jp::ColorRef::rgb(QColor(0x80, 0x40, 0x20)), 1.5);
            d->pages[0]->items.push_back(s);
        }
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join('\n')));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        // Every preset is found by its name, with its place and its adjustments.
        std::function<const jp::ShapeItem *(const jp::ItemPtr &, const QString &)> find = [&](const jp::ItemPtr &it, const QString &name) -> const jp::ShapeItem * {
            if (it->type() == jp::ItemType::Shape) return static_cast<const jp::ShapeItem *>(it.get())->shape == name ? static_cast<const jp::ShapeItem *>(it.get()) : nullptr;
            if (it->type() == jp::ItemType::Group)
                for (const jp::ItemPtr &c : static_cast<const jp::GroupItem *>(it.get())->children)
                    if (const jp::ShapeItem *s = find(c, name)) return s;
            return nullptr;
        };
        for (int i = 0; i < names.size(); ++i) {
            const jp::ShapeItem *s = nullptr;
            for (const jp::ItemPtr &it : back->pages[0]->items)
                if (!s) s = find(it, names[i]);
            QVERIFY2(s, qPrintable(names[i]));
            QVERIFY2(qAbs(s->rect.width() - 136.6) < 0.01 && qAbs(s->rect.height() - 90) < 0.01, qPrintable(names[i]));
            QCOMPARE(int(s->adj.size()), int(adj[i].size()));
            for (int k = 0; k < adj[i].size(); ++k) QVERIFY2(qAbs(s->adj[k] - adj[i][k]) < 1e-6, qPrintable(names[i]));
        }
    }

    // The header and footer of notes pages and handouts are the hf element of the notes master and of the handout master, with their text in the master's own places; they come back.
    void printHeaderFooterIsKeptInTheNotesMasterAndTheHandoutMaster()
    {
        auto d = deck::newPresentation();
        deck::PrintHeaderFooter notes, handouts;
        notes.header = true;
        notes.headerText = QStringLiteral("Notes header");
        notes.footer = true;
        notes.footerText = QStringLiteral("Notes footer");
        notes.pageNumber = false;
        handouts.date = true;
        handouts.dateText = QStringLiteral("3 March");
        handouts.footer = true;
        handouts.footerText = QStringLiteral("Handout footer");
        deck::setPrintHeaderFooter(*d, deck::PrintedPages::Notes, notes);
        deck::setPrintHeaderFooter(*d, deck::PrintedPages::Handouts, handouts);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QString why;
        QVERIFY2(unpack(bytes, &e, &why), qPrintable(why));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString notesXml = QString::fromUtf8(e.zip.value(QStringLiteral("ppt/notesMasters/notesMaster1.xml")));
        QVERIFY2(notesXml.contains(QLatin1String("<p:hf")) && notesXml.contains(QLatin1String("dt=\"0\"")) && notesXml.contains(QLatin1String("sldNum=\"0\"")), qPrintable(notesXml));
        QVERIFY(notesXml.contains(QLatin1String("Notes header")) && notesXml.contains(QLatin1String("Notes footer")));
        QCOMPARE(e.pkg.relationshipsOfType(QStringLiteral("/ppt/presentation.xml"), QStringLiteral("/handoutMaster")).size(), 1);
        const QString handoutXml = QString::fromUtf8(e.zip.value(QStringLiteral("ppt/handoutMasters/handoutMaster1.xml")));
        QVERIFY2(handoutXml.contains(QLatin1String("<p:hf")) && handoutXml.contains(QLatin1String("hdr=\"0\"")) && handoutXml.contains(QLatin1String("3 March")) && handoutXml.contains(QLatin1String("Handout footer")), qPrintable(handoutXml));
        QVERIFY(part(e, QStringLiteral("/ppt/presentation.xml")).contains(QStringLiteral("<p:handoutMasterIdLst>")));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(deck::hasPrintHeaderFooter(*back, deck::PrintedPages::Notes));
        QVERIFY(deck::printHeaderFooter(*back, deck::PrintedPages::Notes) == notes);
        QVERIFY(deck::printHeaderFooter(*back, deck::PrintedPages::Handouts) == handouts);
        // A presentation that never chose has no handout master, and none is made up for it on reading.
        auto plain = deck::newPresentation();
        Exported p;
        QVERIFY(unpack(deck::exportPresentation(*plain, &err), &p, &why));
        QVERIFY(!p.zip.contains(QStringLiteral("ppt/handoutMasters/handoutMaster1.xml")));
        QVERIFY(!QString::fromUtf8(p.zip.value(QStringLiteral("ppt/notesMasters/notesMaster1.xml"))).contains(QLatin1String("<p:hf")));
        auto plainBack = deck::importPresentation(deck::exportPresentation(*plain, &err), &err);
        QVERIFY(plainBack && !deck::hasPrintHeaderFooter(*plainBack, deck::PrintedPages::Notes) && !deck::hasPrintHeaderFooter(*plainBack, deck::PrintedPages::Handouts));
    }

    // A handout master the file had is kept as it was: its places, background, and other shapes stay, and only the hf element and the texts the person changed are written new. One
    // is made only when the file had none and the person chose a header and footer.
    void aHandoutMasterOfTheFileIsKeptAndOnlyItsHeaderAndFooterChange()
    {
        // A file with a customized handout master: made from an export, with a shape of its own and a background color put in.
        auto d = deck::newPresentation();
        deck::PrintHeaderFooter hf;
        hf.header = true;
        hf.headerText = QStringLiteral("Original header");
        hf.footer = true;
        hf.footerText = QStringLiteral("Original footer");
        deck::setPrintHeaderFooter(*d, deck::PrintedPages::Handouts, hf);
        QString err;
        Exported first;
        QString why;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &first, &why), qPrintable(why));
        const QString name = QStringLiteral("ppt/handoutMasters/handoutMaster1.xml");
        QString custom = QString::fromUtf8(first.zip.value(name));
        QVERIFY(custom.contains(QLatin1String("<p:bgRef idx=\"1001\">")));
        custom.replace(QRegularExpression(QStringLiteral("<p:bg>.*</p:bg>")), QStringLiteral("<p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"FFEEDD\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg>"));
        custom.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"99\" name=\"OurOwnShape\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr><a:xfrm><a:off x=\"100\" y=\"100\"/><a:ext cx=\"500000\" cy=\"500000\"/></a:xfrm>"
                                                                  "<a:prstGeom prst=\"ellipse\"><a:avLst/></a:prstGeom></p:spPr></p:sp></p:spTree>"));
        // It has a theme of its own and a picture of its own.
        const QString themeName = relationshipTargets(first, QStringLiteral("/ppt/handoutMasters/handoutMaster1.xml"), QStringLiteral("/theme")).value(0).mid(1);
        QVERIFY(!themeName.isEmpty());
        QByteArray ownTheme = first.zip.value(themeName);
        ownTheme.replace("</a:theme>", "<!--HANDOUT-OWN-THEME--></a:theme>");
        QImage picture(3, 2, QImage::Format_RGB32);
        picture.fill(QColor(12, 200, 99));
        QByteArray png;
        QBuffer pngBuffer(&png);
        pngBuffer.open(QIODevice::WriteOnly);
        picture.save(&pngBuffer, "PNG");
        const QString relsName = QStringLiteral("ppt/handoutMasters/_rels/handoutMaster1.xml.rels");
        QString rels = QString::fromUtf8(first.zip.value(relsName));
        rels.replace(QStringLiteral("</Relationships>"), QStringLiteral("<Relationship Id=\"rId9\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"../media/handoutpic.png\"/></Relationships>"));
        custom.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"98\" name=\"OurPicture\"/><p:cNvPicPr/><p:nvPr/></p:nvPicPr><p:blipFill><a:blip r:embed=\"rId9\"/><a:stretch><a:fillRect/></a:stretch></p:blipFill>"
                                                                  "<p:spPr><a:xfrm><a:off x=\"200\" y=\"200\"/><a:ext cx=\"300000\" cy=\"300000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr></p:pic></p:spTree>"));
        QString types = QString::fromUtf8(first.zip.value(QStringLiteral("[Content_Types].xml")));
        if (!types.contains(QLatin1String("Extension=\"png\"")))
            types.replace(QStringLiteral("</Types>"), QStringLiteral("<Default Extension=\"png\" ContentType=\"image/png\"/></Types>"));
        ZipWriter zip;
        for (auto it = first.zip.cbegin(); it != first.zip.cend(); ++it) {
            const QString key = it.key();
            zip.add(key, key == name ? custom.toUtf8() : key == themeName ? ownTheme : key == relsName ? rels.toUtf8() : key == QLatin1String("[Content_Types].xml") ? types.toUtf8() : it.value(), true);
        }
        zip.add(QStringLiteral("ppt/media/handoutpic.png"), png, true);
        const QByteArray file = zip.finish();

        // Read and written again with no change: the master is as it was, to the byte.
        auto read = deck::importPresentation(file, &err);
        QVERIFY2(read, qPrintable(err));
        Exported same;
        QVERIFY2(unpack(deck::exportPresentation(*read, &err), &same, &why), qPrintable(why));
        QVERIFY2(packageProblem(same).isEmpty(), qPrintable(packageProblem(same)));
        QCOMPARE(QString::fromUtf8(same.zip.value(name)), custom);
        // Its theme and its picture are still its own, and identical.
        auto ownedBy = [&](const Exported &e) {
            QStringList out;
            for (const QString &type : {QStringLiteral("/theme"), QStringLiteral("/image")})
                for (const QString &target : relationshipTargets(e, QStringLiteral("/ppt/handoutMasters/handoutMaster1.xml"), type)) out << QString::fromLatin1(e.zip.value(target.mid(1)).toBase64());
            return out;
        };
        QCOMPARE(ownedBy(same), (QStringList{QString::fromLatin1(ownTheme.toBase64()), QString::fromLatin1(png.toBase64())}));

        // The person changes the header and turns the footer off: the shape and the background stay.
        deck::PrintHeaderFooter now = deck::printHeaderFooter(*read, deck::PrintedPages::Handouts);
        QVERIFY(now.header && now.footer);
        now.headerText = QStringLiteral("New header");
        now.footer = false;
        deck::setPrintHeaderFooter(*read, deck::PrintedPages::Handouts, now);
        Exported edited;
        QVERIFY2(unpack(deck::exportPresentation(*read, &err), &edited, &why), qPrintable(why));
        QVERIFY2(packageProblem(edited).isEmpty(), qPrintable(packageProblem(edited)));
        const QString out = QString::fromUtf8(edited.zip.value(name));
        QVERIFY2(out.contains(QLatin1String("OurOwnShape")) && out.contains(QLatin1String("FFEEDD")), qPrintable(out));
        QVERIFY(out.contains(QLatin1String("New header")) && !out.contains(QLatin1String("Original header")));
        QCOMPARE(ownedBy(edited), ownedBy(same));
        QVERIFY(out.contains(QLatin1String("OurPicture")));
        QVERIFY(out.contains(QLatin1String("Original footer")));   // (its text was not touched: only the footer's switch)
        QVERIFY2(out.contains(QRegularExpression(QStringLiteral("<p:hf[^>]*ftr=\"0\""))), qPrintable(out));
        auto back = deck::importPresentation(deck::exportPresentation(*read, &err), &err);
        QVERIFY(back);
        QVERIFY(deck::printHeaderFooter(*back, deck::PrintedPages::Handouts) == now);

        // A file with no handout master gets none made until a choice is.
        auto none = deck::newPresentation();
        Exported plain;
        QVERIFY(unpack(deck::exportPresentation(*none, &err), &plain, &why));
        QVERIFY(!plain.zip.contains(name));
    }

    // A new presentation is written as a whole package: every part has its
    // type, every link leads to a part, and the main part, its master, the
    // layouts, the slides, and the notes are chained as the presentation
    // program chains them.
    void exportWritesACompletePackage()
    {
        auto d = deck::newPresentation();
        d->props.title = QStringLiteral("Quarterly results");
        d->storyDoc(d->pages[0]->notesStoryId)->setPlainText(QStringLiteral("Say hello first"));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("twoObj")));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        d->pages[2]->hidden = true;
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QString why;
        QVERIFY2(unpack(bytes, &e, &why), qPrintable(why));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));

        QCOMPARE(e.pkg.mainPart(), QStringLiteral("/ppt/presentation.xml"));
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/presentation.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"));
        const QStringList names = e.pkg.partNames();
        for (const char *n : {"/ppt/presProps.xml", "/ppt/viewProps.xml", "/ppt/tableStyles.xml", "/ppt/theme/theme1.xml", "/ppt/slideMasters/slideMaster1.xml",
                              "/ppt/notesMasters/notesMaster1.xml", "/ppt/slides/slide3.xml", "/ppt/slideLayouts/slideLayout9.xml", "/ppt/notesSlides/notesSlide1.xml",
                              "/docProps/core.xml", "/docProps/app.xml", "/docProps/thumbnail.jpeg"})
            QVERIFY2(names.contains(QLatin1String(n)), n);
        QCOMPARE(int(names.filter(QRegularExpression(QStringLiteral("^/ppt/slides/slide\\d+\\.xml$"))).size()), 3);
        QCOMPARE(int(names.filter(QRegularExpression(QStringLiteral("^/ppt/slideLayouts/slideLayout\\d+\\.xml$"))).size()), 9);
        QCOMPARE(int(names.filter(QRegularExpression(QStringLiteral("^/ppt/notesSlides/"))).size()), 1);   // only the first slide has notes
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/slides/slide1.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml"));
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/slideLayouts/slideLayout1.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"));
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/slideMasters/slideMaster1.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"));
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/theme/theme1.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));
        QCOMPARE(e.pkg.contentType(QStringLiteral("/docProps/thumbnail.jpeg")), QStringLiteral("image/jpeg"));

        // The chains: presentation to master, slides in order, notes master; master to layouts and theme.
        const QString pres = QStringLiteral("/ppt/presentation.xml");
        QCOMPARE(relationshipTargets(e, pres, QStringLiteral("/slideMaster")), QStringList{QStringLiteral("/ppt/slideMasters/slideMaster1.xml")});
        QCOMPARE(relationshipTargets(e, pres, QStringLiteral("/slide")),
                 (QStringList{QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/ppt/slides/slide2.xml"), QStringLiteral("/ppt/slides/slide3.xml")}));
        QCOMPARE(relationshipTargets(e, pres, QStringLiteral("/notesMaster")), QStringList{QStringLiteral("/ppt/notesMasters/notesMaster1.xml")});
        for (const char *type : {"/presProps", "/viewProps", "/theme", "/tableStyles"}) QCOMPARE(int(e.pkg.relationshipsOfType(pres, QLatin1String(type)).size()), 1);
        const QString master = QStringLiteral("/ppt/slideMasters/slideMaster1.xml");
        const QStringList layoutParts = relationshipTargets(e, master, QStringLiteral("/slideLayout"));
        QCOMPARE(int(layoutParts.size()), 9);
        QCOMPARE(int(e.pkg.relationshipsOfType(master, QStringLiteral("/theme")).size()), 1);
        for (const QString &l : layoutParts) QCOMPARE(relationshipTargets(e, l, QStringLiteral("/slideMaster")), QStringList{master});
        // Each slide stands on the layout of its type, in the order the master lists them.
        const QVector<MasterPage *> layouts = deck::layouts(*d);
        for (int i = 0; i < 3; ++i) {
            const QStringList on = relationshipTargets(e, QStringLiteral("/ppt/slides/slide%1.xml").arg(i + 1), QStringLiteral("/slideLayout"));
            QCOMPARE(int(on.size()), 1);
            QCOMPARE(on[0], layoutParts.value(int(layouts.indexOf(d->masterFor(*d->pages[i])))));
        }
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/notesSlide")), QStringList{QStringLiteral("/ppt/notesSlides/notesSlide1.xml")});
        QVERIFY(relationshipTargets(e, QStringLiteral("/ppt/slides/slide2.xml"), QStringLiteral("/notesSlide")).isEmpty());
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/notesSlides/notesSlide1.xml"), QStringLiteral("/notesMaster")), QStringList{QStringLiteral("/ppt/notesMasters/notesMaster1.xml")});
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/notesSlides/notesSlide1.xml"), QStringLiteral("/slide")), QStringList{QStringLiteral("/ppt/slides/slide1.xml")});
        QCOMPARE(int(e.pkg.relationshipsOfType(QStringLiteral("/ppt/notesMasters/notesMaster1.xml"), QStringLiteral("/theme")).size()), 1);
        // Master and notes master do not share a theme part.
        QVERIFY(relationshipTargets(e, QStringLiteral("/ppt/notesMasters/notesMaster1.xml"), QStringLiteral("/theme")) != relationshipTargets(e, master, QStringLiteral("/theme")));
        // The package's own links.
        QCOMPARE(int(e.pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/officeDocument")).size()), 1);
        QCOMPARE(int(e.pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/metadata/core-properties")).size()), 1);
        QCOMPARE(int(e.pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/extended-properties")).size()), 1);
        QCOMPARE(int(e.pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/metadata/thumbnail")).size()), 1);

        // What the properties say.
        const QString core = part(e, QStringLiteral("/docProps/core.xml")), app = part(e, QStringLiteral("/docProps/app.xml"));
        QVERIFY(core.contains(QStringLiteral("<dc:title>Quarterly results</dc:title>")));
        QVERIFY2(core.contains(QRegularExpression(QStringLiteral("<dcterms:created xsi:type=\"dcterms:W3CDTF\">\\d{4}-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\dZ</dcterms:created>"))), qPrintable(core));
        QVERIFY(core.contains(QStringLiteral("<dcterms:modified xsi:type=\"dcterms:W3CDTF\">")));
        QVERIFY(app.contains(QStringLiteral("<Application>JeffOffice</Application>")));
        QVERIFY(app.contains(QStringLiteral("<Slides>3</Slides>")));
        QVERIFY(app.contains(QStringLiteral("<Notes>1</Notes>")));
        QVERIFY(app.contains(QStringLiteral("<HiddenSlides>1</HiddenSlides>")));
        // The thumbnail is the first slide, 256 pixels across.
        const QImage thumb = QImage::fromData(e.zip.value(QStringLiteral("docProps/thumbnail.jpeg")));
        QCOMPARE(thumb.width(), 256);
        QVERIFY(std::abs(thumb.height() - 144) <= 1);
    }

    // The comment parts a deck with comments saves are exactly what they were before the comments moved to the shared library: the same parts, byte for byte.
    void commentPartsAreAsTheyWere()
    {
        auto d = deck::newPresentation();
        const comments::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")}, lee{QStringLiteral("Lee Roy"), QStringLiteral("LR")};
        const QDateTime noon(QDate(2026, 10, 12), QTime(12, 0, 5, 125), QTimeZone::UTC);
        const QString first = comments::addComment(*d->pages[0], pat, QStringLiteral("Check the numbers"), QPointF(100.5, 40.25), noon);
        comments::addReply(*d->pages[0], first, lee, QStringLiteral("Agreed"), noon.addSecs(90));
        comments::resolveComment(*d->pages[0], first);
        comments::addComment(*d->pages[0], lee, QStringLiteral("Second"), QPointF(300, 200), noon.addSecs(120));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QString got;
        for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it)
            if (it.key().contains(QLatin1String("omment"))) got += it.key() + QLatin1Char('\n') + QString::fromUtf8(it.value()) + QLatin1Char('\n');
        QVERIFY(!got.isEmpty());
        // (the dates and the zone are the computer's: left out of the comparison)
        got.replace(QRegularExpression(QStringLiteral(" dt=\"[^\"]*\"")), QStringLiteral(" dt=\"T\""));
        got.replace(QRegularExpression(QStringLiteral("timeZoneBias=\"[^\"]*\"")), QStringLiteral("timeZoneBias=\"Z\""));
        // As the parts were written before the comments moved to libs/page (captured then, with the same deck).
        const QString before = QString::fromUtf8(
            "ppt/commentAuthors.xml\n"
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<p:cmAuthorLst xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:cmAuthor id=\"0\" name=\"Pat Doe\" initials=\"PD\" lastIdx=\"1\" clrIdx=\"0\"/><p:cmAuthor id=\"1\" name=\"Lee Roy\" initials=\"LR\" lastIdx=\"2\" clrIdx=\"1\"/></p:cmAuthorLst>\n"
            "ppt/comments/comment1.xml\n"
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<p:cmLst xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:cm authorId=\"0\" dt=\"T\" idx=\"1\"><p:pos x=\"804\" y=\"322\"/><p:text>Check the numbers</p:text><p:extLst><p:ext uri=\"{C676402C-5697-4E1C-873F-D02D1690AC5C}\"><p15:threadingInfo xmlns:p15=\"http://schemas.microsoft.com/office/powerpoint/2012/main\" timeZoneBias=\"Z\"/></p:ext><p:ext uri=\"{7C0E3E5B-5A0B-4B52-9D6E-2F1A8E4C9B21}\"><jo:done xmlns:jo=\"http://jeffoffice.com/ns/deck/comments\" val=\"1\"/></p:ext></p:extLst></p:cm><p:cm authorId=\"1\" dt=\"T\" idx=\"1\"><p:pos x=\"804\" y=\"322\"/><p:text>Agreed</p:text><p:extLst><p:ext uri=\"{C676402C-5697-4E1C-873F-D02D1690AC5C}\"><p15:threadingInfo xmlns:p15=\"http://schemas.microsoft.com/office/powerpoint/2012/main\" timeZoneBias=\"Z\"><p15:parentCm authorId=\"0\" idx=\"1\"/></p15:threadingInfo></p:ext></p:extLst></p:cm><p:cm authorId=\"1\" dt=\"T\" idx=\"2\"><p:pos x=\"2400\" y=\"1600\"/><p:text>Second</p:text><p:extLst><p:ext uri=\"{C676402C-5697-4E1C-873F-D02D1690AC5C}\"><p15:threadingInfo xmlns:p15=\"http://schemas.microsoft.com/office/powerpoint/2012/main\" timeZoneBias=\"Z\"/></p:ext></p:extLst></p:cm></p:cmLst>\n");
        QCOMPARE(got, before);
    }

    // Each part's elements follow the schema's sequence, the ids that must be
    // unique are, and the slide size says what it is.
    void exportWritesElementsInSchemaOrder()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));

        const QByteArray pres = e.zip.value(QStringLiteral("ppt/presentation.xml"));
        QStringList names = childNames(pres);
        QVERIFY2(inSequence(names, {"sldMasterIdLst", "notesMasterIdLst", "handoutMasterIdLst", "sldIdLst", "sldSz", "notesSz", "smartTags", "embeddedFontLst", "custShowLst",
                                    "photoAlbum", "custDataLst", "kinsoku", "defaultTextStyle", "modifyVerifier", "extLst"}), qPrintable(names.join(' ')));
        for (const char *required : {"sldMasterIdLst", "sldIdLst", "sldSz", "notesSz", "defaultTextStyle"}) QVERIFY2(names.contains(QLatin1String(required)), required);
        QVERIFY2(inSequence(childNames(e.zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml"))), {"cSld", "clrMap", "sldLayoutIdLst", "transition", "timing", "hf", "txStyles", "extLst"}), "master");
        QCOMPARE(childNames(e.zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml"))).mid(0, 2), (QStringList{"cSld", "clrMap"}));
        for (const QString &l : {QStringLiteral("ppt/slideLayouts/slideLayout1.xml"), QStringLiteral("ppt/slideLayouts/slideLayout2.xml")}) {
            names = childNames(e.zip.value(l));
            QVERIFY2(inSequence(names, {"cSld", "clrMapOvr", "transition", "timing", "hf", "extLst"}), qPrintable(l + ": " + names.join(' ')));
            QCOMPARE(names, (QStringList{"cSld", "clrMapOvr"}));
        }
        for (const QString &s : {QStringLiteral("ppt/slides/slide1.xml"), QStringLiteral("ppt/slides/slide2.xml")}) {
            names = childNames(e.zip.value(s));
            QVERIFY2(inSequence(names, {"cSld", "clrMapOvr", "transition", "timing", "extLst"}), qPrintable(s + ": " + names.join(' ')));
        }
        // The first slide's transition and timing follow its color mapping.
        QCOMPARE(childNames(e.zip.value(QStringLiteral("ppt/slides/slide1.xml"))), (QStringList{"cSld", "clrMapOvr", "transition", "timing"}));
        QCOMPARE(childNames(e.zip.value(QStringLiteral("ppt/slides/slide2.xml"))), (QStringList{"cSld", "clrMapOvr"}));
        QVERIFY(inSequence(childNames(e.zip.value(QStringLiteral("ppt/notesSlides/notesSlide1.xml"))), {"cSld", "clrMapOvr", "extLst"}));
        QVERIFY(inSequence(childNames(e.zip.value(QStringLiteral("ppt/notesMasters/notesMaster1.xml"))), {"cSld", "clrMap", "hf", "notesStyle", "extLst"}));
        // A shape tree starts with its group's properties, after the background.
        for (const QString &s : {QStringLiteral("ppt/slides/slide1.xml"), QStringLiteral("ppt/slides/slide2.xml"), QStringLiteral("ppt/slideMasters/slideMaster1.xml"), QStringLiteral("ppt/slideLayouts/slideLayout1.xml")}) {
            QDomDocument dom;
            const QDomElement cSld = office::child(rootOf(e.zip.value(s), &dom), office::ns::P, QStringLiteral("cSld"));
            QStringList inside;
            for (const QDomElement &c : office::children(cSld)) inside << c.localName();
            QVERIFY2(inSequence(inside, {"bg", "spTree", "custDataLst", "controls", "extLst"}) && inside.contains(QStringLiteral("spTree")), qPrintable(s + ": " + inside.join(' ')));
            const QDomElement tree = office::child(cSld, office::ns::P, QStringLiteral("spTree"));
            const QVector<QDomElement> kids = office::children(tree);
            QVERIFY(kids.size() >= 2);
            QCOMPARE(kids[0].localName(), QStringLiteral("nvGrpSpPr"));
            QCOMPARE(kids[1].localName(), QStringLiteral("grpSpPr"));
        }

        // The ids: masters and layouts share one range from 2147483648 and none repeats; slides count from 256.
        QDomDocument dom;
        const QDomElement root = rootOf(pres, &dom);
        QSet<qint64> ids;
        for (const QDomElement &m : office::children(office::child(root, office::ns::P, QStringLiteral("sldMasterIdLst")))) ids << office::attr(m, QStringLiteral("id")).toLongLong();
        QDomDocument masterDom;
        const QDomElement master = rootOf(e.zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml")), &masterDom);
        const QVector<QDomElement> layoutIds = office::children(office::child(master, office::ns::P, QStringLiteral("sldLayoutIdLst")));
        QCOMPARE(int(layoutIds.size()), 2);
        for (const QDomElement &l : layoutIds) ids << office::attr(l, QStringLiteral("id")).toLongLong();
        QCOMPARE(int(ids.size()), 3);
        for (qint64 id : std::as_const(ids)) QVERIFY(id >= 2147483648LL && id <= 4294967295LL);
        QStringList slideIds;
        for (const QDomElement &s : office::children(office::child(root, office::ns::P, QStringLiteral("sldIdLst")))) slideIds << office::attr(s, QStringLiteral("id"));
        QCOMPARE(slideIds, (QStringList{"256", "257"}));
        // The master's layouts are the ones the layout parts are, in order.
        QCOMPARE(layoutIds[0].attributeNS(QString::fromLatin1(office::ns::R.data()), QStringLiteral("id")), e.pkg.relationshipsOfType(QStringLiteral("/ppt/slideMasters/slideMaster1.xml"), QStringLiteral("/slideLayout"))[0].id);

        // The size: widescreen says no type; the standard 4:3 slide says screen4x3.
        QDomElement sz = office::child(root, office::ns::P, QStringLiteral("sldSz"));
        QCOMPARE(sz.attribute(QStringLiteral("cx")), QStringLiteral("12192000"));
        QCOMPARE(sz.attribute(QStringLiteral("cy")), QStringLiteral("6858000"));
        QVERIFY(!sz.hasAttribute(QStringLiteral("type")));
        const QDomElement notesSz = office::child(root, office::ns::P, QStringLiteral("notesSz"));
        QCOMPARE(notesSz.attribute(QStringLiteral("cx")), QStringLiteral("6858000"));
        QCOMPARE(notesSz.attribute(QStringLiteral("cy")), QStringLiteral("9144000"));
        deck::setSlideSize(*d, QSizeF(deck::kStandardWidth, deck::kStandardHeight));
        Exported small;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &small, &err), qPrintable(err));
        QDomDocument smallDom;
        sz = office::child(rootOf(small.zip.value(QStringLiteral("ppt/presentation.xml")), &smallDom), office::ns::P, QStringLiteral("sldSz"));
        QCOMPARE(sz.attribute(QStringLiteral("cx")), QStringLiteral("9144000"));
        QCOMPARE(sz.attribute(QStringLiteral("cy")), QStringLiteral("6858000"));
        QCOMPARE(sz.attribute(QStringLiteral("type")), QStringLiteral("screen4x3"));
        QVERIFY2(packageProblem(small).isEmpty(), qPrintable(packageProblem(small)));
    }

    // Exported and read back, the slides are the same slides: their order,
    // layouts, hidden flags, and backgrounds, the notes, and what the reader
    // keeps (transition, timing, frames) as it was, and the shapes and the text in them.
    void exportRoundTripKeepsTheSlides()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));

        QCOMPARE(back->pageSize(), d->pageSize());
        QCOMPARE(back->setup.sizeName, d->setup.sizeName);
        QCOMPARE(int(back->pages.size()), 2);
        QCOMPARE(deck::layouts(*back).size(), deck::layouts(*d).size());
        for (int i = 0; i < 2; ++i) {
            QCOMPARE(back->masterFor(*back->pages[i])->name, d->masterFor(*d->pages[i])->name);
            QCOMPARE(deck::layoutType(*back->masterFor(*back->pages[i])), deck::layoutType(*d->masterFor(*d->pages[i])));
            QCOMPARE(back->pages[i]->hidden, d->pages[i]->hidden);
            QCOMPARE(back->pages[i]->showMasterItems, d->pages[i]->showMasterItems);
        }
        QVERIFY(!back->pages[0]->hidden);
        QVERIFY(back->pages[1]->hidden);
        QVERIFY(!back->pages[1]->showMasterItems);
        QCOMPARE(back->pages[1]->background.type, Fill::Solid);
        QCOMPARE(back->pages[1]->background.color.rgbValue(), QColor(0x11, 0x22, 0x33));
        QCOMPARE(back->pages[0]->background.type, Fill::NoFill);
        // The layouts keep their names, and the Title Slide layout hides the master's graphics.
        QCOMPARE(back->masters[1]->name, QStringLiteral("Title Slide"));
        QVERIFY(!back->masters[1]->showMasterItems);
        QCOMPARE(back->masters[2]->name, QStringLiteral("Title and Content"));
        QVERIFY(back->masters[2]->showMasterItems);
        QCOMPARE(back->masters[0]->name, d->masters[0]->name);

        // What the reader keeps comes back as it was kept.
        QCOMPARE(back->pages[0]->props.value(QStringLiteral("transitionXml")).toString(), d->pages[0]->props.value(QStringLiteral("transitionXml")).toString());
        QVERIFY(!back->pages[0]->props.value(QStringLiteral("transitionXml")).toString().isEmpty());
        QCOMPARE(back->pages[0]->props.value(QStringLiteral("timingXml")).toString(), d->pages[0]->props.value(QStringLiteral("timingXml")).toString());
        QVERIFY(!back->pages[0]->props.value(QStringLiteral("timingXml")).toString().isEmpty());
        QVERIFY(!back->pages[1]->props.contains(QStringLiteral("transitionXml")));
        const QJsonArray was = d->pages[0]->props.value(QStringLiteral("unread")).toArray(), now = back->pages[0]->props.value(QStringLiteral("unread")).toArray();
        QCOMPARE(int(was.size()), 1);
        QCOMPARE(now, was);
        QCOMPARE(back->appData.value(QStringLiteral("themeXml")), d->appData.value(QStringLiteral("themeXml")));
        QCOMPARE(back->appData.value(QStringLiteral("notesSize")), d->appData.value(QStringLiteral("notesSize")));
        QCOMPARE(deck::themeOf(*back).colors[4], deck::themeOf(*d).colors[4]);

        // The notes: a page for the first slide, none for the second.
        QCOMPARE(int(back->pages[0]->notesStoryId.isEmpty()), 0);
        QCOMPARE(deck::notesText(*back, *back->pages[0]), QStringLiteral("Say hello first\nThen the numbers"));
        QVERIFY(deck::notesText(*back, *back->pages[1]).isEmpty());
        // The shapes: the placeholders with their roles, the text, the picture.
        QCOMPARE(int(back->pages[0]->items.size()), int(d->pages[0]->items.size()));
        {
            QStringList roles, backRoles;
            for (const auto &it : d->pages[0]->items) roles << it->role;
            for (const auto &it : back->pages[0]->items) backRoles << it->role;
            QCOMPARE(backRoles, roles);
        }
        QCOMPARE(back->pages[0]->title, QStringLiteral("Quarterly results"));
    }

    // A new presentation, saved and opened again, is the presentation it was.
    void exportRoundTripOfANewPresentation()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("secHead")));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("picTx")));
        d->pages[1]->hidden = true;
        deck::setSlideBackground(*d->pages[2], QColor(0x20, 0x40, 0x60));
        d->storyDoc(d->pages[2]->notesStoryId)->setPlainText(QStringLiteral("Third slide"));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QCOMPARE(back->pageSize(), QSizeF(960, 540));
        QCOMPARE(back->setup.sizeName, QStringLiteral("Widescreen"));
        QCOMPARE(int(back->pages.size()), 3);
        QStringList types, backTypes;
        for (int i = 0; i < 3; ++i) {
            types << deck::layoutType(*d->masterFor(*d->pages[i]));
            backTypes << deck::layoutType(*back->masterFor(*back->pages[i]));
        }
        QCOMPARE(backTypes, (QStringList{"title", "secHead", "picTx"}));
        QCOMPARE(backTypes, types);
        QCOMPARE(back->pages[1]->hidden, true);
        QCOMPARE(back->pages[2]->background.color.rgbValue(), QColor(0x20, 0x40, 0x60));
        // The nine layouts, over the one master, in their order, with the theme.
        QCOMPARE(int(deck::layouts(*back).size()), 9);
        QStringList layoutTypes, backLayoutTypes;
        for (MasterPage *l : deck::layouts(*d)) layoutTypes << deck::layoutType(*l);
        for (MasterPage *l : deck::layouts(*back)) backLayoutTypes << deck::layoutType(*l);
        QCOMPARE(backLayoutTypes, layoutTypes);
        QCOMPARE(deck::themeOf(*back).name, QStringLiteral("JeffOffice"));
        for (int i = 0; i < 12; ++i) QCOMPARE(deck::themeOf(*back).colors[i], deck::themeOf(*d).colors[i]);
        // The theme's font is the standard font a new presentation is drawn to (Calibri).
        QCOMPARE(deck::themeOf(*back).headingFont, QStringLiteral("Calibri"));
        QCOMPARE(deck::themeOf(*back).bodyFont, QStringLiteral("Calibri"));
        QCOMPARE(int(back->pages[0]->items.size()), 2);
        QCOMPARE(deck::notesText(*back, *back->pages[2]), QStringLiteral("Third slide"));
    }

    // What the reader kept that points at parts it did not keep, or at shapes
    // whose numbers it did not keep, is left out with a note, never written
    // as a link that leads nowhere.
    void exportLeavesOutWhatWouldPointNowhere()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        const QString chart = QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"9\" name=\"Chart 8\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                                             "<p:xfrm><a:off x=\"914400\" y=\"914400\"/><a:ext cx=\"3657600\" cy=\"1371600\"/></p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\">"
                                             "<c:chart xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" r:id=\"rId9\"/></a:graphicData></a:graphic></p:graphicFrame>");
        xml.replace(QStringLiteral("</p:spTree>"), chart + QStringLiteral("</p:spTree>"));
        // An animation that aims at a shape that is not there, and a transition with a sound.
        xml.replace(QRegularExpression(QStringLiteral("<p:timing>.*</p:timing>")),
                    QStringLiteral("<p:timing><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"><p:childLst><p:set><p:cBhvr><p:cTn id=\"2\"/><p:tgtEl><p:spTgt spid=\"99\"/></p:tgtEl></p:cBhvr></p:set></p:childLst></p:cTn></p:par></p:tnLst></p:timing>"));
        xml.replace(QStringLiteral("<p:transition spd=\"med\"><p:fade/></p:transition>"),
                    QStringLiteral("<p:transition spd=\"med\"><p:fade/><p:sndAc><p:stSnd><p:snd r:embed=\"rId8\" name=\"chime.wav\"/></p:stSnd></p:sndAc></p:transition>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages[0]->props.value(QStringLiteral("unread")).toArray().size()), 2);   // the unknown frame, and the chart

        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("Cell &amp; &lt;1&gt;")));   // the frame has nothing to point at, and stays
        QVERIFY(!slide.contains(QStringLiteral("c:chart")));
        QVERIFY(!slide.contains(QStringLiteral("rId9")));
        QVERIFY(!slide.contains(QStringLiteral("<p:timing")));
        QVERIFY(!slide.contains(QStringLiteral("chime.wav")));
        // The transition stays, without its sound.
        QVERIFY(slide.contains(QStringLiteral("<p:fade/>")) && !slide.contains(QStringLiteral("sndAc")));
        int notes = 0;
        for (const QString &s : std::as_const(skipped))
            if (s.startsWith(QLatin1String("Slide 1"))) ++notes;
        QVERIFY2(notes >= 3, qPrintable(skipped.join('\n')));   // the chart, the animation, the transition
        // The shape number of the frame is not given to anything else on the slide.
        QDomDocument dom;
        QVERIFY(office::parseXml(slide.toUtf8(), &dom));
        QStringList ids;
        QVector<QDomElement> stack{dom.documentElement()};
        while (!stack.isEmpty()) {
            const QDomElement el = stack.takeLast();
            if (el.localName() == QLatin1String("cNvPr")) ids << el.attribute(QStringLiteral("id"));
            for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
        }
        QVERIFY(ids.contains(QStringLiteral("8")));
        QCOMPARE(int(QSet<QString>(ids.cbegin(), ids.cend()).size()), int(ids.size()));

        // An animation that aims at a frame that is kept, by the number the frame keeps, stays.
        QJsonObject props = d->pages[0]->props;
        props[QStringLiteral("timingXml")] = QStringLiteral("<p:timing xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" nodeType=\"tmRoot\"><p:childTnLst>"
                                                           "<p:set><p:cBhvr><p:cTn id=\"2\"/><p:tgtEl><p:spTgt spid=\"8\"/></p:tgtEl></p:cBhvr></p:set></p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>");
        props.remove(QStringLiteral("transitionXml"));
        d->pages[0]->props = props;
        const QByteArray kept = deck::exportPresentation(*d, &err);
        QVERIFY2(!kept.isEmpty(), qPrintable(err));
        Exported k;
        QVERIFY2(unpack(kept, &k, &err), qPrintable(err));
        QVERIFY(part(k, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<p:spTgt spid=\"8\"/>")));
    }

    // A shape read from a file is written with the number it had, so an animation
    // that aims at it stays; one that is gone, and the new and copied ones, take
    // no number an animation could still aim at.
    void exportKeepsAnimationsOnShapesThatKeptTheirNumbers()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QMap<QString, int> numbers;
        for (const auto &it : d->pages[0]->items) numbers.insert(it->name, it->sourceId);
        QCOMPARE(numbers.value(QStringLiteral("TextBox 4")), 5);
        QCOMPARE(numbers.value(QStringLiteral("Rectangle 5")), 6);
        auto timingAt = [](const QStringList &targets) {
            QString sets;
            for (const QString &t : targets)
                sets += QStringLiteral("<p:set><p:cBhvr><p:cTn id=\"%1\"/><p:tgtEl><p:spTgt spid=\"%1\"/></p:tgtEl></p:cBhvr></p:set>").arg(t);
            return QStringLiteral("<p:timing xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" nodeType=\"tmRoot\"><p:childTnLst>")
                + sets + QStringLiteral("</p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>");
        };
        auto numbersIn = [](const QString &slide) {
            QMap<QString, QString> byName;
            QDomDocument dom;
            if (!office::parseXml(slide.toUtf8(), &dom)) return byName;
            QVector<QDomElement> stack{dom.documentElement()};
            while (!stack.isEmpty()) {
                const QDomElement el = stack.takeLast();
                if (el.localName() == QLatin1String("cNvPr")) byName.insert(el.attribute(QStringLiteral("name")), el.attribute(QStringLiteral("id")));
                for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
            }
            return byName;
        };
        d->pages[0]->props[QStringLiteral("timingXml")] = timingAt({QStringLiteral("5"), QStringLiteral("6")});
        d->pages[0]->props.remove(QStringLiteral("transitionXml"));
        QStringList skipped;
        QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("<p:spTgt spid=\"5\"/>")) && slide.contains(QStringLiteral("<p:spTgt spid=\"6\"/>")));
        QCOMPARE(numbersIn(slide).value(QStringLiteral("TextBox 4")), QStringLiteral("5"));
        QCOMPARE(numbersIn(slide).value(QStringLiteral("Rectangle 5")), QStringLiteral("6"));
        // And once read again they still have them.
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QMap<QString, int> backNumbers;
        for (const auto &it : back->pages[0]->items) backNumbers.insert(it->name, it->sourceId);
        QCOMPARE(backNumbers.value(QStringLiteral("TextBox 4")), 5);
        QCOMPARE(backNumbers.value(QStringLiteral("Rectangle 5")), 6);

        // A shape added, and a copy of one, are written with numbers of their own.
        auto copy = d->pages[0]->items[3]->clone();
        copy->name = QStringLiteral("Copy");
        auto added = std::make_shared<ShapeItem>();
        added->name = QStringLiteral("Added");
        added->rect = QRectF(10, 10, 20, 20);
        d->pages[0]->items.push_back(copy);
        d->pages[0]->items.push_back(added);
        QVERIFY2(!(bytes = deck::exportPresentation(*d, &err)).isEmpty(), qPrintable(err));
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("<p:spTgt spid=\"5\"/>")));
        const QMap<QString, QString> ids = numbersIn(slide);
        QVERIFY(ids.value(QStringLiteral("Copy")) != QStringLiteral("5") && ids.value(QStringLiteral("Copy")) != QStringLiteral("6"));
        QVERIFY(!ids.value(QStringLiteral("Added")).isEmpty());
        QCOMPARE(int(QSet<QString>(ids.cbegin(), ids.cend()).size()), int(ids.size()));

        // With the shape an animation aims at gone, it goes with a note.
        for (auto it = d->pages[0]->items.begin(); it != d->pages[0]->items.end(); ++it)
            if ((*it)->name == QLatin1String("Rectangle 5")) {
                d->pages[0]->items.erase(it);
                break;
            }
        QVERIFY2(!(bytes = deck::exportPresentation(*d, &err, &skipped)).isEmpty(), qPrintable(err));
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY(!part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<p:timing")));
        bool noted = false;
        for (const QString &s : std::as_const(skipped)) noted |= s.startsWith(QLatin1String("Slide 1")) && s.contains(QStringLiteral("animation"));
        QVERIFY2(noted, qPrintable(skipped.join('\n')));
    }

    // A movie and a sound a slide plays are kept: the copy the file holds is
    // written back byte for byte with its relationships, one outside stays a
    // link, and the timing that plays them stays with them. Timing that plays
    // a movie or sound on a shape that has none is left out with a note: the
    // presentation program refuses such a file.
    void exportKeepsMoviesAndSounds()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        auto media = [](int id, const QString &name, const QString &nvPr) {
            return QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"%1\" name=\"%2\"><a:hlinkClick r:id=\"\" action=\"ppaction://media\"/></p:cNvPr><p:cNvPicPr/><p:nvPr>%3</p:nvPr></p:nvPicPr>"
                                  "<p:blipFill><a:blip r:embed=\"rId2\"/><a:stretch><a:fillRect/></a:stretch></p:blipFill><p:spPr>").arg(id).arg(name, nvPr)
                + xfrm(0, 0, 1828800, 1371600) + QStringLiteral("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr></p:pic>");
        };
        xml.replace(QStringLiteral("</p:spTree>"),
                    media(20, QStringLiteral("Movie"), QStringLiteral("<a:videoFile r:link=\"rId4\"/><p:extLst><p:ext uri=\"{DAA4B4D4-6D71-4841-9C94-3DE7FCFB9230}\">"
                                                                      "<p14:media xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\" r:embed=\"rId5\"/></p:ext></p:extLst>"))
                        + media(21, QStringLiteral("Sound"), QStringLiteral("<a:audioFile r:link=\"rId6\"/>")) + QStringLiteral("</p:spTree>"));
        auto plays = [](const char *kind, int id, int spid) {
            return QStringLiteral("<p:%1><p:cMediaNode vol=\"80000\"><p:cTn id=\"%2\" fill=\"hold\" display=\"0\"><p:stCondLst><p:cond delay=\"indefinite\"/></p:stCondLst></p:cTn>"
                                  "<p:tgtEl><p:spTgt spid=\"%3\"/></p:tgtEl></p:cMediaNode></p:%1>").arg(QLatin1String(kind)).arg(id).arg(spid);
        };
        xml.replace(QRegularExpression(QStringLiteral("<p:timing>.*</p:timing>")),
                    QStringLiteral("<p:timing><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"><p:childTnLst>")
                        + plays("video", 2, 20) + plays("audio", 3, 21) + QStringLiteral("</p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        const QByteArray movie("not really a movie, but kept byte for byte");
        p.data["/ppt/media/media1.mp4"] = {QStringLiteral("video/mp4"), movie};
        p.rels << Rel{"/ppt/slides/slide1.xml", "video", "/ppt/media/media1.mp4"}
               << Rel{"/ppt/slides/slide1.xml", "http://schemas.microsoft.com/office/2007/relationships/media", "/ppt/media/media1.mp4"}
               << Rel{"/ppt/slides/slide1.xml", "audio", "file:///C:/Sounds/applause.wav", true};

        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        auto named = [](const Document &doc, const QString &name) -> const PictureItem * {
            for (const auto &it : doc.pages[0]->items)
                if (it->name == name && it->type() == ItemType::Picture) return static_cast<const PictureItem *>(it.get());
            return nullptr;
        };
        const PictureItem *mv = named(*d, QStringLiteral("Movie")), *snd = named(*d, QStringLiteral("Sound"));
        QVERIFY(mv && snd);
        QCOMPARE(mv->mediaKind, QStringLiteral("video"));
        QVERIFY(d->media.contains(mv->mediaId));
        QCOMPARE(d->media.value(mv->mediaId).bytes, movie);
        QCOMPARE(d->media.value(mv->mediaId).contentType, QStringLiteral("video/mp4"));
        QCOMPARE(d->media.value(mv->mediaId).extension, QStringLiteral("mp4"));
        QCOMPARE(snd->mediaKind, QStringLiteral("audio"));
        QVERIFY(snd->mediaId.isEmpty());
        QCOMPARE(snd->mediaLink, QStringLiteral("file:///C:/Sounds/applause.wav"));

        QStringList skipped;
        QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("<a:videoFile r:link=")));
        QVERIFY(slide.contains(QStringLiteral("<p14:media xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\" r:embed=")));
        QVERIFY(slide.contains(QStringLiteral("<a:audioFile r:link=")));
        QCOMPARE(int(slide.count(QStringLiteral("action=\"ppaction://media\""))), 2);
        QVERIFY(slide.contains(QStringLiteral("<p:video>")) && slide.contains(QStringLiteral("<p:spTgt spid=\"20\"/>")));
        QVERIFY(slide.contains(QStringLiteral("<p:audio>")) && slide.contains(QStringLiteral("<p:spTgt spid=\"21\"/>")));
        QString moviePart;
        bool linked = false, copied = false;
        for (const office::Relationship &r : e.pkg.relationships(QStringLiteral("/ppt/slides/slide1.xml"))) {
            if (r.type.endsWith(QLatin1String("/relationships/video")) && !r.external) moviePart = r.target;
            if (r.type == QLatin1String("http://schemas.microsoft.com/office/2007/relationships/media")) copied = !r.external;
            if (r.type.endsWith(QLatin1String("/relationships/audio"))) linked = r.external && r.target == QLatin1String("file:///C:/Sounds/applause.wav");
        }
        QVERIFY(copied && linked);
        QVERIFY(moviePart.endsWith(QLatin1String(".mp4")));
        QCOMPARE(e.pkg.part(moviePart), movie);
        QCOMPARE(e.pkg.contentType(moviePart), QStringLiteral("video/mp4"));

        // Read again, it is the same movie.
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const PictureItem *again = named(*back, QStringLiteral("Movie"));
        QVERIFY(again && back->media.contains(again->mediaId));
        QCOMPARE(back->media.value(again->mediaId).bytes, movie);

        // A movie's picture that plays nothing any more takes the timing that plays it with it.
        for (const auto &it : d->pages[0]->items)
            if (it->name == QLatin1String("Movie")) static_cast<PictureItem &>(*it).mediaKind.clear();
        skipped.clear();
        QVERIFY2(!(bytes = deck::exportPresentation(*d, &err, &skipped)).isEmpty(), qPrintable(err));
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(!slide.contains(QStringLiteral("<p:timing")));
        QVERIFY(!slide.contains(QStringLiteral("<a:videoFile")));
        bool noted = false;
        for (const QString &n : std::as_const(skipped)) noted |= n.startsWith(QLatin1String("Slide 1")) && n.contains(QStringLiteral("movie or sound"));
        QVERIFY2(noted, qPrintable(skipped.join('\n')));
    }

    // What a person makes in the editor is written as it looks: scheme colors as
    // the presentation's own, shapes as the standard's presets, a picture used
    // twice once, lines with their ends and what they are attached to, and
    // groups with what is in them.
    void exportWritesWhatTheEditorMakes()
    {
        auto d = deck::newPresentation();
        d->colors.c[jp::Accent1] = QColor(0x12, 0x34, 0x56);
        const int at = deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("titleOnly")));
        Page &slide = *d->pages[at];
        const size_t placeholders = slide.items.size();
        auto box = std::make_shared<TextItem>();
        box->rect = QRectF(50, 150, 300, 60);
        box->storyId = d->createStory(QStringLiteral("Hello\nWorld"));
        {
            QTextCursor c(d->storyDoc(box->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat cf;
            cf.setFontPointSize(20);
            cf.setProperty(tp::ColorRefP, ColorRef::scheme(jp::Accent1).toString());
            c.mergeCharFormat(cf);
        }
        auto arrow = std::make_shared<ShapeItem>();
        arrow->rect = QRectF(400, 150, 200, 80);
        arrow->shape = QStringLiteral("rightArrow");
        arrow->adj = {0.4, 0.6};
        arrow->fill = Fill::solid(ColorRef::scheme(jp::Accent1));
        arrow->stroke = Stroke::none();
        auto circle = std::make_shared<ShapeItem>();
        circle->rect = QRectF(550, 300, 60, 60);
        circle->shape = QStringLiteral("ellipse");
        auto star = std::make_shared<ShapeItem>();
        star->rect = QRectF(400, 300, 100, 100);
        star->shape = QStringLiteral("star5");
        QImage image(4, 4, QImage::Format_RGB32);
        image.fill(Qt::green);
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        const QString imageId = d->addImage(png, QStringLiteral("png"), QString());
        auto first = std::make_shared<PictureItem>(), second = std::make_shared<PictureItem>();
        for (const auto &p : {first, second}) {
            p->imageId = imageId;
            p->imgRect = QRectF(0, 0, 100, 50);
            p->rect = QRectF(50, 300, 100, 50);
        }
        second->rect.moveTo(200, 300);
        auto line = std::make_shared<LineItem>();
        line->p1 = QPointF(350, 180);
        line->p2 = QPointF(400, 190);
        line->start = LineItem::Glue{box->id, 3};
        line->end = LineItem::Glue{circle->id, 2};
        line->stroke.endArrow = Arrow::Triangle;
        line->syncRect();
        auto group = std::make_shared<GroupItem>();
        group->children = {star, first};
        group->syncRect();
        auto table = std::make_shared<jp::TableItem>();
        table->rect = QRectF(50, 400, 200, 60);
        table->rows = 2;
        table->cols = 2;
        table->colW = {100, 100};
        table->rowH = {30, 30};
        table->cells.resize(4);
        for (jp::TableCell &cell : table->cells) {
            cell.storyId = d->createStory(QStringLiteral("cell"));
            cell.fill = Fill::solid(ColorRef::scheme(jp::Accent1, 60));
            cell.border.top = cell.border.bottom = cell.border.left = cell.border.right = Stroke::line(ColorRef::scheme(jp::Main), 1);
        }
        slide.items.push_back(table);
        slide.items.push_back(box);
        slide.items.push_back(arrow);
        slide.items.push_back(circle);
        slide.items.push_back(second);
        slide.items.push_back(line);
        slide.items.push_back(group);

        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join('\n')));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        int media = 0;
        for (const QString &name : e.pkg.partNames()) media += name.startsWith(QLatin1String("/ppt/media/"));
        QCOMPARE(media, 1);

        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const Page &page = *back->pages[at];
        QCOMPARE(page.items.size(), placeholders + 7);
        // The table comes back as a table, first, as it was written.
        QCOMPARE(page.items[placeholders]->type(), ItemType::Table);
        QVERIFY(page.props.value(QStringLiteral("unread")).toArray().isEmpty());
        const auto *text = static_cast<TextItem *>(page.items[placeholders + 1].get());
        QCOMPARE(text->storyId.isEmpty(), false);
        QCOMPARE(back->storyDoc(text->storyId)->toPlainText(), QStringLiteral("Hello\nWorld"));
        QCOMPARE(back->storyDoc(text->storyId)->begin().begin().fragment().charFormat().stringProperty(tp::ColorRefP), QStringLiteral("#123456"));
        const auto *shape = static_cast<ShapeItem *>(page.items[placeholders + 2].get());
        QCOMPARE(shape->shape, QStringLiteral("rightArrow"));
        QCOMPARE(shape->fill.color.rgbValue(), QColor(0x12, 0x34, 0x56));
        QVERIFY(shape->stroke.isNone());
        QCOMPARE(shape->adj.size(), 2);
        QVERIFY(std::abs(shape->adj[0] - 0.4) < 1e-4 && std::abs(shape->adj[1] - 0.6) < 1e-4);
        QCOMPARE(page.items[placeholders + 4]->type(), ItemType::Picture);
        const auto *connector = static_cast<LineItem *>(page.items[placeholders + 5].get());
        QVERIFY(connector->stroke.endArrow == Arrow::Triangle);
        // The connector names the numbers of the two shapes it is attached to (the importer reads each shape alone, so what it joins is not read back).
        QDomDocument dom;
        QVERIFY(office::parseXml(part(e, QStringLiteral("/ppt/slides/slide%1.xml").arg(at + 1)).toUtf8(), &dom));
        QStringList numbers;
        for (QDomElement el = office::child(office::child(office::child(dom.documentElement(), office::ns::P, QStringLiteral("cSld")), office::ns::P, QStringLiteral("spTree")),
                                            office::ns::P, QStringLiteral("sp"));
             !el.isNull(); el = el.nextSiblingElement(QStringLiteral("sp")))
            numbers << office::child(office::child(el, office::ns::P, QStringLiteral("nvSpPr")), office::ns::P, QStringLiteral("cNvPr")).attribute(QStringLiteral("id"));
        const QDomElement cxn = office::child(office::child(office::child(office::child(dom.documentElement(), office::ns::P, QStringLiteral("cSld")), office::ns::P, QStringLiteral("spTree")),
                                                            office::ns::P, QStringLiteral("cxnSp")), office::ns::P, QStringLiteral("nvCxnSpPr"));
        const QDomElement props = office::child(cxn, office::ns::P, QStringLiteral("cNvCxnSpPr"));
        const QDomElement start = office::child(props, office::ns::A, QStringLiteral("stCxn")), end = office::child(props, office::ns::A, QStringLiteral("endCxn"));
        QCOMPARE(int(numbers.size()), int(placeholders) + 3);
        QCOMPARE(start.attribute(QStringLiteral("id")), numbers[placeholders]);
        QCOMPARE(start.attribute(QStringLiteral("idx")), QStringLiteral("3"));
        QCOMPARE(end.attribute(QStringLiteral("id")), numbers[placeholders + 2]);
        QCOMPARE(end.attribute(QStringLiteral("idx")), QStringLiteral("4"));   // the bottom of an ellipse is its fifth point
        const auto *grouped = static_cast<GroupItem *>(page.items[placeholders + 6].get());
        QCOMPARE(grouped->children.size(), size_t(2));
        QCOMPARE(static_cast<ShapeItem *>(grouped->children[0].get())->shape, QStringLiteral("star5"));
        QCOMPARE(static_cast<PictureItem *>(grouped->children[1].get())->imageId, static_cast<PictureItem *>(page.items[placeholders + 4].get())->imageId);
    }

    // A connector glued to a triangle's left side and an ellipse's left is written with the standard's numbers for those sites (the triangle's second, the ellipse's third), with its ends
    // on the sites the table gives.
    void aGluedConnectorKeepsTheStandardsSiteNumbers()
    {
        auto d = deck::newPresentation();
        const int at = deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("titleOnly")));
        Page &slide = *d->pages[at];
        auto triangle = std::make_shared<ShapeItem>(), circle = std::make_shared<ShapeItem>();
        triangle->rect = QRectF(50, 150, 100, 100);
        triangle->shape = QStringLiteral("triangle");
        circle->rect = QRectF(350, 300, 80, 80);
        circle->shape = QStringLiteral("ellipse");
        auto line = std::make_shared<LineItem>();
        line->start = LineItem::Glue{triangle->id, 1};
        line->end = LineItem::Glue{circle->id, 1};
        slide.items.push_back(triangle);
        slide.items.push_back(circle);
        slide.items.push_back(line);
        d->routeConnectors();
        // The triangle's left site is the middle of its slope, (25, 50) in its box; the ellipse's is the middle of its left side.
        QVERIFY(std::abs(line->p1.x() - 75) < 1e-6 && std::abs(line->p1.y() - 200) < 1e-6);
        QVERIFY(std::abs(line->p2.x() - 350) < 1e-6 && std::abs(line->p2.y() - 340) < 1e-6);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString xml = part(e, QStringLiteral("/ppt/slides/slide%1.xml").arg(at + 1));
        QVERIFY2(xml.contains(QRegularExpression(QStringLiteral("<a:stCxn id=\"[0-9]+\" idx=\"1\"/><a:endCxn id=\"[0-9]+\" idx=\"2\"/>"))), qPrintable(xml.mid(xml.indexOf(QStringLiteral("cNvCxnSpPr")), 200)));
    }

    // Slide masters beyond the first keep their own themes, and every master and
    // layout has an id of its own.
    void exportWritesEachMastersTheme()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        auto second = std::make_shared<MasterPage>(*d->masters[0]);
        second->id = QStringLiteral("SM2");
        second->items.clear();
        second->props[QStringLiteral("themeXml")] = QString::fromLatin1(QByteArray(themeXml()).replace("Test Theme", "Second Theme").toBase64());
        auto layout = std::make_shared<MasterPage>();
        layout->id = QStringLiteral("L3");
        layout->name = QStringLiteral("Only Title");
        layout->baseId = second->id;
        layout->props[QStringLiteral("kind")] = QStringLiteral("layout");
        layout->props[QStringLiteral("type")] = QStringLiteral("cust");
        d->masters << second << layout;
        auto slide = std::make_shared<Page>();
        slide->masterId = layout->id;
        slide->notesStoryId = d->createStory();
        d->pages << slide;

        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QCOMPARE(e.pkg.relationshipsOfType(QStringLiteral("/ppt/presentation.xml"), QStringLiteral("/slideMaster")).size(), 2);
        // The first master's theme is the kept part as it was; the second's is its own; the notes master has a third.
        QCOMPARE(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), themeXml());
        QVERIFY(part(e, QStringLiteral("/ppt/theme/theme2.xml")).contains(QStringLiteral("Second Theme")));
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slideMasters/slideMaster2.xml"), QStringLiteral("/theme")), QStringList{QStringLiteral("/ppt/theme/theme2.xml")});
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slideMasters/slideMaster1.xml"), QStringLiteral("/theme")), QStringList{QStringLiteral("/ppt/theme/theme1.xml")});
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/notesMasters/notesMaster1.xml"), QStringLiteral("/theme")), QStringList{QStringLiteral("/ppt/theme/theme3.xml")});
        // The layouts: two on the first master, one on the second, numbered across.
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slideMasters/slideMaster2.xml"), QStringLiteral("/slideLayout")), QStringList{QStringLiteral("/ppt/slideLayouts/slideLayout3.xml")});
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slides/slide3.xml"), QStringLiteral("/slideLayout")), QStringList{QStringLiteral("/ppt/slideLayouts/slideLayout3.xml")});
        // A custom layout says no type; the master's ids and the layouts' ids do not repeat.
        QDomDocument layoutDom;
        QVERIFY(!rootOf(e.zip.value(QStringLiteral("ppt/slideLayouts/slideLayout3.xml")), &layoutDom).hasAttribute(QStringLiteral("type")));
        QDomDocument presDom;
        const QDomElement pres = rootOf(e.zip.value(QStringLiteral("ppt/presentation.xml")), &presDom);
        QSet<QString> ids;
        int count = 0;
        for (const QDomElement &m : office::children(office::child(pres, office::ns::P, QStringLiteral("sldMasterIdLst")))) ids << office::attr(m, QStringLiteral("id")), ++count;
        for (const QString &m : {QStringLiteral("ppt/slideMasters/slideMaster1.xml"), QStringLiteral("ppt/slideMasters/slideMaster2.xml")}) {
            QDomDocument md;
            for (const QDomElement &l : office::children(office::child(rootOf(e.zip.value(m), &md), office::ns::P, QStringLiteral("sldLayoutIdLst")))) ids << office::attr(l, QStringLiteral("id")), ++count;
        }
        QCOMPARE(count, 5);
        QCOMPARE(int(ids.size()), 5);
        // Read back: two masters, the second with its theme.
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 3);
        int masters = 0;
        for (const auto &m : back->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 2);
        QCOMPARE(back->masterFor(*back->pages[2])->name, QStringLiteral("Only Title"));
        QVERIFY(back->masters[3]->props.contains(QStringLiteral("themeXml")));
    }

    // The master's text styles say what its title and text placeholders look
    // like: sizes, the theme's fonts, bullets, and indents, level by level.
    void exportWritesTheMastersTextStyles()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QDomDocument dom;
        const QDomElement styles = office::child(rootOf(e.zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml")), &dom), office::ns::P, QStringLiteral("txStyles"));
        QStringList kinds;
        for (const QDomElement &c : office::children(styles)) kinds << c.localName();
        QCOMPARE(kinds, (QStringList{"titleStyle", "bodyStyle", "otherStyle"}));
        const QDomElement title = office::child(office::child(styles, office::ns::P, QStringLiteral("titleStyle")), office::ns::A, QStringLiteral("lvl1pPr"));
        QCOMPARE(office::attr(title, QStringLiteral("algn")), QStringLiteral("l"));
        const QDomElement titleRun = office::child(title, office::ns::A, QStringLiteral("defRPr"));
        QCOMPARE(office::attr(titleRun, QStringLiteral("sz")), QStringLiteral("4400"));
        QCOMPARE(office::attr(office::child(titleRun, office::ns::A, QStringLiteral("latin")), QStringLiteral("typeface")), QStringLiteral("+mj-lt"));
        QVERIFY(!office::child(title, office::ns::A, QStringLiteral("buNone")).isNull());
        // The text style has nine levels, none larger than the one before; the first is bulleted as the master's text is.
        const QDomElement body = office::child(styles, office::ns::P, QStringLiteral("bodyStyle"));
        QCOMPARE(int(office::children(body).size()), 9);
        const QDomElement first = office::child(body, office::ns::A, QStringLiteral("lvl1pPr"));
        QCOMPARE(office::attr(first, QStringLiteral("marL")), QStringLiteral("228600"));
        QCOMPARE(office::attr(first, QStringLiteral("indent")), QStringLiteral("-228600"));
        QCOMPARE(office::attr(office::child(first, office::ns::A, QStringLiteral("buChar")), QStringLiteral("char")), QStringLiteral("\u2022"));
        QCOMPARE(office::attr(office::child(first, office::ns::A, QStringLiteral("defRPr")), QStringLiteral("sz")), QStringLiteral("2800"));
        QStringList inLevel;
        for (const QDomElement &c : office::children(first)) inLevel << c.localName();
        QVERIFY2(inSequence(inLevel, {"lnSpc", "spcBef", "spcAft", "buClrTx", "buClr", "buSzTx", "buSzPct", "buSzPts", "buFontTx", "buFont", "buNone", "buAutoNum", "buChar", "buBlip", "tabLst", "defRPr", "extLst"}),
                 qPrintable(inLevel.join(' ')));
        int previous = 100000;
        for (const QDomElement &level : office::children(body)) {
            const int size = office::attr(office::child(level, office::ns::A, QStringLiteral("defRPr")), QStringLiteral("sz")).toInt();
            QVERIFY(size > 0 && size <= previous);
            previous = size;
        }
        // A presentation made here has no bullets in its text, and its master says so.
        auto fresh = deck::newPresentation();
        Exported n;
        QVERIFY2(unpack(deck::exportPresentation(*fresh, &err), &n, &err), qPrintable(err));
        QDomDocument freshDom;
        const QDomElement freshStyles = office::child(rootOf(n.zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml")), &freshDom), office::ns::P, QStringLiteral("txStyles"));
        for (const QDomElement &level : office::children(office::child(freshStyles, office::ns::P, QStringLiteral("bodyStyle")))) {
            QVERIFY(!office::child(level, office::ns::A, QStringLiteral("buNone")).isNull());
            QVERIFY(office::child(level, office::ns::A, QStringLiteral("buChar")).isNull());
        }
    }

    // The theme: kept as the file had it, made from the presentation's colors
    // and fonts when it came from none, and as the presentation says when the
    // colors changed after the file was read.
    void exportWritesTheTheme()
    {
        QString err;
        auto d = deck::newPresentation();
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        office::Theme theme;
        QVERIFY2(office::readTheme(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), &theme, &err), qPrintable(err));
        const deck::DeckTheme was = deck::themeOf(*d);
        for (int i = 0; i < 12; ++i) QCOMPARE(theme.colors[i], was.colors[i]);
        QCOMPARE(theme.majorLatin, was.headingFont);
        QCOMPARE(theme.minorLatin, was.bodyFont);
        QCOMPARE(theme.name, was.name);

        auto imported = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(imported, qPrintable(err));
        Exported kept;
        QVERIFY2(unpack(deck::exportPresentation(*imported, &err), &kept, &err), qPrintable(err));
        QCOMPARE(kept.zip.value(QStringLiteral("ppt/theme/theme1.xml")), themeXml());
        // A color changed since the file was read is written, not the part kept from before.
        deck::DeckTheme changed = deck::themeOf(*imported);
        changed.colors[4] = QColor(0x00, 0x99, 0x66);
        changed.bodyFont = QStringLiteral("Calibri");
        deck::setTheme(*imported, changed);
        Exported fresh;
        QVERIFY2(unpack(deck::exportPresentation(*imported, &err), &fresh, &err), qPrintable(err));
        QVERIFY2(office::readTheme(fresh.zip.value(QStringLiteral("ppt/theme/theme1.xml")), &theme, &err), qPrintable(err));
        QCOMPARE(theme.colors[4], QColor(0x00, 0x99, 0x66));
        QCOMPARE(theme.minorLatin, QStringLiteral("Calibri"));
        QCOMPARE(theme.majorLatin, QStringLiteral("Georgia"));
        QVERIFY2(packageProblem(fresh).isEmpty(), qPrintable(packageProblem(fresh)));
    }

    // An object's name and its hidden mark are kept in the file, and a hidden object stays in it (it was left out when the file was opened).
    void pptxKeepsObjectNamesAndHiddenMarks()
    {
        auto d = deck::newPresentation();
        auto shape = [&](const QString &name, double x, bool hidden) {
            auto s = std::make_shared<jp::ShapeItem>();
            s->name = name;
            s->hidden = hidden;
            s->rect = QRectF(x, 200, 80, 60);
            s->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(0x20, 0x40, 0x80)));
            d->pages[0]->items.push_back(s);
        };
        shape(QStringLiteral("Seen box"), 20, false);
        shape(QStringLiteral("Secret <box> & more"), 140, true);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::Item *seen = nullptr, *secret = nullptr;
        for (const jp::ItemPtr &it : back->pages[0]->items) {
            if (it->name == QLatin1String("Seen box")) seen = it.get();
            if (it->name == QLatin1String("Secret <box> & more")) secret = it.get();
        }
        QVERIFY(seen);
        QVERIFY(secret);
        QVERIFY(!seen->hidden);
        QVERIFY(secret->hidden);
        // (Saved again, the same.)
        auto third = deck::importPresentation(deck::exportPresentation(*back, &err), &err);
        QVERIFY2(third, qPrintable(err));
        int hiddenCount = 0;
        for (const jp::ItemPtr &it : third->pages[0]->items) hiddenCount += it->hidden;
        QCOMPARE(hiddenCount, 1);
    }

    // Picture Styles are kept in .pptx: each style of the gallery (its shape, border, shadow, reflection, glow, soft edges, bevel, and turn in space) reads back as it was, and read
    // and written again it is the same. In .odp the border, shadow, glow, soft edges, reflection, bevel, and turn come back too.
    void everyPictureStyleIsKeptInPptxAndOdp()
    {
        QImage image(60, 40, QImage::Format_RGB32);
        image.fill(Qt::darkCyan);
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        image.save(&buf, "PNG");
        auto picOf = [](const jp::Document &d) -> const jp::PictureItem * {
            for (const jp::ItemPtr &it : d.pages[0]->items)
                if (it->type() == jp::ItemType::Picture) return static_cast<const jp::PictureItem *>(it.get());
            return nullptr;
        };
        for (const bool odp : {false, true}) {
            for (int k = 0; k < jp::pictureStyleCount(); ++k) {
                auto d = deck::newPresentation();
                d->pages[0]->items.clear();
                auto pic = std::make_shared<jp::PictureItem>();
                pic->imageId = d->addImage(png, QStringLiteral("png"));
                pic->rect = QRectF(100, 100, 240, 160);
                pic->imgRect = QRectF(0, 0, 240, 160);
                jp::applyPictureStyle(pic.get(), k);
                d->pages[0]->items.push_back(pic);
                QString err;
                const QByteArray bytes = odp ? deck::exportOdp(*d, false, &err) : deck::exportPresentation(*d, &err);
                QVERIFY2(!bytes.isEmpty(), qPrintable(err));
                auto back = odp ? deck::importOdp(bytes, &err) : deck::importPresentation(bytes, &err);
                QVERIFY2(back, qPrintable(err));
                const jp::PictureItem *got = picOf(*back);
                const QByteArray what = (odp ? "odp " : "pptx ") + jp::pictureStyleId(k).toLatin1();
                QVERIFY2(got, what.constData());
                if (!odp) QCOMPARE(got->maskShape, pic->maskShape);
                // (A picture cut to a shape is saved in .odp as the picture it shows, effects and all: its look is in its pixels.)
                if (odp && pic->maskShape != QLatin1String("rect")) continue;
                QCOMPARE(got->stroke.isNone(), pic->stroke.isNone());
                if (!pic->stroke.isNone()) {
                    QVERIFY2(std::abs(got->stroke.width - pic->stroke.width) < 0.01, what.constData());
                    if (!odp) QCOMPARE(int(got->stroke.compound), int(pic->stroke.compound));
                }
                QCOMPARE(got->fx.shadow.on, pic->fx.shadow.on);
                QCOMPARE(got->fx.glow.on, pic->fx.glow.on);
                QVERIFY2(std::abs(got->fx.softEdge - pic->fx.softEdge) < 0.01, what.constData());
                QCOMPARE(got->fx.reflection.on, pic->fx.reflection.on);
                QCOMPARE(got->fx.bevel.type, pic->fx.bevel.type);
                QVERIFY2(std::abs(got->fx.bevel.width - pic->fx.bevel.width) < 0.01, what.constData());
                QVERIFY2(std::abs(got->fx.rot3d.y - pic->fx.rot3d.y) < 0.01 && std::abs(got->fx.rot3d.x - pic->fx.rot3d.x) < 0.01, what.constData());
                QVERIFY2(std::abs(got->fx.rot3d.perspective - pic->fx.rot3d.perspective) < 0.01, what.constData());
                // Written again, the same.
                const QByteArray again = odp ? deck::exportOdp(*back, false, &err) : deck::exportPresentation(*back, &err);
                auto third = odp ? deck::importOdp(again, &err) : deck::importPresentation(again, &err);
                QVERIFY2(third, qPrintable(err));
                QCOMPARE(picOf(*third)->fx.toJson(), got->fx.toJson());
            }
        }
    }

    // A bevel and a turn in space read from an .odp that holds a number that is none (nan, inf) or too large are not taken as they are: a value that is no number leaves the effect off, and a large
    // one is held to what the effects may have.
    void odpBevelAndTurnNumbersAreCheckedWhenRead()
    {
        QImage image(60, 40, QImage::Format_RGB32);
        image.fill(Qt::darkCyan);
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        image.save(&buf, "PNG");
        auto d = deck::newPresentation();
        d->pages[0]->items.clear();
        auto pic = std::make_shared<jp::PictureItem>();
        pic->imageId = d->addImage(png, QStringLiteral("png"));
        pic->rect = QRectF(100, 100, 240, 160);
        pic->imgRect = QRectF(0, 0, 240, 160);
        pic->fx.bevel.type = 3;
        pic->fx.bevel.width = pic->fx.bevel.height = 6;
        pic->fx.rot3d.y = 20;
        pic->fx.rot3d.perspective = 30;
        d->pages[0]->items.push_back(pic);
        QString err;
        const QByteArray odp = deck::exportOdp(*d, false, &err);
        QVERIFY2(!odp.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(odp, parts));
        QVERIFY(parts.value(QStringLiteral("content.xml")).contains("jo:bevel=\"3 6 6\""));
        auto read = [&](const QByteArray &bevel, const QByteArray &turn) {
            jp::ZipWriter out;
            for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
                QByteArray data = it.value();
                if (it.key() == QLatin1String("content.xml")) {
                    data.replace("jo:bevel=\"3 6 6\"", "jo:bevel=\"" + bevel + "\"");
                    data.replace(QByteArray("jo:rot3d=\"0 20 30\""), "jo:rot3d=\"" + turn + "\"");
                }
                // (The first entry is the type's, stored as it was.)
                out.add(it.key(), data, it.key() != QLatin1String("mimetype"));
            }
            QString e;
            auto back = deck::importOdp(out.finish(), &e);
            if (!back) return QPair<jp::Effects, bool>{jp::Effects(), false};
            for (const jp::ItemPtr &it : back->pages[0]->items)
                if (it->type() == jp::ItemType::Picture) return QPair<jp::Effects, bool>{it->fx, true};
            return QPair<jp::Effects, bool>{jp::Effects(), false};
        };
        const auto good = read("3 6 6", "0 20 30");
        QVERIFY(good.second && good.first.bevel.type == 3 && good.first.rot3d.y == 20);
        for (const char *bad : {"nan 6 6", "3 nan 6", "3 6 inf", "inf 6 6"}) {
            const auto r = read(bad, "0 20 30");
            QVERIFY2(r.second && r.first.bevel.type == 0 && r.first.bevel.width == 6 + 0 * 1 && std::isfinite(r.first.bevel.width), bad);
        }
        for (const char *bad : {"nan 20 30", "0 inf 30", "0 20 nan", "-inf 0 0"}) {
            const auto r = read("3 6 6", bad);
            QVERIFY2(r.second && r.first.rot3d.x == 0 && r.first.rot3d.y == 0 && std::isfinite(r.first.rot3d.perspective), bad);
        }
        const auto huge = read("99 1e30 -1e30", "1e30 -1e30 1e30");
        QVERIFY(huge.second);
        QVERIFY(huge.first.bevel.type == 0 || (huge.first.bevel.type <= 12 && huge.first.bevel.width <= 100 && huge.first.bevel.height >= 0));
        QVERIFY(std::abs(huge.first.rot3d.x) <= 180 && std::abs(huge.first.rot3d.y) <= 180 && huge.first.rot3d.perspective <= 100);
    }

    // A name with control characters (and a very long one) is written as XML a file can hold: the characters are left out, for a shape, a table, and in .odp too.
    void objectNamesWithControlCharactersAreWrittenAsValidXml()
    {
        auto d = deck::newPresentation();
        d->pages[0]->items.clear();
        auto shape = std::make_shared<jp::ShapeItem>();
        shape->name = QStringLiteral("Sh\x01\x0b" "ape");
        shape->altText = QStringLiteral("al\x02t");
        shape->rect = QRectF(10, 10, 80, 40);
        d->pages[0]->items.push_back(shape);
        auto table = std::make_shared<jp::TableItem>();
        table->rows = 1;
        table->cols = 1;
        table->colW = {100};
        table->rowH = {30};
        table->cells.resize(1);
        table->cells[0].storyId = d->createStory();
        table->name = QStringLiteral("Ta\x03" "ble");
        table->altText = QStringLiteral("ta\x04" "ble alt");
        table->rect = QRectF(10, 100, 100, 30);
        d->pages[0]->items.push_back(table);
        for (const bool odp : {false, true}) {
            QString err;
            const QByteArray bytes = odp ? deck::exportOdp(*d, false, &err) : deck::exportPresentation(*d, &err);
            QVERIFY2(!bytes.isEmpty(), qPrintable(err));
            auto back = odp ? deck::importOdp(bytes, &err) : deck::importPresentation(bytes, &err);
            QVERIFY2(back, qPrintable(err));
            QStringList names;
            for (const jp::ItemPtr &it : back->pages[0]->items) names << it->name;
            QVERIFY2(names.contains(QStringLiteral("Shape")), qPrintable(names.join(QLatin1Char('|'))));
            QVERIFY2(names.contains(QStringLiteral("Table")) || odp, qPrintable(names.join(QLatin1Char('|'))));
        }
    }

    // The lists the Bullets, Numbering, and Multilevel List menus make are kept in .pptx and .odp: a bullet with its character, font, and color; a counting style with its punctuation and start; and a level for each item.
    void listsOfTheMenusAreKeptInPptxAndOdp()
    {
        using jp::ListLevelLook;
        struct Case {
            ListLevelLook look;
            int level;
        };
        ListLevelLook bullet;
        bullet.text = QStringLiteral("◆");
        bullet.font = QStringLiteral("Arial");
        bullet.color = QColor(0xC0, 0, 0);
        ListLevelLook number;
        number.bullet = false;
        number.numFmt = QStringLiteral("upperRoman");
        number.text = QStringLiteral("(%1)");
        number.start = 3;
        ListLevelLook letters;
        letters.bullet = false;
        letters.numFmt = QStringLiteral("lowerLetter");
        letters.text = QStringLiteral("%1)");
        const QVector<Case> cases = {{bullet, 0}, {number, 0}, {letters, 1}};
        for (const bool odp : {false, true}) {
            auto d = deck::newPresentation();
            auto box = std::make_shared<jp::TextItem>();
            box->rect = QRectF(50, 100, 400, 300);
            box->storyId = d->createStory(QStringLiteral("one\ntwo\nthree"));
            d->pages[0]->items.push_back(box);
            QTextDocument *story = d->storyDoc(box->storyId);
            QVERIFY(story);
            for (int i = 0; i < cases.size(); ++i) {
                QTextCursor c(story->findBlockByNumber(i));
                QTextListFormat lf = jp::listFormatOf(cases[i].look);
                lf.setProperty(jp::tp::ListLevel, cases[i].level);
                c.createList(lf);
            }
            QString err;
            const QByteArray bytes = odp ? deck::exportOdp(*d, false, &err) : deck::exportPresentation(*d, &err);
            QVERIFY2(!bytes.isEmpty(), qPrintable(err));
            auto back = odp ? deck::importOdp(bytes, &err) : deck::importPresentation(bytes, &err);
            QVERIFY2(back, qPrintable(err));
            const QTextDocument *after = nullptr;
            for (const auto &it : back->pages[0]->items)
                if (it->type() == ItemType::Text && back->storyDoc(static_cast<const jp::TextItem *>(it.get())->storyId)->toPlainText().contains(QStringLiteral("one"))) after = back->storyDoc(static_cast<const jp::TextItem *>(it.get())->storyId);
            QVERIFY(after);
            const QByteArray name = odp ? "odp" : "pptx";
            for (int i = 0; i < cases.size(); ++i) {
                const QTextList *list = after->findBlockByNumber(i).textList();
                QVERIFY2(list, name.constData());
                const ListLevelLook got = jp::listLookOf(list->format());
                QCOMPARE(got.bullet, cases[i].look.bullet);
                if (cases[i].look.bullet) {
                    QCOMPARE(got.text, cases[i].look.text);
                    QCOMPARE(got.font, cases[i].look.font);
                    QCOMPARE(list->format().stringProperty(jp::tp::BulletColor).toUpper(), jp::ColorRef::rgb(cases[i].look.color).toString().toUpper());
                } else {
                    QCOMPARE(got.numFmt, cases[i].look.numFmt);
                    QCOMPARE(got.text, cases[i].look.text);
                    QCOMPARE(got.start, cases[i].look.start);
                }
                QCOMPARE(list->format().intProperty(jp::tp::ListLevel), cases[i].level);
            }
        }
    }

    // ---- Notes Master and Handout Master layouts (November 15, 2026)

    // The places of the notes master and handout master, the page's background, how many slides a handout holds, and whether it lies across are in the file and come back; a file with
    // none has the layout printing always had.
    void notesMasterAndHandoutMasterLayoutsAreKeptInTheFile()
    {
        auto d = deck::newPresentation();
        const QSizeF page(540, 720);
        deck::NotesMaster nm = deck::notesMasterOf(*d, page);
        nm.custom = true;
        nm.slideImage = QRectF(0.05, 0.1, 0.5, 0.25);
        nm.body = QRectF(0.05, 0.5, 0.9, 0.4);
        nm.header = QRectF(0.0, 0.0, 0.3, 0.05);
        nm.date = QRectF(0.6, 0.0, 0.3, 0.05);
        nm.footer = QRectF(0.0, 0.93, 0.3, 0.05);
        nm.number = QRectF(0.6, 0.93, 0.3, 0.05);
        nm.background = QColor(0xDD, 0xEE, 0xFF);
        deck::setNotesMaster(*d, nm);
        deck::HandoutMaster hm = deck::handoutMasterOf(*d, page);
        hm.custom = true;
        hm.landscape = true;
        hm.area = QRectF(0.1, 0.12, 0.8, 0.76);
        hm.header = QRectF(0.02, 0.02, 0.4, 0.05);
        hm.background = QColor(0xFF, 0xEE, 0xDD);
        hm.perPage = 4;
        deck::setHandoutMaster(*d, hm);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QString why;
        QVERIFY2(unpack(bytes, &e, &why), qPrintable(why));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString notesXml = QString::fromUtf8(e.zip.value(QStringLiteral("ppt/notesMasters/notesMaster1.xml")));
        QVERIFY2(notesXml.contains(QLatin1String("<a:srgbClr val=\"DDEEFF\"/>")), qPrintable(notesXml.left(600)));
        QVERIFY2(notesXml.contains(QStringLiteral("<a:off x=\"%1\" y=\"%2\"/>").arg(qRound64(0.05 * 6858000)).arg(qRound64(0.5 * 9144000))), "the notes' place");
        const QString handoutXml = QString::fromUtf8(e.zip.value(QStringLiteral("ppt/handoutMasters/handoutMaster1.xml")));
        QVERIFY2(handoutXml.contains(QLatin1String("FFEEDD")) && handoutXml.contains(QLatin1String("landscape=\"1\"")), qPrintable(handoutXml.left(600)));
        QVERIFY2(part(e, QStringLiteral("/ppt/presProps.xml")).contains(QLatin1String("prnWhat=\"handouts4\"")), qPrintable(part(e, QStringLiteral("/ppt/presProps.xml"))));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(deck::hasNotesMaster(*back) && deck::hasHandoutMaster(*back));
        const deck::NotesMaster nb = deck::notesMasterOf(*back, page);
        for (deck::NotesPart part : {deck::NotesPart::Body, deck::NotesPart::Header, deck::NotesPart::Date, deck::NotesPart::Footer, deck::NotesPart::Number})
            QVERIFY2(deck::nearly(nb.part(part), nm.part(part), 0.002), qPrintable(QString::number(int(part))));
        // (the slide's picture is in the slide's shape inside its place: it is the picture that comes back)
        const QRectF imageWas = deck::notesPageGeometry(nm, page, d->pageSize()).slideImage, imageBack = deck::notesPageGeometry(nb, page, back->pageSize()).slideImage;
        QVERIFY2(qAbs(imageWas.left() - imageBack.left()) < 0.5 && qAbs(imageWas.top() - imageBack.top()) < 0.5 && qAbs(imageWas.width() - imageBack.width()) < 0.5, "the slide picture");
        QCOMPARE(nb.background, nm.background);
        const deck::HandoutMaster hb = deck::handoutMasterOf(*back, page);
        QVERIFY(hb.landscape && hb.perPage == 4);
        QCOMPARE(hb.background, hm.background);
        QVERIFY(deck::nearly(hb.area, hm.area, 0.0001) && deck::nearly(hb.header, hm.header, 0.002));
        // Written again, the same file's layout: nothing drifts.
        Exported again;
        QVERIFY2(unpack(deck::exportPresentation(*back, &err), &again, &why), qPrintable(why));
        QCOMPARE(part(again, QStringLiteral("/ppt/handoutMasters/handoutMaster1.xml")), part(e, QStringLiteral("/ppt/handoutMasters/handoutMaster1.xml")));

        // A presentation with no layout of its own has none after, and one that only chose its header and footer has the standard's places, which are the same as none.
        auto plain = deck::newPresentation();
        deck::PrintHeaderFooter hf;
        hf.header = true;
        hf.headerText = QStringLiteral("Head");
        deck::setPrintHeaderFooter(*plain, deck::PrintedPages::Handouts, hf);
        deck::setPrintHeaderFooter(*plain, deck::PrintedPages::Notes, hf);
        auto plainBack = deck::importPresentation(deck::exportPresentation(*plain, &err), &err);
        QVERIFY2(plainBack, qPrintable(err));
        QVERIFY(!deck::hasNotesMaster(*plainBack) && !deck::hasHandoutMaster(*plainBack) && !deck::hasHandoutPerPage(*plainBack));
        QVERIFY(!deck::notesMasterOf(*plainBack, QSizeF(612, 792)).custom);

        // A master the file had keeps all it had; the place of the header moves when the person moves it, and the shape of its own stays.
        QString custom = QString::fromUtf8(e.zip.value(QStringLiteral("ppt/handoutMasters/handoutMaster1.xml")));
        custom.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"99\" name=\"OurOwnShape\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr><a:xfrm><a:off x=\"100\" y=\"100\"/><a:ext cx=\"500000\" cy=\"500000\"/></a:xfrm>"
                                                                  "<a:prstGeom prst=\"ellipse\"><a:avLst/></a:prstGeom></p:spPr></p:sp></p:spTree>"));
        ZipWriter zip;
        for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it) zip.add(it.key(), it.key() == QLatin1String("ppt/handoutMasters/handoutMaster1.xml") ? custom.toUtf8() : it.value(), true);
        auto read = deck::importPresentation(zip.finish(), &err);
        QVERIFY2(read, qPrintable(err));
        Exported same;
        QVERIFY2(unpack(deck::exportPresentation(*read, &err), &same, &why), qPrintable(why));
        QCOMPARE(part(same, QStringLiteral("/ppt/handoutMasters/handoutMaster1.xml")).contains(QLatin1String("OurOwnShape")), true);
        deck::HandoutMaster moved = deck::handoutMasterOf(*read, page);
        moved.header = QRectF(0.3, 0.3, 0.2, 0.05);
        moved.background = QColor(1, 2, 3);
        deck::setHandoutMaster(*read, moved);
        Exported changed;
        QVERIFY2(unpack(deck::exportPresentation(*read, &err), &changed, &why), qPrintable(why));
        QVERIFY2(packageProblem(changed).isEmpty(), qPrintable(packageProblem(changed)));
        const QString changedXml = part(changed, QStringLiteral("/ppt/handoutMasters/handoutMaster1.xml"));
        QVERIFY(changedXml.contains(QLatin1String("OurOwnShape")) && changedXml.contains(QLatin1String("010203")) && !changedXml.contains(QLatin1String("FFEEDD")));
        QVERIFY(changedXml.contains(QStringLiteral("x=\"%1\"").arg(qRound64(0.3 * 6858000))));
        auto movedBack = deck::importPresentation(deck::exportPresentation(*read, &err), &err);
        QVERIFY(movedBack && deck::nearly(deck::handoutMasterOf(*movedBack, page).header, moved.header, 0.002));
    }

    // A master that lies about its places, its size, or its count is read within limits: places inside the page, no more than 64 shapes looked at, no count but the six.
    void hostileNotesAndHandoutMastersAreBounded()
    {
        auto d = deck::newPresentation();
        deck::PrintHeaderFooter hf;
        hf.header = true;
        deck::setPrintHeaderFooter(*d, deck::PrintedPages::Notes, hf);
        deck::setPrintHeaderFooter(*d, deck::PrintedPages::Handouts, hf);
        QString err;
        Exported e;
        QString why;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &why), qPrintable(why));
        QString notes = QString::fromUtf8(e.zip.value(QStringLiteral("ppt/notesMasters/notesMaster1.xml")));
        QString handout = QString::fromUtf8(e.zip.value(QStringLiteral("ppt/handoutMasters/handoutMaster1.xml")));
        // Places far outside the page, negative, and too big for any number; and thousands of shapes before the places that count.
        QString crowd;
        for (int i = 0; i < 3000; ++i) crowd += QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"x\"/><p:cNvSpPr/><p:nvPr><p:ph type=\"hdr\" idx=\"%1\"/></p:nvPr></p:nvSpPr><p:spPr><a:xfrm><a:off x=\"5\" y=\"5\"/><a:ext cx=\"100\" cy=\"100\"/></a:xfrm></p:spPr></p:sp>").arg(1000 + i);
        const QRegularExpression offs(QStringLiteral("<a:off x=\"\\d+\" y=\"\\d+\"/><a:ext cx=\"\\d+\" cy=\"\\d+\"/>"));
        QString wild = notes;
        wild.replace(QStringLiteral("<p:spTree>"), QStringLiteral("<p:spTree>") + crowd);
        wild.replace(offs, QStringLiteral("<a:off x=\"-9000000000000\" y=\"99999999999999999999\"/><a:ext cx=\"9223372036854775807\" cy=\"-5\"/>"));
        QString wildHandout = handout;
        wildHandout.replace(offs, QStringLiteral("<a:off x=\"1e9\" y=\"NaN\"/><a:ext cx=\"0\" cy=\"0\"/>"));
        wildHandout.replace(QStringLiteral("<p:spTree>"), QStringLiteral("<p:spTree>") + crowd);
        wildHandout.replace(QStringLiteral("</p:handoutMaster>"), QStringLiteral("<p:extLst><p:ext uri=\"{7B0F4F1E-5E7A-4C55-9D3B-2F6E1A9C0D11}\"><jo:handout xmlns:jo=\"http://jeffoffice.com/ns/deck\" landscape=\"1\" area=\"nan inf -1 1e300\"/></p:ext></p:extLst></p:handoutMaster>"));
        QString props = part(e, QStringLiteral("/ppt/presProps.xml"));
        props.replace(QRegularExpression(QStringLiteral("<p:presentationPr([^>]*)/>")), QStringLiteral("<p:presentationPr\\1><p:prnPr prnWhat=\"handouts99999\"/></p:presentationPr>"));
        ZipWriter zip;
        for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it) {
            const QString key = it.key();
            const QByteArray data = key == QLatin1String("ppt/notesMasters/notesMaster1.xml") ? wild.toUtf8() : key == QLatin1String("ppt/handoutMasters/handoutMaster1.xml") ? wildHandout.toUtf8()
                                    : key == QLatin1String("ppt/presProps.xml") ? props.toUtf8() : it.value();
            zip.add(key, data, true);
        }
        QElapsedTimer timer;
        timer.start();
        auto read = deck::importPresentation(zip.finish(), &err);
        QVERIFY2(read, qPrintable(err));
        QVERIFY2(timer.elapsed() < 20000, "the crowd of shapes costs bounded work");
        // The cap is what bounds it: of 3,000 shapes in each master, no more than kMaxMasterShapes are looked at (and that many were, so the cap was reached).
        QCOMPARE(deck::pptxImportWorkForTest().masterShapesRead, qint64(deck::kMaxMasterShapes));
        const QSizeF page(540, 720);
        for (const QRectF &r : {deck::notesMasterOf(*read, page).header, deck::notesMasterOf(*read, page).body, deck::notesMasterOf(*read, page).slideImage, deck::handoutMasterOf(*read, page).area,
                                deck::handoutMasterOf(*read, page).header}) {
            QVERIFY2(r.left() >= 0 && r.top() >= 0 && r.right() <= 1.0000001 && r.bottom() <= 1.0000001 && r.width() >= 0.0049 && r.height() >= 0.0049, qPrintable(QStringLiteral("%1,%2 %3x%4").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height())));
        }
        QVERIFY(deck::isHandoutCount(deck::handoutMasterOf(*read, page).perPage));
        QVERIFY(!deck::hasHandoutPerPage(*read));
    }

    // Reset Layout on a handout master a file had puts the standard's places back for good: what the file had is let go, so saving and opening again reads the standard's places.
    void resettingAForeignHandoutMasterSticks()
    {
        auto d = deck::newPresentation();
        deck::PrintHeaderFooter hf;
        hf.header = true;
        hf.headerText = QStringLiteral("Head");
        deck::setPrintHeaderFooter(*d, deck::PrintedPages::Handouts, hf);
        QString err;
        Exported first;
        QString why;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &first, &why), qPrintable(why));
        const QString name = QStringLiteral("ppt/handoutMasters/handoutMaster1.xml");
        QString foreign = QString::fromUtf8(first.zip.value(name));
        foreign.replace(QRegularExpression(QStringLiteral("<p:bg>.*</p:bg>")), QStringLiteral("<p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg>"));
        foreign.replace(QStringLiteral("<a:off x=\"0\" y=\"0\"/><a:ext cx=\"2971800\""), QStringLiteral("<a:off x=\"1000000\" y=\"2000000\"/><a:ext cx=\"1500000\""));
        ZipWriter zip;
        for (auto it = first.zip.cbegin(); it != first.zip.cend(); ++it) zip.add(it.key(), it.key() == name ? foreign.toUtf8() : it.value(), true);
        auto read = deck::importPresentation(zip.finish(), &err);
        QVERIFY2(read, qPrintable(err));
        QVERIFY(deck::hasHandoutMaster(*read));
        QVERIFY(!deck::keptHandoutMaster(*read).isEmpty());
        deck::clearHandoutMaster(*read);
        QVERIFY(!deck::hasHandoutMaster(*read) && deck::keptHandoutMaster(*read).isEmpty());
        QVERIFY(!read->appData.contains(QStringLiteral("handoutMasterReadLayout")));
        auto again = deck::importPresentation(deck::exportPresentation(*read, &err), &err);
        QVERIFY2(again, qPrintable(err));
        QVERIFY2(!deck::hasHandoutMaster(*again), "the standard's places");
        Exported out;
        QVERIFY2(unpack(deck::exportPresentation(*read, &err), &out, &why), qPrintable(why));
        QVERIFY(!part(out, QStringLiteral("/") + name).contains(QLatin1String("112233")));
        // (the header text the person chose is still there)
        QCOMPARE(deck::printHeaderFooter(*again, deck::PrintedPages::Handouts).headerText, QStringLiteral("Head"));
    }

    // A handout master written by hand, as another program may write one (an alternate-content block, an extension list with more than ours, and an extension nobody here knows):
    // read and written with no change it is the same bytes; changed once, everything it had that we do not know is still there.
    void aForeignHandoutMasterKeepsWhatItHas()
    {
        const QString xml = QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<p:handoutMaster xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
            "xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:x=\"urn:example:unknown\">"
            "<p:cSld><p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"EEEEEE\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg><p:spTree>"
            "<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>"
            "<p:sp><p:nvSpPr><p:cNvPr id=\"2\" name=\"Header Placeholder 1\"/><p:cNvSpPr/><p:nvPr><p:ph type=\"hdr\" sz=\"quarter\"/></p:nvPr></p:nvSpPr>"
            "<p:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"2971800\" cy=\"458788\"/></a:xfrm></p:spPr><p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:endParaRPr lang=\"en-US\"/></a:p></p:txBody></p:sp>"
            "<mc:AlternateContent><mc:Choice Requires=\"x\"><p:sp><p:nvSpPr><p:cNvPr id=\"50\" name=\"ChoiceShape\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr/></p:sp></mc:Choice>"
            "<mc:Fallback><p:sp><p:nvSpPr><p:cNvPr id=\"51\" name=\"FallbackShape\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr/></p:sp></mc:Fallback></mc:AlternateContent>"
            "</p:spTree><p:extLst><p:ext uri=\"{AAAAAAAA-0000-4000-8000-000000000001}\"><x:thing x:keep=\"yes\"/></p:ext>"
            "<p:ext uri=\"{AAAAAAAA-0000-4000-8000-000000000002}\"><x:other/></p:ext></p:extLst></p:cSld>"
            "<p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>"
            "<p:hf hdr=\"1\" ftr=\"0\" dt=\"0\" sldNum=\"1\"/>"
            "<p:extLst><p:ext uri=\"{BBBBBBBB-0000-4000-8000-000000000003}\"><x:handoutOnly/></p:ext></p:extLst></p:handoutMaster>");
        auto d = deck::newPresentation();
        deck::PrintHeaderFooter hf;
        hf.header = true;
        deck::setPrintHeaderFooter(*d, deck::PrintedPages::Handouts, hf);
        QString err, why;
        Exported base;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &base, &why), qPrintable(why));
        const QString name = QStringLiteral("ppt/handoutMasters/handoutMaster1.xml");
        ZipWriter zip;
        for (auto it = base.zip.cbegin(); it != base.zip.cend(); ++it) zip.add(it.key(), it.key() == name ? xml.toUtf8() : it.value(), true);
        const QByteArray file = zip.finish();
        auto read = deck::importPresentation(file, &err);
        QVERIFY2(read, qPrintable(err));
        Exported same;
        QVERIFY2(unpack(deck::exportPresentation(*read, &err), &same, &why), qPrintable(why));
        QCOMPARE(same.zip.value(name), xml.toUtf8());
        // Edited once: the header moves, and the handouts lie across.
        deck::HandoutMaster m = deck::handoutMasterOf(*read, QSizeF(540, 720));
        m.custom = true;
        m.header = QRectF(0.2, 0.2, 0.3, 0.05);
        m.landscape = true;
        deck::setHandoutMaster(*read, m);
        Exported changed;
        QVERIFY2(unpack(deck::exportPresentation(*read, &err), &changed, &why), qPrintable(why));
        QVERIFY2(packageProblem(changed).isEmpty(), qPrintable(packageProblem(changed)));
        const QString out = part(changed, QStringLiteral("/") + name);
        for (const char *kept : {"mc:AlternateContent", "mc:Fallback", "ChoiceShape", "FallbackShape", "AAAAAAAA-0000-4000-8000-000000000001", "x:keep", "AAAAAAAA-0000-4000-8000-000000000002", "BBBBBBBB-0000-4000-8000-000000000003", "x:handoutOnly", "EEEEEE"})
            QVERIFY2(out.contains(QLatin1String(kept)), kept);
        QVERIFY2(out.contains(QStringLiteral("x=\"%1\"").arg(qRound64(0.2 * 6858000))), "the header moved");
        QVERIFY2(out.contains(QLatin1String("landscape=\"1\"")), "the orientation is kept in our extension");
        auto back = deck::importPresentation(deck::exportPresentation(*read, &err), &err);
        QVERIFY(back && deck::handoutMasterOf(*back, QSizeF(540, 720)).landscape);
    }
};

JP_TEST_CLASS(DeckPptxExportTests)
#include "tst_deck_pptxexport.moc"

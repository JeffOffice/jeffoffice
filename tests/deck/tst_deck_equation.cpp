// JeffDeck's tests of Insert > Equation: an equation is an object on the slide (moved, sized, edited again in the shared editor, undone), saved as the pair of alternatives
// the format has (the math, and a picture of it for the programs that don't draw math), and opened again as the same equation; the show, the printer, and the PDF draw it.
#include "deckhelpers.h"
#include "../withdialog.h"
#include "wml/wmlmathedit.h"
#include "wml/wmlmathpath.h"

#include <QBuffer>
#include <QLineEdit>

namespace {

EquationItem *firstEquation(Document &d, int page = 0)
{
    for (const ItemPtr &it : d.pages[size_t(page)]->items)
        if (it->type() == ItemType::Equation) return static_cast<EquationItem *>(it.get());
    return nullptr;
}

int equationCount(Document &d, int page = 0)
{
    int n = 0;
    for (const ItemPtr &it : d.pages[size_t(page)]->items) n += it->type() == ItemType::Equation;
    return n;
}

// Insert > Equation with the dialog answered with `line`.
void insertLine(DeckWindow &w, const QString &line)
{
    withDialog([&] { w.act(QStringLiteral("insert.equation"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
        QCOMPARE(d->windowTitle(), QStringLiteral("Insert Equation"));
        d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->setText(line);
    });
}

// The part named by the first picture link in the fallback of the slide's pair of alternatives.
QByteArray fallbackPicture(const Exported &e, QString *slideXml = nullptr)
{
    const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
    if (slideXml) *slideXml = slide;
    const int fallback = int(slide.indexOf(QStringLiteral("<mc:Fallback")));
    if (fallback < 0) return QByteArray();
    const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("r:embed=\"([^\"]*)\"")).match(slide, fallback);
    if (!m.hasMatch()) return QByteArray();
    return e.pkg.part(e.pkg.relationship(QStringLiteral("/ppt/slides/slide1.xml"), m.captured(1)).target);
}

// How much of `box` (a fraction of the image) holds ink: where the pixels that are not clear (or, for an opaque image, not the corner's color) are.
QRectF inkOf(const QImage &img)
{
    const QImage a = img.convertToFormat(QImage::Format_ARGB32);
    const QRgb ground = a.pixel(0, 0);
    int x0 = a.width(), y0 = a.height(), x1 = -1, y1 = -1;
    for (int y = 0; y < a.height(); ++y)
        for (int x = 0; x < a.width(); ++x) {
            const QRgb p = a.pixel(x, y);
            const bool ink = qAlpha(ground) == 0 ? qAlpha(p) > 64 : p != ground;
            if (!ink) continue;
            x0 = std::min(x0, x);
            x1 = std::max(x1, x);
            y0 = std::min(y0, y);
            y1 = std::max(y1, y);
        }
    if (x1 < 0) return QRectF();
    return QRectF(double(x0) / a.width(), double(y0) / a.height(), double(x1 - x0 + 1) / a.width(), double(y1 - y0 + 1) / a.height());
}

// The number of dark pixels of `img` in `area` (device pixels).
int darkIn(const QImage &img, const QRect &area)
{
    int n = 0;
    for (int y = std::max(0, area.top()); y < std::min(img.height(), area.bottom() + 1); ++y)
        for (int x = std::max(0, area.left()); x < std::min(img.width(), area.right() + 1); ++x) n += qGray(img.pixel(x, y)) < 100;
    return n;
}

} // namespace

class DeckEquationTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // Insert > Equation puts the equation in the middle of the slide, selected, as one step: Undo takes it away and Redo puts it back.
    void insertUndoAndRedo()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        QCOMPARE(equationCount(*ed->doc()), 0);
        insertLine(w, QStringLiteral("a/b+\\sqrt(x)"));
        QCOMPARE(equationCount(*ed->doc()), 1);
        EquationItem *eq = firstEquation(*ed->doc());
        QCOMPARE(eq->linear, QStringLiteral("a/b+\\sqrt(x)"));
        const QSizeF page = ed->doc()->pageSize();
        QVERIFY(QRectF(QPointF(0, 0), page).contains(eq->rect));
        QVERIFY(eq->rect.width() > 40 && eq->rect.height() > 20);
        QCOMPARE(ed->single(), eq);
        ed->undoStack()->undo();
        QCOMPARE(equationCount(*ed->doc()), 0);
        ed->undoStack()->redo();
        QCOMPARE(equationCount(*ed->doc()), 1);
    }

    // The built-in equations of the gallery are put on the slide the same way.
    void theGalleryPutsAnEquationOnTheSlide()
    {
        DeckWindow w;
        QVERIFY(w.act(QStringLiteral("insert.equationDesign0")));
        w.act(QStringLiteral("insert.equationDesign0"))->trigger();
        EquationItem *eq = firstEquation(*w.editor()->doc());
        QVERIFY(eq);
        QCOMPARE(eq->linear, office::wml::equationGallery().at(0).linear);
    }

    // The equation moves and is sized like any object, one step each.
    void itMovesAndSizes()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertLine(w, QStringLiteral("x^2+y^2"));
        EquationItem *eq = firstEquation(*ed->doc());
        const QRectF was = eq->rect;
        ed->moveSelectionBy(30, 20);
        QCOMPARE(eq->rect.topLeft(), was.topLeft() + QPointF(30, 20));
        QVERIFY(eq->lockAspect);
        // (Undo puts the slide back as it was: the equation is looked up again.)
        ed->undoStack()->undo();
        QCOMPARE(firstEquation(*ed->doc())->rect, was);
        // Sized from its corner, it keeps its proportions.
        eq = firstEquation(*ed->doc());
        const QRectF big(was.x(), was.y(), was.width() * 2, was.height() * 2);
        eq->scaleInto(eq->rect, big);
        QCOMPARE(eq->rect, big);
    }

    // A double click (or Insert > Equation with it selected) opens the shared editor on its line; the new line takes its place, one step back.
    void aDoubleClickEditsInTheSharedEditor()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertLine(w, QStringLiteral("a/b"));
        EquationItem *eq = firstEquation(*ed->doc());
        const QString id = eq->id;
        const QPointF center = eq->rect.center();
        withDialog([&] { Q_EMIT w.canvas()->editEquationWanted(id); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QCOMPARE(d->windowTitle(), QStringLiteral("Edit Equation"));
            auto *line = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            QCOMPARE(line->text(), QStringLiteral("a/b"));
            line->setText(QStringLiteral("x^2+y^2"));
        });
        QCOMPARE(equationCount(*ed->doc()), 1);
        eq = firstEquation(*ed->doc());
        QCOMPARE(eq->id, id);
        QCOMPARE(eq->linear, QStringLiteral("x^2+y^2"));
        QVERIFY((eq->rect.center() - center).manhattanLength() < 0.01);
        // With it selected, the command edits it too.
        ed->select(id);
        withDialog([&] { w.act(QStringLiteral("insert.equation"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QCOMPARE(d->windowTitle(), QStringLiteral("Edit Equation"));
            d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->setText(QStringLiteral("z"));
        });
        QCOMPARE(equationCount(*ed->doc()), 1);
        QCOMPARE(firstEquation(*ed->doc())->linear, QStringLiteral("z"));
        ed->undoStack()->undo();
        QCOMPARE(firstEquation(*ed->doc())->linear, QStringLiteral("x^2+y^2"));
        ed->undoStack()->undo();
        QCOMPARE(firstEquation(*ed->doc())->linear, QStringLiteral("a/b"));
    }

    // Saved, the equation is the pair of alternatives the format has (the math in a14:m, and a picture of it); opened again it is the same equation, in the same place.
    void aPptxRoundTripGivesTheSameEquation()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertLine(w, QStringLiteral("\\sum_(k=0)^n x^k=(1-x^(n+1))/(1-x)"));
        EquationItem *eq = firstEquation(*ed->doc());
        eq->rect = QRectF(50, 60, 300, 70);
        eq->name = QStringLiteral("Series");
        eq->altText = QStringLiteral("A geometric series");
        const QString line = eq->linear;
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join('\n')));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QString slide;
        QVERIFY(!fallbackPicture(e, &slide).isEmpty());
        QCOMPARE(int(slide.count(QStringLiteral("<mc:AlternateContent"))), 1);
        QVERIFY(slide.contains(QStringLiteral("Requires=\"a14\"")));
        QVERIFY(slide.contains(QStringLiteral("<a14:m>")));
        QVERIFY(slide.contains(QStringLiteral("<m:oMathPara")));
        QVERIFY(slide.indexOf(QStringLiteral("<mc:Choice")) < slide.indexOf(QStringLiteral("<mc:Fallback")));

        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(equationCount(*back), 1);
        EquationItem *again = firstEquation(*back);
        QCOMPARE(again->linear, office::wml::equationLinear(eq->markup));
        QVERIFY(!again->linear.isEmpty() && !line.isEmpty());
        QCOMPARE(office::wml::equationLine(again->markup), office::wml::equationLine(eq->markup));
        QVERIFY(qAbs(again->rect.x() - 50) < 0.2 && qAbs(again->rect.y() - 60) < 0.2 && qAbs(again->rect.width() - 300) < 0.2 && qAbs(again->rect.height() - 70) < 0.2);
        QCOMPARE(again->name, QStringLiteral("Series"));
        QCOMPARE(again->altText, QStringLiteral("A geometric series"));
        // It is the equation, not a picture left beside it.
        for (const ItemPtr &it : back->pages[0]->items) QVERIFY(it->type() != ItemType::Picture);
        // Saved a second time: still one pair, one picture.
        const QByteArray second = deck::exportPresentation(*back, &err);
        QVERIFY2(!second.isEmpty(), qPrintable(err));
        Exported e2;
        QVERIFY2(unpack(second, &e2, &err), qPrintable(err));
        QCOMPARE(int(part(e2, QStringLiteral("/ppt/slides/slide1.xml")).count(QStringLiteral("<mc:AlternateContent"))), 1);
        QCOMPARE(e2.zip.keys().filter(QRegularExpression(QStringLiteral("^ppt/media/"))).size(), e.zip.keys().filter(QRegularExpression(QStringLiteral("^ppt/media/"))).size());
    }

    // The picture the other programs show has the equation's own size and looks like what JeffDeck draws.
    void theFallbackPictureMatchesWhatWeDraw()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertLine(w, QStringLiteral("x=(-b\\pm\\sqrt(b^2-4ac))/(2a)"));
        EquationItem *eq = firstEquation(*ed->doc());
        eq->rect = QRectF(40, 40, 360, 90);
        QString err;
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*ed->doc(), &err), &e, &err), qPrintable(err));
        QImage png;
        QVERIFY(png.loadFromData(fallbackPicture(e)));
        QVERIFY(png.width() > 0 && png.height() > 0);
        QVERIFY(png.width() <= 4000 && png.height() <= 4000);
        // The picture's proportions are the frame's.
        QVERIFY2(qAbs(double(png.width()) / png.height() - 360.0 / 90.0) < 0.05, qPrintable(QString::number(double(png.width()) / png.height())));
        // Its ink is where ours is.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = ed->doc();
        ctx.cache = &cache;
        const QImage ours = Renderer::renderItemsToImage(ctx, ItemList{ed->doc()->itemPtr(eq->id)}, 3.0, 0);
        QVERIFY(!ours.isNull());
        const QRectF a = inkOf(png), b = inkOf(ours);
        QVERIFY(!a.isEmpty() && !b.isEmpty());
        QVERIFY2(qAbs(a.left() - b.left()) < 0.04 && qAbs(a.top() - b.top()) < 0.06 && qAbs(a.width() - b.width()) < 0.06 && qAbs(a.height() - b.height()) < 0.08,
                 qPrintable(QStringLiteral("%1,%2 %3x%4 against %5,%6 %7x%8").arg(a.left()).arg(a.top()).arg(a.width()).arg(a.height()).arg(b.left()).arg(b.top()).arg(b.width()).arg(b.height())));
    }

    // An equation read from a file (math alone in its shape) is an equation here, one that can be changed; nothing is left beside it as a picture.
    void anEquationInAFileIsAnEquationHere()
    {
        for (const QString &form : {QStringLiteral("pic"), QStringLiteral("shape")}) {
            QString err;
            auto d = deck::importPresentation(pack(equationPptx(form)), &err);
            QVERIFY2(d, qPrintable(err));
            QCOMPARE(equationCount(*d), 1);
            EquationItem *eq = firstEquation(*d);
            QVERIFY(eq->markup.contains("E=mc"));
            QCOMPARE(eq->name, QStringLiteral("TextBox 8"));
            QCOMPARE(eq->rect, QRectF(120, 174, 150, 40));
            QVERIFY(d->pages[0]->props.value(QStringLiteral("charts")).toArray().isEmpty());
            for (const ItemPtr &it : d->pages[0]->items) QVERIFY2(it->name != QStringLiteral("TextBox 8") || it->type() == ItemType::Equation, qPrintable(form));
            // Saved, it is the pair again, with its math and a picture of it.
            Exported e;
            QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
            QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
            QString slide;
            QVERIFY(!fallbackPicture(e, &slide).isEmpty());
            QVERIFY(slide.contains(QStringLiteral("<m:t>E=mc</m:t>")));
            QCOMPARE(int(slide.count(QStringLiteral("name=\"TextBox 8\""))), 2);
            QCOMPARE(int(slide.count(QStringLiteral("<mc:AlternateContent"))), 1);
        }
    }

    // One a file brought, which no typed line makes, opens the editor on the line it reads as; changing it makes the math from the line.
    void aFileEquationOpensTheEditorOnItsLine()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("eq.pptx"));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(pack(equationPptx()));
        }
        DeckWindow w;
        QVERIFY(w.openFile(path));
        Editor *ed = w.editor();
        EquationItem *eq = firstEquation(*ed->doc());
        QVERIFY(eq);
        const QString id = eq->id;
        const QRectF was = eq->rect;
        const QString start = office::wml::equationLine(eq->markup);
        QVERIFY(!start.isEmpty());
        withDialog([&] { Q_EMIT w.canvas()->editEquationWanted(id); }, QStringLiteral("equationDialog"), [&](QDialog *dlg) {
            auto *line = dlg->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            QCOMPARE(line->text(), start);
            line->setText(QStringLiteral("a/b"));
        });
        eq = firstEquation(*ed->doc());
        QCOMPARE(eq->linear, QStringLiteral("a/b"));
        QVERIFY(!eq->markup.contains("E=mc"));
        QVERIFY((eq->rect.center() - was.center()).manhattanLength() < 0.01);
        ed->undoStack()->undo();
        QVERIFY(firstEquation(*ed->doc())->markup.contains("E=mc"));
        // Accepted as it was, nothing changes.
        const int steps = ed->undoStack()->index();
        withDialog([&] { Q_EMIT w.canvas()->editEquationWanted(id); }, QStringLiteral("equationDialog"), [&](QDialog *) {});
        QCOMPARE(ed->undoStack()->index(), steps);
    }

    // Text around an equation is not lost: a box with words and math stays the picture it was and is written back whole.
    void aBoxWithWordsAndMathIsStillKeptWhole()
    {
        QString err;
        auto d = deck::importPresentation(pack(equationPptx(QStringLiteral("pic"), true)), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(equationCount(*d), 0);
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<a:t>Energy </a:t>")));
    }

    // Math nested past the layout's depth, or too large, is not made an equation (the box stays as it was read); and a file holds at most kMaxEquations of them.
    void hostileMathIsBounded()
    {
        QString err;
        // Nested 400 deep: the layout draws what it can (it says it cut) or refuses; either way the read finishes and the count of equations is within the bound.
        QString deep = QStringLiteral("<m:r><m:t>x</m:t></m:r>");
        for (int i = 0; i < 400; ++i) deep = QStringLiteral("<m:sSup><m:e>%1</m:e><m:sup><m:r><m:t>2</m:t></m:r></m:sup></m:sSup>").arg(deep);
        Parts p = equationPptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        const QRegularExpression math(QStringLiteral("<m:oMath>.*?</m:oMath>"), QRegularExpression::DotMatchesEverythingOption);
        const QString pair = xml.mid(xml.indexOf(QStringLiteral("<mc:AlternateContent")));
        const QString hostile = QString(pair).replace(math, QStringLiteral("<m:oMath>") + deep + QStringLiteral("</m:oMath>"));
        xml.replace(pair, hostile);
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(equationCount(*d) <= 1);
        if (EquationItem *eq = firstEquation(*d)) QVERIFY(eq->markup.size() <= (1 << 20));

        // 2,100 equations on one slide: no more than kMaxEquations are read as equations.
        Parts many = equationPptx();
        QString slide = QString::fromUtf8(many.data["/ppt/slides/slide1.xml"].xml);
        const int at = int(slide.indexOf(QStringLiteral("<mc:AlternateContent")));
        const int to = int(slide.indexOf(QStringLiteral("</mc:AlternateContent>"), at)) + 22;
        const QString one = slide.mid(at, to - at);
        QString all;
        for (int i = 0; i < Document::kMaxEquations + 100; ++i) all += QString(one).replace(QStringLiteral("id=\"9\""), QStringLiteral("id=\"%1\"").arg(100 + i));
        slide.replace(one, all);
        many.data["/ppt/slides/slide1.xml"].xml = slide.toUtf8();
        auto big = deck::importPresentation(pack(many), &err);
        QVERIFY2(big, qPrintable(err));
        int total = 0;
        for (size_t i = 0; i < big->pages.size(); ++i) total += equationCount(*big, int(i));
        QCOMPARE(total, Document::kMaxEquations);
    }

    // The slide show, the printer's picture of a slide, and the PDF draw the equation.
    void theShowThePrinterAndThePdfDrawIt()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        Document *doc = ed->doc();
        const QSizeF page = doc->pageSize();
        const int showWidth = 960;
        const double k = showWidth / page.width();
        LayoutCache cache;
        const QImage beforeShow = renderSlideImage(doc, &cache, 0, QSize(showWidth, int(page.height() * k)), 1.0, Qt::white);
        const QImage beforePrint = deck::slideImage(*doc, &cache, 0, showWidth);
        QString err;
        QTemporaryDir dir;
        const QString blank = dir.filePath(QStringLiteral("blank.pdf"));
        QVERIFY2(deck::exportPdf(*doc, blank, deck::OutputOptions(), &err), qPrintable(err));
        insertLine(w, QStringLiteral("\\sum_(k=0)^n x^k=(1-x^(n+1))/(1-x)"));
        EquationItem *eq = firstEquation(*doc);
        const QRect area = QRectF(eq->rect.x() * k, eq->rect.y() * k, eq->rect.width() * k, eq->rect.height() * k).toAlignedRect();
        const QImage show = renderSlideImage(doc, &cache, 0, QSize(showWidth, int(page.height() * k)), 1.0, Qt::white);
        QVERIFY2(darkIn(show, area) > darkIn(beforeShow, area) + 100, "the show");
        const QImage print = deck::slideImage(*doc, &cache, 0, showWidth);
        QVERIFY2(darkIn(print, area) > darkIn(beforePrint, area) + 100, "the printer");
        const QString path = dir.filePath(QStringLiteral("with.pdf"));
        QVERIFY2(deck::exportPdf(*doc, path, deck::OutputOptions(), &err), qPrintable(err));
        const PdfDocument without(fileBytes(blank)), with(fileBytes(path));
        QVERIFY(with.isValid() && without.isValid());
        const QSize size(showWidth, int(page.height() * k));
        QVERIFY2(darkIn(with.render(0, size), area) > darkIn(without.render(0, size), area) + 100, "the PDF");
    }

    // Saved as an open document presentation, the equation is at least a picture of itself (it is not left out), and the message says so.
    void anOdpKeepsAPicture()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertLine(w, QStringLiteral("a/b"));
        EquationItem *eq = firstEquation(*ed->doc());
        eq->rect = QRectF(50, 60, 120, 60);
        QString err;
        QStringList said;
        const QByteArray bytes = deck::exportOdp(*ed->doc(), false, &err, &said);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(said.join(' ').contains(QStringLiteral("Equations are saved as pictures")), qPrintable(said.join('\n')));
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        int pictures = 0;
        for (const ItemPtr &it : back->pages[0]->items) pictures += it->type() == ItemType::Picture && it->rect.width() >= 120 && it->rect.width() <= 130;
        QCOMPARE(pictures, 1);
    }

    // A picture of the window with an equation on the slide, and of the editor open, for the docs, when JO_SHOT_DIR names a folder.
    void picturesOfTheWindowForTheDocs()
    {
        if (qEnvironmentVariableIsEmpty("JO_SHOT_DIR")) return;
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        // (the empty placeholders of the new slide are left out of the picture)
        w.editor()->doc()->pages[0]->items.clear();
        insertLine(w, QStringLiteral("x=(-b\\pm\\sqrt(b^2-4ac))/(2a)"));
        EquationItem *eq = firstEquation(*w.editor()->doc());
        const QSizeF page = w.editor()->doc()->pageSize();
        eq->rect = QRectF(page.width() * 0.15, page.height() * 0.45, page.width() * 0.7, page.height() * 0.25);
        QCoreApplication::processEvents();
        shotOf(&w, QStringLiteral("deck-equation-slide"));
        const QString id = eq->id;
        withDialog([&] { Q_EMIT w.canvas()->editEquationWanted(id); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QCoreApplication::processEvents();
            shotOf(d, QStringLiteral("deck-equation-editor"));
        }, false);
    }
};

JP_TEST_CLASS(DeckEquationTests)
#include "tst_deck_equation.moc"

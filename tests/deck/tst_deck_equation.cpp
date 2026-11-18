// JeffDeck's tests of Insert > Equation: an equation is an object on the slide (moved, sized, edited again in the shared editor, undone), saved as the pair of alternatives
// the format has (the math, and a picture of it for the programs that don't draw math), and opened again as the same equation; the show, the printer, and the PDF draw it.
#include "deckhelpers.h"
#include "../withdialog.h"
#include "app/equationedit.h"
#include "app/equationtab.h"
#include "app/theme.h"
#include "app/ribbon.h"
#include "office/drawingml_write.h"
#include "office/xml.h"
#include <QXmlStreamWriter>
#include "wml/wmlmathedit.h"
#include "wml/wmlmathpath.h"

#include <QBuffer>
#include <QTemporaryDir>
#include <QLineEdit>
#include <QRadioButton>
#include <QToolButton>
#include <QScopeGuard>

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
QByteArray fallbackPicture(const Exported &e, QString *slideXml = nullptr, int number = 1)
{
    const QString slidePart = QStringLiteral("/ppt/slides/slide%1.xml").arg(number);
    const QString slide = part(e, slidePart);
    if (slideXml) *slideXml = slide;
    const int fallback = int(slide.indexOf(QStringLiteral("<mc:Fallback")));
    if (fallback < 0) return QByteArray();
    const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("r:embed=\"([^\"]*)\"")).match(slide, fallback);
    if (!m.hasMatch()) return QByteArray();
    return e.pkg.part(e.pkg.relationship(slidePart, m.captured(1)).target);
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

        // Opened again it is the picture the pair holds, in the same place, with the math kept to write back (it becomes an equation when it is changed here).
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const PictureItem *again = keptPicture(*back, *back->pages[0], QStringLiteral("equation"));
        QVERIFY(again);
        QVERIFY(qAbs(again->rect.x() - 50) < 0.2 && qAbs(again->rect.y() - 60) < 0.2 && qAbs(again->rect.width() - 300) < 0.2 && qAbs(again->rect.height() - 70) < 0.2);
        QCOMPARE(again->name, QStringLiteral("Series"));
        QCOMPARE(again->altText, QStringLiteral("A geometric series"));
        QCOMPARE(int(back->pages[0]->items.size()), int(ed->doc()->pages[0]->items.size()));
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

    // The pair of alternatives in a slide's XML, with the picture's link made alike (a saved file numbers its links as it likes).
    static QString pairOf(const QString &slide)
    {
        const int at = int(slide.indexOf(QStringLiteral("<mc:AlternateContent")));
        if (at < 0) return QString();
        QString pair = slide.mid(at, slide.indexOf(QStringLiteral("</mc:AlternateContent>"), at) + 22 - at);
        // (The file's pair is written whole; the declarations of the prefixes may sit on another element of it, which is the one thing that is not compared.)
        pair.replace(QRegularExpression(QStringLiteral(" xmlns:(p|a|r|m)=\"[^\"]*\"")), QString());
        pair.replace(QRegularExpression(QStringLiteral("r:embed=\"[^\"]*\"")), QStringLiteral("r:embed=\"X\""));
        // (The kept pair is serialized again from its parsed form, which orders each element's attributes as it likes: they are compared in order of name.)
        static const QRegularExpression tag(QStringLiteral("<([\\w:]+)((?:\\s+[\\w:]+=\"[^\"]*\")+)(\\s*/?)>"));
        static const QRegularExpression attribute(QStringLiteral("[\\w:]+=\"[^\"]*\""));
        QString out;
        qsizetype last = 0;
        for (auto m = tag.globalMatch(pair); m.hasNext();) {
            const QRegularExpressionMatch t = m.next();
            QStringList attrs;
            for (auto a = attribute.globalMatch(t.captured(2)); a.hasNext();) attrs << a.next().captured();
            attrs.sort();
            out += pair.mid(last, t.capturedStart() - last) + QLatin1Char('<') + t.captured(1) + QLatin1Char(' ') + attrs.join(QLatin1Char(' ')) + t.captured(3) + QLatin1Char('>');
            last = t.capturedEnd();
        }
        return out + pair.mid(last);
    }

    // An equation read from a file (math alone in its shape, with a fill, an outline, and its math at the left of a wide box) looks and is saved as the file had it
    // until it is changed here: the picture the file keeps is drawn, and the pair is written back as it was. Changing it makes it an equation (the editor starts from
    // the line it reads as), in the same place, as one step that undoes; then the pair written is ours.
    void aFileEquationStaysAsTheFileHasItUntilItIsChanged()
    {
        Parts p = equationPptx(QStringLiteral("pic"));
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(QStringLiteral("cx=\"1905000\""), QStringLiteral("cx=\"6000000\""));
        xml.replace(QStringLiteral("<a:noFill/></p:spPr><p:txBody>"), QStringLiteral("<a:solidFill><a:srgbClr val=\"FFFFC0\"/></a:solidFill><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></p:spPr><p:txBody>"));
        xml.replace(QStringLiteral("<a:p><a14:m>"), QStringLiteral("<a:p><a:pPr algn=\"l\"/><a14:m>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        const QString input = pairOf(xml);
        QVERIFY(input.contains(QStringLiteral("algn=\"l\"")) && input.contains(QStringLiteral("FFFFC0")));
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("eq.pptx"));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(pack(p));
        }
        DeckWindow w;
        QVERIFY(w.openFile(path));
        Editor *ed = w.editor();
        Document *doc = ed->doc();
        // Not an equation yet: the picture the file keeps, at its size, drawn as it is.
        QCOMPARE(equationCount(*doc), 0);
        const PictureItem *pic = keptPicture(*doc, *doc->pages[0], QStringLiteral("equation"));
        QVERIFY(pic);
        QVERIFY(qAbs(pic->rect.width() - 6000000 / 12700.0) < 0.1);
        QCOMPARE(doc->images.value(pic->imageId).bytes, equationPicture());
        const QImage drawn = slideImage(*doc);
        const double k = drawn.width() / doc->pageSize().width();
        const QColor mid = drawn.pixelColor(int(pic->rect.center().x() * k), int(pic->rect.center().y() * k));
        QVERIFY2(qAbs(mid.red() - 0xC0) < 4 && qAbs(mid.green() - 0x30) < 4, qPrintable(mid.name()));
        // Saved, the pair is the file's whole (its text and its picture; only the order of an element's attributes can differ).
        QString err;
        Exported same;
        QVERIFY2(unpack(deck::exportPresentation(*doc, &err), &same, &err), qPrintable(err));
        QCOMPARE(pairOf(part(same, QStringLiteral("/ppt/slides/slide1.xml"))), input);
        QCOMPARE(fallbackPicture(same), equationPicture());
        // Changed: an equation, in the frame the picture had.
        const QString id = pic->id;
        const QRectF frame = pic->rect;
        ed->select(id);
        withDialog([&] { Q_EMIT w.canvas()->pictureTabWanted(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QCOMPARE(d->windowTitle(), QStringLiteral("Edit Equation"));
            auto *line = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            QVERIFY2(line->text().contains(QStringLiteral("E")) && line->text().contains(QStringLiteral("mc")), qPrintable(line->text()));
            line->setText(QStringLiteral("a/b"));
        });
        QCOMPARE(equationCount(*doc), 1);
        EquationItem *eq = firstEquation(*doc);
        QCOMPARE(eq->id, id);
        QCOMPARE(eq->linear, QStringLiteral("a/b"));
        QCOMPARE(eq->name, QStringLiteral("TextBox 8"));
        QVERIFY(frame.contains(eq->rect) || (eq->rect.center() - frame.center()).manhattanLength() < 0.5);
        QVERIFY(doc->pages[0]->props.value(QStringLiteral("charts")).toArray().isEmpty());
        Exported ours;
        QVERIFY2(unpack(deck::exportPresentation(*doc, &err), &ours, &err), qPrintable(err));
        QVERIFY(pairOf(part(ours, QStringLiteral("/ppt/slides/slide1.xml"))) != input);
        QVERIFY(part(ours, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<m:oMathPara")));
        QVERIFY(fallbackPicture(ours) != equationPicture());
        // One step back: the picture, and the pair as the file had it.
        ed->undoStack()->undo();
        QCOMPARE(equationCount(*doc), 0);
        QVERIFY(keptPicture(*doc, *doc->pages[0], QStringLiteral("equation")));
        Exported back;
        QVERIFY2(unpack(deck::exportPresentation(*doc, &err), &back, &err), qPrintable(err));
        QCOMPARE(pairOf(part(back, QStringLiteral("/ppt/slides/slide1.xml"))), input);
        // Accepted as it was, nothing changes.
        const int steps = ed->undoStack()->index();
        ed->select(id);
        withDialog([&] { Q_EMIT w.canvas()->pictureTabWanted(); }, QStringLiteral("equationDialog"), [&](QDialog *) {});
        QCOMPARE(ed->undoStack()->index(), steps);
        QCOMPARE(equationCount(*doc), 0);
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

    // A file with thousands of equations, or math nested hundreds deep, is read with no layout at all (it is a picture until changed here) and no equation objects.
    void hostileMathCostsNothingToRead()
    {
        QString err;
        QString deep = QStringLiteral("<m:r><m:t>x</m:t></m:r>");
        for (int i = 0; i < 400; ++i) deep = QStringLiteral("<m:sSup><m:e>%1</m:e><m:sup><m:r><m:t>2</m:t></m:r></m:sup></m:sSup>").arg(deep);
        Parts many = equationPptx();
        QString slide = QString::fromUtf8(many.data["/ppt/slides/slide1.xml"].xml);
        const int at = int(slide.indexOf(QStringLiteral("<mc:AlternateContent")));
        const int to = int(slide.indexOf(QStringLiteral("</mc:AlternateContent>"), at)) + 22;
        QString one = slide.mid(at, to - at);
        one.replace(QRegularExpression(QStringLiteral("<m:oMath>.*?</m:oMath>"), QRegularExpression::DotMatchesEverythingOption), QStringLiteral("<m:oMath>") + deep + QStringLiteral("</m:oMath>"));
        QString all;
        for (int i = 0; i < Document::kMaxEquations + 100; ++i) all += QString(one).replace(QStringLiteral("id=\"9\""), QStringLiteral("id=\"%1\"").arg(100 + i));
        slide.replace(slide.mid(at, to - at), all);
        many.data["/ppt/slides/slide1.xml"].xml = slide.toUtf8();
        const qint64 before = office::wml::equationLayoutsMadeForTest();
        auto big = deck::importPresentation(pack(many), &err);
        QVERIFY2(big, qPrintable(err));
        int total = 0;
        for (size_t i = 0; i < big->pages.size(); ++i) total += equationCount(*big, int(i));
        QCOMPARE(total, 0);
        QCOMPARE(office::wml::equationLayoutsMadeForTest(), before);
    }

    // A presentation holds at most Document::kMaxEquations equations and kMaxEquationMarkupBytes of their math, counted over its slides, masters, and layouts, and
    // the groups in them; a new one past either is refused and nothing is added.
    void aNewEquationIsRefusedPastTheCountAndByteLimits()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        Document *doc = ed->doc();
        QVERIFY(deck::duplicateSlide(*doc, 0) == 1);
        QVERIFY(!doc->masters.isEmpty());
        auto make = [](qint64 bytes) {
            auto e = std::make_shared<EquationItem>();
            e->markup = QByteArray(int(bytes), 'm');
            e->rect = QRectF(0, 0, 10, 10);
            return e;
        };
        auto group = std::make_shared<GroupItem>();
        doc->pages[0]->items.push_back(group);
        // Spread over a slide, its copy, a group, and a master: 2,000 small ones.
        for (int i = 0; i < Document::kMaxEquations; ++i) {
            const int where = i % 4;
            if (where == 0) doc->pages[0]->items.push_back(make(1));
            else if (where == 1) doc->pages[1]->items.push_back(make(1));
            else if (where == 2) group->children.push_back(make(1));
            else doc->masters[0]->items.push_back(make(1));
        }
        int count = 0;
        auto tally = [&] {
            count = 0;
            for (const auto &pg : doc->pages) walkItems(pg->items, [&](const ItemPtr &it) { count += it->type() == ItemType::Equation; });
            for (const auto &m : doc->masters) walkItems(m->items, [&](const ItemPtr &it) { count += it->type() == ItemType::Equation; });
        };
        tally();
        QCOMPARE(count, Document::kMaxEquations);
        insertLine(w, QStringLiteral("a/b"));
        tally();
        QCOMPARE(count, Document::kMaxEquations);
        // Take them away, and the bytes: 16 of 1 MB each fill the total.
        for (auto &pg : doc->pages) pg->items.erase(std::remove_if(pg->items.begin(), pg->items.end(), [](const ItemPtr &it) { return it->type() == ItemType::Equation || it->type() == ItemType::Group; }), pg->items.end());
        for (auto &m : doc->masters) m->items.erase(std::remove_if(m->items.begin(), m->items.end(), [](const ItemPtr &it) { return it->type() == ItemType::Equation; }), m->items.end());
        tally();
        QCOMPARE(count, 0);
        for (int i = 0; i < 16; ++i) (i % 2 ? doc->pages[1]->items : doc->pages[0]->items).push_back(make(1 << 20));
        insertLine(w, QStringLiteral("a/b"));
        tally();
        QCOMPARE(count, 16);
        // One less, and it fits.
        doc->pages[0]->items.pop_back();
        doc->pages[1]->items.pop_back();
        insertLine(w, QStringLiteral("a/b"));
        tally();
        QCOMPARE(count, 15);
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

    // The pictures of all the equations in a file are held to a total of pixels (past it each is drawn smaller, never to nothing); equal equations share one picture;
    // a frame with no width or no height still has its equation, with a picture of one clear pixel.
    void theFallbackPicturesAreBoundedSharedAndNeverMissing()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        auto add = [&](const QString &line, const QRectF &rect) {
            auto eq = makeEquation(line, QPointF(100, 100));
            eq->rect = rect;
            ed->surfaceItems().push_back(eq);
        };
        for (int i = 0; i < 3; ++i) add(QStringLiteral("a/b"), QRectF(10, 10, 100, 50));
        for (int i = 0; i < 40; ++i) add(QStringLiteral("x^%1+1").arg(i + 2), QRectF(0, 0, 1300, 1300));
        add(QStringLiteral("q/r"), QRectF(10, 10, 0, 40));
        add(QStringLiteral("s/t"), QRectF(10, 10, 80, 0));
        QString err;
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*ed->doc(), &err), &e, &err), qPrintable(err));
        QCOMPARE(int(part(e, QStringLiteral("/ppt/slides/slide1.xml")).count(QStringLiteral("<mc:AlternateContent"))), 40 + 3 + 2);
        qint64 pixels = 0;
        int smallest = 1 << 30, shared = 0;
        for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it) {
            if (!it.key().startsWith(QLatin1String("ppt/media/")) || !it.key().endsWith(QLatin1String(".png"))) continue;
            QImage img;
            QVERIFY(img.loadFromData(it.value()));
            pixels += qint64(img.width()) * img.height();
            smallest = std::min(smallest, img.width() * img.height());
            shared += img.width() == 300 && img.height() == 150;
        }
        QVERIFY(smallest >= 1);
        QCOMPARE(shared, 1);   // the three equal equations have one picture
        QVERIFY2(pixels <= (qint64(64) << 20) + 50 * 4096, qPrintable(QString::number(pixels)));
        QVERIFY(pixels > (qint64(32) << 20));   // (the budget was spent: not every picture was cut to nothing)
        // The two with no size are written, each with a picture of one clear pixel.
        int clear = 0;
        for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it) {
            QImage img;
            if (!it.key().startsWith(QLatin1String("ppt/media/")) || !img.loadFromData(it.value()) || img.width() != 1 || img.height() != 1) continue;
            QCOMPARE(qAlpha(img.convertToFormat(QImage::Format_ARGB32).pixel(0, 0)), 0);
            ++clear;
        }
        QCOMPARE(clear, 2);   // (one for each: they are not the same math)
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("cx=\"0\"")));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("cy=\"0\"")));
    }

    // Editing the line of an equation counts toward the total of math too: a change that would go past 16 MB is refused and the equation stays.
    void changingALineIsHeldToTheMarkupTotalToo()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        Document *doc = ed->doc();
        insertLine(w, QStringLiteral("a"));
        const QString id = firstEquation(*doc)->id;
        const QByteArray before = firstEquation(*doc)->markup;
        // 15 MB of math elsewhere, so that the one new line (a few hundred bytes) fits but not 1 MB more.
        for (int i = 0; i < 15; ++i) {
            auto big = std::make_shared<EquationItem>();
            big->markup = QByteArray(1 << 20, 'm');
            big->rect = QRectF(0, 0, 10, 10);
            doc->pages[0]->items.push_back(big);
        }
        auto edit = [&](const QString &line) {
            withDialog([&] { Q_EMIT w.canvas()->editEquationWanted(id); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
                d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->setText(line);
            });
        };
        edit(QStringLiteral("a/b"));
        QCOMPARE(doc->item(id) ? static_cast<EquationItem *>(doc->item(id))->linear : QString(), QStringLiteral("a/b"));
        // Now the rest of the room is used: a longer line is refused.
        auto filler = std::make_shared<EquationItem>();
        filler->markup = QByteArray(Document::kMaxEquationMarkupBytes - (15 << 20) - static_cast<EquationItem *>(doc->item(id))->markup.size() - 10, 'm');
        filler->rect = QRectF(0, 0, 10, 10);
        doc->pages[0]->items.push_back(filler);
        const QByteArray now = static_cast<EquationItem *>(doc->item(id))->markup;
        edit(QStringLiteral("\\sum_(k=0)^n x^k=(1-x^(n+1))/(1-x)"));
        QCOMPARE(static_cast<EquationItem *>(doc->item(id))->markup, now);
        QCOMPARE(static_cast<EquationItem *>(doc->item(id))->linear, QStringLiteral("a/b"));
        Q_UNUSED(before)
    }

    // A fragment a caller hands the drawing writer that is not well formed writes nothing (a picture is written as itself instead), and what is written still parses.
    void aDamagedRawFragmentWritesNothing()
    {
        for (const QByteArray &kind : {QByteArray("equation"), QByteArray("picture")}) {
            office::DrawingWriteContext ctx;
            ctx.rawFrame = [](const Item &) { return QStringLiteral("<a:unbalanced xmlns:a=\"urn:x\"><a:inner></a:unbalanced>"); };
            ItemPtr item;
            if (kind == "equation") {
                auto eq = std::make_shared<EquationItem>();
                eq->rect = QRectF(0, 0, 10, 10);
                item = eq;
            } else {
                auto pic = std::make_shared<PictureItem>();
                pic->rect = QRectF(0, 0, 10, 10);
                item = pic;
            }
            QByteArray bytes;
            QXmlStreamWriter out(&bytes);
            out.writeStartElement(QStringLiteral("root"));
            out.writeNamespace(QStringLiteral("http://schemas.openxmlformats.org/presentationml/2006/main"), QStringLiteral("p"));
            out.writeNamespace(QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main"), QStringLiteral("a"));
            out.writeNamespace(QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships"), QStringLiteral("r"));
            office::writeShape(out, *item, ctx);
            out.writeEndElement();
            QVERIFY2(!bytes.contains("unbalanced") && !bytes.contains("inner"), kind.constData());
            QDomDocument dom;
            QVERIFY2(office::parseXml(bytes, &dom), kind.constData());
        }
    }

    // A control character in the name or the alternative text is taken out of what is written: the file parses, and opens with the text without it.
    void controlCharactersInTextAreLeftOut()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertLine(w, QStringLiteral("a/b"));
        EquationItem *eq = firstEquation(*ed->doc());
        eq->name = QStringLiteral("Na\x0Bme");
        eq->altText = QStringLiteral("Al\x0B" "t \"text\" <&>");
        QString err;
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const PictureItem *pic = keptPicture(*back, *back->pages[0], QStringLiteral("equation"));
        QVERIFY(pic);
        QCOMPARE(pic->name, QStringLiteral("Name"));
        QCOMPARE(pic->altText, QStringLiteral("Alt \"text\" <&>"));
    }

    // A duplicated slide's equation is written with its own picture, and a changed copy with a picture of its own math.
    void aDuplicatedSlidesEquationsEachHaveTheirOwnPicture()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertLine(w, QStringLiteral("a/b"));
        QCOMPARE(deck::duplicateSlide(*ed->doc(), 0), 1);
        QString err;
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*ed->doc(), &err), &e, &err), qPrintable(err));
        QVERIFY(!fallbackPicture(e, nullptr, 1).isEmpty());
        QCOMPARE(fallbackPicture(e, nullptr, 2), fallbackPicture(e, nullptr, 1));
        EquationItem *second = firstEquation(*ed->doc(), 1);
        QVERIFY(second);
        auto other = makeEquation(QStringLiteral("\\sqrt(x)+y^2"), second->rect.center());
        second->markup = other->markup;
        second->linear = other->linear;
        second->rect = other->rect;
        Exported f;
        QVERIFY2(unpack(deck::exportPresentation(*ed->doc(), &err), &f, &err), qPrintable(err));
        QVERIFY(!fallbackPicture(f, nullptr, 2).isEmpty());
        QVERIFY(fallbackPicture(f, nullptr, 2) != fallbackPicture(f, nullptr, 1));
        QCOMPARE(fallbackPicture(f, nullptr, 1), fallbackPicture(e, nullptr, 1));
    }

    // The letters' color is written as the equation has it: a theme color as the theme's, a plain one as itself.
    void aThemeColoredEquationWritesTheThemeColor()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertLine(w, QStringLiteral("a/b"));
        EquationItem *eq = firstEquation(*ed->doc());
        eq->fill = Fill::solid(ColorRef::scheme(Accent1));
        QString err;
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*ed->doc(), &err), &e, &err), qPrintable(err));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        const int end = int(slide.indexOf(QStringLiteral("<a:endParaRPr"), slide.indexOf(QStringLiteral("</a14:m>"))));
        QVERIFY(end >= 0);
        QVERIFY2(slide.mid(end, 200).contains(QStringLiteral("<a:schemeClr val=\"accent1\"/>")), qPrintable(slide.mid(end, 200)));
        eq->fill = Fill::solid(ColorRef::rgb(QColor(0x12, 0x34, 0x56)));
        Exported f;
        QVERIFY2(unpack(deck::exportPresentation(*ed->doc(), &err), &f, &err), qPrintable(err));
        QVERIFY(part(f, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<a:srgbClr val=\"123456\"")));
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

    static bool tabShown(DeckWindow &w)
    {
        for (int i = 0; i < w.ribbon()->tabCount(); ++i)
            if (w.ribbon()->tabName(i) == QLatin1String("Equation")) return w.ribbon()->tabVisible(i);
        return false;
    }

    // The Equation tab shows exactly while an equation is selected, and the structure buttons put their text in the dialog's line.
    void theEquationTabShowsWhileAnEquationIsSelectedAndItsButtonsEdit()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        QVERIFY(!tabShown(w));
        insertLine(w, QStringLiteral("x"));
        const QString id = firstEquation(*ed->doc())->id;
        QTRY_VERIFY(tabShown(w));
        ed->select(QString());
        QTRY_VERIFY(!tabShown(w));
        ed->select(id);
        QTRY_VERIFY(tabShown(w));
        withDialog([&] { w.act(QStringLiteral("equation.structure.radical"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QCOMPARE(d->windowTitle(), QStringLiteral("Edit Equation"));
            auto *line = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            QCOMPARE(line->text(), QStringLiteral("xsqrt(x)"));
            line->setText(QStringLiteral("sqrt(x)"));
        });
        QCOMPARE(firstEquation(*ed->doc())->linear, QStringLiteral("sqrt(x)"));
        QCOMPARE(firstEquation(*ed->doc())->markup, jp::office::wml::equationMarkup(QStringLiteral("sqrt(x)")));
        ed->deleteSelection();
        QTRY_VERIFY(!tabShown(w));
    }

    // Professional and Linear are kept with the equation (with the markup, whichever it is shown as) and undo; the dialog gives back the form it was left in.
    void anEquationKeepsItsFormAndItsMarkup()
    {
        const bool linearBefore = jp::equationLinearSetting();
        const auto restore = qScopeGuard([&] { jp::setEquationLinearSetting(linearBefore); });
        DeckWindow w;
        Editor *ed = w.editor();
        insertLine(w, QStringLiteral("(a+b)/c"));
        const QString id = firstEquation(*ed->doc())->id;
        const QByteArray markup = firstEquation(*ed->doc())->markup;
        const QSizeF pro = firstEquation(*ed->doc())->rect.size();
        ed->select(id);
        w.act(QStringLiteral("equation.linear"))->trigger();
        QVERIFY(firstEquation(*ed->doc())->linearForm);
        QCOMPARE(firstEquation(*ed->doc())->markup, markup);
        QCOMPARE(firstEquation(*ed->doc())->linear, QStringLiteral("(a+b)/c"));
        QVERIFY(firstEquation(*ed->doc())->rect.height() < pro.height());   // (shown as a line)
        // The form is in the item's own record and in a copy of it.
        const QJsonObject json = firstEquation(*ed->doc())->toJson();
        QVERIFY(json.value("linearForm").toBool());
        QVERIFY(static_cast<EquationItem *>(firstEquation(*ed->doc())->clone().get())->linearForm);
        QTRY_VERIFY(w.act(QStringLiteral("equation.linear"))->isChecked());
        w.act(QStringLiteral("equation.professional"))->trigger();
        QVERIFY(!firstEquation(*ed->doc())->linearForm);
        QCOMPARE(firstEquation(*ed->doc())->markup, markup);
        QVERIFY(std::abs(firstEquation(*ed->doc())->rect.height() - pro.height()) < 0.01);
        ed->undoStack()->undo();
        QVERIFY(firstEquation(*ed->doc())->linearForm);
        // The dialog starts in the form the equation is in and the form chosen there is kept.
        withDialog([&] { w.act(QStringLiteral("equation.edit"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QVERIFY(d->findChild<QRadioButton *>(QStringLiteral("equationLinear"))->isChecked());
            d->findChild<QRadioButton *>(QStringLiteral("equationProfessional"))->setChecked(true);
        });
        QVERIFY(!firstEquation(*ed->doc())->linearForm);
        QCOMPARE(firstEquation(*ed->doc())->markup, markup);
    }

    // Review > Compare has its button on the ribbon, and it is hidden while the command has not been made (the compare window makes it): a hidden command stands in, and the button follows it.
    void theCompareButtonIsOnTheReviewTabAndHiddenWhileItHasNoCommand()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QAction *compare = w.act(QStringLiteral("review.compare"));
        QVERIFY(compare);
        auto button = [&]() -> QToolButton * {
            for (QToolButton *b : w.ribbon()->tab(QStringLiteral("Review"))->findChildren<QToolButton *>())
                if (b->defaultAction() == compare) return b;
            return nullptr;
        };
        QToolButton *b = button();
        QVERIFY(b);
        QVERIFY(!compare->isVisible());
        QVERIFY(b->isHidden());
        // (the group that holds only it shows nothing either: no empty box on the tab)
        RibbonGroup *group = nullptr;
        for (RibbonGroup *g : w.ribbon()->tab(QStringLiteral("Review"))->groups())
            if (g->title() == QLatin1String("Compare")) group = g;
        QVERIFY(group);
        QVERIFY(group->userHidden());
        compare->setVisible(true);
        QVERIFY(!b->isHidden());
        QVERIFY(!group->userHidden());
        compare->setVisible(false);
        QVERIFY(b->isHidden());
        QVERIFY(group->userHidden());
    }

    // The window with the Equation tab shown, when JO_SHOT_DIR names a folder.
    void theEquationTabPicture()
    {
        if (qEnvironmentVariableIsEmpty("JO_SHOT_DIR")) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        w.editor()->doc()->pages[0]->items.clear();
        insertLine(w, QStringLiteral("x=(-b\\pm\\sqrt(b^2-4ac))/(2a)"));
        QTRY_VERIFY(tabShown(w));
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Equation")));
        QTest::qWait(200);
        shotOf(&w, QStringLiteral("equation-tab-deck"));
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

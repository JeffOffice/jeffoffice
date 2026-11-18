// JeffPub's tests of Insert > Equation: the object on the page (inserted, undone, moved, sized, edited), kept in a .jpub, saved as a picture in a .pub, and drawn as vector paths in a PDF.
#include "pubhelpers.h"
#include "../pdfmarkcheck.h"
#include "../withdialog.h"
#include "canvas/canvas.h"
#include "app/help.h"
#include "app/i18n.h"
#include "app/settings.h"
#include "app/equationedit.h"
#include "app/equationdialog.h"
#include "app/equationtab.h"
#include "app/ribbon.h"
#include "app/theme.h"
#include "render/renderer.h"
#include "io/pubimport.h"
#include "io/zip.h"
#include "wml/wmlmathedit.h"
#include "wml/wmlmathast.h"
#include "wml/wmlmathpath.h"
#include "wml/xmltree.h"
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QPushButton>
#include <QRadioButton>
#include <QTemporaryDir>
#include <QThread>
#include <functional>

class PubEquationTests : public QObject {
    Q_OBJECT

    static QVector<jp::EquationItem *> equationsOn(jp::Editor *ed)
    {
        QVector<jp::EquationItem *> out;
        ed->doc()->forEachItem([&](jp::Item *it, int, const QString &) {
            if (auto *e = dynamic_cast<jp::EquationItem *>(it)) out << e;
        });
        return out;
    }

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }
    // Each test starts from the default equation format: the format choice is remembered, and a runner that keeps its settings between runs (or an earlier test) must not change what the dialog opens in.
    void init()
    {
        jp::Settings::get().remove(QStringLiteral("equation/syntax"));
        jp::Settings::get().remove(QStringLiteral("equation/linear"));
    }

    // The gallery of the program's other equation editors is on the Insert tab, and each design puts an equation on the page that undoes and redoes.
    void insertingAnEquationUndoesAndRedoes()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        QVERIFY(w.act(QStringLiteral("ins.equation")));
        QVERIFY(w.act(QStringLiteral("ins.equationDesign1")));
        QVERIFY(equationsOn(ed).isEmpty());
        const QString line = jp::office::wml::equationGallery().at(1).linear;
        w.act(QStringLiteral("ins.equationDesign1"))->trigger();
        QCOMPARE(equationsOn(ed).size(), 1);
        jp::EquationItem *e = equationsOn(ed).first();
        QCOMPARE(e->linear, line);
        QVERIFY(e->markup.contains("<m:oMath"));
        QVERIFY(e->rect.width() > 20 && e->rect.height() > 10);
        QVERIFY(ed->selection().size() == 1 && ed->selection().front() == e->id);
        ed->undo();
        QVERIFY(equationsOn(ed).isEmpty());
        ed->redo();
        QCOMPARE(equationsOn(ed).size(), 1);
        QCOMPARE(equationsOn(ed).first()->linear, line);
    }

    // Help with an equation selected opens the topic on equations (the last kind of object: its name was once read from past the end of the list).
    void helpWithAnEquationSelectedOpensTheEquationsTopic()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        const QString id = jp::insertEquation(ed, QStringLiteral("a/b"));
        QVERIFY(!id.isEmpty());
        ed->select(id);
        QCOMPARE(w.helpContext(), QStringLiteral("equations"));
    }

    // The Help for the tab names the matrices the menu has (a 2 by 2, an identity, and a column of two), not one it lacks.
    void theHelpNamesOnlyMatricesTheMenuHas()
    {
        for (const char *dir : {"", "deck/", "write/", "crunch/"}) {
            QFile f(QStringLiteral(JP_TEST_DATA "/../../resources/help/en/") + QLatin1String(dir) + QStringLiteral("equations.md"));
            QVERIFY2(f.open(QIODevice::ReadOnly), dir);
            const QString text = QString::fromUtf8(f.readAll());
            QVERIFY2(!text.contains(QStringLiteral("3 by 3")), dir);
            QVERIFY2(text.contains(QStringLiteral("identity matrix")), dir);
        }
        for (const jp::EquationCategory &c : jp::equationCategories())
            if (c.id == QLatin1String("matrix")) {
                QStringList ids;
                for (const jp::EquationVariant &v : c.variants) ids << v.id;
                QCOMPARE(ids, QStringList({"two", "identity", "column"}));
            }
    }

    // The Linear form of an equation is made once for its markup, however often it is drawn, hit, or exported.
    void theLinearFormIsMadeOncePerMarkup()
    {
        jp::EquationItem eq;
        eq.markup = jp::office::wml::equationMarkup(QStringLiteral("(a+b)/c+x^2"));
        eq.linearForm = true;
        eq.rect = QRectF(0, 0, 200, 40);
        const qint64 before = jp::Renderer::linearFormsMadeForTest();
        for (int i = 0; i < 25; ++i) QVERIFY(!jp::Renderer::equationOutline(eq).isEmpty());
        QCOMPARE(jp::Renderer::linearFormsMadeForTest() - before, qint64(1));
        eq.markup = jp::office::wml::equationMarkup(QStringLiteral("(a+b)/c+x^3"));
        for (int i = 0; i < 25; ++i) QVERIFY(!jp::Renderer::equationOutline(eq).isEmpty());
        QCOMPARE(jp::Renderer::linearFormsMadeForTest() - before, qint64(2));
        eq.linearForm = false;
        for (int i = 0; i < 5; ++i) jp::Renderer::equationOutline(eq);
        QCOMPARE(jp::Renderer::linearFormsMadeForTest() - before, qint64(2));
    }

    // A full line says so when a structure will not go in (the preview shows a note), and the line stays as it was.
    void aFullLineRefusesAStructureAndSaysSo()
    {
        QLineEdit line;
        line.setText(QString(jp::office::wml::kMostEquationInput - 1, QLatin1Char('x')));
        QVERIFY(!jp::insertIntoEquationLine(&line, jp::equationPickText(QStringLiteral("structure:fraction.stacked"), jp::EquationSyntax::Line)));
        QCOMPARE(line.text().size(), jp::office::wml::kMostEquationInput - 1);
        QVERIFY(jp::insertIntoEquationLine(&line, jp::equationPickText(QStringLiteral("symbol:\u03c0"), jp::EquationSyntax::Line)));
        withDialog([&] { jp::askEquationMarkup(nullptr, QStringLiteral("Edit Equation"), nullptr, nullptr); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->setText(QString(jp::office::wml::kMostEquationInput - 1, QLatin1Char('x')));
            d->findChild<QToolButton *>(QStringLiteral("equationStructure_fraction"))->click();
            QCOMPARE(d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->text().size(), jp::office::wml::kMostEquationInput - 1);
            QVERIFY(d->findChild<QLabel *>(QStringLiteral("equationPreview"))->text().contains(QStringLiteral("no room")));
        });
    }

    // An equation typed in LaTeX (no line of our own makes it) still says itself in words: alt text, the e-book, and the screen reader never get nothing.
    void anEquationTypedInLatexHasReadableText()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        bool ok = false;
        const QByteArray markup = jp::office::wml::equationFromLatex(QStringLiteral("\\frac{a}{b}+\\sqrt{x}"), &ok);
        QVERIFY(ok);
        const QString id = jp::insertEquationMarkup(ed, markup, QString(), false);
        QVERIFY(!id.isEmpty());
        const auto *eq = static_cast<const jp::EquationItem *>(ed->doc()->item(id));
        QVERIFY(eq->linear.isEmpty());
        QVERIFY(!jp::equationReadableText(*eq).isEmpty());
        QCOMPARE(jp::equationReadableText(*eq), jp::office::wml::equationToUnicodeMath(markup));
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("book.epub"));
        QString err;
        QVERIFY2(w.exportEpubTo(path, QStringLiteral("Math"), QString(), true, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> z;
        QVERIFY2(jp::readZip(f.readAll(), z, &err), qPrintable(err));
        int found = 0;
        for (auto it = z.cbegin(); it != z.cend(); ++it) found += it.value().contains("<p class=\"equation\">");
        QCOMPARE(found, 1);
    }

    // The equation is an object: it moves and sizes like one, in steps that undo, and "Equation" with it selected opens the same editor on it.
    void anEquationMovesSizesAndIsEditedInTheDialog()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        const QString id = jp::insertEquation(ed, QStringLiteral("a/b"));
        QVERIFY(!id.isEmpty());
        // (an undo may put the object back as a new copy: it is looked up again each time)
        auto eq = [&] { return dynamic_cast<jp::EquationItem *>(ed->doc()->item(id)); };
        QVERIFY(eq());
        const QRectF first = eq()->rect;
        ed->moveSelectionBy(10, 20);
        QCOMPARE(eq()->rect.topLeft(), first.topLeft() + QPointF(10, 20));
        ed->undo();
        QCOMPARE(eq()->rect, first);
        // Sized: the outline stretches to the frame.
        ed->change(QStringLiteral("Size"), [&] { eq()->rect.setSize(first.size() * 2); });
        QSizeF natural;
        const QPainterPath path = jp::Renderer::equationOutline(*eq(), &natural);
        QVERIFY(std::abs(natural.width() * 2 - eq()->rect.width()) < 0.01);
        QVERIFY(std::abs(path.boundingRect().right() - eq()->rect.width()) < eq()->rect.width() * 0.2);
        // Edited: the dialog opens with the line, and the equation changes in one step that undoes.
        ed->select(id);
        const QSizeF before = eq()->rect.size();
        withDialog([&] { w.act(QStringLiteral("ins.equation"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            auto *input = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            QVERIFY(input);
            QCOMPARE(input->text(), QStringLiteral("a/b"));
            input->setText(QStringLiteral("x^2+y^2"));
            QCOMPARE(d->findChild<QLabel *>(QStringLiteral("equationPreview"))->text(), QStringLiteral("x2 + y2"));
        });
        QCOMPARE(equationsOn(ed).size(), 1);
        QCOMPARE(eq()->linear, QStringLiteral("x^2+y^2"));
        QVERIFY(eq()->rect.size() != before);
        ed->undo();
        QCOMPARE(eq()->linear, QStringLiteral("a/b"));
        QCOMPARE(eq()->rect.size(), before);
    }

    // For a picture of the window with an equation in it (JP_SCREENSHOT_DIR names the folder; the test does nothing without it).
    void aWindowPictureOfAnEquation()
    {
        const QString dir = qEnvironmentVariable("JP_SCREENSHOT_DIR");
        if (dir.isEmpty()) QSKIP("JP_SCREENSHOT_DIR is not set");
        jp::MainWindow w;
        w.resize(1100, 760);
        w.show();
        QVERIFY(!jp::insertEquation(w.editor(), QStringLiteral("x=(-b\\pm\\sqrt(b^2-4ac))/(2a)")).isEmpty());
        QVERIFY(!jp::insertEquation(w.editor(), QStringLiteral("\\sum_(k=0)^n x^k=(1-x^(n+1))/(1-x)")).isEmpty());
        w.editor()->moveSelectionBy(0, 110);
        auto *canvas = w.findChild<jp::Canvas *>();
        QVERIFY(canvas);
        canvas->setZoom(1.6);
        canvas->ensureVisible(w.editor()->selectionBounds());
        QTest::qWait(300);
        w.screenshotTo(QDir(dir).filePath(QStringLiteral("pub-equation.png")));
        QVERIFY(QFileInfo::exists(QDir(dir).filePath(QStringLiteral("pub-equation.png"))));
    }

    // The layout of an equation is kept by a hash of its markup, not the markup itself, and one layout serves every use of the same markup.
    void theLayoutCacheIsKeyedByAHashAndSharedByEveryUse()
    {
        using namespace jp::office::wml;
        clearEquationCacheForTest();
        QByteArray big = "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><m:r><m:t>";
        big += QByteArray(3500, 'x') + "</m:t></m:r></m:oMath>";
        const qint64 before = equationLayoutsMadeForTest();
        const auto first = equationShapeCached(big);
        QVERIFY(first);
        QVERIFY2(equationCacheLongestKeyForTest() <= 48, qPrintable(QString::number(equationCacheLongestKeyForTest())));
        QVERIFY(equationCacheCostForTest() >= 1);
        QByteArray copy = big;
        copy.detach();
        QVERIFY(equationShapeCached(copy) == first);
        QImage img(200, 40, QImage::Format_ARGB32);
        QPainter p(&img);
        QVERIFY(paintEquation(&p, big, QRectF(0, 0, 200, 40), Qt::black));
        p.end();
        QVERIFY(!equationPng(big, 100, 30).isEmpty());
        QCOMPARE(equationLayoutsMadeForTest() - before, qint64(1));
    }

    // A publication holds a number of equations and a total of their markup; the ones past either are left out, counted, and the window says so when it opens the file.
    void aJpubWithTooManyEquationsKeepsTheLimitAndSaysSo()
    {
        auto build = [](int count, int markupBytes) {
            auto doc = jp::Document::blank(QSizeF(612, 792));
            for (int i = 0; i < count; ++i) {
                auto w = std::make_shared<jp::EquationItem>();
                w->markup = QByteArray(markupBytes, 'm');
                w->rect = QRectF(10, 10, 20, 20);
                doc->pages[0]->items.push_back(w);
            }
            return doc;
        };
        auto count = [](jp::Document &d) {
            int n = 0;
            d.forEachItem([&](jp::Item *it, int, const QString &) { n += it->type() == jp::ItemType::Equation; });
            return n;
        };
        QString err;
        auto many = jp::publicationFromBytes(jp::publicationBytes(*build(jp::Document::kMaxEquations + 5, 20), QImage()), &err);
        QVERIFY2(many, qPrintable(err));
        QCOMPARE(count(*many), jp::Document::kMaxEquations);
        QCOMPARE(many->equationsLeftOut, 5);
        const int big = 1 << 20, fit = int(jp::Document::kMaxEquationMarkupBytes / big);
        auto heavy = jp::publicationFromBytes(jp::publicationBytes(*build(fit + 2, big - 1), QImage()), &err);
        QVERIFY2(heavy, qPrintable(err));
        QCOMPARE(count(*heavy), fit);
        QCOMPARE(heavy->equationsLeftOut, 2);
        // the window names the limit when it opens such a file
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("many.jpub"));
        QVERIFY(jp::savePublication(*build(jp::Document::kMaxEquations + 5, 20), path, QImage(), &err));
        jp::MainWindow w;
        QStringList told;
        w.setNotifier([&](const QString &, const QString &text) { told << text; });
        QVERIFY(w.openFile(path));
        QVERIFY2(told.join(QLatin1Char('|')).contains(QStringLiteral("more equations than JeffPub keeps")), qPrintable(told.join(QLatin1Char('|'))));
    }

    // What the machine translations of the dialog's text were before it moved to the shared dialog is what each language shows now.
    void theEquationDialogKeepsItsTranslations()
    {
        struct Restore {
            QVariant before = jp::Settings::get().value(QStringLiteral("ui/language"));
            ~Restore()
            {
                jp::Settings::get().setValue(QStringLiteral("ui/language"), before);
                jp::installTranslations();
            }
        } restore;
        QStringList missing;
        for (const char *lang : {"de", "es", "fr", "it", "nl", "pt_BR", "pt_PT"}) {
            jp::Settings::get().setValue(QStringLiteral("ui/language"), QLatin1String(lang));
            jp::installTranslations();
            // (the long text of the dialog is read from the dialog itself)
            withDialog([] { QString line; jp::askEquation(nullptr, QStringLiteral("x"), &line); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
                const auto labels = d->findChildren<QLabel *>();
                QString help;
                for (QLabel *l : labels)
                    if (l->wordWrap()) help = l->text();
                if (help.isEmpty() || help.startsWith(QLatin1String("Type the equation"))) missing << QLatin1String(lang) + QStringLiteral(" help");
            });
            for (const char *source : {"Equation preview", "Built-In Equations"}) {
                const QString shown = QCoreApplication::translate("EquationDialog", source);
                if (shown.isEmpty() || shown.startsWith(QLatin1String(source))) missing << QLatin1String(lang) + QLatin1Char(' ') + QLatin1String(source);
            }
        }
        QVERIFY2(missing.isEmpty(), qPrintable(missing.join(QStringLiteral(", "))));
    }

    // A language whose Help has no translation of the equations topic yet shows the English one, and its contents link nothing that is not there.
    void aMissingTranslatedHelpTopicShowsTheEnglishAndLeavesNoDeadLink()
    {
        struct Restore {
            QVariant before = jp::Settings::get().value(QStringLiteral("ui/language"));
            ~Restore()
            {
                jp::Settings::get().setValue(QStringLiteral("ui/language"), before);
                jp::installTranslations();
            }
        } restore;
        int fellBack = 0;
        for (const char *lang : {"de", "fr", "pt_BR", "en_GB"}) {
            jp::Settings::get().setValue(QStringLiteral("ui/language"), QLatin1String(lang));
            jp::installTranslations();
            const jp::help::Topic t = jp::help::topic(QStringLiteral("equations"));
            // (a language without the page shows the English one; British English holds only the pages that differ, so one always falls back)
            if (!QFile::exists(QStringLiteral(JP_TEST_DATA "/../../resources/help/") + QLatin1String(lang) + QStringLiteral("/equations.md"))) {
                ++fellBack;
                QCOMPARE(t.title, QStringLiteral("Add an equation"));
            } else {
                QVERIFY2(!t.title.isEmpty(), lang);
            }
            QVERIFY(jp::help::topicIds().contains(QStringLiteral("equations")));
            const QString contents = jp::help::topic(QStringLiteral("index")).markdown;
            static const QRegularExpression link(QStringLiteral("\\]\\(([^)\\s]+)\\)"));
            for (auto m = link.globalMatch(contents); m.hasNext();) {
                const QString target = m.next().captured(1);
                if (!target.startsWith(QLatin1String("https://"))) QVERIFY2(jp::help::topicIds().contains(target), qPrintable(QLatin1String(lang) + QLatin1Char(' ') + target));
            }
        }
        if (fellBack == 0) {
            // (every language has this page now: the fallback is checked with a page British English, which holds only the pages that differ, leaves to the English)
            const QString help = QStringLiteral(JP_TEST_DATA "/../../resources/help/");
            QString missing;
            for (const QFileInfo &f : QDir(help + QStringLiteral("en")).entryInfoList({QStringLiteral("*.md")}, QDir::Files, QDir::Name)) {
                if (!QFile::exists(help + QStringLiteral("en_GB/") + f.fileName())) { missing = f.completeBaseName(); break; }
            }
            QVERIFY(!missing.isEmpty());
            jp::Settings::get().setValue(QStringLiteral("ui/language"), QStringLiteral("en"));
            jp::installTranslations();
            const QString english = jp::help::topic(missing).title;
            QVERIFY(!english.isEmpty());
            jp::Settings::get().setValue(QStringLiteral("ui/language"), QStringLiteral("en_GB"));
            jp::installTranslations();
            QCOMPARE(jp::help::topic(missing).title, english);
            ++fellBack;
        }
        QVERIFY(fellBack > 0);
    }

    // The model is the one the other programs share: a line that makes no equation (too deep, too long, empty) makes no object.
    void aLineThatMakesNoEquationMakesNoObject()
    {
        jp::MainWindow w;
        QVERIFY(jp::insertEquation(w.editor(), QStringLiteral("   ")).isEmpty());
        QVERIFY(jp::insertEquation(w.editor(), QString(jp::office::wml::kMostEquationInput + 1, QLatin1Char('x'))).isEmpty());
        QVERIFY(jp::insertEquation(w.editor(), QString(QStringLiteral("(")).repeated(jp::office::wml::kMostEquationDepth + 10) + QStringLiteral("x")).isEmpty());
        QVERIFY(equationsOn(w.editor()).isEmpty());
    }

    // A .jpub keeps the model: the line, the markup, the size, and the place.
    void aJpubKeepsTheEquation()
    {
        jp::MainWindow w;
        const QString id = jp::insertEquation(w.editor(), QStringLiteral("\\sum_(k=0)^n x^k=(1-x^(n+1))/(1-x)"));
        QVERIFY(!id.isEmpty());
        auto *e = dynamic_cast<jp::EquationItem *>(w.editor()->doc()->item(id));
        e->rect.translate(7, 9);
        e->size = 31;
        QString err;
        const QByteArray bytes = jp::publicationBytes(*w.editor()->doc(), QImage());
        auto back = jp::publicationFromBytes(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVector<jp::EquationItem *> found;
        back->forEachItem([&](jp::Item *it, int, const QString &) {
            if (auto *x = dynamic_cast<jp::EquationItem *>(it)) found << x;
        });
        QCOMPARE(found.size(), 1);
        QCOMPARE(found[0]->linear, e->linear);
        QCOMPARE(found[0]->markup, e->markup);
        QCOMPARE(found[0]->size, 31.0);
        QCOMPARE(found[0]->rect, e->rect);
        QVERIFY(jp::office::wml::equationLine(found[0]->markup).contains(QChar(0x2211)));
    }

    // A markup past the limits is cut to them when it is read from a file.
    void aHostileJpubEquationIsKeptWithinTheLimits()
    {
        QJsonObject o;
        o["type"] = QStringLiteral("equation");
        o["markup"] = QString(2'000'000, QLatin1Char('x'));
        o["size"] = 1e12;
        auto it = jp::Item::fromJsonAny(o);
        QVERIFY(it);
        auto *e = static_cast<jp::EquationItem *>(it.get());
        QVERIFY(e->markup.size() <= (1 << 20));
        QVERIFY(e->size <= 1000);
        QString deep;
        for (int i = 0; i < 40; ++i) deep += QStringLiteral("<m:sSup><m:e>");
        deep += QStringLiteral("<m:r><m:t>x</m:t></m:r>");
        for (int i = 0; i < 40; ++i) deep += QStringLiteral("</m:e><m:sup><m:r><m:t>2</m:t></m:r></m:sup></m:sSup>");
        const auto shape = jp::office::wml::equationShape(("<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">" + deep + "</m:oMath>").toUtf8(), 24);
        QVERIFY(shape.cut);
    }

    // A .pub file has no equation: it is saved as a picture of it, in the place and size it has, and the report says so.
    void aPubSavesAPictureAndSaysSo()
    {
        jp::MainWindow w;
        const QString id = jp::insertEquation(w.editor(), QStringLiteral("a^2+b^2=c^2"));
        QVERIFY(!id.isEmpty());
        const QRectF rect = w.editor()->doc()->item(id)->rect;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("eq.pub"));
        QString note;
        QVERIFY(jp::exportPublisher(*w.editor()->doc(), path, &note));
        QVERIFY2(note.contains(QStringLiteral("equations were saved to .pub as pictures")), qPrintable(note));
        QVERIFY2(!note.contains(QStringLiteral("couldn't be saved")), qPrintable(note));
        QString err;
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        int pictures = 0;
        QRectF at;
        back->forEachItem([&](jp::Item *it, int, const QString &) {
            if (it->type() == jp::ItemType::Picture) {
                ++pictures;
                at = it->rect;
            }
        });
        QCOMPARE(pictures, 1);
        QVERIFY(std::abs(at.width() - rect.width()) < rect.width() * 0.15 && std::abs(at.height() - rect.height()) < rect.height() * 0.15);   // (the size, within the rounding of the file)
    }

    // A PDF draws the equation as vector paths: filled outlines, and no picture.
    void aPdfDrawsTheEquationAsPaths()
    {
        jp::MainWindow w;
        QVERIFY(!jp::insertEquation(w.editor(), QStringLiteral("x=(-b\\pm\\sqrt(b^2-4ac))/(2a)")).isEmpty());
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("eq.pdf"));
        w.exportPdf(path, false, false);
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        QVERIFY(pdf.startsWith("%PDF-"));
        const QVector<QByteArray> pages = pdfcheck::pageContents(pdf);
        QVERIFY(!pages.isEmpty());
        const QString content = QString::fromLatin1(pages.first());
        QVERIFY2(content.count(QRegularExpression(QStringLiteral("(?m)^[-\\d. ]+ (m|l|c|re)$"))) > 10, "no path operators in the page");
        QVERIFY2(content.contains(QRegularExpression(QStringLiteral("(?m)^(f\\*?|B\\*?|S)$"))), "no painted path");
        QVERIFY(!content.contains(QRegularExpression(QStringLiteral("(?m) Do$"))));
        QVERIFY(!pdf.contains("/Subtype /Image"));
    }

    // ---- Plain-text equations: UnicodeMath and LaTeX (the precedence table is in libs/office/wml/wmlmathuni.h) ----

    static QByteArray um(const QString &text) { return jp::office::wml::equationFromUnicodeMath(text); }
    static QByteArray tex(const QString &text) { return jp::office::wml::equationFromLatex(text); }
    // (A function's name and the limit under it have a blank between them as a function, and none as the script of a letter run: the same characters in order.)
    static QString noBlanks(QString s) { return s.remove(QLatin1Char(' ')); }
    static QByteArray md5(const QByteArray &m) { return QCryptographicHash::hash(m, QCryptographicHash::Md5).toHex(); }
    static QString xmlRoot(const QByteArray &body) { return QString::fromLatin1("<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">") + QString::fromUtf8(body) + QStringLiteral("</m:oMath>"); }
    static QByteArray xmlMath(const QByteArray &body) { return xmlRoot(body).toUtf8(); }

    // Each of 13 structures, typed in both syntaxes: the same markup from both, the text it prints as reads back as the same markup, and the markup has the element the structure is.
    void thirteenStructuresRoundTripInBothSyntaxes()
    {
        using namespace jp::office::wml;
        struct Case {
            const char *name;
            QString uni, latex;
            const char *element;
        };
        const QVector<Case> cases = {
            {"fraction", QStringLiteral("a/b"), QStringLiteral("\\frac{a}{b}"), "<m:f>"},
            {"scripts", QStringLiteral("x^2_i"), QStringLiteral("x^2_i"), "<m:sSubSup>"},
            {"root", QStringLiteral("√(x)"), QStringLiteral("\\sqrt{x}"), "<m:rad>"},
            {"nth root", QStringLiteral("√(3&x)"), QStringLiteral("\\sqrt[3]{x}"), "<m:deg><m:r><m:t>3</m:t>"},
            {"sum", QStringLiteral("∑_(i=1)^n i"), QStringLiteral("\\sum_{i=1}^n i"), "<m:chr m:val=\"\u2211\"/><m:limLoc m:val=\"undOvr\"/>"},
            {"integral", QStringLiteral("∫_0^1 x"), QStringLiteral("\\int_0^1 x"), "<m:limLoc m:val=\"subSup\"/>"},
            {"brackets", QStringLiteral("(a|b)"), QStringLiteral("\\left( a \\middle| b \\right)"), "<m:sepChr m:val=\"|\"/>"},
            {"matrix", QStringLiteral("\\matrix(a&b@c&d)"), QStringLiteral("\\begin{matrix} a & b \\\\ c & d \\end{matrix}"), "<m:mr>"},
            {"matrix in brackets", QStringLiteral("(\\matrix(1&0@0&1))"), QStringLiteral("\\begin{pmatrix} 1 & 0 \\\\ 0 & 1 \\end{pmatrix}"), "<m:d><m:dPr><m:begChr m:val=\"(\"/><m:endChr m:val=\")\"/></m:dPr><m:e><m:m>"},
            {"normal text", QStringLiteral("\"if\""), QStringLiteral("\\text{if}"), "<m:nor/>"},
            {"accent", QString::fromUtf8("x\u0302"), QStringLiteral("\\hat{x}"), "<m:acc>"},
            {"function", QStringLiteral("sin x"), QStringLiteral("\\sin x"), "<m:func>"},
            {"limit", QString::fromUtf8("lim_(x→0) f"), QStringLiteral("\\lim_{x\\to 0} f"), "<m:limLow>"},
            {"bar", QStringLiteral("\\overline(x)"), QStringLiteral("\\overline{x}"), "<m:bar>"},
        };
        for (const Case &c : cases) {
            const QByteArray fromUni = um(c.uni), fromLatex = tex(c.latex);
            QVERIFY2(!fromUni.isEmpty(), c.name);
            QVERIFY2(!fromLatex.isEmpty(), c.name);
            QVERIFY2(fromUni.contains(c.element), c.name);
            QVERIFY2(fromLatex.contains(c.element), c.name);
            QCOMPARE(fromLatex, fromUni);
            const QString uniOut = equationToUnicodeMath(fromUni), latexOut = equationToLatex(fromUni);
            QVERIFY2(!uniOut.isEmpty() && !latexOut.isEmpty(), c.name);
            QVERIFY2(um(uniOut) == fromUni, c.name);
            QVERIFY2(tex(latexOut) == fromUni, c.name);
            QVERIFY2(!equationLine(fromUni).isEmpty(), c.name);
        }
    }

    // The precedence table: what binds to what, as two ways of writing the same equation that make the same markup.
    void theUnicodeMathPrecedenceTable()
    {
        const QVector<std::pair<QString, QString>> same = {
            {QStringLiteral("a/b/c"), QStringLiteral("(a/b)/c")},                  // a fraction is left to right
            {QStringLiteral("1+x/2"), QString::fromUtf8("1+〖x/2〗")},                  // only the atom before the slash is the numerator
            {QStringLiteral("ab/c"), QString::fromUtf8("a〖b/c〗")},
            {QStringLiteral("x^a^b"), QStringLiteral("x^(a^b)")},                  // a second script of one kind goes on the first's operand
            {QStringLiteral("x^a_b^c"), QStringLiteral("x_b^(a^c)")},
            {QStringLiteral("x^2_i"), QStringLiteral("x_i^2")},                    // one of each, in either order
            {QStringLiteral("a^2/b"), QStringLiteral("(a^2)/b")},                  // scripts bind tighter than the slash
            {QStringLiteral("a/b^2"), QStringLiteral("a/(b^2)")},
            {QStringLiteral("sin x/2"), QString::fromUtf8("sin〖x/2〗")},           // a function takes a term: an element and the fractions after it
            {QStringLiteral("sin x cos y"), QStringLiteral("sin x cos y")},
            {QString::fromUtf8("√x^2"), QString::fromUtf8("〖√x〗^2")},               // a root takes an atom, then scripts go on the root
            {QString::fromUtf8("x\u0302^2"), QString::fromUtf8("〖x\u0302〗^2")},    // an accent binds tighter than scripts
            {QStringLiteral("(a+b)/c"), QString::fromUtf8("〖a+b〗/c")},            // brackets on either side of a slash are not shown
            {QStringLiteral("x_(i+1)"), QString::fromUtf8("x_〖i+1〗")},
            {QString::fromUtf8("∑_i x_i/2"), QString::fromUtf8("∑_i 〖x_i/2〗")},   // a large operator takes a term
        };
        for (const auto &p : same) {
            const QByteArray a = um(p.first), b = um(p.second);
            QVERIFY2(!a.isEmpty() && !b.isEmpty(), qPrintable(p.first));
            QVERIFY2(a == b, qPrintable(p.first + QStringLiteral(" vs ") + p.second));
        }
        // ... and where they differ from the other reading.
        QVERIFY(um(QStringLiteral("a/b/c")) != um(QStringLiteral("a/(b/c)")));
        QVERIFY(um(QStringLiteral("1+x/2")) != um(QStringLiteral("(1+x)/2")));
        QVERIFY(um(QStringLiteral("x^a^b")) != um(QStringLiteral("(x^a)^b")));
        // Brackets are kept as the base of a script, as the argument of a function, and as the operand of a large operator.
        QVERIFY(um(QStringLiteral("(a+b)^2")).contains("<m:sSup><m:e><m:d>"));
        QVERIFY(um(QStringLiteral("sin(x)")).contains("<m:e><m:d>"));
        QVERIFY(um(QString::fromUtf8("∑_i (x+1)")).contains("<m:e><m:d>"));
        QVERIFY(!um(QStringLiteral("(a+b)/c")).contains("<m:d>"));
        // A sign after a function name or a large operator is not its operand.
        QVERIFY(!um(QStringLiteral("sin+x")).contains("<m:func>"));
        // The same table in LaTeX: an element is an atom with its scripts, and a digit is one operand.
        QCOMPARE(tex(QStringLiteral("\\frac12")), um(QStringLiteral("1/2")));
        QCOMPARE(tex(QStringLiteral("x^10")), tex(QStringLiteral("x^{1}0")));
        QCOMPARE(tex(QStringLiteral("x^a^b")), tex(QStringLiteral("x^{a^b}")));
        QVERIFY(tex(QStringLiteral("\\sum_{i=1}^n i^2")).contains("<m:e><m:sSup>"));
        QVERIFY(tex(QStringLiteral("\\sin^2 x")).contains("<m:fName><m:sSup>"));
        QVERIFY(tex(QStringLiteral("\\sum\\limits_a b")).contains("undOvr"));
        QVERIFY(tex(QStringLiteral("\\int\\limits_a b")).contains("<m:limLoc m:val=\"undOvr\"/>"));
        QVERIFY(tex(QStringLiteral("\\sum\\nolimits_a b")).contains("<m:limLoc m:val=\"subSup\"/>"));
    }

    // 12 from the gallery and 188 made from its pieces, as lines of our own syntax: the markup prints in both syntaxes, reads back, prints the same, and says the same letters.
    void twoHundredEquationsGoThroughBothSyntaxesAndBack()
    {
        using namespace jp::office::wml;
        quint32 seed = 20261117;
        auto next = [&](int n) {
            seed = seed * 1664525u + 1013904223u;
            return int((seed >> 16) % quint32(n));
        };
        const QStringList atoms = {QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("k"), QStringLiteral("2"), QStringLiteral("12"),
                                   QStringLiteral("\\alpha"), QStringLiteral("\\pi"), QStringLiteral("n")};
        std::function<QString(int)> gen = [&](int depth) -> QString {
            if (depth <= 0) return atoms[next(int(atoms.size()))];
            switch (next(16)) {
            case 0: return QLatin1Char('(') + gen(depth - 1) + QStringLiteral(")/(") + gen(depth - 1) + QLatin1Char(')');
            case 1: return atoms[next(int(atoms.size()))] + QStringLiteral("^(") + gen(depth - 1) + QLatin1Char(')');
            case 2: return atoms[next(int(atoms.size()))] + QStringLiteral("_(") + gen(depth - 1) + QLatin1Char(')');
            case 3: return QStringLiteral("\\sqrt(") + gen(depth - 1) + QLatin1Char(')');
            case 4: return QStringLiteral("\\sum_(i=1)^n ") + atoms[next(int(atoms.size()))];
            case 5: return QStringLiteral("\\int_0^1 ") + atoms[next(int(atoms.size()))];
            case 6: return QStringLiteral("\\hat{") + gen(depth - 1) + QLatin1Char('}');
            case 7: return QStringLiteral("\\bar{") + atoms[next(int(atoms.size()))] + QLatin1Char('}');
            case 8: return QStringLiteral("\\left( ") + gen(depth - 1) + QStringLiteral(" \\right)");
            case 9: return QStringLiteral("\\pmatrix{") + gen(depth - 1) + QStringLiteral(" & ") + gen(depth - 1) + QStringLiteral(" \\\\ ") + gen(depth - 1) + QStringLiteral(" & ") + gen(depth - 1) + QLatin1Char('}');
            case 10: return gen(depth - 1) + QLatin1Char('+') + gen(depth - 1);
            case 11: return gen(depth - 1) + QLatin1Char('=') + gen(depth - 1);
            case 12: return QLatin1Char('(') + gen(depth - 1) + QLatin1Char(')');
            case 13: return QStringLiteral("\\sqrt[3](") + gen(depth - 1) + QLatin1Char(')');
            case 14: return QStringLiteral("\"if\" ") + gen(depth - 1);
            default: return atoms[next(int(atoms.size()))] + QLatin1Char(' ') + atoms[next(int(atoms.size()))];
            }
        };
        QStringList lines;
        for (const EquationDesign &d : equationGallery()) lines << d.linear;
        while (lines.size() < 200) lines << gen(3);
        int viaUni = 0, viaLatex = 0;
        for (const QString &line : lines) {
            bool ok = false;
            const QByteArray m0 = equationMarkup(line, &ok);
            QVERIFY2(ok, qPrintable(line));
            for (int syntax = 0; syntax < 2; ++syntax) {
                const QString text = syntax == 0 ? equationToUnicodeMath(m0) : equationToLatex(m0);
                QVERIFY2(!text.isEmpty(), qPrintable(line));
                const QByteArray m1 = syntax == 0 ? um(text) : tex(text);
                QVERIFY2(!m1.isEmpty(), qPrintable(line + QStringLiteral(" -> ") + text));
                QCOMPARE(syntax == 0 ? equationToUnicodeMath(m1) : equationToLatex(m1), text);
                QVERIFY2(noBlanks(equationLine(m1)) == noBlanks(equationLine(m0)), qPrintable(line + QStringLiteral(" -> ") + text + QStringLiteral(": ") + equationLine(m1) + QStringLiteral(" vs ") + equationLine(m0)));
                (syntax == 0 ? viaUni : viaLatex)++;
            }
        }
        QCOMPARE(viaUni, 200);
        QCOMPARE(viaLatex, 200);
    }

    // Our own lines make exactly the markup they made before (a hash of each), and a line with words in quotes makes Normal Text.
    void ourLinesAreUnchangedAndQuotedWordsAreNormalText()
    {
        using namespace jp::office::wml;
        const QVector<std::pair<QString, const char *>> kept = {
            {QStringLiteral("\\alpha+\\beta"), "1a4eb2e96853ec5314aa4dafe2b01f2d"},
            {QStringLiteral("(a+b)^2"), "d7246b0a61bb9510a19f144c25aed8d3"},
            {QStringLiteral("a/b+x_i^2"), "feb16f7997aed9cbc3671d346aeab950"},
            {QStringLiteral("\\sqrt[3](x)"), "66494a390970f1e83e4f6b3815bf30a4"},
            {QStringLiteral("sin(x)+lim_(h\\to 0) h"), "7c636d942182528a7e77ee1d6485fbbc"},
        };
        for (const auto &k : kept) QCOMPARE(md5(equationMarkup(k.first)).constData(), k.second);
        const char *gallery[] = {"4e0a7b76a6dbb7b6476728baf26c0830", "5399e0ef0a40c201fd60c4eb75ec23e7", "08ccec7d5e75364b65acce6a869c8ff4", "285b7cd8083e89db2db80b1ffcd304cb",
                                 "79dd970fcf7375738246f9d67d2418f7", "796fa7da5906f5142f7d3da9c8734e65", "3df9a82db9c79c874603c893f652d19a", "14a604f93e13006fdc814a10c0d3f6f4",
                                 "973300445e18ae6ae636500154db24cb", "4b9c3475dee53386a36e08e19de77ae7", "769227181069896ef8337bb552d3885f", "55f1234502f5ef8b7dd3f05f4bdee463"};
        QCOMPARE(equationGallery().size(), 12);
        for (int i = 0; i < 12; ++i) QCOMPARE(md5(equationMarkup(equationGallery().at(i).linear)).constData(), gallery[i]);
        // Words in quotes: a run of Normal Text, kept as typed, and the line it is shown as comes back.
        const QByteArray m = equationMarkup(QStringLiteral("x=2 \"if x>0\""));
        QVERIFY(m.contains("<m:rPr><m:nor/></m:rPr><m:t xml:space=\"preserve\">if x&gt;0</m:t>"));
        QVERIFY(equationLine(m).endsWith(QStringLiteral("if x>0")));
        QCOMPARE(equationLinear(m), QStringLiteral("x=2 \"if x>0\""));
        QCOMPARE(equationMarkup(equationLinear(m)), m);
        QCOMPARE(equationMarkup(QStringLiteral("\"a \\\"quote\\\"\"")).count("<m:nor/>"), 1);
        QCOMPARE(equationLinear(equationMarkup(QStringLiteral("\"a \\\"quote\\\" and \\\\\""))), QStringLiteral("\"a \\\"quote\\\" and \\\\\""));
    }

    // Markup another program wrote has properties our lines never make (a font on every run): today's equationLinear says nothing for it. The two syntaxes print any markup they can say.
    void markupFromAnotherProgramPrintsInEverySyntax()
    {
        using namespace jp::office::wml;
        const QString run = QStringLiteral("<m:r><w:rPr><w:rFonts w:ascii=\"Serif\" w:hAnsi=\"Serif\"/></w:rPr><m:t>%1</m:t></m:r>");
        const QByteArray frac = xmlMath("<m:f><m:fPr><m:ctrlPr><w:rPr><w:rFonts w:ascii=\"Serif\"/></w:rPr></m:ctrlPr></m:fPr><m:num>" + run.arg(QStringLiteral("a")).toUtf8() + "</m:num><m:den>" + run.arg(QStringLiteral("b")).toUtf8() + "</m:den></m:f>");
        QCOMPARE(equationLinear(frac), QStringLiteral("{a}/{b}"));
        QCOMPARE(equationToUnicodeMath(frac), QStringLiteral("a/b"));
        QCOMPARE(equationToLatex(frac), QStringLiteral("\\frac{a}{b}"));
        QCOMPARE(equationMarkup(equationLinear(frac)).count("<m:f>"), 1);
        // A function, a root with a hidden degree, and a paragraph of one equation.
        const QByteArray func = xmlMath("<m:func><m:fName><m:r><m:rPr><m:sty m:val=\"p\"/></m:rPr><m:t>sin</m:t></m:r></m:fName><m:e><m:r><m:t>x</m:t></m:r></m:e></m:func>");
        QCOMPARE(equationToUnicodeMath(func), QStringLiteral("sin x"));
        QCOMPARE(equationToLatex(func), QStringLiteral("\\sin x"));
        const QByteArray para = "<m:oMathPara xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><m:oMathParaPr><m:jc m:val=\"center\"/></m:oMathParaPr>" + xmlMath("<m:r><m:t>y</m:t></m:r>") + "</m:oMathPara>";
        QCOMPARE(equationToUnicodeMath(para), QStringLiteral("y"));
        // What the syntaxes can't say is refused as a whole: a fraction without a bar, a group of characters, an equation array, a bold-italic run, and brackets that do not grow.
        const char *refused[] = {"<m:f><m:fPr><m:type m:val=\"noBar\"/></m:fPr><m:num><m:r><m:t>n</m:t></m:r></m:num><m:den><m:r><m:t>k</m:t></m:r></m:den></m:f>",
                                 "<m:groupChr><m:e><m:r><m:t>x</m:t></m:r></m:e></m:groupChr>", "<m:eqArr><m:e><m:r><m:t>x</m:t></m:r></m:e></m:eqArr>",
                                 "<m:r><m:rPr><m:sty m:val=\"bi\"/></m:rPr><m:t>x</m:t></m:r>", "<m:d><m:dPr><m:grow m:val=\"0\"/></m:dPr><m:e><m:r><m:t>x</m:t></m:r></m:e></m:d>"};
        for (const char *body : refused) {
            const QByteArray m = xmlMath(body);
            QVERIFY2(equationToUnicodeMath(m).isEmpty(), body);
            QVERIFY2(equationToLatex(m).isEmpty(), body);
            QVERIFY2(equationLinear(m).isEmpty(), body);
        }
        QVERIFY(equationToUnicodeMath(QByteArray("not xml")).isEmpty());
        QVERIFY(equationToLatex(QByteArray()).isEmpty());
    }

    // Normal Text is drawn upright, as typed (no spacing round a sign), where math letters lean and signs have room.
    void normalTextIsDrawnUprightAsTyped()
    {
        using namespace jp::office::wml;
        const double em = 40;
        auto width = [&](const QByteArray &m) {
            const EquationShape shape = equationShape(m, em);
            return shape.path.boundingRect().width();
        };
        const QByteArray normalL = um(QStringLiteral("\"lll\"")), italicL = um(QStringLiteral("lll"));
        QVERIFY(normalL.contains("<m:nor/>") && !italicL.contains("<m:nor/>"));
        const EquationShape n = equationShape(normalL, em), i = equationShape(italicL, em);
        QVERIFY(n.valid() && i.valid());
        QVERIFY(n.path != i.path);
        // (upright: the stem of the letter stands straight, where the same letter drawn italic in the same font leans over; measured as how far the middle of the ink in the top of the
        // letter is from the middle of the ink in the bottom, in the font's own units, so it holds in any font)
        auto lean = [](const QPainterPath &p) {
            const QRectF r = p.boundingRect();
            auto middle = [&](double from, double to) {
                const QRectF part = p.intersected([&] { QPainterPath b; b.addRect(QRectF(r.left() - 1, from, r.width() + 2, to - from)); return b; }()).boundingRect();
                return part.center().x();
            };
            return (middle(r.top(), r.top() + r.height() * 0.2) - middle(r.bottom() - r.height() * 0.2, r.bottom())) / r.height();
        };
        const EquationShape n1 = equationShape(um(QStringLiteral("\"l\"")), em), i1 = equationShape(um(QStringLiteral("l")), em);
        QVERIFY(n1.valid() && i1.valid());
        QVERIFY2(lean(n1.path) <= lean(i1.path), qPrintable(QString::number(lean(n1.path)) + " vs " + QString::number(lean(i1.path))));
        QVERIFY2(std::abs(lean(n1.path)) <= std::abs(lean(i1.path)), "upright leans no more than italic");
        // A sign in Normal Text has no room round it: "1+1" is narrower than math 1+1 by about the two gaps.
        QVERIFY2(width(um(QStringLiteral("\"1+1\""))) < width(um(QStringLiteral("1+1"))) - 0.3 * em, "no room round a sign in Normal Text");
        QVERIFY(equationLine(um(QStringLiteral("\"1+1\""))) == QStringLiteral("1+1"));
        QVERIFY(equationLine(um(QStringLiteral("1+1"))) == QStringLiteral("1 + 1"));
        QCOMPARE(equationLine(um(QStringLiteral("\"if x\""))), QStringLiteral("if x"));
    }

    // Every limit of the parsers and the printers, at the cap exactly: the cap's worth is built, one more is refused.
    void theLimitsOfTheParsersAndPrintersHoldAtTheCapExactly()
    {
        using namespace jp::office::wml;
        using jp::office::wml::mathtree::kMostNodes;
        auto count = [](const QByteArray &m, const char *what) { return m.isEmpty() ? -1 : int(m.count(what)); };
        // The most characters (kMostEquationInput): 2,000 letters make 2,000 runs, 2,001 make none.
        for (int syntax = 0; syntax < 2; ++syntax) {
            auto parse = [&](const QString &t, bool *ok) { return syntax == 0 ? equationFromUnicodeMath(t, ok) : equationFromLatex(t, ok); };
            bool ok = false;
            QCOMPARE(count(parse(QString(kMostEquationInput, QLatin1Char('x')), &ok), "<m:r>"), kMostEquationInput);
            QVERIFY(ok);
            QVERIFY(parse(QString(kMostEquationInput + 1, QLatin1Char('x')), &ok).isEmpty());
            QVERIFY(!ok);
            // ... and the most nested: 24 brackets build, 25 do not.
            const QString open = syntax == 0 ? QStringLiteral("(") : QStringLiteral("\\left("), close = syntax == 0 ? QStringLiteral(")") : QStringLiteral("\\right)");
            QCOMPARE(count(parse(open.repeated(kMostEquationDepth) + QLatin1Char('x') + close.repeated(kMostEquationDepth), &ok), "<m:d>"), kMostEquationDepth);
            QVERIFY(parse(open.repeated(kMostEquationDepth + 1) + QLatin1Char('x') + close.repeated(kMostEquationDepth + 1), &ok).isEmpty());
            // A matrix of 1,024 cells (16 rows of 64), not one of 1,025 (25 rows of 41), not 65 columns, not 65 rows.
            auto matrix = [&](int rows, int cols) {
                QStringList r;
                for (int i = 0; i < rows; ++i) r << QString(cols - 1, QLatin1Char('&'));
                return syntax == 0 ? QStringLiteral("\\matrix(") + r.join(QLatin1Char('@')) + QLatin1Char(')') : QStringLiteral("\\begin{matrix}") + r.join(QStringLiteral("\\\\")) + QStringLiteral("\\end{matrix}");
            };
            QCOMPARE(count(parse(matrix(16, 64), &ok), "<m:e>"), 1024);
            QCOMPARE(count(parse(matrix(16, 64), &ok), "<m:mr>"), 16);
            QVERIFY(parse(matrix(25, 41), &ok).isEmpty());
            QVERIFY(parse(matrix(1, 65), &ok).isEmpty());
            QCOMPARE(count(parse(matrix(1, 64), &ok), "<m:e>"), 64);
            // (rows that hold something, so that the last is not the empty one a trailing separator leaves)
            auto tall = [&](int rows) {
                QStringList r;
                for (int i = 0; i < rows; ++i) r << QStringLiteral("a");
                return syntax == 0 ? QStringLiteral("\\matrix(") + r.join(QLatin1Char('@')) + QLatin1Char(')') : QStringLiteral("\\begin{matrix}") + r.join(QStringLiteral("\\\\")) + QStringLiteral("\\end{matrix}");
            };
            QCOMPARE(count(parse(tall(64), &ok), "<m:mr>"), 64);
            QVERIFY(parse(tall(65), &ok).isEmpty());
            // A command that nests deeper than the depth cap (24 fractions in a chain) is refused too, though no bracket is in it.
            QString chain = syntax == 0 ? QStringLiteral("1") : QString();
            QString latex = QStringLiteral("1");
            for (int i = 0; i < kMostEquationDepth; ++i) {
                chain += QStringLiteral("/1");
                latex = QStringLiteral("\\frac{") + latex + QStringLiteral("}{1}");
            }
            QCOMPARE(count(parse(syntax == 0 ? chain : latex, &ok), "<m:f>"), kMostEquationDepth);
            QVERIFY(parse(syntax == 0 ? chain + QStringLiteral("/1") : QStringLiteral("\\frac{") + latex + QStringLiteral("}{1}"), &ok).isEmpty());
        }
        // Printing: 2,000 characters of text print, 2,001 do not (either syntax).
        auto letters = [](int n) {
            QByteArray body;
            for (int i = 0; i < n; ++i) body += "<m:r><m:t>x</m:t></m:r>";
            return xmlMath(body);
        };
        QCOMPARE(equationToUnicodeMath(letters(kMostEquationInput)).size(), kMostEquationInput);
        QVERIFY(equationToUnicodeMath(letters(kMostEquationInput + 1)).isEmpty());
        QCOMPARE(equationToLatex(letters(kMostEquationInput)).size(), kMostEquationInput);
        QVERIFY(equationToLatex(letters(kMostEquationInput + 1)).isEmpty());
        // Reading: 4,000 pieces are read, 4,001 are not (empty runs print nothing, so the text stays short).
        auto pieces = [](int n) {
            QByteArray body;
            for (int i = 0; i < n - 1; ++i) body += "<m:r><m:t></m:t></m:r>";
            return xmlMath(body + "<m:r><m:t>z</m:t></m:r>");
        };
        QCOMPARE(equationToUnicodeMath(pieces(kMostNodes)), QStringLiteral("z"));
        QVERIFY(equationToUnicodeMath(pieces(kMostNodes + 1)).isEmpty());
        QCOMPARE(equationToLatex(pieces(kMostNodes)), QStringLiteral("z"));
        QVERIFY(equationToLatex(pieces(kMostNodes + 1)).isEmpty());
        // Reading: 24 brackets in brackets print, 25 do not.
        auto nested = [](int n) {
            QByteArray body = "<m:r><m:t>x</m:t></m:r>";
            for (int i = 0; i < n; ++i) body = "<m:d><m:e>" + body + "</m:e></m:d>";
            return xmlMath(body);
        };
        QCOMPARE(equationToUnicodeMath(nested(kMostEquationDepth)).size(), 2 * kMostEquationDepth + 1);
        QVERIFY(equationToUnicodeMath(nested(kMostEquationDepth + 1)).isEmpty());
        QVERIFY(equationToLatex(nested(kMostEquationDepth + 1)).isEmpty());
        QVERIFY(!equationToLatex(nested(kMostEquationDepth)).isEmpty());
        // Reading a matrix: 1,024 cells print, 1,025 do not.
        auto cells = [](int rows, int cols) {
            QByteArray body = "<m:m>";
            for (int r = 0; r < rows; ++r) {
                body += "<m:mr>";
                for (int c = 0; c < cols; ++c) body += "<m:e><m:r><m:t></m:t></m:r></m:e>";
                body += "</m:mr>";
            }
            return xmlMath(body + "</m:m>");
        };
        QVERIFY(!equationToUnicodeMath(cells(16, 64)).isEmpty());
        QVERIFY(equationToUnicodeMath(cells(25, 41)).isEmpty());
        QVERIFY(equationToLatex(cells(25, 41)).isEmpty());
    }

    // Text that nests without a bracket (an operator or a function name in the operand of a script) is refused at the depth cap, not read until the stack is gone: each runs on a thread
    // with the smallest stack (1 MB), which the unbounded reading overran.
    void textThatNestsWithoutBracketsIsRefusedAndNeverOverrunsTheStack()
    {
        using namespace jp::office::wml;
        struct Runner : QThread {
            std::function<void()> body;
            void run() override { body(); }
        };
        QStringList unrefused, accepted;
        Runner thread;
        thread.setStackSize(1 << 20);
        thread.body = [&] {
            const QStringList deep = {QStringLiteral("∑^").repeated(1000), QStringLiteral("∑_").repeated(1000), QStringLiteral("sin^").repeated(500), QStringLiteral("sin_").repeated(500),
                                      QStringLiteral("x^∑^").repeated(500), QStringLiteral("√(").repeated(900) + QLatin1Char('x'), QStringLiteral("x^(").repeated(900) + QLatin1Char('x'),
                                      QStringLiteral("∑_(").repeated(600) + QLatin1Char('x'), QStringLiteral("sin(").repeated(500) + QLatin1Char('x')};
            for (const QString &t : deep) {
                bool ok = true;
                if (!equationFromUnicodeMath(t, &ok).isEmpty() || ok) unrefused << t.left(12);
            }
            const QStringList deepTex = {QStringLiteral("\\sum^").repeated(300), QStringLiteral("\\sum_").repeated(300), QStringLiteral("\\sin^").repeated(300), QStringLiteral("\\sqrt{").repeated(300) + QLatin1Char('x'),
                                         QStringLiteral("x^{").repeated(900) + QLatin1Char('x'), QStringLiteral("\\frac{").repeated(300) + QLatin1Char('x')};
            for (const QString &t : deepTex) {
                bool ok = true;
                if (!equationFromLatex(t, &ok).isEmpty() || ok) unrefused << t.left(12);
            }
            // Markup nested as deep is refused by both printers.
            QByteArray body = "<m:r><m:t>x</m:t></m:r>";
            for (int i = 0; i < 500; ++i) body = "<m:sSup><m:e>" + body + "</m:e><m:sup><m:r><m:t>y</m:t></m:r></m:sup></m:sSup>";
            if (!equationToUnicodeMath(xmlMath(body)).isEmpty() || !equationToLatex(xmlMath(body)).isEmpty()) unrefused << QStringLiteral("markup");
            // ... while a few levels are fine in each.
            for (const QString &t : {QStringLiteral("∑^∑^x"), QStringLiteral("sin^2 x"), QStringLiteral("x^∑_i y")})
                if (equationFromUnicodeMath(t).isEmpty()) accepted << t;
            if (equationFromLatex(QStringLiteral("\\sum^\\sum^x")).isEmpty()) accepted << QStringLiteral("tex");
        };
        thread.start();
        QVERIFY(thread.wait(60000));
        QVERIFY2(unrefused.isEmpty(), qPrintable(unrefused.join(QLatin1Char('|'))));
        QVERIFY2(accepted.isEmpty(), qPrintable(accepted.join(QLatin1Char('|'))));
        // The cap for this nesting is the cap of every other: a chain of operators one short of it builds.
        QVERIFY(!um(QStringLiteral("∑^").repeated(kMostEquationDepth - 2) + QLatin1Char('x')).isEmpty());
    }

    // A quote in a line: a lone one is a quote mark as it was (5" long), a pair is Normal Text, and a line stored when a pair was two quote marks is edited as the line that says so.
    void aLoneQuoteInALineIsAQuoteAndAnOldStoredLineIsEditedAsItWas()
    {
        using namespace jp::office::wml;
        const QByteArray inches = equationMarkup(QStringLiteral("5\" long"));
        QVERIFY(!inches.isEmpty());
        QCOMPARE(equationMarkup(QStringLiteral("5\\\" long")), inches);   // (the same as the quote written as itself)
        QVERIFY(!inches.contains("<m:nor/>"));
        QCOMPARE(equationLine(inches), QStringLiteral("5\" long"));
        QCOMPARE(equationMarkup(equationLinear(inches)), inches);
        QVERIFY(equationMarkup(QStringLiteral("x=\"a b\"")).contains("<m:nor/>"));
        QVERIFY(!equationMarkup(QStringLiteral("a \"b")).contains("<m:nor/>"));
        QVERIFY(!equationMarkup(QStringLiteral("a \\\" b \"c")).contains("<m:nor/>"));   // (an escaped quote does not close one)
        // A line stored before: its markup (quote marks) is kept in the file, and the line is shown as the line that makes it again.
        const QByteArray old = equationMarkup(QStringLiteral("x=\\\"a b\\\""));
        QVERIFY(!old.contains("<m:nor/>"));
        const QString stored = QStringLiteral("x=\"a b\"");
        QVERIFY(equationMarkup(stored) != old);
        const QString shown = equationLineForEditing(stored, old);
        QVERIFY(shown != stored);
        QCOMPARE(equationMarkup(shown), old);
        // A line that makes what is stored, a line with no quote, and a line with nothing stored are as they are.
        const QByteArray fresh = equationMarkup(stored);
        QCOMPARE(equationLineForEditing(stored, fresh), stored);
        QCOMPARE(equationLineForEditing(QStringLiteral("x+1"), old), QStringLiteral("x+1"));
        QCOMPARE(equationLineForEditing(stored, QByteArray()), stored);
        // The dialog opens with the stored line shown so, and OK keeps the equation as it was.
        QString line = stored;
        QByteArray markup = old;
        withDialog([&] { QVERIFY(jp::askEquationMarkup(nullptr, QStringLiteral("x"), &line, &markup)); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QCOMPARE(d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->text(), shown);
        });
        QCOMPARE(markup, old);
        QCOMPARE(line, shown);
    }

    // Markup from a root with no degree (and none hidden) has a line, and a missing part is empty.
    void aRootWithNoDegreePartHasALine()
    {
        using namespace jp::office::wml;
        const QByteArray ns = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";
        const QByteArray bare = "<m:oMath " + ns + "><m:rad><m:e><m:r><m:t>x</m:t></m:r></m:e></m:rad></m:oMath>";
        QCOMPARE(equationLinear(bare), QStringLiteral("\\sqrt{x}"));
        const QByteArray nothing = "<m:oMath " + ns + "><m:rad></m:rad><m:sSup></m:sSup><m:f/></m:oMath>";
        const QString none = equationLinear(nothing);
        QVERIFY(none.contains(QStringLiteral("\\sqrt{}")));
    }

    // All the matrices of one equation together hold at most 16,384 cells (empty ones too), read from markup.
    void theCellsOfAllTheMatricesOfOneEquationAreCounted()
    {
        using namespace jp::office::wml;
        using jp::office::wml::mathtree::kMostCells;
        QCOMPARE(kMostCells, 16384);
        auto matrices = [](int n) {
            QByteArray body;
            for (int k = 0; k < n; ++k) {
                body += "<m:m>";
                for (int r = 0; r < 16; ++r) {
                    body += "<m:mr>";
                    for (int c = 0; c < 64; ++c) body += "<m:e/>";
                    body += "</m:mr>";
                }
                body += "</m:m>";
            }
            return xmlMath(body);
        };
        jp::office::wml::mathtree::Seq items;
        QVERIFY(jp::office::wml::mathtree::read(matrices(16), &items));
        QCOMPARE(int(items.size()), 16);
        QVERIFY(!jp::office::wml::mathtree::read(matrices(17), &items));
        // (inside each other too: the cells of the inner ones count)
        QByteArray inner = "<m:m><m:mr><m:e/></m:mr></m:m>";
        for (int i = 0; i < 20; ++i) {
            QByteArray m = "<m:m>";
            for (int r = 0; r < 16; ++r) {
                m += "<m:mr>";
                for (int c = 0; c < 64; ++c) m += c == 0 && r == 0 ? "<m:e>" + inner + "</m:e>" : QByteArray("<m:e/>");
                m += "</m:mr>";
            }
            inner = m + "</m:m>";
        }
        QVERIFY(!jp::office::wml::mathtree::read(xmlMath(inner), &items));
    }

    // A gallery equation printed in either syntax makes the same MARKUP when read again, except the two kinds that are written another way the markup does not keep: a limit
    // (a function name with a limit under it, where the line has a subscript) and plain brackets (a group of characters, where the syntaxes have brackets as an element).
    void theGalleryMakesTheSameMarkupThroughBothSyntaxes()
    {
        using namespace jp::office::wml;
        QStringList differ;
        for (const EquationDesign &d : equationGallery()) {
            const QByteArray m = equationMarkup(d.linear);
            QVERIFY2(!m.isEmpty(), qPrintable(d.name));
            const QString u = equationToUnicodeMath(m), l = equationToLatex(m);
            QVERIFY2(!u.isEmpty() && !l.isEmpty(), qPrintable(d.name));
            if (um(u) != m) differ << d.name + QStringLiteral(" (UnicodeMath)");
            if (tex(l) != m) differ << d.name + QStringLiteral(" (LaTeX)");
        }
        // (the two known differences, which are all that differ: the limit in the derivative in both syntaxes, and the plain brackets that UnicodeMath reads back as a bracket element,
        // in the normal distribution and the derivative)
        QStringList expected = {QStringLiteral("Normal Distribution (UnicodeMath)"), QStringLiteral("Definition of a Derivative (UnicodeMath)"), QStringLiteral("Definition of a Derivative (LaTeX)")};
        QVERIFY2(differ == expected, qPrintable(differ.join(QLatin1Char('|'))));
    }

    // A line break (\\) outside a matrix is not an equation here: refused, not turned into a space; in a matrix it is the next row.
    void aLineBreakOutsideAMatrixIsRefusedInLatex()
    {
        bool ok = true;
        QVERIFY(tex(QStringLiteral("a\\\\b")).isEmpty());
        QVERIFY(jp::office::wml::equationFromLatex(QStringLiteral("a \\\\ b"), &ok).isEmpty());
        QVERIFY(!ok);
        QVERIFY(tex(QStringLiteral("\\frac{a\\\\b}{c}")).isEmpty());
        QCOMPARE(tex(QStringLiteral("\\begin{matrix} a \\\\ b \\end{matrix}")).count("<m:mr>"), 2);
        QVERIFY(!tex(QStringLiteral("a\\,b")).isEmpty());   // (a thin space is still a space)
    }

    // OK is on only for an equation the program can keep as typed: a line that makes the same markup (\limits is something the line can't say), or the markup itself.
    void theDialogKeepsOnlyALineThatMakesTheSameEquation()
    {
        const QString typed = QString::fromUtf8("∫\\limits_0^1 x");
        const QByteArray made = jp::office::wml::equationFromUnicodeMath(typed);
        QVERIFY(!made.isEmpty());
        QVERIFY(jp::office::wml::equationMarkup(jp::office::wml::equationLinear(made)) != made);
        // A program that keeps only the line: OK stays off.
        QString onlyLine;
        withDialog([&] { jp::askEquation(nullptr, QStringLiteral("x"), &onlyLine); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            auto *syntax = d->findChild<QComboBox *>(QStringLiteral("equationSyntax"));
            auto *input = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            auto *ok = d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok);
            syntax->setCurrentIndex(1);
            Q_EMIT syntax->activated(1);
            input->setText(typed);
            QVERIFY(!ok->isEnabled());
            input->setText(QString::fromUtf8("∫_0^1 x"));
            QVERIFY(ok->isEnabled());
        });
        // A program that keeps the markup: OK is on, the markup is what was typed, and no line is given for it.
        QString line = QStringLiteral("seen");
        QByteArray markup;
        withDialog([&] { QVERIFY(jp::askEquationMarkup(nullptr, QStringLiteral("x"), &line, &markup)); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            auto *syntax = d->findChild<QComboBox *>(QStringLiteral("equationSyntax"));
            syntax->setCurrentIndex(1);
            Q_EMIT syntax->activated(1);
            d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->setText(typed);
            QVERIFY(d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->isEnabled());
        });
        QCOMPARE(markup, made);
        QVERIFY(line.isEmpty());
    }

    // The form chosen (Professional or Linear) is given back, and the one given is the one the dialog starts in.
    void theDialogGivesBackTheFormChosen()
    {
        QString line = QStringLiteral("x_i");
        QByteArray markup;
        jp::EquationForm form = jp::EquationForm::Professional;
        withDialog([&] { QVERIFY(jp::askEquationMarkup(nullptr, QStringLiteral("x"), &line, &markup, &form)); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QVERIFY(d->findChild<QRadioButton *>(QStringLiteral("equationProfessional"))->isChecked());
            d->findChild<QRadioButton *>(QStringLiteral("equationLinear"))->setChecked(true);
        });
        QVERIFY(form == jp::EquationForm::Linear);
        withDialog([&] { QVERIFY(jp::askEquationMarkup(nullptr, QStringLiteral("x"), &line, &markup, &form)); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QVERIFY(d->findChild<QRadioButton *>(QStringLiteral("equationLinear"))->isChecked());
            QVERIFY(d->findChild<QLabel *>(QStringLiteral("equationDrawn"))->isHidden());
            d->findChild<QRadioButton *>(QStringLiteral("equationProfessional"))->setChecked(true);
        });
        QVERIFY(form == jp::EquationForm::Professional);
    }

    // A built-in equation that can't be printed in the format being typed in is put in the line that says it, and the box shows that.
    void aGalleryEquationThatCantBePrintedIsTypedInTheLine()
    {
        const QString big = QString(400, QChar(0x3B1));   // (400 letters in the line, 2,400 characters in LaTeX, which writes each as \alpha)
        QVERIFY(!jp::office::wml::equationMarkup(big).isEmpty());
        QVERIFY(jp::office::wml::equationToLatex(jp::office::wml::equationMarkup(big)).isEmpty());
        QString line;
        QByteArray markup;
        withDialog([&] { QVERIFY(jp::askEquationMarkup(nullptr, QStringLiteral("x"), &line, &markup)); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            auto *syntax = d->findChild<QComboBox *>(QStringLiteral("equationSyntax"));
            auto *input = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            auto *designs = d->findChild<QComboBox *>(QStringLiteral("equationGallery"));
            syntax->setCurrentIndex(2);
            Q_EMIT syntax->activated(2);
            designs->setItemData(1, big);   // (forced: a design the format can't write)
            designs->setCurrentIndex(1);
            Q_EMIT designs->activated(1);
            QCOMPARE(syntax->currentIndex(), 0);
            QCOMPARE(input->text(), big);
            QVERIFY(d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->isEnabled());
            // ... and one it can write is written in it.
            input->clear();
            syntax->setCurrentIndex(2);
            Q_EMIT syntax->activated(2);
            designs->setCurrentIndex(2);
            Q_EMIT designs->activated(2);
            QCOMPARE(syntax->currentIndex(), 2);
            QVERIFY(input->text().contains(QLatin1Char('^')));
        });
        QVERIFY(!markup.isEmpty());
    }

    // Text made of the characters the syntaxes use, in no sense at all: the parsers never break, the markup is well formed, and whatever prints reads back to the same print.
    void randomTextNeverBreaksTheParsersAndPrintsStably()
    {
        using namespace jp::office::wml;
        quint32 seed = 7;
        auto next = [&](int n) {
            seed = seed * 1664525u + 1013904223u;
            return int((seed >> 16) % quint32(n));
        };
        const QStringList uniPieces = {QStringLiteral("a"), QStringLiteral("x"), QStringLiteral("2"), QStringLiteral("1.5"), QStringLiteral("+"), QStringLiteral("="), QStringLiteral("/"), QStringLiteral("^"),
                                       QStringLiteral("_"), QStringLiteral("("), QStringLiteral(")"), QStringLiteral("["), QStringLiteral("]"), QStringLiteral("{"), QStringLiteral("}"), QStringLiteral("|"),
                                       QStringLiteral("&"), QStringLiteral("@"), QStringLiteral("#"), QStringLiteral(" "), QStringLiteral("\""), QStringLiteral("\\"), QStringLiteral("\\ "), QStringLiteral("\\alpha"),
                                       QStringLiteral("\\matrix("), QStringLiteral("\\hat"), QStringLiteral("\\overline"), QStringLiteral("\\sqrt"), QStringLiteral("sin"), QStringLiteral("lim"),
                                       QString::fromUtf8("√"), QString::fromUtf8("∛"), QString::fromUtf8("∑"), QString::fromUtf8("∫"), QString::fromUtf8("├"), QString::fromUtf8("┤"), QString::fromUtf8("〖"),
                                       QString::fromUtf8("〗"), QString::fromUtf8("■"), QString::fromUtf8("̂"), QString::fromUtf8("π"), QStringLiteral("\\limits"), QStringLiteral("\\nolimits")};
        const QStringList texPieces = {QStringLiteral("a"), QStringLiteral("x"), QStringLiteral("2"), QStringLiteral("+"), QStringLiteral("{"), QStringLiteral("}"), QStringLiteral("^"), QStringLiteral("_"),
                                       QStringLiteral("&"), QStringLiteral(" "), QStringLiteral("\\\\"), QStringLiteral("\\frac"), QStringLiteral("\\sqrt"), QStringLiteral("["), QStringLiteral("]"),
                                       QStringLiteral("\\left"), QStringLiteral("\\right"), QStringLiteral("\\middle"), QStringLiteral("("), QStringLiteral(")"), QStringLiteral("|"), QStringLiteral("."),
                                       QStringLiteral("\\sum"), QStringLiteral("\\int"), QStringLiteral("\\limits"), QStringLiteral("\\sin"), QStringLiteral("\\lim"), QStringLiteral("\\hat"),
                                       QStringLiteral("\\text{"), QStringLiteral("\\mathrm{"), QStringLiteral("\\begin{matrix}"), QStringLiteral("\\end{matrix}"), QStringLiteral("\\alpha"),
                                       QStringLiteral("\\overline"), QStringLiteral("\\operatorname{"), QStringLiteral("\\{"), QStringLiteral("\\,"), QStringLiteral("~")};
        int built = 0, printed = 0;
        for (int round = 0; round < 6000; ++round) {
            const bool uni = round % 2 == 0;
            const QStringList &pieces = uni ? uniPieces : texPieces;
            QString text;
            for (int n = 1 + next(14); n > 0; --n) text += pieces[next(int(pieces.size()))];
            const QByteArray m = uni ? um(text) : tex(text);
            if (m.isEmpty()) continue;
            ++built;
            XmlTree t;
            QVERIFY2(t.parse(m), qPrintable(text));
            for (int syntax = 0; syntax < 2; ++syntax) {
                const QString u1 = syntax == 0 ? equationToUnicodeMath(m) : equationToLatex(m);
                if (u1.isEmpty()) continue;
                ++printed;
                const QByteArray m2 = syntax == 0 ? um(u1) : tex(u1);
                QVERIFY2(!m2.isEmpty(), qPrintable(text + QStringLiteral(" -> ") + u1));
                QVERIFY2((syntax == 0 ? equationToUnicodeMath(m2) : equationToLatex(m2)) == u1, qPrintable(QString::number(syntax) + QStringLiteral(" [") + text + QStringLiteral("] -> [") + u1 + QStringLiteral("] -> [") + (syntax == 0 ? equationToUnicodeMath(m2) : equationToLatex(m2)) + QLatin1Char(']')));
            }
        }
        QVERIFY2(built > 1500 && printed > 1500, qPrintable(QString::number(built) + QLatin1Char(' ') + QString::number(printed)));
    }
    // The dialog's choices: the equation typed in our line (the first), UnicodeMath, or LaTeX, shown built up (Professional) or only as typed (Linear). A new format writes what is
    // typed again in it, an equation the line has no way to say is not taken by a program that only keeps lines, and the markup is what a program that keeps it is given.
    void theEquationDialogHasProfessionalAndLinearAndTheThreeFormats()
    {
        QString line = QStringLiteral("x_i");
        QByteArray markup;
        withDialog([&] { QVERIFY(jp::askEquationMarkup(nullptr, QStringLiteral("x"), &line, &markup)); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            auto *syntax = d->findChild<QComboBox *>(QStringLiteral("equationSyntax"));
            auto *input = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            auto *preview = d->findChild<QLabel *>(QStringLiteral("equationPreview"));
            auto *drawn = d->findChild<QLabel *>(QStringLiteral("equationDrawn"));
            auto *professional = d->findChild<QRadioButton *>(QStringLiteral("equationProfessional"));
            auto *linearForm = d->findChild<QRadioButton *>(QStringLiteral("equationLinear"));
            QVERIFY(syntax && input && preview && drawn && professional && linearForm);
            QCOMPARE(syntax->currentIndex(), 0);   // (our line is what it was)
            QCOMPARE(syntax->count(), 3);
            QVERIFY(professional->isChecked() && !drawn->isHidden());
            input->setText(QStringLiteral("a/b"));
            QCOMPARE(preview->text(), QStringLiteral("a/b"));
            // UnicodeMath: the line is written again, and an equation the line can't say is taken (the markup is what is kept).
            syntax->setCurrentIndex(1);
            Q_EMIT syntax->activated(1);
            QCOMPARE(input->text(), QStringLiteral("a/b"));
            input->setText(QStringLiteral("sin x"));
            QVERIFY(d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->isEnabled());
            syntax->setCurrentIndex(2);
            Q_EMIT syntax->activated(2);
            QCOMPARE(input->text(), QStringLiteral("\\sin x"));
            // Linear shows no drawing, Professional shows it again.
            linearForm->setChecked(true);
            QVERIFY(drawn->isHidden());
            professional->setChecked(true);
            QVERIFY(!drawn->isHidden());
            // A format the typed equation can't be written in leaves the text and the format as they were.
            input->setText(QStringLiteral("\\text{if}"));
            syntax->setCurrentIndex(0);
            Q_EMIT syntax->activated(0);
            QCOMPARE(input->text(), QStringLiteral("\"if\""));
            syntax->setCurrentIndex(1);
            Q_EMIT syntax->activated(1);
            input->setText(QStringLiteral("sin x"));
            syntax->setCurrentIndex(0);
            Q_EMIT syntax->activated(0);
            QCOMPARE(syntax->currentIndex(), 1);
            QCOMPARE(input->text(), QStringLiteral("sin x"));
            QVERIFY(preview->text().contains(QStringLiteral("can't be written")));
            input->setText(QStringLiteral("a/b"));
            syntax->setCurrentIndex(0);
            Q_EMIT syntax->activated(0);
            QCOMPARE(input->text(), QStringLiteral("{a}/{b}"));
        });
        QVERIFY(markup.contains("<m:f>"));
        QCOMPARE(line, QStringLiteral("{a}/{b}"));
        // A program that keeps only lines can't take an equation with none (a function): its OK is off until the equation has one.
        QString onlyLine;
        withDialog([&] { jp::askEquation(nullptr, QStringLiteral("x"), &onlyLine); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            auto *syntax = d->findChild<QComboBox *>(QStringLiteral("equationSyntax"));
            auto *input = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            auto *ok = d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok);
            syntax->setCurrentIndex(1);
            Q_EMIT syntax->activated(1);
            input->setText(QStringLiteral("sin x"));
            QVERIFY(!ok->isEnabled());
            input->setText(QStringLiteral("x^2"));
            QVERIFY(ok->isEnabled());
        });
        QCOMPARE(onlyLine, QStringLiteral("{x}^{2}"));
    }

    // For the pictures of the dialog (JO_SHOT_DIR names the folder; the test does nothing without it).
    void theEquationDialogPicture()
    {
        if (qEnvironmentVariableIsEmpty("JO_SHOT_DIR")) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        QString line = QStringLiteral("x=(-b\\pm\\sqrt(b^2-4ac))/(2a)");
        QByteArray markup;
        withDialog([&] { jp::askEquationMarkup(nullptr, QStringLiteral("Insert Equation"), &line, &markup); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            d->resize(560, d->sizeHint().height());
            QTest::qWait(50);
            shotOf(d, QStringLiteral("equation-dialog-professional"));
            d->findChild<QRadioButton *>(QStringLiteral("equationLinear"))->setChecked(true);
            auto *syntax = d->findChild<QComboBox *>(QStringLiteral("equationSyntax"));
            syntax->setCurrentIndex(1);
            Q_EMIT syntax->activated(1);
            QTest::qWait(50);
            shotOf(d, QStringLiteral("equation-dialog-linear-unicodemath"));
        });
    }

    // ---- The Equation tab (libs/shell/app/equationtab.h) ----

    static QString textIn(const jp::EquationVariant &v, jp::EquationSyntax syntax)
    {
        return jp::equationUnmarked(syntax == jp::EquationSyntax::UnicodeMath ? v.unicode : syntax == jp::EquationSyntax::Latex ? v.latex : v.line);
    }
    static QByteArray markupIn(const QString &text, jp::EquationSyntax syntax)
    {
        return syntax == jp::EquationSyntax::UnicodeMath ? jp::office::wml::equationFromUnicodeMath(text) : syntax == jp::EquationSyntax::Latex ? jp::office::wml::equationFromLatex(text)
                                                                                                                                            : jp::office::wml::equationMarkup(text);
    }
    // Markup another program wrote that the line of our own syntax can't say but UnicodeMath can (a paragraph of one equation, or a function with its argument).
    static QByteArray markupNoLineMakes()
    {
        using namespace jp::office::wml;
        const QByteArray para = "<m:oMathPara xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><m:oMathParaPr><m:jc m:val=\"center\"/></m:oMathParaPr>" + xmlMath("<m:r><m:t>y</m:t></m:r>") + "</m:oMathPara>";
        const QByteArray func = xmlMath("<m:func><m:fName><m:r><m:rPr><m:sty m:val=\"p\"/></m:rPr><m:t>sin</m:t></m:r></m:fName><m:e><m:r><m:t>x</m:t></m:r></m:e></m:func>");
        for (const QByteArray &m : {func, para})
            if (equationLinear(m).isEmpty() && !equationToUnicodeMath(m).isEmpty()) return m;
        return QByteArray();
    }
    static bool tabShown(jp::MainWindow &w)
    {
        for (int i = 0; i < w.ribbon()->tabCount(); ++i)
            if (w.ribbon()->tabName(i) == QLatin1String("Equation")) return w.ribbon()->tabVisible(i);
        return false;
    }

    // Each of the eleven structures puts in what its first variant says, and that reads as the equation named.
    void theElevenStructuresMakeTheEquationsTheTabSays()
    {
        using jp::EquationSyntax;
        // (the line the pages show for each: a script is written inline, as it is in the text of a page)
        const QMap<QString, QString> expected = {{"fraction", "a/b"}, {"script", "xn"}, {"radical", "\u221ax"}, {"integral", "\u222b a b x"}, {"operator", "\u2211 (i = 1) n x"}, {"bracket", "(x)"},
                                                 {"function", "sin(x)"}, {"accent", "x^"}, {"limit", "limx\u2192 a y"}, {"op", "a \u00d7 b"}, {"matrix", "(a b; c d)"}};
        QCOMPARE(jp::equationCategories().size(), 11);
        QString mismatches;
        // (the element each category builds, and the ones a variant builds differently)
        const QMap<QString, QString> builtBy = {{"fraction", "<m:f>"}, {"script", "<m:sSup>"}, {"script.sub", "<m:sSub>"}, {"script.supsub", "<m:sSubSup>"}, {"radical", "<m:rad>"}, {"integral", "<m:nary>"}, {"operator", "<m:nary>"},
                                                {"bracket", "<m:d>"}, {"function.sin", "<m:t>sin</m:t>"}, {"function.cos", "<m:t>cos</m:t>"}, {"function.tan", "<m:t>tan</m:t>"}, {"function.log", "<m:t>log</m:t>"}, {"function.ln", "<m:t>ln</m:t>"}, {"accent", "<m:acc>"}, {"accent.over", "<m:bar>"}, {"limit", "<m:limLow>|<m:sSub>"}, {"op", "<m:r>"}, {"matrix", "<m:m>"}};
        for (const jp::EquationCategory &c : jp::equationCategories()) {
            QVERIFY2(expected.contains(c.id), qPrintable(c.id));
            QVERIFY(!c.variants.isEmpty() && !c.name.isEmpty() && !c.icon.isEmpty());
            const QByteArray firstMarkup = markupIn(textIn(c.variants.first(), EquationSyntax::Line), EquationSyntax::Line);
            const QMap<QString, QByteArray> element = {{"fraction", "<m:f>"}, {"script", "<m:sSup>"}, {"radical", "<m:rad>"}, {"integral", "<m:nary>"}, {"operator", "<m:nary>"}, {"bracket", "<m:d>"}, {"accent", "<m:acc>"}, {"accent.over", "<m:bar>"}, {"matrix", "<m:m>"}};
            if (element.contains(c.id)) QVERIFY2(firstMarkup.contains(element.value(c.id)), qPrintable(c.id));
            const QString made = jp::office::wml::equationLine(firstMarkup);
            if (made != expected.value(c.id)) mismatches += c.id + ": " + made + "\n";
            for (const jp::EquationVariant &v : c.variants) {
                QByteArray first;
                for (const EquationSyntax syntax : {EquationSyntax::Line, EquationSyntax::UnicodeMath, EquationSyntax::Latex}) {
                    const QString text = textIn(v, syntax);
                    const QByteArray m = markupIn(text, syntax);
                    QVERIFY2(!m.isEmpty(), qPrintable(c.id + "." + v.id + ": " + text));
                    // (the structure it builds, not only the text it reads as)
                    const QString key = c.id + QLatin1Char('.') + v.id;
                    const QString wanted = builtBy.contains(key) ? builtBy.value(key) : builtBy.value(c.id);
                    bool built = false;
                    for (const QString &one : wanted.split(QLatin1Char('|'), Qt::SkipEmptyParts)) built = built || m.contains(one.toLatin1());   // ("lim" is a script in one format, a limit in another)
                    QVERIFY2(built, qPrintable(key + " should hold " + wanted + " in " + QString::fromUtf8(m)));
                    // (the same equation in every format, whatever each one calls it: only a space can differ, as in sin(x) and sin (x))
                    if (first.isEmpty()) first = m;
                    QCOMPARE(QString(jp::office::wml::equationLine(m)).remove(QLatin1Char(' ')), QString(jp::office::wml::equationLine(first)).remove(QLatin1Char(' ')));
                    // (and it reads back from the markup in the format)
                    const QString back = syntax == EquationSyntax::UnicodeMath ? jp::office::wml::equationToUnicodeMath(m) : syntax == EquationSyntax::Latex ? jp::office::wml::equationToLatex(m) : jp::office::wml::equationLinear(m);
                    QVERIFY2(!back.isEmpty(), qPrintable(c.id + "." + v.id + ": " + text));
                    QCOMPARE(markupIn(back, syntax), m);
                }
            }
        }
        QVERIFY2(mismatches.isEmpty(), qPrintable(mismatches));
    }

    // A symbol is one character, and reads as just that in every format.
    void everySymbolOfTheTabIsOneCharacterInEveryFormat()
    {
        using jp::EquationSyntax;
        int count = 0;
        for (const jp::EquationSymbolGroup &g : jp::equationSymbolGroups())
            for (int i = 0; i < g.symbols.size(); ++i) {
                QString symbol(g.symbols[i]);
                if (g.symbols[i].isHighSurrogate()) symbol += g.symbols[++i];
                ++count;
                for (const EquationSyntax syntax : {EquationSyntax::Line, EquationSyntax::UnicodeMath, EquationSyntax::Latex}) {
                    const QString put = jp::equationPickText(QStringLiteral("symbol:") + symbol, syntax);
                    QCOMPARE(put, symbol);
                    QVERIFY2(markupIn(put, syntax).size() > 0, qPrintable(symbol));
                    QCOMPARE(jp::office::wml::equationLine(markupIn(put, syntax)), symbol);
                }
            }
        QVERIFY(count > 60);
        QVERIFY(jp::equationPickText(QStringLiteral("symbol:ab"), jp::EquationSyntax::Line).isEmpty());   // (a pick that names two characters is none)
        QVERIFY(jp::equationPickText(QStringLiteral("structure:fraction.nothing"), jp::EquationSyntax::Line).isEmpty());
        QVERIFY(jp::equationPickText(QStringLiteral("nothing"), jp::EquationSyntax::Line).isEmpty());
    }

    // A structure goes in the line at the caret; what is selected becomes its first place to fill, and the first place is selected for typing over.
    void aStructureGoesInTheLineAtTheCaretAndSelectsItsFirstPlace()
    {
        QLineEdit line;
        line.setText(QStringLiteral("1+"));
        line.setCursorPosition(2);
        jp::insertIntoEquationLine(&line, jp::equationPickText(QStringLiteral("structure:fraction.stacked"), jp::EquationSyntax::Line));
        QCOMPARE(line.text(), QStringLiteral("1+a/b"));
        QCOMPARE(line.selectedText(), QStringLiteral("a"));
        QCOMPARE(line.selectionStart(), 2);
        // (a selection is the first place)
        line.setText(QStringLiteral("x+y"));
        line.setSelection(2, 1);
        jp::insertIntoEquationLine(&line, jp::equationPickText(QStringLiteral("structure:radical.sqrt"), jp::EquationSyntax::Line));
        QCOMPARE(line.text(), QStringLiteral("x+sqrt(y)"));
        QCOMPARE(line.selectedText(), QStringLiteral("y"));
        // (a symbol has no place to fill: the caret goes after it)
        line.setText(QStringLiteral("2"));
        line.setCursorPosition(1);
        jp::insertIntoEquationLine(&line, jp::equationPickText(QStringLiteral("symbol:\u03c0"), jp::EquationSyntax::Line));
        QCOMPARE(line.text(), QStringLiteral("2\u03c0"));
        QCOMPARE(line.cursorPosition(), 2);
        QVERIFY(!line.hasSelectedText());
        // Past the limit of an equation's line nothing is put in.
        line.setText(QString(jp::office::wml::kMostEquationInput, QLatin1Char('x')));
        line.setCursorPosition(1);
        jp::insertIntoEquationLine(&line, jp::equationPickText(QStringLiteral("structure:fraction.stacked"), jp::EquationSyntax::Line));
        QCOMPARE(line.text().size(), jp::office::wml::kMostEquationInput);
    }

    // The dialog has the same structures in a strip, and one picked on the tab is in the line when it opens.
    void theDialogHasTheStructureStripAndTakesAPickFromTheTab()
    {
        QString line;
        QByteArray markup;
        withDialog([&] { QVERIFY(jp::askEquationMarkup(nullptr, QStringLiteral("x"), &line, &markup, nullptr, QStringLiteral("structure:radical.sqrt"))); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            auto *input = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            QCOMPARE(input->text(), QStringLiteral("sqrt(x)"));
            QCOMPARE(input->selectedText(), QStringLiteral("x"));
            for (const jp::EquationCategory &c : jp::equationCategories()) QVERIFY2(d->findChild<QToolButton *>(QStringLiteral("equationStructure_") + c.id), qPrintable(c.id));
            QVERIFY(d->findChild<QToolButton *>(QStringLiteral("equationSymbols")));
            // The x is the first place of the next structure.
            d->findChild<QToolButton *>(QStringLiteral("equationStructure_script"))->click();
            QCOMPARE(input->text(), QStringLiteral("sqrt(x^n)"));
            QCOMPARE(d->findChild<QLabel *>(QStringLiteral("equationPreview"))->text(), jp::office::wml::equationLine(jp::office::wml::equationMarkup(QStringLiteral("sqrt(x^n)"))));
            // Normal Text is in quotes in the line, and the typing format changes what is put in.
            input->setText(QString());
            d->findChild<QToolButton *>(QStringLiteral("equationNormalText"))->click();
            QCOMPARE(input->text(), QStringLiteral("\"text\""));
            auto *syntax = d->findChild<QComboBox *>(QStringLiteral("equationSyntax"));
            input->setText(QString());
            syntax->setCurrentIndex(2);
            Q_EMIT syntax->activated(2);
            d->findChild<QToolButton *>(QStringLiteral("equationStructure_fraction"))->click();
            QCOMPARE(input->text(), QStringLiteral("\\frac{a}{b}"));
            input->setText(QStringLiteral("x"));
            input->setCursorPosition(1);
        });
        QCOMPARE(markup, jp::office::wml::equationMarkup(QStringLiteral("x")));
    }

    // The dialog opens in the typing format chosen on the tab, and an equation no line of ours makes opens in a format that can write it.
    void theDialogOpensInTheFormatTheTabChose()
    {
        const jp::EquationSyntax before = jp::equationSyntaxSetting();
        const auto restore = qScopeGuard([&] { jp::setEquationSyntaxSetting(before); });
        const QByteArray markup = jp::office::wml::equationMarkup(QStringLiteral("a/b"));
        jp::setEquationSyntaxSetting(jp::EquationSyntax::Latex);
        QString line = QStringLiteral("a/b");
        QByteArray changed = markup;
        withDialog([&] { QVERIFY(jp::askEquationMarkup(nullptr, QStringLiteral("x"), &line, &changed)); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("equationSyntax"))->currentIndex(), 2);
            QCOMPARE(d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->text(), QStringLiteral("\\frac{a}{b}"));
        });
        QCOMPARE(changed, markup);
        // (a new equation starts empty, in that format)
        line.clear();
        changed.clear();
        withDialog([&] { jp::askEquationMarkup(nullptr, QStringLiteral("x"), &line, &changed); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("equationSyntax"))->currentIndex(), 2);
            QVERIFY(d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->text().isEmpty());
        }, false);
        // Markup that has no line of ours is written in UnicodeMath, the first format that can.
        jp::setEquationSyntaxSetting(jp::EquationSyntax::Line);
        const QByteArray limits = markupNoLineMakes();
        QVERIFY(!limits.isEmpty() && jp::office::wml::equationLinear(limits).isEmpty());
        line.clear();
        changed = limits;
        withDialog([&] { QVERIFY(jp::askEquationMarkup(nullptr, QStringLiteral("x"), &line, &changed)); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("equationSyntax"))->currentIndex(), 1);
            QVERIFY(!d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->text().isEmpty());
        });
        QCOMPARE(changed, limits);
    }

    // The tab shows exactly while an equation is selected, and its choices follow the equation.
    void theEquationTabShowsExactlyWhileAnEquationIsSelected()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        QVERIFY(!tabShown(w));
        const QString id = jp::insertEquation(ed, QStringLiteral("a/b"));
        QVERIFY(!id.isEmpty());
        ed->select(id);
        QTRY_VERIFY(tabShown(w));
        QVERIFY(w.act(QStringLiteral("equation.professional"))->isChecked() && !w.act(QStringLiteral("equation.linear"))->isChecked());
        ed->select(QString());
        QTRY_VERIFY(!tabShown(w));
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(50, 50, 100, 40);
        const QString text = ed->addItem(box);
        QVERIFY(!text.isEmpty());
        ed->select(text);
        QTRY_VERIFY(!tabShown(w));
        ed->select(id);
        QTRY_VERIFY(tabShown(w));
        ed->deleteSelection();
        QTRY_VERIFY(!tabShown(w));
        QVERIFY(w.act(QStringLiteral("equation.linear")));   // (the commands stay, for the tab and for the command search)
    }

    // Professional and Linear change how the equation is shown and never what it is; the form is kept in the file, and undoes.
    void professionalAndLinearChangeTheFormAndNotTheMarkup()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        const jp::EquationSyntax before = jp::equationSyntaxSetting();
        const bool linearBefore = jp::equationLinearSetting();
        const auto restore = qScopeGuard([&] {
            jp::setEquationSyntaxSetting(before);
            jp::setEquationLinearSetting(linearBefore);
        });
        const QString id = jp::insertEquation(ed, QStringLiteral("(a+b)/c"));
        auto eq = [&] { return dynamic_cast<jp::EquationItem *>(ed->doc()->item(id)); };
        ed->select(id);
        const QByteArray markup = eq()->markup;
        QSizeF professionalSize;
        jp::Renderer::equationOutline(*eq(), &professionalSize);
        w.act(QStringLiteral("equation.linear"))->trigger();
        QVERIFY(eq()->linearForm);
        QCOMPARE(eq()->markup, markup);
        QCOMPARE(eq()->linear, QStringLiteral("(a+b)/c"));
        QSizeF linearSize;
        jp::Renderer::equationOutline(*eq(), &linearSize);
        QVERIFY(linearSize != professionalSize);   // (it is drawn as a line)
        QVERIFY(linearSize.height() < professionalSize.height());
        QTRY_VERIFY(w.act(QStringLiteral("equation.linear"))->isChecked() && !w.act(QStringLiteral("equation.professional"))->isChecked());
        // It is kept in the file.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QJsonObject json = eq()->toJson();
        QVERIFY(json.value("linearForm").toBool());
        jp::EquationItem again;
        again.fromJson(json);
        QVERIFY(again.linearForm);
        QCOMPARE(again.markup, markup);
        // Back: the same markup, the same size, and one step undoes each change.
        w.act(QStringLiteral("equation.professional"))->trigger();
        QVERIFY(!eq()->linearForm);
        QCOMPARE(eq()->markup, markup);
        QCOMPARE(eq()->linear, QStringLiteral("(a+b)/c"));
        QSizeF backSize;
        jp::Renderer::equationOutline(*eq(), &backSize);
        QCOMPARE(backSize, professionalSize);
        ed->undo();
        QVERIFY(eq()->linearForm);
        ed->undo();
        QVERIFY(!eq()->linearForm);
        // The choices of the typing format are the ones the dialogs open in.
        w.act(QStringLiteral("equation.syntaxLatex"))->trigger();
        QCOMPARE(jp::equationSyntaxSetting(), jp::EquationSyntax::Latex);
        QTRY_VERIFY(w.act(QStringLiteral("equation.syntaxLatex"))->isChecked() && !w.act(QStringLiteral("equation.syntaxLine"))->isChecked());
        w.act(QStringLiteral("equation.syntaxLine"))->trigger();
        QCOMPARE(jp::equationSyntaxSetting(), jp::EquationSyntax::Line);
    }

    // A button of the tab opens the selected equation's dialog with the structure in the line; the markup and the form are kept as the dialog makes them, a file's equation (no line of ours) can be changed,
    // and an old line with quotes is edited as it was stored.
    void aStructureButtonEditsTheSelectedEquation()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        const QString id = jp::insertEquation(ed, QStringLiteral("x"));
        auto eq = [&] { return dynamic_cast<jp::EquationItem *>(ed->doc()->item(id)); };
        ed->select(id);
        withDialog([&] { w.act(QStringLiteral("equation.structure.fraction"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            auto *input = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            QCOMPARE(input->text(), QStringLiteral("xa/b"));
            input->setText(QStringLiteral("x/y"));
        });
        QCOMPARE(eq()->linear, QStringLiteral("x/y"));
        QCOMPARE(eq()->markup, jp::office::wml::equationMarkup(QStringLiteral("x/y")));
        // One picked from a menu of the tab does the same.
        withDialog([&] { w.act(QStringLiteral("equation.pick.script.sub"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QCOMPARE(d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->text(), QStringLiteral("x/yx_i"));
        }, false);
        QCOMPARE(eq()->linear, QStringLiteral("x/y"));   // (Cancel changes nothing)
        // The form chosen in the dialog is kept.
        withDialog([&] { w.act(QStringLiteral("equation.edit"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            d->findChild<QRadioButton *>(QStringLiteral("equationLinear"))->setChecked(true);
        });
        QVERIFY(eq()->linearForm);
        withDialog([&] { w.act(QStringLiteral("equation.edit"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QVERIFY(d->findChild<QRadioButton *>(QStringLiteral("equationLinear"))->isChecked());
            d->findChild<QRadioButton *>(QStringLiteral("equationProfessional"))->setChecked(true);
        });
        QVERIFY(!eq()->linearForm);
        // An equation a file brought, which no line of ours makes, is changed in UnicodeMath, and kept as markup with no line.
        const QByteArray limits = markupNoLineMakes();
        QVERIFY(!limits.isEmpty() && jp::office::wml::equationLinear(limits).isEmpty());
        ed->change(QStringLiteral("Put"), [&] {
            eq()->markup = limits;
            eq()->linear.clear();
        });
        withDialog([&] { w.act(QStringLiteral("equation.edit"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            auto *input = d->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            QVERIFY(!input->text().isEmpty());
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("equationSyntax"))->currentIndex(), 1);
            input->setText(input->text() + QStringLiteral("+1"));
        });
        QVERIFY(eq()->markup != limits);
        QVERIFY(eq()->linear.isEmpty());
        // A line stored with quotes before Normal Text was read means those quotes: it is edited as it was stored, and OK with no change keeps the markup.
        const QByteArray old = jp::office::wml::equationMarkup(QStringLiteral("\\left\"a\\right\""));
        QVERIFY(!old.isEmpty());
        ed->change(QStringLiteral("Put"), [&] {
            eq()->markup = old;
            eq()->linear = QStringLiteral("\"a\"");
        });
        withDialog([&] { w.act(QStringLiteral("equation.edit"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            QCOMPARE(d->findChild<QLineEdit *>(QStringLiteral("equationInput"))->text(), jp::office::wml::equationLineForEditing(QStringLiteral("\"a\""), old));
        });
        QCOMPARE(eq()->markup, old);
    }

    // The window with the Equation tab shown, and the dialog with its strip (JO_SHOT_DIR names the folder; the test does nothing without it).
    void theEquationTabPicture()
    {
        if (qEnvironmentVariableIsEmpty("JO_SHOT_DIR")) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        jp::MainWindow w;
        w.resize(1440, 900);
        w.show();
        const QString id = jp::insertEquation(w.editor(), QStringLiteral("x=(-b\\pm\\sqrt(b^2-4ac))/(2a)"));
        QVERIFY(!id.isEmpty());
        w.editor()->select(id);
        QTRY_VERIFY(tabShown(w));
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Equation")));
        QTest::qWait(200);
        shotOf(&w, QStringLiteral("equation-tab-pub"));
        withDialog([&] { w.act(QStringLiteral("equation.structure.integral"))->trigger(); }, QStringLiteral("equationDialog"), [&](QDialog *d) {
            d->resize(640, d->sizeHint().height());
            QTest::qWait(100);
            shotOf(d, QStringLiteral("equation-dialog-strip"));
        }, false);
    }

    // Text with control characters makes markup that is well formed, and a lone surrogate or a noncharacter is left out.
    void hostileTextMakesWellFormedMarkup()
    {
        using namespace jp::office::wml;
        const QString nasty = QStringLiteral("a\x01 b\x1f ") + QChar(0xD800) + QChar(0xFFFF) + QStringLiteral("\"c\x02\"");
        for (const QByteArray &m : {um(nasty), tex(QStringLiteral("a\x01 b \\text{c\x02}"))}) {
            QVERIFY(!m.isEmpty());
            for (const char c : m) QVERIFY(uchar(c) >= 0x20 || c == '\n' || c == '\t');
            jp::office::wml::XmlTree t;
            QVERIFY(t.parse(m));
        }
        QVERIFY(um(QStringLiteral("\"never closed")).contains("never closed"));
        QVERIFY(tex(QStringLiteral("\\frac{a}{b")).isEmpty());          // a brace that does not close
        QVERIFY(tex(QStringLiteral("a}")).isEmpty());                   // ... or does not open
        QVERIFY(tex(QStringLiteral("\\left( x")).isEmpty());            // brackets with no \right
        QVERIFY(tex(QStringLiteral("\\begin{matrix} a")).isEmpty());   // an environment with no \end
        QVERIFY(tex(QStringLiteral("\\begin{array}{c} a \\end{array}")).isEmpty());   // an environment there is no form for
        QVERIFY(tex(QStringLiteral("\\begin{matrix} a \\end{pmatrix}")).isEmpty());
        QVERIFY(um(QStringLiteral("\\matrix a")).isEmpty());
        QVERIFY(um(QStringLiteral("   ")).isEmpty());
    }
};

JP_TEST_CLASS(PubEquationTests)
#include "tst_pub_equation.moc"

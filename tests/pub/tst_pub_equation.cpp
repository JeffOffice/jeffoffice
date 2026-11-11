// JeffPub's tests of Insert > Equation: the object on the page (inserted, undone, moved, sized, edited), kept in a .jpub, saved as a picture in a .pub, and drawn as vector paths in a PDF.
#include "pubhelpers.h"
#include "../pdfmarkcheck.h"
#include "../withdialog.h"
#include "canvas/canvas.h"
#include "app/equationedit.h"
#include "app/equationdialog.h"
#include "io/pubimport.h"
#include "wml/wmlmathedit.h"
#include "wml/wmlmathpath.h"
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QTemporaryDir>

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

    // The gallery of the program's other equation editors is on the Insert tab, and each design puts an equation on the page that undoes and redoes.
    void insertingAnEquationUndoesAndRedoes()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        QVERIFY(w.act(QStringLiteral("ins.equation")));
        QVERIFY(w.act(QStringLiteral("ins.equation.design.1")));
        QVERIFY(equationsOn(ed).isEmpty());
        const QString line = jp::office::wml::equationGallery().at(1).linear;
        w.act(QStringLiteral("ins.equation.design.1"))->trigger();
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
        QVERIFY2(note.contains(QStringLiteral("equation")) && note.contains(QStringLiteral("picture")), qPrintable(note));
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
        QVERIFY(std::abs(at.width() - rect.width()) < 1.5 && std::abs(at.height() - rect.height()) < 1.5);
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
        QVERIFY2(content.count(QRegularExpression(QStringLiteral("(?m) c$"))) > 20, "no curves in the page");
        QVERIFY2(content.contains(QRegularExpression(QStringLiteral("(?m)^f\\*?$"))), "no filled path");
        QVERIFY(!content.contains(QRegularExpression(QStringLiteral("(?m) Do$"))));
        QVERIFY(!pdf.contains("/Subtype /Image"));
    }
};

JP_TEST_CLASS(PubEquationTests)
#include "tst_pub_equation.moc"

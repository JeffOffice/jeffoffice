// JeffPub's test that a publication is saved as the same bytes in every process, as a .pub file and as JeffPub's own file.
#include "../stablesaves.h"
#include "pubhelpers.h"

namespace {

// A publication with shapes, two stories of text with styles, and a second page.
std::unique_ptr<jp::Document> richPublication()
{
    auto doc = jp::Document::blank(QSizeF(612, 792));
    for (int i = 0; i < 6; ++i) {
        auto box = std::make_shared<jp::ShapeItem>();
        box->rect = QRectF(36 + 40 * i, 72 + 20 * i, 120, 80);
        box->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(40 * i, 90, 200 - 20 * i)));
        doc->pages[0]->items.push_back(box);
    }
    for (int i = 0; i < 3; ++i) {
        auto text = std::make_shared<jp::TextItem>();
        text->rect = QRectF(72, 300 + 100 * i, 400, 80);
        text->storyId = doc->createStory();
        QTextCursor c(doc->storyDoc(text->storyId));
        QTextCharFormat bold;
        bold.setFontWeight(QFont::Bold);
        c.insertText(QStringLiteral("Heading %1").arg(i), bold);
        c.insertBlock(QTextBlockFormat());
        c.insertText(QStringLiteral("Some words for the box number %1, enough to make a sentence.").arg(i));
        doc->pages[0]->items.push_back(text);
    }
    return doc;
}

} // namespace

class PubStableSavesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // Not a test of its own: the work of a process that the next function starts. The publication is saved as a .pub file (in a folder of its own, by a plain name: the file
    // carries the name it is saved under) and the file is copied to where the test looks.
    void savesThePublicationAsPub()
    {
        QString out;
        if (!stable::childOutput(&out)) return;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString before = QDir::currentPath(); // put back below: the results file of the run is named from where the run began
        QVERIFY(QDir::setCurrent(dir.path()));
        QString err;
        const bool saved = jp::exportPublisher(*richPublication(), QStringLiteral("saved.pub"), &err);
        QVERIFY(QDir::setCurrent(before));
        QVERIFY2(saved, qPrintable(err));
        QFile in(dir.filePath(QStringLiteral("saved.pub")));
        QVERIFY(in.open(QIODevice::ReadOnly));
        QFile f(out);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(in.readAll());
    }

    // The file's tables carry a tag: made from the text, not at random, so that the same publication is the same bytes. (JeffPub's own file holds the ids that items are given,
    // new in each process, and is compared by tools/check_stable_saves.py after one reading.)
    // The same publication saved twice in one process, with nothing changed between, is the same bytes (a .pub file is not a ZIP: it is compared whole).
    void aPubFileSavedTwiceIsTheSameBytes()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString before = QDir::currentPath();
        const auto restore = qScopeGuard([before] { QDir::setCurrent(before); });
        const auto doc = richPublication();
        QString err;
        QByteArray files[2];
        for (int i = 0; i < 2; ++i) {
            // (the file carries the name it is saved under: the same name, in a folder of its own for each)
            QVERIFY(QDir().mkpath(dir.filePath(QString::number(i))));
            QVERIFY(QDir::setCurrent(dir.filePath(QString::number(i))));
            const QString name = QStringLiteral("saved.pub");
            QVERIFY2(jp::exportPublisher(*doc, name, &err), qPrintable(err));
            QFile in(dir.filePath(QString::number(i) + QStringLiteral("/") + name));
            QVERIFY(in.open(QIODevice::ReadOnly));
            files[i] = in.readAll();
        }
        QCOMPARE(files[1].size(), files[0].size());
        QVERIFY(files[1] == files[0]);
    }

    void aPubFileIsSavedAsTheSameBytesInEachProcess() { JP_VERIFY_SAME_SAVES("savesThePublicationAsPub"); }
};

JP_TEST_CLASS(PubStableSavesTests)
#include "tst_pub_stablesaves.moc"

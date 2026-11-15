// Replace Fonts, the parts every app shares: counting and swapping the fonts a QTextDocument's runs name, the same over a document of placed objects (text frames, shapes, notes, master
// pages, styles, and Text Art), and the box that asks which font to put in the place of which.
#include "app/editor.h"
#include "app/pagefonts.h"
#include "app/replacefonts.h"
#include "core/document.h"
#include "core/items.h"
#include "text/fontreplace.h"
#include "text/textprops.h"
#include "../testrunner.h"
#include "../withdialog.h"

#include <QComboBox>
#include <QPushButton>
#include <QTextCursor>
#include <QTextDocument>
#include <QtTest>

using namespace jp;

namespace {

// A document with these runs: each is (text, family or empty, "major" or "minor" for a run that follows the theme).
void fill(QTextDocument *doc, const QVector<std::tuple<QString, QString, QString>> &runs)
{
    QTextCursor c(doc);
    for (const auto &[text, family, theme] : runs) {
        QTextCharFormat f;
        if (!family.isEmpty()) f.setFontFamilies(QStringList{family});
        if (!theme.isEmpty()) f.setProperty(tp::ThemeFont, theme);
        c.insertText(text, f);
    }
}

QString familyAt(QTextDocument *doc, int position)
{
    QTextCursor c(doc);
    c.setPosition(position + 1);
    return fontFamiliesOf(c.charFormat()).value(0);
}

} // namespace

class ShellReplaceFontsTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void textRunsAreCountedAndSwappedKeepingTheRestOfTheirLook()
    {
        QTextDocument doc;
        fill(&doc, {{QStringLiteral("aaa"), QStringLiteral("Arial"), {}}, {QStringLiteral("bbb"), QStringLiteral("arial"), {}}, {QStringLiteral("ccc"), QStringLiteral("Georgia"), {}},
                    {QStringLiteral("ddd"), {}, QStringLiteral("major")}, {QStringLiteral("eee"), {}, QStringLiteral("minor")}});
        QTextCursor bold(&doc);
        bold.setPosition(0);
        bold.setPosition(3, QTextCursor::KeepAnchor);
        QTextCharFormat b;
        b.setFontWeight(QFont::Bold);
        bold.mergeCharFormat(b);
        QMap<QString, int> uses;
        int major = 0, minor = 0;
        collectFontFamilies(&doc, &uses, &major, &minor);
        QCOMPARE(uses.value(QStringLiteral("Arial")) + uses.value(QStringLiteral("arial")), 2);
        QCOMPARE(uses.value(QStringLiteral("Georgia")), 1);
        QCOMPARE(major, 1);
        QCOMPARE(minor, 1);
        QCOMPARE(replaceFontFamily(&doc, QStringLiteral("ARIAL"), QStringLiteral("Verdana")), 2);
        QCOMPARE(familyAt(&doc, 0), QStringLiteral("Verdana"));
        QCOMPARE(familyAt(&doc, 3), QStringLiteral("Verdana"));
        QCOMPARE(familyAt(&doc, 6), QStringLiteral("Georgia"));
        QTextCursor c(&doc);
        c.setPosition(1);
        QCOMPARE(c.charFormat().fontWeight(), int(QFont::Bold));   // (what else the run said stays)
        // A run that follows the theme is not touched, and nothing is left to change.
        c.setPosition(10);
        QVERIFY(!c.charFormat().hasProperty(QTextFormat::FontFamilies));
        QCOMPARE(replaceFontFamily(&doc, QStringLiteral("Arial"), QStringLiteral("Verdana")), 0);
    }

    void aDocumentOfPlacedObjectsIsSwappedEverywhereAsOneStep()
    {
        Editor ed;
        Document &d = *ed.doc();
        d.fonts.heading = QStringLiteral("Calibri Light");
        d.fonts.body = QStringLiteral("Calibri");
        auto frame = [&](const QString &id, const QVector<std::tuple<QString, QString, QString>> &runs) {
            auto t = std::make_shared<TextItem>();
            t->id = id;
            t->rect = QRectF(20, 20, 200, 50);
            t->storyId = d.createStory();
            fill(d.storyDoc(t->storyId), runs);
            return t;
        };
        d.pages[0]->items.push_back(frame(QStringLiteral("a"), {{QStringLiteral("one"), QStringLiteral("Zephyr Test"), {}}, {QStringLiteral("two"), {}, QStringLiteral("major")}}));
        auto shape = std::make_shared<ShapeItem>();
        shape->id = QStringLiteral("s");
        shape->rect = QRectF(20, 100, 100, 50);
        shape->storyId = d.createStory();
        fill(d.storyDoc(shape->storyId), {{QStringLiteral("in a shape"), QStringLiteral("Zephyr Test"), {}}});
        d.pages[0]->items.push_back(shape);
        auto art = std::make_shared<TextArtItem>();
        art->id = QStringLiteral("art");
        art->rect = QRectF(20, 200, 100, 50);
        art->font = QStringLiteral("Zephyr Test");
        d.pages[0]->items.push_back(art);
        if (d.masters.isEmpty()) d.masters.push_back(std::make_shared<MasterPage>());
        d.masters[0]->items.push_back(frame(QStringLiteral("m"), {{QStringLiteral("on the master"), QStringLiteral("Zephyr Test"), {}}}));
        TextStyle style;
        style.name = QStringLiteral("Loud");
        style.chr.setFontFamilies(QStringList{QStringLiteral("Zephyr Test")});
        d.styles.push_back(style);

        QVector<FontUse> used = pageFontsUsed(d);
        int arial = 0, heading = 0;
        for (const FontUse &u : used) {
            if (u.key == QLatin1String("Zephyr Test")) arial = u.uses;
            if (u.key == QLatin1String("+Heading")) heading = u.uses;
        }
        QCOMPARE(arial, 5);   // (a frame's run, a shape's, the master's, the style, and the Text Art)
        QCOMPARE(heading, 1);
        QVERIFY(used.first().key == QLatin1String("Zephyr Test"));

        const int steps = ed.undoStack()->count();
        int n = 0;
        ed.change(QStringLiteral("Replace Fonts"), [&] { n = replacePageFont(d, QStringLiteral("Zephyr Test"), QStringLiteral("Zephyr Other")); });
        QCOMPARE(n, 5);
        QCOMPARE(ed.undoStack()->count(), steps + 1);
        QCOMPARE(familyAt(d.storyDoc(static_cast<TextItem *>(d.pages[0]->items[0].get())->storyId), 0), QStringLiteral("Zephyr Other"));
        QCOMPARE(familyAt(d.storyDoc(static_cast<ShapeItem *>(d.pages[0]->items[1].get())->storyId), 0), QStringLiteral("Zephyr Other"));
        QCOMPARE(static_cast<TextArtItem *>(d.pages[0]->items[2].get())->font, QStringLiteral("Zephyr Other"));
        QCOMPARE(familyAt(d.storyDoc(static_cast<TextItem *>(d.masters[0]->items[0].get())->storyId), 0), QStringLiteral("Zephyr Other"));
        QCOMPARE(fontFamiliesOf(d.styles.last().chr).value(0), QStringLiteral("Zephyr Other"));
        // The theme's font is left alone, and is changed only when it is the one picked.
        QCOMPARE(d.fonts.heading, QStringLiteral("Calibri Light"));
        QCOMPARE(replacePageFont(d, QStringLiteral("+Heading"), QStringLiteral("Georgia")), 1);
        QCOMPARE(d.fonts.heading, QStringLiteral("Georgia"));
        QCOMPARE(d.fonts.body, QStringLiteral("Calibri"));
        // Undo puts all of it back at once.
        ed.undo();
        QCOMPARE(ed.undoStack()->count(), steps + 1);
        QCOMPARE(familyAt(ed.doc()->storyDoc(static_cast<TextItem *>(ed.doc()->pages[0]->items[0].get())->storyId), 0), QStringLiteral("Zephyr Test"));
        QCOMPARE(static_cast<TextArtItem *>(ed.doc()->pages[0]->items[2].get())->font, QStringLiteral("Zephyr Test"));
    }

    void theBoxListsTheFontsUsedAndReplacesOne()
    {
        QVector<FontUse> used = {{QStringLiteral("Arial"), QString(), 3, false}, {QStringLiteral("+Body"), QStringLiteral("Calibri (Body)"), 2, true}};
        QString askedKey, askedTo;
        ReplaceFontsHost host;
        host.fontsUsed = [&] { return used; };
        host.replace = [&](const QString &key, const QString &to) {
            askedKey = key;
            askedTo = to;
            return 3;
        };
        ReplaceFontsResult result;
        withDialog([&] { result = askReplaceFonts(nullptr, host); }, QStringLiteral("replaceFontsDialog"), [&](QDialog *d) {
            auto *from = d->findChild<QComboBox *>(QStringLiteral("replaceFontsFrom"));
            auto *to = d->findChild<QComboBox *>(QStringLiteral("replaceFontsTo"));
            auto *replace = d->findChild<QPushButton *>(QStringLiteral("replaceFontsReplace"));
            QVERIFY(from && to && replace);
            QCOMPARE(from->count(), 2);
            QVERIFY(from->itemText(0).startsWith(QLatin1String("Arial")));
            QVERIFY(from->itemText(1).startsWith(QLatin1String("Calibri (Body)")));
            QVERIFY(!replace->isEnabled());   // (no font chosen to put in its place yet)
            to->setEditText(QStringLiteral("Arial"));
            QVERIFY(!replace->isEnabled());   // (the same font is no change)
            to->setEditText(QStringLiteral("Georgia"));
            QVERIFY(replace->isEnabled());
        });
        QCOMPARE(askedKey, QStringLiteral("Arial"));
        QCOMPARE(askedTo, QStringLiteral("Georgia"));
        QCOMPARE(result.uses, 3);
        QCOMPARE(result.from, QStringLiteral("Arial"));
        QVERIFY(result.message().contains(QLatin1String("Georgia")));
        // Cancelled, nothing is replaced.
        askedKey.clear();
        withDialog([&] { result = askReplaceFonts(nullptr, host); }, QStringLiteral("replaceFontsDialog"), [&](QDialog *d) { d->findChild<QComboBox *>(QStringLiteral("replaceFontsTo"))->setEditText(QStringLiteral("Georgia")); }, false);
        QVERIFY(askedKey.isEmpty());
        QCOMPARE(result.uses, 0);
    }
};

JP_TEST_CLASS(ShellReplaceFontsTests)
#include "tst_shell_replacefonts.moc"

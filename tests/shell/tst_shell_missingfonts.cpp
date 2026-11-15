// The shared part of "missing fonts" (libs/shell/app/missingfonts): finding the fonts a document uses that this computer lacks, the bar and its Details dialog, a stand-in chosen for display
// only, Replace in Document, the choices kept for each font name, and the PDF that uses the same stand-in. A family no computer has stands for the missing font.
#include "app/missingfonts.h"
#include "app/settings.h"
#include "core/fonts.h"
#include "../testrunner.h"
#include "../withdialog.h"

#include <QComboBox>
#include <QDockWidget>
#include <QFontDatabase>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPdfWriter>
#include <QPushButton>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QTextCursor>
#include <QTextTable>
#include <QTextDocument>
#include <QScopeGuard>
#include <QtTest>

using namespace jp;

namespace {

const QString kGone = QStringLiteral("Zzyzx Gothic Display");   // (no computer has it)
const QString kGone2 = QStringLiteral("Quuxbold Script");

QString anInstalledFamily()
{
    for (const QString &f : {QStringLiteral("Arimo"), QStringLiteral("Cousine"), QStringLiteral("Carlito")})
        if (QFontDatabase::hasFamily(f)) return f;
    return QFontDatabase::families().value(0);
}

class ShellMissingFontsTests : public QObject {
    Q_OBJECT
    QString m_stand;
    // The document the test stands for: its text, and a count of how many times it was told to change a font.
    QStringList m_uses{kGone, kGone2, kGone};
    int m_refreshes = 0;
    QString m_replacedFrom, m_replacedTo;

    MissingFontsHost host()
    {
        MissingFontsHost h;
        h.usage = [this] {
            FontUsage u;
            for (int i = 0; i < m_uses.size(); ++i) u.add(m_uses.at(i), QStringLiteral("Slide %1").arg(i + 1));
            return u;
        };
        h.replace = [this](const QString &from, const QString &to) {
            int n = 0;
            for (QString &u : m_uses)
                if (u == from) {
                    u = to;
                    ++n;
                }
            m_replacedFrom = from;
            m_replacedTo = to;
            return n;
        };
        h.refresh = [this] { ++m_refreshes; };
        return h;
    }
    static void forget()
    {
        for (const QString &f : {kGone, kGone2}) setFontChoice(f, QString());
    }

private Q_SLOTS:
    void initTestCase()
    {
        initCore();
        m_stand = anInstalledFamily();
    }
    void init()
    {
        forget();
        m_uses = {kGone, kGone2, kGone};
        m_refreshes = 0;
        m_replacedFrom.clear();
        m_replacedTo.clear();
    }
    void cleanup() { forget(); }

    void aUsageKeepsEachFamilyOnceWithItsPlaces()
    {
        FontUsage u;
        u.add(QStringLiteral("A"), QStringLiteral("Slide 1"));
        u.add(QStringLiteral("B"), QStringLiteral("Slide 1"));
        u.add(QStringLiteral("A"), QStringLiteral("Slide 3"));
        u.add(QStringLiteral("A"), QStringLiteral("Slide 3"));
        u.add(QString(), QStringLiteral("x"));
        QCOMPARE(u.families(), (QStringList{QStringLiteral("A"), QStringLiteral("B")}));
        QCOMPARE(u.places(QStringLiteral("A")), (QStringList{QStringLiteral("Slide 1"), QStringLiteral("Slide 3")}));
    }
    void aDocumentsTextGivesItsFamiliesAndTheDefaultForUnnamedText()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat named;
        named.setFontFamilies({kGone});
        c.insertText(QStringLiteral("named "), named);
        c.insertText(QStringLiteral("plain"), QTextCharFormat());
        QFont own;
        own.setFamily(kGone2);
        doc.setDefaultFont(own);
        FontUsage u;
        u.addDocument(&doc, QStringLiteral("Body"));
        QCOMPARE(u.families(), (QStringList{kGone, kGone2}));
    }
    void onlyTheFontsThisComputerLacksAreMissing()
    {
        FontUsage u;
        u.add(m_stand, QStringLiteral("Body"));
        u.add(kGone, QStringLiteral("Body"));
        const QList<MissingFont> m = missingFonts(u);
        QCOMPARE(m.size(), 1);
        QCOMPARE(m.first().family, kGone);
        QVERIFY(!m.first().inPlace.isEmpty());
        QVERIFY(!m.first().chosen);
        // (a test's own idea of what is installed)
        QCOMPARE(missingFonts(u, [&](const QString &f) { return f == kGone; }).size(), 1);
        QCOMPARE(missingFonts(u, [&](const QString &f) { return f == kGone; }).first().family, m_stand);
    }
    void aChoiceIsKeptForTheFontNameAndPutInForce()
    {
        setFontChoice(kGone, m_stand);
        QCOMPARE(fontChoice(kGone), m_stand);
        QCOMPARE(fontReplacement(kGone), m_stand);
        FontUsage u;
        u.add(kGone, QStringLiteral("Body"));
        QCOMPARE(missingFonts(u).first().inPlace, m_stand);
        QVERIFY(missingFonts(u).first().chosen);
        // a font with a choice draws as an installed one: no stand-in's stretch
        QCOMPARE(substituteStretch(kGone), 100);
        QVERIFY(substituteFor(kGone).isEmpty());
        // taken away
        setFontChoice(kGone, QString());
        QVERIFY(fontChoice(kGone).isEmpty());
        QVERIFY(fontReplacement(kGone).isEmpty());
    }
    void aChoiceMadeBeforeIsAppliedWhenAnotherDocumentIsChecked()
    {
        Settings::get().setValue(QStringLiteral("fonts/replace/") + kGone.toLower(), m_stand);   // (kept from an earlier run)
        QVERIFY(fontReplacement(kGone).isEmpty());
        QMainWindow win;
        checkMissingFonts(&win, host());
        QCOMPARE(fontReplacement(kGone), m_stand);
        QCOMPARE(m_refreshes, 1);   // (the text is drawn again with it)
    }
    void aTextInAMissingFontIsDrawnInTheChoiceAndSoIsThePdf()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto pdfWith = [&](const QString &name) {
            const QString path = dir.filePath(name);
            {
                QPdfWriter pdf(path);
                QPainter p(&pdf);
                QFont f(kGone);
                f.setPointSize(24);
                p.setFont(f);
                p.drawText(200, 600, QStringLiteral("Quick brown fox"));
            }
            QFile file(path);
            file.open(QIODevice::ReadOnly);
            return file.readAll();
        };
        const QByteArray before = pdfWith(QStringLiteral("a.pdf"));
        setFontChoice(kGone, m_stand);
        const QByteArray after = pdfWith(QStringLiteral("b.pdf"));
        QVERIFY2(after.contains(m_stand.toLatin1().replace(' ', '#').constData()) || after.contains(m_stand.toLatin1().constData()), "the PDF names the chosen font");
        QVERIFY(before != after);
    }
    void noBarWhenNothingIsMissingAndABarWhenSomethingIs()
    {
        QMainWindow win;
        win.show();
        m_uses = {m_stand};
        checkMissingFonts(&win, host());
        QDockWidget *dock = win.findChild<QDockWidget *>(QStringLiteral("missingFontsDock"));
        QVERIFY(!dock || !dock->isVisible());
        m_uses = {kGone};
        checkMissingFonts(&win, host());
        dock = win.findChild<QDockWidget *>(QStringLiteral("missingFontsDock"));
        QVERIFY(dock);
        QTRY_VERIFY(dock->isVisible());
        auto *text = dock->findChild<QLabel *>(QStringLiteral("missingFontsText"));
        QVERIFY(text);
        QCOMPARE(text->text(), QStringLiteral("This document uses fonts that aren't on this computer"));
        QVERIFY(dock->findChild<QPushButton *>(QStringLiteral("missingFontsDetails")));
        // a document with none takes the bar away
        m_uses = {m_stand};
        checkMissingFonts(&win, host());
        QTRY_VERIFY(!dock->isVisible());
    }
    void detailsListsEachFontWithItsStandInAndPlaces()
    {
        QMainWindow win;
        win.show();
        checkMissingFonts(&win, host());
        auto *details = win.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"));
        QVERIFY(details);
        withDialog([&] { details->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
            auto *t = d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"));
            QVERIFY(t);
            QCOMPARE(t->rowCount(), 2);
            QCOMPARE(t->item(0, 0)->text(), kGone);
            QCOMPARE(t->item(1, 0)->text(), kGone2);
            QCOMPARE(t->item(0, 2)->text(), QStringLiteral("Slide 1, Slide 3"));
            auto *combo = qobject_cast<QComboBox *>(t->cellWidget(0, 1));
            QVERIFY(combo);
            QVERIFY(combo->itemText(0).contains(QStringLiteral("automatic")));
        });
    }
    void aChoiceInTheDialogIsDisplayOnlyAndRemembered()
    {
        QMainWindow win;
        win.show();
        checkMissingFonts(&win, host());
        auto *details = win.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"));
        withDialog([&] { details->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
            auto *t = d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"));
            auto *combo = qobject_cast<QComboBox *>(t->cellWidget(0, 1));
            combo->showPopup();
            combo->hidePopup();
            const int at = combo->findData(m_stand);
            QVERIFY(at > 0);
            combo->setCurrentIndex(at);
            Q_EMIT combo->activated(at);
        });
        QCOMPARE(fontChoice(kGone), m_stand);
        QCOMPARE(fontReplacement(kGone), m_stand);
        QVERIFY(m_refreshes >= 1);
        QVERIFY(m_replacedFrom.isEmpty());   // (the document was not changed)
        QCOMPARE(m_uses, (QStringList{kGone, kGone2, kGone}));
    }
    void replaceInDocumentChangesTheFileAndTheFontIsNoLongerMissing()
    {
        QMainWindow win;
        win.show();
        checkMissingFonts(&win, host());
        auto *details = win.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"));
        withDialog([&] { details->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
            auto *t = d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"));
            auto *combo = qobject_cast<QComboBox *>(t->cellWidget(0, 1));
            combo->showPopup();   // (the fonts are put in the box when it is first opened)
            combo->hidePopup();
            combo->setCurrentIndex(combo->findData(m_stand));
            Q_EMIT combo->activated(combo->currentIndex());
            t->selectRow(0);
            auto *replace = d->findChild<QPushButton *>(QStringLiteral("missingFontsReplace"));
            QVERIFY(replace->isEnabled());
            replace->click();
            QCOMPARE(t->rowCount(), 1);   // (the other font is still missing)
            QCOMPARE(t->item(0, 0)->text(), kGone2);
        });
        QCOMPARE(m_replacedFrom, kGone);
        QCOMPARE(m_replacedTo, m_stand);
        QCOMPARE(m_uses, (QStringList{m_stand, kGone2, m_stand}));
    }
    void aThousandFontNamesGiveADialogThatOpensFastWithAtMostTwoHundredRows()
    {
        const QString longName = QString(500, QLatin1Char('x'));
        MissingFontsHost h = host();
        h.usage = [&] {
            FontUsage u;
            u.add(longName, QStringLiteral("Slide 1"));
            for (int i = 0; i < 1000; ++i) u.add(QStringLiteral("Hostile Font %1").arg(i), QStringLiteral("Slide %1").arg(i));
            return u;
        };
        QMainWindow win;
        win.show();
        checkMissingFonts(&win, h);
        QElapsedTimer clock;
        qint64 took = 0;
        withDialog([&] {
            clock.start();
            win.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"))->click();
        }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
            took = clock.elapsed();
            auto *t = d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"));
            QCOMPARE(t->rowCount(), 200);
            QCOMPARE(d->findChild<QLabel *>(QStringLiteral("missingFontsMore"))->text(), QStringLiteral("and 801 more"));
            // a long name is cut (the whole one is in its tip), and the boxes of fonts are not filled until they are opened
            QVERIFY(t->item(0, 0)->text().size() <= 100);
            QCOMPARE(t->item(0, 0)->data(Qt::UserRole).toString(), longName);
            QVERIFY(qobject_cast<QComboBox *>(t->cellWidget(0, 1))->count() <= 2);
        });
        QVERIFY2(took < 1000, qPrintable(QStringLiteral("%1 ms").arg(took)));
    }
    void aFontCountsAsPresentWhenAnyFontOfItsListIsInstalled()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat both, only;
        both.setFontFamilies({kGone, m_stand});
        only.setFontFamilies({kGone2});
        c.insertText(QStringLiteral("a "), both);
        c.insertText(QStringLiteral("b"), only);
        FontUsage u;
        u.addDocument(&doc, QStringLiteral("Body"));
        QCOMPARE(u.families(), QStringList{kGone2});   // (the first run is drawn in the installed font of its list)
    }
    void theFontsOfParagraphMarksAndTableCellsAreLookedAtToo()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat mark;
        mark.setFontFamilies({kGone});
        c.setBlockCharFormat(mark);
        c.insertText(QStringLiteral("x"), QTextCharFormat());
        QTextTable *table = c.insertTable(1, 1);
        QTextTableCellFormat cell;
        cell.setFontFamilies({kGone2});
        table->cellAt(0, 0).setFormat(cell);
        FontUsage u;
        u.addDocument(&doc, QStringLiteral("Body"));
        QVERIFY(u.families().contains(kGone));
        QVERIFY(u.families().contains(kGone2));
    }
    void manyChoicesAreAppliedWithOneChangeOfTheFontGeneration()
    {
        const QStringList names{QStringLiteral("Zzyzx A"), QStringLiteral("Zzyzx B"), QStringLiteral("Zzyzx C")};
        for (const QString &n : names) Settings::get().setValue(QStringLiteral("fonts/replace/") + n.toLower(), m_stand);
        const auto cleanup = qScopeGuard([&] {
            for (const QString &n : names) setFontChoice(n, QString());
        });
        const int before = fontGeneration();
        QVERIFY(applyFontChoices(names));
        QCOMPARE(fontGeneration(), before + 1);
        for (const QString &n : names) QCOMPARE(fontReplacement(n), m_stand);
    }
    void aReplaceThatChangedNothingSaysSoAndNeverClaimsSuccess()
    {
        MissingFontsHost h = host();
        h.replace = [](const QString &, const QString &) { return 0; };
        QMainWindow win;
        win.show();
        checkMissingFonts(&win, h);
        withDialog([&] { win.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"))->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
            d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"))->selectRow(0);
            d->findChild<QPushButton *>(QStringLiteral("missingFontsReplace"))->click();
            const QString said = d->findChild<QLabel *>(QStringLiteral("missingFontsNote"))->text();
            QVERIFY2(said.contains(QStringLiteral("nothing was changed")), qPrintable(said));
            QVERIFY(!said.contains(QStringLiteral("is replaced")));
        });
    }
    void aRefusedReplaceChangesNothingAndSaysSo()
    {
        MissingFontsHost h = host();
        h.replace = [](const QString &, const QString &) { return -1; };
        QMainWindow win;
        win.show();
        checkMissingFonts(&win, h);
        auto *details = win.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"));
        withDialog([&] { details->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
            d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"))->selectRow(0);
            d->findChild<QPushButton *>(QStringLiteral("missingFontsReplace"))->click();
            QVERIFY(!d->findChild<QLabel *>(QStringLiteral("missingFontsNote"))->text().isEmpty());
            QCOMPARE(d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"))->rowCount(), 2);
        });
    }
};

} // namespace

JP_TEST_CLASS(ShellMissingFontsTests)
#include "tst_shell_missingfonts.moc"

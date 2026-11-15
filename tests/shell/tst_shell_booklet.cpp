// The booklet imposition both JeffPub and JeffWrite print with (libs/base/core/booklet.h): which pages go on which side of a sheet, so that sheets folded in half and stacked read in order.
#include "core/booklet.h"
#include "../testrunner.h"

#include <QtTest>

using namespace jp;

namespace {

using Sides = QVector<QVector<int>>;

class ShellBookletTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // Each side holds two pages, left then right; -1 is a blank. The numbers are the pages from 0.
    void onePageIsTheFrontOfASheetOfFour()
    {
        QCOMPARE(bookletOrder(1), (Sides{{-1, 0}, {-1, -1}}));
    }

    void sevenPagesAreFilledOutToEightWithTheBlankOnTheOuterBack()
    {
        QCOMPARE(bookletOrder(7), (Sides{{-1, 0}, {1, 6}, {5, 2}, {3, 4}}));
    }

    void eightPagesMakeTwoSheets()
    {
        QCOMPARE(bookletOrder(8), (Sides{{7, 0}, {1, 6}, {5, 2}, {3, 4}}));
    }

    void thirteenPagesAreFilledOutToSixteen()
    {
        QCOMPARE(bookletOrder(13), (Sides{{-1, 0}, {1, -1}, {-1, 2}, {3, 12}, {11, 4}, {5, 10}, {9, 6}, {7, 8}}));
    }

    void noPagesMakeNoSides() { QVERIFY(bookletOrder(0).isEmpty()); }

    // All of the pages in one booklet, or booklets of 4 to 40 pages: each is imposed by itself, the pages of the next following those of the one before.
    void allIsOneBookletAndANumberSplitsThemUp()
    {
        QCOMPARE(bookletOrder(8, 0), bookletOrder(8));
        QCOMPARE(bookletOrder(8, 40), bookletOrder(8));   // (a booklet larger than the document is the one booklet)
        QCOMPARE(bookletOrder(8, 4), (Sides{{3, 0}, {1, 2}, {7, 4}, {5, 6}}));
        // (the last booklet is filled out with blanks, as a document is)
        QCOMPARE(bookletOrder(6, 4), (Sides{{3, 0}, {1, 2}, {-1, 4}, {5, -1}}));
        // (a number that is not a multiple of four is taken up to one)
        QCOMPARE(bookletOrder(8, 5), bookletOrder(8, 8));
    }

    void thePagesInEverySideAreEachInTheDocumentOnce()
    {
        for (int pages : {1, 2, 3, 4, 9, 16, 21, 40, 41}) {
            for (int per : {0, 4, 8, 12, 40}) {
                QSet<int> seen;
                for (const QVector<int> &side : bookletOrder(pages, per)) {
                    QCOMPARE(side.size(), 2);
                    for (int p : side) {
                        if (p < 0) continue;
                        QVERIFY2(p < pages && !seen.contains(p), qPrintable(QStringLiteral("%1 pages, %2 per booklet").arg(pages).arg(per)));
                        seen.insert(p);
                    }
                }
                QCOMPARE(seen.size(), pages);
            }
        }
    }
};

} // namespace

JP_TEST_CLASS(ShellBookletTests)
#include "tst_shell_booklet.moc"

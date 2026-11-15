#pragma once
// What a card of the File screen shows is whole, in every app: its title and description, wrapped, fit inside it, stay clear of each other and of the chevron at its right, and every card of
// a row is as tall as the row. Used by each app's tests, for each page of its File screen.
#include "app/appwindow.h"
#include "app/filescreen.h"
#include "app/filescreenparts.h"

#include <QFontMetrics>
#include <QMap>
#include <QtTest>

namespace jptest {

inline void checkCard(const jp::ActionCard *c, const QString &where)
{
    const int w = c->width();
    const QString at = where + QLatin1Char(' ') + c->text() + QStringLiteral(" at ") + QString::number(w);
    QVERIFY2(c->height() >= c->heightForWidth(w), qPrintable(at + QStringLiteral(": the card is shorter than its text")));
    const QRect title = c->titleRect(w), description = c->descriptionRect(w), chevron = c->chevronRect(w, c->height());
    QVERIFY2(c->rect().contains(title) && c->rect().contains(description), qPrintable(at + QStringLiteral(": the text leaves the card")));
    // (the text as it is painted: the whole of each wrapped at the width of its rectangle fits the rectangle's height)
    QFont bold = c->font();
    bold.setWeight(QFont::DemiBold);
    QVERIFY2(QFontMetrics(c->font()).boundingRect(QRect(0, 0, description.width(), 100000), Qt::TextWordWrap, c->description()).height() <= description.height(), qPrintable(at + QStringLiteral(": the description is cut off")));
    QVERIFY2(QFontMetrics(bold).boundingRect(QRect(0, 0, title.width(), 100000), Qt::TextWordWrap, c->text()).height() <= title.height(), qPrintable(at + QStringLiteral(": the title is cut off")));
    QVERIFY2(title.bottom() < description.top(), qPrintable(at + QStringLiteral(": the title and the description overlap")));
    // (the chevron is at the right edge, and the text ends at least 40 pixels before it)
    QVERIFY2(chevron.right() > w - 40 && chevron.right() < w, qPrintable(at + QStringLiteral(": the chevron is not at the right edge")));
    QVERIFY2(title.right() <= w - 40 && description.right() <= w - 40, qPrintable(at + QStringLiteral(": the text reaches the chevron's strip")));
    QVERIFY2(!title.intersects(chevron) && !description.intersects(chevron), qPrintable(at + QStringLiteral(": the text runs under the chevron")));
}

// Every card of the pages, in a wide window and a narrower one. `firstRow` is how many cards the Open page puts side by side in the wide window.
inline void checkFileScreenCards(jp::AppWindow *window, const QStringList &pages, int firstRow)
{
    for (const int width : {1230, 800}) {
        window->resize(width, 990);
        for (const QString &page : pages) {
            window->showFileScreenPage(page);
            auto *screen = window->findChild<jp::FileScreen *>();
            QVERIFY(screen);
            QTRY_VERIFY(screen->isVisible() && screen->width() == width);
            QCoreApplication::processEvents();
            QMap<int, int> rowHeight, rowCount;   // (the top of a row of cards: the height of its first card, and how many are in it)
            int cards = 0;
            for (auto *b : screen->findChildren<QAbstractButton *>()) {
                auto *c = dynamic_cast<jp::ActionCard *>(b);
                if (!c || !c->isVisibleTo(screen)) continue;
                ++cards;
                checkCard(c, page + QLatin1Char(' ') + QString::number(width));
                if (QTest::currentTestFailed()) return;
                const int top = c->mapTo(screen, QPoint(0, 0)).y();
                if (!rowHeight.contains(top)) rowHeight.insert(top, c->height());
                ++rowCount[top];
                QCOMPARE(c->height(), rowHeight.value(top));
            }
            if (page == QLatin1String("open")) {
                QVERIFY2(cards >= firstRow, "the Open page has its cards");
                const int widest = *std::max_element(rowCount.begin(), rowCount.end());
                if (width == 1230) QCOMPARE(widest, firstRow);   // (all of the cards are side by side)
                else if (firstRow >= 3) QVERIFY2(widest < firstRow, "the cards did not stack in the narrower window");
            }
        }
    }
}

} // namespace jptest

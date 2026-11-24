// Changing the formats of a selection run by run (libs/page/app/editor_text.cpp), checked the same way for JeffPub and JeffDeck: a change that
// makes neighboring runs alike lets the text document merge them, and the loop that walks the runs must not stand on one that is gone.
#pragma once
#include "app/editor.h"
#include "text/textprops.h"
#include <QTest>
#include <QTextBlock>
#include <QTextDocument>
#include <functional>

namespace runchecks {

struct Case {
    QString name;
    std::function<QTextCharFormat(int)> make;   // the format of run i: runs differ ONLY in what the change sets or clears
    std::function<void(jp::Editor *)> apply;
};

inline QVector<Case> cases()
{
    using namespace jp;
    auto only = [](int prop, const QString &a, const QString &b) {
        return [=](int i) {
            QTextCharFormat f;
            f.setProperty(prop, i % 2 ? a : b);
            return f;
        };
    };
    QVector<Case> out;
    out.push_back({QStringLiteral("font color automatic"), only(tp::ColorRefP, QStringLiteral("#FF0000"), QStringLiteral("#0000FF")),
                   [](Editor *ed) { ed->setTextColor(ColorRef::none()); }});
    out.push_back({QStringLiteral("heading font"), [](int i) {
                       QTextCharFormat f;
                       f.setFontFamilies(QStringList{i % 2 ? QStringLiteral("Alpha") : QStringLiteral("Beta")});
                       return f;
                   },
                   [](Editor *ed) { ed->setFontFamily(QStringLiteral("+Heading")); }});
    out.push_back({QStringLiteral("text outline none"), only(tp::OutlineRef, QStringLiteral("#111111"), QStringLiteral("#222222")),
                   [](Editor *ed) { ed->clearCharProperty(tp::OutlineRef, QStringLiteral("Text Outline")); }});
    out.push_back({QStringLiteral("text glow none"), only(tp::GlowRef, QStringLiteral("#111111"), QStringLiteral("#222222")),
                   [](Editor *ed) { ed->clearCharProperty(tp::GlowRef, QStringLiteral("Text Glow")); }});
    out.push_back({QStringLiteral("font dialog clear"), [](int i) {
                       QTextCharFormat f;
                       f.setFontFamilies(QStringList{i % 2 ? QStringLiteral("Alpha") : QStringLiteral("Beta")});
                       return f;
                   },
                   [](Editor *ed) { ed->clearCharProperty(QTextFormat::FontFamilies, QStringLiteral("Font")); }});
    out.push_back({QStringLiteral("set one color on every run"), only(tp::ColorRefP, QStringLiteral("#FF0000"), QStringLiteral("#0000FF")),
                   [](Editor *ed) {
                       ed->editCharFormats([](QTextCharFormat &cf) { cf.setProperty(tp::ColorRefP, QStringLiteral("#00AA00")); }, QStringLiteral("Format"));
                   }});
    out.push_back({QStringLiteral("shadow on"), [](int i) {
                       QTextCharFormat f;
                       f.setProperty(tp::Shadow, i % 2 == 1);
                       return f;
                   },
                   [](Editor *ed) { ed->setTextShadow(true, QStringLiteral("Shadow")); }});
    out.push_back({QStringLiteral("clear all formatting"), [](int i) {
                       QTextCharFormat f;
                       f.setFontWeight(i % 2 ? QFont::Bold : QFont::Normal);
                       return f;
                   },
                   [](Editor *ed) { ed->clearFormatting(); }});
    return out;
}

// What a document says: its text, and for each character what the properties these cases touch are.
inline QStringList look(QTextDocument *d)
{
    using namespace jp;
    QStringList out;
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFormat f = it.fragment().charFormat();
            QString s;
            for (int prop : {int(tp::ColorRefP), int(tp::OutlineRef), int(tp::GlowRef), int(tp::Shadow), int(tp::TextFx), int(tp::ThemeFont),
                             int(QTextFormat::FontFamilies), int(QTextFormat::FontWeight)})
                s += f.property(prop).toString() + QLatin1Char('|');
            out << it.fragment().text() + QLatin1Char('@') + s;
        }
        out << QStringLiteral("¶");
    }
    return out;
}

inline int fragments(QTextDocument *d)
{
    int n = 0;
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) ++n;
    return n;
}

// Fills the story with `paragraphs` paragraphs of `runs` runs each, selects the text item, runs the change, and checks: the text is the same,
// the runs did become alike (so the document merged them), it is one undo step, and undo gives back the text and every run's format.
inline void check(jp::Editor *ed, const QString &itemId, const QString &storyId, const Case &c, int paragraphs, int runs)
{
    QTextDocument *d = ed->doc()->storyDoc(storyId);
    QVERIFY2(d, "no story");
    QTextCursor w(d);
    w.select(QTextCursor::Document);
    w.removeSelectedText();
    int n = 0;
    QString text;
    for (int p = 0; p < paragraphs; ++p) {
        if (p) {
            w.insertBlock();
            text += QLatin1Char('\n');
        }
        for (int r = 0; r < runs; ++r, ++n) {
            const QString piece = QStringLiteral("r%1 ").arg(n);
            w.insertText(piece, c.make(n));
            text += piece;
        }
    }
    QCOMPARE(fragments(d), paragraphs * runs);   // (runs that differ are runs of their own)
    const QStringList before = look(d);
    ed->select(itemId);
    const int steps = ed->undoStack()->count();
    c.apply(ed);
    d = ed->doc()->storyDoc(storyId);
    QVERIFY2(d, "no story after");
    QCOMPARE(d->toPlainText().replace(QChar::ParagraphSeparator, QLatin1Char('\n')), text);
    QCOMPARE(fragments(d), paragraphs);   // (alike now: one run in each paragraph)
    QCOMPARE(ed->undoStack()->count(), steps + 1);
    ed->undo();
    d = ed->doc()->storyDoc(storyId);
    QCOMPARE(look(d), before);
    QCOMPARE(d->toPlainText().replace(QChar::ParagraphSeparator, QLatin1Char('\n')), text);
}

}   // namespace runchecks

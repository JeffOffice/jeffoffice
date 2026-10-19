// JeffPub's tests of text layout: lines, tabs, bullets, and page-number fields.
#include "pubhelpers.h"
#include "../timing.h"

class PubTextTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // Underlines keep their kind: Double Underline (Ctrl+Shift+D) made a
    // single one, and the Font dialog's Words only and Double did too; .pub
    // files kept only single ones, both opening and saving.
    void underlineKindsKept()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 400, 200), QStringLiteral("one two\nthree four\nfive six\nseven eight\nnine ten")));
        ed->addItem(box);
        auto runFormat = [&](int block) {
            QTextCursor c(ed->doc()->storyDoc(box->storyId)->findBlockByNumber(block));
            c.movePosition(QTextCursor::NextCharacter);
            return c.charFormat();
        };
        auto selectBlock = [&](int block) {
            ed->beginTextEdit(box->id);
            QTextCursor c(ed->editDoc()->findBlockByNumber(block));
            c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            ed->setCursor(c);
        };
        selectBlock(0);
        w.act(QStringLiteral("fmt.underlineDouble"))->trigger();
        ed->endTextEdit();
        QCOMPARE(runFormat(0).underlineStyle(), QTextCharFormat::SingleUnderline);
        QCOMPARE(runFormat(0).intProperty(jp::tp::UnderlineKind), 1);
        const struct { int block; QTextCharFormat::UnderlineStyle style; int kind; } want[] = {
            {1, QTextCharFormat::SingleUnderline, 2}, {2, QTextCharFormat::DotLine, 4}, {3, QTextCharFormat::DashDotLine, 0}, {4, QTextCharFormat::WaveUnderline, 1}};
        for (const auto &u : want) {
            selectBlock(u.block);
            ed->toggleUnderline(u.style, u.kind);
            ed->endTextEdit();
        }
        QCOMPARE(runFormat(1).intProperty(jp::tp::UnderlineKind), 2);
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            QString e;
            jp::savePublication(*ed->doc(), qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/underlines.jpub"), QImage(), &e);
        }
        // Through a .pub file and back.
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("underlines.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*ed->doc(), path, &err), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QTextDocument *story = nullptr;
        for (auto it = back->stories.cbegin(); it != back->stories.cend(); ++it)
            if ((*it)->doc->toPlainText().startsWith(QLatin1String("one two"))) story = (*it)->doc.get();
        QVERIFY(story);
        auto backFormat = [&](int block) {
            QTextCursor c(story->findBlockByNumber(block));
            c.movePosition(QTextCursor::NextCharacter);
            return c.charFormat();
        };
        QCOMPARE(backFormat(0).intProperty(jp::tp::UnderlineKind), 1);
        QCOMPARE(backFormat(1).intProperty(jp::tp::UnderlineKind), 2);
        QCOMPARE(backFormat(2).underlineStyle(), QTextCharFormat::DotLine);
        QCOMPARE(backFormat(2).intProperty(jp::tp::UnderlineKind), 4);
        QCOMPARE(backFormat(3).underlineStyle(), QTextCharFormat::DashDotLine);
        QCOMPARE(backFormat(4).underlineStyle(), QTextCharFormat::WaveUnderline);
        QCOMPARE(backFormat(4).intProperty(jp::tp::UnderlineKind), 1);
    }

    // Bullets and Numbering's "Indent list by" sets the list's indent (it
    // was ignored), and a style takes what it doesn't set from the style
    // it's based on (Based on was saved but never used).
    void listIndentAndBasedOn()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 400, 200), QStringLiteral("first\nsecond")));
        ed->addItem(box);
        ed->beginTextEdit(box->id, 0);
        double shown = -1;
        QTimer::singleShot(0, [&shown] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            if (auto *indent = d->findChild<QDoubleSpinBox *>(QStringLiteral("indent"))) {
                shown = indent->value();
                indent->setValue(36);
            }
            d->accept();   // whatever was found, so a failure can't leave it open
        });
        w.act(QStringLiteral("para.bulletsDialog"))->trigger();
        QCOMPARE(shown, 18.0);
        QTextBlockFormat bf = ed->editDoc()->firstBlock().blockFormat();
        QCOMPARE(bf.leftMargin(), 36.0);
        QCOMPARE(bf.textIndent(), -36.0);
        QVERIFY(ed->editDoc()->firstBlock().textList());
        ed->endTextEdit();
        // A base style with line spacing the dialog doesn't show; one based on it with a size.
        jp::TextStyle base;
        base.name = QStringLiteral("Base");
        base.blk.setLineHeight(150, QTextBlockFormat::ProportionalHeight);
        base.chr.setFontItalic(true);
        jp::TextStyle child;
        child.name = QStringLiteral("Child");
        child.basedOn = QStringLiteral("Base");
        child.chr.setFontPointSize(20);
        ed->doc()->styles << base << child;
        const jp::TextStyle r = ed->doc()->resolvedStyle(QStringLiteral("Child"));
        QCOMPARE(r.chr.fontPointSize(), 20.0);
        QVERIFY(r.chr.fontItalic());
        QCOMPARE(r.blk.lineHeight(), 150.0);
        ed->beginTextEdit(box->id, 0);
        ed->applyStyle(QStringLiteral("Child"));
        const QTextBlock b = ed->editDoc()->firstBlock();
        QCOMPARE(b.blockFormat().lineHeight(), 150.0);
        QTextCursor c(b);
        c.movePosition(QTextCursor::NextCharacter);
        QVERIFY(c.charFormat().fontItalic());
        QCOMPARE(c.charFormat().fontPointSize(), 20.0);
        ed->endTextEdit();
        // A loop of bases ends.
        ed->doc()->styles[ed->doc()->styles.size() - 2].basedOn = QStringLiteral("Child");
        QCOMPARE(ed->doc()->resolvedStyle(QStringLiteral("Child")).chr.fontPointSize(), 20.0);
    }

    // Format Page Numbers sets the numbers' style and the first page's number
    // (it opened Insert Page Number, which added another number box).
    void formatPageNumbers()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->insertPages(0, 2, false, false);
        ed->setCurrentPage(0);
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 200, 40)));
        ed->addItem(box);
        ed->beginTextEdit(box->id, 0);
        ed->insertField(QStringLiteral("page"));
        ed->endTextEdit();
        const int itemsBefore = int(ed->doc()->pages[0]->items.size());
        jp::FrameSpec spec = jp::Renderer::frameSpec(*ed->doc(), *box, 1, jp::RenderOptions());
        const QString keyBefore = spec.ctx.key();
        QCOMPARE(spec.ctx.resolve(QStringLiteral("page")), QStringLiteral("1"));
        QTimer::singleShot(0, [] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            if (auto *f = d->findChild<QComboBox *>(QStringLiteral("format"))) f->setCurrentIndex(3);   // i, ii, iii
            if (auto *s = d->findChild<QSpinBox *>(QStringLiteral("start"))) s->setValue(4);
            d->accept();
        });
        w.act(QStringLiteral("ins.pageNumberFormat"))->trigger();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), itemsBefore);   // no new box
        spec = jp::Renderer::frameSpec(*ed->doc(), *box, 1, jp::RenderOptions());
        QCOMPARE(spec.ctx.resolve(QStringLiteral("page")), QStringLiteral("iv"));
        QCOMPARE(spec.ctx.resolve(QStringLiteral("page:ROMAN")), QStringLiteral("IV"));   // its own style stays
        QVERIFY(spec.ctx.key() != keyBefore);   // laid out again
        ed->doc()->setup.pageNumberFormat = QStringLiteral("ALPHA");
        spec.ctx.pageNumber = 26;   // number 29 (from 4): past Z the letters go round again, doubled
        QCOMPARE(spec.ctx.resolve(QStringLiteral("page")), QStringLiteral("CC"));
        // Kept in the file.
        const jp::PageSetup back = jp::PageSetup::fromJson(ed->doc()->setup.toJson());
        QCOMPARE(back.firstPageNumber, 4);
        QCOMPARE(back.pageNumberFormat, QStringLiteral("ALPHA"));
    }

    // Increase Indent on list items nests them a level (numbered a., b. under
    // 1., bulleted with a circle under a dot), and Decrease Indent brings them
    // back, continuing the outer numbers; saved and opened again the same.
    void multilevelLists()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 300, 200), QStringLiteral("one\ntwo\nthree\nfour")));
        ed->addItem(box);
        auto selectBlocks = [&](int from, int to) {
            ed->beginTextEdit(box->id);
            QTextCursor c(ed->editDoc()->findBlockByNumber(from));
            c.setPosition(ed->editDoc()->findBlockByNumber(to).position(), QTextCursor::KeepAnchor);
            ed->setCursor(c);
        };
        auto markers = [](QTextDocument *d) {
            QStringList m;
            for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) m << (b.textList() ? b.textList()->itemText(b) : QString());
            return m.join(QLatin1Char(' '));
        };
        selectBlocks(0, 3);
        ed->setList(2, 1);
        QCOMPARE(markers(ed->editDoc()), QStringLiteral("1. 2. 3. 4."));
        selectBlocks(1, 2);
        ed->changeIndent(1);
        QCOMPARE(markers(ed->editDoc()), QStringLiteral("1. a. b. 2."));
        selectBlocks(2, 2);
        ed->changeIndent(-1);
        QCOMPARE(markers(ed->editDoc()), QStringLiteral("1. a. 2. 3."));
        ed->endTextEdit();
        QTextDocument copy;
        jp::storyFromJson(&copy, jp::storyToJson(ed->doc()->storyDoc(box->storyId)));
        QCOMPARE(markers(&copy), QStringLiteral("1. a. 2. 3."));
        // Bullets: a circle a level in.
        selectBlocks(0, 3);
        ed->setList(1);
        selectBlocks(1, 1);
        ed->changeIndent(1);
        QCOMPARE(ed->editDoc()->findBlockByNumber(1).textList()->format().style(), QTextListFormat::ListCircle);
        ed->endTextEdit();
    }

    // A centered line is centered on its letters, a right-aligned one ends
    // at its last letter: Publisher lets the spaces a line ends with hang
    // past them. Qt counted them whenever they fit, half a space off.
    void centeredLinesHangTrailingSpaces()
    {
        auto layout = [](Qt::Alignment al, double *left, double *right) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setAlignment(al);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(24);
            c.insertText(QStringLiteral("Heading   "), cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(300, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            int frame = -1;
            QRectF r;
            lay.caretRect(0, &frame, &r);
            *left = r.x();
            lay.caretRect(7, &frame, &r);
            *right = r.x();
        };
        double l = 0, r = 0;
        layout(Qt::AlignHCenter, &l, &r);
        QVERIFY2(std::abs(l - (300 - r)) < 0.01, qPrintable(QStringLiteral("%1 .. %2").arg(l).arg(r)));
        layout(Qt::AlignRight, &l, &r);
        QVERIFY2(std::abs(r - 300) < 0.01, qPrintable(QString::number(r)));
        layout(Qt::AlignLeft, &l, &r);
        QVERIFY(std::abs(l) < 0.01);
    }

    // The same in a right-to-left paragraph, where the spaces a line ends with
    // sit on the left of its letters: aligned to the end (the left), the
    // letters start at the margin and the spaces hang outside it; centered,
    // the letters are centered; aligned to the start, they end at the right
    // margin as they did. The spaces had pushed a line aligned to the end
    // 33 points in.
    void rightToLeftLinesHangTrailingSpaces()
    {
        // Where the Hebrew word's letters lie: x of the caret before the
        // spaces (the letters' left end), and of the one before the letters.
        auto layout = [](Qt::Alignment al, double *left, double *right) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLayoutDirection(Qt::RightToLeft);
            bf.setAlignment(al);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(24);
            c.insertText(QStringLiteral("\u05E9\u05DC\u05D5\u05DD     "), cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(300, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            int frame = -1;
            QRectF r;
            lay.caretRect(4, &frame, &r);
            *left = r.x();
            lay.caretRect(0, &frame, &r);
            *right = r.x();
        };
        double l = 0, r = 0;
        layout(Qt::AlignRight, &l, &r);   // the end of a right-to-left paragraph
        QVERIFY2(std::abs(l) < 0.01, qPrintable(QStringLiteral("%1 .. %2").arg(l).arg(r)));
        QVERIFY(r > 20 && r < 150);
        layout(Qt::AlignHCenter, &l, &r);
        QVERIFY2(std::abs(l - (300 - r)) < 0.01, qPrintable(QStringLiteral("%1 .. %2").arg(l).arg(r)));
        layout(Qt::AlignLeft, &l, &r);    // the start
        QVERIFY2(std::abs(r - 300) < 0.01, qPrintable(QStringLiteral("%1 .. %2").arg(l).arg(r)));
    }

    // Wave underlines, and the double and thick ones, are drawn under Hebrew
    // and Arabic text too: a right-to-left run's two ends came the wrong way
    // round, so the wave had no length and nothing was drawn.
    void waveUnderlinesDrawUnderRightToLeftText()
    {
        // How many pixels below the baseline are not white: only the underline gets there.
        auto inkBelow = [](const QString &text, Qt::LayoutDirection dir, QTextCharFormat::UnderlineStyle style, int kind) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLayoutDirection(dir);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(24);
            if (style != QTextCharFormat::NoUnderline) {
                cf.setUnderlineStyle(style);
                cf.setProperty(jp::tp::UnderlineKind, kind);
            }
            c.insertText(text, cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(200, 60);
            fs.insets = QMarginsF(0, 0, 0, 0);
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            QImage img(800, 240, QImage::Format_RGB32);
            img.fill(Qt::white);
            QPainter p(&img);
            p.scale(4, 4);
            lay.paint(&p, 0, jp::PaintOptions());
            p.end();
            int ink = 0;
            for (int y = int((lay.lineInfo(0).first().baseline + 1) * 4); y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (qGray(img.pixel(x, y)) < 200) ++ink;
            return ink;
        };
        const QString hebrew = QStringLiteral("\u05E9\u05DC\u05D5\u05DD"), latin = QStringLiteral("wave");
        for (Qt::LayoutDirection dir : {Qt::LeftToRight, Qt::RightToLeft}) {
            QCOMPARE(inkBelow(hebrew, dir, QTextCharFormat::NoUnderline, 0), 0);   // nothing else reaches below the baseline
            for (int kind : {1, 2, 4}) {
                const int latinInk = inkBelow(latin, dir, QTextCharFormat::WaveUnderline, kind);
                QVERIFY(latinInk > 100);
                const int hebrewInk = inkBelow(hebrew, dir, QTextCharFormat::WaveUnderline, kind);
                QVERIFY2(hebrewInk > 100, qPrintable(QStringLiteral("kind %1, direction %2: %3 pixels").arg(kind).arg(int(dir)).arg(hebrewInk)));
            }
        }
    }

    // A dashed underline is one line under the pieces a missing font's run is
    // cut into, its dashes even from end to end: the pattern started over in
    // each piece ("=====" on Impact came out ragged).
    void dashedUnderlinesRunOnAcrossPieces()
    {
        const QString fam = QStringLiteral("Gill Sans MT");
        if (QFontDatabase::hasFamily(fam)) QSKIP("Gill Sans MT is installed here");
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat cf;
        cf.setFontFamilies(QStringList{fam});
        cf.setFontPointSize(24);
        cf.setUnderlineStyle(QTextCharFormat::DashUnderline);
        cf.setProperty(jp::tp::UnderlineKind, 2);
        c.insertText(QStringLiteral("Wilhelminas"), cf);
        jp::FrameSpec fs;
        fs.size = QSizeF(400, 60);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.hyphenate = false;
        jp::StoryLayout lay;
        lay.build(&doc, {fs}, jp::LayoutEnv());
        QImage img(3200, 480, QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.scale(8, 8);
        lay.paint(&p, 0, jp::PaintOptions());
        p.end();
        // The row under the baseline with the most ink is the underline.
        int row = -1, best = 0;
        for (int y = int((lay.lineInfo(0).first().baseline + 1) * 8); y < img.height(); ++y) {
            int ink = 0;
            for (int x = 0; x < img.width(); ++x) ink += qGray(img.pixel(x, y)) < 128;
            if (ink > best) { best = ink; row = y; }
        }
        QVERIFY(row > 0);
        // The dashes along it, their lengths and the gaps between.
        QVector<int> dash, gap;
        int runStart = -1, lastEnd = -1;
        for (int x = 0; x <= img.width(); ++x) {
            const bool on = x < img.width() && qGray(img.pixel(x, row)) < 128;
            if (on && runStart < 0) {
                runStart = x;
                if (lastEnd >= 0) gap << x - lastEnd;
            } else if (!on && runStart >= 0) {
                dash << x - runStart;
                lastEnd = x;
                runStart = -1;
            }
        }
        QVERIFY2(dash.size() >= 8, qPrintable(QString::number(dash.size())));
        // (The first and last dashes may be cut short by the line's ends.)
        const auto [dashLo, dashHi] = std::minmax_element(dash.cbegin() + 1, dash.cend() - 1);
        QVERIFY2(*dashHi - *dashLo <= 2, qPrintable(QStringLiteral("dashes %1..%2 px").arg(*dashLo).arg(*dashHi)));
        const auto [gapLo, gapHi] = std::minmax_element(gap.cbegin(), gap.cend());
        QVERIFY2(*gapHi - *gapLo <= 2, qPrintable(QStringLiteral("gaps %1..%2 px").arg(*gapLo).arg(*gapHi)));
    }

    // A line break (Shift+Enter) goes into a .pub as \n, as Publisher's own
    // files hold it; JeffPub wrote \v, which Publisher 2021 draws as a box
    // ("BASIC INCIDENT□COMMAND" on a cover). Opened again, it is a line break.
    void lineBreaksSavedAsPublishersOwn()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(72, 72, 300, 100);
        box->storyId = doc->createStory();
        QTextCursor(doc->storyDoc(box->storyId)).insertText(QStringLiteral("Basic Incident") + QChar(QChar::LineSeparator) + QStringLiteral("Command System"));
        doc->pages[0]->items.push_back(box);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("breaks.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        const QByteArray quill = jp::cfb::readStream(path, QStringLiteral("Quill/QuillSub/CONTENTS"), &err);
        QVERIFY2(!quill.isEmpty(), qPrintable(err));
        auto utf16 = [](const QString &s) { return QByteArray(reinterpret_cast<const char *>(s.utf16()), s.size() * 2); };
        QVERIFY(quill.contains(utf16(QStringLiteral("Incident\nCommand"))));
        QVERIFY(!quill.contains(utf16(QStringLiteral("Incident\vCommand"))));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        bool found = false;   // (toRawText: toPlainText shows a line break as \n)
        for (auto it = back->stories.cbegin(); it != back->stories.cend(); ++it)
            found |= (*it)->doc->toRawText().contains(QStringLiteral("Incident") + QChar(QChar::LineSeparator) + QStringLiteral("Command"));
        QVERIFY(found);
    }

    // Line spacing in points with neither flag bit set (12 pt as 1219200,
    // eighths of an EMU) is Publisher's too, its "At least": a booklet's list
    // double-spaced in Publisher came in at its style's 1.19 lines, the value
    // dropped.
    void unflaggedLineSpacingInPoints()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(72, 72, 300, 100);
        box->storyId = doc->createStory();
        QTextCursor c(doc->storyDoc(box->storyId));
        QTextBlockFormat bf;
        bf.setLineHeight(12, QTextBlockFormat::FixedHeight);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("Purpose/Essential Task:"));
        doc->pages[0]->items.push_back(box);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("spacing.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        // JeffPub writes the exact points kind (1219200 | 1); make it the unflagged kind.
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        jp::cfb::File file;
        QVERIFY(jp::cfb::read(f.readAll(), &file, &err));
        f.close();
        QByteArray quill = file.stream(QStringLiteral("Quill/QuillSub/CONTENTS"));
        QByteArray flagged("\x34\x22", 2), plain = flagged;
        for (quint32 v : {1219201u, 1219200u}) {
            QByteArray &b = v == 1219201u ? flagged : plain;
            for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xff));
        }
        QVERIFY(quill.contains(flagged));
        quill.replace(flagged, plain);
        QVERIFY(file.setStream(QStringLiteral("Quill/QuillSub/CONTENTS"), quill));
        QFile out(path);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write(jp::cfb::write(file));
        out.close();
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        bool found = false;
        for (auto it = back->stories.cbegin(); it != back->stories.cend(); ++it) {
            const QTextBlock b = (*it)->doc->begin();
            if (!b.text().startsWith(QLatin1String("Purpose"))) continue;
            found = true;
            QCOMPARE(b.blockFormat().lineHeightType(), int(QTextBlockFormat::MinimumHeight));
            QCOMPARE(b.blockFormat().lineHeight(), 12.0);
        }
        QVERIFY(found);
    }

    // A paragraph that hangs (its first line indented less than the rest)
    // starts its first line at the left edge of the room it has, and a list's
    // text comes one hanging indent after that: past a picture's text wrap,
    // the bullet starts at the wrap and the text 9 points on (Publisher drew
    // the bullet beside the picture, not under it). A line can't start left
    // of the text area either: with the left indent at 0 and the first line
    // at -18, the line starts at the edge and a list's text is 18 in.
    void hangingFirstLineStartsAtTheEdge()
    {
        auto firstX = [](double left, double hang, double wrapRight, bool bullet) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLeftMargin(left);
            bf.setTextIndent(-hang);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(10);
            c.insertText(QStringLiteral("First item of a list that runs on to a second line"), cf);
            if (bullet) {
                QTextListFormat lf;
                lf.setStyle(QTextListFormat::ListDisc);
                c.createList(lf);
            }
            jp::FrameSpec fs;
            fs.size = QSizeF(200, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            if (wrapRight > 0) fs.obstacles << QPolygonF(QRectF(-100, -100, 100 + wrapRight, 400));
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            const auto lines = lay.lineInfo(0);
            return QPointF(lines.value(0).rect.x(), lines.value(1).rect.x());
        };
        // Room all the way to the edge: the text is at the left indent.
        QCOMPARE(firstX(9, 9, 0, true), QPointF(9, 9));
        // A wrap to 30: the bullet starts at 30, its text at 39; the second
        // line is at the left indent, past the wrap.
        QCOMPARE(firstX(9, 9, 30, true), QPointF(39, 30));
        // Hung outside the text area: the bullet at the edge, the text 18 in.
        QCOMPARE(firstX(0, 18, 0, true), QPointF(18, 0));
        // Without a bullet, the line starts at the edge.
        QCOMPARE(firstX(0, 18, 0, false), QPointF(0, 0));
        QCOMPARE(firstX(30, 12, 0, false), QPointF(18, 30));
    }

    // A tab after the hanging text of a first line goes to the left indent
    // (Publisher's "b.<tab>text" lists), then to the stops 36 points apart
    // from the text area's edge; on the lines after, the stops are the same
    // places, not 36 points apart from the line's own start.
    void hangingIndentIsATabStop()
    {
        auto tabX = [](double left, double first, const QString &text) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLeftMargin(left);
            bf.setTextIndent(first);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(8);
            c.insertText(text, cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(400, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            int frame = -1;
            QRectF r;
            lay.caretRect(int(text.lastIndexOf(QLatin1Char('\t'))) + 1, &frame, &r);
            return r.x();
        };
        const QString b = QStringLiteral("b.\tUnited States Code");
        // The left indent 57.6, the first line 28.8 (a lettered list): the text is at the left indent, not at 36 on from where the line starts (64.8).
        QVERIFY2(std::abs(tabX(57.6, -28.8, b) - 57.6) < 0.01, qPrintable(QString::number(tabX(57.6, -28.8, b))));
        // After the left indent, the next stop is the next of those 36 apart from the edge: 72.
        QVERIFY2(std::abs(tabX(57.6, -28.8, QStringLiteral("b.\t\tText")) - 72) < 0.01, qPrintable(QString::number(tabX(57.6, -28.8, QStringLiteral("b.\t\tText")))));
        // A line after a line break starts at the left indent; its tab goes to
        // the next stop from the edge: 36, not 54.
        QVERIFY2(std::abs(tabX(18, -18, QStringLiteral("Saturday \t0900")) - 36) < 0.01, qPrintable(QString::number(tabX(18, -18, QStringLiteral("Saturday \t0900")))));
        // Not hanging, the left indent 13.5: the stops are still 36, 72, ... from the
        // edge, not 36 on from where the line starts: a tab at the start of a line (after
        // a line break) goes to 36, not 49.5, and one after "Name " (past 36) to 72.
        QVERIFY2(std::abs(tabX(13.5, 0, QStringLiteral("Name \t989")) - 36) < 0.01, qPrintable(QString::number(tabX(13.5, 0, QStringLiteral("Name \t989")))));
        QVERIFY2(std::abs(tabX(13.5, 0, QStringLiteral("Name \t989")) - 72) < 0.01, qPrintable(QString::number(tabX(13.5, 0, QStringLiteral("Name \t989")))));
    }

    // The same from a file: a lettered paragraph and a bulleted one, as saved
    // and read back, lay out with the tab at the left indent and the bullet's
    // text a hanging indent from the text area's edge.
    void hangingParagraphsFromAFile()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(72, 72, 300, 200);
        box->storyId = doc->createStory();
        QTextCursor c(doc->storyDoc(box->storyId));
        QTextCharFormat cf;
        cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
        cf.setFontPointSize(8);
        QTextBlockFormat lettered;
        lettered.setLeftMargin(57.6);
        lettered.setTextIndent(-28.8);
        c.setBlockFormat(lettered);
        c.insertText(QStringLiteral("b.\tUnited States Code, Title 32"), cf);
        QTextBlockFormat bulleted;
        bulleted.setLeftMargin(0);
        bulleted.setTextIndent(-18);
        c.insertBlock(bulleted);
        c.insertText(QStringLiteral("Bulleted item"), cf);
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        c.createList(lf);
        doc->pages[0]->items.push_back(box);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("hanging.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TextItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Text) bt = static_cast<const jp::TextItem *>(it.get());
        });
        QVERIFY(bt);
        jp::LayoutCache cache;
        jp::RenderOptions opt;
        const auto fl = cache.textFrame(*back, *bt, 1, opt);
        QVERIFY(fl.layout);
        const QTextDocument *sd = back->storyDoc(bt->storyId);
        const QTextBlock first = sd->begin();
        QVERIFY(first.text().startsWith(QLatin1String("b.")));
        int frame = -1;
        QRectF r;
        QVERIFY(fl.layout->caretRect(first.position() + first.text().indexOf(QLatin1Char('\t')) + 1, &frame, &r));
        const double inset = fl.layout->lineInfo(0).value(0).rect.x() - first.blockFormat().leftMargin() - first.blockFormat().textIndent();
        QVERIFY2(std::abs(r.x() - (inset + 57.6)) < 0.05, qPrintable(QString::number(r.x() - inset)));
        // The bulleted paragraph: its text 18 points in from the text area's edge.
        const QTextBlock second = first.next();
        QVERIFY(second.textList());
        double textX = -1;
        for (const auto &li : fl.layout->lineInfo(0))
            if (li.docStart == second.position()) { textX = li.rect.x(); break; }
        QVERIFY2(std::abs(textX - (inset + 18)) < 0.05, qPrintable(QString::number(textX - inset)));
    }

    // A tab goes with the text after it. When that text doesn't fit after the
    // tab's stop, the tab starts the next line with it, and there goes to the
    // first stop past the left indent (Publisher's "Name<spaces><tab>number"
    // blocks: the numbers stand in a column, not off the end of the lines
    // above; a course list's second column went to the next line with a
    // title too long for it). The stops are 36 points apart from the text
    // area's edge, not from where the lines start: with the left indent at
    // 13.5, a tab at the start of a line goes to 36.
    void tabMovesToTheNextLineWithItsText()
    {
        struct Result {
            QStringList lines;
            double textX = 0;
        };
        // The lines of a 100-point box, and where the text after the last tab starts
        // (or `at`, a word in it).
        auto lay = [](double left, const QString &text, const QString &at = QString()) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLeftMargin(left);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(8);
            c.insertText(text, cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(100, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            jp::StoryLayout l;
            l.build(&doc, {fs}, jp::LayoutEnv());
            Result r;
            for (const auto &li : l.lineInfo(0)) r.lines << li.text;
            int frame = -1;
            QRectF rect;
            l.caretRect(at.isEmpty() ? int(text.lastIndexOf(QLatin1Char('\t'))) + 1 : int(text.indexOf(at)), &frame, &rect);
            r.textX = rect.x();
            return r;
        };
        const QString name = QStringLiteral("Colonel Smith");
        // Spaces that end short of the line's end, and spaces that run past it:
        // the tab is at the start of the second line either way, and goes to 36.
        for (int spaces : {8, 25}) {
            const Result r = lay(13.5, name + QString(spaces, QLatin1Char(' ')) + QStringLiteral("\t98"));
            const QString what = QStringLiteral("%1 spaces: %2").arg(spaces).arg(r.lines.join(QLatin1Char('|')));
            QVERIFY2(r.lines.size() == 2, qPrintable(what));
            QVERIFY2(r.lines[1].endsWith(QLatin1String("98")), qPrintable(what));
            QVERIFY2(std::abs(r.textX - 36) < 0.01, qPrintable(what + QStringLiteral(": ") + QString::number(r.textX)));
        }
        // The stop (72) is on the line, the word after it is too long for what is left of it: both move.
        const Result word = lay(0, name + QStringLiteral("\tManagement"));
        QVERIFY2(word.lines.size() == 2, qPrintable(word.lines.join(QLatin1Char('|'))));
        QVERIFY2(std::abs(word.textX - 36) < 0.01, qPrintable(QString::number(word.textX)));
        // Two tabs: the first, whose stop (72) is on the line, stays; the second goes on with the word.
        const Result two = lay(0, name + QStringLiteral("\t\tManagement"));
        QVERIFY2(two.lines.size() == 2, qPrintable(two.lines.join(QLatin1Char('|'))));
        QVERIFY2(std::abs(two.textX - 36) < 0.01, qPrintable(QString::number(two.textX)));
        // The spaces between the tab and the word go with them: six are 13.3 points.
        const Result spaced = lay(0, name + QStringLiteral("\t      Management"), QStringLiteral("Man"));
        QVERIFY2(spaced.lines.size() == 2, qPrintable(spaced.lines.join(QLatin1Char('|'))));
        QVERIFY2(std::abs(spaced.textX - (36 + 13.3)) < 0.2, qPrintable(QString::number(spaced.textX)));
        // A tab with room for the text after it stays: the stop after "Colonel" is 72.
        const Result near = lay(13.5, QStringLiteral("Colonel\t98"));
        QVERIFY2(near.lines.size() == 1, qPrintable(near.lines.join(QLatin1Char('|'))));
        QVERIFY2(std::abs(near.textX - 72) < 0.01, qPrintable(QString::number(near.textX)));
    }

    // Tab stops are the same places on every line of a paragraph, 36 points
    // apart from the text area's edge, wherever a line starts: at the left
    // indent, indented or hung on its first line, beside a picture, or after a
    // tab that went on to it. Qt counts a stop from each line's own start and
    // keeps one list for the layout, so a paragraph's lines starting in
    // different places had the stops of the first line's start, and where the
    // text was asked for later (the caret, the drawing) those of the last.
    void tabStopsStayOnTheGridOnEveryLine()
    {
        struct Tab {
            double before, after;   // the caret before the tab, and after it
        };
        struct Laid {
            QList<Tab> tabs;
            QList<double> starts;   // where each line starts
        };
        auto lay = [](double left, double first, const QString &text, double width, double wrapRight = 0, double wrapBottom = 0) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLeftMargin(left);
            bf.setTextIndent(first);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(8);
            c.insertText(text, cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(width, 200);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            if (wrapRight > 0) fs.obstacles << QPolygonF(QRectF(-100, -100, 100 + wrapRight, 100 + wrapBottom));
            jp::StoryLayout l;
            l.build(&doc, {fs}, jp::LayoutEnv());
            Laid r;
            for (const auto &li : l.lineInfo(0)) r.starts << li.rect.x();
            for (int i = 0; i < text.size(); ++i) {
                if (text[i] != QLatin1Char('\t')) continue;
                int frame = -1;
                QRectF a, b;
                l.caretRect(i, &frame, &a);
                l.caretRect(i + 1, &frame, &b);
                r.tabs << Tab{a.x(), b.x()};
            }
            return r;
        };
        // A tab that stays on its line goes to the first stop past where it starts.
        auto onTheGrid = [](const Laid &r, int skip = 0) {
            int checked = 0;
            for (int i = skip; i < r.tabs.size(); ++i) {
                const Tab &t = r.tabs[i];
                if (t.after <= t.before) continue;   // went on to the next line
                const double want = (std::floor((t.before + 0.01) / 36) + 1) * 36;
                if (std::abs(t.after - want) > 0.02) return -1;
                ++checked;
            }
            return checked;
        };
        auto what = [](const Laid &r) {
            QStringList tabs, starts;
            for (const Tab &t : r.tabs) tabs << QStringLiteral("%1 to %2").arg(t.before).arg(t.after);
            for (double x : r.starts) starts << QString::number(x);
            return tabs.join(QStringLiteral("; ")) + QStringLiteral(" (lines at ") + starts.join(QStringLiteral(", ")) + QLatin1Char(')');
        };
        // The first line indented 20: its tab goes to 36, the one on the lines
        // after (starting at 0, not 20) to 72, and the tab at the end of the
        // second line goes on to the third, which starts at the first stop, 36.
        const Laid indented = lay(0, 20, QStringLiteral("aa\tbb cc dd ee ff gg hh ii jj kk ll mm nn oo pp qq rr ss\ttt uu vv\tww"), 112);
        QVERIFY2(indented.tabs.size() == 3 && std::abs(indented.tabs[0].after - 36) < 0.02, qPrintable(what(indented)));
        QVERIFY2(onTheGrid(indented) >= 2, qPrintable(what(indented)));
        QVERIFY2(indented.starts.size() == 3 && std::abs(indented.starts[2] - 36) < 0.02, qPrintable(what(indented)));
        // Hung by 28.8 at a left indent of 57.6: the first tab goes to the left
        // indent, the ones after it on the second line to 108 and 144.
        const Laid hung = lay(57.6, -28.8, QStringLiteral("b.\tcc dd ee ff gg hh ii jj kk ll mm nn oo pp qq rr ss tt\tuu\tvv ww"), 200);
        QVERIFY2(hung.tabs.size() == 3 && std::abs(hung.tabs[0].after - 57.6) < 0.02, qPrintable(what(hung)));
        QVERIFY2(onTheGrid(hung, 1) == 2, qPrintable(what(hung)));
        // A first line beside a picture, the rest of the paragraph clear of it.
        const Laid beside = lay(0, 0, QStringLiteral("ab\tcd ef gh ij kl mn op qr st uv wx yz\tqq\trr ss tt uu vv"), 120, 7, 8);
        QVERIFY2(beside.starts.size() == 3 && std::abs(beside.starts[0] - 7) < 0.02 && std::abs(beside.starts[1]) < 0.02, qPrintable(what(beside)));
        QVERIFY2(onTheGrid(beside) == 3, qPrintable(what(beside)));
        // Left indent 13.5: the tab that goes on to the second line goes to 36, and that line's own tab from there to 72, not 58.5.
        const Laid indent = lay(13.5, 0, QStringLiteral("Colonel Smith, Esq\tMgr\t98 units"), 120);
        QVERIFY2(indent.tabs.size() == 2 && std::abs(indent.tabs[0].after - 36) < 0.02 && std::abs(indent.tabs[1].after - 72) < 0.02, qPrintable(what(indent)));
    }

    // A price list in one paragraph (rows ended by line breaks, two tabs in
    // each) laid out in a few milliseconds until Qt was handed a list of
    // stops: every tab the list answered cost a pass over the rest of the
    // paragraph's items, and 3,000 rows took 5.9 s against 0.26. A paragraph
    // whose lines start on a stop needs no list: Qt's own spacing is the same
    // places.
    void tabbedRowsLayOutLikePlainOnes()
    {
        auto timed = [](bool tabs) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(9);
            QString text;
            for (int i = 0; i < 1000; ++i) {
                if (i) text += QChar(0x2028);
                text += tabs ? QStringLiteral("Item number %1\t$%2.50\tin stock").arg(i).arg(i % 90) : QStringLiteral("Item number %1 $%2.50 in stock").arg(i).arg(i % 90);
            }
            c.insertText(text, cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(400, 700);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            return bestNanoseconds([&] {
                jp::StoryLayout l;
                l.build(&doc, {fs}, jp::LayoutEnv());
            }) / 1000000;
        };
        const qint64 plain = timed(false), tabbed = timed(true);
        QVERIFY2(tabbed < 5 * plain + 200, qPrintable(QStringLiteral("%1 ms with tabs, %2 ms without").arg(tabbed).arg(plain)));
    }

    // A tab goes on to the next line with the text after it only when that
    // text fits there, after the tab's stop. Where it does not (a quantity
    // in a box 36 wide, a name's word longer than the 24 left after the stop)
    // the tab stays at the end of the line above and the text starts the next
    // line at the left indent, whole, as Qt sets it: the tab had been moved
    // anyway, and left a point or none for the text, which broke after a
    // letter or two (or hyphenated: "Man-", "agement").
    void tabStaysWhenItsTextWouldNotFitAfterItsStop()
    {
        struct Laid {
            QStringList lines;
            QList<double> starts;
        };
        auto lay = [](double width, const QString &text, bool hyphenate = false) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(8);
            c.insertText(text, cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(width, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = hyphenate;
            jp::StoryLayout l;
            l.build(&doc, {fs}, jp::LayoutEnv());
            Laid r;
            for (const auto &li : l.lineInfo(0)) {
                r.lines << QString(li.text).replace(QLatin1Char('\t'), QLatin1Char('|')).remove(QChar(0x00AD));
                r.starts << li.rect.x();
            }
            return r;
        };
        auto what = [](const Laid &r) { return r.lines.join(QLatin1Char('/')); };
        // The stop (36) is at the edge of the line: "125" stays whole at the left indent.
        for (double width : {30.0, 36.0}) {
            const Laid qty = lay(width, QStringLiteral("Qty\t125 units"));
            QVERIFY2(qty.lines.size() >= 2 && qty.lines[1].startsWith(QLatin1String("125")) && std::abs(qty.starts[1]) < 0.01, qPrintable(what(qty)));
        }
        // 60 wide: "Management" is 46 long, 24 would be left after the stop.
        for (bool hyphenate : {false, true}) {
            const Laid name = lay(60, QStringLiteral("Name Smith\tManagement"), hyphenate);
            QVERIFY2(name.lines.size() == 2 && name.lines[1] == QLatin1String("Management") && std::abs(name.starts[1]) < 0.01, qPrintable(what(name)));
        }
        // With room for it after the stop, the tab goes on with the word.
        const Laid roomy = lay(100, QStringLiteral("Name Smith\tManagement"));
        QVERIFY2(roomy.lines.size() == 2 && std::abs(roomy.starts[1] - 36) < 0.01, qPrintable(what(roomy)));
    }

    // A publication's default tab stops (Publisher's: 36 points
    // unless it says otherwise) are the places the stops of a
    // paragraph without its own are, counted from the text area's edge, not
    // from the line's start: Publisher's PDFs of a publication set to 20 and
    // to 14.4 put the text after a tab at multiples of those from the edge,
    // with the paragraph indented 13.5 too (a MIVDF flyer has 14.4).
    void defaultTabStopsFollowThePublication()
    {
        auto after = [](double def, double left, const QString &text) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLeftMargin(left);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(8);
            c.insertText(text, cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(200, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            jp::LayoutEnv env;
            env.defaultTab = def;
            jp::StoryLayout l;
            l.build(&doc, {fs}, env);
            QStringList xs;
            for (int i = 0; i < text.size(); ++i) {
                if (text[i] != QLatin1Char('\t')) continue;
                int frame = -1;
                QRectF r;
                l.caretRect(i + 1, &frame, &r);
                xs << QString::number(r.x(), 'f', 1);
            }
            return xs.join(QLatin1Char(' '));
        };
        const QString text = QStringLiteral("a\tb\tc\td");
        QCOMPARE(after(36, 0, text), QStringLiteral("36.0 72.0 108.0"));
        QCOMPARE(after(20, 0, text), QStringLiteral("20.0 40.0 60.0"));
        QCOMPARE(after(20, 13.5, QStringLiteral("a\tb\tc")), QStringLiteral("20.0 40.0"));
        QCOMPARE(after(14.4, 0, QStringLiteral("aa\tbb\tcc")), QStringLiteral("14.4 28.8"));
        QCOMPARE(after(14.4, 13.5, QStringLiteral("g\th\ti")), QStringLiteral("28.8 43.2"));
    }

    // Into pixels no line is thinner than one pixel: Publisher's pictures of
    // lines 0.1, 0.25, 0.5, and 0.75 points wide (Oct 11, at 96 dpi) are all
    // one pixel of black, where a 0.25 point rule drawn as it is came out a
    // faint quarter. A line already wider (3 points) is as wide as it is.
    // The template art's stripes leave gaps, which must be clear, not whatever
    // the new picture's memory held: the Grand Opening banner came out
    // differently from run to run in a band at its stripes' edge.
    void templateStripesHaveClearGaps()
    {
        const jp::TemplateInfo *t = jp::findTemplate(QStringLiteral("banner-grand-opening"));
        QVERIFY(t);
        // Colors no other test uses, so the art is painted here, not taken
        // from the session's cache; memory left full of a pattern first, so
        // that a picture that isn't cleared shows it.
        jp::TemplateOptions o;
        jp::ColorScheme cs;
        cs.name = QStringLiteral("Gaps");
        for (int k = 0; k < jp::SlotCount; ++k) cs.c[k] = QColor::fromRgb(17 + k * 13, 101 + k * 7, 59 + k * 11);
        o.colors = cs;
        for (int round = 0; round < 6; ++round) {
            std::vector<QByteArray> dirt;
            for (int k = 0; k < 8; ++k) dirt.emplace_back(3000 * 400 * 4 + k * 4096, char(0x5a));
            dirt.clear();
            cs.c[0] = QColor::fromRgb(17 + round, 101, 59);
            o.colors = cs;
            auto doc = t->build(o);
            int checked = 0;
            for (const auto &it : doc->pages[0]->items) {
                const auto *pic = dynamic_cast<const PictureItem *>(it.get());
                if (!pic || !doc->images.contains(pic->imageId)) continue;
                const QImage img = QImage::fromData(doc->images[pic->imageId].bytes).convertToFormat(QImage::Format_ARGB32_Premultiplied);
                QVERIFY(!img.isNull());
                for (int y = 0; y < img.height(); ++y) {
                    const QRgb *row = reinterpret_cast<const QRgb *>(img.constScanLine(y));
                    for (int x = 0; x < img.width(); ++x) {
                        // Premultiplied: no channel above the alpha; and a clear gap is all zero.
                        const QRgb px = row[x];
                        QVERIFY2(qRed(px) <= qAlpha(px) && qGreen(px) <= qAlpha(px) && qBlue(px) <= qAlpha(px) && px != 0x5a5a5a5a,
                                 qPrintable(QStringLiteral("pixel %1,%2 is %3").arg(x).arg(y).arg(px, 8, 16)));
                    }
                }
                ++checked;
            }
            QCOMPARE(checked, 2);
        }
    }

    void hairlinesAreAPixelWide()
    {
        using namespace jp;
        auto render = [](double width, double scale) {
            auto doc = Document::blank(QSizeF(200, 200));
            auto line = std::make_shared<LineItem>();
            line->p1 = QPointF(20, 100.5);
            line->p2 = QPointF(180, 100.5);
            line->stroke = Stroke::line(ColorRef::rgb(Qt::black), width);
            line->syncRect();
            doc->pages[0]->items.push_back(line);
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            return Renderer::renderToImage(ctx, 0, scale);
        };
        // At 72 dpi, a point is a pixel: the row the line is on, dark all through.
        QVERIFY2(qGray(render(0.25, 1.0).pixel(100, 100)) < 40, qPrintable(QString::number(qGray(render(0.25, 1.0).pixel(100, 100)))));
        QVERIFY(qGray(render(0.1, 1.0).pixel(100, 100)) < 40);
        // The rows on either side stay clear.
        QVERIFY(qGray(render(0.25, 1.0).pixel(100, 98)) > 240 && qGray(render(0.25, 1.0).pixel(100, 102)) > 240);
        // Zoomed in, the line is as thick as it is: 3 points at 4 times is 12 pixels.
        const QImage thick = render(3, 4.0);
        QVERIFY(qGray(thick.pixel(400, 402 - 5)) < 40 && qGray(thick.pixel(400, 402 + 4)) < 40);
        QVERIFY(qGray(thick.pixel(400, 402 - 8)) > 240 && qGray(thick.pixel(400, 402 + 8)) > 240);
    }

    // Publisher sets a box's text inside its line: the margin is measured from
    // the line's inner edge, so a line of weight w moves the text in w/2 when
    // it is centered on the edge and w when it is an inset one (measured Oct
    // 11, a text box and a rectangle with text, weights 1 to 24: the ink of a
    // letter 0.48 pt further in for each point of weight, and 12 for an inset
    // line of 12). A rectangle behind a text box with no line of its own,
    // read as the box's border, leaves the text where it was.
    void textIsSetInsideItsBoxsLine()
    {
        using namespace jp;
        auto left = [](double width, bool inset, bool shape) {
            auto doc = Document::blank(QSizeF(300, 200));
            Stroke line = width > 0 ? Stroke::line(ColorRef::rgb(Qt::black), width) : Stroke::none();
            line.inset = inset;
            LayoutCache cache;
            if (!shape) {
                auto t = std::make_shared<TextItem>();
                t->rect = QRectF(20, 20, 200, 100);
                t->insets = QMarginsF(3, 3, 3, 3);
                t->storyId = doc->createStory(QStringLiteral("Hi"));
                t->stroke = line;
                doc->pages[0]->items.push_back(t);
                return cache.textFrame(*doc, *t, 1, RenderOptions()).layout->lineInfo(0)[0].rect.left();
            }
            auto sh = std::make_shared<ShapeItem>();
            sh->shape = QStringLiteral("rect");
            sh->rect = QRectF(20, 20, 200, 100);
            sh->insets = QMarginsF(3, 3, 3, 3);
            sh->storyId = doc->createStory(QStringLiteral("Hi"));
            sh->stroke = line;
            doc->pages[0]->items.push_back(sh);
            PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            QPointF origin;
            return Renderer::shapeTextLayout(ctx, *sh, &origin)->lineInfo(0)[0].rect.left();
        };
        for (bool shape : {false, true}) {
            QVERIFY2(std::abs(left(0, false, shape) - 3) < 0.01, qPrintable(QString::number(left(0, false, shape))));
            QVERIFY2(std::abs(left(6, false, shape) - 6) < 0.01, qPrintable(QString::number(left(6, false, shape))));
            QVERIFY2(std::abs(left(6, true, shape) - 9) < 0.01, qPrintable(QString::number(left(6, true, shape))));
        }
        // A rectangle with a line behind a text box with none (a056300e's frame): read as one box, the text where it was.
        auto doc = Document::blank(QSizeF(300, 200));
        auto rect = std::make_shared<ShapeItem>();
        rect->shape = QStringLiteral("rect");
        rect->rect = QRectF(20, 20, 200, 100);
        rect->fill = Fill::none();
        rect->stroke = Stroke::line(ColorRef::rgb(Qt::black), 2);
        auto box = std::make_shared<TextItem>();
        box->rect = rect->rect;
        box->insets = QMarginsF(2.88, 2.88, 2.88, 2.88);
        box->storyId = doc->createStory(QStringLiteral("Hi"));
        doc->pages[0]->items.push_back(rect);
        doc->pages[0]->items.push_back(box);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("framed.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const TextItem *merged = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (it->type() == ItemType::Text) merged = static_cast<const TextItem *>(it.get());
        QVERIFY(merged);
        QVERIFY2(std::abs(merged->stroke.width - 2) < 0.01 && std::abs(merged->insets.left() - 1.88) < 0.02 && std::abs(merged->insets.top() - 1.88) < 0.02,
                 qPrintable(QStringLiteral("line %1, margin %2").arg(merged->stroke.width).arg(merged->insets.left())));
    }

    // A text box with a line of its own is one box through a save: its
    // rectangle, margin, and line come back as they went, so JeffPub and
    // Publisher both set the text the line's share inside the margin. (The box
    // of a publication read before this had its top moved by half the line,
    // and its margin given up a second time on the way back.)
    void aTextBoxKeepsItsOwnLineThroughASave()
    {
        using namespace jp;
        for (bool inset : {false, true}) {
            auto doc = Document::blank(QSizeF(300, 200));
            auto box = std::make_shared<TextItem>();
            box->rect = QRectF(20, 20, 200, 100);
            box->insets = QMarginsF(3, 3, 3, 3);
            box->storyId = doc->createStory(QStringLiteral("Hi"));
            box->stroke = Stroke::line(ColorRef::rgb(Qt::black), 6);
            box->stroke.inset = inset;
            doc->pages[0]->items.push_back(box);
            QTemporaryDir dir;
            const QString path = dir.filePath(QStringLiteral("own-line.pub"));
            QString err;
            QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
            auto back = importPublisherFile(path, &err);
            QVERIFY2(back, qPrintable(err));
            const TextItem *t = nullptr;
            for (const auto &it : back->pages[0]->items)
                if (it->type() == ItemType::Text) t = static_cast<const TextItem *>(it.get());
            QVERIFY2(t, qPrintable(QStringLiteral("items %1, first type %2").arg(back->pages[0]->items.size()).arg(back->pages[0]->items.empty() ? -1 : int(back->pages[0]->items[0]->type()))));
            const QString what = QStringLiteral("inset %1: rect y %2 h %3, margin %4 %5, line %6").arg(inset).arg(t->rect.y()).arg(t->rect.height()).arg(t->insets.left()).arg(t->insets.top()).arg(t->stroke.width);
            QVERIFY2(std::abs(t->rect.y() - 20) < 0.05 && std::abs(t->rect.height() - 100) < 0.05 && std::abs(t->rect.x() - 20) < 0.05 && std::abs(t->rect.width() - 200) < 0.05,
                     qPrintable(what));
            QVERIFY2(std::abs(t->insets.left() - 3) < 0.02 && std::abs(t->insets.top() - 3) < 0.02 && std::abs(t->insets.right() - 3) < 0.02 && std::abs(t->insets.bottom() - 3) < 0.02,
                     qPrintable(what));
            QVERIFY2(std::abs(t->stroke.width - 6) < 0.01 && t->stroke.inset == inset, qPrintable(what));
        }
    }

    // The default is saved in a .pub file's text stream (its SGP section, a
    // property block with 00:22 in EMU, left empty at 36 points: found in
    // Publisher's files and in ones it saved with several settings), kept by
    // JeffPub's own files, and laid out with: a publication set to 14.4
    // reads back at 14.4, and its tabs go there.
    void defaultTabStopIsSavedAndRead()
    {
        using namespace jp;
        auto make = [](double def) {
            auto doc = Document::blank(QSizeF(612, 792));
            doc->defaultTabStop = def;
            auto t = std::make_shared<TextItem>();
            t->rect = QRectF(72, 72, 300, 100);
            t->storyId = doc->createStory(QStringLiteral("aa\tbb\tcc"));
            doc->pages[0]->items.push_back(t);
            return doc;
        };
        // The SGP section's bytes.
        auto sgp = [](const QString &path) {
            const QByteArray q = cfb::readStream(path, QStringLiteral("Quill/QuillSub/CONTENTS"));
            auto u32 = [&](qsizetype at) { return qFromLittleEndian<quint32>(q.constData() + at); };
            for (quint32 at = 0x18; at != 0xffffffffu && at + 8 <= quint32(q.size()); at = u32(at + 4))
                for (int k = 0; k < qFromLittleEndian<quint16>(q.constData() + at + 2); ++k) {
                    const qsizetype e = at + 8 + 24 * k;
                    if (q.mid(e + 2, 4) == "SGP ") return q.mid(u32(e + 16), u32(e + 20));
                }
            return QByteArray("none");
        };
        QTemporaryDir dir;
        QString err;
        const QString plain = dir.filePath(QStringLiteral("plain.pub")), set = dir.filePath(QStringLiteral("set.pub"));
        QVERIFY2(exportPublisher(*make(36), plain, &err), qPrintable(err));
        QVERIFY2(exportPublisher(*make(14.4), set, &err), qPrintable(err));
        QCOMPARE(sgp(plain), QByteArray::fromHex("04000000"));
        QCOMPARE(sgp(set), QByteArray::fromHex("0a000000" "0022" "60ca0200"));   // 182,880 EMU
        auto back = importPublisherFile(set, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY2(std::abs(back->defaultTabStop - 14.4) < 0.001, qPrintable(QString::number(back->defaultTabStop)));
        auto backPlain = importPublisherFile(plain, &err);
        QVERIFY2(backPlain, qPrintable(err));
        QCOMPARE(backPlain->defaultTabStop, 36.0);
        // Laid out with it.
        const TextItem *bt = nullptr;
        walkItems(back->pages[0]->items, [&](const ItemPtr &it) {
            if (it->type() == ItemType::Text) bt = static_cast<const TextItem *>(it.get());
        });
        QVERIFY(bt);
        LayoutCache cache;
        RenderOptions opt;
        const auto fl = cache.textFrame(*back, *bt, 1, opt);
        QVERIFY(fl.layout);
        const QTextBlock b = back->storyDoc(bt->storyId)->begin();
        int frame = -1;
        QRectF origin, first, second;
        fl.layout->caretRect(b.position(), &frame, &origin);
        fl.layout->caretRect(b.position() + 3, &frame, &first);
        fl.layout->caretRect(b.position() + 6, &frame, &second);
        QVERIFY2(std::abs(first.x() - origin.x() - 14.4) < 0.01 && std::abs(second.x() - origin.x() - 28.8) < 0.01, qPrintable(QStringLiteral("%1 %2 from %3").arg(first.x()).arg(second.x()).arg(origin.x())));
        // And in JeffPub's own file, which says nothing at 36.
        QVERIFY(!publicationBytes(*make(36), QImage()).contains("defaultTab"));
        const QString own = dir.filePath(QStringLiteral("set.jpub"));
        QVERIFY2(savePublication(*make(20), own, QImage(), &err), qPrintable(err));
        auto again = loadPublication(own, &err);
        QVERIFY2(again, qPrintable(err));
        QCOMPARE(again->defaultTabStop, 20.0);
    }

    // Paragraph > Tabs has the publication's Default tab stops under the list
    // of stops, in the units of the dialog's other measurements and from 1 to
    // 1,584 points (what Publisher takes), with or without a text box
    // selected. Setting it is one undo step that moves the tabs of
    // paragraphs without stops of their own, and it is saved in .jpub and
    // .pub files.
    void defaultTabStopIsSetInTheTabsDialog()
    {
        using namespace jp;
        Settings &st = Settings::get();
        const Unit was = st.unit();
        const auto restoreUnit = qScopeGuard([&] { st.setUnit(was); });
        st.setUnit(Unit::Centimeter);
        MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        ed->setDocument(Document::blank(QSizeF(612, 792)));
        auto box = ed->newTextBox(QRectF(72, 72, 400, 100), QStringLiteral("aa\tbb\tcc"));
        ed->addItem(box);
        const QString id = box->id;
        ed->select(id);
        // How far from the line's start the text after each tab begins.
        auto stops = [&] {
            const auto *t = static_cast<const TextItem *>(ed->doc()->item(id));
            const auto fl = ed->cache().textFrame(*ed->doc(), *t, 1, ed->renderOptions());
            if (!fl.layout) return QStringLiteral("no layout");
            const int start = ed->doc()->storyDoc(t->storyId)->begin().position();
            int frame = -1;
            QRectF origin, first, second;
            fl.layout->caretRect(start, &frame, &origin);
            fl.layout->caretRect(start + 3, &frame, &first);
            fl.layout->caretRect(start + 6, &frame, &second);
            return QStringLiteral("%1 %2").arg(first.x() - origin.x(), 0, 'f', 1).arg(second.x() - origin.x(), 0, 'f', 1);
        };
        // Opens Tabs, reads the box, sets it to `points` (when positive), and presses OK.
        struct Seen {
            bool found = false;
            QString text, name, label;
            double lowest = 0, highest = 0;
            int page = -1;
        } seen;
        auto tabsDialog = [&](double points) {
            seen = Seen();
            QTimer::singleShot(0, &w, [&, points] {
                auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!d) return;
                if (auto *tabs = d->findChild<QTabWidget *>()) seen.page = tabs->currentIndex();
                if (auto *spin = d->findChild<MeasureSpin *>(QStringLiteral("defaultTabStop"))) {
                    seen.found = true;
                    seen.text = spin->text();
                    seen.name = spin->accessibleName();
                    seen.lowest = spin->minimum();
                    seen.highest = spin->maximum();
                    for (QLabel *l : d->findChildren<QLabel *>())
                        if (l->buddy() == spin) seen.label = l->text();
                    if (points > 0) spin->setValue(points);
                    if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) d->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/tabs-dialog.png"));
                    d->accept();
                } else {
                    d->reject();
                }
            });
            w.act(QStringLiteral("para.tabs"))->trigger();
        };
        QCOMPARE(stops(), QStringLiteral("36.0 72.0"));

        // The box shows 36 points as 1.27 cm, on the Tabs page, named, from 1 to 1,584 points.
        tabsDialog(0);
        QVERIFY(seen.found);
        QCOMPARE(seen.page, 2);
        QCOMPARE(seen.text, QStringLiteral("1.27 cm"));
        QCOMPARE(seen.lowest, 1.0);
        QCOMPARE(seen.highest, 1584.0);
        QCOMPARE(seen.name, QStringLiteral("Default tab stops"));
        QVERIFY2(seen.label.startsWith(QLatin1String("Default tab stops")), qPrintable(seen.label));
        QCOMPARE(ed->doc()->defaultTabStop, 36.0);

        // Setting it is one step, and the tabs move.
        int steps = ed->undoStack()->index();
        tabsDialog(20);
        QCOMPARE(ed->doc()->defaultTabStop, 20.0);
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Default Tab Stops"));
        QCOMPARE(stops(), QStringLiteral("20.0 40.0"));
        tabsDialog(0);
        QCOMPARE(seen.text, QStringLiteral("0.706 cm"));   // 20 points
        ed->undo();
        QCOMPARE(ed->doc()->defaultTabStop, 36.0);
        QCOMPARE(stops(), QStringLiteral("36.0 72.0"));
        ed->redo();
        QCOMPARE(ed->doc()->defaultTabStop, 20.0);
        QCOMPARE(stops(), QStringLiteral("20.0 40.0"));

        // Past Publisher's range it stops at the ends of it.
        tabsDialog(5000);
        QCOMPARE(ed->doc()->defaultTabStop, 1584.0);
        tabsDialog(0.2);
        QCOMPARE(ed->doc()->defaultTabStop, 1.0);

        // With nothing selected it still sets the publication's.
        ed->clearSelection();
        steps = ed->undoStack()->index();
        tabsDialog(14.4);
        QVERIFY(seen.found);
        QCOMPARE(ed->doc()->defaultTabStop, 14.4);
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QCOMPARE(stops(), QStringLiteral("14.4 28.8"));

        // Saved and opened again as JeffPub's own file and as a .pub file.
        QTemporaryDir dir;
        QString err;
        const QString own = dir.filePath(QStringLiteral("tabs.jpub")), pub = dir.filePath(QStringLiteral("tabs.pub"));
        QVERIFY2(savePublication(*ed->doc(), own, QImage(), &err), qPrintable(err));
        QVERIFY2(exportPublisher(*ed->doc(), pub, &err), qPrintable(err));
        auto fromOwn = loadPublication(own, &err);
        QVERIFY2(fromOwn, qPrintable(err));
        QCOMPARE(fromOwn->defaultTabStop, 14.4);
        auto fromPub = importPublisherFile(pub, &err);
        QVERIFY2(fromPub, qPrintable(err));
        QVERIFY2(std::abs(fromPub->defaultTabStop - 14.4) < 0.001, qPrintable(QString::number(fromPub->defaultTabStop)));
    }

    // Boxes that grow to fit their text, and table rows that grow, follow
    // the new tab stops when words now wrap onto another line; a box whose
    // text has no tab is left as it was.
    void defaultTabStopRefitsGrowingBoxesAndTableRows()
    {
        using namespace jp;
        MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        ed->setDocument(Document::blank(QSizeF(612, 792)));
        auto grows = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(72, 72, 150, 20), QStringLiteral("a\tb\tc\td")));
        auto plain = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(300, 72, 150, 90), QStringLiteral("hello")));
        grows->autofit = plain->autofit = TextItem::GrowBox;
        auto table = std::static_pointer_cast<TableItem>(ed->newTable(QRectF(72, 300, 150, 20), 1, 1));
        table->growToFit = true;
        QTextCursor(ed->doc()->storyDoc(table->cells[0].storyId)).insertText(QStringLiteral("a\tb\tc\td"));
        for (const ItemPtr &it : {ItemPtr(grows), ItemPtr(plain), ItemPtr(table)}) ed->addItem(it);
        ed->autoGrowText(grows.get());
        ed->fitTableRows(table.get());
        const double boxHeight = grows->rect.height(), rowHeight = table->rowH[0];
        QVERIFY2(boxHeight < 40 && rowHeight < 40, qPrintable(QStringLiteral("%1 %2").arg(boxHeight).arg(rowHeight)));   // one line each
        QTimer::singleShot(0, &w, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            auto *spin = d->findChild<MeasureSpin *>(QStringLiteral("defaultTabStop"));
            if (spin) {
                spin->setValue(100);
                d->accept();
            } else {
                d->reject();
            }
        });
        w.act(QStringLiteral("para.tabs"))->trigger();
        QCOMPARE(ed->doc()->defaultTabStop, 100.0);
        QVERIFY2(grows->rect.height() > boxHeight * 1.5, qPrintable(QStringLiteral("%1 from %2").arg(grows->rect.height()).arg(boxHeight)));
        QVERIFY2(table->rowH[0] > rowHeight * 1.5, qPrintable(QStringLiteral("%1 from %2").arg(table->rowH[0]).arg(rowHeight)));
        QCOMPARE(plain->rect.height(), 90.0);
        // Undoing puts the sizes back with the stops.
        ed->undo();
        QCOMPARE(ed->doc()->defaultTabStop, 36.0);
        QCOMPARE(static_cast<TextItem *>(ed->doc()->item(grows->id))->rect.height(), boxHeight);
        QCOMPARE(static_cast<TableItem *>(ed->doc()->item(table->id))->rowH[0], rowHeight);
    }

    // Of several tabs together the last goes on with the text: when the line
    // breaks inside the group, so that the next line starts with the last of
    // them, the tab at the end of the line above stays there (its stop is on
    // the line) and is not moved too. A MIVDF flyer's list items (default stops
    // 14.4) have "Strategic<spaces><tab><2 spaces>" ending a line and
    // "<tab><6 spaces>National" starting the next, which Publisher's PDF
    // sets with the text at 28.1 from the edge (one stop and the spaces),
    // where the tab that stayed had gone on too, a stop further.
    void tabGroupSplitByALineBreakKeepsItsLastTabForTheNextLine()
    {
        auto at = [](const QString &text, const QString &word) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setTextIndent(25.44);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Times New Roman")});
            cf.setFontPointSize(9);
            c.insertText(text, cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(201.6, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            jp::LayoutEnv env;
            env.defaultTab = 14.4;
            jp::StoryLayout l;
            l.build(&doc, {fs}, env);
            int frame = -1;
            QRectF r;
            l.caretRect(int(text.indexOf(word)), &frame, &r);
            return r.x();
        };
        const double national = at(QStringLiteral("Support the operation of the SNS (Strategic       \t  \t      National Stockpile)"), QStringLiteral("National"));
        QVERIFY2(std::abs(national - 27.9) < 0.3, qPrintable(QString::number(national)));
        // Three tabs in a row, the last two starting the next line: the text after a stop and a stop (28.8, Publisher's PDF 28.84).
        const double emergency = at(QStringLiteral(" Logistics management of donations during        \t\t\tan emergency."), QStringLiteral("an emergency"));
        QVERIFY2(std::abs(emergency - 28.8) < 0.1, qPrintable(QString::number(emergency)));
    }

    // A paragraph whose lines use different tab stops draws each line with its
    // own, selected text too: with a selection given, the layout drew every
    // line with the stops of the last one, so a selected first line's text
    // after its tab jumped to where the last line's stops put it.
    void selectedTextInAParagraphKeepsEachLinesTabStops()
    {
        QFont font(QStringLiteral("Arial"));
        font.setPixelSize(16);
        jp::ParagraphLayout tl(QStringLiteral("a\tb") + QChar(0x2028) + QStringLiteral("c\td"), font);
        const QList<QTextOption::Tab> stops[] = {{QTextOption::Tab(40, QTextOption::LeftTab)}, {QTextOption::Tab(100, QTextOption::LeftTab)}};
        tl.beginLayout();
        double y = 0;
        for (int i = 0; i < 2; ++i) {
            QTextLine l = tl.createLine();
            tl.setLineTabs(stops[i]);
            l.setLineWidth(300);
            l.setPosition(QPointF(0, y));
            y += l.height();
            tl.keepLineTabs(i);
        }
        tl.endLayout();
        // Where the second letter of each line starts, by its dark pixels.
        auto inkAfterTab = [&](bool selected, double line) {
            QImage img(300, 80, QImage::Format_RGB32);
            img.fill(Qt::white);
            QPainter p(&img);
            p.setPen(Qt::black);
            QList<QTextLayout::FormatRange> sel;
            if (selected) {
                QTextLayout::FormatRange r;
                r.start = 0;
                r.length = int(tl.text().size());
                r.format.setBackground(QColor(255, 255, 0));
                sel << r;
            }
            tl.draw(&p, QPointF(0, 0), sel);
            p.end();
            const QTextLine l = tl.lineAt(int(line));
            for (int x = 20; x < img.width(); ++x)
                for (int yy = int(l.y()); yy < int(l.y() + l.height()); ++yy)
                    if (qGray(img.pixel(x, yy)) < 100) return x;
            return -1;
        };
        for (bool selected : {false, true}) {
            const int first = inkAfterTab(selected, 0), second = inkAfterTab(selected, 1);
            QVERIFY2(first >= 40 && first <= 46 && second >= 100 && second <= 106, qPrintable(QStringLiteral("%1 selected: %2 and %3").arg(selected).arg(first).arg(second)));
        }
    }

    // That drawing of a selection is Qt's own, on lines that don't depend on
    // their stops (no tab in the text): the selection's background behind the
    // text, and the selected text in its color where it has one.
    void selectionDrawsLikeQtsOnLinesWithTwoStopLists()
    {
        QFont font(QStringLiteral("Arial"));
        font.setPixelSize(16);
        const QString text = QStringLiteral("alpha beta gamma delta epsilon zeta eta theta iota kappa lambda");
        auto lay = [&](jp::ParagraphLayout &tl, bool lists) {
            tl.beginLayout();
            double y = 0;
            for (int i = 0; true; ++i) {
                QTextLine l = tl.createLine();
                if (!l.isValid()) break;
                if (lists) tl.setLineTabs({QTextOption::Tab(30 + 20 * (i % 2), QTextOption::LeftTab)});
                l.setLineWidth(150);
                l.setPosition(QPointF(0, y));
                y += l.height();
                if (lists) tl.keepLineTabs(i);
            }
            tl.endLayout();
        };
        auto render = [&](bool lists, const QTextLayout::FormatRange &sel) {
            jp::ParagraphLayout tl(text, font);
            lay(tl, lists);
            QImage img(200, 160, QImage::Format_RGB32);
            img.fill(Qt::white);
            QPainter p(&img);
            p.setPen(Qt::black);
            tl.draw(&p, QPointF(5, 5), {sel});
            p.end();
            return img;
        };
        QTextLayout::FormatRange back, both;
        back.start = both.start = 8;
        back.length = both.length = 40;
        back.format.setBackground(QColor(255, 255, 0));
        both.format.setBackground(QColor(51, 153, 255));
        both.format.setForeground(Qt::white);
        // The same pixels, but for the last column of a highlight, which Qt fills on some lines and not on others.
        for (const auto &sel : {back, both}) {
            const QImage plain = render(false, sel), lines = render(true, sel);
            QSet<int> columns;
            for (int y = 0; y < plain.height(); ++y)
                for (int x = 0; x < plain.width(); ++x)
                    if (plain.pixel(x, y) != lines.pixel(x, y)) columns.insert(x);
            QVERIFY2(columns.size() <= 3, qPrintable(QStringLiteral("%1 columns differ").arg(columns.size())));
        }
    }

    // Centered and right-aligned lines have their text shifted by Qt, which
    // the stops don't share: a tab at the end of one was taken for past its
    // stop (the shift added to where it started) and went on to the next line
    // with its neighbor, which started 36 points in too far. They are set as
    // a line at the left is.
    void tabsInCenteredAndRightTextGoOnAsAtTheLeft()
    {
        auto lay = [](Qt::Alignment al, const QString &text) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setAlignment(al);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(8);
            c.insertText(text, cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(100, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            jp::StoryLayout l;
            l.build(&doc, {fs}, jp::LayoutEnv());
            QStringList lines;
            for (const auto &li : l.lineInfo(0)) lines << QStringLiteral("%1 at %2").arg(QString(li.text).replace(QLatin1Char('\t'), QLatin1Char('|'))).arg(li.rect.x());
            return lines.join(QStringLiteral(" / "));
        };
        for (const QString &text : {QStringLiteral("Colonel Smith, Esq\t\tManagement of it"), QStringLiteral("Colonel Smith, Esq\tMgr\t98 units")}) {
            const QString left = lay(Qt::AlignLeft, text);
            for (Qt::Alignment al : {Qt::AlignHCenter, Qt::AlignRight, Qt::AlignJustify})
                QCOMPARE(lay(al, text), left);
        }
    }

    // The room a line needs beside a picture counts the hang of a bullet:
    // the first line of a list item starts one hang in from the first place
    // it has, so a gap of 20 points beside a picture left an 18-point hanging
    // bullet a line 2 points wide. A gap too narrow for the bullet and a
    // word is passed over, and the line goes below the picture (as for text
    // without a bullet, whose 20 points are enough for a word).
    void bulletHangCountsInTheRoomBesideAPicture()
    {
        auto first = [](bool bullet, double gap) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLeftMargin(18);
            bf.setTextIndent(-18);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(8);
            c.insertText(QStringLiteral("Item with several words that must wrap around"), cf);
            if (bullet) {
                QTextListFormat lf;
                lf.setStyle(QTextListFormat::ListDisc);
                c.createList(lf);
            }
            jp::FrameSpec fs;
            fs.size = QSizeF(200, 300);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            fs.obstacles << QPolygonF(QRectF(-50, -50, 50 + (200 - gap), 70));   // the top 20 points, leaving `gap` on its right
            jp::StoryLayout l;
            l.build(&doc, {fs}, jp::LayoutEnv());
            return l.lineInfo(0).first().rect;
        };
        const auto what = [](const QRectF &r) { return QStringLiteral("line at %1,%2, %3 wide").arg(r.x()).arg(r.y()).arg(r.width()); };
        const QRectF plain = first(false, 20);
        QVERIFY2(plain.y() < 1 && plain.width() > 19.9, qPrintable(what(plain)));
        const QRectF bullet = first(true, 20);
        QVERIFY2(bullet.y() > 19.9 && bullet.width() > 100, qPrintable(what(bullet)));
        // Room for the hang and a word: beside the picture, after the bullet.
        const QRectF roomy = first(true, 60);
        QVERIFY2(roomy.y() < 1 && std::abs(roomy.x() - (140 + 18)) < 0.01 && roomy.width() > 40, qPrintable(what(roomy)));
    }

    // A bullet drawn as a glyph fitted to Publisher's box (a missing symbol
    // font's stand-in) has its outline built once, not for each bullet each
    // time it is painted: 400 arrows took 23 ms a paint.
    void fittedBulletOutlinesAreKept()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        for (int i = 0; i < 400; ++i) {
            QTextBlockFormat bf;
            bf.setLeftMargin(12);
            bf.setTextIndent(-12);
            if (i) c.insertBlock(bf);
            else c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(10);
            c.insertText(QStringLiteral("Item %1").arg(i), cf);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDisc);
            lf.setProperty(jp::tp::BulletFont, QStringLiteral("Wingdings"));
            lf.setProperty(jp::tp::BulletChar, QString(QChar(0xF0E8)));   // the heavy arrow
            c.createList(lf);
        }
        jp::FrameSpec fs;
        fs.size = QSizeF(300, 5000);
        fs.insets = QMarginsF(0, 0, 0, 0);
        jp::StoryLayout lay;
        lay.build(&doc, {fs}, jp::LayoutEnv());
        QImage img(300, 600, QImage::Format_RGB32);
        img.fill(Qt::white);
        const int before = jp::fittedMarkerOutlinesBuiltForTest();
        for (int pass = 0; pass < 2; ++pass) {
            QPainter p(&img);
            lay.paint(&p, 0, jp::PaintOptions());
        }
        const int built = jp::fittedMarkerOutlinesBuiltForTest() - before;
        if (QFontDatabase::hasFamily(QStringLiteral("Wingdings"))) QSKIP("Wingdings is installed here: its own arrow is drawn");
        QVERIFY2(built == 1, qPrintable(QStringLiteral("%1 outlines built for 800 bullets").arg(built)));
    }

    // A bullet drawn in the list's own font (as .pub lists name it: Symbol,
    // Wingdings) keeps that font's regular weight whatever the item's first
    // run is. Publisher's asterisk bullets stand thin beside bold items (its
    // pictures; its PDFs hold one Symbol face); JeffPub drew them bold.
    void bulletInItsOwnFontIgnoresTheRunsWeight()
    {
        // The ink in the 12 points before the text, where only the bullet is.
        auto bulletInk = [](bool bold, bool italic, const QString &bulletFont) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLeftMargin(12);
            bf.setTextIndent(-12);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(24);
            cf.setFontWeight(bold ? QFont::Bold : QFont::Normal);
            cf.setFontItalic(italic);
            c.insertText(QStringLiteral("Item"), cf);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDisc);
            lf.setProperty(jp::tp::BulletChar, QString(QChar(0xF02A)));   // Symbol's asterisk
            lf.setProperty(jp::tp::BulletFont, bulletFont);
            c.createList(lf);
            jp::FrameSpec fs;
            fs.size = QSizeF(200, 40);
            fs.insets = QMarginsF(0, 0, 0, 0);
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            QImage img(800, 160, QImage::Format_RGB32);
            img.fill(Qt::white);
            QPainter p(&img);
            p.scale(4, 4);
            lay.paint(&p, 0, jp::PaintOptions());
            p.end();
            int ink = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < 11 * 4; ++x) ink += 255 - qGray(img.pixel(x, y));
            return ink;
        };
        for (const QString &font : {QStringLiteral("Symbol"), QStringLiteral("Wingdings")}) {
            const int regular = bulletInk(false, false, font);
            QVERIFY2(regular > 1000, qPrintable(QStringLiteral("%1: %2").arg(font).arg(regular)));
            QCOMPARE(bulletInk(true, false, font), regular);    // a bold run's bullet
            QCOMPARE(bulletInk(false, true, font), regular);    // an italic run's
        }
    }

    // A bullet from Symbol or Wingdings is drawn the size and place Publisher's
    // glyph is, measured in its PDFs (SymbolMT, Wingdings-Regular), whichever
    // font the item is in and whether or not this machine has the symbol font:
    // the round Symbol bullet, which nearly every .pub list has, is a circle
    // 0.355 em across with its middle 0.28 em above the baseline (the text
    // font's own bullet, as it was drawn, was 0.24 across in a condensed face
    // and 0.27 in Arial), and the text after a bullet that doesn't hang starts
    // at the glyph's advance, with no gap (it started 4 points on).
    void symbolBulletsHavePublishersSize()
    {
        struct Drawn {
            QRectF ink;       // in ems of the bullet's size: x from its origin, y (its bottom) up from the baseline
            double textX = 0; // where the text after it starts, in ems
        };
        auto bullet = [](const QString &font, uint code, double size, const QString &run, bool bold) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{run});
            cf.setFontPointSize(12);
            cf.setFontWeight(bold ? QFont::Bold : QFont::Normal);
            c.insertText(QStringLiteral("Item"), cf);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDisc);
            if (code) lf.setProperty(jp::tp::BulletChar, QString(QChar(char16_t(0xF000 + code))));
            lf.setProperty(jp::tp::BulletFont, font);
            lf.setProperty(jp::tp::BulletSize, size);
            c.createList(lf);
            jp::FrameSpec fs;
            fs.size = QSizeF(200, 60);
            fs.insets = QMarginsF(0, 0, 0, 0);
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            const double scale = 16;
            QImage img(int(200 * scale), int(60 * scale), QImage::Format_RGB32);
            img.fill(Qt::white);
            QPainter p(&img);
            p.scale(scale, scale);
            lay.paint(&p, 0, jp::PaintOptions());
            p.end();
            const auto first = lay.lineInfo(0).first();
            int x0 = img.width(), x1 = 0, y0 = img.height(), y1 = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < int(first.rect.x() * scale); ++x)
                    if (255 - qGray(img.pixel(x, y)) > 60) {
                        x0 = std::min(x0, x); x1 = std::max(x1, x + 1);
                        y0 = std::min(y0, y); y1 = std::max(y1, y + 1);
                    }
            Drawn d;
            d.ink = QRectF(x0 / scale / size, (first.baseline - y1 / scale) / size, (x1 - x0) / scale / size, (y1 - y0) / scale / size);
            d.textX = first.rect.x() / size;
            return d;
        };
        auto check = [&](const QString &what, const Drawn &got, const QRectF &want, double advance) {
            const double tol = 0.025;
            QVERIFY2(std::abs(got.ink.x() - want.x()) < tol && std::abs(got.ink.y() - want.y()) < tol
                         && std::abs(got.ink.width() - want.width()) < tol && std::abs(got.ink.height() - want.height()) < tol,
                     qPrintable(QStringLiteral("%1: ink at %2,%3, %4 by %5 em; wanted %6,%7, %8 by %9").arg(what).arg(got.ink.x()).arg(got.ink.y()).arg(got.ink.width())
                                    .arg(got.ink.height()).arg(want.x()).arg(want.y()).arg(want.width()).arg(want.height())));
            if (advance > 0) QVERIFY2(std::abs(got.textX - advance) < 0.01, qPrintable(QStringLiteral("%1: text at %2 em, wanted %3").arg(what).arg(got.textX).arg(advance)));
        };
        // The round bullet: 0.052..0.407 across, 0.103..0.458 up; the advance 0.458.
        for (const QString &run : {QStringLiteral("Arial"), QStringLiteral("Times New Roman"), QStringLiteral("Gill Sans MT")})
            for (bool bold : {false, true})
                check(QStringLiteral("round bullet, %1 %2").arg(run, bold ? "bold" : "regular"), bullet(QStringLiteral("Symbol"), 0, 10, run, bold),
                      QRectF(0.052, 0.103, 0.355, 0.355), 0.458);
        // The others are drawn from a stand-in glyph fitted to the real one's box, when the symbol font is missing
        // (and a font on this machine has the stand-in). The greater-than sign's box is estimated from Publisher's
        // pictures (96 dpi); the rest are measured in its PDFs.
        struct Glyph { QString font; uint code; QRectF want; double advance; };
        const QList<Glyph> glyphs = {
            {QStringLiteral("Symbol"), 0x2A, QRectF(0.070, 0.129, 0.360, 0.405), 0.5},                 // the asterisk
            {QStringLiteral("Symbol"), 0x3E, QRectF(0.090, 0.030, 0.410, 0.440), 0},                   // the greater-than sign
            {QStringLiteral("Wingdings"), 0xE8, QRectF(0.085, 0.0, 0.899, 0.723), 1.067},              // the arrow
            {QStringLiteral("Wingdings"), 0xD8, QRectF(0.084, 0.0, 0.627, 0.724), 0.795},              // the arrowhead
        };
        for (const Glyph &g : glyphs) {
            if (QFontDatabase::hasFamily(g.font)) continue;   // the real glyph is drawn
            jp::SymbolBullet sb;
            QVERIFY(jp::symbolBullet(g.font, g.code, &sb));
            QPainterPath standIn;
            standIn.addText(QPointF(), QFont(QStringLiteral("Arial"), 100), sb.standIn);
            if (standIn.boundingRect().isEmpty()) continue;   // no font here has it
            check(QStringLiteral("%1 %2").arg(g.font).arg(g.code, 0, 16), bullet(g.font, g.code, 12, QStringLiteral("Arial"), false), g.want, g.advance);
        }
    }

    // The text after a bullet that doesn't hang starts at the list's indent
    // from the text area's edge (a quarter inch unless the style says
    // otherwise: 18 in most of Publisher's Web designs, 7.2 in some), or at
    // the bullet's end when that is further (a 10-point bullet's is 4.58, a
    // 32-point arrowhead's 25.4). Publisher's tables of contents and flyers
    // had it so: the bullet at 15.3 ended at 19.88, past 18. A list of JeffPub's
    // own has no indent, and follows its bullet with a little room. The indent is
    // saved and read back as Publisher keeps it (a design's own, 7.2 here).
    void bulletTabStartsTheText()
    {
        auto textX = [](const QString &font, uint code, double size, bool tab, double left = 0, double first = 0) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLeftMargin(left);
            bf.setTextIndent(first);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
            cf.setFontPointSize(9);
            c.insertText(QStringLiteral("More details"), cf);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDisc);
            if (code) lf.setProperty(jp::tp::BulletChar, QString(QChar(char16_t(0xF000 + code))));
            lf.setProperty(jp::tp::BulletFont, font);
            lf.setProperty(jp::tp::BulletSize, size);
            if (tab) lf.setProperty(jp::tp::BulletTab, 18.0);
            c.createList(lf);
            jp::FrameSpec fs;
            fs.size = QSizeF(200, 60);
            fs.insets = QMarginsF(0, 0, 0, 0);
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            return lay.lineInfo(0).first().rect.x();
        };
        QVERIFY(std::abs(textX(QStringLiteral("Symbol"), 0, 10, true) - 18) < 0.01);
        QVERIFY(std::abs(textX(QStringLiteral("Symbol"), 0, 10, false) - 4.58) < 0.01);
        QVERIFY2(std::abs(textX(QStringLiteral("Wingdings"), 0xD8, 32, true) - 25.44) < 0.05, qPrintable(QString::number(textX(QStringLiteral("Wingdings"), 0xD8, 32, true))));
        // A table of contents' entry: the bullet at 15.3 (the left indent 10.8, the first line 4.5) ends at 19.88, past the 18.
        QVERIFY2(std::abs(textX(QStringLiteral("Symbol"), 0, 10, true, 10.8, 4.5) - 19.88) < 0.01, qPrintable(QString::number(textX(QStringLiteral("Symbol"), 0, 10, true, 10.8, 4.5))));
        // A bullet starting nearer the edge has the indent to reach: the left indent 0, the first line 2.
        QVERIFY2(std::abs(textX(QStringLiteral("Symbol"), 0, 10, true, 0, 2) - 18) < 0.01, qPrintable(QString::number(textX(QStringLiteral("Symbol"), 0, 10, true, 0, 2))));
        // Through a saved file.
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(72, 72, 300, 100);
        box->storyId = doc->createStory();
        QTextCursor c(doc->storyDoc(box->storyId));
        c.insertText(QStringLiteral("More details"));
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        lf.setProperty(jp::tp::BulletTab, 7.2);   // a design's own indent, not the quarter inch
        c.createList(lf);
        doc->pages[0]->items.push_back(box);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("tabbed.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TextItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Text) bt = static_cast<const jp::TextItem *>(it.get());
        });
        QVERIFY(bt);
        const QTextBlock b = back->storyDoc(bt->storyId)->begin();
        QVERIFY(b.textList());
        QVERIFY2(std::abs(b.textList()->format().property(jp::tp::BulletTab).toDouble() - 7.2) < 0.01, qPrintable(b.textList()->format().property(jp::tp::BulletTab).toString()));
    }

    // The hyphenation zone: a word is broken only if moving it whole would
    // leave more than the zone empty at the end of the line.
    void hyphenationZone()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 110, 200);
        t->storyId = doc->createStory(QStringLiteral("We met at the international conference today."));
        doc->pages[0]->items.push_back(t);
        auto firstLine = [&](double zone) {
            t->hyphenZone = zone;
            jp::LayoutCache cache;
            jp::RenderOptions opt;
            const auto fl = cache.textFrame(*doc, *t, 1, opt);
            return fl.layout ? fl.layout->lineInfo(0).value(0).text.remove(QChar(0x00AD)) : QString();
        };
        const QString tight = firstLine(0), loose = firstLine(200);
        QVERIFY2(tight.contains(QStringLiteral("inter")), qPrintable(tight));
        QVERIFY2(!loose.contains(QStringLiteral("inter")), qPrintable(loose));
        // A justified line the zone shortened still fits the box (Qt would
        // otherwise spread its words over an endless line).
        {
            QTextCursor jc(doc->storyDoc(t->storyId));
            jc.movePosition(QTextCursor::End);
            jc.insertText(QStringLiteral(" We talked about everything we learned there."));
            QTextBlockFormat jf;
            jf.setAlignment(Qt::AlignJustify);
            jc.mergeBlockFormat(jf);
            t->hyphenZone = 200;
            // Ink of the black plate inside the box: every word is drawn
            // there, justified or not.
            auto ink = [&] {
                const QImage k = jp::renderPlate(*doc, 0, 3, 72);
                int n = 0;
                for (int y = 72; y < 272; ++y)
                    for (int x = 72; x < 182; ++x) n += qGray(k.pixel(x, y)) < 128;
                return n;
            };
            const int justified = ink();
            QTextBlockFormat lf;
            lf.setAlignment(Qt::AlignLeft);
            jc.mergeBlockFormat(lf);
            const int left = ink();
            QVERIFY2(justified > left * 0.9, qPrintable(QStringLiteral("%1 vs %2").arg(justified).arg(left)));
        }
        // Saved with the text box.
        t->hyphenZone = 30;
        QString err;
        auto again = jp::publicationFromBytes(jp::publicationBytes(*doc, QImage()), &err);
        QCOMPARE(static_cast<jp::TextItem *>(again->pages[0]->items[0].get())->hyphenZone, 30.0);
    }

    void lineSpacingBelowLines()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 400);
        t->insets = QMarginsF(0, 0, 0, 0);
        t->storyId = doc->createStory(QString());
        doc->pages[0]->items.push_back(t);
        QTextCursor c(doc->storyDoc(t->storyId));
        QTextCharFormat cf;
        cf.setFontFamilies(QStringList{QStringLiteral("Times New Roman")});
        cf.setFontPointSize(10);
        QTextBlockFormat wide, single;
        wide.setLineHeight(150, QTextBlockFormat::ProportionalHeight);
        single.setLineHeight(100, QTextBlockFormat::ProportionalHeight);
        c.setBlockFormat(wide);
        c.insertText(QStringLiteral("Widely spaced"), cf);
        c.insertBlock(single, cf);
        c.insertText(QStringLiteral("Single spaced"), cf);
        jp::LayoutCache cache;
        jp::RenderOptions opt;
        const auto lines = cache.textFrame(*doc, *t, 1, opt).layout->lineInfo(0);
        QCOMPARE(lines.size(), 2);
        const double s = 10 * (1420.0 + 442 + 307) / 2048, kd = 10 * 442.0 / 2048;
        QVERIFY2(std::abs(lines[0].baseline - (s - kd)) < 0.05, qPrintable(QString::number(lines[0].baseline)));
        // The wide paragraph's extra half line comes before the next paragraph.
        QVERIFY2(std::abs(lines[1].baseline - lines[0].baseline - 1.5 * s) < 0.05, qPrintable(QString::number(lines[1].baseline - lines[0].baseline)));
        // A box exactly three wide lines tall still holds three lines.
        doc->storyDoc(t->storyId)->setPlainText(QString());
        QTextCursor c2(doc->storyDoc(t->storyId));
        c2.setBlockFormat(wide);
        c2.insertText(QStringLiteral("one\ntwo\nthree"), cf);
        QTextCursor all(doc->storyDoc(t->storyId));
        all.select(QTextCursor::Document);
        all.mergeBlockFormat(wide);
        t->rect.setHeight(s + 2 * 1.5 * s + 0.01);
        jp::LayoutCache cache2;
        const auto fl = cache2.textFrame(*doc, *t, 1, opt);
        QCOMPARE(fl.layout->lineInfo(0).size(), 3);
        QVERIFY(!fl.layout->overflow());
    }

    // A Symbol bullet (as .pub lists have) sets the height of its item's
    // first line when it is taller than the text's, at the bullet's own size.
    void listBulletLineHeight()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 120, 300);
        t->storyId = doc->createStory(QString());
        doc->pages[0]->items.push_back(t);
        QTextDocument *d = doc->storyDoc(t->storyId);
        QTextCursor c(d);
        QTextCharFormat cf;
        cf.setFontFamilies(QStringList{QStringLiteral("Times New Roman")});
        cf.setFontPointSize(10);
        QTextBlockFormat bf;
        bf.setLineHeight(100, QTextBlockFormat::ProportionalHeight);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("First item, long enough to wrap onto a second line"), cf);
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        lf.setProperty(jp::tp::BulletFont, QStringLiteral("Symbol"));
        QTextList *list = c.createList(lf);
        c.insertBlock(bf, cf);
        c.insertText(QStringLiteral("Second item"), cf);
        list->add(c.block());
        auto heights = [&] {
            jp::LayoutCache cache;
            jp::RenderOptions opt;
            QVector<double> h;
            for (const auto &li : cache.textFrame(*doc, *t, 1, opt).layout->lineInfo(0)) h << li.rect.height();
            return h;
        };
        QVector<double> h = heights();
        QVERIFY(h.size() >= 3);
        const double symbolLine = 10 * (2059.0 + 450) / 2048, timesLine = 10 * (1420.0 + 442 + 307) / 2048;
        QVERIFY2(std::abs(h[0] - symbolLine) < 0.05, qPrintable(QString::number(h[0])));
        QVERIFY2(std::abs(h[1] - timesLine) < 0.05, qPrintable(QString::number(h[1])));   // the item's second line
        QVERIFY2(std::abs(h.last() - symbolLine) < 0.05, qPrintable(QString::number(h.last())));
        // A smaller bullet no longer sets the height.
        lf.setProperty(jp::tp::BulletSize, 8.0);
        list->setFormat(lf);
        h = heights();
        QVERIFY2(std::abs(h[0] - timesLine) < 0.05, qPrintable(QString::number(h[0])));
    }

    // Widow and orphan control, keep lines together and keep with next, on a
    // story running from one text box into the next.
    void paragraphBreakRules()
    {
        // paras: (line count, flags) ; returns each paragraph's lines in box 1 and box 2.
        enum { Widow = 1, Together = 2, WithNext = 4 };
        auto run = [](const QVector<QPair<int, int>> &paras) {
            auto doc = jp::Document::blank(QSizeF(612, 792));
            auto a = std::make_shared<jp::TextItem>(), b = std::make_shared<jp::TextItem>();
            a->rect = QRectF(72, 72, 300, 150);
            b->rect = QRectF(72, 400, 300, 300);
            a->storyId = doc->createStory();
            b->storyId = a->storyId;
            a->nextId = b->id;
            doc->pages[0]->items.push_back(a);
            doc->pages[0]->items.push_back(b);
            QTextCursor c(doc->storyDoc(a->storyId));
            for (int i = 0; i < paras.size(); ++i) {
                if (i) c.insertBlock();
                QTextBlockFormat bf;
                bf.setProperty(jp::tp::WidowControl, bool(paras[i].second & Widow));
                bf.setProperty(jp::tp::KeepTogether, bool(paras[i].second & Together));
                bf.setProperty(jp::tp::KeepWithNext, bool(paras[i].second & WithNext));
                c.setBlockFormat(bf);
                QStringList lines;
                for (int k = 0; k < paras[i].first; ++k) lines << QStringLiteral("P%1 line %2").arg(i).arg(k);
                c.insertText(lines.join(QChar(QChar::LineSeparator)));
            }
            jp::LayoutCache cache;
            jp::RenderOptions opt;
            const auto fl = cache.textFrame(*doc, *a, 1, opt);
            QVector<QPair<int, int>> out(paras.size());
            for (int f = 0; f < 2; ++f)
                for (const auto &li : fl.layout->lineInfo(f)) {
                    const int pi = li.text.mid(1, li.text.indexOf(' ') - 1).toInt();
                    if (f == 0) ++out[pi].first; else ++out[pi].second;
                }
            return out;
        };
        // How many one-line paragraphs fit in the first box.
        QVector<QPair<int, int>> ones(30, qMakePair(1, 0));
        int fit = 0;
        for (const auto &r : run(ones)) fit += r.first;
        QVERIFY(fit > 6);
        auto fill = [&](int n) { return QVector<QPair<int, int>>(n, qMakePair(1, 0)); };
        // Orphan: one line left at the bottom moves on.
        auto paras = fill(fit - 1) << qMakePair(6, 0);
        QCOMPARE(run(paras).last(), qMakePair(1, 5));
        paras.last().second = Widow;
        QCOMPARE(run(paras).last(), qMakePair(0, 6));
        // Widow: one line carried over takes another with it.
        paras = fill(fit - 5) << qMakePair(6, 0);
        QCOMPARE(run(paras).last(), qMakePair(5, 1));
        paras.last().second = Widow;
        QCOMPARE(run(paras).last(), qMakePair(4, 2));
        // Keep lines together.
        paras = fill(fit - 3) << qMakePair(6, Together);
        QCOMPARE(run(paras).last(), qMakePair(0, 6));
        // Keep with next: a heading on the last line goes with its paragraph.
        paras = fill(fit - 1) << qMakePair(1, 0) << qMakePair(3, 0);
        auto got = run(paras);
        QCOMPARE(got[fit - 1], qMakePair(1, 0));
        paras[fit - 1].second = WithNext;
        got = run(paras);
        QCOMPARE(got[fit - 1], qMakePair(0, 1));
        QCOMPARE(got.last(), qMakePair(0, 3));
    }

    // Distribute spreads the last line too; baseline alignment puts lines on
    // the baseline guides; a raised cap stands on the first line.
    void paragraphDistributeBaselineRaisedCap()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto box = [&](const QRectF &r, const QString &text, const std::function<void(QTextBlockFormat &)> &fmt) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = r;
            t->storyId = doc->createStory(text);
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextBlockFormat bf;
            fmt(bf);
            c.mergeBlockFormat(bf);
            doc->pages[0]->items.push_back(t);
            return t;
        };
        // Distribute: "Hi there" reaches both sides of the box.
        auto d = box(QRectF(100, 100, 300, 40), QStringLiteral("Hi there"), [](QTextBlockFormat &bf) {
            bf.setAlignment(Qt::AlignJustify);
            bf.setProperty(jp::tp::Distribute, true);
        });
        jp::PaintContext ctx;
        ctx.doc = doc.get();
        jp::LayoutCache cache;
        ctx.cache = &cache;
        ctx.opt.output = true;
        QImage img(612, 792, QImage::Format_RGB32);
        img.fill(Qt::white);
        {
            QPainter p(&img);
            jp::Renderer::paintPage(&p, ctx, 0);
        }
        int minX = 9999, maxX = -1;
        for (int y = 100; y < 140; ++y)
            for (int x = 100; x < 400; ++x)
                if (qGray(img.pixel(x, y)) < 128) { minX = std::min(minX, x); maxX = std::max(maxX, x); }
        QVERIFY2(minX < 112 && maxX > 386, qPrintable(QStringLiteral("%1..%2").arg(minX).arg(maxX)));
        Q_UNUSED(d);
        // Baseline guides every 18 pt: lines land 18 pt apart.
        doc->masters.first()->grid.baseline = 18;
        auto g = box(QRectF(100, 200, 300, 200), QStringLiteral("one\ntwo\nthree"), [](QTextBlockFormat &bf) { bf.setProperty(jp::tp::AlignToBaseline, true); });
        jp::RenderOptions opt;
        const auto lines = cache.textFrame(*doc, *g, 1, opt).layout->lineInfo(0);
        QCOMPARE(lines.size(), 3);
        // (Paragraph spacing can carry a line past one guide to the next.)
        auto onGrid = [](double d) { const double r = std::fmod(d, 18.0); return d > 1 && (r < 0.01 || r > 17.99); };
        QVERIFY2(onGrid(lines[1].rect.top() - lines[0].rect.top()) && onGrid(lines[2].rect.top() - lines[1].rect.top()),
                 qPrintable(QStringLiteral("%1 %2 %3").arg(lines[0].rect.top()).arg(lines[1].rect.top()).arg(lines[2].rect.top())));
        // A raised cap: room above the first line, and only it is indented.
        const QString words = QStringLiteral("Once upon a time there was a long paragraph with enough words to fill several lines of this box, "
                                             "and then it went on a good while longer so that it runs well past the drop cap's lines.");
        auto drop = box(QRectF(100, 450, 200, 200), words, [](QTextBlockFormat &bf) { bf.setProperty(jp::tp::DropCapLines, 3); bf.setProperty(jp::tp::DropCapChars, 1); });
        auto up = box(QRectF(320, 450, 200, 200), words, [](QTextBlockFormat &bf) {
            bf.setProperty(jp::tp::DropCapLines, 3);
            bf.setProperty(jp::tp::DropCapChars, 1);
            bf.setProperty(jp::tp::DropCapUpper, true);
        });
        const auto dl = cache.textFrame(*doc, *drop, 1, opt).layout->lineInfo(0), ul = cache.textFrame(*doc, *up, 1, opt).layout->lineInfo(0);
        QVERIFY(ul[0].rect.top() > dl[0].rect.top() + 5);      // room for the raised letter
        QVERIFY2(ul.size() >= 4 && dl.size() >= 5, qPrintable(QStringLiteral("%1 %2").arg(ul.size()).arg(dl.size())));
        QVERIFY(std::abs(ul[1].rect.left() - ul[2].rect.left()) < 0.5);   // a raised one only the first
        QVERIFY(ul[0].rect.left() > ul[1].rect.left() + 5);
        QVERIFY(dl[1].rect.left() > dl[3].rect.left() + 5);      // a dropped cap indents its lines

        // A gradient text fill: red on the left of the word, blue on the right.
        auto gt = std::make_shared<jp::TextItem>();
        gt->rect = QRectF(50, 700, 500, 80);
        gt->storyId = doc->createStory(QStringLiteral("MMMMMMMMMM"));
        {
            QTextCursor c(doc->storyDoc(gt->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontPointSize(48);
            const jp::Fill g = jp::Fill::gradient(jp::ColorRef::rgb(QColor(220, 0, 0)), jp::ColorRef::rgb(QColor(0, 0, 220)), 0);
            f.setProperty(jp::tp::TextFill, QString::fromUtf8(QJsonDocument(g.toJson()).toJson(QJsonDocument::Compact)));
            c.mergeCharFormat(f);
        }
        doc->pages[0]->items.push_back(gt);
        QImage gimg(612, 792, QImage::Format_RGB32);
        gimg.fill(Qt::white);
        {
            jp::LayoutCache gc;
            ctx.cache = &gc;
            QPainter p(&gimg);
            jp::Renderer::paintPage(&p, ctx, 0);
        }
        auto inkColor = [&](int x0, int x1) {
            long r = 0, b = 0;
            for (int y = 700; y < 780; ++y)
                for (int x = x0; x < x1; ++x) {
                    const QRgb px = gimg.pixel(x, y);
                    if (qGray(px) < 200) { r += qRed(px); b += qBlue(px); }
                }
            return qMakePair(r, b);
        };
        const auto leftInk = inkColor(55, 120), rightInk = inkColor(380, 545);
        QVERIFY2(leftInk.first > leftInk.second && rightInk.second > rightInk.first,
                 qPrintable(QStringLiteral("%1/%2 %3/%4").arg(leftInk.first).arg(leftInk.second).arg(rightInk.first).arg(rightInk.second)));
    }

    // Many hyperlinks in one story (a link directory) open in moments and
    // all come back: Qt finished each insertion on its own (6,000 links took
    // 10 seconds), and the reader dropped every address past 10,000.
    void pubManyLinksOpenQuickly()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 400, 600);
        t->storyId = doc->createStory(QString());
        const int links = 12000;
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.beginEditBlock();
            for (int i = 0; i < links; ++i) {
                QTextCharFormat lf;
                lf.setAnchor(true);
                lf.setAnchorHref(QStringLiteral("https://example.com/%1").arg(i));
                c.insertText(QStringLiteral("site%1").arg(i), lf);
                c.insertText(QStringLiteral(" "), QTextCharFormat());
            }
            c.endEditBlock();
        }
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("links.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        decltype(importPublisherFile(path, &err)) back;
        const qint64 ms = bestNanoseconds([&] { back = importPublisherFile(path, &err); }, 3, [&] { back.reset(); }) / 1000000;
        QVERIFY(back);
        QSet<QString> found;
        for (const auto &st : back->stories)
            for (QTextBlock b = st->doc->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it)
                    if (it.fragment().charFormat().isAnchor()) found << it.fragment().charFormat().anchorHref();
        QCOMPARE(found.size(), links);
        QVERIFY(found.contains(QStringLiteral("https://example.com/11999")));
        QVERIFY2(ms < 15000, qPrintable(QStringLiteral("%1 ms").arg(ms)));   // was over 40 s; a sanitizer build takes 5
        // And as .jpub, reopened just as quickly.
        const QString jpub = dir.filePath(QStringLiteral("links.jpub"));
        QVERIFY2(savePublication(*back, jpub, QImage(), &err), qPrintable(err));
        const qint64 reopened = bestNanoseconds([&] { QVERIFY(loadPublication(jpub, &err)); }, 3) / 1000000;
        QVERIFY2(reopened < 15000, qPrintable(QStringLiteral("%1 ms").arg(reopened)));
    }

    // Fields and hyperlinks are saved as Publisher saves them: each story's
    // TOKN section lists them (page numbers, dates and times in Publisher's
    // 17 formats, hyperlinks with their address) and they come back as
    // fields and links. Publisher counted the same fields in JeffPub's copies
    // of its samples as in its own files, and pictured them the same (Oct 7).
    void pubFieldsAndLinksRoundTrip()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto box = [&](const QString &text, const QString &field, double y) {
            auto t = std::make_shared<TextItem>();
            t->rect = QRectF(72, y, 300, 30);
            t->storyId = doc->createStory(text);
            if (!field.isEmpty()) {
                QTextCursor c(doc->storyDoc(t->storyId));
                c.movePosition(QTextCursor::End);
                QTextCharFormat ff;
                ff.setProperty(tp::Field, field);
                c.insertText(QString(QChar::ObjectReplacementCharacter), ff);
            }
            doc->pages[0]->items.push_back(t);
            return t;
        };
        box(QStringLiteral("f0 X"), QStringLiteral("page"), 72);
        box(QStringLiteral("Next "), QStringLiteral("page:next"), 112);
        box(QStringLiteral("Prev "), QStringLiteral("page:prev"), 152);
        box(QStringLiteral("Day "), QStringLiteral("datetime:MMMM d, yyyy"), 192);
        box(QStringLiteral("Clock "), QStringLiteral("datetime:h:mm:ss AP"), 232);
        auto linked = box(QStringLiteral("Visit "), QString(), 272);
        {
            QTextCursor c(doc->storyDoc(linked->storyId));
            c.movePosition(QTextCursor::End);
            QTextCharFormat lf;
            lf.setAnchor(true);
            lf.setAnchorHref(QStringLiteral("https://example.com/page"));
            c.insertText(QStringLiteral("example site"), lf);
            c.insertText(QStringLiteral(" now"), QTextCharFormat());
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("fields.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        const QByteArray q = cfb::readStream(path, QStringLiteral("Quill/QuillSub/CONTENTS"));
        // A page number's section, byte for byte as Publisher wrote one for
        // "f0 X#", then the empty payload header just past it (without it
        // Publisher refused the file).
        const QByteArray page = QByteArray::fromHex("010000000c000000ffff01000400000006000000160000000022000000000122010000000222fbffffff0a0000000022ffffffff"
                                                    "00000000000000000eca59d6");
        QVERIFY(q.contains(page));
        // A date: its kind (1), format number (3), language and format.
        QVERIFY(q.contains(QByteArray::fromHex("10000100030009042000") + QByteArray(reinterpret_cast<const char *>(u"MMMM d, yyyy"), 24)));
        // A time: kind 12, number 15, "am/pm" for AP.
        QVERIFY(q.contains(QByteArray::fromHex("11000c000f0009042000") + QByteArray(reinterpret_cast<const char *>(u"h:mm:ss am/pm"), 26)));
        // The hyperlink: flags 0x8C0, its 12 letters, kind 1; its address.
        QVERIFY(q.contains(QByteArray::fromHex("160000000022c00800000122") + QByteArray::fromHex("0c0000000222") + QByteArray::fromHex("01000000")));
        QVERIFY(q.contains(QByteArray::fromHex("1800") + QByteArray(reinterpret_cast<const char *>(u"https://example.com/page"), 48)));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QStringList fields, links;
        for (const auto &it : back->pages[0]->items) {
            auto *t = dynamic_cast<TextItem *>(it.get());
            QVERIFY(t);
            for (QTextBlock b = back->storyDoc(t->storyId)->begin(); b.isValid(); b = b.next())
                for (auto f = b.begin(); !f.atEnd(); ++f) {
                    const QTextCharFormat cf = f.fragment().charFormat();
                    if (!cf.stringProperty(tp::Field).isEmpty()) fields << cf.stringProperty(tp::Field);
                    if (cf.isAnchor()) links << cf.anchorHref() + QLatin1Char('=') + f.fragment().text();
                }
        }
        QCOMPARE(fields, (QStringList{"page", "page:next", "page:prev", "datetime:MMMM d, yyyy", "datetime:h:mm:ss AP"}));
        QCOMPARE(links, QStringList{QStringLiteral("https://example.com/page=example site")});

        // A damaged TOKN section (its entry and count claim more than the
        // stream holds) loses its fields, never the publication's text.
        QFile in(path);
        QVERIFY(in.open(QIODevice::ReadOnly));
        cfb::File file;
        QVERIFY(cfb::read(in.readAll(), &file));
        in.close();
        QByteArray quill = file.stream(QStringLiteral("Quill/QuillSub/CONTENTS"));
        for (qsizetype entry = quill.indexOf("TOKN") - 2; entry > 0 && entry < 512; entry = quill.indexOf("TOKN", entry + 3) - 2) {
            const quint32 at = qFromLittleEndian<quint32>(quill.constData() + entry + 16);
            qToLittleEndian<quint32>(0x00ffffff, quill.data() + entry + 20);
            qToLittleEndian<quint32>(50000, quill.data() + at);
        }
        QVERIFY(file.setStream(QStringLiteral("Quill/QuillSub/CONTENTS"), quill));
        const QString damaged = dir.filePath(QStringLiteral("damaged.pub"));
        QFile out(damaged);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write(cfb::write(file));
        out.close();
        auto rescued = importPublisherFile(damaged, &err);
        QVERIFY2(rescued, qPrintable(err));
        QCOMPARE(int(rescued->pages[0]->items.size()), 6);
    }

    // Publisher's date formats are Windows' letters; Qt's differ only in
    // AM/PM and the one- and three-letter years.
    void pubDateFormats()
    {
        QCOMPARE(jp::pubDateFormat(QStringLiteral(" dddd, MMMM d, yyyy")), QStringLiteral("dddd, MMMM d, yyyy"));
        QCOMPARE(jp::pubDateFormat(QStringLiteral(" M/d/yyyy h:mm:ss am/pm")), QStringLiteral("M/d/yyyy h:mm:ss AP"));
        QCOMPARE(jp::pubDateFormat(QStringLiteral("h:mm tt")), QStringLiteral("h:mm AP"));
        QCOMPARE(jp::pubDateFormat(QStringLiteral("d-MMM-y")), QStringLiteral("d-MMM-yy"));
        QCOMPARE(jp::pubDateFormat(QStringLiteral("yyy")), QStringLiteral("yyyy"));
        // Every one of Publisher's own formats is in JeffPub's Date and Time list.
        for (const char *f : {" M/d/yyyy", " dddd, MMMM d, yyyy", " MMMM d, yyyy", " M/d/yy", " yyyy-MM-dd", " d-MMM-yy", " M.d.yyyy", " MMM. d, yy",
                              " d MMMM yyyy", " MMMM yy", " MMM-yy", " M/d/yyyy h:mm am/pm", " M/d/yyyy h:mm:ss am/pm", " h:mm am/pm", " h:mm:ss am/pm",
                              " HH:mm", " HH:mm:ss"})
            QVERIFY2(jp::dateTimeFormats().contains(jp::pubDateFormat(QString::fromLatin1(f))), f);
    }

    // "Next page" and "previous page" numbers show the page of the next or
    // previous box of a linked chain, and "#" in a box without one, as in
    // Publisher.
    void nextAndPreviousPageNumbers()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        doc->addPage();
        doc->addPage();
        auto a = std::make_shared<TextItem>();
        a->rect = QRectF(72, 72, 300, 100);
        a->storyId = doc->createStory(QStringLiteral("Story"));
        auto b = std::make_shared<TextItem>();
        b->rect = QRectF(72, 72, 300, 100);
        b->storyId = a->storyId;
        a->nextId = b->id;
        doc->pages[0]->items.push_back(a);
        doc->pages[2]->items.push_back(b);
        LayoutCache cache;
        FrameSpec sa = Renderer::frameSpec(*doc, *a, 1, RenderOptions()), sb = Renderer::frameSpec(*doc, *b, 3, RenderOptions());
        QCOMPARE(sa.ctx.resolve(QStringLiteral("page:next")), QStringLiteral("3"));
        QCOMPARE(sb.ctx.resolve(QStringLiteral("page:prev")), QStringLiteral("1"));
        QCOMPARE(sa.ctx.resolve(QStringLiteral("page:prev")), QStringLiteral("#"));
        QCOMPARE(sb.ctx.resolve(QStringLiteral("page:next")), QStringLiteral("#"));
        QCOMPARE(sb.ctx.resolve(QStringLiteral("page")), QStringLiteral("3"));
    }

    // A .pub page number field is a "#" whose character run has 00 = 5 (low
    // byte) and 22 = -1, as Publisher writes it; JeffPub showed the "#".
    void pubPageNumberField()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        doc->addPage();
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 200, 40);
        t->storyId = doc->createStory(QStringLiteral("Page "));
        QTextCursor c(doc->storyDoc(t->storyId));
        c.movePosition(QTextCursor::End);
        QTextCharFormat ff;
        ff.setProperty(tp::Field, QStringLiteral("page"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), ff);
        doc->masters[0]->items.push_back(t);   // on every page
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("pn.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        const QByteArray q = cfb::readStream(path, QStringLiteral("Quill/QuillSub/CONTENTS"));
        // The story text holds "#" where the number goes (UTF-16LE).
        QVERIFY(q.contains(QByteArray(reinterpret_cast<const char *>(u"Page #"), 12)));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(!back->masters.isEmpty() && !back->masters[0]->items.empty());
        auto *bt = dynamic_cast<TextItem *>(back->masters[0]->items[0].get());
        QVERIFY(bt);
        QTextCursor bc(back->storyDoc(bt->storyId));
        bc.movePosition(QTextCursor::End);   // charFormat() is the character before
        QCOMPARE(bc.charFormat().stringProperty(tp::Field), QStringLiteral("page"));
        QCOMPARE(back->storyDoc(bt->storyId)->toPlainText(), QStringLiteral("Page ") + QChar(QChar::ObjectReplacementCharacter));
    }

    // The first page number and the number style are kept in a .pub file: the
    // DOCUMENT chunk names (field 2f) a section chunk (type 75) with an entry
    // for the first page holding its start number (02) and, unless it is 1 2
    // 3, its style (03: 1 I II III, 2 i ii iii, 3 A B C, 4 a b c). A
    // publication numbered 1 2 3 has no such chunk, as in Publisher's files.
    void pubFirstPageNumberAndStyle()
    {
        using namespace jp;
        struct Case { int first; QString style; };
        const QStringList styles = {QString(), QStringLiteral("ROMAN"), QStringLiteral("roman"), QStringLiteral("ALPHA"), QStringLiteral("alpha")};
        auto le16 = [](int v) {
            QByteArray b;
            b.append(char(v & 0xff));
            b.append(char(v >> 8));
            return b;
        };
        QTemporaryDir dir;
        int n = 0;
        for (const Case &c : {Case{5, QStringLiteral("roman")}, Case{1, QStringLiteral("ROMAN")}, Case{12, QStringLiteral("alpha")},
                              Case{3, QStringLiteral("ALPHA")}, Case{7, QString()}, Case{1000, QStringLiteral("roman")}, Case{1, QString()}}) {
            auto doc = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 3);
            doc->setup.firstPageNumber = c.first;
            doc->setup.pageNumberFormat = c.style;
            const QString path = dir.filePath(QStringLiteral("num%1.pub").arg(n++));
            QString err;
            QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
            auto back = importPublisherFile(path, &err);
            QVERIFY2(back, qPrintable(err));
            QCOMPARE(back->pages.size(), 3);
            QCOMPARE(back->setup.firstPageNumber, c.first);
            QCOMPARE(back->setup.pageNumberFormat, c.style);
            // The entry: first page (01 = 1), start number, and style.
            const QByteArray contents = cfb::readStream(path, QStringLiteral("Contents"));
            const QByteArray first = QByteArray::fromHex("012001000000") + QByteArray::fromHex("0218");
            if (c.first == 1 && c.style.isEmpty()) {
                QVERIFY(!contents.contains(first));
            } else {
                QByteArray entry = first + le16(c.first);
                if (!c.style.isEmpty()) entry += QByteArray::fromHex("0318") + le16(int(styles.indexOf(c.style)));
                QVERIFY(contents.contains(entry));
            }
        }
        // Publisher won't open a file whose numbering starts past 1000.
        auto doc = Document::blank(QSizeF(612, 792));
        doc->setup.firstPageNumber = 5000;
        const QString path = dir.filePath(QStringLiteral("big.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->setup.firstPageNumber, 1000);
    }

    // A hyphenation zone read from a .pub file stops at 720 points (10 inches), the writer's limit too: the
    // text layout takes it as a distance and a huge one would break nothing.
    void pubHyphenationZoneIsClamped()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(36, 36, 300, 40);
        box->storyId = doc->createStory(QStringLiteral("Internationalization"));
        box->hyphenZone = 36;
        doc->pages[0]->items.push_back(box);
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 1;
        t->cols = 2;
        t->colW = {100, 100};
        t->rowH = {40};
        t->rect = QRectF(36, 100, 0, 0);
        t->syncRect();
        t->cells.resize(2);
        for (jp::TableCell &c : t->cells) c.storyId = doc->createStory(QStringLiteral("Cell"));
        t->cell(0, 1).hyphenZone = 36;
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("zones.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QFile in(path);
        QVERIFY(in.open(QIODevice::ReadOnly));
        jp::cfb::File file;
        QVERIFY(jp::cfb::read(in.readAll(), &file));
        in.close();
        QByteArray contents = file.stream(QStringLiteral("Contents"));
        // Each story's record holds its zone (0a, a u32 of EMU): 36 points are 457200.
        const QByteArray zone("\x0a\x20\xf0\xf9\x06\x00", 6);
        QCOMPARE(contents.count(zone), 2);
        contents.replace(zone, QByteArray("\x0a\x20\xff\xff\xff\xff", 6));
        QVERIFY(file.setStream(QStringLiteral("Contents"), contents));
        QFile out(path);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write(jp::cfb::write(file));
        out.close();
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TextItem *bx = nullptr;
        const jp::TableItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Text) bx = static_cast<const jp::TextItem *>(it.get());
            if (it->type() == jp::ItemType::Table) bt = static_cast<const jp::TableItem *>(it.get());
        });
        QVERIFY(bx && bt);
        QCOMPARE(bx->hyphenZone, 720.0);
        for (const jp::TableCell &c : bt->cells) QCOMPARE(c.hyphenZone, 720.0);
    }

    // Line spacing in points, as Publisher sets it (measured in its PDFs of 10 and 24 point text in Times New
    // Roman, Arial, and Calibri, every value from 1 to 30 points): "Exactly" spaces the lines by the value
    // whatever the text, with the baseline one descent above the bottom of the line, or, when the line is
    // shorter than the font's own, the same part of the line (the whole line shrinks); "At least" is the
    // larger of the value and the font's own spacing, extra above the line.
    void lineSpacingInPointsAsPublisher()
    {
        const double size = 100, single = (1420.0 + 442 + 307) / 2048 * size, descent = 442.0 / 2048 * size;   // Times New Roman's own
        struct Case { QTextBlockFormat::LineHeightTypes type; double value, pitch, first; };
        const double closer = 1 - descent / single;
        const QVector<Case> cases = {
            {QTextBlockFormat::FixedHeight, 50, 50, 50 * closer},
            {QTextBlockFormat::FixedHeight, 1.4, 1.4, 1.4 * closer},
            {QTextBlockFormat::FixedHeight, 130, 130, 130 - descent},
            {QTextBlockFormat::MinimumHeight, 1.4, single, single - descent},
            {QTextBlockFormat::MinimumHeight, 50, single, single - descent},
            {QTextBlockFormat::MinimumHeight, 130, 130, 130 - descent},
        };
        for (const Case &k : cases) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Times New Roman")});
            cf.setFontPointSize(size);
            QTextBlockFormat bf;
            bf.setLineHeight(k.value, k.type);
            c.setBlockFormat(bf);
            c.insertText(QStringLiteral("Alpha"), cf);
            c.insertBlock(bf);
            c.insertText(QStringLiteral("Bravo"), cf);
            c.insertBlock(bf);
            c.insertText(QStringLiteral("Charlie"), cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(600, 2000);
            fs.insets = QMarginsF(0, 0, 0, 0);
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            const auto lines = lay.lineInfo(0);
            const QString what = QStringLiteral("%1 %2").arg(int(k.type)).arg(k.value);
            QCOMPARE(lines.size(), 3);
            QVERIFY2(std::abs(lines[1].baseline - lines[0].baseline - k.pitch) < 0.05, qPrintable(what + QStringLiteral(" pitch ") + QString::number(lines[1].baseline - lines[0].baseline)));
            QVERIFY2(std::abs(lines[2].baseline - lines[1].baseline - k.pitch) < 0.05, qPrintable(what));
            QVERIFY2(std::abs(lines[0].baseline - k.first) < 0.1, qPrintable(what + QStringLiteral(" first ") + QString::number(lines[0].baseline) + QStringLiteral(" not ") + QString::number(k.first)));
        }
    }

    // A paragraph's "At least" and "Exactly" line spacing are different records in a .pub file (the points
    // with 1 added, or as they are), so each is kept as its own kind.
    void atLeastLineSpacingKeptInPubFiles()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(72, 72, 300, 300);
        box->storyId = doc->createStory();
        QTextDocument *sd = doc->storyDoc(box->storyId);
        QTextCursor c(sd);
        QTextBlockFormat exact, atLeast, lines;
        exact.setLineHeight(12, QTextBlockFormat::FixedHeight);
        atLeast.setLineHeight(1.4, QTextBlockFormat::MinimumHeight);
        lines.setLineHeight(150, QTextBlockFormat::ProportionalHeight);
        c.select(QTextCursor::Document);
        c.removeSelectedText();
        c.setBlockFormat(exact);
        c.insertText(QStringLiteral("Exactly"));
        c.insertBlock(atLeast);
        c.insertText(QStringLiteral("At least"));
        c.insertBlock(lines);
        c.insertText(QStringLiteral("Lines"));
        doc->pages[0]->items.push_back(box);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("spacing.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QTextDocument *bd = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Text) bd = back->storyDoc(static_cast<jp::TextItem *>(it.get())->storyId);
        });
        QVERIFY(bd);
        QVERIFY(bd->blockCount() >= 3);
        QCOMPARE(int(bd->findBlockByNumber(0).blockFormat().lineHeightType()), int(QTextBlockFormat::FixedHeight));
        QCOMPARE(bd->findBlockByNumber(0).blockFormat().lineHeight(), 12.0);
        QCOMPARE(int(bd->findBlockByNumber(1).blockFormat().lineHeightType()), int(QTextBlockFormat::MinimumHeight));
        QVERIFY(std::abs(bd->findBlockByNumber(1).blockFormat().lineHeight() - 1.4) < 0.001);
        QCOMPARE(int(bd->findBlockByNumber(2).blockFormat().lineHeightType()), int(QTextBlockFormat::ProportionalHeight));
    }

    // A text box's hyphenation zone is kept in a .pub file with its story's
    // hyphenation setting, in every box of a linked chain.
    void pubTextBoxHyphenationZoneKept()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto box = [&](double y, bool hyphenate, double zone) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(72, y, 200, 60);
            t->storyId = doc->createStory(QStringLiteral("Internationalization characteristics"));
            t->hyphenate = hyphenate;
            t->hyphenZone = zone;
            doc->pages[0]->items.push_back(t);
            return t;
        };
        box(72, true, 18);
        box(144, true, 36);
        box(216, false, 30);
        auto first = box(288, true, 27);
        auto second = box(360, true, 27);
        second->storyId = first->storyId;
        first->nextId = second->id;

        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("box-zone.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/box-zone.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QMap<int, const jp::TextItem *> boxes;   // by top edge
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Text) boxes.insert(int(std::lround(it->rect.top())), static_cast<const jp::TextItem *>(it.get()));
        });
        QCOMPARE(boxes.size(), 5);
        auto check = [&](int y, bool hyphenate, double zone) {
            const jp::TextItem *t = boxes.value(y);
            return t && t->hyphenate == hyphenate && std::abs(t->hyphenZone - zone) < 0.01;
        };
        QVERIFY(check(72, true, 18));
        QVERIFY(check(144, true, 36));
        QVERIFY(check(216, false, 30));
        QVERIFY(check(288, true, 27));
        QVERIFY(check(360, true, 27));
    }

    // A right-to-left paragraph keeps its direction when saved and reopened.
    void rightToLeftParagraph()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QString::fromUtf8("\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d"));
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            QTextBlockFormat f;
            f.setLayoutDirection(Qt::RightToLeft);
            f.setAlignment(Qt::AlignRight);
            c.mergeBlockFormat(f);
        }
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("rtl.jpub"));
        QString err;
        QVERIFY2(jp::savePublication(*doc, path, QImage(), &err), qPrintable(err));
        auto back = jp::loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TextItem *bt = static_cast<const jp::TextItem *>(back->pages[0]->items.front().get());
        QCOMPARE(back->storyDoc(bt->storyId)->begin().blockFormat().layoutDirection(), Qt::RightToLeft);
    }

    // Edit Wrap Points: start from the outline, drag a point, add one on an
    // edge, delete it, and text wraps around the new outline.
    // A line may break after a hyphen before a digit, as in Publisher:
    // "(555) 012-" then "3456" (Qt's own rules keep the number whole).
    void breakAfterHyphenBeforeDigit()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->storyId = doc->createStory(QStringLiteral("Call us at (555) 012-3456 today"));
        doc->pages[0]->items.push_back(t);
        LayoutCache cache;
        RenderOptions opt;
        auto firstLine = [&](double width) {
            t->rect = QRectF(72, 72, width, 200);
            QString s = cache.textFrame(*doc, *t, 1, opt).layout->lineInfo(0).value(0).text;
            return s.remove(QChar(0x00AD)).remove(QChar(0x200B)).trimmed();
        };
        // Just too narrow for the whole number: the line ends at its hyphen.
        double w = 40;
        while (w < 400 && !firstLine(w).contains(QLatin1String("012-"))) w += 1;
        QCOMPARE(firstLine(w), QStringLiteral("Call us at (555) 012-"));
        QVERIFY(!firstLine(w).contains(QLatin1String("3456")));
    }

    // A drop cap is the first character of its paragraph, and the line beside it begins after it: the line's first position, where Home goes, and where
    // a click at the line's start lands are the position after the cap (a text engine that has the word processor's hidden text must not give them as
    // the cap's own).
    void aDropCapLineBeginsAfterTheCap()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextBlockFormat bf;
        bf.setProperty(jp::tp::DropCapLines, 3);
        bf.setProperty(jp::tp::DropCapChars, 1);
        c.setBlockFormat(bf);
        QTextCharFormat cf;
        cf.setFontPointSize(11);
        c.insertText(QStringLiteral("Dropcap paragraph text that runs on for a few lines so the layout has something to wrap around the big letter at its start."), cf);
        jp::StoryLayout lay;
        jp::FrameSpec fs;
        fs.size = QSizeF(300, 400);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.hyphenate = false;
        lay.build(&doc, {fs}, jp::LayoutEnv());
        const auto lines = lay.lineInfo(0);
        QVERIFY(lines.size() > 2);
        QCOMPARE(lines[0].docPos, 1);
        QCOMPARE(lay.lineBriefs().first().pos, 1);
        QCOMPARE(lay.firstPosition(0), 1);
        QVERIFY2(lay.hitTest(0, QPointF(lines[0].rect.left() + 0.5, lines[0].rect.center().y())) >= 1, "a click at the start of the first line lands after the cap");
    }
};

JP_TEST_CLASS(PubTextTests)
#include "tst_pub_text.moc"

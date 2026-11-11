#include "tst_write.h"
#include "core/commentops.h"


// English gets "1 word" and "652 words" from the suite's own plural file, not the "(s)" of the source text.
void WriteTests::theWordCountSaysWordAndWordsInEnglish()
{
    QTranslator english;
    if (!english.load(QStringLiteral(":/i18n/jeffoffice_en.qm"))) QSKIP("the translations are not built into this build");
    QCoreApplication::installTranslator(&english);
    struct Remove {
        QTranslator *t;
        ~Remove() { QCoreApplication::removeTranslator(t); }
    } remove{&english};
    WriteWindow w;
    w.show();
    QLabel *label = w.findChild<QLabel *>(QStringLiteral("wordCount"));
    QVERIFY(label);
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("one"));
    QTRY_COMPARE(label->text(), QStringLiteral("1 word"));
    QString more;
    for (int i = 1; i < 652; ++i) more += QStringLiteral(" word");
    typeText(*w.document(), w.view()->cursor(), more);
    QTRY_COMPARE(label->text(), QStringLiteral("652 words"));
}


// Every text that counts (%n) has its two English forms in the context the program asks for it in, and no count on screen is worded with "(s)" and an argument, which has no forms at
// all. (The context is the one a call of translate() names, or the class that tr() belongs to: the class of the function the call is in, or the one that names it, in the namespaces
// around it. A form kept under another context is never found, and the text is shown with its marker.)
void WriteTests::countedTextHasItsEnglishFormsAndNoneIsLeftAsPluralMarker()
{
    QHash<QString, QStringList> forms;   // (by context, a tab, and the text)
    QFile ts(QStringLiteral(JW_SOURCE_DIR "/translations/jeffoffice_en.ts"));
    QVERIFY(ts.open(QIODevice::ReadOnly));
    QXmlStreamReader xml(&ts);
    QString context, source;
    QStringList got;
    bool inMessage = false;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == QLatin1String("message")) {
            source.clear();
            got.clear();
            inMessage = true;
        } else if (xml.isStartElement() && xml.name() == QLatin1String("name") && !inMessage) {
            context = xml.readElementText();
        } else if (xml.isStartElement() && xml.name() == QLatin1String("source")) {
            source = xml.readElementText();
        } else if (xml.isStartElement() && xml.name() == QLatin1String("numerusform")) {
            got << xml.readElementText();
        } else if (xml.isEndElement() && xml.name() == QLatin1String("message")) {
            if (!got.isEmpty()) forms.insert(context + QLatin1Char('\t') + source, got);
            inMessage = false;
        }
    }
    QVERIFY(!xml.hasError());
    QStringList problems;
    int counted = 0;
    static const QRegularExpression literal(QStringLiteral("(?:\\b(\\w+(?:::\\w+)*)::)?\\b(translate|tr)\\(\\s*(?:\"((?:[^\"\\\\\\n]|\\\\.)*)\"\\s*,\\s*)?\"((?:[^\"\\\\\\n]|\\\\.)*)\""));
    static const QRegularExpression marker(QStringLiteral("[A-Za-z]\\((s|es)\\)"));
    static const QRegularExpression opens(QStringLiteral("^namespace\\s*([\\w:]*)\\s*\\{")), closes(QStringLiteral("^\\}\\s*(//\\s*)?namespace\\b"));
    static const QRegularExpression definition(QStringLiteral("^[A-Za-z_][^\\n(;]*?\\b([A-Za-z_]\\w*(?:::[A-Za-z_]\\w*)*)::~?[A-Za-z_]\\w*\\s*\\("));
    auto unescaped = [](QString t) { return t.replace(QStringLiteral("\\\""), QStringLiteral("\"")).replace(QStringLiteral("\\\\"), QStringLiteral("\\")); };
    static const QRegularExpression classHead(QStringLiteral("^(?:class|struct)\\s+(\\w+)\\b[^;]*$"));
    // The class a tr() at `at` belongs to, with the namespaces open there: the one whose body at the left margin it is in, or of the function definition at the left margin before it.
    auto classOf = [&](const QString &text, qsizetype at, const QString &named) {
        QStringList spaces;
        QString function, body;
        for (const QString &line : text.left(at).split(QLatin1Char('\n'))) {
            if (const auto m = opens.match(line); m.hasMatch()) spaces << m.captured(1);
            else if (closes.match(line).hasMatch() && !spaces.isEmpty()) spaces.removeLast();
            else if (const auto m = classHead.match(line); m.hasMatch()) body = m.captured(1);
            else if (line.startsWith(QStringLiteral("};"))) body.clear();
            else if (const auto m = definition.match(line); m.hasMatch()) function = m.captured(1);
        }
        spaces.removeAll(QString());
        return (spaces + QStringList{!named.isEmpty() ? named : !body.isEmpty() ? body : function}).join(QStringLiteral("::"));
    };
    for (const char *dir : {"/apps", "/libs"}) {
        QDirIterator it(QStringLiteral(JW_SOURCE_DIR) + QLatin1String(dir), {QStringLiteral("*.cpp")}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QFile f(it.next());
            if (!f.open(QIODevice::ReadOnly)) continue;
            const QString text = QString::fromUtf8(f.readAll());
            for (auto m = literal.globalMatch(text); m.hasNext();) {
                const QRegularExpressionMatch call = m.next();
                const QString s = unescaped(call.captured(4));
                const bool translate = call.captured(2) == QLatin1String("translate");
                if (translate != call.hasCaptured(3)) continue;   // (translate() names its context; tr() does not)
                if (s.contains(QStringLiteral("%n"))) {
                    ++counted;
                    const QString where = translate ? unescaped(call.captured(3)) : classOf(text, call.capturedStart(), call.captured(1));
                    const QStringList f2 = forms.value(where + QLatin1Char('\t') + s);
                    if (f2.size() != 2 || f2[0].isEmpty() || f2[1].isEmpty() || !f2[0].contains(QStringLiteral("%n")) || !f2[1].contains(QStringLiteral("%n")) || f2[0].contains(marker) || f2[1].contains(marker))
                        problems << QStringLiteral("no English forms in %1: ").arg(where) + s.left(60);
                } else if (s.contains(marker)) {
                    problems << QStringLiteral("counts with (s), not %n: ") + s.left(60);
                }
            }
        }
    }
    QVERIFY2(counted >= 19, qPrintable(QStringLiteral("%1 counted texts were found in the sources").arg(counted)));
    QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QLatin1Char('\n'))));
}


void WriteTests::rightClickingInsideASelectionOffersTheWordClickedNotTheCaret()
{
    auto doc = textDoc({"First line has recieve in it and more text after it."});
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    QTextCursor &c = view.cursor();
    const int word = doc->story()->toPlainText().indexOf(QStringLiteral("recieve"));
    c.setPosition(word + 2);
    view.setCursor(c);
    QRect r;
    QVERIFY(view.caretRect(&r));
    // The paragraph is selected, the caret at its end, and the click is on the word.
    c.setPosition(0);
    c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    view.setCursor(c);
    const int selected = int(view.cursor().selectedText().size());
    QTest::mousePress(view.viewport(), Qt::RightButton, Qt::NoModifier, QPoint(r.x() + 1, r.y() + r.height() / 2));
    QCOMPARE(int(view.cursor().selectedText().size()), selected);   // (the selection stays)
    QMenu menu;
    QVERIFY(addSpellingSuggestions(menu, &view));
    const QString chosen = menu.actions().first()->text();
    menu.actions().first()->trigger();
    QCOMPARE(doc->story()->toPlainText(), QStringLiteral("First line has %1 in it and more text after it.").arg(chosen));
    QTest::mouseRelease(view.viewport(), Qt::RightButton, Qt::NoModifier, QPoint(r.x() + 1, r.y() + r.height() / 2));
}


void WriteTests::aMisspelledWordInATableCellIsUnderlinedLikeAnyOther()
{
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid>"
                             "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>fine words</w:t></w:r></w:p></w:tc>"
                             "<w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>a recieve here</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
    auto doc = open(makeDocx("<w:p><w:r><w:t>before</w:t></w:r></w:p>" + table + "<w:p><w:r><w:t>after</w:t></w:r></w:p>", kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.paginateAll();
    QTextDocument *story = doc->story();
    int start = -1;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        if (b.text() == QStringLiteral("a recieve here")) start = b.position() + 2;
    QVERIFY(start > 0);
    QTextCursor &c = view.cursor();
    c.setPosition(start);
    view.setCursor(c);
    QRect from;
    QVERIFY(view.caretRect(&from));
    c.setPosition(start + 7);
    view.setCursor(c);
    QRect to;
    QVERIFY(view.caretRect(&to));
    c.setPosition(0);
    view.setCursor(c);
    int count = 0;
    QRect wave;
    QTRY_VERIFY((wave = redBounds(view.grab().toImage(), view.pageRect(0), &count), count > 8));
    // Under the word, just below its letters, and nowhere else on the page (a placeholder line of the table has none).
    QVERIFY2(wave.left() >= from.left() - 3 && wave.right() <= to.left() + 3, qPrintable(QStringLiteral("%1..%2 for the word at %3..%4").arg(wave.left()).arg(wave.right()).arg(from.left()).arg(to.left())));
    QVERIFY2(wave.top() >= from.top() && wave.bottom() <= from.bottom() + 8, qPrintable(QStringLiteral("%1..%2 for the line at %3..%4").arg(wave.top()).arg(wave.bottom()).arg(from.top()).arg(from.bottom())));
    // As printed, no.
    redBounds(view.pageImage(0, 900), QRect(0, 0, 900, 1200), &count);
    QCOMPARE(count, 0);
}


void WriteTests::aCommentAtTheEndOfAParagraphHasItsBalloonInTheMargin()
{
    // (The comment's marks come after the paragraph's last word: no line of the page holds them, and the balloon still belongs by the paragraph's last line.)
    auto doc = open(makeDocx("<w:p><w:r><w:t>Alpha beta gamma delta.</w:t></w:r></w:p><w:p><w:r><w:t>Second paragraph, with no comment on it.</w:t></w:r></w:p>", kStyles));
    QVERIFY(doc);
    doc->setReviewer(QStringLiteral("Ann Bee"), QStringLiteral("AB"));
    QTextCursor c(doc->story());
    c.setPosition(0);
    c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    QVERIFY(addComment(*doc, c, QStringLiteral("Please check")) >= 0);
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.paginateAll();
    auto margin = [&](bool on) {
        PageView::ReviewMarks marks;
        marks.comments = on;
        view.setReviewMarks(marks);
        const QImage img = view.pageImage(0, 816);
        int dark = 0;
        for (int y = 90; y < 140; ++y)
            for (int x = 730; x < 806; ++x)
                if (qGray(img.pixel(x, y)) < 235) ++dark;
        return dark;
    };
    QCOMPARE(margin(false), 0);
    QVERIFY2(margin(true) > 40, "the balloon of a comment at the end of a paragraph is missing from the margin");
}


void WriteTests::theStyleGalleryListsTheHeadingsOfAFileThatGivesThemNoPriority()
{
    // (A file may leave the display priority out of its own headings; they still come before the built-in ones the document has not used yet, as Heading 1 comes before Heading 4.)
    const QByteArray styles =
        "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>"
        "<w:style w:type=\"paragraph\" w:styleId=\"Title\"><w:name w:val=\"Title\"/><w:basedOn w:val=\"Normal\"/><w:qFormat/><w:rPr><w:sz w:val=\"56\"/></w:rPr></w:style>"
        "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:qFormat/><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:rPr><w:sz w:val=\"36\"/></w:rPr></w:style>"
        "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\"><w:name w:val=\"heading 2\"/><w:basedOn w:val=\"Normal\"/><w:qFormat/><w:pPr><w:outlineLvl w:val=\"1\"/></w:pPr><w:rPr><w:sz w:val=\"28\"/></w:rPr></w:style>";
    auto doc = open(makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>", styles));
    QVERIFY(doc);
    QStringList order;
    for (const StyleEntry &e : doc->styleEntries())
        if (e.quick && !e.character) order << e.id;
    const qsizetype h1 = order.indexOf(QStringLiteral("Heading1")), h2 = order.indexOf(QStringLiteral("Heading2")), h4 = order.indexOf(QStringLiteral("Heading4"));
    QVERIFY(h1 >= 0 && h2 >= 0 && h4 >= 0);
    QVERIFY2(h1 < h4 && h2 < h4, qPrintable(order.join(QLatin1Char(' '))));
}


void WriteTests::theRibbonCommandsDoTheirWork()
{
    WriteWindow w;
    w.show();
    PageView *v = w.view();
    QTest::keyClicks(v, QStringLiteral("alpha beta"));
    QTest::keyClick(v, Qt::Key_Return);
    QTest::keyClicks(v, QStringLiteral("gamma"));
    v->selectAll();
    QTRY_VERIFY(w.act("edit.copy")->isEnabled());
    w.act("fmt.bold")->trigger();
    QTRY_VERIFY(w.act("fmt.bold")->isChecked());
    w.act("fmt.italic")->trigger();
    w.act("fmt.alignCenter")->trigger();
    QTRY_VERIFY(w.act("fmt.alignCenter")->isChecked());
    w.act("para.bullets")->trigger();
    QTRY_VERIFY(w.act("para.bullets")->isChecked());
    QVERIFY(w.document()->story()->findBlockByNumber(1).textList());
    w.act("para.bullets")->trigger();
    QVERIFY(!w.document()->story()->findBlockByNumber(1).textList());
    // The page.
    QTextCursor &c = v->cursor();
    c.movePosition(QTextCursor::End);
    v->setCursor(c);
    w.act("insert.pageBreak")->trigger();
    QTRY_COMPARE(v->pageCount(), 2);   // (the second page is laid out in the background)
    QTRY_COMPARE(w.findChild<QLabel *>(QStringLiteral("pageNumber"))->text(), QStringLiteral("Page 2 of 2"));
    w.act("layout.landscape")->trigger();
    QTRY_VERIFY(w.act("layout.landscape")->isChecked());
    QVERIFY(w.document()->flow().section(0).orientation == flow::Section::Orientation::Landscape);
    QCOMPARE(w.document()->pager().page(0).size.width() > w.document()->pager().page(0).size.height(), true);
    w.act("layout.columns.2")->trigger();
    QCOMPARE(w.document()->flow().section(0).columns, 2);
    w.act("layout.margins.narrow")->trigger();
    QCOMPARE(w.document()->flow().section(0).margins.left(), 36.0);
    w.act("layout.size.a4")->trigger();
    QCOMPARE(w.document()->flow().section(0).paper.width(), 595.3);
    // The zoom.
    w.act("zoom.200")->trigger();
    QCOMPARE(v->zoom(), 2.0);
    w.act("zoom.pageWidth")->trigger();
    QVERIFY(v->zoomMode() == PageView::ZoomMode::PageWidth);
    w.act("zoom.100")->trigger();
    QCOMPARE(v->zoom(), 1.0);
    // The styles, by key and by the gallery's command.
    c.setPosition(0);
    v->setCursor(c);
    w.act("style.heading1")->trigger();
    QCOMPARE(w.document()->story()->firstBlock().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading1"));
    w.act("style.normal")->trigger();
    QCOMPARE(w.document()->story()->firstBlock().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Normal"));
}


// A developer's hook, not a test: JW_ROUNDTRIP_IN names a folder (searched through) of .docx files; each is opened, written again, and opened
// again, and a summary says which could not be, and which came back with other text or other sections. JW_ROUNDTRIP_OUT, if set, is where
// the files JeffWrite wrote go (to be opened by other programs); JW_ROUNDTRIP_LIMIT stops after that many files.
void WriteTests::developerRoundTripOfAFolder()
{
    const QString in = qEnvironmentVariable("JW_ROUNDTRIP_IN"), out = qEnvironmentVariable("JW_ROUNDTRIP_OUT");
    if (in.isEmpty()) QSKIP("JW_ROUNDTRIP_IN is not set");
    if (!out.isEmpty()) QDir().mkpath(out);
    const int limit = qEnvironmentVariableIntValue("JW_ROUNDTRIP_LIMIT") > 0 ? qEnvironmentVariableIntValue("JW_ROUNDTRIP_LIMIT") : 1 << 30;
    int files = 0, opened = 0, refused = 0, changed = 0;
    qint64 largest = 0;
    double slowest = 0;
    QStringList refusals, changes;
    for (QDirIterator it(in, {QStringLiteral("*.docx")}, QDir::Files, QDirIterator::Subdirectories); it.hasNext() && files < limit;) {
        const QString path = it.next();
        ++files;
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) continue;
        const QByteArray bytes = f.readAll();
        largest = std::max<qint64>(largest, bytes.size());
        QElapsedTimer t;
        t.start();
        QString err;
        auto doc = WriteDoc::fromDocx(bytes, &err);
        if (!doc) {
            ++refused;
            refusals << QFileInfo(path).fileName() + QStringLiteral(": ") + err.left(80);
            continue;
        }
        ++opened;
        const QByteArray zip = doc->toDocx(&err);
        doc->pager().pageCount();
        slowest = std::max(slowest, t.elapsed() / 1000.0);
        if (zip.isEmpty()) {
            ++changed;
            changes << QFileInfo(path).fileName() + QStringLiteral(": not written: ") + err;
            continue;
        }
        auto back = WriteDoc::fromDocx(zip, &err);
        QString a = doc->story()->toPlainText(), b = back ? back->story()->toPlainText() : QString();
        a.remove(QChar::ObjectReplacementCharacter);
        b.remove(QChar::ObjectReplacementCharacter);
        if (!back || a != b || back->sectionCount() != doc->sectionCount()) {
            ++changed;
            changes << QFileInfo(path).fileName() + (back ? QStringLiteral(": text or sections differ") : QStringLiteral(": does not open again: ") + err);
        } else if (!out.isEmpty()) {
            QFile o(QDir(out).filePath(QFileInfo(path).fileName()));
            if (o.open(QIODevice::WriteOnly)) o.write(zip);
        }
    }
    qInfo("round trip: %d files, %d opened, %d refused, %d came back changed; the largest %lld bytes, the slowest %.1f s", files, opened, refused, changed, largest, slowest);
    for (const QString &r : std::as_const(refusals)) qInfo("refused: %s", qPrintable(r));
    for (const QString &c : std::as_const(changes)) qInfo("changed: %s", qPrintable(c));
}


// A developer's hook, not a test: the strict round trip. JW_STRICT_IN names a folder (searched through) of .docx files and JW_STRICT_LIST a file
// of paths, one to a line; each is opened and written again, and the file written is checked for well-formed XML in every part the program wrote,
// for losing or changing what the file held (rtcheck.h), and for a story that is not the one that was opened. The summary counts the files for
// each kind of problem; JW_STRICT_OUT keeps the files written; JW_STRICT_LIMIT stops after that many files; JW_STRICT_REPORT names a file to put every
// problem of every file in (the log shows the first sixty).
void WriteTests::developerStrictRoundTrip()
{
    const QString in = qEnvironmentVariable("JW_STRICT_IN"), list = qEnvironmentVariable("JW_STRICT_LIST"), out = qEnvironmentVariable("JW_STRICT_OUT");
    if (in.isEmpty() && list.isEmpty()) QSKIP("JW_STRICT_IN and JW_STRICT_LIST are not set");
    if (!out.isEmpty()) QDir().mkpath(out);
    const int limit = qEnvironmentVariableIntValue("JW_STRICT_LIMIT") > 0 ? qEnvironmentVariableIntValue("JW_STRICT_LIMIT") : 1 << 30;
    QFile report(qEnvironmentVariable("JW_STRICT_REPORT"));
    if (!report.fileName().isEmpty()) report.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QStringList paths;
    if (!in.isEmpty())
        for (QDirIterator it(in, {QStringLiteral("*.docx"), QStringLiteral("*.docm"), QStringLiteral("*.dotx")}, QDir::Files, QDirIterator::Subdirectories); it.hasNext();) paths << it.next();
    if (!list.isEmpty()) {
        QFile lf(list);
        if (lf.open(QIODevice::ReadOnly))
            for (const QByteArray &l : lf.readAll().split('\n'))
                if (!l.trimmed().isEmpty()) paths << QString::fromLocal8Bit(l.trimmed());
    }
    int files = 0, refused = 0, notWritten = 0, malformed = 0, lost = 0, story = 0, clean = 0;
    double slowest = 0;
    QString slowestName;
    QMap<QString, int> kinds;
    QStringList lines;
    for (const QString &path : std::as_const(paths)) {
        if (files >= limit) break;
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) continue;
        const QByteArray bytes = f.readAll();
        ++files;
        QString err;
        QElapsedTimer fileTimer;
        fileTimer.start();
        auto doc = WriteDoc::fromDocx(bytes, &err);
        if (!doc) {
            ++refused;
            continue;
        }
        const QByteArray zip = doc->toDocx(&err);
        if (fileTimer.elapsed() / 1000.0 > slowest) {
            slowest = fileTimer.elapsed() / 1000.0;
            slowestName = QFileInfo(path).fileName();
        }
        if (zip.isEmpty()) {
            ++notWritten;
            lines << QFileInfo(path).fileName() + QStringLiteral(": not written: ") + err;
            continue;
        }
        const QStringList strict = rt::strictProblems(zip, bytes), loss = rt::lossProblems(bytes, zip);
        QStringList differs;
        auto back = WriteDoc::fromDocx(zip, &err);
        if (!back) differs << QStringLiteral("does not open again: ") + err;
        else differs = rt::storyProblems(*doc, *back);
        malformed += !strict.isEmpty();
        lost += !loss.isEmpty();
        story += !differs.isEmpty();
        clean += strict.isEmpty() && loss.isEmpty() && differs.isEmpty();
        for (const QStringList *l : std::initializer_list<const QStringList *>{&strict, &loss, &differs})
            for (const QString &p : *l) {
                QString kind = p.left(p.indexOf(QRegularExpression(QStringLiteral("[:0-9>-]"))) > 0 ? p.indexOf(QRegularExpression(QStringLiteral("[:0-9>-]"))) : 24);
                kind = (l == &strict ? QStringLiteral("strict: ") : l == &loss ? QStringLiteral("loss: ") : QStringLiteral("story: ")) + kind.trimmed();
                kinds[kind] += 1;
            }
        if (report.isOpen())
            for (const QStringList *l : std::initializer_list<const QStringList *>{&strict, &loss, &differs})
                for (const QString &p : *l) report.write((QFileInfo(path).fileName() + QLatin1Char('\t') + (l == &strict ? QStringLiteral("strict") : l == &loss ? QStringLiteral("loss") : QStringLiteral("story")) + QLatin1Char('\t') + p + QLatin1Char('\n')).toUtf8());
        if (lines.size() < 60 && (!strict.isEmpty() || !loss.isEmpty() || !differs.isEmpty()))
            lines << QFileInfo(path).fileName() + QStringLiteral(": ") + (strict + loss + differs).join(QStringLiteral(" | ")).left(300);
        if (!out.isEmpty()) {
            QFile o(QDir(out).filePath(QFileInfo(path).fileName()));
            if (o.open(QIODevice::WriteOnly)) o.write(zip);
        }
    }
    qInfo("strict round trip: %d files, %d refused, %d not written, %d not well-formed, %d lost or changed something, %d came back with another story, %d clean; the slowest to open and save, %s, %.1f s", files, refused, notWritten, malformed, lost, story, clean, qPrintable(slowestName), slowest);
    for (auto it = kinds.cbegin(); it != kinds.cend(); ++it) qInfo("  %s: %d", qPrintable(it.key()), it.value());
    for (const QString &l : std::as_const(lines)) qInfo("  %s", qPrintable(l));
}


// A developer's hook, not a test: JW_BENCH times what editing sections costs in long documents (milliseconds, to look at).
void WriteTests::developerSectionTimings()
{
    if (!qEnvironmentVariableIsSet("JW_BENCH")) QSKIP("JW_BENCH is not set");
    auto ms = [](QElapsedTimer &t) { const double v = t.nsecsElapsed() / 1e6; t.restart(); return v; };
    {
        auto doc = open(makeDocx(paragraphs(5000, 60), kStyles));
        QElapsedTimer t;
        t.start();
        const int pages = doc->pager().pageCount();
        const double paginate = ms(t);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(2500).position() + 5);
        doc->pager().paginate(doc->pager().pageOfPosition(c.position()) + 2);
        t.restart();
        insertSectionBreak(*doc, c);
        const double insert = ms(t);
        doc->pager().paginate(doc->pager().pageOfPosition(c.position()) + 2);
        const double after = ms(t);
        qInfo("sections: %d pages, paginating %.0f ms; a section break in the middle %.0f ms, and the pages around it %.0f ms", pages, paginate, insert, after);
    }
    {
        QByteArray body;
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        for (int i = 0; i < 20000; ++i) body += "<w:p><w:pPr>" + sect + "</w:pPr><w:r><w:t>section " + QByteArray::number(i) + "</w:t></w:r></w:p>";
        QElapsedTimer t;
        t.start();
        auto doc = open(makeDocx(body, kStyles));
        const double open_ = ms(t);
        doc->pager().paginate(5);
        const double first = ms(t);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(10000).position() + 5);
        insertParagraph(*doc, c);
        const double enter = ms(t);
        insertText(*doc, c, QStringLiteral("x"));
        const double type = ms(t);
        qInfo("20,000 sections: open %.0f ms, first pages %.0f ms; Enter %.0f ms; a letter %.0f ms", open_, first, enter, type);
    }
}


// A developer's hook, not a test: JW_SHOTS names a folder to put screenshots of the window in (each ribbon tab, the File screen's pages, the
// text selected, the paragraph marks, Help), of the document JW_SHOTS_DOC names.
void WriteTests::developerScreenshots()
{
    const QString dir = qEnvironmentVariable("JW_SHOTS"), doc = qEnvironmentVariable("JW_SHOTS_DOC");
    if (dir.isEmpty()) QSKIP("JW_SHOTS is not set");
    QDir().mkpath(dir);
    WriteWindow w;
    w.resize(1400, 900);
    w.show();
    QVERIFY(QTest::qWaitForWindowActive(&w));
    if (!doc.isEmpty()) QVERIFY(w.openFile(doc));
    w.view()->paginateAll();
    w.view()->setCaretWithoutFocus(true);
    for (const char *tab : {"Insert", "Layout", "View"}) {
        w.ribbon()->showTab(w.ribbon()->tab(QString::fromLatin1(tab)));
        QTest::qWait(100);
        w.grab().save(QDir(dir).filePath(QStringLiteral("tab-%1.png").arg(QString::fromLatin1(tab).toLower())));
    }
    w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Home")));
    // Text selected across two paragraphs, and the paragraph marks.
    QTextCursor &c = w.view()->cursor();
    c.setPosition(w.document()->story()->findBlockByNumber(3).position() + 10);
    c.setPosition(w.document()->story()->findBlockByNumber(4).position() + 30, QTextCursor::KeepAnchor);
    w.view()->setCursor(c);
    w.act("view.marks")->trigger();
    QTest::qWait(150);
    w.grab().save(QDir(dir).filePath(QStringLiteral("selection-marks.png")));
    w.act("view.marks")->trigger();
    w.showHelp(QStringLiteral("start"));
    QTest::qWait(150);
    w.grab().save(QDir(dir).filePath(QStringLiteral("help.png")));
    for (const char *page : {"new", "open", "print", "export", "about"}) {
        w.showFileScreen(QString::fromLatin1(page));
        QTest::qWait(250);
        w.grab().save(QDir(dir).filePath(QStringLiteral("file-%1.png").arg(QString::fromLatin1(page))));
    }
}


// About lists the third-party software in the app, with each project's license (the list all four apps share),
// and the translation service Translate opens.
void WriteTests::aboutListsTheThirdPartySoftware()
{
    WriteWindow w;
    w.resize(1200, 900);
    w.show();
    w.showFileScreen(QStringLiteral("about"));
    auto *list = w.findChild<QLabel *>(QStringLiteral("thirdParty"));
    QVERIFY(list && list->isVisibleTo(&w));
    QVERIFY2(list->text().contains(QLatin1String("Qt 6")), "Qt 6");
    QVERIFY2(list->text().contains(QLatin1String("Hunspell")), "Hunspell");
    QVERIFY2(list->text().contains(QLatin1String("PDFium")), "PDFium");
    QVERIFY2(list->text().contains(QLatin1String("zlib")), "zlib");
    QVERIFY2(list->text().contains(QLatin1String("Lucide icons")), "Lucide icons");
    QVERIFY2(list->text().contains(QLatin1String("Open fonts")), "Open fonts");
    QVERIFY2(list->text().contains(QLatin1String("ICU")), "ICU");
    QVERIFY2(list->text().contains(QLatin1String("LibreTranslate")), "LibreTranslate");
    QVERIFY2(!list->text().contains(QLatin1String("libmspub")), "libmspub");
}


// ---------------------------------------------------------------- editing
void WriteTests::typingIsOneStepThatUndoesAndRedoes()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("Hello"));
    QCOMPARE(plain(*doc), QStringLiteral("Hello"));
    doc->story()->undo(&c);
    QCOMPARE(plain(*doc), QString());
    doc->story()->redo(&c);
    QCOMPARE(plain(*doc), QStringLiteral("Hello"));
    // The commands are what is undone, not the document: a word is one step, and the step after it another.
    typeText(*doc, c, QStringLiteral(" world"));
    insertParagraph(*doc, c);
    typeText(*doc, c, QStringLiteral("Next"));
    QCOMPARE(doc->story()->blockCount(), 2);
    doc->story()->undo(&c);   // "Next"
    QCOMPARE(plain(*doc), QStringLiteral("Hello world\n"));
    doc->story()->undo(&c);   // the Enter
    QCOMPARE(doc->story()->blockCount(), 1);
    doc->story()->undo(&c);   // " world", which joined the typing before it
    QCOMPARE(plain(*doc), QString());
    QVERIFY(!doc->story()->isUndoAvailable());
}


void WriteTests::anEditCostsTheCommandNotTheDocument()
{
    // 20,000 paragraphs: an undo that kept the whole document would cost in proportion to it (the design's reason for commands). The same edits in a tenth of the
    // paragraphs change as many characters, not a tenth.
    auto charactersChanged = [](int paragraphCount) {
        auto doc = open(makeDocx(paragraphs(paragraphCount, 12), kStyles));
        QTextCursor c(doc->story());
        c.setPosition(5000);
        qint64 touched = 0;
        QObject::connect(doc->story(), &QTextDocument::contentsChange, doc->story(), [&](int, int removed, int added) { touched += removed + added; });
        for (int i = 0; i < 50; ++i) insertText(*doc, c, QStringLiteral("x"));
        insertParagraph(*doc, c);
        for (int i = 0; i < 50; ++i) doc->story()->undo(&c);
        return doc->story()->blockCount() == paragraphCount ? touched : qint64(-1);
    };
    const qint64 small = charactersChanged(2000), large = charactersChanged(20000);
    // Counted: the characters the story changed in the process are those of the edits and their undo, the same for ten times the paragraphs (an undo that kept the whole
    // document would change 20,000 paragraphs' worth each time).
    QVERIFY2(small > 0 && large > 0 && large <= 2 * small + 200, qPrintable(QStringLiteral("2,000 paragraphs %1 characters changed, 20,000 paragraphs %2").arg(small).arg(large)));
}


void WriteTests::enterAndBackspaceSplitAndJoinParagraphs()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("abcd"));
    c.setPosition(2);
    insertParagraph(*doc, c);
    QCOMPARE(plain(*doc), QStringLiteral("ab\ncd"));
    QCOMPARE(c.blockNumber(), 1);
    QCOMPARE(c.positionInBlock(), 0);
    deleteBack(*doc, c, false);
    QCOMPARE(plain(*doc), QStringLiteral("abcd"));
    QCOMPARE(c.position(), 2);
    deleteForward(*doc, c, false);
    QCOMPARE(plain(*doc), QStringLiteral("abd"));
    // A selection typed over is replaced; the replacement is one step.
    c.setPosition(0);
    c.setPosition(2, QTextCursor::KeepAnchor);
    insertText(*doc, c, QStringLiteral("Z"));
    QCOMPARE(plain(*doc), QStringLiteral("Zd"));
    doc->story()->undo(&c);
    QCOMPARE(plain(*doc), QStringLiteral("abd"));
}


void WriteTests::aHeadingIsFollowedByBodyText()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("Title words"));
    applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
    QCOMPARE(paragraphStyleAt(*doc, c), QStringLiteral("Heading1"));
    const QTextCharFormat heading = c.block().charFormat();
    QVERIFY(c.block().begin().fragment().charFormat().fontPointSize() > 12);
    insertParagraph(*doc, c);   // at the end: the style that follows
    QCOMPARE(paragraphStyleAt(*doc, c), QStringLiteral("Normal"));
    typeText(*doc, c, QStringLiteral("body"));
    const QTextCharFormat body = c.block().begin().fragment().charFormat();
    QVERIFY(body.fontPointSize() < c.document()->firstBlock().begin().fragment().charFormat().fontPointSize());
    QCOMPARE(body.fontWeight(), int(QFont::Normal));
    Q_UNUSED(heading);
}


void WriteTests::characterFormatsApplyToTheSelectionAndSurviveASaveAndOpen()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("Some words here"));
    c.setPosition(5);
    c.setPosition(10, QTextCursor::KeepAnchor);
    toggleBold(*doc, c);
    toggleItalic(*doc, c);
    toggleUnderline(*doc, c);
    setFontSize(*doc, c, 20);
    setTextColor(*doc, c, QColor(0xC0, 0, 0));
    setHighlight(*doc, c, QStringLiteral("yellow"));
    setFontFamily(*doc, c, QStringLiteral("Cousine"));
    QCOMPARE(c.charFormat().fontWeight(), int(QFont::Bold));
    // Toggling again, with all of it bold, turns it off.
    QVERIFY(allSelected(c, [](const QTextCharFormat &f) { return f.fontWeight() >= QFont::DemiBold; }));
    const QByteArray zip = doc->toDocx();
    QVERIFY(!zip.isEmpty());
    const QByteArray x = documentXml(zip);
    // (The word "words" is the run with the properties; the rest has none of its own, as the style gives it its look.)
    QRegularExpression run(QStringLiteral("<w:r><w:rPr>(.*?)</w:rPr><w:t xml:space=\"preserve\">words</w:t>"));
    const auto m = run.match(QString::fromUtf8(x));
    QVERIFY2(m.hasMatch(), x.constData());
    const QString props = m.captured(1);
    QVERIFY(props.contains(QLatin1String("<w:b/>")));
    QVERIFY(props.contains(QLatin1String("<w:i/>")));
    QVERIFY(props.contains(QLatin1String("<w:u w:val=\"single\"/>")));
    QVERIFY(props.contains(QLatin1String("<w:sz w:val=\"40\"/>")));
    QVERIFY(props.contains(QLatin1String("<w:color w:val=\"C00000\"/>")));
    QVERIFY(props.contains(QLatin1String("<w:highlight w:val=\"yellow\"/>")));
    QVERIFY(props.contains(QLatin1String("Cousine")));
    QVERIFY(x.contains("<w:t xml:space=\"preserve\">Some </w:t>"));
    const int firstRun = x.indexOf("<w:r>");
    QVERIFY(x.mid(firstRun, 40).startsWith("<w:r><w:t"));   // (no direct properties on text that has none of its own)
    // Opening it again gives the same look.
    auto back = open(zip);
    QTextBlock b = back->story()->firstBlock();
    QTextCursor t(back->story());
    t.setPosition(7);
    const QTextCharFormat f = t.charFormat();
    QCOMPARE(f.fontWeight(), int(QFont::Bold));
    QVERIFY(f.fontItalic());
    QCOMPARE(f.fontPointSize(), 20.0);
    QCOMPARE(f.foreground().color(), QColor(0xC0, 0, 0));
    QCOMPARE(f.fontFamilies().toStringList().value(0), QStringLiteral("Cousine"));
    QVERIFY(f.underlineStyle() != QTextCharFormat::NoUnderline);
    t.setPosition(2);
    QCOMPARE(t.charFormat().fontWeight(), int(QFont::Normal));
    Q_UNUSED(b);
}


void WriteTests::paragraphFormatsApplyToTheParagraphsSelected()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("one\ntwo\nthree"));
    c.setPosition(1);
    c.setPosition(doc->story()->findBlockByNumber(1).position() + 1, QTextCursor::KeepAnchor);
    setAlignment(*doc, c, Qt::AlignHCenter);
    setIndents(*doc, c, 36, 18, -18);
    setSpacing(*doc, c, 6, 12);
    setLineSpacing(*doc, c, QTextBlockFormat::ProportionalHeight, 1.5);
    QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
    QCOMPARE(doc->story()->findBlockByNumber(1).blockFormat().leftMargin(), 36.0);
    QCOMPARE(doc->story()->findBlockByNumber(2).blockFormat().leftMargin(), 0.0);   // (the third was not selected)
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:spacing w:before=\"120\" w:after=\"240\" w:line=\"360\" w:lineRule=\"auto\"/>"), x.constData());
    QVERIFY(x.contains("<w:ind w:left=\"720\" w:right=\"360\" w:hanging=\"360\"/>"));
    QVERIFY(x.contains("<w:jc w:val=\"center\"/>"));
    QCOMPARE(int(x.count("<w:jc w:val=\"center\"/>")), 2);
    // Undone as one step each.
    for (int i = 0; i < 4; ++i) doc->story()->undo(&c);
    QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().leftMargin(), 0.0);
    QVERIFY(!documentXml(doc->toDocx()).contains("w:jc"));
}


void WriteTests::listsAreTurnedOnAndOffAndKeptInTheFile()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("apple\nbanana\ncherry"));
    c.setPosition(0);
    c.setPosition(doc->story()->characterCount() - 1, QTextCursor::KeepAnchor);
    toggleList(*doc, c, true);
    QCOMPARE(listKindOf(*doc, doc->story()->firstBlock()), ListKind::Numbers);
    QTextList *list = doc->story()->findBlockByNumber(2).textList();
    QVERIFY(list);
    QCOMPARE(list->itemText(doc->story()->findBlockByNumber(2)), QStringLiteral("3."));
    QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:numPr>")), 3);
    QVERIFY(x.contains("<w:ilvl w:val=\"0\"/>"));
    // The file it makes opens to the same list.
    auto back = open(doc->toDocx());
    QCOMPARE(back->story()->findBlockByNumber(1).textList()->itemText(back->story()->findBlockByNumber(1)), QStringLiteral("2."));
    // Bullets over numbers change the kind; again, they turn off.
    toggleList(*doc, c, false);
    QCOMPARE(listKindOf(*doc, doc->story()->firstBlock()), ListKind::Bullets);
    toggleList(*doc, c, false);
    QCOMPARE(listKindOf(*doc, doc->story()->firstBlock()), ListKind::None);
    x = documentXml(doc->toDocx());
    QVERIFY(!x.contains("<w:numPr>"));
    // Enter on an empty item ends the list; a Tab makes an item a level deeper.
    toggleList(*doc, c, false);
    c.movePosition(QTextCursor::End);
    insertParagraph(*doc, c);
    QCOMPARE(c.block().blockFormat().intProperty(prop::NumId) > 0, true);
    insertParagraph(*doc, c);
    QVERIFY(!c.block().blockFormat().hasProperty(prop::NumId));
    QCOMPARE(doc->story()->blockCount(), 4);
}


// A page break is a character of its paragraph (the page ends after the line that has it); Insert > Page Break puts one in and goes on in a
// paragraph of its own, as the file does it, and the file keeps what it was given: the break in the paragraph's runs, one for one.
void WriteTests::aPageBreakIsACharacterOfItsParagraphAndTheFileKeepsItWhole()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("before after"));
    c.setPosition(7);
    insertPageBreak(*doc, c);
    QCOMPARE(doc->pager().pageCount(), 2);
    QCOMPARE(doc->story()->blockCount(), 2);
    QCOMPARE(doc->story()->findBlockByNumber(0).text(), QStringLiteral("before ") + flow::kPageBreak);
    QCOMPARE(c.block().text(), QStringLiteral("after"));   // (the cursor is at the top of the next page)
    QCOMPARE(c.position(), doc->story()->findBlockByNumber(1).position());
    const QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:p>")) + int(x.count("<w:p ")), 2);
    QCOMPARE(int(x.count("<w:br w:type=\"page\"/>")), 1);
    QVERIFY2(x.indexOf("<w:br w:type=\"page\"/>") < x.indexOf("</w:p>"), "the break ends the first paragraph");
    auto back = open(doc->toDocx());
    QCOMPARE(back->story()->blockCount(), 2);
    QCOMPARE(back->pager().pageCount(), 2);
    QCOMPARE(documentXml(back->toDocx()), x);
    // Backspace at the start of the second takes the paragraph mark out, and again the break.
    QTextCursor d(back->story());
    d.setPosition(back->story()->findBlockByNumber(1).position());
    deleteBack(*back, d, false);
    QCOMPARE(back->story()->blockCount(), 1);
    QCOMPARE(back->pager().pageCount(), 2);
    deleteBack(*back, d, false);
    QCOMPARE(back->pager().pageCount(), 1);
    QVERIFY(!documentXml(back->toDocx()).contains("w:br"));
    // A break in the middle of a paragraph of a file, a run of its own or in the text's, is read as the character, and written as before.
    auto mid = open(makeDocx("<w:p><w:r><w:t>one</w:t><w:br w:type=\"page\"/><w:t>two</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>three</w:t></w:r></w:p>"));
    QCOMPARE(mid->story()->findBlockByNumber(0).text(), QStringLiteral("one") + flow::kPageBreak + QStringLiteral("two") + flow::kPageBreak + QStringLiteral("three"));
    QCOMPARE(mid->story()->blockCount(), 1);
    QCOMPARE(mid->pager().pageCount(), 3);
    QCOMPARE(int(documentXml(mid->toDocx()).count("<w:br w:type=\"page\"/>")), 2);
}


// Page Setup on the last section (or the only one) is a step of the undo, as on the others, and marks the document as changed.
void WriteTests::pageSetupOnTheLastSectionUndoesAndMarksTheDocumentModified()
{
    for (int which = 0; which < 2; ++which) {
        std::unique_ptr<WriteDoc> doc = which == 0 ? WriteDoc::blank() : open(richDocx());
        QVERIFY(doc);
        const int last = doc->sectionCount() - 1;
        QCOMPARE(last, which);
        const QByteArray was = packSection(doc->sectionProps(last));
        const QSizeF size = doc->pager().page(doc->pager().pageCount() - 1).size;
        doc->setModified(false);
        const int steps = doc->story()->availableUndoSteps();
        wml::SectionProps p = doc->sectionProps(last);
        std::swap(p.pageW, p.pageH);
        p.orient = QStringLiteral("landscape");
        doc->setSectionProps(last, p);
        QVERIFY2(doc->isModified(), "the document is marked as changed");
        QCOMPARE(doc->story()->availableUndoSteps(), steps + 1);
        QCOMPARE(doc->pager().page(doc->pager().pageCount() - 1).size, QSizeF(size.height(), size.width()));
        doc->story()->undo();
        QCOMPARE(packSection(doc->sectionProps(last)), was);
        QCOMPARE(doc->pager().page(doc->pager().pageCount() - 1).size, size);
        doc->story()->redo();
        QCOMPARE(doc->pager().page(doc->pager().pageCount() - 1).size, QSizeF(size.height(), size.width()));
        // And the file has it.
        auto back = open(doc->toDocx());
        QCOMPARE(back->sectionProps(back->sectionCount() - 1).orient, QStringLiteral("landscape"));
    }
}


// A style given to the first paragraph, and undone: the file is what it was (the engine can't undo a change to the first paragraph's mark
// by itself; the program keeps it where it can).
void WriteTests::undoingAStyleOnTheFirstParagraphRestoresItsMark()
{
    auto doc = open(makeDocx("<w:p><w:r><w:t>first</w:t></w:r></w:p><w:p><w:r><w:t>second</w:t></w:r></w:p>", kStyles));
    QVERIFY(doc);
    const QByteArray before = documentXml(doc->toDocx());
    for (int block = 0; block < 2; ++block) {
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(block).position());
        applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
        QVERIFY(documentXml(doc->toDocx()) != before);
        QVERIFY(markFormat(doc->story()->findBlockByNumber(block)).fontPointSize() == 17.0);
        doc->story()->undo();
        QCOMPARE(documentXml(doc->toDocx()), before);
        doc->story()->redo();
        QVERIFY(markFormat(doc->story()->findBlockByNumber(block)).fontPointSize() == 17.0);
        doc->story()->undo();
    }
    QCOMPARE(documentXml(doc->toDocx()), before);
}


// Attributes the program has no use for, on elements it does understand, go back as they were read: a theme color of an underline, the paper
// code, the numbering of chapters in the page numbers.
void WriteTests::unknownAttributesOnKnownElementsGoBackAsRead()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\" w:code=\"1\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\" w:odd=\"x\"/>"
                            "<w:pgNumType w:fmt=\"decimal\" w:chapStyle=\"1\" w:chapSep=\"colon\"/><w:cols w:num=\"2\" w:space=\"720\" w:odd=\"y\"/></w:sectPr>";
    auto doc = open(makeDocx("<w:p><w:r><w:rPr><w:u w:val=\"single\" w:color=\"FF0000\" w:themeColor=\"accent2\" w:themeTint=\"99\"/></w:rPr><w:t>underlined</w:t></w:r></w:p>", kStyles, QByteArray(), sect));
    QVERIFY(doc);
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("w:themeColor=\"accent2\"") && x.contains("w:themeTint=\"99\""), x.constData());
    QVERIFY2(x.contains("w:code=\"1\""), x.constData());
    QVERIFY2(x.contains("w:chapStyle=\"1\"") && x.contains("w:chapSep=\"colon\""), x.constData());
    QVERIFY2(x.contains("w:odd=\"x\"") && x.contains("w:odd=\"y\""), x.constData());
    // They survive an edit of the page setup, and turning the underline off takes its theme color with it.
    wml::SectionProps p = doc->sectionProps(0);
    p.cols = 1;
    doc->setSectionProps(0, p);
    QVERIFY(documentXml(doc->toDocx()).contains("w:chapSep=\"colon\""));
    QTextCursor c(doc->story());
    c.select(QTextCursor::Document);
    toggleUnderline(*doc, c);
    QVERIFY(!documentXml(doc->toDocx()).contains("themeColor"));
    QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), documentXml(doc->toDocx()));
}


// A file's headers and footers are drawn on its pages: the first-page, even-page, and ordinary ones by the section's titlePg and the file's
// evenAndOddHeaders, the page's number and the page counts live; and saving keeps their parts as they were.
void WriteTests::aFilesHeadersAndFootersAreDrawnAndKept()
{
        const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:headerReference w:type=\"first\" r:id=\"rIdheader2\"/>"
                                "<w:headerReference w:type=\"even\" r:id=\"rIdheader3\"/><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/>"
                                "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"500\" w:footer=\"600\" w:gutter=\"0\"/>"
                                "<w:pgNumType w:start=\"4\"/><w:titlePg/></w:sectPr>";
        const QByteArray footer = headerPart("ftr", {"<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> PAGE \\* ROMAN </w:instrText></w:r>"
                                                       "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>IV</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r><w:r><w:t xml:space=\"preserve\"> of </w:t></w:r>"
                                                       "<w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>1</w:t></w:r></w:fldSimple><w:r><w:t xml:space=\"preserve\"> or </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r>"
                                                       "<w:r><w:instrText> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>4</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>"});
        const QMap<QString, QByteArray> parts{
            {QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>ORDINARY HEADER</w:t></w:r>"})},
            {QStringLiteral("/word/header2.xml"), headerPart("hdr", {"<w:r><w:t>FIRST HEADER</w:t></w:r>"})},
            {QStringLiteral("/word/header3.xml"), headerPart("hdr", {"<w:r><w:t>EVEN HEADER</w:t></w:r>"})},
            {QStringLiteral("/word/footer1.xml"), footer}};
        const QByteArray zip = makeDocxWithParts(paragraphs(60, 40), sect, parts, "<w:evenAndOddHeaders/>");
        auto doc = open(zip);
        QVERIFY(doc);
        const int pages = doc->pager().pageCount();
        QVERIFY(pages >= 4);
        // (The numbers go 4, 5, 6, 7: the first page is the section's, the sixth is the even one.)
        const QStringList want{QStringLiteral("FIRST HEADER"), QStringLiteral("ORDINARY HEADER"), QStringLiteral("EVEN HEADER"), QStringLiteral("ORDINARY HEADER")};
        for (int k = 0; k < 4; ++k) {
            const auto page = doc->pager().materialize(k);
            QVERIFY(page);
            QCOMPARE(page->number, 4 + k);
            const TextItem *h = boxWithText(*page, want[k]);
            QVERIFY2(h, qPrintable(QStringLiteral("page %1 has no header %2").arg(k).arg(want[k])));
            QVERIFY(std::abs(h->rect.top() - 25) < 0.01);   // the header distance, 500 twentieths of a point
            // (The footer is the odd pages' only: the first page and the even pages have none.)
            const TextItem *f = boxWithText(*page, QStringLiteral("Page "));
            if (k != 1 && k != 3) {
                QVERIFY(!f);
                continue;
            }
            QVERIFY(f);
            QVERIFY(std::abs(f->rect.bottom() - (792 - 30)) < 0.02);   // the footer distance
            // The page's number and the count of the pages are fields the page's document answers.
            const QString shown = page->doc->story(f->storyId)->doc->toPlainText();
            QCOMPARE(int(shown.count(QChar::ObjectReplacementCharacter)), 3);
            FieldContext ctx;
            ctx.doc = page->doc.get();
            QCOMPARE(ctx.resolve(QStringLiteral("page:ROMAN")), QStringLiteral("IV|V|VI|VII").split(QLatin1Char('|'))[k]);
            QCOMPARE(ctx.resolve(QStringLiteral("page")), QString::number(4 + k));
            QCOMPARE(ctx.resolve(QStringLiteral("pages")), QString::number(pages));
        }
        // The parts and the references are kept as they were.
        const QByteArray saved = doc->toDocx();
        office::Package before, after;
        before.read(zip);
        after.read(saved);
        for (auto it = parts.cbegin(); it != parts.cend(); ++it) QCOMPARE(after.part(it.key()), before.part(it.key()));
        const QByteArray x = documentXml(saved);
        QCOMPARE(int(x.count("headerReference")), 3);
        QCOMPARE(int(x.count("footerReference")), 1);
    }


// A section that names no header or footer has the section before's; one that names its own has it.
void WriteTests::sectionsTakeTheHeadersOfTheSectionBefore()
{
        const QByteArray one = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        const QByteArray two = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        const QByteArray three = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader2\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        const QByteArray body = "<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr>" + two + "</w:pPr><w:r><w:t>two</w:t></w:r></w:p><w:p><w:r><w:t>three</w:t></w:r></w:p>";
        const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>FIRST HEADER</w:t></w:r>"})},
                                              {QStringLiteral("/word/header2.xml"), headerPart("hdr", {"<w:r><w:t>THIRD HEADER</w:t></w:r>"})}};
        auto doc = open(makeDocxWithParts(body, three, parts));
        QVERIFY(doc);
        QCOMPARE(doc->pager().pageCount(), 3);
        const QStringList want{QStringLiteral("FIRST HEADER"), QStringLiteral("FIRST HEADER"), QStringLiteral("THIRD HEADER")};
        for (int k = 0; k < 3; ++k) {
            const auto page = doc->pager().materialize(k);
            QVERIFY2(page && boxWithText(*page, want[k]), qPrintable(QString::number(k)));
        }
    }


// ---- editing a header or a footer (the story is edited as the text is; pages show it, and saving writes it)

// Typing in a header changes it on every page that uses it, and only there: the first page has a header of its own, and so has a section that names one.
void WriteTests::typingInAHeaderChangesEveryPageThatUsesItAndOnlyThere()
{
    const QByteArray one = sectionNaming({{"default", "rIdheader1"}, {"first", "rIdheader2"}}, "<w:titlePg/>");
    const QByteArray two = sectionNaming({{"default", "rIdheader3"}});
    const QByteArray body = paragraphs(70, 30) + "<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>end of the first section</w:t></w:r></w:p><w:p><w:r><w:t>second</w:t></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, two,
                                      {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>ORDINARY</w:t></w:r>"})},
                                       {QStringLiteral("/word/header2.xml"), headerPart("hdr", {"<w:r><w:t>FIRST</w:t></w:r>"})},
                                       {QStringLiteral("/word/header3.xml"), headerPart("hdr", {"<w:r><w:t>SECOND</w:t></w:r>"})}}));
    QVERIFY(doc);
    const int pages = doc->pager().pageCount();
    QVERIFY(pages >= 4);
    QCOMPARE(doc->sectionCount(), 2);
    QTextDocument *ordinary = doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot);
    QVERIFY(ordinary);
    QCOMPARE(ordinary->toPlainText(), QStringLiteral("ORDINARY"));
    typeAtEnd(*doc, ordinary, QStringLiteral(" typed"));
    QCOMPARE(ordinary->toPlainText(), QStringLiteral("ORDINARY typed"));
    // (Every page of the first section but its first, and none else.)
    int ordinaryPages = 0;
    for (int k = 0; k < pages; ++k) {
        const QString shown = headerOfPage(*doc, k);
        if (k == 0) {
            QString items;
            const auto page = doc->pager().materialize(0);
            for (const auto &item : page->doc->pages[0]->items) items += item->id + QLatin1Char(' ');
            QVERIFY2(shown == QStringLiteral("FIRST"), qPrintable(QStringLiteral("page 0 shows '%1': %2 first-page %3 story %4 sections %5").arg(shown, items).arg(doc->differentFirstPage(0)).arg(doc->headerFooterStory(0, false, 1) != nullptr).arg(doc->sectionCount())));
        }
        else if (shown.startsWith(QLatin1String("ORDINARY"))) {
            QCOMPARE(shown, QStringLiteral("ORDINARY typed"));
            ++ordinaryPages;
        } else {
            QCOMPARE(shown, QStringLiteral("SECOND"));
        }
    }
    QVERIFY(ordinaryPages >= 2);
    QVERIFY(doc->isModified());
    // And the other sections' parts are not touched by the save.
    const QByteArray saved = doc->toDocx();
    QCOMPARE(partOf(saved, QStringLiteral("/word/header2.xml")), xml(headerPart("hdr", {"<w:r><w:t>FIRST</w:t></w:r>"})));
    QCOMPARE(partOf(saved, QStringLiteral("/word/header3.xml")), xml(headerPart("hdr", {"<w:r><w:t>SECOND</w:t></w:r>"})));
    QVERIFY(partOf(saved, QStringLiteral("/word/header1.xml")).contains("ORDINARY typed"));
}


// A header or footer that is not edited is written back byte for byte, even when it was made ready to edit; one that is edited keeps the parts of it the program does not edit.
void WriteTests::anUneditedHeaderIsWrittenBackByteForByte()
{
    // (Bytes a rewrite would change: a comment, odd spacing, an attribute order, an unknown element, a declared prefix nobody uses.)
    const QByteArray odd = "<w:hdr " + QByteArray(kNs) + " xmlns:x=\"urn:x\"><!-- kept --><w:p  w:rsidR=\"00A1\"   ><w:pPr><x:odd a=\"1\"/></w:pPr><w:r ><w:t>ODD  bytes</w:t></w:r></w:p>\n</w:hdr>";
    const QByteArray other = headerPart("ftr", {"<w:r><w:t>FOOT</w:t></w:r>"});
    const QByteArray sect = sectionNaming({{"default", "rIdheader1"}, {"f-default", "rIdfooter1"}});
    const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), odd}, {QStringLiteral("/word/footer1.xml"), other}};
    auto doc = open(makeDocxWithParts(paragraphs(3, 20), sect, parts));
    QVERIFY(doc);
    // Made ready to edit, not edited.
    QVERIFY(doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot));
    QVERIFY(doc->editableHeaderFooter(0, true, WriteDoc::kDefaultSlot));
    QByteArray saved = doc->toDocx();
    QCOMPARE(partOf(saved, QStringLiteral("/word/header1.xml")), xml(odd));
    QCOMPARE(partOf(saved, QStringLiteral("/word/footer1.xml")), xml(other));
    // The footer is edited; the header still comes out as it was.
    typeAtEnd(*doc, doc->editableHeaderFooter(0, true, WriteDoc::kDefaultSlot), QStringLiteral("!"));
    saved = doc->toDocx();
    QCOMPARE(partOf(saved, QStringLiteral("/word/header1.xml")), xml(odd));
    QVERIFY(partOf(saved, QStringLiteral("/word/footer1.xml")).contains("FOOT!"));
}


// An edited header keeps what the program can't edit (a picture and its relationship, a bookmark, a hyperlink, a content control, a field's code) where it was.
void WriteTests::anEditedHeaderKeepsWhatTheProgramCannotEdit()
{
    QImage image(8, 8, QImage::Format_RGB32);
    image.fill(Qt::red);
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    const QByteArray drawing = "<w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"190500\" cy=\"190500\"/><wp:docPr id=\"7\" name=\"Logo\"/>"
                               "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr><pic:cNvPr id=\"0\" name=\"l.png\"/><pic:cNvPicPr/></pic:nvPicPr>"
                               "<pic:blipFill><a:blip r:embed=\"rId9\"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill><pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"190500\" cy=\"190500\"/></a:xfrm>"
                               "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing>";
    const QByteArray field = "<w:r><w:rPr><w:b/></w:rPr><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:rPr><w:b/></w:rPr><w:instrText xml:space=\"preserve\"> DATE \\@ \"MMMM d, yyyy\" </w:instrText></w:r>"
                             "<w:r><w:rPr><w:b/></w:rPr><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:rPr><w:b/></w:rPr><w:t>June 5, 2024</w:t></w:r><w:r><w:rPr><w:b/></w:rPr><w:fldChar w:fldCharType=\"end\"/></w:r>";
    const QByteArray ns = QByteArray(kNs) + " xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                          "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";
    const QByteArray hyperlink = "<w:hyperlink r:id=\"rId10\" w:history=\"1\"><w:r><w:t>link</w:t></w:r></w:hyperlink>";
    const QByteArray header = "<w:hdr " + ns + "><w:p><w:pPr><w:pStyle w:val=\"Header\"/><w:tabs><w:tab w:val=\"center\" w:pos=\"4680\"/><w:tab w:val=\"right\" w:pos=\"9360\"/></w:tabs></w:pPr>"
                              "<w:bookmarkStart w:id=\"4\" w:name=\"hdr\"/><w:r><w:t>Left</w:t></w:r><w:r><w:tab/></w:r>" + field + "<w:r><w:tab/></w:r><w:r>" + drawing + "</w:r><w:bookmarkEnd w:id=\"4\"/></w:p>"
                              "<w:sdt><w:sdtPr><w:alias w:val=\"Title\"/></w:sdtPr><w:sdtContent><w:p><w:r><w:t>Controlled</w:t></w:r></w:p></w:sdtContent></w:sdt>"
                              "<w:p>" + hyperlink + "</w:p></w:hdr>";
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + paragraphs(2, 10) + sectionNaming({{"default", "rIdheader1"}}) + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), xml(QByteArray("<w:styles ") + kNs + ">" + kStyles + "</w:styles>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
    w.addPart(QStringLiteral("/word/header1.xml"), xml(header), QString::fromLatin1(kHeaderType));
    w.addPart(QStringLiteral("/word/media/logo.png"), png, QStringLiteral("image/png"));
    const QString rel = QString::fromLatin1(kRelNs);
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdheader1"), rel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/header1.xml"), QStringLiteral("rId9"), rel + QStringLiteral("image"), QStringLiteral("/word/media/logo.png"));
    w.addRelationshipWithId(QStringLiteral("/word/header1.xml"), QStringLiteral("rId10"), rel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/"), true);
    auto doc = open(w.finish());
    QVERIFY(doc);
    QTextDocument *story = doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot);
    QVERIFY(story);
    QCOMPARE(story->blockCount(), 3);   // (the content control's paragraph is a paragraph of the story)
    // The picture is a character of the story as it is in the file's header, on the page.
    auto pictures = [&] {
        int n = 0;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) n += !it.fragment().charFormat().stringProperty(tp::InlineObject).isEmpty();
        return n;
    };
    QCOMPARE(pictures(), 1);
    QTextCursor c(story);
    insertText(*doc, c, QStringLiteral(">> "));
    QCOMPARE(pictures(), 1);
    QVERIFY2(QString(story->toPlainText()).remove(QChar::ObjectReplacementCharacter).startsWith(QLatin1String(">> Left")), qPrintable(story->toPlainText()));
    const QByteArray saved = doc->toDocx();
    const QByteArray out = partOf(saved, QStringLiteral("/word/header1.xml"));
    QVERIFY2(out.contains("&gt;&gt; ") && out.indexOf("&gt;&gt; ") < out.indexOf("<w:bookmarkStart"), out.constData());   // (typed before the bookmark's mark, at the start)
    QVERIFY(out.contains(drawing));
    QVERIFY(out.contains("<w:bookmarkStart w:id=\"4\" w:name=\"hdr\"/>"));
    QVERIFY(out.contains("<w:bookmarkEnd w:id=\"4\"/>"));
    QVERIFY(out.contains("<w:sdtPr><w:alias w:val=\"Title\"/></w:sdtPr>"));
    QVERIFY(out.contains("Controlled"));
    QVERIFY(out.contains("<w:instrText xml:space=\"preserve\"> DATE \\@ \"MMMM d, yyyy\" </w:instrText>"));
    QVERIFY(out.contains("June 5, 2024"));
    QVERIFY(out.contains("<w:hyperlink r:id=\"rId10\" w:history=\"1\">"));
    QVERIFY(out.contains("<w:tab w:val=\"center\" w:pos=\"4680\"/>"));
    QVERIFY(out.contains("<w:pStyle w:val=\"Header\"/>"));
    // In the order the file had: the bookmark, the text, the field, the drawing, the bookmark's end.
    QVERIFY(out.indexOf("bookmarkStart") < out.indexOf("Left"));
    QVERIFY(out.indexOf("Left") < out.indexOf("DATE"));
    QVERIFY(out.indexOf("DATE") < out.indexOf("<w:drawing>"));
    QVERIFY(out.indexOf("<w:drawing>") < out.indexOf("bookmarkEnd"));
    // The relationships of the part and what they point at are as they were.
    office::Package before, after;
    before.read(w.finish());
    after.read(saved);
    QCOMPARE(after.relationships(QStringLiteral("/word/header1.xml")).size(), 2);
    QCOMPARE(after.part(QStringLiteral("/word/media/logo.png")), png);
    // And it reads again: the picture is still on the page, and the text is what was typed.
    auto again = open(saved);
    QVERIFY(again);
    QVERIFY(headerOfPage(*again, 0).remove(QChar::ObjectReplacementCharacter).startsWith(QLatin1String(">> Left")));
    QVERIFY(!again->flow().objects.isEmpty() || !again->flow().images.isEmpty());
}


// A shape set at a place in a header stays where it is, and the text below it still goes round it, when the header's text is edited; and it is written back as it was.
void WriteTests::aFloatingShapeInAHeaderStaysWhereItIsWhenTheHeaderIsEdited()
{
    const QByteArray shape = "<w:r><w:pict><v:shapetype id=\"t202\" coordsize=\"21600,21600\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\"/>"
                             "<v:shape id=\"s1\" type=\"#t202\" style=\"position:absolute;margin-left:-20pt;margin-top:0;width:400pt;height:150pt;mso-position-vertical-relative:page;z-index:1\" filled=\"f\" stroked=\"f\">"
                             "<v:textbox><w:txbxContent><w:p><w:r><w:t>banner</w:t></w:r></w:p></w:txbxContent></v:textbox><w10:wrap type=\"square\"/></v:shape></w:pict></w:r>";
    QByteArray header = headerPart("hdr", {"<w:r><w:t>text</w:t></w:r>" + shape});
    header.replace("<w:hdr ", "<w:hdr xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" ");
    auto doc = open(makeDocxWithParts(paragraphs(10, 5), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    const double top = doc->pager().page(0).content.top();
    QVERIFY2(top > 149.9 && top < 170, qPrintable(QString::number(top)));
    QTextDocument *story = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(story);
    int anchored = 0;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) anchored += it.fragment().charFormat().intProperty(flow::fp::AnchorId) > 0;
    QCOMPARE(anchored, 1);
    typeAtEnd(*doc, story, QStringLiteral(" and more words"));
    QCOMPARE(doc->pager().page(0).content.top(), top);
    const QByteArray saved = doc->toDocx();
    const QByteArray out = partOf(saved, QStringLiteral("/word/header1.xml"));
    QVERIFY2(out.contains("<v:shape id=\"s1\" type=\"#t202\" style=\"position:absolute;margin-left:-20pt;margin-top:0;width:400pt;height:150pt;mso-position-vertical-relative:page;z-index:1\""), out.constData());
    QVERIFY(out.contains("<w10:wrap type=\"square\"/>"));
    QVERIFY(out.contains("banner"));
    QVERIFY(out.indexOf("</w:pict>") > 0 && out.indexOf("</w:pict>") < out.indexOf("and more words"));   // (typed after it, at the end of the paragraph)
    auto again = open(saved);
    QVERIFY(again);
    QCOMPARE(again->pager().page(0).content.top(), top);
}


// The fields of a header are one character each, shown as their results: an edit can't break into one, and deleting takes all of it. What a field is made of is written back.
void WriteTests::theFieldsOfAHeaderAreOneCharacterEach()
{
    auto field = [](const QByteArray &code, const QByteArray &result, const QByteArray &rpr = QByteArray()) {
        const QByteArray p = rpr.isEmpty() ? QByteArray() : "<w:rPr>" + rpr + "</w:rPr>";
        return "<w:r>" + p + "<w:fldChar w:fldCharType=\"begin\"/></w:r><w:r>" + p + "<w:instrText xml:space=\"preserve\"> " + code + " </w:instrText></w:r><w:r>" + p + "<w:fldChar w:fldCharType=\"separate\"/></w:r>"
               "<w:r>" + p + "<w:t>" + result + "</w:t></w:r><w:r>" + p + "<w:fldChar w:fldCharType=\"end\"/></w:r>";
    };
    const QByteArray text = "<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r>" + field("PAGE", "7") + "<w:r><w:t xml:space=\"preserve\"> of </w:t></w:r>" + field("NUMPAGES", "9") +
                            "<w:r><w:t xml:space=\"preserve\"> - </w:t></w:r>" + field("STYLEREF \"Heading 1\"", "Old title") + "<w:r><w:t xml:space=\"preserve\"> - </w:t></w:r>" + field("DATE \\@ \"MMMM d, yyyy\"", "June 5, 2024");
    auto doc = open(makeDocxWithParts(paragraphs(3, 20), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {text})}}));
    QVERIFY(doc);
    QTextDocument *story = doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot);
    QVERIFY(story);
    // Four characters for the four fields, with what each shows.
    const QString plain = story->toPlainText();
    QCOMPARE(int(plain.count(QChar::ObjectReplacementCharacter)), 4);
    QStringList codes;
    QVector<int> places;
    for (int i = 0; i < plain.size(); ++i)
        if (plain[i] == QChar::ObjectReplacementCharacter) {
            QTextCursor t(story);
            t.setPosition(i + 1);
            codes << t.charFormat().stringProperty(tp::Field);
            places << i;
        }
    QCOMPARE(codes.value(0), QStringLiteral("page"));
    QCOMPARE(codes.value(1), QStringLiteral("pages"));
    QVERIFY2(codes.value(2).startsWith(QLatin1String("styleref:")), qPrintable(codes.value(2)));
    QCOMPARE(codes.value(3), QStringLiteral("raw:June 5, 2024"));
    // The page shows the number it is on, and the date as the file has it.
    {
        const auto page = doc->pager().materialize(0);
        QVERIFY(page);
        QVERIFY(boxWithText(*page, QStringLiteral("Page ")));
    }
    // A caret right after a field and Backspace: the whole field goes, and nothing else.
    QTextCursor c(story);
    c.setPosition(places[1] + 1);
    deleteBack(*doc, c, false);
    QCOMPARE(story->toPlainText().count(QChar::ObjectReplacementCharacter), 3);
    QVERIFY(story->toPlainText().startsWith(QLatin1String("Page ")));
    QVERIFY(story->toPlainText().contains(QLatin1String(" of ")));
    // Delete forward from just before the date does the same.
    c.setPosition(story->toPlainText().lastIndexOf(QChar::ObjectReplacementCharacter));
    deleteForward(*doc, c, false);
    QCOMPARE(story->toPlainText().count(QChar::ObjectReplacementCharacter), 2);
    // A selection across a field takes it with the text round it.
    c.setPosition(1);
    c.setPosition(story->toPlainText().indexOf(QChar::ObjectReplacementCharacter) + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, c);
    QCOMPARE(story->toPlainText().count(QChar::ObjectReplacementCharacter), 1);
    QVERIFY(story->toPlainText().startsWith(QLatin1Char('P')));
    // Written back: the fields that stay are whole, the ones taken are gone.
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QVERIFY2(!out.contains(" NUMPAGES "), out.constData());
    QVERIFY(!out.contains("<w:t>7</w:t>"));
    QVERIFY(out.contains("STYLEREF \"Heading 1\""));
    QVERIFY(!out.contains("DATE"));
    QCOMPARE(int(out.count("w:fldCharType=\"begin\"")), int(out.count("w:fldCharType=\"end\"")));
}


// What a person does to a field's look is written to all its runs; text typed beside a field is not in it.
void WriteTests::aFieldTakesTheLookAndTheTextBesideItDoesNotEnterIt()
{
    auto field = [](const QByteArray &code, const QByteArray &result) {
        return "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> " + code + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
               "<w:r><w:t>" + result + "</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    };
    auto doc = open(makeDocxWithParts(paragraphs(2, 20), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>A</w:t></w:r>" + field("FILENAME", "name.docx") + "<w:r><w:t>B</w:t></w:r>"})}}));
    QVERIFY(doc);
    QTextDocument *story = doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot);
    QVERIFY(story);
    QCOMPARE(story->toPlainText().size(), 3);
    // Typed right after the field: a character of its own, after the field.
    QTextCursor c(story);
    c.setPosition(2);
    insertText(*doc, c, QStringLiteral("x"));
    QCOMPARE(story->toPlainText().at(2), QLatin1Char('x'));
    QCOMPARE(story->toPlainText().at(1), QChar(QChar::ObjectReplacementCharacter));
    // Bold on everything: the field's runs are bold in the file.
    c.select(QTextCursor::Document);
    toggleBold(*doc, c);
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QVERIFY2(out.count("<w:rPr><w:b/></w:rPr>") >= 7, out.constData());
    QVERIFY(out.contains("<w:instrText xml:space=\"preserve\"> FILENAME </w:instrText>"));
    // The runs of the field stay in order between the text before it and the text after it.
    QVERIFY(out.indexOf(">A<") < out.indexOf("FILENAME"));
    QVERIFY(out.indexOf("FILENAME") < out.indexOf(">xB<"));
}


// Insert Page Number: a PAGE field where the caret is, live on every page, and written as a field.
void WriteTests::insertPageNumberMakesALivePageField()
{
    auto doc = open(makeDocxWithParts(paragraphs(70, 30), QByteArray(), {}));
    QVERIFY(doc);
    QVERIFY(doc->createHeaderFooter(0, true, WriteDoc::kDefaultSlot));
    QTextDocument *footer = doc->headerFooterStory(0, true, WriteDoc::kDefaultSlot);
    QVERIFY(footer);
    typeAtEnd(*doc, footer, QStringLiteral("Page "));
    QTextCursor c(footer);
    c.movePosition(QTextCursor::End);
    insertPageNumber(*doc, c);
    QCOMPARE(int(footer->toPlainText().count(QChar::ObjectReplacementCharacter)), 1);
    QVERIFY(footer->toPlainText().startsWith(QLatin1String("Page ")));
    const int pages = doc->pager().pageCount();
    QVERIFY(pages >= 3);
    for (int k : {0, 1, 2}) {
        const auto page = doc->pager().materialize(k);
        QVERIFY(page);
        const TextItem *f = boxWithText(*page, QStringLiteral("Page "));
        QVERIFY(f);
        FieldContext ctx;
        ctx.doc = page->doc.get();
        QCOMPARE(ctx.resolve(QStringLiteral("page")), QString::number(k + 1));
        QCOMPARE(page->doc->story(f->storyId)->doc->toPlainText().count(QChar::ObjectReplacementCharacter), 1);
    }
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/footer1.xml"));
    QVERIFY2(out.contains("<w:instrText xml:space=\"preserve\"> PAGE </w:instrText>"), out.constData());
    QVERIFY(out.contains("<w:fldChar w:fldCharType=\"begin\"/>"));
    QVERIFY(out.contains("<w:fldChar w:fldCharType=\"separate\"/>"));
    QVERIFY(out.contains("<w:fldChar w:fldCharType=\"end\"/>"));
    // Opened again, it is the same field.
    auto again = open(doc->toDocx());
    QVERIFY(again);
    QTextDocument *back = again->editableHeaderFooter(0, true, WriteDoc::kDefaultSlot);
    QVERIFY(back);
    QCOMPARE(back->toPlainText().count(QChar::ObjectReplacementCharacter), 1);
    QCOMPARE(back->toPlainText(), footer->toPlainText());
}


// Sections that name no header have the section before's and share its story: an edit in one is in both. Link to Previous off gives a copy of its own to edit; on gives it up.
void WriteTests::linkToPreviousSharesAndUnlinksAHeader()
{
    const QByteArray one = sectionNaming({{"default", "rIdheader1"}});
    const QByteArray two = sectionNaming({});
    const QByteArray three = sectionNaming({{"default", "rIdheader3"}});
    const QByteArray body = "<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr>" + two + "</w:pPr><w:r><w:t>two</w:t></w:r></w:p><w:p><w:r><w:t>three</w:t></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, three, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>SHARED</w:t></w:r>"})},
                                                    {QStringLiteral("/word/header3.xml"), headerPart("hdr", {"<w:r><w:t>THIRD</w:t></w:r>"})}}));
    QVERIFY(doc);
    QCOMPARE(doc->sectionCount(), 3);
    QVERIFY(doc->hasOwnHeaderFooter(0, false, 0));
    QVERIFY(!doc->hasOwnHeaderFooter(1, false, 0));
    QVERIFY(doc->hasOwnHeaderFooter(2, false, 0));
    QVERIFY(!doc->linkedToPrevious(0, false, 0));
    QVERIFY(doc->linkedToPrevious(1, false, 0));
    QVERIFY(!doc->linkedToPrevious(2, false, 0));
    QVERIFY(doc->headerFooterStory(1, false, 0));
    QCOMPARE(doc->headerFooterStory(1, false, 0), doc->headerFooterStory(0, false, 0));
    QVERIFY(doc->headerFooterStory(2, false, 0) != doc->headerFooterStory(0, false, 0));
    // An edit through the second section is in the first as well.
    typeAtEnd(*doc, doc->editableHeaderFooter(1, false, 0), QStringLiteral(" 1"));
    QCOMPARE(headerOfPage(*doc, 0), QStringLiteral("SHARED 1"));
    QCOMPARE(headerOfPage(*doc, 1), QStringLiteral("SHARED 1"));
    QCOMPARE(headerOfPage(*doc, 2), QStringLiteral("THIRD"));
    // Off: the second section has its own, a copy of what it showed, and edits it alone.
    QVERIFY(doc->setLinkedToPrevious(1, false, 0, false));
    QVERIFY(doc->hasOwnHeaderFooter(1, false, 0));
    QVERIFY(!doc->linkedToPrevious(1, false, 0));
    QVERIFY(doc->headerFooterStory(1, false, 0) != doc->headerFooterStory(0, false, 0));
    QCOMPARE(headerOfPage(*doc, 1), QStringLiteral("SHARED 1"));
    typeAtEnd(*doc, doc->editableHeaderFooter(1, false, 0), QStringLiteral(" 2"));
    QCOMPARE(headerOfPage(*doc, 0), QStringLiteral("SHARED 1"));
    QCOMPARE(headerOfPage(*doc, 1), QStringLiteral("SHARED 1 2"));
    QCOMPARE(headerOfPage(*doc, 2), QStringLiteral("THIRD"));
    // The file has a new part for it, and three references.
    QByteArray saved = doc->toDocx();
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    int headerParts = 0;
    for (const QString &n : pkg.partNames()) headerParts += n.startsWith(QLatin1String("/word/header"));
    QCOMPARE(headerParts, 3);
    QCOMPARE(int(documentXml(saved).count("headerReference")), 3);
    QVERIFY(partOf(saved, QStringLiteral("/word/header1.xml")).contains("SHARED 1"));
    QVERIFY(!partOf(saved, QStringLiteral("/word/header1.xml")).contains("SHARED 1 2"));
    // On again: the section has the first's, and what it had is let go (no part for it in the file).
    QVERIFY(doc->setLinkedToPrevious(1, false, 0, true));
    QVERIFY(doc->linkedToPrevious(1, false, 0));
    QCOMPARE(headerOfPage(*doc, 1), QStringLiteral("SHARED 1"));
    saved = doc->toDocx();
    QVERIFY(pkg.read(saved));
    headerParts = 0;
    for (const QString &n : pkg.partNames()) headerParts += n.startsWith(QLatin1String("/word/header"));
    QCOMPARE(headerParts, 2);
    QCOMPARE(int(documentXml(saved).count("headerReference")), 2);
    // The first section has none before it to link to.
    QVERIFY(!doc->setLinkedToPrevious(0, false, 0, true));
    QVERIFY(!doc->linkedToPrevious(0, false, 0));
    // Linking a slot of a section that has nothing before it in that slot is refused (and so is unlinking a slot with nothing to copy).
    QVERIFY(!doc->setLinkedToPrevious(1, false, 1, false));
    // The undo of the steps is the same history as typing: the link goes back off.
    QVERIFY(doc->canUndo());
}


// Different first page is a section's (titlePg), different odd and even pages the document's (evenAndOddHeaders); each slot has a header of its own, made blank.
void WriteTests::differentFirstAndOddEvenPagesHaveASlotEach()
{
    const QByteArray sect = sectionNaming({{"default", "rIdheader1"}});
    auto doc = open(makeDocxWithParts(paragraphs(70, 30), sect, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>D</w:t></w:r>"})}}));
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() >= 4);
    QVERIFY(!doc->differentFirstPage(0));
    QVERIFY(!doc->differentOddEven());
    QCOMPARE(doc->headerSlotOf(0, true, 1), WriteDoc::kDefaultSlot);
    QCOMPARE(doc->headerSlotOf(0, false, 2), WriteDoc::kDefaultSlot);
    // Different first page: the first page uses its own slot, which is empty.
    doc->setDifferentFirstPage(0, true);
    QVERIFY(doc->differentFirstPage(0));
    QCOMPARE(doc->headerSlotOf(0, true, 1), WriteDoc::kFirstSlot);
    QCOMPARE(doc->headerSlotOf(0, false, 2), WriteDoc::kDefaultSlot);
    QCOMPARE(headerOfPage(*doc, 0), QString());
    QCOMPARE(headerOfPage(*doc, 1), QStringLiteral("D"));
    QVERIFY(!doc->headerFooterStory(0, false, WriteDoc::kFirstSlot));
    QVERIFY(!doc->hasOwnHeaderFooter(0, false, WriteDoc::kFirstSlot));
    QVERIFY(!doc->hasOwnHeaderFooter(0, true, WriteDoc::kDefaultSlot));
    QVERIFY(doc->hasOwnHeaderFooter(0, false, WriteDoc::kDefaultSlot));
    QVERIFY(doc->createHeaderFooter(0, false, WriteDoc::kFirstSlot));
    QVERIFY(doc->hasOwnHeaderFooter(0, false, WriteDoc::kFirstSlot));
    typeAtEnd(*doc, doc->headerFooterStory(0, false, WriteDoc::kFirstSlot), QStringLiteral("FIRST"));
    QCOMPARE(headerOfPage(*doc, 0), QStringLiteral("FIRST"));
    QCOMPARE(headerOfPage(*doc, 1), QStringLiteral("D"));
    // Different odd and even pages: the even pages have an empty slot of their own.
    doc->setDifferentOddEven(true);
    QVERIFY(doc->differentOddEven());
    QCOMPARE(doc->headerSlotOf(0, false, 2), WriteDoc::kEvenSlot);
    QCOMPARE(doc->headerSlotOf(0, false, 3), WriteDoc::kDefaultSlot);
    QCOMPARE(doc->headerSlotOf(0, true, 2), WriteDoc::kFirstSlot);
    QCOMPARE(headerOfPage(*doc, 1), QString());
    QCOMPARE(headerOfPage(*doc, 2), QStringLiteral("D"));
    QVERIFY(doc->createHeaderFooter(0, false, WriteDoc::kEvenSlot));
    typeAtEnd(*doc, doc->headerFooterStory(0, false, WriteDoc::kEvenSlot), QStringLiteral("EVEN"));
    QCOMPARE(headerOfPage(*doc, 1), QStringLiteral("EVEN"));
    QCOMPARE(headerOfPage(*doc, 2), QStringLiteral("D"));
    QCOMPARE(headerOfPage(*doc, 3), QStringLiteral("EVEN"));
    // Saved: titlePg, the references by type, the setting (in a settings part the file did not have), and read again.
    QByteArray saved = doc->toDocx();
    const QByteArray x = documentXml(saved);
    QVERIFY(x.contains("<w:titlePg/>"));
    QVERIFY(x.contains("w:type=\"first\""));
    QVERIFY(x.contains("w:type=\"even\""));
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    const QVector<office::Relationship> settings = pkg.relationshipsOfType(QStringLiteral("/word/document.xml"), QStringLiteral("/settings"));
    QCOMPARE(int(settings.size()), 1);
    QVERIFY(pkg.part(settings[0].target).contains("<w:evenAndOddHeaders/>"));
    QVERIFY(pkg.contentType(settings[0].target).endsWith(QLatin1String("settings+xml")));
    auto again = open(saved);
    QVERIFY(again);
    QVERIFY(again->differentFirstPage(0));
    QVERIFY(again->differentOddEven());
    QCOMPARE(headerOfPage(*again, 0), QStringLiteral("FIRST"));
    QCOMPARE(headerOfPage(*again, 1), QStringLiteral("EVEN"));
    QCOMPARE(headerOfPage(*again, 2), QStringLiteral("D"));
    // Off again: the flags are gone from the file.
    doc->setDifferentOddEven(false);
    doc->setDifferentFirstPage(0, false);
    saved = doc->toDocx();
    QVERIFY(!documentXml(saved).contains("titlePg"));
    QVERIFY(pkg.read(saved));
    for (const QString &n : pkg.partNames())
        if (n.endsWith(QLatin1String("settings.xml"))) QVERIFY(!pkg.part(n).contains("evenAndOddHeaders"));
    QCOMPARE(headerOfPage(*doc, 0), QStringLiteral("D"));
    // A file that already has the setting: it is put out when turned off.
    auto with = open(makeDocxWithParts(paragraphs(3, 20), sect, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>D</w:t></w:r>"})}}, "<w:evenAndOddHeaders/><w:defaultTabStop w:val=\"720\"/>"));
    QVERIFY(with);
    QVERIFY(with->differentOddEven());
    with->setDifferentOddEven(false);
    QVERIFY(pkg.read(with->toDocx()));
    QVERIFY(!pkg.part(QStringLiteral("/word/settings.xml")).contains("evenAndOddHeaders"));
    QVERIFY(pkg.part(QStringLiteral("/word/settings.xml")).contains("defaultTabStop"));
}


// A header made where a section has none is a new part with its relationship and the section's reference to it: valid, and read again; a footer too, beside one the file has.
void WriteTests::aNewHeaderIsAValidPartWithItsRelationship()
{
    auto doc = open(makeDocxWithParts(paragraphs(3, 20), QByteArray(), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>OLD</w:t></w:r>"})}}));
    QVERIFY(doc);   // (a part the section does not name: it is kept, and the new one takes another name)
    QVERIFY(!doc->headerFooterStory(0, false, 0));
    QVERIFY(doc->createHeaderFooter(0, false, 0));
    QVERIFY(doc->createHeaderFooter(0, true, 0));
    QVERIFY(doc->isModified());
    typeAtEnd(*doc, doc->headerFooterStory(0, false, 0), QStringLiteral("Top"));
    typeAtEnd(*doc, doc->headerFooterStory(0, true, 0), QStringLiteral("Bottom"));
    const QByteArray saved = doc->toDocx();
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    QVERIFY(pkg.hasPart(QStringLiteral("/word/header1.xml")));
    QVERIFY(pkg.part(QStringLiteral("/word/header1.xml")).contains("OLD"));
    QVERIFY(pkg.hasPart(QStringLiteral("/word/header2.xml")));
    QVERIFY(pkg.hasPart(QStringLiteral("/word/footer1.xml")));
    QCOMPARE(pkg.contentType(QStringLiteral("/word/header2.xml")), QString::fromLatin1(kHeaderType));
    QCOMPARE(pkg.contentType(QStringLiteral("/word/footer1.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"));
    // The section's references name relationships to them.
    const QByteArray x = documentXml(saved);
    QRegularExpressionMatch m = QRegularExpression(QStringLiteral("<w:headerReference w:type=\"default\" r:id=\"([^\"]+)\"/>")).match(QString::fromUtf8(x));
    QVERIFY2(m.hasMatch(), x.constData());
    const office::Relationship hr = pkg.relationship(QStringLiteral("/word/document.xml"), m.captured(1));
    QCOMPARE(hr.target, QStringLiteral("/word/header2.xml"));
    QVERIFY(hr.type.endsWith(QLatin1String("/header")));
    m = QRegularExpression(QStringLiteral("<w:footerReference w:type=\"default\" r:id=\"([^\"]+)\"/>")).match(QString::fromUtf8(x));
    QVERIFY(m.hasMatch());
    QVERIFY(pkg.relationship(QStringLiteral("/word/document.xml"), m.captured(1)).type.endsWith(QLatin1String("/footer")));
    // The references come before the page size in the section's properties.
    QVERIFY(x.indexOf("headerReference") < x.indexOf("<w:pgSz") || !x.contains("<w:pgSz"));
    // The part is a header with a paragraph of the Header style, which the file's styles now define.
    const QByteArray header = pkg.part(QStringLiteral("/word/header2.xml"));
    QVERIFY2(header.startsWith("<?xml") && header.contains("<w:hdr ") && header.contains("<w:pStyle w:val=\"Header\"/>") && header.contains(">Top<"), header.constData());
    QVERIFY(pkg.part(QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Header\""));
    QVERIFY(pkg.part(QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Footer\""));
    QVector<wml::Block> blocks;
    QVERIFY(wml::readBlocks(header, &blocks));
    QCOMPARE(int(blocks.size()), 1);
    // Read again.
    auto again = open(saved);
    QVERIFY(again);
    QCOMPARE(headerOfPage(*again, 0), QStringLiteral("Top"));
    QCOMPARE(headerOfPage(*again, 0, true), QStringLiteral("Bottom"));
    // A header blank and left so is still a valid part.
    auto blank = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}));
    QVERIFY(blank);
    QVERIFY(blank->createHeaderFooter(0, false, 0));
    blocks.clear();
    QVERIFY(pkg.read(blank->toDocx()));
    QVERIFY(pkg.hasPart(QStringLiteral("/word/header1.xml")));
    QVERIFY(wml::readBlocks(pkg.part(QStringLiteral("/word/header1.xml")), &blocks));
    QVERIFY2(blocks.size() == 1, pkg.part(QStringLiteral("/word/header1.xml")).constData());
}


// One history: an undo undoes the last edit wherever it was, the body or any header or footer, in the order they were made; so does a redo. A burst of typing is a step.
void WriteTests::oneHistoryHoldsTheTextAndTheHeaders()
{
    auto doc = open(makeDocxWithParts(paragraphs(2, 10), sectionNaming({{"default", "rIdheader1"}, {"f-default", "rIdfooter1"}}),
                                      {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>H</w:t></w:r>"})}, {QStringLiteral("/word/footer1.xml"), headerPart("ftr", {"<w:r><w:t>F</w:t></w:r>"})}}));
    QVERIFY(doc);
    QVERIFY(!doc->isModified());
    QVERIFY(!doc->canUndo());
    QTextDocument *header = doc->editableHeaderFooter(0, false, 0), *footer = doc->editableHeaderFooter(0, true, 0), *body = doc->story();
    QVERIFY(header && footer);
    const QString bodyBefore = body->toPlainText();
    // The body: a burst of three characters; the header: a burst of two; the footer; the body again; the header again.
    QTextCursor bc(body);
    bc.movePosition(QTextCursor::End);
    for (const char ch : {'a', 'b', 'c'}) insertText(*doc, bc, QString(QLatin1Char(ch)));
    typeAtEnd(*doc, header, QStringLiteral("xy"));
    typeAtEnd(*doc, footer, QStringLiteral("z"));
    for (const char ch : {'d', 'e'}) insertText(*doc, bc, QString(QLatin1Char(ch)));
    typeAtEnd(*doc, header, QStringLiteral("w"));
    QVERIFY(doc->isModified());
    QVERIFY(doc->canUndo());
    QVERIFY(!doc->canRedo());
    auto text = [&](QTextDocument *s) { return s->toPlainText(); };
    QCOMPARE(text(header), QStringLiteral("Hxyw"));
    QCOMPARE(doc->undo(), header);
    QCOMPARE(text(header), QStringLiteral("Hxy"));
    QCOMPARE(doc->undo(), body);
    QCOMPARE(text(body), bodyBefore + QStringLiteral("abc"));
    QCOMPARE(doc->undo(), footer);
    QCOMPARE(text(footer), QStringLiteral("F"));
    QCOMPARE(doc->undo(), header);
    QCOMPARE(text(header), QStringLiteral("H"));
    QCOMPARE(doc->undo(), body);
    QCOMPARE(text(body), bodyBefore);
    QVERIFY(!doc->canUndo());
    QVERIFY(!doc->isModified());   // (back to the state that was read)
    QVERIFY(doc->canRedo());
    QVERIFY(!doc->undo());
    // Redo does them again in the order they were made.
    QCOMPARE(doc->redo(), body);
    QCOMPARE(text(body), bodyBefore + QStringLiteral("abc"));
    QCOMPARE(doc->redo(), header);
    QCOMPARE(text(header), QStringLiteral("Hxy"));
    QVERIFY(doc->isModified());
    // A new edit anywhere clears what could be done again.
    typeAtEnd(*doc, footer, QStringLiteral("!"));
    QVERIFY(!doc->canRedo());
    QCOMPARE(doc->undo(), footer);
    QCOMPARE(text(footer), QStringLiteral("F"));
    QCOMPARE(doc->undo(), header);
    QCOMPARE(doc->undo(), body);
    QVERIFY(!doc->isModified());
    // The saved state is the one an undo comes back to: clean there, modified before and after.
    typeAtEnd(*doc, header, QStringLiteral("1"));
    doc->setModified(false);
    QVERIFY(!doc->isModified());
    typeAtEnd(*doc, header, QStringLiteral("2"));
    QVERIFY(doc->isModified());
    QCOMPARE(doc->undo(), header);
    QVERIFY(!doc->isModified());
    QCOMPARE(doc->undo(), header);
    QVERIFY(doc->isModified());
}


// A step in a story is its own after a step in another: a burst typed in the body, then in a header, then in the body again is three steps.
void WriteTests::aSwitchBetweenStoriesEndsABurst()
{
    auto doc = open(makeDocxWithParts(paragraphs(1, 5), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>H</w:t></w:r>"})}}));
    QVERIFY(doc);
    QTextDocument *header = doc->editableHeaderFooter(0, false, 0), *body = doc->story();
    const QString before = body->toPlainText();
    QTextCursor bc(body);
    bc.movePosition(QTextCursor::End);
    insertText(*doc, bc, QStringLiteral("a"));
    insertText(*doc, bc, QStringLiteral("b"));
    typeAtEnd(*doc, header, QStringLiteral("x"));
    insertText(*doc, bc, QStringLiteral("c"));
    insertText(*doc, bc, QStringLiteral("d"));
    QCOMPARE(doc->undo(), body);
    QCOMPARE(body->toPlainText(), before + QStringLiteral("ab"));
    QCOMPARE(doc->undo(), header);
    QCOMPARE(header->toPlainText(), QStringLiteral("H"));
    QCOMPARE(doc->undo(), body);
    QCOMPARE(body->toPlainText(), before);
    // After an undo, the next edit is a step of its own too.
    QCOMPARE(doc->redo(), body);
    typeAtEnd(*doc, header, QStringLiteral("y"));
    QCOMPARE(doc->undo(), header);
    QCOMPARE(doc->undo(), body);
    QCOMPARE(body->toPlainText(), before);
}


// Hostile headers: one part named by many sections is one story and one part; a part named many times costs no more; a reference to nothing, an empty part, a huge one, one that is not
// XML, and more parts than the program reads are all edited or left alone safely, and written within bounds.
void WriteTests::aHeaderPartTheSectionsShareIsOneStoryAndOnePart()
{
    const QByteArray s1 = sectionNaming({{"default", "rIdheader1"}}), s2 = sectionNaming({{"default", "rIdheader1"}});
    const QByteArray body = "<w:p><w:pPr>" + s1 + "</w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:r><w:t>two</w:t></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, s2, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>BOTH</w:t></w:r>"})}}));
    QVERIFY(doc);
    QCOMPARE(doc->headerFooterStory(0, false, 0), doc->headerFooterStory(1, false, 0));
    QVERIFY(doc->hasOwnHeaderFooter(1, false, 0));   // (it names it itself, though it is the same part)
    typeAtEnd(*doc, doc->editableHeaderFooter(1, false, 0), QStringLiteral("!"));
    QCOMPARE(headerOfPage(*doc, 0), QStringLiteral("BOTH!"));
    QCOMPARE(headerOfPage(*doc, 1), QStringLiteral("BOTH!"));
    const QByteArray saved = doc->toDocx();
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    int parts = 0;
    for (const QString &n : pkg.partNames()) parts += n.startsWith(QLatin1String("/word/header"));
    QCOMPARE(parts, 1);
    QCOMPARE(int(documentXml(saved).count("r:id=\"rIdheader1\"")), 2);
    QVERIFY(pkg.part(QStringLiteral("/word/header1.xml")).contains("BOTH!"));
}


void WriteTests::aHeaderPartNamedBySeveralHundredSectionsIsBounded()
{
    constexpr int sections = 300;
    const QByteArray ref = sectionNaming({{"default", "rIdheader1"}, {"f-default", "rIdfooter1"}});
    QByteArray body;
    for (int i = 0; i < sections - 1; ++i) body += "<w:p><w:pPr>" + ref + "</w:pPr><w:r><w:t>s" + QByteArray::number(i) + "</w:t></w:r></w:p>";
    body += "<w:p><w:r><w:t>last</w:t></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, ref, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>ALL</w:t></w:r>"})}, {QStringLiteral("/word/footer1.xml"), headerPart("ftr", {"<w:r><w:t>FOOT</w:t></w:r>"})}}));
    QVERIFY(doc);
    QCOMPARE(doc->sectionCount(), sections);
    for (int s : {0, 150, sections - 1}) QCOMPARE(doc->headerFooterStory(s, false, 0), doc->headerFooterStory(0, false, 0));
    typeAtEnd(*doc, doc->editableHeaderFooter(sections - 1, false, 0), QStringLiteral("+"));
    QCOMPARE(headerOfPage(*doc, 0), QStringLiteral("ALL+"));
    QCOMPARE(headerOfPage(*doc, sections - 1), QStringLiteral("ALL+"));
    const QByteArray saved = doc->toDocx();
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    QVERIFY(pkg.part(QStringLiteral("/word/header1.xml")).contains("ALL+"));
    QCOMPARE(int(documentXml(saved).count("r:id=\"rIdheader1\"")), sections);
    // Counted: the sections the document made are the 300 it names, once each (the header is one story for all of them).
    QVERIFY2(doc->sectionsMadeForTest() <= 2 * sections, qPrintable(QString::number(doc->sectionsMadeForTest())));
}


void WriteTests::aReferenceToAMissingPartIsKeptAndCanBeReplaced()
{
    // A section names a relationship that does not exist; the next one names a relationship whose part is missing.
    const QByteArray one = sectionNaming({{"default", "rId999"}}), two = sectionNaming({{"default", "rIdgone"}});
    const QByteArray body = "<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:r><w:t>two</w:t></w:r></w:p>";
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + two + "</w:body></w:document>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), QString::fromLatin1(kRelNs) + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdgone"), QString::fromLatin1(kRelNs) + QStringLiteral("header"), QStringLiteral("/word/header9.xml"));
    const QByteArray zip = w.finish();
    auto doc = open(zip);
    QVERIFY(doc);
    QVERIFY(!doc->headerFooterStory(0, false, 0));
    QVERIFY(!doc->editableHeaderFooter(0, false, 0));
    QVERIFY(!doc->editableHeaderFooter(1, false, 0));
    QCOMPARE(headerOfPage(*doc, 0), QString());
    // Saved as it is, the references are as they were.
    QCOMPARE(int(documentXml(doc->toDocx()).count("headerReference")), 2);
    // A reference to nothing is no header: a new one replaces it.
    QVERIFY(doc->createHeaderFooter(0, false, 0));
    typeAtEnd(*doc, doc->headerFooterStory(0, false, 0), QStringLiteral("new"));
    QCOMPARE(headerOfPage(*doc, 0), QStringLiteral("new"));
    QVERIFY(doc->createHeaderFooter(1, false, 0));
    const QByteArray saved = doc->toDocx();
    const QByteArray x = documentXml(saved);
    QVERIFY(!x.contains("rId999"));
    QVERIFY(!x.contains("rIdgone"));
    QCOMPARE(int(x.count("headerReference")), 2);
    auto again = open(saved);
    QVERIFY(again);
    QCOMPARE(headerOfPage(*again, 0), QStringLiteral("new"));
}


// A file whose document part declares no relationships namespace gets it declared when a section needs it for the reference to a new header.
void WriteTests::aNewHeaderInAFileWithoutTheRelationshipsNamespaceDeclaresIt()
{
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"),
              xml("<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body><w:p><w:r><w:t>plain</w:t></w:r></w:p></w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), QString::fromLatin1(kRelNs) + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    auto doc = open(w.finish());
    QVERIFY(doc);
    QVERIFY(doc->createHeaderFooter(0, false, 0));
    typeAtEnd(*doc, doc->headerFooterStory(0, false, 0), QStringLiteral("Hello"));
    const QByteArray saved = doc->toDocx();
    const QByteArray x = documentXml(saved);
    QVERIFY2(x.contains("xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""), x.constData());
    QXmlStreamReader reader(x);
    while (!reader.atEnd()) reader.readNext();
    QVERIFY2(!reader.hasError(), qPrintable(reader.errorString()));
    auto again = open(saved);
    QVERIFY(again);
    QCOMPARE(headerOfPage(*again, 0), QStringLiteral("Hello"));
    // And a file that has it declared is not given a second declaration.
    auto with = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}));
    QVERIFY(with);
    QVERIFY(with->createHeaderFooter(0, false, 0));
    QCOMPARE(int(documentXml(with->toDocx()).count("xmlns:r=")), 1);
}


void WriteTests::anEmptyHeaderPartHasAParagraphToEdit()
{
    auto doc = open(makeDocxWithParts(paragraphs(1, 5), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), "<w:hdr " + QByteArray(kNs) + "/>"}}));
    QVERIFY(doc);
    QTextDocument *story = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(story);
    QCOMPARE(story->blockCount(), 1);
    typeAtEnd(*doc, story, QStringLiteral("filled"));
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QVERIFY2(out.contains("<w:p") && out.contains(">filled<"), out.constData());
    QVector<wml::Block> blocks;
    QVERIFY(wml::readBlocks(out, &blocks));
    QCOMPARE(int(blocks.size()), 1);
    // One that is nothing but a comment.
    auto odd = open(makeDocxWithParts(paragraphs(1, 5), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), "<w:hdr " + QByteArray(kNs) + "><!-- nothing --></w:hdr>"}}));
    QVERIFY(odd);
    QVERIFY(odd->editableHeaderFooter(0, false, 0));
    QCOMPARE(odd->editableHeaderFooter(0, false, 0)->blockCount(), 1);
}


void WriteTests::aHugeHeaderKeepsEveryParagraphWhenOneIsEdited()
{
    constexpr int count = 3000;
    QList<QByteArray> paras;
    for (int i = 0; i < count; ++i) paras << "<w:r><w:t>P" + QByteArray::number(i) + "</w:t></w:r>";
    auto doc = open(makeDocxWithParts(paragraphs(2, 20), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", paras)}}));
    QVERIFY(doc);
    QTextDocument *story = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(story);
    QVERIFY(story->blockCount() <= count);
    QTextCursor c(story);
    insertText(*doc, c, QStringLiteral("X"));
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QVERIFY(out.contains(">XP0<"));
    QCOMPARE(int(out.count("<w:p>")) + int(out.count("<w:p ")), count);
    QVERIFY(out.contains(">P2999<"));
    QVERIFY(out.indexOf(">P1000<") < out.indexOf(">P1001<"));
    QVERIFY(out.indexOf(">P399<") < out.indexOf(">P400<"));
}


void WriteTests::anUnreadableHeaderPartIsLeftAsItIsAndNotReplaced()
{
    const QByteArray garbage = "this is not xml <<<";
    auto doc = open(makeDocxWithParts(paragraphs(1, 5), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), garbage}}));
    QVERIFY(doc);
    QVERIFY(!doc->editableHeaderFooter(0, false, 0));
    QVERIFY(!doc->createHeaderFooter(0, false, 0));
    QVERIFY(!doc->setLinkedToPrevious(0, false, 0, false));
    const QByteArray saved = doc->toDocx();
    QVERIFY(partOf(saved, QStringLiteral("/word/header1.xml")).contains("this is not xml"));
    QVERIFY(documentXml(saved).contains("rIdheader1"));
}


void WriteTests::morePartsThanTheProgramReadsAreKeptAsTheyAre()
{
    constexpr int parts = 80;
    QMap<QString, QByteArray> files;
    QByteArray body;
    for (int i = 1; i <= parts; ++i) {
        files.insert(QStringLiteral("/word/header%1.xml").arg(i), headerPart("hdr", {"<w:r><w:t>H" + QByteArray::number(i) + "</w:t></w:r>"}));
        const QByteArray sect = sectionNaming({{"default", QStringLiteral("rIdheader%1").arg(i)}});
        if (i < parts) body += "<w:p><w:pPr>" + sect + "</w:pPr><w:r><w:t>s" + QByteArray::number(i) + "</w:t></w:r></w:p>";
    }
    auto doc = open(makeDocxWithParts(body, sectionNaming({{"default", QStringLiteral("rIdheader%1").arg(parts)}}), files));
    QVERIFY(doc);
    QCOMPARE(doc->sectionCount(), parts);
    QVERIFY(doc->editableHeaderFooter(0, false, 0));
    // A section whose part is past the limit has no story, and no new header takes its place.
    QVERIFY(!doc->headerFooterStory(parts - 1, false, 0));
    QVERIFY(!doc->createHeaderFooter(parts - 1, false, 0));
    typeAtEnd(*doc, doc->editableHeaderFooter(0, false, 0), QStringLiteral("e"));
    const QByteArray saved = doc->toDocx();
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    for (int i = 2; i <= parts; ++i) QCOMPARE(pkg.part(QStringLiteral("/word/header%1.xml").arg(i)), xml(files.value(QStringLiteral("/word/header%1.xml").arg(i))));
    QVERIFY(pkg.part(QStringLiteral("/word/header1.xml")).contains("H1e"));
    QCOMPARE(int(documentXml(saved).count("headerReference")), parts);
}


// ---- editing a header or footer in the window

// Double-clicking in a page's header or footer area edits the header or footer that page uses; Esc, a double click in the text, and Close Header and Footer end it. A click in the
// text does nothing meanwhile; a click in the area of the footer goes to the footer.
void WriteTests::doubleClickingInTheHeaderAreaEntersAndEscapeOrADoubleClickInTheTextLeaves()
{
    const QByteArray sect = sectionNaming({{"default", "rIdheader1"}, {"f-default", "rIdfooter1"}});
    auto doc = open(makeDocxWithParts(paragraphs(40, 50), sect, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>HEAD</w:t></w:r>"})}, {QStringLiteral("/word/footer1.xml"), headerPart("ftr", {"<w:r><w:t>FOOT</w:t></w:r>"})}}));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    QVERIFY(view.pageCount() >= 3);
    QSignalSpy mode(&view, &PageView::headerFooterModeChanged);
    auto at = [&](int page, double x, double y) {
        const QRect r = view.pageRect(page);
        return QPoint(r.left() + int(x * 96 / 72 * view.zoom()), r.top() + int(y * 96 / 72 * view.zoom()));
    };
    view.goToPage(1);
    QVERIFY(!view.inHeaderFooter());
    QCOMPARE(view.areaAt(at(1, 300, 20)), PageView::Area::Header);
    QCOMPARE(view.areaAt(at(1, 300, 400)), PageView::Area::Body);
    QCOMPARE(view.areaAt(at(1, 300, 780)), PageView::Area::Footer);
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, at(1, 300, 20));
    QVERIFY(view.inHeaderFooter());
    QCOMPARE(mode.count(), 1);
    QCOMPARE(view.headerFooterEdit().section, 0);
    QCOMPARE(view.headerFooterEdit().slot, WriteDoc::kDefaultSlot);
    QCOMPARE(view.headerFooterEdit().footer, false);
    QCOMPARE(view.headerFooterEdit().page, 1);
    QCOMPARE(view.cursor().document(), doc->headerFooterStory(0, false, 0));
    QCOMPARE(view.currentPage(), 1);
    // The text of the page is dim while it is edited (the pages are drawn for it), and the header is not.
    {
        bool dim = false;
        setPaintObserver([&](const flow::MaterializedPage &, const PageDrawing &d) { dim = dim || d.editedStory == doc->headerFooterStory(0, false, 0); });
        view.grab();
        setPaintObserver(nullptr);
        QVERIFY(dim);
    }
    // A click in the text does nothing; one in the footer's area goes to the footer.
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, at(1, 300, 300));
    QVERIFY(view.inHeaderFooter());
    QCOMPARE(view.headerFooterEdit().footer, false);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, at(1, 300, 778));
    QVERIFY(view.inHeaderFooter());
    QCOMPARE(view.headerFooterEdit().footer, true);
    QCOMPARE(view.cursor().document(), doc->headerFooterStory(0, true, 0));
    // Esc leaves, with the text's caret where it was.
    QTest::keyClick(&view, Qt::Key_Escape);
    QVERIFY(!view.inHeaderFooter());
    QCOMPARE(view.cursor().document(), doc->story());
    // A double click in the footer's area; a double click in the text leaves.
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, at(1, 300, 780));
    QVERIFY(view.inHeaderFooter());
    QCOMPARE(view.headerFooterEdit().footer, true);
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, at(1, 300, 400));
    QVERIFY(!view.inHeaderFooter());
    QCOMPARE(view.cursor().document(), doc->story());
    QVERIFY(view.cursor().position() > 0);
    // The same by the commands.
    QVERIFY(view.enterHeaderFooter(0, false));
    QVERIFY(view.inHeaderFooter());
    view.leaveHeaderFooter();
    QVERIFY(!view.inHeaderFooter());
    QVERIFY(mode.count() >= 6);
}


// The slot the page uses decides which header a double click edits: the first page of a section with a different first page, the even pages where the document has them, else the
// ordinary header.
void WriteTests::theSlotOfThePageDecidesWhichHeaderIsEntered()
{
    const QByteArray sect = sectionNaming({{"default", "rIdheader1"}, {"first", "rIdheader2"}, {"even", "rIdheader3"}}, "<w:titlePg/>");
    auto doc = open(makeDocxWithParts(paragraphs(40, 50), sect,
                                      {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>ODD</w:t></w:r>"})},
                                       {QStringLiteral("/word/header2.xml"), headerPart("hdr", {"<w:r><w:t>FIRST</w:t></w:r>"})},
                                       {QStringLiteral("/word/header3.xml"), headerPart("hdr", {"<w:r><w:t>EVEN</w:t></w:r>"})}},
                                      "<w:evenAndOddHeaders/>"));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 900);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    QVERIFY(view.pageCount() >= 3);
    const QVector<int> want{WriteDoc::kFirstSlot, WriteDoc::kEvenSlot, WriteDoc::kDefaultSlot};
    const QStringList text{QStringLiteral("FIRST"), QStringLiteral("EVEN"), QStringLiteral("ODD")};
    for (int page = 0; page < 3; ++page) {
        QVERIFY(view.enterHeaderFooter(page, false));
        QCOMPARE(view.headerFooterEdit().slot, want[page]);
        QCOMPARE(view.cursor().document()->toPlainText(), text[page]);
        view.leaveHeaderFooter();
    }
}


// Insert > Header and Footer make a blank one where the section has none and edit it; the Header & Footer tab is there while one is edited.
void WriteTests::insertHeaderAndFooterMakeABlankOneAndEnterIt()
{
    WriteWindow w;
    w.resize(1100, 800);
    w.show();
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("some text"));
    auto tabShown = [&] {
        for (int i = 0; i < w.ribbon()->tabCount(); ++i)
            if (w.ribbon()->tabName(i) == QLatin1String("Header & Footer")) return w.ribbon()->tabVisible(i);
        return false;
    };
    QVERIFY(w.act("insert.header"));
    QVERIFY(w.act("insert.footer"));
    QVERIFY(!tabShown());
    QVERIFY(!w.act("hf.close")->isEnabled());
    w.document()->setModified(false);
    w.act("insert.header")->trigger();
    QVERIFY(w.view()->inHeaderFooter());
    QVERIFY(!w.view()->headerFooterEdit().footer);
    QVERIFY(w.document()->hasOwnHeaderFooter(0, false, 0));
    QCOMPARE(w.view()->cursor().document(), w.document()->headerFooterStory(0, false, 0));
    QVERIFY(w.isModified());
    QTRY_VERIFY(tabShown());
    QVERIFY(w.act("hf.close")->isEnabled());
    QVERIFY(!w.act("hf.linkPrevious")->isEnabled());   // (the first section has none before it)
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("Title"));
    QCOMPARE(w.document()->headerFooterStory(0, false, 0)->toPlainText(), QStringLiteral("Title"));
    // The footer from the header: the footer is made and entered.
    w.act("insert.footer")->trigger();
    QVERIFY(w.view()->inHeaderFooter());
    QVERIFY(w.view()->headerFooterEdit().footer);
    QVERIFY(w.document()->hasOwnHeaderFooter(0, true, 0));
    // Close.
    w.act("hf.close")->trigger();
    QVERIFY(!w.view()->inHeaderFooter());
    QTRY_VERIFY(!tabShown());
    QCOMPARE(w.view()->cursor().document(), w.document()->story());
    // Undo is one history: the footer's creation, then the header's text, then the header's creation.
    w.act("edit.undo")->trigger();
    QVERIFY(!w.document()->hasOwnHeaderFooter(0, true, 0));
    w.act("edit.undo")->trigger();
    QCOMPARE(w.document()->headerFooterStory(0, false, 0)->toPlainText(), QString());
    QVERIFY(w.view()->inHeaderFooter());   // (the header the step was in is shown)
    w.act("edit.undo")->trigger();
    QVERIFY(!w.document()->hasOwnHeaderFooter(0, false, 0));
    QTRY_VERIFY(!w.view()->inHeaderFooter());
    QVERIFY(!w.document()->isModified());
}


// Previous Section, Next Section, Go to Header, Go to Footer, Date & Time, and Page Number Format are on the tab.
void WriteTests::theHeaderAndFooterTabNavigatesAndInsertsDatesAndSetsNumbering()
{
    const QByteArray one = sectionNaming({{"default", "rIdheader1"}});
    const QByteArray body = paragraphs(30, 50) + "<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>end of one</w:t></w:r></w:p>" + paragraphs(30, 50);
    WriteWindow w;
    w.resize(1100, 800);
    w.show();
    w.setDocument(open(makeDocxWithParts(body, sectionNaming({}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>SHARED</w:t></w:r>"})}})), QString());
    WriteDoc *doc = w.document();
    w.view()->paginateAll();
    QVERIFY(w.view()->enterHeaderFooter(0, false));
    w.view()->setFocus();
    QTRY_VERIFY(w.act("hf.nextSection")->isEnabled());
    QVERIFY(!w.act("hf.previousSection")->isEnabled());
    QVERIFY(!w.act("hf.goToHeader")->isEnabled() && w.act("hf.goToFooter")->isEnabled());
    // Go to Footer: the footer of the same page
    w.act("hf.goToFooter")->trigger();
    QVERIFY(w.view()->headerFooterEdit().footer && w.view()->headerFooterEdit().section == 0);
    QTRY_VERIFY(w.act("hf.goToHeader")->isEnabled() && !w.act("hf.goToFooter")->isEnabled());
    w.act("hf.goToHeader")->trigger();
    QVERIFY(!w.view()->headerFooterEdit().footer);
    // Next Section: the same kind of one in the section after, on its first page
    w.act("hf.nextSection")->trigger();
    QCOMPARE(w.view()->headerFooterEdit().section, 1);
    QVERIFY(!w.view()->headerFooterEdit().footer);
    QCOMPARE(doc->pager().page(w.view()->headerFooterEdit().page).start.section, 1);
    QTRY_VERIFY(!w.act("hf.nextSection")->isEnabled() && w.act("hf.previousSection")->isEnabled());
    w.act("hf.previousSection")->trigger();
    QCOMPARE(w.view()->headerFooterEdit().section, 0);
    // Date & Time puts a field in the header being edited (the dialog stood in for by its first choice)
    QTimer::singleShot(0, [] {
        QWidget *d = QApplication::activeModalWidget();
        if (d) static_cast<QDialog *>(d)->accept();
    });
    QTRY_VERIFY(w.act("hf.dateTime")->isEnabled());
    w.act("hf.dateTime")->trigger();
    QVERIFY2(doc->headerFooterStory(0, false, 0)->toPlainText().size() > 6, qPrintable(doc->headerFooterStory(0, false, 0)->toPlainText()));
    QVERIFY(doc->headerFooterStory(0, false, 0)->toPlainText().startsWith(QLatin1String("SHARED")) || doc->headerFooterStory(0, false, 0)->toPlainText().contains(QLatin1String("SHARED")));
    // Page Number Format is on the tab
    QTRY_VERIFY(w.act("hf.pageNumberFormat")->isEnabled());
    QTimer::singleShot(0, [] {
        QWidget *d = QApplication::activeModalWidget();
        if (!d) return;
        auto *format = d->findChild<QComboBox *>(QStringLiteral("numberFormat"));
        format->setCurrentIndex(format->findData(QStringLiteral("upperRoman")));
        static_cast<QDialog *>(d)->accept();
    });
    w.act("hf.pageNumberFormat")->trigger();
    QCOMPARE(pageNumberingOf(doc->sectionProps(0)).format, QStringLiteral("upperRoman"));
    QCOMPARE(pageNumberingOf(doc->sectionProps(1)).format, QStringLiteral("decimal"));
}


void WriteTests::removeHeaderAndFooterLetGoOfTheSectionsOwnAndBlankWhatItWouldShare()
{
    const QByteArray one = sectionNaming({{"default", "rIdheader1"}});
    const QByteArray body = paragraphs(30, 50) + "<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>end of one</w:t></w:r></w:p>" + paragraphs(30, 50);
    auto make = [&] { return open(makeDocxWithParts(body, sectionNaming({}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>SHARED</w:t></w:r>"})}})); };
    auto doc = make();
    QVERIFY(doc);
    doc->pager().paginate(-1);
    int second = -1;
    for (int k = 0; k < doc->pager().pagesKnown() && second < 0; ++k)
        if (doc->pager().page(k).start.section == 1) second = k;
    QVERIFY(second > 0);
    QCOMPARE(headerOfPage(*doc, second), QStringLiteral("SHARED"));
    QVERIFY(!doc->removeHeaderFooter(0, true));   // (no footer to remove)
    // Section 0's own header goes, and the second section, which took it, shows none.
    QVERIFY(doc->removeHeaderFooter(0, false));
    QVERIFY(!doc->hasOwnHeaderFooter(0, false, 0));
    QCOMPARE(headerOfPage(*doc, 0), QString());
    QCOMPARE(headerOfPage(*doc, second), QString());
    QVERIFY(!doc->removeHeaderFooter(0, false));   // (nothing left)
    QVERIFY(!documentXml(doc->toDocx()).contains("headerReference"));
    QVERIFY(doc->undo());
    QVERIFY(!doc->canUndo() || headerOfPage(*doc, 0) == QStringLiteral("SHARED"));
    QCOMPARE(headerOfPage(*doc, 0), QStringLiteral("SHARED"));
    // The second section alone: it gets a blank header of its own and the first keeps its.
    auto again = make();
    again->pager().paginate(-1);
    QVERIFY(again->removeHeaderFooter(1, false));
    QVERIFY(again->hasOwnHeaderFooter(1, false, 0));
    QCOMPARE(headerOfPage(*again, 0), QStringLiteral("SHARED"));
    QCOMPARE(headerOfPage(*again, second), QString());
}


void WriteTests::headerDesignsFillAHeaderWithFieldsAndTabsInOneStep()
{
    const QVector<HeaderDesign> designs = headerFooterDesigns();
    QStringList ids;
    for (const HeaderDesign &d : designs) ids << d.id;
    QVERIFY2(designs.size() >= 6 && ids.contains(QStringLiteral("blank")) && ids.contains(QStringLiteral("threeColumns")) && ids.contains(QStringLiteral("pageRight")), qPrintable(ids.join(QLatin1Char(' '))));
    auto doc = WriteDoc::blank();
    QVERIFY(!applyHeaderFooterDesign(*doc, 0, false, 0, QStringLiteral("nonsense")));
    QVERIFY(applyHeaderFooterDesign(*doc, 0, false, 0, QStringLiteral("pageRight")));
    QTextDocument *header = doc->headerFooterStory(0, false, 0);
    QVERIFY(header);
    QCOMPARE(header->toPlainText().count(QChar::ObjectReplacementCharacter), 1);   // (the page number)
    QCOMPARE(header->firstBlock().blockFormat().alignment() & Qt::AlignRight, Qt::AlignRight);
    QVERIFY(documentXmlOfPart(doc->toDocx(), QStringLiteral("/word/header1.xml")).contains("PAGE"));
    // Another design replaces it, in one step; three columns have two tab stops.
    QVERIFY(applyHeaderFooterDesign(*doc, 0, false, 0, QStringLiteral("threeColumns")));
    header = doc->headerFooterStory(0, false, 0);
    QCOMPARE(header->toPlainText().count(QLatin1Char('\t')), 2);
    QCOMPARE(int(header->firstBlock().blockFormat().tabPositions().size()), 2);
    QCOMPARE(header->blockCount(), 1);
    QVERIFY(doc->undo());
    header = doc->headerFooterStory(0, false, 0);
    QCOMPARE(header->toPlainText().count(QChar::ObjectReplacementCharacter), 1);
    QCOMPARE(header->toPlainText().count(QLatin1Char('\t')), 0);
    // Blank empties it.
    QVERIFY(applyHeaderFooterDesign(*doc, 0, false, 0, QStringLiteral("blank")));
    QCOMPARE(doc->headerFooterStory(0, false, 0)->toPlainText(), QString());
    // A footer too.
    QVERIFY(applyHeaderFooterDesign(*doc, 0, true, 0, QStringLiteral("pageCenter")));
    QVERIFY(doc->headerFooterStory(0, true, 0)->toPlainText().count(QChar::ObjectReplacementCharacter) == 1);
}


void WriteTests::documentInformationIsPutInAsFields()
{
        auto doc = WriteDoc::blank();
        office::PackageProps props = doc->packageProps();
        props.title = QStringLiteral("Annual Report");
        props.creator = QStringLiteral("A. Writer");
        doc->setPackageProps(props);
        QVERIFY(doc->createHeaderFooter(0, false, 0));
        QTextDocument *header = doc->editableHeaderFooter(0, false, 0);
        QTextCursor c(header);
        QVERIFY(!insertDocumentInfo(*doc, c, QStringLiteral("nonsense")));
        for (const char *kind : {"title", "author", "fileName", "filePath"}) QVERIFY(insertDocumentInfo(*doc, c, QString::fromLatin1(kind)));
        const QByteArray xml = documentXmlOfPart(doc->toDocx(), QStringLiteral("/word/header1.xml"));
        for (const char *word : {"TITLE", "AUTHOR", "FILENAME", "\\p", "Annual Report", "A. Writer"}) QVERIFY2(xml.contains(word), word);
    }


// The tab's options: Different First Page (a section's), Different Odd & Even Pages (the document's), Link to Previous (a section's, for what is being edited), and Insert Page Number.
void WriteTests::theHeaderAndFooterTabHasTheOptions()
{
    const QByteArray one = sectionNaming({{"default", "rIdheader1"}});
    const QByteArray body = paragraphs(30, 50) + "<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>end of one</w:t></w:r></w:p>" + paragraphs(30, 50);
    WriteWindow w;
    w.resize(1100, 800);
    w.show();
    w.setDocument(open(makeDocxWithParts(body, sectionNaming({}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>SHARED</w:t></w:r>"})}})), QString());
    WriteDoc *doc = w.document();
    QCOMPARE(doc->sectionCount(), 2);
    w.view()->paginateAll();
    // The second section's first page: its header is the first section's.
    int secondPage = -1;
    for (int k = 0; k < w.view()->pageCount() && secondPage < 0; ++k)
        if (doc->pager().page(k).start.section == 1) secondPage = k;
    QVERIFY(secondPage > 0);
    QVERIFY(w.view()->enterHeaderFooter(secondPage + 1, false));
    QCOMPARE(w.view()->headerFooterEdit().section, 1);
    w.view()->setFocus();
    QTRY_VERIFY(w.act("hf.linkPrevious")->isEnabled());
    QTRY_VERIFY(w.act("hf.linkPrevious")->isChecked());
    QVERIFY(!w.act("hf.differentFirst")->isChecked());
    QVERIFY(!w.act("hf.differentOddEven")->isChecked());
    // Link to Previous off: a copy of its own, edited alone.
    w.act("hf.linkPrevious")->trigger();
    QVERIFY(doc->hasOwnHeaderFooter(1, false, 0));
    QVERIFY(w.view()->inHeaderFooter());
    QCOMPARE(w.view()->cursor().document(), doc->headerFooterStory(1, false, 0));
    QVERIFY(doc->headerFooterStory(1, false, 0) != doc->headerFooterStory(0, false, 0));
    QTRY_VERIFY(!w.act("hf.linkPrevious")->isChecked());
    w.view()->cursor().movePosition(QTextCursor::End);
    typeText(*doc, w.view()->cursor(), QStringLiteral(" two"));
    QCOMPARE(headerOfPage(*doc, 0), QStringLiteral("SHARED"));
    QCOMPARE(headerOfPage(*doc, secondPage + 1), QStringLiteral("SHARED two"));
    // On again.
    w.act("hf.linkPrevious")->trigger();
    QVERIFY(doc->linkedToPrevious(1, false, 0));
    QCOMPARE(headerOfPage(*doc, secondPage + 1), QStringLiteral("SHARED"));
    QVERIFY(w.view()->inHeaderFooter());
    QCOMPARE(w.view()->cursor().document(), doc->headerFooterStory(0, false, 0));
    // Different first page: the second section's first page has its own, blank, header, which is edited.
    QVERIFY(w.view()->enterHeaderFooter(secondPage, false));
    QTRY_VERIFY(w.act("hf.differentFirst")->isEnabled());
    w.act("hf.differentFirst")->trigger();
    QVERIFY(doc->differentFirstPage(1));
    QCOMPARE(w.view()->headerFooterEdit().slot, WriteDoc::kFirstSlot);
    QCOMPARE(w.view()->cursor().document()->toPlainText(), QString());
    QVERIFY(!doc->hasOwnHeaderFooter(1, false, WriteDoc::kFirstSlot));   // (it is made when something is typed in it)
    typeText(*doc, w.view()->cursor(), QStringLiteral("S2 FIRST"));
    QVERIFY(doc->hasOwnHeaderFooter(1, false, WriteDoc::kFirstSlot));
    QCOMPARE(headerOfPage(*doc, secondPage), QStringLiteral("S2 FIRST"));
    // Insert Page Number puts the page's number in the header being edited.
    QTRY_VERIFY(w.act("hf.pageNumber")->isEnabled());
    w.act("hf.pageNumber")->trigger();
    QCOMPARE(w.view()->cursor().document()->toPlainText().count(QChar::ObjectReplacementCharacter), 1);
    // Different odd and even: the even pages have an empty header of their own, edited from the page it is for.
    QTRY_VERIFY(w.act("hf.differentOddEven")->isEnabled());
    w.act("hf.differentOddEven")->trigger();
    QVERIFY(doc->differentOddEven());
    QVERIFY(w.act("hf.differentOddEven")->isChecked());
    w.act("hf.differentOddEven")->trigger();
    QVERIFY(!doc->differentOddEven());
    // And all of it is undone, one step at a time, in order, back to the file as it was.
    int guard = 0;
    while (doc->canUndo() && guard++ < 50) w.view()->undo();
    QVERIFY(!doc->differentFirstPage(1));
    QVERIFY(doc->linkedToPrevious(1, false, 0));
    QVERIFY(!doc->isModified());
}

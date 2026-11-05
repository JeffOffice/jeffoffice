#include "core/templates.h"
#include "app/settings.h"

#include "core/writedoc.h"
#include "office/package.h"

#include <QCoreApplication>
#include <QLocale>

namespace jp::write {

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
const QString kRel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
const QString kWord = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.");

// (the texts are marked where they stand with QT_TRANSLATE_NOOP, and translated here)
QString t(const char *text) { return QCoreApplication::translate("Templates", text); }

QByteArray esc(const QString &s) { return s.toHtmlEscaped().toUtf8(); }

// A paragraph: its style, extra paragraph properties (already in schema order), and its text, which is a run each of the parts split at "\t" (a tab).
QByteArray para(const char *style, const QString &text, const QByteArray &pPr = {}, const QByteArray &rPr = {})
{
    QByteArray x = "<w:p>";
    if (style || !pPr.isEmpty()) x += "<w:pPr>" + (style ? QByteArray("<w:pStyle w:val=\"") + style + "\"/>" : QByteArray()) + pPr + "</w:pPr>";
    const QStringList parts = text.split(QLatin1Char('\t'));
    for (int i = 0; i < parts.size(); ++i) {
        if (i) x += "<w:r><w:tab/></w:r>";
        if (!parts[i].isEmpty()) x += "<w:r>" + (rPr.isEmpty() ? QByteArray() : "<w:rPr>" + rPr + "</w:rPr>") + "<w:t xml:space=\"preserve\">" + esc(parts[i]) + "</w:t></w:r>";
    }
    return x + "</w:p>";
}

QByteArray pageField()
{
    return "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
           "<w:r><w:t>1</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
}

// Styles shared by all: a default look (font, size, spacing) and the named styles the templates use, each a little different per template through `look`.
struct Look {
    QByteArray body = "Carlito";        // the text's font
    QByteArray head = "Carlito";        // the headings' font
    int size = 22;                      // half points
    int after = 160;                    // twips after a paragraph
    int line = 264;                     // 240 is single
    QByteArray accent = "1F5F7A";       // a color: hexadecimal, no #
    int titleSize = 52;
};

QByteArray stylesPart(const Look &k)
{
    auto font = [](const QByteArray &f) { return "<w:rFonts w:ascii=\"" + f + "\" w:hAnsi=\"" + f + "\" w:eastAsia=\"" + f + "\" w:cs=\"" + f + "\"/>"; };
    QByteArray x = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:styles " + QByteArray(kNs) + ">";
    x += "<w:docDefaults><w:rPrDefault><w:rPr>" + font(k.body) + "<w:sz w:val=\"" + QByteArray::number(k.size) + "\"/><w:szCs w:val=\"" + QByteArray::number(k.size) +
         "\"/><w:lang w:val=\"en-US\"/></w:rPr></w:rPrDefault><w:pPrDefault><w:pPr><w:spacing w:after=\"" + QByteArray::number(k.after) + "\" w:line=\"" + QByteArray::number(k.line) +
         "\" w:lineRule=\"auto\"/></w:pPr></w:pPrDefault></w:docDefaults>";
    x += "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>";
    x += "<w:style w:type=\"character\" w:default=\"1\" w:styleId=\"DefaultParagraphFont\"><w:name w:val=\"Default Paragraph Font\"/><w:uiPriority w:val=\"1\"/><w:semiHidden/></w:style>";
    x += "<w:style w:type=\"paragraph\" w:styleId=\"Title\"><w:name w:val=\"Title\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/><w:pPr><w:keepNext/>"
         "<w:spacing w:before=\"0\" w:after=\"120\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr><w:rPr>" + font(k.head) + "<w:b/><w:color w:val=\"" + k.accent + "\"/><w:sz w:val=\"" +
         QByteArray::number(k.titleSize) + "\"/><w:szCs w:val=\"" + QByteArray::number(k.titleSize) + "\"/></w:rPr></w:style>";
    x += "<w:style w:type=\"paragraph\" w:styleId=\"Subtitle\"><w:name w:val=\"Subtitle\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/><w:pPr><w:spacing w:after=\"240\"/></w:pPr>"
         "<w:rPr><w:color w:val=\"595959\"/><w:sz w:val=\"" + QByteArray::number(k.size + 4) + "\"/></w:rPr></w:style>";
    const int sizes[] = {32, 26, 23};
    for (int i = 1; i <= 3; ++i) {
        const QByteArray n = QByteArray::number(i);
        x += "<w:style w:type=\"paragraph\" w:styleId=\"Heading" + n + "\"><w:name w:val=\"heading " + n + "\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/>"
             "<w:pPr><w:keepNext/><w:keepLines/><w:spacing w:before=\"" + QByteArray::number(i == 1 ? 320 : 200) + "\" w:after=\"80\"/><w:outlineLvl w:val=\"" + QByteArray::number(i - 1) +
             "\"/></w:pPr><w:rPr>" + font(k.head) + "<w:b/><w:color w:val=\"" + k.accent + "\"/><w:sz w:val=\"" + QByteArray::number(sizes[i - 1]) + "\"/><w:szCs w:val=\"" +
             QByteArray::number(sizes[i - 1]) + "\"/></w:rPr></w:style>";
    }
    x += "<w:style w:type=\"paragraph\" w:styleId=\"Header\"><w:name w:val=\"header\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:tabs><w:tab w:val=\"center\" w:pos=\"4680\"/><w:tab w:val=\"right\" w:pos=\"9360\"/></w:tabs>"
         "<w:spacing w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr></w:style>";
    x += "<w:style w:type=\"paragraph\" w:styleId=\"Footer\"><w:name w:val=\"footer\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:tabs><w:tab w:val=\"center\" w:pos=\"4680\"/><w:tab w:val=\"right\" w:pos=\"9360\"/></w:tabs>"
         "<w:spacing w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr></w:style>";
    x += "<w:style w:type=\"paragraph\" w:styleId=\"Quote\"><w:name w:val=\"Quote\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/><w:pPr><w:ind w:left=\"567\" w:right=\"567\"/></w:pPr>"
         "<w:rPr><w:i/><w:color w:val=\"404040\"/></w:rPr></w:style>";
    return x + "</w:styles>";
}

struct Page {
    int w, h;
    int top = 1440, right = 1440, bottom = 1440, left = 1440;
};

Page pageSize()
{
    // (a letter page where Letter is used, else A4, as a blank document is)
    if (jp::usesLetterPaper(QLocale())) return {12240, 15840};
    return {11906, 16838};
}

QByteArray sectPr(const Page &p, bool header, bool footer, int columns = 1, bool continuous = false)
{
    QByteArray x = "<w:sectPr>";
    if (header) x += "<w:headerReference w:type=\"default\" r:id=\"rId10\"/>";
    if (footer) x += "<w:footerReference w:type=\"default\" r:id=\"rId11\"/>";
    if (continuous) x += "<w:type w:val=\"continuous\"/>";
    x += "<w:pgSz w:w=\"" + QByteArray::number(p.w) + "\" w:h=\"" + QByteArray::number(p.h) + "\"/><w:pgMar w:top=\"" + QByteArray::number(p.top) + "\" w:right=\"" + QByteArray::number(p.right) +
         "\" w:bottom=\"" + QByteArray::number(p.bottom) + "\" w:left=\"" + QByteArray::number(p.left) + "\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";
    if (columns > 1) x += "<w:cols w:num=\"" + QByteArray::number(columns) + "\" w:space=\"432\"/>";
    return x + "</w:sectPr>";
}

QByteArray part(const char *tag, const QByteArray &content)
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:" + QByteArray(tag) + " " + kNs + ">" + content + "</w:" + tag + ">";
}

QByteArray assemble(const Look &look, const QByteArray &body, const QByteArray &header = {}, const QByteArray &footer = {})
{
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), part("document", "<w:body>" + body + "</w:body>"), kWord + QStringLiteral("document.main+xml"));
    w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), stylesPart(look), kWord + QStringLiteral("styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    if (!header.isEmpty()) {
        w.addPart(QStringLiteral("/word/header1.xml"), part("hdr", header), kWord + QStringLiteral("header+xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId10"), kRel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"), false);
    }
    if (!footer.isEmpty()) {
        w.addPart(QStringLiteral("/word/footer1.xml"), part("ftr", footer), kWord + QStringLiteral("footer+xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId11"), kRel + QStringLiteral("footer"), QStringLiteral("/word/footer1.xml"), false);
    }
    return w.finish();
}

QByteArray rule(const QByteArray &color, int size = 8) { return "<w:pBdr><w:bottom w:val=\"single\" w:sz=\"" + QByteArray::number(size) + "\" w:space=\"4\" w:color=\"" + color + "\"/></w:pBdr>"; }

QByteArray letter()
{
    Look k;
    k.body = k.head = "Caladea";
    k.size = 23;
    k.after = 200;
    k.line = 252;
    k.accent = "2E3B4E";
    Page p = pageSize();
    p.top = 1701;
    QByteArray b;
    const QByteArray right = "<w:jc w:val=\"right\"/>";
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Your Name")), "<w:spacing w:after=\"0\"/>" + right, "<w:b/><w:sz w:val=\"28\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Street Address")), "<w:spacing w:after=\"0\"/>" + right);
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "City, State ZIP")), "<w:spacing w:after=\"0\"/>" + right);
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Email address  |  Phone number")), rule(k.accent) + "<w:spacing w:after=\"480\"/>" + right);
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Month 1, 2027")), "<w:spacing w:after=\"360\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Recipient Name")), "<w:spacing w:after=\"0\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Title or Company")), "<w:spacing w:after=\"0\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Street Address")), "<w:spacing w:after=\"0\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "City, State ZIP")), "<w:spacing w:after=\"360\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Dear Recipient,")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Begin with the reason you are writing. Say it in a sentence or two, so the reader knows at once what the letter is about.")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Use the next paragraph for the details: what happened, what you need, and when you need it. Keep each paragraph to one idea.")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Close by saying what happens next, and thank the reader for the time.")), "<w:spacing w:after=\"360\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Sincerely,")), "<w:keepNext/><w:spacing w:after=\"720\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Your Name")));
    b += sectPr(p, false, false);
    return assemble(k, b);
}

QByteArray report()
{
    Look k;
    k.body = "Carlito";
    k.head = "Lora";
    k.accent = "1F5F7A";
    k.titleSize = 60;
    k.after = 140;
    Page p = pageSize();
    QByteArray b;
    b += para(nullptr, QString(), "<w:spacing w:before=\"1800\" w:after=\"0\"/>");
    b += para("Title", t(QT_TRANSLATE_NOOP("Templates", "Report Title")), rule(k.accent, 12));
    b += para("Subtitle", t(QT_TRANSLATE_NOOP("Templates", "A short line about what the report covers")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Prepared by Your Name")), "<w:spacing w:after=\"0\"/>", "<w:color w:val=\"595959\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Month 1, 2027")), "<w:spacing w:after=\"600\"/>", "<w:color w:val=\"595959\"/>");
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Summary")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Give the main points in a few sentences. A reader who stops here should still know what you found and what you suggest.")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Background")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Say why the report was written and what the reader needs to know first.")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Findings")));
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "First finding")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Describe what you found. Add numbers, a table, or a picture where they help.")));
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "Second finding")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Describe the next point in the same way.")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Recommendations")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "List what should be done, who should do it, and by when.")));
    b += sectPr(p, false, true);
    const QByteArray footer = para("Footer", QString(), "<w:jc w:val=\"center\"/>").replace("</w:p>", pageField() + "</w:p>");
    return assemble(k, b, {}, footer);
}

QByteArray brief()
{
    Look k;
    k.body = "Lato";
    k.head = "Lato";
    k.size = 21;
    k.after = 120;
    k.line = 252;
    k.accent = "7A2E3A";
    k.titleSize = 44;
    Page p = pageSize();
    p.top = 1080;
    QByteArray b;
    b += para("Title", t(QT_TRANSLATE_NOOP("Templates", "Brief")), rule(k.accent, 18));
    const QByteArray tabs = "<w:tabs><w:tab w:val=\"left\" w:pos=\"1100\"/></w:tabs><w:spacing w:after=\"40\"/><w:ind w:left=\"1100\" w:hanging=\"1100\"/>";
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "To:\tName or team")), tabs, "<w:b/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "From:\tYour name")), tabs, "<w:b/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Date:\tMonth 1, 2027")), tabs, "<w:b/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Subject:\tWhat this brief is about")), tabs, "<w:b/>");
    b += para(nullptr, QString(), rule("BFBFBF", 4) + "<w:spacing w:after=\"120\"/>");
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Purpose")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "State in one or two sentences what you want the reader to know or to decide.")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Background")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Give only the facts the reader needs to follow the rest.")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Next steps")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Say who does what, and by when.")));
    b += sectPr(p, false, false);
    return assemble(k, b);
}

QByteArray manuscript()
{
    Look k;
    k.body = k.head = "Courier Prime";
    k.size = 24;
    k.after = 0;
    k.line = 480;
    k.accent = "000000";
    k.titleSize = 24;
    Page p = pageSize();
    QByteArray b;
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Your Name")), "<w:spacing w:line=\"240\" w:lineRule=\"auto\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Street Address")), "<w:spacing w:line=\"240\" w:lineRule=\"auto\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "City, State ZIP")), "<w:spacing w:line=\"240\" w:lineRule=\"auto\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Email address")), "<w:spacing w:line=\"240\" w:lineRule=\"auto\"/>");
    b += para("Title", t(QT_TRANSLATE_NOOP("Templates", "TITLE OF THE WORK")), "<w:spacing w:before=\"2880\" w:after=\"0\"/><w:jc w:val=\"center\"/>", "<w:b w:val=\"0\"/><w:caps/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "by Your Name")), "<w:spacing w:after=\"960\"/><w:jc w:val=\"center\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Begin the story here. Every paragraph starts with an indent, the lines are double spaced, and the pages are numbered in the top corner.")), "<w:ind w:firstLine=\"720\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Press Enter to start the next paragraph. Do not add a blank line between paragraphs.")), "<w:ind w:firstLine=\"720\"/>");
    b += sectPr(p, true, false);
    const QByteArray header = para("Header", t(QT_TRANSLATE_NOOP("Templates", "Surname / TITLE / ")), "<w:jc w:val=\"right\"/>").replace("</w:p>", pageField() + "</w:p>");
    return assemble(k, b, header);
}

QByteArray newsletter()
{
    Look k;
    k.body = "Lora";
    k.head = "Playfair Display";
    k.size = 21;
    k.after = 120;
    k.line = 252;
    k.accent = "2F6B3A";
    k.titleSize = 72;
    Page p = pageSize();
    p.left = p.right = 1080;
    p.top = 1080;
    p.bottom = 1080;
    QByteArray b;
    // The masthead is a section of its own with one column; the stories follow in two.
    b += para("Title", t(QT_TRANSLATE_NOOP("Templates", "The Newsletter Name")), rule(k.accent, 24) + "<w:jc w:val=\"center\"/>");
    b += para("Subtitle", t(QT_TRANSLATE_NOOP("Templates", "Volume 1, Issue 1  |  Month 2027")), "<w:jc w:val=\"center\"/>");
    b += "<w:p><w:pPr>" + sectPr(p, false, false, 1) + "</w:pPr></w:p>";
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Lead story headline")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Open with the most important news. Write short paragraphs, because a reader scans a newsletter before reading it.")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Say who, what, where, and when in the first few lines, then add the details in the lines that follow.")));
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "A smaller story")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Use a heading like this one for each new story, so the columns are easy to scan.")));
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "Dates to remember")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Month 1: what happens, and where.")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Month 2: what happens, and where.")));
    b += para("Quote", t(QT_TRANSLATE_NOOP("Templates", "A short quotation from someone in the story can break up a column.")));
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "From the editor")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "End with a note to the readers and a way to reach you.")));
    b += sectPr(p, false, false, 2, true);
    return assemble(k, b);
}

} // namespace

QVector<BuiltInTemplate> builtInTemplates()
{
    return {
        {QStringLiteral("letter"), t(QT_TRANSLATE_NOOP("Templates", "Letter")), t(QT_TRANSLATE_NOOP("Templates", "Letters"))},
        {QStringLiteral("report"), t(QT_TRANSLATE_NOOP("Templates", "Report")), t(QT_TRANSLATE_NOOP("Templates", "Reports and briefs"))},
        {QStringLiteral("brief"), t(QT_TRANSLATE_NOOP("Templates", "Brief")), t(QT_TRANSLATE_NOOP("Templates", "Reports and briefs"))},
        {QStringLiteral("manuscript"), t(QT_TRANSLATE_NOOP("Templates", "Manuscript")), t(QT_TRANSLATE_NOOP("Templates", "Writing"))},
        {QStringLiteral("newsletter"), t(QT_TRANSLATE_NOOP("Templates", "Newsletter")), t(QT_TRANSLATE_NOOP("Templates", "Publications"))},
    };
}

std::unique_ptr<WriteDoc> builtInTemplateDocument(const QString &id)
{
    QByteArray docx;
    if (id == QLatin1String("letter")) docx = letter();
    else if (id == QLatin1String("report")) docx = report();
    else if (id == QLatin1String("brief")) docx = brief();
    else if (id == QLatin1String("manuscript")) docx = manuscript();
    else if (id == QLatin1String("newsletter")) docx = newsletter();
    else return nullptr;
    QString err;
    return WriteDoc::fromDocx(docx, &err, nullptr);
}

} // namespace jp::write

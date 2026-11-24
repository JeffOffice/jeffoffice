#include "core/templates.h"
#include "app/settings.h"
#include "app/i18n.h"

#include "core/writedoc.h"
#include "core/tocops.h"
#include "office/package.h"

#include <QCoreApplication>
#include <QLocale>
#include <QTextCursor>
#include <QTextDocument>

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
    QByteArray extra;                   // styles of the template's own (already written out)
    QByteArray lang = "en-US";          // the language of the text (see documentLanguage)
};

QByteArray stylesPart(const Look &k)
{
    auto font = [](const QByteArray &f) { return "<w:rFonts w:ascii=\"" + f + "\" w:hAnsi=\"" + f + "\" w:eastAsia=\"" + f + "\" w:cs=\"" + f + "\"/>"; };
    QByteArray x = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:styles " + QByteArray(kNs) + ">";
    x += "<w:docDefaults><w:rPrDefault><w:rPr>" + font(k.body) + "<w:sz w:val=\"" + QByteArray::number(k.size) + "\"/><w:szCs w:val=\"" + QByteArray::number(k.size) +
         "\"/><w:lang w:val=\"" + k.lang + "\"/></w:rPr></w:rPrDefault><w:pPrDefault><w:pPr><w:spacing w:after=\"" + QByteArray::number(k.after) + "\" w:line=\"" + QByteArray::number(k.line) +
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
    return x + k.extra + "</w:styles>";
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

QByteArray assemble(const Look &look, const QByteArray &body, const QByteArray &header = {}, const QByteArray &footer = {}, const QByteArray &numbering = {})
{
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), part("document", "<w:body>" + body + "</w:body>"), kWord + QStringLiteral("document.main+xml"));
    w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), stylesPart(look), kWord + QStringLiteral("styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    if (!numbering.isEmpty()) {
        w.addPart(QStringLiteral("/word/numbering.xml"), numbering, kWord + QStringLiteral("numbering+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
    }
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

// The language the text of a template is in: the language of the program ("de_DE" is German as written in Germany), American English for English.
QByteArray documentLanguage(const QString &ui)
{
    QString tag = ui;
    tag.replace(QLatin1Char('_'), QLatin1Char('-'));
    if (tag.isEmpty() || tag == QLatin1String("en")) tag = QStringLiteral("en-US");
    return tag.toLatin1();
}

QByteArray rule(const QByteArray &color, int size = 8) { return "<w:pBdr><w:bottom w:val=\"single\" w:sz=\"" + QByteArray::number(size) + "\" w:space=\"4\" w:color=\"" + color + "\"/></w:pBdr>"; }

QByteArray letter(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
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

QByteArray report(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
    k.body = "Carlito";
    k.head = "Lora";
    k.accent = "1F5F7A";
    k.titleSize = 60;
    k.after = 140;
    Page p = pageSize();
    QByteArray b;
    b += para("Title", t(QT_TRANSLATE_NOOP("Templates", "Report Title")), rule(k.accent, 12) + "<w:spacing w:before=\"1800\" w:after=\"120\"/>");
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

QByteArray brief(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
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
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Subject:\tWhat this brief is about")), rule("BFBFBF") + QByteArray(tabs).replace("w:after=\"40\"", "w:after=\"200\""), "<w:b/>");
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Purpose")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "State in one or two sentences what you want the reader to know or to decide.")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Background")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Give only the facts the reader needs to follow the rest.")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Next steps")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Say who does what, and by when.")));
    b += sectPr(p, false, false);
    return assemble(k, b);
}

QByteArray manuscript(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
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

QByteArray newsletter(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
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

// ---- the templates added Nov 24: a resume (two), a cover letter, a business letter, an invoice, a meeting agenda and notes, a report with a cover page, and a flyer ----

QByteArray runOf(const QString &text, const QByteArray &rPr = {})
{
    if (text.isEmpty()) return {};
    return "<w:r>" + (rPr.isEmpty() ? QByteArray() : "<w:rPr>" + rPr + "</w:rPr>") + "<w:t xml:space=\"preserve\">" + esc(text) + "</w:t></w:r>";
}

QByteArray paraOf(const char *style, const QByteArray &content, const QByteArray &pPr = {})
{
    QByteArray x = "<w:p>";
    if (style || !pPr.isEmpty()) x += "<w:pPr>" + (style ? QByteArray("<w:pStyle w:val=\"") + style + "\"/>" : QByteArray()) + pPr + "</w:pPr>";
    return x + content + "</w:p>";
}

// A line with its words at the left and a second part (a date, a price) at the right tab stop of its style.
QByteArray twoPart(const char *style, const QString &left, const QString &right, const QByteArray &pPr = {})
{
    return paraOf(style, runOf(left) + "<w:r><w:tab/></w:r>" + runOf(right, "<w:rStyle w:val=\"Dates\"/>"), pPr);
}

QByteArray pstyle(const char *id, const char *name, const QByteArray &pPr, const QByteArray &rPr, const char *based = "Normal", const char *next = nullptr)
{
    QByteArray x = QByteArray("<w:style w:type=\"paragraph\" w:customStyle=\"1\" w:styleId=\"") + id + "\"><w:name w:val=\"" + name + "\"/><w:basedOn w:val=\"" + based + "\"/>";
    if (next) x += QByteArray("<w:next w:val=\"") + next + "\"/>";
    x += "<w:qFormat/>";
    if (!pPr.isEmpty()) x += "<w:pPr>" + pPr + "</w:pPr>";
    if (!rPr.isEmpty()) x += "<w:rPr>" + rPr + "</w:rPr>";
    return x + "</w:style>";
}

QByteArray cstyle(const char *id, const char *name, const QByteArray &rPr)
{
    return QByteArray("<w:style w:type=\"character\" w:customStyle=\"1\" w:styleId=\"") + id + "\"><w:name w:val=\"" + name + "\"/><w:basedOn w:val=\"DefaultParagraphFont\"/><w:qFormat/><w:rPr>" + rPr + "</w:rPr></w:style>";
}

QByteArray fontOf(const QByteArray &f) { return "<w:rFonts w:ascii=\"" + f + "\" w:hAnsi=\"" + f + "\" w:eastAsia=\"" + f + "\" w:cs=\"" + f + "\"/>"; }

// The bullets of the List Bullet style.
QByteArray bulletStyle(int after = 40)
{
    return pstyle("ListBullet", "List Bullet", "<w:numPr><w:numId w:val=\"1\"/></w:numPr><w:spacing w:after=\"" + QByteArray::number(after) + "\"/><w:ind w:left=\"360\" w:hanging=\"260\"/>", {});
}

QByteArray numberingPart(const QByteArray &font, const QByteArray &color)
{
    return part("numbering", "<w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"hybridMultilevel\"/><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/>"
                             "<w:lvlText w:val=\"\xE2\x80\xA2\"/><w:lvlJc w:val=\"left\"/><w:pPr><w:ind w:left=\"360\" w:hanging=\"260\"/></w:pPr><w:rPr>" + fontOf(font) + "<w:color w:val=\"" + color +
                             "\"/></w:rPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>");
}

// A table: column widths (twips, which say how the room is shared), its rows (from row()), and its look. `borders` is the inside and outside rules, `margins` the room inside each cell.
// The widths are written as shares of `room` (the text's width), so the table stays inside the margins on any paper.
QByteArray tableOf(int room, const QVector<int> &cols, const QByteArray &rows, const QByteArray &borders = {}, const QByteArray &margins = "<w:top w:w=\"60\" w:type=\"dxa\"/><w:left w:w=\"100\" w:type=\"dxa\"/><w:bottom w:w=\"60\" w:type=\"dxa\"/><w:right w:w=\"100\" w:type=\"dxa\"/>",
                   const QByteArray &jc = {}, int indent = 0)
{
    int sum = 0;
    for (int c : cols) sum += c;
    auto share = [&](int twips) { return QByteArray::number(qRound(twips * 5000.0 / room)); };
    // (each cell's width is a share too)
    QByteArray cells = rows;
    for (int at = 0; (at = cells.indexOf("<w:tcW w:w=\"", at)) >= 0;) {
        const int from = at + 12, to = cells.indexOf('"', from);
        const int twips = cells.mid(from, to - from).toInt();
        const QByteArray old = "<w:tcW w:w=\"" + cells.mid(from, to - from) + "\" w:type=\"dxa\"/>";
        const QByteArray now = "<w:tcW w:w=\"" + share(twips) + "\" w:type=\"pct\"/>";
        cells.replace(at, old.size(), now);
        at += now.size();
    }
    return "<w:tbl><w:tblPr><w:tblW w:w=\"" + share(sum) + "\" w:type=\"pct\"/>" + jc + (indent ? "<w:tblInd w:w=\"" + QByteArray::number(indent) + "\" w:type=\"dxa\"/>" : QByteArray()) + "<w:tblBorders>" + (borders.isEmpty() ? QByteArray("<w:top w:val=\"nil\"/><w:left w:val=\"nil\"/><w:bottom w:val=\"nil\"/><w:right w:val=\"nil\"/><w:insideH w:val=\"nil\"/><w:insideV w:val=\"nil\"/>") : borders) +
           "</w:tblBorders><w:tblCellMar>" + margins + "</w:tblCellMar><w:tblLook w:val=\"0000\"/></w:tblPr><w:tblGrid/>" + cells + "</w:tbl>";
}

QByteArray row(const QByteArray &cells, int minHeight = 0, bool header = false)
{
    return "<w:tr><w:trPr>" + (minHeight ? "<w:trHeight w:val=\"" + QByteArray::number(minHeight) + "\" w:hRule=\"atLeast\"/>" : QByteArray()) + (header ? QByteArray("<w:tblHeader/>") : QByteArray()) + "</w:trPr>" + cells + "</w:tr>";
}

QByteArray cell(int width, const QByteArray &content, const QByteArray &tcPr = {})
{
    return "<w:tc><w:tcPr><w:tcW w:w=\"" + QByteArray::number(width) + "\" w:type=\"dxa\"/>" + tcPr + "</w:tcPr>" + content + "</w:tc>";
}

QByteArray shade(const QByteArray &fill) { return "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"" + fill + "\"/>"; }

QByteArray cellMargins(int top, int left, int bottom, int right)
{
    return "<w:tcMar><w:top w:w=\"" + QByteArray::number(top) + "\" w:type=\"dxa\"/><w:left w:w=\"" + QByteArray::number(left) + "\" w:type=\"dxa\"/><w:bottom w:w=\"" + QByteArray::number(bottom) +
           "\" w:type=\"dxa\"/><w:right w:w=\"" + QByteArray::number(right) + "\" w:type=\"dxa\"/></w:tcMar>";
}

// (a rule is 1 point or more: a thinner one is drawn faint or uneven at some zooms)
QByteArray line(const char *edge, const QByteArray &color, int size = 8, int space = 0) { return QByteArray("<w:") + edge + " w:val=\"single\" w:sz=\"" + QByteArray::number(size) + "\" w:space=\"" + QByteArray::number(space) + "\" w:color=\"" + color + "\"/>"; }

// Every part and the rules of the page of a template of this group.
struct Made {
    Look look;
    Page page;
    int width = 0;   // the room between the margins
};

Made made(Look k, Page p)
{
    return {k, p, p.w - p.left - p.right};
}

QByteArray assembleMore(Made &m, const QByteArray &body, const QByteArray &header = {}, const QByteArray &footer = {}, bool bullets = false)
{
    return assemble(m.look, body, header, footer, bullets ? numberingPart(m.look.body, m.look.accent) : QByteArray());
}

QByteArray tabRight(int pos) { return "<w:tabs><w:tab w:val=\"right\" w:pos=\"" + QByteArray::number(pos) + "\"/></w:tabs>"; }

QByteArray resume(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
    k.body = "Carlito";
    k.head = "Caladea";
    k.size = 21;
    k.after = 80;
    k.line = 252;
    k.accent = "2E3B4E";
    k.titleSize = 68;
    Page p = pageSize();
    p.top = p.bottom = 900;
    p.left = p.right = 1200;
    Made m = made(k, p);
    const QByteArray accent = k.accent;
    m.look.extra = cstyle("Dates", "Dates", "<w:b w:val=\"0\"/><w:color w:val=\"595959\"/>") +
                   pstyle("ContactLine", "Contact Line", "<w:pBdr>" + line("bottom", "BFBFBF") + "</w:pBdr><w:spacing w:after=\"160\"/>", "<w:color w:val=\"595959\"/>") +
                   pstyle("SectionHeading", "Section Heading", "<w:keepNext/><w:pBdr>" + line("bottom", accent, 12, 2) + "</w:pBdr><w:spacing w:before=\"320\" w:after=\"120\"/><w:outlineLvl w:val=\"0\"/>",
                          fontOf(k.head) + "<w:b/><w:caps/><w:color w:val=\"" + accent + "\"/><w:spacing w:val=\"20\"/><w:sz w:val=\"24\"/>") +
                   pstyle("EntryTitle", "Entry Title", "<w:keepNext/>" + tabRight(m.width) + "<w:spacing w:before=\"100\" w:after=\"0\"/>", "<w:b/>") +
                   pstyle("EntryDetail", "Entry Detail", "<w:keepNext/><w:spacing w:after=\"40\"/>", "<w:i/><w:color w:val=\"595959\"/>") + bulletStyle();
    QByteArray b;
    b += para("Title", t(QT_TRANSLATE_NOOP("Templates", "[Your Name]")));
    b += para("ContactLine", t(QT_TRANSLATE_NOOP("Templates", "[Street Address, City, State ZIP]  |  [Phone number]  |  [Email address]")));
    b += para("SectionHeading", t(QT_TRANSLATE_NOOP("Templates", "Profile")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Two or three sentences: who you are, what you do best, and the kind of work you want next.]")));
    b += para("SectionHeading", t(QT_TRANSLATE_NOOP("Templates", "Experience")));
    for (int i = 0; i < 2; ++i) {
        b += twoPart("EntryTitle", t(QT_TRANSLATE_NOOP("Templates", "[Job Title], [Company]")), t(QT_TRANSLATE_NOOP("Templates", "[Start year] - [End year]")));
        b += para("EntryDetail", t(QT_TRANSLATE_NOOP("Templates", "[City, State]")));
        b += para("ListBullet", t(QT_TRANSLATE_NOOP("Templates", "[Start with a verb and say what you did and what came of it. Use a number where you can.]")));
        b += para("ListBullet", t(QT_TRANSLATE_NOOP("Templates", "[Name a project you led, a problem you solved, or a goal you met.]")));
    }
    b += para("SectionHeading", t(QT_TRANSLATE_NOOP("Templates", "Education")));
    b += twoPart("EntryTitle", t(QT_TRANSLATE_NOOP("Templates", "[Degree], [School]")), t(QT_TRANSLATE_NOOP("Templates", "[Year]")));
    b += para("EntryDetail", t(QT_TRANSLATE_NOOP("Templates", "[City, State]")));
    b += para("SectionHeading", t(QT_TRANSLATE_NOOP("Templates", "Skills")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Skill], [Skill], [Skill], [Skill], [Skill]")));
    b += sectPr(m.page, false, false);
    return assembleMore(m, b, {}, {}, true);
}

QByteArray resumeWithSideColumn(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
    k.body = "Carlito";
    k.head = "Lato";
    k.size = 21;
    k.after = 80;
    k.line = 252;
    k.accent = "1F5F7A";
    k.titleSize = 56;
    Page p = pageSize();
    p.top = p.bottom = 720;
    p.left = p.right = 720;
    Made m = made(k, p);
    const int side = 3000, main = m.width - side;
    const int entryWidth = main - 480 - 320;
    const QByteArray accent = k.accent;
    m.look.extra = cstyle("Dates", "Dates", "<w:b w:val=\"0\"/><w:color w:val=\"595959\"/>") +
                   pstyle("SideHeading", "Side Heading", "<w:keepNext/><w:spacing w:before=\"280\" w:after=\"80\"/>", fontOf(k.head) + "<w:b/><w:caps/><w:color w:val=\"" + accent + "\"/><w:spacing w:val=\"20\"/><w:sz w:val=\"20\"/>") +
                   pstyle("SideText", "Side Text", "<w:spacing w:after=\"60\"/>", "<w:sz w:val=\"20\"/>") +
                   pstyle("SectionHeading", "Section Heading", "<w:keepNext/><w:pBdr>" + line("bottom", accent, 8, 2) + "</w:pBdr><w:spacing w:before=\"280\" w:after=\"100\"/><w:outlineLvl w:val=\"0\"/>",
                          fontOf(k.head) + "<w:b/><w:caps/><w:color w:val=\"" + accent + "\"/><w:spacing w:val=\"20\"/><w:sz w:val=\"22\"/>") +
                   pstyle("EntryTitle", "Entry Title", "<w:keepNext/>" + tabRight(entryWidth) + "<w:spacing w:before=\"100\" w:after=\"0\"/>", "<w:b/>") +
                   pstyle("EntryDetail", "Entry Detail", "<w:keepNext/><w:spacing w:after=\"40\"/>", "<w:i/><w:color w:val=\"595959\"/>") + bulletStyle();
    QByteArray left, right;
    left += para("SideHeading", t(QT_TRANSLATE_NOOP("Templates", "Contact")));
    left += para("SideText", t(QT_TRANSLATE_NOOP("Templates", "[Street Address]")), "<w:spacing w:after=\"0\"/>");
    left += para("SideText", t(QT_TRANSLATE_NOOP("Templates", "[City, State ZIP]")));
    left += para("SideText", t(QT_TRANSLATE_NOOP("Templates", "[Phone number]")));
    left += para("SideText", t(QT_TRANSLATE_NOOP("Templates", "[Email address]")));
    left += para("SideHeading", t(QT_TRANSLATE_NOOP("Templates", "Skills")));
    for (int i = 0; i < 5; ++i) left += para("SideText", t(QT_TRANSLATE_NOOP("Templates", "[Skill]")));
    left += para("SideHeading", t(QT_TRANSLATE_NOOP("Templates", "Languages")));
    left += para("SideText", t(QT_TRANSLATE_NOOP("Templates", "[Language and level]")));
    left += para("SideText", t(QT_TRANSLATE_NOOP("Templates", "[Language and level]")));
    left += para("SideHeading", t(QT_TRANSLATE_NOOP("Templates", "Education")));
    left += para("SideText", t(QT_TRANSLATE_NOOP("Templates", "[Degree]")), "<w:spacing w:after=\"0\"/>", "<w:b/>");
    left += para("SideText", t(QT_TRANSLATE_NOOP("Templates", "[School, year]")));
    right += para("Title", t(QT_TRANSLATE_NOOP("Templates", "[Your Name]")), "<w:spacing w:after=\"0\"/>");
    right += para("Subtitle", t(QT_TRANSLATE_NOOP("Templates", "[The job title you want, or the one you have]")));
    right += para("SectionHeading", t(QT_TRANSLATE_NOOP("Templates", "Profile")));
    right += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Two or three sentences: who you are, what you do best, and the kind of work you want next.]")));
    right += para("SectionHeading", t(QT_TRANSLATE_NOOP("Templates", "Experience")));
    for (int i = 0; i < 3; ++i) {
        right += twoPart("EntryTitle", t(QT_TRANSLATE_NOOP("Templates", "[Job Title], [Company]")), t(QT_TRANSLATE_NOOP("Templates", "[Start year] - [End year]")));
        right += para("EntryDetail", t(QT_TRANSLATE_NOOP("Templates", "[City, State]")));
        right += para("ListBullet", t(QT_TRANSLATE_NOOP("Templates", "[Start with a verb and say what you did and what came of it. Use a number where you can.]")));
        right += para("ListBullet", t(QT_TRANSLATE_NOOP("Templates", "[Name a project you led, a problem you solved, or a goal you met.]")));
    }
    const int height = p.h - p.top - p.bottom - 1700;   // (the room inside the cells is added to a row's height)
    const QByteArray rows = row(cell(side, left, shade("EAF0F4") + cellMargins(480, 320, 360, 320)) + cell(main, right, cellMargins(480, 480, 360, 320)), height);
    QByteArray b = tableOf(m.width, {side, main}, rows, {}, "<w:top w:w=\"0\" w:type=\"dxa\"/><w:left w:w=\"0\" w:type=\"dxa\"/><w:bottom w:w=\"0\" w:type=\"dxa\"/><w:right w:w=\"0\" w:type=\"dxa\"/>", {}, 320);
    b += para(nullptr, QString(), "<w:spacing w:after=\"0\" w:line=\"20\" w:lineRule=\"exact\"/>");
    b += sectPr(m.page, false, false);
    return assembleMore(m, b, {}, {}, true);
}

QByteArray coverLetter(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
    k.body = "Lato";
    k.head = "Lato";
    k.size = 21;
    k.after = 180;
    k.line = 264;
    k.accent = "1F5F7A";
    k.titleSize = 48;
    Page p = pageSize();
    p.top = 1200;
    p.left = p.right = 1500;
    Made m = made(k, p);
    m.look.extra = pstyle("ContactLine", "Contact Line", "<w:pBdr>" + line("bottom", k.accent, 12, 8) + "</w:pBdr><w:spacing w:after=\"600\"/>", "<w:color w:val=\"595959\"/>") +
                   pstyle("Recipient", "Recipient", "<w:spacing w:after=\"0\"/>", {}) + pstyle("Subject", "Subject Line", "<w:spacing w:before=\"240\" w:after=\"240\"/>", "<w:b/><w:color w:val=\"" + k.accent + "\"/>");
    QByteArray b;
    b += para("Title", t(QT_TRANSLATE_NOOP("Templates", "[Your Name]")), "<w:spacing w:after=\"40\"/>");
    b += para("ContactLine", t(QT_TRANSLATE_NOOP("Templates", "[Phone number]  |  [Email address]  |  [City, State]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Date]")), "<w:spacing w:after=\"360\"/>");
    b += para("Recipient", t(QT_TRANSLATE_NOOP("Templates", "[Hiring Manager's Name]")));
    b += para("Recipient", t(QT_TRANSLATE_NOOP("Templates", "[Title]")));
    b += para("Recipient", t(QT_TRANSLATE_NOOP("Templates", "[Company or Organization]")));
    b += para("Recipient", t(QT_TRANSLATE_NOOP("Templates", "[Street Address, City, State ZIP]")), "<w:spacing w:after=\"240\"/>");
    b += para("Subject", t(QT_TRANSLATE_NOOP("Templates", "Re: [The job you are applying for]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Dear [Hiring Manager's Name],")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Open with the job you want and one reason you are a strong fit. Say how you heard about it.]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Say what you bring: two or three things you have done that match what the job asks for. Give a number or a result for each.]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Say why this company. Show that you know what it does and why the work matters to you.]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Close by asking for a conversation, and thank the reader for the time.]")), "<w:spacing w:after=\"360\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Sincerely,")), "<w:keepNext/><w:spacing w:after=\"600\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Your Name]")), {}, "<w:b/>");
    b += sectPr(m.page, false, false);
    return assembleMore(m, b);
}

QByteArray businessLetter(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
    k.body = "Carlito";
    k.head = "Montserrat";
    k.size = 22;
    k.after = 200;
    k.line = 264;
    k.accent = "2E3B4E";
    Page p = pageSize();
    p.top = 1800;
    p.bottom = 1500;
    p.left = p.right = 1500;
    Made m = made(k, p);
    m.look.extra = pstyle("Letterhead", "Letterhead", "<w:pBdr>" + line("bottom", k.accent, 8) + "</w:pBdr><w:spacing w:after=\"0\"/>", fontOf(k.head) + "<w:b/><w:caps/><w:color w:val=\"" + k.accent + "\"/><w:spacing w:val=\"40\"/><w:sz w:val=\"26\"/>") +
                   pstyle("Recipient", "Recipient", "<w:spacing w:after=\"0\"/>", {}) + pstyle("Subject", "Subject Line", "<w:spacing w:before=\"120\" w:after=\"240\"/>", "<w:b/>") +
                   pstyle("Address", "Address Line", "<w:jc w:val=\"center\"/><w:spacing w:after=\"0\"/>", "<w:color w:val=\"595959\"/><w:sz w:val=\"18\"/>");
    QByteArray b;
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Date]")), "<w:spacing w:after=\"360\"/>");
    b += para("Recipient", t(QT_TRANSLATE_NOOP("Templates", "[Recipient Name]")));
    b += para("Recipient", t(QT_TRANSLATE_NOOP("Templates", "[Title]")));
    b += para("Recipient", t(QT_TRANSLATE_NOOP("Templates", "[Company or Organization]")));
    b += para("Recipient", t(QT_TRANSLATE_NOOP("Templates", "[Street Address]")));
    b += para("Recipient", t(QT_TRANSLATE_NOOP("Templates", "[City, State ZIP]")), "<w:spacing w:after=\"360\"/>");
    b += para("Subject", t(QT_TRANSLATE_NOOP("Templates", "Re: [What the letter is about]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Dear [Recipient Name]:")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[State your purpose in the first sentence. A business reader wants to know at once why you are writing.]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Give the details: what you are offering or asking, the dates and amounts that matter, and anything the reader must do.]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Close with the next step, who will take it, and how to reach you.]")), "<w:spacing w:after=\"360\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Sincerely,")), "<w:keepNext/><w:spacing w:after=\"720\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Your Name]")), "<w:keepNext/><w:spacing w:after=\"0\"/>", "<w:b/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Your Title]")), "<w:spacing w:after=\"240\"/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Enclosure: [what is enclosed]")), "<w:spacing w:after=\"0\"/>", "<w:sz w:val=\"20\"/>");
    b += sectPr(m.page, true, true);
    const QByteArray header = para("Letterhead", t(QT_TRANSLATE_NOOP("Templates", "[Company Name]")));
    const QByteArray footer = para("Address", t(QT_TRANSLATE_NOOP("Templates", "[Street Address]  |  [City, State ZIP]  |  [Phone number]  |  [Website]")));
    return assembleMore(m, b, header, footer);
}

QByteArray invoice(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
    k.body = "Carlito";
    k.head = "Lato";
    k.size = 21;
    k.after = 60;
    k.line = 252;
    k.accent = "1F4E79";
    Page p = pageSize();
    p.top = p.bottom = 1080;
    p.left = p.right = 1080;
    Made m = made(k, p);
    const QByteArray gray = "<w:color w:val=\"595959\"/>";
    m.look.extra = pstyle("CompanyName", "Company Name", "<w:spacing w:after=\"40\"/>", fontOf(k.head) + "<w:b/><w:color w:val=\"" + k.accent + "\"/><w:sz w:val=\"30\"/>") +
                   pstyle("InvoiceTitle", "Invoice Title", "<w:spacing w:after=\"0\"/><w:jc w:val=\"right\"/>", fontOf(k.head) + "<w:b/><w:caps/><w:color w:val=\"" + k.accent + "\"/><w:spacing w:val=\"60\"/><w:sz w:val=\"48\"/>") +
                   pstyle("SmallLabel", "Small Label", "<w:keepNext/><w:spacing w:before=\"360\" w:after=\"40\"/>", fontOf(k.head) + "<w:b/><w:caps/>" + gray + "<w:spacing w:val=\"20\"/><w:sz w:val=\"17\"/>") +
                   pstyle("TableHead", "Table Heading", "<w:spacing w:after=\"0\"/>", fontOf(k.head) + "<w:b/><w:color w:val=\"FFFFFF\"/><w:sz w:val=\"19\"/>") +
                   pstyle("TableText", "Table Text", "<w:spacing w:after=\"0\"/>", {}) +
                   pstyle("TableNumber", "Table Number", "<w:spacing w:after=\"0\"/><w:jc w:val=\"right\"/>", {}) +
                   pstyle("BillLine", "Bill Line", "<w:tabs><w:tab w:val=\"left\" w:pos=\"4800\"/><w:tab w:val=\"right\" w:pos=\"" + QByteArray::number(m.width) + "\"/></w:tabs><w:spacing w:after=\"40\"/>", {}) +
                   pstyle("SumLine", "Sum Line", "<w:tabs><w:tab w:val=\"right\" w:pos=\"" + QByteArray::number(m.width - 120) + "\"/></w:tabs><w:spacing w:after=\"40\"/><w:ind w:left=\"" + QByteArray::number(m.width - 3400) + "\"/>", {});
    const int bill = 4800, meta = m.width - bill;
    const QByteArray zero = "<w:top w:w=\"0\" w:type=\"dxa\"/><w:left w:w=\"0\" w:type=\"dxa\"/><w:bottom w:w=\"0\" w:type=\"dxa\"/><w:right w:w=\"0\" w:type=\"dxa\"/>";
    QByteArray b;
    QByteArray from = para("CompanyName", t(QT_TRANSLATE_NOOP("Templates", "[Your Company Name]")));
    from += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Street Address]")), "<w:spacing w:after=\"0\"/>");
    from += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[City, State ZIP]")), "<w:spacing w:after=\"0\"/>");
    from += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Phone number]  |  [Email address]")));
    b += tableOf(m.width, {bill, meta}, row(cell(bill, from) + cell(meta, para("InvoiceTitle", t(QT_TRANSLATE_NOOP("Templates", "Invoice"))))), {}, zero);
    // (who is billed at the left, and the number and dates at the right: a line each, with a tab between)
    b += para("SmallLabel", t(QT_TRANSLATE_NOOP("Templates", "Bill to")));
    auto billLine = [&](const QString &left, const QString &label, const QString &value, const QByteArray &leftLook = {}, const QByteArray &pPr = {}) {
        return paraOf("BillLine", runOf(left, leftLook) + "<w:r><w:tab/></w:r>" + runOf(label) + "<w:r><w:tab/></w:r>" + runOf(value), pPr);
    };
    b += billLine(t(QT_TRANSLATE_NOOP("Templates", "[Client Name]")), t(QT_TRANSLATE_NOOP("Templates", "Invoice number")), t(QT_TRANSLATE_NOOP("Templates", "[0001]")), "<w:b/>");
    b += billLine(t(QT_TRANSLATE_NOOP("Templates", "[Client Company]")), t(QT_TRANSLATE_NOOP("Templates", "Invoice date")), t(QT_TRANSLATE_NOOP("Templates", "[Date]")));
    b += billLine(t(QT_TRANSLATE_NOOP("Templates", "[Street Address, City, State ZIP]")), t(QT_TRANSLATE_NOOP("Templates", "Due date")), t(QT_TRANSLATE_NOOP("Templates", "[Date]")), {}, "<w:spacing w:after=\"280\"/>");
    const int qty = 900, unit = 1700, amount = 1800, desc = m.width - qty - unit - amount;
    const QByteArray hfill = shade(k.accent), hmargin = cellMargins(100, 120, 100, 120);
    QByteArray rows = row(cell(desc, para("TableHead", t(QT_TRANSLATE_NOOP("Templates", "Description"))), hfill + hmargin) + cell(qty, para("TableHead", t(QT_TRANSLATE_NOOP("Templates", "Qty")), "<w:jc w:val=\"right\"/>"), hfill + hmargin) +
                              cell(unit, para("TableHead", t(QT_TRANSLATE_NOOP("Templates", "Unit price")), "<w:jc w:val=\"right\"/>"), hfill + hmargin) + cell(amount, para("TableHead", t(QT_TRANSLATE_NOOP("Templates", "Amount")), "<w:jc w:val=\"right\"/>"), hfill + hmargin), 0, true);
    for (int i = 0; i < 7; ++i) {
        const bool first = i == 0;
        const QByteArray margin = cellMargins(110, 120, 110, 120);
        rows += row(cell(desc, para("TableText", first ? t(QT_TRANSLATE_NOOP("Templates", "[Describe the item or service]")) : QString()), margin) + cell(qty, para("TableNumber", first ? t(QT_TRANSLATE_NOOP("Templates", "[1]")) : QString()), margin) +
                    cell(unit, para("TableNumber", first ? t(QT_TRANSLATE_NOOP("Templates", "[0.00]")) : QString()), margin) + cell(amount, para("TableNumber", first ? t(QT_TRANSLATE_NOOP("Templates", "[0.00]")) : QString()), margin));
    }
    b += tableOf(m.width, {desc, qty, unit, amount}, rows, line("bottom", "BFBFBF") + line("insideH", "D9D9D9"), zero, {}, 120);
    // (the sums: a line each, the amount at the right edge, the total ruled off above)
    b += para("SumLine", t(QT_TRANSLATE_NOOP("Templates", "Subtotal\t[0.00]")), "<w:spacing w:before=\"160\" w:after=\"40\"/>");
    b += para("SumLine", t(QT_TRANSLATE_NOOP("Templates", "Tax\t[0.00]")));
    b += para("SumLine", t(QT_TRANSLATE_NOOP("Templates", "Total due\t[0.00]")), "<w:pBdr>" + line("top", k.accent, 12, 4) + "</w:pBdr><w:spacing w:before=\"60\"/>", "<w:b/><w:sz w:val=\"24\"/>");
    b += para("SmallLabel", t(QT_TRANSLATE_NOOP("Templates", "Payment terms")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Say when payment is due and how to pay: a bank transfer, a check address, or a payment link.]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "Thank you for your business.")), "<w:spacing w:before=\"360\"/><w:jc w:val=\"center\"/>", gray);
    b += sectPr(m.page, false, false);
    return assembleMore(m, b);
}

QByteArray meetingAgenda(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
    k.body = "Carlito";
    k.head = "Lato";
    k.size = 22;
    k.after = 100;
    k.line = 259;
    k.accent = "1F5F7A";
    k.titleSize = 52;
    Page p = pageSize();
    Made m = made(k, p);
    const QByteArray gray = "<w:color w:val=\"595959\"/>";
    m.look.extra = pstyle("SmallLabel", "Small Label", "<w:keepNext/><w:spacing w:after=\"20\"/>", fontOf(k.head) + "<w:b/><w:caps/>" + gray + "<w:spacing w:val=\"20\"/><w:sz w:val=\"17\"/>") +
                   pstyle("TableHead", "Table Heading", "<w:spacing w:after=\"0\"/>", fontOf(k.head) + "<w:b/><w:color w:val=\"" + k.accent + "\"/><w:sz w:val=\"19\"/>") +
                   pstyle("TableText", "Table Text", "<w:spacing w:after=\"0\"/>", {}) + bulletStyle();
    QByteArray b;
    b += para("Title", t(QT_TRANSLATE_NOOP("Templates", "Meeting Agenda")), "<w:spacing w:after=\"40\"/>");
    b += para("Subtitle", t(QT_TRANSLATE_NOOP("Templates", "[Meeting name or topic]")));
    const int half = m.width / 2;
    const QByteArray none = "<w:top w:w=\"0\" w:type=\"dxa\"/><w:left w:w=\"0\" w:type=\"dxa\"/><w:bottom w:w=\"0\" w:type=\"dxa\"/><w:right w:w=\"0\" w:type=\"dxa\"/>";
    auto fact = [](const QString &label, const QString &value) { return para("SmallLabel", label) + para(nullptr, value, "<w:spacing w:after=\"160\"/>"); };
    b += tableOf(m.width, {half, m.width - half}, row(cell(half, fact(t(QT_TRANSLATE_NOOP("Templates", "Date")), t(QT_TRANSLATE_NOOP("Templates", "[Date]"))) + fact(t(QT_TRANSLATE_NOOP("Templates", "Location")), t(QT_TRANSLATE_NOOP("Templates", "[Room, address, or meeting link]")))) +
                                                cell(m.width - half, fact(t(QT_TRANSLATE_NOOP("Templates", "Time")), t(QT_TRANSLATE_NOOP("Templates", "[Start time] to [End time]"))) + fact(t(QT_TRANSLATE_NOOP("Templates", "Facilitator")), t(QT_TRANSLATE_NOOP("Templates", "[Name]"))))), {}, none);
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "Attendees")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Names of the people who are invited]")));
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "Agenda")));
    const int time = 1300, lead = 2000, topic = m.width - time - lead;
    const QByteArray hm = cellMargins(100, 120, 100, 120), fill = shade("EAF0F4");
    QByteArray rows = row(cell(time, para("TableHead", t(QT_TRANSLATE_NOOP("Templates", "Time"))), fill + hm) + cell(topic, para("TableHead", t(QT_TRANSLATE_NOOP("Templates", "Topic"))), fill + hm) + cell(lead, para("TableHead", t(QT_TRANSLATE_NOOP("Templates", "Led by"))), fill + hm), 0, true);
    for (int i = 0; i < 5; ++i) {
        const QByteArray cm = cellMargins(120, 120, 120, 120);
        rows += row(cell(time, para("TableText", i == 0 ? t(QT_TRANSLATE_NOOP("Templates", "[5 min]")) : QString()), cm) + cell(topic, para("TableText", i == 0 ? t(QT_TRANSLATE_NOOP("Templates", "[What will be covered, and what you need from the group]")) : QString()), cm) +
                    cell(lead, para("TableText", i == 0 ? t(QT_TRANSLATE_NOOP("Templates", "[Name]")) : QString()), cm));
    }
    b += tableOf(m.width, {time, topic, lead}, rows, line("top", "BFBFBF") + line("bottom", "BFBFBF") + line("insideH", "D9D9D9"), none, {}, 120);
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "Before the meeting")));
    b += para("ListBullet", t(QT_TRANSLATE_NOOP("Templates", "[A document to read, or something to prepare]")));
    b += para("ListBullet", t(QT_TRANSLATE_NOOP("Templates", "[Something to bring]")));
    b += sectPr(m.page, false, false);
    return assembleMore(m, b, {}, {}, true);
}

QByteArray meetingNotes(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
    k.body = "Carlito";
    k.head = "Lato";
    k.size = 22;
    k.after = 100;
    k.line = 259;
    k.accent = "1F5F7A";
    k.titleSize = 52;
    Page p = pageSize();
    Made m = made(k, p);
    const QByteArray gray = "<w:color w:val=\"595959\"/>";
    m.look.extra = pstyle("MeetingFact", "Meeting Fact", "<w:tabs><w:tab w:val=\"left\" w:pos=\"1500\"/></w:tabs><w:spacing w:after=\"40\"/><w:ind w:left=\"1500\" w:hanging=\"1500\"/>", {}) +
                   pstyle("TableHead", "Table Heading", "<w:spacing w:after=\"0\"/>", fontOf(k.head) + "<w:b/><w:color w:val=\"" + k.accent + "\"/><w:sz w:val=\"19\"/>") +
                   pstyle("TableText", "Table Text", "<w:spacing w:after=\"0\"/>", {}) + bulletStyle();
    QByteArray b;
    b += para("Title", t(QT_TRANSLATE_NOOP("Templates", "Meeting Notes")), "<w:spacing w:after=\"40\"/>");
    b += para("Subtitle", t(QT_TRANSLATE_NOOP("Templates", "[Meeting name or topic]")));
    b += para("MeetingFact", t(QT_TRANSLATE_NOOP("Templates", "Date:\t[Date], [Start time] to [End time]")), {}, "<w:b/>");
    b += para("MeetingFact", t(QT_TRANSLATE_NOOP("Templates", "Location:\t[Room, address, or meeting link]")), {}, "<w:b/>");
    b += para("MeetingFact", t(QT_TRANSLATE_NOOP("Templates", "Present:\t[Names of the people who came]")), {}, "<w:b/>");
    b += para("MeetingFact", t(QT_TRANSLATE_NOOP("Templates", "Absent:\t[Names of the people who did not]")), {}, "<w:b/>");
    b += para("MeetingFact", t(QT_TRANSLATE_NOOP("Templates", "Notes by:\t[Name]")), "<w:spacing w:after=\"200\"/>" , "<w:b/>");
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Discussion")));
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "[First topic]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[What was said, in a few lines. Write down reasons and numbers, not every word.]")));
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "[Second topic]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[What was said, in a few lines.]")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Decisions")));
    b += para("ListBullet", t(QT_TRANSLATE_NOOP("Templates", "[What was decided, and why]")));
    b += para("ListBullet", t(QT_TRANSLATE_NOOP("Templates", "[What was decided, and why]")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Action items")));
    const int owner = 2200, due = 1800, action = m.width - owner - due;
    const QByteArray hm = cellMargins(100, 120, 100, 120), fill = shade("EAF0F4"), cm = cellMargins(120, 120, 120, 120);
    QByteArray rows = row(cell(action, para("TableHead", t(QT_TRANSLATE_NOOP("Templates", "Action"))), fill + hm) + cell(owner, para("TableHead", t(QT_TRANSLATE_NOOP("Templates", "Owner"))), fill + hm) + cell(due, para("TableHead", t(QT_TRANSLATE_NOOP("Templates", "Due"))), fill + hm), 0, true);
    for (int i = 0; i < 4; ++i)
        rows += row(cell(action, para("TableText", i == 0 ? t(QT_TRANSLATE_NOOP("Templates", "[What needs to be done]")) : QString()), cm) + cell(owner, para("TableText", i == 0 ? t(QT_TRANSLATE_NOOP("Templates", "[Name]")) : QString()), cm) + cell(due, para("TableText", i == 0 ? t(QT_TRANSLATE_NOOP("Templates", "[Date]")) : QString()), cm));
    b += tableOf(m.width, {action, owner, due}, rows, line("top", "BFBFBF") + line("bottom", "BFBFBF") + line("insideH", "D9D9D9"),
                 "<w:top w:w=\"0\" w:type=\"dxa\"/><w:left w:w=\"0\" w:type=\"dxa\"/><w:bottom w:w=\"0\" w:type=\"dxa\"/><w:right w:w=\"0\" w:type=\"dxa\"/>", {}, 120);
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Next meeting")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Date, time, and place, and what the group will cover]")), {}, gray);
    b += sectPr(m.page, false, false);
    return assembleMore(m, b, {}, {}, true);
}

QByteArray reportWithCover(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
    k.body = "Carlito";
    k.head = "Lora";
    k.accent = "1F5F7A";
    k.titleSize = 80;
    k.after = 140;
    Page p = pageSize();
    Made m = made(k, p);
    m.look.extra = pstyle("CoverTitle", "Cover Title", "<w:pBdr>" + line("bottom", k.accent, 18, 4) + "</w:pBdr><w:spacing w:before=\"3000\" w:after=\"240\"/>", {}, "Title") +
                   pstyle("CoverSubtitle", "Cover Subtitle", "<w:spacing w:after=\"2400\"/>", {}, "Subtitle") +
                   pstyle("TOCHeading", "TOC Heading", "<w:keepNext/><w:pageBreakBefore/><w:spacing w:before=\"0\" w:after=\"240\"/>", fontOf(k.head) + "<w:b/><w:color w:val=\"" + k.accent + "\"/><w:sz w:val=\"32\"/>") +
                   pstyle("CoverLine", "Cover Line", "<w:spacing w:after=\"0\"/>", "<w:color w:val=\"595959\"/><w:sz w:val=\"24\"/>");
    QByteArray b;
    b += para("CoverTitle", t(QT_TRANSLATE_NOOP("Templates", "[Report Title]")));
    b += para("CoverSubtitle", t(QT_TRANSLATE_NOOP("Templates", "[A line about what the report covers]")));
    b += para("CoverLine", t(QT_TRANSLATE_NOOP("Templates", "Prepared by [Your Name]")));
    b += para("CoverLine", t(QT_TRANSLATE_NOOP("Templates", "[Team or Organization]")));
    b += para("CoverLine", t(QT_TRANSLATE_NOOP("Templates", "[Date]")));
    b += para("TOCHeading", t(QT_TRANSLATE_NOOP("Templates", "Contents")));
    // (the contents are made from the headings once the document is open: see builtInTemplateDocument)
    b += para(nullptr, QStringLiteral("@@contents@@"));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Summary")), "<w:pageBreakBefore/>");
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Give the main points in a few sentences. A reader who stops here should know what you found and what you suggest.]")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Introduction")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Say why the report was written, who it is for, and what the reader needs to know first.]")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Findings")));
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "[First finding]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Describe what you found. Add numbers, a table, or a picture where they help.]")));
    b += para("Heading2", t(QT_TRANSLATE_NOOP("Templates", "[Second finding]")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Describe the next point in the same way.]")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Recommendations")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[List what should be done, who should do it, and by when.]")));
    b += para("Heading1", t(QT_TRANSLATE_NOOP("Templates", "Appendix")));
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Put long tables, sources, and other detail here.]")));
    // The cover has no page number: the first page of the section is its own.
    QByteArray sect = sectPr(m.page, false, true);
    sect.replace("</w:sectPr>", "<w:titlePg/></w:sectPr>");
    b += sect;
    const QByteArray footer = para("Footer", QString(), "<w:jc w:val=\"center\"/>").replace("</w:p>", pageField() + "</w:p>");
    return assembleMore(m, b, {}, footer);
}

QByteArray flyer(const QByteArray &lang)
{
    Look k;
    k.lang = lang;
    k.body = "Lato";
    k.head = "Montserrat";
    k.size = 24;
    k.after = 120;
    k.line = 264;
    k.accent = "B4381E";
    Page p = pageSize();
    p.top = p.bottom = 720;
    p.left = p.right = 720;
    Made m = made(k, p);
    const QByteArray zero = "<w:top w:w=\"0\" w:type=\"dxa\"/><w:left w:w=\"0\" w:type=\"dxa\"/><w:bottom w:w=\"0\" w:type=\"dxa\"/><w:right w:w=\"0\" w:type=\"dxa\"/>";
    m.look.extra = pstyle("FlyerTitle", "Flyer Title", "<w:spacing w:after=\"120\" w:line=\"216\" w:lineRule=\"auto\"/>", fontOf(k.head) + "<w:b/><w:caps/><w:color w:val=\"FFFFFF\"/><w:sz w:val=\"88\"/>") +
                   pstyle("FlyerTagline", "Flyer Tagline", "<w:spacing w:after=\"0\"/>", fontOf(k.head) + "<w:color w:val=\"FFFFFF\"/><w:sz w:val=\"30\"/>") +
                   pstyle("FlyerHeadline", "Flyer Headline", "<w:spacing w:before=\"480\" w:after=\"360\" w:line=\"240\" w:lineRule=\"auto\"/>", fontOf(k.head) + "<w:b/><w:color w:val=\"1F2937\"/><w:sz w:val=\"48\"/>") +
                   pstyle("DetailLabel", "Detail Label", "<w:spacing w:after=\"0\"/>", fontOf(k.head) + "<w:b/><w:caps/><w:color w:val=\"" + k.accent + "\"/><w:spacing w:val=\"30\"/><w:sz w:val=\"24\"/>") +
                   pstyle("DetailText", "Detail Text", "<w:spacing w:after=\"0\"/>", "<w:sz w:val=\"30\"/>") +
                   pstyle("ContactStrip", "Contact Strip", "<w:spacing w:after=\"0\"/><w:jc w:val=\"center\"/>", "<w:color w:val=\"1F2937\"/><w:sz w:val=\"26\"/>");
    QByteArray b;
    QByteArray top = para("FlyerTitle", t(QT_TRANSLATE_NOOP("Templates", "[Event or Offer Name]"))) + para("FlyerTagline", t(QT_TRANSLATE_NOOP("Templates", "[One line that says what it is]")));
    b += tableOf(m.width, {m.width}, row(cell(m.width, top, shade(k.accent) + cellMargins(900, 700, 900, 700)), 4200), {}, zero, {}, 700);
    b += para("FlyerHeadline", t(QT_TRANSLATE_NOOP("Templates", "[A short, bold line that gives people a reason to come]")));
    const int lab = 2000, val = m.width - lab;
    QByteArray rows;
    const QString labels[] = {t(QT_TRANSLATE_NOOP("Templates", "When")), t(QT_TRANSLATE_NOOP("Templates", "Where")), t(QT_TRANSLATE_NOOP("Templates", "Cost"))};
    const QString values[] = {t(QT_TRANSLATE_NOOP("Templates", "[Day, date, and time]")), t(QT_TRANSLATE_NOOP("Templates", "[Place and address]")), t(QT_TRANSLATE_NOOP("Templates", "[Price, or Free]"))};
    for (int i = 0; i < 3; ++i) rows += row(cell(lab, para("DetailLabel", labels[i]), cellMargins(160, 0, 160, 120)) + cell(val, para("DetailText", values[i]), cellMargins(160, 0, 160, 0)));
    b += tableOf(m.width, {lab, val}, rows, line("top", "D9D9D9") + line("bottom", "D9D9D9") + line("insideH", "D9D9D9"), zero);
    b += para(nullptr, t(QT_TRANSLATE_NOOP("Templates", "[Two or three sentences about what people will see, learn, or get. Keep it short: a flyer is read in seconds.]")), "<w:spacing w:before=\"360\" w:after=\"480\"/>");
    b += tableOf(m.width, {m.width}, row(cell(m.width, para("ContactStrip", t(QT_TRANSLATE_NOOP("Templates", "[Phone number]   |   [Email address]   |   [Website]"))), shade("F1F3F5") + cellMargins(240, 200, 240, 200))), {}, zero, {}, 200);
    b += para(nullptr, QString(), "<w:spacing w:after=\"0\" w:line=\"20\" w:lineRule=\"exact\"/>");
    b += sectPr(m.page, false, false);
    return assembleMore(m, b);
}

} // namespace

QVector<BuiltInTemplate> builtInTemplates()
{
    return {
        {QStringLiteral("letter"), t(QT_TRANSLATE_NOOP("Templates", "Letter")), t(QT_TRANSLATE_NOOP("Templates", "Letters")), t(QT_TRANSLATE_NOOP("Templates", "A personal letter with your address at the top."))},
        {QStringLiteral("coverletter"), t(QT_TRANSLATE_NOOP("Templates", "Cover Letter")), t(QT_TRANSLATE_NOOP("Templates", "Letters")), t(QT_TRANSLATE_NOOP("Templates", "A letter that goes with a resume when you apply for a job."))},
        {QStringLiteral("businessletter"), t(QT_TRANSLATE_NOOP("Templates", "Business Letter")), t(QT_TRANSLATE_NOOP("Templates", "Letters")), t(QT_TRANSLATE_NOOP("Templates", "A formal letter with your company name at the top and its address at the bottom."))},
        {QStringLiteral("report"), t(QT_TRANSLATE_NOOP("Templates", "Report")), t(QT_TRANSLATE_NOOP("Templates", "Reports and briefs")), t(QT_TRANSLATE_NOOP("Templates", "A short report with a title and numbered pages."))},
        {QStringLiteral("reportcover"), t(QT_TRANSLATE_NOOP("Templates", "Report with Cover Page")), t(QT_TRANSLATE_NOOP("Templates", "Reports and briefs")), t(QT_TRANSLATE_NOOP("Templates", "A longer report with a cover page and a table of contents that fills in from your headings."))},
        {QStringLiteral("brief"), t(QT_TRANSLATE_NOOP("Templates", "Brief")), t(QT_TRANSLATE_NOOP("Templates", "Reports and briefs")), t(QT_TRANSLATE_NOOP("Templates", "A one-page brief with To, From, Date, and Subject lines."))},
        {QStringLiteral("manuscript"), t(QT_TRANSLATE_NOOP("Templates", "Manuscript")), t(QT_TRANSLATE_NOOP("Templates", "Writing and publications")), t(QT_TRANSLATE_NOOP("Templates", "Double-spaced pages for a story or a book."))},
        {QStringLiteral("newsletter"), t(QT_TRANSLATE_NOOP("Templates", "Newsletter")), t(QT_TRANSLATE_NOOP("Templates", "Writing and publications")), t(QT_TRANSLATE_NOOP("Templates", "A masthead and two columns of stories."))},
        {QStringLiteral("flyer"), t(QT_TRANSLATE_NOOP("Templates", "Flyer")), t(QT_TRANSLATE_NOOP("Templates", "Writing and publications")), t(QT_TRANSLATE_NOOP("Templates", "A one-page flyer for an event, a sale, or a service."))},
        {QStringLiteral("resume"), t(QT_TRANSLATE_NOOP("Templates", "Resume")), t(QT_TRANSLATE_NOOP("Templates", "Resumes")), t(QT_TRANSLATE_NOOP("Templates", "A classic one-page resume."))},
        {QStringLiteral("resumeside"), t(QT_TRANSLATE_NOOP("Templates", "Resume with Side Column")), t(QT_TRANSLATE_NOOP("Templates", "Resumes")), t(QT_TRANSLATE_NOOP("Templates", "A resume with your contact details and skills in a column at the side."))},
        {QStringLiteral("invoice"), t(QT_TRANSLATE_NOOP("Templates", "Invoice")), t(QT_TRANSLATE_NOOP("Templates", "Business")), t(QT_TRANSLATE_NOOP("Templates", "A bill for your work, with a table for the items and the total."))},
        {QStringLiteral("agenda"), t(QT_TRANSLATE_NOOP("Templates", "Meeting Agenda")), t(QT_TRANSLATE_NOOP("Templates", "Business")), t(QT_TRANSLATE_NOOP("Templates", "The time, place, and topics of a meeting, with who leads each one."))},
        {QStringLiteral("meetingnotes"), t(QT_TRANSLATE_NOOP("Templates", "Meeting Notes")), t(QT_TRANSLATE_NOOP("Templates", "Business")), t(QT_TRANSLATE_NOOP("Templates", "A place to write down what was said, what was decided, and who does what next."))},
    };
}

std::unique_ptr<WriteDoc> builtInTemplateDocument(const QString &id, const QString &uiLanguage)
{
    const QByteArray lang = documentLanguage(uiLanguage.isNull() ? currentUiLanguage() : uiLanguage);
    QByteArray docx;
    if (id == QLatin1String("letter")) docx = letter(lang);
    else if (id == QLatin1String("report")) docx = report(lang);
    else if (id == QLatin1String("brief")) docx = brief(lang);
    else if (id == QLatin1String("manuscript")) docx = manuscript(lang);
    else if (id == QLatin1String("newsletter")) docx = newsletter(lang);
    else if (id == QLatin1String("coverletter")) docx = coverLetter(lang);
    else if (id == QLatin1String("businessletter")) docx = businessLetter(lang);
    else if (id == QLatin1String("reportcover")) docx = reportWithCover(lang);
    else if (id == QLatin1String("flyer")) docx = flyer(lang);
    else if (id == QLatin1String("resume")) docx = resume(lang);
    else if (id == QLatin1String("resumeside")) docx = resumeWithSideColumn(lang);
    else if (id == QLatin1String("invoice")) docx = invoice(lang);
    else if (id == QLatin1String("agenda")) docx = meetingAgenda(lang);
    else if (id == QLatin1String("meetingnotes")) docx = meetingNotes(lang);
    else return nullptr;
    QString err;
    auto doc = WriteDoc::fromDocx(docx, &err, nullptr);
    if (doc && id == QLatin1String("reportcover")) {
        // The table of contents is built from the template's own headings, in the place the cover's page break leaves for it.
        QTextDocument *story = doc->story();
        QTextCursor at = story->find(QStringLiteral("@@contents@@"));
        if (!at.isNull()) {
            at.removeSelectedText();
            TocOptions options;
            options.hyperlinks = false;   // (links to the headings are a note for a plain text or .rtf copy: the contents are page numbers, as in a book)
            insertTableOfContents(*doc, at, options);
        }
        doc->setModified(false);
    }
    return doc;
}

} // namespace jp::write

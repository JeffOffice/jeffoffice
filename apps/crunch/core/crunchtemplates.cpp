// The built-in templates. A small builder lays out cells by name (text, numbers, formulas, a style from the list below) and writes the .xlsx package a workbook is read from, as the blank workbook is.

#include "core/crunchtemplates.h"

#include "office/package.h"

#include <QCoreApplication>
#include <QFont>
#include <QImage>
#include <QPainter>
#include <QMap>

#include <utility>

namespace jp::crunch {

namespace {

// The styles, by place in cellXfs.
enum Style {
    Plain = 0, Title, Header, Money, MoneyIn, PercentIn, DateIn, TextIn, NumberIn, Bold, MoneyTotal, DateOut, TimeIn, Hours, PercentOut, DayNumber, Label, BoldTotal, TextOut, WholeOut
};

QString columnName(int col)   // 1 is A
{
    QString name;
    for (; col > 0; col = (col - 1) / 26) name.prepend(QChar(u'A' + (col - 1) % 26));
    return name;
}

QString ref(int row, int col) { return columnName(col) + QString::number(row); }

class Sheet {
public:
    void text(int row, int col, const QString &t, int style = Plain) {
        m_drawn[{row, col}] = {t, style}; m_cells[{row, col}] = QStringLiteral("<c r=\"%1\" s=\"%2\" t=\"inlineStr\"><is><t xml:space=\"preserve\">%3</t></is></c>").arg(ref(row, col)).arg(style).arg(t.toHtmlEscaped()); }
    void number(int row, int col, double v, int style = Plain) {
        m_drawn[{row, col}] = {QString::number(v), style}; m_cells[{row, col}] = QStringLiteral("<c r=\"%1\" s=\"%2\"><v>%3</v></c>").arg(ref(row, col)).arg(style).arg(v); }
    void formula(int row, int col, const QString &f, int style = Plain) {
        m_drawn[{row, col}] = {QString(), style}; m_cells[{row, col}] = QStringLiteral("<c r=\"%1\" s=\"%2\"><f>%3</f></c>").arg(ref(row, col)).arg(style).arg(f.toHtmlEscaped()); }
    void blank(int row, int col, int style) {
        m_drawn[{row, col}] = {QString(), style}; m_cells[{row, col}] = QStringLiteral("<c r=\"%1\" s=\"%2\"/>").arg(ref(row, col)).arg(style); }
    void width(int first, int last, double w) { m_widths.push_back({first, last, w}); }
    // A row of headers, from `col`.
    void headers(int row, const QStringList &names, int col = 1)
    {
        for (const QString &n : names) text(row, col++, n, Header);
    }
    // A small picture of the top of the sheet: the fills, borders, and typed texts as the styles give them (formulas show empty, as they have nothing worked out yet).
    QImage preview(int width) const
    {
        constexpr int kCanvasW = 640, kCanvasH = 440;
        QVector<double> colPx(40, 64.0);
        for (const Width &w : m_widths)
            for (int c = w.first; c <= w.last && c <= 40; ++c) colPx[c - 1] = w.width * 7 + 5;
        QImage img(kCanvasW, kCanvasH, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::TextAntialiasing);
        const double rowPx = 20;
        auto left = [&](int col) {
            double x = 8;
            for (int c = 1; c < col; ++c) x += colPx[c - 1];
            return x;
        };
        for (auto it = m_drawn.cbegin(); it != m_drawn.cend(); ++it) {
            const int row = it.key().first, col = it.key().second;
            const double x = left(col), y = 8 + (row - 1) * rowPx;
            if (col > 40 || x > kCanvasW || y > kCanvasH) continue;
            const QRectF cell(x, y, colPx[col - 1], rowPx);
            const int st = it.value().style;
            const bool header = st == Header, input = st == MoneyIn || st == PercentIn || st == DateIn || st == TextIn || st == NumberIn || st == TimeIn;
            const bool shaded = st == Label || st == BoldTotal || st == MoneyTotal;
            if (header) p.fillRect(cell, QColor(0x1F, 0x6F, 0x5C));
            else if (input) p.fillRect(cell, QColor(0xFF, 0xF4, 0xCC));
            else if (shaded) p.fillRect(cell, QColor(0xE8, 0xF1, 0xEE));
            if (header || input || st == Money || st == DateOut || st == Hours || st == PercentOut || st == DayNumber || st == TextOut || st == WholeOut) {
                p.setPen(QColor(0xBF, 0xBF, 0xBF));
                p.drawRect(cell);
            }
            QFont f = p.font();
            f.setPixelSize(st == Title ? 18 : 13);
            f.setBold(st == Title || header || st == Bold || st == Label || st == BoldTotal || st == MoneyTotal);
            p.setFont(f);
            p.setPen(st == Title ? QColor(0x1F, 0x6F, 0x5C) : header ? QColor(Qt::white) : QColor(0x20, 0x20, 0x20));
            if (!it.value().text.isEmpty())
                p.drawText(cell.adjusted(4, 0, st == Title ? 400 : -2, 0), (header ? Qt::AlignCenter : Qt::AlignVCenter | Qt::AlignLeft) | Qt::TextSingleLine, it.value().text);
        }
        p.end();
        return img.scaledToWidth(width, Qt::SmoothTransformation);
    }
    QByteArray xml() const
    {
        QString cols;
        for (const auto &w : m_widths) cols += QStringLiteral("<col min=\"%1\" max=\"%2\" width=\"%3\" customWidth=\"1\"/>").arg(w.first).arg(w.last).arg(w.width);
        QString rows;
        int open = -1;
        for (auto it = m_cells.cbegin(); it != m_cells.cend(); ++it) {
            if (it.key().first != open) {
                if (open != -1) rows += QStringLiteral("</row>");
                open = it.key().first;
                rows += QStringLiteral("<row r=\"%1\">").arg(open);
            }
            rows += it.value();
        }
        if (open != -1) rows += QStringLiteral("</row>");
        return ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheetViews><sheetView workbookViewId=\"0\" showGridLines=\"0\"/></sheetViews>"
                "<sheetFormatPr defaultRowHeight=\"15\"/>" + (cols.isEmpty() ? QString() : "<cols>" + cols + "</cols>") + "<sheetData>" + rows + "</sheetData></worksheet>").toUtf8();
    }

private:
    struct Width {
        int first, last;
        double width;
    };
    struct Drawn {
        QString text;
        int style = 0;
    };
    QMap<std::pair<int, int>, QString> m_cells;
    QMap<std::pair<int, int>, Drawn> m_drawn;
    QVector<Width> m_widths;
};

QByteArray stylesXml()
{
    const QString xf = QStringLiteral("<xf numFmtId=\"%1\" fontId=\"%2\" fillId=\"%3\" borderId=\"%4\" xfId=\"0\" applyNumberFormat=\"1\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\"%5/>");
    const auto x = [&](int fmt, int font, int fill, int border, const QString &align = QString()) {
        return align.isEmpty() ? xf.arg(fmt).arg(font).arg(fill).arg(border).arg(QString()) : xf.arg(fmt).arg(font).arg(fill).arg(border).arg(QStringLiteral(" applyAlignment=\"1\"")).replace(QStringLiteral("/>"), QStringLiteral(">") + align + QStringLiteral("</xf>"));
    };
    const QString center = QStringLiteral("<alignment horizontal=\"center\" vertical=\"center\" wrapText=\"1\"/>");
    QString cellXfs;
    cellXfs += x(0, 0, 0, 0);            // Plain
    cellXfs += x(0, 2, 0, 0);            // Title
    cellXfs += x(0, 3, 2, 1, center);    // Header
    cellXfs += x(164, 0, 0, 1);          // Money
    cellXfs += x(164, 0, 3, 1);          // MoneyIn
    cellXfs += x(10, 0, 3, 1);           // PercentIn
    cellXfs += x(14, 0, 3, 1);           // DateIn
    cellXfs += x(0, 0, 3, 1);            // TextIn
    cellXfs += x(0, 0, 3, 1);            // NumberIn
    cellXfs += x(0, 1, 0, 0);            // Bold
    cellXfs += x(164, 1, 4, 2);          // MoneyTotal
    cellXfs += x(14, 0, 0, 1);           // DateOut
    cellXfs += x(20, 0, 3, 1);           // TimeIn
    cellXfs += x(2, 0, 0, 1);            // Hours
    cellXfs += x(10, 0, 0, 1);           // PercentOut
    cellXfs += x(165, 0, 0, 1, QStringLiteral("<alignment horizontal=\"right\" vertical=\"top\"/>"));   // DayNumber
    cellXfs += x(0, 1, 4, 0);            // Label
    cellXfs += x(0, 1, 4, 2);            // BoldTotal
    cellXfs += x(0, 0, 0, 1);            // TextOut
    cellXfs += x(1, 0, 0, 1);            // WholeOut
    return ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
            "<numFmts count=\"2\"><numFmt numFmtId=\"164\" formatCode=\"&quot;$&quot;#,##0.00\"/><numFmt numFmtId=\"165\" formatCode=\"d\"/></numFmts>"
            "<fonts count=\"4\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/></font><font><b/><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/></font>"
            "<font><b/><sz val=\"18\"/><color rgb=\"FF1F6F5C\"/><name val=\"Calibri\"/><family val=\"2\"/></font><font><b/><sz val=\"11\"/><color rgb=\"FFFFFFFF\"/><name val=\"Calibri\"/><family val=\"2\"/></font></fonts>"
            "<fills count=\"5\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"
            "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF1F6F5C\"/><bgColor indexed=\"64\"/></patternFill></fill>"
            "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFFFF4CC\"/><bgColor indexed=\"64\"/></patternFill></fill>"
            "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFE8F1EE\"/><bgColor indexed=\"64\"/></patternFill></fill></fills>"
            "<borders count=\"3\"><border><left/><right/><top/><bottom/><diagonal/></border>"
            "<border><left style=\"thin\"><color rgb=\"FFBFBFBF\"/></left><right style=\"thin\"><color rgb=\"FFBFBFBF\"/></right><top style=\"thin\"><color rgb=\"FFBFBFBF\"/></top><bottom style=\"thin\"><color rgb=\"FFBFBFBF\"/></bottom><diagonal/></border>"
            "<border><left/><right/><top style=\"thin\"><color auto=\"1\"/></top><bottom/><diagonal/></border></borders>"
            "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs><cellXfs count=\"20\">" + cellXfs
            + "</cellXfs><cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles></styleSheet>").toUtf8();
}

QByteArray package(const Sheet &sheet, const QString &sheetName)
{
    using office::PackageWriter;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    PackageWriter w;
    w.addPart(QStringLiteral("/xl/workbook.xml"), QByteArray(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/xl/workbook.xml"));
    w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), sheet.xml(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    const QString sheetId = w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + QStringLiteral("worksheet"), QStringLiteral("/xl/worksheets/sheet1.xml"));
    w.addPart(QStringLiteral("/xl/styles.xml"), stylesXml(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + QStringLiteral("styles"), QStringLiteral("/xl/styles.xml"));
    w.addPart(QStringLiteral("/xl/workbook.xml"),
              ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"" + rel.chopped(1)
               + "\"><bookViews><workbookView/></bookViews><sheets><sheet name=\"" + sheetName + "\" sheetId=\"1\" r:id=\"" + sheetId + "\"/></sheets><calcPr calcId=\"191029\" fullCalcOnLoad=\"1\"/></workbook>").toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    return w.finish();
}

// ---- the templates, one function each

Sheet budget(QString *name)
{
    Sheet s;
    s.width(1, 1, 28);
    s.width(2, 4, 16);
    s.text(1, 1, QStringLiteral("Monthly Budget"), Title);
    s.text(2, 1, QStringLiteral("Type what you plan in the yellow boxes. The other numbers are worked out."));
    int row = 4;
    auto block = [&](const QString &title, const QStringList &items, bool income) {
        s.headers(row, {title, QStringLiteral("Planned"), QStringLiteral("Actual"), QStringLiteral("Difference")});
        const int first = ++row;
        for (const QString &item : items) {
            s.text(row, 1, item, TextOut);
            s.blank(row, 2, MoneyIn);
            s.blank(row, 3, MoneyIn);
            s.formula(row, 4, income ? QStringLiteral("C%1-B%1").arg(row) : QStringLiteral("B%1-C%1").arg(row), Money);
            ++row;
        }
        s.text(row, 1, income ? QStringLiteral("Total income") : QStringLiteral("Total expenses"), BoldTotal);
        for (int c = 2; c <= 4; ++c) s.formula(row, c, QStringLiteral("SUM(%1%2:%1%3)").arg(columnName(c)).arg(first).arg(row - 1), MoneyTotal);
        const int total = row;
        row += 2;
        return total;
    };
    const int income = block(QStringLiteral("Income"), {QStringLiteral("Pay"), QStringLiteral("Other income")}, true);
    const int spent = block(QStringLiteral("Expenses"), {QStringLiteral("Housing"), QStringLiteral("Food"), QStringLiteral("Transportation"), QStringLiteral("Utilities"), QStringLiteral("Insurance"), QStringLiteral("Savings"), QStringLiteral("Fun")}, false);
    s.text(row, 1, QStringLiteral("Left over"), Label);
    s.formula(row, 2, QStringLiteral("B%1-B%2").arg(income).arg(spent), MoneyTotal);
    s.formula(row, 3, QStringLiteral("C%1-C%2").arg(income).arg(spent), MoneyTotal);
    *name = QStringLiteral("Budget");
    return s;
}

Sheet invoice(QString *name)
{
    Sheet s;
    s.width(1, 1, 36);
    s.width(2, 4, 15);
    s.text(1, 1, QStringLiteral("Invoice"), Title);
    s.text(3, 1, QStringLiteral("Bill to"), Label);
    s.blank(3, 2, TextIn);
    s.text(4, 1, QStringLiteral("Invoice number"), Label);
    s.blank(4, 2, TextIn);
    s.text(5, 1, QStringLiteral("Date"), Label);
    s.blank(5, 2, DateIn);
    s.headers(7, {QStringLiteral("Description"), QStringLiteral("Quantity"), QStringLiteral("Unit price"), QStringLiteral("Amount")});
    for (int r = 8; r <= 17; ++r) {
        s.blank(r, 1, TextIn);
        s.blank(r, 2, NumberIn);
        s.blank(r, 3, MoneyIn);
        s.formula(r, 4, QStringLiteral("IF(COUNT(B%1:C%1)=0,\"\",B%1*C%1)").arg(r), Money);
    }
    s.text(19, 3, QStringLiteral("Subtotal"), Bold);
    s.formula(19, 4, QStringLiteral("SUM(D8:D17)"), Money);
    s.text(20, 3, QStringLiteral("Tax rate"), Bold);
    s.number(20, 2, 0, PercentIn);
    s.formula(20, 4, QStringLiteral("D19*B20"), Money);
    s.text(21, 3, QStringLiteral("Total due"), BoldTotal);
    s.formula(21, 4, QStringLiteral("D19+D20"), MoneyTotal);
    *name = QStringLiteral("Invoice");
    return s;
}

Sheet expenseReport(QString *name)
{
    Sheet s;
    s.width(1, 1, 14);
    s.width(2, 2, 34);
    s.width(3, 4, 16);
    s.text(1, 1, QStringLiteral("Expense Report"), Title);
    s.text(3, 1, QStringLiteral("Name"), Label);
    s.blank(3, 2, TextIn);
    s.text(4, 1, QStringLiteral("Period"), Label);
    s.blank(4, 2, TextIn);
    s.headers(6, {QStringLiteral("Date"), QStringLiteral("Description"), QStringLiteral("Category"), QStringLiteral("Amount")});
    for (int r = 7; r <= 26; ++r) {
        s.blank(r, 1, DateIn);
        s.blank(r, 2, TextIn);
        s.blank(r, 3, TextIn);
        s.blank(r, 4, MoneyIn);
    }
    s.text(28, 3, QStringLiteral("Total"), BoldTotal);
    s.formula(28, 4, QStringLiteral("SUM(D7:D26)"), MoneyTotal);
    *name = QStringLiteral("Expenses");
    return s;
}

Sheet loanSchedule(QString *name)
{
    Sheet s;
    s.width(1, 1, 22);
    s.width(2, 5, 15);
    s.text(1, 1, QStringLiteral("Loan Schedule"), Title);
    s.text(3, 1, QStringLiteral("Loan amount"), Label);
    s.number(3, 2, 20000, MoneyIn);
    s.text(4, 1, QStringLiteral("Yearly interest rate"), Label);
    s.number(4, 2, 0.06, PercentIn);
    s.text(5, 1, QStringLiteral("Years"), Label);
    s.number(5, 2, 5, NumberIn);
    s.text(6, 1, QStringLiteral("Monthly payment"), Label);
    s.formula(6, 2, QStringLiteral("IF(B4=0,B3/(B5*12),-PMT(B4/12,B5*12,B3))"), MoneyTotal);
    s.text(7, 1, QStringLiteral("Total interest"), Label);
    s.formula(7, 2, QStringLiteral("SUM(C11:C370)"), MoneyTotal);
    s.headers(9, {QStringLiteral("Payment"), QStringLiteral("Paid"), QStringLiteral("Interest"), QStringLiteral("Principal"), QStringLiteral("Balance")});
    s.number(10, 1, 0, WholeOut);
    s.formula(10, 5, QStringLiteral("B3"), Money);
    for (int r = 11; r <= 370; ++r) {
        s.formula(r, 1, QStringLiteral("IF(ROW()-10<=$B$5*12,ROW()-10,\"\")"), WholeOut);
        s.formula(r, 2, QStringLiteral("IF(A%1=\"\",\"\",$B$6)").arg(r), Money);
        s.formula(r, 3, QStringLiteral("IF(A%1=\"\",\"\",E%2*$B$4/12)").arg(r).arg(r - 1), Money);
        s.formula(r, 4, QStringLiteral("IF(A%1=\"\",\"\",B%1-C%1)").arg(r), Money);
        s.formula(r, 5, QStringLiteral("IF(A%1=\"\",\"\",E%2-D%1)").arg(r).arg(r - 1), Money);
    }
    *name = QStringLiteral("Loan");
    return s;
}

Sheet calendar(QString *name)
{
    Sheet s;
    s.width(1, 7, 13);
    s.text(1, 1, QStringLiteral("Calendar"), Title);
    s.text(2, 1, QStringLiteral("Year"), Label);
    s.number(2, 2, 2027, NumberIn);
    s.text(2, 3, QStringLiteral("Month"), Label);
    s.number(2, 4, 1, NumberIn);
    s.formula(4, 1, QStringLiteral("TEXT(DATE(B2,D2,1),\"mmmm yyyy\")"), Bold);
    const QStringList days = {QStringLiteral("Sun"), QStringLiteral("Mon"), QStringLiteral("Tue"), QStringLiteral("Wed"), QStringLiteral("Thu"), QStringLiteral("Fri"), QStringLiteral("Sat")};
    s.headers(5, days);
    for (int week = 0; week < 6; ++week)
        for (int d = 0; d < 7; ++d) {
            const int row = 6 + week, col = 1 + d;
            if (week == 0 && d == 0) s.formula(row, col, QStringLiteral("DATE(B2,D2,1)-WEEKDAY(DATE(B2,D2,1))+1"), DayNumber);
            else s.formula(row, col, d == 0 ? QStringLiteral("G%1+1").arg(row - 1) : QStringLiteral("%1%2+1").arg(columnName(col - 1)).arg(row), DayNumber);
        }
    *name = QStringLiteral("Calendar");
    return s;
}

Sheet timesheet(QString *name)
{
    Sheet s;
    s.width(1, 1, 18);
    s.width(2, 5, 13);
    s.text(1, 1, QStringLiteral("Timesheet"), Title);
    s.text(3, 1, QStringLiteral("Name"), Label);
    s.blank(3, 2, TextIn);
    s.text(4, 1, QStringLiteral("Week starting"), Label);
    s.blank(4, 2, DateIn);
    s.headers(6, {QStringLiteral("Day"), QStringLiteral("Start"), QStringLiteral("End"), QStringLiteral("Break (hours)"), QStringLiteral("Hours")});
    for (int r = 7; r <= 13; ++r) {
        s.formula(r, 1, r == 7 ? QStringLiteral("IF($B$4=\"\",\"\",$B$4)") : QStringLiteral("IF(A%1=\"\",\"\",A%1+1)").arg(r - 1), DateOut);
        s.blank(r, 2, TimeIn);
        s.blank(r, 3, TimeIn);
        s.blank(r, 4, NumberIn);
        s.formula(r, 5, QStringLiteral("IF(OR(B%1=\"\",C%1=\"\"),0,(C%1-B%1)*24-N(D%1))").arg(r), Hours);
    }
    s.text(14, 4, QStringLiteral("Total hours"), BoldTotal);
    s.formula(14, 5, QStringLiteral("SUM(E7:E13)"), Hours);
    s.text(15, 4, QStringLiteral("Hourly rate"), Bold);
    s.blank(15, 5, MoneyIn);
    s.text(16, 4, QStringLiteral("Pay"), BoldTotal);
    s.formula(16, 5, QStringLiteral("E14*N(E15)"), MoneyTotal);
    *name = QStringLiteral("Timesheet");
    return s;
}

Sheet checkbookRegister(QString *name)
{
    Sheet s;
    s.width(1, 2, 13);
    s.width(3, 3, 34);
    s.width(4, 6, 15);
    s.text(1, 1, QStringLiteral("Checkbook Register"), Title);
    s.text(2, 1, QStringLiteral("Starting balance"), Label);
    s.number(2, 2, 0, MoneyIn);
    s.headers(4, {QStringLiteral("Date"), QStringLiteral("Number"), QStringLiteral("Description"), QStringLiteral("Payment"), QStringLiteral("Deposit"), QStringLiteral("Balance")});
    for (int r = 5; r <= 104; ++r) {
        s.blank(r, 1, DateIn);
        s.blank(r, 2, TextIn);
        s.blank(r, 3, TextIn);
        s.blank(r, 4, MoneyIn);
        s.blank(r, 5, MoneyIn);
        s.formula(r, 6, QStringLiteral("IF(AND(D%1=\"\",E%1=\"\"),\"\",$B$2+SUM(E$5:E%1)-SUM(D$5:D%1))").arg(r), Money);
    }
    *name = QStringLiteral("Register");
    return s;
}

Sheet gradeBook(QString *name)
{
    Sheet s;
    s.width(1, 1, 24);
    s.width(2, 7, 12);
    s.text(1, 1, QStringLiteral("Grade Book"), Title);
    s.text(2, 1, QStringLiteral("Weight of each score"), Label);
    const double weights[] = {0.2, 0.2, 0.25, 0.35};
    for (int i = 0; i < 4; ++i) s.number(2, 2 + i, weights[i], PercentIn);
    s.headers(3, {QStringLiteral("Student"), QStringLiteral("Homework"), QStringLiteral("Quiz"), QStringLiteral("Midterm"), QStringLiteral("Final"), QStringLiteral("Total"), QStringLiteral("Letter")});
    for (int r = 4; r <= 33; ++r) {
        s.blank(r, 1, TextIn);
        for (int c = 2; c <= 5; ++c) s.blank(r, c, NumberIn);
        s.formula(r, 6, QStringLiteral("IF(COUNT(B%1:E%1)=0,\"\",SUMPRODUCT(B%1:E%1,$B$2:$E$2))").arg(r), Hours);
        s.formula(r, 7, QStringLiteral("IF(F%1=\"\",\"\",IF(F%1>=90,\"A\",IF(F%1>=80,\"B\",IF(F%1>=70,\"C\",IF(F%1>=60,\"D\",\"F\")))))").arg(r), TextOut);
    }
    s.text(35, 1, QStringLiteral("Class average"), BoldTotal);
    for (int c = 2; c <= 6; ++c) {
        const QString range = columnName(c) + QStringLiteral("4:") + columnName(c) + QStringLiteral("33");
        s.formula(35, c, QStringLiteral("IF(COUNT(%1)=0,\"\",AVERAGE(%1))").arg(range), Hours);
    }
    *name = QStringLiteral("Grades");
    return s;
}

Sheet inventoryList(QString *name)
{
    Sheet s;
    s.width(1, 1, 28);
    s.width(2, 3, 14);
    s.width(4, 8, 13);
    s.text(1, 1, QStringLiteral("Inventory List"), Title);
    s.headers(3, {QStringLiteral("Item"), QStringLiteral("Code"), QStringLiteral("Location"), QStringLiteral("In stock"), QStringLiteral("Reorder at"), QStringLiteral("Unit cost"), QStringLiteral("Value"), QStringLiteral("Status")});
    for (int r = 4; r <= 53; ++r) {
        for (int c = 1; c <= 3; ++c) s.blank(r, c, TextIn);
        s.blank(r, 4, NumberIn);
        s.blank(r, 5, NumberIn);
        s.blank(r, 6, MoneyIn);
        s.formula(r, 7, QStringLiteral("IF(D%1=\"\",\"\",D%1*N(F%1))").arg(r), Money);
        s.formula(r, 8, QStringLiteral("IF(D%1=\"\",\"\",IF(D%1<=N(E%1),\"Reorder\",\"OK\"))").arg(r), TextOut);
    }
    s.text(55, 6, QStringLiteral("Total value"), BoldTotal);
    s.formula(55, 7, QStringLiteral("SUM(G4:G53)"), MoneyTotal);
    *name = QStringLiteral("Inventory");
    return s;
}

Sheet mileageLog(QString *name)
{
    Sheet s;
    s.width(1, 1, 13);
    s.width(2, 2, 32);
    s.width(3, 6, 15);
    s.text(1, 1, QStringLiteral("Mileage Log"), Title);
    s.text(2, 1, QStringLiteral("Rate per mile"), Label);
    s.number(2, 2, 0.5, MoneyIn);
    s.headers(4, {QStringLiteral("Date"), QStringLiteral("Purpose"), QStringLiteral("Start odometer"), QStringLiteral("End odometer"), QStringLiteral("Miles"), QStringLiteral("Amount")});
    for (int r = 5; r <= 54; ++r) {
        s.blank(r, 1, DateIn);
        s.blank(r, 2, TextIn);
        s.blank(r, 3, NumberIn);
        s.blank(r, 4, NumberIn);
        s.formula(r, 5, QStringLiteral("IF(OR(C%1=\"\",D%1=\"\"),\"\",D%1-C%1)").arg(r), WholeOut);
        s.formula(r, 6, QStringLiteral("IF(E%1=\"\",\"\",E%1*$B$2)").arg(r), Money);
    }
    s.text(56, 4, QStringLiteral("Totals"), BoldTotal);
    s.formula(56, 5, QStringLiteral("SUM(E5:E54)"), WholeOut);
    s.formula(56, 6, QStringLiteral("SUM(F5:F54)"), MoneyTotal);
    *name = QStringLiteral("Mileage");
    return s;
}

Sheet projectPlan(QString *name)
{
    Sheet s;
    s.width(1, 1, 32);
    s.width(2, 2, 16);
    s.width(3, 7, 13);
    s.text(1, 1, QStringLiteral("Project Plan"), Title);
    s.headers(3, {QStringLiteral("Task"), QStringLiteral("Owner"), QStringLiteral("Start"), QStringLiteral("Days"), QStringLiteral("End"), QStringLiteral("Done"), QStringLiteral("Status")});
    for (int r = 4; r <= 23; ++r) {
        s.blank(r, 1, TextIn);
        s.blank(r, 2, TextIn);
        s.blank(r, 3, DateIn);
        s.blank(r, 4, NumberIn);
        s.formula(r, 5, QStringLiteral("IF(OR(C%1=\"\",D%1=\"\"),\"\",C%1+D%1-1)").arg(r), DateOut);
        s.blank(r, 6, PercentIn);
        s.formula(r, 7, QStringLiteral("IF(F%1=\"\",\"\",IF(F%1>=1,\"Done\",IF(F%1>0,\"In progress\",\"Not started\")))").arg(r), TextOut);
    }
    s.text(25, 1, QStringLiteral("Overall"), BoldTotal);
    s.formula(25, 6, QStringLiteral("IF(COUNT(F4:F23)=0,\"\",AVERAGE(F4:F23))"), PercentOut);
    *name = QStringLiteral("Plan");
    return s;
}

Sheet salesReport(QString *name)
{
    Sheet s;
    s.width(1, 1, 22);
    s.width(2, 15, 11);
    s.text(1, 1, QStringLiteral("Sales Report"), Title);
    QStringList head = {QStringLiteral("Product")};
    for (const char *m : {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}) head << QString::fromLatin1(m);
    head << QStringLiteral("Total") << QStringLiteral("Share");
    s.headers(3, head);
    for (int r = 4; r <= 13; ++r) {
        s.blank(r, 1, TextIn);
        for (int c = 2; c <= 13; ++c) s.blank(r, c, MoneyIn);
        s.formula(r, 14, QStringLiteral("SUM(B%1:M%1)").arg(r), Money);
        s.formula(r, 15, QStringLiteral("IF($N$14=0,\"\",N%1/$N$14)").arg(r), PercentOut);
    }
    s.text(14, 1, QStringLiteral("Total"), BoldTotal);
    for (int c = 2; c <= 14; ++c) s.formula(14, c, QStringLiteral("SUM(") + columnName(c) + QStringLiteral("4:") + columnName(c) + QStringLiteral("13)"), MoneyTotal);
    *name = QStringLiteral("Sales");
    return s;
}

struct Entry {
    const char *id, *name, *category, *description;
    Sheet (*make)(QString *name);
};

const Entry kEntries[] = {
    {"budget", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Monthly Budget"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Finance"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Plan income and spending; see what is left."), budget},
    {"loan", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Loan Schedule"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Finance"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Interest and principal of each loan payment."), loanSchedule},
    {"checkbook", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Checkbook Register"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Finance"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "A running balance of payments and deposits."), checkbookRegister},
    {"invoice", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Invoice"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Business"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Bill a customer, with tax and a total."), invoice},
    {"expenses", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Expense Report"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Business"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "List what you spent, to be paid back."), expenseReport},
    {"timesheet", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Timesheet"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Business"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Hours worked in a week, and the pay."), timesheet},
    {"inventory", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Inventory List"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Business"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Stock counts, and what to reorder."), inventoryList},
    {"mileage", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Mileage Log"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Business"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Log your trips and what each mile is worth."), mileageLog},
    {"sales", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Sales Report"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Business"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Monthly sales by product, with totals."), salesReport},
    {"calendar", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Calendar"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Planning"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "A month at a glance."), calendar},
    {"project", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Project Plan"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Planning"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Tasks with owners, dates, and progress."), projectPlan},
    {"grades", QT_TRANSLATE_NOOP("jp::crunch::Templates", "Grade Book"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Education"), QT_TRANSLATE_NOOP("jp::crunch::Templates", "Weighted scores and letter grades."), gradeBook},
};

} // namespace

QVector<TemplateInfo> builtinTemplates()
{
    QVector<TemplateInfo> out;
    for (const Entry &e : kEntries)
        out.push_back({QString::fromLatin1(e.id), QCoreApplication::translate("jp::crunch::Templates", e.name), QCoreApplication::translate("jp::crunch::Templates", e.category),
                       QCoreApplication::translate("jp::crunch::Templates", e.description)});
    return out;
}

QImage builtinTemplatePreview(const QString &id, int width)
{
    for (const Entry &e : kEntries)
        if (id == QLatin1String(e.id)) {
            QString name;
            return e.make(&name).preview(width);
        }
    return QImage();
}

QByteArray builtinTemplateBytes(const QString &id)
{
    for (const Entry &e : kEntries)
        if (id == QLatin1String(e.id)) {
            QString name;
            const Sheet sheet = e.make(&name);
            return package(sheet, name);
        }
    return QByteArray();
}

} // namespace jp::crunch

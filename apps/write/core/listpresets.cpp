#include "core/listpresets.h"

#include <QCoreApplication>

using namespace jp::office::wml;

namespace jp::write {

AbstractNum plainListDefinition(bool numbers)
{
    AbstractNum a;
    a.multiLevelType = QStringLiteral("hybridMultilevel");
    static const char *const bullets[] = {"•", "◦", "▪"};
    static const char *const formats[] = {"decimal", "lowerLetter", "lowerRoman"};
    for (int i = 0; i < 9; ++i) {
        Level &l = a.levels[i];
        l.present = true;
        l.jc = QStringLiteral("left");
        if (numbers) {
            l.numFmt = QString::fromLatin1(formats[i % 3]);
            l.text = QStringLiteral("%%1.").arg(i + 1);
        } else {
            l.numFmt = QStringLiteral("bullet");
            l.text = QString::fromUtf8(bullets[i % 3]);
        }
        l.pPr.indLeft = 720 * (i + 1);
        l.pPr.indFirst = -360;
    }
    return a;
}

Level bulletLevel(const QString &text, const QString &font)
{
    Level l;
    l.present = true;
    l.numFmt = QStringLiteral("bullet");
    l.text = text;
    if (!font.isEmpty()) {
        Fonts f;
        f.ascii = f.hAnsi = font;
        l.rPr.fonts = f;
    }
    return l;
}

Level numberLevel(const QString &numFmt, const QString &text, int start)
{
    Level l;
    l.present = true;
    l.numFmt = numFmt;
    l.text = text;
    l.start = start;
    return l;
}

QVector<BulletPreset> bulletPresets()
{
    return {{QStringLiteral("•"), QString(), QCoreApplication::translate("jp::write::Lists", "Bullet")},      {QStringLiteral("○"), QString(), QCoreApplication::translate("jp::write::Lists", "Circle")},    {QStringLiteral("■"), QString(), QCoreApplication::translate("jp::write::Lists", "Square")},
            {QStringLiteral("◆"), QString(), QCoreApplication::translate("jp::write::Lists", "Diamond")},     {QStringLiteral("➢"), QString(), QCoreApplication::translate("jp::write::Lists", "Arrowhead")}, {QStringLiteral("✓"), QString(), QCoreApplication::translate("jp::write::Lists", "Check mark")},
            {QStringLiteral("–"), QString(), QCoreApplication::translate("jp::write::Lists", "Dash")}};
}

QVector<NumberPreset> numberPresets()
{
    return {{QStringLiteral("1."), QStringLiteral("decimal"), QStringLiteral("%1."), QStringLiteral("1. 2. 3.")},
            {QStringLiteral("1)"), QStringLiteral("decimal"), QStringLiteral("%1)"), QStringLiteral("1) 2) 3)")},
            {QStringLiteral("I."), QStringLiteral("upperRoman"), QStringLiteral("%1."), QStringLiteral("I. II. III.")},
            {QStringLiteral("A."), QStringLiteral("upperLetter"), QStringLiteral("%1."), QStringLiteral("A. B. C.")},
            {QStringLiteral("a)"), QStringLiteral("lowerLetter"), QStringLiteral("%1)"), QStringLiteral("a) b) c)")},
            {QStringLiteral("a."), QStringLiteral("lowerLetter"), QStringLiteral("%1."), QStringLiteral("a. b. c.")},
            {QStringLiteral("i."), QStringLiteral("lowerRoman"), QStringLiteral("%1."), QStringLiteral("i. ii. iii.")}};
}

namespace {

// "%1.%2.%3" up to level i (0 to 8), with `tail` after it.
QString chain(int i, const QString &tail)
{
    // (built a character at a time: the mixed temporaries of the first version crashed the clang build on macOS)
    QString s;
    for (int k = 0; k <= i; ++k) {
        if (k) s += QLatin1Char('.');
        s += QLatin1Char('%');
        s += QString::number(k + 1);
    }
    s += tail;
    return s;
}

AbstractNum multilevelBase()
{
    AbstractNum a;
    a.multiLevelType = QStringLiteral("multilevel");
    for (int i = 0; i < 9; ++i) {
        Level &l = a.levels[i];
        l.present = true;
        l.numFmt = QStringLiteral("decimal");
        l.jc = QStringLiteral("left");
        l.pPr.indLeft = 720 * (i + 1);
        l.pPr.indFirst = -360;
    }
    return a;
}

void linkToHeadings(AbstractNum &a)
{
    for (int i = 0; i < 9; ++i) {
        a.levels[i].pStyle = QStringLiteral("Heading%1").arg(i + 1);
        a.levels[i].pPr.indLeft = 432 + 144 * i;
        a.levels[i].pPr.indFirst = -(432 + 144 * i);
    }
}

} // namespace

QVector<MultilevelPreset> multilevelPresets()
{
    QVector<MultilevelPreset> out;
    {
        AbstractNum a = multilevelBase();
        static const char *const f[] = {"decimal", "lowerLetter", "lowerRoman"};
        for (int i = 0; i < 9; ++i) {
            a.levels[i].numFmt = QString::fromLatin1(f[i % 3]);
            a.levels[i].text = QStringLiteral("%%1)").arg(i + 1);
        }
        out.push_back({QStringLiteral("1)a)i)"), QCoreApplication::translate("jp::write::Lists", "1) a) i)"), a});
    }
    {
        AbstractNum a = multilevelBase();
        for (int i = 0; i < 9; ++i) a.levels[i].text = chain(i, QStringLiteral("."));
        out.push_back({QStringLiteral("1.1.1."), QCoreApplication::translate("jp::write::Lists", "1. 1.1. 1.1.1."), a});
    }
    {
        AbstractNum a = multilevelBase();
        static const char *const f[] = {"upperRoman", "upperLetter", "decimal", "lowerLetter"};
        for (int i = 0; i < 9; ++i) {
            a.levels[i].numFmt = QString::fromLatin1(f[i % 4]);
            a.levels[i].text = QStringLiteral("%") + QString::number(i + 1) + (i % 4 == 3 ? QStringLiteral(")") : QStringLiteral("."));
        }
        out.push_back({QStringLiteral("I.A.1."), QCoreApplication::translate("jp::write::Lists", "I. A. 1. a)"), a});
    }
    {
        AbstractNum a = multilevelBase();
        static const char *const t[] = {"•", "○", "■"};
        for (int i = 0; i < 9; ++i) {
            a.levels[i].numFmt = QStringLiteral("bullet");
            a.levels[i].text = QString::fromUtf8(t[i % 3]);
        }
        out.push_back({QStringLiteral("bullets"), QCoreApplication::translate("jp::write::Lists", "Bullets, in three shapes"), a});
    }
    {
        AbstractNum a = multilevelBase();
        for (int i = 0; i < 9; ++i) a.levels[i].text = chain(i, QString());
        linkToHeadings(a);
        out.push_back({QStringLiteral("headings"), QCoreApplication::translate("jp::write::Lists", "1 Heading 1, 1.1 Heading 2"), a});
    }
    {
        AbstractNum a = multilevelBase();
        a.levels[0].numFmt = QStringLiteral("upperRoman");
        a.levels[0].text = QStringLiteral("Article %1.");
        a.levels[1].text = QStringLiteral("Section %1.%2");
        a.levels[2].numFmt = QStringLiteral("lowerLetter");
        a.levels[2].text = QStringLiteral("(%3)");
        for (int i = 3; i < 9; ++i) a.levels[i].text = QStringLiteral("%%1.").arg(i + 1);
        linkToHeadings(a);
        out.push_back({QStringLiteral("article"), QCoreApplication::translate("jp::write::Lists", "Article I. Section 1.1 (a)"), a});
    }
    return out;
}

QString definitionSignature(const AbstractNum &def)
{
    QString s = def.multiLevelType;
    for (int i = 0; i < 9; ++i) {
        const Level &l = def.levels[i];
        if (!l.present) {
            s += QStringLiteral("|-");
            continue;
        }
        const Fonts f = l.rPr.fonts.value_or(Fonts());
        s += QStringLiteral("|%1;%2;%3;%4;%5;%6;%7;%8;%9;").arg(l.start).arg(l.numFmt, l.text, l.jc, l.suff, l.pStyle).arg(int(l.legal)).arg(l.restart.value_or(-1)).arg(l.pPr.indLeft.value_or(-99999));
        s += QStringLiteral("%1;%2;%3;%4;%5;%6;%7").arg(l.pPr.indFirst.value_or(-99999)).arg(f.ascii, f.hAnsi).arg(l.rPr.size.value_or(0)).arg(l.rPr.bold.value_or(false) ? 1 : 0)
                 .arg(l.rPr.italic.value_or(false) ? 1 : 0).arg(l.rPr.color ? l.rPr.color->val : QString());
    }
    return s;
}

} // namespace jp::write

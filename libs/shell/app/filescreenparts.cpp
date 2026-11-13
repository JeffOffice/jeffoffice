#include "app/filescreenparts.h"

#include "core/appinfo.h"

#include <QCoreApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "app/plaintext.h"

namespace jp {

QString cardStyle()
{
    return QStringLiteral("#jpCard{background:%1; border:1px solid %2; border-radius:12px;}").arg(uiCard().name(), uiLine().name());
}

QLabel *heading(const QString &t, QWidget *parent, double scale)
{
    auto *l = plainLabel(new QLabel(t, parent));
    QFont f = l->font();
    f.setPointSizeF(f.pointSizeF() * scale);
    f.setWeight(QFont::DemiBold);
    l->setFont(f);
    l->setContentsMargins(0, 0, 0, 6);
    return l;
}

// A page drawn as paper: soft layered shadow and a hairline edge.
QPixmap paperPixmap(const QImage &page, const QSize &box, qreal dpr)
{
    QPixmap pm(box * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    const QSizeF avail = QSizeF(box) - QSizeF(16, 18);
    QSizeF sz = QSizeF(page.size()) / page.devicePixelRatio();
    sz.scale(avail, Qt::KeepAspectRatio);
    const QRectF r(QPointF((box.width() - sz.width()) / 2.0, (box.height() - sz.height()) / 2.0 - 2), sz);
    for (int i = 6; i >= 1; --i) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 7));
        p.drawRoundedRect(r.adjusted(-i * 0.6, -i * 0.3 + 1, i * 0.6, i * 0.9 + 1), 2 + i * 0.5, 2 + i * 0.5);
    }
    p.drawImage(r, page);
    p.setPen(QPen(QColor(0, 0, 0, 38), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(r.adjusted(0.5, 0.5, -0.5, -0.5));
    return pm;
}

// A rounded white (or dark) panel used for grouped content.
QFrame *cardFrame(QWidget *parent)
{
    auto *f = new QFrame(parent);
    f->setObjectName(QStringLiteral("jpCard"));
    f->setStyleSheet(cardStyle());
    return f;
}

QLabel *mutedLabel(const QString &t, QWidget *parent, double alpha)
{
    auto *l = new QLabel(t, parent);
    QPalette pl = l->palette();
    QColor c = uiText();
    c.setAlphaF(float(alpha));
    pl.setColor(QPalette::WindowText, c);
    l->setPalette(pl);
    return l;
}

QLabel *legalLinks(QWidget *parent)
{
    auto *l = mutedLabel(QStringLiteral("<a href=\"https://github.com/JeffOffice/jeffoffice/blob/main/ADDITIONAL-TERMS.md\">%1</a> &nbsp;·&nbsp; "
                                        "<a href=\"https://www.jeffoffice.com/privacy\">%2</a> &nbsp;·&nbsp; "
                                        "<a href=\"https://www.jeffoffice.com/trademarks\">%3</a>")
                             .arg(QCoreApplication::translate("FileScreen", "Additional terms"), QCoreApplication::translate("FileScreen", "Privacy notice"),
                                  QCoreApplication::translate("FileScreen", "Trademark policy")),
                         parent, 0.75);
    l->setObjectName(QStringLiteral("legalLinks"));
    l->setTextFormat(Qt::RichText);
    l->setOpenExternalLinks(true);
    return l;
}

QLabel *sectionLabel(const QString &t, QWidget *parent)
{
    auto *l = new QLabel(t.toUpper(), parent);
    QFont f = l->font();
    f.setPointSizeF(f.pointSizeF() * 0.78);
    f.setWeight(QFont::DemiBold);
    f.setLetterSpacing(QFont::PercentageSpacing, 108);
    l->setFont(f);
    QPalette pl = l->palette();
    QColor c = uiText();
    c.setAlphaF(0.55f);
    pl.setColor(QPalette::WindowText, c);
    l->setPalette(pl);
    l->setContentsMargins(24, 14, 12, 4);
    return l;
}

namespace {
// Keeps a word-wrapped label as tall as its text at the width it gets. The
// About cards are capped at 820 pixels, but Qt works out a card's height for
// the page's full width, where text wraps less: in a wide window the cards
// came out short and cut off their last lines.
class FitHeight : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Resize)
            if (auto *l = qobject_cast<QLabel *>(o)) {
                const int h = l->heightForWidth(l->width());
                if (h > 0 && h != l->minimumHeight()) l->setMinimumHeight(h);
            }
        return false;
    }
};
} // namespace

void fitHeight(QLabel *l) { l->installEventFilter(new FitHeight(l)); }

// One app of the suite: a rounded tile in the app's color holding its two
// letters, its name in bold, what it makes, and where it stands.
// The columns are the widths given, so the rows line up.
static QWidget *suiteRow(QWidget *parent, const QString &name, const QString &letters, const QColor &color, const QString &what, const QString &state,
                         bool muted, const int (&widths)[3], const QString &accessible)
{
    auto *row = new QWidget(parent);
    row->setAccessibleName(accessible);
    auto *h = new QHBoxLayout(row);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(12);
    auto *tile = new QLabel(row);
    const qreal dpr = parent->devicePixelRatioF();
    QPixmap pm(QSize(28, 28) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawRoundedRect(QRectF(0, 0, 28, 28), 28 * 0.24, 28 * 0.24);
    QFont tf = parent->font();
    tf.setWeight(QFont::DemiBold);
    tf.setPixelSize(12);
    p.setFont(tf);
    p.setPen(Qt::white);
    p.drawText(QRectF(0, 0, 28, 28), Qt::AlignCenter, letters);
    p.end();
    tile->setPixmap(pm);
    h->addWidget(tile);
    auto *nameLabel = plainLabel(new QLabel(name, row));
    QFont nf = nameLabel->font();
    nf.setBold(true);
    nameLabel->setFont(nf);
    nameLabel->setFixedWidth(widths[0]);
    h->addWidget(nameLabel);
    auto *whatLabel = plainLabel(new QLabel(what, row));
    // A narrow window clips this column, not the state; the stretches give it
    // all the room it can use before the spare goes to the end of the row.
    whatLabel->setMaximumWidth(widths[1]);
    whatLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    h->addWidget(whatLabel, 1000);
    QLabel *stateLabel = muted ? mutedLabel(state, row) : plainLabel(new QLabel(state, row));
    stateLabel->setFixedWidth(widths[2]);
    h->addWidget(stateLabel);
    h->addStretch(1);
    return row;
}

QPixmap appBadge(const QWidget *on, int size, int pixelSize, qreal radius)
{
    const qreal dpr = on->devicePixelRatioF();
    QPixmap pm(QSize(size, size) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(uiAccent());
    p.drawRoundedRect(QRectF(0, 0, size, size), radius, radius);
    QFont bf = on->font();
    bf.setBold(true);
    bf.setPixelSize(pixelSize);
    p.setFont(bf);
    p.setPen(Qt::white);
    p.drawText(QRectF(0, 0, size, size), Qt::AlignCenter, appInfo().badge);
    p.end();
    return pm;
}

QWidget *suiteCard(QWidget *parent, const QWidget *screen)
{
    auto *suite = cardFrame(parent);
    suite->setMaximumWidth(820);
    auto *suv = new QVBoxLayout(suite);
    suv->setContentsMargins(20, 16, 20, 16);
    suv->setSpacing(10);
    auto *logo = plainLabel(new QLabel(suite));
    {
        const qreal dpr = screen->devicePixelRatioF();
        const QImage lockup(uiDark() ? QStringLiteral(":/brand/jeffoffice-lockup-on-dark.png") : QStringLiteral(":/brand/jeffoffice-lockup.png"));
        QPixmap pm = QPixmap::fromImage(lockup.scaledToHeight(qRound(40 * dpr), Qt::SmoothTransformation));
        pm.setDevicePixelRatio(dpr);
        logo->setPixmap(pm);
        logo->setAccessibleName(QStringLiteral("JeffOffice"));
    }
    suv->addWidget(logo);
    auto *suh = new QLabel(QCoreApplication::translate("jp::FileScreen", "JeffOffice apps"), suite);
    QFont suf = suh->font();
    suf.setWeight(QFont::DemiBold);
    suh->setFont(suf);
    suv->addWidget(suh);
    // The app running says so; one that is out says it is available, or a preview.
    enum Stage { Soon, Preview, Out };
    struct App { const char *name, *letters, *color; QString what; Stage stage; };
    const App apps[] = {
        {"JeffPub", "JP", "#9E1F63", QCoreApplication::translate("jp::FileScreen", "Publications"), Out},
        {"JeffDeck", "JD", "#B6420F", QCoreApplication::translate("jp::FileScreen", "Presentations"), Out},
        {"JeffWrite", "JW", "#1D55C4", QCoreApplication::translate("jp::FileScreen", "Documents"), Preview},
        {"JeffCrunch", "JC", "#13773A", QCoreApplication::translate("jp::FileScreen", "Spreadsheets"), Preview},
        {"JeffMail", "JM", "#0A6C8C", QCoreApplication::translate("jp::FileScreen", "Email and calendar"), Soon},
        {"JeffBase", "JB", "#5E2CA5", QCoreApplication::translate("jp::FileScreen", "Databases"), Soon},
    };
    const QString inUse = QCoreApplication::translate("jp::FileScreen", "You're using it"), soon = QCoreApplication::translate("jp::FileScreen", "Coming soon"),
                  available = QCoreApplication::translate("jp::FileScreen", "Available"), preview = QCoreApplication::translate("jp::FileScreen", "Preview");
    auto here = [](const App &a) { return QLatin1String(a.name) == appInfo().name; };
    auto stateOf = [&](const App &a) { return here(a) ? inUse : a.stage == Out ? available : a.stage == Preview ? preview : soon; };
    auto line = [](const QString &what) { return QStringLiteral("— %1").arg(what); };
    QFont bold = screen->font();
    bold.setBold(true);
    const QFontMetrics plain(screen->font()), strong(bold);
    int widths[3] = {0, 0, 0};
    for (const App &a : apps) {
        widths[0] = qMax(widths[0], strong.horizontalAdvance(QLatin1String(a.name)));
        widths[1] = qMax(widths[1], plain.horizontalAdvance(line(a.what)));
        widths[2] = qMax(widths[2], plain.horizontalAdvance(stateOf(a)));
    }
    widths[0] += 4;
    widths[1] += 16;   // and room before the state
    widths[2] += 4;
    for (const App &a : apps) {
        const QString state = stateOf(a);
        suv->addWidget(suiteRow(suite, QLatin1String(a.name), QLatin1String(a.letters), QColor(QLatin1String(a.color)), line(a.what), state,
                                !here(a), widths, QCoreApplication::translate("jp::FileScreen", "%1, %2, %3").arg(QLatin1String(a.name), a.what, state)));
    }
    return suite;
}

void addThirdPartyList(QBoxLayout *card, QWidget *parent, const QFont &headingFont, const QList<ThirdParty> &extras)
{
    // (The descriptions keep JeffPub's translation context, where they were first written and translated: all four
    // apps load the same translation files, so each language's wording carries over.)
    QList<ThirdParty> parts = {
        {QStringLiteral("Qt 6"), QStringLiteral("https://www.qt.io/"), QCoreApplication::translate("jp::Backstage", "Interface, text layout, PDF and printing"), QStringLiteral("LGPL-3.0"), QStringLiteral("https://www.gnu.org/licenses/lgpl-3.0.html")},
        {QStringLiteral("Hunspell"), QStringLiteral("https://hunspell.github.io/"), QCoreApplication::translate("jp::Backstage", "Spelling checker"), QStringLiteral("MPL-1.1, GPL-2.0+ or LGPL-2.1+"), QStringLiteral("https://www.mozilla.org/MPL/1.1/")},
        // The dictionaries, from LibreOffice's (resources/dict/README.md).
        {QStringLiteral("English dictionaries (SCOWL; UK)"), QStringLiteral("http://wordlist.aspell.net/"), QCoreApplication::translate("jp::Backstage", "Spelling"), QStringLiteral("SCOWL license; LGPL (UK)"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/en/README_en_US.txt")},
        {QStringLiteral("English hyphenation patterns"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/en/README_hyph_en_US.txt"), QCoreApplication::translate("jp::Backstage", "Hyphenation"), QStringLiteral("BSD-style"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/en/README_hyph_en_US.txt")},
        {QStringLiteral("WordNet"), QStringLiteral("https://wordnet.princeton.edu/"), QCoreApplication::translate("jp::Backstage", "Thesaurus"), QStringLiteral("WordNet license"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/en/WordNet_license.txt")},
        {QStringLiteral("Spanish dictionaries"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/es/README_hunspell_es.txt"), QCoreApplication::translate("jp::Backstage", "Spelling and hyphenation"), QStringLiteral("GPL-3.0+, LGPL-3.0+ or MPL-1.1+"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/es/LICENSE.md")},
        {QStringLiteral("French dictionary (Grammalecte)"), QStringLiteral("https://grammalecte.net/"), QCoreApplication::translate("jp::Backstage", "Spelling and hyphenation"), QStringLiteral("MPL-2.0; patterns LGPL"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/fr_FR/README_dict_fr.txt")},
        {QStringLiteral("German dictionary (igerman98)"), QStringLiteral("https://www.j3e.de/ispell/igerman98/"), QCoreApplication::translate("jp::Backstage", "Spelling and hyphenation"), QStringLiteral("GPL-2.0 or GPL-3.0; patterns LGPL-2.0+"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/de/README_de_DE_frami.txt")},
        {QStringLiteral("Italian dictionary"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/it_IT/README_it_IT.txt"), QCoreApplication::translate("jp::Backstage", "Spelling and hyphenation"), QStringLiteral("GPL-3.0; patterns LGPL"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/it_IT/README_it_IT.txt")},
        {QStringLiteral("Dutch dictionary (OpenTaal)"), QStringLiteral("https://www.opentaal.org/"), QCoreApplication::translate("jp::Backstage", "Spelling and hyphenation"), QStringLiteral("BSD or CC BY 3.0"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/nl_NL/LICENSE.txt")},
        {QStringLiteral("Brazilian Portuguese dictionary (VERO)"), QStringLiteral("http://pt-br.libreoffice.org/projetos/projeto-vero-verificador-ortografico/"), QCoreApplication::translate("jp::Backstage", "Spelling and hyphenation"),
         QStringLiteral("LGPL-3.0 or MPL"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/pt_BR/README_en.txt")},
        {QStringLiteral("European Portuguese dictionary"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/pt_PT/README_pt_PT.txt"), QCoreApplication::translate("jp::Backstage", "Spelling and hyphenation"), QStringLiteral("GPL-2.0+"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/pt_PT/LICENSES.txt")},
        {QStringLiteral("Russian dictionary"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/ru_RU/README_ru_RU.txt"), QCoreApplication::translate("jp::Backstage", "Spelling and hyphenation"), QStringLiteral("BSD-style; patterns LPPL-1.2+"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/ru_RU/README_hyph_ru_RU.txt")},
        {QStringLiteral("Indonesian dictionary (hunspell-id)"), QStringLiteral("https://github.com/shuLhan/hunspell-id"), QCoreApplication::translate("jp::Backstage", "Spelling and hyphenation"), QStringLiteral("LGPL-3.0"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/id/LICENSE-dict")},
        {QStringLiteral("Ukrainian dictionary (dict_uk)"), QStringLiteral("https://github.com/brown-uk/dict_uk"), QCoreApplication::translate("jp::Backstage", "Spelling and hyphenation"), QStringLiteral("MPL-1.1; patterns GPL-2.0+"),
         QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/dict/uk_UA/README_uk_UA.txt")},
        {QStringLiteral("Lucide icons"), QStringLiteral("https://lucide.dev/"), QCoreApplication::translate("jp::Backstage", "Interface icons, Insert > Icons"), QStringLiteral("ISC"), QStringLiteral("https://lucide.dev/license")},
        {QStringLiteral("Open fonts"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/fonts/README.md"), QCoreApplication::translate("jp::Backstage", "Fonts and substitutes"),
         QStringLiteral("SIL OFL 1.1, Apache 2.0, GUST, Bitstream Vera, GPL v2 + font exceptions"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/resources/fonts/README.md")},
        {QStringLiteral("zlib"), QStringLiteral("https://zlib.net/"), QCoreApplication::translate("jp::Backstage", "Compression (also in PDFium)"), QStringLiteral("zlib license"), QStringLiteral("https://zlib.net/zlib_license.html")},
        // PDFium and what's built into it; the texts ship with the program
        // (third_party/pdfium/licenses).
        {QStringLiteral("PDFium"), QStringLiteral("https://pdfium.googlesource.com/pdfium/"), QCoreApplication::translate("jp::Backstage", "Placing PDF pages"), QStringLiteral("BSD-3-Clause; parts Apache-2.0"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/pdfium.txt")},
        // qpdf writes a PDF again with a password on it; libjpeg-turbo is what it reads JPEG pictures with (the texts ship with the program: third_party/qpdf/licenses, third_party/libjpeg_turbo/licenses).
        {QStringLiteral("qpdf"), QStringLiteral("https://qpdf.org/"), QCoreApplication::translate("jp::Backstage", "Passwords in PDF files"), QStringLiteral("Apache-2.0"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/qpdf/licenses/qpdf-Apache-2.0.txt")},
        {QStringLiteral("libjpeg-turbo (in qpdf)"), QStringLiteral("https://libjpeg-turbo.org/"), QCoreApplication::translate("jp::Backstage", "JPEG pictures in PDF files"), QStringLiteral("IJG and BSD-3-Clause"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/libjpeg_turbo/licenses/libjpeg-turbo.md")},
        {QStringLiteral("pdfium-binaries"), QStringLiteral("https://github.com/bblanchon/pdfium-binaries"), QCoreApplication::translate("jp::Backstage", "PDFium, built"), QStringLiteral("MIT"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/pdfium-binaries.txt")},
        {QStringLiteral("FreeType (in PDFium)"), QStringLiteral("https://freetype.org/"), QCoreApplication::translate("jp::Backstage", "Fonts in PDF pages"), QStringLiteral("FreeType License"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/freetype.txt")},
        {QStringLiteral("HarfBuzz (in PDFium)"), QStringLiteral("https://harfbuzz.github.io/"), QCoreApplication::translate("jp::Backstage", "Text in PDF pages"), QStringLiteral("Old MIT"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/harfbuzz.txt")},
        {QStringLiteral("Little CMS (in PDFium)"), QStringLiteral("https://www.littlecms.com/"), QCoreApplication::translate("jp::Backstage", "Color in PDF pages"), QStringLiteral("MIT"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/lcms.txt")},
        {QStringLiteral("libjpeg-turbo (in PDFium)"), QStringLiteral("https://libjpeg-turbo.org/"), QCoreApplication::translate("jp::Backstage", "JPEG pictures in PDF pages"), QStringLiteral("IJG and BSD-3-Clause"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/libjpeg_turbo.md")},
        {QStringLiteral("OpenJPEG (in PDFium)"), QStringLiteral("https://www.openjpeg.org/"), QCoreApplication::translate("jp::Backstage", "JPEG 2000 in PDF pages"), QStringLiteral("BSD-2-Clause"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/libopenjpeg.txt")},
        {QStringLiteral("libpng (in PDFium)"), QStringLiteral("http://www.libpng.org/pub/png/libpng.html"), QCoreApplication::translate("jp::Backstage", "PNG pictures in PDF pages"), QStringLiteral("libpng License v2"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/libpng.txt")},
        {QStringLiteral("Abseil (in PDFium)"), QStringLiteral("https://abseil.io/"), QCoreApplication::translate("jp::Backstage", "Support code"), QStringLiteral("Apache-2.0"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/abseil.txt")},
        {QStringLiteral("Anti-Grain Geometry 2.3 (in PDFium)"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/agg23.txt"), QCoreApplication::translate("jp::Backstage", "Drawing"), QStringLiteral("AGG 2.3 license"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/agg23.txt")},
        {QStringLiteral("dragonbox (in PDFium)"), QStringLiteral("https://github.com/jk-jeon/dragonbox"), QCoreApplication::translate("jp::Backstage", "Numbers"), QStringLiteral("Apache-2.0 (LLVM) or BSL-1.0"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/dragonbox-Boost.txt")},
        {QStringLiteral("fast_float (in PDFium)"), QStringLiteral("https://github.com/fastfloat/fast_float"), QCoreApplication::translate("jp::Backstage", "Numbers"), QStringLiteral("MIT"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/fast_float.txt")},
        {QStringLiteral("simdutf (in PDFium)"), QStringLiteral("https://github.com/simdutf/simdutf"), QCoreApplication::translate("jp::Backstage", "Unicode text"), QStringLiteral("MIT"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/simdutf.txt")},
        {QStringLiteral("Highway (in PDFium)"), QStringLiteral("https://github.com/google/highway"), QCoreApplication::translate("jp::Backstage", "Support code"), QStringLiteral("Apache-2.0"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/highway.txt")},
        {QStringLiteral("LLVM libc (in PDFium)"), QStringLiteral("https://libc.llvm.org/"), QCoreApplication::translate("jp::Backstage", "Support code"), QStringLiteral("Apache-2.0 (LLVM)"), QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/pdfium/licenses/llvm-libc.txt")},
        {QStringLiteral("ICU"), QStringLiteral("https://icu.unicode.org/"), QCoreApplication::translate("jp::Backstage", "Unicode text (in PDFium; with Qt on Linux)"), QStringLiteral("Unicode License v3"), QStringLiteral("https://www.unicode.org/license.txt")},
#ifdef Q_OS_WIN
        {QStringLiteral("MinGW-w64 runtime"), QStringLiteral("https://www.mingw-w64.org/"), QCoreApplication::translate("jp::Backstage", "C++ runtime"), QStringLiteral("GPL-3.0 with GCC Runtime Library Exception; MIT"),
         QStringLiteral("https://www.gnu.org/licenses/gcc-exception-3.1.html")},
        {QStringLiteral("Inno Setup"), QStringLiteral("https://jrsoftware.org/isinfo.php"), QCoreApplication::translate("jp::Backstage", "Installer"), QStringLiteral("Inno Setup License"), QStringLiteral("https://jrsoftware.org/files/is/license.txt")},
#endif
    };
    parts += extras;
    QString rows;
    for (const ThirdParty &pt : parts)
        rows += QStringLiteral("<tr><td style=\"padding:3px 14px 3px 0\"><a href=\"%1\">%2</a></td><td style=\"padding:3px 14px 3px 0\">%3</td>"
                               "<td style=\"padding:3px 0\"><a href=\"%4\">%5</a></td></tr>")
                    .arg(pt.url, pt.name.toHtmlEscaped(), pt.use.toHtmlEscaped(), pt.licenseUrl, pt.license.toHtmlEscaped());
    auto *th = new QLabel(QCoreApplication::translate("jp::Backstage", "Third-party software"), parent);
    th->setFont(headingFont);
    card->addSpacing(6);
    card->addWidget(th);
    auto *table = mutedLabel(QStringLiteral("<table cellspacing=\"0\">%1</table>").arg(rows), parent, 0.75);
    table->setObjectName(QStringLiteral("thirdParty"));
    table->setTextFormat(Qt::RichText);
    // Wrapped at the card's width: unwrapped, a narrow window cut off the long names and the last rows.
    table->setWordWrap(true);
    fitHeight(table);
    table->setOpenExternalLinks(true);
    card->addWidget(table);
}

} // namespace jp

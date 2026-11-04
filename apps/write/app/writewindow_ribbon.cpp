// JeffWrite's ribbon (its layout is resources/ribbon.json, built by ribbonbuilder.cpp): the controls and menus the file names, and the
// status bar.

#include "app/writewindow.h"

#include "app/icons.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/widgets.h"
#include "core/editops.h"
#include "core/wordformat.h"
#include "text/textprops.h"

#include <QAction>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QSlider>
#include <QStatusBar>
#include <QToolButton>
#include <cmath>
#include "app/plaintext.h"

namespace jp {

namespace {

// The picture of a style in the gallery: the look of its text, and its name.
QIcon styleIcon(write::WriteDoc *doc, const write::StyleEntry &e, const QSize &size, qreal dpr)
{
    QPixmap pm(size * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    const QRectF box = QRectF(QPointF(0, 0), QSizeF(size)).adjusted(1, 1, -1, -1);
    p.setPen(QPen(QColor(0, 0, 0, 40), 1));
    p.setBrush(Qt::white);
    p.drawRoundedRect(box, 4, 4);
    const QString paraStyle = e.character ? doc->defaultParagraphStyle() : e.id;
    const QTextCharFormat cf = doc->styleCharFormat(paraStyle, e.character ? e.id : QString());
    const FontScheme &fonts = doc->flow().env.fonts;
    QString family = cf.hasProperty(QTextFormat::FontFamilies) && !cf.fontFamilies().toStringList().isEmpty()
                         ? cf.fontFamilies().toStringList().first()
                         : (cf.stringProperty(tp::ThemeFont) == QLatin1String("major") ? fonts.heading : fonts.body);
    QFont f(family);
    f.setPixelSize(std::clamp(int(std::lround(cf.fontPointSize() * 1.1)) + 3, 11, 20));
    f.setBold(cf.fontWeight() >= QFont::DemiBold);
    f.setItalic(cf.fontItalic());
    f.setUnderline(cf.underlineStyle() != QTextCharFormat::NoUnderline);
    p.setFont(f);
    QColor ink = cf.hasProperty(QTextFormat::ForegroundBrush) ? cf.foreground().color() : QColor(0, 0, 0);
    p.setPen(ink);
    p.drawText(QRectF(box.left() + 4, box.top() + 2, box.width() - 8, box.height() * 0.6), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("AaBbCc"));
    QFont small;
    small.setPixelSize(10);
    p.setFont(small);
    p.setPen(QColor(0x55, 0x55, 0x55));
    p.drawText(QRectF(box.left() + 4, box.top() + box.height() * 0.62, box.width() - 8, box.height() * 0.34), Qt::AlignLeft | Qt::AlignVCenter,
               QFontMetrics(small).elidedText(e.name, Qt::ElideRight, int(box.width()) - 8));
    return QIcon(pm);
}

QIcon colorSquare(const QColor &c)
{
    QPixmap pm(16, 16);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setPen(QColor(0, 0, 0, 90));
    p.setBrush(c);
    p.drawRect(1, 1, 13, 13);
    return QIcon(pm);
}

} // namespace

void WriteWindow::buildRibbon()
{
    RibbonParts parts;
    parts.action = [this](const QString &id) { return m_actions.value(id); };
    ribbonParts(parts);
    parts.colors[QStringLiteral("headerFooter")] = QColor(0x2E, 0x8B, 0x57);
    parts.colors[QStringLiteral("table")] = QColor(0x9B, 0x59, 0xB6);
    QFile f(QStringLiteral(":/write/ribbon.json"));
    QByteArray json;
    if (f.open(QIODevice::ReadOnly)) json = f.readAll();
    parts.launchers[QStringLiteral("font")] = [this] { act("fmt.fontDialog")->trigger(); };
    parts.launchers[QStringLiteral("paragraph")] = [this] { act("para.dialog")->trigger(); };
    parts.launchers[QStringLiteral("styles")] = [this] { act("style.pane")->trigger(); };
    parts.launchers[QStringLiteral("pageSetup")] = [this] { act("layout.pageSetup")->trigger(); };
    parts.launchers[QStringLiteral("notes")] = [this] { act("refs.notesOptions")->trigger(); };
    jp::buildRibbon(m_ribbon, json, parts, this, &m_ribbonErrors);
    // The Copy Formatting button is double-clicked to keep it on, and Esc in the text turns it off (see eventFilter).
    for (QToolButton *b : m_ribbon->findChildren<QToolButton *>())
        if (b->defaultAction() && b->defaultAction() == act(QStringLiteral("edit.formatPainter"))) b->installEventFilter(this);
    m_view->installEventFilter(this);
}

void WriteWindow::ribbonParts(RibbonParts &parts)
{
    tableRibbonParts(parts);
    pictureRibbonParts(parts);
    insertRibbonParts(parts);
    layoutRibbonParts(parts);
    mailRibbonParts(parts);
    parts.widgets[QStringLiteral("fontCombo")] = [this]() -> QWidget * {
        auto *font = new FontCombo();
        connect(font, &FontCombo::familyChosen, this, [this](const QString &f) {
            if (refuseIfReadOnly()) return;
            if (m_doc) write::setFontFamily(*m_doc, m_view->cursor(), f);
            m_view->setFocus();
        });
        m_fontCombos << font;
        return font;
    };
    parts.widgets[QStringLiteral("sizeCombo")] = [this]() -> QWidget * {
        auto *size = new SizeCombo();
        connect(size, &SizeCombo::sizeChosen, this, [this](double s) {
            if (refuseIfReadOnly()) return;
            if (m_doc) write::setFontSize(*m_doc, m_view->cursor(), s);
            m_view->setFocus();
        });
        m_sizeCombos << size;
        return size;
    };
    parts.widgets[QStringLiteral("textColor")] = [this]() -> QWidget * {
        auto *b = new ColorButton(QStringLiteral("baseline"), tr("Font Color"), true, tr("Automatic"));
        b->setCurrent(ColorRef::rgb(QColor(0xC0, 0, 0)));
        connect(b, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
            if (!m_doc || refuseIfReadOnly()) return;
            write::setTextColor(*m_doc, m_view->cursor(), c.isNone() ? QColor() : c.resolve(m_doc->flow().env.colors));
            m_view->setFocus();
        });
        m_colorButtons << b;
        return b;
    };
    parts.menus[QStringLiteral("highlightColors")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        for (const QString &name : write::highlightNames()) {
            QAction *a = m->addAction(colorSquare(write::highlightColor(name)), name);
            connect(a, &QAction::triggered, this, [this, name] {
                if (refuseIfReadOnly()) return;
                if (m_doc) write::setHighlight(*m_doc, m_view->cursor(), name);
                m_view->setFocus();
            });
        }
        m->addSeparator();
        connect(m->addAction(icon(QStringLiteral("circle-slash")), tr("No Color")), &QAction::triggered, this, [this] {
            if (refuseIfReadOnly()) return;
            if (m_doc) write::setHighlight(*m_doc, m_view->cursor(), QString());
            m_view->setFocus();
        });
        return m;
    };
    parts.menus[QStringLiteral("paperSizes")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        for (const char *id : {"letter", "legal", "executive", "a3", "a4", "a5", "b5"}) m->addAction(act(QStringLiteral("layout.size.") + QLatin1String(id)));
        m->addSeparator();
        m->addAction(act("layout.moreSizes"));
        return m;
    };
    parts.widgets[QStringLiteral("styleGallery")] = [this]() -> QWidget * {
        auto *g = new Gallery(QSize(70, 46), 5);
        g->setTitle(tr("Styles"));
        g->setItemsProvider([this]() {
            QVector<GalleryItem> items;
            if (!m_doc) return items;
            const qreal dpr = devicePixelRatioF();
            for (const write::StyleEntry &e : m_doc->styleEntries()) {
                if (!e.quick) continue;
                items << GalleryItem{(e.character ? QStringLiteral("c:") : QStringLiteral("p:")) + e.id, e.name, styleIcon(m_doc.get(), e, QSize(70, 46), dpr), QString()};
            }
            return items;
        });
        g->reload();
        connect(g, &Gallery::activated, this, [this](const QString &id) {
            if (!m_doc || refuseIfReadOnly()) return;
            if (id.startsWith(QLatin1String("c:"))) write::applyCharacterStyle(*m_doc, m_view->cursor(), id.mid(2));
            else write::applyParagraphStyle(*m_doc, m_view->cursor(), id.mid(2));
            m_view->setFocus();
        });
        m_styleGallery = g;
        return g;
    };
    // The indents and spacing of Layout > Paragraph: in the person's units, a field each.
    for (int which = 0; which < 4; ++which) {
        static const char *const names[] = {"indentLeft", "indentRight", "spaceBefore", "spaceAfter"};
        parts.widgets[QString::fromLatin1(names[which])] = [this, which]() -> QWidget * {
            auto *spin = new MeasureSpin();
            spin->setObjectName(QStringLiteral("paragraphField%1").arg(which));
            spin->setAccessibleName(which == 0 ? tr("Indent left") : which == 1 ? tr("Indent right") : which == 2 ? tr("Spacing before") : tr("Spacing after"));
            m_indentFields << spin;
            connect(spin, &QDoubleSpinBox::editingFinished, this, [this, spin, which] {
                if (m_updatingFields || !m_doc || refuseIfReadOnly()) return;
                const double v = spin->value();
                write::changeBlockFormat(*m_doc, m_view->cursor(), [which, v](QTextBlockFormat &bf) {
                    if (which == 0) bf.setLeftMargin(v);
                    else if (which == 1) bf.setRightMargin(v);
                    else if (which == 2) write::setSpaceBefore(bf, v);
                    else write::setSpaceAfter(bf, v);
                });
                m_view->setFocus();
            });
            return spin;
        };
    }
}

// "Page 2 of 5" and the words, then at the right the view, and the zoom.
void WriteWindow::buildStatusBar()
{
    QStatusBar *sb = statusBar();
    m_pageLabel = plainLabel(new QLabel(this));
    m_pageLabel->setObjectName(QStringLiteral("pageNumber"));
    m_pageLabel->setMinimumWidth(110);
    sb->addWidget(m_pageLabel);
    // The section the caret is in, shown where the document has more than one.
    m_sectionLabel = plainLabel(new QLabel(this));
    m_sectionLabel->setObjectName(QStringLiteral("sectionNumber"));
    m_sectionLabel->setToolTip(tr("The section the caret is in"));
    m_sectionLabel->hide();
    sb->addWidget(m_sectionLabel);
    m_wordLabel = new QLabel(this);
    m_wordLabel->setObjectName(QStringLiteral("wordCount"));
    m_wordLabel->setTextFormat(Qt::PlainText);
    m_wordLabel->setMinimumWidth(90);
    sb->addWidget(m_wordLabel);
    // Track Changes, on or off, which a click turns the other way.
    m_trackLabel = new QToolButton();
    m_trackLabel->setObjectName(QStringLiteral("trackStatus"));
    m_trackLabel->setAutoRaise(true);
    m_trackLabel->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_trackLabel->setText(tr("Track Changes: Off"));
    m_trackLabel->setToolTip(tr("Track Changes (Ctrl+Shift+E)"));
    m_trackLabel->setAccessibleName(tr("Track Changes"));
    connect(m_trackLabel, &QToolButton::clicked, this, [this] { act(QStringLiteral("review.trackChanges"))->trigger(); });
    sb->addWidget(m_trackLabel);
    // The language the text at the caret is checked in, which a click changes.
    m_langLabel = new QToolButton();
    m_langLabel->setObjectName(QStringLiteral("languageStatus"));
    m_langLabel->setAutoRaise(true);
    m_langLabel->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_langLabel->setToolTip(tr("Proofing language"));
    m_langLabel->setAccessibleName(tr("Proofing language"));
    connect(m_langLabel, &QToolButton::clicked, this, [this] { act(QStringLiteral("review.language"))->trigger(); });
    sb->addWidget(m_langLabel);

    auto button = [&](const char *id) {
        auto *b = new QToolButton();
        b->setDefaultAction(act(QString::fromLatin1(id)));
        b->setAutoRaise(true);
        b->setIconSize(QSize(16, 16));
        b->setToolButtonStyle(Qt::ToolButtonIconOnly);
        sb->addPermanentWidget(b);
        return b;
    };
    button("view.printLayout");
    button("zoom.out");
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setObjectName(QStringLiteral("zoomSlider"));
    m_zoomSlider->setRange(-47, 33);   // 1.05^n: 10% .. 500%
    m_zoomSlider->setFixedWidth(140);
    m_zoomSlider->setAccessibleName(tr("Zoom"));
    m_zoomSlider->setAccessibleDescription(tr("Left and Right arrows zoom out and in."));
    connect(m_zoomSlider, &QSlider::valueChanged, this, [this](int v) { m_view->setZoom(std::pow(1.05, v)); });
    sb->addPermanentWidget(m_zoomSlider);
    button("zoom.in");
    m_zoomLabel = plainLabel(new QLabel(QStringLiteral("100%")));
    m_zoomLabel->setMinimumWidth(44);
    m_zoomLabel->setObjectName(QStringLiteral("zoomPercent"));
    sb->addPermanentWidget(m_zoomLabel);
}

} // namespace jp

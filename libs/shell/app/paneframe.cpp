#include "app/paneframe.h"

#include "app/icons.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <algorithm>
#include "app/plaintext.h"

namespace jp {

namespace {

// The heading of a section: a chevron, the title, and a line under it. It is a button, so the keyboard reaches it.
class SectionHeader : public QToolButton {
public:
    explicit SectionHeader(QWidget *parent) : QToolButton(parent)
    {
        setCheckable(true);
        setFocusPolicy(Qt::StrongFocus);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    QSize sizeHint() const override { return QSize(160, std::max(28, fontMetrics().height() + 12)); }
    QSize minimumSizeHint() const override { return QSize(60, sizeHint().height()); }

protected:
    void keyPressEvent(QKeyEvent *e) override
    {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            click();
            return;
        }
        QToolButton::keyPressEvent(e);
    }
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect());
        if (underMouse() || isDown()) p.fillRect(r, QColor(uiAccent().red(), uiAccent().green(), uiAccent().blue(), 24));
        p.setPen(QPen(uiLine(), 1));
        p.drawLine(QPointF(r.left(), r.bottom() - 0.5), QPointF(r.right(), r.bottom() - 0.5));
        // The chevron points right while the section is folded, down while it is open.
        const QPointF c(r.left() + 14, r.center().y());
        p.setPen(QPen(uiText(), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        if (isChecked()) p.drawPolyline(QPolygonF({c + QPointF(-4, -2), c + QPointF(0, 2), c + QPointF(4, -2)}));
        else p.drawPolyline(QPolygonF({c + QPointF(-2, -4), c + QPointF(2, 0), c + QPointF(-2, 4)}));
        QFont f = font();
        f.setBold(true);
        p.setFont(f);
        const QRectF text(r.left() + 28, r.top(), r.width() - 34, r.height());
        p.drawText(text, Qt::AlignVCenter | Qt::AlignLeft, QFontMetricsF(f).elidedText(this->text(), Qt::ElideRight, text.width()));
        if (hasFocus()) {
            p.setPen(QPen(focusRingColor(), 2));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(r.adjusted(1, 1, -1, -1), 4, 4);
        }
    }
};

// A tab of the frame: only its icon, with a line under it while it is the one open.
class TabButton : public QToolButton {
public:
    explicit TabButton(QWidget *parent) : QToolButton(parent)
    {
        setCheckable(true);
        setAutoExclusive(true);
        setFocusPolicy(Qt::StrongFocus);
        setToolButtonStyle(Qt::ToolButtonIconOnly);
        setIconSize(QSize(20, 20));
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    QSize sizeHint() const override { return QSize(48, 36); }
    QSize minimumSizeHint() const override { return QSize(32, 36); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect());
        if (underMouse() && !isChecked()) p.fillRect(r, QColor(uiAccent().red(), uiAccent().green(), uiAccent().blue(), 24));
        const QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
        icon().paint(&p, QRect(int(r.center().x()) - 10, int(r.center().y()) - 11, 20, 20), Qt::AlignCenter, mode, isChecked() ? QIcon::On : QIcon::Off);
        if (isChecked()) {
            p.setPen(QPen(uiAccent(), 2.5, Qt::SolidLine, Qt::FlatCap));
            p.drawLine(QPointF(r.left() + 6, r.bottom() - 1.5), QPointF(r.right() - 6, r.bottom() - 1.5));
        }
        if (hasFocus()) {
            p.setPen(QPen(focusRingColor(), 2));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(r.adjusted(2, 2, -2, -2), 4, 4);
        }
    }
};

} // namespace

// ---------------- PaneSection ----------------

PaneSection::PaneSection(const QString &title, const QString &id, QWidget *parent) : QWidget(parent), m_title(title)
{
    if (!id.isEmpty()) setObjectName(QStringLiteral("section:") + id);
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);
    m_header = new SectionHeader(this);
    m_header->setText(title);
    m_header->setObjectName(QStringLiteral("section:") + title);
    m_header->setChecked(true);
    v->addWidget(m_header);
    m_body = new QWidget(this);
    m_bodyLayout = new QVBoxLayout(m_body);
    m_bodyLayout->setContentsMargins(12, 8, 12, 12);
    m_bodyLayout->setSpacing(8);
    v->addWidget(m_body);
    connect(m_header, &QToolButton::toggled, this, [this](bool on) { setExpanded(on); });
    refreshHeader();
}

void PaneSection::setExpanded(bool on)
{
    if (m_expanded == on && m_header->isChecked() == on) return;
    m_expanded = on;
    {
        const QSignalBlocker block(m_header);
        m_header->setChecked(on);
    }
    m_body->setVisible(on);
    refreshHeader();
    Q_EMIT toggled(on);
}

void PaneSection::refreshHeader()
{
    m_header->setAccessibleName(m_title);
    m_header->setAccessibleDescription(m_expanded ? tr("Expanded. Press Space to fold it up.") : tr("Folded. Press Space to open it."));
    m_header->setToolTip(jp::plainToolTip(m_expanded ? tr("Fold up %1").arg(m_title) : tr("Open %1").arg(m_title)));
    m_header->update();
}

// ---------------- PanePage ----------------

PanePage::PanePage(QWidget *parent) : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addStretch(1);
}

PaneSection *PanePage::addSection(const QString &title, bool expanded, const QString &id)
{
    auto *s = new PaneSection(title, id, this);
    m_layout->insertWidget(m_layout->count() - 1, s);
    m_sections << s;
    s->setExpanded(expanded);
    return s;
}

PaneSection *PanePage::sectionById(const QString &id) const
{
    for (PaneSection *s : m_sections)
        if (s->objectName() == QLatin1String("section:") + id) return s;
    return nullptr;
}

PaneSection *PanePage::section(const QString &title) const
{
    for (PaneSection *s : m_sections)
        if (s->title() == title) return s;
    return nullptr;
}

// ---------------- PaneFrame ----------------

PaneFrame::PaneFrame(QWidget *parent) : QFrame(parent)
{
    setObjectName(QStringLiteral("paneFrame"));
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);

    auto *head = new QWidget(this);
    auto *hl = new QHBoxLayout(head);
    hl->setContentsMargins(12, 8, 6, 4);
    m_title = new QLabel(head);
    m_title->setObjectName(QStringLiteral("paneTitle"));
    QFont f = m_title->font();
    f.setBold(true);
    f.setPointSizeF(f.pointSizeF() * 1.1);
    m_title->setFont(f);
    m_title->setTextFormat(Qt::PlainText);
    hl->addWidget(m_title, 1);
    m_close = new QToolButton(head);
    m_close->setObjectName(QStringLiteral("paneClose"));
    m_close->setIcon(icon(QStringLiteral("x")));
    m_close->setAutoRaise(true);
    m_close->setToolTip(tr("Close"));
    m_close->setAccessibleName(tr("Close"));
    m_close->setFocusPolicy(Qt::StrongFocus);
    hl->addWidget(m_close);
    v->addWidget(head);

    m_tabRow = new QWidget(this);
    m_tabRow->setObjectName(QStringLiteral("paneTabs"));
    new QHBoxLayout(m_tabRow);
    m_tabRow->layout()->setContentsMargins(6, 0, 6, 0);
    m_tabRow->layout()->setSpacing(0);
    v->addWidget(m_tabRow);

    m_stack = new QStackedWidget(this);
    v->addWidget(m_stack, 1);
    m_message = new QLabel(this);
    m_message->setObjectName(QStringLiteral("paneMessage"));
    m_message->setWordWrap(true);
    m_message->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_message->setMargin(14);
    m_message->setTextFormat(Qt::PlainText);
    m_message->hide();
    v->addWidget(m_message, 1);

    connect(m_close, &QToolButton::clicked, this, &PaneFrame::closeRequested);
    setMinimumWidth(280);
    setAccessibleName(tr("Format pane"));
}

QString PaneFrame::title() const { return m_title->text(); }

void PaneFrame::setTitle(const QString &title)
{
    m_title->setText(title);
    setAccessibleName(title);
}

PanePage *PaneFrame::addPage(const QString &id, const QIcon &icon, const QString &name)
{
    auto *page = new PanePage();
    auto *scroll = new QScrollArea(m_stack);
    scroll->setObjectName(QStringLiteral("panePage:") + id);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFocusPolicy(Qt::NoFocus);   // the keyboard goes to what is in the page, not to the scrolled area
    scroll->viewport()->setFocusPolicy(Qt::NoFocus);
    scroll->setWidget(page);
    m_stack->addWidget(scroll);

    auto *tab = new TabButton(m_tabRow);
    tab->setObjectName(QStringLiteral("paneTab:") + id);
    tab->setIcon(icon);
    tab->setToolTip(jp::plainToolTip(name));
    tab->setAccessibleName(name);
    tab->setAccessibleDescription(tr("Tab"));
    m_tabRow->layout()->addWidget(tab);
    // The close button, then the tabs, then what is in the pages.
    setTabOrder(m_tabs.isEmpty() ? static_cast<QWidget *>(m_close) : static_cast<QWidget *>(m_tabs.last()), tab);
    const int index = int(m_tabs.size());
    m_tabs << tab;
    m_ids << id;
    m_scrolls << scroll;
    m_pages << page;
    connect(tab, &QToolButton::clicked, this, [this, index] { setCurrentPage(index); });
    if (m_current < 0) setCurrentPage(index);
    return page;
}

PanePage *PaneFrame::page(int i) const { return m_pages.value(i); }

QString PaneFrame::pageId(int i) const { return m_ids.value(i); }

int PaneFrame::indexOfPage(const QString &id) const { return int(m_ids.indexOf(id)); }

void PaneFrame::setCurrentPage(int i)
{
    if (i < 0 || i >= m_tabs.size() || !isPageVisible(i)) return;
    const bool changed = i != m_current;
    m_current = i;
    m_stack->setCurrentWidget(m_scrolls[i]);
    m_tabs[i]->setChecked(true);
    if (changed) Q_EMIT currentPageChanged(i);
}

bool PaneFrame::isPageVisible(int i) const { return i >= 0 && i < m_tabs.size() && !m_tabs[i]->isHidden(); }

void PaneFrame::setPageVisible(int i, bool visible)
{
    if (i < 0 || i >= m_tabs.size()) return;
    m_tabs[i]->setVisible(visible);
    if (!visible && m_current == i) {
        for (int k = 0; k < m_tabs.size(); ++k)
            if (isPageVisible(k)) {
                setCurrentPage(k);
                return;
            }
    }
    if (visible && (m_current < 0 || !isPageVisible(m_current))) setCurrentPage(i);
}

void PaneFrame::setMessage(const QString &text)
{
    m_message->setText(text);
    const bool shown = !text.isEmpty();
    m_message->setVisible(shown);
    m_tabRow->setVisible(!shown);
    m_stack->setVisible(!shown);
}

QString PaneFrame::message() const { return m_message->text(); }

void PaneFrame::focusTabs()
{
    if (m_current >= 0 && m_current < m_tabs.size() && m_tabs[m_current]->isVisible()) m_tabs[m_current]->setFocus(Qt::TabFocusReason);
    else m_close->setFocus(Qt::TabFocusReason);
}

void PaneFrame::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        Q_EMIT escaped();
        e->accept();
        return;
    }
    QFrame::keyPressEvent(e);
}

} // namespace jp

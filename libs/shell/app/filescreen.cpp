#include "app/filescreen.h"
#include "app/filescreenparts.h"
#include "app/theme.h"

#include "core/appinfo.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace jp {

FileScreen::FileScreen(QWidget *parent) : QWidget(parent)
{
    setAutoFillBackground(true);
    auto *h = new QHBoxLayout(this);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);

    // Sidebar: a quiet panel with the wordmark, a back button and grouped commands.
    auto *side = new QFrame(this);
    m_side = side;
    side->setObjectName(QStringLiteral("jpSide"));
    side->setFixedWidth(236);
    auto *sv = new QVBoxLayout(side);
    m_sideLayout = sv;
    sv->setContentsMargins(0, 18, 0, 14);
    sv->setSpacing(0);

    auto *brand = new QWidget(side);
    auto *bh = new QHBoxLayout(brand);
    bh->setContentsMargins(22, 0, 12, 0);
    bh->setSpacing(10);
    auto *badge = new QLabel(brand);
    badge->setPixmap(appBadge(this, 30, 13, 8));
    auto *word = new QLabel(appInfo().name, brand);
    QFont wf = word->font();
    wf.setPointSizeF(wf.pointSizeF() * 1.35);
    wf.setWeight(QFont::DemiBold);
    word->setFont(wf);
    bh->addWidget(badge);
    bh->addWidget(word);
    bh->addStretch(1);
    sv->addWidget(brand);
    sv->addSpacing(14);

    m_back = new NavItem(QStringLiteral("arrow-left"), QString(), side);
    m_back->setCheckable(false);
    connect(m_back, &QAbstractButton::clicked, this, &FileScreen::closeRequested);
    sv->addWidget(m_back);
    m_next = sv->count();

    m_navGroup = new QButtonGroup(this);
    m_navGroup->setExclusive(true);
    sv->addStretch(1);
    m_version = new QLabel(tr("Version %1").arg(QStringLiteral(JP_VERSION)), side);
    m_version->setContentsMargins(24, 0, 12, 0);
    sv->addWidget(m_version);
    h->addWidget(side);

    m_stack = new QStackedWidget(this);
    h->addWidget(m_stack, 1);
    restyle();
    // The look switched in Options: the colors set here, and the page shown.
    connect(UiTheme::instance(), &UiTheme::changed, this, [this] {
        restyle();
        if (isVisible())
            for (auto it = m_index.cbegin(); it != m_index.cend(); ++it)
                if (it.value() == m_stack->currentIndex()) rebuild(it.key());
    });
}

void FileScreen::setBackButton(const QString &text, const QString &toolTip)
{
    m_back->setText(text);
    m_back->setToolTip(toolTip);
}

void FileScreen::insertEntry(QWidget *w) { m_sideLayout->insertWidget(m_next++, w); }

void FileScreen::addSection(const QString &title) { insertEntry(sectionLabel(title, m_side)); }

void FileScreen::addHiddenPage(const QString &key)
{
    if (!m_index.contains(key)) m_index[key] = m_stack->addWidget(new QWidget());
}

void FileScreen::addPage(const QString &key, const QString &icon, const QString &label)
{
    addHiddenPage(key);
    auto *item = new NavItem(icon, label, m_side);
    m_navGroup->addButton(item);
    m_navItems.insert(key, item);
    insertEntry(item);
    connect(item, &QAbstractButton::clicked, this, [this, key] { showPage(key); });
}

void FileScreen::addCommand(const QString &key, const QString &icon, const QString &label, const std::function<void()> &run)
{
    auto *item = new NavItem(icon, label, m_side);
    item->setCheckable(false);
    m_navItems.insert(key, item);
    insertEntry(item);
    connect(item, &QAbstractButton::clicked, this, [run] { run(); });
}

void FileScreen::restyle()
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, uiWindow());
    setPalette(pal);
    m_side->setStyleSheet(QStringLiteral("#jpSide{background:%1; border-right:1px solid %2;}").arg(uiPanel().name(), uiLine().name()));
    QPalette vp = m_version->palette();
    QColor vc = uiText();
    vc.setAlphaF(0.5f);
    vp.setColor(QPalette::WindowText, vc);
    m_version->setPalette(vp);
}

void FileScreen::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) { Q_EMIT closeRequested(); return; }
    QWidget::keyPressEvent(e);
}

void FileScreen::rebuild(const QString &key)
{
    QWidget *w = m_builder ? m_builder(key) : nullptr;
    if (!w) return;
    auto *scroll = new QScrollArea();
    scroll->setWidget(w);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    const int idx = m_index.value(key, -1);
    if (idx < 0) return;
    QWidget *old = m_stack->widget(idx);
    m_stack->insertWidget(idx, scroll);
    m_stack->removeWidget(old);
    old->deleteLater();
}

QString FileScreen::currentPage() const { return m_index.key(m_stack->currentIndex()); }

void FileScreen::showPage(const QString &key)
{
    rebuild(key);
    m_stack->setCurrentIndex(m_index.value(key, 0));
    if (QAbstractButton *b = m_navItems.value(key)) b->setChecked(true);
    else if (QAbstractButton *on = m_navGroup->checkedButton()) {   // a page with no item of its own: none is lit
        m_navGroup->setExclusive(false);
        on->setChecked(false);
        m_navGroup->setExclusive(true);
    }
}

} // namespace jp

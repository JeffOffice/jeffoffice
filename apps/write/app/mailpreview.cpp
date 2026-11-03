// The pane that previews a mail merge (app/mailpreview.h).

#include "app/mailpreview.h"

#include "app/pageview.h"
#include "core/writedoc.h"

#include <QCoreApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

namespace jp::write {

namespace {

// Keeps the preview from being edited: what the keyboard and the mouse would do to the text is not done (scrolling is).
class ReadOnly : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *, QEvent *e) override
    {
        switch (e->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::ContextMenu:
        case QEvent::InputMethod:
        case QEvent::Drop:
        case QEvent::DragEnter:
        case QEvent::DragMove:
            return true;
        default: return false;
        }
    }
};

} // namespace

MailPreviewPane::MailPreviewPane(const std::function<std::unique_ptr<WriteDoc>(int row)> &merge, QWidget *parent) : QWidget(parent), m_merge(merge)
{
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(4, 4, 4, 4);
    auto *bar = new QHBoxLayout();
    auto button = [&](const QString &text, const QString &tip, void (MailPreviewPane::*slot)()) {
        auto *b = new QToolButton(this);
        b->setText(text);
        b->setToolTip(tip);
        b->setAutoRaise(true);
        connect(b, &QToolButton::clicked, this, slot);
        bar->addWidget(b);
        return b;
    };
    button(QStringLiteral("|<"), tr("First Record"), &MailPreviewPane::first);
    button(QStringLiteral("<"), tr("Previous Record"), &MailPreviewPane::previous);
    m_spin = new QSpinBox(this);
    m_spin->setRange(1, 1);
    m_spin->setToolTip(tr("Go to Record"));
    bar->addWidget(m_spin);
    button(QStringLiteral(">"), tr("Next Record"), &MailPreviewPane::next);
    button(QStringLiteral(">|"), tr("Last Record"), &MailPreviewPane::last);
    m_label = new QLabel(this);
    bar->addWidget(m_label);
    bar->addStretch(1);
    v->addLayout(bar);
    m_view = new PageView(this);
    m_view->setFocusPolicy(Qt::NoFocus);
    m_view->setAcceptDrops(false);
    auto *blocker = new ReadOnly(this);
    m_view->viewport()->installEventFilter(blocker);
    m_view->installEventFilter(blocker);
    v->addWidget(m_view, 1);
    connect(m_spin, &QSpinBox::valueChanged, this, [this](int n) {
        if (!m_changing) setPlace(n - 1);
    });
}

MailPreviewPane::~MailPreviewPane()
{
    m_view->setDocument(nullptr);
}

void MailPreviewPane::setRows(const QVector<int> &rows, int total)
{
    m_rows = rows;
    m_total = total;
    m_place = std::clamp(m_place, 0, std::max(0, int(rows.size()) - 1));
    refresh();
}

void MailPreviewPane::setPlace(int place)
{
    place = std::clamp(place, 0, std::max(0, int(m_rows.size()) - 1));
    if (place == m_place && m_doc) return;
    m_place = place;
    refresh();
    Q_EMIT placeChanged(m_place);
}

void MailPreviewPane::refresh()
{
    m_changing = true;
    m_spin->setRange(1, std::max(1, int(m_rows.size())));
    m_spin->setValue(m_place + 1);
    m_label->setText(tr("of %1").arg(m_rows.size()));
    m_changing = false;
    m_view->setDocument(nullptr);
    m_doc.reset();
    if (m_rows.isEmpty() || !m_merge) return;
    m_doc = m_merge(m_rows[m_place]);
    m_view->setDocument(m_doc.get());
    m_view->setZoomMode(PageView::ZoomMode::PageWidth);
    m_view->viewport()->update();
}

} // namespace jp::write

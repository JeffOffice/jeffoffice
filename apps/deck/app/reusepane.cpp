#include "app/reusepane.h"

#include "app/icons.h"
#include "app/settings.h"
#include "app/theme.h"
#include "core/reuse.h"
#include "io/presentationfile.h"
#include "render/renderer.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

namespace jp {

namespace {
constexpr int kThumbWidth = 200;
}

ReusePane::ReusePane(QWidget *parent) : QWidget(parent), m_cache(std::make_unique<LayoutCache>())
{
    setObjectName(QStringLiteral("reusePane"));
    setAccessibleName(tr("Reuse Slides"));
    auto *column = new QVBoxLayout(this);
    column->setContentsMargins(8, 8, 8, 8);
    auto *browse = new QPushButton(icon("folder-open"), tr("Browse…"), this);
    browse->setObjectName(QStringLiteral("reuseBrowse"));
    browse->setToolTip(tr("Choose a presentation to take slides from"));
    connect(browse, &QPushButton::clicked, this, &ReusePane::browse);
    m_status = new QLabel(tr("Browse for a presentation to take slides from."), this);
    m_status->setObjectName(QStringLiteral("reuseStatus"));
    m_status->setWordWrap(true);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("reuseSlides"));
    m_list->setAccessibleName(tr("Slides of the presentation"));
    m_list->setViewMode(QListView::IconMode);
    m_list->setFlow(QListView::TopToBottom);
    m_list->setWrapping(false);
    m_list->setResizeMode(QListView::Adjust);
    m_list->setMovement(QListView::Static);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setSpacing(8);
    m_list->setWordWrap(true);
    m_list->setTextElideMode(Qt::ElideRight);
    m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->installEventFilter(this);
    m_keep = new QCheckBox(tr("Keep source formatting"), this);
    m_keep->setObjectName(QStringLiteral("keepSourceFormatting"));
    m_keep->setToolTip(tr("Bring the slides' layouts and slide masters along, as they look in their presentation"));
    column->addWidget(browse);
    column->addWidget(m_status);
    column->addWidget(m_list, 1);
    column->addWidget(m_keep);
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) { choose(m_list->row(item)); });
    m_timer.setSingleShot(true);
    m_timer.setInterval(0);
    connect(&m_timer, &QTimer::timeout, this, &ReusePane::renderNext);
    setMinimumWidth(kThumbWidth + 56);
}

ReusePane::~ReusePane() = default;

int ReusePane::slideCount() const { return m_doc ? int(m_doc->pages.size()) : 0; }

bool ReusePane::keepFormatting() const { return m_keep->isChecked(); }
void ReusePane::setKeepFormatting(bool keep) { m_keep->setChecked(keep); }

void ReusePane::browse()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Reuse Slides"), Settings::get().value("dirs/open", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                      tr("Presentations (*.pptx *.potx *.ppsx *.ppt *.pps *.pot)") + QStringLiteral(";;") + tr("All Files (*)"));
    if (path.isEmpty()) return;
    Settings::get().setValue("dirs/open", QFileInfo(path).absolutePath());
    open(path);
}

bool ReusePane::open(const QString &path)
{
    m_timer.stop();
    m_list->clear();
    m_doc.reset();
    m_cache = std::make_unique<LayoutCache>();
    m_next = m_rendered = 0;
    QString why;
    m_doc = deck::importAnyPresentationFile(path, &why);
    if (!m_doc) {
        m_error = why.isEmpty() ? tr("The file can't be read.") : why;
        m_status->setText(tr("%1 can't be read: %2").arg(QFileInfo(path).fileName(), m_error));
        return false;
    }
    m_error.clear();
    m_status->setText(QFileInfo(path).fileName());
    m_status->setToolTip(QDir::toNativeSeparators(path));
    const QSizeF page = m_doc->pageSize();
    const QSize thumb(kThumbWidth, std::max(1, int(std::lround(kThumbWidth * page.height() / std::max(1.0, page.width())))));
    m_list->setIconSize(thumb);
    m_list->setGridSize(QSize(thumb.width() + 24, thumb.height() + 44));
    QPixmap blank(thumb);
    blank.fill(Qt::white);
    for (int i = 0; i < m_doc->pages.size(); ++i) {
        const QString title = deck::slideTitle(*m_doc, i);
        auto *item = new QListWidgetItem(QIcon(blank), title.isEmpty() ? QString::number(i + 1) : QStringLiteral("%1  %2").arg(i + 1).arg(title), m_list);
        item->setToolTip(title);
        item->setData(Qt::AccessibleTextRole, title.isEmpty() ? tr("Slide %1").arg(i + 1) : tr("Slide %1, %2").arg(i + 1).arg(title));
    }
    if (m_list->count()) m_list->setCurrentRow(0);
    m_timer.start();
    return true;
}

// Draws the next thumbnail, then the one after when idle.
void ReusePane::renderNext()
{
    if (!m_doc || m_next >= m_doc->pages.size()) return;
    PaintContext ctx;
    ctx.doc = m_doc.get();
    ctx.cache = m_cache.get();
    const double dpr = devicePixelRatioF();
    QImage img = Renderer::renderToImage(ctx, m_next, kThumbWidth * dpr / std::max(1.0, m_doc->pageSize().width()));
    img.setDevicePixelRatio(dpr);
    if (QListWidgetItem *item = m_list->item(m_next)) item->setIcon(QIcon(QPixmap::fromImage(img)));
    ++m_next;
    ++m_rendered;
    m_timer.start();
}

void ReusePane::choose(int row)
{
    if (m_doc && row >= 0 && row < m_doc->pages.size()) Q_EMIT slideChosen(row, keepFormatting());
}

// Enter takes the slide that is current (a click does so by itself).
bool ReusePane::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_list && e->type() == QEvent::KeyPress) {
        const int key = static_cast<QKeyEvent *>(e)->key();
        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            choose(m_list->currentRow());
            return true;
        }
    }
    return QWidget::eventFilter(o, e);
}

} // namespace jp

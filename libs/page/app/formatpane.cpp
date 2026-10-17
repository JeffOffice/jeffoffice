#include "app/formatpane.h"

#include "app/editor.h"
#include "app/filedialogs.h"
#include "app/formatpane_p.h"
#include "app/icons.h"
#include "app/paneframe.h"
#include "app/settings.h"

#include <QFileInfo>
#include <QShowEvent>
#include <QStandardPaths>
#include <QVBoxLayout>

namespace jp {

// ---------------- FormatSession ----------------

FormatSession::FormatSession(Editor *ed, QObject *parent) : QObject(parent), m_ed(ed) {}

QVector<Item *> FormatSession::items() const { return m_ed->selectedItems(); }

QVector<Item *> FormatSession::leaves() const
{
    QVector<Item *> out;
    std::function<void(Item *)> add = [&](Item *it) {
        if (it->type() == ItemType::Group) {
            for (const auto &c : static_cast<GroupItem *>(it)->children) add(c.get());
        } else if (!out.contains(it)) {
            out << it;
        }
    };
    for (Item *it : items()) add(it);
    return out;
}

void FormatSession::apply(const QString &label, const std::function<void(Item &)> &change, Scope scope, bool last, bool includeLocked)
{
    const QVector<Item *> list = scope == Scope::Leaves ? leaves() : items();
    if (list.isEmpty()) return;
    if (!m_open) {
        m_ed->beginChange(label);
        m_open = true;
    }
    for (Item *it : list)
        if (includeLocked || !it->locked) change(*it);
    m_ed->notifyLive();
    if (last) finish();
}

void FormatSession::finish()
{
    if (!m_open) return;
    m_open = false;
    m_ed->endChange();
}

void FormatSession::refresh()
{
    for (const auto &fn : std::as_const(m_refresh)) fn();
}

// ---------------- FormatPane ----------------

FormatPane::FormatPane(Editor *ed, QWidget *parent) : QWidget(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("formatPane"));
    m_session = new FormatSession(ed, this);
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    m_frame = new PaneFrame(this);
    v->addWidget(m_frame);
    m_frame->setTitle(tr("Format Shape"));
    m_frame->setAccessibleName(tr("Format pane"));

    PanePage *fill = m_frame->addPage(QStringLiteral("fill"), icon(QStringLiteral("paint-bucket")), tr("Fill & Line"));
    PanePage *effects = m_frame->addPage(QStringLiteral("effects"), icon(QStringLiteral("sparkles")), tr("Effects"));
    PanePage *size = m_frame->addPage(QStringLiteral("size"), icon(QStringLiteral("scaling")), tr("Size & Properties"));
    PanePage *picture = m_frame->addPage(QStringLiteral("picture"), icon(QStringLiteral("image")), tr("Picture"));
    format::buildPages(this, fill, effects, size, picture);

    connect(m_frame, &PaneFrame::closeRequested, this, [this] {
        m_session->finish();
        Q_EMIT closeRequested();
    });
    connect(m_frame, &PaneFrame::escaped, this, &FormatPane::escaped);
    // Following the selection: a step still open ends first, with the selection it was made on.
    auto leave = [this] {
        m_session->finish();
        follow();
    };
    connect(ed, &Editor::selectionChanged, this, leave);
    connect(ed, &Editor::pageChanged, this, leave);
    connect(ed, &Editor::documentReplaced, this, leave);
    connect(ed, &Editor::changed, this, [this] { follow(); });
    reload();
}

FormatPane::~FormatPane() = default;

QString FormatPane::title() const { return m_frame->title(); }

FormatPane::Tab FormatPane::currentTab() const { return Tab(std::max(0, m_frame->currentPage())); }

bool FormatPane::isTabShown(Tab tab) const { return m_frame->isPageVisible(int(tab)); }

void FormatPane::showTab(Tab tab)
{
    if (isTabShown(tab)) m_frame->setCurrentPage(int(tab));
}

QString FormatPane::choosePicture()
{
    if (m_pictureChooser) return m_pictureChooser();
    const QString dir = Settings::get().value(QStringLiteral("dirs/pictures"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
    const QString filter = tr("All Pictures (*.png *.jpg *.jpeg *.gif *.bmp *.tif *.tiff *.webp *.svg *.wmf *.emf)") + QStringLiteral(";;") + tr("All Files (*)");
    const QStringList paths = askPicturePaths(window(), tr("Insert Picture"), dir, filter, false, nullptr);
    if (paths.isEmpty()) return QString();
    Settings::get().setValue(QStringLiteral("dirs/pictures"), QFileInfo(paths.first()).absolutePath());
    return paths.first();
}

void FormatPane::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    reload();
}

// A change the pane doesn't need to show while it is closed; it brings itself up to date as it opens.
void FormatPane::follow()
{
    if (isHidden()) return;
    reload();
}

void FormatPane::reload()
{
    const QVector<Item *> sel = m_session->items();
    if (sel.isEmpty()) {
        m_frame->setMessage(tr("Select a shape, text box, line, or picture to format it."));
        return;
    }
    m_frame->setMessage(QString());
    bool pictures = true, corrected = true;
    for (const Item *it : sel) {
        pictures = pictures && it->type() == ItemType::Picture;
        // (A movie's or a sound's picture is its poster, and has a tab of its own in the app.)
        corrected = corrected && it->type() == ItemType::Picture && static_cast<const PictureItem *>(it)->mediaKind.isEmpty();
    }
    m_frame->setTitle(pictures ? tr("Format Picture") : tr("Format Shape"));
    const bool fill = format::anyTakes(m_session->leaves(), [](const Item &it) { return format::takesFill(it) || format::takesLine(it); });
    m_frame->setPageVisible(int(Tab::FillLine), fill);
    m_frame->setPageVisible(int(Tab::Picture), corrected);
    m_session->refresh();
}

} // namespace jp

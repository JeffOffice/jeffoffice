// JeffDeck's Insert tab: Pictures from this device or online, Icons, and Text
// Art. Each puts its object in the middle of the slide, as one step that undoes.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "app/iconpicker.h"
#include "app/icons.h"
#include "app/onlinepictures.h"
#include "app/ribbonbuilder.h"
#include "app/textartgallery.h"
#include "app/textdialogs.h"
#include "app/widgets.h"
#include "canvas/canvas.h"

#include <QAction>
#include <QDockWidget>
#include <QVBoxLayout>

namespace jp {

void DeckWindow::createPictureActions()
{
    mk("insert.pictureDevice", tr("This Device…"), "monitor", QKeySequence(), [this] { insertPictureFromFile(); });
    mk("insert.onlinePicture", tr("Online Pictures…"), "globe", QKeySequence(), [this] { showOnlinePictures(); });
    mk("insert.icons", tr("Icons"), "smile-plus", QKeySequence(), [this] { insertIcons(pickIcons(this)); });
    // Text Art asks for its words again on a double click, or on Enter with it selected.
    connect(m_canvas, &Canvas::editTextArtWanted, this, [this](const QString &id) { textArtTextDialog(this, m_ed, id); });
}

// Insert > Text > Text Art: the gallery of styles.
void DeckWindow::addInsertParts(RibbonParts &parts)
{
    parts.widgets[QStringLiteral("textArtButton")] = [this]() -> QWidget * {
        auto *art = new GalleryButton(icon("type"), tr("Text Art"), QSize(64, 40), 6, true);
        art->setToolTip(tr("Text Art"));
        art->setItemsProvider([this] { return textArtGalleryItems(m_ed); });
        connect(art, &GalleryButton::activated, this, [this](const QString &styleId) { insertTextArt(styleId); });
        return art;
    };
}

void DeckWindow::showOnlinePictures()
{
    if (!m_online) {
        m_onlineDock = new QDockWidget(tr("Online Pictures"), this);
        m_onlineDock->setObjectName(QStringLiteral("onlinePicturesDock"));
        m_onlineDock->setFeatures(QDockWidget::DockWidgetClosable);
        // The pane puts a picture it downloaded on the slide, with the picture's title for alt text and, as asked, its credit.
        m_online = new OnlinePicturesPane([this](const QString &path, const online::Picture &picture, bool withCredit) {
            return insertPicture(path, QPointF(-1, -1), picture.title, withCredit ? online::credit(picture) : QString());
        });
        // With the margins JeffDeck's other panes have.
        auto *frame = new QWidget(m_onlineDock);
        auto *column = new QVBoxLayout(frame);
        column->setContentsMargins(8, 8, 8, 8);
        column->addWidget(m_online);
        m_onlineDock->setWidget(frame);
        addDockWidget(Qt::RightDockWidgetArea, m_onlineDock);
        resizeDocks({m_onlineDock}, {300}, Qt::Horizontal);
    }
    m_onlineDock->show();
    m_onlineDock->raise();
}

void DeckWindow::insertIcons(const QStringList &names)
{
    Document *d = m_ed->doc();
    const QSizeF ps = d->pageSize();
    const QColor color = ColorRef::scheme(Main).resolve(d->colors);
    // The icons there are, each with its picture.
    QStringList shown;
    QVector<QByteArray> svgs;
    for (const QString &name : names)
        if (const QByteArray svg = iconSvg(name, color); !svg.isEmpty()) {
            shown << QString(name).replace(QLatin1Char('-'), QLatin1Char(' '));
            svgs << svg;
        }
    if (svgs.isEmpty()) return;
    // An inch square each, in rows across the middle of the slide.
    const double size = 72, step = size * 1.25;
    const int perRow = std::max(1, int(ps.width() * 0.8 / step)), n = int(svgs.size());
    const int rows = (n + perRow - 1) / perRow;
    m_ed->endTextEdit();
    m_ed->beginChange(n == 1 ? tr("Insert Icon") : tr("Insert Icons"));
    QStringList made;
    for (int i = 0; i < n; ++i) {
        const int row = i / perRow, inRow = std::min(perRow, n - row * perRow);
        const QPointF c(ps.width() / 2 + (i % perRow - (inRow - 1) / 2.0) * step, ps.height() / 2 + (row - (rows - 1) / 2.0) * step);
        auto pic = std::make_shared<PictureItem>();
        pic->rect = QRectF(c.x() - size / 2, c.y() - size / 2, size, size);
        pic->imgRect = QRectF(0, 0, size, size);
        pic->imageId = d->addImage(svgs[i], QStringLiteral("svg"));
        if (pic->imageId.isEmpty()) continue;
        pic->name = shown[i] + QStringLiteral(" icon");
        pic->altText = shown[i];
        m_ed->surfaceItems().push_back(pic);
        made << pic->id;
    }
    m_ed->endChange();
    m_ed->select(made);
}

void DeckWindow::insertTextArt(const QString &styleId)
{
    auto art = newTextArt(styleId);
    // Four times as wide as it is high, half the width of the slide.
    const QSizeF ps = m_ed->doc()->pageSize();
    const double width = ps.width() * 0.5, height = width / 4;
    art->rect = QRectF((ps.width() - width) / 2, (ps.height() - height) / 2, width, height);
    m_ed->endTextEdit();
    m_ed->addItem(art);
    textArtTextDialog(this, m_ed, art->id);
}

} // namespace jp

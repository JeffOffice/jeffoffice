// JeffWrite's Graphics Format tab: what is done to an SVG picture (a graphic). Change Graphic puts another SVG in its frame, Convert to Shape makes shapes of the drawing's parts, and the
// graphic's fills and lines are put in other colors (the work on the document is in core/pictureops.h).

#include "app/writewindow.h"

#include "app/filedialogs.h"
#include "app/iconpicker.h"
#include "app/pageview.h"
#include "app/picturetools.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/widgets.h"
#include "core/pictureops.h"

#include <QAction>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QMenu>
#include <QStandardPaths>
#include <QStatusBar>

namespace jp {

namespace {

// The color as the graphic holds it: RRGGBB-style, with none where the choice is no color.
write::GraphicColors colorsOf(bool fill, const ColorRef &color, const ColorScheme &scheme)
{
    write::GraphicColors out;
    write::GraphicColors::Mode &mode = fill ? out.fill : out.outline;
    QColor &value = fill ? out.fillColor : out.outlineColor;
    if (color.isNone()) {
        mode = write::GraphicColors::None;
    } else {
        mode = write::GraphicColors::Color;
        value = color.resolve(scheme);
    }
    return out;
}

} // namespace

void WriteWindow::createGraphicsActions()
{
    mk("graphics.change.file", tr("From a File…"), "image-plus", QKeySequence(), [this] { changeSelectedGraphic(false); });
    mk("graphics.change.icons", tr("From Icons…"), "smile-plus", QKeySequence(), [this] { changeSelectedGraphic(true); });
    mk("graphics.convert", tr("Convert to Shape"), "shapes", QKeySequence(), [this] { convertSelectedGraphic(); });
}

void WriteWindow::graphicsRibbonParts(RibbonParts &parts)
{
    parts.menus[QStringLiteral("graphicsChange")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        m->addAction(act("graphics.change.file"));
        m->addAction(act("graphics.change.icons"));
        return m;
    };
    parts.menus[QStringLiteral("graphicsEffects")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        fillPictureEffectsMenu(m, [this](const QString &, const EffectChange &change) {
            m_view->changeSelectedPicture([change](write::WriteDoc &d, int position) { return write::setPictureEffects(d, position, change); });
            m_view->setFocus(Qt::OtherFocusReason);
        });
        return m;
    };
    parts.widgets[QStringLiteral("graphicsStyles")] = [this]() -> QWidget * {
        auto *g = new Gallery(QSize(60, 46), 6);
        g->setObjectName(QStringLiteral("graphicsStyles"));
        g->setTitle(tr("Graphic Styles"));
        g->setItemsProvider([this] { return pictureStyleItems(m_doc ? m_doc->flow().env.colors : builtinColorSchemes().value(0), QSize(60, 46), m_graphicSample); });
        connect(g, &Gallery::activated, this, [this](const QString &id) {
            const int i = id.toInt();
            if (i < 0 || i >= pictureStyleCount()) return;
            m_view->changeSelectedPicture([i](write::WriteDoc &d, int position) { return write::setPictureStyle(d, position, i); });
            m_view->setFocus(Qt::OtherFocusReason);
        });
        m_graphicStyles = g;
        return g;
    };
    parts.widgets[QStringLiteral("graphicsFill")] = [this]() -> QWidget * {
        auto *b = new ColorButton(QStringLiteral("paint-bucket"), tr("Graphics Fill"), true, tr("No Fill"));
        b->setObjectName(QStringLiteral("graphicsFill"));
        b->setCurrent(ColorRef::none());
        connect(b, &ColorButton::colorPicked, this, [this](const ColorRef &c) { setSelectedGraphicColors(true, c); });
        return b;
    };
    parts.widgets[QStringLiteral("graphicsOutline")] = [this]() -> QWidget * {
        auto *b = new ColorButton(QStringLiteral("pencil-line"), tr("Graphics Outline"), true, tr("No Outline"));
        b->setObjectName(QStringLiteral("graphicsOutline"));
        b->setCurrent(ColorRef::none());
        connect(b, &ColorButton::colorPicked, this, [this](const ColorRef &c) { setSelectedGraphicColors(false, c); });
        return b;
    };
    for (int which = 0; which < 2; ++which) {
        parts.widgets[which == 0 ? QStringLiteral("graphicsHeight") : QStringLiteral("graphicsWidth")] = [this, which]() -> QWidget * {
            auto *spin = new MeasureSpin();
            spin->setObjectName(which == 0 ? QStringLiteral("graphicsHeightField") : QStringLiteral("graphicsWidthField"));
            spin->setAccessibleName(which == 0 ? tr("Graphic height") : tr("Graphic width"));
            m_graphicFields << spin;
            connect(spin, &QDoubleSpinBox::editingFinished, this, [this, spin, which] {
                const int at = m_view->selectedPicture();
                if (m_updatingFields || !m_doc || at < 0) return;
                const write::PictureInfo now = write::pictureAt(*m_doc, m_doc->story(), at);
                if (!now.valid || now.width <= 0 || now.height <= 0) return;
                double w = now.width, h = now.height;
                const double v = spin->value();
                if (which == 0) {
                    h = v;
                    if (act("picture.lockAspect")->isChecked()) w = now.width * v / now.height;
                } else {
                    w = v;
                    if (act("picture.lockAspect")->isChecked()) h = now.height * v / now.width;
                }
                if (act("picture.lockAspect")->isChecked()) write::clampKeepingRatio(&w, &h);
                if (std::abs(w - now.width) < 0.005 && std::abs(h - now.height) < 0.005) return;
                m_view->changeSelectedPicture([w, h](write::WriteDoc &d, int position) { return write::setPictureSize(d, position, w, h); });
            });
            return spin;
        };
    }
}

// What follows the selection: the tab shows with a graphic selected, and its commands work on that graphic.
void WriteWindow::refreshGraphicsUi(const write::PictureInfo &info)
{
    const bool graphic = info.valid && info.kind == write::DrawnKind::Picture && info.svg;
    m_ribbon->setContextVisible(QStringLiteral("Graphics Tools"), graphic, true);
    const bool editable = graphic && info.editable && !(m_view && m_view->readOnly());
    for (const char *id : {"graphics.change.file", "graphics.change.icons", "graphics.convert"}) act(QString::fromLatin1(id))->setEnabled(editable);
    m_updatingFields = true;
    for (int i = 0; i + 1 < m_graphicFields.size(); i += 2) {
        if (!info.valid) break;
        m_graphicFields[i]->setPoints(info.height);
        m_graphicFields[i + 1]->setPoints(info.width);
    }
    m_updatingFields = false;
    // The gallery's tiles show the graphic that is selected, in each style (made again when another graphic is selected, or this one is changed).
    if (graphic && m_graphicStyles) {
        const QByteArray svg = write::graphicSvgAt(*m_doc, m_view->selectedPicture());
        const QByteArray key = QCryptographicHash::hash(svg, QCryptographicHash::Sha1);
        if (key != m_graphicSampleKey) {
            m_graphicSampleKey = key;
            m_graphicSample = write::graphicSample(*m_doc, m_view->selectedPicture());
            m_graphicStyles->reload();
        }
    }
}

// Graphics Format > Change Graphic: another SVG, from a file or from the icons, goes in the frame of the one selected; the frame's size and place stay.
void WriteWindow::changeSelectedGraphic(bool fromIcons)
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0 || refuseIfReadOnly()) return;
    QByteArray svg;
    if (fromIcons) {
        const QStringList names = pickIcons(this);
        if (names.isEmpty()) return;
        svg = iconSvg(names.first(), ColorRef::scheme(Main).resolve(m_doc->flow().env.colors));
        if (svg.isEmpty()) {
            statusBar()->showMessage(tr("That icon can't be put here."), 6000);
            return;
        }
    } else {
        QString path;
        if (m_pictureChooser) {
            path = m_pictureChooser();
        } else {
            const QString dir = Settings::get().value(QStringLiteral("dirs/pictures"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
            const QString filter = tr("SVG graphics (*.svg)") + QStringLiteral(";;") + tr("All Files (*)");
            const QStringList paths = askPicturePaths(this, tr("Change Graphic"), dir, filter, false, nullptr);
            if (paths.isEmpty()) return;
            Settings::get().setValue(QStringLiteral("dirs/pictures"), QFileInfo(paths.first()).absolutePath());
            path = paths.first();
        }
        if (path.isEmpty()) return;
        const write::PictureResult read = write::readPictureFile(path, &svg);
        if (read != write::PictureResult::Ok) {
            statusBar()->showMessage(read == write::PictureResult::TooLarge ? tr("That graphic is too large to put in a document.") : tr("%1 can't be opened as a graphic.").arg(QFileInfo(path).fileName()), 8000);
            return;
        }
    }
    m_doc->pictureNotice.clear();
    const bool changed = m_view->changeSelectedPicture([svg](write::WriteDoc &d, int position) { return write::changeGraphic(d, position, svg) == write::PictureResult::Ok; });
    if (!changed && m_doc->pictureNotice.isEmpty()) statusBar()->showMessage(tr("That can't be used as a graphic."), 8000);
    m_view->setFocus(Qt::OtherFocusReason);
}

// Graphics Format > Convert to Shape: the graphic becomes shapes of the drawing, in its place; Undo brings the picture back.
void WriteWindow::convertSelectedGraphic()
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0 || refuseIfReadOnly()) return;
    write::GraphicConversion done;
    m_doc->pictureNotice.clear();
    const bool changed = m_view->changeSelectedPicture([&done](write::WriteDoc &d, int position) {
        done = write::convertGraphicToShapes(d, position);
        return done.result == write::GraphicConvert::Done;
    });
    if (changed && done.partial) statusBar()->showMessage(tr("Text and pictures inside this drawing can't become shapes, so they were left out. Undo brings the picture back."), 10000);
    else if (!changed && m_doc->pictureNotice.isEmpty()) statusBar()->showMessage(tr("This graphic can't be turned into shapes here."), 8000);
    m_view->setFocus(Qt::OtherFocusReason);
}

// Graphics Fill and Graphics Outline: the fills (or the lines) of the drawing are all this color, or none.
void WriteWindow::setSelectedGraphicColors(bool fill, const ColorRef &color)
{
    if (!m_doc || m_view->selectedPicture() < 0 || refuseIfReadOnly()) return;
    const write::GraphicColors colors = colorsOf(fill, color, m_doc->flow().env.colors);
    m_view->changeSelectedPicture([colors](write::WriteDoc &d, int position) { return write::setGraphicColors(d, position, colors); });
    m_view->setFocus(Qt::OtherFocusReason);
}

} // namespace jp

// JeffWrite's pictures, page setup, and ruler: Insert > Pictures, the Picture Format tab and the menu of a right click on a picture, the Page Setup dialog, and the ruler (the work on
// the document is core/pictureops.h, core/pagesetup.h, and core/rulerops.h).

#include "app/writewindow.h"

#include "app/barcodedialog.h"
#include "core/mergeops.h"
#include "app/filedialogs.h"
#include "app/iconpicker.h"
#include "app/onlinepictures.h"
#include "app/icons.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/ruler.h"
#include "app/settings.h"
#include "app/widgets.h"
#include "app/uistyle.h"
#include "app/writedialogs.h"
#include "core/pictureops.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QClipboard>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QMenu>
#include <QMimeData>
#include <QStandardPaths>
#include <QStatusBar>
#include <QVBoxLayout>

namespace jp {

namespace {

struct AspectRatio {
    int w, h;
    QString name;
};

// The shapes of Aspect Ratio: a square, then the tall ones and the wide ones.
const QVector<AspectRatio> &aspectRatios()
{
    static const QVector<AspectRatio> ratios = {
        {1, 1, WriteWindow::tr("Square 1:1")},      {2, 3, WriteWindow::tr("Portrait 2:3")},   {3, 4, WriteWindow::tr("Portrait 3:4")},    {3, 5, WriteWindow::tr("Portrait 3:5")},
        {4, 5, WriteWindow::tr("Portrait 4:5")},    {9, 16, WriteWindow::tr("Portrait 9:16")}, {3, 2, WriteWindow::tr("Landscape 3:2")},  {4, 3, WriteWindow::tr("Landscape 4:3")},
        {5, 3, WriteWindow::tr("Landscape 5:3")},   {5, 4, WriteWindow::tr("Landscape 5:4")},  {16, 9, WriteWindow::tr("Landscape 16:9")}, {16, 10, WriteWindow::tr("Landscape 16:10")}};
    return ratios;
}

}

void WriteWindow::createPictureActions()
{
    mk("insert.picture", tr("Picture…"), "image-plus", QKeySequence(), [this] { insertPictureFromFile(); });
    mk("insert.icons", tr("Icons…"), "smile-plus", QKeySequence(), [this] { insertIconsFromPicker(); });
    mk("insert.barcode", tr("Barcode…"), "barcode", QKeySequence(), [this] { insertBarcodeFromDialog(); });
    // A double click on a barcode asks for its data again.
    connect(m_view, &write::PageView::editBarcodeWanted, this, [this] { insertBarcodeFromDialog(); });
    connect(m_view, &write::PageView::editBarcodeFieldWanted, this, [this](int position) { editBarcodeField(position); });
    mk("insert.onlinePicture", tr("Online Pictures…"), "globe", QKeySequence(), [this] { showOnlinePictures(); });
    mk("insert.pictureClipboard", tr("Picture from Clipboard"), "clipboard-paste", QKeySequence(), [this] { insertPictureFromClipboard(); });
    struct Wrap {
        const char *id;
        const char *text;
        write::PictureWrap wrap;
        const char *icon;
    };
    for (const Wrap &w : {Wrap{"inline", QT_TRANSLATE_NOOP("jp::WriteWindow", "In Line with Text"), write::PictureWrap::Inline, "image"},
                          Wrap{"square", QT_TRANSLATE_NOOP("jp::WriteWindow", "Square"), write::PictureWrap::Square, "wrap-text"},
                          Wrap{"tight", QT_TRANSLATE_NOOP("jp::WriteWindow", "Tight"), write::PictureWrap::Tight, "wrap-text"},
                          Wrap{"topBottom", QT_TRANSLATE_NOOP("jp::WriteWindow", "Top and Bottom"), write::PictureWrap::TopAndBottom, "separator-horizontal"},
                          Wrap{"behind", QT_TRANSLATE_NOOP("jp::WriteWindow", "Behind Text"), write::PictureWrap::BehindText, "send-to-back"},
                          Wrap{"front", QT_TRANSLATE_NOOP("jp::WriteWindow", "In Front of Text"), write::PictureWrap::InFrontOfText, "bring-to-front"}}) {
        const write::PictureWrap wrap = w.wrap;
        mk(QStringLiteral("picture.wrap.") + QLatin1String(w.id), QCoreApplication::translate("jp::WriteWindow", w.text), w.icon, QKeySequence(), [this, wrap] {
            m_view->changeSelectedPicture([wrap](write::WriteDoc &d, int at) { return write::setPictureWrap(d, at, wrap); });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
    // Aspect Ratio, Fill, and Fit: one step each, from the shared plans (core/picturecrop.h).
    for (const auto &r : aspectRatios()) {
        const int w = r.w, h = r.h;
        mk(QStringLiteral("picture.aspect.%1x%2").arg(w).arg(h), r.name, "ratio", QKeySequence(), [this, w, h] {
            m_view->changeSelectedPicture([w, h](write::WriteDoc &d, int at) { return write::setPictureCropAspect(d, at, w, h); });
            m_view->setFocus(Qt::OtherFocusReason);
        });
    }
    mk("picture.fill", tr("Fill"), "expand", QKeySequence(), [this] {
        m_view->changeSelectedPicture([](write::WriteDoc &d, int at) { return write::setPictureCropFill(d, at, true); });
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("picture.fit", tr("Fit"), "shrink", QKeySequence(), [this] {
        m_view->changeSelectedPicture([](write::WriteDoc &d, int at) { return write::setPictureCropFill(d, at, false); });
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("picture.wrap.editPoints", tr("Edit Wrap Points"), "wrap-points", QKeySequence(), [this] { editWrapPoints(); }, true);
    struct Side {
        const char *id;
        const char *text;
        write::PictureWrapSide side;
    };
    for (const Side &w : {Side{"both", QT_TRANSLATE_NOOP("jp::WriteWindow", "Text on Both Sides"), write::PictureWrapSide::Both},
                          Side{"left", QT_TRANSLATE_NOOP("jp::WriteWindow", "Text on the Left Only"), write::PictureWrapSide::Left},
                          Side{"right", QT_TRANSLATE_NOOP("jp::WriteWindow", "Text on the Right Only"), write::PictureWrapSide::Right},
                          Side{"largest", QT_TRANSLATE_NOOP("jp::WriteWindow", "Text on the Larger Side"), write::PictureWrapSide::Largest}}) {
        const write::PictureWrapSide side = w.side;
        mk(QStringLiteral("picture.wrap.side.") + QLatin1String(w.id), QCoreApplication::translate("jp::WriteWindow", w.text), "wrap-text", QKeySequence(), [this, side] {
            m_view->changeSelectedPicture([side](write::WriteDoc &d, int at) { return write::setPictureWrapSide(d, at, side); });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
    mk("picture.wrap.distance", tr("Distance from Text…"), "wrap-text", QKeySequence(), [this] { wrapDistanceDialog(); });
    // The wrap, side, and room of the selected picture become the layout of the pictures put in afterward (kept with the program's settings, not the file).
    mk("picture.wrap.setDefault", tr("Set as Default Layout"), "wrap-text", QKeySequence(), [this] {
        const int at = m_view->selectedPicture();
        if (!m_doc || at < 0) return;
        const write::PictureInfo info = write::pictureAt(*m_doc, m_doc->story(), at);
        if (!info.valid || info.kind != write::DrawnKind::Picture) return;
        write::setPictureWrapDefault(write::pictureWrapDefaultOf(info));
        statusBar()->showMessage(tr("Pictures you put in from now on are laid out like this one."), 6000);
    });
    struct Order {
        const char *id;
        const char *text;
        write::PictureOrder order;
        const char *icon;
    };
    for (const Order &o : {Order{"front", QT_TRANSLATE_NOOP("jp::WriteWindow", "Bring to Front"), write::PictureOrder::BringToFront, "bring-to-front"},
                           Order{"forward", QT_TRANSLATE_NOOP("jp::WriteWindow", "Bring Forward"), write::PictureOrder::BringForward, "arrow-up"},
                           Order{"back", QT_TRANSLATE_NOOP("jp::WriteWindow", "Send to Back"), write::PictureOrder::SendToBack, "send-to-back"},
                           Order{"backward", QT_TRANSLATE_NOOP("jp::WriteWindow", "Send Backward"), write::PictureOrder::SendBackward, "arrow-down"}}) {
        const write::PictureOrder order = o.order;
        mk(QStringLiteral("picture.order.") + QLatin1String(o.id), QCoreApplication::translate("jp::WriteWindow", o.text), o.icon, QKeySequence(), [this, order] {
            m_view->changeSelectedPicture([order](write::WriteDoc &d, int at) { return write::setPictureOrder(d, at, order); });
            m_view->setFocus(Qt::OtherFocusReason);
        });
    }
    mk("picture.altText", tr("Alt Text…"), "captions", QKeySequence(), [this] {
        const int at = m_view->selectedPicture();
        if (!m_doc || at < 0) return;
        QString alt = write::pictureAt(*m_doc, m_doc->story(), at).alt;
        if (write::altTextDialog(this, &alt)) m_view->changeSelectedPicture([alt](write::WriteDoc &d, int position) { return write::setPictureAltText(d, position, alt); });
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("picture.decorative", tr("Mark as Decorative"), "eye-off", QKeySequence(), [this] {
        const int at = m_view->selectedPicture();
        if (!m_doc || at < 0) return;
        const bool on = act("picture.decorative")->isChecked();
        m_view->changeSelectedPicture([on](write::WriteDoc &d, int position) { return write::setPictureDecorative(d, position, on); });
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    mk("picture.lockAspect", tr("Lock Aspect Ratio"), "link", QKeySequence(), [] {}, true)->setChecked(true);
    connect(m_view, &write::PageView::notice, this, [this](const QString &text) { statusBar()->showMessage(text, 8000); });
}

void WriteWindow::pictureRibbonParts(RibbonParts &parts)
{
    parts.colors[QStringLiteral("picture")] = QColor(0xD3, 0x7B, 0x2B);
    parts.menus[QStringLiteral("pictureWrap")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        auto *group = new QActionGroup(m);
        for (const char *id : {"inline", "square", "tight", "topBottom", "behind", "front"}) {
            QAction *a = act(QStringLiteral("picture.wrap.") + QLatin1String(id));
            group->addAction(a);
            m->addAction(a);
        }
        addWrapDetails(m);
        return m;
    };
    parts.menus[QStringLiteral("pictureAspect")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        for (const auto &r : aspectRatios()) m->addAction(act(QStringLiteral("picture.aspect.%1x%2").arg(r.w).arg(r.h)));
        return m;
    };
    parts.menus[QStringLiteral("pictureForward")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        m->addAction(act("picture.order.forward"));
        m->addAction(act("picture.order.front"));
        return m;
    };
    parts.menus[QStringLiteral("pictureBackward")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        m->addAction(act("picture.order.backward"));
        m->addAction(act("picture.order.back"));
        return m;
    };
    for (int which = 0; which < 2; ++which) {
        parts.widgets[which == 0 ? QStringLiteral("pictureHeight") : QStringLiteral("pictureWidth")] = [this, which]() -> QWidget * {
            auto *spin = new MeasureSpin();
            spin->setObjectName(which == 0 ? QStringLiteral("pictureHeightField") : QStringLiteral("pictureWidthField"));
            spin->setAccessibleName(which == 0 ? tr("Picture height") : tr("Picture width"));
            m_pictureFields << spin;
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
                if (act("picture.lockAspect")->isChecked()) write::clampKeepingRatio(&w, &h);   // (the limits hold the size as a whole: its proportions stay)
                if (std::abs(w - now.width) < 0.005 && std::abs(h - now.height) < 0.005) return;   // (the same size is no command: no step to undo)
                m_view->changeSelectedPicture([w, h](write::WriteDoc &d, int position) { return write::setPictureSize(d, position, w, h); });
            });
            return spin;
        };
    }
}

// Insert > Pictures > Online Pictures: the shared pane on the right searches the two open libraries and puts the picture chosen in at the caret, its title its alt text and, as asked, its credit under it.
void WriteWindow::showOnlinePictures()
{
    if (!m_doc) return;
    if (!m_online) {
        m_onlineDock = new QDockWidget(tr("Online Pictures"), this);
        m_onlineDock->setObjectName(QStringLiteral("onlinePicturesDock"));
        m_onlineDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_online = new OnlinePicturesPane([this](const QString &path, const online::Picture &picture, bool withCredit) {
            if (!m_doc || refuseIfReadOnly()) return false;
            QByteArray bytes;
            if (write::readPictureFile(path, &bytes) != write::PictureResult::Ok) return false;
            const bool done = m_view->insertOnlinePictureBytes(bytes, picture.title, withCredit ? online::credit(picture) : QString());
            m_view->setFocus(Qt::OtherFocusReason);
            return done;
        });
        // With the margins the window's other panes have.
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

void WriteWindow::insertPictureFromFile()
{
    if (!m_doc) return;
    QString path;
    if (m_pictureChooser) {
        path = m_pictureChooser();
    } else {
        const QString dir = Settings::get().value(QStringLiteral("dirs/pictures"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
        const QString filter = tr("Pictures (*.png *.jpg *.jpeg *.gif *.bmp *.tif *.tiff *.webp *.svg)") + QStringLiteral(";;") + tr("All Files (*)");
        const QStringList paths = askPicturePaths(this, tr("Insert Picture"), dir, filter, false, nullptr);
        if (paths.isEmpty()) return;
        Settings::get().setValue(QStringLiteral("dirs/pictures"), QFileInfo(paths.first()).absolutePath());
        path = paths.first();
    }
    if (path.isEmpty()) return;
    QByteArray bytes;
    const write::PictureResult read = write::readPictureFile(path, &bytes);
    if (read == write::PictureResult::TooLarge) {
        statusBar()->showMessage(tr("That picture is too large to put in a document."), 8000);
        return;
    }
    if (read != write::PictureResult::Ok) {
        statusBar()->showMessage(tr("%1 can't be opened as a picture.").arg(QFileInfo(path).fileName()), 8000);
        return;
    }
    m_view->insertPictureBytes(bytes, QFileInfo(path).completeBaseName());
    m_view->setFocus(Qt::OtherFocusReason);
}

// Insert > Icons: the shared icon picker, and each icon chosen put in at the caret as a shape an inch square, in the color scheme's main color.
void WriteWindow::insertIconsFromPicker()
{
    if (!m_doc || refuseIfReadOnly()) return;
    insertIcons(pickIcons(this));
}

void WriteWindow::insertIcons(const QStringList &names)
{
    if (!m_doc || refuseIfReadOnly()) return;
    for (const QString &name : names)
        if (!m_view->insertIconObject(name)) break;
    m_view->setFocus(Qt::OtherFocusReason);
}

// A barcode field (Mailings > Barcode from Field): the dialog asks for its type and look again, with the value it shows now as the sample, and the field takes them.
void WriteWindow::editBarcodeField(int position)
{
    if (!m_doc) return;
    const auto field = write::mail::barcodeFieldAt(*m_doc, position);
    if (!field) return;
    if (!field->editable) {
        statusBar()->showMessage(tr("This barcode field can't be edited here."), 6000);   // (no merge field in it, or it is locked)
        return;
    }
    QJsonObject settings;
    if (!askBarcode(this, field->settings, &settings, true)) return;
    if (!write::mail::changeBarcodeField(*m_doc, position, settings)) statusBar()->showMessage(tr("That barcode can't be put in as a field."), 6000);
    m_view->setFocus(Qt::OtherFocusReason);
    m_view->viewport()->update();
    refreshUi();
}

// Insert > Barcode: the dialog of the other apps asks for the barcode, which goes in at the caret as a floating picture. With a barcode picture selected, the dialog edits it: it keeps its place and
// its width, and one step undoes the change.
void WriteWindow::insertBarcodeFromDialog()
{
    if (!m_doc || refuseIfReadOnly()) return;
    const int at = m_view->selectedPicture();
    const QJsonObject was = at >= 0 ? write::pictureAt(*m_doc, m_doc->story(), at).barcode : QJsonObject();
    QJsonObject settings;
    if (!askBarcode(this, was, &settings)) return;
    if (was.isEmpty()) m_view->insertBarcodeSettings(settings);
    else m_view->changeSelectedPicture([settings](write::WriteDoc &d, int position) { return write::changeBarcode(d, position, settings); });
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::insertPictureFromClipboard()
{
    if (!m_doc) return;
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    const QVariant data = mime && mime->hasImage() ? mime->imageData() : QVariant();
    if (!data.canConvert<QImage>()) {
        statusBar()->showMessage(tr("There is no picture on the clipboard."), 6000);
        return;
    }
    m_view->insertPictureImage(data.value<QImage>());
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::showPageSetupDialog()
{
    if (!m_doc) return;
    // ("This point forward" needs a place in the text for the section break: not in a header, a footer, or a note.)
    QTextCursor *place = !m_view->inHeaderFooter() && !m_view->inNote() ? &m_view->cursor() : nullptr;
    if (write::pageSetupDialog(this, m_doc.get(), m_view->cursorSection(), place)) m_view->refresh();
    m_view->setFocus(Qt::OtherFocusReason);
}

// What follows the selection: the Picture Format tab and its commands.
void WriteWindow::refreshPictureUi()
{
    const bool body = m_doc && !m_view->inHeaderFooter() && !m_view->inNote();
    act("insert.picture")->setEnabled(body);
    act("insert.pictureClipboard")->setEnabled(body);
    act("insert.onlinePicture")->setEnabled(body);
    act("insert.icons")->setEnabled(body);
    act("insert.barcode")->setEnabled(body);
    const int at = m_doc ? m_view->selectedPicture() : -1;
    const write::PictureInfo info = at >= 0 ? write::pictureAt(*m_doc, m_doc->story(), at) : write::PictureInfo();
    m_ribbon->setContextVisible(QStringLiteral("Picture Tools"), info.valid && info.kind == write::DrawnKind::Picture, false);
    const bool editable = info.valid && info.editable;
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("picture."))) a->setEnabled(editable);
    act("picture.lockAspect")->setEnabled(true);
    // (A picture moves in the layers only where it floats and there is a place for it to go.)
    for (const auto &o : {std::pair<const char *, write::PictureOrder>{"front", write::PictureOrder::BringToFront}, {"forward", write::PictureOrder::BringForward},
                          {"back", write::PictureOrder::SendToBack}, {"backward", write::PictureOrder::SendBackward}})
        act(QStringLiteral("picture.order.") + QLatin1String(o.first))->setEnabled(editable && info.floating && write::canOrderPicture(*m_doc, at, o.second));
    static const std::pair<const char *, write::PictureWrap> wraps[] = {{"inline", write::PictureWrap::Inline}, {"square", write::PictureWrap::Square}, {"tight", write::PictureWrap::Tight},
                                                                        {"topBottom", write::PictureWrap::TopAndBottom}, {"behind", write::PictureWrap::BehindText},
                                                                        {"front", write::PictureWrap::InFrontOfText}};
    act("picture.decorative")->setChecked(editable && info.decorative);
    // (A side and room round it are for a picture the text goes round; the outline is for a tight one, and the square one gets it when it is edited.)
    const bool round = editable && info.floating && (info.wrap == write::PictureWrap::Square || info.wrap == write::PictureWrap::Tight || info.wrap == write::PictureWrap::TopAndBottom);
    const bool sided = round && info.wrap != write::PictureWrap::TopAndBottom;
    static const std::pair<const char *, write::PictureWrapSide> sides[] = {{"both", write::PictureWrapSide::Both}, {"left", write::PictureWrapSide::Left}, {"right", write::PictureWrapSide::Right},
                                                                            {"largest", write::PictureWrapSide::Largest}};
    for (const auto &w : sides) {
        act(QStringLiteral("picture.wrap.side.") + QLatin1String(w.first))->setEnabled(sided);
        act(QStringLiteral("picture.wrap.side.") + QLatin1String(w.first))->setChecked(sided && info.wrapSide == w.second);
    }
    act("picture.wrap.distance")->setEnabled(round);
    act("picture.wrap.setDefault")->setEnabled(editable && info.kind == write::DrawnKind::Picture);
    act("picture.wrap.setDefault")->setVisible(!info.valid || info.kind == write::DrawnKind::Picture);   // (the layout is a picture's: no entry in the menus of a shape or a text box)
    act("picture.wrap.editPoints")->setEnabled(editable && info.floating && write::hasWrapPoints(info.kind) && (info.wrap == write::PictureWrap::Square || info.wrap == write::PictureWrap::Tight));
    if (!editable || !info.floating || !write::hasWrapPoints(info.kind) || (info.wrap != write::PictureWrap::Tight && info.wrap != write::PictureWrap::Square)) m_view->setEditWrap(false);
    act("picture.wrap.editPoints")->setChecked(m_view->editingWrap());
    for (const auto &w : wraps) act(QStringLiteral("picture.wrap.") + QLatin1String(w.first))->setChecked(editable && info.wrap == w.second);
    m_updatingFields = true;
    if (m_pictureFields.size() >= 2 && info.valid) {
        m_pictureFields[0]->setPoints(info.height);
        m_pictureFields[1]->setPoints(info.width);
    }
    m_updatingFields = false;
    refreshShapeUi();
}

void WriteWindow::addPictureContextMenu(QMenu &menu)
{
    menu.addSeparator();
    QMenu *wrap = menu.addMenu(tr("Wrap Text"));
    for (const char *id : {"inline", "square", "tight", "topBottom", "behind", "front"}) wrap->addAction(act(QStringLiteral("picture.wrap.") + QLatin1String(id)));
    addWrapDetails(wrap);
    QMenu *order = menu.addMenu(tr("Arrange"));
    for (const char *id : {"front", "forward", "back", "backward"}) order->addAction(act(QStringLiteral("picture.order.") + QLatin1String(id)));
    menu.addAction(act("arrange.group"));
    menu.addAction(act("arrange.ungroup"));
    menu.addAction(act("picture.altText"));
    menu.addAction(act("picture.decorative"));
    if (act("shape.editText")->isEnabled()) menu.addAction(act("shape.editText"));
}

// The side and the room round the picture, after the kinds of wrap in a menu.
void WriteWindow::addWrapDetails(QMenu *menu)
{
    menu->addSeparator();
    menu->addAction(act("picture.wrap.editPoints"));
    QMenu *sides = menu->addMenu(tr("Text Sides"));
    auto *group = new QActionGroup(sides);
    for (const char *id : {"both", "left", "right", "largest"}) {
        QAction *a = act(QStringLiteral("picture.wrap.side.") + QLatin1String(id));
        group->addAction(a);
        sides->addAction(a);
    }
    menu->addAction(act("picture.wrap.distance"));
    menu->addSeparator();
    menu->addAction(act("picture.wrap.setDefault"));
}

// Edit Wrap Points: a square wrap becomes a tight one, with the frame's points to drag (Ctrl and a click on the outline puts one in, on a point takes it away); Esc, or this again, ends it.
void WriteWindow::editWrapPoints()
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0) return;
    if (m_view->editingWrap()) {
        m_view->setEditWrap(false);
        refreshPictureUi();
        return;
    }
    const write::PictureInfo info = write::pictureAt(*m_doc, m_doc->story(), at);
    if (!info.valid || !info.editable || !info.floating || !write::hasWrapPoints(info.kind)) return;
    m_view->setEditWrap(true);
    refreshPictureUi();
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::wrapDistanceDialog()
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0) return;
    const write::PictureInfo info = write::pictureAt(*m_doc, m_doc->story(), at);
    if (!info.valid || !info.floating) return;
    QDialog dlg(this);
    dlg.setObjectName(QStringLiteral("wrapDistanceDialog"));
    dlg.setWindowTitle(tr("Distance from Text"));
    auto *layout = new QVBoxLayout(&dlg);
    auto *label = new QLabel(tr("How much room the text leaves around the picture, in points."), &dlg);
    label->setWordWrap(true);
    layout->addWidget(label);
    auto *form = new QFormLayout();
    QDoubleSpinBox *edges[4];
    const double values[4] = {info.distTop, info.distBottom, info.distLeft, info.distRight};
    const QString names[4] = {tr("Top"), tr("Bottom"), tr("Left"), tr("Right")};
    const char *objectNames[4] = {"distTop", "distBottom", "distLeft", "distRight"};
    for (int i = 0; i < 4; ++i) {
        edges[i] = new QDoubleSpinBox(&dlg);
        edges[i]->setObjectName(QLatin1String(objectNames[i]));
        edges[i]->setRange(0, write::kMostWrapDistance);
        edges[i]->setDecimals(1);
        edges[i]->setSuffix(tr(" pt"));
        edges[i]->setValue(std::round(values[i] * 10.0) / 10.0);
        form->addRow(names[i], edges[i]);
    }
    layout->addLayout(form);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted) return;
    const double t = edges[0]->value(), b = edges[1]->value(), l = edges[2]->value(), r = edges[3]->value();
    m_view->changeSelectedPicture([=](write::WriteDoc &d, int position) { return write::setPictureWrapDistances(d, position, t, b, l, r); });
    m_view->setFocus(Qt::OtherFocusReason);
}

} // namespace jp

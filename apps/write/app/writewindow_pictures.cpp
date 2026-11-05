// JeffWrite's pictures, page setup, and ruler: Insert > Pictures, the Picture Format tab and the menu of a right click on a picture, the Page Setup dialog, and the ruler (the work on
// the document is core/pictureops.h, core/pagesetup.h, and core/rulerops.h).

#include "app/writewindow.h"

#include "app/filedialogs.h"
#include "app/icons.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/ruler.h"
#include "app/settings.h"
#include "app/widgets.h"
#include "app/writedialogs.h"
#include "core/pictureops.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QMenu>
#include <QMimeData>
#include <QStandardPaths>
#include <QStatusBar>

namespace jp {

void WriteWindow::createPictureActions()
{
    mk("insert.picture", tr("Picture…"), "image-plus", QKeySequence(), [this] { insertPictureFromFile(); });
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
    if (write::pageSetupDialog(this, m_doc.get(), m_view->cursorSection())) m_view->refresh();
    m_view->setFocus(Qt::OtherFocusReason);
}

// What follows the selection: the Picture Format tab and its commands.
void WriteWindow::refreshPictureUi()
{
    const bool body = m_doc && !m_view->inHeaderFooter() && !m_view->inNote();
    act("insert.picture")->setEnabled(body);
    act("insert.pictureClipboard")->setEnabled(body);
    const int at = m_doc ? m_view->selectedPicture() : -1;
    const write::PictureInfo info = at >= 0 ? write::pictureAt(*m_doc, m_doc->story(), at) : write::PictureInfo();
    m_ribbon->setContextVisible(QStringLiteral("Picture Tools"), info.valid, false);
    const bool editable = info.valid && info.editable;
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("picture."))) a->setEnabled(editable);
    act("picture.lockAspect")->setEnabled(true);
    static const std::pair<const char *, write::PictureWrap> wraps[] = {{"inline", write::PictureWrap::Inline}, {"square", write::PictureWrap::Square}, {"tight", write::PictureWrap::Tight},
                                                                        {"topBottom", write::PictureWrap::TopAndBottom}, {"behind", write::PictureWrap::BehindText},
                                                                        {"front", write::PictureWrap::InFrontOfText}};
    act("picture.decorative")->setChecked(editable && info.decorative);
    for (const auto &w : wraps) act(QStringLiteral("picture.wrap.") + QLatin1String(w.first))->setChecked(editable && info.wrap == w.second);
    m_updatingFields = true;
    if (m_pictureFields.size() >= 2 && info.valid) {
        m_pictureFields[0]->setPoints(info.height);
        m_pictureFields[1]->setPoints(info.width);
    }
    m_updatingFields = false;
}

void WriteWindow::addPictureContextMenu(QMenu &menu)
{
    menu.addSeparator();
    QMenu *wrap = menu.addMenu(tr("Wrap Text"));
    for (const char *id : {"inline", "square", "tight", "topBottom", "behind", "front"}) wrap->addAction(act(QStringLiteral("picture.wrap.") + QLatin1String(id)));
    menu.addAction(act("picture.altText"));
    menu.addAction(act("picture.decorative"));
}

} // namespace jp

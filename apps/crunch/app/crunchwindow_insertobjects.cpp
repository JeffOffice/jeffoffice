// Insert > Pictures, Shapes, and Text Box: a picture from a file, a shape from the gallery, or a text box goes on the sheet as the objects a file holds do (core/sheetobjects.h makes them from
// the element a drawing holds for them), one step of Undo, and is saved with the workbook's drawing.
#include "app/crunchwindow.h"

#include "core/objectclip.h"
#include "core/sheetobjects.h"

#include <QAction>
#include <QBuffer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>

#include <algorithm>

namespace jp {

using namespace jp::office::sheet;

namespace {

struct ShapeKind {
    const char *id;
    const char *preset;
    const char *text;
    const char *icon;
};

const ShapeKind kShapes[] = {
    {"insert.shape.rect", "rect", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Rectangle"), "rectangle-horizontal"},
    {"insert.shape.roundRect", "roundRect", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Rounded Rectangle"), "square"},
    {"insert.shape.ellipse", "ellipse", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Oval"), "circle"},
    {"insert.shape.triangle", "triangle", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Triangle"), "triangle"},
    {"insert.shape.diamond", "diamond", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Diamond"), "diamond"},
    {"insert.shape.pentagon", "homePlate", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Pentagon Arrow"), "pentagon"},
    {"insert.shape.hexagon", "hexagon", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Hexagon"), "hexagon"},
    {"insert.shape.rightArrow", "rightArrow", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Right Arrow"), "arrow-right"},
    {"insert.shape.leftArrow", "leftArrow", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Left Arrow"), "arrow-left"},
    {"insert.shape.upArrow", "upArrow", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Up Arrow"), "arrow-up"},
    {"insert.shape.downArrow", "downArrow", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Down Arrow"), "arrow-down"},
    {"insert.shape.star", "star5", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Five-Point Star"), "star"},
    {"insert.shape.heart", "heart", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Heart"), "heart"},
    {"insert.shape.cloud", "cloud", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Cloud"), "cloud"},
    {"insert.shape.callout", "wedgeRectCallout", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Speech Callout"), "message-square"},
    {"insert.shape.line", "line", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Line"), "minus"},
};

constexpr double kEmuPerUnit = 9525;   // the grid's unit is a pixel at 100%

const char kFill[] = "BDD7EE", kLine[] = "2E75B6";

QByteArray shapeXml(const QString &preset, double w, double h, bool textBox)
{
    const qint64 cx = qint64(w * kEmuPerUnit), cy = qint64(h * kEmuPerUnit);
    const QByteArray xfrm = "<a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + QByteArray::number(cx) + "\" cy=\"" + QByteArray::number(cy) + "\"/></a:xfrm>";
    const QByteArray geometry = "<a:prstGeom prst=\"" + preset.toLatin1() + "\"><a:avLst/></a:prstGeom>";
    if (preset == QLatin1String("line"))
        return "<xdr:cxnSp macro=\"\"><xdr:nvCxnSpPr><xdr:cNvPr id=\"0\" name=\"\"/><xdr:cNvCxnSpPr/></xdr:nvCxnSpPr><xdr:spPr>" + xfrm + geometry + "<a:ln w=\"19050\"><a:solidFill><a:srgbClr val=\"" + kLine
               + "\"/></a:solidFill></a:ln></xdr:spPr></xdr:cxnSp>";
    const QByteArray paint = textBox ? QByteArray("<a:noFill/><a:ln w=\"9525\"><a:noFill/></a:ln>")
                                     : "<a:solidFill><a:srgbClr val=\"" + QByteArray(kFill) + "\"/></a:solidFill><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"" + kLine + "\"/></a:solidFill></a:ln>";
    return QByteArray("<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"0\" name=\"\"/><xdr:cNvSpPr") + (textBox ? " txBox=\"1\"" : "") + "/></xdr:nvSpPr><xdr:spPr>" + xfrm + geometry + paint
           + "</xdr:spPr><xdr:txBody><a:bodyPr vertOverflow=\"clip\" horzOverflow=\"clip\" wrap=\"square\" rtlCol=\"0\" anchor=\"" + (textBox ? "t" : "ctr") + "\"/><a:lstStyle/><a:p><a:pPr algn=\""
           + (textBox ? "l" : "ctr") + "\"/><a:endParaRPr lang=\"en-US\" sz=\"1100\"/></a:p></xdr:txBody></xdr:sp>";
}

} // namespace

void CrunchWindow::createInsertObjectActions()
{
    mk("insert.picture", tr("Picture…"), "image", QKeySequence(), [this] { insertPicture(); });
    m_opsActions << QStringLiteral("insert.picture");
    for (const ShapeKind &k : kShapes) {
        const QString preset = QString::fromLatin1(k.preset);
        mk(QString::fromLatin1(k.id), QCoreApplication::translate("jp::CrunchWindow", k.text), k.icon, QKeySequence(), [this, preset] { insertShape(preset); });
        m_opsActions << QString::fromLatin1(k.id);
    }
    mk("insert.textBox", tr("Text Box"), "type", QKeySequence(), [this] { insertTextBox(); });
    m_opsActions << QStringLiteral("insert.textBox");
}

// Puts the element on the sheet at the cell the cursor is in, as one step of Undo, picked. `why` is the problem if it is not.
bool CrunchWindow::placeObjectClip(const crunch::ObjectClip &clip, const QString &stepText)
{
    if (!m_session || !m_objects || m_chartSheetShown) return false;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return false;
    const DrawingAnchor anchor = newObjectAnchor(clip.width, clip.height);
    QString why;
    const int id = m_objects->makeElement(m_sheet, clip, anchor, &why);
    if (id < 0) {
        notice(stepText, why);
        return false;
    }
    crunch::ObjectEdit e;
    e.sheet = m_sheet;
    e.id = id;
    e.before = m_objects->stateOf(m_sheet, id);
    e.after = e.before;
    e.after.deleted = false;
    const crunch::EditSession::Outcome o = m_session->changeObjects({e}, stepText);
    if (!o.ok) {
        m_objects->discardMade(m_sheet, id);
        notice(stepText, o.problem);
        return false;
    }
    m_objects->layer()->select(id);
    m_view->setFocus(Qt::OtherFocusReason);
    return true;
}

void CrunchWindow::insertPicture()
{
    if (!m_session || m_chartSheetShown) return;
    const QString filter = tr("Pictures (*.png *.jpg *.jpeg *.gif *.bmp *.tif *.tiff *.svg);;All files (*)");
    const QString path = m_picturePicker ? m_picturePicker() : QFileDialog::getOpenFileName(this, tr("Insert Picture"), QString(), filter);
    if (path.isEmpty()) return;
    insertPictureFile(path);
}

// A picture from a file: PNG, JPEG, GIF, BMP, and TIFF are kept as they are; any other kind the program can draw (a vector picture, say) is kept as a PNG of it.
bool CrunchWindow::insertPictureFile(const QString &path)
{
    const QString title = tr("Insert Picture");
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        notice(title, tr("The picture \"%1\" can't be opened.").arg(QFileInfo(path).fileName()));
        return false;
    }
    if (f.size() > crunch::kMaxClipMediaBytes) {
        notice(title, tr("That picture is larger than %1 MB, which is as much as JeffCrunch keeps in a workbook.").arg(crunch::kMaxClipMediaBytes >> 20));
        return false;
    }
    QByteArray bytes = f.readAll();
    f.close();
    QBuffer in(&bytes);
    in.open(QIODevice::ReadOnly);
    QImageReader reader(&in);
    const QByteArray format = reader.format().toLower();
    const QSize size = reader.size();
    constexpr qint64 kMaxPixels = qint64(100) * 1000 * 1000;
    if (format.isEmpty() || !size.isValid() || size.isEmpty() || size.width() > 30000 || size.height() > 30000 || qint64(size.width()) * size.height() > kMaxPixels) {
        notice(title, tr("\"%1\" is not a picture JeffCrunch can read, or it is too large.").arg(QFileInfo(path).fileName()));
        return false;
    }
    crunch::ClipMedia media;
    media.token = QStringLiteral("rIdInsertPicture");
    if (format == "png" || format == "gif" || format == "bmp") {
        media.extension = QString::fromLatin1(format);
        media.contentType = QStringLiteral("image/") + media.extension;
        media.bytes = bytes;
    } else if (format == "jpg" || format == "jpeg") {
        media.extension = QStringLiteral("jpeg");
        media.contentType = QStringLiteral("image/jpeg");
        media.bytes = bytes;
    } else if (format == "tif" || format == "tiff") {
        media.extension = QStringLiteral("tiff");
        media.contentType = QStringLiteral("image/tiff");
        media.bytes = bytes;
    } else {
        const QImage image = reader.read();
        if (image.isNull()) {
            notice(title, tr("\"%1\" is not a picture JeffCrunch can read, or it is too large.").arg(QFileInfo(path).fileName()));
            return false;
        }
        QBuffer out(&media.bytes);
        out.open(QIODevice::WriteOnly);
        if (!image.save(&out, "PNG")) return false;
        media.extension = QStringLiteral("png");
        media.contentType = QStringLiteral("image/png");
    }
    // the size on the sheet: the picture's own, at most 480 wide and 360 tall, with its proportions
    double w = size.width(), h = size.height();
    const double scale = std::min({1.0, 480.0 / w, 360.0 / h});
    w = std::max(8.0, std::round(w * scale));
    h = std::max(8.0, std::round(h * scale));
    crunch::ObjectClip clip;
    clip.kind = QStringLiteral("picture");
    clip.width = w;
    clip.height = h;
    clip.media.append(media);
    const QByteArray base = "<xdr:pic><xdr:nvPicPr><xdr:cNvPr id=\"0\" name=\"\"/><xdr:cNvPicPr><a:picLocks noChangeAspect=\"1\"/></xdr:cNvPicPr></xdr:nvPicPr><xdr:blipFill><a:blip r:embed=\"";
    clip.elementXml = base + media.token.toUtf8() + "\"/><a:stretch><a:fillRect/></a:stretch></xdr:blipFill><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + QByteArray::number(qint64(w * kEmuPerUnit))
                     + "\" cy=\"" + QByteArray::number(qint64(h * kEmuPerUnit)) + "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></xdr:spPr></xdr:pic>";
    if (!placeObjectClip(clip, title)) return false;
    setStatus(tr("Picture inserted"));
    return true;
}

bool CrunchWindow::insertShape(const QString &preset)
{
    const bool line = preset == QLatin1String("line");
    crunch::ObjectClip clip;
    clip.kind = line ? QStringLiteral("connector") : QStringLiteral("shape");
    clip.width = 120;
    clip.height = line ? 1 : 80;
    clip.elementXml = shapeXml(preset, clip.width, clip.height, false);
    if (!placeObjectClip(clip, tr("Insert Shape"))) return false;
    setStatus(tr("Shape inserted"));
    return true;
}

bool CrunchWindow::insertTextBox()
{
    crunch::ObjectClip clip;
    clip.kind = QStringLiteral("shape");
    clip.width = 160;
    clip.height = 48;
    clip.elementXml = shapeXml(QStringLiteral("rect"), clip.width, clip.height, true);
    if (!placeObjectClip(clip, tr("Insert Text Box"))) return false;
    setStatus(tr("Text box inserted"));
    return true;
}

} // namespace jp

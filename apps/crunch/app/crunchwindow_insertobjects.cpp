// Insert > Pictures, Shapes, and Text Box: a picture from a file, a shape from the gallery, or a text box goes on the sheet as the objects a file holds do (core/sheetobjects.h makes them from
// the element a drawing holds for them), one step of Undo, and is saved with the workbook's drawing.
#include "app/crunchwindow.h"

#include "core/barcodeclip.h"
#include "core/objectclip.h"
#include "core/sheetobjects.h"
#include "app/barcodedialog.h"
#include "app/iconpicker.h"
#include "app/onlinepictures.h"
#include "app/textdialogs.h"
#include "render/metafile.h"
#include "app/equationdialog.h"
#include "wml/wmlmathedit.h"
#include "wml/wmlmathpath.h"

#include <QAction>
#include <QDockWidget>
#include <QBuffer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QUndoStack>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

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

QByteArray shapeXml(const QString &preset, double w, double h, bool textBox, const QString &words = QString())
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
           + (textBox ? "l" : "ctr") + "\"/>"
           + (words.isEmpty() ? QByteArray() : "<a:r><a:rPr lang=\"en-US\" sz=\"900\" i=\"1\"/><a:t>" + words.toHtmlEscaped().toUtf8() + "</a:t></a:r>") + "<a:endParaRPr lang=\"en-US\" sz=\"1100\"/></a:p></xdr:txBody></xdr:sp>";
}

// The element of a drawing for an equation: a text box with no fill or line whose text is the math (the markup in a14:m, as a spreadsheet's equations are kept). `name` is its name; the
// size is in the grid's units.
QByteArray equationXml(const QByteArray &markup, double w, double h, const QString &name)
{
    const qint64 cx = qint64(w * kEmuPerUnit), cy = qint64(h * kEmuPerUnit);
    return "<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"0\" name=\"" + name.toHtmlEscaped().toUtf8() + "\"/><xdr:cNvSpPr txBox=\"1\"/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\""
           + QByteArray::number(cx) + "\" cy=\"" + QByteArray::number(cy)
           + "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:noFill/></xdr:spPr><xdr:txBody><a:bodyPr wrap=\"none\" rtlCol=\"0\" anchor=\"ctr\"/><a:lstStyle/><a:p>"
             "<a14:m xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\"><m:oMathPara xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><m:oMathParaPr><m:jc m:val=\"centerGroup\"/></m:oMathParaPr>"
           + markup + "</m:oMathPara></a14:m><a:endParaRPr lang=\"en-US\"/></a:p></xdr:txBody></xdr:sp>";
}

} // namespace

void CrunchWindow::createInsertObjectActions()
{
    mk("insert.picture", tr("Picture…"), "image", QKeySequence(), [this] { insertPicture(); });
    mk("insert.icons", tr("Icons…"), "smile-plus", QKeySequence(), [this] { insertIcons(); });
    mk("insert.onlinePicture", tr("Online Pictures…"), "globe", QKeySequence(), [this] { showOnlinePictures(); });
    mk("insert.barcode", tr("Barcode…"), "barcode", QKeySequence(), [this] { insertBarcode(); });
    m_opsActions << QStringLiteral("insert.picture") << QStringLiteral("insert.icons") << QStringLiteral("insert.onlinePicture") << QStringLiteral("insert.barcode");
    for (const ShapeKind &k : kShapes) {
        const QString preset = QString::fromLatin1(k.preset);
        mk(QString::fromLatin1(k.id), QCoreApplication::translate("jp::CrunchWindow", k.text), k.icon, QKeySequence(), [this, preset] { insertShape(preset); });
        m_opsActions << QString::fromLatin1(k.id);
    }
    mk("insert.textBox", tr("Text Box"), "type", QKeySequence(), [this] { insertTextBox(); });
    m_opsActions << QStringLiteral("insert.textBox");
    mk("insert.equation", tr("Equation…"), "square-pi", QKeySequence(), [this] { insertEquationDialog(); });
    m_opsActions << QStringLiteral("insert.equation");
    for (int i = 0; i < office::wml::equationGallery().size(); ++i) {
        const QString id = QStringLiteral("insert.equation.design.%1").arg(i);
        mk(id, office::wml::equationGallery().at(i).name, "square-pi", QKeySequence(), [this, i] { insertEquation(office::wml::equationGallery().at(i).linear); });
        m_opsActions << id;
    }
    mk("insert.symbol", tr("Symbol"), "omega", QKeySequence(), [this] {
        symbolDialog(this, [this](QWidget *, const QString &ch, const QString &font) {
            if (!insertSymbolInCell(ch)) return false;
            rememberSymbol(ch, font);
            return true;
        });
    });
    m_opsActions << QStringLiteral("insert.symbol");
}

// Puts the character at the cursor of the entry being typed (in the formula bar when that has the cursor); with no entry open, the cell's own entry is opened first, with the
// character at its end.
bool CrunchWindow::insertSymbolInCell(const QString &ch)
{
    if (!m_edit.active) {
        if (!editingAllowed()) return false;
        const grid::Selection &sel = m_view->selection();
        beginEdit(m_session->editText(m_sheet, quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol))), CellEditor::Mode::Editing, false);
        if (!m_edit.active) return false;
        m_cellEditor->setCursorPosition(m_cellEditor->text().size());
    }
    QLineEdit *line = editingInFormulaBar() ? m_formulaBar : m_cellEditor;
    line->insert(ch);
    Q_EMIT line->textEdited(line->text());   // (the other line shows it too)
    return true;
}

// Puts the element on the sheet at the cell the cursor is in, as one step of Undo, picked. `why` is the problem if it is not.
bool CrunchWindow::placeObjectClip(const crunch::ObjectClip &clip, const QString &stepText, const DrawingAnchor *at, int order)
{
    if (!m_session || !m_objects || m_chartSheetShown) return false;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return false;
    const DrawingAnchor anchor = at ? *at : newObjectAnchor(clip.width, clip.height);
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
    if (order >= 0) e.after.order = order;
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
    return insertPictureData(bytes, QFileInfo(path).fileName());
}

// The bytes of a picture, as insertPictureFile keeps them. A metafile (WMF or EMF) is drawn and kept as a PNG of it; `rasterSide` (not 0) is the side a vector picture is drawn at, in pixels,
// and `sheetSide` the side it has on the sheet.
bool CrunchWindow::insertPictureData(QByteArray bytes, const QString &name, int rasterSide, double sheetSide, const QString &alt)
{
    crunch::ObjectClip clip;
    if (!pictureClip(bytes, name, rasterSide, sheetSide, alt, &clip)) return false;
    if (!placeObjectClip(clip, tr("Insert Picture"))) return false;
    setStatus(tr("Picture inserted"));
    return true;
}

// The picture of these bytes as an object to put on a sheet (see insertPictureData); false, with the reason told, for bytes that are not a picture JeffCrunch keeps.
bool CrunchWindow::pictureClip(QByteArray bytes, const QString &name, int rasterSide, double sheetSide, const QString &alt, crunch::ObjectClip *made)
{
    const QString title = tr("Insert Picture");
    if (jp::Metafile::looksLikeMetafile(bytes)) {
        jp::Metafile metafile;
        if (!metafile.load(bytes)) {
            notice(title, tr("\"%1\" is not a picture JeffCrunch can read, or it is too large.").arg(name));
            return false;
        }
        const QImage metafileImage = metafile.toImage(1600);
        bytes.clear();
        QBuffer png(&bytes);
        png.open(QIODevice::WriteOnly);
        if (metafileImage.isNull() || !metafileImage.save(&png, "PNG")) {
            notice(title, tr("\"%1\" is not a picture JeffCrunch can read, or it is too large.").arg(name));
            return false;
        }
    }
    QBuffer in(&bytes);
    in.open(QIODevice::ReadOnly);
    QImageReader reader(&in);
    const QByteArray format = reader.format().toLower();
    QSize size = reader.size();
    if (rasterSide > 0 && format == "svg" && size.isValid() && !size.isEmpty()) {
        size.scale(rasterSide, rasterSide, Qt::KeepAspectRatio);
        reader.setScaledSize(size);
    }
    constexpr qint64 kMaxPixels = qint64(100) * 1000 * 1000;
    if (format.isEmpty() || !size.isValid() || size.isEmpty() || size.width() > 30000 || size.height() > 30000 || qint64(size.width()) * size.height() > kMaxPixels) {
        notice(title, tr("\"%1\" is not a picture JeffCrunch can read, or it is too large.").arg(name));
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
            notice(title, tr("\"%1\" is not a picture JeffCrunch can read, or it is too large.").arg(name));
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
    const double scale = sheetSide > 0 ? sheetSide / std::max(w, h) : std::min({1.0, 480.0 / w, 360.0 / h});
    w = std::max(8.0, std::round(w * scale));
    h = std::max(8.0, std::round(h * scale));
    crunch::ObjectClip clip;
    clip.kind = QStringLiteral("picture");
    clip.width = w;
    clip.height = h;
    clip.media.append(media);
    const QByteArray description = alt.isEmpty() ? QByteArray() : " descr=\"" + alt.toHtmlEscaped().toUtf8().replace("\"", "&quot;") + "\"";
    const QByteArray base = "<xdr:pic><xdr:nvPicPr><xdr:cNvPr id=\"0\" name=\"\"" + description + "/><xdr:cNvPicPr><a:picLocks noChangeAspect=\"1\"/></xdr:cNvPicPr></xdr:nvPicPr><xdr:blipFill><a:blip r:embed=\"";
    clip.elementXml = base + media.token.toUtf8() + "\"/><a:stretch><a:fillRect/></a:stretch></xdr:blipFill><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + QByteArray::number(qint64(w * kEmuPerUnit))
                     + "\" cy=\"" + QByteArray::number(qint64(h * kEmuPerUnit)) + "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></xdr:spPr></xdr:pic>";
    *made = clip;
    return true;
}

// Insert > Pictures > Online Pictures: the shared pane on the right searches the two open libraries; the picture chosen goes on the sheet as a picture of a file does, with its title as alt text
// and, as asked, a credit line in a small text box under it; the picture and its credit are one step of Undo.
void CrunchWindow::showOnlinePictures()
{
    if (!m_session) return;
    if (!m_online) {
        m_onlineDock = new QDockWidget(tr("Online Pictures"), this);
        m_onlineDock->setObjectName(QStringLiteral("onlinePicturesDock"));
        m_onlineDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_online = new OnlinePicturesPane([this](const QString &path, const online::Picture &picture, bool withCredit) {
            return insertOnlinePicture(path, picture.title, withCredit ? online::credit(picture) : QString());
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

bool CrunchWindow::insertOnlinePicture(const QString &path, const QString &title, const QString &credit)
{
    if (!m_session || !m_objects || m_chartSheetShown) return false;
    const QString stepText = tr("Insert Picture");
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        notice(stepText, tr("The picture \"%1\" can't be opened.").arg(QFileInfo(path).fileName()));
        return false;
    }
    if (f.size() > crunch::kMaxClipMediaBytes) {
        notice(stepText, tr("That picture is larger than %1 MB, which is as much as JeffCrunch keeps in a workbook.").arg(crunch::kMaxClipMediaBytes >> 20));
        return false;
    }
    crunch::ObjectClip clip;
    if (!pictureClip(f.readAll(), QFileInfo(path).fileName(), 0, 0, title, &clip)) return false;
    // The picture and its credit are made first and put on the sheet together: one step of Undo, and none at all when the picture can't be put there.
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return false;
    QString why;
    const int picture = m_objects->makeElement(m_sheet, clip, newObjectAnchor(clip.width, clip.height), &why);
    if (picture < 0) {
        notice(stepText, why);
        return false;
    }
    auto appears = [&](int id) {
        crunch::ObjectEdit e;
        e.sheet = m_sheet;
        e.id = id;
        e.before = m_objects->stateOf(m_sheet, id);
        e.after = e.before;
        e.after.deleted = false;
        return e;
    };
    std::vector<crunch::ObjectEdit> edits{appears(picture)};
    std::vector<int> made{picture};
    QString creditWhy;
    if (!credit.isEmpty()) {
        // The credit under the picture, as wide as the picture (at least 160), in a text box of its own.
        const grid::ObjectBox box = m_objects->layer()->boxNow(picture);
        const DrawingAnchor was = m_objects->anchorOf(m_sheet, picture);
        crunch::ObjectClip words;
        words.kind = QStringLiteral("shape");
        words.width = std::max(160.0, box.w);
        words.height = 36;
        words.elementXml = shapeXml(QStringLiteral("rect"), words.width, words.height, true, credit);
        grid::ObjectBox under = box;
        under.y = box.y + box.h + 4;
        under.w = words.width;
        under.h = words.height;
        DrawingAnchor anchor = was;
        m_objects->layer()->attach(&m_view->sheet().layout());
        int text = -1;
        if (m_objects->layer()->anchorAtBox(under, was.kind, &anchor)) {
            anchor.editAs = was.editAs;
            text = m_objects->makeElement(m_sheet, words, anchor, &creditWhy);
        } else {
            creditWhy = tr("there is no room for it under the picture");
        }
        if (text >= 0) {
            edits.push_back(appears(text));
            made.push_back(text);
        }
    }
    const crunch::EditSession::Outcome o = m_session->changeObjects(edits, stepText);
    if (!o.ok) {
        for (const int id : made) m_objects->discardMade(m_sheet, id);
        notice(stepText, o.problem);
        return false;
    }
    m_objects->layer()->select(picture);
    m_view->setFocus(Qt::OtherFocusReason);
    if (!credit.isEmpty() && made.size() == 1) notice(stepText, tr("The picture was put in, but its credit could not be: %1").arg(creditWhy));
    setStatus(tr("Picture inserted"));
    return true;
}

// Insert > Icons: the picker of the shared icons (the ones JeffPub and JeffDeck have) asks which, and each is put on the sheet as a picture, drawn at 256 pixels and 48 points square on the sheet.
bool CrunchWindow::insertIcons()
{
    if (!m_session || m_chartSheetShown || !editingAllowed()) return false;
    const QStringList names = m_iconsPicker ? m_iconsPicker() : pickIcons(this);
    int done = 0;
    for (const QString &name : names.mid(0, 50)) {
        const QByteArray svg = iconSvg(name, QColor(0x26, 0x26, 0x26));
        if (svg.isEmpty() || !insertPictureData(svg, name, 256, 48)) continue;
        ++done;
    }
    if (done == 0) return false;
    setStatus(tr("%n icon(s) inserted", nullptr, done));
    return true;
}

// Insert > Barcode: the dialog of the other apps asks for the barcode, and it goes on the sheet as a picture, as one step of Undo. With a barcode selected, the dialog edits that one: the new barcode
// is put where the old one was (as wide, and as tall as its proportions make it) and the old one is taken away, in the same step.
bool CrunchWindow::insertBarcode()
{
    if (!m_session || !m_objects || m_chartSheetShown || !editingAllowed()) return false;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active) return false;
    const int picked = m_objects->layer()->selectedId();
    const QJsonObject was = picked >= 0 ? m_objects->barcodeOf(m_sheet, picked) : QJsonObject();
    QJsonObject settings;
    if (!askBarcode(this, was, &settings)) return false;
    const QString stepText = was.isEmpty() ? tr("Insert Barcode") : tr("Edit Barcode");
    crunch::ObjectClip clip;
    QString why;
    if (!crunch::barcodeClip(settings, &clip, &why)) {
        notice(stepText, why);
        return false;
    }
    if (was.isEmpty()) {
        if (!placeObjectClip(clip, stepText)) return false;
        setStatus(tr("Barcode inserted"));
        return true;
    }
    // The old barcode's place: its left and top, and its width (the new one's height follows its own proportions).
    const grid::ObjectBox box = m_objects->layer()->boxNow(picked);
    const DrawingAnchor oldAnchor = m_objects->anchorOf(m_sheet, picked);
    DrawingAnchor anchor = oldAnchor;
    grid::ObjectBox now = box;
    now.h = box.w * clip.height / std::max(1.0, clip.width);
    m_objects->layer()->attach(&m_view->sheet().layout());
    if (!m_objects->layer()->anchorAtBox(now, oldAnchor.kind, &anchor)) anchor = oldAnchor;
    anchor.editAs = oldAnchor.editAs;
    const int fresh = m_objects->makeElement(m_sheet, clip, anchor, &why);
    if (fresh < 0) {
        notice(stepText, why);
        return false;
    }
    crunch::ObjectEdit gone;
    gone.sheet = m_sheet;
    gone.id = picked;
    gone.before = m_objects->stateOf(m_sheet, picked);
    gone.after = gone.before;
    gone.after.deleted = true;
    gone.after.revive = m_objects->captureRevive(m_sheet, picked);
    gone.before.revive = gone.after.revive;
    crunch::ObjectEdit put;
    put.sheet = m_sheet;
    put.id = fresh;
    put.before = m_objects->stateOf(m_sheet, fresh);
    put.after = put.before;
    put.after.deleted = false;
    put.after.order = gone.before.order;
    const crunch::EditSession::Outcome o = m_session->changeObjects({gone, put}, stepText);
    if (!o.ok) {
        m_objects->discardMade(m_sheet, fresh);
        notice(stepText, o.problem);
        return false;
    }
    m_objects->layer()->select(fresh);
    m_view->setFocus(Qt::OtherFocusReason);
    setStatus(tr("Barcode changed"));
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

// Insert > Equation: the equation typed in a line is put on the sheet at the size its letters make (20 points tall), picked.
bool CrunchWindow::insertEquation(const QString &linear)
{
    if (!m_session || !m_objects || m_chartSheetShown || !editingAllowed()) return false;
    const QString title = tr("Insert Equation");
    bool ok = false;
    const QByteArray markup = office::wml::equationMarkup(linear, &ok);
    const office::wml::EquationShape shape = ok ? office::wml::equationShape(markup, 20) : office::wml::EquationShape();
    if (!shape.valid()) {
        notice(title, tr("An equation can't be made from that."));
        return false;
    }
    QString name = tr("Equation");
    for (int n = 2; m_objects->nameTaken(m_sheet, -1, name); ++n) name = tr("Equation %1").arg(n);
    crunch::ObjectClip clip;
    clip.kind = QStringLiteral("shape");
    clip.width = std::clamp(std::ceil(shape.box.width() * 96.0 / 72.0), 8.0, 4000.0);
    clip.height = std::clamp(std::ceil(shape.box.height() * 96.0 / 72.0), 8.0, 2000.0);
    clip.elementXml = equationXml(markup, clip.width, clip.height, name);
    if (!placeObjectClip(clip, title)) return false;
    setStatus(tr("Equation inserted"));
    return true;
}

// Changes the line of the equation `id` (it is picked first): the object is made again with the new math, as the other changes of a shape are, one step of Undo; it keeps its height and its place.
bool CrunchWindow::editEquation(int id, const QString &linear)
{
    if (!m_session || !m_objects || m_chartSheetShown) return false;
    QString now;
    if (!m_objects->equationOf(m_sheet, id, &now, nullptr)) return false;
    bool ok = false;
    const QByteArray markup = office::wml::equationMarkup(linear, &ok);
    const office::wml::EquationShape shape = ok ? office::wml::equationShape(markup, 20) : office::wml::EquationShape();
    if (!shape.valid()) {
        notice(tr("Edit Equation"), tr("An equation can't be made from that."));
        return false;
    }
    if (now == linear.trimmed()) return false;
    m_objects->layer()->select(id);
    return restyleObject(tr("Edit Equation"), [&](QDomElement &el, grid::ObjectBox *box) {
        const QByteArray fresh = equationXml(markup, 1, 1, QString());
        QDomDocument doc;
        if (!doc.setContent(QByteArray("<w xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">") + fresh + "</w>",
                            QDomDocument::ParseOption::UseNamespaceProcessing))
            return false;
        // (the new math in the place of the old: the first a14:m of the shape's text)
        QDomElement oldMath, newMath;
        std::function<QDomElement(const QDomElement &)> find = [&](const QDomElement &e) -> QDomElement {
            if (e.localName() == QLatin1String("m") && e.namespaceURI().endsWith(QLatin1String("2010/main"))) return e;
            for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
                if (const QDomElement f = find(c); !f.isNull()) return f;
            return QDomElement();
        };
        oldMath = find(el);
        newMath = find(doc.documentElement());
        if (oldMath.isNull() || newMath.isNull()) return false;
        oldMath.parentNode().replaceChild(el.ownerDocument().importNode(newMath, true), oldMath);
        // The height stays, and the width is as the new equation's proportions make it.
        box->w = std::clamp(std::round(box->h * shape.box.width() / std::max(1.0, shape.box.height())), 8.0, 4000.0);
        return true;
    });
}

// Insert > Equation: the editor the other programs have, with the picked equation's line when one is picked (changing it), else with none (putting a new one in).
void CrunchWindow::insertEquationDialog()
{
    if (!m_session || !m_objects || m_chartSheetShown || !editingAllowed()) return;
    const int id = pickedObject();
    QString line;
    const bool editing = id >= 0 && m_objects->equationOf(m_sheet, id, &line, nullptr);
    if (editing && line.isEmpty()) {
        notice(tr("Edit Equation"), tr("This equation can't be changed here."));
        return;
    }
    if (!askEquation(this, editing ? tr("Edit Equation") : tr("Insert Equation"), &line)) return;
    if (editing) editEquation(id, line);
    else insertEquation(line);
}

} // namespace jp

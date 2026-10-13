#pragma once
// Dialog boxes.

#include "app/proofingdialogs.h"
#include "app/textdialogs.h"
#include "core/barcode.h"
#include "core/document.h"
#include "core/style.h"

#include <QPainterPath>

#include <QPair>
#include <QString>
#include <QVector>

class QWidget;

namespace jp {

class Editor;
class MainWindow;

void pageSetupDialog(QWidget *p, Editor *ed);
// Layout Guides: Margin Guides (tab 0), Grid Guides (1), Baseline Guides (2).
void gridGuidesDialog(QWidget *p, Editor *ed, int startTab = 1);
// Ruler Guides: the page's horizontal and vertical guides by position;
// add one or a series, move, remove.
void rulerGuidesDialog(QWidget *p, Editor *ed);
// Insert > Barcode, and what it places: the bars and digits as one vector
// outline (barcode-local points), grouped on a white quiet zone.
void barcodeDialog(QWidget *p, Editor *ed);
QPainterPath barcodePath(const barcode::Layout &l);
ItemPtr barcodeItem(const barcode::Layout &l, const QPointF &topLeft, bool whiteBackground, const QString &description,
                    const QJsonObject &settings = QJsonObject());
// Page sizes the user created (name and whole setup), and their dialogs.
QVector<QPair<QString, PageSetup>> customPageSizes();
// With apply false the new size is only saved (for a new publication).
bool createPageSizeDialog(QWidget *p, Editor *ed, int editIndex = -1, bool apply = true);
void customPageSizesDialog(QWidget *p, Editor *ed);
void applyPageSetup(Editor *ed, const PageSetup &setup, const QString &sizeName, const QString &undoName);
// The Size gallery's standard sizes: the page keeps its orientation.
void applyPageSize(Editor *ed, const QSizeF &size, const QString &sizeName);
void dropCapDialog(QWidget *p, Editor *ed);
void textGradientDialog(QWidget *p, Editor *ed);   // Text Fill > Gradient
void formatObjectDialog(QWidget *p, Editor *ed, int tab = 0);
void insertPageDialog(QWidget *p, Editor *ed);
void bookmarkDialog(QWidget *p, Editor *ed);
void businessInfoDialog(QWidget *p, Editor *ed);
// Change Template, after a design is chosen: 1 to apply it to the current
// publication, 2 to make a new publication with this one's text and graphics
// in it, 0 when cancelled.
int changeTemplateDialog(QWidget *p);
void colorSchemeDialog(QWidget *p, Editor *ed);
void fontSchemeDialog(QWidget *p, Editor *ed);
void shadowDialog(QWidget *p, Editor *ed);
void calendarDialog(QWidget *p, Editor *ed);
void styleDialog(QWidget *p, Editor *ed, const QString &styleName = QString());
void optionsDialog(QWidget *p, Editor *ed);
void pageNumberDialog(QWidget *p, Editor *ed);
// Format Page Numbers: the numbers' style and the first page's number.
void pageNumberFormatDialog(QWidget *p, Editor *ed);
void documentPropertiesDialog(QWidget *p, Editor *ed, int tab = 0);   // 1 = Commercial Print
void measurementWindow(QWidget *p, Editor *ed);
void hyphenationDialog(QWidget *p, Editor *ed);
void pasteSpecialDialog(QWidget *p, Editor *ed);
void insertFileDialog(QWidget *p, Editor *ed);
int autoflowText(Editor *ed, const QString &boxId);   // new pages and linked boxes until the story fits; returns pages added
void recipientsDialog(QWidget *p, Editor *ed, bool typeNew);
void mergeFieldDialog(QWidget *p, Editor *ed, int kind);   // 0 address block, 1 greeting line

} // namespace jp

// Opening a text file of values: the first bytes of the file are read (never more than a megabyte), the person is asked how it is split into fields and columns with a preview of the first
// rows, and the choices go to the reader with the file. A window made for a test does not ask unless told to (the file is detected); the program's windows ask.

#include "app/crunchwindow.h"

#include "app/textimportdialog.h"
#include "core/appinfo.h"
#include "sheet/csv.h"

#include <QFile>
#include <QFileInfo>

namespace jp {

bool CrunchWindow::askTextImport(const QString &path, std::shared_ptr<const jp::office::sheet::CsvImport> *chosen)
{
    TextImportRequest req;
    req.path = path;
    req.locale = QLocale();
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        notice(tr("Open"), tr("%1 can't open \"%2\".\nThe file can't be read.").arg(appInfo().name, QFileInfo(path).fileName()));
        return false;
    }
    req.size = f.size();
    req.head = f.read(jp::office::sheet::kCsvPreviewBytes);
    f.close();
    if (m_anchorHooks.textImportAsker) {
        if (!m_anchorHooks.textImportAsker(&req)) return false;
    } else {
        TextImportDialog dialog(req, this);
        if (dialog.exec() != QDialog::Accepted) return false;
        req.choices = dialog.choices();
    }
    *chosen = std::make_shared<const jp::office::sheet::CsvImport>(req.choices);
    return true;
}

} // namespace jp

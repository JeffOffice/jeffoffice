#include "app/getdata.h"

#include "app/uistyle.h"
#include "core/workthread.h"
#include "sheet/datasourcebook.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace jp {

using namespace jp::office::sheet;

namespace {

QString kindName(char k)
{
    switch (k) {
    case 'n': return QCoreApplication::translate("jp::GetDataDialog", "number");
    case 'b': return QCoreApplication::translate("jp::GetDataDialog", "true or false");
    case 'd': return QCoreApplication::translate("jp::GetDataDialog", "date");
    default: return QCoreApplication::translate("jp::GetDataDialog", "text");
    }
}

}   // namespace

QString guessGetDataKind(const QString &path)
{
    const QString ending = QFileInfo(path).suffix().toLower();
    if (ending == QLatin1String("json")) return QStringLiteral("json");
    if (ending == QLatin1String("xml")) return QStringLiteral("xml");
    for (const char *w : {"xlsx", "xlsm", "xltx", "xltm", "xls", "ods", "ots"})
        if (ending == QLatin1String(w)) return QStringLiteral("workbook");
    QFile f(path);
    if (!dataPathIsRemote(path) && QFileInfo(path).isFile() && f.open(QIODevice::ReadOnly)) {
        const QByteArray head = f.read(256).trimmed();
        if (head.startsWith('{') || head.startsWith('[')) return QStringLiteral("json");
        if (head.startsWith('<')) return QStringLiteral("xml");
    }
    return QString();
}

GetDataFetch fetchGetData(const DataConnection &conn, int previewRows, const std::function<bool()> &cancelled)
{
    GetDataFetch out;
    const DataLimits limits;
    if (conn.kind == QLatin1String("workbook")) {
        out.lists = dataBookLists(conn.path, limits, cancelled, &out.why);
        if (out.lists.isEmpty()) return out;
        const QString list = conn.list.isEmpty() ? out.lists.first() : conn.list;
        out.why.clear();
        if (!dataReadBook(conn.path, list, previewRows, limits, cancelled, &out.rows, &out.why)) return out;
        out.kinds = dataColumnKinds(out.rows, conn.asText);
        out.ok = true;
        return out;
    }
    QByteArray bytes;
    if (!dataReadFile(conn.path, limits, cancelled, &bytes, &out.why)) return out;
    if (cancelled && cancelled()) {
        out.why = QCoreApplication::translate("jp::GetDataDialog", "The read was stopped.");
        return out;
    }
    if (conn.kind == QLatin1String("json")) {
        out.lists = dataJsonLists(bytes, limits, &out.why);
        if (out.lists.isEmpty()) return out;
        const QString list = conn.list.isEmpty() ? out.lists.first() : conn.list;
        out.why.clear();
        if (!dataReadJson(bytes, list, conn.mode, previewRows, limits, &out.rows, &out.why)) return out;
    } else if (conn.kind == QLatin1String("xml")) {
        out.lists = dataXmlLists(bytes, limits, &out.why);
        if (out.lists.isEmpty()) return out;
        const QString list = conn.list.isEmpty() ? out.lists.first() : conn.list;
        out.why.clear();
        if (!dataReadXml(bytes, list, previewRows, limits, &out.rows, &out.why)) return out;
    } else {
        out.why = QCoreApplication::translate("jp::GetDataDialog", "That kind of file can't be loaded.");
        return out;
    }
    out.kinds = dataColumnKinds(out.rows, conn.asText);
    out.ok = true;
    return out;
}

bool runGetDataWork(QWidget *parent, const QString &text, std::atomic<bool> *cancel, const std::function<void()> &work)
{
    cancel->store(false);
    std::atomic<bool> done{false};
    jp::WorkThread worker([&] {
        work();
        done.store(true);
    });
    QProgressDialog dialog(text, QObject::tr("Cancel"), 0, 0, parent);
    dialog.setObjectName(QStringLiteral("getDataProgress"));
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setMinimumDuration(400);
    bool cancelled = false;
    QObject::connect(&dialog, &QProgressDialog::canceled, &dialog, [&] {
        cancelled = true;
        cancel->store(true);
    });
    QEventLoop loop;
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &loop, [&] {
        if (done.load()) loop.quit();
    });
    timer.start(10);
    loop.exec();
    worker.join();
    return !cancelled;
}

GetDataDialog::GetDataDialog(const QString &place, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("getDataDialog"));
    setWindowTitle(tr("Get Data from a File"));
    auto *layout = new QVBoxLayout(this);
    auto *about = new QLabel(tr("Loads the rows of a JSON file, an XML file, or a workbook as a table. The table remembers the file, so Refresh can read it again."), this);
    about->setWordWrap(true);
    layout->addWidget(about);
    auto *fileRow = new QHBoxLayout;
    m_path = new QLineEdit(this);
    m_path->setObjectName(QStringLiteral("getDataPath"));
    m_path->setAccessibleName(tr("File"));
    m_path->setPlaceholderText(tr("The file on this computer"));
    auto *browse = new QPushButton(tr("Browse…"), this);
    browse->setObjectName(QStringLiteral("getDataBrowse"));
    fileRow->addWidget(new QLabel(tr("File"), this));
    fileRow->addWidget(m_path, 1);
    fileRow->addWidget(browse);
    layout->addLayout(fileRow);
    auto *listRow = new QHBoxLayout;
    m_list = new QComboBox(this);
    m_list->setObjectName(QStringLiteral("getDataList"));
    m_list->setAccessibleName(tr("List to load"));
    m_mode = new QComboBox(this);
    m_mode->setObjectName(QStringLiteral("getDataMode"));
    m_mode->setAccessibleName(tr("Lists of values"));
    m_mode->addItem(tr("Join lists of values into one cell"), QStringLiteral("join"));
    m_mode->addItem(tr("Spread lists of values over columns"), QStringLiteral("columns"));
    listRow->addWidget(new QLabel(tr("List to load"), this));
    listRow->addWidget(m_list, 1);
    listRow->addWidget(m_mode, 1);
    layout->addLayout(listRow);
    m_asText = new QCheckBox(tr("Load every column as text"), this);
    m_asText->setObjectName(QStringLiteral("getDataAsText"));
    layout->addWidget(m_asText);
    m_preview = new QTableWidget(this);
    m_preview->setObjectName(QStringLiteral("getDataPreview"));
    m_preview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_preview->setMinimumSize(480, 200);
    m_preview->setAccessibleName(tr("Preview of the first rows"));
    layout->addWidget(m_preview, 1);
    m_message = new QLabel(this);
    m_message->setObjectName(QStringLiteral("getDataMessage"));
    m_message->setWordWrap(true);
    m_message->setTextFormat(Qt::PlainText);
    layout->addWidget(m_message);
    auto *placeRow = new QHBoxLayout;
    m_place = new QLineEdit(place, this);
    m_place->setObjectName(QStringLiteral("getDataPlace"));
    m_place->setAccessibleName(tr("Put the table at"));
    placeRow->addWidget(new QLabel(tr("Put the table at"), this));
    placeRow->addWidget(m_place);
    placeRow->addStretch(1);
    layout->addLayout(placeRow);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_load = buttons->button(QDialogButtonBox::Ok);
    m_load->setText(tr("Load"));
    m_load->setEnabled(false);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
    connect(browse, &QPushButton::clicked, this, [this] {
        const QString p = QFileDialog::getOpenFileName(this, tr("Get Data from a File"), QString(),
                                                       tr("Data files (*.json *.xml *.xlsx *.xlsm *.xls *.ods)") + QStringLiteral(";;") + tr("All Files (*)"));
        if (!p.isEmpty()) setPath(p);
    });
    connect(m_path, &QLineEdit::editingFinished, this, [this] { refreshPreview(); });
    connect(m_list, &QComboBox::currentIndexChanged, this, [this] {
        if (!m_loadingLists) refreshPreview();
    });
    connect(m_mode, &QComboBox::currentIndexChanged, this, [this] { refreshPreview(); });
    connect(m_asText, &QCheckBox::toggled, this, [this] { refreshPreview(); });
    m_mode->hide();
}

void GetDataDialog::setPath(const QString &path)
{
    m_path->setText(path);
    m_list->clear();
    refreshPreview();
}

void GetDataDialog::chooseList(const QString &list)
{
    const int i = m_list->findData(list);
    if (i >= 0) m_list->setCurrentIndex(i);
}

GetDataRequest GetDataDialog::request() const
{
    GetDataRequest r;
    r.conn.kind = m_kind;
    r.conn.path = m_path->text().trimmed();
    r.conn.list = m_list->currentData().toString();
    r.conn.mode = m_mode->currentData().toString() == QLatin1String("columns") ? ArrayMode::Columns : ArrayMode::Join;
    r.conn.asText = m_asText->isChecked();
    r.place = m_place->text().trimmed();
    return r;
}

QString GetDataDialog::message() const { return m_message->text(); }
int GetDataDialog::previewRows() const { return m_preview->rowCount(); }
int GetDataDialog::previewColumns() const { return m_preview->columnCount(); }
QString GetDataDialog::previewCell(int row, int column) const { return m_preview->item(row, column) ? m_preview->item(row, column)->text() : QString(); }
QStringList GetDataDialog::listNames() const
{
    QStringList out;
    for (int i = 0; i < m_list->count(); ++i) out << m_list->itemData(i).toString();
    return out;
}

// Reads the first rows (on a thread of its own) and shows them; nothing on the sheet is touched.
void GetDataDialog::refreshPreview()
{
    m_preview->clear();
    m_preview->setRowCount(0);
    m_preview->setColumnCount(0);
    m_load->setEnabled(false);
    const QString path = m_path->text().trimmed();
    if (path.isEmpty()) {
        m_message->clear();
        return;
    }
    m_kind = guessGetDataKind(path);
    m_mode->setVisible(m_kind == QLatin1String("json"));
    if (dataPathIsRemote(path)) {
        m_message->setText(tr("Only a file on this computer can be loaded, not a web address or a file on another computer."));
        return;
    }
    if (m_kind.isEmpty()) {
        m_message->setText(QFileInfo(path).isFile() ? tr("That file is not JSON, XML, or a workbook.") : tr("The file is not there."));
        return;
    }
    const GetDataRequest req = request();
    GetDataFetch result;
    std::atomic<bool> cancel{false};
    const bool finished = runGetDataWork(this, tr("Reading the file…"), &cancel, [&] { result = fetchGetData(req.conn, 20, [&cancel] { return cancel.load(); }); });
    if (!finished) {
        m_message->setText(tr("The read was stopped."));
        return;
    }
    if (!result.ok) {
        m_message->setText(result.why);
        return;
    }
    if (m_list->count() == 0) {
        m_loadingLists = true;
        for (const QString &l : result.lists) {
            QString label = l;
            if (l.startsWith(QLatin1String("sheet:"))) label = tr("Sheet %1").arg(l.mid(6));
            else if (l.startsWith(QLatin1String("table:"))) label = tr("Table %1").arg(l.mid(6));
            else if (l.startsWith(QLatin1String("name:"))) label = tr("Named range %1").arg(l.mid(5));
            m_list->addItem(label, l);
        }
        m_loadingLists = false;
    }
    m_list->setEnabled(m_list->count() > 1);
    m_preview->setColumnCount(result.rows.header.size());
    QStringList labels;
    for (int c = 0; c < result.rows.header.size(); ++c) labels << result.rows.header[c] + QStringLiteral(" (") + kindName(result.kinds.value(c, 't')) + QLatin1Char(')');
    m_preview->setHorizontalHeaderLabels(labels);
    m_preview->setRowCount(result.rows.rows.size());
    for (int r = 0; r < result.rows.rows.size(); ++r)
        for (int c = 0; c < result.rows.header.size(); ++c) m_preview->setItem(r, c, new QTableWidgetItem(result.rows.rows[r].value(c).text));
    m_preview->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    QString text = result.rows.rows.size() >= 20 ? tr("The first 20 rows are shown.") : tr("All the rows are shown.");
    if (!result.rows.notes.isEmpty()) text += QLatin1Char(' ') + result.rows.notes.join(QLatin1Char(' '));
    m_message->setText(text);
    m_load->setEnabled(true);
}

}   // namespace jp

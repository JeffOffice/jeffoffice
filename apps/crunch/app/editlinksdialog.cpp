#include "app/editlinksdialog.h"

#include "app/plaintext.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QGuiApplication>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace jp {

EditLinksDialog::EditLinksDialog(crunch::EditSession *session, const QString &baseDir, QWidget *parent) : QDialog(parent), m_session(session), m_baseDir(baseDir)
{
    setObjectName(QStringLiteral("editLinksDialog"));
    setWindowTitle(tr("Edit Links"));
    auto *layout = new QVBoxLayout(this);
    m_note = plainLabel(new QLabel);
    m_note->setWordWrap(true);
    layout->addWidget(m_note);
    m_table = new QTableWidget(0, 3, this);
    m_table->setObjectName(QStringLiteral("editLinksTable"));
    m_table->setHorizontalHeaderLabels({tr("Source"), tr("Type"), tr("Status")});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    layout->addWidget(m_table);
    auto *row = new QHBoxLayout;
    m_open = new QPushButton(tr("Open Source"), this);
    m_open->setObjectName(QStringLiteral("editLinksOpen"));
    auto *check = new QPushButton(tr("Check Status"), this);
    check->setObjectName(QStringLiteral("editLinksCheck"));
    m_break = new QPushButton(tr("Break Link"), this);
    m_break->setObjectName(QStringLiteral("editLinksBreak"));
    m_update = new QPushButton(tr("Update Values"), this);
    m_update->setObjectName(QStringLiteral("editLinksUpdate"));
    m_change = new QPushButton(tr("Change Source…"), this);
    m_change->setObjectName(QStringLiteral("editLinksChange"));
    row->addWidget(m_update);
    row->addWidget(m_change);
    row->addWidget(m_open);
    row->addWidget(check);
    row->addWidget(m_break);
    row->addStretch(1);
    layout->addLayout(row);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(check, &QPushButton::clicked, this, [this] { checkStatus(); });
    connect(m_open, &QPushButton::clicked, this, [this] {
        const int at = m_table->currentRow();
        if (at < 0 || at >= int(m_sources.size())) return;
        const QString path = localPath(m_sources[size_t(at)], m_baseDir);
        if (path.isEmpty() || !QFileInfo::exists(path)) {
            plainInformation(this, windowTitle(), tr("That workbook can't be found on this computer."));
            return;
        }
        Q_EMIT openRequested(path);
    });
    connect(m_break, &QPushButton::clicked, this, [this] { breakRow(m_table->currentRow()); });
    connect(m_update, &QPushButton::clicked, this, [this] { updateRow(m_table->currentRow()); });
    connect(m_change, &QPushButton::clicked, this, [this] { changeRow(m_table->currentRow()); });
    auto sync = [this] {
        const bool one = m_table->currentRow() >= 0 && m_table->currentRow() < int(m_sources.size());
        m_open->setEnabled(one);
        m_break->setEnabled(one);
        m_update->setEnabled(one);
        m_change->setEnabled(one);
    };
    connect(m_table, &QTableWidget::currentCellChanged, this, sync);
    resize(620, 300);
    reload();
    if (m_table->rowCount() > 0) m_table->selectRow(0);
    sync();
}

QString EditLinksDialog::localPath(const crunch::EditSession::ExternalSource &source, const QString &baseDir)
{
    if (!source.local || source.shown.isEmpty()) return QString();
    const QFileInfo info(source.shown);
    if (info.isAbsolute()) return source.shown;
    return baseDir.isEmpty() ? QString() : QDir(baseDir).absoluteFilePath(source.shown);
}

void EditLinksDialog::reload()
{
    m_sources = m_session->externalLinkSources();
    m_note->setText(m_sources.empty() ? tr("This workbook does not read values from other workbooks.")
                                      : tr("This workbook reads values from these workbooks. Until it is opened again with them, it shows the values its file stored."));
    m_table->setRowCount(int(m_sources.size()));
    for (int r = 0; r < int(m_sources.size()); ++r) {
        const crunch::EditSession::ExternalSource &s = m_sources[size_t(r)];
        m_table->setItem(r, 0, new QTableWidgetItem(s.shown.isEmpty() ? tr("(no place)") : s.shown));
        m_table->setItem(r, 1, new QTableWidgetItem(s.local ? tr("Workbook") : tr("Workbook on the web")));
        m_table->setItem(r, 2, new QTableWidgetItem(tr("Not checked")));
    }
}

void EditLinksDialog::checkStatus()
{
    for (int r = 0; r < int(m_sources.size()); ++r) {
        const crunch::EditSession::ExternalSource &s = m_sources[size_t(r)];
        QString text;
        if (!s.local) text = tr("Not checked: it is on the web");
        else {
            const QString path = localPath(s, m_baseDir);
            text = !path.isEmpty() && QFileInfo::exists(path) ? tr("Found") : tr("Not found: the stored values are shown");
        }
        if (QTableWidgetItem *item = m_table->item(r, 2)) item->setText(text);
    }
}

QString EditLinksDialog::note() const { return m_note->text(); }

bool EditLinksDialog::updateRow(int row)
{
    if (row < 0 || row >= int(m_sources.size())) return false;
    const crunch::EditSession::ExternalSource s = m_sources[size_t(row)];
    const QString path = localPath(s, m_baseDir);
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        m_note->setText(tr("That workbook can't be found on this computer, so its values are not updated. Use Change Source to point to it."));
        return false;
    }
    crunch::EditSession::ExternalUpdate done;
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    const crunch::EditSession::Outcome o = m_session->updateExternalValues(s.index, path, &done);
    QGuiApplication::restoreOverrideCursor();
    if (!o.ok) {
        m_note->setText(o.problem);
        return false;
    }
    m_note->setText(done.kept > 0 ? tr("Cells given the values the source has now: %1. Cells that combine them with other values keep the values their file stored: %2.").arg(done.updated).arg(done.kept)
                                  : tr("Cells given the values the source has now: %1.").arg(done.updated));
    if (done.cut) m_note->setText(m_note->text() + QLatin1Char(' ') + tr("...and more cells were not read: there are too many."));
    Q_EMIT linksChanged();
    return true;
}

bool EditLinksDialog::changeRow(int row)
{
    if (row < 0 || row >= int(m_sources.size())) return false;
    const crunch::EditSession::ExternalSource s = m_sources[size_t(row)];
    const QString from = localPath(s, m_baseDir);
    const QString start = !from.isEmpty() ? QFileInfo(from).absolutePath() : m_baseDir;
    const QString path = m_chooser ? m_chooser(start) : QFileDialog::getOpenFileName(this, tr("Change Source"), start, tr("Workbooks (*.xlsx *.xlsm *.xls *.ods);;All files (*)"));
    if (path.isEmpty()) return false;
    const crunch::EditSession::Outcome o = m_session->changeExternalSource(s.index, path);
    if (!o.ok) {
        plainWarning(this, windowTitle(), o.problem);
        return false;
    }
    reload();
    m_table->selectRow(row);
    m_note->setText(tr("The link now points to the workbook \"%1\". Use Update Values to read its values.").arg(QFileInfo(path).fileName()));
    Q_EMIT linksChanged();
    return true;
}

bool EditLinksDialog::breakRow(int row, bool ask)
{
    if (row < 0 || row >= int(m_sources.size())) return false;
    const crunch::EditSession::ExternalSource s = m_sources[size_t(row)];
    if (ask) {
        const auto answer = plainQuestion(this, windowTitle(), tr("Break the links to \"%1\"? Each cell that reads it keeps the value it shows now. Undo brings the links back until you save the workbook.").arg(s.shown));
        if (answer != QMessageBox::Yes) return false;
    }
    const crunch::EditSession::Outcome o = m_session->breakExternalLinks(nullptr, nullptr, s.index);
    if (!o.ok) {
        plainWarning(this, windowTitle(), o.problem);
        return false;
    }
    reload();
    Q_EMIT linksBroken();
    return true;
}

} // namespace jp

#include "app/connectionspane.h"

#include "app/plaintext.h"
#include "app/uistyle.h"
#include "sheet/datasource.h"

#include <QDateTime>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace jp {

using jp::office::sheet::DataConnection;

ConnectionsPane::ConnectionsPane(crunch::EditSession *session, bool canChange, QWidget *parent) : QDialog(parent), m_session(session), m_canChange(canChange)
{
    setObjectName(QStringLiteral("connectionsPane"));
    setWindowTitle(tr("Queries & Connections"));
    resize(760, 360);
    auto *layout = new QVBoxLayout(this);
    m_note = plainLabel(new QLabel);
    m_note->setWordWrap(true);
    layout->addWidget(m_note);
    m_table = new QTableWidget(0, 5, this);
    m_table->setObjectName(QStringLiteral("connectionsTable"));
    m_table->setHorizontalHeaderLabels({tr("Name"), tr("Source file"), tr("Last run"), tr("Rows"), tr("State")});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    for (int c : {0, 2, 3, 4}) m_table->horizontalHeader()->setSectionResizeMode(c, QHeaderView::ResizeToContents);
    layout->addWidget(m_table);
    auto *row = new QHBoxLayout;
    m_refresh = new QPushButton(tr("Refresh"), this);
    m_refresh->setObjectName(QStringLiteral("connectionsRefresh"));
    m_refreshAll = new QPushButton(tr("Refresh All"), this);
    m_refreshAll->setObjectName(QStringLiteral("connectionsRefreshAll"));
    row->addWidget(m_refresh);
    row->addWidget(m_refreshAll);
    row->addStretch(1);
    layout->addLayout(row);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Close, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, [this] { updateButtons(); });
    connect(m_refresh, &QPushButton::clicked, this, [this] {
        const int at = tableOfRow(m_table->currentRow());
        if (at >= 0) Q_EMIT refreshRequested(QVector<int>{at}, false);
    });
    connect(m_refreshAll, &QPushButton::clicked, this, [this] {
        QVector<int> all;
        for (int t : m_tables) {
            DataConnection conn;
            if (DataConnection::parse(m_session->workbook()->tables().value(t).dataSource, &conn) && conn.stepsProblem.isEmpty()) all << t;
        }
        if (!all.isEmpty()) Q_EMIT refreshRequested(all, true);
    });
    reload();
}

QString ConnectionsPane::note() const { return m_note->text(); }
void ConnectionsPane::setNote(const QString &text) { m_note->setText(text); }

void ConnectionsPane::reload()
{
    const int keep = m_table->currentRow();
    m_tables.clear();
    m_table->setRowCount(0);
    const auto &tables = m_session->workbook()->tables();
    for (int t = 0; t < tables.size(); ++t) {
        if (tables[t].dataSource.isEmpty()) continue;
        DataConnection conn;
        const bool readable = DataConnection::parse(tables[t].dataSource, &conn);
        const int r = m_table->rowCount();
        m_table->insertRow(r);
        m_tables << t;
        QString state = tr("Ready");
        if (!readable) state = tr("Made by a newer version, can't be run");
        else if (!conn.stepsProblem.isEmpty()) state = tr("Steps can't be run: made by a newer version");
        const QString last = conn.ranAt > 0 ? QLocale().toString(QDateTime::fromSecsSinceEpoch(conn.ranAt), QLocale::ShortFormat) : tr("Not recorded");
        const QString rows = conn.rowCount >= 0 ? QLocale().toString(conn.rowCount) : QString();
        const QStringList cells{conn.name.isEmpty() ? tables[t].name : conn.name, readable ? conn.path : QString(), readable ? last : QString(), readable ? rows : QString(), state};
        for (int c = 0; c < cells.size(); ++c) {
            auto *item = new QTableWidgetItem(cells[c]);
            if (c == 1) item->setToolTip(jp::plainToolTip(cells[c]));
            m_table->setItem(r, c, item);
        }
    }
    if (m_table->rowCount() > 0) m_table->selectRow(std::min(std::max(keep, 0), m_table->rowCount() - 1));
    m_note->setText(m_tables.isEmpty() ? tr("This workbook has no queries. Get Data > From File makes one.") : (m_canChange ? QString() : tr("This workbook is read-only, so a query can't be run.")));
    updateButtons();
}

void ConnectionsPane::updateButtons()
{
    const int at = tableOfRow(m_table->currentRow());
    bool runnable = false, any = false;
    for (int t : m_tables) {
        DataConnection conn;
        if (DataConnection::parse(m_session->workbook()->tables().value(t).dataSource, &conn) && conn.stepsProblem.isEmpty()) {
            any = true;
            if (t == at) runnable = true;
        }
    }
    m_refresh->setEnabled(m_canChange && runnable);
    m_refreshAll->setEnabled(m_canChange && any);
}

}   // namespace jp

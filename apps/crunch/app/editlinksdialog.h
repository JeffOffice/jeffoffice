#pragma once
// Edit Links: the workbooks this one reads, each with where it is and whether it can be found, and the ways to open one or break the links to it.

#include "core/editsession.h"

#include <QDialog>

class QLabel;
class QPushButton;
class QTableWidget;

namespace jp {

class EditLinksDialog : public QDialog {
    Q_OBJECT
public:
    // `baseDir` is the folder of this workbook, where a source named with no folder is looked for.
    EditLinksDialog(crunch::EditSession *session, const QString &baseDir, QWidget *parent = nullptr);
    QTableWidget *table() const { return m_table; }
    // Looks for each source: the status column says found, not found, or not checked (an address on the web is not fetched).
    void checkStatus();
    // The place of a source's file on this computer, or empty (a web address, or a name with no folder and no base folder).
    static QString localPath(const crunch::EditSession::ExternalSource &source, const QString &baseDir);
    // Breaks the links to the source in the row (the person is asked first, unless `ask` is false); true if they were.
    bool breakRow(int row, bool ask = true);

Q_SIGNALS:
    void openRequested(const QString &path);
    void linksBroken();

private:
    void reload();
    crunch::EditSession *m_session;
    QString m_baseDir;
    QTableWidget *m_table = nullptr;
    QPushButton *m_open = nullptr, *m_break = nullptr;
    QLabel *m_note = nullptr;
    std::vector<crunch::EditSession::ExternalSource> m_sources;
};

} // namespace jp

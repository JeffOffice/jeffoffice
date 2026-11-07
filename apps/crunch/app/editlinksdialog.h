#pragma once
// Edit Links: the workbooks this one reads, each with where it is and whether it can be found, and the ways to open one or break the links to it.

#include "core/editsession.h"

#include <QDialog>

#include <functional>

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
    // Gives the cells that read the source in the row the values the source has now (the file on this computer is read); false, with the reason in the note, if it can't be.
    bool updateRow(int row);
    // Points the source in the row at another file: the one the chooser gives (it is asked with the folder to begin in; a test gives its own); false if none is chosen.
    bool changeRow(int row);
    void setSourceChooser(std::function<QString(const QString &)> chooser) { m_chooser = std::move(chooser); }
    QString note() const;

Q_SIGNALS:
    void openRequested(const QString &path);
    void linksBroken();
    void linksChanged();   // values were updated or a source was changed

private:
    void reload();
    crunch::EditSession *m_session;
    QString m_baseDir;
    QTableWidget *m_table = nullptr;
    QPushButton *m_open = nullptr, *m_break = nullptr, *m_update = nullptr, *m_change = nullptr;
    std::function<QString(const QString &)> m_chooser;
    QLabel *m_note = nullptr;
    std::vector<crunch::EditSession::ExternalSource> m_sources;
};

} // namespace jp

#pragma once
// The dialogs of notes and links, and what the window passes them: a note's words, and a link's kind, address, shown text, and screen tip. The window asks for them through functions it is
// given (a test gives its own); these are the ones it uses by default.

#include "app/protectdialogs.h"
#include "app/textimportdialog.h"
#include "sheet/links.h"

#include <QDialog>
#include <QStringList>

#include <functional>

class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QStackedWidget;
class QLabel;

namespace jp {

struct NoteRequest {
    QString where;          // the cell, as a person reads it ("B7")
    QString text;           // in: the words now (empty for a new note); out: what the person made them
    QString author;
    bool readOnly = false;  // a note that belongs to a conversation of comments: shown, not changed
};

struct LinkRequest {
    QString where;                               // the cell or cells ("B7", "B7:C9")
    QString shownText;                           // the text the first cell holds
    bool editing = false;                        // there is a link already (the link below is it)
    jp::office::sheet::LinkSpec link;            // in: the link now; out: what the person made it
    QString shownTextOut;                        // out: the text to put in the cell (empty: leave the cell as it is)
    QStringList sheetNames;                      // the workbook's sheets, in order of the tabs
    QStringList names;                           // the defined names a link can go to
    int sheet = 0;                               // the position of the sheet the cell is on, among sheetNames
    QStringList recentFiles;                     // the files used lately (a link to one is a choice in the dialog)
};

// The question put when a file that is protected with a password is opened: `wrong` is true when the password just given was not it (the question is put again).
struct FilePasswordRequest {
    QString file;       // the file's name, as a person knows it
    bool wrong = false;
    QString password;   // out
};

// The question put when a person protects the workbook with a password (or changes or removes it): `hasPassword` says the workbook has one now.
struct SetFilePasswordRequest {
    bool hasPassword = false;
    QString password;      // out: the new password
    bool remove = false;   // out: the person chose to remove the password
};

// The functions the window asks, and how it opens an address. A function left empty makes the window use the dialog.
struct AnchorHooks {
    std::function<bool(NoteRequest *)> noteAsker;
    std::function<bool(LinkRequest *)> linkAsker;
    // Opens an address outside the program (the default hands it to the system's opener, after the person has clicked). Returns whether it was opened.
    std::function<bool(const QUrl &)> opener;
    // Protection: the dialog that protects a sheet (the settings, and the password typed), the one that locks the workbook's structure, and the question for a password.
    std::function<bool(ProtectRequest *)> protectAsker;
    std::function<bool(WorkbookProtectRequest *)> workbookProtectAsker;
    std::function<bool(PasswordRequest *)> passwordAsker;
    // Allow Edit Ranges: the dialog that lists, adds, changes, and deletes the ranges a protected sheet leaves open.
    std::function<bool(EditRangesRequest *)> editRangesAsker;
    // Opening a text file of values: the dialog that says how it is read (the request holds the file's first bytes and what it is found to be).
    std::function<bool(TextImportRequest *)> textImportAsker;
    // Opening a file that is protected with a password: the question (false: the person cancelled).
    std::function<bool(FilePasswordRequest *)> filePasswordAsker;
    // Protecting the workbook with a password: the dialog that takes the password twice, or removes it (false: the person cancelled).
    std::function<bool(SetFilePasswordRequest *)> setFilePasswordAsker;
};

class NoteDialog : public QDialog {
    Q_OBJECT
public:
    explicit NoteDialog(const NoteRequest &request, QWidget *parent = nullptr);
    QString text() const;

private:
    QPlainTextEdit *m_text = nullptr;
};

class LinkDialog : public QDialog {
    Q_OBJECT
public:
    explicit LinkDialog(const LinkRequest &request, QWidget *parent = nullptr);
    // Fills `request` with what the dialog says; false (with `problem` filled) if what it says is not a link.
    bool result(LinkRequest *request, QString *problem) const;

private:
    QComboBox *m_kind = nullptr, *m_sheet = nullptr, *m_name = nullptr, *m_recent = nullptr;
    QStackedWidget *m_pages = nullptr;
    QLineEdit *m_address = nullptr, *m_cell = nullptr, *m_mail = nullptr, *m_subject = nullptr, *m_shown = nullptr, *m_tip = nullptr;
    QLabel *m_note = nullptr;
    LinkRequest m_request;
};

} // namespace jp

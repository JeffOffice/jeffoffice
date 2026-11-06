#pragma once
// The dialogs of protection: what a protected sheet still lets people do (and its password), and what protecting the workbook's structure locks (and its password); and the box that asks
// for a password to take protection off. The window asks for them through the functions in AnchorHooks (a test gives its own).

#include "sheet/protection.h"

#include <QDialog>
#include <QMap>
#include <QStringList>
#include <QVector>

class QCheckBox;
class QLabel;
class QLineEdit;
class QListWidget;

namespace jp {

struct ProtectRequest {
    jp::office::sheet::SheetProtection settings;   // in: what the sheet has now (the defaults for a sheet with none); out: what the person chose
    QString password;                              // out: the password typed (empty for none); never kept past the call that makes its hash
    bool hadPassword = false;                      // in: the sheet already has a password (the one typed replaces it)
};

struct WorkbookProtectRequest {
    bool lockStructure = true;
    bool lockWindows = false;
    QString password;
    bool hadPassword = false;
};

// Allow Edit Ranges: the ranges a protected sheet leaves open.
struct EditRangesRequest {
    QVector<jp::office::sheet::EditRange> ranges;   // in: the sheet's; out: as chosen (a range whose password was not touched keeps its hash)
    QMap<QString, QString> passwords;               // out: the password typed for a range (by its title): new or changed; never kept past the call that makes its hash
    QStringList removePasswords;                    // out: the titles of ranges whose password the person took off
    QString selection;                              // in: the cells selected, which a new range starts with
    bool sheetProtected = false;                    // in: the sheet is protected now (the dialog says the ranges only matter then)
};

struct PasswordRequest {
    QString what;       // the thing to be unprotected, in words ("Sheet1")
    QString password;   // out
};

// The box that protects the whole file with a password: the password and its confirmation, and, when the file has one, a button that removes it.
class SetFilePasswordDialog : public QDialog {
    Q_OBJECT
public:
    explicit SetFilePasswordDialog(bool hasPassword, QWidget *parent = nullptr);
    QString password() const;
    bool removed() const { return m_removed; }
    // Empties the two boxes (the password is not left in them once it has been read).
    void clearPasswords();

private:
    void accept() override;
    QLineEdit *m_password = nullptr, *m_again = nullptr;
    QLabel *m_problem = nullptr;
    bool m_removed = false;
};

class ProtectSheetDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProtectSheetDialog(const ProtectRequest &request, QWidget *parent = nullptr);
    // What the dialog says; false (with `problem`) if the two passwords differ.
    bool result(ProtectRequest *request, QString *problem) const;

private:
    QLineEdit *m_password = nullptr, *m_again = nullptr;
    QLabel *m_problem = nullptr;
    QMap<QString, QCheckBox *> m_checks;
    ProtectRequest m_request;
};

class EditRangesDialog : public QDialog {
    Q_OBJECT
public:
    explicit EditRangesDialog(const EditRangesRequest &request, QWidget *parent = nullptr);
    EditRangesRequest result() const { return m_request; }
    QListWidget *list() const { return m_list; }
    // Adds a range (as New does, after the box that asks for it), or changes the one in row `row`; false, with `problem`, if the title is empty or used, or the cells are not a range.
    bool setRange(int row, const jp::office::sheet::EditRange &range, const QString &typedPassword, bool removePassword, QString *problem);
    void removeRow(int row);

private:
    void reload();
    void ask(int row);
    QListWidget *m_list = nullptr;
    EditRangesRequest m_request;
};

class ProtectWorkbookDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProtectWorkbookDialog(const WorkbookProtectRequest &request, QWidget *parent = nullptr);
    bool result(WorkbookProtectRequest *request, QString *problem) const;

private:
    QCheckBox *m_structure = nullptr, *m_windows = nullptr;
    QLineEdit *m_password = nullptr, *m_again = nullptr;
    QLabel *m_problem = nullptr;
};

} // namespace jp

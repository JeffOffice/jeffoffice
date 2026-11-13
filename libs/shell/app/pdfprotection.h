#pragma once
// The row every app's PDF options carry: "Encrypt with a password…", which asks (passworddialogs.h) and then says in a line that the PDF will be protected. The password is kept in
// this widget alone, for the one export, and wiped when the widget goes (it is never written to the settings or the recent lists). A standard that forbids encryption (PDF/A) is turned off
// and disabled while a password is set.

#include "core/export.h"
#include "io/pdfcrypt.h"

#include <QWidget>

class QAbstractButton;
class QLabel;
class QPushButton;

namespace jp {

class JO_COMMON_EXPORT PdfProtectionRow : public QWidget {
    Q_OBJECT

public:
    explicit PdfProtectionRow(QWidget *parent = nullptr);
    ~PdfProtectionRow() override;

    bool isProtected() const { return m_protection.isSet(); }
    // The protection to make the PDF with; the caller wipes its copy after use (PdfProtection::wipe).
    PdfProtection protection() const { return m_protection.copy(); }
    // Takes a copy of the protection (a row that starts from the choice made on another page).
    void setProtection(const PdfProtection &protection);
    // Forgets the password (the export is done): a password is for one export only.
    void clear();
    // These boxes are for standards that forbid encryption: each is unticked and disabled while a password is set, and `why` (the app's own words) joins the note.
    void forbiddenBy(const QList<QAbstractButton *> &boxes, const QString &why);

Q_SIGNALS:
    void protectionChanged(bool isProtected);

private:
    void ask();
    void refresh();
    PdfProtection m_protection;
    QPushButton *m_button = nullptr;
    QLabel *m_note = nullptr;
    QList<QAbstractButton *> m_boxes;
    QString m_why;
};

} // namespace jp

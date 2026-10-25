#include "app/anchordialogs.h"
#include "app/plaintext.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>

namespace jp {

using jp::office::sheet::LinkSpec;

namespace {
enum Kind { WebKind, PlaceKind, MailKind };
}

// ---- the note

NoteDialog::NoteDialog(const NoteRequest &request, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("noteDialog"));
    setWindowTitle(tr("Note on %1").arg(request.where));
    auto *layout = new QVBoxLayout(this);
    if (!request.author.isEmpty()) {
        auto *by = plainLabel(new QLabel(tr("Written by %1").arg(request.author), this));
        by->setObjectName(QStringLiteral("noteAuthor"));
        layout->addWidget(by);
    }
    m_text = new QPlainTextEdit(this);
    m_text->setObjectName(QStringLiteral("noteText"));
    m_text->setAccessibleName(tr("Note text"));
    m_text->setPlainText(request.text);
    m_text->setReadOnly(request.readOnly);
    m_text->setMinimumSize(320, 140);
    layout->addWidget(m_text);
    if (request.readOnly) {
        auto *why = new QLabel(tr("This note is a conversation of comments. JeffCrunch shows it and keeps it, but doesn't change it."), this);
        why->setWordWrap(true);
        why->setObjectName(QStringLiteral("noteReadOnly"));
        layout->addWidget(why);
    }
    auto *buttons = new QDialogButtonBox(request.readOnly ? QDialogButtonBox::Close : (QDialogButtonBox::Ok | QDialogButtonBox::Cancel), this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
    m_text->setFocus();
}

QString NoteDialog::text() const { return m_text->toPlainText(); }

// ---- the link

LinkDialog::LinkDialog(const LinkRequest &request, QWidget *parent) : QDialog(parent), m_request(request)
{
    setObjectName(QStringLiteral("linkDialog"));
    setWindowTitle(request.editing ? tr("Change Link") : tr("Put In a Link"));
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;
    m_shown = new QLineEdit(request.link.display.isEmpty() ? request.shownText : request.link.display, this);
    m_shown->setObjectName(QStringLiteral("linkShown"));
    m_shown->setAccessibleName(tr("Text to show"));
    form->addRow(tr("Text to show:"), m_shown);
    m_kind = new QComboBox(this);
    m_kind->setObjectName(QStringLiteral("linkKind"));
    m_kind->setAccessibleName(tr("Go to"));
    m_kind->addItem(tr("A web page or a file"), WebKind);
    m_kind->addItem(tr("A place in this workbook"), PlaceKind);
    m_kind->addItem(tr("An e-mail address"), MailKind);
    form->addRow(tr("Go to:"), m_kind);
    layout->addLayout(form);
    m_pages = new QStackedWidget(this);
    // a web page, or a file
    {
        auto *page = new QWidget(m_pages);
        auto *f = new QFormLayout(page);
        m_address = new QLineEdit(page);
        m_address->setObjectName(QStringLiteral("linkAddress"));
        m_address->setAccessibleName(tr("Address"));
        f->addRow(tr("Address:"), m_address);
        m_pages->addWidget(page);
    }
    // a place in the workbook
    {
        auto *page = new QWidget(m_pages);
        auto *f = new QFormLayout(page);
        m_sheet = new QComboBox(page);
        m_sheet->setObjectName(QStringLiteral("linkSheet"));
        m_sheet->setAccessibleName(tr("Sheet"));
        m_sheet->addItems(request.sheetNames);
        m_sheet->setCurrentIndex(std::max(0, std::min(request.sheet, int(request.sheetNames.size()) - 1)));
        f->addRow(tr("Sheet:"), m_sheet);
        m_cell = new QLineEdit(QStringLiteral("A1"), page);
        m_cell->setObjectName(QStringLiteral("linkCell"));
        m_cell->setAccessibleName(tr("Cell or cells"));
        f->addRow(tr("Cell or cells:"), m_cell);
        m_name = new QComboBox(page);
        m_name->setObjectName(QStringLiteral("linkName"));
        m_name->setAccessibleName(tr("Name"));
        m_name->addItem(tr("(none)"));
        m_name->addItems(request.names);
        f->addRow(tr("Or a name:"), m_name);
        m_pages->addWidget(page);
    }
    // an e-mail address
    {
        auto *page = new QWidget(m_pages);
        auto *f = new QFormLayout(page);
        m_mail = new QLineEdit(page);
        m_mail->setObjectName(QStringLiteral("linkMail"));
        m_mail->setAccessibleName(tr("E-mail address"));
        f->addRow(tr("E-mail address:"), m_mail);
        m_subject = new QLineEdit(page);
        m_subject->setObjectName(QStringLiteral("linkSubject"));
        m_subject->setAccessibleName(tr("Subject"));
        f->addRow(tr("Subject:"), m_subject);
        m_pages->addWidget(page);
    }
    layout->addWidget(m_pages);
    auto *tipForm = new QFormLayout;
    m_tip = new QLineEdit(request.link.tip, this);
    m_tip->setObjectName(QStringLiteral("linkTip"));
    m_tip->setAccessibleName(tr("Screen tip"));
    tipForm->addRow(tr("Screen tip:"), m_tip);
    layout->addLayout(tipForm);
    m_note = plainLabel(new QLabel(this));
    m_note->setObjectName(QStringLiteral("linkProblem"));
    m_note->setWordWrap(true);
    layout->addWidget(m_note);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        QString problem;
        LinkRequest probe = m_request;
        if (!result(&probe, &problem)) {
            m_note->setText(problem);
            return;
        }
        accept();
    });
    layout->addWidget(buttons);
    connect(m_kind, &QComboBox::currentIndexChanged, m_pages, &QStackedWidget::setCurrentIndex);
    // what the link says now
    const LinkSpec &l = request.link;
    if (request.editing) {
        if (!l.target.isEmpty() && l.target.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
            m_kind->setCurrentIndex(MailKind);
            QString rest = l.target.mid(7);
            const int q = int(rest.indexOf(QLatin1Char('?')));
            if (q >= 0) {
                static const QString key = QStringLiteral("subject=");
                const int s = int(rest.indexOf(key, q, Qt::CaseInsensitive));
                if (s >= 0) {
                    int e = int(rest.indexOf(QLatin1Char('&'), s));
                    m_subject->setText(QUrl::fromPercentEncoding(rest.mid(s + key.size(), e < 0 ? -1 : e - s - key.size()).toUtf8()));
                }
                rest = rest.left(q);
            }
            m_mail->setText(rest);
        } else if (!l.target.isEmpty()) {
            m_kind->setCurrentIndex(WebKind);
            m_address->setText(l.target);
        } else {
            m_kind->setCurrentIndex(PlaceKind);
            const int bang = int(l.location.lastIndexOf(QLatin1Char('!')));
            if (bang > 0) {
                QString sheetName = l.location.left(bang);
                if (sheetName.startsWith(QLatin1Char('\'')) && sheetName.endsWith(QLatin1Char('\'')) && sheetName.size() >= 2) sheetName = sheetName.mid(1, sheetName.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
                const int at = int(request.sheetNames.indexOf(sheetName));
                if (at >= 0) m_sheet->setCurrentIndex(at);
                m_cell->setText(l.location.mid(bang + 1));
            } else if (request.names.contains(l.location)) {
                m_name->setCurrentIndex(int(request.names.indexOf(l.location)) + 1);
            } else {
                m_cell->setText(l.location);
            }
        }
    }
    m_pages->setCurrentIndex(m_kind->currentIndex());
    (request.editing ? m_shown : m_address)->setFocus();
}

bool LinkDialog::result(LinkRequest *request, QString *problem) const
{
    LinkSpec link = request->link;
    link.target.clear();
    link.location.clear();
    link.tip = m_tip->text().trimmed();
    link.display = m_shown->text();
    switch (m_kind->currentData().toInt()) {
    case WebKind: {
        link.target = jp::office::sheet::webAddressFromTyped(m_address->text());
        if (link.target.isEmpty()) {
            *problem = tr("Type the address to go to.");
            return false;
        }
        if (jp::office::sheet::targetKind(link.target) == jp::office::sheet::LinkKind::Unsafe) {
            *problem = tr("That kind of address runs a script or carries a page inside itself, and JeffCrunch does not follow it. Use a web page, an e-mail address, or a file.");
            return false;
        }
        break;
    }
    case PlaceKind: {
        if (m_name->currentIndex() > 0) {
            link.location = m_name->currentText();
        } else {
            jp::office::sheet::CellRange r;
            if (!jp::office::sheet::parseRangeRef(m_cell->text().trimmed(), &r)) {
                *problem = tr("Type a cell like B7, or cells like B7:C9, or choose a name.");
                return false;
            }
            QString sheetName = m_sheet->currentText();
            const bool plain = !sheetName.isEmpty() && !sheetName.contains(QRegularExpression(QStringLiteral("[^A-Za-z0-9_.]"))) && !sheetName.at(0).isDigit();
            if (!plain) sheetName = QLatin1Char('\'') + sheetName.replace(QLatin1Char('\''), QStringLiteral("''")) + QLatin1Char('\'');
            link.location = sheetName + QLatin1Char('!') + jp::office::sheet::rangeName(r);
        }
        break;
    }
    case MailKind: {
        const QString a = m_mail->text().trimmed();
        if (a.isEmpty() || !a.contains(QLatin1Char('@'))) {
            *problem = tr("Type an e-mail address, like name@example.org.");
            return false;
        }
        link.target = QStringLiteral("mailto:") + a;
        if (!m_subject->text().trimmed().isEmpty()) link.target += QStringLiteral("?subject=") + QString::fromLatin1(QUrl::toPercentEncoding(m_subject->text().trimmed()));
        break;
    }
    }
    if (link.target.size() > jp::office::sheet::kMaxLinkTarget || link.location.size() > jp::office::sheet::kMaxLinkTarget) {
        *problem = tr("That address is too long for a link.");
        return false;
    }
    request->link = link;
    request->shownTextOut = m_shown->text();
    return true;
}

} // namespace jp

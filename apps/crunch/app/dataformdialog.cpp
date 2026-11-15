#include "app/dataformdialog.h"

#include "app/plaintext.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>

namespace jp {

DataFormDialog::DataFormDialog(crunch::EditSession *session, int sheet, quint32 row, quint32 col, const QString &title, QWidget *parent, qint64 regionWork) : QDialog(parent), m_title(title), m_session(session)
{
    setObjectName(QStringLiteral("dataForm"));
    m_ready = m_form.open(session, sheet, row, col, &m_problem, regionWork);
    if (!m_ready) return;
    build(title);
    showRecord(m_form.current());
    if (m_form.truncated()) {
        m_notice->setText(tr("This list is very long. The form shows only its first %1 rows.").arg(m_form.recordCount()));
        m_notice->show();
    }
}

void DataFormDialog::build(const QString &title)
{
    setWindowTitle(title);
    auto *outer = new QHBoxLayout(this);
    auto *left = new QVBoxLayout;
    m_position = new QLabel(this);
    m_position->setObjectName(QStringLiteral("dataFormPosition"));
    m_position->setTextFormat(Qt::PlainText);
    m_position->setAlignment(Qt::AlignRight);
    left->addWidget(m_position);
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::StyledPanel);
    auto *inner = new QWidget;
    m_fields = new QFormLayout(inner);
    for (int f = 0; f < m_form.fieldCount(); ++f) {
        auto *label = new QLabel(m_form.fields()[size_t(f)].name + QLatin1Char(':'), inner);
        label->setTextFormat(Qt::PlainText);
        auto *box = new QLineEdit(inner);
        box->setObjectName(QStringLiteral("dataFormField%1").arg(f));
        box->setAccessibleName(m_form.fields()[size_t(f)].name);
        box->setMinimumWidth(220);
        label->setBuddy(box);
        m_fields->addRow(label, box);
        m_labels.push_back(label);
        m_boxes.push_back(box);
        connect(box, &QLineEdit::returnPressed, this, [this] { findNext(); });
    }
    scroll->setWidget(inner);
    scroll->setMinimumHeight(std::min(360, 40 + 32 * m_form.fieldCount()));
    scroll->setMinimumWidth(380);
    left->addWidget(scroll, 1);
    m_notice = new QLabel(this);
    m_notice->setObjectName(QStringLiteral("dataFormNotice"));
    m_notice->setTextFormat(Qt::PlainText);
    m_notice->setWordWrap(true);
    m_notice->hide();
    left->addWidget(m_notice);
    m_status = new QLabel(this);
    m_status->setObjectName(QStringLiteral("dataFormStatus"));
    m_status->setTextFormat(Qt::PlainText);
    m_status->setWordWrap(true);
    left->addWidget(m_status);
    outer->addLayout(left, 1);
    auto *buttons = new QVBoxLayout;
    auto button = [&](const QString &text, const char *name, void (DataFormDialog::*slot)()) {
        auto *b = new QPushButton(text, this);
        b->setObjectName(QLatin1String(name));
        b->setAutoDefault(false);
        connect(b, &QPushButton::clicked, this, slot);
        buttons->addWidget(b);
        return b;
    };
    m_new = button(tr("New"), "dataFormNew", &DataFormDialog::newRecord);
    m_delete = button(tr("Delete"), "dataFormDelete", &DataFormDialog::deleteRecord);
    m_restore = button(tr("Restore"), "dataFormRestore", &DataFormDialog::restore);
    buttons->addSpacing(12);
    m_previous = button(tr("Find Prev"), "dataFormFindPrevious", &DataFormDialog::findPrevious);
    m_next = button(tr("Find Next"), "dataFormFindNext", &DataFormDialog::findNext);
    buttons->addSpacing(12);
    m_criteria = button(tr("Criteria"), "dataFormCriteria", &DataFormDialog::toggleCriteria);
    m_close = new QPushButton(tr("Close"), this);
    m_close->setObjectName(QStringLiteral("dataFormClose"));
    m_close->setAutoDefault(false);
    connect(m_close, &QPushButton::clicked, this, [this] { closeForm(); });
    buttons->addWidget(m_close);
    buttons->addStretch(1);
    outer->addLayout(buttons);
}

QStringList DataFormDialog::typed() const
{
    QStringList out;
    for (const QLineEdit *box : m_boxes) out << box->text();
    return out;
}

void DataFormDialog::tell(const QString &text)
{
    if (m_notifier) m_notifier(text);
    else jp::plainWarning(this, m_title, text);
}

void DataFormDialog::updateButtons()
{
    const bool criteria = m_mode == Mode::Criteria, isNew = m_mode == Mode::New;
    m_new->setEnabled(!criteria);
    m_delete->setEnabled(m_mode == Mode::Record && m_form.recordCount() > 0);
    m_restore->setEnabled(true);
    m_restore->setText(criteria ? tr("Clear") : tr("Restore"));
    m_previous->setEnabled(true);
    m_next->setEnabled(true);
    m_criteria->setText(criteria ? tr("Form") : tr("Criteria"));
    Q_UNUSED(isNew);
}

// A box that shows what a formula comes to (or a column's own value) is read-only and dimmed; set only when it is not so already.
void DataFormDialog::setLook(QLineEdit *box, bool readOnly, const QString &why)
{
    if (box->isReadOnly() == readOnly && box->property("looked").isValid()) return;
    box->setProperty("looked", true);
    box->setReadOnly(readOnly);
    box->setFocusPolicy(readOnly ? Qt::NoFocus : Qt::StrongFocus);
    box->setToolTip(readOnly ? jp::plainToolTip(why) : QString());
    box->setStyleSheet(readOnly ? QStringLiteral("QLineEdit { background: palette(window); color: palette(placeholder-text); }") : QString());
}

void DataFormDialog::showRecord(qint64 record)
{
    m_mode = Mode::Record;
    m_form.setCurrent(record);
    // (the texts are read from the sheet once for the record, and a box's look is set only when it changes: a list of very many fields is not worked over again for each)
    const QStringList texts = m_form.values(record);
    for (int f = 0; f < m_form.fieldCount(); ++f) {
        QLineEdit *box = m_boxes[size_t(f)];
        const bool readOnly = m_form.isReadOnly(record, f);
        setLook(box, readOnly, tr("This field holds a formula. Change it in the sheet."));
        box->setText(texts[f]);
        box->setCursorPosition(0);
    }
    m_position->setText(tr("%1 of %2").arg(record + 1).arg(m_form.recordCount()));
    updateButtons();
    for (QLineEdit *box : m_boxes)
        if (!box->isReadOnly()) {
            box->setFocus(Qt::OtherFocusReason);
            box->selectAll();
            break;
        }
}

void DataFormDialog::showBlank(Mode mode)
{
    m_mode = mode;
    const bool criteria = mode == Mode::Criteria;
    for (int f = 0; f < m_form.fieldCount(); ++f) {
        QLineEdit *box = m_boxes[size_t(f)];
        // a new record has no value for what a table's column works out; every field takes a condition
        const bool readOnly = !criteria && m_form.isCalculatedColumn(f);
        setLook(box, readOnly, tr("This column works out its own value."));
        box->setText(criteria && f < m_form.criteria().size() ? m_form.criteria()[f] : QString());
    }
    m_position->setText(criteria ? tr("Criteria") : tr("New Record"));
    updateButtons();
    for (QLineEdit *box : m_boxes)
        if (!box->isReadOnly()) {
            box->setFocus(Qt::OtherFocusReason);
            break;
        }
}

bool DataFormDialog::commitCurrent(bool *added)
{
    if (added) *added = false;
    m_status->clear();
    if (m_mode == Mode::Criteria) return true;
    crunch::EditSession::Outcome o;
    if (m_mode == Mode::New) {
        bool made = false;
        o = m_form.add(typed(), &made);
        if (!o.ok) {
            tell(o.problem);
            return false;
        }
        if (made) Q_EMIT recordsChanged();
        if (added) *added = made;
        return true;
    }
    const int before = m_session->undoStack()->index();
    o = m_form.change(m_form.current(), typed());
    if (!o.ok) {
        tell(o.problem);
        return false;
    }
    if (m_session->undoStack()->index() != before) Q_EMIT recordsChanged();
    return true;
}

void DataFormDialog::newRecord()
{
    if (m_mode == Mode::Criteria) return;
    if (!commitCurrent()) return;
    showBlank(Mode::New);
}

void DataFormDialog::deleteRecord()
{
    if (m_mode != Mode::Record || m_form.recordCount() < 1) return;
    const QString question = tr("Delete this record? It is taken out of the list and the rows under it move up. You can undo this in the sheet.");
    const bool yes = m_deleteAsker ? m_deleteAsker(question) : jp::plainQuestion(this, m_title, question, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
    if (!yes) return;
    const crunch::EditSession::Outcome o = m_form.remove(m_form.current());
    if (!o.ok) {
        tell(o.problem);
        return;
    }
    Q_EMIT recordsChanged();
    if (m_form.recordCount() < 1) {
        accept();   // (a list with no record left has nothing to show)
        return;
    }
    showRecord(m_form.current());
}

void DataFormDialog::restore()
{
    if (m_mode == Mode::Criteria) {
        m_form.clearCriteria();
        showBlank(Mode::Criteria);
        return;
    }
    if (m_mode == Mode::New) showBlank(Mode::New);
    else showRecord(m_form.current());
    m_status->clear();
}

void DataFormDialog::findNext()
{
    if (m_mode == Mode::Criteria) {
        m_form.setCriteria(typed());
        const qint64 from = m_form.current();
        // the search begins at the record that was shown, and takes the one it is on when that meets the criteria too
        qint64 found = m_form.matches(from) && m_form.hasCriteria() ? from : m_form.findNext(from);
        if (found < 0) {
            m_mode = Mode::Record;
            showRecord(from);
            m_status->setText(tr("No record meets the criteria."));
            return;
        }
        showRecord(found);
        return;
    }
    if (m_mode == Mode::New) {
        // the record is added, and another blank one is ready
        if (!commitCurrent()) return;
        showBlank(Mode::New);
        return;
    }
    if (!commitCurrent()) return;
    const qint64 found = m_form.findNext(m_form.current());
    if (found < 0) {
        showRecord(m_form.current());
        m_status->setText(m_form.hasCriteria() ? tr("No more records meet the criteria.") : tr("This is the last record."));
        return;
    }
    showRecord(found);
}

void DataFormDialog::findPrevious()
{
    if (m_mode == Mode::Criteria) {
        m_form.setCriteria(typed());
        const qint64 from = m_form.current();
        const qint64 found = m_form.matches(from) && m_form.hasCriteria() ? from : m_form.findPrevious(from);
        if (found < 0) {
            m_mode = Mode::Record;
            showRecord(from);
            m_status->setText(tr("No record meets the criteria."));
            return;
        }
        showRecord(found);
        return;
    }
    bool added = false;
    if (!commitCurrent(&added)) return;
    // from a new record that was not made the one before is the last of the list; from one that was just made, the one before it
    const qint64 from = m_mode == Mode::New && !added ? m_form.recordCount() : m_form.current();
    const qint64 found = m_form.findPrevious(from);
    if (found < 0) {
        showRecord(m_form.current());
        m_status->setText(m_form.hasCriteria() ? tr("No earlier record meets the criteria.") : tr("This is the first record."));
        return;
    }
    showRecord(found);
}

void DataFormDialog::toggleCriteria()
{
    if (m_mode == Mode::Criteria) {
        m_form.setCriteria(typed());
        showRecord(m_form.current());
        return;
    }
    if (!commitCurrent()) return;
    showBlank(Mode::Criteria);
}

bool DataFormDialog::closeForm()
{
    if (!commitCurrent()) return false;
    accept();
    return true;
}

} // namespace jp

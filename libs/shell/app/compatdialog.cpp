#include "app/compatdialog.h"
#include "app/plaintext.h"
#include "app/uistyle.h"
#include "core/lossnotes.h"
#include <QComboBox>
#include <QCoreApplication>
#include <QLabel>
#include <QPointer>
#include <QScrollArea>
#include <QVBoxLayout>

namespace jp {

QStringList compatFileChoices(const QString &what, bool markedFinal, bool alwaysReadOnly)
{
    QStringList lines;
    if (markedFinal) lines << QCoreApplication::translate("jp::CompatibilityDialog", "The mark that the %1 is final.").arg(what);
    if (alwaysReadOnly) lines << QCoreApplication::translate("jp::CompatibilityDialog", "The choice to always open the %1 for reading only.").arg(what);
    return lines;
}

CompatAnswer CompatAnswer::failed(const QString &why)
{
    CompatAnswer answer;
    answer.couldNot = why.isEmpty() ? QCoreApplication::translate("jp::CompatibilityDialog", "Something went wrong.") : why;
    return answer;
}

QStringList compatLines(const CompatFormat &format, const CompatHost &host, const CompatAnswer &answer)
{
    // (the kind the app saves as its own holds the choices; every other kind leaves them out)
    const QStringList choices = compatFileChoices(format.what, host.markedFinal && host.markedFinal(), host.alwaysReadOnly && host.alwaysReadOnly());
    // The file's choices always stay inside the bound; when something is cut, the last line of the list says how many.
    const qsizetype room = kCompatMaxLines - choices.size();
    qint64 hidden = answer.more;
    qsizetype keep = answer.found.size();
    if (keep + (hidden > 0 ? 1 : 0) > room) {
        keep = room - 1;
        hidden += answer.found.size() - keep;
    }
    QStringList lines;
    for (qsizetype i = 0; i < keep; ++i) {
        const QString &line = answer.found[i];
        lines << (line.size() > kCompatMaxLineChars ? line.left(kCompatMaxLineChars) + QChar(0x2026) : line);
    }
    if (hidden > 0) lines << moreLossNotes(hidden);
    return lines + choices;
}

QString compatReport(const CompatFormat &format, const CompatHost &host, const CompatAnswer &answer)
{
    if (!answer.couldNot.isEmpty())
        return QCoreApplication::translate("jp::CompatibilityDialog", "Couldn't check what a %1 file would leave out of this %2, so nothing is said either way. %3").arg(format.format, format.what, answer.couldNot);
    return lossyReport({QString(), format.what, format.format, format.ownFormat, compatLines(format, host, answer), kCompatMaxLines});
}

CompatibilityDialog::CompatibilityDialog(const CompatHost &host, int first, QWidget *parent) : QDialog(parent), m_host(host)
{
    setObjectName(QStringLiteral("compatibilityDialog"));
    setWindowTitle(QCoreApplication::translate("jp::CompatibilityDialog", "Check Compatibility"));
    resize(520, 240);
    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(QCoreApplication::translate("jp::CompatibilityDialog", "Pick a kind of file to see what it can't hold of this file. Nothing is saved."), this);
    intro->setWordWrap(true);
    layout->addWidget(plainLabel(intro));
    m_picker = new QComboBox(this);
    m_picker->setObjectName(QStringLiteral("compatibilityPicker"));
    for (const CompatFormat &f : m_host.formats) m_picker->addItem(f.label);
    m_picker->setAccessibleName(QCoreApplication::translate("jp::CompatibilityDialog", "Kind of file"));
    layout->addWidget(m_picker);
    m_report = plainLabel(new QLabel(this));
    m_report->setObjectName(QStringLiteral("compatibilityReport"));
    m_report->setWordWrap(true);
    m_report->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_report->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    auto *scroll = new QScrollArea(this);   // (a long list scrolls)
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(m_report);
    layout->addWidget(scroll, 1);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);
    layout->addWidget(buttons);
    if (m_host.formats.size() < 2) m_picker->hide();
    connect(m_picker, &QComboBox::currentIndexChanged, this, [this] { work(); });
    if (first > 0 && first < m_host.formats.size()) m_picker->setCurrentIndex(first);
    else work();
}

void CompatibilityDialog::work()
{
    const int index = m_picker->currentIndex();
    if (index < 0 || index >= m_host.formats.size()) return;
    const int generation = ++m_generation;
    m_working = true;
    m_report->setText(QCoreApplication::translate("jp::CompatibilityDialog", "Checking…"));
    const CompatFormat format = m_host.formats[index];
    const QPointer<CompatibilityDialog> self(this);
    format.lost([self, generation, format](const CompatAnswer &answer) {
        // (an answer for a kind that is no longer picked, or to a box that is gone, is dropped)
        if (!self || self->m_generation != generation) return;
        self->m_working = false;
        self->m_report->setText(compatReport(format, self->m_host, answer));
    });
}

QString CompatibilityDialog::reportText() const { return m_report->text(); }

void CompatibilityDialog::choose(int index) { m_picker->setCurrentIndex(index); }

void showCompatibility(QWidget *parent, const CompatHost &host, int first, const std::function<void(const QString &, const QString &, bool)> &notifier)
{
    if (host.formats.isEmpty()) return;
    const QString title = QCoreApplication::translate("jp::CompatibilityDialog", "Check Compatibility");
    if (notifier) {
        const int index = first >= 0 && first < host.formats.size() ? first : 0;
        const CompatFormat format = host.formats[index];
        format.lost([format, host, title, notifier](const CompatAnswer &answer) {
            notifier(title, compatReport(format, host, answer), !answer.couldNot.isEmpty() || !compatLines(format, host, answer).isEmpty());
        });
        return;
    }
    CompatibilityDialog dialog(host, first, parent);
    dialog.exec();
}

} // namespace jp

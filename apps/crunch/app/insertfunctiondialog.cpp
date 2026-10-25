#include "app/insertfunctiondialog.h"

#include "core/functioninfo.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>
#include "app/plaintext.h"

namespace jp {

using namespace jp::crunch;

InsertFunctionDialog::InsertFunctionDialog(QWidget *parent, const QString &search) : QDialog(parent)
{
    setObjectName(QStringLiteral("insertFunctionDialog"));
    setWindowTitle(tr("Pick a Function"));
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;
    m_search = new QLineEdit(search);
    m_search->setObjectName(QStringLiteral("functionSearch"));
    m_search->setAccessibleName(tr("Search for a function"));
    m_search->setPlaceholderText(tr("A name, or what you want to do"));
    m_search->setClearButtonEnabled(true);
    form->addRow(tr("Search:"), m_search);
    m_category = new QComboBox;
    m_category->setObjectName(QStringLiteral("functionCategory"));
    m_category->setAccessibleName(tr("Kind of function"));
    m_category->addItem(tr("All"), -1);
    for (FunctionCategory c : {FunctionCategory::Financial, FunctionCategory::DateTime, FunctionCategory::MathTrig, FunctionCategory::Statistical, FunctionCategory::LookupReference,
                               FunctionCategory::Text, FunctionCategory::Logical, FunctionCategory::Information, FunctionCategory::Database})
        m_category->addItem(categoryName(c), int(c));
    form->addRow(tr("Kind:"), m_category);
    layout->addLayout(form);
    m_list = new QListWidget;
    m_list->setObjectName(QStringLiteral("functionList"));
    m_list->setAccessibleName(tr("Functions"));
    m_list->setMinimumHeight(180);
    layout->addWidget(m_list, 1);
    m_count = new QLabel;
    m_count->setObjectName(QStringLiteral("functionCount"));
    layout->addWidget(m_count);
    m_signature = new QLabel;
    m_signature->setObjectName(QStringLiteral("functionSignature"));
    m_signature->setTextFormat(Qt::PlainText);
    m_signature->setWordWrap(true);
    QFont f = m_signature->font();
    f.setBold(true);
    m_signature->setFont(f);
    layout->addWidget(m_signature);
    m_summary = new QLabel;
    m_summary->setObjectName(QStringLiteral("functionSummary"));
    m_summary->setTextFormat(Qt::PlainText);
    m_summary->setWordWrap(true);
    layout->addWidget(m_summary);
    layout->addWidget(new QLabel(tr("Arguments:")));
    m_arguments = new QListWidget;
    m_arguments->setObjectName(QStringLiteral("functionArguments"));
    m_arguments->setAccessibleName(tr("Arguments of the function"));
    m_arguments->setMaximumHeight(110);
    m_arguments->setSelectionMode(QAbstractItemView::NoSelection);
    layout->addWidget(m_arguments);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_ok = buttons->button(QDialogButtonBox::Ok);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_search, &QLineEdit::textChanged, this, [this] { refill(); });
    connect(m_category, &QComboBox::currentIndexChanged, this, [this] { refill(); });
    connect(m_list, &QListWidget::currentRowChanged, this, [this] { showPicked(); });
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this] { accept(); });
    refill();
    m_search->setFocus();
}

QString InsertFunctionDialog::chosen() const { return m_list->currentItem() ? m_list->currentItem()->data(Qt::UserRole).toString() : QString(); }

void InsertFunctionDialog::refill()
{
    const QString was = chosen();
    const int category = m_category->currentData().toInt();
    const QStringList words = m_search->text().toCaseFolded().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    int count = 0;
    const FunctionDoc *docs = functionDocs(&count);
    std::vector<const FunctionDoc *> found;
    for (int i = 0; i < count; ++i) {
        const FunctionDoc &d = docs[i];
        if (category >= 0 && int(d.category) != category) continue;
        const QString name = QString::fromLatin1(d.name).toCaseFolded();
        const QString about = name + QLatin1Char(' ') + summaryOf(d).toCaseFolded();
        bool all = true;
        for (const QString &w : words)
            if (!about.contains(w)) {
                all = false;
                break;
            }
        if (all) found.push_back(&d);
    }
    // the names that begin with what was typed first, then the rest, each by name
    const QString first = words.isEmpty() ? QString() : words.first();
    std::stable_sort(found.begin(), found.end(), [&](const FunctionDoc *a, const FunctionDoc *b) {
        const bool pa = !first.isEmpty() && QString::fromLatin1(a->name).toCaseFolded().startsWith(first), pb = !first.isEmpty() && QString::fromLatin1(b->name).toCaseFolded().startsWith(first);
        if (pa != pb) return pa;
        return QLatin1String(a->name) < QLatin1String(b->name);
    });
    const QSignalBlocker block(m_list);
    m_list->clear();
    int keep = -1;
    for (const FunctionDoc *d : found) {
        auto *item = new QListWidgetItem(QString::fromLatin1(d->name));
        item->setData(Qt::UserRole, QString::fromLatin1(d->name));
        item->setToolTip(jp::plainToolTip(summaryOf(*d)));
        m_list->addItem(item);
        if (QString::fromLatin1(d->name) == was) keep = m_list->count() - 1;
    }
    m_list->setCurrentRow(keep >= 0 ? keep : (m_list->count() > 0 ? 0 : -1));
    const int shown = int(found.size());
    m_count->setText(shown == 0 ? tr("No function fits.") : tr("%n function(s)", nullptr, shown));
    showPicked();
}

void InsertFunctionDialog::showPicked()
{
    m_arguments->clear();
    const FunctionDoc *d = docOf(chosen());
    m_ok->setEnabled(d != nullptr);
    if (!d) {
        m_signature->clear();
        m_summary->clear();
        return;
    }
    m_signature->setText(signatureOf(*d));
    m_summary->setText(summaryOf(*d));
    const QStringList args = argumentsOf(*d);
    if (args.isEmpty()) {
        m_arguments->addItem(tr("(none)"));
        return;
    }
    for (const QString &a : args) {
        if (a == QLatin1String("...")) m_arguments->addItem(tr("more of the same, as many as you need"));
        else if (a.startsWith(QLatin1Char('['))) m_arguments->addItem(tr("%1 (optional)").arg(a.mid(1, a.size() - 2)));
        else m_arguments->addItem(a);
    }
}

} // namespace jp

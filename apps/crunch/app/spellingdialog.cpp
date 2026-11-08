#include "app/spellingdialog.h"

#include "app/crunchwindow.h"
#include "app/plaintext.h"
#include "app/spelling.h"
#include "text/dictionaries.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QPointer>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

namespace jp {

SpellingDialog::SpellingDialog(CrunchWindow *window, int sheet, const jp::office::sheet::CellRange &range) : QDialog(window), m_win(window), m_sheet(sheet)
{
    setObjectName(QStringLiteral("spellingDialog"));
    setWindowTitle(QCoreApplication::translate("Proofing", "Check Spelling"));
    m_walk = std::make_unique<crunch::SpellWalk>(window->session(), sheet, range);
    auto *v = new QVBoxLayout(this);
    m_notIn = new QLabel(this);   // (rich text: the word in bold; it is escaped)
    m_notIn->setObjectName(QStringLiteral("spellingWord"));
    m_change = new QLineEdit(this);
    m_change->setObjectName(QStringLiteral("spellingChange"));
    m_suggestions = new QListWidget(this);
    m_suggestions->setObjectName(QStringLiteral("spellingSuggestions"));
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("Proofing", "Not in dictionary:"), this)));
    v->addWidget(m_notIn);
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("Proofing", "Change to:"), this)));
    v->addWidget(m_change);
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("Proofing", "Suggestions:"), this)));
    v->addWidget(m_suggestions);
    auto *row = new QHBoxLayout();
    struct Button {
        QString label;
        const char *name;
        bool (SpellingDialog::*work)();
    };
    for (const Button &b : {Button{QCoreApplication::translate("Proofing", "Ignore"), "spellingIgnore", &SpellingDialog::ignore},
                            Button{QCoreApplication::translate("Proofing", "Ignore All"), "spellingIgnoreAll", &SpellingDialog::ignoreAll},
                            Button{QCoreApplication::translate("Proofing", "Change"), "spellingChangeButton", &SpellingDialog::change},
                            Button{QCoreApplication::translate("Proofing", "Change All"), "spellingChangeAll", &SpellingDialog::changeAll},
                            Button{QCoreApplication::translate("Proofing", "Add"), "spellingAdd", &SpellingDialog::addToDictionary}}) {
        auto *button = new QPushButton(b.label, this);
        m_buttons << button;
        button->setObjectName(QLatin1String(b.name));
        row->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, work = b.work] { (this->*work)(); });
    }
    auto *close = new QPushButton(QCoreApplication::translate("Proofing", "Close"), this);
    close->setObjectName(QStringLiteral("spellingClose"));
    row->addWidget(close);
    // (Close while the dialog is still looking gives the looking up)
    connect(close, &QPushButton::clicked, this, [this] {
        if (m_looking) m_cancelled = true;
        else reject();
    });
    v->addLayout(row);
    m_progress = new QLabel(this);
    m_progress->setObjectName(QStringLiteral("spellingProgress"));
    m_progress->setTextFormat(Qt::PlainText);
    v->addWidget(m_progress);
    connect(m_suggestions, &QListWidget::currentTextChanged, m_change, &QLineEdit::setText);
}

QString SpellingDialog::replacement() const { return m_change->text(); }
void SpellingDialog::setReplacement(const QString &text) { m_change->setText(text); }
QStringList SpellingDialog::suggestions() const
{
    QStringList out;
    for (int i = 0; i < m_suggestions->count(); ++i) out << m_suggestions->item(i)->text();
    return out;
}

bool SpellingDialog::start(bool quiet)
{
    m_quiet = quiet;
    return advance();
}

bool SpellingDialog::advance()
{
    // The cells are looked at a slice at a time, and the window gets its turn between slices (a sheet may have a million text cells); Close gives it up.
    m_looking = true;
    m_cancelled = false;
    for (QPushButton *b : m_buttons) b->setEnabled(false);
    crunch::SpellHit hit;
    crunch::SpellWalk::Result result;
    const QPointer<SpellingDialog> self(this);
    for (;;) {
        QElapsedTimer slice;
        slice.start();
        result = m_walk->step(40, &hit);
        m_longestSlice = std::max(m_longestSlice, slice.elapsed());
        if (result != crunch::SpellWalk::Result::More) break;
        m_progress->setText(QCoreApplication::translate("Proofing", "Checking… %1%").arg(int(m_walk->progress() * 100)));
        QCoreApplication::processEvents();
        if (!self) return false;
        if (m_cancelled) break;
    }
    m_progress->clear();
    m_looking = false;
    for (QPushButton *b : m_buttons) b->setEnabled(true);
    if (m_cancelled) {
        m_hit.reset();
        reject();
        return false;
    }
    if (result == crunch::SpellWalk::Result::Done) {
        m_hit.reset();
        m_notIn->clear();
        m_suggestions->clear();
        m_change->clear();
        if (!m_quiet) jp::plainInformation(this, QCoreApplication::translate("Proofing", "Check Spelling"), QCoreApplication::translate("Proofing", "The spelling check is complete."));
        accept();
        return false;
    }
    m_hit = hit;
    m_win->showCell(m_sheet, m_hit->row, m_hit->col);
    m_notIn->setText(QStringLiteral("<b>%1</b>").arg(m_hit->word.toHtmlEscaped()));
    m_suggestions->clear();
    m_suggestions->addItems(spellingSuggestions(m_hit->word, crunch::proofingLanguage()));
    m_change->setText(m_suggestions->count() ? m_suggestions->item(0)->text() : m_hit->word);
    return true;
}

bool SpellingDialog::ignore() { return m_hit && advance(); }

bool SpellingDialog::ignoreAll()
{
    if (!m_hit) return false;
    spellingIgnore(m_hit->word);
    return advance();
}

bool SpellingDialog::addToDictionary()
{
    if (!m_hit) return false;
    QString failure;
    if (!spellingAdd(m_hit->word, &failure)) spellingReportFailure(this, failure);
    return advance();
}

bool SpellingDialog::change()
{
    if (!m_hit) return false;
    const QString with = m_change->text();
    QString text = m_hit->text;
    text.replace(m_hit->from, m_hit->to - m_hit->from, with);
    if (!m_win->replaceCellTexts({{m_sheet, m_hit->row, m_hit->col, text}}, QCoreApplication::translate("Proofing", "Spelling"))) return false;
    m_walk->changed(m_hit->from + int(with.size()));
    return advance();
}

bool SpellingDialog::changeAll()
{
    if (!m_hit) return false;
    const QString with = m_change->text();
    // (the word as a whole word: not a part of a longer one)
    const QRegularExpression whole(QStringLiteral("(?<![\\p{L}\\p{N}'’])%1(?![\\p{L}\\p{N}'’])").arg(QRegularExpression::escape(m_hit->word)));
    std::vector<CrunchWindow::CellText> changes;
    int offset = m_hit->to;   // (where the walk goes on in the cell it stands in)
    for (const auto &cell : m_walk->cells()) {
        QString text;
        if (!crunch::spellableText(*m_win->session(), m_sheet, cell.first, cell.second, &text)) continue;
        QString changed;
        int last = 0, before = 0;
        const bool here = cell.first == m_hit->row && cell.second == m_hit->col;
        for (auto it = whole.globalMatch(text); it.hasNext();) {
            const auto m = it.next();
            changed += text.mid(last, int(m.capturedStart()) - last) + with;
            last = int(m.capturedEnd());
            if (here && m.capturedStart() < m_hit->from) ++before;
        }
        if (last == 0 && changed.isEmpty()) continue;
        changed += text.mid(last);
        if (here) offset = m_hit->from + before * (int(with.size()) - int(m_hit->word.size())) + int(with.size());
        changes.push_back({m_sheet, cell.first, cell.second, changed});
    }
    if (changes.empty() || !m_win->replaceCellTexts(changes, QCoreApplication::translate("Proofing", "Spelling"))) return false;
    m_walk->changed(offset);
    return advance();
}

} // namespace jp

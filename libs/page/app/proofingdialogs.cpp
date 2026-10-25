// Spelling (Hunspell with LibreOffice's dictionaries, by the text's
// language) and thesaurus (WordNet based th_en_US_v2, English).

#include "app/proofingdialogs.h"

#include "app/editor.h"
#include "app/spelling.h"
#include "text/dictionaries.h"
#include "text/textprops.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTextBlock>
#include <QTextDocument>
#include <QTimer>
#include <QVBoxLayout>

#include <memory>
#include "app/plaintext.h"

namespace jp {

// ---------------- spelling ----------------
// The bundled language for the text at pos ("" when it has no dictionary).
static QString languageAt(QTextDocument *doc, int pos)
{
    QTextCursor c(doc);
    c.setPosition(std::min(pos + 1, doc->characterCount() - 1));
    return dict::match(c.charFormat().stringProperty(tp::Language));
}

void spellingDialog(QWidget *p, Editor *ed)
{
    if (dict::root().isEmpty()) {
        jp::plainWarning(p, QCoreApplication::translate("Proofing", "Spelling"), QCoreApplication::translate("Proofing", "The dictionaries are missing from this installation."));
        return;
    }
    // Stories in reading order.
    struct S { QString itemId, storyId; int page; int row = -1, col = -1; int half = 0; };
    QVector<S> stories;
    QSet<QString> seen;
    Document *d = ed->doc();
    for (int pg = 0; pg < d->pages.size(); ++pg)
        walkItems(d->pages[pg]->items, [&](const ItemPtr &it) {
            if (it->type() == ItemType::Text) {
                auto *t = static_cast<TextItem *>(it.get());
                if (!seen.contains(t->storyId)) { seen.insert(t->storyId); stories << S{d->chainOf(t->id).value(0, t)->id, t->storyId, pg}; }
            } else if (it->type() == ItemType::Shape && !static_cast<ShapeItem *>(it.get())->storyId.isEmpty()) {
                stories << S{it->id, static_cast<ShapeItem *>(it.get())->storyId, pg};
            } else if (it->type() == ItemType::Table) {
                auto *tb = static_cast<TableItem *>(it.get());
                for (int r = 0; r < tb->rows; ++r)
                    for (int c = 0; c < tb->cols; ++c) {
                        stories << S{tb->id, tb->cell(r, c).storyId, pg, r, c};
                        if (tb->cell(r, c).divided() && !tb->cell(r, c).storyId2.isEmpty()) stories << S{tb->id, tb->cell(r, c).storyId2, pg, r, c, 1};
                    }
            }
        });
    QDialog dlg(p);
    dlg.setWindowTitle(QCoreApplication::translate("Proofing", "Check Spelling"));
    auto *v = new QVBoxLayout(&dlg);
    auto *notIn = new QLabel(&dlg);
    auto *change = new QLineEdit(&dlg);
    auto *sugg = new QListWidget(&dlg);
    v->addWidget(new QLabel(QCoreApplication::translate("Proofing", "Not in dictionary:")));
    v->addWidget(notIn);
    v->addWidget(new QLabel(QCoreApplication::translate("Proofing", "Change to:")));
    v->addWidget(change);
    v->addWidget(new QLabel(QCoreApplication::translate("Proofing", "Suggestions:")));
    v->addWidget(sugg);
    auto *row = new QHBoxLayout();
    auto *ignore = new QPushButton(QCoreApplication::translate("Proofing", "Ignore"), &dlg), *ignoreAll = new QPushButton(QCoreApplication::translate("Proofing", "Ignore All"), &dlg);
    auto *chg = new QPushButton(QCoreApplication::translate("Proofing", "Change"), &dlg), *chgAll = new QPushButton(QCoreApplication::translate("Proofing", "Change All"), &dlg);
    auto *add = new QPushButton(QCoreApplication::translate("Proofing", "Add"), &dlg), *close = new QPushButton(QCoreApplication::translate("Proofing", "Close"), &dlg);
    for (auto *b : {ignore, ignoreAll, chg, chgAll, add, close}) row->addWidget(b);
    v->addLayout(row);
    QObject::connect(sugg, &QListWidget::currentTextChanged, change, &QLineEdit::setText);
    int si = 0, pos = 0;
    QString word;
    std::function<bool()> next = [&]() -> bool {
        for (; si < stories.size(); ++si, pos = 0) {
            QTextDocument *doc = d->storyDoc(stories[si].storyId);
            if (!doc) continue;
            for (const auto &m : misspellings(doc)) {
                if (m.first < pos) continue;
                const S &s = stories[si];
                if (s.page != ed->currentPage()) ed->setCurrentPage(s.page);
                ed->beginTextEdit(s.itemId, m.first, s.row, s.col, s.half);
                QTextCursor c = ed->cursor();
                c.setPosition(m.first);
                c.setPosition(m.second, QTextCursor::KeepAnchor);
                ed->setCursor(c);
                word = c.selectedText();
                notIn->setText(QStringLiteral("<b>%1</b>").arg(word.toHtmlEscaped()));
                sugg->clear();
                sugg->addItems(Speller::get().suggest(languageAt(doc, m.first), word));
                change->setText(sugg->count() ? sugg->item(0)->text() : word);
                pos = m.second;
                return true;
            }
        }
        jp::plainInformation(&dlg, QCoreApplication::translate("Proofing", "Check Spelling"), QCoreApplication::translate("Proofing", "The spelling check is complete."));
        dlg.accept();
        return false;
    };
    QObject::connect(ignore, &QPushButton::clicked, &dlg, [&] { next(); });
    QObject::connect(ignoreAll, &QPushButton::clicked, &dlg, [&] { Speller::get().ignore(word); next(); });
    QObject::connect(add, &QPushButton::clicked, &dlg, [&] {
        QString failure;
        if (!Speller::get().add(word, &failure)) spellingReportFailure(&dlg, failure);
        next();
    });
    QObject::connect(chg, &QPushButton::clicked, &dlg, [&] {
        ed->beginChange(QCoreApplication::translate("Proofing", "Spelling"));
        ed->cursor().insertText(change->text());
        ed->endChange();
        pos = ed->cursor().position();
        next();
    });
    QObject::connect(chgAll, &QPushButton::clicked, &dlg, [&] {
        const QString from = word, to = change->text();
        ed->change(QCoreApplication::translate("Proofing", "Spelling"), [&] {
            for (const S &s : stories) {
                QTextDocument *doc = d->storyDoc(s.storyId);
                QTextCursor c = doc->find(from, 0, QTextDocument::FindCaseSensitively | QTextDocument::FindWholeWords);
                while (!c.isNull()) {
                    c.insertText(to);
                    c = doc->find(from, c, QTextDocument::FindCaseSensitively | QTextDocument::FindWholeWords);
                }
            }
        });
        next();
    });
    QObject::connect(close, &QPushButton::clicked, &dlg, &QDialog::reject);
    if (!next()) return;
    dlg.exec();
}

// ---------------- thesaurus ----------------
class Thesaurus {
public:
    static Thesaurus &get()
    {
        static Thesaurus t;
        return t;
    }
    QStringList lookup(const QString &word)
    {
        load();
        // Try the word as typed, then its base form ("running" -> "run").
        for (const QString &key : baseForms(word.toLower())) {
            const QStringList out = lookupExact(key, word);
            if (!out.isEmpty()) return out;
        }
        return {};
    }

private:
    QStringList lookupExact(const QString &key, const QString &word) const
    {
        QStringList out;
        auto it = m_index.find(key);
        if (it == m_index.end()) return out;
        qsizetype p = it.value();
        // Line: word|count, then count lines "(pos)|syn|syn".
        const qsizetype eol = m_data.indexOf('\n', p);
        const int count = m_data.mid(p, eol - p).trimmed().split('|').value(1).toInt();
        p = eol + 1;
        for (int i = 0; i < count && p > 0 && p < m_data.size(); ++i) {
            const qsizetype e = m_data.indexOf('\n', p);
            const QList<QByteArray> parts = m_data.mid(p, (e < 0 ? m_data.size() : e) - p).trimmed().split('|');
            const QString pos = QString::fromUtf8(parts.value(0));
            for (int k = 1; k < parts.size(); ++k) {
                const QString s = QString::fromUtf8(parts[k]).trimmed();
                if (!s.isEmpty() && s.compare(word, Qt::CaseInsensitive) != 0 && s.compare(key, Qt::CaseInsensitive) != 0)
                    out << QStringLiteral("%1 %2").arg(s, pos);
            }
            if (e < 0) break;
            p = e + 1;
        }
        out.removeDuplicates();
        return out;
    }
    static QStringList baseForms(const QString &w)
    {
        QStringList f{w};
        auto add = [&](const QString &s) { if (s.size() >= 2 && !f.contains(s)) f << s; };
        auto undouble = [&](const QString &stem) {   // "runn" -> "run"
            if (stem.size() >= 3 && stem.back() == stem.at(stem.size() - 2)) add(stem.chopped(1));
        };
        if (w.endsWith(QLatin1String("'s"))) add(w.chopped(2));
        if (w.endsWith(QLatin1String("ies"))) add(w.chopped(3) + 'y');
        if (w.endsWith(QLatin1String("ied"))) add(w.chopped(3) + 'y');
        if (w.endsWith(QLatin1String("es"))) add(w.chopped(2));
        if (w.endsWith('s') && !w.endsWith(QLatin1String("ss"))) add(w.chopped(1));
        if (w.endsWith(QLatin1String("ing"))) { add(w.chopped(3)); add(w.chopped(3) + 'e'); undouble(w.chopped(3)); }
        if (w.endsWith(QLatin1String("ed"))) { add(w.chopped(2)); add(w.chopped(1)); undouble(w.chopped(2)); }
        if (w.endsWith(QLatin1String("ier"))) add(w.chopped(3) + 'y');
        if (w.endsWith(QLatin1String("iest"))) add(w.chopped(4) + 'y');
        if (w.endsWith(QLatin1String("er"))) { add(w.chopped(2)); add(w.chopped(1)); undouble(w.chopped(2)); }
        if (w.endsWith(QLatin1String("est"))) { add(w.chopped(3)); add(w.chopped(2)); undouble(w.chopped(3)); }
        if (w.endsWith(QLatin1String("ily"))) add(w.chopped(3) + 'y');
        if (w.endsWith(QLatin1String("ly"))) add(w.chopped(2));
        return f;
    }

    void load()
    {
        if (m_loaded) return;
        m_loaded = true;
        QFile f(dict::root() + "/en/th_en_US_v2.dat");
        if (!f.open(QIODevice::ReadOnly)) return;
        m_data = f.readAll();
        qsizetype p = m_data.indexOf('\n') + 1;   // skip encoding line
        while (p > 0 && p < m_data.size()) {
            const qsizetype e = m_data.indexOf('\n', p);
            if (e < 0) break;
            const QByteArray line = m_data.mid(p, e - p).trimmed();   // tolerate CRLF
            if (!line.startsWith('(')) {
                const int bar = line.indexOf('|');
                if (bar > 0) m_index.insert(QString::fromUtf8(line.left(bar)).toLower(), p);
            }
            p = e + 1;
        }
    }
    bool m_loaded = false;
    QByteArray m_data;
    QHash<QString, qsizetype> m_index;
};

QStringList thesaurusLookup(const QString &word) { return Thesaurus::get().lookup(word.trimmed()); }

void thesaurusDialog(QWidget *p, Editor *ed)
{
    QString word;
    if (ed->isEditingText()) {
        QTextCursor c = ed->cursor();
        if (!c.hasSelection()) {
            c.select(QTextCursor::WordUnderCursor);
            ed->setCursor(c);
        }
        word = c.selectedText().trimmed();
    }
    QDialog dlg(p);
    dlg.setWindowTitle(QCoreApplication::translate("Proofing", "Thesaurus"));
    auto *v = new QVBoxLayout(&dlg);
    auto *q = new QLineEdit(word, &dlg);
    auto *list = new QListWidget(&dlg);
    v->addWidget(new QLabel(QCoreApplication::translate("Proofing", "Look up:")));
    v->addWidget(q);
    v->addWidget(list);
    auto fill = [&] {
        list->clear();
        const QStringList syn = thesaurusLookup(q->text());
        if (syn.isEmpty()) {
            QString note;
            if (q->text().trimmed().isEmpty())
                //: Keep the parentheses: a list entry that starts with ( is a note, not a synonym.
                note = QCoreApplication::translate("Proofing", "(Type a word to look up)");
            else
                //: Keep the parentheses: a list entry that starts with ( is a note, not a synonym.
                note = QCoreApplication::translate("Proofing", "(No suggestions)");
            list->addItem(note);
        }
        list->addItems(syn);
    };
    QObject::connect(q, &QLineEdit::returnPressed, &dlg, fill);
    // Look up as you type.
    auto *typing = new QTimer(&dlg);
    typing->setSingleShot(true);
    typing->setInterval(250);
    QObject::connect(typing, &QTimer::timeout, &dlg, fill);
    QObject::connect(q, &QLineEdit::textEdited, typing, qOverload<>(&QTimer::start));
    fill();
    if (list->count() && !list->item(0)->text().startsWith('(')) list->setCurrentRow(0);
    auto *bb = new QDialogButtonBox(&dlg);
    // Look up the chosen synonym in turn.
    auto *again = bb->addButton(QCoreApplication::translate("Proofing", "Look Up"), QDialogButtonBox::ActionRole);
    QObject::connect(again, &QPushButton::clicked, &dlg, [&] {
        if (!list->currentItem() || list->currentItem()->text().startsWith('(')) return;
        q->setText(list->currentItem()->text().section(QStringLiteral(" ("), 0, 0));
        fill();
    });
    auto *insert = bb->addButton(QCoreApplication::translate("Proofing", "Insert"), QDialogButtonBox::AcceptRole);
    bb->addButton(QDialogButtonBox::Cancel);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(insert, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(list, &QListWidget::itemDoubleClicked, &dlg, &QDialog::accept);
    v->addWidget(bb);
    if (dlg.exec() != QDialog::Accepted || !list->currentItem() || !ed->isEditingText()) return;
    QString s = list->currentItem()->text();
    s = s.section(" (", 0, 0);
    if (s.startsWith('(')) return;
    ed->beginChange(QCoreApplication::translate("Proofing", "Thesaurus"));
    ed->cursor().insertText(s);
    ed->endChange();
    ed->textEdited();
}

// ---------------- the right-click menu ----------------
bool addSpellingSuggestions(QMenu &menu, Editor *ed)
{
    const QTextCursor cur = ed->cursor();
    const int pos = cur.position();
    for (const auto &m : ed->misspelledIn(ed->textTarget().storyId)) {
        if (pos < m.first || pos > m.second || cur.hasSelection()) continue;
        QTextCursor wc = cur;
        wc.setPosition(m.first);
        wc.setPosition(m.second, QTextCursor::KeepAnchor);
        const QString word = wc.selectedText();
        QTextCursor lc = cur;
        lc.setPosition(m.first + 1);
        const QStringList sugg = spellingSuggestions(word, lc.charFormat().stringProperty(tp::Language)).mid(0, 6);
        QFont bold = menu.font();
        bold.setBold(true);
        for (const QString &sgg : sugg) {
            QAction *a = menu.addAction(sgg, &menu, [ed, m, sgg] {
                ed->editText([&](QTextCursor &c) {
                    c.setPosition(m.first);
                    c.setPosition(m.second, QTextCursor::KeepAnchor);
                    c.insertText(sgg);
                });
            });
            a->setFont(bold);
        }
        if (sugg.isEmpty()) menu.addAction(QCoreApplication::translate("Proofing", "(No spelling suggestions)"))->setEnabled(false);
        menu.addAction(QCoreApplication::translate("Proofing", "Ignore All"), &menu, [ed, word] { spellingIgnore(word); ed->invalidateSpelling(); });
        menu.addAction(QCoreApplication::translate("Proofing", "Add to Dictionary"), &menu, [ed, word] {
            QString failure;
            if (!spellingAdd(word, &failure)) spellingReportFailure(QApplication::activeWindow(), failure);
            ed->invalidateSpelling();
        });
        menu.addSeparator();
        return true;
    }
    return false;
}

} // namespace jp

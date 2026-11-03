// The Mailings tab: Start Mail Merge, Select Recipients, Edit Recipient List, the fields (address block, greeting line, any column, the rules), Preview Results, Finish & Merge, and the Envelopes
// and Labels that are made without a merge. The work on the document is core/mergeops.h and core/labelops.h; the dialogs are app/maildialogs.h and the shared recipients window.

#include "app/writewindow.h"

#include "app/filedialogs.h"
#include "app/maildialogs.h"
#include "app/mailpreview.h"
#include "app/mergedialogs.h"
#include "app/pageoutput.h"
#include "app/pageview.h"
#include "app/plaintext.h"
#include "app/ribbonbuilder.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fieldops.h"
#include "core/labelops.h"
#include "core/mergeops.h"
#include "core/tableops.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextBlock>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QPointer>
#include <QProgressDialog>
#include <QPdfWriter>
#include <QPushButton>
#include <QSaveFile>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTextDocumentFragment>
#include <QVBoxLayout>

namespace jp {

namespace {

const QString kLeft = QString::fromUtf8("\xC2\xAB"), kRight = QString::fromUtf8("\xC2\xBB");

QString shown(const QString &name) { return kLeft + name + kRight; }

QString listFilter() { return QCoreApplication::translate("jp::WriteMail", "Recipient lists (*.csv *.tsv *.txt *.xlsx *.xlsm *.xls *.ods *.fods *.vcf *.vcard)"); }

// A small pane that walks through a merge a step at a time; each step has the buttons of the commands it needs.
class WizardPane : public QWidget {
public:
    WizardPane(WriteWindow *window, QWidget *parent) : QWidget(parent), m_window(window)
    {
        auto *v = new QVBoxLayout(this);
        m_title = new QLabel(this);
        QFont f = m_title->font();
        f.setBold(true);
        m_title->setFont(f);
        v->addWidget(m_title);
        m_text = new QLabel(this);
        m_text->setWordWrap(true);
        v->addWidget(m_text);
        m_buttons = new QVBoxLayout();
        v->addLayout(m_buttons);
        v->addStretch(1);
        auto *nav = new QHBoxLayout();
        m_back = new QPushButton(QCoreApplication::translate("jp::WriteMail", "Previous"), this);
        m_next = new QPushButton(QCoreApplication::translate("jp::WriteMail", "Next"), this);
        nav->addWidget(m_back);
        nav->addWidget(m_next);
        v->addLayout(nav);
        connect(m_back, &QPushButton::clicked, this, [this] { show(m_step - 1); });
        connect(m_next, &QPushButton::clicked, this, [this] { show(m_step + 1); });
        show(0);
    }
    void show(int step)
    {
        using T = QCoreApplication;
        struct Step {
            QString title, text;
            QVector<QPair<QString, QString>> buttons;   // (text, action)
        };
        const QVector<Step> steps = {
            {T::translate("jp::WriteMail", "Step 1 of 6: Select the kind of document"), T::translate("jp::WriteMail", "What are you making? Pick one."),
             {{T::translate("jp::WriteMail", "Letters"), QStringLiteral("mail.type.letters")},
              {T::translate("jp::WriteMail", "Email messages"), QStringLiteral("mail.type.email")},
              {T::translate("jp::WriteMail", "Envelopes"), QStringLiteral("mail.type.envelopes")},
              {T::translate("jp::WriteMail", "Labels"), QStringLiteral("mail.type.labels")},
              {T::translate("jp::WriteMail", "Directory"), QStringLiteral("mail.type.directory")}}},
            {T::translate("jp::WriteMail", "Step 2 of 6: Select the recipients"), T::translate("jp::WriteMail", "Choose the list of people. A list is a file with one row for each person and a column for each detail."),
             {{T::translate("jp::WriteMail", "Use an Existing List…"), QStringLiteral("mail.recipients.existing")},
              {T::translate("jp::WriteMail", "Select from Contacts…"), QStringLiteral("mail.recipients.contacts")},
              {T::translate("jp::WriteMail", "Type a New List…"), QStringLiteral("mail.recipients.typeNew")},
              {T::translate("jp::WriteMail", "Edit Recipient List"), QStringLiteral("mail.editList")}}},
            {T::translate("jp::WriteMail", "Step 3 of 6: Write the document"), T::translate("jp::WriteMail", "Type the text. Put the cursor where a detail goes, then add a field."),
             {{T::translate("jp::WriteMail", "Address Block…"), QStringLiteral("mail.addressBlock")},
              {T::translate("jp::WriteMail", "Greeting Line…"), QStringLiteral("mail.greeting")},
              {T::translate("jp::WriteMail", "Match Fields…"), QStringLiteral("mail.matchFields")}}},
            {T::translate("jp::WriteMail", "Step 4 of 6: Preview"), T::translate("jp::WriteMail", "See the document with a real person's details. Use the arrows to move through the people."),
             {{T::translate("jp::WriteMail", "Preview Results"), QStringLiteral("mail.preview")}, {T::translate("jp::WriteMail", "Check for Errors"), QStringLiteral("mail.check")}}},
            {T::translate("jp::WriteMail", "Step 5 of 6: Finish the merge"), T::translate("jp::WriteMail", "Make one new document with every person's copy, print them, save a PDF, or make email files."),
             {{T::translate("jp::WriteMail", "Edit Individual Documents…"), QStringLiteral("mail.finish.edit")},
              {T::translate("jp::WriteMail", "Print Documents…"), QStringLiteral("mail.finish.print")},
              {T::translate("jp::WriteMail", "Save as PDF…"), QStringLiteral("mail.finish.pdf")},
              {T::translate("jp::WriteMail", "Send Email Messages…"), QStringLiteral("mail.finish.email")}}},
            {T::translate("jp::WriteMail", "Step 6 of 6: Done"), T::translate("jp::WriteMail", "The merge is finished. Save your document to keep the merge for next time."), {}},
        };
        m_step = std::clamp(step, 0, int(steps.size()) - 1);
        m_title->setText(steps[m_step].title);
        m_text->setText(steps[m_step].text);
        while (QLayoutItem *item = m_buttons->takeAt(0)) {
            delete item->widget();
            delete item;
        }
        for (const auto &b : steps[m_step].buttons) {
            auto *button = new QPushButton(b.first, this);
            const QString id = b.second;
            connect(button, &QPushButton::clicked, this, [this, id] { m_window->act(id)->trigger(); });
            m_buttons->addWidget(button);
        }
        m_back->setEnabled(m_step > 0);
        m_next->setEnabled(m_step + 1 < steps.size());
    }

private:
    WriteWindow *m_window;
    QLabel *m_title, *m_text;
    QVBoxLayout *m_buttons;
    QPushButton *m_back, *m_next;
    int m_step = 0;
};

} // namespace

// What the Mailings tab does, kept apart from the window's other commands.
struct MailUi {
    WriteWindow *w;
    QDockWidget *dock = nullptr, *wizardDock = nullptr;
    write::MailPreviewPane *pane = nullptr;

    write::WriteDoc *doc() const { return w->m_doc.get(); }
    write::mail::State *state() const { return doc() ? write::mail::stateIfAny(*doc()) : nullptr; }
    write::mail::State &make() const { return write::mail::stateOf(*doc()); }
    void touch()
    {
        make().changed = true;
        doc()->setModified(true);
    }
    void message(const QString &text) { w->statusBar()->showMessage(text, 6000); }

    // The file a document's list may be read from without asking: the file itself where it is in the folder the document is in (or a folder inside it), else a file of the same name beside the
    // document. A file the document names anywhere else is not read (a document from someone else could name a private file), and the person is asked to choose the list.
    QString listFileFor(const QString &named) const
    {
        if (w->m_path.isEmpty() || named.isEmpty()) return QString();
        const QString folder = QFileInfo(QFileInfo(w->m_path).absolutePath()).canonicalFilePath();   // (the file itself may not be on disk yet)
        if (folder.isEmpty()) return QString();
        // (Both are made canonical, links resolved, before they are held to the folder: a link in the folder to a file outside it is outside.)
        auto inside = [&](const QString &candidate) {
            const QString own = QFileInfo(candidate).canonicalFilePath();
            return !own.isEmpty() && own.startsWith(folder + QLatin1Char('/')) && QFileInfo(own).isFile() ? own : QString();
        };
        QString name = named;
        name.replace(QLatin1Char('\\'), QLatin1Char('/'));
        const QString own = inside(QFileInfo(name).isAbsolute() ? name : folder + QLatin1Char('/') + name);
        if (!own.isEmpty()) return own;
        return inside(folder + QLatin1Char('/') + name.section(QLatin1Char('/'), -1));
    }

    // The list the document is merged with: read where it is not yet. With `ask`, says what is wrong where there is none.
    bool list(bool ask)
    {
        write::mail::State *s = state();
        if (s && s->loaded) return true;
        if (s && !s->sourcePath.isEmpty()) {
            const QString original = s->sourcePath;
            const QString path = listFileFor(original);
            if (!path.isEmpty() && write::mail::loadSource(*s, path)) {
                s->sourcePath = original;   // (the link in the file is as it was)
                if (s->data.truncated) message(QCoreApplication::translate("jp::WriteMail", "The list was too big to read whole. %1 recipients are in it.").arg(s->data.rows.size()));
                return true;
            }
            if (ask) {
                w->notice(QCoreApplication::translate("jp::WriteMail", "Mail Merge"),
                          QCoreApplication::translate("jp::WriteMail", "The list of recipients this document is merged with is not next to it. Choose it again (Mailings > Select Recipients > Use an Existing List)."));
                return false;
            }
        }
        if (ask) w->notice(QCoreApplication::translate("jp::WriteMail", "Mail Merge"), QCoreApplication::translate("jp::WriteMail", "Choose the list of recipients first (Mailings > Select Recipients)."));
        return false;
    }

    QStringList columns()
    {
        write::mail::State *s = state();
        return s && s->loaded ? s->data.fields : QStringList();
    }

    // The window shows another document: this one when nothing has been done to the one it has, else a new window.
    WriteWindow *showDocument(std::unique_ptr<write::WriteDoc> d, const QString &title)
    {
        WriteWindow *target = w;
        if (w->isModified() || !w->m_path.isEmpty()) {
            target = new WriteWindow();
            target->setAttribute(Qt::WA_DeleteOnClose);
            target->show();
        }
        target->m_untitled = title;
        target->setDocument(std::move(d), QString());
        target->m_doc->setModified(true);
        target->updateTitle();
        return target;
    }

    write::mail::MergeOptions options(bool pages = true)
    {
        write::mail::MergeOptions o;
        o.pageBreaks = pages;
        o.cancelled = &m_cancelled;
        o.ask = [this](const QString &prompt, const QString &def) {
            bool ok = false;
            const QString a = QInputDialog::getText(w, QCoreApplication::translate("jp::WriteMail", "Mail Merge"), prompt, QLineEdit::Normal, def, &ok);
            if (!ok) m_cancelled = true;
            return ok ? a : def;
        };
        {
            o.progress = [this](int done, int total) {
                if (!m_progress) {
                    m_progress = new QProgressDialog(QCoreApplication::translate("jp::WriteMail", "Merging…"), QCoreApplication::translate("jp::WriteMail", "Cancel"), 0, total, w);
                    m_progress->setWindowModality(Qt::WindowModal);
                    m_progress->setMinimumDuration(400);
                    m_progress->setAutoClose(false);
                    m_progress->setAutoReset(false);
                }
                m_progress->setMaximum(total);
                m_progress->setValue(done);
                QCoreApplication::processEvents();
                if (m_progress->wasCanceled()) m_cancelled = true;
                return !m_progress->wasCanceled();
            };
        }
        return o;
    }

    // A merge runs: the cancel the person made on a question or on the progress window, and the progress window.
    bool m_cancelled = false;
    QPointer<QProgressDialog> m_progress;
    void beginMerge() { m_cancelled = false; }
    void endMerge()
    {
        delete m_progress;
        m_progress = nullptr;
    }

    // ---- start
    void start(write::mail::Kind kind)
    {
        if (!doc()) return;
        using write::mail::Kind;
        if (kind == Kind::Envelopes || kind == Kind::Labels) {
            startSheet(kind);
            return;
        }
        make().settings.type = write::mail::kindValue(kind);
        touch();
        message(QCoreApplication::translate("jp::WriteMail", "This document is now set up for a mail merge. Choose the recipients next."));
    }

    void clear()
    {
        if (!doc() || !state()) return;
        write::mail::State &s = make();
        s.settings = office::wml::MailMergeSettings();
        s.sourcePath.clear();
        s.data = MergeSource();
        s.loaded = false;
        s.fieldMap.clear();
        touch();
        if (dock) dock->hide();
        message(QCoreApplication::translate("jp::WriteMail", "The merge is cleared. This is an ordinary document again."));
    }

    // Envelopes and labels for a merge: a new document made for them, with the merge's list.
    void startSheet(write::mail::Kind kind)
    {
        using namespace write;
        using write::mail::Kind;
        const bool haveList = list(false);
        std::unique_ptr<WriteDoc> d;
        const QString block = mail::addressBlockCode(QString());
        if (kind == Kind::Envelopes) {
            EnvelopeChoice c;
            c.useBlock = true;
            if (!envelopeDialog(w, true, &c)) return;
            const QVector<mail::EnvelopeSize> sizes = mail::envelopeSizes();
            d = mail::envelopeDocument(sizes.value(c.size), c.returnAddress, mail::fieldRuns(block, shown(QCoreApplication::translate("jp::WriteMail", "Address Block"))));
        } else {
            LabelChoice c;
            c.useBlock = true;
            c.custom = mail::labelSheets().first();
            if (!labelsDialog(w, true, &c)) return;
            const mail::LabelSheet sheet = c.sheet < 0 ? c.custom : mail::labelSheets().value(c.sheet);
            const QString result = shown(QCoreApplication::translate("jp::WriteMail", "Address Block"));
            d = mail::labelDocument(sheet, [&](int i) {
                QByteArray runs = mail::fieldRuns(block, result);
                if (i > 0) runs = mail::fieldRuns(QStringLiteral("NEXT"), shown(QCoreApplication::translate("jp::WriteMail", "Next Record"))) + runs;
                return mail::tightParagraph(runs);
            });
        }
        if (!d) {
            w->notice(QCoreApplication::translate("jp::WriteMail", "Mail Merge"), QCoreApplication::translate("jp::WriteMail", "The page can't be made."), true);
            return;
        }
        // The new document is merged as the old one was, with the same list.
        auto fresh = std::make_shared<mail::State>();
        if (write::mail::State *s = state()) *fresh = *s;
        fresh->settings.type = mail::kindValue(kind);
        fresh->changed = true;
        d->mailMerge = fresh;
        Q_UNUSED(haveList)
        showDocument(std::move(d), kind == Kind::Envelopes ? QCoreApplication::translate("jp::WriteMail", "Envelopes") : QCoreApplication::translate("jp::WriteMail", "Labels"));
    }

    // ---- recipients
    bool chooseList(bool contacts)
    {
        if (!doc()) return false;
        const QString dir = w->m_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(w->m_path).absolutePath();
        const QString filter = contacts ? QCoreApplication::translate("jp::WriteMail", "Contacts (*.vcf *.vcard)") : listFilter();
        const QString path = QFileDialog::getOpenFileName(w, contacts ? QCoreApplication::translate("jp::WriteMail", "Select Contacts") : QCoreApplication::translate("jp::WriteMail", "Select Data Source"), dir, filter);
        if (path.isEmpty()) return false;
        return takeList(path);
    }

    bool takeList(const QString &path)
    {
        QString error;
        MergeSource m;
        QApplication::setOverrideCursor(Qt::WaitCursor);
        const bool ok = loadMergeSource(path, &m, &error);
        QApplication::restoreOverrideCursor();
        if (!ok) {
            w->notice(QCoreApplication::translate("jp::WriteMail", "Select Data Source"), QCoreApplication::translate("jp::WriteMail", "The list can't be read.\n%1").arg(error), true);
            return false;
        }
        write::mail::State &s = make();
        if (s.kind() == write::mail::Kind::None) s.settings.type = write::mail::kindValue(write::mail::Kind::Letters);
        write::mail::dropBlankRecords(m);
        const bool cut = m.truncated || m.rows.size() > write::mail::kMostRecords;
        write::mail::limitRecords(m);
        s.data = m;
        s.sourcePath = QFileInfo(path).absoluteFilePath();
        s.loaded = true;
        touch();
        refreshPreview();
        message(cut ? QCoreApplication::translate("jp::WriteMail", "The list was too big to read whole. %1 recipients are in it.").arg(s.data.rows.size())
                                                         : QCoreApplication::translate("jp::WriteMail", "%1 recipients are in the list.").arg(m.rows.size()));
        return true;
    }

    void typeNew()
    {
        if (!doc()) return;
        MergeSource m;
        if (!editRecipientList(w, m, true)) return;
        write::mail::State &s = make();
        if (s.kind() == write::mail::Kind::None) s.settings.type = write::mail::kindValue(write::mail::Kind::Letters);
        s.data = m;
        s.sourcePath = m.path;
        s.loaded = true;
        touch();
        refreshPreview();
    }

    void editList()
    {
        if (!doc() || !list(true)) return;
        write::mail::State &s = make();
        MergeSource m = s.data;
        if (!editRecipientList(w, m, false)) return;
        s.data = m;
        touch();
        refreshPreview();
    }

    // ---- fields
    QTextCursor &caret() { return w->m_view->cursor(); }
    bool inText() const { return !w->m_view->inHeaderFooter() && !w->m_view->inNote(); }
    bool put(const QString &code, const QString &result)
    {
        if (!doc() || !inText()) {
            message(QCoreApplication::translate("jp::WriteMail", "Put the caret in the text of the document first."));
            return false;
        }
        const bool ok = write::mail::insertRule(*doc(), caret(), code, result);
        if (ok) {
            if (make().kind() == write::mail::Kind::None) {
                make().settings.type = write::mail::kindValue(write::mail::Kind::Letters);
                touch();
            }
            w->m_view->showCursor(caret());
            w->m_view->setFocus(Qt::OtherFocusReason);
        }
        return ok;
    }

    void addressBlock()
    {
        if (!doc() || !list(true)) return;
        write::mail::State &s = make();
        QString format = write::mail::addressBlockFormats().first(), home;
        if (!write::addressBlockDialog(w, s.data, s.fieldMap, &format, &home)) return;
        put(write::mail::addressBlockCode(format, home), shown(QCoreApplication::translate("jp::WriteMail", "Address Block")));
    }

    void greeting()
    {
        if (!doc() || !list(true)) return;
        write::mail::State &s = make();
        QString format = write::mail::greetingFormats().first(), other = QCoreApplication::translate("jp::WriteMail", "Dear Sir or Madam,");
        if (!write::greetingLineDialog(w, s.data, s.fieldMap, &format, &other)) return;
        put(write::mail::greetingCode(format, other), shown(QCoreApplication::translate("jp::WriteMail", "Greeting Line")));
    }

    void insertColumn(const QString &column) { put(write::mail::mergeFieldCode(column), shown(column)); }

    void rule(const QString &name)
    {
        if (!doc()) return;
        static const QHash<QString, QString> results = {{QStringLiteral("NEXT"), QCoreApplication::translate("jp::WriteMail", "Next Record")},
                                                        {QStringLiteral("MERGEREC"), QCoreApplication::translate("jp::WriteMail", "Merge Record #")},
                                                        {QStringLiteral("MERGESEQ"), QCoreApplication::translate("jp::WriteMail", "Merge Sequence #")},
                                                        {QStringLiteral("NEXTIF"), QCoreApplication::translate("jp::WriteMail", "Next Record If")},
                                                        {QStringLiteral("SKIPIF"), QCoreApplication::translate("jp::WriteMail", "Skip Record If")},
                                                        {QStringLiteral("IF"), QCoreApplication::translate("jp::WriteMail", "If…Then…Else")},
                                                        {QStringLiteral("ASK"), QCoreApplication::translate("jp::WriteMail", "Ask")},
                                                        {QStringLiteral("SET"), QCoreApplication::translate("jp::WriteMail", "Set Bookmark")},
                                                        {QStringLiteral("FILLIN"), QCoreApplication::translate("jp::WriteMail", "Fill-in")}};
        if (name == QLatin1String("NEXT") || name == QLatin1String("MERGEREC") || name == QLatin1String("MERGESEQ")) {
            put(name, shown(results.value(name)));
            return;
        }
        if (!list(true)) return;
        QString code;
        if (!write::ruleDialog(w, name, columns(), &code)) return;
        put(code, shown(results.value(name)));
    }

    void matchFields()
    {
        if (!doc() || !list(true)) return;
        write::mail::State &s = make();
        QHash<QString, QString> map = s.fieldMap;
        if (!write::matchFieldsDialog(w, s.data, &map)) return;
        s.fieldMap = map;
        touch();
        refreshPreview();
    }

    // Each cell of a sheet of labels after the first has the first one's content again, after a Next Record rule: the sheet is made to match the first label.
    void updateLabels()
    {
        using namespace write;
        if (!doc() || make().kind() != mail::Kind::Labels) {
            message(QCoreApplication::translate("jp::WriteMail", "Update Labels is for a document set up for mailing labels."));
            return;
        }
        QTextDocument *story = doc()->story();
        // The cells, by the key of the cell each paragraph is in; the first label's paragraphs are the model.
        QVector<QPair<int, int>> cells;   // (first position, end) of each cell's paragraphs, in order
        int lastKey = 0;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            const int key = cellKeyOf(b);
            if (key == 0) {
                lastKey = 0;
                continue;
            }
            if (key != lastKey) cells.push_back({b.position(), b.position() + b.length() - 1});
            else cells.last().second = b.position() + b.length() - 1;
            lastKey = key;
        }
        if (cells.isEmpty()) return;
        // (The cells that hold labels are those with a field of the merge; the narrow ones between are empty.)
        const StoryScan scan = scanStory(story, false);
        auto hasCode = [&](int from, int to, const QString &name) {
            for (int i = 0; i < scan.fields.size(); ++i)
                if (scan.fields[i].begin >= from && scan.fields[i].end <= to && fields::parseCode(scan.instructionOf(i)).name == name) return true;
            return false;
        };
        QVector<int> labelCells;
        for (int i = 0; i < cells.size(); ++i)
            if (hasCode(cells[i].first, cells[i].second, QStringLiteral("ADDRESSBLOCK")) || hasCode(cells[i].first, cells[i].second, QStringLiteral("NEXT")) || i == 0) labelCells << i;
        if (labelCells.size() < 2) return;
        // The model: the first label's text, with any Next Record rule left out of it.
        QTextDocumentFragment model;
        {
            QTextCursor c(story);
            c.setPosition(cells[labelCells.first()].first);
            c.setPosition(cells[labelCells.first()].second, QTextCursor::KeepAnchor);
            model = QTextDocumentFragment(c);
        }
        write::WriteDoc::StepGroup group(*doc());
        for (int k = int(labelCells.size()) - 1; k >= 1; --k) {
            const QPair<int, int> cell = cells[labelCells[k]];
            QTextCursor c(story);
            c.setPosition(cell.first);
            c.setPosition(cell.second, QTextCursor::KeepAnchor);
            c.insertFragment(model);
            QTextCursor at(story);
            at.setPosition(cell.first);
            insertField(*doc(), at, QStringLiteral("NEXT"), shown(QCoreApplication::translate("jp::WriteMail", "Next Record")));
        }
        w->m_view->viewport()->update();
        message(QCoreApplication::translate("jp::WriteMail", "The labels are updated."));
    }

    // ---- preview
    void refreshPreview()
    {
        if (!pane || !state() || !state()->loaded) return;
        pane->setRows(state()->data.includedRows(), int(state()->data.rows.size()));
    }

    void showPreview(bool on)
    {
        if (!doc()) return;
        if (!on) {
            if (dock) dock->hide();
            return;
        }
        if (!list(true)) {
            w->act("mail.preview")->setChecked(false);
            return;
        }
        if (!dock) {
            dock = new QDockWidget(QCoreApplication::translate("jp::WriteMail", "Preview Results"), w);
            dock->setObjectName(QStringLiteral("mailPreviewDock"));
            dock->setFeatures(QDockWidget::DockWidgetClosable);
            pane = new write::MailPreviewPane(
                [this](int row) -> std::unique_ptr<write::WriteDoc> {
                    if (!doc() || !state() || !state()->loaded) return nullptr;
                    write::mail::MergeOptions o;
                    o.rows = {row};
                    return write::mail::mergeToDocument(*doc(), state()->data, state()->fieldMap, o);
                },
                dock);
            dock->setWidget(pane);
            w->addDockWidget(Qt::RightDockWidgetArea, dock);
            w->resizeDocks({dock}, {520}, Qt::Horizontal);
            // (The window's own button follows the pane; it holds the action, not this, which goes before the pane does when the window closes.)
            QPointer<QAction> button = w->act("mail.preview");
            QObject::connect(dock, &QDockWidget::visibilityChanged, dock, [button](bool visible) {
                if (button) button->setChecked(visible);
            });
        }
        refreshPreview();
        dock->show();
        dock->raise();
    }

    void go(int how)
    {
        if (!doc() || !list(true)) return;
        showPreview(true);
        if (!pane) return;
        if (how == 0) pane->first();
        else if (how == 1) pane->previous();
        else if (how == 2) pane->next();
        else pane->last();
    }

    void find()
    {
        if (!doc() || !list(true)) return;
        showPreview(true);
        bool ok = false;
        const QString q = QInputDialog::getText(w, QCoreApplication::translate("jp::WriteMail", "Find Recipient"), QCoreApplication::translate("jp::WriteMail", "Find:"), QLineEdit::Normal, QString(), &ok);
        if (!ok || q.isEmpty() || !pane) return;
        const MergeSource &m = state()->data;
        const QVector<int> rows = m.includedRows();
        const int from = pane->place() + 1;
        for (int k = 0; k < rows.size(); ++k) {
            const int place = (from + k) % rows.size();
            for (const QString &v : m.rows[rows[place]])
                if (v.contains(q, Qt::CaseInsensitive)) {
                    pane->setPlace(place);
                    return;
                }
        }
        message(QCoreApplication::translate("jp::WriteMail", "No recipient has \"%1\".").arg(q));
    }

    void check()
    {
        if (!doc()) return;
        QStringList problems;
        if (!list(false)) problems << QCoreApplication::translate("jp::WriteMail", "There is no list of recipients. Choose one with Select Recipients.");
        else {
            if (state()->data.includedRows().isEmpty()) problems << QCoreApplication::translate("jp::WriteMail", "No recipient is included.");
            const QStringList missing = write::mail::missingColumns(*doc(), state()->data);
            for (const QString &m : missing) problems << QCoreApplication::translate("jp::WriteMail", "The list has no column named \"%1\", so that field stays empty.").arg(m);
        }
        w->notice(QCoreApplication::translate("jp::WriteMail", "Check for Errors"), problems.isEmpty() ? QCoreApplication::translate("jp::WriteMail", "No errors were found.") : problems.join(QLatin1Char('\n')));
    }

    // ---- finish
    std::unique_ptr<write::WriteDoc> merged()
    {
        if (!doc() || !list(true)) return nullptr;
        if (state()->data.includedRows().isEmpty()) {
            w->notice(QCoreApplication::translate("jp::WriteMail", "Mail Merge"), QCoreApplication::translate("jp::WriteMail", "No recipient is included."));
            return nullptr;
        }
        const write::mail::Kind kind = state()->kind();
        QString error;
        beginMerge();
        auto out = write::mail::mergeToDocument(*doc(), state()->data, state()->fieldMap, options(kind != write::mail::Kind::Directory), &error);
        endMerge();
        if (!out && m_cancelled) return nullptr;
        if (!out) w->notice(QCoreApplication::translate("jp::WriteMail", "Mail Merge"), QCoreApplication::translate("jp::WriteMail", "The merge can't be made.\n%1").arg(error), true);
        return out;
    }

    void finishEdit(bool print)
    {
        auto out = merged();
        if (!out) return;
        const QString title = QCoreApplication::translate("jp::WriteMail", "Merged Document");
        WriteWindow *target = showDocument(std::move(out), title);
        if (print) target->showFileScreen(QStringLiteral("print"));
    }

    void finishPdf()
    {
        auto out = merged();
        if (!out) return;
        const QString dir = w->m_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(w->m_path).absolutePath();
        const QString suggested = dir + QLatin1Char('/') + w->displayName() + QStringLiteral("-merged.pdf");
        QString path = w->m_pathChooser ? w->m_pathChooser(suggested, QCoreApplication::translate("jp::WriteMail", "PDF (*.pdf)")) : askSavePath(w, QCoreApplication::translate("jp::WriteMail", "Merge to PDF"), suggested, QCoreApplication::translate("jp::WriteMail", "PDF (*.pdf)"));
        if (path.isEmpty()) return;
        if (QFileInfo(path).suffix().compare(QLatin1String("pdf"), Qt::CaseInsensitive) != 0) path += QStringLiteral(".pdf");
        QVector<int> pages;
        const int count = out->pager().pageCount();
        for (int i = 0; i < count; ++i) pages << i;
        QSaveFile file(path);
        bool ok = !pages.isEmpty() && file.open(QIODevice::WriteOnly);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        if (ok) {
            {
                QPdfWriter pdf(&file);
                pdf.setCreator(appInfo().name);
                pdf.setTitle(w->displayName());
                pdf.setResolution(1200);
                pdf.setPageMargins(QMarginsF(0, 0, 0, 0));
                ok = write::paintSheets(*out, &pdf, pages, count);
            }
            ok = ok && file.commit();
        }
        QApplication::restoreOverrideCursor();
        if (!ok) w->notice(QCoreApplication::translate("jp::WriteMail", "Merge to PDF"), QCoreApplication::translate("jp::WriteMail", "The PDF can't be saved.\n%1").arg(file.errorString()), true);
        else message(QCoreApplication::translate("jp::WriteMail", "Saved %1").arg(QFileInfo(path).fileName()));
    }

    void finishEmail()
    {
        if (!doc() || !list(true)) return;
        write::mail::State &s = make();
        QString column, subject = s.settings.mailSubject, folder = w->m_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(w->m_path).absolutePath();
        if (!write::messageDialog(w, s.data.fields, &column, &subject, &folder)) return;
        s.settings.mailSubject = subject;
        s.settings.addressFieldName = column;
        touch();
        QString error;
        QVector<int> sourceRows;
        beginMerge();
        const QStringList texts = write::mail::mergeToTexts(*doc(), s.data, s.fieldMap, options(false), &error, &sourceRows);
        endMerge();
        if (m_cancelled) return;
        QApplication::setOverrideCursor(Qt::WaitCursor);
        int refused = 0;
        const int written = error.isEmpty() ? write::mail::writeEmailFiles(texts, s.data, sourceRows, column, subject, folder, &error, &refused) : 0;
        QApplication::restoreOverrideCursor();
        if (!error.isEmpty()) w->notice(QCoreApplication::translate("jp::WriteMail", "Mail Merge"), error, true);
        else if (refused > 0) w->notice(QCoreApplication::translate("jp::WriteMail", "Mail Merge"), QCoreApplication::translate("jp::WriteMail", "%1 message files are saved in %2.\n%3 recipients have no message, because their address is not the address of one person.").arg(written).arg(folder).arg(refused));
        else message(QCoreApplication::translate("jp::WriteMail", "%1 message files are saved in %2.").arg(written).arg(folder));
    }

    // ---- envelopes and labels without a merge
    void envelopes()
    {
        using namespace write;
        if (!doc()) return;
        EnvelopeChoice c;
        const bool merging = state() && state()->kind() != mail::Kind::None && list(false);
        if (!envelopeDialog(w, merging, &c)) return;
        const QVector<mail::EnvelopeSize> sizes = mail::envelopeSizes();
        const QByteArray runs = c.useBlock ? mail::fieldRuns(mail::addressBlockCode(QString()), shown(QCoreApplication::translate("jp::WriteMail", "Address Block"))) : mail::textRuns(c.delivery);
        auto env = mail::envelopeDocument(sizes.value(c.size), c.returnAddress, runs);
        if (!env) {
            w->notice(QCoreApplication::translate("jp::WriteMail", "Envelopes"), QCoreApplication::translate("jp::WriteMail", "The envelope can't be made."), true);
            return;
        }
        if (c.print) {
            WriteWindow *target = showDocument(std::move(env), QCoreApplication::translate("jp::WriteMail", "Envelope"));
            target->showFileScreen(QStringLiteral("print"));
            return;
        }
        addToDocument(*env);
    }

    void addToDocument(write::WriteDoc &env)
    {
        if (!write::mail::addAsFirstSection(*doc(), env)) {
            w->notice(QCoreApplication::translate("jp::WriteMail", "Envelopes"), QCoreApplication::translate("jp::WriteMail", "The envelope can't be added here."), true);
            return;
        }
        w->m_view->viewport()->update();
        message(QCoreApplication::translate("jp::WriteMail", "The envelope is added at the start of the document."));
    }

    void labels()
    {
        using namespace write;
        if (!doc()) return;
        LabelChoice c;
        c.custom = mail::labelSheets().first();
        const bool merging = state() && state()->kind() != mail::Kind::None && list(false);
        if (!labelsDialog(w, merging, &c)) return;
        const mail::LabelSheet sheet = c.sheet < 0 ? c.custom : mail::labelSheets().value(c.sheet);
        const QByteArray runs = c.useBlock ? mail::fieldRuns(mail::addressBlockCode(QString()), shown(QCoreApplication::translate("jp::WriteMail", "Address Block"))) : mail::textRuns(c.address);
        const int only = c.single ? (c.row - 1) * sheet.columns + (c.column - 1) : -1;
        auto d = mail::labelDocument(sheet, [&](int i) { return only >= 0 && i != only ? QByteArray() : mail::tightParagraph(runs); });
        if (!d) {
            w->notice(QCoreApplication::translate("jp::WriteMail", "Labels"), QCoreApplication::translate("jp::WriteMail", "The labels can't be made."), true);
            return;
        }
        WriteWindow *target = showDocument(std::move(d), QCoreApplication::translate("jp::WriteMail", "Labels"));
        if (c.print) target->showFileScreen(QStringLiteral("print"));
    }

    void wizard()
    {
        if (!wizardDock) {
            wizardDock = new QDockWidget(QCoreApplication::translate("jp::WriteMail", "Mail Merge"), w);
            wizardDock->setObjectName(QStringLiteral("mailWizardDock"));
            wizardDock->setFeatures(QDockWidget::DockWidgetClosable);
            wizardDock->setWidget(new WizardPane(w, wizardDock));
            w->addDockWidget(Qt::RightDockWidgetArea, wizardDock);
            w->resizeDocks({wizardDock}, {300}, Qt::Horizontal);
        }
        wizardDock->show();
        wizardDock->raise();
    }
};

void WriteWindow::createMailActions()
{
    auto ui = std::make_shared<MailUi>(MailUi{this});
    m_mailHolder = ui;
    m_mail = ui.get();
    MailUi *m = m_mail;
    using write::mail::Kind;
    mk("mail.type.letters", tr("Letters"), "mail", QKeySequence(), [m] { m->start(Kind::Letters); });
    mk("mail.type.email", tr("Email Messages"), "mail", QKeySequence(), [m] { m->start(Kind::Email); });
    mk("mail.type.envelopes", tr("Envelopes…"), "mail-open", QKeySequence(), [m] { m->start(Kind::Envelopes); });
    mk("mail.type.labels", tr("Labels…"), "tag", QKeySequence(), [m] { m->start(Kind::Labels); });
    mk("mail.type.directory", tr("Directory"), "book-user", QKeySequence(), [m] { m->start(Kind::Directory); });
    mk("mail.type.clear", tr("Clear Merge Type"), "circle-x", QKeySequence(), [m] { m->clear(); });
    mk("mail.wizard", tr("Step-by-Step Mail Merge Wizard"), "wand-sparkles", QKeySequence(), [m] { m->wizard(); });
    mk("mail.recipients.existing", tr("Use an Existing List…"), "file-spreadsheet", QKeySequence(), [m] {
        if (m->chooseList(false)) m->editList();
    });
    mk("mail.recipients.contacts", tr("Select from Contacts…"), "contact", QKeySequence(), [m] {
        if (m->chooseList(true)) m->editList();
    });
    mk("mail.recipients.typeNew", tr("Type a New List…"), "user-plus", QKeySequence(), [m] { m->typeNew(); });
    mk("mail.editList", tr("Edit Recipient List"), "user-pen", QKeySequence(), [m] { m->editList(); });
    mk("mail.addressBlock", tr("Address Block…"), "mail-open", QKeySequence(), [m] { m->addressBlock(); });
    mk("mail.greeting", tr("Greeting Line…"), "hand", QKeySequence(), [m] { m->greeting(); });
    mk("mail.highlight", tr("Highlight Merge Fields"), "highlighter", QKeySequence(), [this] { m_view->setShadeFields(act("mail.highlight")->isChecked()); }, true);
    mk("mail.matchFields", tr("Match Fields…"), "arrow-left-right", QKeySequence(), [m] { m->matchFields(); });
    mk("mail.updateLabels", tr("Update Labels"), "refresh-cw", QKeySequence(), [m] { m->updateLabels(); });
    mk("mail.rule.if", tr("If…Then…Else…"), "", QKeySequence(), [m] { m->rule(QStringLiteral("IF")); });
    mk("mail.rule.ask", tr("Ask…"), "", QKeySequence(), [m] { m->rule(QStringLiteral("ASK")); });
    mk("mail.rule.fillin", tr("Fill-in…"), "", QKeySequence(), [m] { m->rule(QStringLiteral("FILLIN")); });
    mk("mail.rule.mergeRec", tr("Merge Record #"), "", QKeySequence(), [m] { m->rule(QStringLiteral("MERGEREC")); });
    mk("mail.rule.mergeSeq", tr("Merge Sequence #"), "", QKeySequence(), [m] { m->rule(QStringLiteral("MERGESEQ")); });
    mk("mail.rule.next", tr("Next Record"), "", QKeySequence(), [m] { m->rule(QStringLiteral("NEXT")); });
    mk("mail.rule.nextIf", tr("Next Record If…"), "", QKeySequence(), [m] { m->rule(QStringLiteral("NEXTIF")); });
    mk("mail.rule.set", tr("Set Bookmark…"), "", QKeySequence(), [m] { m->rule(QStringLiteral("SET")); });
    mk("mail.rule.skipIf", tr("Skip Record If…"), "", QKeySequence(), [m] { m->rule(QStringLiteral("SKIPIF")); });
    mk("mail.envelopes", tr("Envelopes…"), "mail", QKeySequence(), [m] { m->envelopes(); });
    mk("mail.labels", tr("Labels…"), "tag", QKeySequence(), [m] { m->labels(); });
    mk("mail.preview", tr("Preview Results"), "eye", QKeySequence(), [m, this] { m->showPreview(act("mail.preview")->isChecked()); }, true);
    mk("mail.first", tr("First Record"), "chevrons-left", QKeySequence(), [m] { m->go(0); });
    mk("mail.prev", tr("Previous Record"), "chevron-left", QKeySequence(), [m] { m->go(1); });
    mk("mail.next", tr("Next Record"), "chevron-right", QKeySequence(), [m] { m->go(2); });
    mk("mail.last", tr("Last Record"), "chevrons-right", QKeySequence(), [m] { m->go(3); });
    mk("mail.find", tr("Find Recipient"), "search", QKeySequence(), [m] { m->find(); });
    mk("mail.check", tr("Check for Errors"), "circle-check", QKeySequence(), [m] { m->check(); });
    mk("mail.finish.edit", tr("Edit Individual Documents…"), "files", QKeySequence(), [m] { m->finishEdit(false); });
    mk("mail.finish.print", tr("Print Documents…"), "printer", QKeySequence(), [m] { m->finishEdit(true); });
    mk("mail.finish.pdf", tr("Save as PDF…"), "file-text", QKeySequence(), [m] { m->finishPdf(); });
    mk("mail.finish.email", tr("Send Email Messages…"), "mail", QKeySequence(), [m] { m->finishEmail(); });
}

void WriteWindow::mailRibbonParts(RibbonParts &parts)
{
    // Insert Merge Field: a menu of the list's columns, made when it opens.
    parts.menus[QStringLiteral("mailFields")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        QObject::connect(menu, &QMenu::aboutToShow, menu, [this, menu] {
            menu->clear();
            m_mail->list(false);
            const QStringList columns = m_mail->columns();
            if (columns.isEmpty()) {
                menu->addAction(tr("Choose the recipients first"))->setEnabled(false);
                return;
            }
            for (const QString &column : columns) menu->addAction(column, menu, [this, column] { m_mail->insertColumn(column); });
        });
        return menu;
    };
}

} // namespace jp

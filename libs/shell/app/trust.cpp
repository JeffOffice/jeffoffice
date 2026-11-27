#include "app/trust.h"

#include "app/plaintext.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/remotepath.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QSet>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#ifndef Q_OS_WIN
#include <sys/types.h>
#include <sys/xattr.h>
#endif

namespace jp::trust {

namespace {

std::function<bool(const QString &)> g_outsideStandIn;
std::function<bool(const QString &)> g_askStandIn;

Choice choiceOf(const QString &key, Choice fallback)
{
    const QString v = Settings::get().value(key).toString();
    if (v == QLatin1String("ask")) return Choice::Ask;
    if (v == QLatin1String("yes")) return Choice::Yes;
    if (v == QLatin1String("never")) return Choice::Never;
    return fallback;
}

void storeChoice(const QString &key, Choice c)
{
    Settings::get().setValue(key, c == Choice::Ask ? QStringLiteral("ask") : c == Choice::Yes ? QStringLiteral("yes") : QStringLiteral("never"));
}

// The system's own mark on a file that was downloaded: the zone of a Windows file (3 is the internet, 4 is untrusted), the quarantine mark of a Mac, and, on Linux, the address a browser saved.
bool markedAsDownloaded(const QString &path)
{
#ifdef Q_OS_WIN
    QFile zone(path + QStringLiteral(":Zone.Identifier"));
    if (!zone.open(QIODevice::ReadOnly)) return false;
    const QByteArray text = zone.read(4096);
    const qsizetype at = text.indexOf("ZoneId=");
    return at >= 0 && text.mid(at + 7, 1).toInt() >= 3;
#else
    const QByteArray name = QFile::encodeName(path);
#ifdef Q_OS_MACOS
    return getxattr(name.constData(), "com.apple.quarantine", nullptr, 0, 0, 0) >= 0;
#else
    return getxattr(name.constData(), "user.xdg.origin.url", nullptr, 0) >= 0 || getxattr(name.constData(), "user.xdg.referrer.url", nullptr, 0) >= 0;
#endif
#endif
}

const QString kApprovalsKey = QStringLiteral("trust/approvals");
const QString kApprovalKeyKey = QStringLiteral("trust/approvalKey");

// The random key (32 bytes, as hex) this person's hashes are made with; made the first time it is needed. A key that is not what we wrote is replaced, which forgets the pairs made with the old one.
QByteArray approvalKey()
{
    Settings &st = Settings::get();
    const QByteArray stored = QByteArray::fromHex(st.value(kApprovalKeyKey).toString().toLatin1());
    if (stored.size() == 32) return stored;
    QByteArray fresh(32, '\0');
    for (int i = 0; i < 32; i += 4) {
        const quint32 r = QRandomGenerator::system()->generate();
        memcpy(fresh.data() + i, &r, 4);
    }
    st.setValue(kApprovalKeyKey, QString::fromLatin1(fresh.toHex()));
    st.remove(kApprovalsKey);
    return fresh;
}

// One pair as it is kept: 32 hex digits for each path.
QString approvalEntry(const QString &workbook, const QString &file)
{
    if (workbook.isEmpty() || file.isEmpty() || workbook.size() > kLongestApprovedPath || file.size() > kLongestApprovedPath) return QString();
    const QByteArray key = approvalKey();
    auto part = [&](const QString &path) { return QMessageAuthenticationCode::hash(path.toUtf8(), key, QCryptographicHash::Sha256).left(16).toHex(); };
    return QString::fromLatin1(part(workbook) + part(file));
}

// The kept entries, oldest first: only those of the form written (64 lowercase hex digits), no repeats, never more than kMostApprovals (the oldest are left off).
QStringList loadApprovals()
{
    static const QRegularExpression form(QStringLiteral("^[0-9a-f]{64}$"));
    QStringList out;
    QSet<QString> seen;
    for (const QString &e : Settings::get().value(kApprovalsKey).toStringList())
        if (form.match(e).hasMatch() && !seen.contains(e)) {
            seen.insert(e);
            out << e;
        }
    if (out.size() > kMostApprovals) out = out.mid(out.size() - kMostApprovals);
    return out;
}

void storeApprovals(const QStringList &list)
{
    if (list.isEmpty()) Settings::get().remove(kApprovalsKey);
    else Settings::get().setValue(kApprovalsKey, list);
}

} // namespace

bool readApproved(const QString &workbook, const QString &file)
{
    const QString entry = approvalEntry(workbook, file);
    if (entry.isEmpty()) return false;
    QStringList list = loadApprovals();
    const qsizetype at = list.indexOf(entry);
    if (at < 0) return false;
    if (at != list.size() - 1) {   // (used just now: it goes last, so the one used longest ago is the one to go)
        list.move(at, list.size() - 1);
        storeApprovals(list);
    }
    return true;
}

void rememberRead(const QString &workbook, const QString &file)
{
    const QString entry = approvalEntry(workbook, file);
    if (entry.isEmpty()) return;
    QStringList list = loadApprovals();
    list.removeAll(entry);
    list << entry;
    while (list.size() > kMostApprovals) list.removeFirst();
    storeApprovals(list);
}

int approvalCount() { return int(loadApprovals().size()); }
void forgetApprovals() { Settings::get().remove(kApprovalsKey); }

static QString filesKey(const QString &app) { return QStringLiteral("trust/linkedFiles/") + (app.isEmpty() ? appInfo().fileName : app); }
static bool asksAboutFiles(const QString &app) { return (app.isEmpty() ? appInfo().fileName : app) != QLatin1String("jeffpub"); }

Choice linkedFiles(const QString &app)
{
    const Choice c = choiceOf(filesKey(app), asksAboutFiles(app) ? Choice::Ask : Choice::Yes);
    return c == Choice::Ask && !asksAboutFiles(app) ? Choice::Yes : c;
}
void setLinkedFiles(Choice c, const QString &app) { storeChoice(filesKey(app), c); }
Choice linkedMedia() { return choiceOf(QStringLiteral("trust/linkedMedia"), Choice::Never); }
void setLinkedMedia(Choice c) { storeChoice(QStringLiteral("trust/linkedMedia"), c); }
bool outsideFilesReadOnly() { return Settings::get().value(QStringLiteral("trust/outsideReadOnly"), true).toBool(); }
void setOutsideFilesReadOnly(bool on) { Settings::get().setValue(QStringLiteral("trust/outsideReadOnly"), on); }

void setOutsideStandIn(const std::function<bool(const QString &)> &standIn) { g_outsideStandIn = standIn; }

bool fromOutside(const QString &path)
{
    if (g_outsideStandIn) return g_outsideStandIn(path);
    if (path.isEmpty()) return false;
    // (a file on another computer is judged by its name: asking the system about it would reach that computer)
    if (isOnAnotherComputer(path)) return true;
    return markedAsDownloaded(path);
}

OpenHow openHowFor(const QString &path, OpenHow how, bool *held)
{
    if (held) *held = false;
    if (how != OpenHow::Normal || !outsideFilesReadOnly() || editingAllowed(path) || !fromOutside(path)) return how;
    if (held) *held = true;
    return OpenHow::ReadOnly;
}

QString heldBackText()
{
    return QCoreApplication::translate("Trust", "This file came from the internet or another computer, so it is open for reading only.");
}

namespace {
QSet<QString> g_editingAllowed;
QString canonicalOrClean(const QString &path)
{
    const QString c = QFileInfo(path).canonicalFilePath();
    return c.isEmpty() ? QDir::cleanPath(path) : c;
}
} // namespace

void allowEditing(const QString &path) { g_editingAllowed.insert(canonicalOrClean(path)); }
bool editingAllowed(const QString &path) { return g_editingAllowed.contains(canonicalOrClean(path)); }

bool clearOutsideMark(const QString &path)
{
    if (path.isEmpty() || isOnAnotherComputer(path)) return false;
#ifdef Q_OS_WIN
    return QFile::remove(path + QStringLiteral(":Zone.Identifier"));
#else
    const QByteArray name = QFile::encodeName(path);
    bool any = false;
#ifdef Q_OS_MACOS
    any = removexattr(name.constData(), "com.apple.quarantine", 0) == 0;
#else
    any = removexattr(name.constData(), "user.xdg.origin.url") == 0;
    any = removexattr(name.constData(), "user.xdg.referrer.url") == 0 || any;
#endif
    return any;
#endif
}

void setAskStandIn(const std::function<bool(const QString &)> &standIn) { g_askStandIn = standIn; }

bool ask(QWidget *parent, const QString &title, const QString &question, const QString &detail)
{
    if (g_askStandIn) return g_askStandIn(question);
    jp::MessageBox box(QMessageBox::Question, title, question, QMessageBox::NoButton, parent);
    box.setTextFormat(Qt::PlainText);
    if (!detail.isEmpty()) box.setInformativeText(detail);
    box.addButton(QCoreApplication::translate("Trust", "Yes"), QMessageBox::AcceptRole);
    QPushButton *no = box.addButton(QCoreApplication::translate("Trust", "No"), QMessageBox::RejectRole);
    box.setDefaultButton(no);
    box.setEscapeButton(no);
    box.exec();
    return box.clickedButton() != no && box.result() != QMessageBox::Cancel;
}

Applies appliesTo(const QString &appFileName)
{
    Applies a;
    a.app = appFileName;
    if (appFileName == QLatin1String("jeffpub")) {
        a.linkedFiles = true;
        a.linkedFilesAbout = QCoreApplication::translate("Trust", "Pictures that are linked to a file instead of kept in the publication. Only a file in the publication's own folder is read when it opens, and no other is ever read.");
    } else if (appFileName == QLatin1String("jeffcrunch")) {
        a.linkedFiles = true;
        a.linkedFilesAsks = true;
        a.linkedFilesAbout = QCoreApplication::translate("Trust", "Values a workbook reads from other workbooks. A workbook on another computer is never read when a file opens.");
    } else if (appFileName == QLatin1String("jeffdeck")) {
        a.linkedMedia = true;
        a.linkedMediaAbout = QCoreApplication::translate("Trust", "A movie or sound that a slide links to a file instead of holding. Only a movie or sound file whose name is on this computer is played; a web address, a file on another computer, and a program never are. A network drive that looks like a local one is read like a local file.");
    }
    return a;
}

TrustPage::TrustPage(const Applies &applies, QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("trustPage"));
    auto *outer = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    outer->addLayout(form);
    auto note = [this](const QString &text, const QString &name = QString()) {
        auto *l = jp::plainLabel(new QLabel(text, this));
        l->setWordWrap(true);
        if (!name.isEmpty()) l->setObjectName(name);
        return l;
    };
    // Macros: not a choice.
    form->addRow(tr("Macros"), note(tr("Macros in a file are kept in it, and are never run. There is no setting to run them."), QStringLiteral("trustMacros")));
    auto choiceBox = [this](const QString &name, const QStringList &items, bool enabled) {
        auto *box = new QComboBox(this);
        box->setObjectName(name);
        for (int i = 0; i < items.size(); ++i) box->addItem(items[i], i);
        box->setEnabled(enabled);
        return box;
    };
    auto row = [&](const QString &label, QComboBox *box, bool applies, const QString &about, const QString &none) {
        auto *col = new QWidget(this);
        auto *cl = new QVBoxLayout(col);
        cl->setContentsMargins(0, 0, 0, 0);
        cl->addWidget(box);
        auto *text = note(applies ? about : none, box->objectName() + QStringLiteral("Note"));
        cl->addWidget(text);
        form->addRow(label, col);
    };
    // (JeffPub has no question to put: it reads what is in the publication's folder, or nothing)
    m_files = applies.linkedFilesAsks ? choiceBox(QStringLiteral("trustLinkedFiles"), {tr("Ask me first"), tr("Update them"), tr("Never update them")}, applies.linkedFiles)
                                      : choiceBox(QStringLiteral("trustLinkedFiles"), {tr("Update them"), tr("Never update them")}, applies.linkedFiles);
    m_filesApp = applies.app;
    m_filesAsks = applies.linkedFilesAsks;
    m_files->setCurrentIndex(m_filesAsks ? int(linkedFiles(applies.app)) : linkedFiles(applies.app) == Choice::Never ? 1 : 0);
    row(tr("Linked files and workbook links:"), m_files, applies.linkedFiles, applies.linkedFilesAbout, tr("This doesn't apply in this app: it has no linked files to update."));
    if (applies.linkedFilesAsks) {
        auto *line = new QWidget(this);
        auto *hl = new QHBoxLayout(line);
        hl->setContentsMargins(0, 0, 0, 0);
        m_approvals = jp::plainLabel(new QLabel(this));
        m_approvals->setObjectName(QStringLiteral("trustApprovalCount"));
        m_forget = new QPushButton(tr("Forget All"), this);
        m_forget->setObjectName(QStringLiteral("trustForgetAll"));
        hl->addWidget(m_approvals);
        hl->addStretch(1);
        hl->addWidget(m_forget);
        auto show = [this] {
            const int n = approvalCount();
            // plain: reviewed (a count of ours)
            m_approvals->setText(tr("Remembered approvals: %1").arg(n));
            m_forget->setEnabled(n > 0);
        };
        show();
        connect(m_forget, &QPushButton::clicked, this, [show] {
            forgetApprovals();
            show();
        });
        form->addRow(QString(), line);
    }
    m_media = choiceBox(QStringLiteral("trustLinkedMedia"), {tr("Ask me first"), tr("Play it"), tr("Never play it")}, applies.linkedMedia);
    m_media->setCurrentIndex(int(linkedMedia()));
    row(tr("Linked movies and sounds:"), m_media, applies.linkedMedia, applies.linkedMediaAbout, tr("This doesn't apply in this app: it has no linked movies or sounds to play."));
    m_outside = choiceBox(QStringLiteral("trustOutside"), {tr("Open as read-only first"), tr("Open normally")}, true);
    m_outside->setCurrentIndex(outsideFilesReadOnly() ? 0 : 1);
    row(tr("Files from the internet or other computers:"), m_outside, true,
        tr("A file that was downloaded, or is on another computer, opens for reading only. The bar under it has Enable Editing for that one file, and Save As makes a copy you can change."), QString());
    form->addRow(tr("Links that start a program"), note(tr("These are always refused, and there is no setting to allow them. A link to a file on another computer is never followed without asking."), QStringLiteral("trustPrograms")));
    outer->addStretch(1);
}

void TrustPage::apply()
{
    if (m_files->isEnabled()) setLinkedFiles(m_filesAsks ? Choice(m_files->currentData().toInt()) : m_files->currentData().toInt() == 1 ? Choice::Never : Choice::Yes, m_filesApp);
    if (m_media->isEnabled()) setLinkedMedia(Choice(m_media->currentData().toInt()));
    setOutsideFilesReadOnly(m_outside->currentIndex() == 0);
}

} // namespace jp::trust

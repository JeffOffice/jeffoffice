#include "app/pagesetupdialog.h"

#include "sheet/headerfooter.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFontDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace jp {

using namespace jp::office::sheet;

namespace {

QString tr_(const char *text) { return QCoreApplication::translate("jp::PageSetupDialog", text); }

jp::office::sheet::HfContext sampleContext(const QString &sheet, const QString &file)
{
    HfContext c;
    c.page = 1;
    c.pages = 3;
    c.now = QDateTime(QDate::currentDate(), QTime(9, 30));
    c.sheet = sheet;
    c.file = file;
    c.path = QStringLiteral("/");
    c.locale = QLocale();
    return c;
}

double round2(double v) { return std::round(v * 100.0) / 100.0; }

// "A1:D20" for a block, "3:5" for whole rows, "A:C" for whole columns: how the boxes write them.
QString blockWords(const CellRange &r)
{
    const bool allRows = r.row1 == 0 && r.row2 == kMaxRows - 1, allCols = r.col1 == 0 && r.col2 == kMaxColumns - 1;
    if (allRows && !allCols) return columnName(r.col1) + QLatin1Char(':') + columnName(r.col2);
    if (allCols && !allRows) return QString::number(r.row1 + 1) + QLatin1Char(':') + QString::number(r.row2 + 1);
    return rangeName(r);
}

QString areaWords(const PageSetup &p)
{
    QStringList l;
    for (const CellRange &r : p.printArea) l << blockWords(r);
    return l.join(QStringLiteral(", "));
}

} // namespace

// ---- the editor of a header or footer

HeaderFooterEditor::HeaderFooterEditor(bool header, const HeaderFooter &texts, const QString &sheetName, const QString &fileName, QWidget *parent)
    : QDialog(parent), m_header(header), m_hf(texts), m_sheetName(sheetName), m_fileName(fileName)
{
    setObjectName(QStringLiteral("headerFooterEditor"));
    setWindowTitle(header ? tr("Header Text") : tr("Footer Text"));
    // the versions the setup keeps: every page (or the odd ones and the even ones), and the first page
    if (m_hf.differentOddEven) {
        m_versions.push_back({tr("Odd pages"), header ? &m_hf.oddHeader : &m_hf.oddFooter});
        m_versions.push_back({tr("Even pages"), header ? &m_hf.evenHeader : &m_hf.evenFooter});
    } else {
        m_versions.push_back({tr("Every page"), header ? &m_hf.oddHeader : &m_hf.oddFooter});
    }
    if (m_hf.differentFirst) m_versions.push_back({tr("First page"), header ? &m_hf.firstHeader : &m_hf.firstFooter});
    auto *v = new QVBoxLayout(this);
    auto *intro = new QLabel(tr("Type the words for each part of the page, and use the buttons to add things that change from page to page. The parts print at the left, in the middle, and at the right."), this);
    intro->setWordWrap(true);
    v->addWidget(intro);
    auto *top = new QHBoxLayout;
    top->addWidget(new QLabel(tr("Text for:"), this));
    m_version = new QComboBox(this);
    m_version->setObjectName(QStringLiteral("hfVersion"));
    m_version->setAccessibleName(tr("Which pages this text is for"));
    for (const Version &ver : m_versions) m_version->addItem(ver.name);
    top->addWidget(m_version, 1);
    v->addLayout(top);
    auto *buttons = new QHBoxLayout;
    struct Insert {
        const char *label, *tip, *code, *name;
    };
    static const Insert inserts[] = {{QT_TR_NOOP("Page number"), QT_TR_NOOP("The number of the page"), "&P", "hfPageNumber"},
                                     {QT_TR_NOOP("Pages"), QT_TR_NOOP("How many pages there are"), "&N", "hfPageCount"},
                                     {QT_TR_NOOP("Date"), QT_TR_NOOP("The day it is printed"), "&D", "hfDate"},
                                     {QT_TR_NOOP("Time"), QT_TR_NOOP("The time it is printed"), "&T", "hfTime"},
                                     {QT_TR_NOOP("File name"), QT_TR_NOOP("The name of the workbook's file"), "&F", "hfFile"},
                                     {QT_TR_NOOP("Folder"), QT_TR_NOOP("The folder the file is in"), "&Z", "hfFolder"},
                                     {QT_TR_NOOP("Sheet name"), QT_TR_NOOP("The name of the sheet"), "&A", "hfSheet"}};
    for (const Insert &ins : inserts) {
        auto *b = new QPushButton(tr(ins.label), this);
        b->setObjectName(QString::fromLatin1(ins.name));
        b->setToolTip(tr(ins.tip));
        b->setAutoDefault(false);
        const QString code = QString::fromLatin1(ins.code);
        connect(b, &QPushButton::clicked, this, [this, code] { insert(code); });
        buttons->addWidget(b);
    }
    buttons->addStretch(1);
    v->addLayout(buttons);
    auto *styles = new QHBoxLayout;
    struct Style {
        const char *label, *code, *name;
    };
    static const Style styleButtons[] = {{QT_TR_NOOP("Bold"), "&B", "hfBold"}, {QT_TR_NOOP("Italic"), "&I", "hfItalic"}, {QT_TR_NOOP("Underline"), "&U", "hfUnderline"}};
    for (const Style &st : styleButtons) {
        auto *b = new QPushButton(tr(st.label), this);
        b->setObjectName(QString::fromLatin1(st.name));
        b->setToolTip(tr("Turns it on where you are typing, and off again the next time"));
        b->setAutoDefault(false);
        const QString code = QString::fromLatin1(st.code);
        connect(b, &QPushButton::clicked, this, [this, code] { insert(code); });
        styles->addWidget(b);
    }
    auto *size = new QSpinBox(this);
    size->setObjectName(QStringLiteral("hfSize"));
    size->setRange(1, 409);
    size->setValue(11);
    size->setSuffix(tr(" pt"));
    size->setAccessibleName(tr("Letter size in points"));
    auto *useSize = new QPushButton(tr("Set size"), this);
    useSize->setObjectName(QStringLiteral("hfSetSize"));
    useSize->setAutoDefault(false);
    connect(useSize, &QPushButton::clicked, this, [this, size] { insert(QStringLiteral("&%1").arg(size->value())); });
    auto *font = new QPushButton(tr("Font…"), this);
    font->setObjectName(QStringLiteral("hfFont"));
    font->setAutoDefault(false);
    connect(font, &QPushButton::clicked, this, [this] {
        bool ok = false;
        const QFont f = QFontDialog::getFont(&ok, QFont(), this, tr("Font"));
        if (ok) insert(QStringLiteral("&\"%1,%2\"").arg(f.family(), f.bold() && f.italic() ? tr("Bold Italic") : f.bold() ? tr("Bold") : f.italic() ? tr("Italic") : tr("Regular")));
    });
    styles->addWidget(size);
    styles->addWidget(useSize);
    styles->addWidget(font);
    styles->addStretch(1);
    v->addLayout(styles);
    static const char *const names[3] = {QT_TR_NOOP("Left part"), QT_TR_NOOP("Middle part"), QT_TR_NOOP("Right part")};
    static const char *const objectNames[3] = {"hfLeft", "hfCenter", "hfRight"};
    for (int i = 0; i < 3; ++i) {
        v->addWidget(new QLabel(tr(names[i]), this));
        m_boxes[i] = new QPlainTextEdit(this);
        m_boxes[i]->setObjectName(QString::fromLatin1(objectNames[i]));
        m_boxes[i]->setAccessibleName(tr(names[i]));
        m_boxes[i]->setFixedHeight(54);
        m_boxes[i]->installEventFilter(this);
        connect(m_boxes[i], &QPlainTextEdit::textChanged, this, [this] {
            if (!m_loading) refreshPreview();
        });
        v->addWidget(m_boxes[i]);
    }
    m_focused = m_boxes[1];
    m_preview = new QLabel(this);
    m_preview->setObjectName(QStringLiteral("hfPreview"));
    m_preview->setWordWrap(true);
    m_preview->setFrameShape(QFrame::StyledPanel);
    v->addWidget(m_preview);
    auto *ok = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(ok, &QDialogButtonBox::accepted, this, [this] {
        commit();
        accept();
    });
    connect(ok, &QDialogButtonBox::rejected, this, &QDialog::reject);
    v->addWidget(ok);
    connect(m_version, &QComboBox::currentIndexChanged, this, [this](int i) { selectVersion(i); });
    load(0);
}

bool HeaderFooterEditor::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FocusIn)
        for (QPlainTextEdit *b : m_boxes)
            if (b == o) m_focused = b;
    return QDialog::eventFilter(o, e);
}

QStringList HeaderFooterEditor::versionNames() const
{
    QStringList l;
    for (const Version &v : m_versions) l << v.name;
    return l;
}

QPlainTextEdit *HeaderFooterEditor::box(int part) const { return m_boxes[std::clamp(part, 0, 2)]; }

void HeaderFooterEditor::selectVersion(int index)
{
    if (index < 0 || index >= int(m_versions.size()) || index == m_current) return;
    commit();
    load(index);
    const QSignalBlocker block(m_version);
    m_version->setCurrentIndex(index);
}

void HeaderFooterEditor::load(int version)
{
    m_current = version;
    QString l, c, r;
    splitHeaderFooter(*m_versions[size_t(version)].text, &l, &c, &r);
    m_loading = true;
    m_boxes[0]->setPlainText(l);
    m_boxes[1]->setPlainText(c);
    m_boxes[2]->setPlainText(r);
    m_loading = false;
    refreshPreview();
}

void HeaderFooterEditor::commit()
{
    if (m_current < 0 || m_current >= int(m_versions.size())) return;
    *m_versions[size_t(m_current)].text = composeHeaderFooter(m_boxes[0]->toPlainText(), m_boxes[1]->toPlainText(), m_boxes[2]->toPlainText()).left(kMaxHeaderFooterChars);
}

HeaderFooter HeaderFooterEditor::texts() const
{
    HeaderFooterEditor *self = const_cast<HeaderFooterEditor *>(this);
    self->commit();
    return m_hf;
}

void HeaderFooterEditor::insert(const QString &code)
{
    (m_focused ? m_focused : m_boxes[1])->insertPlainText(code);
    (m_focused ? m_focused : m_boxes[1])->setFocus();
}

void HeaderFooterEditor::refreshPreview()
{
    const QString code = composeHeaderFooter(m_boxes[0]->toPlainText(), m_boxes[1]->toPlainText(), m_boxes[2]->toPlainText());
    const QString text = headerFooterPreview(code, sampleContext(m_sheetName, m_fileName));
    m_preview->setText(text.isEmpty() ? tr("It prints nothing here.") : tr("It will read: %1").arg(text));
}

// ---- the dialog

QVector<QPair<QString, QString>> PageSetupDialog::headerFooterPresets(const QString &sheetName, const QString &fileName)
{
    const HfContext c = sampleContext(sheetName, fileName);
    const QStringList codes = {QString(),
                               QStringLiteral("&CPage &P"),
                               QStringLiteral("&CPage &P of &N"),
                               QStringLiteral("&C&A"),
                               QStringLiteral("&C&F"),
                               QStringLiteral("&L&A&RPage &P"),
                               QStringLiteral("&L&D&RPage &P"),
                               QStringLiteral("&L&F&C&A&R&D")};
    QVector<QPair<QString, QString>> out;
    for (const QString &code : codes) out.append({code.isEmpty() ? tr_("(nothing)") : headerFooterPreview(code, c), code});
    return out;
}

PageSetupDialog::PageSetupDialog(const PageSetupRequest &request, QWidget *parent) : QDialog(parent), m_request(request), m_setup(request.setup)
{
    setObjectName(QStringLiteral("pageSetupDialog"));
    setWindowTitle(tr("Page Setup"));
    auto *v = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName(QStringLiteral("pageSetupTabs"));
    auto *page = new QWidget, *margins = new QWidget, *header = new QWidget, *sheet = new QWidget;
    m_tabs->addTab(page, tr("Page"));
    m_tabs->addTab(margins, tr("Margins"));
    m_tabs->addTab(header, tr("Header and Footer"));
    m_tabs->addTab(sheet, tr("Sheet"));
    buildPage(page);
    buildMargins(margins);
    buildHeaderFooter(header);
    buildSheet(sheet);
    v->addWidget(m_tabs);
    m_problem = new QLabel(this);
    m_problem->setObjectName(QStringLiteral("psProblem"));
    m_problem->setWordWrap(true);
    m_problem->setStyleSheet(QStringLiteral("color: #c42b1c;"));
    m_problem->hide();
    v->addWidget(m_problem);
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &PageSetupDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    v->addWidget(m_buttons);
    m_tabs->setCurrentIndex(std::clamp(request.tab, 0, 3));
    m_initial = view();
}

void PageSetupDialog::buildPage(QWidget *tab)
{
    auto *v = new QVBoxLayout(tab);
    auto *direction = new QGroupBox(tr("Which way the paper turns"), tab);
    auto *dl = new QHBoxLayout(direction);
    m_portrait = new QRadioButton(tr("Tall"), direction);
    m_portrait->setObjectName(QStringLiteral("psPortrait"));
    m_portrait->setToolTip(tr("The short edge of the paper is at the top (portrait)."));
    m_landscape = new QRadioButton(tr("Wide"), direction);
    m_landscape->setObjectName(QStringLiteral("psLandscape"));
    m_landscape->setToolTip(tr("The long edge of the paper is at the top (landscape)."));
    dl->addWidget(m_portrait);
    dl->addWidget(m_landscape);
    dl->addStretch(1);
    (isLandscape(m_setup) ? m_landscape : m_portrait)->setChecked(true);
    v->addWidget(direction);
    auto *size = new QGroupBox(tr("How big the cells print"), tab);
    auto *g = new QGridLayout(size);
    m_scaleRadio = new QRadioButton(tr("Print at"), size);
    m_scaleRadio->setObjectName(QStringLiteral("psScaleRadio"));
    m_scale = new QSpinBox(size);
    m_scale->setObjectName(QStringLiteral("psScale"));
    m_scale->setRange(kMinPageScale, kMaxPageScale);
    m_scale->setSuffix(tr(" % of full size"));
    m_scale->setAccessibleName(tr("Percent of full size"));
    m_scale->setValue(m_setup.scale);
    m_fitRadio = new QRadioButton(tr("Shrink until it fits"), size);
    m_fitRadio->setObjectName(QStringLiteral("psFitRadio"));
    m_fitAcross = new QSpinBox(size);
    m_fitAcross->setObjectName(QStringLiteral("psFitAcross"));
    m_fitAcross->setRange(0, 999);
    m_fitAcross->setSpecialValueText(tr("any number of"));
    m_fitAcross->setSuffix(tr(" pages across"));
    m_fitAcross->setAccessibleName(tr("Pages across, or 0 for as many as it takes"));
    m_fitAcross->setValue(std::min(m_setup.fitToWidth, 999));
    m_fitDown = new QSpinBox(size);
    m_fitDown->setObjectName(QStringLiteral("psFitDown"));
    m_fitDown->setRange(0, 999);
    m_fitDown->setSpecialValueText(tr("any number of"));
    m_fitDown->setSuffix(tr(" pages down"));
    m_fitDown->setAccessibleName(tr("Pages down, or 0 for as many as it takes"));
    m_fitDown->setValue(std::min(m_setup.fitToHeight, 999));
    g->addWidget(m_scaleRadio, 0, 0);
    g->addWidget(m_scale, 0, 1, 1, 2);
    g->addWidget(m_fitRadio, 1, 0);
    g->addWidget(m_fitAcross, 1, 1);
    g->addWidget(m_fitDown, 1, 2);
    auto *hint = new QLabel(tr("Choose \"any number of\" to let that direction take as many pages as it needs. The page breaks you set are ignored while the cells are being shrunk to fit."), size);
    hint->setWordWrap(true);
    g->addWidget(hint, 2, 0, 1, 3);
    (m_setup.fitToPage && (m_setup.fitToWidth > 0 || m_setup.fitToHeight > 0) ? m_fitRadio : m_scaleRadio)->setChecked(true);
    v->addWidget(size);
    auto *form = new QFormLayout;
    m_paper = new QComboBox(tab);
    m_paper->setObjectName(QStringLiteral("psPaper"));
    m_paper->setAccessibleName(tr("Paper size"));
    QVector<int> ids = commonPaperSizes();
    for (const int id : ids) m_paper->addItem(paperSizeName(id), id);
    m_paper->insertSeparator(m_paper->count());
    for (int id = 1; id < 128; ++id)
        if (isKnownPaperSize(id) && !ids.contains(id)) m_paper->addItem(paperSizeName(id), id);
    if (!isKnownPaperSize(m_setup.paperSize)) {
        m_paper->addItem(m_setup.paperWidth > 0 ? tr("Size set by the file (%1 by %2 points)").arg(qRound(m_setup.paperWidth)).arg(qRound(m_setup.paperHeight)) : tr("Size set by the file"), m_setup.paperSize);
    }
    m_paper->setCurrentIndex(std::max(0, m_paper->findData(m_setup.paperSize)));
    form->addRow(tr("Paper:"), m_paper);
    m_useFirst = new QCheckBox(tr("Start the page numbers at"), tab);
    m_useFirst->setObjectName(QStringLiteral("psUseFirst"));
    m_firstNumber = new QSpinBox(tab);
    m_firstNumber->setObjectName(QStringLiteral("psFirstNumber"));
    m_firstNumber->setRange(0, 32767);
    m_firstNumber->setValue(m_setup.firstPageNumber);
    m_firstNumber->setAccessibleName(tr("Number of the first page"));
    m_useFirst->setChecked(m_setup.useFirstPageNumber);
    auto *row = new QHBoxLayout;
    row->addWidget(m_useFirst);
    row->addWidget(m_firstNumber);
    row->addStretch(1);
    form->addRow(QString(), row);
    v->addLayout(form);
    v->addStretch(1);
    connect(m_scaleRadio, &QRadioButton::toggled, this, [this] { updateFitState(); });
    connect(m_fitRadio, &QRadioButton::toggled, this, [this] { updateFitState(); });
    connect(m_useFirst, &QCheckBox::toggled, this, [this] { updateFitState(); });
    updateFitState();
}

void PageSetupDialog::updateFitState()
{
    m_scale->setEnabled(m_scaleRadio->isChecked());
    m_fitAcross->setEnabled(m_fitRadio->isChecked());
    m_fitDown->setEnabled(m_fitRadio->isChecked());
    m_firstNumber->setEnabled(m_useFirst->isChecked());
}

void PageSetupDialog::buildMargins(QWidget *tab)
{
    const bool metric = m_request.metric;
    auto *v = new QVBoxLayout(tab);
    auto *form = new QFormLayout;
    static const char *const labels[6] = {QT_TR_NOOP("Left:"), QT_TR_NOOP("Right:"), QT_TR_NOOP("Top:"), QT_TR_NOOP("Bottom:"), QT_TR_NOOP("Header, from the top edge:"), QT_TR_NOOP("Footer, from the bottom edge:")};
    static const char *const names[6] = {"psMarginLeft", "psMarginRight", "psMarginTop", "psMarginBottom", "psMarginHeader", "psMarginFooter"};
    const double values[6] = {m_setup.margins.left, m_setup.margins.right, m_setup.margins.top, m_setup.margins.bottom, m_setup.margins.header, m_setup.margins.footer};
    for (int i = 0; i < 6; ++i) {
        m_margin[i] = new QDoubleSpinBox(tab);
        m_margin[i]->setObjectName(QString::fromLatin1(names[i]));
        m_margin[i]->setDecimals(2);
        m_margin[i]->setRange(0, metric ? kMaxMarginInches * 2.54 : kMaxMarginInches);
        m_margin[i]->setSingleStep(metric ? 0.25 : 0.1);
        m_margin[i]->setSuffix(metric ? tr(" cm") : tr(" in"));
        m_margin[i]->setAccessibleName(tr(labels[i]));
        m_margin[i]->setValue(round2(values[i] * (metric ? 2.54 : 1.0)));
        form->addRow(tr(labels[i]), m_margin[i]);
    }
    v->addLayout(form);
    auto *center = new QGroupBox(tr("Put the cells in the middle of the page"), tab);
    auto *cl = new QHBoxLayout(center);
    m_centerAcross = new QCheckBox(tr("Across the page"), center);
    m_centerAcross->setObjectName(QStringLiteral("psCenterAcross"));
    m_centerAcross->setChecked(m_setup.centerHorizontally);
    m_centerDown = new QCheckBox(tr("Down the page"), center);
    m_centerDown->setObjectName(QStringLiteral("psCenterDown"));
    m_centerDown->setChecked(m_setup.centerVertically);
    cl->addWidget(m_centerAcross);
    cl->addWidget(m_centerDown);
    cl->addStretch(1);
    v->addWidget(center);
    v->addStretch(1);
}

void PageSetupDialog::fillPresets(QComboBox *box, const QString &code)
{
    const QSignalBlocker block(box);
    box->clear();
    int found = -1;
    const auto presets = headerFooterPresets(m_request.sheetName, m_request.fileName);
    for (const auto &p : presets) {
        box->addItem(p.first, p.second);
        if (p.second == code) found = box->count() - 1;
    }
    if (found < 0) {
        box->addItem(tr("(your own text)"), code);
        found = box->count() - 1;
    }
    box->setCurrentIndex(found);
}

void PageSetupDialog::buildHeaderFooter(QWidget *tab)
{
    auto *v = new QVBoxLayout(tab);
    auto *form = new QFormLayout;
    m_headerPreset = new QComboBox(tab);
    m_headerPreset->setObjectName(QStringLiteral("psHeaderPreset"));
    m_headerPreset->setAccessibleName(tr("Header for every page"));
    m_footerPreset = new QComboBox(tab);
    m_footerPreset->setObjectName(QStringLiteral("psFooterPreset"));
    m_footerPreset->setAccessibleName(tr("Footer for every page"));
    auto *editHeader = new QPushButton(tr("Edit header…"), tab);
    editHeader->setObjectName(QStringLiteral("psEditHeader"));
    auto *editFooter = new QPushButton(tr("Edit footer…"), tab);
    editFooter->setObjectName(QStringLiteral("psEditFooter"));
    auto *hr = new QHBoxLayout, *fr = new QHBoxLayout;
    hr->addWidget(m_headerPreset, 1);
    hr->addWidget(editHeader);
    fr->addWidget(m_footerPreset, 1);
    fr->addWidget(editFooter);
    form->addRow(tr("Header:"), hr);
    form->addRow(tr("Footer:"), fr);
    v->addLayout(form);
    m_headerShown = new QLabel(tab);
    m_headerShown->setObjectName(QStringLiteral("psHeaderShown"));
    m_headerShown->setWordWrap(true);
    m_footerShown = new QLabel(tab);
    m_footerShown->setObjectName(QStringLiteral("psFooterShown"));
    m_footerShown->setWordWrap(true);
    v->addWidget(m_headerShown);
    v->addWidget(m_footerShown);
    auto check = [&](const QString &name, const QString &text, bool on, const QString &tip) {
        auto *c = new QCheckBox(text, tab);
        c->setObjectName(name);
        c->setChecked(on);
        c->setToolTip(tip);
        v->addWidget(c);
        return c;
    };
    const HeaderFooter &hf = m_setup.headerFooter;
    m_differentFirst = check(QStringLiteral("psDifferentFirst"), tr("The first page has its own header and footer"), hf.differentFirst, tr("Use Edit header and Edit footer to write the first page's own."));
    m_differentOddEven = check(QStringLiteral("psDifferentOddEven"), tr("Odd and even pages have their own"), hf.differentOddEven, tr("Use Edit header and Edit footer to write each."));
    m_scaleWith = check(QStringLiteral("psScaleWith"), tr("Letters grow and shrink with the cells"), hf.scaleWithDoc, tr("When the cells print at half size, so do the header and footer."));
    m_alignMargins = check(QStringLiteral("psAlignMargins"), tr("Line the header and footer up with the margins"), hf.alignWithMargins, tr("Off: they begin and end near the paper's edges."));
    v->addStretch(1);
    fillPresets(m_headerPreset, hf.oddHeader);
    fillPresets(m_footerPreset, hf.oddFooter);
    connect(m_headerPreset, &QComboBox::activated, this, [this](int i) {
        m_setup.headerFooter.oddHeader = m_headerPreset->itemData(i).toString();
        refreshHeaderFooterPreview();
    });
    connect(m_footerPreset, &QComboBox::activated, this, [this](int i) {
        m_setup.headerFooter.oddFooter = m_footerPreset->itemData(i).toString();
        refreshHeaderFooterPreview();
    });
    connect(editHeader, &QPushButton::clicked, this, [this] { editHeaderFooter(true); });
    connect(editFooter, &QPushButton::clicked, this, [this] { editHeaderFooter(false); });
    connect(m_differentFirst, &QCheckBox::toggled, this, [this] { refreshHeaderFooterPreview(); });
    connect(m_differentOddEven, &QCheckBox::toggled, this, [this] { refreshHeaderFooterPreview(); });
    refreshHeaderFooterPreview();
}

void PageSetupDialog::editHeaderFooter(bool header)
{
    HeaderFooter hf = m_setup.headerFooter;
    hf.differentFirst = m_differentFirst->isChecked();
    hf.differentOddEven = m_differentOddEven->isChecked();
    HeaderFooterEditor editor(header, hf, m_request.sheetName, m_request.fileName, this);
    if (m_hfEditorAsker) {
        if (!m_hfEditorAsker(&editor)) return;
    } else if (editor.exec() != QDialog::Accepted) {
        return;
    }
    const HeaderFooter made = editor.texts();
    if (header) {
        m_setup.headerFooter.oddHeader = made.oddHeader;
        m_setup.headerFooter.evenHeader = made.evenHeader;
        m_setup.headerFooter.firstHeader = made.firstHeader;
    } else {
        m_setup.headerFooter.oddFooter = made.oddFooter;
        m_setup.headerFooter.evenFooter = made.evenFooter;
        m_setup.headerFooter.firstFooter = made.firstFooter;
    }
    refreshHeaderFooterPreview();
}

void PageSetupDialog::refreshHeaderFooterPreview()
{
    fillPresets(m_headerPreset, m_setup.headerFooter.oddHeader);
    fillPresets(m_footerPreset, m_setup.headerFooter.oddFooter);
    const HfContext c = sampleContext(m_request.sheetName, m_request.fileName);
    auto line = [&](const QString &what, const QString &code) {
        const QString text = headerFooterPreview(code, c);
        return text.isEmpty() ? QString() : tr("%1: %2").arg(what, text);
    };
    QStringList header, footer;
    header << line(m_differentOddEven->isChecked() ? tr("Odd pages") : tr("Every page"), m_setup.headerFooter.oddHeader);
    footer << line(m_differentOddEven->isChecked() ? tr("Odd pages") : tr("Every page"), m_setup.headerFooter.oddFooter);
    if (m_differentOddEven->isChecked()) {
        header << line(tr("Even pages"), m_setup.headerFooter.evenHeader);
        footer << line(tr("Even pages"), m_setup.headerFooter.evenFooter);
    }
    if (m_differentFirst->isChecked()) {
        header << line(tr("First page"), m_setup.headerFooter.firstHeader);
        footer << line(tr("First page"), m_setup.headerFooter.firstFooter);
    }
    header.removeAll(QString());
    footer.removeAll(QString());
    m_headerShown->setText(header.isEmpty() ? tr("The header prints nothing.") : tr("The header reads:\n%1").arg(header.join(QLatin1Char('\n'))));
    m_footerShown->setText(footer.isEmpty() ? tr("The footer prints nothing.") : tr("The footer reads:\n%1").arg(footer.join(QLatin1Char('\n'))));
}

void PageSetupDialog::buildSheet(QWidget *tab)
{
    auto *v = new QVBoxLayout(tab);
    auto *form = new QFormLayout;
    m_area = new QLineEdit(areaWords(m_setup), tab);
    m_area->setObjectName(QStringLiteral("psArea"));
    m_area->setAccessibleName(tr("Cells to print"));
    m_area->setPlaceholderText(tr("Everything that has something in it"));
    m_area->setToolTip(m_request.selectionText.isEmpty() ? tr("Cell ranges separated by commas, such as A1:D20, F1:G9. Each block prints on pages of its own.")
                                                          : tr("Cell ranges separated by commas, such as A1:D20, F1:G9 (the selection is %1). Each block prints on pages of its own.").arg(m_request.selectionText));
    form->addRow(tr("Cells to print:"), m_area);
    QString rows, cols;
    if (m_setup.titles.hasRows) rows = QStringLiteral("%1:%2").arg(m_setup.titles.row1 + 1).arg(m_setup.titles.row2 + 1);
    if (m_setup.titles.hasCols) cols = QStringLiteral("%1:%2").arg(columnName(m_setup.titles.col1), columnName(m_setup.titles.col2));
    m_titleRows = new QLineEdit(rows, tab);
    m_titleRows->setObjectName(QStringLiteral("psTitleRows"));
    m_titleRows->setAccessibleName(tr("Rows to repeat at the top"));
    m_titleRows->setPlaceholderText(tr("None, or rows such as 1:3"));
    m_titleCols = new QLineEdit(cols, tab);
    m_titleCols->setObjectName(QStringLiteral("psTitleCols"));
    m_titleCols->setAccessibleName(tr("Columns to repeat at the left"));
    m_titleCols->setPlaceholderText(tr("None, or columns such as A:B"));
    form->addRow(tr("Repeat these rows at the top:"), m_titleRows);
    form->addRow(tr("Repeat these columns at the left:"), m_titleCols);
    v->addLayout(form);
    auto *include = new QGroupBox(tr("Also print"), tab);
    auto *il = new QVBoxLayout(include);
    auto check = [&](const QString &name, const QString &text, bool on, const QString &tip = QString()) {
        auto *c = new QCheckBox(text, include);
        c->setObjectName(name);
        c->setChecked(on);
        if (!tip.isEmpty()) c->setToolTip(tip);
        il->addWidget(c);
        return c;
    };
    m_grid = check(QStringLiteral("psGrid"), tr("The lines between cells"), m_setup.gridLines);
    m_headings = check(QStringLiteral("psHeadings"), tr("The row numbers and column letters"), m_setup.headings);
    m_blackWhite = check(QStringLiteral("psBlackWhite"), tr("In black and white only"), m_setup.blackAndWhite, tr("Colors and fills are left out; text and lines are black."));
    m_draft = check(QStringLiteral("psDraft"), tr("As a quick draft"), m_setup.draft, tr("No lines between cells and no pictures, charts, or shapes."));
    v->addWidget(include);
    auto *form2 = new QFormLayout;
    m_notes = new QComboBox(tab);
    m_notes->setObjectName(QStringLiteral("psNotes"));
    m_notes->setAccessibleName(tr("Notes"));
    m_notes->addItem(tr("Leave them out"), int(PrintComments::None));
    m_notes->addItem(tr("List them after the last page"), int(PrintComments::AtEnd));
    m_notes->addItem(tr("Show them where they are"), int(PrintComments::AsDisplayed));
    m_notes->setCurrentIndex(std::max(0, m_notes->findData(int(m_setup.comments))));
    m_notes->setToolTip(tr("This is kept in the file for other programs. JeffCrunch does not read or print notes yet."));
    form2->addRow(tr("Notes:"), m_notes);
    m_errors = new QComboBox(tab);
    m_errors->setObjectName(QStringLiteral("psErrors"));
    m_errors->setAccessibleName(tr("Cells that show an error"));
    m_errors->addItem(tr("Print what the cell shows"), int(PrintErrors::Displayed));
    m_errors->addItem(tr("Print nothing"), int(PrintErrors::Blank));
    m_errors->addItem(tr("Print two dashes"), int(PrintErrors::Dash));
    m_errors->addItem(tr("Print #N/A"), int(PrintErrors::NA));
    m_errors->setCurrentIndex(std::max(0, m_errors->findData(int(m_setup.errors))));
    form2->addRow(tr("Cells that show an error:"), m_errors);
    v->addLayout(form2);
    auto *order = new QGroupBox(tr("Which page comes next"), tab);
    auto *ol = new QHBoxLayout(order);
    m_down = new QRadioButton(tr("The one below, then the one to the right"), order);
    m_down->setObjectName(QStringLiteral("psDown"));
    m_across = new QRadioButton(tr("The one to the right, then the one below"), order);
    m_across->setObjectName(QStringLiteral("psAcross"));
    (m_setup.pageOrder == PageOrder::OverThenDown ? m_across : m_down)->setChecked(true);
    ol->addWidget(m_down);
    ol->addWidget(m_across);
    v->addWidget(order);
    v->addStretch(1);
}

PageSetupDialog::View PageSetupDialog::view() const
{
    View w;
    w.landscape = m_landscape->isChecked();
    w.fit = m_fitRadio->isChecked();
    w.scale = m_scale->value();
    w.across = m_fitAcross->value();
    w.down = m_fitDown->value();
    w.paper = m_paper->currentData().toInt();
    w.useFirst = m_useFirst->isChecked();
    w.first = m_firstNumber->value();
    for (int i = 0; i < 6; ++i) w.margin[i] = m_margin[i]->value();
    w.centerAcross = m_centerAcross->isChecked();
    w.centerDown = m_centerDown->isChecked();
    w.differentFirst = m_differentFirst->isChecked();
    w.differentOddEven = m_differentOddEven->isChecked();
    w.scaleWith = m_scaleWith->isChecked();
    w.align = m_alignMargins->isChecked();
    w.area = m_area->text();
    w.rows = m_titleRows->text();
    w.cols = m_titleCols->text();
    w.grid = m_grid->isChecked();
    w.headings = m_headings->isChecked();
    w.blackWhite = m_blackWhite->isChecked();
    w.draft = m_draft->isChecked();
    w.notes = m_notes->currentData().toInt();
    w.errors = m_errors->currentData().toInt();
    w.across_first = m_across->isChecked();
    return w;
}

bool PageSetupDialog::readAreas(QVector<CellRange> *out, QString *problem) const
{
    out->clear();
    const QString text = m_area->text().trimmed();
    if (text.isEmpty()) return true;
    for (const QString &piece : text.split(QRegularExpression(QStringLiteral("[,;]")), Qt::SkipEmptyParts)) {
        PageSetup tmp;
        readPrintAreaText(piece.trimmed(), QString(), &tmp);
        if (tmp.printArea.size() != 1) {
            if (problem) *problem = tr("\"%1\" is not a block of cells. Write blocks such as A1:D20, separated by commas.").arg(piece.trimmed());
            return false;
        }
        out->append(tmp.printArea[0]);
        if (out->size() >= kMaxPrintAreas) break;
    }
    return true;
}

bool PageSetupDialog::readTitles(PrintTitles *out, QString *problem) const
{
    *out = PrintTitles();
    auto one = [&](const QString &text, bool rows) {
        const QString t = text.trimmed();
        if (t.isEmpty()) return true;
        QString shaped = t;
        shaped.remove(QLatin1Char('$'));
        if (!shaped.contains(QLatin1Char(':'))) shaped += QLatin1Char(':') + shaped;   // (one row or column)
        PageSetup tmp;
        readPrintTitlesText(shaped, QString(), &tmp);
        if (rows && tmp.titles.hasRows) {
            out->hasRows = true;
            out->row1 = tmp.titles.row1;
            out->row2 = tmp.titles.row2;
            return true;
        }
        if (!rows && tmp.titles.hasCols) {
            out->hasCols = true;
            out->col1 = tmp.titles.col1;
            out->col2 = tmp.titles.col2;
            return true;
        }
        if (problem) *problem = rows ? tr("\"%1\" is not a run of rows. Write it as 1:3, or a single row as 2.").arg(t) : tr("\"%1\" is not a run of columns. Write it as A:B, or a single column as C.").arg(t);
        return false;
    };
    return one(m_titleRows->text(), true) && one(m_titleCols->text(), false);
}

PageSetup PageSetupDialog::setup() const
{
    PageSetup s = m_setup;   // (the header and footer texts as the presets and editor left them)
    const View now = view();
    const View &was = m_initial;
    if (now.landscape != was.landscape) s.orientation = now.landscape ? PageOrientation::Landscape : PageOrientation::Portrait;
    if (now.fit != was.fit || now.scale != was.scale || now.across != was.across || now.down != was.down) {
        if (now.fit) {
            s.fitToPage = true;
            s.fitToWidth = now.across;
            s.fitToHeight = now.down;
        } else {
            s.fitToPage = false;
            s.scale = now.scale;
        }
    }
    if (now.paper != was.paper) {
        s.paperSize = now.paper;
        s.paperWidth = s.paperHeight = 0;
    }
    if (now.useFirst != was.useFirst || now.first != was.first) {
        s.useFirstPageNumber = now.useFirst;
        s.firstPageNumber = now.first;
    }
    const double unit = m_request.metric ? 2.54 : 1.0;
    double *fields[6] = {&s.margins.left, &s.margins.right, &s.margins.top, &s.margins.bottom, &s.margins.header, &s.margins.footer};
    for (int i = 0; i < 6; ++i)
        if (now.margin[i] != was.margin[i]) *fields[i] = std::clamp(now.margin[i] / unit, 0.0, kMaxMarginInches);
    if (now.centerAcross != was.centerAcross) s.centerHorizontally = now.centerAcross;
    if (now.centerDown != was.centerDown) s.centerVertically = now.centerDown;
    if (now.differentFirst != was.differentFirst) s.headerFooter.differentFirst = now.differentFirst;
    if (now.differentOddEven != was.differentOddEven) s.headerFooter.differentOddEven = now.differentOddEven;
    if (now.scaleWith != was.scaleWith) s.headerFooter.scaleWithDoc = now.scaleWith;
    if (now.align != was.align) s.headerFooter.alignWithMargins = now.align;
    if (now.area != was.area) {
        QVector<CellRange> areas;
        if (readAreas(&areas, nullptr)) s.printArea = areas;
    }
    if (now.rows != was.rows || now.cols != was.cols) {
        PrintTitles t;
        if (readTitles(&t, nullptr)) s.titles = t;
    }
    if (now.grid != was.grid) s.gridLines = now.grid;
    if (now.headings != was.headings) s.headings = now.headings;
    if (now.blackWhite != was.blackWhite) s.blackAndWhite = now.blackWhite;
    if (now.draft != was.draft) s.draft = now.draft;
    if (now.notes != was.notes) s.comments = PrintComments(now.notes);
    if (now.errors != was.errors) s.errors = PrintErrors(now.errors);
    if (now.across_first != was.across_first) s.pageOrder = now.across_first ? PageOrder::OverThenDown : PageOrder::DownThenOver;
    return s;
}

void PageSetupDialog::accept()
{
    QString problem;
    QVector<CellRange> areas;
    PrintTitles titles;
    if (!readAreas(&areas, &problem) || !readTitles(&titles, &problem)) {
        m_problem->setText(problem);
        m_problem->show();
        m_tabs->setCurrentIndex(3);
        return;
    }
    m_problem->hide();
    QDialog::accept();
}

} // namespace jp

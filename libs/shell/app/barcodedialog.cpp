// Insert > Barcode, the dialog every app shares: book ISBNs with or without a
// price add-on, EAN-13, UPC-A, EAN-8, Code 128, Code 39, and QR codes. It asks
// for the settings of a barcode (core/barcodeitem.h makes the object from
// them) and shows the barcode as it will be drawn.

#include "app/barcodedialog.h"
#include "core/version.h"
#include "app/settings.h"
#include "core/barcode.h"
#include "core/barcodeitem.h"

#include <functional>

#include <QButtonGroup>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QSaveFile>
#include <QStandardPaths>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include "app/plaintext.h"

namespace jp {

namespace {
// The International ISBN Agency's range table, which places the hyphens in
// ISBNs from every country. Its terms don't allow bundling it, so JeffPub
// downloads it from the agency and keeps a copy, renewed monthly.
const char kIsbnRangesUrl[] = "https://www.isbn-international.org/export_rangemessage.xml";

QString isbnRangesPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/isbn-ranges.xml");
}

// Loads the saved copy (once a session) and, when it's missing or a month
// old, asks the agency for a new one; `arrived` runs if one comes while
// `receiver` is still open.
void useIsbnRanges(QObject *receiver, const std::function<void()> &arrived)
{
    static bool loaded = false, asked = false;
    const QFileInfo saved(isbnRangesPath());
    if (!loaded && saved.exists()) {
        QFile f(saved.filePath());
        loaded = f.open(QIODevice::ReadOnly) && barcode::loadIsbnRanges(f.readAll());
    }
    if (asked || (loaded && saved.lastModified().daysTo(QDateTime::currentDateTime()) < 30)) return;
    if (QGuiApplication::platformName() == QLatin1String("offscreen")) return;
    asked = true;
    static auto *net = new QNetworkAccessManager(qApp);
    QNetworkRequest req{QUrl(QString::fromLatin1(kIsbnRangesUrl))};
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("JeffPub/%1").arg(jp::versionText()));
    QNetworkReply *r = net->get(req);
    QPointer<QObject> open(receiver);
    QObject::connect(r, &QNetworkReply::finished, net, [r, open, arrived] {
        r->deleteLater();
        if (r->error() != QNetworkReply::NoError) return;
        const QByteArray xml = r->read(8 << 20);   // the table is about 230 KB
        if (!barcode::loadIsbnRanges(xml)) return;
        loaded = true;
        QDir().mkpath(QFileInfo(isbnRangesPath()).path());
        QSaveFile f(isbnRangesPath());
        if (f.open(QIODevice::WriteOnly)) {
            f.write(xml);
            f.commit();
        }
        if (open) arrived();
    });
}
} // namespace

bool askBarcode(QWidget *p, const QJsonObject &was, QJsonObject *out)
{
    using namespace barcode;
    Settings &st = Settings::get();
    // With the settings of a barcode, the dialog edits that barcode.
    const bool editing = !was.isEmpty();
    auto setting = [&](const char *key, const QVariant &def) {
        return was.isEmpty() ? st.value(QStringLiteral("barcode/") + QLatin1String(key), def) : was.value(QLatin1String(key)).toVariant();
    };
    QDialog dlg(p);
    dlg.setObjectName(QStringLiteral("barcodeDialog"));
    dlg.setWindowTitle(editing ? QCoreApplication::translate("BarcodeDialog", "Edit Barcode") : QCoreApplication::translate("BarcodeDialog", "Insert Barcode"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout();
    auto *type = new QComboBox(&dlg);
    type->addItems({QCoreApplication::translate("BarcodeDialog", "Book (ISBN)"), QStringLiteral("EAN-13"), QStringLiteral("UPC-A"), QStringLiteral("EAN-8"),
                    QCoreApplication::translate("BarcodeDialog", "Code 128 (letters and digits)"), QCoreApplication::translate("BarcodeDialog", "Code 39 (capitals and digits)"), QCoreApplication::translate("BarcodeDialog", "QR code (any text, such as a web address)")});
    type->setCurrentIndex(std::clamp(setting("type", 0).toInt(), 0, int(Type::Qr)));
    form->addRow(QCoreApplication::translate("BarcodeDialog", "Type:"), type);
    auto *data = new QLineEdit(&dlg);
    data->setText(setting("data", QString()).toString());
    data->setMinimumWidth(260);
    form->addRow(QCoreApplication::translate("BarcodeDialog", "ISBN:"), data);
    auto *addOn = new QLineEdit(&dlg);
    addOn->setPlaceholderText(QCoreApplication::translate("BarcodeDialog", "none, or 2 or 5 digits"));
    form->addRow(QCoreApplication::translate("BarcodeDialog", "Add-on:"), addOn);
    auto *qrLevel = new QComboBox(&dlg);
    qrLevel->addItems({QCoreApplication::translate("BarcodeDialog", "Low (repairs 7% damage)"), QCoreApplication::translate("BarcodeDialog", "Medium (15%)"), QCoreApplication::translate("BarcodeDialog", "Quartile (25%)"),
                       QCoreApplication::translate("BarcodeDialog", "High (30%)")});
    qrLevel->setCurrentIndex(std::clamp(setting("qrLevel", 1).toInt(), 0, 3));
    form->addRow(QCoreApplication::translate("BarcodeDialog", "Error correction:"), qrLevel);
    v->addLayout(form);

    // A book's price add-on.
    auto *price = new QGroupBox(QCoreApplication::translate("BarcodeDialog", "Price add-on"), &dlg);
    auto *pv = new QVBoxLayout(price);
    auto *noAddOn = new QRadioButton(QCoreApplication::translate("BarcodeDialog", "No add-on"), price);
    auto *withPrice = new QRadioButton(QCoreApplication::translate("BarcodeDialog", "Price:"), price);
    auto *noPrice = new QRadioButton(QCoreApplication::translate("BarcodeDialog", "No suggested price (90000)"), price);
    auto *group = new QButtonGroup(price);
    group->addButton(noAddOn, 0);
    group->addButton(withPrice, 1);
    group->addButton(noPrice, 2);
    auto *currency = new QComboBox(price);
    currency->addItems({QCoreApplication::translate("BarcodeDialog", "US dollars"), QCoreApplication::translate("BarcodeDialog", "Canadian dollars"), QCoreApplication::translate("BarcodeDialog", "British pounds"), QCoreApplication::translate("BarcodeDialog", "Australian dollars"),
                        QCoreApplication::translate("BarcodeDialog", "New Zealand dollars")});
    currency->setCurrentIndex(std::clamp(setting("currency", 0).toInt(), 0, 4));
    auto *amount = new QDoubleSpinBox(price);
    amount->setRange(0, 99.99);
    amount->setDecimals(2);
    amount->setValue(setting("price", 19.95).toDouble());
    auto *pr = new QHBoxLayout();
    pr->addWidget(withPrice);
    pr->addWidget(amount);
    pr->addWidget(currency);
    pr->addStretch(1);
    pv->addWidget(noAddOn);
    pv->addLayout(pr);
    pv->addWidget(noPrice);
    group->button(std::clamp(setting("priceMode", 1).toInt(), 0, 2))->setChecked(true);
    v->addWidget(price);

    auto *size = new QGroupBox(QCoreApplication::translate("BarcodeDialog", "Size"), &dlg);
    auto *sf = new QFormLayout(size);
    auto *mag = new QSpinBox(size);
    mag->setRange(50, 300);
    mag->setSuffix(QStringLiteral("%"));
    mag->setValue(setting("magnification", 100).toInt());
    mag->setToolTip(QCoreApplication::translate("BarcodeDialog", "100% is the standard size; books usually print at 80% to 100%."));
    mag->setFixedWidth(110);
    auto *height = new QSpinBox(size);
    height->setRange(40, 100);
    height->setSuffix(QStringLiteral("%"));
    height->setValue(setting("height", 100).toInt());
    height->setToolTip(QCoreApplication::translate("BarcodeDialog", "Shorter bars save room; scanners need most of the standard height."));
    height->setFixedWidth(110);
    sf->addRow(QCoreApplication::translate("BarcodeDialog", "Magnification:"), mag);
    sf->addRow(QCoreApplication::translate("BarcodeDialog", "Bar height:"), height);
    v->addWidget(size);
    auto *showText = new QCheckBox(QCoreApplication::translate("BarcodeDialog", "Print the digits"), &dlg);
    showText->setChecked(setting("text", true).toBool());
    auto *white = new QCheckBox(QCoreApplication::translate("BarcodeDialog", "White background (the clear space scanners need)"), &dlg);
    white->setChecked(setting("white", true).toBool());
    auto *check39 = new QCheckBox(QCoreApplication::translate("BarcodeDialog", "Add a check character"), &dlg);
    check39->setChecked(setting("check39", false).toBool());
    addOn->setText(setting("addOn", QString()).toString());
    v->addWidget(showText);
    v->addWidget(white);
    v->addWidget(check39);

    auto *preview = plainLabel(new QLabel(&dlg));
    preview->setMinimumSize(380, 190);
    preview->setAlignment(Qt::AlignCenter);
    preview->setStyleSheet(QStringLiteral("QLabel{background:#ffffff; border:1px solid #d0d4da;}"));
    preview->setObjectName(QStringLiteral("barcodePreview"));
    v->addWidget(preview);
    auto *problem = plainLabel(new QLabel(&dlg));
    problem->setStyleSheet(QStringLiteral("color:#c0392b;"));
    problem->setWordWrap(true);
    v->addWidget(problem);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Cancel, &dlg);
    QPushButton *insert = bb->addButton(editing ? QCoreApplication::translate("BarcodeDialog", "Update") : QCoreApplication::translate("BarcodeDialog", "Insert"), QDialogButtonBox::AcceptRole);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    v->addWidget(bb);

    Layout current;
    // The settings the widgets show, as they are kept with the barcode and remembered for the next one.
    auto settingsNow = [&] {
        return QJsonObject{{"type", type->currentIndex()}, {"data", data->text()}, {"currency", currency->currentIndex()},
                           {"price", amount->value()}, {"priceMode", group->checkedId()}, {"addOn", addOn->text()},
                           {"magnification", mag->value()}, {"height", height->value()}, {"text", showText->isChecked()},
                           {"white", white->isChecked()}, {"check39", check39->isChecked()}, {"qrLevel", qrLevel->currentIndex()}};
    };
    auto update = [&] {
        const Type t = Type(type->currentIndex());
        const bool book = t == Type::Isbn, ean = t == Type::Ean13 || t == Type::UpcA || t == Type::Ean8, isQr = t == Type::Qr;
        if (auto *lab = qobject_cast<QLabel *>(form->labelForField(data)))
            lab->setText(book ? QCoreApplication::translate("BarcodeDialog", "ISBN:") : ean ? QCoreApplication::translate("BarcodeDialog", "Digits:") : QCoreApplication::translate("BarcodeDialog", "Text:"));
        data->setPlaceholderText(book ? QCoreApplication::translate("BarcodeDialog", "978-1-23456-789-7 or a 10-digit ISBN") : ean ? QCoreApplication::translate("BarcodeDialog", "with or without the check digit") : isQr ? QCoreApplication::translate("BarcodeDialog", "https://example.com") : QString());
        price->setVisible(book);
        addOn->setVisible(ean);
        form->labelForField(addOn)->setVisible(ean);
        check39->setVisible(t == Type::Code39);
        qrLevel->setVisible(isQr);
        form->labelForField(qrLevel)->setVisible(isQr);
        showText->setVisible(!isQr);
        height->setVisible(!isQr);
        sf->labelForField(height)->setVisible(!isQr);
        amount->setEnabled(group->checkedId() == 1);
        currency->setEnabled(group->checkedId() == 1);
        current = data->text().trimmed().isEmpty() ? Layout() : make(barcodeOptions(settingsNow()));
        if (book && group->checkedId() == 1 && current.error.isEmpty()) {
            QString err;
            priceAddOn(Currency(currency->currentIndex() + 1), amount->value(), &err);
            if (!err.isEmpty()) {
                current = Layout();
                current.error = err;
            }
        }
        problem->setText(data->text().trimmed().isEmpty() ? QString() : current.error);
        insert->setEnabled(current.error.isEmpty() && !current.bars.isEmpty());
        // The preview, scaled to fit.
        QPixmap pm(preview->size() * preview->devicePixelRatioF());
        pm.setDevicePixelRatio(preview->devicePixelRatioF());
        pm.fill(Qt::white);
        if (current.error.isEmpty() && !current.bars.isEmpty()) {
            QPainter pp(&pm);
            pp.setRenderHint(QPainter::Antialiasing);
            const QSizeF room = QSizeF(preview->size()) - QSizeF(20, 20);
            const double k = std::min(room.width() / current.size.width(), room.height() / current.size.height());
            pp.translate((preview->width() - current.size.width() * k) / 2, (preview->height() - current.size.height() * k) / 2);
            pp.scale(k, k);
            pp.fillPath(barcodePath(current), Qt::black);
        }
        preview->setPixmap(pm);
    };
    for (QLineEdit *e : {data, addOn}) QObject::connect(e, &QLineEdit::textChanged, &dlg, update);
    // A whole ISBN-13 typed or pasted without hyphens gets the standard ones.
    QObject::connect(data, &QLineEdit::textChanged, &dlg, [&] {
        if (type->currentIndex() != int(Type::Isbn)) return;
        const QString t = data->text().trimmed();
        if (t.size() != 13 || t.contains(QLatin1Char('-'))) return;
        QString err;
        const QString h = hyphenateIsbn(isbn13(t, &err));
        if (!h.isEmpty()) data->setText(h);
    });
    // ISBNs from every country, once the agency's table is here.
    useIsbnRanges(&dlg, [&] {
        const QString t = data->text().trimmed();
        QString err;
        const QString h = type->currentIndex() == int(Type::Isbn) && t.size() == 13 ? hyphenateIsbn(isbn13(t, &err)) : QString();
        if (!h.isEmpty()) data->setText(h);
        else update();
    });
    for (QComboBox *c : {type, currency, qrLevel}) QObject::connect(c, &QComboBox::currentIndexChanged, &dlg, update);
    for (QSpinBox *s : {mag, height}) QObject::connect(s, &QSpinBox::valueChanged, &dlg, update);
    QObject::connect(amount, &QDoubleSpinBox::valueChanged, &dlg, update);
    QObject::connect(group, &QButtonGroup::idClicked, &dlg, update);
    for (QCheckBox *c : {showText, check39}) QObject::connect(c, &QCheckBox::toggled, &dlg, update);
    update();
    if (dlg.exec() != QDialog::Accepted || !current.error.isEmpty() || current.bars.isEmpty()) return false;

    // The settings, remembered for the next barcode and kept with this one.
    const QJsonObject settings = settingsNow();
    for (auto it = settings.begin(); it != settings.end(); ++it) st.setValue(QStringLiteral("barcode/") + it.key(), it.value().toVariant());
    *out = settings;
    return true;
}

} // namespace jp

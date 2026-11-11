#include "app/equationdialog.h"

#include "wml/wmlmathedit.h"
#include "wml/wmlmathpath.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

QImage equationPicture(const QString &linear, double emSize, const QColor &color, double pixelRatio)
{
    bool ok = false;
    const QByteArray markup = office::wml::equationMarkup(linear, &ok);
    if (!ok) return QImage();
    const office::wml::EquationShape shape = office::wml::equationShape(markup, emSize);
    if (!shape.valid()) return QImage();
    const QSize px((shape.box.size() * pixelRatio).toSize().expandedTo(QSize(1, 1)).boundedTo(QSize(4000, 2000)));
    QImage img(px, QImage::Format_ARGB32_Premultiplied);
    img.setDevicePixelRatio(pixelRatio);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(px.width() / pixelRatio / shape.box.width(), px.height() / pixelRatio / shape.box.height());
    p.fillPath(shape.path, color);
    p.end();
    return img;
}

bool askEquation(QWidget *parent, const QString &title, QString *linear)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("equationDialog"));
    dlg.setWindowTitle(title);
    auto *layout = new QVBoxLayout(&dlg);
    auto *help = new QLabel(QCoreApplication::translate("EquationDialog", "Type the equation in a line. Use a/b for a fraction, x^2 for a power, x_i for a subscript, sqrt(x) for a root, and a backslash and a name for a symbol, such as \\alpha, \\pi, \\infty, \\leq, or \\times. "
                               "Write \\sum_(i=1)^n, \\int_0^1, or \\prod for a sum, an integral, or a product with limits. Use \\hat{x}, \\vec{v}, or \\bar{x} for an accent, \\overline{x} for a bar, "
                               "\\left( ... \\right) for brackets that grow, and \\pmatrix{a & b \\\\ c & d} for a matrix."),
                            &dlg);
    help->setWordWrap(true);
    help->setTextFormat(Qt::PlainText);
    layout->addWidget(help);
    auto *designs = new QComboBox(&dlg);
    designs->setObjectName(QStringLiteral("equationGallery"));
    designs->setAccessibleName(QCoreApplication::translate("EquationDialog", "Built-In Equations"));
    designs->addItem(QCoreApplication::translate("EquationDialog", "Built-In Equations"), QString());
    for (const office::wml::EquationDesign &d : office::wml::equationGallery()) designs->addItem(d.name, d.linear);
    layout->addWidget(designs);
    auto *input = new QLineEdit(linear ? *linear : QString(), &dlg);
    input->setObjectName(QStringLiteral("equationInput"));
    input->setMinimumWidth(420);
    layout->addWidget(input);
    auto *drawn = new QLabel(&dlg);
    drawn->setObjectName(QStringLiteral("equationDrawn"));
    drawn->setAccessibleName(QCoreApplication::translate("EquationDialog", "Equation as drawn"));
    drawn->setMinimumHeight(64);
    drawn->setAlignment(Qt::AlignCenter);
    layout->addWidget(drawn);
    auto *preview = new QLabel(&dlg);
    preview->setObjectName(QStringLiteral("equationPreview"));
    preview->setTextFormat(Qt::PlainText);
    preview->setAccessibleName(QCoreApplication::translate("EquationDialog", "Equation preview"));
    layout->addWidget(preview);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    auto update = [input, preview, drawn, buttons, &dlg] {
        bool ok = false;
        const QByteArray markup = office::wml::equationMarkup(input->text(), &ok);
        preview->setText(ok ? office::wml::equationLine(markup) : QString());
        buttons->button(QDialogButtonBox::Ok)->setEnabled(ok);
        const qreal ratio = dlg.devicePixelRatioF();
        const QImage img = ok ? equationPicture(input->text(), 28, dlg.palette().color(QPalette::WindowText), ratio) : QImage();
        drawn->setPixmap(img.isNull() ? QPixmap() : QPixmap::fromImage(img));
    };
    QObject::connect(input, &QLineEdit::textChanged, &dlg, update);
    QObject::connect(designs, &QComboBox::activated, &dlg, [designs, input](int i) {
        const QString line = designs->itemData(i).toString();
        if (!line.isEmpty()) input->setText(line);
        input->setFocus();
    });
    update();
    if (dlg.exec() != QDialog::Accepted) return false;
    if (linear) *linear = input->text();
    return true;
}

} // namespace jp

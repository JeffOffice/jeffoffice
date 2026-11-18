#include "app/uistyle.h"
#include "app/equationdialog.h"

#include "wml/wmlmathedit.h"
#include "wml/wmlmathpath.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace jp {

namespace {

QImage pictureOfMarkup(const QByteArray &markup, double emSize, const QColor &color, double pixelRatio)
{
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

enum Syntax { LineSyntax = 0, UnicodeMathSyntax = 1, LatexSyntax = 2 };

QByteArray markupOf(int syntax, const QString &text)
{
    switch (syntax) {
    case UnicodeMathSyntax: return office::wml::equationFromUnicodeMath(text);
    case LatexSyntax: return office::wml::equationFromLatex(text);
    default: return office::wml::equationMarkup(text);
    }
}

QString textOf(int syntax, const QByteArray &markup)
{
    switch (syntax) {
    case UnicodeMathSyntax: return office::wml::equationToUnicodeMath(markup);
    case LatexSyntax: return office::wml::equationToLatex(markup);
    default: return office::wml::equationLinear(markup);
    }
}

} // namespace

QImage equationPicture(const QString &linear, double emSize, const QColor &color, double pixelRatio)
{
    bool ok = false;
    const QByteArray markup = office::wml::equationMarkup(linear, &ok);
    return ok ? pictureOfMarkup(markup, emSize, color, pixelRatio) : QImage();
}

bool askEquation(QWidget *parent, const QString &title, QString *linear) { return askEquationMarkup(parent, title, linear, nullptr); }

bool askEquationMarkup(QWidget *parent, const QString &title, QString *linear, QByteArray *markupOut)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("equationDialog"));
    dlg.setWindowTitle(title);
    auto *layout = new QVBoxLayout(&dlg);
    // (The hint for the other syntaxes is made before the help of the line syntax, whose text the tests of the translations read as the dialog's last long label.)
    auto *hint = new QLabel(&dlg);
    hint->setWordWrap(true);
    hint->setTextFormat(Qt::PlainText);
    hint->setObjectName(QStringLiteral("equationSyntaxHint"));
    auto *help = new QLabel(QCoreApplication::translate("EquationDialog", "Type the equation in a line. Use a/b for a fraction, x^2 for a power, x_i for a subscript, sqrt(x) for a root, and a backslash and a name for a symbol, such as \\alpha, \\pi, \\infty, \\leq, or \\times. "
                               "Write \\sum_(i=1)^n, \\int_0^1, or \\prod for a sum, an integral, or a product with limits. Use \\hat{x}, \\vec{v}, or \\bar{x} for an accent, \\overline{x} for a bar, "
                               "\\left( ... \\right) for brackets that grow, and \\pmatrix{a & b \\\\ c & d} for a matrix."),
                            &dlg);
    help->setWordWrap(true);
    help->setTextFormat(Qt::PlainText);
    layout->addWidget(help);
    layout->addWidget(hint);
    // How it is shown and how it is typed.
    auto *choices = new QHBoxLayout;
    auto *professional = new QRadioButton(QCoreApplication::translate("EquationDialog", "Professional"), &dlg);
    professional->setObjectName(QStringLiteral("equationProfessional"));
    auto *linearForm = new QRadioButton(QCoreApplication::translate("EquationDialog", "Linear"), &dlg);
    linearForm->setObjectName(QStringLiteral("equationLinear"));
    professional->setChecked(true);
    auto *form = new QButtonGroup(&dlg);
    form->addButton(professional);
    form->addButton(linearForm);
    auto *syntax = new QComboBox(&dlg);
    syntax->setObjectName(QStringLiteral("equationSyntax"));
    syntax->setAccessibleName(QCoreApplication::translate("EquationDialog", "Typing format"));
    syntax->addItem(QCoreApplication::translate("EquationDialog", "Simple line"), int(LineSyntax));
    syntax->addItem(QStringLiteral("UnicodeMath"), int(UnicodeMathSyntax));
    syntax->addItem(QStringLiteral("LaTeX"), int(LatexSyntax));
    choices->addWidget(professional);
    choices->addWidget(linearForm);
    choices->addStretch(1);
    choices->addWidget(syntax);
    layout->addLayout(choices);
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
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);

    // The equation as typed now: its markup, and its line of our own syntax (empty if it has none).
    QByteArray current;
    QString currentLine;
    int shown = LineSyntax;
    const bool needsLine = markupOut == nullptr;
    auto setHints = [&] {
        help->setVisible(shown == LineSyntax);
        hint->setVisible(shown != LineSyntax);
        if (shown == UnicodeMathSyntax)
            hint->setText(QCoreApplication::translate("EquationDialog", "Type the equation as UnicodeMath. Use a/b for a fraction, x^2_i for scripts, √(x) for a root, ∑_(i=1)^n for a sum, \"words\" for text that is not math, "
                                                                        "(a|b) for brackets, and \\matrix(a&b@c&d) for a matrix. A name with a backslash, such as \\alpha, is a symbol."));
        else if (shown == LatexSyntax)
            hint->setText(QCoreApplication::translate("EquationDialog", "Type the equation as LaTeX. Use \\frac{a}{b} for a fraction, x^2_i for scripts, \\sqrt{x} for a root, \\sum_{i=1}^n for a sum, \\text{words} for text that is not math, "
                                                                        "\\left( ... \\right) for brackets, and \\begin{pmatrix} a & b \\\\ c & d \\end{pmatrix} for a matrix."));
    };
    auto update = [&] {
        current = markupOf(shown, input->text());
        const bool ok = !current.isEmpty();
        currentLine = ok ? (shown == LineSyntax ? input->text() : office::wml::equationLinear(current)) : QString();
        const bool keepable = ok && (!needsLine || !currentLine.isEmpty());
        preview->setText(ok ? office::wml::equationLine(current) : QString());
        if (ok && !keepable) preview->setText(QCoreApplication::translate("EquationDialog", "This equation can't be put in here yet."));
        buttons->button(QDialogButtonBox::Ok)->setEnabled(keepable);
        const qreal ratio = dlg.devicePixelRatioF();
        const QImage img = ok ? pictureOfMarkup(current, 28, dlg.palette().color(QPalette::WindowText), ratio) : QImage();
        drawn->setPixmap(img.isNull() ? QPixmap() : QPixmap::fromImage(img));
    };
    QObject::connect(input, &QLineEdit::textChanged, &dlg, update);
    QObject::connect(linearForm, &QRadioButton::toggled, &dlg, [drawn](bool on) { drawn->setVisible(!on); });
    // Another typing format: what is typed is written again in it (kept as it is, and the choice undone, when it can't be).
    QObject::connect(syntax, &QComboBox::activated, &dlg, [&](int i) {
        const int to = syntax->itemData(i).toInt();
        if (to == shown) return;
        const QByteArray markup = markupOf(shown, input->text());
        const QString text = markup.isEmpty() ? QString() : textOf(to, markup);
        if (!input->text().trimmed().isEmpty() && text.isEmpty()) {
            syntax->setCurrentIndex(syntax->findData(shown));
            preview->setText(QCoreApplication::translate("EquationDialog", "This equation can't be written in that format."));
            return;
        }
        shown = to;
        setHints();
        QSignalBlocker block(input);
        input->setText(text);
        block.unblock();
        update();
    });
    QObject::connect(designs, &QComboBox::activated, &dlg, [&](int i) {
        const QString line = designs->itemData(i).toString();
        if (!line.isEmpty()) {
            const QString text = shown == LineSyntax ? line : textOf(shown, office::wml::equationMarkup(line));
            input->setText(text.isEmpty() ? line : text);
        }
        input->setFocus();
    });
    setHints();
    update();
    if (dlg.exec() != QDialog::Accepted) return false;
    if (linear) *linear = currentLine;
    if (markupOut) *markupOut = current;
    return true;
}

} // namespace jp

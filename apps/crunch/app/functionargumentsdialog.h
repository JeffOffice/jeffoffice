#pragma once
// The Function Arguments dialog: one row for each argument of a function (its name, a box to type it in with a button to point at cells on the sheet, and what it comes to as it stands), what the
// function does, what the argument being typed is, and the result of the whole call, worked out as the person types (a moment after they stop). A function that takes as many arguments as
// there are (SUM) gets a new row when the last one is filled. It only builds the call; the window puts it in the formula.

#include "core/editsession.h"
#include "core/functioninfo.h"

#include <QDialog>
#include <QTimer>

#include <functional>
#include <map>
#include <vector>

class QDialogButtonBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QScrollArea;
class QToolButton;

namespace jp {

class FunctionArgumentsDialog : public QDialog {
    Q_OBJECT
public:
    // `arguments` are the ones already written (as typed: in `separator`'s locale), for a call that is being changed.
    FunctionArgumentsDialog(QWidget *parent, const crunch::FunctionDoc &doc, const QStringList &arguments, QChar separator);
    // Works out a formula as typed (without "="): the window gives the session's preview. Without it no values are shown.
    using Evaluator = std::function<crunch::EditSession::Preview(const QString &typed)>;
    void setEvaluator(const Evaluator &evaluator);
    // The Help topic the link opens (the link is hidden when empty).
    void setHelpTopic(const QString &topic);
    QString helpTopic() const { return m_helpTopic; }

    QString functionName() const { return m_name; }
    // "SUM(A1:A3, 5)": the call as the formula gets it (empty arguments at the end left off).
    QString callText() const;
    QStringList arguments() const;
    int rowCount() const { return int(m_rows.size()); }
    QString argumentName(int row) const;
    bool isOptional(int row) const;
    QLineEdit *edit(int row) const { return m_rows[size_t(row)].edit; }
    QLabel *valueLabel(int row) const { return m_rows[size_t(row)].value; }
    QToolButton *pickButton(int row) const { return m_rows[size_t(row)].pick; }
    void setArgumentText(int row, const QString &text);
    QLabel *summaryLabel() const { return m_summary; }
    QLabel *argumentHelpLabel() const { return m_argumentHelp; }
    QLabel *resultLabel() const { return m_result; }
    QLabel *problemLabel() const { return m_problem; }
    QLabel *helpLink() const { return m_helpLink; }
    QDialogButtonBox *buttons() const { return m_buttons; }
    // Works out what is waiting now (it is done a moment after typing stops, and a test does not wait for that).
    void evaluateNow();
    // The strip that stands for the dialog while cells are pointed at on the sheet (null when none is open), and the row it is for.
    bool isPicking() const { return m_strip != nullptr; }
    QLineEdit *stripEdit() const;
    int pickingRow() const { return m_pickRow; }
    void beginPicking(int row);
    void endPicking();

Q_SIGNALS:
    // Cells are to be pointed at for argument `row`: the window then writes what the person selects with setArgumentText until pickingEnded.
    void pickingStarted(int row);
    void pickingEnded();
    void helpRequested(const QString &topic);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct Row {
        QString name, helpName;   // (a row of a repeat is helped by the name of the first: number2 as number1)
        bool optional = false;
        int group = -1;   // the number of the repeat of the group this row is of; -1 for a row that is always there
        QLabel *label = nullptr, *value = nullptr;
        QLineEdit *edit = nullptr;
        QToolButton *pick = nullptr;
    };
    void addRow(const QString &name, bool optional, int group, const QString &text, const QString &helpName = QString());
    void addGroup();
    void textChanged(int row);
    void focusRow(int row);
    QString expression(const QString &text) const;
    const crunch::EditSession::Preview &previewOf(const QString &expression);
    QString fullCall() const;
    void showPreview(QLabel *label, const crunch::EditSession::Preview &p, bool withEquals);
    QString m_name;
    const crunch::FunctionDoc &m_doc;
    crunch::ArgumentPlan m_plan;
    QChar m_separator;
    Evaluator m_evaluator;
    QString m_helpTopic;
    std::vector<Row> m_rows;
    int m_groups = 0;
    QGridLayout *m_grid = nullptr;
    QScrollArea *m_scroll = nullptr;
    QLabel *m_summary = nullptr, *m_argumentHelp = nullptr, *m_result = nullptr, *m_problem = nullptr, *m_helpLink = nullptr;
    QDialogButtonBox *m_buttons = nullptr;
    QTimer m_timer;
    std::map<QString, crunch::EditSession::Preview> m_known;   // (what each text came to: typing in one box does not work the others out again)
    QWidget *m_strip = nullptr;
    int m_pickRow = -1;
    bool m_syncing = false, m_building = true;
};

} // namespace jp

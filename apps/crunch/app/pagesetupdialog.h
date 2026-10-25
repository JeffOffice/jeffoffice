#pragma once
// The Page Setup dialog: how a sheet prints, in four tabs (Page, Margins, Header and Footer, Sheet), and the editor of a header or footer's text with its insert buttons. The dialog
// works on a copy of the sheet's setup and gives it back changed; the window makes the change one step of Undo. A setup the dialog is given and the person changes nothing in comes
// back as it was (the fields the dialog does not show are never touched).

#include "sheet/pagesetup.h"

#include <QDialog>

#include <functional>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QRadioButton;
class QSpinBox;
class QTabWidget;

namespace jp {

// What the dialog is given besides the setup: the sheet's name (the presets of the header and footer are made from it), the file's name, and where to open.
struct PageSetupRequest {
    jp::office::sheet::PageSetup setup;
    QString sheetName;
    QString fileName;
    QString selectionText;      // the selection as "A1:D20", for the print area box's tip
    int tab = 0;                // 0 Page, 1 Margins, 2 Header and Footer, 3 Sheet
    bool metric = false;        // margins in centimeters
};

// A picture's file read to be put in a header or footer: kept as the file has it, with the size it is drawn at (its pixels at 96 to the inch, the longest side held to two inches), a title from
// the file's name, and no place yet. False, with the reason in plain words, when the file is not a PNG, JPEG, GIF, or BMP picture, is too large, or can't be read.
bool loadHeaderPicture(const QString &path, jp::office::sheet::HeaderPicture *out, QString *problem, double roomPoints = 0);
// How tall a picture may be to fit the margin it goes in: the margin's room between the header's (footer's) distance from the edge and the cells, in points (at least 12).
double headerPictureRoom(const jp::office::sheet::PageMargins &margins, bool header);

// The text of a header or a footer for each version the setup keeps: edited in three boxes (left, center, and right), with buttons that put the page number, the date, and so on in.
class HeaderFooterEditor : public QDialog {
    Q_OBJECT
public:
    HeaderFooterEditor(bool header, const jp::office::sheet::HeaderFooter &texts, const QString &sheetName, const QString &fileName, QWidget *parent = nullptr);
    // The texts as they are now (the flags of the setup are not changed here).
    jp::office::sheet::HeaderFooter texts() const;
    // The three boxes of the version shown, and the versions offered (in the order of the list).
    QStringList versionNames() const;
    void selectVersion(int index);
    QPlainTextEdit *box(int part) const;   // 0 left, 1 center, 2 right
    // Which box the buttons put their text in (the one last typed in, to begin with the middle).
    void setInsertPart(int part) { m_focused = box(part); }
    // For tests: stands in for the file picker of the Picture button (it gives the picture's file name; empty is giving it up).
    void setPictureAsker(const std::function<QString()> &asker) { m_pictureAsker = asker; }
    // How tall a picture put in may be, in points, so that it fits the margin (0: no limit): a picture is made to fit it, and can be made larger afterwards.
    void setPictureRoom(double points) { m_room = points; }
    // What was said of the last picture that was refused (empty when it was taken).
    QString pictureProblem() const;

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void commit();
    void load(int version);
    void insert(const QString &code);
    void refreshPreview();
    void addPicture();
    void removePicture();
    QString place() const;
    void showPictureState();
    struct Version {
        QString name;
        QString *text;
        QString form;      // the end of a picture's place for this version: nothing, "E" for the even pages, "F" for the first
    };
    bool m_header;
    jp::office::sheet::HeaderFooter m_hf;
    QString m_sheetName, m_fileName;
    std::vector<Version> m_versions;
    int m_current = 0;
    QComboBox *m_version = nullptr;
    QPlainTextEdit *m_boxes[3] = {nullptr, nullptr, nullptr};
    QPlainTextEdit *m_focused = nullptr;
    QLabel *m_preview = nullptr;
    QLabel *m_pictureLabel = nullptr;
    QPushButton *m_addPicture = nullptr, *m_dropPicture = nullptr;
    std::function<QString()> m_pictureAsker;
    double m_room = 0;
    bool m_loading = false;
};

class PageSetupDialog : public QDialog {
    Q_OBJECT
public:
    explicit PageSetupDialog(const PageSetupRequest &request, QWidget *parent = nullptr);
    // The setup with what the person set in the dialog put over the one it was given.
    jp::office::sheet::PageSetup setup() const;
    QTabWidget *tabs() const { return m_tabs; }
    // For tests: stands in for the window that edits a header or footer (it is given the editor, and says whether to go on).
    void setEditorAsker(const std::function<bool(HeaderFooterEditor *)> &asker) { m_hfEditorAsker = asker; }

    // The text of the preset lists of the header and footer: the strings of the codes they make, for the sheet named (a test reads them).
    static QVector<QPair<QString, QString>> headerFooterPresets(const QString &sheetName, const QString &fileName);

public Q_SLOTS:
    void accept() override;

private:
    void buildPage(QWidget *tab);
    void buildMargins(QWidget *tab);
    void buildHeaderFooter(QWidget *tab);
    void buildSheet(QWidget *tab);
    void updateFitState();
    void editHeaderFooter(bool header);
    void fillPresets(QComboBox *box, const QString &code);
    void refreshHeaderFooterPreview();
    // What the dialog's controls say, all in one place: what is compared with what they said when it opened, so that only what the person changed goes back.
    struct View {
        bool landscape = false, fit = false;
        int scale = 100, across = 0, down = 0, paper = 1;
        bool useFirst = false;
        int first = 1;
        double margin[6] = {0, 0, 0, 0, 0, 0};
        bool centerAcross = false, centerDown = false, differentFirst = false, differentOddEven = false, scaleWith = true, align = true;
        QString area, rows, cols;
        bool grid = false, headings = false, blackWhite = false, draft = false;
        int notes = 0, errors = 0;
        bool across_first = false;
    };
    View view() const;
    bool readAreas(QVector<jp::office::sheet::CellRange> *out, QString *problem) const;
    bool readTitles(jp::office::sheet::PrintTitles *out, QString *problem) const;

    PageSetupRequest m_request;
    std::function<bool(HeaderFooterEditor *)> m_hfEditorAsker;
    View m_initial;
    jp::office::sheet::PageSetup m_setup;   // as it is being edited: the header and footer texts live here until the dialog is accepted
    QTabWidget *m_tabs = nullptr;
    // page
    QRadioButton *m_portrait = nullptr, *m_landscape = nullptr, *m_scaleRadio = nullptr, *m_fitRadio = nullptr;
    QSpinBox *m_scale = nullptr, *m_fitAcross = nullptr, *m_fitDown = nullptr, *m_firstNumber = nullptr;
    QCheckBox *m_useFirst = nullptr;
    QComboBox *m_paper = nullptr;
    // margins
    QDoubleSpinBox *m_margin[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};   // left, right, top, bottom, header, footer
    QCheckBox *m_centerAcross = nullptr, *m_centerDown = nullptr;
    // header and footer
    QComboBox *m_headerPreset = nullptr, *m_footerPreset = nullptr;
    QCheckBox *m_differentFirst = nullptr, *m_differentOddEven = nullptr, *m_scaleWith = nullptr, *m_alignMargins = nullptr;
    QLabel *m_headerShown = nullptr, *m_footerShown = nullptr;
    // sheet
    QLineEdit *m_area = nullptr, *m_titleRows = nullptr, *m_titleCols = nullptr;
    QCheckBox *m_grid = nullptr, *m_blackWhite = nullptr, *m_draft = nullptr, *m_headings = nullptr;
    QComboBox *m_notes = nullptr, *m_errors = nullptr;
    QRadioButton *m_down = nullptr, *m_across = nullptr;
    QLabel *m_problem = nullptr;
    QDialogButtonBox *m_buttons = nullptr;
};

} // namespace jp

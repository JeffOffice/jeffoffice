#pragma once
// The one place a link goes, chosen the same way in every app: a web address, an email address, a place in this document (the app gives the list), an existing file, and a new
// document (an empty one of the app's own kind is made at a chosen path, and the link goes to it). The widget holds no link of its own: the app turns what it says into the link
// its file format keeps. It never looks for a file that is typed (a path on another computer is not touched by it).

#include "core/export.h"

#include <QString>
#include <QStringList>
#include <QVector>
#include <QWidget>
#include <functional>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

namespace jp {

struct LinkPlace {
    QString label;   // what the list shows
    QString value;   // what the app keeps for it (a bookmark's name, a slide's link)
};

// The most places a list holds (a file can hold more headings, bookmarks, or sheets than anyone picks from); the rest are left off.
constexpr int kMostLinkPlaces = 5000;

class JO_COMMON_EXPORT LinkTargetPicker : public QWidget {
    Q_OBJECT
public:
    // The kinds, in the order the list shows them (AppDocument and NewDocument only when the app asks for them).
    enum Kind { Web, Email, Place, File, AppDocument, NewDocument };

    explicit LinkTargetPicker(QWidget *parent = nullptr);

    // What the app calls its places ("Place in this document") and the list's label ("Place").
    void setPlaceNames(const QString &kindName, const QString &fieldLabel);
    // The places to pick from (at most kMostLinkPlaces; the count that was left off is truncated()).
    // `current` is the place the link has now (when it has one): it is always in the list, first, inside the most, and chosen, so that editing a link never changes where it goes.
    void setPlaces(const QVector<LinkPlace> &places, const LinkPlace &current = LinkPlace());
    int placeCount() const;
    bool truncated() const { return m_truncated; }
    // Another document of the app's own kind to link to by its path (a presentation that is played from the link): `kindName` is its name in the list.
    void enableAppDocument(const QString &kindName, const QString &filter);
    // A new document: `createEmpty` writes an empty one of the app's kind at the path (false, with the reason, when it can't); `filter` and `suffix` are for the box that picks its name.
    // `suffixes` are the endings a new document may have (the first is added to a name with none); a path of any other ending, or not a whole path on this computer, is refused before anything is made.
    void enableNewDocument(const QString &filter, const QStringList &suffixes, const std::function<bool(const QString &path, QString *why)> &createEmpty);

    Kind kind() const;
    void setKind(Kind kind);
    // The text of the address (or path) box.
    QString address() const;
    void setAddress(const QString &text);
    // The value of the place chosen ("" when none); chooses the place with that value (false when the list has none).
    QString placeValue() const;
    bool setPlaceValue(const QString &value);

    // What the choice says, for the app to turn into its link: the address as typed (an email address with "mailto:" in front), the path of a file, or the value of a place. For a new
    // document, the path once prepare() made it. Empty where nothing is chosen.
    QString target() const;
    // Makes the new document when that is the kind (never over a file that is there): true, or false with `problem` told. Nothing is done for the other kinds.
    bool prepare(QString *problem = nullptr);
    // Takes away the document prepare() made (the link could not be made after all).
    void discardCreated();

Q_SIGNALS:
    void changed();

private:
    void refresh();
    void browse();
    QComboBox *m_kind, *m_place;
    QLineEdit *m_address;
    QLabel *m_addressLabel, *m_placeLabel;
    QWidget *m_addressRow, *m_placeRow;
    QPushButton *m_browse;
    QString m_placeKindName, m_appDocumentFilter, m_newFilter, m_created;
    QStringList m_newSuffixes;
    QLabel *m_placeNote;
    std::function<bool(const QString &, QString *)> m_createEmpty;
    bool m_truncated = false;
};

} // namespace jp

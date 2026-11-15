#pragma once
// The layout of the printed notes page and handout, which the Notes Master and Handout Master views edit: where the slide picture, the notes, the header, date, footer, and page
// number sit on the page, how many slides a handout holds and whether it lies across, and the page's background. Places are fractions of the page (0 to 1), so they hold for any
// paper. They are the presentation's, kept in its appData ("notesMaster", "handoutMaster") and saved in the file as the places of the notes master and the handout master.
// A presentation with none has the layout printing always had (defaultNotesMaster, defaultHandoutMaster).

#include "core/document.h"

#include <QColor>
#include <QJsonObject>
#include <QRectF>
#include <QSizeF>
#include <QVector>

namespace jp::deck {

enum class NotesPart { SlideImage, Body, Header, Date, Footer, Number };

struct NotesMaster {
    bool custom = false;   // the presentation has a layout of its own
    QRectF slideImage, body, header, date, footer, number;   // fractions of the page
    QColor background;     // invalid: the page's white
    QRectF &part(NotesPart p);
    const QRectF &part(NotesPart p) const;
    bool operator==(const NotesMaster &o) const;
    bool operator!=(const NotesMaster &o) const { return !(*this == o); }
};

enum class HandoutPart { Area, Header, Date, Footer, Number };

struct HandoutMaster {
    bool custom = false;
    int perPage = 6;       // 1, 2, 3, 4, 6, or 9
    bool landscape = false;
    QRectF area, header, date, footer, number;   // fractions of the page; the slides fill `area`
    QColor background;
    QRectF &part(HandoutPart p);
    const QRectF &part(HandoutPart p) const;
    bool operator==(const HandoutMaster &o) const;
    bool operator!=(const HandoutMaster &o) const { return !(*this == o); }
};

// The layout printing always had, for a page of `paper` points (the notes page's slide picture is as large as the page's upper half lets it be, in the slide's shape).
NotesMaster defaultNotesMaster(const QSizeF &paper, const QSizeF &slide);
HandoutMaster defaultHandoutMaster(const QSizeF &paper);
bool isHandoutCount(int perPage);
// The places the file formats' own masters have (a master with these is the same as having none: printing keeps its layout), and whether two places are the same within `tolerance`.
NotesMaster standardNotesMaster(const QSizeF &slide);
HandoutMaster standardHandoutMaster();
bool nearly(const QRectF &a, const QRectF &b, double tolerance = 0.003);

// What the presentation has (the default for a page of `paper` when it has none).
NotesMaster notesMasterOf(const Document &d, const QSizeF &paper);
HandoutMaster handoutMasterOf(const Document &d, const QSizeF &paper);
bool hasNotesMaster(const Document &d);
bool hasHandoutMaster(const Document &d);
// Keeps the layout (no step of its own: the caller records one). Places are brought inside the page and given a least size; an odd count is brought to the nearest of 1, 2, 3, 4, 6, 9.
void setNotesMaster(Document &d, const NotesMaster &m);
void setHandoutMaster(Document &d, const HandoutMaster &m);
// How many slides a handout page holds is kept on its own (a file can say it with no handout master).
void setHandoutPerPage(Document &d, int perPage);
bool hasHandoutPerPage(const Document &d);
void clearNotesMaster(Document &d);
void clearHandoutMaster(Document &d);

// The places in points on a page of `paper`.
struct NotesPageGeometry {
    QRectF slideImage, body, header, date, footer, number;
};
NotesPageGeometry notesPageGeometry(const NotesMaster &m, const QSizeF &paper, const QSizeF &slide);

// One slide of a handout page: the cell it stands in and the room it is drawn in (as large as fits, in the slide's shape); `lines` are the lines to write on beside it (three to a page).
struct HandoutSlot {
    QRectF cell, room;
    bool lines = false;
};
struct HandoutGeometry {
    QRectF area, header, date, footer, number;
    QVector<HandoutSlot> slots;   // the slot of each slide of the page, in the order the slides go
};
HandoutGeometry handoutGeometry(const HandoutMaster &m, const QSizeF &paper, int perPage, bool horizontalOrder);
// The page of a handout: `paper` turned across when the master says so.
QSizeF handoutPaper(const HandoutMaster &m, const QSizeF &paper);

// A place read from a file or typed in, made usable: inside the page, at least half a percent each way (a NaN or infinity becomes the fallback).
QRectF sanitizedPlace(const QRectF &r, const QRectF &fallback);

// The master as the JSON in appData, and back (what is read is bounded by sanitizedPlace).
QJsonObject notesMasterJson(const NotesMaster &m);
NotesMaster notesMasterFromJson(const QJsonObject &o, const NotesMaster &fallback);
QJsonObject handoutMasterJson(const HandoutMaster &m);
HandoutMaster handoutMasterFromJson(const QJsonObject &o, const HandoutMaster &fallback);

} // namespace jp::deck

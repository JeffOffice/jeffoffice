#pragma once
// What a link on a slide, a shape, a picture, or a run of text says, as one string: a web address ("https://..."), a mail address ("mailto:..."), a file (a path, or
// "file:" and a path), or a place in the presentation ("#next", "#previous", "#first", "#last", "#page3" for slide 3) or the end of the show ("#end"). Anything else is not
// a link that is ever followed.

#include <QString>

class QDomElement;

namespace jp::office {

enum class LinkKind { None, Web, Mail, File, Slide, Next, Previous, First, Last, EndShow };

struct LinkTarget {
    LinkKind kind = LinkKind::None;
    int slide = 0;         // for Slide: counted from 1
    QString address;       // for Web, Mail, and File: what to open
    bool operator==(const LinkTarget &o) const { return kind == o.kind && slide == o.slide && address == o.address; }
};

LinkTarget parseLink(const QString &link);
// The string for a link to slide `number` (from 1).
QString slideLink(int number);
// A file on another computer: a path that starts with two slashes or two backslashes, or a "file:" address with a host (not "localhost"). It is never touched (not looked for,
// not opened) unless the person agrees.
bool isRemoteFile(const QString &link);
// The address a person is shown and asked about: the target's address, or "" for links inside the presentation.
QString linkAddress(const QString &link);

struct DrawingContext;
// The link an a:hlinkClick says: an address, or one of the strings above for a jump (r:id and ppaction://hlinkshowjump, hlinksldjump, or hlinkfile); empty for a click
// that is no link (a program, a macro, a sound), or a jump to a slide that has no number (DrawingContext::slideNumberOf).
QString linkOfClick(const QDomElement &click, const DrawingContext &ctx);

} // namespace jp::office

#pragma once
// What a link on a slide, a shape, a picture, or a run of text says, as one string: a web address ("https://..."), a mail address ("mailto:..."), a file (a path, or
// "file:" and a path), or a place in the presentation ("#next", "#previous", "#first", "#last", "#page3" for slide 3) or the end of the show ("#end"). Anything else is not
// a link that is ever followed.

#include <QString>
#include <QStringList>

class QDomElement;

namespace jp::office {

// Also: "#lastviewed"; a custom show ("#show:4", or "#showback:4" to come back to the slide it was followed from); and another presentation ("#deck:" and its path).
enum class LinkKind { None, Web, Mail, File, Slide, Next, Previous, First, Last, EndShow, LastViewed, CustomShow, Deck };

struct LinkTarget {
    LinkKind kind = LinkKind::None;
    int slide = 0;         // for Slide: counted from 1
    QString address;       // for Web, Mail, File, and Deck: what to open
    bool back = false;     // for CustomShow: the show comes back when it ends (slide is the show's id)
    bool operator==(const LinkTarget &o) const { return kind == o.kind && slide == o.slide && address == o.address && back == o.back; }
};

LinkTarget parseLink(const QString &link);
// The string for a link to slide `number` (from 1).
QString slideLink(int number);
// The strings for a link to the custom show with that id (coming back when it ends, or not), and to the presentation at a path.
QString customShowLink(int id, bool back);
QString presentationLink(const QString &path);
// A file on another computer: a path that starts with two slashes or two backslashes, or a "file:" address with a host (not "localhost"). It is never touched (not looked for,
// not opened) unless the person agrees.
bool isRemoteFile(const QString &link);
// A file that runs when it is opened (a program or a script, by the ending of its name, or on this computer by being marked as one that runs). Following a link to one is
// refused. A file on another computer is judged by its name alone and is never looked at.
bool isProgramFile(const QString &link);
// The same for a path (or a "file:" address) taken as it is, whatever it looks like as a link: "evil.exe", "a.pptx.exe", and "sub/a.exe" are programs.
bool isProgramPath(const QString &path);
// How many times a file on this computer was looked at to judge a link (a test's counter: a file on another computer never is).
int linkFileLooks();
// The address a person is shown and asked about: the target's address, or "" for links inside the presentation.
QString linkAddress(const QString &link);

// What a link to a file may be followed to, decided from the text alone (nothing is looked at, so a path on another computer is never touched): an absolute path on this computer (or a
// "file:" address with no host or "localhost"), with no control character, no "..%00"-style hidden character, no stream or device name, whose ending is a document or picture type (never a
// script or a program, and not .svg, which can hold scripts). `path` gets the path when it is. A relative path, a path on another computer, and any other ending are refused.
bool isOpenableFileLink(const QString &fileAddressOrPath, QString *path = nullptr);
// Whether the text is a whole path on this computer (absolute, not on another computer, no control character), judged from the text alone.
bool isLocalAbsolutePath(const QString &path);
// Whether a path ends as one of these (compared without regard to case).
bool pathEndsWithOneOf(const QString &path, const QStringList &endings);

struct DrawingContext;
// The link an a:hlinkClick says: an address, or one of the strings above for a jump (r:id and ppaction://hlinkshowjump, hlinksldjump, or hlinkfile); empty for a click
// that is no link (a program, a macro, a sound), or a jump to a slide that has no number (DrawingContext::slideNumberOf).
QString linkOfClick(const QDomElement &click, const DrawingContext &ctx);

// A click that is no link here and must not be lost on saving (a program, a macro, another presentation, a custom show): its address (empty when it names none), a unit
// separator, and its XML without the relationship id. Empty for a click that is a link, a sound or movie (written from the object), or has no action.
QString keptClickOf(const QDomElement &click, const DrawingContext &ctx);

// What a click or mouse-over element that is a link carried besides it: its XML (without the relationship id) when it has attributes other than the id and the action (a tooltip,
// highlightClick), has children (a sound), or is a link to another presentation (which says the slide and its title); else empty.
QString extrasOfClick(const QDomElement &click, const DrawingContext &ctx);

} // namespace jp::office

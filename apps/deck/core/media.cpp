#include "core/media.h"

#include "core/animation.h"

#include <QBuffer>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <algorithm>
#include <vector>

namespace jp::deck {

namespace {

// ---- the timing as a small tree ----
//
// The slide's timing is kept as the file wrote it. Changing what it says of
// one picture reads it into this tree (names and attributes as written, in
// their order, so what isn't touched is written back as it was), changes the
// tree, and writes it again.

constexpr int kMaxTreeNodes = 200000, kMaxTreeDepth = 128;

struct Node {
    QString name;                                 // with its prefix, as written
    std::vector<std::pair<QString, QString>> attrs;
    std::vector<Node> kids;                       // elements
    QString text;                                 // of an element that has no element inside it

    QString local() const { return name.mid(name.indexOf(QLatin1Char(':')) + 1); }
    QString attr(const char *key) const
    {
        for (const auto &a : attrs)
            if (a.first == QLatin1String(key)) return a.second;
        return QString();
    }
    void set(const char *key, const QString &value)
    {
        for (auto &a : attrs)
            if (a.first == QLatin1String(key)) {
                a.second = value;
                return;
            }
        attrs.emplace_back(QLatin1String(key), value);
    }
    const Node *find(const char *localName) const
    {
        for (const Node &k : kids)
            if (k.local() == QLatin1String(localName)) return &k;
        return nullptr;
    }
    Node *find(const char *localName)
    {
        for (Node &k : kids)
            if (k.local() == QLatin1String(localName)) return &k;
        return nullptr;
    }
};

bool readTree(const QString &xml, Node *out)
{
    QXmlStreamReader r(xml);
    std::vector<Node> open;
    int count = 0;
    while (!r.atEnd()) {
        switch (r.readNext()) {
        case QXmlStreamReader::StartElement: {
            if (++count > kMaxTreeNodes || int(open.size()) >= kMaxTreeDepth) return false;
            Node n;
            n.name = r.qualifiedName().toString();
            for (const QXmlStreamNamespaceDeclaration &d : r.namespaceDeclarations())
                n.attrs.emplace_back(d.prefix().isEmpty() ? QStringLiteral("xmlns") : QStringLiteral("xmlns:") + d.prefix().toString(), d.namespaceUri().toString());
            for (const QXmlStreamAttribute &a : r.attributes()) n.attrs.emplace_back(a.qualifiedName().toString(), a.value().toString());
            open.push_back(std::move(n));
            break;
        }
        case QXmlStreamReader::EndElement: {
            Node n = std::move(open.back());
            open.pop_back();
            if (!n.kids.empty()) n.text.clear();
            if (open.empty()) {
                *out = std::move(n);
                return !r.hasError();
            }
            open.back().kids.push_back(std::move(n));
            break;
        }
        case QXmlStreamReader::Characters:
            if (!open.empty() && open.back().kids.empty()) open.back().text += r.text().toString();
            break;
        default:
            break;
        }
    }
    return false;
}

void writeTree(QXmlStreamWriter &w, const Node &n)
{
    w.writeStartElement(n.name);
    for (const auto &a : n.attrs) w.writeAttribute(a.first, a.second);
    if (n.kids.empty() && !n.text.isEmpty()) w.writeCharacters(n.text);
    for (const Node &k : n.kids) writeTree(w, k);
    w.writeEndElement();
}

QString treeXml(const Node &root)
{
    QString out;
    QXmlStreamWriter w(&out);
    writeTree(w, root);
    return out;
}

using Attrs = std::initializer_list<std::pair<const char *, QString>>;

Node el(const char *name, Attrs attrs = {}, std::vector<Node> kids = {})
{
    Node n;
    n.name = QLatin1String(name);
    for (const auto &a : attrs) n.attrs.emplace_back(QLatin1String(a.first), a.second);
    n.kids = std::move(kids);
    return n;
}

// ---- reading what the timing says of the media ----

// The shape number a node's target names (the first p:spTgt under it), 0 for none.
int targetOf(const Node &n, int depth = 0)
{
    if (n.local() == QLatin1String("spTgt")) return n.attr("spid").toInt();
    if (depth > kMaxTreeDepth) return 0;
    for (const Node &k : n.kids)
        if (const int s = targetOf(k, depth + 1)) return s;
    return 0;
}

struct MediaNode {
    bool video = false;
    int volume = 50000;
    bool mute = false;
    int across = 1;
    bool showWhenStopped = true;
    bool fullScreen = false;
    bool loop = false;
    bool rewind = false;
};

// The p:video and p:audio nodes under the timing's root, by the shape they play.
void collectMediaNodes(const Node &n, QHash<int, MediaNode> &out, int depth = 0)
{
    const QString local = n.local();
    if (local == QLatin1String("video") || local == QLatin1String("audio")) {
        const Node *media = n.find("cMediaNode");
        const int shape = media ? targetOf(*media) : 0;
        if (shape > 0 && !out.contains(shape)) {
            MediaNode m;
            m.video = local == QLatin1String("video");
            const QString full = n.attr("fullScrn");
            m.fullScreen = m.video && (full == QLatin1String("1") || full == QLatin1String("true"));
            auto number = [&](const char *key, int fallback) {
                bool ok = false;
                const qint64 v = media->attr(key).toLongLong(&ok);
                return ok ? int(std::clamp<qint64>(v, 0, 1000000000)) : fallback;
            };
            m.volume = std::min(number("vol", 50000), 100000);
            const QString mute = media->attr("mute");
            m.mute = mute == QLatin1String("1") || mute == QLatin1String("true");
            m.across = std::max(1, number("numSld", 1));
            const QString show = media->attr("showWhenStopped");
            m.showWhenStopped = !(show == QLatin1String("0") || show == QLatin1String("false"));
            if (const Node *cTn = media->find("cTn")) {
                m.loop = cTn->attr("repeatCount") == QLatin1String("indefinite");
                m.rewind = cTn->attr("fill") == QLatin1String("remove");
            }
            out.insert(shape, m);
        }
        return;
    }
    if (depth > kMaxTreeDepth) return;
    for (const Node &k : n.kids) collectMediaNodes(k, out, depth + 1);
}

QHash<int, MediaNode> mediaNodesOf(const QString &timingXml)
{
    QHash<int, MediaNode> out;
    Node root;
    if (!timingXml.isEmpty() && readTree(timingXml, &root)) collectMediaNodes(root, out);
    return out;
}

// How the timing starts each picture: in the click sequence (or automatically,
// when its call goes with or after another, or its step begins by itself), or
// by a click on a shape.
struct Starts {
    QHash<int, MediaStart> byShape;
    QHash<int, int> length;   // ms the call to the media says it lasts, where it says
};

Starts startsOf(const Timeline &tl)
{
    Starts s;
    for (const Step &step : tl.steps)
        for (const Effect &e : step.effects) {
            if (e.cls != EffectClass::Media) continue;
            for (const Behavior &b : e.behaviors)
                if (b.kind == Behavior::Kind::Command && b.target.shape > 0 && !s.byShape.contains(b.target.shape)) {
                    s.byShape.insert(b.target.shape, step.automatic || e.start != EffectStart::OnClick ? MediaStart::Automatically : MediaStart::ClickSequence);
                    if (b.timing.duration > 1 && b.timing.duration < 86400000) s.length.insert(b.target.shape, int(b.timing.duration));
                }
        }
    for (const Trigger &t : tl.triggers)
        for (const Effect &e : t.effects)
            if (e.cls == EffectClass::Media)
                for (const Behavior &b : e.behaviors)
                    if (b.kind == Behavior::Kind::Command && b.target.shape > 0 && !s.byShape.contains(b.target.shape)) {
                        s.byShape.insert(b.target.shape, MediaStart::WhenClickedOn);
                        if (b.timing.duration > 1 && b.timing.duration < 86400000) s.length.insert(b.target.shape, int(b.timing.duration));
                    }
    return s;
}

MediaPlay playOf(const PictureItem &pic, const QHash<int, MediaNode> &nodes, const Starts &starts)
{
    MediaPlay p;
    p.shape = pic.sourceId;
    p.video = pic.mediaKind == QLatin1String("video");
    p.trimStart = std::max(0, pic.mediaTrimStart);
    p.trimEnd = std::max(0, pic.mediaTrimEnd);
    p.fadeIn = std::max(0, pic.mediaFadeIn);
    p.fadeOut = std::max(0, pic.mediaFadeOut);
    if (pic.sourceId > 0) {
        const auto node = nodes.constFind(pic.sourceId);
        if (node != nodes.constEnd()) {
            p.volume = node->volume;
            p.mute = node->mute;
            p.loop = node->loop;
            p.rewind = node->rewind;
            p.fullScreen = node->fullScreen;
            p.hideWhenStopped = !node->showWhenStopped;
            p.acrossSlides = node->across;
        }
        p.start = starts.byShape.value(pic.sourceId, MediaStart::ClickSequence);
        p.lengthHint = starts.length.value(pic.sourceId, 0);
    }
    return p;
}

// ---- writing ----

// Where the timing's nodes are: its root's list of children, and the main sequence in it.
struct TimingTree {
    Node root;                        // p:timing
    Node *list = nullptr;             // the childTnLst of the root time node
    Node *mainSeq = nullptr;          // the p:seq of the main sequence, if any
    int nextId = 2;

    bool locate()
    {
        Node *tnLst = root.find("tnLst");
        Node *par = tnLst ? tnLst->find("par") : nullptr;
        Node *cTn = par ? par->find("cTn") : nullptr;
        list = cTn ? cTn->find("childTnLst") : nullptr;
        mainSeq = nullptr;
        if (!list) return false;
        for (Node &k : list->kids)
            if (k.local() == QLatin1String("seq"))
                if (const Node *c = k.find("cTn"); c && c->attr("nodeType") == QLatin1String("mainSeq")) {
                    mainSeq = &k;
                    break;
                }
        return true;
    }
    int takeId() { return nextId++; }
    void scanIds(const Node &n, int depth = 0)
    {
        if (n.local() == QLatin1String("cTn")) nextId = std::max(nextId, n.attr("id").toInt() + 1);
        if (depth > kMaxTreeDepth) return;
        for (const Node &k : n.kids) scanIds(k, depth + 1);
    }
};

Node skeleton()
{
    return el("p:timing", {{"xmlns:p", QStringLiteral("http://schemas.openxmlformats.org/presentationml/2006/main")}},
              {el("p:tnLst", {}, {el("p:par", {}, {el("p:cTn", {{"id", QStringLiteral("1")}, {"dur", QStringLiteral("indefinite")}, {"restart", QStringLiteral("never")},
                                                                {"nodeType", QStringLiteral("tmRoot")}},
                                                   {el("p:childTnLst")})})})});
}

Node mainSequence(int id)
{
    auto cond = [](const char *evt) {
        return el("p:cond", {{"evt", QLatin1String(evt)}, {"delay", QStringLiteral("0")}}, {el("p:tgtEl", {}, {el("p:sldTgt")})});
    };
    return el("p:seq", {{"concurrent", QStringLiteral("1")}, {"nextAc", QStringLiteral("seek")}},
              {el("p:cTn", {{"id", QString::number(id)}, {"dur", QStringLiteral("indefinite")}, {"nodeType", QStringLiteral("mainSeq")}}, {el("p:childTnLst")}),
               el("p:prevCondLst", {}, {cond("onPrev")}), el("p:nextCondLst", {}, {cond("onNext")})});
}

bool isMediaCall(const Node &par)
{
    const Node *cTn = par.local() == QLatin1String("par") ? par.find("cTn") : nullptr;
    return cTn && cTn->attr("presetClass") == QLatin1String("mediacall");
}

// Takes out of an effect of media calls the calls aimed at `shape`; whether any went.
bool removeCmds(Node &effect, int shape)
{
    bool removed = false;
    Node *cTn = effect.find("cTn");
    Node *list = cTn ? cTn->find("childTnLst") : nullptr;
    if (!list) return false;
    for (size_t i = 0; i < list->kids.size();) {
        if (list->kids[i].local() == QLatin1String("cmd") && targetOf(list->kids[i]) == shape) {
            list->kids.erase(list->kids.begin() + i);
            removed = true;
        } else {
            ++i;
        }
    }
    return removed;
}

bool effectIsEmpty(const Node &effect)
{
    const Node *cTn = effect.find("cTn");
    const Node *list = cTn ? cTn->find("childTnLst") : nullptr;
    return !list || list->kids.empty();
}

bool containerIsEmpty(const Node &par)
{
    const Node *cTn = par.find("cTn");
    const Node *list = cTn ? cTn->find("childTnLst") : nullptr;
    return !list || list->kids.empty();
}

// Takes the calls to `shape` out of the nodes under `n`, and what held only them with them; whether any went.
bool stripCalls(Node &n, int shape, int depth = 0)
{
    if (depth > kMaxTreeDepth) return false;
    bool changed = false;
    for (size_t i = 0; i < n.kids.size();) {
        Node &k = n.kids[i];
        bool gone = false;
        if (isMediaCall(k)) {
            if (removeCmds(k, shape)) {
                changed = true;
                gone = effectIsEmpty(k);
            }
        } else if (stripCalls(k, shape, depth + 1)) {
            changed = true;
            gone = k.local() == QLatin1String("par") && containerIsEmpty(k);
        }
        if (gone) n.kids.erase(n.kids.begin() + i);
        else ++i;
    }
    return changed;
}

// Takes out everything the timing does for the shape's media.
void removeMedia(TimingTree &t, int shape)
{
    if (!t.list) return;
    for (size_t i = 0; i < t.list->kids.size();) {
        Node &k = t.list->kids[i];
        const QString local = k.local();
        bool gone = false;
        if ((local == QLatin1String("video") || local == QLatin1String("audio")) && targetOf(k) == shape) {
            gone = true;
        } else if (local == QLatin1String("seq")) {
            Node *cTn = k.find("cTn");
            if (stripCalls(k, shape) && cTn && cTn->attr("nodeType") == QLatin1String("interactiveSeq")) {
                const Node *inner = cTn->find("childTnLst");
                gone = !inner || inner->kids.empty();
            }
        }
        if (gone) t.list->kids.erase(t.list->kids.begin() + i);
        else ++i;
    }
}

Node target(int shape) { return el("p:tgtEl", {}, {el("p:spTgt", {{"spid", QString::number(shape)}})}); }

// An effect that calls `command` on the shape's media.
Node mediaCall(TimingTree &t, int shape, const QString &command, int preset, const char *nodeType, qint64 ms, int *effectId = nullptr)
{
    const int id = t.takeId(), cmdId = t.takeId();
    if (effectId) *effectId = id;
    return el("p:par", {},
              {el("p:cTn", {{"id", QString::number(id)}, {"presetID", QString::number(preset)}, {"presetClass", QStringLiteral("mediacall")}, {"presetSubtype", QStringLiteral("0")},
                            {"fill", QStringLiteral("hold")}, {"nodeType", QLatin1String(nodeType)}},
                    {el("p:stCondLst", {}, {el("p:cond", {{"delay", QStringLiteral("0")}})}),
                     el("p:childTnLst", {},
                        {el("p:cmd", {{"type", QStringLiteral("call")}, {"cmd", command}},
                            {el("p:cBhvr", {}, {el("p:cTn", {{"id", QString::number(cmdId)}, {"dur", QString::number(std::max<qint64>(1, ms))}, {"fill", QStringLiteral("hold")}}), target(shape)})})})})});
}

// A group at a moment of a step, holding an effect.
Node group(int id, const QString &delay, Node effect)
{
    return el("p:par", {}, {el("p:cTn", {{"id", QString::number(id)}, {"fill", QStringLiteral("hold")}},
                                {el("p:stCondLst", {}, {el("p:cond", {{"delay", delay}})}), el("p:childTnLst", {}, {std::move(effect)})})});
}

// A click on the shape does what the effect says.
Node trigger(TimingTree &t, int shape, const QString &command, int preset, qint64 ms)
{
    const int id = t.takeId(), outer = t.takeId(), inner = t.takeId();
    Node effect = group(outer, QStringLiteral("0"), group(inner, QStringLiteral("0"), mediaCall(t, shape, command, preset, "clickEffect", ms)));
    return el("p:seq", {{"concurrent", QStringLiteral("1")}, {"nextAc", QStringLiteral("seek")}},
              {el("p:cTn", {{"id", QString::number(id)}, {"restart", QStringLiteral("whenNotActive")}, {"fill", QStringLiteral("hold")}, {"evtFilter", QStringLiteral("cancelBubble")},
                            {"nodeType", QStringLiteral("interactiveSeq")}},
                    {el("p:stCondLst", {}, {el("p:cond", {{"evt", QStringLiteral("onClick")}, {"delay", QStringLiteral("0")}}, {target(shape)})}),
                     el("p:endSync", {{"evt", QStringLiteral("end")}, {"delay", QStringLiteral("0")}}, {el("p:rtn", {{"val", QStringLiteral("all")}})}),
                     el("p:childTnLst", {}, {std::move(effect)})}),
               el("p:nextCondLst", {}, {el("p:cond", {{"evt", QStringLiteral("onClick")}, {"delay", QStringLiteral("0")}}, {target(shape)})})});
}

// The p:video or p:audio that says how the media plays.
Node mediaNode(TimingTree &t, const MediaPlay &p, int beginsWith)
{
    const int id = t.takeId();
    Node stCond = el("p:stCondLst", {}, {el("p:cond", {{"delay", QStringLiteral("indefinite")}})});
    if (beginsWith > 0)
        stCond.kids.push_back(el("p:cond", {{"evt", QStringLiteral("onBegin")}, {"delay", QStringLiteral("0")}}, {el("p:tn", {{"val", QString::number(beginsWith)}})}));
    Node cTn = el("p:cTn", {{"id", QString::number(id)}});
    if (p.loop) cTn.attrs.emplace_back(QStringLiteral("repeatCount"), QStringLiteral("indefinite"));
    cTn.attrs.emplace_back(QStringLiteral("fill"), p.rewind ? QStringLiteral("remove") : QStringLiteral("hold"));
    cTn.attrs.emplace_back(QStringLiteral("display"), QStringLiteral("0"));
    cTn.kids.push_back(std::move(stCond));
    if (!p.video)
        cTn.kids.push_back(el("p:endCondLst", {}, {el("p:cond", {{"evt", QStringLiteral("onStopAudio")}, {"delay", QStringLiteral("0")}}, {el("p:tgtEl", {}, {el("p:sldTgt")})})}));
    Node media = el("p:cMediaNode", {{"vol", QString::number(p.volume)}});
    if (p.mute) media.set("mute", QStringLiteral("1"));
    if (p.acrossSlides > 1) media.set("numSld", QString::number(p.acrossSlides));
    if (p.hideWhenStopped) media.set("showWhenStopped", QStringLiteral("0"));
    media.kids.push_back(std::move(cTn));
    media.kids.push_back(target(p.shape));
    Node out = el(p.video ? "p:video" : "p:audio");
    if (p.video && p.fullScreen) out.set("fullScrn", QStringLiteral("1"));
    out.kids.push_back(std::move(media));
    return out;
}

// Puts what the timing does to play the media as `p` says into the tree.
void addMedia(TimingTree &t, const MediaPlay &p, qint64 lengthMs)
{
    if (!t.list) return;
    // How long the call lasts: what the file's length leaves after the trim, else what the settings it comes from say, else nothing known.
    const qint64 ms = lengthMs > 0 ? std::max<qint64>(1, (p.trimEnd > 0 && p.trimEnd < lengthMs ? p.trimEnd : lengthMs) - p.trimStart) : std::max<qint64>(1, p.lengthHint);
    int effectId = 0;
    const QString start = QStringLiteral("playFrom(0.0)");
    if (p.start != MediaStart::WhenClickedOn) {
        if (!t.mainSeq) {
            t.list->kids.insert(t.list->kids.begin(), mainSequence(t.takeId()));
            t.mainSeq = &t.list->kids.front();
        }
        Node *steps = t.mainSeq->find("cTn")->find("childTnLst");
        const bool first = steps->kids.empty();
        if (p.start == MediaStart::Automatically && !first) {
            // After the last step's effects, in that step.
            Node &last = steps->kids.back();
            Node *inner = last.find("cTn") ? last.find("cTn")->find("childTnLst") : nullptr;
            if (inner) {
                const Timeline tl = readTimeline(treeXml(t.root));
                const double end = tl.steps.isEmpty() ? 0 : tl.steps.last().duration();
                const int groupId = t.takeId();
                inner->kids.push_back(group(groupId, QString::number(qint64(end + 0.5)), mediaCall(t, p.shape, start, 1, "afterEffect", ms, &effectId)));
            }
        } else {
            const int stepId = t.takeId(), groupId = t.takeId();
            Node stCond = el("p:stCondLst", {}, {el("p:cond", {{"delay", QStringLiteral("indefinite")}})});
            if (p.start == MediaStart::Automatically)
                stCond.kids.push_back(el("p:cond", {{"evt", QStringLiteral("onBegin")}, {"delay", QStringLiteral("0")}}, {el("p:tn", {{"val", t.mainSeq->find("cTn")->attr("id")}})}));
            Node call = group(groupId, QStringLiteral("0"), mediaCall(t, p.shape, start, 1, p.start == MediaStart::Automatically ? "afterEffect" : "clickEffect", ms, &effectId));
            steps->kids.push_back(el("p:par", {}, {el("p:cTn", {{"id", QString::number(stepId)}, {"fill", QStringLiteral("hold")}}, {std::move(stCond), el("p:childTnLst", {}, {std::move(call)})})}));
        }
    }
    // The media's own node, after the main sequence and before the sequences a click starts.
    Node node = mediaNode(t, p, p.start == MediaStart::Automatically ? effectId : 0);
    size_t at = t.list->kids.size();
    for (size_t i = 0; i < t.list->kids.size(); ++i) {
        const Node &k = t.list->kids[i];
        const Node *cTn = k.local() == QLatin1String("seq") ? k.find("cTn") : nullptr;
        if (cTn && cTn->attr("nodeType") == QLatin1String("interactiveSeq")) {
            at = i;
            break;
        }
    }
    t.list->kids.insert(t.list->kids.begin() + at, std::move(node));
    // A click on a movie pauses it or plays it again; a click starts a sound or a movie that waits for one.
    if (p.start == MediaStart::WhenClickedOn) t.list->kids.push_back(trigger(t, p.shape, p.video ? QStringLiteral("togglePause") : start, p.video ? 2 : 1, p.video ? 1 : ms));
    else if (p.video) t.list->kids.push_back(trigger(t, p.shape, QStringLiteral("togglePause"), 2, 1));
}

// Whether anything is left in the timing worth keeping.
bool timingIsEmpty(const TimingTree &t)
{
    if (!t.list) return true;
    for (const Node &k : t.list->kids) {
        if (k.local() != QLatin1String("seq")) return false;
        const Node *cTn = k.find("cTn");
        if (!cTn || cTn->attr("nodeType") != QLatin1String("mainSeq")) return false;
        const Node *inner = cTn->find("childTnLst");
        if (inner && !inner->kids.empty()) return false;
    }
    // Builds and extensions the file had stay with it.
    for (const Node &k : t.root.kids)
        if (k.local() != QLatin1String("tnLst")) return false;
    return true;
}

int highestShapeNumber(const Page &slide)
{
    int highest = 1;
    walkItems(slide.items, [&](const ItemPtr &it) { highest = std::max(highest, it->sourceId); });
    // The frames the reader kept as they were, and the charts' frames, keep the numbers they had.
    static const QRegularExpression numbers(QStringLiteral("<\\w+:cNvPr\\b[^>]*?\\bid=\"(\\d+)\""));
    auto scan = [&](const QJsonValue &v, const char *key) {
        const QString xml = v.toObject().value(QLatin1String(key)).toString();
        for (auto it = numbers.globalMatch(xml); it.hasNext();) highest = std::max<qint64>(highest, std::min<qint64>(it.next().captured(1).toLongLong(), 2000000000));
    };
    for (const QJsonValue &v : slide.props.value(QStringLiteral("unread")).toArray()) scan(v, "xml");
    for (const QJsonValue &v : slide.props.value(QStringLiteral("charts")).toArray()) scan(v, "frameXml");
    return highest;
}

} // namespace

bool MediaPlay::operator==(const MediaPlay &o) const
{
    return shape == o.shape && video == o.video && start == o.start && volume == o.volume && mute == o.mute && loop == o.loop && rewind == o.rewind && fullScreen == o.fullScreen &&
           hideWhenStopped == o.hideWhenStopped && acrossSlides == o.acrossSlides && trimStart == o.trimStart && trimEnd == o.trimEnd && fadeIn == o.fadeIn && fadeOut == o.fadeOut;
}

QVector<const PictureItem *> mediaPicturesOf(const Page &slide)
{
    QVector<const PictureItem *> out;
    walkItems(slide.items, [&](const ItemPtr &it) {
        if (it->type() != ItemType::Picture) return;
        const auto *pic = static_cast<const PictureItem *>(it.get());
        if ((pic->mediaKind == QLatin1String("video") || pic->mediaKind == QLatin1String("audio")) && (!pic->mediaId.isEmpty() || !pic->mediaLink.isEmpty())) out.push_back(pic);
    });
    return out;
}

MediaPlay mediaPlayOf(const Page &slide, const PictureItem &pic)
{
    const QString timing = slide.props.value(QStringLiteral("timingXml")).toString();
    if (pic.sourceId <= 0 || timing.isEmpty()) return playOf(pic, {}, {});
    return playOf(pic, mediaNodesOf(timing), startsOf(readTimeline(timing)));
}

void setMediaPlay(Page &slide, PictureItem &pic, const MediaPlay &play, qint64 lengthMs)
{
    if (pic.mediaKind != QLatin1String("video") && pic.mediaKind != QLatin1String("audio")) return;
    if (pic.sourceId < 2) pic.sourceId = std::min(highestShapeNumber(slide) + 1, 2000000000);
    pic.mediaTrimStart = std::clamp(play.trimStart, 0, 86400000);
    pic.mediaTrimEnd = std::clamp(play.trimEnd, 0, 86400000);
    pic.mediaFadeIn = std::clamp(play.fadeIn, 0, 86400000);
    pic.mediaFadeOut = std::clamp(play.fadeOut, 0, 86400000);
    MediaPlay p = play;
    p.shape = pic.sourceId;
    p.video = pic.mediaKind == QLatin1String("video");
    p.volume = std::clamp(p.volume, 0, 100000);
    p.acrossSlides = std::clamp(p.acrossSlides, 1, 999);
    TimingTree t;
    const QString was = slide.props.value(QStringLiteral("timingXml")).toString();
    if (was.isEmpty() || !readTree(was, &t.root) || t.root.local() != QLatin1String("timing")) t.root = skeleton();
    if (!t.locate()) t.root = skeleton(), t.locate();
    t.scanIds(t.root);
    removeMedia(t, p.shape);
    addMedia(t, p, lengthMs);
    if (timingIsEmpty(t)) slide.props.remove(QStringLiteral("timingXml"));
    else slide.props[QStringLiteral("timingXml")] = treeXml(t.root);
}

void clearMediaPlay(Page &slide, int shape)
{
    const QString was = slide.props.value(QStringLiteral("timingXml")).toString();
    TimingTree t;
    if (shape <= 0 || was.isEmpty() || !readTree(was, &t.root) || t.root.local() != QLatin1String("timing") || !t.locate()) return;
    removeMedia(t, shape);
    if (timingIsEmpty(t)) slide.props.remove(QStringLiteral("timingXml"));
    else slide.props[QStringLiteral("timingXml")] = treeXml(t.root);
}

// Every shape the timing plays or calls media of.
void collectMediaShapes(const Node &n, QSet<int> &out, int depth = 0)
{
    const QString local = n.local();
    if (local == QLatin1String("video") || local == QLatin1String("audio") || local == QLatin1String("cmd")) {
        if (const int shape = targetOf(n); shape > 0) out.insert(shape);
        return;
    }
    if (depth > kMaxTreeDepth) return;
    for (const Node &k : n.kids) collectMediaShapes(k, out, depth + 1);
}

QString mediaTimingForShapes(const QString &timingXml, const QSet<qint64> &shapes)
{
    TimingTree t;
    if (timingXml.isEmpty() || !readTree(timingXml, &t.root) || t.root.local() != QLatin1String("timing") || !t.locate()) return timingXml;
    QSet<int> played;
    collectMediaShapes(t.root, played);
    bool changed = false;
    for (const int shape : std::as_const(played))
        if (!shapes.contains(shape)) {
            removeMedia(t, shape);
            changed = true;
        }
    if (!changed) return timingXml;
    return timingIsEmpty(t) ? QString() : treeXml(t.root);
}

MediaPlan mediaPlanOf(const Page &slide)
{
    MediaPlan plan;
    const QVector<const PictureItem *> pictures = mediaPicturesOf(slide);
    if (pictures.isEmpty()) return plan;
    const QString timing = slide.props.value(QStringLiteral("timingXml")).toString();
    const QHash<int, MediaNode> nodes = mediaNodesOf(timing);
    const Timeline tl = readTimeline(timing);
    const Starts starts = startsOf(tl);
    for (const PictureItem *pic : pictures) {
        MediaPlay p = playOf(*pic, nodes, starts);
        if (pic->sourceId > 0) plan.plays.insert(pic->sourceId, p);
    }
    // Only calls to the slide's own media count, and a step or a click holds no more than a few of them.
    constexpr int kMaxCues = 64;
    auto cuesOf = [&](const QVector<Effect> &effects) {
        QVector<MediaCue> cues;
        for (const Effect &e : effects) {
            if (e.cls != EffectClass::Media) continue;
            for (const Behavior &b : e.behaviors)
                if (b.kind == Behavior::Kind::Command && plan.plays.contains(b.target.shape) && cues.size() < kMaxCues) cues.push_back(MediaCue{b.target.shape, b.command, b.timing.begin});
        }
        return cues;
    };
    for (const Step &s : tl.steps) plan.steps.push_back(MediaStep{s.automatic, s.visual(), cuesOf(s.effects)});
    for (const Trigger &t : tl.triggers)
        if (t.shape > 0 && plan.triggers.size() < 1000) plan.triggers[t.shape] += cuesOf(t.effects);
    return plan;
}

QSet<QString> mediaHiddenWhenStopped(const Page &slide)
{
    QSet<QString> ids;
    const QVector<const PictureItem *> pictures = mediaPicturesOf(slide);
    if (pictures.isEmpty()) return ids;
    const QHash<int, MediaNode> nodes = mediaNodesOf(slide.props.value(QStringLiteral("timingXml")).toString());
    for (const PictureItem *pic : pictures)
        if (pic->sourceId > 0 && nodes.contains(pic->sourceId) && !nodes.value(pic->sourceId).showWhenStopped) ids.insert(pic->id);
    return ids;
}

// ---- a file put on a slide ----

namespace {
struct FileKind {
    const char *extension, *kind, *type;
};
const FileKind kFiles[] = {
    {"mp4", "video", "video/mp4"},           {"m4v", "video", "video/mp4"},         {"mov", "video", "video/quicktime"},   {"avi", "video", "video/avi"},
    {"wmv", "video", "video/x-ms-wmv"},      {"asf", "video", "video/x-ms-asf"},    {"mpg", "video", "video/mpeg"},        {"mpeg", "video", "video/mpeg"},
    {"mkv", "video", "video/x-matroska"},    {"webm", "video", "video/webm"},       {"ogv", "video", "video/ogg"},         {"3gp", "video", "video/3gpp"},
    {"mp3", "audio", "audio/mpeg"},          {"wav", "audio", "audio/wav"},         {"m4a", "audio", "audio/x-m4a"},       {"wma", "audio", "audio/x-ms-wma"},
    {"aac", "audio", "audio/aac"},           {"ogg", "audio", "audio/ogg"},         {"oga", "audio", "audio/ogg"},         {"flac", "audio", "audio/flac"},
    {"mid", "audio", "audio/mid"},           {"midi", "audio", "audio/mid"},        {"aif", "audio", "audio/aiff"},        {"aiff", "audio", "audio/aiff"},
    {"opus", "audio", "audio/ogg"},
};
} // namespace

QString mediaKindOfFile(const QString &fileName)
{
    const QString ext = QFileInfo(fileName).suffix().toLower();
    for (const FileKind &f : kFiles)
        if (ext == QLatin1String(f.extension)) return QLatin1String(f.kind);
    return QString();
}

QString mediaTypeOfFile(const QString &fileName, const QString &kind)
{
    const QString ext = QFileInfo(fileName).suffix().toLower();
    for (const FileKind &f : kFiles)
        if (ext == QLatin1String(f.extension)) return QLatin1String(f.type);
    return kind == QLatin1String("audio") ? QStringLiteral("audio/unknown") : QStringLiteral("video/unknown");
}

QStringList mediaExtensions(const QString &kind)
{
    QStringList out;
    for (const FileKind &f : kFiles)
        if (kind == QLatin1String(f.kind)) out << QLatin1String(f.extension);
    return out;
}

QImage soundPoster()
{
    QImage img(256, 256, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0x3B, 0x44, 0x52));
    p.drawEllipse(QRectF(8, 8, 240, 240));
    p.setBrush(Qt::white);
    QPainterPath speaker;
    speaker.addPolygon(QPolygonF({QPointF(62, 106), QPointF(94, 106), QPointF(132, 72), QPointF(132, 184), QPointF(94, 150), QPointF(62, 150)}));
    speaker.closeSubpath();
    p.drawPath(speaker);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(Qt::white, 11, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(QRectF(100, 98, 60, 60), -50 * 16, 100 * 16);
    p.drawArc(QRectF(82, 70, 116, 116), -52 * 16, 104 * 16);
    return img;
}

std::shared_ptr<PictureItem> newMediaPicture(Document &doc, Page &slide, const QByteArray &bytes, const QString &fileName, const QImage &poster, qint64 lengthMs, const QPointF &at)
{
    const QString kind = mediaKindOfFile(fileName);
    if (kind.isEmpty() || bytes.isEmpty()) return nullptr;
    const bool video = kind == QLatin1String("video");
    QImage image = video ? poster : soundPoster();
    if (image.isNull()) {
        // A movie that gives no picture: a dark frame the size of a screen.
        image = QImage(640, 360, QImage::Format_RGB32);
        image.fill(QColor(0x20, 0x24, 0x2B));
    }
    // A picture of a frame is kept no larger than a screen needs.
    if (image.width() > 1920) image = image.scaledToWidth(1920, Qt::SmoothTransformation);
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    auto pic = std::make_shared<PictureItem>();
    const QSizeF page = doc.pageSize();
    QSizeF size = video ? QSizeF(image.width() * 0.75, image.height() * 0.75) : QSizeF(48, 48);
    if (video) {
        const QSizeF most = page * 0.8;
        if (size.width() > most.width() || size.height() > most.height()) size.scale(most, Qt::KeepAspectRatio);
    }
    pic->rect = QRectF(at.x() < 0 ? QPointF((page.width() - size.width()) / 2, (page.height() - size.height()) / 2) : at, size);
    pic->imgRect = QRectF(QPointF(0, 0), size);
    pic->imageId = doc.addImage(png, QStringLiteral("png"), QString());
    pic->name = QFileInfo(fileName).fileName();
    pic->mediaKind = kind;
    pic->mediaId = doc.addMedia(bytes, mediaTypeOfFile(fileName, kind), QFileInfo(fileName).suffix());
    if (pic->imageId.isEmpty() || pic->mediaId.isEmpty()) return nullptr;
    MediaPlay play;
    setMediaPlay(slide, *pic, play, lengthMs);
    return pic;
}

} // namespace jp::deck

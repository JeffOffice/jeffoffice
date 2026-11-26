#include "text/storyio.h"

#include <QBrush>
#include <QColor>
#include <QJsonArray>
#include <QPen>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextLength>
#include <QTextList>
#include <algorithm>
#include <QTextOption>

namespace jp {

// Each property is stored as [typeTag, value] keyed by its numeric id, so it
// comes back with the exact QVariant type QTextFormat expects.
QJsonObject formatToJson(const QTextFormat &f)
{
    QJsonObject o;
    const auto props = f.properties();
    for (auto it = props.cbegin(); it != props.cend(); ++it) {
        const QVariant &v = it.value();
        const QString key = QString::number(it.key());
        switch (v.typeId()) {
        case QMetaType::Bool: o[key] = QJsonArray{"b", v.toBool()}; break;
        case QMetaType::Int: o[key] = QJsonArray{"i", v.toInt()}; break;
        case QMetaType::Double: o[key] = QJsonArray{"d", v.toDouble()}; break;
        case QMetaType::QString: o[key] = QJsonArray{"s", v.toString()}; break;
        case QMetaType::QStringList: o[key] = QJsonArray{"sl", QJsonArray::fromStringList(v.toStringList())}; break;
        case QMetaType::QByteArray: o[key] = QJsonArray{"ba", QString::fromLatin1(v.toByteArray().toBase64())}; break;
        case QMetaType::QColor: o[key] = QJsonArray{"c", v.value<QColor>().name(QColor::HexArgb)}; break;
        case QMetaType::QBrush: {
            const QBrush b = v.value<QBrush>();
            if (b.style() == Qt::SolidPattern) o[key] = QJsonArray{"br", b.color().name(QColor::HexArgb)};
            else if (b.style() == Qt::NoBrush) o[key] = QJsonArray{"br", QString()};
            break;
        }
        case QMetaType::QPen: {
            const QPen p = v.value<QPen>();
            o[key] = QJsonArray{"pen", p.style() == Qt::NoPen ? QString() : p.color().name(QColor::HexArgb), p.widthF()};
            break;
        }
        default:
            if (v.canConvert<QTextLength>() && v.userType() == qMetaTypeId<QTextLength>()) {
                const auto l = v.value<QTextLength>();
                o[key] = QJsonArray{"len", int(l.type()), l.rawValue()};
            } else if (v.typeId() == QMetaType::QVariantList) {
                QJsonArray tabs;
                for (const auto &t : v.toList()) {
                    if (t.userType() == qMetaTypeId<QTextOption::Tab>()) {
                        const auto tab = t.value<QTextOption::Tab>();
                        tabs.append(QJsonArray{tab.position, int(tab.type), QString(tab.delimiter)});
                    }
                }
                o[key] = QJsonArray{"tabs", tabs};
            }
            break;
        }
    }
    return o;
}

void formatFromJson(QTextFormat &f, const QJsonObject &o)
{
    for (auto it = o.begin(); it != o.end(); ++it) {
        bool ok = false;
        const int key = it.key().toInt(&ok);
        if (!ok) continue;
        const QJsonArray a = it.value().toArray();
        if (a.size() < 2) continue;
        const QString t = a[0].toString();
        if (t == "b") f.setProperty(key, a[1].toBool());
        else if (t == "i") f.setProperty(key, a[1].toInt());
        else if (t == "d") f.setProperty(key, a[1].toDouble());
        else if (t == "s") f.setProperty(key, a[1].toString());
        else if (t == "ba") f.setProperty(key, QByteArray::fromBase64(a[1].toString().toLatin1()));
        else if (t == "sl") {
            QStringList l;
            for (const auto &x : a[1].toArray()) l << x.toString();
            f.setProperty(key, l);
        } else if (t == "c") f.setProperty(key, QColor(a[1].toString()));
        else if (t == "br") {
            const QString c = a[1].toString();
            f.setProperty(key, c.isEmpty() ? QBrush(Qt::NoBrush) : QBrush(QColor(c)));
        } else if (t == "pen") {
            const QString c = a[1].toString();
            f.setProperty(key, c.isEmpty() ? QPen(Qt::NoPen) : QPen(QColor(c), a.size() > 2 ? a[2].toDouble() : 1.0));
        } else if (t == "len" && a.size() > 2) {
            f.setProperty(key, QTextLength(QTextLength::Type(a[1].toInt()), a[2].toDouble()));
        } else if (t == "tabs") {
            QList<QVariant> tabs;
            for (const auto &x : a[1].toArray()) {
                const auto ta = x.toArray();
                QTextOption::Tab tab;
                tab.position = ta[0].toDouble();
                tab.type = QTextOption::TabType(ta[1].toInt());
                const QString d = ta[2].toString();
                if (!d.isEmpty()) tab.delimiter = d[0];
                tabs << QVariant::fromValue(tab);
            }
            f.setProperty(key, tabs);
        }
    }
}

QJsonObject storyToJson(const QTextDocument *doc)
{
    QJsonArray blocks;
    QHash<const QTextList *, int> listIds;
    QJsonArray lists;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        QJsonObject bo;
        const QJsonObject bf = formatToJson(b.blockFormat());
        if (!bf.isEmpty()) bo["f"] = bf;
        const QJsonObject cf = formatToJson(b.charFormat());
        if (!cf.isEmpty()) bo["cf"] = cf;
        if (QTextList *l = b.textList()) {
            if (!listIds.contains(l)) {
                listIds[l] = lists.size();
                lists.append(formatToJson(l->format()));
            }
            bo["list"] = listIds[l];
        }
        QJsonArray runs;
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid()) continue;
            QJsonObject r{{"t", frag.text()}};
            const QJsonObject rf = formatToJson(frag.charFormat());
            if (!rf.isEmpty()) r["f"] = rf;
            runs.append(r);
        }
        bo["r"] = runs;
        blocks.append(bo);
    }
    QJsonObject o{{"blocks", blocks}};
    if (!lists.isEmpty()) o["lists"] = lists;
    return o;
}

void storyFromJson(QTextDocument *doc, const QJsonObject &o)
{
    doc->clear();
    QTextCursor c(doc);
    // One edit block: Qt otherwise finishes each run on its own, at a cost
    // that grows with the paragraph (12,000 runs took 40 seconds).
    c.beginEditBlock();
    const QJsonArray blocks = o["blocks"].toArray();
    const QJsonArray lists = o["lists"].toArray();
    QHash<int, QTextList *> made;
    bool first = true;
    for (const auto &bv : blocks) {
        const QJsonObject bo = bv.toObject();
        QTextBlockFormat bf;
        formatFromJson(bf, bo["f"].toObject());
        QTextCharFormat bcf;
        formatFromJson(bcf, bo["cf"].toObject());
        if (first) {
            c.setBlockFormat(bf);
            c.setBlockCharFormat(bcf);
            first = false;
        } else {
            c.insertBlock(bf, bcf);
        }
        if (bo.contains("list")) {
            const int li = bo["list"].toInt();
            if (made.contains(li)) {
                made[li]->add(c.block());
            } else {
                QTextListFormat lf;
                formatFromJson(lf, lists[li].toObject());
                made[li] = c.createList(lf);
            }
        }
        for (const auto &rv : bo["r"].toArray()) {
            const QJsonObject r = rv.toObject();
            QTextCharFormat cf;
            formatFromJson(cf, r["f"].toObject());
            c.insertText(r["t"].toString(), cf);
        }
    }
    c.endEditBlock();
    doc->clearUndoRedoStacks();
    doc->setModified(false);
}

void setStoryText(QTextDocument *doc, const QString &text)
{
    doc->clear();
    QTextCursor c(doc);
    const QStringList paras = text.split('\n');
    for (int i = 0; i < paras.size(); ++i) {
        if (i) c.insertBlock();
        c.insertText(paras[i]);
    }
}

QString storyTextKeepingBreaks(const QTextDocument *doc)
{
    QString out;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        if (b != doc->begin()) out += QLatin1Char('\n');
        out += b.text();
    }
    return out;
}

// What is kept is the text both share at the start and at the end (so the formatting of the runs, links, and lists there stays). When nothing is kept, the
// new text takes the look of the FIRST paragraph and its first run (its paragraph format, its character format, and a list only if that paragraph was in one),
// not of whatever paragraph the removal happens to leave standing: select all, type, and a note that ended in a bullet is not made a bullet.
void replaceStoryText(QTextDocument *doc, const QString &text)
{
    const QString was = storyTextKeepingBreaks(doc);
    const qsizetype most = std::min(was.size(), text.size());
    qsizetype head = 0;
    while (head < most && was.at(head) == text.at(head)) ++head;
    qsizetype tail = 0;
    while (tail < most - head && was.at(was.size() - 1 - tail) == text.at(text.size() - 1 - tail)) ++tail;
    const bool keepsNothing = head == 0 && tail == 0;
    const QTextBlock first = doc->begin();
    const QTextBlockFormat firstBlock = first.blockFormat();
    QTextCharFormat firstChars = first.charFormat();
    if (!first.begin().atEnd()) firstChars = first.begin().fragment().charFormat();
    QTextListFormat firstList;
    const bool firstInList = first.textList();
    if (firstInList) firstList = first.textList()->format();
    QTextCursor c(doc);
    c.setPosition(int(head));
    c.setPosition(int(was.size() - tail), QTextCursor::KeepAnchor);
    c.removeSelectedText();
    if (keepsNothing) {
        if (c.currentList()) c.currentList()->remove(c.block());
        QTextBlockFormat plain = firstBlock;
        plain.setObjectIndex(-1);
        c.setBlockFormat(plain);
        c.setBlockCharFormat(firstChars);
        c.setCharFormat(firstChars);
        if (firstInList) c.createList(firstList);
    }
    const QStringList paras = text.mid(head, text.size() - head - tail).split('\n');
    for (int i = 0; i < paras.size(); ++i) {
        if (i) c.insertBlock();
        if (!paras[i].isEmpty()) c.insertText(paras[i]);
    }
}

} // namespace jp

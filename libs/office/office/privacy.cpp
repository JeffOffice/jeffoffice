#include "office/privacy.h"

#include "io/zip.h"
#include "io/zipindex.h"

#include <QRegularExpression>
#include <QString>
#include <functional>

namespace jp::office {

const char *const kAnonymousAuthor = "Author";

int removePersonalNames(PackageProps &p)
{
    int n = 0;
    for (QString *s : {&p.creator, &p.lastModifiedBy, &p.company, &p.manager})
        if (!s->isEmpty()) {
            s->clear();
            ++n;
        }
    return n;
}

namespace {

constexpr const char *kQuoted = R"((?:"[^"]*"|'[^']*'))";

// The value of the attribute `name` made `with`, in every tag that has it (an empty `with` takes the attribute out).
void setAttribute(QString &xml, const QString &name, const QString &with)
{
    const QRegularExpression re(QStringLiteral("(\\s)%1\\s*=\\s*%2").arg(QRegularExpression::escape(name), QLatin1String(kQuoted)));
    xml.replace(re, with.isEmpty() ? QStringLiteral("\\1") : QStringLiteral("\\1%1=\"%2\"").arg(name, with));
}

// The text of the elements named `qname` made `with`.
void setElementText(QString &xml, const QString &qname, const QString &with)
{
    const QString q = QRegularExpression::escape(qname);
    const QRegularExpression re(QStringLiteral("<%1(\\s[^>]*)?>[^<]*</%1>").arg(q));
    xml.replace(re, QStringLiteral("<%1>%2</%1>").arg(qname, with));
}

void removeElements(QString &xml, const QString &qname)
{
    xml.remove(QRegularExpression(QStringLiteral("<%1(\\s[^>]*)?/>").arg(QRegularExpression::escape(qname))));
}

using Rule = std::function<void(QString &)>;

Rule ruleFor(const QString &name)
{
    const QString n = name.toLower();
    const QString anon = QString::fromLatin1(kAnonymousAuthor);
    if (n == QLatin1String("docprops/core.xml"))
        return [](QString &x) {
            setElementText(x, QStringLiteral("dc:creator"), QString());
            setElementText(x, QStringLiteral("cp:lastModifiedBy"), QString());
        };
    if (n == QLatin1String("docprops/app.xml"))
        return [](QString &x) {
            setElementText(x, QStringLiteral("Company"), QString());
            setElementText(x, QStringLiteral("Manager"), QString());
        };
    if (n == QLatin1String("word/people.xml"))
        return [anon](QString &x) {
            setAttribute(x, QStringLiteral("w15:author"), anon);
            removeElements(x, QStringLiteral("w15:presenceInfo"));
        };
    if (n.startsWith(QLatin1String("word/")) && n.endsWith(QLatin1String(".xml")))
        return [anon](QString &x) {
            setAttribute(x, QStringLiteral("w:author"), anon);
            setAttribute(x, QStringLiteral("w:initials"), QString());
            setAttribute(x, QStringLiteral("w:date"), QString());
        };
    if (n == QLatin1String("ppt/commentauthors.xml"))
        return [anon](QString &x) {
            setAttribute(x, QStringLiteral("name"), anon);
            setAttribute(x, QStringLiteral("initials"), QStringLiteral("A"));
        };
    if (n == QLatin1String("ppt/authors.xml"))
        return [anon](QString &x) {
            setAttribute(x, QStringLiteral("name"), anon);
            setAttribute(x, QStringLiteral("initials"), QStringLiteral("A"));
            setAttribute(x, QStringLiteral("userId"), anon);
            setAttribute(x, QStringLiteral("providerId"), QStringLiteral("None"));
        };
    if (n.startsWith(QLatin1String("xl/comments")) && n.endsWith(QLatin1String(".xml")))
        return [anon](QString &x) { setElementText(x, QStringLiteral("author"), anon); };
    if (n.startsWith(QLatin1String("xl/persons/")) && n.endsWith(QLatin1String(".xml")))
        return [anon](QString &x) {
            setAttribute(x, QStringLiteral("displayName"), anon);
            setAttribute(x, QStringLiteral("userId"), anon);
            setAttribute(x, QStringLiteral("providerId"), QStringLiteral("None"));
        };
    if (n == QLatin1String("xl/workbook.xml"))
        return [](QString &x) {
            setAttribute(x, QStringLiteral("userName"), QString());
            setAttribute(x, QStringLiteral("url"), QString());   // (the path the workbook was last saved at)
        };
    if (n.startsWith(QLatin1String("xl/worksheets/")) && n.endsWith(QLatin1String(".xml")))
        return [](QString &x) {
            // (the user of a scenario)
            if (!x.contains(QLatin1String("<scenario"))) return;
            x.replace(QRegularExpression(QStringLiteral("(<scenario\\s[^>]*?)\\s+user\\s*=\\s*%1").arg(QLatin1String(kQuoted))), QStringLiteral("\\1"));
        };
    if (n == QLatin1String("meta.xml"))
        return [](QString &x) {
            setElementText(x, QStringLiteral("meta:initial-creator"), QString());
            setElementText(x, QStringLiteral("dc:creator"), QString());
        };
    if (n == QLatin1String("content.xml") || n == QLatin1String("styles.xml"))
        return [anon](QString &x) {
            setElementText(x, QStringLiteral("dc:creator"), anon);
            setElementText(x, QStringLiteral("meta:creator-initials"), QString());
        };
    return {};
}

} // namespace

QByteArray removePersonalFromPackage(const QByteArray &zip, int *parts)
{
    if (parts) *parts = 0;
    QVector<ZipEntry> directory;
    if (!readZipDirectory(zip, &directory)) return zip;
    ZipWriter out;
    int changed = 0;
    for (const ZipEntry &e : std::as_const(directory)) {
        const Rule rule = e.name.endsWith(QLatin1Char('/')) ? Rule() : ruleFor(e.name);
        if (rule) {
            QByteArray data;
            // (a part that can't be read is copied as it is, and so is one that is not UTF-8: the names stay in it)
            if (inflateZipEntry(zip, e, &data) && !data.startsWith("\xFF\xFE") && !data.startsWith("\xFE\xFF")) {
                const QString before = QString::fromUtf8(data);
                QString x = before;
                rule(x);
                if (x != before) {
                    out.add(e.name, x.toUtf8(), true);
                    ++changed;
                    continue;
                }
            }
        }
        out.addRaw(e.name, e.method, e.crc, quint32(e.csize), quint32(e.usize), zip.constData() + e.dataAt);
    }
    if (parts) *parts = changed;
    return out.finish();
}

} // namespace jp::office

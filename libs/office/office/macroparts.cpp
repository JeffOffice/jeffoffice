#include "office/macroparts.h"

#include <QJsonArray>
#include <QSet>

namespace jp::office {

namespace {

const QString kProjectRel = QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProject");
const QString kSignatureRel = QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProjectSignature");
const QString kProjectType = QStringLiteral("application/vnd.ms-office.vbaProject");
const QString kSignatureType = QStringLiteral("application/vnd.ms-office.vbaProjectSignature");
constexpr int kMaxTypeLength = 200;
constexpr int kMaxNameLength = 256;

bool nameIsUsable(const QString &name)
{
    if (!name.startsWith(QLatin1Char('/')) || name.size() > kMaxNameLength || name.endsWith(QLatin1Char('/'))) return false;
    for (const QString &segment : name.split(QLatin1Char('/')).mid(1))
        if (segment.isEmpty() || segment == QLatin1String(".") || segment == QLatin1String("..")) return false;
    for (const QChar c : name)
        if (c.unicode() < 0x20 || c == QLatin1Char('\\')) return false;
    return true;
}

} // namespace

bool isMacroContentType(const QString &type)
{
    return type.startsWith(QLatin1String("application/vnd.ms-office.vba")) || type.contains(QLatin1String("macroEnabled")) || type.contains(QLatin1String("vbaData"));
}

bool isMacroRelationshipType(const QString &type)
{
    return type.endsWith(QLatin1String("/vbaProject")) || type.endsWith(QLatin1String("/vbaProjectSignature")) || type.endsWith(QLatin1String("/wordVbaData"));
}

QString macroMainType(MacroProgram program, MacroKind kind, bool macros)
{
    const bool tmpl = kind == MacroKind::Template, show = kind == MacroKind::Show;
    switch (program) {
    case MacroProgram::Document:
        return macros ? tmpl ? QStringLiteral("application/vnd.ms-word.template.macroEnabledTemplate.main+xml") : QStringLiteral("application/vnd.ms-word.document.macroEnabled.main+xml")
               : tmpl ? QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml")
                      : QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml");
    case MacroProgram::Presentation:
        if (macros) return tmpl ? QStringLiteral("application/vnd.ms-powerpoint.template.macroEnabled.main+xml") : show ? QStringLiteral("application/vnd.ms-powerpoint.slideshow.macroEnabled.main+xml") : QStringLiteral("application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml");
        return tmpl ? QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.template.main+xml")
               : show ? QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideshow.main+xml")
                      : QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml");
    case MacroProgram::Workbook:
        if (macros) return tmpl ? QStringLiteral("application/vnd.ms-excel.template.macroEnabled.main+xml") : QStringLiteral("application/vnd.ms-excel.sheet.macroEnabled.main+xml");
        return tmpl ? QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml") : QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml");
    }
    return QString();
}

bool isMacroMainType(const QString &type) { return type.contains(QLatin1String("macroEnabled")); }

MacroProject readMacroProject(const Package &package, const QString &mainPart)
{
    MacroProject out;
    qint64 total = 0;
    QSet<QString> seen;
    enum class Got { Kept, Skipped, OverBound };
    // A part is taken only if it is what the link says: a part of the type the role has, that starts as a compound file does (the form a project and its signature are in). Its
    // size is read from the package's directory and held to the bounds before it is inflated, so a part that claims more than a bound is never inflated.
    auto take = [&](const Relationship &r, const QString &type, bool signature, int from) {
        if (r.external || !nameIsUsable(r.target) || !package.hasPart(r.target) || seen.contains(r.target.toLower())) return Got::Skipped;
        if (package.contentType(r.target).compare(type, Qt::CaseInsensitive) != 0) return Got::Skipped;
        const qint64 size = package.partSize(r.target);
        if (size < 0) return Got::Skipped;
        if (int(out.parts.size()) >= kMaxMacroParts || size > kMaxMacroPartBytes || total + size > kMaxMacroBytes) return Got::OverBound;
        MacroPart part;
        part.data = package.part(r.target);
        if (part.data.size() != size || !part.data.startsWith(QByteArray::fromHex("D0CF11E0"))) return Got::Skipped;
        seen.insert(r.target.toLower());
        total += size;
        part.name = r.target;
        part.contentType = type;
        part.from = from;
        part.signature = signature;
        out.parts.push_back(part);
        return Got::Kept;
    };
    for (const Relationship &r : package.relationshipsOfType(mainPart, QStringLiteral("/vbaProject"))) {
        const int at = int(out.parts.size());
        const Got got = take(r, kProjectType, false, -1);
        if (got == Got::OverBound) return MacroProject{{}, true};
        if (got != Got::Kept) continue;
        for (const Relationship &s : package.relationshipsOfType(r.target, QStringLiteral("/vbaProjectSignature"))) {
            const Got sig = take(s, kSignatureType, true, at);
            if (sig == Got::OverBound) return MacroProject{{}, true};
            if (sig == Got::Kept) break;
        }
    }
    return out;
}

void writeMacroProject(PackageWriter *writer, const QString &mainPart, const MacroProject &project, const QString &folder)
{
    QVector<QString> written;   // the name each part was written under
    int counter = 0;
    for (int i = 0; i < int(project.parts.size()) && i < kMaxMacroParts; ++i) {
        const MacroPart &p = project.parts[i];
        const int from = p.from;
        if (from >= i || (from >= 0 && written[from].isEmpty())) {
            written.push_back(QString());
            continue;
        }
        QString name = p.name;
        while (!nameIsUsable(name) || writer->hasPart(name) || name.compare(mainPart, Qt::CaseInsensitive) == 0) {
            ++counter;
            name = folder + (p.signature ? QStringLiteral("vbaProjectSignature") : QStringLiteral("vbaProject")) + (counter > 1 ? QString::number(counter) : QString()) + QStringLiteral(".bin");
        }
        writer->addPart(name, p.data, p.contentType);
        writer->addRelationship(from < 0 ? mainPart : written[from], p.signature ? kSignatureRel : kProjectRel, name);
        written.push_back(name);
    }
}

QJsonObject macroProjectToJson(const MacroProject &project)
{
    QJsonArray list;
    for (const MacroPart &p : project.parts)
        list.append(QJsonObject{{"name", p.name}, {"type", p.contentType}, {"from", p.from}, {"signature", p.signature}, {"data", QString::fromLatin1(p.data.toBase64())}});
    return QJsonObject{{"parts", list}};
}

MacroProject macroProjectFromJson(const QJsonObject &json)
{
    MacroProject out;
    const QJsonArray list = json.value(QStringLiteral("parts")).toArray();
    if (list.size() > kMaxMacroParts) return out;
    qint64 total = 0;
    for (int i = 0; i < int(list.size()); ++i) {
        const QJsonObject o = list[i].toObject();
        MacroPart p;
        p.name = o.value(QStringLiteral("name")).toString();
        p.contentType = o.value(QStringLiteral("type")).toString();
        p.from = o.value(QStringLiteral("from")).toInt(-1);
        p.signature = o.value(QStringLiteral("signature")).toBool();
        const QString text = o.value(QStringLiteral("data")).toString();
        if (text.size() > (kMaxMacroPartBytes / 3 + 1) * 4) return MacroProject();
        p.data = QByteArray::fromBase64(text.toLatin1());
        total += p.data.size();
        if (!nameIsUsable(p.name) || p.contentType.isEmpty() || p.contentType.size() > kMaxTypeLength || p.from < -1 || p.from >= i || p.data.size() > kMaxMacroPartBytes || total > kMaxMacroBytes) return MacroProject();
        out.parts.push_back(p);
    }
    return out;
}

} // namespace jp::office

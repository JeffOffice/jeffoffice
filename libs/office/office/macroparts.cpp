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
    auto tooMuch = [&](const QByteArray &data) {
        total += data.size();
        return int(out.parts.size()) >= kMaxMacroParts || data.size() > kMaxMacroPartBytes || total > kMaxMacroBytes;
    };
    auto typeOf = [&](const QString &name, const QString &canonical) {
        const QString t = package.contentType(name);
        return !t.isEmpty() && t.size() <= kMaxTypeLength && isMacroContentType(t) ? t : canonical;
    };
    QSet<QString> seen;
    for (const Relationship &r : package.relationshipsOfType(mainPart, QStringLiteral("/vbaProject"))) {
        if (r.external || !package.hasPart(r.target) || seen.contains(r.target.toLower())) continue;
        seen.insert(r.target.toLower());
        MacroPart project;
        project.name = r.target;
        project.data = package.part(r.target);
        if (!nameIsUsable(project.name) || tooMuch(project.data)) return MacroProject{{}, true};
        project.contentType = typeOf(r.target, kProjectType);
        const int at = int(out.parts.size());
        out.parts.push_back(project);
        for (const Relationship &s : package.relationshipsOfType(r.target, QStringLiteral("/vbaProjectSignature"))) {
            if (s.external || !package.hasPart(s.target) || seen.contains(s.target.toLower())) continue;
            seen.insert(s.target.toLower());
            MacroPart sig;
            sig.name = s.target;
            sig.data = package.part(s.target);
            if (!nameIsUsable(sig.name) || tooMuch(sig.data)) return MacroProject{{}, true};
            sig.contentType = typeOf(s.target, kSignatureType);
            sig.from = at;
            sig.signature = true;
            out.parts.push_back(sig);
            break;
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
        while (!nameIsUsable(name) || !writer->partData(name).isEmpty() || name.compare(mainPart, Qt::CaseInsensitive) == 0) {
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

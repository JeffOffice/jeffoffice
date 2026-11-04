// The file's properties: the parts of docProps, kept as the file has them, with the person's edits put in.
#include "core/writedoc.h"

#include <QTextBlock>
#include <functional>

namespace jp::write {

using office::Relationship;

namespace {

const QString kCoreName = QStringLiteral("/docProps/core.xml");
const QString kAppName = QStringLiteral("/docProps/app.xml");
const QString kCustomName = QStringLiteral("/docProps/custom.xml");

QString plainTarget(const QString &target)
{
    QString t = target;
    while (t.startsWith(QLatin1Char('/')) || t.startsWith(QLatin1String("../")) || t.startsWith(QLatin1String("./"))) t.remove(0, t.startsWith(QLatin1Char('/')) ? 1 : t.startsWith(QLatin1String("../")) ? 3 : 2);
    return t.toLower();
}

} // namespace

// The part the package's relationship of this type names (the part's name as the file has it), "" if there is none.
QString WriteDoc::propsPartName(const QString &typeSuffix) const
{
    for (const Relationship &r : m_wml.packageRelationships)
        if (!r.external && r.type.endsWith(typeSuffix)) {
            QString name = r.target;
            if (!name.startsWith(QLatin1Char('/'))) name.prepend(QLatin1Char('/'));
            return name;
        }
    return {};
}

QByteArray WriteDoc::propsPartData(const QString &typeSuffix) const
{
    const QString name = propsPartName(typeSuffix);
    return name.isEmpty() ? QByteArray() : partData(name);
}

office::PackageProps WriteDoc::packageProps() const
{
    return office::readPackageProps(propsPartData(QStringLiteral("/core-properties")), propsPartData(QStringLiteral("/extended-properties")), propsPartData(QStringLiteral("/custom-properties")));
}

bool WriteDoc::markedAsFinal() const { return office::isMarkedFinal(packageProps().custom); }

void WriteDoc::putPackagePart(const QString &name, const QString &contentType, const QByteArray &data, const QString &relationshipType)
{
    bool found = false;
    for (office::wml::KeptPart &part : m_wml.parts)
        if (part.name.compare(name, Qt::CaseInsensitive) == 0) {
            part.data = data;
            found = true;
            break;
        }
    if (!found) m_wml.parts.push_back({name, contentType, data});
    if (relationshipType.isEmpty()) return;
    for (const Relationship &r : std::as_const(m_wml.packageRelationships))
        if (!r.external && (plainTarget(r.target) == plainTarget(name) || r.type == relationshipType)) return;
    m_wml.packageRelationships.push_back({nextRelationshipId(m_wml.packageRelationships), relationshipType, name, false});
}

bool WriteDoc::removePackagePart(const QString &name)
{
    bool found = false;
    for (qsizetype i = m_wml.parts.size() - 1; i >= 0; --i)
        if (m_wml.parts[i].name.compare(name, Qt::CaseInsensitive) == 0) {
            m_wml.parts.removeAt(i);
            found = true;
        }
    auto drop = [&](QVector<Relationship> &rels) {
        for (qsizetype i = rels.size() - 1; i >= 0; --i)
            if (!rels[i].external && plainTarget(rels[i].target) == plainTarget(name)) rels.removeAt(i);
    };
    drop(m_wml.packageRelationships);
    drop(m_wml.mainRelationships);
    m_wml.partRelationships.remove(name);
    return found;
}

QStringList WriteDoc::packagePartNames() const
{
    QStringList names;
    for (const office::wml::KeptPart &part : m_wml.parts) names << part.name;
    return names;
}

void WriteDoc::setPackageProps(const office::PackageProps &props)
{
    // (a part the file has too much in to be read through is left as it is, and so are the properties)
    if (props.truncated) return;
    // The parts are the ones the package's relationships name, whatever their paths are; a file that has none gets them at the usual places.
    auto put = [this](const QString &suffix, const QString &usual, const char *type, const char *rel, const std::function<QByteArray(const QByteArray &)> &make) {
        const QString name = propsPartName(suffix).isEmpty() ? usual : propsPartName(suffix);
        const QByteArray old = partData(name);
        const QByteArray now = make(old);
        if (now.isEmpty()) removePackagePart(name);
        else if (now != old) putPackagePart(name, QString::fromLatin1(type), now, QString::fromLatin1(rel));
        else if (propsPartName(suffix).isEmpty()) putPackagePart(name, QString::fromLatin1(type), now, QString::fromLatin1(rel));
    };
    put(QStringLiteral("/core-properties"), kCoreName, office::kCorePartType, office::kCoreRelationship, [&](const QByteArray &old) { return office::writeCoreProps(old, props); });
    put(QStringLiteral("/extended-properties"), kAppName, office::kAppPartType, office::kAppRelationship, [&](const QByteArray &old) { return office::writeAppProps(old, props); });
    // (the custom part is written only when the list changed, and goes when the last property does)
    const office::PackageProps before = packageProps();
    if (!(before.custom == props.custom))
        put(QStringLiteral("/custom-properties"), kCustomName, office::kCustomPartType, office::kCustomRelationship,
            [&](const QByteArray &old) { return props.custom.isEmpty() ? QByteArray() : office::writeCustomProps(old, props); });
    setModified(true);
}

WriteDoc::TextCounts WriteDoc::textCounts() const
{
    TextCounts c;
    QString text = story()->toPlainText();
    text.remove(QChar::ObjectReplacementCharacter);
    bool in = false;
    for (const QChar ch : text) {
        if (ch == QLatin1Char('\n') || ch == QChar::ParagraphSeparator) {
            ++c.paragraphs;
            in = false;
            continue;
        }
        ++c.charactersWithSpaces;
        if (ch.isSpace()) {
            in = false;
        } else {
            ++c.characters;
            if (!in) ++c.words;
            in = true;
        }
    }
    ++c.paragraphs;
    return c;
}

} // namespace jp::write

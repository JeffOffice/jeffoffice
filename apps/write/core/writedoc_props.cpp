// The file's properties: the parts of docProps, kept as the file has them, with the person's edits put in.
#include "core/writedoc.h"

#include <QTextBlock>

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

office::PackageProps WriteDoc::packageProps() const
{
    return office::readPackageProps(partData(kCoreName), partData(kAppName), partData(kCustomName));
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
        if (!r.external && plainTarget(r.target) == plainTarget(name)) return;
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
    putPackagePart(kCoreName, QString::fromLatin1(office::kCorePartType), office::writeCoreProps(partData(kCoreName), props), QString::fromLatin1(office::kCoreRelationship));
    putPackagePart(kAppName, QString::fromLatin1(office::kAppPartType), office::writeAppProps(partData(kAppName), props), QString::fromLatin1(office::kAppRelationship));
    // (the custom part is made when there is something for it, and taken out when the last property goes)
    if (!props.custom.isEmpty()) putPackagePart(kCustomName, QString::fromLatin1(office::kCustomPartType), office::writeCustomProps(props), QString::fromLatin1(office::kCustomRelationship));
    else removePackagePart(kCustomName);
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

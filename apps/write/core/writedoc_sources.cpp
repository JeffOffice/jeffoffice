// The sources of the bibliography as a part of the document: a custom XML part whose root is the bibliography's own (read from the file, kept as it is, and written when the sources are changed).

#include "core/writedoc.h"

#include <QUuid>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {
constexpr char kBibliographyNs[] = "officeDocument/2006/bibliography";
constexpr char kCustomXmlRel[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXml";
constexpr char kCustomXmlPropsRel[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXmlProps";
constexpr char kCustomXmlPropsType[] = "application/vnd.openxmlformats-officedocument.customXmlProperties+xml";
}

QByteArray WriteDoc::sourcesXml()
{
    if (m_sourcesRead) return m_sourcesXml;
    m_sourcesRead = true;
    for (const KeptPart &p : std::as_const(m_wml.parts)) {
        if (!p.name.startsWith(QLatin1String("/customXml/"), Qt::CaseInsensitive) || p.name.contains(QLatin1String("Props"), Qt::CaseInsensitive)) continue;
        const QByteArray head = p.data.left(2048);
        if (head.contains(kBibliographyNs) && head.contains("Sources")) {
            m_sourcesXml = p.data;
            m_sourcesPartName = p.name;
            break;
        }
    }
    return m_sourcesXml;
}

void WriteDoc::setSourcesXml(const QByteArray &xml)
{
    sourcesXml();
    if (xml == m_sourcesXml) return;
    m_sourcesXml = xml;
    m_sourcesDirty = true;
    setModified(true);
}

void WriteDoc::writeSourcesPart(office::wml::Document *out)
{
    if (!m_sourcesDirty) return;
    if (!m_sourcesPartName.isEmpty()) {
        for (KeptPart &kept : out->parts)
            if (kept.name.compare(m_sourcesPartName, Qt::CaseInsensitive) == 0) {
                kept.data = m_sourcesXml;
                return;
            }
    }
    if (m_sourcesXml.isEmpty()) return;
    int n = 1;
    while (partExists(QStringLiteral("/customXml/item%1.xml").arg(n)) || partExists(QStringLiteral("/customXml/itemProps%1.xml").arg(n))) ++n;
    KeptPart item;
    item.name = QStringLiteral("/customXml/item%1.xml").arg(n);
    item.contentType = QStringLiteral("application/xml");
    item.data = m_sourcesXml;
    KeptPart props;
    props.name = QStringLiteral("/customXml/itemProps%1.xml").arg(n);
    props.contentType = QString(kCustomXmlPropsType);
    props.data = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\r\n<ds:datastoreItem ds:itemID=\"" + QUuid::createUuid().toString(QUuid::WithBraces).toUpper().toUtf8() +
                 "\" xmlns:ds=\"http://schemas.openxmlformats.org/officeDocument/2006/customXml\"><ds:schemaRefs><ds:schemaRef ds:uri=\"http://schemas.openxmlformats.org/officeDocument/2006/bibliography\"/>"
                 "</ds:schemaRefs></ds:datastoreItem>";
    out->parts.push_back(item);
    out->parts.push_back(props);
    Relationship toItem;
    toItem.id = nextRelationshipId(out->mainRelationships);
    toItem.type = QString(kCustomXmlRel);
    toItem.target = item.name;
    out->mainRelationships.push_back(toItem);
    Relationship toProps;
    toProps.id = QStringLiteral("rId1");
    toProps.type = QString(kCustomXmlPropsRel);
    toProps.target = props.name;
    out->partRelationships[item.name].push_back(toProps);
    m_sourcesPartName = item.name;
}

} // namespace jp::write

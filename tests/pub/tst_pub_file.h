#pragma once
// JeffPub's tests of reading and writing .pub files, and templates.
#include "../plainprobe.h"
#include "pubhelpers.h"
#include <libmspub/jp_hooks.h>
#include <private/qabstractfileengine_p.h>

// Rebuilds a .pub's drawing records (Escher/EscherStm) with the first
// shape properties table (OPT) that `edit` changes: `edit` gets its body
// and header and returns true once it has changed them. Container lengths
// are recomputed; padding between drawings is kept.
static QByteArray editFirstOpt(const QByteArray &escher, const std::function<bool(QByteArray &body, quint16 &head)> &edit)
{
    bool done = false;
    std::function<QByteArray(int, int)> rebuild = [&](int from, int to) {
        QByteArray out;
        int off = from;
        while (off + 8 <= to) {
            const quint16 vi = qFromLittleEndian<quint16>(escher.constData() + off);
            const quint16 type = qFromLittleEndian<quint16>(escher.constData() + off + 2);
            const quint32 len = qFromLittleEndian<quint32>(escher.constData() + off + 4);
            if (type < 0xF000 || type > 0xF200 || off + 8 + qint64(len) > to) {   // padding between drawings
                out += escher.at(off++);
                continue;
            }
            QByteArray body = escher.mid(off + 8, len);
            quint16 head = vi;
            if ((vi & 0xF) == 0xF) body = rebuild(off + 8, off + 8 + int(len));
            else if (type == 0xF00B && !done) done = edit(body, head);
            QByteArray h(8, 0);
            qToLittleEndian<quint16>(head, h.data());
            qToLittleEndian<quint16>(type, h.data() + 2);
            qToLittleEndian<quint32>(quint32(body.size()), h.data() + 4);
            out += h + body;
            off += 8 + int(len);
        }
        out += escher.mid(off, to - off);
        return out;
    };
    const QByteArray out = rebuild(0, int(escher.size()));
    return done ? out : QByteArray();
}

// Adds a property to an OPT body (after the others, its complex data last).
static void addOptProp(QByteArray &body, quint16 &head, quint16 id, quint32 value, const QByteArray &complex = {})
{
    const int count = head >> 4;
    QByteArray entry(6, 0);
    qToLittleEndian<quint16>(id, entry.data());
    qToLittleEndian<quint32>(complex.isEmpty() ? value : quint32(complex.size()), entry.data() + 2);
    body = body.left(count * 6) + entry + body.mid(count * 6) + complex;
    head = quint16((head & 0xF) | ((count + 1) << 4));
}

// Where a complex property's data starts in an OPT body, or -1.
static int optComplexAt(const QByteArray &body, quint16 head, quint16 want)
{
    const int count = head >> 4;
    int at = count * 6;
    for (int i = 0; i < count; ++i) {
        const quint16 id = qFromLittleEndian<quint16>(body.constData() + i * 6);
        const quint32 v = qFromLittleEndian<quint32>(body.constData() + i * 6 + 2);
        if ((id & 0x3FFF) == want) return at;
        if (id & 0x8000) at += int(v);
    }
    return -1;
}


class PubFileTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void publisherSamplesImport_data();
    void publisherSamplesImport();
    void pubWriterKeepsJpegPhotos();
    void firstPageNumberLoadsWithinTheLimits();
    void damagedStyleNamesStillOpen();
    void jpubHostileContent();
    void jpubPicturesAreHeldToThePictureChecks();
    void publisherFuzzFilesDoNotCrash();
    void privateCorpus();
    void templateOptions();
    void changeTemplate();
    void hostilePublicationTextIsShownAsText();
    void textFileImportNeverReadsThePicturesItNames();
    void changeTemplateGallery();
    void templatesFitTheirText();
    void pubTextIndexChains();
    void pub2000ScratchIsNotAPage();
    void pubAutofitSetting();
    void pubWriterRoundTrip();
    void pubWriterTextArt();
    void pubWriterShapeText();
    void pubWriterShapes();
    void pubWriterFills();
    void pubWriterPictures();
    void pubWriterLinked();
    void pubWriterTextFormats();
    void pubWriterTabsLists();
    void pubWriterMaster();
    void pubWriterHyphenation();
    void pubWriterStyles();
    void pubWriterTwoMasters();
    void pubWriterPages();
    void pubWriterTable();
    void pubLineWeightsAndListIndentsDoNotWrap();
    void pubWriterObjects();
    void pubInkPacking();
    void pubTextScaleRead();
    void pubWriterSpotAndScaling();
    void pubPreviewAndLanguages();
    void pubRightToLeft();
    void pubDashStylesRoundTrip();
    void pubPreviewThumbnail();
    void pubSpotColorRead();
    void pubWriterProcessInks();
    void pubGradientsRoundTrip();
    void pubPictureTurnsAndFill();
    void pubFormulaChainIsFast();
    void pubHugeCoordinateSpace();
    void pubShadowFromLineColor();
    void pubLineCornersRoundTrip();
    void pubOutlineKeepsFrame();
    void pubGradientAnglesAsPublisher();
    void pubStraightLineIsALine();
    void pubTextWithoutSizeIs12();
    void pubSuperscriptHighByte();
    void pubLinkKeepsLinkColor();
    void pubNoneNumberingIsNoList();
    void colorRefRoundTrip();
    void documentRoundTrip();
    void layoutFlowsAcrossFrames();
    void fieldsDisplay();
    void renderSmoke();
};

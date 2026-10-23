// Loads fonts the way a program does, for the test of which fonts win (tests/pub/tst_pub_fonts.cpp,
// bundledFontsWinWhereverTheyAreInstalled):
//   jpfonttest <folder> <font file name>...
// Each named file of the folder first gets a twin, as a font the computer already had would: another
// build of the same font (its revision changed), added from memory before the program's own fonts are.
// Then the folder is loaded as the program's own fonts. For each name, prints "<name> yes" when the
// face the text engine draws its family and style with is the file's own, "<name> no" when it is the twin.

#include "core/fonts.h"
#include "core/sfnt.h"

#include <QDir>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHash>
#include <QRawFont>

#include <cstdio>
#include <cstring>

namespace {

// A font's head table, where it starts in the file (-1 when it has none), and its 54 bytes: they hold the revision, so two builds of a font differ there.
QByteArray headTable(const QByteArray &font, qsizetype *at = nullptr)
{
    if (at) *at = -1;
    if (font.size() < 12) return {};
    const uchar *p = reinterpret_cast<const uchar *>(font.constData());
    const int tables = (p[4] << 8) | p[5];
    for (int i = 0; i < tables && 12 + 16 * (i + 1) <= font.size(); ++i) {
        const uchar *entry = p + 12 + 16 * i;
        if (std::memcmp(entry, "head", 4) != 0) continue;
        const qsizetype offset = (qsizetype(entry[8]) << 24) | (entry[9] << 16) | (entry[10] << 8) | entry[11];
        if (at) *at = offset;
        return font.mid(offset, 54);
    }
    return {};
}

QByteArray readAll(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

} // namespace

int main(int argc, char **argv)
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);
    if (argc < 3) return 2;
    const QDir dir(QString::fromLocal8Bit(argv[1]));
    QHash<QString, QByteArray> own;
    for (int i = 2; i < argc; ++i) {
        const QString name = QString::fromLocal8Bit(argv[i]);
        QByteArray twin = readAll(dir.filePath(name));
        qsizetype head = -1;
        own.insert(name, headTable(twin, &head));
        if (head < 0) return 3;
        twin[head + 4] = char(twin[head + 4] ^ 0x55);
        if (QFontDatabase::addApplicationFontFromData(twin) < 0) return 4;
    }
    if (jp::loadFontsFrom({dir.path()}) == 0) return 5;
    for (int i = 2; i < argc; ++i) {
        const QString name = QString::fromLocal8Bit(argv[i]);
        const QByteArray font = readAll(dir.filePath(name));
        jp::FontFileInfo info;
        if (!jp::inspectFont(font, &info)) return 6;
        QFont f(info.family);
        f.setBold(info.bold);
        f.setItalic(info.italic);
        f.setPixelSize(24);
        const bool ours = QRawFont::fromFont(f).fontTable("head") == own.value(name);
        std::printf("%s %s\n", name.toLatin1().constData(), ours ? "yes" : "no");
    }
    return 0;
}

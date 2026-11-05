// Speed bench helper (not part of the product): opens a workbook, reads every sheet in full, writes it back
// as .xlsx, and prints the phases in milliseconds. Built by speedbench.py with the flags of jcrunchtool.
//   xlsxsave_bench <in.xlsx> <out.xlsx>

#include "office/package.h"
#include "sheet/sheetreader.h"
#include "sheet/xlsxsave.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <cstdio>

using namespace jp::office;
using namespace jp::office::sheet;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if (argc < 3) return 2;
    QElapsedTimer t;
    t.start();
    Workbook book;
    QString why;
    if (!book.openFile(QString::fromLocal8Bit(argv[1]), &why)) {
        fprintf(stderr, "open failed: %s\n", qPrintable(why));
        return 1;
    }
    const qint64 opened = t.elapsed();
    qint64 cells = 0;
    for (int i = 0; i < book.info().sheets.size(); ++i) {
        Sheet s;
        if (book.readSheet(i, &s)) cells += s.cells.cellCount();
    }
    const qint64 read = t.elapsed();
    QByteArray bytes;
    if (!saveXlsx(book, SaveChanges(), &bytes, &why)) {
        fprintf(stderr, "save failed: %s\n", qPrintable(why));
        return 3;
    }
    const qint64 saved = t.elapsed();
    QFile f(QString::fromLocal8Bit(argv[2]));
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) return 4;
    f.close();
    printf("phases_ms\topen=%lld\tread=%lld\tsave=%lld\tcells=%lld\n", opened, read - opened, saved - read, cells);
    return 0;
}

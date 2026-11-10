#pragma once
// The PDFs Qt writes, opened for changes after the fact (spot colors,
// PDF/X): objects in file order (Qt writes no object streams or incremental
// updates), their stream data, and a fresh cross-reference table on saving.

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>

namespace jp {

class QtPdf {
public:
    struct Obj {
        int id = 0;
        QByteArray body;   // between "N 0 obj\n" and "endobj\n"
    };

    bool load(const QString &path, QString *error = nullptr);
    bool loadBytes(const QByteArray &pdf, QString *error = nullptr);
    bool save(const QString &path, QString *error = nullptr) const;
    QByteArray toBytes() const;   // the file, as save() writes it

    QByteArray header;    // the version line and the binary marker line
    QVector<Obj> objects;
    QByteArray trailer;   // "trailer\n<< ... >>\n"

    Obj *object(int id);
    int addObject(const QByteArray &body);

    // A stream's dictionary (the part before "stream"), or the whole body.
    static QByteArray dictOf(const QByteArray &body);
    static bool isStream(const QByteArray &body);
    // The stream's data, inflated when Flate-compressed.
    QByteArray streamData(const Obj &o, bool *ok) const;
    // Replaces the data, written as given; `dict` is the new dictionary
    // without /Length, which is set here.
    void setStream(Obj &o, const QByteArray &dict, const QByteArray &data);
    // N for "/Key N 0 R" in a dictionary, or 0.
    static int ref(const QByteArray &dict, const QByteArray &key);

    static QByteArray inflate(const QByteArray &in, bool *ok);
    static QByteArray deflate(const QByteArray &in);

private:
    // Where each object is in `objects`, found once (so that looking one up, or adding one, costs the same however many there are); made again when `objects` was changed by hand.
    void index();
    QHash<int, int> m_at;
    int m_indexed = -1, m_maxId = 0;
};

} // namespace jp

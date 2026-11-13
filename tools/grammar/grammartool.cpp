// Measures the grammar rules on real text: reads files of paragraphs (UTF-8, one paragraph a line), runs every rule over them, and writes, for each rule, the number of
// flags, the flags per 10,000 words, and a random sample of the flagged contexts (a few words round each) for review. Used by tools/grammar/measure.py.
// Usage: grammartool [--language en|fr|de|es] [--sample N] [--seed S] [--rule NAME] FILE...
#include "grammar/grammarrules.h"

#include <QCoreApplication>
#include <QFile>
#include <QMap>
#include <QRandomGenerator>
#include <QTextStream>

using namespace jp;

namespace {

struct Seen {
    qint64 flags = 0;
    QVector<QString> sample;
};

qint64 countWords(const QString &t)
{
    qint64 n = 0;
    bool in = false;
    for (QChar c : t) {
        const bool letter = c.isLetterOrNumber();
        if (letter && !in) ++n;
        in = letter || (in && (c == QLatin1Char('\'') || c == QChar(0x2019) || c == QLatin1Char('-')));
    }
    return n;
}

} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    int sampleSize = 20;
    quint32 seed = 1;
    QString only;
    grammar::Language language = grammar::Language::English;
    QStringList files;
    const QStringList args = app.arguments().mid(1);
    for (int i = 0; i < args.size(); ++i) {
        if (args[i] == QLatin1String("--sample") && i + 1 < args.size()) sampleSize = args[++i].toInt();
        else if (args[i] == QLatin1String("--seed") && i + 1 < args.size()) seed = args[++i].toUInt();
        else if (args[i] == QLatin1String("--rule") && i + 1 < args.size()) only = args[++i];
        else if (args[i] == QLatin1String("--language") && i + 1 < args.size()) {
            const QString l = args[++i];
            language = l == QLatin1String("fr") ? grammar::Language::French : l == QLatin1String("de") ? grammar::Language::German : l == QLatin1String("es") ? grammar::Language::Spanish : grammar::Language::English;
        }
        else files << args[i];
    }
    if (files.isEmpty()) {
        QTextStream(stderr) << "usage: grammartool [--language en|fr|de|es] [--sample N] [--seed S] [--rule NAME] FILE...\n";
        return 2;
    }
    QRandomGenerator rng(seed);
    QMap<QString, Seen> seen;
    for (const auto &r : grammar::rules(language)) seen[r.name];
    qint64 words = 0, paragraphs = 0;
    for (const QString &path : files) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) continue;
        while (!f.atEnd()) {
            const QString line = QString::fromUtf8(f.readLine(1 << 20)).trimmed();
            if (line.isEmpty()) continue;
            ++paragraphs;
            words += countWords(line);
            for (const GrammarIssue &g : grammar::check(line, nullptr, grammar::kAllKinds, language)) {
                if (!only.isEmpty() && g.rule != only) continue;
                Seen &s = seen[g.rule];
                ++s.flags;
                QString ctx = line.mid(qMax(0, g.start - 50), g.start - qMax(0, g.start - 50)) + QStringLiteral("[[") + line.mid(g.start, g.end - g.start) + QStringLiteral("]]") +
                              line.mid(g.end, 50);
                ctx += QStringLiteral("  => ") + (g.suggestions.isEmpty() ? QStringLiteral("(none)") : QStringLiteral("\"") + g.suggestions.first() + QStringLiteral("\""));
                if (s.sample.size() < sampleSize) s.sample << ctx;
                else if (const qint64 j = qint64(rng.bounded(quint32(qMin<qint64>(s.flags, 0x7FFFFFFF)))); j < sampleSize) s.sample[int(j)] = ctx;   // (reservoir sampling)
            }
        }
    }
    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);
    out << "words\t" << words << "\nparagraphs\t" << paragraphs << "\n";
    for (auto it = seen.cbegin(); it != seen.cend(); ++it) {
        out << "rule\t" << it.key() << "\t" << it->flags << "\t" << QString::number(words ? double(it->flags) * 10000.0 / double(words) : 0.0, 'f', 3) << "\n";
        for (const QString &s : it->sample) out << "sample\t" << it.key() << "\t" << QString(s).replace(QLatin1Char('\t'), QLatin1Char(' ')) << "\n";
    }
    return 0;
}

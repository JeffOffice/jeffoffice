// The system's voices, through Qt TextToSpeech (Windows' own speech, the Mac's, and speech-dispatcher on Linux). Built only where the module is (CMake sets JO_TTS).
#ifdef JO_TTS
#include "app/readaloud.h"

#include <QLibrary>
#include <QLocale>
#include <QTextToSpeech>
#include <QVoice>

#include <utility>

namespace jp {

namespace {

// The engine to ask Qt for, chosen here so that no plug-in is loaded that can't work (Qt prints a warning for each that fails): on Linux the speech-dispatcher plug-in only when its
// library can be loaded, else flite when it is there; elsewhere the system's own. Empty: none.
QString chooseEngine()
{
    const QStringList engines = QTextToSpeech::availableEngines();
#ifdef Q_OS_LINUX
    if (engines.contains(QLatin1String("speechd")) && !speechdMissingForTest() && QLibrary(QStringLiteral("speechd"), 2).load()) return QStringLiteral("speechd");
    if (engines.contains(QLatin1String("flite"))) return QStringLiteral("flite");
    return QString();
#else
    for (const QString &e : engines)
        if (e != QLatin1String("mock")) return e;
    return QString();
#endif
}

class SystemSpeechEngine : public SpeechEngine {
public:
    explicit SystemSpeechEngine(const QString &engine) : m_tts(new QTextToSpeech(engine, this))
    {
        connect(m_tts, &QTextToSpeech::sayingWord, this, [this](const QString &, qsizetype id, qsizetype start, qsizetype length) {
            if (id == m_id) Q_EMIT wordSpoken(m_token, int(start), int(length));
        });
        connect(m_tts, &QTextToSpeech::stateChanged, this, [this](QTextToSpeech::State state) {
            if (m_token == 0) return;
            if (state == QTextToSpeech::Error) {
                // The speech service failed: speech has ended, and the person is told why.
                const int token = std::exchange(m_token, 0);
                m_spoke = false;
                Q_EMIT errorOccurred(token, m_tts->errorString());
            } else if (state == QTextToSpeech::Speaking) {
                m_spoke = true;
            } else if (state == QTextToSpeech::Ready && m_spoke && !m_stopping) {
                // Speaking, then ready again, without a stop of ours: the text was spoken to its end.
                m_spoke = false;
                Q_EMIT finished(m_token);
            }
        });
    }
    // Listed when first asked for (making the list sets the engine to each locale in turn, which is not done when a program starts).
    QList<SpeechVoice> voices() const override
    {
        if (!m_listed) {
            m_listed = true;
            const QLocale first = m_tts->locale();
            for (const QLocale &l : m_tts->availableLocales()) {
                m_tts->setLocale(l);
                for (const QVoice &v : m_tts->availableVoices()) m_voices.push_back(v);
            }
            m_tts->setLocale(first);
        }
        QList<SpeechVoice> out;
        for (const QVoice &v : std::as_const(m_voices)) out.push_back({idOf(v), v.name(), v.locale().name()});
        return out;
    }
    void setVoice(const QString &id) override
    {
        voices();
        for (const QVoice &v : std::as_const(m_voices))
            if (idOf(v) == id) {
                m_tts->setLocale(v.locale());
                m_tts->setVoice(v);
                return;
            }
    }
    void setRate(double rate) override { m_tts->setRate(rate); }
    int say(const QString &text) override
    {
        m_stopping = false;
        m_spoke = false;
        m_token = ++m_serial;
        m_id = m_tts->enqueue(text);
        return m_token;
    }
    void pause() override { m_tts->pause(); }
    void resume() override { m_tts->resume(); }
    void stop() override
    {
        m_stopping = true;
        m_spoke = false;
        m_token = 0;
        m_tts->stop();
    }

private:
    static QString idOf(const QVoice &v) { return v.name() + QLatin1Char('|') + v.locale().name(); }
    QTextToSpeech *m_tts;
    mutable QList<QVoice> m_voices;
    mutable bool m_listed = false;
    bool m_spoke = false, m_stopping = false;
    int m_token = 0, m_serial = 0;
    qsizetype m_id = -1;
};

} // namespace

SpeechEngine *makeSystemSpeechEngine()
{
    const QString engine = chooseEngine();
    return engine.isEmpty() ? nullptr : new SystemSpeechEngine(engine);
}

} // namespace jp
#endif

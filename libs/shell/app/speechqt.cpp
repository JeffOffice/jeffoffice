// The system's voices, through Qt TextToSpeech (Windows' own speech, the Mac's, and speech-dispatcher on Linux). Built only where the module is (CMake sets JO_TTS).
#ifdef JO_TTS
#include "app/readaloud.h"

#include <QLocale>
#include <QTextToSpeech>
#include <QVoice>

namespace jp {

namespace {

class SystemSpeechEngine : public SpeechEngine {
public:
    SystemSpeechEngine() : m_tts(new QTextToSpeech(this))
    {
        // (a voice is listed for the locale the engine is set to: every locale is asked, and the first is put back)
        const QLocale first = m_tts->locale();
        for (const QLocale &l : m_tts->availableLocales()) {
            m_tts->setLocale(l);
            for (const QVoice &v : m_tts->availableVoices()) m_voices.push_back(v);
        }
        m_tts->setLocale(first);
        connect(m_tts, &QTextToSpeech::sayingWord, this, [this](const QString &, qsizetype, qsizetype start, qsizetype length) { Q_EMIT wordSpoken(int(start), int(length)); });
        connect(m_tts, &QTextToSpeech::stateChanged, this, [this](QTextToSpeech::State state) {
            // Speaking, then ready again, without a stop of ours: the text was spoken to its end.
            if (state == QTextToSpeech::Speaking) m_spoke = true;
            else if (state == QTextToSpeech::Ready && m_spoke && !m_stopping) {
                m_spoke = false;
                Q_EMIT finished();
            }
        });
    }
    QList<SpeechVoice> voices() const override
    {
        QList<SpeechVoice> out;
        for (const QVoice &v : m_voices) out.push_back({idOf(v), v.name(), v.locale().name()});
        return out;
    }
    void setVoice(const QString &id) override
    {
        for (const QVoice &v : m_voices)
            if (idOf(v) == id) {
                m_tts->setLocale(v.locale());
                m_tts->setVoice(v);
                return;
            }
    }
    void setRate(double rate) override { m_tts->setRate(rate); }
    void say(const QString &text) override
    {
        m_stopping = false;
        m_spoke = false;
        m_tts->say(text);
    }
    void pause() override { m_tts->pause(); }
    void resume() override { m_tts->resume(); }
    void stop() override
    {
        m_stopping = true;
        m_spoke = false;
        m_tts->stop();
    }

private:
    static QString idOf(const QVoice &v) { return v.name() + QLatin1Char('|') + v.locale().name(); }
    QTextToSpeech *m_tts;
    QList<QVoice> m_voices;
    bool m_spoke = false, m_stopping = false;
};

} // namespace

SpeechEngine *makeSystemSpeechEngine() { return new SystemSpeechEngine; }

} // namespace jp
#endif

#include "app/readaloud.h"
#include "app/icons.h"
#include "app/plaintext.h"
#include "app/settings.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QTextBlock>
#include <QTextDocument>
#include <QToolButton>

#include <algorithm>

namespace jp {

namespace {

SpeechEngineFactory &factory()
{
    static SpeechEngineFactory f;
    return f;
}

// What a program built without the system's voices makes: no voice, and nothing said.
class SilentEngine : public SpeechEngine {
public:
    QList<SpeechVoice> voices() const override { return {}; }
    void setVoice(const QString &) override {}
    void setRate(double) override {}
    int say(const QString &) override { return 0; }
    void pause() override {}
    void resume() override {}
    void stop() override {}
};

const double kSpeeds[] = {0.5, 0.75, 1.0, 1.25, 1.5, 2.0, 3.0};

} // namespace

#ifdef JO_TTS
SpeechEngine *makeSystemSpeechEngine();   // speechqt.cpp (null when this computer has no engine to load)
#endif

namespace {
bool g_speechdMissing = false;
}

void setSpeechdMissingForTest(bool missing) { g_speechdMissing = missing; }
bool speechdMissingForTest() { return g_speechdMissing; }

void setSpeechEngineFactory(const SpeechEngineFactory &f) { factory() = f; }

SpeechEngine *makeSpeechEngine()
{
    if (factory()) return factory()();
#ifdef JO_TTS
    if (SpeechEngine *e = makeSystemSpeechEngine()) return e;
    return new SilentEngine;   // (no engine on this computer: no voices, and nothing was loaded to warn about)
#else
    return new SilentEngine;
#endif
}

bool speechIsBuiltIn()
{
#ifdef JO_TTS
    return true;
#else
    return false;
#endif
}

double speechRateForSpeed(double speed)
{
    speed = std::clamp(speed, 0.5, 3.0);
    return speed <= 1 ? (speed - 1) * 2 : (speed - 1) / 2;
}

ReadAloudBar::ReadAloudBar(const ReadAloudHost &host, QWidget *parent) : QWidget(parent), m_host(host), m_engine(makeSpeechEngine())
{
    setObjectName(QStringLiteral("readAloudBar"));
    m_engine->setParent(this);
    auto button = [this](const char *objectName, const QString &name, const char *icon) {
        auto *b = new QToolButton(this);
        b->setObjectName(QLatin1String(objectName));
        b->setIcon(jp::icon(QLatin1String(icon)));
        b->setToolTip(plainToolTip(name));
        b->setAccessibleName(name);
        b->setAutoRaise(true);
        b->setFocusPolicy(Qt::NoFocus);
        return b;
    };
    m_prev = button("readAloudPrevious", QCoreApplication::translate("jp::ReadAloudBar", "Previous Paragraph"), "skip-back");
    m_play = button("readAloudPlay", QCoreApplication::translate("jp::ReadAloudBar", "Play"), "play");
    m_next = button("readAloudNext", QCoreApplication::translate("jp::ReadAloudBar", "Next Paragraph"), "skip-forward");
    m_stop = button("readAloudStop", QCoreApplication::translate("jp::ReadAloudBar", "Stop"), "square");
    m_close = button("readAloudClose", QCoreApplication::translate("jp::ReadAloudBar", "Close"), "x");
    m_speed = new QComboBox(this);
    m_speed->setObjectName(QStringLiteral("readAloudSpeed"));
    m_speed->setAccessibleName(QCoreApplication::translate("jp::ReadAloudBar", "Speed"));
    m_speed->setToolTip(plainToolTip(m_speed->accessibleName()));
    m_speed->setFocusPolicy(Qt::NoFocus);
    for (double s : kSpeeds) m_speed->addItem(QStringLiteral("%1x").arg(s), s);
    m_voice = new QComboBox(this);
    m_voice->setObjectName(QStringLiteral("readAloudVoice"));
    m_voice->setAccessibleName(QCoreApplication::translate("jp::ReadAloudBar", "Voice"));
    m_voice->setToolTip(plainToolTip(m_voice->accessibleName()));
    m_voice->setFocusPolicy(Qt::NoFocus);
    m_voice->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_note = new QLabel(this);
    m_note->setObjectName(QStringLiteral("readAloudNote"));
    m_note->setTextFormat(Qt::PlainText);

    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(8, 2, 8, 2);
    for (QWidget *w : {static_cast<QWidget *>(m_prev), static_cast<QWidget *>(m_play), static_cast<QWidget *>(m_next), static_cast<QWidget *>(m_stop)}) row->addWidget(w);
    row->addSpacing(8);
    row->addWidget(m_speed);
    row->addWidget(m_voice);
    row->addWidget(m_note, 1);
    row->addWidget(m_close);

    // The remembered speed and voice (the voice only when this computer still has it).
    const double wanted = Settings::get().value(QStringLiteral("speech/speed"), 1.0).toDouble();
    int at = 2;
    for (int i = 0; i < m_speed->count(); ++i)
        if (qFuzzyCompare(m_speed->itemData(i).toDouble(), wanted)) at = i;
    m_speed->setCurrentIndex(at);
    for (const SpeechVoice &v : m_engine->voices()) m_voice->addItem(v.name, v.id);
    const int vi = m_voice->findData(Settings::get().value(QStringLiteral("speech/voice")).toString());
    if (vi >= 0) m_voice->setCurrentIndex(vi);
    m_engine->setRate(speechRateForSpeed(speed()));
    if (!voiceId().isEmpty()) m_engine->setVoice(voiceId());
    m_voice->setEnabled(m_voice->count() > 1);

    connect(m_engine, &SpeechEngine::wordSpoken, this, &ReadAloudBar::onWord);
    connect(m_engine, &SpeechEngine::finished, this, &ReadAloudBar::onFinished);
    connect(m_engine, &SpeechEngine::errorOccurred, this, &ReadAloudBar::onError);
    connect(m_play, &QToolButton::clicked, this, [this] {
        if (m_state == State::Stopped) start();
        else togglePause();
    });
    connect(m_stop, &QToolButton::clicked, this, [this] { stop(); });
    connect(m_prev, &QToolButton::clicked, this, [this] { previousParagraph(); });
    connect(m_next, &QToolButton::clicked, this, [this] { nextParagraph(); });
    connect(m_close, &QToolButton::clicked, this, [this] {
        stop();
        Q_EMIT closeRequested();
    });
    connect(m_speed, &QComboBox::activated, this, [this] { setSpeed(speed()); });
    connect(m_voice, &QComboBox::activated, this, [this] { setVoice(voiceId()); });
    updateControls();
}

ReadAloudBar::~ReadAloudBar()
{
    m_engine->stop();
}

QString ReadAloudBar::note() const { return m_note->text(); }
double ReadAloudBar::speed() const { return m_speed->currentData().toDouble(); }
QString ReadAloudBar::voiceId() const { return m_voice->currentData().toString(); }

void ReadAloudBar::start()
{
    for (const QMetaObject::Connection &c : std::as_const(m_watching)) disconnect(c);
    m_watching.clear();
    m_engine->stop();
    if (m_host.clear) m_host.clear();
    m_state = State::Stopped;
    if (m_engine->voices().isEmpty()) {
        finish(QCoreApplication::translate("jp::ReadAloudBar", "No voices are installed on this computer, so nothing can be read aloud. Add a voice in the system's speech settings."));
        return;
    }
    m_paras.clear();
    if (m_host.text) m_paras = m_host.text();
    m_paras.erase(std::remove_if(m_paras.begin(), m_paras.end(), [](const ReadAloudParagraph &p) { return p.text.trimmed().isEmpty(); }), m_paras.end());
    if (m_paras.isEmpty()) {
        finish(QCoreApplication::translate("jp::ReadAloudBar", "There is no text to read."));
        return;
    }
    m_note->setText(m_host.note ? m_host.note() : QString());
    if (m_host.watch) m_watching = m_host.watch([this] { documentChanged(); });
    speak(0, 0);
}

void ReadAloudBar::speak(int index, int from)
{
    m_index = index;
    m_base = from;
    m_word = from;
    m_state = State::Playing;
    m_engine->stop();
    m_token = m_engine->say(m_paras.at(index).text.mid(from));
    updateControls();
}

void ReadAloudBar::onWord(int utterance, int start, int length)
{
    if (utterance != m_token || m_state == State::Stopped || m_index >= m_paras.size()) return;
    m_word = m_base + start;
    if (m_host.highlight) m_host.highlight(m_paras.at(m_index), m_word, length);
}

void ReadAloudBar::onError(int utterance, const QString &message)
{
    if (utterance != m_token || m_state == State::Stopped) return;
    finish(QCoreApplication::translate("jp::ReadAloudBar", "Reading stopped: %1").arg(message.isEmpty() ? QCoreApplication::translate("jp::ReadAloudBar", "the voice failed.") : message));
}

void ReadAloudBar::onFinished(int utterance)
{
    if (utterance != m_token || m_state == State::Stopped) return;
    if (m_index + 1 < m_paras.size()) speak(m_index + 1, 0);
    else finish(QString());
}

void ReadAloudBar::finish(const QString &note)
{
    m_engine->stop();
    m_state = State::Stopped;
    for (const QMetaObject::Connection &c : std::as_const(m_watching)) disconnect(c);
    m_watching.clear();
    if (m_host.clear) m_host.clear();
    m_note->setText(note);
    updateControls();
}

void ReadAloudBar::documentChanged()
{
    if (m_state != State::Stopped) finish(QString());
    Q_EMIT closeRequested();
}

void ReadAloudBar::stop()
{
    if (m_state == State::Stopped) return;
    finish(QString());
}

void ReadAloudBar::togglePause()
{
    if (m_state == State::Playing) {
        m_engine->pause();
        m_state = State::Paused;
    } else if (m_state == State::Paused) {
        m_engine->resume();
        m_state = State::Playing;
    }
    updateControls();
}

void ReadAloudBar::previousParagraph()
{
    if (m_state == State::Stopped) return;
    speak(m_index > 0 ? m_index - 1 : 0, 0);
}

void ReadAloudBar::nextParagraph()
{
    if (m_state == State::Stopped) return;
    if (m_index + 1 < m_paras.size()) speak(m_index + 1, 0);
    else finish(QString());
}

void ReadAloudBar::setSpeed(double speed)
{
    const int i = m_speed->findData(speed);
    if (i >= 0) m_speed->setCurrentIndex(i);
    Settings::get().setValue(QStringLiteral("speech/speed"), this->speed());
    m_engine->setRate(speechRateForSpeed(this->speed()));
    if (m_state != State::Stopped) speak(m_index, m_word);
}

void ReadAloudBar::setVoice(const QString &id)
{
    const int i = m_voice->findData(id);
    if (i >= 0) m_voice->setCurrentIndex(i);
    Settings::get().setValue(QStringLiteral("speech/voice"), voiceId());
    if (!voiceId().isEmpty()) m_engine->setVoice(voiceId());
    if (m_state != State::Stopped) speak(m_index, m_word);
}

void ReadAloudBar::updateControls()
{
    const bool going = m_state != State::Stopped;
    const bool playing = m_state == State::Playing;
    m_play->setIcon(jp::icon(playing ? QStringLiteral("pause") : QStringLiteral("play")));
    m_play->setToolTip(playing ? QCoreApplication::translate("jp::ReadAloudBar", "Pause") : QCoreApplication::translate("jp::ReadAloudBar", "Play"));
    m_play->setAccessibleName(m_play->toolTip());
    m_stop->setEnabled(going);
    m_prev->setEnabled(going);
    m_next->setEnabled(going);
}

QList<ReadAloudParagraph> readAloudParagraphs(const QTextDocument *doc, int story, int from, int to)
{
    QList<ReadAloudParagraph> out;
    if (!doc) return out;
    if (to < 0) to = doc->characterCount();
    for (QTextBlock b = doc->findBlock(std::max(from, 0)); b.isValid() && b.position() < to; b = b.next()) {
        const int start = std::max(from, b.position()), end = std::min(to, b.position() + b.length() - 1);
        if (end <= start) continue;
        QString text = b.text().mid(start - b.position(), end - start);
        for (QChar &c : text)
            if (c == QChar::ObjectReplacementCharacter || c == QChar::LineSeparator || c == QChar::ParagraphSeparator || c == QChar::Nbsp) c = QLatin1Char(' ');
        out.push_back({text, story, start});
    }
    return out;
}

ReadAloudBar *readAloudBarOf(QMainWindow *win)
{
    return win ? win->findChild<ReadAloudBar *>(QStringLiteral("readAloudBar")) : nullptr;
}

ReadAloudBar *showReadAloud(QMainWindow *win, const ReadAloudHost &host)
{
    ReadAloudBar *bar = readAloudBarOf(win);
    QDockWidget *dock = bar ? qobject_cast<QDockWidget *>(bar->parentWidget()) : nullptr;
    if (!bar) {
        dock = new QDockWidget(win);
        dock->setObjectName(QStringLiteral("readAloudDock"));
        dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
        dock->setTitleBarWidget(new QWidget(dock));
        bar = new ReadAloudBar(host, dock);
        dock->setWidget(bar);
        win->addDockWidget(Qt::BottomDockWidgetArea, dock);
        QObject::connect(bar, &ReadAloudBar::closeRequested, dock, &QDockWidget::hide);
    }
    dock->show();
    bar->start();
    return bar;
}

} // namespace jp

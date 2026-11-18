#include "text/numbersystems.h"

#include <QCoreApplication>
#include <QHash>
#include <QVector>
#include <algorithm>

namespace jp::numsys {

namespace {

using Fn = QString (*)(qint64);

QString dec(qint64 n) { return QString::number(n); }

// ---- digits and letters

QString digitsFrom(qint64 n, char32_t zero)
{
    if (n < 0) return dec(n);
    QString out = QString::number(n);
    for (QChar &c : out) c = QChar(char16_t(zero + (c.unicode() - '0')));
    return out;
}

QString digitsOf(qint64 n, const QString &digits)
{
    if (n < 0) return dec(n);
    QString out;
    for (const QChar c : QString::number(n)) out += digits[c.unicode() - '0'];
    return out;
}

// A letter for 1 to the alphabet's size, then the letter twice, three times... (as the letter formats of a document count; at most 40 times).
QString repeated(qint64 n, const QString &alphabet)
{
    const qint64 k = alphabet.size();
    if (n < 1 || n > k * 40) return QString();
    return QString(int((n - 1) / k + 1), alphabet[int((n - 1) % k)]);
}

// One of a fixed list of characters (a sequence that has no more).
QString oneOf(qint64 n, const QString &list)
{
    return n >= 1 && n <= list.size() ? QString(list[int(n - 1)]) : QString();
}

QString bijective(qint64 n, bool upper)
{
    QString s;
    for (n = std::max<qint64>(1, n); n > 0; n = (n - 1) / 26) s.prepend(QChar(char16_t((upper ? 'A' : 'a') + (n - 1) % 26)));
    return s;
}

const QString kArabicAlpha = QStringLiteral("أبتثجحخدذرزسشصضطظعغفقكلمنهوي");
const QString kArabicAbjad = QStringLiteral("أبجدهوزحطيكلمنسعفصقرشتثخذضظغ");
const QString kHebrewAlpha = QStringLiteral("אבגדהוזחטיכלמנסעפצקרשת");
const QString kThaiLetters = QStringLiteral("กขคงจฉชซฌญฎฏฐฑฒณดตถทธนบปผฝพฟภมยรลวศษสหฬอฮ");
const QString kRussianLower = QStringLiteral("абвгдежзиклмнопрстуфхцчшщэюя");
const QString kHindiVowels = QStringLiteral("अआइईउऊऋएऐओऔ");
const QString kHindiConsonants = QStringLiteral("कखगघङचछजझञटठडढणतथदधनपफबभमयरलवशषसह");
const QString kGanada = QStringLiteral("가나다라마바사아자차카타파하거너더러머버서어저처커터퍼허");
const QString kChosung = QStringLiteral("ㄱㄴㄷㄹㅁㅂㅅㅇㅈㅊㅋㅌㅍㅎ");
const QString kAiueoFull = QStringLiteral("アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲン");
const QString kAiueoHalf = QStringLiteral("ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝ");
const QString kIrohaFull = QStringLiteral("イロハニホヘトチリヌルヲワカヨタレソツネナラムウヰノオクヤマケフコエテアサキユメミシヱヒモセス");
const QString kIrohaHalf = QStringLiteral("ｲﾛﾊﾆﾎﾍﾄﾁﾘﾇﾙｦﾜｶﾖﾀﾚｿﾂﾈﾅﾗﾑｳヰﾉｵｸﾔﾏｹﾌｺｴﾃｱｻｷﾕﾒﾐｼヱﾋﾓｾｽ");

// ---- Hebrew numerals: the letters add up to the number (15 and 16 are 9 + 6 and 9 + 7, not the name of God), from 1 to 999

QString hebrewNumerals(qint64 n)
{
    if (n < 1 || n > 999) return QString();
    static const QString ones = QStringLiteral("אבגדהוזחט"), tens = QStringLiteral("יכלמנסעפצ"), hundreds = QStringLiteral("קרשת");
    QString s;
    int h = int(n / 100), rest = int(n % 100);
    while (h > 4) {
        s += QChar(0x05EA);
        h -= 4;
    }
    if (h > 0) s += hundreds[h - 1];
    if (rest == 15) return s + QStringLiteral("טו");
    if (rest == 16) return s + QStringLiteral("טז");
    if (rest >= 10) s += tens[rest / 10 - 1];
    if (rest % 10) s += ones[rest % 10 - 1];
    return s;
}

// ---- Thai counting

QString thaiBelowMillion(qint64 n, bool afterMillion)
{
    static const char16_t *const digit[] = {u"", u"หนึ่ง", u"สอง", u"สาม", u"สี่", u"ห้า", u"หก", u"เจ็ด", u"แปด", u"เก้า"};
    static const char16_t *const place[] = {u"", u"สิบ", u"ร้อย", u"พัน", u"หมื่น", u"แสน"};
    QString s;
    for (int p = 5; p >= 0; --p) {
        qint64 unit = 1;
        for (int i = 0; i < p; ++i) unit *= 10;
        const int d = int(n / unit % 10);
        if (d == 0) continue;
        if (p == 0) s += (d == 1 && (n > 1 || afterMillion)) ? QStringLiteral("เอ็ด") : QString::fromUtf16(digit[d]);
        else if (p == 1 && d == 1) s += QString::fromUtf16(place[1]);
        else if (p == 1 && d == 2) s += QStringLiteral("ยี่") + QString::fromUtf16(place[1]);
        else s += QString::fromUtf16(digit[d]) + QString::fromUtf16(place[p]);
    }
    return s;
}

QString thaiCounting(qint64 n)
{
    if (n < 0 || n > kMostCounted) return QString();
    if (n == 0) return QStringLiteral("ศูนย์");
    QString s;
    if (n >= 1000000) s += thaiBelowMillion(n / 1000000 % 1000000, false) + QStringLiteral("ล้าน");
    if (n % 1000000) s += thaiBelowMillion(n % 1000000, n >= 1000000);
    return s;
}

// ---- Chinese, Japanese, and Korean counting: groups of four digits, each with its tens, hundreds, and thousands

struct Han {
    QString digit;             // the ten digit characters, zero first
    QString unit;              // tens, hundreds, thousands (one character each)
    QString big;               // ten thousands, a hundred millions
    bool zeroWord;             // a gap of zeros inside the number is written (Chinese: 一千零一), or not (Japanese: 千一)
    bool dropOne;              // "one" before tens, hundreds, and thousands is left out everywhere (Japanese: 千百十)...
    bool dropOneTens;          // ... or only before the tens at the start (Chinese: 十一, but 一百一十)
    bool dropOneBig;           // and before ten thousand, not before a hundred million (Korean: 만, but 일억)
};

QString han(qint64 n, const Han &h)
{
    if (n < 0 || n > kMostCounted) return QString();
    if (n == 0) return QString(h.digit[0]);
    const int groups[3] = {int(n % 10000), int(n / 10000 % 10000), int(n / 100000000 % 10000)};
    QString out;
    bool pendingZero = false;
    for (int gi = 2; gi >= 0; --gi) {
        const int g = groups[gi];
        if (g == 0) {
            pendingZero = !out.isEmpty();
            continue;
        }
        QString part;
        bool zero = false;
        for (int pos = 3; pos >= 0; --pos) {
            const int d = g / (pos == 3 ? 1000 : pos == 2 ? 100 : pos == 1 ? 10 : 1) % 10;
            if (d == 0) {
                zero = !part.isEmpty();
                continue;
            }
            if (zero && h.zeroWord) part += h.digit[0];
            zero = false;
            const bool skipOne = d == 1 && pos > 0 && (h.dropOne || (h.dropOneTens && pos == 1 && out.isEmpty() && part.isEmpty()));
            if (!skipOne) part += h.digit[d];
            if (pos > 0) part += h.unit[pos - 1];
        }
        if ((pendingZero || g < 1000) && !out.isEmpty() && h.zeroWord) out += h.digit[0];
        pendingZero = false;
        if (gi == 1 && g == 1 && h.dropOneBig) part.clear();
        out += part;
        if (gi > 0) out += h.big[gi - 1];
    }
    return out;
}

const Han kChinese = {QStringLiteral("零一二三四五六七八九"), QStringLiteral("十百千"), QStringLiteral("万亿"), true, false, true, false};
const Han kTaiwanese = {QStringLiteral("零一二三四五六七八九"), QStringLiteral("十百千"), QStringLiteral("萬億"), true, false, true, false};
const Han kChineseLegal = {QStringLiteral("零壹贰叁肆伍陆柒捌玖"), QStringLiteral("拾佰仟"), QStringLiteral("万亿"), true, false, false, false};
const Han kTraditionalLegal = {QStringLiteral("零壹貳參肆伍陸柒捌玖"), QStringLiteral("拾佰仟"), QStringLiteral("萬億"), true, false, false, false};
const Han kJapanese = {QStringLiteral("〇一二三四五六七八九"), QStringLiteral("十百千"), QStringLiteral("万億"), false, true, false, false};
const Han kJapaneseLegal = {QStringLiteral("零壱弐参四五六七八九"), QStringLiteral("拾百千"), QStringLiteral("万億"), false, false, false, false};
const Han kKorean = {QStringLiteral("영일이삼사오육칠팔구"), QStringLiteral("십백천"), QStringLiteral("만억"), false, true, false, true};

QString chineseCounting(qint64 n) { return han(n, kChinese); }
QString taiwaneseCounting(qint64 n) { return han(n, kTaiwanese); }
QString chineseLegalSimplified(qint64 n) { return han(n, kChineseLegal); }
QString ideographLegalTraditional(qint64 n) { return han(n, kTraditionalLegal); }
QString japaneseCounting(qint64 n) { return han(n, kJapanese); }
QString japaneseLegal(qint64 n) { return han(n, kJapaneseLegal); }
QString koreanCounting(qint64 n) { return han(n, kKorean); }

// Digit by digit
QString ideographDigital(qint64 n) { return digitsOf(n, QStringLiteral("〇一二三四五六七八九")); }
QString koreanDigital(qint64 n) { return digitsOf(n, QStringLiteral("영일이삼사오육칠팔구")); }
QString koreanDigital2(qint64 n) { return digitsOf(n, QStringLiteral("零一二三四五六七八九")); }

// Digits in groups of four, with the character for ten thousand between: 12345 is 一万二三四五.
QString japaneseDigitalTenThousand(qint64 n)
{
    if (n < 0) return dec(n);
    QString out;
    qint64 high = n / 10000;
    if (high == 0) return ideographDigital(n);
    out = ideographDigital(high) + QChar(0x4E07);
    const QString low = ideographDigital(n % 10000);
    return out + QString(4 - low.size(), QChar(0x3007)) + low;
}

// Korean's own numbers, from one to ninety-nine
QString koreanLegal(qint64 n)
{
    if (n < 1 || n > 99) return QString();
    static const char16_t *const tens[] = {u"", u"열", u"스물", u"서른", u"마흔", u"쉰", u"예순", u"일흔", u"여든", u"아흔"};
    static const char16_t *const ones[] = {u"", u"하나", u"둘", u"셋", u"넷", u"다섯", u"여섯", u"일곱", u"여덟", u"아홉"};
    return QString::fromUtf16(tens[n / 10]) + QString::fromUtf16(ones[n % 10]);
}

// Circled, parenthesized, and full-stopped digits
QString circled(qint64 n)
{
    if (n == 0) return QString(QChar(0x24EA));
    if (n >= 1 && n <= 20) return QString(QChar(char16_t(0x2460 + n - 1)));
    if (n >= 21 && n <= 35) return QString(QChar(char16_t(0x3251 + n - 21)));
    if (n >= 36 && n <= 50) return QString(QChar(char16_t(0x32B1 + n - 36)));
    return QString();
}
QString ideographCircled(qint64 n) { return n >= 1 && n <= 10 ? QString(QChar(char16_t(0x3280 + n - 1))) : QString(); }
QString parenthesized(qint64 n) { return n >= 1 && n <= 20 ? QString(QChar(char16_t(0x2474 + n - 1))) : QString(); }
QString fullStopped(qint64 n) { return n >= 1 && n <= 20 ? QString(QChar(char16_t(0x2488 + n - 1))) : QString(); }

// ---- English

const char *const kOnes[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};
const char *const kTens[] = {"", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};

QString belowThousand(int n)
{
    QStringList parts;
    if (n >= 100) {
        parts << QLatin1String(kOnes[n / 100]) + QLatin1String(" hundred");
        n %= 100;
    }
    if (n >= 20) parts << (n % 10 ? QLatin1String(kTens[n / 10]) + QLatin1Char('-') + QLatin1String(kOnes[n % 10]) : QLatin1String(kTens[n / 10]));
    else if (n > 0) parts << QLatin1String(kOnes[n]);
    return parts.join(QLatin1Char(' '));
}

QString capitalized(QString s)
{
    if (!s.isEmpty()) s[0] = s[0].toUpper();
    return s;
}

QString ordinalDigits(qint64 n)
{
    const quint64 a = n < 0 ? quint64(0) - quint64(n) : quint64(n);
    const int last = int(a % 100) >= 11 && int(a % 100) <= 13 ? 0 : int(a % 10);
    return QString::number(n) + (last == 1 ? QStringLiteral("st") : last == 2 ? QStringLiteral("nd") : last == 3 ? QStringLiteral("rd") : QStringLiteral("th"));
}

QString cardinalText(qint64 n) { return n >= 0 && n <= kMostCounted ? capitalized(cardinalWords(n)) : QString(); }
QString ordinalText(qint64 n) { return n >= 0 && n <= kMostCounted ? capitalized(ordinalWords(n)) : QString(); }

// ---- the table

struct Entry {
    const char *name;
    const char *label;   // what a person is shown (translated), empty for a format not offered
    Fn fn;
};

const Entry kEntries[] = {
    {"decimal", "", [](qint64 n) { return dec(n); }},
    {"decimalZero", "", [](qint64 n) { return QStringLiteral("%1").arg(n, 2, 10, QLatin1Char('0')); }},
    {"lowerLetter", "", [](qint64 n) { return bijective(n, false); }},
    {"upperLetter", "", [](qint64 n) { return bijective(n, true); }},
    {"lowerRoman", "", [](qint64 n) { return romanNumeral(n, false); }},
    {"upperRoman", "", [](qint64 n) { return romanNumeral(n, true); }},
    {"numberInDash", "", [](qint64 n) { return QStringLiteral("- %1 -").arg(n); }},
    {"hex", "", [](qint64 n) { return n >= 0 ? QString::number(n, 16).toUpper() : QString(); }},
    {"ordinal", QT_TRANSLATE_NOOP("jp::NumberSystems", "Ordinal numbers"), ordinalDigits},
    {"cardinalText", QT_TRANSLATE_NOOP("jp::NumberSystems", "Number words"), cardinalText},
    {"ordinalText", QT_TRANSLATE_NOOP("jp::NumberSystems", "Ordinal words"), ordinalText},
    {"decimalFullWidth", QT_TRANSLATE_NOOP("jp::NumberSystems", "Full-width digits"), [](qint64 n) { return digitsFrom(n, 0xFF10); }},
    {"decimalFullWidth2", "", [](qint64 n) { return digitsFrom(n, 0xFF10); }},
    {"decimalHalfWidth", "", [](qint64 n) { return dec(n); }},
    {"hindiNumbers", QT_TRANSLATE_NOOP("jp::NumberSystems", "Arabic-Indic digits"), [](qint64 n) { return digitsFrom(n, 0x0660); }},
    {"arabicAlpha", QT_TRANSLATE_NOOP("jp::NumberSystems", "Arabic letters"), [](qint64 n) { return repeated(n, kArabicAlpha); }},
    {"arabicAbjad", QT_TRANSLATE_NOOP("jp::NumberSystems", "Arabic abjad letters"), [](qint64 n) { return repeated(n, kArabicAbjad); }},
    {"hebrew1", QT_TRANSLATE_NOOP("jp::NumberSystems", "Hebrew numerals"), hebrewNumerals},
    {"hebrew2", QT_TRANSLATE_NOOP("jp::NumberSystems", "Hebrew letters"), [](qint64 n) { return repeated(n, kHebrewAlpha); }},
    {"hindiCounting", QT_TRANSLATE_NOOP("jp::NumberSystems", "Devanagari digits"), [](qint64 n) { return digitsFrom(n, 0x0966); }},
    {"hindiVowels", QT_TRANSLATE_NOOP("jp::NumberSystems", "Hindi vowels"), [](qint64 n) { return repeated(n, kHindiVowels); }},
    {"hindiConsonants", QT_TRANSLATE_NOOP("jp::NumberSystems", "Hindi consonants"), [](qint64 n) { return repeated(n, kHindiConsonants); }},
    {"thaiNumbers", QT_TRANSLATE_NOOP("jp::NumberSystems", "Thai digits"), [](qint64 n) { return digitsFrom(n, 0x0E50); }},
    {"thaiLetters", QT_TRANSLATE_NOOP("jp::NumberSystems", "Thai letters"), [](qint64 n) { return repeated(n, kThaiLetters); }},
    {"thaiCounting", QT_TRANSLATE_NOOP("jp::NumberSystems", "Thai number words"), thaiCounting},
    {"russianLower", QT_TRANSLATE_NOOP("jp::NumberSystems", "Russian lowercase letters"), [](qint64 n) { return repeated(n, kRussianLower); }},
    {"russianUpper", QT_TRANSLATE_NOOP("jp::NumberSystems", "Russian uppercase letters"), [](qint64 n) { return repeated(n, kRussianLower).toUpper(); }},
    {"chineseCounting", QT_TRANSLATE_NOOP("jp::NumberSystems", "Chinese counting"), chineseCounting},
    {"chineseCountingThousand", "", chineseCounting},
    {"chineseLegalSimplified", QT_TRANSLATE_NOOP("jp::NumberSystems", "Chinese formal numerals"), chineseLegalSimplified},
    {"taiwaneseCounting", QT_TRANSLATE_NOOP("jp::NumberSystems", "Traditional Chinese counting"), taiwaneseCounting},
    {"taiwaneseCountingThousand", "", taiwaneseCounting},
    {"ideographLegalTraditional", QT_TRANSLATE_NOOP("jp::NumberSystems", "Traditional Chinese formal numerals"), ideographLegalTraditional},
    {"ideographDigital", QT_TRANSLATE_NOOP("jp::NumberSystems", "Chinese digits"), ideographDigital},
    {"taiwaneseDigital", "", ideographDigital},
    {"ideographTraditional", QT_TRANSLATE_NOOP("jp::NumberSystems", "Heavenly stems"), [](qint64 n) { return oneOf(n, QStringLiteral("甲乙丙丁戊己庚辛壬癸")); }},
    {"ideographZodiac", QT_TRANSLATE_NOOP("jp::NumberSystems", "Earthly branches"), [](qint64 n) { return oneOf(n, QStringLiteral("子丑寅卯辰巳午未申酉戌亥")); }},
    {"ideographZodiacTraditional", "", [](qint64 n) { return oneOf(n, QStringLiteral("子丑寅卯辰巳午未申酉戌亥")); }},
    {"ideographEnclosedCircle", QT_TRANSLATE_NOOP("jp::NumberSystems", "Circled Chinese numerals"), ideographCircled},
    {"japaneseCounting", QT_TRANSLATE_NOOP("jp::NumberSystems", "Japanese counting"), japaneseCounting},
    {"japaneseLegal", QT_TRANSLATE_NOOP("jp::NumberSystems", "Japanese formal numerals"), japaneseLegal},
    {"japaneseDigitalTenThousand", QT_TRANSLATE_NOOP("jp::NumberSystems", "Japanese digits with ten thousands"), japaneseDigitalTenThousand},
    {"aiueo", QT_TRANSLATE_NOOP("jp::NumberSystems", "Katakana, half width"), [](qint64 n) { return oneOf(n, kAiueoHalf); }},
    {"aiueoFullWidth", QT_TRANSLATE_NOOP("jp::NumberSystems", "Katakana"), [](qint64 n) { return oneOf(n, kAiueoFull); }},
    {"iroha", QT_TRANSLATE_NOOP("jp::NumberSystems", "Iroha, half width"), [](qint64 n) { return oneOf(n, kIrohaHalf); }},
    {"irohaFullWidth", QT_TRANSLATE_NOOP("jp::NumberSystems", "Iroha"), [](qint64 n) { return oneOf(n, kIrohaFull); }},
    {"decimalEnclosedCircle", QT_TRANSLATE_NOOP("jp::NumberSystems", "Circled digits"), circled},
    {"decimalEnclosedCircleChinese", "", circled},
    {"decimalEnclosedParen", QT_TRANSLATE_NOOP("jp::NumberSystems", "Parenthesized digits"), parenthesized},
    {"decimalEnclosedFullstop", QT_TRANSLATE_NOOP("jp::NumberSystems", "Digits with a full stop"), fullStopped},
    {"koreanCounting", QT_TRANSLATE_NOOP("jp::NumberSystems", "Korean counting"), koreanCounting},
    {"koreanLegal", QT_TRANSLATE_NOOP("jp::NumberSystems", "Korean native numbers"), koreanLegal},
    {"koreanDigital", QT_TRANSLATE_NOOP("jp::NumberSystems", "Korean digits"), koreanDigital},
    {"koreanDigital2", QT_TRANSLATE_NOOP("jp::NumberSystems", "Korean digits in Chinese characters"), koreanDigital2},
    {"ganada", QT_TRANSLATE_NOOP("jp::NumberSystems", "Korean ganada letters"), [](qint64 n) { return oneOf(n, kGanada); }},
    {"chosung", QT_TRANSLATE_NOOP("jp::NumberSystems", "Korean chosung letters"), [](qint64 n) { return oneOf(n, kChosung); }},
};

const Entry *entryOf(const QString &format)
{
    static const QHash<QString, const Entry *> byName = [] {
        QHash<QString, const Entry *> h;
        for (const Entry &e : kEntries) h.insert(QString::fromLatin1(e.name), &e);
        return h;
    }();
    return byName.value(format, nullptr);
}

} // namespace

QString romanNumeral(qint64 n, bool upper)
{
    static const std::pair<int, const char *> table[] = {{1000, "m"}, {900, "cm"}, {500, "d"}, {400, "cd"}, {100, "c"}, {90, "xc"}, {50, "l"}, {40, "xl"}, {10, "x"}, {9, "ix"}, {5, "v"}, {4, "iv"}, {1, "i"}};
    QString s;
    int v = int(std::clamp<qint64>(n, 1, 3999));
    for (const auto &t : table)
        while (v >= t.first) {
            s += QLatin1String(t.second);
            v -= t.first;
        }
    return upper ? s.toUpper() : s;
}

QString cardinalWords(qint64 n)
{
    if (n < 0 || n > kMostCounted) return QString();
    if (n == 0) return QStringLiteral("zero");
    static const char *const scales[] = {"", " thousand", " million", " billion"};
    QStringList parts;
    for (int scale = 0; n > 0; ++scale, n /= 1000)
        if (const int group = int(n % 1000)) parts.prepend(belowThousand(group) + QLatin1String(scales[scale]));
    return parts.join(QLatin1Char(' '));
}

QString ordinalWords(qint64 n)
{
    QString words = cardinalWords(n);
    if (words.isEmpty()) return words;
    // The last word becomes its ordinal.
    const int cut = std::max(words.lastIndexOf(QLatin1Char(' ')), words.lastIndexOf(QLatin1Char('-')));
    const QString head = words.left(cut + 1), last = words.mid(cut + 1);
    static const QHash<QString, QString> irregular = {{QStringLiteral("one"), QStringLiteral("first")}, {QStringLiteral("two"), QStringLiteral("second")}, {QStringLiteral("three"), QStringLiteral("third")},
                                                      {QStringLiteral("five"), QStringLiteral("fifth")}, {QStringLiteral("eight"), QStringLiteral("eighth")}, {QStringLiteral("nine"), QStringLiteral("ninth")},
                                                      {QStringLiteral("twelve"), QStringLiteral("twelfth")}, {QStringLiteral("zero"), QStringLiteral("zeroth")}};
    QString ordinal;
    if (irregular.contains(last)) ordinal = irregular.value(last);
    else if (last.endsWith(QLatin1Char('y'))) ordinal = last.chopped(1) + QStringLiteral("ieth");
    else ordinal = last + QStringLiteral("th");
    return head + ordinal;
}

bool knows(const QString &format) { return entryOf(format) != nullptr; }

bool shows(const QString &format, qint64 n)
{
    const Entry *e = entryOf(format);
    return e && !e->fn(n).isEmpty();
}

QString numberIn(const QString &format, qint64 n)
{
    const Entry *e = entryOf(format);
    if (!e) return dec(n);
    const QString s = e->fn(n);
    return s.isEmpty() ? dec(n) : s;
}

QStringList offeredFormats()
{
    QStringList out;
    for (const Entry &e : kEntries)
        if (e.label[0]) out << QString::fromLatin1(e.name);
    return out;
}

QString formatLabel(const QString &format)
{
    const Entry *e = entryOf(format);
    if (!e || !e->label[0]) return QString();
    QStringList sample;
    for (int n = 1; n <= 3; ++n) sample << numberIn(format, n);
    // (The whole sample is isolated, so that a right-to-left one does not reorder the commas of a left-to-right label.)
    return QCoreApplication::translate("jp::NumberSystems", e->label) + QStringLiteral(" (\u2068") + sample.join(QStringLiteral(", ")) + QStringLiteral(", …\u2069)");
}

// ---- ODF: the first three numbers of the sequence, as a text document's style:num-format spells it

QString odfFormatOf(const QString &format)
{
    static const QHash<QString, QString> own = {{QStringLiteral("decimal"), QStringLiteral("1")}, {QStringLiteral("lowerLetter"), QStringLiteral("a")}, {QStringLiteral("upperLetter"), QStringLiteral("A")},
                                                {QStringLiteral("lowerRoman"), QStringLiteral("i")}, {QStringLiteral("upperRoman"), QStringLiteral("I")}, {QStringLiteral("decimalZero"), QStringLiteral("01")}};
    if (own.contains(format)) return own.value(format);
    const Entry *e = entryOf(format);
    if (!e || format == QLatin1String("numberInDash")) return QString();
    QStringList sample;
    for (int n = 1; n <= 3; ++n) sample << numberIn(format, n);
    // (A format that counts as decimal digits does, here, is not told apart from them.)
    if (sample == QStringList{QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")}) return QString();
    return sample.join(QStringLiteral(", ")) + QStringLiteral(", ...");
}

QString formatOfOdf(const QString &odf)
{
    static const QHash<QString, QString> own = {{QStringLiteral("1"), QStringLiteral("decimal")}, {QStringLiteral("a"), QStringLiteral("lowerLetter")}, {QStringLiteral("A"), QStringLiteral("upperLetter")},
                                                {QStringLiteral("i"), QStringLiteral("lowerRoman")}, {QStringLiteral("I"), QStringLiteral("upperRoman")}, {QStringLiteral("01"), QStringLiteral("decimalZero")}};
    if (own.contains(odf)) return own.value(odf);
    if (odf.size() < 3 || odf.size() > 200) return QString();
    // (Compared without spaces, and with the ellipsis character as three periods.)
    auto plain = [](QString t) { return t.replace(QChar(0x2026), QStringLiteral("...")).remove(QLatin1Char(' ')); };
    static const QHash<QString, QString> bySample = [&] {
        QHash<QString, QString> h;
        for (const Entry &e : kEntries) {
            const QString name = QString::fromLatin1(e.name);
            const QString text = plain(odfFormatOf(name));
            if (!text.isEmpty() && !h.contains(text)) h.insert(text, name);
        }
        return h;
    }();
    return bySample.value(plain(odf));
}

QString formatOfOdf(const QString &odf, const QString &exact)
{
    const QString found = formatOfOdf(odf);
    if (exact.isEmpty() || exact.size() >= 64 || !knows(exact)) return found;
    // (The exact name counts when it is spelled the way the file's sample says; the sample of a format that has none is "1".)
    const QString spelled = odfFormatOf(exact);
    if (spelled.isEmpty() ? odf == QLatin1String("1") : formatOfOdf(spelled) == found) return exact;
    return found;
}

} // namespace jp::numsys

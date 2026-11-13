// Pieces the rules of French, German, and Spanish share. (Included in grammarrules.cpp, inside its unnamed namespace: the tokens, the context, and the word lists are its own.)

// Word lists with letters beyond ASCII in them (words written in UTF-8, in lowercase). Made once, like WORDS.
QSet<QString> setOf8(std::initializer_list<const char *> words)
{
    QSet<QString> out;
    for (const char *w : words) out.insert(QString::fromUtf8(w));
    return out;
}
#define WORDS8(...) ([]() -> const QSet<QString> & { static const QSet<QString> s = setOf8({__VA_ARGS__}); return s; }())

QString capitalized(const QString &w) { return w.isEmpty() ? w : w.at(0).toUpper() + w.mid(1); }

// Whether the word at token j begins with a small letter as written.
bool beginsLower(const Ctx &c, int j) { return c.isWord(j) && c.text.at(c.tokens[j].start).isLower(); }

// Whether the space token i is one plain space or one no-break space (what sits between two words of a phrase).
bool oneSpace(const Ctx &c, int i)
{
    if (!c.isSpace(i) || c.tokens[i].end - c.tokens[i].start != 1) return false;
    const QChar ch = c.text.at(c.tokens[i].start);
    return ch == QLatin1Char(' ') || ch == QChar(0x00A0);
}

// The next word, as `nextWord` finds it, but only over a space of the kind a phrase has.
int phraseNext(const Ctx &c, int i)
{
    const int j = c.nextWord(i);
    return (j >= 0 && oneSpace(c, i + 1)) ? j : -1;
}

// "the the": the second of two words alike, with the space before it taken out. `legitimate`: words that can be said twice ("nous nous"); `shortWords`: words that are
// a slip when a capital begins the first ("Le le"), where any other capitalized word said twice is a name or a type and its name.
void doubledWord(Ctx &c, const QSet<QString> &legitimate, const QSet<QString> &shortWords)
{
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const int j = c.nextWord(i);
        if (j < 0 || c.tokens[j].kind != Tok::Word || c.tokens[i].lower != c.tokens[j].lower) continue;
        const QString between = c.text.mid(c.tokens[i].end, c.tokens[j].start - c.tokens[i].end);
        if (between.contains(QLatin1Char('\n')) || between.contains(QChar(0x2028)) || between.contains(QChar(0x2029)) || between.contains(QLatin1Char('\t'))) continue;   // (across a line, or in a table: left to the person)
        const QString &w = c.tokens[i].lower;
        if (w.size() < 2 || legitimate.contains(w)) continue;
        if (c.firstUpper(j)) continue;   // (a name said twice, or a name after a word that names its type)
        if (c.firstUpper(i) && !shortWords.contains(w)) continue;
        if (c.tokens[j].start - c.tokens[i].end > 2) continue;
        // (the quotation mark may stand off by a space, as the guillemets of French do)
        auto at = [&](int p) { return p >= 0 && p < c.text.size() ? c.text.at(p) : QChar(); };
        const int before = c.tokens[i].start - 1, after = c.tokens[j].end;
        const QChar quoteBefore = at(before).isSpace() ? at(before - 1) : at(before);
        const QChar quoteAfter = at(after).isSpace() ? at(after + 1) : at(after);
        if (QStringView(u"\"'“‘«”’»„").contains(quoteBefore) || QStringView(u"\"'“‘«”’»„").contains(quoteAfter)) continue;   // (a word mentioned)
        bool digits = false;
        for (QChar ch : w) digits = digits || ch.isDigit();
        if (digits) continue;
        c.add(c.tokens[i].end, c.tokens[j].end, GK::Repetition, QCoreApplication::translate("Grammar", "Repeated word."), QString());
    }
}

// A space before a comma, a period, or another mark (the rule of English), except before a "?" or "!" that has a small letter after it: a button or a character that is
// named ("die Schaltfläche ? am Rand"), not a question.
void spacedMark(Ctx &c)
{
    const int first = int(c.issues.size());
    spaceBeforeMark(c);
    for (int k = int(c.issues.size()) - 1; k >= first; --k) {
        const int m = c.issues[k].end;
        if (m + 2 < c.text.size() && (c.text.at(m) == QLatin1Char('?') || c.text.at(m) == QLatin1Char('!')) && c.text.at(m + 1) == QLatin1Char(' ') && c.text.at(m + 2).isLower()) c.issues.removeAt(k);
    }
}

// The rule of English for a sentence that begins with a small letter after a period, with the short forms of the language in `shortened`, and without a flag on a name of code
// ("p2s3 is the page") or after one ("_xlfn. pour").
void sentenceStartOf(Ctx &c, const QSet<QString> &shortened)
{
    const int first = int(c.issues.size());
    lowercaseSentenceStartWith(c, &shortened);
    for (int k = int(c.issues.size()) - 1; k >= first; --k) {
        const int s0 = c.issues[k].start;
        bool code = false;
        for (int p = s0; p < c.text.size() && c.text.at(p).isLetterOrNumber(); ++p) code = code || c.text.at(p).isDigit();
        int q = s0 - 2;   // (the period is before the space)
        while (q > 0 && c.text.at(q - 1).isLetter()) --q;
        if (code || (q > 0 && c.text.at(q - 1) == QLatin1Char('_'))) c.issues.removeAt(k);
    }
}

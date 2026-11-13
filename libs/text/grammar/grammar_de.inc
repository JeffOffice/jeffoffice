// German. (Included in grammarrules.cpp.) The rules look for what is a mistake in every style of German: a word for another that sounds like it (das and dass, seid and seit,
// wider and wieder), a comma that a clause needs, a common noun with its capital left off, and the marks and doubled words that every language has.

// "der der": das das, die die, and the like are not slips.
void deRepeatedWord(Ctx &c)
{
    doubledWord(c, WORDS8("das", "die", "der", "den", "dem", "des", "sie", "ihr", "ihm", "ihn", "wir", "es", "ja", "nein", "so", "was", "wer", "wie", "wo", "sehr", "viel", "mehr", "immer", "bla", "ha", "hm", "na", "ach",
                          "oh", "ah", "aua", "bitte", "danke", "gut", "ganz", "weit", "hoch", "lange", "tick", "tack", "pfui", "hey", "hallo"),
                WORDS8("der", "die", "das", "ein", "eine", "und", "oder", "in", "zu", "mit", "von", "für", "auf", "an", "im", "ist", "hat", "dass"));
}

// "Ich glaube, das er kommt" for "dass", and "Dass ist gut" for "Das ist gut".
void deDasDass(Ctx &c)
{
    const QSet<QString> &verbs = WORDS8("glaube", "glaubt", "glaubst", "glauben", "denke", "denkt", "dachte", "denken", "meine", "meint", "finde", "findet", "weiß", "weiss", "wusste", "wissen", "sage", "sagt", "sagte",
                                        "hoffe", "hofft", "hoffte", "hoffen", "fürchte", "befürchte", "vermute", "behaupte", "behauptet", "wünsche", "bedauere", "bemerkte", "sagten", "glaubten", "dachten");
    const QSet<QString> &subjects = WORDS8("er", "sie", "es", "wir", "ihr", "ich", "du", "man");
    const QSet<QString> &verbsAfter = WORDS8("ist", "war", "wird", "sind", "kann", "muss", "hat", "geht", "klingt", "stimmt", "gefällt", "funktioniert", "passt", "bedeutet", "heißt", "stört");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        const int n = phraseNext(c, i);
        if (l == QLatin1String("das") && c.original(i) == QLatin1String("das")) {
            if (!c.isSpace(i - 1) || !c.isPunct(i - 2, QLatin1Char(',')) || !c.isWord(i - 3) || !verbs.contains(c.tokens[i - 3].lower)) continue;
            if (n < 0 || c.tokens[n].kind != Tok::Word || !subjects.contains(c.tokens[n].lower)) continue;
            replaceToken(c, i, QStringLiteral("dass"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"dass\" (the conjunction that begins a clause), not \"das\"."));
        } else if (l == QLatin1String("dass") && c.original(i) == QLatin1String("Dass") && startsSentence(c, i)) {
            if (n < 0 || c.tokens[n].kind != Tok::Word || !verbsAfter.contains(c.tokens[n].lower)) continue;
            replaceToken(c, i, QStringLiteral("das"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"Das\" (the pronoun) here: \"dass\" begins a clause with its verb at the end."));
        }
    }
}

// seid (you are) and seit (since): "ihr seit nicht allein", "seid zwei Jahren".
void deSeidSeit(Ctx &c)
{
    const QSet<QString> &units = WORDS8("jahren", "monaten", "wochen", "tagen", "stunden", "minuten", "jahrzehnten", "langem", "kurzem", "längerem", "ewigkeiten");
    const QSet<QString> &counts = WORDS8("zwei", "drei", "vier", "fünf", "sechs", "sieben", "acht", "neun", "zehn", "elf", "zwölf", "einigen", "vielen", "mehreren", "wenigen");
    const QSet<QString> &predicate = WORDS8("nicht", "alle", "so", "sehr", "willkommen", "bereit", "dabei", "doch", "wirklich", "wieder", "ein", "eine", "die", "der");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        const int n = phraseNext(c, i);
        if (n < 0) continue;
        if (l == QLatin1String("seid")) {
            bool time = c.tokens[n].kind == Tok::Word && units.contains(c.tokens[n].lower);
            if (!time && (c.tokens[n].kind == Tok::Number || (c.tokens[n].kind == Tok::Word && counts.contains(c.tokens[n].lower)))) {
                const int m = phraseNext(c, n);
                time = m >= 0 && c.tokens[m].kind == Tok::Word && units.contains(c.tokens[m].lower);
            }
            if (time) replaceToken(c, i, QStringLiteral("seit"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"seit\" (since, for a time), not \"seid\" (you are)."));
        } else if (l == QLatin1String("seit")) {
            const int p = c.prevWord(i);
            if (p >= 0 && c.tokens[p].lower == QLatin1String("ihr") && c.tokens[n].kind == Tok::Word && predicate.contains(c.tokens[n].lower))
                replaceToken(c, i, QStringLiteral("seid"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"seid\" (you are), not \"seit\" (since)."));
        }
    }
}

// wider (against) and wieder (again): "komm wider", "wieder Willen".
void deWiderWieder(Ctx &c)
{
    const QSet<QString> &againAfter = WORDS8("mal", "einmal", "da", "hier", "dort", "zurück", "gut", "gesund", "jetzt", "anders", "neu");
    const QSet<QString> &againBefore = WORDS8("immer", "nie", "nun", "schon");
    const QSet<QString> &againstAfter = WORDS8("willen", "erwarten", "besseres", "besseren");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        if (l != QLatin1String("wider") && l != QLatin1String("wieder")) continue;
        const int n = phraseNext(c, i), p = c.prevWord(i);
        const bool nextWord = n >= 0 && c.tokens[n].kind == Tok::Word;
        if (l == QLatin1String("wider")) {
            if (p >= 0 && c.tokens[p].lower == QLatin1String("und")) continue;   // (hin und wider)
            const bool before = p >= 0 && againBefore.contains(c.tokens[p].lower) && !(nextWord && againstAfter.contains(c.tokens[n].lower));
            if ((nextWord && againAfter.contains(c.tokens[n].lower)) || before)
                replaceToken(c, i, QStringLiteral("wieder"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"wieder\" (again), not \"wider\" (against)."));
        } else if (nextWord && againstAfter.contains(c.tokens[n].lower)) {
            replaceToken(c, i, QStringLiteral("wider"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"wider\" (against) in this phrase, not \"wieder\" (again)."));
        }
    }
}

// The comma before dass, weil, obwohl, and ob: "Ich glaube dass er kommt".
void deCommaBeforeConjunction(Ctx &c)
{
    const QSet<QString> &conjunctions = WORDS("dass", "weil", "obwohl", "ob");
    const QSet<QString> &pronouns = WORDS8("er", "sie", "es", "wir", "ihr", "ich", "du", "man", "das", "dies");
    // (words that join the clause to the one before, or that go with the conjunction itself: "und weil", "nur weil", "als ob", "ohne dass", "sodass")
    const QSet<QString> &goWith = WORDS8("und", "oder", "aber", "sondern", "denn", "sowie", "bzw", "beziehungsweise", "so", "ohne", "außer", "statt", "anstatt", "kaum", "als", "auf", "bis", "nur", "gerade", "auch",
                                         "selbst", "sogar", "besonders", "schon", "erst", "zumal", "nicht", "wie", "sowohl", "noch", "doch", "eben", "vor", "allem", "insbesondere", "vielleicht", "ja", "wenn", "zwar",
                                         "dass", "weil", "obwohl", "ob", "sei", "bloß", "allein", "umso", "je", "es", "mal", "etwa", "beispielsweise", "wohl", "eher",
                                         "bereits", "nämlich", "speziell", "unter");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || !conjunctions.contains(c.tokens[i].lower) || c.firstUpper(i)) continue;
        const int p = c.prevWord(i);
        if (p < 0 || !oneSpace(c, i - 1) || c.tokens[p].kind != Tok::Word || c.chunkAddress[i]) continue;
        const bool question = c.tokens[i].lower == QLatin1String("ob") || c.tokens[i].lower == QLatin1String("dass");
        if (goWith.contains(c.tokens[p].lower) && !(question && c.tokens[p].lower == QLatin1String("nicht"))) continue;   // ("er weiß nicht, ob" has a comma; "nicht weil" has none)
        if (c.tokens[i].lower == QLatin1String("ob")) {
            const int n = phraseNext(c, i);
            if (n < 0 || c.tokens[n].kind != Tok::Word || !pronouns.contains(c.tokens[n].lower)) continue;
        }
        c.add(c.tokens[p].start, c.tokens[i].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Put a comma before \"%1\": it begins a clause.").arg(c.original(i)), c.original(p) + QStringLiteral(", ") + c.original(i));
    }
}

// A common noun after its article with a small first letter: "das haus", "im auto".
void deNounCapital(Ctx &c)
{
    const QSet<QString> &articles = WORDS8("der", "die", "das", "den", "dem", "des", "ein", "eine", "einen", "einem", "einer", "eines", "im", "am", "zum", "zur", "vom", "beim", "ins", "ans");
    // (nouns that are no verb, adjective, or adverb in small letters: not "essen", "leben", "kosten", "recht", "macht", "weg", "morgen")
    const QSet<QString> &nouns = WORDS8("haus", "hund", "katze", "mann", "kind", "zeit", "jahr", "tag", "land", "stadt", "welt", "buch", "auto", "schule", "wasser", "geld", "name", "thema", "frage", "antwort", "gruppe",
                                        "woche", "monat", "stunde", "minute", "seite", "ende", "anfang", "beispiel", "ergebnis", "platz", "ort", "regel", "grund", "firma", "straße", "tisch", "stuhl", "vater",
                                        "mutter", "bruder", "schwester", "freund", "arbeit", "problem", "projekt", "programm", "system", "bild", "wort", "satz", "text", "ziel", "zahl", "preis", "markt", "gesetz",
                                        "staat", "gemeinde", "mitglied", "vertrag", "entscheidung", "möglichkeit", "person", "familie", "gesellschaft", "politik", "wirtschaft", "regierung", "bereich", "ebene",
                                        "gebiet", "richtung", "rolle", "weise", "form", "fall", "sinn", "kraft", "nacht", "abend", "sommer", "winter", "herbst", "kopf", "hand", "auge", "herz", "körper", "gesicht",
                                        "stimme", "kirche", "kunst", "musik", "sprache", "geschichte", "lösung", "ursache", "wirkung", "grenze", "ordnung", "angst", "freiheit", "gesundheit", "bildung", "entwicklung",
                                        "erfahrung", "verbindung", "hilfe", "ahnung");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || !articles.contains(c.tokens[i].lower)) continue;
        const int n = phraseNext(c, i);
        if (n < 0 || c.tokens[n].kind != Tok::Word || !nouns.contains(c.tokens[n].lower) || !beginsLower(c, n) || c.original(n) != c.tokens[n].lower || c.chunkAddress[n]) continue;
        c.add(c.tokens[n].start, c.tokens[n].end, GK::Capitalization, QCoreApplication::translate("Grammar", "A noun begins with a capital letter."), capitalized(c.original(n)));
    }
}

// "größer wie" for "größer als".
void deAlsWie(Ctx &c)
{
    const QSet<QString> &comparatives = WORDS8("besser", "schlechter", "größer", "kleiner", "höher", "länger", "kürzer", "schneller", "später", "früher", "älter", "jünger", "stärker", "schwächer", "breiter", "näher",
                                               "billiger", "teurer", "schöner", "leichter", "schwerer", "lieber", "weniger", "tiefer", "enger", "kälter", "wärmer", "dicker", "dünner", "wichtiger", "einfacher");
    const QSet<QString> &equal = WORDS8("so", "genauso", "ebenso", "fast", "nicht");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || !comparatives.contains(c.tokens[i].lower)) continue;
        const int n = phraseNext(c, i);
        if (n < 0 || c.tokens[n].lower != QLatin1String("wie") || c.tokens[n].kind != Tok::Word) continue;
        if (const int p = c.prevWord(i); p >= 0 && equal.contains(c.tokens[p].lower)) continue;
        replaceToken(c, n, QStringLiteral("als"), GK::Grammar, QCoreApplication::translate("Grammar", "After a comparative, use \"als\" (than), not \"wie\"."));
    }
}

// Two words written as one: "garnicht", "vorallem".
void deRunTogether(Ctx &c)
{
    static const QHash<QString, QString> joined = {{QStringLiteral("garnicht"), QStringLiteral("gar nicht")}, {QStringLiteral("garnichts"), QStringLiteral("gar nichts")}, {QStringLiteral("garkein"), QStringLiteral("gar kein")},
                                                  {QStringLiteral("vorallem"), QStringLiteral("vor allem")}, {QStringLiteral("aufeinmal"), QStringLiteral("auf einmal")}, {QStringLiteral("nocheinmal"), QStringLiteral("noch einmal")},
                                                  {QStringLiteral("garnie"), QStringLiteral("gar nie")}};
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const auto it = joined.constFind(c.tokens[i].lower);
        if (it == joined.constEnd() || c.chunkAddress[i]) continue;
        replaceToken(c, i, *it, GK::WordChoice, QCoreApplication::translate("Grammar", "These are two words."));
    }
}

// A sentence that starts with a small letter, after a period.
void deSentenceStart(Ctx &c)
{
    sentenceStartOf(c, WORDS8("usw", "bzw", "evtl", "ggf", "vgl", "inkl", "zzgl", "abb", "tab", "str", "sog", "bez", "mind", "max", "min", "std", "mio", "mrd", "ebd", "ders", "dies", "hrsg", "bsp", "betr",
                                          "dgl", "engl", "franz", "lat", "geb", "gest", "verh", "zit", "zus", "abs", "art", "kap", "aufl", "vorm", "nachm", "tel", "fax", "inh", "ges", "zzt", "ugs", "einschl",
                                          "ausschl", "versch", "bzgl", "gem", "entspr", "anm", "prof", "tsd", "ung", "unt", "sek", "jhd", "geschr", "dipl", "ing", "med", "rer", "nat", "iur", "phil", "habil"));
}

// Phrases with a plainer way of saying them (style: off unless asked for).
void deWordyPhrases(Ctx &c)
{
    static const PhraseTable table({{"zum jetzigen zeitpunkt", "jetzt"}, {"zum gegenwärtigen zeitpunkt", "jetzt"}, {"im falle von", "bei"}, {"aufgrund von", "wegen"}, {"mit hilfe von", "mit"},
                                    {"nach wie vor", "weiterhin"}});
    table.each(c, [&](int i, int j, const QString &with) {
        replacePhrase(c, i, j, with, GK::Style, QCoreApplication::translate("Grammar", "A plainer way to say this."));
    });
}

// "z.B." for "z. B.": the letters of a short form of two or three words are set apart by a no-break space (style: off unless asked for).
void deAbbreviationSpace(Ctx &c)
{
    const QSet<QString> &forms = WORDS8("z.b.", "d.h.", "u.a.", "u.u.", "o.ä.", "s.o.", "s.u.", "v.a.", "z.t.", "u.ä.", "i.d.r.", "m.e.", "z.zt.", "u.v.m.");
    for (int i = 0; i + 3 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.tokens[i].end - c.tokens[i].start != 1 || !c.isPunct(i + 1, QLatin1Char('.'))) continue;
        if (i > 0 && c.tokens[i - 1].kind != Tok::Space && !(c.tokens[i - 1].kind == Tok::Punct && QStringView(u"(\"„“[").contains(c.text.at(c.tokens[i - 1].start)))) continue;
        int j = i;
        QString joinedLower;
        QStringList letters;
        while (j + 1 < c.size() && c.tokens[j].kind == Tok::Word && c.tokens[j].end - c.tokens[j].start <= 2 && c.isPunct(j + 1, QLatin1Char('.')) && letters.size() < 4) {
            letters << c.original(j);
            joinedLower += c.tokens[j].lower + QLatin1Char('.');
            if (forms.contains(joinedLower)) break;
            j += 2;
        }
        if (!forms.contains(joinedLower) || letters.size() < 2) continue;
        c.add(c.tokens[i].start, c.tokens[j + 1].end, GK::Style, QCoreApplication::translate("Grammar", "Put a no-break space between the parts of this short form."), letters.join(QStringLiteral(". ")) + QLatin1Char('.'));
    }
}

const Rule kGermanRules[] = {
    {"de-repeated-word", deRepeatedWord, GK::Repetition},
    {"de-das-dass", deDasDass, GK::WordChoice},
    {"de-seid-seit", deSeidSeit, GK::WordChoice},
    {"de-wider-wieder", deWiderWieder, GK::WordChoice},
    {"de-comma-before-conjunction", deCommaBeforeConjunction, GK::Punctuation},
    {"de-noun-capital", deNounCapital, GK::Capitalization},
    {"de-als-wie", deAlsWie, GK::Grammar},
    {"de-run-together", deRunTogether, GK::WordChoice},
    {"de-sentence-start", deSentenceStart, GK::Capitalization},
    {"double-space", doubleSpace, GK::Punctuation},
    {"space-before-mark", spacedMark, GK::Punctuation},
    {"comma-space", commaSpace, GK::Punctuation},
    {"doubled-mark", doubledMark, GK::Punctuation},
    {"de-wordy-phrases", deWordyPhrases, GK::Style},
    {"de-abbreviation-space", deAbbreviationSpace, GK::Style},
    {"long-sentence", longSentence, GK::Style},
    {"repeated-sentence-start", repeatedSentenceStart, GK::Style},
};

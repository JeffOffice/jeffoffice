// Spanish. (Included in grammarrules.cpp.) The question and exclamation marks of Spanish come in pairs, with the opening one upside down: the rules look for a sentence that
// ends with "?" or "!" and has no "¿" or "¡" in it.

// "que que": the sounds and the words that are said twice on purpose are not slips.
void esRepeatedWord(Ctx &c)
{
    doubledWord(c, WORDS8("ha", "no", "sí", "si", "muy", "mucho", "poco", "bien", "mal", "así", "más", "ya", "ay", "ja", "je", "ji", "tan", "bla", "ni", "tic", "tac", "bum", "pum", "ah", "oh", "ey", "claro", "vamos",
                          "venga", "vale", "bueno", "hola", "adiós", "gracias", "despacio", "cerca", "lejos", "dale", "nada", "todo", "sol", "ta", "na", "pa", "cu", "cuac", "miau", "guau", "chin", "chas", "está", "es", "son", "fue", "era", "estaba"),   // ("la ventana en la que está está vacía", "lo que es es")
                WORDS8("el", "la", "los", "las", "de", "del", "un", "una", "y", "o", "en", "que", "se", "a", "con", "por", "es", "lo", "su", "al"), true);
}

// Whether the token at i begins a clause: the first token, or after a mark (and spaces).
bool esClauseStart(const Ctx &c, int i)
{
    int k = i - 1;
    while (k >= 0 && c.tokens[k].kind == Tok::Space) --k;
    return k < 0 || c.tokens[k].kind == Tok::Punct;
}

// A "?" or "!" that closes a question or an exclamation with no "¿" or "¡" at its start.
void esInvertedMarks(Ctx &c)
{
    const QSet<QString> &shortened = WORDS8("etc", "pág", "págs", "núm", "art", "ejem", "aprox", "dpto", "tel", "vol", "cap", "fig", "máx", "mín", "ibíd", "col", "gral", "admón", "avda", "sres", "sras", "uds", "atte", "cía",
                                            "ltda", "prof", "edit", "trad", "véase", "sr", "sra", "dr", "dra", "srta", "ud", "ee", "uu");
    int first = -1, commas = 0;   // the first word of the clause that is open, and the commas in it
    bool question = false, exclamation = false;
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        const Token &t = c.tokens[i];
        if (t.kind == Tok::Space) {
            if (QStringView(c.text).mid(t.start, t.end - t.start).contains(QLatin1Char('\n'))) {
                first = -1;
                commas = 0;
                question = exclamation = false;
            }
            continue;
        }
        if (t.kind != Tok::Punct) {
            if (first < 0) first = i;
            continue;
        }
        const QChar mark = c.text.at(t.start);
        if (mark == QChar(0x00BF)) {
            question = true;
            if (first < 0) first = i;
        } else if (mark == QChar(0x00A1)) {
            exclamation = true;
            if (first < 0) first = i;
        } else if (mark == QLatin1Char(',')) {
            ++commas;
        } else if (mark == QLatin1Char('?') || mark == QLatin1Char('!')) {
            const bool q = mark == QLatin1Char('?');
            // (a mark that ends something said: after a word, and followed by a space, the end, or a closing mark; and a small letter after it goes on with the sentence)
            const bool afterWord = i >= 1 && (c.tokens[i - 1].kind == Tok::Word || c.tokens[i - 1].kind == Tok::Number);
            bool closes = i + 1 >= c.size() || c.tokens[i + 1].kind == Tok::Space || (c.tokens[i + 1].kind == Tok::Punct && QStringView(u")]}»”’\"?!").contains(c.text.at(c.tokens[i + 1].start)));
            if (closes && i + 2 < c.size() && c.tokens[i + 1].kind == Tok::Space && c.tokens[i + 2].kind == Tok::Word && c.text.at(c.tokens[i + 2].start).isLower()) closes = false;
            if (afterWord && closes && !(q ? question : exclamation) && !c.chunkAddress[i] && first >= 0 && c.tokens[first].kind == Tok::Word && c.tokens[i].start > c.tokens[first].end) {
                const QString message = q ? QCoreApplication::translate("Grammar", "A question begins with \"¿\".") : QCoreApplication::translate("Grammar", "An exclamation begins with \"¡\".");
                const QString open = q ? QStringLiteral("¿") : QStringLiteral("¡");
                if (commas == 0)
                    c.add(c.tokens[first].start, c.tokens[first].end, GK::Punctuation, message, open + c.original(first));
                else
                    note(c, t.start, t.end, GK::Punctuation, message);
            }
            first = -1;
            commas = 0;
            question = exclamation = false;
        } else if (mark == QLatin1Char('.')) {
            // a period ends a sentence before a space and a capital, unless a short form comes before it
            int n = i + 1;
            if (n < c.size() && c.tokens[n].kind == Tok::Punct && QStringView(u"\"')”’»").contains(c.text.at(c.tokens[n].start))) ++n;
            if (n < c.size() && c.tokens[n].kind != Tok::Space) continue;
            if (n + 1 < c.size() && c.tokens[n + 1].kind == Tok::Word && !c.firstUpper(n + 1)) continue;
            if (i > 0 && c.tokens[i - 1].kind == Tok::Word && (shortened.contains(c.tokens[i - 1].lower) || c.tokens[i - 1].end - c.tokens[i - 1].start == 1 || allCaps(c, i - 1))) continue;
            first = -1;
            commas = 0;
            question = exclamation = false;
        }
    }
}

// The question words take an accent in a question: "¿Como estás?", "¿Que haces?", "¿Porque no vienes?".
void esQuestionAccent(Ctx &c)
{
    static const QHash<QString, QString> accented = {{QStringLiteral("que"), QStringLiteral("qué")},       {QStringLiteral("como"), QStringLiteral("cómo")},       {QStringLiteral("donde"), QStringLiteral("dónde")},
                                                     {QStringLiteral("cuando"), QStringLiteral("cuándo")}, {QStringLiteral("quien"), QStringLiteral("quién")},     {QStringLiteral("quienes"), QStringLiteral("quiénes")},
                                                     {QStringLiteral("cual"), QStringLiteral("cuál")},     {QStringLiteral("cuales"), QStringLiteral("cuáles")},   {QStringLiteral("cuanto"), QStringLiteral("cuánto")},
                                                     {QStringLiteral("cuanta"), QStringLiteral("cuánta")}, {QStringLiteral("cuantos"), QStringLiteral("cuántos")}, {QStringLiteral("cuantas"), QStringLiteral("cuántas")},
                                                     {QStringLiteral("adonde"), QStringLiteral("adónde")}};
    const QSet<QString> &connectors = WORDS8("y", "pero", "entonces", "pues", "o", "e", "bueno", "oye");
    const QString message = QCoreApplication::translate("Grammar", "A question word takes an accent: qué, cómo, dónde, cuándo, quién, cuál, cuánto.");
    for (int i = 0; i + 1 < c.size(); ++i) {
        c.step();
        if (!c.isPunct(i, QChar(0x00BF))) continue;
        int w = i + 1;
        if (c.isSpace(w)) ++w;
        if (!c.isWord(w)) continue;
        if (connectors.contains(c.tokens[w].lower)) {
            w = phraseNext(c, w);
            if (w < 0 || c.tokens[w].kind != Tok::Word) continue;
        }
        const QString &l = c.tokens[w].lower;
        const int n = phraseNext(c, w);
        const bool nextWord = n >= 0 && c.tokens[n].kind == Tok::Word;
        if (l == QLatin1String("porque")) {
            c.add(c.tokens[w].start, c.tokens[w].end, GK::WordChoice, QCoreApplication::translate("Grammar", "In a question, write \"por qué\" (why) as two words, with the accent."), matchCase(c.original(w), QStringLiteral("por qué")));
        } else if (l == QLatin1String("por") && nextWord && c.tokens[n].lower == QLatin1String("que")) {
            c.add(c.tokens[w].start, c.tokens[n].end, GK::WordChoice, QCoreApplication::translate("Grammar", "In a question, write \"por qué\" (why) with the accent."), matchCase(c.original(w), QStringLiteral("por qué")));
        } else if (const auto it = accented.constFind(l); it != accented.constEnd()) {
            if (l == QLatin1String("que") && nextWord && (c.tokens[n].lower == QLatin1String("no") || c.tokens[n].lower == QLatin1String("si") || c.tokens[n].lower == QStringLiteral("sí"))) continue;   // ("¿Que no vienes?": you say you are not coming?)
            replaceToken(c, w, *it, GK::WordChoice, message);
        }
    }
}

// "haber si" for "a ver si", and "puede a ver" for "puede haber".
void esAVer(Ctx &c)
{
    const QSet<QString> &modals = WORDS8("debe", "deben", "debería", "deberían", "puede", "pueden", "podría", "podrían", "pudo", "pudiera", "podrá", "podrán", "suele", "suelen", "debió", "debiera");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        const int n = phraseNext(c, i);
        if (n < 0 || c.tokens[n].kind != Tok::Word) continue;
        if (l == QLatin1String("haber") && c.tokens[n].lower == QLatin1String("si") && esClauseStart(c, i)) {
            c.add(c.tokens[i].start, c.tokens[n].end, GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"a ver si\" (let us see if): \"haber\" is the verb."), matchCase(c.original(i), QStringLiteral("a ver si")));
        } else if (modals.contains(l) && c.tokens[n].lower == QLatin1String("a")) {
            const int v = phraseNext(c, n);
            if (v >= 0 && c.tokens[v].lower == QLatin1String("ver"))
                c.add(c.tokens[n].start, c.tokens[v].end, GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"haber\" (the verb) after this word, not \"a ver\"."), QStringLiteral("haber"));
        }
    }
}

// hay, ahí, and ay: "ahí muchas personas", "ay que ir", "por hay".
void esHayAhiAy(Ctx &c)
{
    const QSet<QString> &many = WORDS8("muchos", "muchas", "mucha", "mucho", "varios", "varias", "pocos", "pocas", "algunos", "algunas", "bastantes", "demasiados", "demasiadas", "tantos", "tantas", "suficientes");
    const QSet<QString> &ayAfter = WORDS8("muchos", "muchas", "mucha", "mucho", "un", "una", "que", "nada", "nadie", "alguien", "algo", "poco", "varios", "más", "menos", "tiempo", "gente", "problema", "ninguno", "ninguna",
                                          "otro", "otra", "cosas", "personas");
    const QSet<QString> &before = WORDS8("de", "por", "hasta", "desde");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        const int n = phraseNext(c, i);
        const bool nextWord = n >= 0 && c.tokens[n].kind == Tok::Word;
        if (l == QStringLiteral("ahí")) {
            const int p = c.prevWord(i);
            if (p >= 0 && before.contains(c.tokens[p].lower)) continue;   // ("de ahí que": hence)
            if (nextWord && (many.contains(c.tokens[n].lower) || c.tokens[n].lower == QLatin1String("que")))
                replaceToken(c, i, QStringLiteral("hay"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"hay\" (there is, there are) here, not \"ahí\" (there)."));
        } else if (l == QLatin1String("ay") && c.original(i) == QLatin1String("ay")) {
            const int p = c.prevWord(i);
            if (p >= 0 && nextWord && ayAfter.contains(c.tokens[n].lower))
                replaceToken(c, i, QStringLiteral("hay"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"hay\" (there is, there are) here, not \"ay\" (ouch)."));
        } else if (l == QLatin1String("hay")) {
            const int p = c.prevWord(i);
            if (p >= 0 && before.contains(c.tokens[p].lower))
                replaceToken(c, i, QStringLiteral("ahí"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"ahí\" (there) here, not \"hay\" (there is)."));
        }
    }
}

// A sentence that starts with a small letter, after a period.
void esSentenceStart(Ctx &c)
{
    sentenceStartOf(c, WORDS8("pág", "págs", "núm", "art", "ejem", "aprox", "dpto", "tel", "vol", "cap", "fig", "máx", "mín", "ibíd", "col", "gral", "admón", "avda", "sres", "sras", "uds", "atte", "cía", "ltda",
                                          "prof", "edit", "trad", "véase", "apdo", "urb", "pza", "ctra", "cód", "núms", "arts", "caps", "dto", "pdte", "ref", "tít"));
}

// "de el" for "del" and "a el" for "al", before a noun.
void esDelAl(Ctx &c)
{
    const QSet<QString> &notNouns = WORDS8("le", "lo", "la", "les", "los", "las", "se", "me", "te", "nos", "ya", "no", "que", "qué", "también", "solo", "sólo", "siempre", "nunca", "mismo", "más", "menos", "muy", "es", "fue", "era",
                                           "ha", "he", "hay", "está", "puede", "tiene", "tengo", "va", "vas", "voy", "hace", "como", "con", "para", "por", "sin", "sobre", "y", "o", "pero", "si", "sí", "cuando", "donde", "mucho",
                                           "poco", "todo", "nada");
    for (int i = 0; i + 2 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || (c.tokens[i].lower != QLatin1String("de") && c.tokens[i].lower != QLatin1String("a"))) continue;
        const int j = phraseNext(c, i);
        if (j < 0 || c.tokens[j].kind != Tok::Word || c.original(j) != QLatin1String("el")) continue;
        const int k = phraseNext(c, j);
        if (k < 0 || !beginsLower(c, k) || notNouns.contains(c.tokens[k].lower) || c.tokens[k].end - c.tokens[k].start < 3) continue;
        if (allCaps(c, i) || quotedAround(c, i, j) || (c.tokens[i].start > 0 && c.text.at(c.tokens[i].start - 1) == QLatin1Char('&'))) continue;   // ("&A el nombre": a code of a header)
        const bool de = c.tokens[i].lower == QLatin1String("de");
        c.add(c.tokens[i].start, c.tokens[j].end, GK::Grammar, QCoreApplication::translate("Grammar", "Use \"%1\": the two words run together before a masculine noun.").arg(de ? QStringLiteral("del") : QStringLiteral("al")),
              matchCase(c.original(i), de ? QStringLiteral("del") : QStringLiteral("al")));
    }
}

// "pienso de que" for "pienso que" (dequeísmo), and "me alegro que", "me di cuenta que", "me acuerdo que" for the ones with "de que" (queísmo).
void esDeQue(Ctx &c)
{
    const QSet<QString> &thinking = WORDS8("creo", "cree", "creen", "creemos", "creía", "creyó", "pienso", "piensa", "piensan", "pensamos", "pensaba", "opino", "opina", "opinan", "supongo", "supone", "suponemos", "supuso");
    const QSet<QString> &gave = WORDS8("di", "dio", "dimos", "dieron", "da", "dan", "doy", "daba", "daban", "dé", "den", "dar", "darse", "darme", "darte", "darle", "darnos");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        const int n = phraseNext(c, i);
        if (n < 0 || c.tokens[n].kind != Tok::Word) continue;
        const int m = phraseNext(c, n);
        const bool nextQue = m >= 0 && c.tokens[m].kind == Tok::Word && c.tokens[m].lower == QLatin1String("que");
        if (thinking.contains(l) && c.tokens[n].lower == QLatin1String("de") && nextQue) {
            c.add(c.tokens[n].start, c.tokens[m].end, GK::Grammar, QCoreApplication::translate("Grammar", "Take out \"de\": this verb is followed by \"que\" alone."), c.original(m));
        } else if (c.tokens[n].lower == QLatin1String("que") && (l == QLatin1String("alegro") || l == QLatin1String("alegré") || l == QLatin1String("acuerdo"))) {
            const int p = c.prevWord(i);
            if (p >= 0 && c.tokens[p].lower == QLatin1String("me"))
                c.add(c.tokens[i].start, c.tokens[n].end, GK::Grammar, QCoreApplication::translate("Grammar", "This verb takes \"de que\"."), c.original(i) + QStringLiteral(" de que"));
        } else if (l == QLatin1String("cuenta") && c.tokens[n].lower == QLatin1String("que")) {
            const int p = c.prevWord(i);
            if (p >= 0 && gave.contains(c.tokens[p].lower))
                c.add(c.tokens[i].start, c.tokens[n].end, GK::Grammar, QCoreApplication::translate("Grammar", "\"Darse cuenta\" takes \"de que\"."), c.original(i) + QStringLiteral(" de que"));
        }
    }
}

// "ha echo" for "ha hecho" (or "ha echado").
void esHecho(Ctx &c)
{
    eachPair(c, WORDS("he", "has", "ha", "hemos", "han", "había", "habían", "habrá", "haya", "hubo"), WORDS("echo"), [&](int i, int j) {
        GrammarIssue g;
        g.start = c.tokens[j].start;
        g.end = c.tokens[j].end;
        g.kind = GK::WordChoice;
        g.message = QCoreApplication::translate("Grammar", "After \"haber\", \"hecho\" (from hacer) or \"echado\" (from echar) is written.");
        g.suggestions << matchCase(c.original(j), QStringLiteral("hecho")) << matchCase(c.original(j), QStringLiteral("echado"));
        g.rule = QLatin1String(c.rule);
        c.issues << g;
    });
}

// Phrases with a plainer way of saying them (style: off unless asked for).
void esWordyPhrases(Ctx &c)
{
    static const PhraseTable table({{"en base a", "con base en"}, {"de acuerdo a", "de acuerdo con"}, {"debido a que", "porque"}, {"con el fin de", "para"}, {"en el caso de que", "si"}, {"a pesar de que", "aunque"},
                                    {"mas sin embargo", "sin embargo"}, {"subir para arriba", "subir"}, {"bajar para abajo", "bajar"}, {"salir para afuera", "salir"}, {"volver a repetir", "repetir"},
                                    {"lapso de tiempo", "lapso"}});
    table.each(c, [&](int i, int j, const QString &with) {
        replacePhrase(c, i, j, with, GK::Style, QCoreApplication::translate("Grammar", "A plainer way to say this."));
    });
}

const Rule kSpanishRules[] = {
    {"es-repeated-word", esRepeatedWord, GK::Repetition},
    {"es-inverted-marks", esInvertedMarks, GK::Punctuation},
    {"es-question-accent", esQuestionAccent, GK::WordChoice},
    {"es-a-ver", esAVer, GK::WordChoice},
    {"es-hay-ahi-ay", esHayAhiAy, GK::WordChoice},
    {"es-del-al", esDelAl, GK::Grammar},
    {"es-de-que", esDeQue, GK::Grammar},
    {"es-hecho", esHecho, GK::WordChoice},
    {"es-sentence-start", esSentenceStart, GK::Capitalization},
    {"double-space", doubleSpace, GK::Punctuation},
    {"space-before-mark", spacedMark, GK::Punctuation},
    {"comma-space", commaSpace, GK::Punctuation},
    {"doubled-mark", doubledMark, GK::Punctuation},
    {"es-wordy-phrases", esWordyPhrases, GK::Style},
    {"long-sentence", longSentence, GK::Style},
    {"repeated-sentence-start", repeatedSentenceStart, GK::Style},
};

// French. (Included in grammarrules.cpp.) Spaces: before ; : ! ? and inside « » the usual French typing has a no-break space (U+00A0, the one every word processor puts there and
// every font has), so that is what a suggestion puts; a narrow no-break space (U+202F) or a thin space, which careful typesetting uses, is left alone. The apostrophe a suggestion
// writes is the one the paragraph uses (curly if it has a curly one, straight if not).

bool frVowel(QChar ch) { return QStringView(u"aeiouàâäéèêëîïôöùûüœæ").contains(ch.toLower()); }

// "le le": nous nous, vous vous, and the sounds that repeat are not slips.
void frRepeatedWord(Ctx &c)
{
    doubledWord(c, WORDS8("nous", "vous", "si", "oui", "non", "tout", "très", "plus", "bien", "bon", "bye", "ha", "ah", "oh", "eh", "hé", "ben", "bof", "bla", "coucou", "hop", "hou", "ouh", "ouf", "chut",
                          "boum", "aïe", "ho", "hi", "la", "na", "y", "en", "ça", "pouet", "ding", "dong", "tic", "tac", "pan", "pif", "paf", "mal", "tant", "peu", "beau", "belle", "grand", "petit", "gros"),
                WORDS8("le", "la", "les", "de", "des", "du", "un", "une", "et", "à", "en", "que", "qui", "il", "elle", "ce", "dans", "pour", "sur", "est", "a"));
}

// A mark that French sets off with a space: ; : ! ? take one before them (no-break), and « » one inside.
void frSpaceBeforeMark(Ctx &c)
{
    const QString nb(QChar(0x00A0));
    for (int i = 1; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Punct) continue;
        const QChar mark = c.text.at(c.tokens[i].start);
        if (!QLatin1String(";:!?").contains(mark)) continue;
        const bool colon = mark == QLatin1Char(':') || mark == QLatin1Char(';');
        const int after = i + 1;
        if (after < c.size()) {   // (the text goes on with a space, or ends the clause: not ":)", "?x", or "http://")
            const Token &a = c.tokens[after];
            if (a.kind == Tok::Punct) {
                if (colon || !QStringView(u")]}»”’\"?!").contains(c.text.at(a.start))) continue;
            } else if (a.kind != Tok::Space) {
                continue;
            }
        }
        if (c.tokens[i - 1].kind == Tok::Space) {   // a plain space
            if (!c.plainSpaces(i - 1) || c.tokens[i - 1].end - c.tokens[i - 1].start != 1 || i < 2) continue;
            const int b = i - 2;
            const Tok bk = c.tokens[b].kind;
            if (bk != Tok::Word && bk != Tok::Number && !(bk == Tok::Punct && QStringView(u")]»%").contains(c.text.at(c.tokens[b].start)))) continue;
            if (mark == QLatin1Char(':') && bk == Tok::Word && c.tokens[b].end - c.tokens[b].start == 1) continue;   // (f : X, A : B)
            if (mark == QLatin1Char(':') && bk == Tok::Number && after + 1 < c.size() && c.tokens[after].kind == Tok::Space && c.tokens[after + 1].kind == Tok::Number) continue;   // (3 : 1)
            c.add(c.tokens[i - 1].start, c.tokens[i - 1].end, GK::Punctuation, QCoreApplication::translate("Grammar", "In French, put a no-break space before this mark."), nb);
            continue;
        }
        // no space: a word that stands by itself, then the mark, then the rest of the sentence
        if (c.tokens[i - 1].kind != Tok::Word) continue;
        // (what stands before the word: a space, or an opening mark; not "#REF!", "LET(a; b)", or "mailto:")
        if (i >= 2 && c.tokens[i - 2].kind != Tok::Space && !(c.tokens[i - 2].kind == Tok::Punct && (colon ? QStringView(u"«\"“‘") : QStringView(u"(«[\"“‘'*_-–—>")).contains(c.text.at(c.tokens[i - 2].start)))) continue;
        if (colon && WORDS("mailto", "http", "https", "ftp", "file", "tel", "sms", "www").contains(c.tokens[i - 1].lower)) continue;
        if (colon && (c.tokens[i - 1].end - c.tokens[i - 1].start == 1 || after + 1 >= c.size())) continue;   // (Q: a one-letter label; a colon that ends the text)
        {   // (a cell or a name with a digit in it, "B2; C3": a formula, not a sentence)
            bool digit = false;
            for (int p = c.tokens[i - 1].start; p < c.tokens[i - 1].end; ++p) digit = digit || c.text.at(p).isDigit();
            if (digit) continue;
        }
        c.add(c.tokens[i].start, c.tokens[i].end, GK::Punctuation, QCoreApplication::translate("Grammar", "In French, put a no-break space before this mark."), nb + QString(mark));
    }
}

// « and » take a space inside them (a no-break one).
void frGuillemets(Ctx &c)
{
    const QString nb(QChar(0x00A0));
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Punct) continue;
        const QChar mark = c.text.at(c.tokens[i].start);
        if (mark == QChar(0x00AB)) {
            const int a = i + 1;
            if (a >= c.size()) continue;
            if (c.tokens[a].kind == Tok::Word || c.tokens[a].kind == Tok::Number)
                c.add(c.tokens[i].start, c.tokens[i].end, GK::Punctuation, QCoreApplication::translate("Grammar", "In French, put a no-break space inside the guillemets."), QString(mark) + nb);
            else if (c.tokens[a].kind == Tok::Space && c.plainSpaces(a) && c.tokens[a].end - c.tokens[a].start == 1 && a + 1 < c.size() && c.tokens[a + 1].kind != Tok::Space)
                c.add(c.tokens[a].start, c.tokens[a].end, GK::Punctuation, QCoreApplication::translate("Grammar", "In French, put a no-break space inside the guillemets."), nb);
        } else if (mark == QChar(0x00BB) && i >= 1) {
            const int b = i - 1;
            const bool glued = c.tokens[b].kind == Tok::Word || c.tokens[b].kind == Tok::Number || (c.tokens[b].kind == Tok::Punct && QStringView(u".!?…").contains(c.text.at(c.tokens[b].start)));
            if (glued)
                c.add(c.tokens[i].start, c.tokens[i].end, GK::Punctuation, QCoreApplication::translate("Grammar", "In French, put a no-break space inside the guillemets."), nb + QString(mark));
            else if (c.tokens[b].kind == Tok::Space && c.plainSpaces(b) && c.tokens[b].end - c.tokens[b].start == 1 && b >= 1 && c.tokens[b - 1].kind != Tok::Space)
                c.add(c.tokens[b].start, c.tokens[b].end, GK::Punctuation, QCoreApplication::translate("Grammar", "In French, put a no-break space inside the guillemets."), nb);
        }
    }
}

// The article, the pronoun, or the conjunction that loses its vowel before a word that starts with one: "le ami" for "l'ami", "de eau", "que il", "je ai".
void frElision(Ctx &c)
{
    static const QHash<QString, QString> forms = {{QStringLiteral("le"), QStringLiteral("l")},          {QStringLiteral("la"), QStringLiteral("l")},          {QStringLiteral("de"), QStringLiteral("d")},
                                                  {QStringLiteral("que"), QStringLiteral("qu")},        {QStringLiteral("ne"), QStringLiteral("n")},          {QStringLiteral("je"), QStringLiteral("j")},
                                                  {QStringLiteral("me"), QStringLiteral("m")},          {QStringLiteral("te"), QStringLiteral("t")},          {QStringLiteral("se"), QStringLiteral("s")},
                                                  {QStringLiteral("jusque"), QStringLiteral("jusqu")}, {QStringLiteral("lorsque"), QStringLiteral("lorsqu")}, {QStringLiteral("puisque"), QStringLiteral("puisqu")},
                                                  {QStringLiteral("quoique"), QStringLiteral("quoiqu")}};
    // (words that begin with a vowel letter and do not take the elision: "le onze", "le oui", "la une"; "de un à dix")
    const QSet<QString> &standAlone = WORDS8("un", "une", "onze", "onzième", "oui", "ouais", "ouate", "ouistiti", "ou", "où", "et", "uhlan", "yéti");   // ("de et à", "de et vers": "et" is the word and, which does not elide)
    for (int i = 0; i + 2 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const auto f = forms.constFind(c.tokens[i].lower);
        if (f == forms.constEnd()) continue;
        const int j = phraseNext(c, i);
        if (j < 0 || c.tokens[j].kind != Tok::Word || !beginsLower(c, j)) continue;
        const QString &w = c.tokens[j].lower;
        const bool article = c.tokens[i].lower == QLatin1String("le") || c.tokens[i].lower == QLatin1String("la");
        const bool y = w == QLatin1String("y") && !article && c.tokens[i].lower != QLatin1String("de") && c.tokens[i].lower != QLatin1String("que");   // ("de y": a name of a value)
        if (w.size() <= 2 && c.isPunct(j + 1, QLatin1Char(','))) continue;   // ("tel que en, es ou pt": a list of codes)
        if (!y && (w.size() < 2 || !frVowel(w.at(0)))) continue;
        if (w == QLatin1String("un") || w == QLatin1String("une")) {
            if (c.tokens[i].lower != QLatin1String("que")) continue;   // ("le un", "la une", "de un à dix"; but "que un" is "qu'un")
        } else if (standAlone.contains(w) && !(w == QStringLiteral("où") && c.tokens[i].lower == QLatin1String("de"))) {
            continue;
        }
        if (allCaps(c, i) || (c.firstUpper(i) && !startsSentence(c, i))) continue;
        c.add(c.tokens[i].start, c.tokens[j].end, GK::Grammar, QCoreApplication::translate("Grammar", "Before a word that starts with a vowel, join the two with an apostrophe (l'ami, d'eau, qu'il)."),
              matchCase(c.original(i), *f) + c.apostrophe() + c.original(j));
    }
}

// "si il" for "s'il".
void frSiIl(Ctx &c)
{
    eachPair(c, WORDS("si"), WORDS("il", "ils"), [&](int i, int j) {
        if (!oneSpace(c, i + 1) || allCaps(c, i)) return;
        c.add(c.tokens[i].start, c.tokens[j].end, GK::Grammar, QCoreApplication::translate("Grammar", "Write \"s'il\" (si and il run together)."), matchCase(c.original(i), QStringLiteral("s")) + c.apostrophe() + c.original(j));
    });
}

// a and à: "il à mangé", and the phrases that begin with à ("a partir de").
void frAAccent(Ctx &c)
{
    const QSet<QString> &subjects = WORDS("il", "elle", "on");
    const QSet<QString> &participles = WORDS8("été", "eu", "fait", "dit", "pris", "mis", "vu", "pu", "su", "lu", "dû", "cru", "reçu", "voulu", "connu", "perdu", "vendu", "entendu", "fini", "choisi", "réussi", "suivi",
                                              "écrit", "offert", "permis", "promis", "ouvert", "couvert", "bu", "tenu", "venu", "devenu", "revenu", "appris", "compris", "repris", "surpris", "produit", "conduit",
                                              "construit", "détruit", "atteint", "éteint");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.tokens[i].lower != QStringLiteral("à")) continue;
        const int p = c.prevWord(i), n = phraseNext(c, i);
        if (p < 0 || n < 0 || c.tokens[n].kind != Tok::Word || !subjects.contains(c.tokens[p].lower)) continue;
        const QString &w = c.tokens[n].lower;
        const bool shaped = w.size() >= 4 && (w.endsWith(QStringLiteral("é")) || w.endsWith(QStringLiteral("ée")) || w.endsWith(QStringLiteral("és")) || w.endsWith(QStringLiteral("ées")));
        if (!shaped && !participles.contains(w)) continue;
        replaceToken(c, i, QStringLiteral("a"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"a\" (the verb avoir, without the accent) here."));
    }
    static const PhraseTable table({{"a partir de", "à partir de"}, {"a cause de", "à cause de"}, {"a propos de", "à propos de"}, {"a peu près", "à peu près"}, {"a peu pres", "à peu près"}, {"a travers", "à travers"},
                                    {"a côté de", "à côté de"}, {"a coté de", "à côté de"}, {"a l'aide de", "à l'aide de"}, {"a condition de", "à condition de"}, {"a condition que", "à condition que"},
                                    {"a force de", "à force de"}, {"a mesure que", "à mesure que"}, {"a moins de", "à moins de"}, {"a défaut de", "à défaut de"}, {"a l'égard de", "à l'égard de"}});
    table.each(c, [&](int i, int j, const QString &with) {
        replacePhrase(c, i, j, with, GK::WordChoice, QCoreApplication::translate("Grammar", "This phrase begins with \"à\" (with the accent)."));
    });
}

// ou and où: "Ou est la gare?", "d'ou", "oui où non".
void frOuAccent(Ctx &c)
{
    const QSet<QString> &verbs = WORDS8("est", "sont", "était", "étaient", "se", "sera", "serait");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        if (l == QLatin1String("d'ou") || l == QLatin1String("jusqu'ou")) {
            QString o = c.original(i);
            o.chop(2);
            c.add(c.tokens[i].start, c.tokens[i].end, GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"où\" (where), with the accent."), o + QStringLiteral("où"));
        } else if (l == QLatin1String("ou") && c.original(i) == QLatin1String("Ou") && startsSentence(c, i)) {
            const int n = phraseNext(c, i);
            if (n >= 0 && c.tokens[n].kind == Tok::Word && verbs.contains(c.tokens[n].lower) && !c.isPunct(n + 1, QLatin1Char('-')))
                c.add(c.tokens[i].start, c.tokens[i].end, GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"Où\" (where), with the accent."), QStringLiteral("Où"));
        }
    }
    static const PhraseTable table({{"oui où non", "oui ou non"}});
    table.each(c, [&](int i, int j, const QString &with) { replacePhrase(c, i, j, with, GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"ou\" (or), without the accent.")); });
}

// ce and se, ces and ses: "il ce passe", "tout se que", "ces sont", "il c'est passé".
void frCeSe(Ctx &c)
{
    const QSet<QString> &subjects = WORDS("il", "ils", "on");
    const QSet<QString> &notVerbs = WORDS8("que", "qui", "dont", "est", "sont", "soit", "sera", "serait", "était", "étaient", "fut", "fût", "a", "ont", "avait", "faut", "ce");
    const QSet<QString> &participles = WORDS8("été", "eu", "fait", "dit", "pris", "mis", "vu");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        const int n = phraseNext(c, i);
        const bool nextWord = n >= 0 && c.tokens[n].kind == Tok::Word;
        if (l == QLatin1String("ce")) {
            const int p = c.prevWord(i);
            if (p < 0 || !nextWord || !subjects.contains(c.tokens[p].lower) || notVerbs.contains(c.tokens[n].lower) || c.tokens[n].lower.contains(QLatin1Char('\''))) continue;
            replaceToken(c, i, QStringLiteral("se"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"se\" (the pronoun of a verb like se lever) here, not \"ce\"."));
        } else if (l == QLatin1String("se")) {
            if (nextWord && (c.tokens[n].lower == QLatin1String("que") || c.tokens[n].lower == QLatin1String("qui") || c.tokens[n].lower == QLatin1String("dont") || c.tokens[n].lower.startsWith(QLatin1String("qu'"))))
                replaceToken(c, i, QStringLiteral("ce"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"ce\" (what) here, not \"se\"."));
        } else if (l == QLatin1String("ces")) {
            if (nextWord && c.tokens[n].lower == QLatin1String("sont") && !c.firstUpper(n))
                replaceToken(c, i, QStringLiteral("ce"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"ce sont\" here: \"ces\" goes before a noun."));
        } else if (l == QLatin1String("c'est") || l == QLatin1String("c'était")) {
            const int p = c.prevWord(i);
            if (p < 0 || !nextWord || !subjects.contains(c.tokens[p].lower)) continue;
            const QString &w = c.tokens[n].lower;
            if (!(participles.contains(w) || (w.size() >= 4 && (w.endsWith(QStringLiteral("é")) || w.endsWith(QStringLiteral("ée")) || w.endsWith(QStringLiteral("és")) || w.endsWith(QStringLiteral("ées")))))) continue;
            const QString o = c.original(i);
            c.add(c.tokens[i].start, c.tokens[i].end, GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"s'est\" (the verb is reflexive) here, not \"c'est\"."), matchCase(o, QStringLiteral("s")) + o.mid(1));
        }
    }
}

// de le and à le for "du" and "au" (and les for "des" and "aux"), before a noun.
void frContraction(Ctx &c)
{
    const QSet<QString> &subjects = WORDS8("il", "elle", "on", "qui", "ça", "cela");
    const QSet<QString> &notNouns = WORDS8("lui", "leur", "y", "en", "me", "te", "se", "nous", "vous", "ne", "pas", "plus", "jamais", "déjà", "bien", "mieux", "aussi", "toujours", "très", "trop", "tous", "toutes", "tout",
                                           "moins", "même", "rien", "peu", "fait", "faire", "avoir", "être", "étant", "ayant");
    for (int i = 0; i + 4 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        if (l != QLatin1String("de") && l != QStringLiteral("à")) continue;
        const int j = phraseNext(c, i);
        if (j < 0 || c.tokens[j].kind != Tok::Word || (c.tokens[j].lower != QLatin1String("le") && c.tokens[j].lower != QLatin1String("les")) || c.firstUpper(j)) continue;
        const int k = phraseNext(c, j);
        if (k < 0 || !beginsLower(c, k)) continue;
        const QString &w = c.tokens[k].lower;
        if (w.size() < 3 || w.contains(QLatin1Char('\'')) || notNouns.contains(w)) continue;
        if (w.endsWith(QLatin1String("er")) || w.endsWith(QLatin1String("ir")) || w.endsWith(QLatin1String("re")) || w.endsWith(QLatin1String("dre")) || w.endsWith(QLatin1String("oir"))) continue;   // (an infinitive: "de le voir")
        if (const int p = c.prevWord(i); p >= 0 && subjects.contains(c.tokens[p].lower)) continue;   // ("il à le droit": the verb avoir)
        const bool one = c.tokens[j].lower == QLatin1String("le");
        const QString with = l == QLatin1String("de") ? (one ? QStringLiteral("du") : QStringLiteral("des")) : (one ? QStringLiteral("au") : QStringLiteral("aux"));
        c.add(c.tokens[i].start, c.tokens[j].end, GK::Grammar, QCoreApplication::translate("Grammar", "Use \"%1\" here: the two words run together before a noun.").arg(with), matchCase(c.original(i), with));
    }
}

// "ca va" for "ça va", and "sa va".
void frCa(Ctx &c)
{
    eachPair(c, WORDS("ca"), WORDS8("va", "fait", "marche", "suffit", "dépend", "depend", "y", "me", "te", "nous", "vous", "ne", "sert", "existe", "change", "coûte", "veut", "vient", "arrive"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("ça"), GK::WordChoice, QCoreApplication::translate("Grammar", "Write \"ça\" (with the cedilla)."));
    });
    eachPair(c, WORDS("sa"), WORDS("va"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("ça"), GK::WordChoice, QCoreApplication::translate("Grammar", "Write \"ça va\" (it goes), not \"sa\" (his or her)."));
    });
}

// "quand à lui" and "quant a" for "quant à".
void frQuant(Ctx &c)
{
    const QSet<QString> &pronouns = WORDS8("moi", "toi", "lui", "elle", "nous", "vous", "eux", "elles", "ce", "cela", "ceci", "ça", "ma", "mon", "ta", "ton", "sa", "son", "notre", "votre", "leur");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        const int j = phraseNext(c, i);
        if (j < 0 || c.tokens[j].kind != Tok::Word) continue;
        if (l == QLatin1String("quand") && c.tokens[j].lower == QStringLiteral("à")) {
            const int k = phraseNext(c, j);
            if (k >= 0 && c.tokens[k].kind == Tok::Word && pronouns.contains(c.tokens[k].lower)) replaceToken(c, i, QStringLiteral("quant"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"quant à\" (as for), not \"quand\" (when)."));
        } else if (l == QLatin1String("quant") && c.tokens[j].lower == QLatin1String("a")) {
            replaceToken(c, j, QStringLiteral("à"), GK::WordChoice, QCoreApplication::translate("Grammar", "Write \"quant à\" (as for), with the accent."));
        }
    }
}

// The hyphens of "qu'est-ce que", "n'est-ce pas", and "c'est-à-dire".
void frHyphens(Ctx &c)
{
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        if (l != QLatin1String("qu'est") && l != QLatin1String("n'est") && l != QLatin1String("c'est")) continue;
        const int j = phraseNext(c, i);
        if (j < 0 || c.tokens[j].kind != Tok::Word) continue;
        const int k = phraseNext(c, j);
        if (l == QLatin1String("c'est")) {
            if (k < 0 || c.tokens[k].kind != Tok::Word || (c.tokens[j].lower != QStringLiteral("à") && c.tokens[j].lower != QLatin1String("a")) || c.tokens[k].lower != QLatin1String("dire")) continue;
            const int m = phraseNext(c, k);
            if (!c.isPunct(k + 1, QLatin1Char(',')) && !(m >= 0 && c.tokens[m].lower == QLatin1String("que"))) continue;
            c.add(c.tokens[i].start, c.tokens[k].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Write \"c'est-à-dire\" with hyphens."), c.original(i) + QStringLiteral("-à-dire"));
            continue;
        }
        if (c.tokens[j].lower != QLatin1String("ce") || k < 0 || c.tokens[k].kind != Tok::Word) continue;
        const QString &w = c.tokens[k].lower;
        if (!(l == QLatin1String("n'est") ? w == QLatin1String("pas") : (w == QLatin1String("que") || w == QLatin1String("qui") || w.startsWith(QLatin1String("qu'"))))) continue;
        c.add(c.tokens[i].start, c.tokens[j].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Join these with a hyphen: \"%1-ce\".").arg(c.original(i)), c.original(i) + QLatin1Char('-') + c.original(j));
    }
}

// A sentence that starts with a small letter, after a period.
void frSentenceStart(Ctx &c)
{
    sentenceStartOf(c, WORDS8("env", "fig", "vol", "chap", "art", "avr", "janv", "févr", "juil", "sept", "oct", "nov", "déc", "dép", "prof", "mme", "mlle", "boul", "cie", "trad", "suiv", "ibid", "cit", "loc",
                                          "sup", "inf", "gén", "min", "max", "tél", "réf", "adj", "adv", "doc", "dir", "mod", "nat", "prés", "litt", "écon", "hist", "phil", "techn", "sing", "plur", "masc", "fém",
                                          "subj", "ind", "imp", "pron", "prép", "conj", "resp", "spéc", "obs", "voir", "paragr", "nos", "sté", "ste", "bd", "fbg", "tom"));
}

// Phrases with a plainer way of saying them (style: off unless asked for).
void frWordyPhrases(Ctx &c)
{
    static const PhraseTable table({{"malgré que", "bien que"}, {"voire même", "voire"}, {"au jour d'aujourd'hui", "aujourd'hui"}, {"sortir dehors", "sortir"}, {"monter en haut", "monter"}, {"descendre en bas", "descendre"},
                                    {"reculer en arrière", "reculer"}, {"collaborer ensemble", "collaborer"}, {"puis ensuite", "puis"}, {"pallier à", "pallier"}, {"suite à", "à la suite de"},
                                    {"comme par exemple", "par exemple"}, {"prévoir à l'avance", "prévoir"}});
    table.each(c, [&](int i, int j, const QString &with) {
        replacePhrase(c, i, j, with, GK::Style, QCoreApplication::translate("Grammar", "A plainer way to say this."));
    });
}

const Rule kFrenchRules[] = {
    {"fr-repeated-word", frRepeatedWord, GK::Repetition},
    {"fr-space-before-mark", frSpaceBeforeMark, GK::Punctuation},
    {"fr-guillemets", frGuillemets, GK::Punctuation},
    {"fr-elision", frElision, GK::Grammar},
    {"fr-si-il", frSiIl, GK::Grammar},
    {"fr-a-accent", frAAccent, GK::WordChoice},
    {"fr-ou-accent", frOuAccent, GK::WordChoice},
    {"fr-ce-se", frCeSe, GK::WordChoice},
    {"fr-contraction", frContraction, GK::Grammar},
    {"fr-ca", frCa, GK::WordChoice},
    {"fr-quant", frQuant, GK::WordChoice},
    {"fr-hyphens", frHyphens, GK::Punctuation},
    {"fr-sentence-start", frSentenceStart, GK::Capitalization},
    {"double-space", doubleSpace, GK::Punctuation},
    {"comma-space", commaSpace, GK::Punctuation},
    {"doubled-mark", doubledMark, GK::Punctuation},
    {"fr-wordy-phrases", frWordyPhrases, GK::Style},
    {"long-sentence", longSentence, GK::Style},
    {"repeated-sentence-start", repeatedSentenceStart, GK::Style},
};

// The grammar rules of libs/text/grammar, one group of tests for each rule: text with the mistake is flagged where it is and fixed by its first suggestion, and text that is
// correct (the legitimate look-alikes above all) is left alone. Also that the work of a check grows in line with the text.
#include "grammar/grammarrules.h"
#include "../testrunner.h"

#include <QElapsedTimer>
#include <QProcess>
#include <QtTest>
#include <atomic>
#include <thread>
#include <vector>

using namespace jp;

namespace {

// The text with the first suggestion of every issue put in (from the end, so the places hold).
QString fixed(const QString &text)
{
    QString out = text;
    const auto issues = grammar::check(text);
    for (int i = int(issues.size()) - 1; i >= 0; --i)
        if (!issues[i].suggestions.isEmpty()) out.replace(issues[i].start, issues[i].end - issues[i].start, issues[i].suggestions.first());
    return out;
}

// The words each issue is about, joined (what the issues mark).
QStringList marked(const QString &text)
{
    QStringList out;
    for (const GrammarIssue &g : grammar::check(text)) out << text.mid(g.start, g.end - g.start);
    return out;
}

} // namespace

class ShellGrammarRulesTests : public QObject {
    Q_OBJECT

    // Each pair is a text with a mistake and the text as its suggestions fix it.
    void fixes(const QList<QPair<const char *, const char *>> &cases)
    {
        for (const auto &c : cases) {
            const QString text = QString::fromUtf8(c.first);
            QVERIFY2(!grammar::check(text).isEmpty(), c.first);
            QCOMPARE(fixed(text), QString::fromUtf8(c.second));
        }
    }
    // Each text is correct: nothing is flagged.
    void leavesAlone(const QStringList &texts)
    {
        for (const QString &t : texts) QVERIFY2(grammar::check(t).isEmpty(), qPrintable(t + QStringLiteral(" -> ") + marked(t).join(QLatin1Char('|'))));
    }

    // The rules added in round 38: each has a sentence it must flag (with its fix) and sentences that look alike and must be left alone. A rule's issues are those it names.
    struct RuleCase {
        const char *rule, *bad, *fixed;
        QStringList ok;
        const char *span = nullptr, *message = nullptr;   // for a rule with nothing to put in place: the words it marks, and a part of its message (in place of `fixed`)
        grammar::Language language = grammar::Language::English;
    };
    static QVector<GrammarIssue> byRule(const QString &text, const QString &rule, grammar::Language language = grammar::Language::English)
    {
        QVector<GrammarIssue> out;
        for (const GrammarIssue &g : grammar::check(text, nullptr, grammar::kAllKinds, language))
            if (g.rule == rule) out << g;
        return out;
    }
    static QString fixedBy(const QString &text, const QString &rule, grammar::Language language = grammar::Language::English)
    {
        QString out = text;
        const auto issues = byRule(text, rule, language);
        for (int i = int(issues.size()) - 1; i >= 0; --i)
            if (!issues[i].suggestions.isEmpty()) out.replace(issues[i].start, issues[i].end - issues[i].start, issues[i].suggestions.first());
        return out;
    }
    static QStringList L(std::initializer_list<const char *> l)
    {
        QStringList out;
        for (const char *s : l) out << QString::fromUtf8(s);
        return out;
    }
    void check(const RuleCase &rc)
    {
        const QString rule = QString::fromLatin1(rc.rule), bad = QString::fromUtf8(rc.bad);
        QVERIFY2(!byRule(bad, rule, rc.language).isEmpty(), qPrintable(rule + QStringLiteral(" does not flag: ") + bad));
        if (rc.span) {
            const GrammarIssue g = byRule(bad, rule, rc.language).first();
            QCOMPARE(bad.mid(g.start, g.end - g.start), QString::fromUtf8(rc.span));
            QVERIFY2(g.message.contains(QString::fromUtf8(rc.message)), qPrintable(g.message));
            QVERIFY(g.suggestions.isEmpty());
        } else {
            QCOMPARE(fixedBy(bad, rule, rc.language), QString::fromUtf8(rc.fixed));
        }
        QVERIFY2(rc.ok.size() >= 3, rc.rule);
        for (const QString &t : rc.ok) QVERIFY2(byRule(t, rule, rc.language).isEmpty(), qPrintable(rule + QStringLiteral(" flags: ") + t));
    }
    static QVector<RuleCase> newRuleCases();
    static QVector<RuleCase> frenchRuleCases();
    static QVector<RuleCase> germanRuleCases();
    static QVector<RuleCase> spanishRuleCases();

private Q_SLOTS:
    void everyIssueIsInsideTheTextAndHasAMessage()
    {
        const QString text = QStringLiteral("the the cat sat on an mat , and alot of people could of come.");
        const auto issues = grammar::check(text);
        QVERIFY(issues.size() >= 4);
        int last = 0;
        for (const GrammarIssue &g : issues) {
            QVERIFY(g.start >= last && g.end > g.start && g.end <= text.size());
            QVERIFY(!g.message.isEmpty());
            QVERIFY(!g.suggestions.isEmpty());
            last = g.end;
        }
    }

    void aRepeatedWordIsFlaggedAndTheSecondOneRemoved()
    {
        fixes({{"This is the the best day.", "This is the best day."}, {"I went to to the store.", "I went to the store."}, {"It was The the end.", "It was The end."},
               {"We saw saw a bird.", "We saw a bird."}});
        QCOMPARE(marked(QStringLiteral("This is the the best day.")), QStringList{QStringLiteral(" the")});
    }
    void legitimateRepeatsAreLeftAlone()
    {
        leavesAlone({QStringLiteral("He said that that was fine."), QStringLiteral("She had had enough."), QStringLiteral("Bye bye, see you."), 
                     QStringLiteral("It is what it is is a saying."), QStringLiteral("Go to the left, left of the door."), QStringLiteral("I think, therefore I am. Am I?"),
                     QStringLiteral("Line one one more."), QStringLiteral("1 1 2 3 5 8"), QStringLiteral("no no no"), QStringLiteral("Bora Bora, Walla Walla, and Pago Pago."), QStringLiteral("He said \"the the\" is a typo."), QStringLiteral("He said \u201Cthe the\u201D is a typo."), QStringLiteral("What it was was a mistake."),
                     QStringLiteral("The truth is is plain."), QStringLiteral("We looked at at least two sites."), QStringLiteral("Receiving bank Bank One."), QStringLiteral("Declare Context context here."), QStringLiteral("column    prepared      prepared     prepared"), QStringLiteral("a\tthe\tthe"), QStringLiteral("The cell is put in in time."), QStringLiteral("It went on on Monday.")});
    }

    void aAndAnFollowTheSoundOfTheNextWord()
    {
        fixes({{"This is an test.", "This is a test."}, {"I ate a apple.", "I ate an apple."}, {"Wait a hour.", "Wait an hour."}, {"He is an user.", "He is a user."},
               {"It was a honest answer.", "It was an honest answer."}, {"She is an unicorn.", "She is a unicorn."}, {"We need an one-time code.", "We need a one-time code."},
               {"An dog barked.", "A dog barked."}, {"A elephant came.", "An elephant came."}, {"It is a MRI scan.", "It is an MRI scan."}, {"He has an UN role.", "He has a UN role."},
               {"That is a 8 percent rise.", "That is an 8 percent rise."}, {"It cost an 5 dollars.", "It cost a 5 dollars."}, {"Use a 11 inch screen.", "Use an 11 inch screen."},
               {"Add a X-ray room.", "Add an X-ray room."}, {"Buy an T-shirt.", "Buy a T-shirt."}});
    }
    void correctArticlesAreLeftAlone()
    {
        leavesAlone({QStringLiteral("This is a test of an hour."), QStringLiteral("He is a user of an app."), QStringLiteral("An honest man ate a unique apple."), QStringLiteral("A one-time offer."),
                     QStringLiteral("An MRI and a CT scan."), QStringLiteral("A NASA launch and an FBI file."), QStringLiteral("An 8 percent rise and a 7 percent fall."),
                     QStringLiteral("An 11 inch screen, a 12 inch one, an 18 inch one."), QStringLiteral("An unimportant detail and an onerous task."), QStringLiteral("A university and a European union."),
                     QStringLiteral("An heir to an heirloom."), QStringLiteral("An X-ray and a T-shirt."), QStringLiteral("Vitamin A is good, and so is Type A blood."),
                     QStringLiteral("In an 1800s house, a 1990s car."), QStringLiteral("An historic occasion, a historic occasion."), QStringLiteral("a \"test\" and an \"apple\""),
                     QStringLiteral("Trading as d/b/a Arctic Charters."), QStringLiteral("An Rr and an hbook file."),
                     QStringLiteral("An ugly duckling, a utensil, an umbrella, a useful tool."), QStringLiteral("A BIG DEAL FOR AN OLD MAN"), QStringLiteral("Read a u32 and an x86 chip, a h264 file."), QStringLiteral("An 0x88 record and an 03 value."),
                     QStringLiteral("A usable tool and an unusable one, a usage count."), QStringLiteral("A Utah ranch, a Uruguayan, and a Ugandan."), QStringLiteral("An Euler characteristic."),
                     QStringLiteral("The letter a appears twice.")});
    }

    void aSentenceThatStartsInLowercaseIsFlagged()
    {
        fixes({{"It rained. we stayed in.", "It rained. We stayed in."}, {"He left. she stayed.", "He left. She stayed."}});
    }
    void lowercaseAfterAbbreviationsNumbersAndNamesIsLeftAlone()
    {
        leavesAlone({QStringLiteral("See e.g. the list, i.e. the long one."), QStringLiteral("Cats, dogs, etc. are pets."), QStringLiteral("Dr. smith and Mr. jones came."), QStringLiteral("It was 3.5 percent."),
                     QStringLiteral("Open report.docx and read it."), QStringLiteral("Visit www.example.com or example.org today."), QStringLiteral("Wait... then go."),
                     QStringLiteral("\"Is it late?\" she asked."), QStringLiteral("iPhone sales rose. eBay did too."), QStringLiteral("At 5 p.m. we left."), QStringLiteral("Chapter 1. introduction"),
                     QStringLiteral("good morning"), QStringLiteral("and then he left, so we did"), QStringLiteral("vs. the rest of the world"),
                     QStringLiteral("Prices include tax, incl. tax and esp. when sold."), QStringLiteral("Yahoo! is a company."), QStringLiteral("Who? what? where?"), QStringLiteral("what a day it was."),
                     QStringLiteral("bake the cake and let it cool."), QStringLiteral("Is it late? yes it is."),
                     QStringLiteral("The var. nevadensis and ssp. varia are listed."), QStringLiteral("In meats. pp. 72-107 follow."), QStringLiteral("Cells of post-E. coli sepsis rose."),
                     QStringLiteral("Each addt'l. hour is extra."), QStringLiteral("Use the org. code for it."), QStringLiteral("Atriplex hylloides auct. ex Schwarz."), QStringLiteral("See Dougl. ex Torr. in the list."), QStringLiteral("The PhDs. are professors."),
                     QStringLiteral("Hundred weight: cwt. : the unit"), QStringLiteral("Enter highest order of age units. than 100. . . . 0 Years"), QStringLiteral("The alt. code is shown."),
                     QStringLiteral("Camp. lari is a name."), QStringLiteral("\u0411\u043E\u043B\u0435\u0435 100 \u043C\u043B\u043D. \u044D\u043A\u0441\u043F\u043E\u043D\u0430\u0442\u043E\u0432."), QStringLiteral("Yr. Imp. from the north."), QStringLiteral("Enter the first subm. date here."), QStringLiteral("Use a cont. inverse method."), QStringLiteral("Department of Admin. and Information."), QStringLiteral("See Educ. of Oak Park v. Kelly, 207 F. Supp. at 206.")});
    }

    void itsAndItIsAreToldApart()
    {
        fixes({{"Its a good day.", "It's a good day."}, {"I think its not ready.", "I think it's not ready."}, {"Its been a year.", "It's been a year."}, {"The dog has it's own bed.", "The dog has its own bed."},
               {"Its the best.", "It's the best."}});
    }
    void itsAndItIsCorrectUseIsLeftAlone()
    {
        leavesAlone({QStringLiteral("The dog wagged its tail."), QStringLiteral("It's a good day."), QStringLiteral("Its own merits and its time have come."), QStringLiteral("It's been fun, and its end is near."), QStringLiteral("A business valued as its going concern.")});
    }

    void theirThereAndTheyAreAreToldApart()
    {
        fixes({{"Their is a cat here.", "There is a cat here."}, {"I think their are two.", "I think there are two."}, {"They did there own work.", "They did their own work."}});
    }
    void theirThereAndTheyAreCorrectUseIsLeftAlone()
    {
        leavesAlone({QStringLiteral("There is a cat, and their dog is here."), QStringLiteral("They're late, but their car is there."), QStringLiteral("Their own work is theirs.")});
    }

    void yourAndYouAreAreToldApart()
    {
        fixes({{"Your a good friend.", "You're a good friend."}, {"I know your not sure.", "I know you're not sure."}, {"Your the best.", "You're the best."}, {"Is that you're own car?", "Is that your own car?"}});
    }
    void yourAndYouAreCorrectUseIsLeftAlone()
    {
        leavesAlone({QStringLiteral("Is that your car?"), QStringLiteral("You're a good friend."), QStringLiteral("Your A grade is in."), QStringLiteral("You're welcome, and your welcome pack is here."),
                     QStringLiteral("Your own name and you're sure."), QStringLiteral("I appreciate your being here."), QStringLiteral("Your a cappella performance."), QStringLiteral("That is your a priori claim.")});
    }

    void thenAndThanAreToldApart()
    {
        fixes({{"It is better then that.", "It is better than that."}, {"More then enough.", "More than enough."}, {"Rather then wait, go.", "Rather than wait, go."},
               {"First eat and than sleep.", "First eat and then sleep."}});
    }
    void thenAndThanCorrectUseIsLeftAlone()
    {
        leavesAlone({QStringLiteral("It is better than that."), QStringLiteral("Add more, then stir."), QStringLiteral("Eat, and then sleep."), QStringLiteral("If so, then go."),
                     QStringLiteral("Bigger than before, and then some."), QStringLiteral("If you feel better then call me."), QStringLiteral("If the price is higher then we sell."),
                     QStringLiteral("When it is faster then we go.")});
    }

    void loseAndLooseAreToldApart()
    {
        fixes({{"Don't loose weight.", "Don't lose weight."}, {"We will loose money.", "We will lose money."}, {"I could loose track.", "I could lose track."}, {"The shirt is too lose.", "The shirt is too loose."}});
    }
    void loseAndLooseCorrectUseIsLeftAlone()
    {
        leavesAlone({QStringLiteral("Don't lose weight."), QStringLiteral("A loose tooth."), QStringLiteral("The game is lose-lose."), QStringLiteral("To loose the dogs."),
                     QStringLiteral("They lose, and the loose ends stay.")});
    }

    void couldOfAndTheSameAreFlaggedAndMadeHave()
    {
        fixes({{"I could of gone.", "I could have gone."}, {"You should of seen it.", "You should have seen it."}, {"It would of been fine.", "It would have been fine."},
               {"He must of left.", "He must have left."}});
    }
    void couldOfInOtherUseIsLeftAlone()
    {
        leavesAlone({QStringLiteral("I could of course go."), QStringLiteral("I could have gone."), QStringLiteral("A must of the trade, one of them."), QStringLiteral("Some of them could go.")});
    }

    void aDoubleSpaceIsFlaggedAndMadeOne()
    {
        fixes({{"One  two.", "One two."}, {"Two  spaces  twice.", "Two spaces twice."}});
    }
    void singleSpacesAndIndentsAreLeftAlone()
    {
        leavesAlone({QStringLiteral("One two three."), QStringLiteral("  indented line"), QStringLiteral("Trailing  "), QStringLiteral("a\ttab\tseparated"), QStringLiteral("non\u00A0\u00A0breaking"), QStringLiteral("End.  Start."),
                     QStringLiteral("Home!  Then away?  Yes."), QStringLiteral("One   two   three."), QStringLiteral("Said \"stop.\"  Then left."), QStringLiteral("Said \u201Cstop.\u201D  Then left."),
                     QStringLiteral("Open 800 -  859 AM for it."), QStringLiteral("(2)  Small business set-asides."), QStringLiteral("See (0x0020,0x0052)  FrameOfReference now."),
                     QStringLiteral("Totals  # OF  % ON  # OF  % ON"), QStringLiteral("Value of X  xi is shown."), QStringLiteral("Name\tOriginator  Georgia Natural Resources"),
                     QStringLiteral("Phase 6:  Phase 6 is the site."), QStringLiteral("Zip  50319 in Des Moines."), QStringLiteral("-----  -----  --"), QStringLiteral("\"N\",\"\",  \"YESHIVA UNIVERSITY\",\"Y\""), QStringLiteral("Then P(U1  u1) = G(F)"),
                     QStringLiteral("a  b  c  d  e  f  g  h  i  j  k  l"), QStringLiteral("We saw  that  the  text  was  spread  out  to  fill  the  line.")});
        fixes({{"Near the coast  (Stimpson 1857) it grows.", "Near the coast (Stimpson 1857) it grows."}, {"Then,  a supplier might say.", "Then, a supplier might say."}});
    }

    void aSpaceBeforePunctuationIsFlaggedAndTaken()
    {
        fixes({{"Hello , world.", "Hello, world."}, {"Yes .", "Yes."}, {"Wait ; stop.", "Wait; stop."}, {"Why ?", "Why?"}, {"Look !", "Look!"}});
    }
    void spacesBeforeOtherMarksAreLeftAlone()
    {
        leavesAlone({QStringLiteral("Hello, world."), QStringLiteral("Wait ... what"), QStringLiteral("Smile :) and wink ;) ok"), QStringLiteral("See .txt files and 3 .5"), QStringLiteral("Ratio 3 : 1 and 4:5"),
                     QStringLiteral("Dash - and dash — fine"), QStringLiteral("( and ) and \" ok"), QStringLiteral("Wait … what"), QStringLiteral("Dry weight 56.89 ?"), QStringLiteral("Blank field 19      ."), QStringLiteral("set x   ! a comment"), QStringLiteral("(call ?*frame* x)"), QStringLiteral("Name : John"), QStringLiteral("f : X -> Y"), QStringLiteral("Wait . . . what"), QStringLiteral("Done . . .")});
    }

    void aMissingSpaceAfterAPeriodIsFlagged()
    {
        fixes({{"It ended.The next began.", "It ended. The next began."}, {"Really?No way.", "Really? No way."}, {"Stop!Go now.", "Stop! Go now."}});
    }
    void periodsInNamesNumbersAndAddressesAreLeftAlone()
    {
        leavesAlone({QStringLiteral("Visit example.com or Node.js now."), QStringLiteral("See Ph.D.Smith and e.g.Mr. Jones."), QStringLiteral("U.S.Army and 3.14 and v2.Next."),
                     QStringLiteral("Mr.Smith came."), QStringLiteral("The sheet Data!MyName and Sheet1!A1 are cells."), QStringLiteral("Open http://example.com/Index.Html today."), QStringLiteral("Mail me@example.Com now."), QStringLiteral("file.Docx and A.B and report.Pdf"), QStringLiteral("Use jess.Fact, jess.Value, and jess.Rete objects."), QStringLiteral("The jess.Rete class has two methods."), QStringLiteral("Read the Rete.Context class."), QStringLiteral("Call app.Run to start it."), QStringLiteral("Use Console.Log(x) here.")});
    }

    void alotIsFlaggedAndMadeALot()
    {
        fixes({{"I like it alot.", "I like it a lot."}, {"Alot of people came.", "A lot of people came."}});
    }
    void everydayAndEveryDayAreToldApartInClearPatterns()
    {
        fixes({{"I go there almost everyday.", "I go there almost every day."}, {"Everyday I walk.", "Every day I walk."}, {"Everyday we eat.", "Every day we eat."}});
    }
    void alotLookAlikesAndOrdinaryEverydayAreLeftAlone()
    {
        leavesAlone({QStringLiteral("A lot of people and allot of time."), QStringLiteral("Everyday life and everyday items."), QStringLiteral("Every day I walk."), QStringLiteral("Every day items arrive."),
                     QStringLiteral("An everyday occurrence."), QStringLiteral("My every day was spent at work."), QStringLiteral("Those are the every day tools.")});
    }

    void rulesDoNotHideEachOther()
    {
        const QString text = QStringLiteral("The the cat sat on an mat , and alot of people could of come.");
        QCOMPARE(fixed(text), QStringLiteral("The cat sat on a mat, and a lot of people could have come."));
    }

    void everyRuleHasAnEntryInTheNewRuleTests()
    {
        const QSet<QString> old = {QStringLiteral("repeated-word"), QStringLiteral("a-an"), QStringLiteral("sentence-start"), QStringLiteral("its"), QStringLiteral("their-there"),
                                   QStringLiteral("your-youre"), QStringLiteral("then-than"), QStringLiteral("lose-loose"), QStringLiteral("could-of"), QStringLiteral("double-space"),
                                   QStringLiteral("space-before-mark"), QStringLiteral("missing-space"), QStringLiteral("alot"), QStringLiteral("everyday")};
        QSet<QString> covered;
        for (const RuleCase &rc : newRuleCases()) covered.insert(QString::fromLatin1(rc.rule));
        for (const auto &r : grammar::rules())
            QVERIFY2(old.contains(r.name) || covered.contains(r.name), qPrintable(r.name));
        QVERIFY(grammar::rules().size() >= 58);
    }
    void theNewRulesFlagWhatTheyShouldAndLeaveTheLookAlikesAlone()
    {
        for (const RuleCase &rc : newRuleCases()) check(rc);
    }
    // The rules of French, German, and Spanish (round 38): each language has its own table; a rule it has from English keeps its name.
    void everyRuleOfAnotherLanguageHasAnEntry(grammar::Language language, const QString &prefix, const QVector<RuleCase> &cases)
    {
        QSet<QString> covered;
        for (const RuleCase &rc : cases) covered.insert(QString::fromLatin1(rc.rule));
        QSet<QString> english;
        for (const auto &r : grammar::rules()) english.insert(r.name);
        int own = 0;
        for (const auto &r : grammar::rules(language)) {
            if (r.name.startsWith(prefix)) {
                ++own;
                QVERIFY2(covered.contains(r.name), qPrintable(r.name));
            } else {
                QVERIFY2(english.contains(r.name), qPrintable(r.name));   // (a rule of English that the language uses as it is)
            }
        }
        for (const QString &name : covered) {
            bool found = false;
            for (const auto &r : grammar::rules(language)) found = found || r.name == name;
            QVERIFY2(found, qPrintable(name));
        }
        QVERIFY(own >= 10);
        QVERIFY(grammar::rules(language).size() >= 15);
    }
    // The kinds a language's rules have: Style is off by default (as in English), and a kind that is off is not looked for.
    void kindsOfAnotherLanguage(grammar::Language language, const QString &styleText, const QString &otherText, GrammarIssue::Kind otherKind)
    {
        QVERIFY(grammar::check(styleText, nullptr, grammar::kDefaultKinds, language).isEmpty());
        QVERIFY(!grammar::check(styleText, nullptr, grammar::kAllKinds, language).isEmpty());
        QVERIFY(!grammar::check(styleText, nullptr, grammar::kindBit(GrammarIssue::Style), language).isEmpty());
        QVERIFY(!grammar::check(otherText, nullptr, grammar::kindBit(otherKind), language).isEmpty());
        QVERIFY(grammar::check(otherText, nullptr, grammar::kAllKinds & ~grammar::kindBit(otherKind), language).isEmpty());
    }
    static QString U(const char *s) { return QString::fromUtf8(s); }

    void frenchRulesFlagWhatTheyShouldAndLeaveTheLookAlikesAlone()
    {
        for (const RuleCase &rc : frenchRuleCases()) check(rc);
    }
    void everyFrenchRuleHasAnEntry() { everyRuleOfAnotherLanguageHasAnEntry(grammar::Language::French, QStringLiteral("fr-"), frenchRuleCases()); }
    void frenchKindsFollowTheChoice()
    {
        kindsOfAnotherLanguage(grammar::Language::French, U("Il vient malgré que la pluie tombe."), U("Il à mangé."), GrammarIssue::WordChoice);
    }
    void frenchDetails()
    {
        const grammar::Language fr = grammar::Language::French;
        const QString nb = QString(QChar(0x00A0));
        // The mark takes a no-break space; one that is already there (a narrow one too) is left alone, and a text with none before ";" or "!" is told so.
        QCOMPARE(fixedBy(U("Bonjour; salut!"), QStringLiteral("fr-space-before-mark"), fr), U("Bonjour\u00A0; salut\u00A0!"));
        QCOMPARE(fixedBy(U("Voulez-vous partir ?"), QStringLiteral("fr-space-before-mark"), fr), U("Voulez-vous partir\u00A0?"));
        QVERIFY(byRule(U("Voulez-vous partir\u202F?"), QStringLiteral("fr-space-before-mark"), fr).isEmpty());
        // Plain spaces inside the guillemets become no-break ones.
        QCOMPARE(fixedBy(U("Il dit « bonjour » à tous."), QStringLiteral("fr-guillemets"), fr), U("Il dit «\u00A0bonjour\u00A0» à tous."));
        QVERIFY(byRule(U("Il dit « bonjour » à tous."), QStringLiteral("fr-guillemets"), fr).size() == 2);
        // The apostrophe of a suggestion is the one the paragraph uses.
        QCOMPARE(fixedBy(U("Je ai vu l\u2019homme."), QStringLiteral("fr-elision"), fr), U("J\u2019ai vu l\u2019homme."));
        QCOMPARE(fixedBy(U("Je ai vu l'homme."), QStringLiteral("fr-elision"), fr), U("J'ai vu l'homme."));
        QCOMPARE(fixedBy(U("Elle dit que il vient, et que elle part."), QStringLiteral("fr-elision"), fr), U("Elle dit qu'il vient, et qu'elle part."));
        QCOMPARE(fixedBy(U("Il est parti a cause de la pluie."), QStringLiteral("fr-a-accent"), fr), U("Il est parti à cause de la pluie."));
        QCOMPARE(fixedBy(U("Tout se que je sais, il c'est trompé."), QStringLiteral("fr-ce-se"), fr), U("Tout ce que je sais, il s'est trompé."));
        QCOMPARE(fixedBy(U("Il parle à les voisins de le jardin."), QStringLiteral("fr-contraction"), fr), U("Il parle aux voisins du jardin."));
        QCOMPARE(fixedBy(U("D'ou vient-il ?"), QStringLiteral("fr-ou-accent"), fr), U("D'où vient-il ?"));
        QCOMPARE(fixedBy(U("C'est à dire que non."), QStringLiteral("fr-hyphens"), fr), U("C'est-à-dire que non."));
        QCOMPARE(fixedBy(U("Il n'est pas là, n'est ce pas ?"), QStringLiteral("fr-hyphens"), fr), U("Il n'est pas là, n'est-ce pas ?"));
        QCOMPARE(fixedBy(U("Quant a moi."), QStringLiteral("fr-quant"), fr), U("Quant à moi."));
    }
    void germanRulesFlagWhatTheyShouldAndLeaveTheLookAlikesAlone()
    {
        for (const RuleCase &rc : germanRuleCases()) check(rc);
    }
    void everyGermanRuleHasAnEntry() { everyRuleOfAnotherLanguageHasAnEntry(grammar::Language::German, QStringLiteral("de-"), germanRuleCases()); }
    void germanKindsFollowTheChoice()
    {
        kindsOfAnotherLanguage(grammar::Language::German, U("Wir bleiben nach wie vor hier."), U("Ich glaube, das er kommt."), GrammarIssue::WordChoice);
    }
    void germanDetails()
    {
        const grammar::Language de = grammar::Language::German;
        QCOMPARE(fixedBy(U("Er sagte dass er kommt, aber er weiß nicht ob es regnet und blieb weil er müde war."), QStringLiteral("de-comma-before-conjunction"), de),
                 U("Er sagte, dass er kommt, aber er weiß nicht, ob es regnet und blieb, weil er müde war."));
        QCOMPARE(fixedBy(U("Das haus und der hund im auto."), QStringLiteral("de-noun-capital"), de), U("Das Haus und der Hund im Auto."));
        QCOMPARE(fixedBy(U("Ihr seit nicht allein, seid zwei Jahren schon."), QStringLiteral("de-seid-seit"), de), U("Ihr seid nicht allein, seit zwei Jahren schon."));
        QCOMPARE(fixedBy(U("Dass ist gut, und hin und wider auch."), QStringLiteral("de-das-dass"), de), U("Das ist gut, und hin und wider auch."));
        QCOMPARE(fixedBy(U("Es ist wieder Willen, und er kommt wider mal."), QStringLiteral("de-wider-wieder"), de), U("Es ist wider Willen, und er kommt wieder mal."));
        QCOMPARE(fixedBy(U("Das gilt z.B. und d.h. auch u.a. hier."), QStringLiteral("de-abbreviation-space"), de), U("Das gilt z.\u00A0B. und d.\u00A0h. auch u.\u00A0a. hier."));
        QVERIFY(byRule(U("Das gilt z. B. hier und www.a.b.de auch."), QStringLiteral("de-abbreviation-space"), de).isEmpty());
        // The rules that English has too (spaces and marks) run on German text as they are.
        QVERIFY(!byRule(U("Das ist gut , sehr gut."), QStringLiteral("space-before-mark"), de).isEmpty());
        QVERIFY(byRule(U("Klicken Sie auf die Schaltfläche ? am rechten Rand."), QStringLiteral("space-before-mark"), de).isEmpty());
        QVERIFY(!byRule(U("Was machst du ? Wir gehen."), QStringLiteral("space-before-mark"), de).isEmpty());
        QVERIFY(byRule(U("Er kam, etwa weil er musste."), QStringLiteral("de-comma-before-conjunction"), de).isEmpty());
    }
    void spanishRulesFlagWhatTheyShouldAndLeaveTheLookAlikesAlone()
    {
        for (const RuleCase &rc : spanishRuleCases()) check(rc);
    }
    void everySpanishRuleHasAnEntry() { everyRuleOfAnotherLanguageHasAnEntry(grammar::Language::Spanish, QStringLiteral("es-"), spanishRuleCases()); }
    void spanishKindsFollowTheChoice()
    {
        kindsOfAnotherLanguage(grammar::Language::Spanish, U("Salió en base a la ley."), U("Pienso de que sí."), GrammarIssue::Grammar);
    }
    void spanishDetails()
    {
        const grammar::Language es = grammar::Language::Spanish;
        // A question or an exclamation with no opening mark: the first word of the sentence gets it; a sentence with a comma before the mark is only told.
        QCOMPARE(fixedBy(U("Hola. Cómo estás? Bien. Qué bien!"), QStringLiteral("es-inverted-marks"), es), U("Hola. ¿Cómo estás? Bien. ¡Qué bien!"));
        const auto told = byRule(U("Hola, cómo estás?"), QStringLiteral("es-inverted-marks"), es);
        QCOMPARE(told.size(), 1);
        QVERIFY(told.first().suggestions.isEmpty());
        QCOMPARE(U("Hola, cómo estás?").mid(told.first().start, told.first().end - told.first().start), U("?"));
        QVERIFY(byRule(U("Sr. García, ¿vienes? ¡Claro! Dime ¿qué quieres?"), QStringLiteral("es-inverted-marks"), es).isEmpty());
        QVERIFY(byRule(U("Es de EE. UU. ¿Y tú?"), QStringLiteral("es-inverted-marks"), es).isEmpty());
        QCOMPARE(fixedBy(U("¿Porque no vienes? ¿Por que no? ¿Y donde está?"), QStringLiteral("es-question-accent"), es), U("¿Por qué no vienes? ¿Por qué no? ¿Y dónde está?"));
        QCOMPARE(fixedBy(U("Puede a ver problemas, haber si vienes."), QStringLiteral("es-a-ver"), es), U("Puede haber problemas, a ver si vienes."));
        QCOMPARE(fixedBy(U("Por hay no, y no ay que ir."), QStringLiteral("es-hay-ahi-ay"), es), U("Por ahí no, y no hay que ir."));
        QCOMPARE(fixedBy(U("Me alegro que vengas, y me di cuenta que era tarde."), QStringLiteral("es-de-que"), es), U("Me alegro de que vengas, y me di cuenta de que era tarde."));
        QCOMPARE(fixedBy(U("Viene de el sur y va a el norte."), QStringLiteral("es-del-al"), es), U("Viene del sur y va al norte."));
        const auto echo = byRule(U("Ha echo mucho."), QStringLiteral("es-hecho"), es);
        QCOMPARE(echo.first().suggestions, (QStringList{U("hecho"), U("echado")}));
        QVERIFY(!byRule(U("Esto es bueno , sí."), QStringLiteral("space-before-mark"), es).isEmpty());
    }
    void frenchOfCanadaHasNoSpaceBeforeSemicolonExclamationAndQuestionMark()
    {
        const grammar::Language ca = grammar::Language::FrenchCanada, fr = grammar::Language::French;
        const QString rule = QStringLiteral("fr-space-before-mark");
        QVERIFY(byRule(U("Bonjour!"), rule, ca).isEmpty());
        QVERIFY(byRule(U("Quoi?"), rule, ca).isEmpty());
        QVERIFY(byRule(U("Quoi ? Oui ; non !"), rule, ca).isEmpty());
        QCOMPARE(fixedBy(U("Note: texte"), rule, ca), U("Note\u00A0: texte"));
        QCOMPARE(fixedBy(U("Il dit «bonjour»."), QStringLiteral("fr-guillemets"), ca), U("Il dit «\u00A0bonjour\u00A0»."));
        // France (and a French with no country) has them all.
        QVERIFY(!byRule(U("Bonjour!"), rule, fr).isEmpty());
        QVERIFY(!byRule(U("Quoi?"), rule, fr).isEmpty());
        QCOMPARE(grammar::rules(ca).size(), grammar::rules(fr).size());
    }
    void aTextIsCheckedByTheRulesOfItsLanguageOnly()
    {
        // The rules of French do not run on English (and the other way round): the text of one language is not flagged by another's rules.
        QVERIFY(!grammar::check(U("Le ami de la école."), nullptr, grammar::kAllKinds, grammar::Language::French).isEmpty());
        QVERIFY(grammar::check(U("Le ami de la école."), nullptr, grammar::kAllKinds, grammar::Language::English).isEmpty());
        QVERIFY(grammar::check(U("We went to see the mountains, and a fine day."), nullptr, grammar::kAllKinds, grammar::Language::French).isEmpty());
        QVERIFY(!grammar::check(U("He said the the thing."), nullptr, grammar::kAllKinds, grammar::Language::English).isEmpty());
        QCOMPARE(grammar::check(U("Il fait beau.")).size(), 0);
    }

    void styleRulesAreOffUnlessAskedFor()
    {
        const QString text = QStringLiteral("We did this in order to win, and we could care less.");
        QVERIFY(grammar::check(text).isEmpty());
        QVERIFY(!grammar::check(text, nullptr, grammar::kAllKinds).isEmpty());
        QCOMPARE(grammar::check(text, nullptr, grammar::kindBit(GrammarIssue::Style)).size(), 2);
        // A kind that is off is not looked for: the other rules still are.
        const QString mixed = QStringLiteral("He don't go alot in order to win.");
        QCOMPARE(grammar::check(mixed, nullptr, grammar::kindBit(GrammarIssue::Grammar)).size(), 1);
        QCOMPARE(grammar::check(mixed, nullptr, grammar::kindBit(GrammarIssue::WordChoice)).size(), 1);
        QCOMPARE(grammar::check(mixed).size(), 2);
    }
    void anAAnBeforeAbbreviationsFollowsTheLetters()
    {
        fixes({{"He is a FBI agent.", "He is an FBI agent."}, {"We saw an UFO.", "We saw a UFO."}});
        leavesAlone({QStringLiteral("He is an FBI agent."), QStringLiteral("We saw a UFO."), QStringLiteral("An MBA and a PhD."), QStringLiteral("A NASA engineer and an HTML page.")});
    }
    void aSentenceOfFortyWordsIsNotLongButFortyOneIs()
    {
        const QString forty = QStringLiteral("word ").repeated(40).trimmed() + QStringLiteral(".");
        QVERIFY(byRule(forty, QStringLiteral("long-sentence")).isEmpty());
        QCOMPARE(byRule(QStringLiteral("Word ") + forty, QStringLiteral("long-sentence")).size(), 1);
    }

    void theWorkOfACheckGrowsInLineWithTheText()
    {
        const QString unit = QStringLiteral("This is an test of the the checker, and alot of people could of come.  Its a good day , isn't it?No. ");
        quint64 one = 0, many = 0, huge = 0;
        grammar::check(unit.repeated(1000), &one);
        grammar::check(unit.repeated(4000), &many);
        grammar::check(unit.repeated(16000), &huge);   // (1.7 million characters: three hundred pages of text are about a tenth of it)
        QVERIFY(one > 0);
        // Four times the text, four times the steps (within a tenth): no step looks back over the text.
        QVERIFY2(many <= one * 4 + one / 10, qPrintable(QString::number(one) + QLatin1Char(' ') + QString::number(many)));
        QVERIFY2(huge <= many * 4 + many / 10, qPrintable(QString::number(many) + QLatin1Char(' ') + QString::number(huge)));
        // And a bounded number of steps for each character.
        QVERIFY2(huge <= quint64(unit.size()) * 16000 * 150, qPrintable(QString::number(huge)));
        QVERIFY(grammar::check(unit.repeated(16000)).size() > 0);
    }

    // Shapes that make a rule look back or ahead over the whole paragraph for every word: a text of any of them, 100,000 characters long, must be counted in linear steps AND finish in well under a second
    // (work that is not counted, such as a scan inside a loop, shows only on the clock).
    static QVector<grammar::Language> allLanguages() { return {grammar::Language::English, grammar::Language::French, grammar::Language::German, grammar::Language::Spanish}; }
    static QStringList hostileShapes()
    {
        QStringList out;
        for (const char *unit : {"abcd. efgh. ", "abcd,abcd,", "Mr. a. b. ", "a  b  ", "x. ", "\"\"\"\"", "an a an ", "he don't ", "It's a, ", "Texas, Texas, ", "monday ", "i think ", ". . . ",
                                 "well known ", "in order to ", "The cat. The cat. ", "(1) (2) ", "A. B. C. ", "1, 2, 3, ", "June 5, 2020 we ", "( ( ( ", "in Austin, Texas the ", "the the ", "Dougl. ex Torr. ", "a.B c.D ",
                                 "it have and ", "Me and him went ", "can't hardly ", "e.g. i.e. ",
                                 // French
                                 "le ami de la école ", "de le ", "il ce ", "mot ; ", "mot: x ", "« a ", "a » ", "qu'est ce que ", "si il ", "il à ", "à les ", "ca va ", "quand à lui ", "c'est à dire, ",
                                 "x ? ", "x !! ", "l'a l'a ", "Ou est ", "d'ou ", "nous nous ",
                                 // German and Spanish
                                 "ihr seit ", "seid zwei ", ", das er ", "weil ", "Dass ist ", "das haus ", "größer wie ", "z.B. ", "wider mal ", "hola? ", "¡hola ", "¿que ", "¿ ", "haber si ", "de el ", "ahí que ",
                                 "pienso de que ", "ha echo ", "me di cuenta que ", "a ver ", "? ", "! ", "¿a? ¡b! "})
            out << QString::fromUtf8(unit).repeated(100000 / int(strlen(unit)));
        return out;
    }
    void hostileTextsAreCheckedInLinearWork()
    {
        QStringList texts = hostileShapes();
        for (const QString &t : {QString(200000, QLatin1Char(' ')), QString(200000, QLatin1Char('.')), QString(QStringLiteral("a ")).repeated(100000), QString(QStringLiteral(". a")).repeated(70000), QString(QStringLiteral("a:")).repeated(100000), QString(QStringLiteral("\u00AB ")).repeated(100000),
                                 QString(QStringLiteral("the ")).repeated(50000), QString(QStringLiteral("a 5 ")).repeated(50000), QString(QStringLiteral("a 12,500th ")).repeated(20000), QString(QStringLiteral(" , ")).repeated(60000), QString(200000, QChar(0xD83D))})
            texts << t;
        for (const grammar::Language language : allLanguages())
            for (const QString &text : texts) {
                quint64 work = 0;
                grammar::check(text, &work, grammar::kAllKinds, language);
                QVERIFY2(work <= quint64(text.size()) * 150 + 1000, qPrintable(QString::number(work) + QLatin1Char(' ') + QString::number(text.size()) + QLatin1Char(' ') + text.left(20)));
            }
    }
    void aParagraphOfAnyHostileShapeIsCheckedInUnderASecond()
    {
        for (const grammar::Language language : allLanguages()) {
            grammar::check(QStringLiteral("Warm up."), nullptr, grammar::kDefaultKinds, language);
            for (const QString &text : hostileShapes()) {
                QElapsedTimer t;
                t.start();
                grammar::check(text, nullptr, grammar::kAllKinds, language);
                QVERIFY2(t.elapsed() < 1000, qPrintable(QString::number(t.elapsed()) + QStringLiteral(" ms: ") + text.left(20)));
            }
        }
    }

    // The word lists the rules use are made on first use, which many threads can reach at once (the Review pane checks on a worker while another window does): in a program of its own, whose first use
    // it is, several threads check text that reaches the lists together, and all get the same answer. (Run again by this test with JO_GRAMMAR_WORKER set; the work is firstUseByManyThreadsWorker.)
    void firstUseByManyThreadsWorker()
    {
        if (qgetenv("JO_GRAMMAR_WORKER").isEmpty()) QSKIP("run by firstUseByManyThreadsIsSafe");
        const QString text = QStringLiteral("Your a fool. It was a very big the the mistake, and he could of gone. Its a nice day.Do not stop. Mr. Smith vs. Jones etc. a apple. "
                                            "She will loose weight, and I would of known. Your not wrong. There going to there house.");
        std::vector<std::thread> threads;
        std::vector<size_t> counts(16, 0);
        std::atomic<bool> go(false);
        for (int t = 0; t < 16; ++t)
            threads.emplace_back([&, t] {
                while (!go.load()) {}   // (all begin together)
                counts[size_t(t)] = grammar::check(text).size();
            });
        go = true;
        for (std::thread &th : threads) th.join();
        for (size_t t = 1; t < counts.size(); ++t) QCOMPARE(counts[t], counts[0]);
        QVERIFY(counts[0] > 0);
    }
    void firstUseByManyThreadsIsSafe()
    {
        if (!qgetenv("JO_GRAMMAR_WORKER").isEmpty()) QSKIP("the worker run");
        for (int round = 0; round < 150; ++round) {
            QProcess p;
            p.setProcessEnvironment([] {
                QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
                e.insert(QStringLiteral("JO_GRAMMAR_WORKER"), QStringLiteral("1"));
                return e;
            }());
            p.start(QCoreApplication::applicationFilePath(), {QStringLiteral("firstUseByManyThreadsWorker")});
            QVERIFY(p.waitForFinished(60000));
            QVERIFY2(p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0, qPrintable(QString::fromLocal8Bit(p.readAllStandardOutput().right(400)) + QString::fromLocal8Bit(p.readAllStandardError().right(400))));
        }
    }
};

QVector<ShellGrammarRulesTests::RuleCase> ShellGrammarRulesTests::newRuleCases()
{
    const QString forty1 = QStringLiteral("Word ").repeated(41).trimmed() + QStringLiteral(".");
    static QByteArray keep = forty1.toUtf8();   // (the cases hold their text as C strings)
    return {
        {"he-dont", "She don't know.", "She doesn't know.", L({"I don't know.", "They don't know.", "He said \"she don't\" is wrong.", "Don't let it die."})},
        {"they-was", "They was late.", "They were late.", L({"They were late.", "What I like about you was your humor.", "I was late and he was too.", "He said \"they was\" in the play."})},
        {"she-have", "It rained and she have no coat.", "It rained and she has no coat.", L({"Does she have a coat?", "She has a coat.", "It is vital that he have a coat.", "They have it."})},
        {"plural-has", "We has a plan.", "We have a plan.", L({"We have a plan.", "Someone like you has a plan.", "He has a plan.", "Has anyone seen us?", "The ones we have are good."})},
        {"plural-does", "They does it well.", "They do it well.", L({"They do it well.", "Someone like you does it well.", "She does it well.", "Does the team win?"})},
        {"they-is", "They is ready.", "They are ready.", L({"They are ready.", "All I am telling you is that it is ready.", "This is ready.", "He is ready."})},
        {"there-is-many", "There is many reasons.", "There are many reasons.", L({"There are many reasons.", "There is many a slip.", "There is one reason.", "There was a man."})},
        {"indefinite-are", "Everyone are here.", "Everyone is here.", L({"Everyone is here.", "Everyone, are you here?", "They are here.", "If anyone were here."})},
        {"whos-whose", "I know who's car it is.", "I know whose car it is.", L({"I know who's coming.", "I know whose car it is.", "Who's there?", "The man whose car it is."})},
        {"affect-effect", "It had an affect on me.", "It had an effect on me.", L({"It had an effect on me.", "It will affect me.", "It will affect the result.", "The affect of the patient was flat and calm.", "It had no affect of that kind."})},
        {"less-fewer", "We had less people.", "We had fewer people.", L({"We had fewer people.", "We had less water.", "It cost less than ten dollars.", "She was less happy."})},
        {"much-many", "We had much people.", "We had many people.", L({"We had many people.", "We had much rain.", "Much more people came, yes.", "It is much better."})},
        {"irregardless", "Irregardless of cost, go.", "Regardless of cost, go.", L({"Regardless of cost, go.", "Regards to all.", "He regarded it.", "Regardless, go."})},
        {"whether-weather", "I don't know weather it works.", "I don't know whether it works.", L({"I don't know whether it works.", "The weather is fine.", "Check the weather report.", "Good weather or not, we go."})},
        {"quite-quiet", "We saw quiet a few cars.", "We saw quite a few cars.", L({"We saw quite a few cars.", "Keep quiet a lot of the time.", "It was a quiet night.", "Be quiet a minute."})},
        {"chose-choose", "You can chose one.", "You can choose one.", L({"You can choose one.", "She chose one yesterday.", "He chose to go.", "Which chose the best?"})},
        {"advice-advise", "I need some advise.", "I need some advice.", L({"I need some advice.", "I advise you to wait.", "My advice is to wait.", "They advise caution."})},
        {"to-much", "That is to much for me.", "That is too much for me.", L({"That is too much for me.", "It will never amount to much.", "She gave to much of her time.", "From much to little."})},
        {"too-to", "I want too go.", "I want to go.", L({"I want to go.", "I want it too.", "It is too far to go.", "I like you too much."})},
        {"breath-breathe", "I can't breath in here.", "I can't breathe in here.", L({"I can't breathe in here.", "Take a deep breath.", "Breath by breath, he walked.", "She held her breath."})},
        {"run-together", "Infact, it works.", "In fact, it works.", L({"In fact, it works.", "Each other and at least one.", "See noone.com for details.", "A little bit."})},
        {"idioms", "That is a mute point.", "That is a moot point.", L({"That is a moot point.", "The mute button.", "A moot court.", "Point taken."})},
        {"demonstrative-number", "These thing is odd.", "This thing is odd.", L({"This thing is odd.", "These things are odd.", "This years program ran.", "These day trips are fun."})},
        {"between-you-and-i", "This is between you and I.", "This is between you and me.", L({"This is between you and me.", "You and I will go.", "Between you and I am a wall.", "He gave it to you and me."})},
        {"me-and-him", "Me and him went home.", "He and I went home.", L({"He and I went home.", "Me and my dog went home.", "Between me and him is a wall.", "Give me and him a chance."})},
        {"double-comparative", "It is more better now.", "It is better now.", L({"It is better now.", "It is more beautiful now.", "More, better, faster.", "It is more than better."})},
        {"modal-past", "She didn't went home.", "She didn't go home.", L({"She didn't go home.", "She went home.", "The tin can went missing.", "The will went to court."})},
        {"have-past", "He has went home.", "He has gone home.", L({"He has gone home.", "He went home.", "They had saw dust everywhere.", "He has rose petals."})},
        {"double-negative", "I can't hardly see.", "I can hardly see.", L({"I can hardly see.", "I can't see well.", "I don't have any money.", "No one can see."})},
        {"comma-after-year", "On June 5, 2020 we left.", "On June 5, 2020, we left.", L({"On June 5, 2020, we left.", "June 5, 2020 was a Friday.", "On June 5, 2020 in Boston.", "In June 2020 we left."})},
        {"comma-after-state", "In Austin, Texas the heat is high.", "In Austin, Texas, the heat is high.", L({"In Austin, Texas, the heat is high.", "Austin, Texas is hot.", "I met Smith, Texas and Jones.", "In Austin, Texas and Ohio.", "In Texas, Florida has a similar statute.", "From Maine, Vermont is next.", "In March, Texas is dry.", "Although it rained, Texas is dry."})},
        {"compound-hyphen", "He is a well known author.", "He is a well-known author.", L({"He is a well-known author.", "He is well known for it.", "A long term of office.", "A full time was set."})},
        {"comma-space", "It works,but not well.", "It works, but not well.", L({"It works, but not well.", "Open a,b,c,d now.", "See 1,000 items.", "Items: red,green,blue,tan."})},
        {"doubled-mark", "It works,, I think.", "It works, I think.", L({"It works, I think.", "Wait... what?", "Mr.. is short.", "Name,,Age,,City"})},
        {"your-welcome", "Thanks. Your welcome.", "Thanks. You're welcome.", L({"Thanks. You're welcome.", "Thank you for your welcome.", "Your welcome message is ready.", "Your welcome to join us"})},
        {"lowercase-i", "Today i think so.", "Today I think so.", L({"Today I think so.", "Let i be the index.", "for i in the list", "Chapter i is short.", "See section i", "Part (i) is short.", "Read item i. Then stop."})},
        {"day-month-names", "We meet on monday.", "We meet on Monday.", L({"We meet on Monday.", "See monday.com for it.", "We may march in may.", "A sunday-best suit."})},
        {"amount-number", "The amount of people grew.", "The number of people grew.", L({"The number of people grew.", "The amount of water grew.", "The amount of work grew.", "A large amount of money."})},
        {"could-care-less", "I could care less.", "I couldn't care less.", L({"I couldn't care less.", "I could care for it.", "He could care a lot.", "We care less now."})},
        {"try-and", "Try and see.", "Try to see.", L({"Try to see.", "A tried and true plan.", "Try and Catch blocks.", "We try hard."})},
        {"in-order-to", "We left in order to win.", "We left to win.", L({"We left to win.", "In order, the steps run.", "Put them in order.", "A court order to pay."})},
        {"wordy-phrases", "Due to the fact that it rained, we left.", "Because it rained, we left.", L({"Because it rained, we left.", "Due to rain, we left.", "In fact it rained.", "The fact that it rained."})},
        {"redundant-phrases", "It was a free gift.", "It was a gift.", L({"It was a gift.", "It was free.", "A free ride.", "The gift shop."})},
        {"passive-voice", "The book was written by Ann.", nullptr, L({"Ann wrote the book.", "The book was long.", "He was tired.", "It was by the door."}), "was written", "passive"},
        {"long-sentence", keep.constData(), nullptr, L({"Word word.", "Short one. Another short one.", "It is fine.", "Yes."}), "Word", "41 words"},
        {"repeated-sentence-start", "The cat ran. The dog ran. The bird ran.", nullptr, L({"The cat ran. The dog ran.", "The cat ran. A dog ran. The bird ran.", "We ran. They ran. We ran.", "One. Two. Three."}), "The", "Three sentences"},
    };
}

QVector<ShellGrammarRulesTests::RuleCase> ShellGrammarRulesTests::frenchRuleCases()
{
#define FRC(rule, bad, fixed, ...) {rule, bad, fixed, L({__VA_ARGS__}), nullptr, nullptr, grammar::Language::French}
    return {
        FRC("fr-repeated-word", "Il faut le le faire.", "Il faut le faire.", "Nous nous lavons.", "Vous vous trompez.", "Il dit oui oui.", "C'est très très bon.", "Bora Bora est belle."),
        FRC("fr-space-before-mark", "Que voulez-vous ? Rien.", "Que voulez-vous\u00A0? Rien.",
            "Que voulez-vous\u00A0? Rien.", "Que voulez-vous\u202F? Rien.", "Rendez-vous à 10:30 à Paris.", "Voir http://exemple.fr/page?id=3 pour plus.", "Bonjour :) à tous.", "Soit f : X vers Y."),
        FRC("fr-guillemets", "Il dit «bonjour» à tous.", "Il dit «\u00A0bonjour\u00A0» à tous.", "Il dit «\u00A0bonjour\u00A0» à tous.", "Il dit «\u202Fbonjour\u202F» à tous.", "Il dit bonjour à tous."),
        FRC("fr-elision", "Le ami de la école est ici.", "L'ami de l'école est ici.",
            "Le onze mai, la une du journal.", "Le haricot et le héros.", "Elle parle de un à dix.", "Le Ami est ici.", "Le yaourt est bon.", "Elle y va.", "Les valeurs de y sont connues.", "Une langue tel que en, es ou pt."),
        FRC("fr-si-il", "Si il pleut, nous restons.", "S'il pleut, nous restons.", "S'il pleut, nous restons.", "Si elle pleure, partez.", "Si on y va, il vient.", "Si les enfants partent, il reste."),
        FRC("fr-a-accent", "Il à mangé tout le gâteau.", "Il a mangé tout le gâteau.", "Il a mangé tout le gâteau.", "Il va à Paris demain.", "Elle donne le livre à Marie.", "À partir de demain, c'est fermé."),
        FRC("fr-ou-accent", "Ou est la gare ?", "Où est la gare ?", "Où est la gare ?", "Tu viens ou tu restes ?", "Ou bien tu restes.", "Du thé ou du café ?"),
        FRC("fr-ce-se", "Il ce lève tôt.", "Il se lève tôt.", "Il se lève tôt.", "Pour lui ce n'est pas facile.", "Il sait ce qu'il veut.", "Pour elle ce film est long."),
        FRC("fr-contraction", "Il parle de le chat.", "Il parle du chat.", "Il a décidé de le voir.", "Il parle du chat.", "Il pense à les voir.", "Il pense à le dire demain.", "Il à le droit de venir."),
        FRC("fr-ca", "Ca va bien.", "Ça va bien.", "Ça va bien.", "Voir ca. 1950.", "Sa voiture va vite.", "Son cas est clair."),
        FRC("fr-quant", "Quand à lui, il reste.", "Quant à lui, il reste.", "Quant à lui, il reste.", "Quand il arrive, nous partons.", "Il sourit quand à la plage."),
        FRC("fr-hyphens", "Qu'est ce que tu fais ?", "Qu'est-ce que tu fais ?", "Qu'est-ce que tu fais ?", "Ce n'est pas ce que tu crois.", "C'est-à-dire que oui.", "Il sait qu'est-ce que c'est."),
        FRC("fr-sentence-start", "Il pleut. nous restons.", "Il pleut. Nous restons.", "Voir l'art. premier du code.", "Il pleut. Nous restons.", "Voir env. dix pages.", "Il pleut, etc. et nous restons.", "Les noms _xlfn. sont réservés.", "Il pleut. p2s3 est une page."),
        FRC("fr-wordy-phrases", "Il vient malgré que la pluie tombe.", "Il vient bien que la pluie tombe.", "Il vient bien que la pluie tombe.", "Il vient malgré la pluie.", "Voire plus encore.", "La suite est claire."),
    };
#undef FRC
}

QVector<ShellGrammarRulesTests::RuleCase> ShellGrammarRulesTests::germanRuleCases()
{
#define DEC(rule, bad, fixed, ...) {rule, bad, fixed, L({__VA_ARGS__}), nullptr, nullptr, grammar::Language::German}
    return {
        DEC("de-repeated-word", "Wir gehen in in die Stadt.", "Wir gehen in die Stadt.", "Ich glaube, dass das das Problem ist.", "Die, die kommen, sind da.", "Sehr sehr gut.", "Bora Bora ist schön."),
        DEC("de-das-dass", "Ich glaube, das er kommt.", "Ich glaube, dass er kommt.", "Das Haus, das er baute, ist groß.", "Ich glaube, dass er kommt.", "Das ist gut.", "Es ist das Beste, das man kennt."),
        DEC("de-seid-seit", "Ich bin seid zwei Jahren hier.", "Ich bin seit zwei Jahren hier.", "Ihr seid nicht allein.", "Seit zwei Jahren sind wir hier.", "Ihr seid gestern gekommen.", "Er hilft ihr seit Jahren."),
        DEC("de-wider-wieder", "Er kommt wider mal.", "Er kommt wieder mal.", "Das geschah wider Willen.", "Wir sehen uns wieder.", "Hin und wider regnet es.", "Er sagte es wider besseres Wissen."),
        DEC("de-comma-before-conjunction", "Ich weiß dass du kommst.", "Ich weiß, dass du kommst.", "Ich weiß, dass du kommst.", "Er sieht aus, als ob er schliefe.", "Er kam nur weil er musste.", "Er blieb ohne dass wir es merkten.",
            "Sie fragte und weil er schwieg ging sie."),
        DEC("de-noun-capital", "Das haus ist groß.", "Das Haus ist groß.", "Das Haus ist groß.", "Wir essen das essen nicht.", "Das ist weg und der morgen kommt.", "Er hat im Haus gewohnt."),
        DEC("de-als-wie", "Er ist größer wie ich.", "Er ist größer als ich.", "Er ist größer als ich.", "Er ist so groß wie ich.", "Er ist nicht größer wie ein Haus, sondern gleich.", "Es geht weiter wie bisher."),
        DEC("de-run-together", "Das ist garnicht gut.", "Das ist gar nicht gut.", "Das ist gar nicht gut.", "Das ist nicht gut.", "Vor allem im Winter.", "Noch einmal bitte."),
        DEC("de-sentence-start", "Es regnet. wir bleiben hier.", "Es regnet. Wir bleiben hier.", "Es regnet. Wir bleiben hier.", "Das gilt usw. und so weiter.", "Siehe vgl. oben.", "Er kam am 3. oktober an."),
        DEC("de-wordy-phrases", "Wir bleiben nach wie vor hier.", "Wir bleiben weiterhin hier.", "Wir bleiben weiterhin hier.", "Wie vor hundert Jahren.", "Nach dem Essen kam er.", "Wir bleiben hier."),
        DEC("de-abbreviation-space", "Das gilt z.B. hier.", "Das gilt z.\u00A0B. hier.", "Das gilt z.\u00A0B. hier.", "Das gilt z. B. hier.", "Die Datei a.b.c liegt dort.", "Das gilt zum Beispiel hier."),
    };
#undef DEC
}

QVector<ShellGrammarRulesTests::RuleCase> ShellGrammarRulesTests::spanishRuleCases()
{
#define ESC(rule, bad, fixed, ...) {rule, bad, fixed, L({__VA_ARGS__}), nullptr, nullptr, grammar::Language::Spanish}
    return {
        ESC("es-repeated-word", "Es el el libro.", "Es el libro.", "Dijo no no y se fue.", "Es muy muy bueno.", "Poco a poco.", "Bora Bora es bonita.", "La ventana en la que está está vacía.", "La página Acerca de de cada aplicación."),
        ESC("es-inverted-marks", "Cómo estás?", "¿Cómo estás?", "¿Cómo estás?", "¡Hola!", "Dime, ¿cómo estás?", "Yahoo! es una empresa.", "El valor (?) es incierto.", "Sr. García, ¿vienes?"),
        ESC("es-question-accent", "¿Como estás?", "¿Cómo estás?", "¿Cómo estás?", "Como tú dices.", "¿Que no vienes?", "¿Quieres que vayamos?", "No sé donde está."),
        ESC("es-a-ver", "Haber si vienes.", "A ver si vienes.", "Va a ver la casa.", "Voy a ver si viene.", "Puede haber si hay tiempo.", "A ver si vienes."),
        ESC("es-hay-ahi-ay", "Ahí muchas personas.", "Hay muchas personas.", "Hay muchas personas.", "Ahí mismo.", "Ven ahí.", "¡Ay, un bicho!", "De ahí que lo hiciera."),
        ESC("es-del-al", "Voy a el parque.", "Voy al parque.", "Voy al parque.", "Viene de El Salvador.", "Habla a él de eso.", "Es de el que hablo.", "Pon &A el nombre de la hoja."),
        ESC("es-de-que", "Pienso de que es verdad.", "Pienso que es verdad.", "Pienso que es verdad.", "Me alegro de que vengas.", "Hablamos de que viene.", "Me di cuenta de que era tarde."),
        ESC("es-hecho", "Ha echo mucho.", "Ha hecho mucho.", "Ha hecho mucho.", "Yo echo sal.", "He echado sal.", "Echo de menos."),
        ESC("es-sentence-start", "Llueve. vamos a casa.", "Llueve. Vamos a casa.", "Véase pág. siguiente.", "Llueve, etc. y vamos a casa.", "Llueve. Vamos a casa.", "Ver art. primero."),
        ESC("es-wordy-phrases", "Salió en base a la ley.", "Salió con base en la ley.", "Salió con base en la ley.", "La base de la ley.", "Salió a la calle."),
    };
#undef ESC
}

JP_TEST_CLASS(ShellGrammarRulesTests)
#include "tst_shell_grammar_rules.moc"

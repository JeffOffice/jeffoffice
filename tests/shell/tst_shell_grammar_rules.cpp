// The grammar rules of libs/text/grammar, one group of tests for each rule: text with the mistake is flagged where it is and fixed by its first suggestion, and text that is
// correct (the legitimate look-alikes above all) is left alone. Also that the work of a check grows in line with the text.
#include "grammar/grammarrules.h"
#include "../testrunner.h"

#include <QtTest>

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
                     QStringLiteral("Line one one more."), QStringLiteral("1 1 2 3 5 8"), QStringLiteral("no no no"), QStringLiteral("The cell is put in in time."), QStringLiteral("It went on on Monday.")});
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
                     QStringLiteral("An ugly duckling, a utensil, an umbrella, a useful tool."), QStringLiteral("A BIG DEAL FOR AN OLD MAN"), QStringLiteral("Read a u32 and an x86 chip, a h264 file."), QStringLiteral("An 0x88 record and an 03 value."),
                     QStringLiteral("A usable tool and an unusable one, a usage count.")});
    }

    void aSentenceThatStartsInLowercaseIsFlagged()
    {
        fixes({{"It rained. we stayed in.", "It rained. We stayed in."}, {"Is it late? yes it is.", "Is it late? Yes it is."}, {"what a day it was.", "What a day it was."},
               {"Stop! that hurts.", "Stop! That hurts."}});
    }
    void lowercaseAfterAbbreviationsNumbersAndNamesIsLeftAlone()
    {
        leavesAlone({QStringLiteral("See e.g. the list, i.e. the long one."), QStringLiteral("Cats, dogs, etc. are pets."), QStringLiteral("Dr. smith and Mr. jones came."), QStringLiteral("It was 3.5 percent."),
                     QStringLiteral("Open report.docx and read it."), QStringLiteral("Visit www.example.com or example.org today."), QStringLiteral("Wait... then go."),
                     QStringLiteral("\"Is it late?\" she asked."), QStringLiteral("iPhone sales rose. eBay did too."), QStringLiteral("At 5 p.m. we left."), QStringLiteral("Chapter 1. introduction"),
                     QStringLiteral("good morning"), QStringLiteral("and then he left, so we did"), QStringLiteral("vs. the rest of the world")});
    }

    void itsAndItIsAreToldApart()
    {
        fixes({{"Its a good day.", "It's a good day."}, {"I think its not ready.", "I think it's not ready."}, {"Its been a year.", "It's been a year."}, {"The dog has it's own bed.", "The dog has its own bed."},
               {"Its the best.", "It's the best."}});
    }
    void itsAndItIsCorrectUseIsLeftAlone()
    {
        leavesAlone({QStringLiteral("The dog wagged its tail."), QStringLiteral("It's a good day."), QStringLiteral("Its own merits and its time have come."), QStringLiteral("It's been fun, and its end is near.")});
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
                     QStringLiteral("Your own name and you're sure.")});
    }

    void thenAndThanAreToldApart()
    {
        fixes({{"It is better then that.", "It is better than that."}, {"More then enough.", "More than enough."}, {"Rather then wait, go.", "Rather than wait, go."},
               {"First eat and than sleep.", "First eat and then sleep."}});
    }
    void thenAndThanCorrectUseIsLeftAlone()
    {
        leavesAlone({QStringLiteral("It is better than that."), QStringLiteral("Add more, then stir."), QStringLiteral("Eat, and then sleep."), QStringLiteral("If so, then go."),
                     QStringLiteral("Bigger than before, and then some.")});
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
        fixes({{"One  two.", "One two."}, {"One   two   three.", "One two three."}, {"End.  Start.", "End. Start."}});
    }
    void singleSpacesAndIndentsAreLeftAlone()
    {
        leavesAlone({QStringLiteral("One two three."), QStringLiteral("  indented line"), QStringLiteral("Trailing  "), QStringLiteral("a\ttab\tseparated"), QStringLiteral("non  breaking")});
    }

    void aSpaceBeforePunctuationIsFlaggedAndTaken()
    {
        fixes({{"Hello , world.", "Hello, world."}, {"Yes .", "Yes."}, {"Wait ; stop.", "Wait; stop."}, {"Why ?", "Why?"}, {"Look !", "Look!"}, {"Note : this.", "Note: this."}});
    }
    void spacesBeforeOtherMarksAreLeftAlone()
    {
        leavesAlone({QStringLiteral("Hello, world."), QStringLiteral("Wait ... what"), QStringLiteral("Smile :) and wink ;) ok"), QStringLiteral("See .txt files and 3 .5"), QStringLiteral("Ratio 3 : 1 and 4:5"),
                     QStringLiteral("Dash - and dash — fine"), QStringLiteral("( and ) and \" ok"), QStringLiteral("Wait … what")});
    }

    void aMissingSpaceAfterAPeriodIsFlagged()
    {
        fixes({{"It ended.The next began.", "It ended. The next began."}, {"Really?No way.", "Really? No way."}, {"Stop!Go now.", "Stop! Go now."}});
    }
    void periodsInNamesNumbersAndAddressesAreLeftAlone()
    {
        leavesAlone({QStringLiteral("Visit example.com or Node.js now."), QStringLiteral("See Ph.D.Smith and e.g.Mr. Jones."), QStringLiteral("U.S.Army and 3.14 and v2.Next."),
                     QStringLiteral("Mr.Smith came."), QStringLiteral("The sheet Data!MyName and Sheet1!A1 are cells."), QStringLiteral("Open http://example.com/Index.Html today."), QStringLiteral("Mail me@example.Com now."), QStringLiteral("file.Docx and A.B and report.Pdf")});
    }

    void alotIsFlaggedAndMadeALot()
    {
        fixes({{"I like it alot.", "I like it a lot."}, {"Alot of people came.", "A lot of people came."}});
    }
    void everydayAndEveryDayAreToldApartInClearPatterns()
    {
        fixes({{"I go there almost everyday.", "I go there almost every day."}, {"Everyday I walk.", "Every day I walk."}, {"Everyday we eat.", "Every day we eat."},
               {"Those are the every day tools.", "Those are the everyday tools."}});
    }
    void alotLookAlikesAndOrdinaryEverydayAreLeftAlone()
    {
        leavesAlone({QStringLiteral("A lot of people and allot of time."), QStringLiteral("Everyday life and everyday items."), QStringLiteral("Every day I walk."), QStringLiteral("Every day items arrive."),
                     QStringLiteral("An everyday occurrence.")});
    }

    void rulesDoNotHideEachOther()
    {
        const QString text = QStringLiteral("the the cat sat on an mat , and alot of people could of come.");
        QCOMPARE(fixed(text), QStringLiteral("The cat sat on a mat, and a lot of people could have come."));
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
        QVERIFY2(huge <= quint64(unit.size()) * 16000 * 40, qPrintable(QString::number(huge)));
        QVERIFY(grammar::check(unit.repeated(16000)).size() > 0);
    }

    void hostileTextsAreCheckedInLinearWork()
    {
        for (const QString &text : {QString(200000, QLatin1Char(' ')), QString(200000, QLatin1Char('.')), QString(QStringLiteral("a ")).repeated(100000), QString(QStringLiteral(". a")).repeated(70000),
                                    QString(QStringLiteral("the ")).repeated(50000), QString(QStringLiteral(" , ")).repeated(60000), QString(200000, QChar(0xD83D))}) {
            quint64 work = 0;
            grammar::check(text, &work);
            QVERIFY2(work <= quint64(text.size()) * 60 + 1000, qPrintable(QString::number(work) + QLatin1Char(' ') + QString::number(text.size())));
        }
    }
};

JP_TEST_CLASS(ShellGrammarRulesTests)
#include "tst_shell_grammar_rules.moc"

// Tests of the number format codes a person chooses (apps/crunch/core/formatcodes.h): the category of a code, the code a category's options make and the options a code has, the lists of
// ready-made formats, and what Increase Decimal and Decrease Decimal do to a code. Pure text; the languages are set explicitly.
#include "core/formatcodes.h"

#include <QtTest>

#include "../testrunner.h"

using namespace jp::crunch;

class CrunchFormatCodesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void theCategoryOfACode()
    {
        struct Case {
            const char *code;
            NumberCategory category;
        };
        const Case cases[] = {
            {"General", NumberCategory::General},        {"", NumberCategory::General},
            {"0", NumberCategory::Number},               {"0.00", NumberCategory::Number},
            {"#,##0", NumberCategory::Number},           {"#,##0.00;[Red]-#,##0.00", NumberCategory::Number},
            {"\"$\"#,##0.00", NumberCategory::Currency}, {"#,##0.00\\ \"\xe2\x82\xac\"", NumberCategory::Currency},
            {"[$$-409]#,##0.00", NumberCategory::Currency},
            {"_(\"$\"* #,##0.00_);_(\"$\"* \\(#,##0.00\\);_(\"$\"* \"-\"??_);_(@_)", NumberCategory::Accounting},
            {"_(* #,##0.00_);_(* \\(#,##0.00\\);_(* \"-\"??_);_(@_)", NumberCategory::Accounting},
            {"m/d/yyyy", NumberCategory::Date},          {"d-mmm-yy", NumberCategory::Date},
            {"dddd, mmmm d, yyyy", NumberCategory::Date}, {"mmm-yy", NumberCategory::Date},
            {"m/d/yyyy h:mm", NumberCategory::Date},
            {"h:mm", NumberCategory::Time},              {"h:mm:ss AM/PM", NumberCategory::Time},
            {"[h]:mm:ss", NumberCategory::Time},         {"mm:ss.0", NumberCategory::Time},
            {"0%", NumberCategory::Percentage},          {"0.00%", NumberCategory::Percentage},
            {"# ?/?", NumberCategory::Fraction},         {"# ??/16", NumberCategory::Fraction},
            {"0.00E+00", NumberCategory::Scientific},
            {"@", NumberCategory::Text},
            {"00000", NumberCategory::Special},          {"000\\-00\\-0000", NumberCategory::Special},
            {"\"Total: \"#,##0.00", NumberCategory::Custom}, {"[Red][<0]0;[Blue][>0]0;0", NumberCategory::Custom},
        };
        for (const Case &c : cases) QVERIFY2(categoryOf(QString::fromUtf8(c.code)) == c.category, c.code);
    }

    // The code a category's options make, and the options read back from it: for every combination the dialog offers.
    void optionsMakeCodesAndComeBack()
    {
        QCOMPARE(codeFor(NumberCategory::Number, {2, false, 0, QString(), true}), QStringLiteral("0.00"));
        QCOMPARE(codeFor(NumberCategory::Number, {0, true, 0, QString(), true}), QStringLiteral("#,##0"));
        QCOMPARE(codeFor(NumberCategory::Number, {2, true, 1, QString(), true}), QStringLiteral("#,##0.00;[Red]-#,##0.00"));
        QCOMPARE(codeFor(NumberCategory::Number, {1, false, 2, QString(), true}), QStringLiteral("0.0_);\\(0.0\\)"));
        QCOMPARE(codeFor(NumberCategory::Number, {1, false, 3, QString(), true}), QStringLiteral("0.0_);[Red]\\(0.0\\)"));
        QCOMPARE(codeFor(NumberCategory::Currency, {2, true, 0, QStringLiteral("$"), true}), QStringLiteral("\"$\"#,##0.00"));
        QCOMPARE(codeFor(NumberCategory::Currency, {2, true, 0, QString(QChar(0x20AC)), false}), QString::fromUtf8("#,##0.00\\ \"\xe2\x82\xac\""));
        QCOMPARE(codeFor(NumberCategory::Accounting, {2, true, 0, QStringLiteral("$"), true}), QStringLiteral("_(\"$\"* #,##0.00_);_(\"$\"* \\(#,##0.00\\);_(\"$\"* \"-\"??_);_(@_)"));
        QCOMPARE(codeFor(NumberCategory::Accounting, {0, true, 0, QString(), true}), QStringLiteral("_(* #,##0_);_(* \\(#,##0\\);_(* \"-\"_);_(@_)"));
        QCOMPARE(codeFor(NumberCategory::Percentage, {2, false, 0, QString(), true}), QStringLiteral("0.00%"));
        QCOMPARE(codeFor(NumberCategory::Scientific, {3, false, 0, QString(), true}), QStringLiteral("0.000E+00"));
        QCOMPARE(codeFor(NumberCategory::General, NumberOptions()), QStringLiteral("General"));
        QCOMPARE(codeFor(NumberCategory::Text, NumberOptions()), QStringLiteral("@"));
        // every combination comes back as it went
        const NumberCategory withNegatives[] = {NumberCategory::Number, NumberCategory::Currency};
        for (const NumberCategory category : withNegatives)
            for (int decimals = 0; decimals <= 4; ++decimals)
                for (const bool thousands : {false, true})
                    for (int negative = 0; negative < negativeStyles(); ++negative)
                        for (const bool first : {true, false}) {
                            NumberOptions o{decimals, thousands, negative, category == NumberCategory::Currency ? QStringLiteral("$") : QString(), first};
                            if (category == NumberCategory::Number && !first) continue;
                            const QString code = codeFor(category, o);
                            NumberCategory back = NumberCategory::Custom;
                            NumberOptions got;
                            QVERIFY2(optionsOf(code, &back, &got), qPrintable(code));
                            QVERIFY2(back == category, qPrintable(code));
                            QVERIFY2(got == o, qPrintable(code));
                        }
        for (int decimals = 0; decimals <= 4; ++decimals) {
            for (const NumberCategory category : {NumberCategory::Accounting, NumberCategory::Percentage, NumberCategory::Scientific}) {
                NumberOptions o;
                o.decimals = decimals;
                o.thousands = category == NumberCategory::Accounting;
                o.symbol = category == NumberCategory::Accounting ? QStringLiteral("$") : QString();
                const QString code = codeFor(category, o);
                NumberCategory back = NumberCategory::Custom;
                NumberOptions got;
                QVERIFY2(optionsOf(code, &back, &got) && back == category && got.decimals == decimals, qPrintable(code));
            }
        }
        // a code the dialog would not make is not one of its categories' options
        NumberCategory category = NumberCategory::General;
        NumberOptions o;
        QVERIFY(!optionsOf(QStringLiteral("0.00;[Blue]0.00"), &category, &o));
        QVERIFY(!optionsOf(QStringLiteral("\"Total: \"#,##0.00"), &category, &o));
        QVERIFY(category == NumberCategory::Custom);
    }

    void theListsOfReadyMadeFormats()
    {
        const QLocale us(QLocale::English, QLocale::UnitedStates), de(QLocale::German, QLocale::Germany);
        QCOMPARE(typesFor(NumberCategory::Date, us).first().code, QStringLiteral("M/d/yyyy").toLower());
        QCOMPARE(typesFor(NumberCategory::Date, de).first().code, QStringLiteral("dd\\.mm\\.yyyy"));
        for (const NumberCategory c : {NumberCategory::Date, NumberCategory::Time, NumberCategory::Fraction, NumberCategory::Special})
            for (const FormatType &t : typesFor(c, us)) QVERIFY2(categoryOf(t.code) == c || (c == NumberCategory::Date && categoryOf(t.code) == NumberCategory::Date), qPrintable(t.code));
        QVERIFY(typesFor(NumberCategory::Number, us).isEmpty());
        QVERIFY(typesFor(NumberCategory::Custom, us).size() > 10);
    }

    // Increase and Decrease Decimal: a place more or less in every part that shows a number; General begins at one place; a date or a text is left alone.
    void increaseAndDecreaseDecimal()
    {
        QCOMPARE(withDecimals(QStringLiteral("General"), 1), QStringLiteral("0.0"));
        QCOMPARE(withDecimals(QStringLiteral("0"), 1), QStringLiteral("0.0"));
        QCOMPARE(withDecimals(QStringLiteral("0.0"), 1), QStringLiteral("0.00"));
        QCOMPARE(withDecimals(QStringLiteral("0.00"), -1), QStringLiteral("0.0"));
        QCOMPARE(withDecimals(QStringLiteral("0.0"), -1), QStringLiteral("0"));
        QCOMPARE(withDecimals(QStringLiteral("0"), -1), QStringLiteral("0"));
        QCOMPARE(withDecimals(QStringLiteral("#,##0.00"), 1), QStringLiteral("#,##0.000"));
        QCOMPARE(withDecimals(QStringLiteral("0%"), 1), QStringLiteral("0.0%"));
        QCOMPARE(withDecimals(QStringLiteral("0.00%"), -1), QStringLiteral("0.0%"));
        QCOMPARE(withDecimals(QStringLiteral("0.00E+00"), 1), QStringLiteral("0.000E+00"));
        QCOMPARE(withDecimals(QStringLiteral("\"$\"#,##0.00"), -1), QStringLiteral("\"$\"#,##0.0"));
        QCOMPARE(withDecimals(QStringLiteral("0.00;[Red]-0.00"), 1), QStringLiteral("0.000;[Red]-0.000"));
        QCOMPARE(withDecimals(QStringLiteral("0.0_);\\(0.0\\)"), -1), QStringLiteral("0_);\\(0\\)"));
        QCOMPARE(withDecimals(QStringLiteral("m/d/yyyy"), 1), QStringLiteral("m/d/yyyy"));
        QCOMPARE(withDecimals(QStringLiteral("h:mm"), -1), QStringLiteral("h:mm"));
        QCOMPARE(withDecimals(QStringLiteral("@"), 1), QStringLiteral("@"));
        // an accounting code: the places in each of its parts, and the dash of a zero
        const QString two = commaCode();
        QCOMPARE(withDecimals(two, -1), QStringLiteral("_(* #,##0.0_);_(* \\(#,##0.0\\);_(* \"-\"??_);_(@_)"));
    }

    void theButtonsCodes()
    {
        QCOMPARE(percentCode(), QStringLiteral("0%"));
        QCOMPARE(commaCode(), QStringLiteral("_(* #,##0.00_);_(* \\(#,##0.00\\);_(* \"-\"??_);_(@_)"));
        QCOMPARE(currencyCode(QLocale(QLocale::English, QLocale::UnitedStates)), QStringLiteral("\"$\"#,##0.00"));
        const QString de = currencyCode(QLocale(QLocale::German, QLocale::Germany));
        QVERIFY2(de.contains(QString::fromUtf8("\xe2\x82\xac")) && de.endsWith(QLatin1Char('"')), qPrintable(de));   // (after the number)
        bool first = true;
        QCOMPARE(currencySymbolOf(QLocale(QLocale::Japanese, QLocale::Japan), &first).isEmpty(), false);
        QVERIFY(first);
    }
};

JP_TEST_CLASS(CrunchFormatCodesTests)
#include "tst_crunch_formatcodes.moc"

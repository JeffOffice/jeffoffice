# Finanzfunktionen
<!-- keywords: Funktionen, ACCRINT, ACCRINTM, AMORDEGRC, AMORLINC, COUPDAYBS, COUPDAYS, COUPDAYSNC, COUPNCD, COUPNUM, COUPPCD, CUMIPMT, CUMPRINC, DB, DDB, DISC, DOLLARDE, DOLLARFR, DURATION, EFFECT, FV, FVSCHEDULE, INTRATE, IPMT, IRR, ISPMT, MDURATION, MIRR, NOMINAL, NPER, NPV, ODDFPRICE, ODDFYIELD, ODDLPRICE, ODDLYIELD, PDURATION, PMT, PPMT, PRICE, PRICEDISC, PRICEMAT, PV, RATE, RECEIVED, RRI, SLN, SYD, TBILLEQ, TBILLPRICE, TBILLYIELD, VDB, XIRR, XNPV, YIELD, YIELDDISC, YIELDMAT, Finanzmathematik, Funktion, Formel, Abschreibung, Zinsen, Rendite, Kredit, Anleihe, Wertpapier -->

Dies sind die Finanzfunktionen, die JeffCrunch berechnen kann. Jede zeigt, wie sie geschrieben wird, was sie tut und was ihre Argumente bedeuten. Ein Argument in eckigen Klammern ist optional, und „...“ bedeutet, dass Sie beliebig viele weitere des letzten Arguments angeben können. Um eine Funktion auszuprobieren, lesen Sie [Funktion auswählen](insert-function). Während Sie eine Funktion in eine Zelle eingeben, zeigt ein kleiner Hinweis unter der Zelle ihre Argumente an. Ein Klick auf den Namen der Funktion im Hinweis öffnet ihren Eintrag hier.

## ACCRINT

`ACCRINT(issue, first_interest, settlement, rate, par, frequency, [basis], [calc_method])`

Gibt die aufgelaufenen Zinsen eines Wertpapiers an, das Zinsen nach einem Zeitplan zahlt.

- `issue`: Das Datum, an dem das Wertpapier ausgegeben wurde.
- `first_interest`: Das Datum, an dem die erste Zinszahlung erfolgt.
- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `rate`: Der Zinssatz pro Periode.
- `par`: Der Nennwert des Wertpapiers (1.000, wenn weggelassen).
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.
- `calc_method` (optional): WAHR oder weggelassen, um die Zinsen ab dem Ausgabedatum zu zählen; FALSCH, um sie ab dem Datum der ersten Zinszahlung zu zählen, wenn der Kauf später erfolgt.

## ACCRINTM

`ACCRINTM(issue, settlement, rate, [par], [basis])`

Gibt die aufgelaufenen Zinsen eines Wertpapiers an, das die Zinsen bei Fälligkeit zahlt.

- `issue`: Das Datum, an dem das Wertpapier ausgegeben wurde.
- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `rate`: Der Zinssatz pro Periode.
- `par` (optional): Der Nennwert des Wertpapiers (1.000, wenn weggelassen).
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## AMORDEGRC

`AMORDEGRC(cost, date_purchased, first_period, salvage, period, rate, [basis])`

Gibt die Abschreibung einer Periode nach der französischen degressiven Methode an.

- `cost`: Die Anschaffungskosten des Wirtschaftsguts.
- `date_purchased`: Das Datum, an dem das Wirtschaftsgut gekauft wurde.
- `first_period`: Das Datum, an dem die erste Periode endet.
- `salvage`: Der Restwert des Wirtschaftsguts am Ende.
- `period`: Welche Periode berechnet werden soll, wobei die erste als 1 zählt.
- `rate`: Der Zinssatz pro Periode.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## AMORLINC

`AMORLINC(cost, date_purchased, first_period, salvage, period, rate, [basis])`

Gibt die Abschreibung einer Periode nach der französischen linearen Methode an.

- `cost`: Die Anschaffungskosten des Wirtschaftsguts.
- `date_purchased`: Das Datum, an dem das Wirtschaftsgut gekauft wurde.
- `first_period`: Das Datum, an dem die erste Periode endet.
- `salvage`: Der Restwert des Wirtschaftsguts am Ende.
- `period`: Welche Periode berechnet werden soll, wobei die erste als 1 zählt.
- `rate`: Der Zinssatz pro Periode.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## COUPDAYBS

`COUPDAYBS(settlement, maturity, frequency, [basis])`

Gibt die Tage vom Beginn der Kuponperiode bis zum Abrechnungsdatum an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## COUPDAYS

`COUPDAYS(settlement, maturity, frequency, [basis])`

Gibt die Tage der Kuponperiode an, in die das Abrechnungsdatum fällt.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## COUPDAYSNC

`COUPDAYSNC(settlement, maturity, frequency, [basis])`

Gibt die Tage vom Abrechnungsdatum bis zum nächsten Kupontermin an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## COUPNCD

`COUPNCD(settlement, maturity, frequency, [basis])`

Gibt den nächsten Kupontermin nach dem Abrechnungsdatum an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## COUPNUM

`COUPNUM(settlement, maturity, frequency, [basis])`

Gibt die Anzahl der Kupons an, die zwischen Abrechnungs- und Fälligkeitsdatum zu zahlen sind.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## COUPPCD

`COUPPCD(settlement, maturity, frequency, [basis])`

Gibt den Kupontermin vor dem Abrechnungsdatum an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## CUMIPMT

`CUMIPMT(rate, nper, pv, start_period, end_period, type)`

Gibt die Zinsen an, die für einen Kredit zwischen zwei Perioden gezahlt werden.

- `rate`: Der Zinssatz pro Periode.
- `nper`: Wie viele Zahlungen insgesamt erfolgen.
- `pv`: Der heutige Wert der Beträge (der gesamte Kredit oder die gesamte Anlage).
- `start_period`: Die Periode, mit der der Zeitraum beginnt.
- `end_period`: Die Periode, mit der der Zeitraum endet.
- `type`: Wann die Zahlungen erfolgen: 0 oder weggelassen am Ende jeder Periode, 1 am Anfang.

## CUMPRINC

`CUMPRINC(rate, nper, pv, start_period, end_period, type)`

Gibt die Tilgung an, die für einen Kredit zwischen zwei Perioden gezahlt wird.

- `rate`: Der Zinssatz pro Periode.
- `nper`: Wie viele Zahlungen insgesamt erfolgen.
- `pv`: Der heutige Wert der Beträge (der gesamte Kredit oder die gesamte Anlage).
- `start_period`: Die Periode, mit der der Zeitraum beginnt.
- `end_period`: Die Periode, mit der der Zeitraum endet.
- `type`: Wann die Zahlungen erfolgen: 0 oder weggelassen am Ende jeder Periode, 1 am Anfang.

## DB

`DB(cost, salvage, life, period, [month])`

Gibt die Abschreibung eines Wirtschaftsguts für eine Periode nach der festen degressiven Methode an.

- `cost`: Die Anschaffungskosten des Wirtschaftsguts.
- `salvage`: Der Restwert des Wirtschaftsguts am Ende.
- `life`: Wie viele Perioden das Wirtschaftsgut genutzt wird.
- `period`: Welche Periode berechnet werden soll, wobei die erste als 1 zählt.
- `month` (optional): Der Monat, von 1 bis 12.

## DDB

`DDB(cost, salvage, life, period, [factor])`

Gibt die Abschreibung für eine Periode nach der degressiven Methode an.

- `cost`: Die Anschaffungskosten des Wirtschaftsguts.
- `salvage`: Der Restwert des Wirtschaftsguts am Ende.
- `life`: Wie viele Perioden das Wirtschaftsgut genutzt wird.
- `period`: Welche Periode berechnet werden soll, wobei die erste als 1 zählt.
- `factor` (optional): Wie schnell der Restbuchwert sinkt (2, wenn weggelassen, was die doppelt degressive Abschreibung ergibt).

## DISC

`DISC(settlement, maturity, pr, redemption, [basis])`

Gibt den Diskontsatz eines Wertpapiers an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `pr`: Der Kurs des Wertpapiers pro 100 Nennwert.
- `redemption`: Der Wert des Wertpapiers bei Fälligkeit, pro 100 Nennwert.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## DOLLARDE

`DOLLARDE(fractional_dollar, fraction)`

Wandelt einen Preis, der als Bruch geschrieben ist, etwa 1,02, in eine Dezimalzahl um.

- `fractional_dollar`: Ein Preis, der als Bruch geschrieben ist, etwa 1,02 für eins und zwei Sechzehntel.
- `fraction`: Die Zahl, durch die ein Dollar geteilt wird: zum Beispiel 16 für Sechzehntel.

## DOLLARFR

`DOLLARFR(decimal_dollar, fraction)`

Wandelt einen Dezimalpreis in einen Preis um, der als Bruch geschrieben ist, etwa 1,02.

- `decimal_dollar`: Ein Preis als Dezimalzahl, etwa 1,125.
- `fraction`: Die Zahl, durch die ein Dollar geteilt wird: zum Beispiel 16 für Sechzehntel.

## DURATION

`DURATION(settlement, maturity, coupon, yld, frequency, [basis])`

Gibt die durchschnittliche Zeit an, bis die Kosten einer Anleihe durch die Zahlungen zurückgeflossen sind, gewichtet nach den Zahlungen.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `coupon`: Der jährliche Kuponzinssatz der Anleihe.
- `yld`: Die jährliche Rendite des Wertpapiers.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## EFFECT

`EFFECT(nominal_rate, npery)`

Gibt den effektiven Jahreszinssatz an, den ein angegebener Zinssatz mit mehreren Perioden im Jahr ergibt.

- `nominal_rate`: Der angegebene Jahreszinssatz.
- `npery`: Wie viele Perioden es in einem Jahr gibt.

## FV

`FV(rate, nper, pmt, [pv], [type])`

Gibt an, was regelmäßige Zahlungen am Ende wert sind.

- `rate`: Der Zinssatz pro Periode.
- `nper`: Wie viele Zahlungen insgesamt erfolgen.
- `pmt`: Die in jeder Periode geleistete Zahlung.
- `pv` (optional): Der heutige Wert der Beträge (der gesamte Kredit oder die gesamte Anlage).
- `type` (optional): Wann die Zahlungen erfolgen: 0 oder weggelassen am Ende jeder Periode, 1 am Anfang.

## FVSCHEDULE

`FVSCHEDULE(principal, schedule)`

Gibt den Zukunftswert eines Betrags nach einer Liste von Zinssätzen an.

- `principal`: Der Betrag, mit dem begonnen wird.
- `schedule`: Der Zellbereich mit den Zinssätzen, die nacheinander angewendet werden.

## INTRATE

`INTRATE(settlement, maturity, investment, redemption, [basis])`

Gibt den Zinssatz eines vollständig angelegten Wertpapiers an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `investment`: Der im Wertpapier angelegte Betrag.
- `redemption`: Der Wert des Wertpapiers bei Fälligkeit, pro 100 Nennwert.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## IPMT

`IPMT(rate, per, nper, pv, [fv], [type])`

Gibt den Zinsanteil einer Zahlung an.

- `rate`: Der Zinssatz pro Periode.
- `per`: Welche Zahlung berechnet werden soll, wobei die erste als 1 zählt.
- `nper`: Wie viele Zahlungen insgesamt erfolgen.
- `pv`: Der heutige Wert der Beträge (der gesamte Kredit oder die gesamte Anlage).
- `fv` (optional): Was nach der letzten Zahlung übrig bleibt (0, wenn weggelassen).
- `type` (optional): Wann die Zahlungen erfolgen: 0 oder weggelassen am Ende jeder Periode, 1 am Anfang.

## IRR

`IRR(values, [guess])`

Gibt die Rendite einer Reihe von Beträgen an, die zu regelmäßigen Zeitpunkten anfallen.

- `values`: Der Zellbereich, der die Beträge enthält.
- `guess` (optional): Ihre Schätzung des Ergebnisses, mit der die Suche beginnt (0,1, wenn weggelassen).

## ISPMT

`ISPMT(rate, per, nper, pv)`

Gibt die Zinsen an, die in einer Periode eines Kredits gezahlt werden, der in gleichen Tilgungsteilen zurückgezahlt wird.

- `rate`: Der Zinssatz pro Periode.
- `per`: Welche Zahlung berechnet werden soll, wobei die erste als 1 zählt.
- `nper`: Wie viele Zahlungen insgesamt erfolgen.
- `pv`: Der heutige Wert der Beträge (der gesamte Kredit oder die gesamte Anlage).

## MDURATION

`MDURATION(settlement, maturity, coupon, yld, frequency, [basis])`

Gibt die Duration einer Anleihe an, bereinigt um ihre Rendite.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `coupon`: Der jährliche Kuponzinssatz der Anleihe.
- `yld`: Die jährliche Rendite des Wertpapiers.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## MIRR

`MIRR(values, finance_rate, reinvest_rate)`

Gibt die Rendite von Zahlungsströmen an, mit einem Zinssatz für Kredite und einem anderen für die Wiederanlage.

- `values`: Der Zellbereich, der die Beträge enthält.
- `finance_rate`: Der Zinssatz, den Sie für geliehenes Geld zahlen.
- `reinvest_rate`: Der Zinssatz, den Sie für wieder angelegtes Geld erzielen.

## NOMINAL

`NOMINAL(effect_rate, npery)`

Gibt den angegebenen Zinssatz für einen Jahreszinssatz und eine Anzahl von Perioden im Jahr an.

- `effect_rate`: Der Jahreszinssatz.
- `npery`: Wie viele Perioden es in einem Jahr gibt.

## NPER

`NPER(rate, pmt, pv, [fv], [type])`

Gibt an, wie viele Zahlungen nötig sind, um einen Kredit zurückzuzahlen.

- `rate`: Der Zinssatz pro Periode.
- `pmt`: Die in jeder Periode geleistete Zahlung.
- `pv`: Der heutige Wert der Beträge (der gesamte Kredit oder die gesamte Anlage).
- `fv` (optional): Was nach der letzten Zahlung übrig bleibt (0, wenn weggelassen).
- `type` (optional): Wann die Zahlungen erfolgen: 0 oder weggelassen am Ende jeder Periode, 1 am Anfang.

## NPV

`NPV(rate, value1, ...)`

Gibt den Barwert einer Reihe künftiger Beträge an.

- `rate`: Der Zinssatz pro Periode.
- `value1`: Der erste Wert oder Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie der vorherige.

## ODDFPRICE

`ODDFPRICE(settlement, maturity, issue, first_coupon, rate, yld, redemption, frequency, [basis])`

Gibt den Kurs pro 100 einer Anleihe mit unregelmäßiger erster Periode an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `issue`: Das Datum, an dem das Wertpapier ausgegeben wurde.
- `first_coupon`: Das Datum des ersten Kupons.
- `rate`: Der Zinssatz pro Periode.
- `yld`: Die jährliche Rendite des Wertpapiers.
- `redemption`: Der Wert des Wertpapiers bei Fälligkeit, pro 100 Nennwert.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## ODDFYIELD

`ODDFYIELD(settlement, maturity, issue, first_coupon, rate, pr, redemption, frequency, [basis])`

Gibt die Rendite einer Anleihe mit unregelmäßiger erster Periode an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `issue`: Das Datum, an dem das Wertpapier ausgegeben wurde.
- `first_coupon`: Das Datum des ersten Kupons.
- `rate`: Der Zinssatz pro Periode.
- `pr`: Der Kurs des Wertpapiers pro 100 Nennwert.
- `redemption`: Der Wert des Wertpapiers bei Fälligkeit, pro 100 Nennwert.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## ODDLPRICE

`ODDLPRICE(settlement, maturity, last_interest, rate, yld, redemption, frequency, [basis])`

Gibt den Kurs pro 100 einer Anleihe mit unregelmäßiger letzter Periode an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `last_interest`: Das Datum, an dem die letzte Zinszahlung erfolgte.
- `rate`: Der Zinssatz pro Periode.
- `yld`: Die jährliche Rendite des Wertpapiers.
- `redemption`: Der Wert des Wertpapiers bei Fälligkeit, pro 100 Nennwert.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## ODDLYIELD

`ODDLYIELD(settlement, maturity, last_interest, rate, pr, redemption, frequency, [basis])`

Gibt die Rendite einer Anleihe mit unregelmäßiger letzter Periode an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `last_interest`: Das Datum, an dem die letzte Zinszahlung erfolgte.
- `rate`: Der Zinssatz pro Periode.
- `pr`: Der Kurs des Wertpapiers pro 100 Nennwert.
- `redemption`: Der Wert des Wertpapiers bei Fälligkeit, pro 100 Nennwert.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## PDURATION

`PDURATION(rate, pv, fv)`

Gibt die Anzahl der Perioden an, die eine Anlage braucht, um einen Wert zu erreichen.

- `rate`: Der Zinssatz pro Periode.
- `pv`: Der heutige Wert der Beträge (der gesamte Kredit oder die gesamte Anlage).
- `fv`: Was nach der letzten Zahlung übrig bleibt (0, wenn weggelassen).

## PMT

`PMT(rate, nper, pv, [fv], [type])`

Gibt die regelmäßige Zahlung für einen Kredit an.

- `rate`: Der Zinssatz pro Periode.
- `nper`: Wie viele Zahlungen insgesamt erfolgen.
- `pv`: Der heutige Wert der Beträge (der gesamte Kredit oder die gesamte Anlage).
- `fv` (optional): Was nach der letzten Zahlung übrig bleibt (0, wenn weggelassen).
- `type` (optional): Wann die Zahlungen erfolgen: 0 oder weggelassen am Ende jeder Periode, 1 am Anfang.

## PPMT

`PPMT(rate, per, nper, pv, [fv], [type])`

Gibt den Teil einer Zahlung an, mit dem der Kredit selbst getilgt wird.

- `rate`: Der Zinssatz pro Periode.
- `per`: Welche Zahlung berechnet werden soll, wobei die erste als 1 zählt.
- `nper`: Wie viele Zahlungen insgesamt erfolgen.
- `pv`: Der heutige Wert der Beträge (der gesamte Kredit oder die gesamte Anlage).
- `fv` (optional): Was nach der letzten Zahlung übrig bleibt (0, wenn weggelassen).
- `type` (optional): Wann die Zahlungen erfolgen: 0 oder weggelassen am Ende jeder Periode, 1 am Anfang.

## PRICE

`PRICE(settlement, maturity, rate, yld, redemption, frequency, [basis])`

Gibt den Kurs pro 100 einer Anleihe an, die Zinsen nach einem Zeitplan zahlt.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `rate`: Der Zinssatz pro Periode.
- `yld`: Die jährliche Rendite des Wertpapiers.
- `redemption`: Der Wert des Wertpapiers bei Fälligkeit, pro 100 Nennwert.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## PRICEDISC

`PRICEDISC(settlement, maturity, discount, redemption, [basis])`

Gibt den Kurs pro 100 eines mit Abschlag gekauften Wertpapiers an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `discount`: Der Diskontsatz des Wertpapiers.
- `redemption`: Der Wert des Wertpapiers bei Fälligkeit, pro 100 Nennwert.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## PRICEMAT

`PRICEMAT(settlement, maturity, issue, rate, yld, [basis])`

Gibt den Kurs pro 100 eines Wertpapiers an, das die Zinsen bei Fälligkeit zahlt.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `issue`: Das Datum, an dem das Wertpapier ausgegeben wurde.
- `rate`: Der Zinssatz pro Periode.
- `yld`: Die jährliche Rendite des Wertpapiers.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## PV

`PV(rate, nper, pmt, [fv], [type])`

Gibt an, was eine Reihe künftiger Zahlungen heute wert ist.

- `rate`: Der Zinssatz pro Periode.
- `nper`: Wie viele Zahlungen insgesamt erfolgen.
- `pmt`: Die in jeder Periode geleistete Zahlung.
- `fv` (optional): Was nach der letzten Zahlung übrig bleibt (0, wenn weggelassen).
- `type` (optional): Wann die Zahlungen erfolgen: 0 oder weggelassen am Ende jeder Periode, 1 am Anfang.

## RATE

`RATE(nper, pmt, pv, [fv], [type], [guess])`

Gibt den Zinssatz pro Periode eines Kredits oder einer Anlage an.

- `nper`: Wie viele Zahlungen insgesamt erfolgen.
- `pmt`: Die in jeder Periode geleistete Zahlung.
- `pv`: Der heutige Wert der Beträge (der gesamte Kredit oder die gesamte Anlage).
- `fv` (optional): Was nach der letzten Zahlung übrig bleibt (0, wenn weggelassen).
- `type` (optional): Wann die Zahlungen erfolgen: 0 oder weggelassen am Ende jeder Periode, 1 am Anfang.
- `guess` (optional): Ihre Schätzung des Ergebnisses, mit der die Suche beginnt (0,1, wenn weggelassen).

## RECEIVED

`RECEIVED(settlement, maturity, investment, discount, [basis])`

Gibt den Betrag an, der bei Fälligkeit für ein vollständig angelegtes Wertpapier ausgezahlt wird.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `investment`: Der im Wertpapier angelegte Betrag.
- `discount`: Der Diskontsatz des Wertpapiers.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## RRI

`RRI(nper, pv, fv)`

Gibt den Zinssatz an, mit dem eine Anlage auf einen Wert anwächst.

- `nper`: Wie viele Zahlungen insgesamt erfolgen.
- `pv`: Der heutige Wert der Beträge (der gesamte Kredit oder die gesamte Anlage).
- `fv`: Was nach der letzten Zahlung übrig bleibt (0, wenn weggelassen).

## SLN

`SLN(cost, salvage, life)`

Gibt die lineare Abschreibung für ein Jahr an.

- `cost`: Die Anschaffungskosten des Wirtschaftsguts.
- `salvage`: Der Restwert des Wirtschaftsguts am Ende.
- `life`: Wie viele Perioden das Wirtschaftsgut genutzt wird.

## SYD

`SYD(cost, salvage, life, per)`

Gibt die Abschreibung für ein Jahr nach der arithmetisch-degressiven Methode (Summe der Jahresziffern) an.

- `cost`: Die Anschaffungskosten des Wirtschaftsguts.
- `salvage`: Der Restwert des Wirtschaftsguts am Ende.
- `life`: Wie viele Perioden das Wirtschaftsgut genutzt wird.
- `per`: Welche Zahlung berechnet werden soll, wobei die erste als 1 zählt.

## TBILLEQ

`TBILLEQ(settlement, maturity, discount)`

Gibt die anleihenäquivalente Rendite eines US-Schatzwechsels an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `discount`: Der Diskontsatz des Wertpapiers.

## TBILLPRICE

`TBILLPRICE(settlement, maturity, discount)`

Gibt den Kurs pro 100 eines US-Schatzwechsels an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `discount`: Der Diskontsatz des Wertpapiers.

## TBILLYIELD

`TBILLYIELD(settlement, maturity, pr)`

Gibt die Rendite eines US-Schatzwechsels an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `pr`: Der Kurs des Wertpapiers pro 100 Nennwert.

## VDB

`VDB(cost, salvage, life, start_period, end_period, [factor], [no_switch])`

Gibt die Abschreibung für einen beliebigen Zeitraum von Perioden nach der degressiven Methode an.

- `cost`: Die Anschaffungskosten des Wirtschaftsguts.
- `salvage`: Der Restwert des Wirtschaftsguts am Ende.
- `life`: Wie viele Perioden das Wirtschaftsgut genutzt wird.
- `start_period`: Die Periode, mit der der Zeitraum beginnt.
- `end_period`: Die Periode, mit der der Zeitraum endet.
- `factor` (optional): Wie schnell der Restbuchwert sinkt (2, wenn weggelassen, was die doppelt degressive Abschreibung ergibt).
- `no_switch` (optional): WAHR, um weiter nach der degressiven Methode abzuschreiben, auch wenn die lineare Methode mehr ergeben würde.

## XIRR

`XIRR(values, dates, [guess])`

Gibt die Rendite von Beträgen an, die an von Ihnen gewählten Terminen anfallen.

- `values`: Der Zellbereich, der die Beträge enthält.
- `dates`: Die Termine der Beträge, in derselben Reihenfolge wie die Beträge.
- `guess` (optional): Ihre Schätzung des Ergebnisses, mit der die Suche beginnt (0,1, wenn weggelassen).

## XNPV

`XNPV(rate, values, dates)`

Gibt den Barwert von Beträgen an, die an von Ihnen gewählten Terminen anfallen.

- `rate`: Der Zinssatz pro Periode.
- `values`: Der Zellbereich, der die Beträge enthält.
- `dates`: Die Termine der Beträge, in derselben Reihenfolge wie die Beträge.

## YIELD

`YIELD(settlement, maturity, rate, pr, redemption, frequency, [basis])`

Gibt die Rendite einer Anleihe an, die Zinsen nach einem Zeitplan zahlt.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `rate`: Der Zinssatz pro Periode.
- `pr`: Der Kurs des Wertpapiers pro 100 Nennwert.
- `redemption`: Der Wert des Wertpapiers bei Fälligkeit, pro 100 Nennwert.
- `frequency`: Wie viele Kupons pro Jahr gezahlt werden: 1, 2 oder 4.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## YIELDDISC

`YIELDDISC(settlement, maturity, pr, redemption, [basis])`

Gibt die jährliche Rendite eines mit Abschlag gekauften Wertpapiers an.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `pr`: Der Kurs des Wertpapiers pro 100 Nennwert.
- `redemption`: Der Wert des Wertpapiers bei Fälligkeit, pro 100 Nennwert.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

## YIELDMAT

`YIELDMAT(settlement, maturity, issue, rate, pr, [basis])`

Gibt die jährliche Rendite eines Wertpapiers an, das die Zinsen bei Fälligkeit zahlt.

- `settlement`: Das Datum, an dem Sie das Wertpapier kaufen.
- `maturity`: Das Datum, an dem das Wertpapier endet und zurückgezahlt wird.
- `issue`: Das Datum, an dem das Wertpapier ausgegeben wurde.
- `rate`: Der Zinssatz pro Periode.
- `pr`: Der Kurs des Wertpapiers pro 100 Nennwert.
- `basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 tatsächliche Tage, 2 tatsächliche Tage in einem 360-Tage-Jahr, 3 tatsächliche Tage in einem 365-Tage-Jahr, 4 europäische 30/360.

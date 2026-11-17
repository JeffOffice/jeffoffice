# Financiële functies
<!-- keywords: functions, ACCRINT, ACCRINTM, AMORDEGRC, AMORLINC, COUPDAYBS, COUPDAYS, COUPDAYSNC, COUPNCD, COUPNUM, COUPPCD, CUMIPMT, CUMPRINC, DB, DDB, DISC, DOLLARDE, DOLLARFR, DURATION, EFFECT, FV, FVSCHEDULE, INTRATE, IPMT, IRR, ISPMT, MDURATION, MIRR, NOMINAL, NPER, NPV, ODDFPRICE, ODDFYIELD, ODDLPRICE, ODDLYIELD, PDURATION, PMT, PPMT, PRICE, PRICEDISC, PRICEMAT, PV, RATE, RECEIVED, RRI, SLN, SYD, TBILLEQ, TBILLPRICE, TBILLYIELD, VDB, XIRR, XNPV, YIELD, YIELDDISC, YIELDMAT -->

Dit zijn de financiële functies die JeffCrunch kan berekenen. Bij elke functie ziet u hoe die wordt geschreven, wat die doet en wat elk van de argumenten betekent. Een argument tussen haakjes is optioneel, en "..." betekent dat u zoveel extra argumenten van het laatste soort kunt opgeven als nodig is. Om een functie te proberen, zie [Een functie kiezen](insert-function). Terwijl u een functie in een cel typt, toont een kleine tip onder de cel de argumenten ervan, en een klik op de naam van de functie in de tip opent het bijbehorende onderdeel hier.

## ACCRINT

`ACCRINT(issue, first_interest, settlement, rate, par, frequency, [basis], [calc_method])`

Geeft de opgelopen rente op een effect dat volgens een schema rente betaalt.

- `issue`: De datum waarop het effect is uitgegeven.
- `first_interest`: De datum waarop de eerste rente wordt betaald.
- `settlement`: De datum waarop u het effect koopt.
- `rate`: Het rentepercentage per periode.
- `par`: De nominale waarde van het effect (1.000 als dit wordt weggelaten).
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.
- `calc_method` (optioneel): WAAR of weggelaten om de rente vanaf de uitgiftedatum te tellen; ONWAAR om haar vanaf de datum van de eerste rente te tellen wanneer de afwikkeling later is.

## ACCRINTM

`ACCRINTM(issue, settlement, rate, [par], [basis])`

Geeft de opgelopen rente op een effect dat rente betaalt op de vervaldag.

- `issue`: De datum waarop het effect is uitgegeven.
- `settlement`: De datum waarop u het effect koopt.
- `rate`: Het rentepercentage per periode.
- `par` (optioneel): De nominale waarde van het effect (1.000 als dit wordt weggelaten).
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## AMORDEGRC

`AMORDEGRC(cost, date_purchased, first_period, salvage, period, rate, [basis])`

Geeft de afschrijving over een periode volgens de Franse degressieve methode.

- `cost`: Wat het actief kostte toen het werd gekocht.
- `date_purchased`: De datum waarop het actief is gekocht.
- `first_period`: De datum waarop de eerste periode eindigt.
- `salvage`: Wat het actief aan het einde waard is.
- `period`: Welke periode u wilt berekenen, waarbij de eerste als 1 telt.
- `rate`: Het rentepercentage per periode.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## AMORLINC

`AMORLINC(cost, date_purchased, first_period, salvage, period, rate, [basis])`

Geeft de afschrijving over een periode volgens de Franse lineaire methode.

- `cost`: Wat het actief kostte toen het werd gekocht.
- `date_purchased`: De datum waarop het actief is gekocht.
- `first_period`: De datum waarop de eerste periode eindigt.
- `salvage`: Wat het actief aan het einde waard is.
- `period`: Welke periode u wilt berekenen, waarbij de eerste als 1 telt.
- `rate`: Het rentepercentage per periode.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## COUPDAYBS

`COUPDAYBS(settlement, maturity, frequency, [basis])`

Geeft het aantal dagen van het begin van de couponperiode tot de afwikkelingsdatum.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## COUPDAYS

`COUPDAYS(settlement, maturity, frequency, [basis])`

Geeft het aantal dagen in de couponperiode waarin de afwikkelingsdatum valt.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## COUPDAYSNC

`COUPDAYSNC(settlement, maturity, frequency, [basis])`

Geeft het aantal dagen van de afwikkelingsdatum tot de volgende coupondatum.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## COUPNCD

`COUPNCD(settlement, maturity, frequency, [basis])`

Geeft de eerstvolgende coupondatum na de afwikkelingsdatum.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## COUPNUM

`COUPNUM(settlement, maturity, frequency, [basis])`

Geeft het aantal coupons dat moet worden betaald tussen de afwikkelings- en de vervaldatum.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## COUPPCD

`COUPPCD(settlement, maturity, frequency, [basis])`

Geeft de couponddatum vóór de afwikkelingsdatum.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## CUMIPMT

`CUMIPMT(rate, nper, pv, start_period, end_period, type)`

Geeft de rente die tussen twee periodes op een lening is betaald.

- `rate`: Het rentepercentage per periode.
- `nper`: Hoeveel betalingen er in totaal zijn.
- `pv`: Wat de bedragen nu waard zijn (de volledige lening of belegging).
- `start_period`: De periode waarmee het traject begint.
- `end_period`: De periode waarin het traject eindigt.
- `type`: Wanneer de betalingen worden gedaan: 0 of weggelaten aan het einde van elke periode, 1 aan het begin.

## CUMPRINC

`CUMPRINC(rate, nper, pv, start_period, end_period, type)`

Geeft de aflossing die tussen twee periodes op een lening is betaald.

- `rate`: Het rentepercentage per periode.
- `nper`: Hoeveel betalingen er in totaal zijn.
- `pv`: Wat de bedragen nu waard zijn (de volledige lening of belegging).
- `start_period`: De periode waarmee het traject begint.
- `end_period`: De periode waarin het traject eindigt.
- `type`: Wanneer de betalingen worden gedaan: 0 of weggelaten aan het einde van elke periode, 1 aan het begin.

## DB

`DB(cost, salvage, life, period, [month])`

Geeft de afschrijving van een actief voor een periode, volgens de vaste degressieve methode.

- `cost`: Wat het actief kostte toen het werd gekocht.
- `salvage`: Wat het actief aan het einde waard is.
- `life`: Hoeveel periodes het actief meegaat.
- `period`: Welke periode u wilt berekenen, waarbij de eerste als 1 telt.
- `month` (optioneel): De maand, van 1 tot 12.

## DDB

`DDB(cost, salvage, life, period, [factor])`

Geeft de afschrijving voor een periode volgens de degressieve methode.

- `cost`: Wat het actief kostte toen het werd gekocht.
- `salvage`: Wat het actief aan het einde waard is.
- `life`: Hoeveel periodes het actief meegaat.
- `period`: Welke periode u wilt berekenen, waarbij de eerste als 1 telt.
- `factor` (optioneel): Hoe snel de restwaarde daalt (2 als dit wordt weggelaten, wat dubbel degressief is).

## DISC

`DISC(settlement, maturity, pr, redemption, [basis])`

Geeft de disconteringsvoet van een effect.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `pr`: De koers van het effect per 100 nominale waarde.
- `redemption`: De waarde van het effect op de vervaldag, per 100 nominale waarde.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## DOLLARDE

`DOLLARDE(fractional_dollar, fraction)`

Zet een koers die als breuk is geschreven, zoals 1,02, om in een decimaal getal.

- `fractional_dollar`: Een koers die als breuk is geschreven, zoals 1,02 voor één en twee zestiende.
- `fraction`: Het getal waardoor een geheel wordt gedeeld: bijvoorbeeld 16 voor zestiende.

## DOLLARFR

`DOLLARFR(decimal_dollar, fraction)`

Zet een decimale koers om in een koers die als breuk is geschreven, zoals 1,02.

- `decimal_dollar`: Een koers als decimaal getal, zoals 1,125.
- `fraction`: Het getal waardoor een geheel wordt gedeeld: bijvoorbeeld 16 voor zestiende.

## DURATION

`DURATION(settlement, maturity, coupon, yld, frequency, [basis])`

Geeft de gemiddelde tijd om de kosten van een obligatie terug te verdienen, gewogen naar de betalingen.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `coupon`: Het jaarlijkse couponrentepercentage van de obligatie.
- `yld`: Het jaarlijkse rendement van het effect.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## EFFECT

`EFFECT(nominal_rate, npery)`

Geeft het jaarlijkse rentepercentage dat overeenkomt met een opgegeven rentepercentage met meerdere periodes per jaar.

- `nominal_rate`: Het opgegeven jaarlijkse rentepercentage.
- `npery`: Hoeveel periodes er per jaar zijn.

## FV

`FV(rate, nper, pmt, [pv], [type])`

Geeft wat regelmatige betalingen aan het einde waard zijn.

- `rate`: Het rentepercentage per periode.
- `nper`: Hoeveel betalingen er in totaal zijn.
- `pmt`: De betaling die elke periode wordt gedaan.
- `pv` (optioneel): Wat de bedragen nu waard zijn (de volledige lening of belegging).
- `type` (optioneel): Wanneer de betalingen worden gedaan: 0 of weggelaten aan het einde van elke periode, 1 aan het begin.

## FVSCHEDULE

`FVSCHEDULE(principal, schedule)`

Geeft de toekomstige waarde van een bedrag na een reeks rentepercentages.

- `principal`: Het bedrag waarmee wordt begonnen.
- `schedule`: Het celbereik met de rentepercentages die na elkaar worden toegepast.

## INTRATE

`INTRATE(settlement, maturity, investment, redemption, [basis])`

Geeft het rentepercentage van een volledig belegd effect.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `investment`: Het bedrag dat in het effect is gestoken.
- `redemption`: De waarde van het effect op de vervaldag, per 100 nominale waarde.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## IPMT

`IPMT(rate, per, nper, pv, [fv], [type])`

Geeft het rentedeel van een betaling.

- `rate`: Het rentepercentage per periode.
- `per`: Welke betaling u wilt berekenen, waarbij de eerste als 1 telt.
- `nper`: Hoeveel betalingen er in totaal zijn.
- `pv`: Wat de bedragen nu waard zijn (de volledige lening of belegging).
- `fv` (optioneel): Wat er na de laatste betaling overblijft (0 als dit wordt weggelaten).
- `type` (optioneel): Wanneer de betalingen worden gedaan: 0 of weggelaten aan het einde van elke periode, 1 aan het begin.

## IRR

`IRR(values, [guess])`

Geeft het rendement van een reeks bedragen die op regelmatige tijdstippen komen.

- `values`: Het celbereik met de bedragen.
- `guess` (optioneel): Uw schatting van het resultaat, waarmee het zoeken begint (0,1 als dit wordt weggelaten).

## ISPMT

`ISPMT(rate, per, nper, pv)`

Geeft de rente die in een periode is betaald op een lening die in gelijke delen van de hoofdsom wordt afgelost.

- `rate`: Het rentepercentage per periode.
- `per`: Welke betaling u wilt berekenen, waarbij de eerste als 1 telt.
- `nper`: Hoeveel betalingen er in totaal zijn.
- `pv`: Wat de bedragen nu waard zijn (de volledige lening of belegging).

## MDURATION

`MDURATION(settlement, maturity, coupon, yld, frequency, [basis])`

Geeft de looptijd van een obligatie, aangepast voor het rendement.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `coupon`: Het jaarlijkse couponrentepercentage van de obligatie.
- `yld`: Het jaarlijkse rendement van het effect.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## MIRR

`MIRR(values, finance_rate, reinvest_rate)`

Geeft het rendement van kasstromen, met één rente voor het lenen en een andere voor het herbeleggen.

- `values`: Het celbereik met de bedragen.
- `finance_rate`: Het percentage dat u betaalt over het geleende geld.
- `reinvest_rate`: Het percentage dat u verdient over het herbelegde geld.

## NOMINAL

`NOMINAL(effect_rate, npery)`

Geeft het opgegeven rentepercentage bij een jaarlijks rentepercentage en een aantal periodes per jaar.

- `effect_rate`: Het jaarlijkse rentepercentage.
- `npery`: Hoeveel periodes er per jaar zijn.

## NPER

`NPER(rate, pmt, pv, [fv], [type])`

Geeft hoeveel betalingen nodig zijn om een lening af te lossen.

- `rate`: Het rentepercentage per periode.
- `pmt`: De betaling die elke periode wordt gedaan.
- `pv`: Wat de bedragen nu waard zijn (de volledige lening of belegging).
- `fv` (optioneel): Wat er na de laatste betaling overblijft (0 als dit wordt weggelaten).
- `type` (optioneel): Wanneer de betalingen worden gedaan: 0 of weggelaten aan het einde van elke periode, 1 aan het begin.

## NPV

`NPV(rate, value1, ...)`

Geeft de huidige waarde van een reeks toekomstige bedragen.

- `rate`: Het rentepercentage per periode.
- `value1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## ODDFPRICE

`ODDFPRICE(settlement, maturity, issue, first_coupon, rate, yld, redemption, frequency, [basis])`

Geeft de koers per 100 van een obligatie met een oneven eerste periode.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `issue`: De datum waarop het effect is uitgegeven.
- `first_coupon`: De datum van de eerste coupon.
- `rate`: Het rentepercentage per periode.
- `yld`: Het jaarlijkse rendement van het effect.
- `redemption`: De waarde van het effect op de vervaldag, per 100 nominale waarde.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## ODDFYIELD

`ODDFYIELD(settlement, maturity, issue, first_coupon, rate, pr, redemption, frequency, [basis])`

Geeft het rendement van een obligatie met een oneven eerste periode.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `issue`: De datum waarop het effect is uitgegeven.
- `first_coupon`: De datum van de eerste coupon.
- `rate`: Het rentepercentage per periode.
- `pr`: De koers van het effect per 100 nominale waarde.
- `redemption`: De waarde van het effect op de vervaldag, per 100 nominale waarde.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## ODDLPRICE

`ODDLPRICE(settlement, maturity, last_interest, rate, yld, redemption, frequency, [basis])`

Geeft de koers per 100 van een obligatie met een oneven laatste periode.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `last_interest`: De datum waarop de laatste rente is betaald.
- `rate`: Het rentepercentage per periode.
- `yld`: Het jaarlijkse rendement van het effect.
- `redemption`: De waarde van het effect op de vervaldag, per 100 nominale waarde.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## ODDLYIELD

`ODDLYIELD(settlement, maturity, last_interest, rate, pr, redemption, frequency, [basis])`

Geeft het rendement van een obligatie met een oneven laatste periode.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `last_interest`: De datum waarop de laatste rente is betaald.
- `rate`: Het rentepercentage per periode.
- `pr`: De koers van het effect per 100 nominale waarde.
- `redemption`: De waarde van het effect op de vervaldag, per 100 nominale waarde.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## PDURATION

`PDURATION(rate, pv, fv)`

Geeft het aantal periodes dat een belegging nodig heeft om een waarde te bereiken.

- `rate`: Het rentepercentage per periode.
- `pv`: Wat de bedragen nu waard zijn (de volledige lening of belegging).
- `fv`: Wat er na de laatste betaling overblijft (0 als dit wordt weggelaten).

## PMT

`PMT(rate, nper, pv, [fv], [type])`

Geeft de regelmatige betaling voor een lening.

- `rate`: Het rentepercentage per periode.
- `nper`: Hoeveel betalingen er in totaal zijn.
- `pv`: Wat de bedragen nu waard zijn (de volledige lening of belegging).
- `fv` (optioneel): Wat er na de laatste betaling overblijft (0 als dit wordt weggelaten).
- `type` (optioneel): Wanneer de betalingen worden gedaan: 0 of weggelaten aan het einde van elke periode, 1 aan het begin.

## PPMT

`PPMT(rate, per, nper, pv, [fv], [type])`

Geeft het deel van een betaling dat de lening zelf afbetaalt.

- `rate`: Het rentepercentage per periode.
- `per`: Welke betaling u wilt berekenen, waarbij de eerste als 1 telt.
- `nper`: Hoeveel betalingen er in totaal zijn.
- `pv`: Wat de bedragen nu waard zijn (de volledige lening of belegging).
- `fv` (optioneel): Wat er na de laatste betaling overblijft (0 als dit wordt weggelaten).
- `type` (optioneel): Wanneer de betalingen worden gedaan: 0 of weggelaten aan het einde van elke periode, 1 aan het begin.

## PRICE

`PRICE(settlement, maturity, rate, yld, redemption, frequency, [basis])`

Geeft de koers per 100 van een obligatie die volgens een schema rente betaalt.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `rate`: Het rentepercentage per periode.
- `yld`: Het jaarlijkse rendement van het effect.
- `redemption`: De waarde van het effect op de vervaldag, per 100 nominale waarde.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## PRICEDISC

`PRICEDISC(settlement, maturity, discount, redemption, [basis])`

Geeft de koers per 100 van een effect dat met korting is gekocht.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `discount`: De disconteringsvoet van het effect.
- `redemption`: De waarde van het effect op de vervaldag, per 100 nominale waarde.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## PRICEMAT

`PRICEMAT(settlement, maturity, issue, rate, yld, [basis])`

Geeft de koers per 100 van een effect dat rente betaalt op de vervaldag.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `issue`: De datum waarop het effect is uitgegeven.
- `rate`: Het rentepercentage per periode.
- `yld`: Het jaarlijkse rendement van het effect.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## PV

`PV(rate, nper, pmt, [fv], [type])`

Geeft wat een reeks toekomstige betalingen vandaag waard is.

- `rate`: Het rentepercentage per periode.
- `nper`: Hoeveel betalingen er in totaal zijn.
- `pmt`: De betaling die elke periode wordt gedaan.
- `fv` (optioneel): Wat er na de laatste betaling overblijft (0 als dit wordt weggelaten).
- `type` (optioneel): Wanneer de betalingen worden gedaan: 0 of weggelaten aan het einde van elke periode, 1 aan het begin.

## RATE

`RATE(nper, pmt, pv, [fv], [type], [guess])`

Geeft het rentepercentage per periode van een lening of een belegging.

- `nper`: Hoeveel betalingen er in totaal zijn.
- `pmt`: De betaling die elke periode wordt gedaan.
- `pv`: Wat de bedragen nu waard zijn (de volledige lening of belegging).
- `fv` (optioneel): Wat er na de laatste betaling overblijft (0 als dit wordt weggelaten).
- `type` (optioneel): Wanneer de betalingen worden gedaan: 0 of weggelaten aan het einde van elke periode, 1 aan het begin.
- `guess` (optioneel): Uw schatting van het resultaat, waarmee het zoeken begint (0,1 als dit wordt weggelaten).

## RECEIVED

`RECEIVED(settlement, maturity, investment, discount, [basis])`

Geeft het bedrag dat op de vervaldag wordt ontvangen voor een volledig belegd effect.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `investment`: Het bedrag dat in het effect is gestoken.
- `discount`: De disconteringsvoet van het effect.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## RRI

`RRI(nper, pv, fv)`

Geeft het rentepercentage waarmee een belegging groeit tot een waarde.

- `nper`: Hoeveel betalingen er in totaal zijn.
- `pv`: Wat de bedragen nu waard zijn (de volledige lening of belegging).
- `fv`: Wat er na de laatste betaling overblijft (0 als dit wordt weggelaten).

## SLN

`SLN(cost, salvage, life)`

Geeft de lineaire afschrijving van één jaar.

- `cost`: Wat het actief kostte toen het werd gekocht.
- `salvage`: Wat het actief aan het einde waard is.
- `life`: Hoeveel periodes het actief meegaat.

## SYD

`SYD(cost, salvage, life, per)`

Geeft de afschrijving van één jaar volgens de sommethode van de jaarcijfers.

- `cost`: Wat het actief kostte toen het werd gekocht.
- `salvage`: Wat het actief aan het einde waard is.
- `life`: Hoeveel periodes het actief meegaat.
- `per`: Welke betaling u wilt berekenen, waarbij de eerste als 1 telt.

## TBILLEQ

`TBILLEQ(settlement, maturity, discount)`

Geeft het obligatie-equivalente rendement van een schatkistbiljet.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `discount`: De disconteringsvoet van het effect.

## TBILLPRICE

`TBILLPRICE(settlement, maturity, discount)`

Geeft de koers per 100 van een schatkistbiljet.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `discount`: De disconteringsvoet van het effect.

## TBILLYIELD

`TBILLYIELD(settlement, maturity, pr)`

Geeft het rendement van een schatkistbiljet.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `pr`: De koers van het effect per 100 nominale waarde.

## VDB

`VDB(cost, salvage, life, start_period, end_period, [factor], [no_switch])`

Geeft de afschrijving over een willekeurig aantal periodes volgens de degressieve methode.

- `cost`: Wat het actief kostte toen het werd gekocht.
- `salvage`: Wat het actief aan het einde waard is.
- `life`: Hoeveel periodes het actief meegaat.
- `start_period`: De periode waarmee het traject begint.
- `end_period`: De periode waarin het traject eindigt.
- `factor` (optioneel): Hoe snel de restwaarde daalt (2 als dit wordt weggelaten, wat dubbel degressief is).
- `no_switch` (optioneel): WAAR om de degressieve methode te blijven gebruiken, nadat de lineaire methode meer zou opleveren.

## XIRR

`XIRR(values, dates, [guess])`

Geeft het rendement van bedragen die op data komen die u kiest.

- `values`: Het celbereik met de bedragen.
- `dates`: De data van de bedragen, in dezelfde volgorde als de bedragen.
- `guess` (optioneel): Uw schatting van het resultaat, waarmee het zoeken begint (0,1 als dit wordt weggelaten).

## XNPV

`XNPV(rate, values, dates)`

Geeft de huidige waarde van bedragen die op data komen die u kiest.

- `rate`: Het rentepercentage per periode.
- `values`: Het celbereik met de bedragen.
- `dates`: De data van de bedragen, in dezelfde volgorde als de bedragen.

## YIELD

`YIELD(settlement, maturity, rate, pr, redemption, frequency, [basis])`

Geeft het rendement van een obligatie die volgens een schema rente betaalt.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `rate`: Het rentepercentage per periode.
- `pr`: De koers van het effect per 100 nominale waarde.
- `redemption`: De waarde van het effect op de vervaldag, per 100 nominale waarde.
- `frequency`: Hoeveel coupons per jaar worden betaald: 1, 2 of 4.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## YIELDDISC

`YIELDDISC(settlement, maturity, pr, redemption, [basis])`

Geeft het jaarlijkse rendement van een effect dat met korting is gekocht.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `pr`: De koers van het effect per 100 nominale waarde.
- `redemption`: De waarde van het effect op de vervaldag, per 100 nominale waarde.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

## YIELDMAT

`YIELDMAT(settlement, maturity, issue, rate, pr, [basis])`

Geeft het jaarlijkse rendement van een effect dat rente betaalt op de vervaldag.

- `settlement`: De datum waarop u het effect koopt.
- `maturity`: De datum waarop het effect eindigt en wordt terugbetaald.
- `issue`: De datum waarop het effect is uitgegeven.
- `rate`: Het rentepercentage per periode.
- `pr`: De koers van het effect per 100 nominale waarde.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.

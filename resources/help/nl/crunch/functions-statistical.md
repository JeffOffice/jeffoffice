# Statistische functies
<!-- keywords: functies, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.ETS, FORECAST.ETS.CONFINT, FORECAST.ETS.SEASONALITY, FORECAST.ETS.STAT, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST, functiebibliotheek, statistiek -->

Dit zijn de statistische functies die JeffCrunch kan berekenen. Elke functie laat zien hoe ze wordt geschreven, wat ze doet en wat elk van de argumenten betekent. Een argument tussen haakjes is optioneel, en "..." betekent dat u zoveel extra van het laatste argument kunt opgeven als nodig is. Om een functie uit te proberen, zie [Een functie kiezen](insert-function). Terwijl u een functie in een cel typt, toont een kleine tip onder de cel de argumenten ervan. Als u in de tip op de naam van de functie klikt, opent zich het onderdeel hier.

## AVEDEV

`AVEDEV(getal1, ...)`

Geeft het gemiddelde van de afstanden van getallen tot hun gemiddelde.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## AVERAGE

`AVERAGE(getal1, ...)`

Geeft het gemiddelde van getallen.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## AVERAGEA

`AVERAGEA(waarde1, ...)`

Geeft het gemiddelde van waarden, waarbij tekst als nul en waar als één wordt geteld.

- `waarde1`: De eerste waarde, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## AVERAGEIF

`AVERAGEIF(bereik, criteria, [gemiddelde_bereik])`

Berekent het gemiddelde van de cellen die aan een voorwaarde voldoen.

- `bereik`: Het celbereik dat wordt getest.
- `criteria`: Waarnaar moet worden gezocht: een getal, een tekst of een test zoals ">5".
- `gemiddelde_bereik` (optioneel): De cellen waarvan het gemiddelde wordt berekend (de cellen waarin wordt gezocht als dit wordt weggelaten).

## AVERAGEIFS

`AVERAGEIFS(gemiddelde_bereik, criteria_bereik1, criteria1, ...)`

Berekent het gemiddelde van de cellen die aan meerdere voorwaarden voldoen.

- `gemiddelde_bereik`: De cellen waarvan het gemiddelde wordt berekend (de cellen waarin wordt gezocht als dit wordt weggelaten).
- `criteria_bereik1`: Het eerste celbereik dat wordt getest.
- `criteria1`: Waarnaar moet worden gezocht in het eerste celbereik: een getal, een tekst of een test zoals ">5".
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## BETA.DIST

`BETA.DIST(x, alfa, bèta, cumulatief, [A], [B])`

Geeft de waarde of het cumulatieve aandeel van de bètacurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de curve vormgeeft (de schaal van de curve voor de gamma- en Weibullcurven).
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.
- `A` (optioneel): De ondergrens van x (0 als dit wordt weggelaten).
- `B` (optioneel): De bovengrens van x (1 als dit wordt weggelaten).

## BETA.INV

`BETA.INV(kans, alfa, bèta, [A], [B])`

Geeft de x-waarde voor een aandeel van een bètacurve.

- `kans`: Een kans, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de curve vormgeeft (de schaal van de curve voor de gamma- en Weibullcurven).
- `A` (optioneel): De ondergrens van x (0 als dit wordt weggelaten).
- `B` (optioneel): De bovengrens van x (1 als dit wordt weggelaten).

## BETADIST

`BETADIST(x, alfa, bèta, [A], [B])`

Geeft het cumulatieve aandeel van de bètacurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de curve vormgeeft (de schaal van de curve voor de gamma- en Weibullcurven).
- `A` (optioneel): De ondergrens van x (0 als dit wordt weggelaten).
- `B` (optioneel): De bovengrens van x (1 als dit wordt weggelaten).

## BETAINV

`BETAINV(kans, alfa, bèta, [A], [B])`

Geeft de x-waarde voor een aandeel van een bètacurve.

- `kans`: Een kans, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de curve vormgeeft (de schaal van de curve voor de gamma- en Weibullcurven).
- `A` (optioneel): De ondergrens van x (0 als dit wordt weggelaten).
- `B` (optioneel): De bovengrens van x (1 als dit wordt weggelaten).

## BINOM.DIST

`BINOM.DIST(aantal_successen, pogingen, kans_succes, cumulatief)`

Geeft de kans op een aantal successen in een aantal pogingen.

- `aantal_successen`: Het aantal successen.
- `pogingen`: Het aantal pogingen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(pogingen, kans_succes, aantal_successen, [aantal_successen2])`

Geeft de kans dat het aantal successen in een bereik valt.

- `pogingen`: Het aantal pogingen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `aantal_successen`: Het aantal successen.
- `aantal_successen2` (optioneel): Het hoogste aantal successen dat wordt geteld (hetzelfde als het laagste als dit wordt weggelaten).

## BINOM.INV

`BINOM.INV(pogingen, kans_succes, alfa)`

Geeft het laagste aantal successen waarbij de cumulatieve kans een waarde bereikt.

- `pogingen`: Het aantal pogingen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.

## BINOMDIST

`BINOMDIST(aantal_successen, pogingen, kans_succes, cumulatief)`

Geeft de kans op een aantal successen in een aantal pogingen.

- `aantal_successen`: Het aantal successen.
- `pogingen`: Het aantal pogingen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## CHIDIST

`CHIDIST(x, vrijheidsgraden)`

Geeft het aandeel van de chi-kwadraatcurve boven x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.

## CHIINV

`CHIINV(kans, vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de chi-kwadraatcurve, gerekend vanaf rechts.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.

## CHISQ.DIST

`CHISQ.DIST(x, vrijheidsgraden, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de chi-kwadraatcurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x, vrijheidsgraden)`

Geeft het aandeel van de chi-kwadraatcurve boven x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.

## CHISQ.INV

`CHISQ.INV(kans, vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de chi-kwadraatcurve, gerekend vanaf links.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.

## CHISQ.INV.RT

`CHISQ.INV.RT(kans, vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de chi-kwadraatcurve, gerekend vanaf rechts.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.

## CHISQ.TEST

`CHISQ.TEST(werkelijk_bereik, verwacht_bereik)`

Geeft de kans dat de aantallen alleen door toeval afwijken van de verwachte aantallen.

- `werkelijk_bereik`: Het celbereik met de aantallen die u hebt gekregen.
- `verwacht_bereik`: Het celbereik met de aantallen die u had verwacht, even groot als de aantallen die u hebt gekregen.

## CHITEST

`CHITEST(werkelijk_bereik, verwacht_bereik)`

Geeft de kans dat de aantallen alleen door toeval afwijken van de verwachte aantallen.

- `werkelijk_bereik`: Het celbereik met de aantallen die u hebt gekregen.
- `verwacht_bereik`: Het celbereik met de aantallen die u had verwacht, even groot als de aantallen die u hebt gekregen.

## CONFIDENCE

`CONFIDENCE(alfa, standaardafwijking, grootte)`

Geeft de marge van een betrouwbaarheidsinterval voor een gemiddelde, met de normale curve.

- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `standaardafwijking`: De standaarddeviatie van de verdeling.
- `grootte`: De grootte van de steekproef, een geheel getal van 1 of meer.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(alfa, standaardafwijking, grootte)`

Geeft de marge van een betrouwbaarheidsinterval voor een gemiddelde, met de normale curve.

- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `standaardafwijking`: De standaarddeviatie van de verdeling.
- `grootte`: De grootte van de steekproef, een geheel getal van 1 of meer.

## CONFIDENCE.T

`CONFIDENCE.T(alfa, standaardafwijking, grootte)`

Geeft de marge van een betrouwbaarheidsinterval voor een gemiddelde, met de t-curve van Student.

- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `standaardafwijking`: De standaarddeviatie van de verdeling.
- `grootte`: De grootte van de steekproef, een geheel getal van 1 of meer.

## CORREL

`CORREL(matrix1, matrix2)`

Geeft hoe sterk twee reeksen getallen samen bewegen, van -1 tot 1.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## COUNT

`COUNT(waarde1, ...)`

Telt de getallen in een lijst of een celbereik.

- `waarde1`: De eerste waarde, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## COUNTA

`COUNTA(waarde1, ...)`

Telt de cellen die niet leeg zijn.

- `waarde1`: De eerste waarde, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## COUNTBLANK

`COUNTBLANK(bereik)`

Telt de lege cellen in een bereik.

- `bereik`: Het celbereik dat wordt getest.

## COUNTIF

`COUNTIF(bereik, criteria)`

Telt de cellen die aan een voorwaarde voldoen.

- `bereik`: Het celbereik dat wordt getest.
- `criteria`: Waarnaar moet worden gezocht: een getal, een tekst of een test zoals ">5".

## COUNTIFS

`COUNTIFS(criteria_bereik1, criteria1, ...)`

Telt de rijen waarin aan meerdere voorwaarden tegelijk is voldaan.

- `criteria_bereik1`: Het eerste celbereik dat wordt getest.
- `criteria1`: Waarnaar moet worden gezocht in het eerste celbereik: een getal, een tekst of een test zoals ">5".
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## COVAR

`COVAR(matrix1, matrix2)`

Geeft hoeveel twee reeksen getallen samen variëren.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## COVARIANCE.P

`COVARIANCE.P(matrix1, matrix2)`

Geeft hoeveel twee reeksen getallen samen variëren, voor een populatie.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## COVARIANCE.S

`COVARIANCE.S(matrix1, matrix2)`

Geeft hoeveel twee reeksen getallen samen variëren, voor een steekproef.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## CRITBINOM

`CRITBINOM(pogingen, kans_succes, alfa)`

Geeft het laagste aantal successen waarbij de cumulatieve kans een waarde bereikt.

- `pogingen`: Het aantal pogingen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.

## DEVSQ

`DEVSQ(getal1, ...)`

Geeft de som van de kwadraten van de afstanden van getallen tot hun gemiddelde.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## EXPON.DIST

`EXPON.DIST(x, lambda, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de exponentiële curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `lambda`: De snelheid van de curve, groter dan 0.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## EXPONDIST

`EXPONDIST(x, lambda, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de exponentiële curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `lambda`: De snelheid van de curve, groter dan 0.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## F.DIST

`F.DIST(x, vrijheidsgraden1, vrijheidsgraden2, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de F-curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of meer.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of meer.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## F.DIST.RT

`F.DIST.RT(x, vrijheidsgraden1, vrijheidsgraden2)`

Geeft het aandeel van de F-curve boven x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of meer.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of meer.

## F.INV

`F.INV(kans, vrijheidsgraden1, vrijheidsgraden2)`

Geeft de x-waarde voor een aandeel van de F-curve, gerekend vanaf links.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of meer.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of meer.

## F.INV.RT

`F.INV.RT(kans, vrijheidsgraden1, vrijheidsgraden2)`

Geeft de x-waarde voor een aandeel van de F-curve, gerekend vanaf rechts.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of meer.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of meer.

## F.TEST

`F.TEST(matrix1, matrix2)`

Geeft de kans dat twee reeksen getallen dezelfde variantie hebben.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## FDIST

`FDIST(x, vrijheidsgraden1, vrijheidsgraden2)`

Geeft het aandeel van de F-curve boven x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of meer.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of meer.

## FINV

`FINV(kans, vrijheidsgraden1, vrijheidsgraden2)`

Geeft de x-waarde voor een aandeel van de F-curve, gerekend vanaf rechts.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of meer.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of meer.

## FISHER

`FISHER(x)`

Geeft de Fisher-transformatie van een getal tussen -1 en 1.

- `x`: De x-waarde waarmee moet worden gewerkt.

## FISHERINV

`FISHERINV(y)`

Geeft de inverse van de Fisher-transformatie.

- `y`: Het getal waarvan de inverse van de Fisher-transformatie moet worden gevonden.

## FORECAST

`FORECAST(x, bekende_y's, bekende_x's)`

Voorspelt een waarde langs de best passende rechte lijn.

- `x`: De x-waarde waarvoor een y-waarde moet worden voorspeld.
- `bekende_y's`: Het celbereik dat de y-waarden bevat die u al hebt.
- `bekende_x's`: Het celbereik dat de x-waarden bevat die u al hebt.

## FORECAST.ETS

`FORECAST.ETS(doeldatum, waarden, tijdlijn, [seizoenspatroon], [gegevensaanvulling], [samenvoeging])`

Voorspelt een toekomstige waarde van een reeks met een trend en een seizoen, met exponentiële afvlakking.

- `doeldatum`: Het tijdstip waarvoor een waarde moet worden voorspeld.
- `waarden`: De waarden die u al hebt.
- `tijdlijn`: De datums of getallen van de waarden, op gelijke afstand van elkaar.
- `seizoenspatroon`: 1 om het seizoen uit de waarden te bepalen, 0 voor geen seizoen, of de lengte van het seizoen.
- `gegevensaanvulling`: 1 om een ontbrekend punt aan te vullen met een lijn tussen de buren, 0 om het met nul aan te vullen.
- `samenvoeging`: Hoe waarden op hetzelfde tijdstip worden gecombineerd: 1 gemiddelde, 2 aantal, 3 aantalA, 4 maximum, 5 mediaan, 6 minimum, 7 som.

## FORECAST.ETS.CONFINT

`FORECAST.ETS.CONFINT(doeldatum, waarden, tijdlijn, [betrouwbaarheidsniveau], [seizoenspatroon], [gegevensaanvulling], [samenvoeging])`

Geeft hoe ver de waarde waarschijnlijk boven en onder een afgevlakte prognose ligt.

- `betrouwbaarheidsniveau`: Een getal boven 0 en onder 1, 0,95 als dit wordt weggelaten.

## FORECAST.ETS.SEASONALITY

`FORECAST.ETS.SEASONALITY(waarden, tijdlijn, [gegevensaanvulling], [samenvoeging])`

Geeft de lengte van het zich herhalende seizoen in een reeks, of 1 als er geen seizoen is.

## FORECAST.ETS.STAT

`FORECAST.ETS.STAT(waarden, tijdlijn, statistiektype, [seizoenspatroon], [gegevensaanvulling], [samenvoeging])`

Geeft een van de afvlakkingsconstanten of maten voor de passing van een afgevlakte prognose.

- `statistiektype`: 1 Alfa, 2 Bèta, 3 Gamma, 4 MASE, 5 SMAPE, 6 MAE, 7 RMSE, 8 de stap van de tijdlijn.

## FORECAST.LINEAR

`FORECAST.LINEAR(x, bekende_y's, bekende_x's)`

Voorspelt een waarde langs de best passende rechte lijn.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `bekende_y's`: Het celbereik dat de y-waarden bevat die u al hebt.
- `bekende_x's`: Het celbereik dat de x-waarden bevat die u al hebt.

## FREQUENCY

`FREQUENCY(gegevensmatrix, klassenmatrix)`

Telt hoeveel waarden in elk bereik vallen, en geeft de aantallen als een kolom.

- `gegevensmatrix`: Het celbereik met de waarden die moeten worden geteld.
- `klassenmatrix`: Het celbereik met de grenzen van de bereiken waarin moet worden geteld.

## FTEST

`FTEST(matrix1, matrix2)`

Geeft de kans dat twee reeksen getallen dezelfde variantie hebben.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## GAMMA

`GAMMA(x)`

Geeft de gammafunctie van een getal.

- `x`: De x-waarde waarmee moet worden gewerkt.

## GAMMA.DIST

`GAMMA.DIST(x, alfa, bèta, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de gammacurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de curve vormgeeft (de schaal van de curve voor de gamma- en Weibullcurven).
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## GAMMA.INV

`GAMMA.INV(kans, alfa, bèta)`

Geeft de x-waarde voor een aandeel van een gammacurve.

- `kans`: Een kans, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de curve vormgeeft (de schaal van de curve voor de gamma- en Weibullcurven).

## GAMMADIST

`GAMMADIST(x, alfa, bèta, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de gammacurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de curve vormgeeft (de schaal van de curve voor de gamma- en Weibullcurven).
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## GAMMAINV

`GAMMAINV(kans, alfa, bèta)`

Geeft de x-waarde voor een aandeel van een gammacurve.

- `kans`: Een kans, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de curve vormgeeft (de schaal van de curve voor de gamma- en Weibullcurven).

## GAMMALN

`GAMMALN(x)`

Geeft de natuurlijke logaritme van de gammafunctie.

- `x`: De x-waarde waarmee moet worden gewerkt.

## GAMMALN.PRECISE

`GAMMALN.PRECISE(x)`

Geeft de natuurlijke logaritme van de gammafunctie.

- `x`: De x-waarde waarmee moet worden gewerkt.

## GAUSS

`GAUSS(z)`

Geeft het aandeel van de standaard normale curve tussen het gemiddelde en z.

- `z`: De z-waarde.

## GEOMEAN

`GEOMEAN(getal1, ...)`

Geeft het meetkundig gemiddelde van positieve getallen.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## GROWTH

`GROWTH(bekende_y's, [bekende_x's], [nieuwe_x's], [const])`

Geeft de waarden langs de best passende exponentiële curve voor nieuwe x-waarden.

- `bekende_y's`: Het celbereik dat de y-waarden bevat die u al hebt.
- `bekende_x's` (optioneel): Het celbereik dat de x-waarden bevat die u al hebt.
- `nieuwe_x's` (optioneel): De x-waarden waarvoor de y-waarden worden gegeven (de bekende x-waarden als dit wordt weggelaten).
- `const` (optioneel): Waar of weggelaten om een lijn te laten passen die niet door nul hoeft te gaan; onwaar om de lijn door nul te laten gaan.

## HARMEAN

`HARMEAN(getal1, ...)`

Geeft het harmonisch gemiddelde van positieve getallen.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## HYPGEOM.DIST

`HYPGEOM.DIST(steekproef_successen, aantal_steekproef, populatie_successen, aantal_populatie, cumulatief)`

Geeft de kans om een aantal successen te trekken zonder items terug te leggen.

- `steekproef_successen`: Het aantal successen in de steekproef.
- `aantal_steekproef`: Het aantal getrokken items.
- `populatie_successen`: Het aantal successen in de hele populatie.
- `aantal_populatie`: Het aantal items in de hele populatie.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## HYPGEOMDIST

`HYPGEOMDIST(steekproef_successen, aantal_steekproef, populatie_successen, aantal_populatie)`

Geeft de kans om een aantal successen te trekken zonder items terug te leggen.

- `steekproef_successen`: Het aantal successen in de steekproef.
- `aantal_steekproef`: Het aantal getrokken items.
- `populatie_successen`: Het aantal successen in de hele populatie.
- `aantal_populatie`: Het aantal items in de hele populatie.

## INTERCEPT

`INTERCEPT(bekende_y's, bekende_x's)`

Geeft het punt waar de best passende rechte lijn de verticale as snijdt.

- `bekende_y's`: Het celbereik dat de y-waarden bevat die u al hebt.
- `bekende_x's`: Het celbereik dat de x-waarden bevat die u al hebt.

## KURT

`KURT(getal1, ...)`

Geeft hoe puntig of plat een reeks getallen is in vergelijking met een normale curve.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## LARGE

`LARGE(matrix, k)`

Geeft het k-de grootste getal van een reeks.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `k`: Welke plaats wordt gegeven, geteld vanaf het grootste: 1 is het grootste.

## LINEST

`LINEST(bekende_y's, [bekende_x's], [const], [statistieken])`

Past een rechte lijn of vlak door de gegevens volgens de kleinste-kwadratenmethode, en geeft de getallen als een rij.

- `bekende_y's`: Het celbereik dat de y-waarden bevat die u al hebt.
- `bekende_x's` (optioneel): Het celbereik dat de x-waarden bevat die u al hebt.
- `const` (optioneel): Waar of weggelaten om een lijn te laten passen die niet door nul hoeft te gaan; onwaar om de lijn door nul te laten gaan.
- `statistieken` (optioneel): Waar om daarnaast meer getallen over de passing te geven; onwaar of weggelaten om alleen de lijn te geven.

## LOGEST

`LOGEST(bekende_y's, [bekende_x's], [const], [statistieken])`

Past een exponentiële curve door de gegevens en geeft de getallen als een rij.

- `bekende_y's`: Het celbereik dat de y-waarden bevat die u al hebt.
- `bekende_x's` (optioneel): Het celbereik dat de x-waarden bevat die u al hebt.
- `const` (optioneel): Waar of weggelaten om een lijn te laten passen die niet door nul hoeft te gaan; onwaar om de lijn door nul te laten gaan.
- `statistieken` (optioneel): Waar om daarnaast meer getallen over de passing te geven; onwaar of weggelaten om alleen de lijn te geven.

## LOGINV

`LOGINV(kans, gemiddelde, standaardafwijking)`

Geeft de x-waarde voor een aandeel van een lognormale curve.

- `kans`: Een kans, van 0 tot 1.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaardafwijking`: De standaarddeviatie van de verdeling.

## LOGNORM.DIST

`LOGNORM.DIST(x, gemiddelde, standaardafwijking, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de lognormale curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaardafwijking`: De standaarddeviatie van de verdeling.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## LOGNORM.INV

`LOGNORM.INV(kans, gemiddelde, standaardafwijking)`

Geeft de x-waarde voor een aandeel van een lognormale curve.

- `kans`: Een kans, van 0 tot 1.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaardafwijking`: De standaarddeviatie van de verdeling.

## LOGNORMDIST

`LOGNORMDIST(x, gemiddelde, standaardafwijking)`

Geeft het cumulatieve aandeel van de lognormale curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaardafwijking`: De standaarddeviatie van de verdeling.

## MAXA

`MAXA(waarde1, ...)`

Geeft de grootste waarde, waarbij tekst als nul en waar als één wordt geteld.

- `waarde1`: De eerste waarde, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MAXIFS

`MAXIFS(maximumbereik, criteria_bereik1, criteria1, ...)`

Geeft het grootste getal van de cellen die aan meerdere voorwaarden voldoen.

- `maximumbereik`: De cellen waaruit de grootste waarde wordt genomen.
- `criteria_bereik1`: Het eerste celbereik dat wordt getest.
- `criteria1`: Waarnaar moet worden gezocht in het eerste celbereik: een getal, een tekst of een test zoals ">5".
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MEDIAN

`MEDIAN(getal1, ...)`

Geeft het middelste getal van een reeks.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MINA

`MINA(waarde1, ...)`

Geeft de kleinste waarde, waarbij tekst als nul en waar als één wordt geteld.

- `waarde1`: De eerste waarde, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MINIFS

`MINIFS(minimumbereik, criteria_bereik1, criteria1, ...)`

Geeft het kleinste getal van de cellen die aan meerdere voorwaarden voldoen.

- `minimumbereik`: De cellen waaruit de kleinste waarde wordt genomen.
- `criteria_bereik1`: Het eerste celbereik dat wordt getest.
- `criteria1`: Waarnaar moet worden gezocht in het eerste celbereik: een getal, een tekst of een test zoals ">5".
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MODE

`MODE(getal1, ...)`

Geeft het getal dat het vaakst voorkomt.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MODE.MULT

`MODE.MULT(getal1, ...)`

Geeft een kolom met de getallen die in een reeks het vaakst voorkomen.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MODE.SNGL

`MODE.SNGL(getal1, ...)`

Geeft het getal dat in een reeks het vaakst voorkomt.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## NEGBINOM.DIST

`NEGBINOM.DIST(aantal_mislukkingen, aantal_successen, kans_succes, cumulatief)`

Geeft de kans op een aantal mislukkingen voor een aantal successen.

- `aantal_mislukkingen`: Het aantal mislukkingen.
- `aantal_successen`: Het aantal successen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## NEGBINOMDIST

`NEGBINOMDIST(aantal_mislukkingen, aantal_successen, kans_succes)`

Geeft de kans op een aantal mislukkingen voor een aantal successen.

- `aantal_mislukkingen`: Het aantal mislukkingen.
- `aantal_successen`: Het aantal successen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.

## NORM.DIST

`NORM.DIST(x, gemiddelde, standaardafwijking, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de normale curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaardafwijking`: De standaarddeviatie van de verdeling.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## NORM.INV

`NORM.INV(kans, gemiddelde, standaardafwijking)`

Geeft de x-waarde voor een aandeel van een normale curve.

- `kans`: Een kans, van 0 tot 1.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaardafwijking`: De standaarddeviatie van de verdeling.

## NORM.S.DIST

`NORM.S.DIST(z, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de standaard normale curve bij z.

- `z`: De z-waarde.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## NORM.S.INV

`NORM.S.INV(kans)`

Geeft de z-waarde voor een aandeel van de standaard normale curve.

- `kans`: Een kans, van 0 tot 1.

## NORMDIST

`NORMDIST(x, gemiddelde, standaardafwijking, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de normale curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaardafwijking`: De standaarddeviatie van de verdeling.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## NORMINV

`NORMINV(kans, gemiddelde, standaardafwijking)`

Geeft de x-waarde voor een aandeel van een normale curve.

- `kans`: Een kans, van 0 tot 1.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaardafwijking`: De standaarddeviatie van de verdeling.

## NORMSDIST

`NORMSDIST(z)`

Geeft het cumulatieve aandeel van de standaard normale curve tot z.

- `z`: De z-waarde.

## NORMSINV

`NORMSINV(kans)`

Geeft de z-waarde voor een aandeel van de standaard normale curve.

- `kans`: Een kans, van 0 tot 1.

## PEARSON

`PEARSON(matrix1, matrix2)`

Geeft hoe sterk twee reeksen getallen samen bewegen, van -1 tot 1.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## PERCENTILE

`PERCENTILE(matrix, k)`

Geeft de waarde waaronder een aandeel k van de getallen valt.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `k`: Het aandeel van de waarden, van 0 tot 1, waarbij de waarde wordt gegeven.

## PERCENTILE.EXC

`PERCENTILE.EXC(matrix, k)`

Geeft de waarde waaronder een aandeel k van de getallen valt, met weglating van 0 en 1.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `k`: De plaats in de volgorde, geteld vanaf 1.

## PERCENTILE.INC

`PERCENTILE.INC(matrix, k)`

Geeft de waarde waaronder een aandeel k van de getallen valt, van 0 tot 1.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `k`: De plaats in de volgorde, geteld vanaf 1.

## PERCENTRANK

`PERCENTRANK(matrix, x, [significantie])`

Geeft de rangorde van een waarde in een reeks getallen als een aandeel van 0 tot 1.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `x`: De x-waarde waarmee moet worden gewerkt.
- `significantie` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.

## PERCENTRANK.EXC

`PERCENTRANK.EXC(matrix, x, [significantie])`

Geeft de rangorde van een waarde in een reeks getallen als een aandeel, met weglating van 0 en 1.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `x`: De x-waarde waarmee moet worden gewerkt.
- `significantie` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.

## PERCENTRANK.INC

`PERCENTRANK.INC(matrix, x, [significantie])`

Geeft de rangorde van een waarde in een reeks getallen als een aandeel van 0 tot 1.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `x`: De x-waarde waarmee moet worden gewerkt.
- `significantie` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.

## PERMUTATIONA

`PERMUTATIONA(getal, aantal_gekozen)`

Geeft het aantal geordende keuzes wanneer een item meer dan één keer mag worden gekozen.

- `getal`: Het getal waarmee moet worden gewerkt.
- `aantal_gekozen`: Het aantal items dat in elke groep wordt gekozen.

## PHI

`PHI(x)`

Geeft de hoogte van de standaard normale curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.

## POISSON

`POISSON(x, gemiddelde, cumulatief)`

Geeft de kans op een aantal gebeurtenissen, op basis van hoeveel er worden verwacht.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## POISSON.DIST

`POISSON.DIST(x, gemiddelde, cumulatief)`

Geeft de kans op een aantal gebeurtenissen, op basis van hoeveel er worden verwacht.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## PROB

`PROB(x_bereik, kans_bereik, ondergrens, [bovengrens])`

Geeft de kans dat waarden tussen twee grenzen vallen.

- `x_bereik`: Het celbereik met de x-waarden.
- `kans_bereik`: De kansen die bij elke x-waarde horen, elk van 0 tot 1, samen 1.
- `ondergrens`: De ondergrens van het bereik.
- `bovengrens` (optioneel): De bovengrens van het bereik (alleen de ondergrens als dit wordt weggelaten).

## QUARTILE

`QUARTILE(matrix, kwartiel)`

Geeft de kwartielwaarden van een reeks getallen.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `kwartiel`: Welk kwartiel: 0 kleinste, 1 eerste kwartiel, 2 mediaan, 3 derde kwartiel, 4 grootste.

## QUARTILE.EXC

`QUARTILE.EXC(matrix, kwartiel)`

Geeft de kwartielwaarden van een reeks getallen, met weglating van de uiteinden.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `kwartiel`: Welk kwartiel: 0 kleinste, 1 eerste kwartiel, 2 mediaan, 3 derde kwartiel, 4 grootste.

## QUARTILE.INC

`QUARTILE.INC(matrix, kwartiel)`

Geeft de kwartielwaarden van een reeks getallen, van de laagste tot de hoogste.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `kwartiel`: Welk kwartiel: 0 kleinste, 1 eerste kwartiel, 2 mediaan, 3 derde kwartiel, 4 grootste.

## RANK

`RANK(getal, verwijzing, [volgorde])`

Geeft de plaats van een getal in een lijst wanneer de lijst op volgorde is gezet.

- `getal`: Het getal waarmee moet worden gewerkt.
- `verwijzing`: Een cel of celbereik.
- `volgorde` (optioneel): 0 of weggelaten zet het grootste eerst; elk ander getal zet het kleinste eerst.

## RANK.AVG

`RANK.AVG(getal, verwijzing, [volgorde])`

Geeft de rangorde van een getal in een lijst, en de gemiddelde rangorde als er gelijke waarden zijn.

- `getal`: Het getal waarmee moet worden gewerkt.
- `verwijzing`: Een cel of celbereik.
- `volgorde` (optioneel): 0 of weggelaten zet het grootste eerst; elk ander getal zet het kleinste eerst.

## RANK.EQ

`RANK.EQ(getal, verwijzing, [volgorde])`

Geeft de rangorde van een getal in een lijst, en de hoogste rangorde als er gelijke waarden zijn.

- `getal`: Het getal waarmee moet worden gewerkt.
- `verwijzing`: Een cel of celbereik.
- `volgorde` (optioneel): 0 of weggelaten zet het grootste eerst; elk ander getal zet het kleinste eerst.

## RSQ

`RSQ(bekende_y's, bekende_x's)`

Geeft hoe goed een rechte lijn past bij twee reeksen getallen.

- `bekende_y's`: Het celbereik dat de y-waarden bevat die u al hebt.
- `bekende_x's`: Het celbereik dat de x-waarden bevat die u al hebt.

## SKEW

`SKEW(getal1, ...)`

Geeft hoe scheef een reeks getallen is rond het gemiddelde, voor een steekproef.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## SKEW.P

`SKEW.P(getal1, ...)`

Geeft hoe scheef een reeks getallen is rond het gemiddelde, voor een hele populatie.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## SLOPE

`SLOPE(bekende_y's, bekende_x's)`

Geeft de helling van de rechte lijn die het best past bij twee reeksen getallen.

- `bekende_y's`: Het celbereik dat de y-waarden bevat die u al hebt.
- `bekende_x's`: Het celbereik dat de x-waarden bevat die u al hebt.

## SMALL

`SMALL(matrix, k)`

Geeft het k-de kleinste getal van een reeks.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `k`: Welke plaats wordt gegeven, geteld vanaf het kleinste: 1 is het kleinste.

## STANDARDIZE

`STANDARDIZE(x, gemiddelde, standaardafwijking)`

Geeft hoeveel standaarddeviaties een waarde van het gemiddelde af ligt.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaardafwijking`: De standaarddeviatie van de verdeling.

## STDEV

`STDEV(getal1, ...)`

Schat de spreiding van een hele groep op basis van een steekproef daarvan.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STDEV.P

`STDEV.P(getal1, ...)`

Geeft de spreiding van een hele groep.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STDEV.S

`STDEV.S(getal1, ...)`

Schat de spreiding van een hele groep op basis van een steekproef daarvan.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STDEVA

`STDEVA(waarde1, ...)`

Geeft de standaarddeviatie van een steekproef, waarbij tekst als 0 en waar als 1 wordt geteld.

- `waarde1`: De eerste waarde, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STDEVP

`STDEVP(getal1, ...)`

Geeft de spreiding van een hele groep.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STDEVPA

`STDEVPA(waarde1, ...)`

Geeft de standaarddeviatie van een populatie, waarbij tekst als 0 en waar als 1 wordt geteld.

- `waarde1`: De eerste waarde, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STEYX

`STEYX(bekende_y's, bekende_x's)`

Geeft de standaardfout van de voorspelde y-waarden van een passende lijn.

- `bekende_y's`: Het celbereik dat de y-waarden bevat die u al hebt.
- `bekende_x's`: Het celbereik dat de x-waarden bevat die u al hebt.

## SUMIF

`SUMIF(bereik, criteria, [sombereik])`

Telt de cellen op die aan een voorwaarde voldoen.

- `bereik`: Het celbereik dat wordt getest.
- `criteria`: Waarnaar moet worden gezocht: een getal, een tekst of een test zoals ">5".
- `sombereik` (optioneel): De cellen die worden opgeteld (de cellen die worden getest als dit wordt weggelaten).

## SUMIFS

`SUMIFS(sombereik, criteria_bereik1, criteria1, ...)`

Telt de cellen op die aan meerdere voorwaarden voldoen.

- `sombereik`: De cellen die worden opgeteld (de cellen die worden getest als dit wordt weggelaten).
- `criteria_bereik1`: Het eerste celbereik dat wordt getest.
- `criteria1`: Waarnaar moet worden gezocht in het eerste celbereik: een getal, een tekst of een test zoals ">5".
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## T.DIST

`T.DIST(x, vrijheidsgraden, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de t-curve van Student bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## T.DIST.2T

`T.DIST.2T(x, vrijheidsgraden)`

Geeft het aandeel van de t-curve van Student buiten min x en x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.

## T.DIST.RT

`T.DIST.RT(x, vrijheidsgraden)`

Geeft het aandeel van de t-curve van Student boven x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.

## T.INV

`T.INV(kans, vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de t-curve van Student, gerekend vanaf links.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.

## T.INV.2T

`T.INV.2T(kans, vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de t-curve van Student, verdeeld over beide staarten.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.

## T.TEST

`T.TEST(matrix1, matrix2, staarten, type)`

Geeft de kans dat de gemiddelden van twee reeksen getallen gelijk zijn.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.
- `staarten`: 1 voor één staart van de curve, 2 voor beide.
- `type`: Wanneer betalingen worden gedaan: 0 of weggelaten aan het eind van elke periode, 1 aan het begin.

## TDIST

`TDIST(x, vrijheidsgraden, staarten)`

Geeft het aandeel van de t-curve van Student in één staart of in beide.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.
- `staarten`: 1 voor één staart van de curve, 2 voor beide.

## TINV

`TINV(kans, vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de t-curve van Student, verdeeld over beide staarten.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of meer.

## TREND

`TREND(bekende_y's, [bekende_x's], [nieuwe_x's], [const])`

Geeft de waarden langs de best passende rechte lijn voor nieuwe x-waarden.

- `bekende_y's`: Het celbereik dat de y-waarden bevat die u al hebt.
- `bekende_x's` (optioneel): Het celbereik dat de x-waarden bevat die u al hebt.
- `nieuwe_x's` (optioneel): De x-waarden waarvoor de y-waarden worden gegeven (de bekende x-waarden als dit wordt weggelaten).
- `const` (optioneel): Waar of weggelaten om een lijn te laten passen die niet door nul hoeft te gaan; onwaar om de lijn door nul te laten gaan.

## TRIMMEAN

`TRIMMEAN(matrix, percentage)`

Geeft het gemiddelde van een reeks getallen nadat een aandeel van de hoogste en laagste is weggelaten.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `percentage`: Het aandeel van de waarden, van 0 tot maar niet inclusief 1, dat aan beide uiteinden wordt weggelaten.

## TTEST

`TTEST(matrix1, matrix2, staarten, type)`

Geeft de kans dat de gemiddelden van twee reeksen getallen gelijk zijn.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.
- `staarten`: 1 voor één staart van de curve, 2 voor beide.
- `type`: Wanneer betalingen worden gedaan: 0 of weggelaten aan het eind van elke periode, 1 aan het begin.

## VAR

`VAR(getal1, ...)`

Schat de variantie van een hele groep op basis van een steekproef daarvan.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## VAR.P

`VAR.P(getal1, ...)`

Geeft de variantie van een hele groep.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## VAR.S

`VAR.S(getal1, ...)`

Schat de variantie van een hele groep op basis van een steekproef daarvan.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## VARA

`VARA(waarde1, ...)`

Geeft de variantie van een steekproef, waarbij tekst als 0 en waar als 1 wordt geteld.

- `waarde1`: De eerste waarde, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## VARP

`VARP(getal1, ...)`

Geeft de variantie van een hele groep.

- `getal1`: Het eerste getal, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## VARPA

`VARPA(waarde1, ...)`

Geeft de variantie van een populatie, waarbij tekst als 0 en waar als 1 wordt geteld.

- `waarde1`: De eerste waarde, of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## WEIBULL

`WEIBULL(x, alfa, bèta, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de Weibullcurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de curve vormgeeft (de schaal van de curve voor de gamma- en Weibullcurven).
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## WEIBULL.DIST

`WEIBULL.DIST(x, alfa, bèta, cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de Weibullcurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, zoals 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de curve vormgeeft (de schaal van de curve voor de gamma- en Weibullcurven).
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## Z.TEST

`Z.TEST(matrix, x, [sigma])`

Geeft de eenzijdige kans dat het gemiddelde van een steekproef boven een waarde ligt.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `x`: De x-waarde waarmee moet worden gewerkt.
- `sigma` (optioneel): De standaarddeviatie van de hele populatie (die van de steekproef als dit wordt weggelaten).

## ZTEST

`ZTEST(matrix, x, [sigma])`

Geeft de eenzijdige kans dat het gemiddelde van een steekproef boven een waarde ligt.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `x`: De x-waarde waarmee moet worden gewerkt.
- `sigma` (optioneel): De standaarddeviatie van de hele populatie (die van de steekproef als dit wordt weggelaten).

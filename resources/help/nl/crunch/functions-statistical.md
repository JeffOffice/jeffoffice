# Statistische functies
<!-- keywords: functies, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST -->

Dit zijn de statistische functies die JeffCrunch kan berekenen. Bij elke functie ziet u hoe ze wordt geschreven, wat ze doet en wat elk van de argumenten betekent. Een argument tussen haakjes is optioneel, en "..." betekent dat u zoveel extra argumenten van het laatste soort kunt opgeven als nodig is. Om een functie uit te proberen, zie [Een functie kiezen](insert-function). Terwijl u in een cel een functie typt, verschijnt onder de cel een kleine tip met de argumenten. Als u in die tip op de naam van de functie klikt, opent u het onderdeel hierover.

## AVEDEV

`AVEDEV(getal1; ...)`

Geeft het gemiddelde van de afstanden van getallen tot hun gemiddelde.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## AVERAGE

`AVERAGE(getal1; ...)`

Geeft het gemiddelde van getallen.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## AVERAGEA

`AVERAGEA(waarde1; ...)`

Geeft het gemiddelde van waarden, waarbij tekst als nul en waar als één wordt geteld.

- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## AVERAGEIF

`AVERAGEIF(bereik; criterium; [gemiddelde_bereik])`

Berekent het gemiddelde van de cellen die aan een voorwaarde voldoen.

- `bereik`: Het celbereik dat wordt getest.
- `criterium`: Waarnaar moet worden gezocht: een getal, een tekst of een test zoals ">5".
- `gemiddelde_bereik` (optioneel): De cellen waarvan het gemiddelde wordt berekend (de cellen waarin wordt gezocht als dit wordt weggelaten).

## AVERAGEIFS

`AVERAGEIFS(gemiddelde_bereik; criteriumbereik1; criterium1; ...)`

Berekent het gemiddelde van de cellen die aan meerdere voorwaarden voldoen.

- `gemiddelde_bereik`: De cellen waarvan het gemiddelde wordt berekend (de cellen waarin wordt gezocht als dit wordt weggelaten).
- `criteriumbereik1`: Het eerste celbereik dat wordt getest.
- `criterium1`: Waarnaar moet worden gezocht in het eerste celbereik: een getal, een tekst of een test zoals ">5".
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## BETA.DIST

`BETA.DIST(x; alfa; bèta; cumulatief; [A]; [B])`

Geeft de waarde of het cumulatieve aandeel van de bètacurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de vorm van de curve bepaalt (de schaal van de curve bij de gammacurve en de Weibullcurve).
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.
- `A` (optioneel): De ondergrens van x (0 als dit wordt weggelaten).
- `B` (optioneel): De bovengrens van x (1 als dit wordt weggelaten).

## BETA.INV

`BETA.INV(kans; alfa; bèta; [A]; [B])`

Geeft de x-waarde voor een aandeel van een bètacurve.

- `kans`: Een kans, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de vorm van de curve bepaalt (de schaal van de curve bij de gammacurve en de Weibullcurve).
- `A` (optioneel): De ondergrens van x (0 als dit wordt weggelaten).
- `B` (optioneel): De bovengrens van x (1 als dit wordt weggelaten).

## BETADIST

`BETADIST(x; alfa; bèta; [A]; [B])`

Geeft het cumulatieve aandeel van de bètacurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de vorm van de curve bepaalt (de schaal van de curve bij de gammacurve en de Weibullcurve).
- `A` (optioneel): De ondergrens van x (0 als dit wordt weggelaten).
- `B` (optioneel): De bovengrens van x (1 als dit wordt weggelaten).

## BETAINV

`BETAINV(kans; alfa; bèta; [A]; [B])`

Geeft de x-waarde voor een aandeel van een bètacurve.

- `kans`: Een kans, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de vorm van de curve bepaalt (de schaal van de curve bij de gammacurve en de Weibullcurve).
- `A` (optioneel): De ondergrens van x (0 als dit wordt weggelaten).
- `B` (optioneel): De bovengrens van x (1 als dit wordt weggelaten).

## BINOM.DIST

`BINOM.DIST(aantal_successen; pogingen; kans_succes; cumulatief)`

Geeft de kans op een aantal successen in een aantal pogingen.

- `aantal_successen`: Het aantal successen.
- `pogingen`: Het aantal pogingen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(pogingen; kans_succes; aantal_successen; [aantal_successen2])`

Geeft de kans dat het aantal successen binnen een bereik valt.

- `pogingen`: Het aantal pogingen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `aantal_successen`: Het aantal successen.
- `aantal_successen2` (optioneel): Het maximale aantal successen dat moet worden geteld (hetzelfde als het minimum als dit wordt weggelaten).

## BINOM.INV

`BINOM.INV(pogingen; kans_succes; alfa)`

Geeft het minimale aantal successen waarbij de cumulatieve kans een waarde bereikt.

- `pogingen`: Het aantal pogingen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.

## BINOMDIST

`BINOMDIST(aantal_successen; pogingen; kans_succes; cumulatief)`

Geeft de kans op een aantal successen in een aantal pogingen.

- `aantal_successen`: Het aantal successen.
- `pogingen`: Het aantal pogingen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## CHIDIST

`CHIDIST(x; vrijheidsgraden)`

Geeft het aandeel van de chi-kwadraatcurve boven x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.

## CHIINV

`CHIINV(kans; vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de chi-kwadraatcurve, vanaf rechts geteld.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.

## CHISQ.DIST

`CHISQ.DIST(x; vrijheidsgraden; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de chi-kwadraatcurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x; vrijheidsgraden)`

Geeft het aandeel van de chi-kwadraatcurve boven x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.

## CHISQ.INV

`CHISQ.INV(kans; vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de chi-kwadraatcurve, vanaf links geteld.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.

## CHISQ.INV.RT

`CHISQ.INV.RT(kans; vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de chi-kwadraatcurve, vanaf rechts geteld.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.

## CHISQ.TEST

`CHISQ.TEST(werkelijk_bereik; verwacht_bereik)`

Geeft de kans dat tellingen alleen door toeval afwijken van de verwachte tellingen.

- `werkelijk_bereik`: Het celbereik met de tellingen die u hebt gekregen.
- `verwacht_bereik`: Het celbereik met de tellingen die u had verwacht, even groot als de tellingen die u hebt gekregen.

## CHITEST

`CHITEST(werkelijk_bereik; verwacht_bereik)`

Geeft de kans dat tellingen alleen door toeval afwijken van de verwachte tellingen.

- `werkelijk_bereik`: Het celbereik met de tellingen die u hebt gekregen.
- `verwacht_bereik`: Het celbereik met de tellingen die u had verwacht, even groot als de tellingen die u hebt gekregen.

## CONFIDENCE

`CONFIDENCE(alfa; standaarddeviatie; omvang)`

Geeft de marge van een betrouwbaarheidsinterval voor een gemiddelde, met de normaalcurve.

- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.
- `omvang`: De omvang van de steekproef, een geheel getal van 1 of hoger.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(alfa; standaarddeviatie; omvang)`

Geeft de marge van een betrouwbaarheidsinterval voor een gemiddelde, met de normaalcurve.

- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.
- `omvang`: De omvang van de steekproef, een geheel getal van 1 of hoger.

## CONFIDENCE.T

`CONFIDENCE.T(alfa; standaarddeviatie; omvang)`

Geeft de marge van een betrouwbaarheidsinterval voor een gemiddelde, met de t-curve van Student.

- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.
- `omvang`: De omvang van de steekproef, een geheel getal van 1 of hoger.

## CORREL

`CORREL(matrix1; matrix2)`

Geeft hoe sterk twee reeksen getallen samen bewegen, van -1 tot 1.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## COUNT

`COUNT(waarde1; ...)`

Telt de getallen in een lijst of een celbereik.

- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## COUNTA

`COUNTA(waarde1; ...)`

Telt de cellen die niet leeg zijn.

- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## COUNTBLANK

`COUNTBLANK(bereik)`

Telt de lege cellen in een bereik.

- `bereik`: Het celbereik dat wordt getest.

## COUNTIF

`COUNTIF(bereik; criterium)`

Telt de cellen die aan een voorwaarde voldoen.

- `bereik`: Het celbereik dat wordt getest.
- `criterium`: Waarnaar moet worden gezocht: een getal, een tekst of een test zoals ">5".

## COUNTIFS

`COUNTIFS(criteriumbereik1; criterium1; ...)`

Telt de rijen waarin aan meerdere voorwaarden tegelijk is voldaan.

- `criteriumbereik1`: Het eerste celbereik dat wordt getest.
- `criterium1`: Waarnaar moet worden gezocht in het eerste celbereik: een getal, een tekst of een test zoals ">5".
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## COVAR

`COVAR(matrix1; matrix2)`

Geeft hoeveel twee reeksen getallen samen variëren.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## COVARIANCE.P

`COVARIANCE.P(matrix1; matrix2)`

Geeft hoeveel twee reeksen getallen samen variëren, voor een populatie.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## COVARIANCE.S

`COVARIANCE.S(matrix1; matrix2)`

Geeft hoeveel twee reeksen getallen samen variëren, voor een steekproef.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## CRITBINOM

`CRITBINOM(pogingen; kans_succes; alfa)`

Geeft het minimale aantal successen waarbij de cumulatieve kans een waarde bereikt.

- `pogingen`: Het aantal pogingen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.

## DEVSQ

`DEVSQ(getal1; ...)`

Geeft de som van de kwadraten van de afstanden van getallen tot hun gemiddelde.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## EXPON.DIST

`EXPON.DIST(x; lambda; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de exponentiële curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `lambda`: De snelheid van de curve, groter dan 0.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## EXPONDIST

`EXPONDIST(x; lambda; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de exponentiële curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `lambda`: De snelheid van de curve, groter dan 0.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## F.DIST

`F.DIST(x; vrijheidsgraden1; vrijheidsgraden2; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de F-curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of hoger.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of hoger.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## F.DIST.RT

`F.DIST.RT(x; vrijheidsgraden1; vrijheidsgraden2)`

Geeft het aandeel van de F-curve boven x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of hoger.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of hoger.

## F.INV

`F.INV(kans; vrijheidsgraden1; vrijheidsgraden2)`

Geeft de x-waarde voor een aandeel van de F-curve, vanaf links geteld.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of hoger.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of hoger.

## F.INV.RT

`F.INV.RT(kans; vrijheidsgraden1; vrijheidsgraden2)`

Geeft de x-waarde voor een aandeel van de F-curve, vanaf rechts geteld.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of hoger.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of hoger.

## F.TEST

`F.TEST(matrix1; matrix2)`

Geeft de kans dat twee reeksen getallen dezelfde variantie hebben.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## FDIST

`FDIST(x; vrijheidsgraden1; vrijheidsgraden2)`

Geeft het aandeel van de F-curve boven x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of hoger.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of hoger.

## FINV

`FINV(kans; vrijheidsgraden1; vrijheidsgraden2)`

Geeft de x-waarde voor een aandeel van de F-curve, vanaf rechts geteld.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden1`: De vrijheidsgraden van de teller, een geheel getal van 1 of hoger.
- `vrijheidsgraden2`: De vrijheidsgraden van de noemer, een geheel getal van 1 of hoger.

## FISHER

`FISHER(x)`

Geeft de transformatie van Fisher van een getal tussen -1 en 1.

- `x`: De x-waarde waarmee moet worden gewerkt.

## FISHERINV

`FISHERINV(y)`

Geeft de inverse van de transformatie van Fisher.

- `y`: Het getal waarvan de inverse van de transformatie van Fisher moet worden berekend.

## FORECAST

`FORECAST(x; bekende_y's; bekende_x's)`

Voorspelt een waarde langs de best passende rechte lijn.

- `x`: De x-waarde waarvoor een y-waarde moet worden voorspeld.
- `bekende_y's`: Het celbereik met de y-waarden die u al hebt.
- `bekende_x's`: Het celbereik met de x-waarden die u al hebt.

## FORECAST.LINEAR

`FORECAST.LINEAR(x; bekende_y's; bekende_x's)`

Voorspelt een waarde langs de best passende rechte lijn.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `bekende_y's`: Het celbereik met de y-waarden die u al hebt.
- `bekende_x's`: Het celbereik met de x-waarden die u al hebt.

## FREQUENCY

`FREQUENCY(gegevensmatrix; klassematrix)`

Telt hoeveel waarden in elk bereik vallen en geeft de tellingen als kolom.

- `gegevensmatrix`: Het celbereik met de waarden die moeten worden geteld.
- `klassematrix`: Het celbereik met de grenzen van de bereiken waarin wordt geteld.

## FTEST

`FTEST(matrix1; matrix2)`

Geeft de kans dat twee reeksen getallen dezelfde variantie hebben.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## GAMMA

`GAMMA(x)`

Geeft de gammafunctie van een getal.

- `x`: De x-waarde waarmee moet worden gewerkt.

## GAMMA.DIST

`GAMMA.DIST(x; alfa; bèta; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de gammacurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de vorm van de curve bepaalt (de schaal van de curve bij de gammacurve en de Weibullcurve).
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## GAMMA.INV

`GAMMA.INV(kans; alfa; bèta)`

Geeft de x-waarde voor een aandeel van een gammacurve.

- `kans`: Een kans, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de vorm van de curve bepaalt (de schaal van de curve bij de gammacurve en de Weibullcurve).

## GAMMADIST

`GAMMADIST(x; alfa; bèta; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de gammacurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de vorm van de curve bepaalt (de schaal van de curve bij de gammacurve en de Weibullcurve).
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## GAMMAINV

`GAMMAINV(kans; alfa; bèta)`

Geeft de x-waarde voor een aandeel van een gammacurve.

- `kans`: Een kans, van 0 tot 1.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de vorm van de curve bepaalt (de schaal van de curve bij de gammacurve en de Weibullcurve).

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

Geeft het aandeel van de standaardnormale curve tussen het gemiddelde en z.

- `z`: De z-waarde.

## GEOMEAN

`GEOMEAN(getal1; ...)`

Geeft het meetkundig gemiddelde van positieve getallen.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## GROWTH

`GROWTH(bekende_y's; [bekende_x's]; [nieuwe_x's]; [const])`

Geeft de waarden langs de best passende exponentiële curve voor nieuwe x-waarden.

- `bekende_y's`: Het celbereik met de y-waarden die u al hebt.
- `bekende_x's` (optioneel): Het celbereik met de x-waarden die u al hebt.
- `nieuwe_x's` (optioneel): De x-waarden waarvoor de y-waarden moeten worden gegeven (de bekende x-waarden als dit wordt weggelaten).
- `const` (optioneel): Waar of weggelaten om een lijn te laten passen die niet door nul hoeft te gaan; onwaar om de lijn door nul te laten gaan.

## HARMEAN

`HARMEAN(getal1; ...)`

Geeft het harmonisch gemiddelde van positieve getallen.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## HYPGEOM.DIST

`HYPGEOM.DIST(aantal_successen_steekproef; aantal_steekproef; aantal_successen_populatie; aantal_populatie; cumulatief)`

Geeft de kans op het trekken van een aantal successen zonder dat items worden teruggelegd.

- `aantal_successen_steekproef`: Het aantal successen in de steekproef.
- `aantal_steekproef`: Het aantal getrokken items.
- `aantal_successen_populatie`: Het aantal successen in de hele populatie.
- `aantal_populatie`: Het aantal items in de hele populatie.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## HYPGEOMDIST

`HYPGEOMDIST(aantal_successen_steekproef; aantal_steekproef; aantal_successen_populatie; aantal_populatie)`

Geeft de kans op het trekken van een aantal successen zonder dat items worden teruggelegd.

- `aantal_successen_steekproef`: Het aantal successen in de steekproef.
- `aantal_steekproef`: Het aantal getrokken items.
- `aantal_successen_populatie`: Het aantal successen in de hele populatie.
- `aantal_populatie`: Het aantal items in de hele populatie.

## INTERCEPT

`INTERCEPT(bekende_y's; bekende_x's)`

Geeft het punt waar de best passende rechte lijn de verticale as snijdt.

- `bekende_y's`: Het celbereik met de y-waarden die u al hebt.
- `bekende_x's`: Het celbereik met de x-waarden die u al hebt.

## KURT

`KURT(getal1; ...)`

Geeft hoe puntig of plat een reeks getallen is in vergelijking met een normaalcurve.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## LARGE

`LARGE(matrix; k)`

Geeft het k-de grootste getal van een reeks.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `k`: Welke plaats moet worden gegeven, geteld vanaf het grootste: 1 is het grootste.

## LINEST

`LINEST(bekende_y's; [bekende_x's]; [const]; [statistieken])`

Past een rechte lijn of vlak op de gegevens met de kleinste-kwadratenmethode en geeft de getallen als rij.

- `bekende_y's`: Het celbereik met de y-waarden die u al hebt.
- `bekende_x's` (optioneel): Het celbereik met de x-waarden die u al hebt.
- `const` (optioneel): Waar of weggelaten om een lijn te laten passen die niet door nul hoeft te gaan; onwaar om de lijn door nul te laten gaan.
- `statistieken` (optioneel): Waar om ook meer getallen over de pasvorm te geven; onwaar of weggelaten om alleen de lijn te geven.

## LOGEST

`LOGEST(bekende_y's; [bekende_x's]; [const]; [statistieken])`

Past een exponentiële curve op de gegevens en geeft de getallen als rij.

- `bekende_y's`: Het celbereik met de y-waarden die u al hebt.
- `bekende_x's` (optioneel): Het celbereik met de x-waarden die u al hebt.
- `const` (optioneel): Waar of weggelaten om een lijn te laten passen die niet door nul hoeft te gaan; onwaar om de lijn door nul te laten gaan.
- `statistieken` (optioneel): Waar om ook meer getallen over de pasvorm te geven; onwaar of weggelaten om alleen de lijn te geven.

## LOGINV

`LOGINV(kans; gemiddelde; standaarddeviatie)`

Geeft de x-waarde voor een aandeel van een lognormale curve.

- `kans`: Een kans, van 0 tot 1.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.

## LOGNORM.DIST

`LOGNORM.DIST(x; gemiddelde; standaarddeviatie; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de lognormale curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## LOGNORM.INV

`LOGNORM.INV(kans; gemiddelde; standaarddeviatie)`

Geeft de x-waarde voor een aandeel van een lognormale curve.

- `kans`: Een kans, van 0 tot 1.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.

## LOGNORMDIST

`LOGNORMDIST(x; gemiddelde; standaarddeviatie)`

Geeft het cumulatieve aandeel van de lognormale curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.

## MAXA

`MAXA(waarde1; ...)`

Geeft de grootste waarde, waarbij tekst als nul en waar als één wordt geteld.

- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MAXIFS

`MAXIFS(maximumbereik; criteriumbereik1; criterium1; ...)`

Geeft het grootste getal onder de cellen die aan meerdere voorwaarden voldoen.

- `maximumbereik`: De cellen waarvan de grootste waarde wordt genomen.
- `criteriumbereik1`: Het eerste celbereik dat wordt getest.
- `criterium1`: Waarnaar moet worden gezocht in het eerste celbereik: een getal, een tekst of een test zoals ">5".
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MEDIAN

`MEDIAN(getal1; ...)`

Geeft het middelste getal van een reeks.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MINA

`MINA(waarde1; ...)`

Geeft de kleinste waarde, waarbij tekst als nul en waar als één wordt geteld.

- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MINIFS

`MINIFS(minimumbereik; criteriumbereik1; criterium1; ...)`

Geeft het kleinste getal onder de cellen die aan meerdere voorwaarden voldoen.

- `minimumbereik`: De cellen waarvan de kleinste waarde wordt genomen.
- `criteriumbereik1`: Het eerste celbereik dat wordt getest.
- `criterium1`: Waarnaar moet worden gezocht in het eerste celbereik: een getal, een tekst of een test zoals ">5".
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MODE

`MODE(getal1; ...)`

Geeft het getal dat het vaakst voorkomt.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MODE.MULT

`MODE.MULT(getal1; ...)`

Geeft een kolom met de getallen die het vaakst voorkomen in een reeks.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## MODE.SNGL

`MODE.SNGL(getal1; ...)`

Geeft het getal dat het vaakst voorkomt in een reeks.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## NEGBINOM.DIST

`NEGBINOM.DIST(aantal_mislukkingen; aantal_successen; kans_succes; cumulatief)`

Geeft de kans op een aantal mislukkingen voor een aantal successen.

- `aantal_mislukkingen`: Het aantal mislukkingen.
- `aantal_successen`: Het aantal successen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## NEGBINOMDIST

`NEGBINOMDIST(aantal_mislukkingen; aantal_successen; kans_succes)`

Geeft de kans op een aantal mislukkingen voor een aantal successen.

- `aantal_mislukkingen`: Het aantal mislukkingen.
- `aantal_successen`: Het aantal successen.
- `kans_succes`: De kans op een succes bij elke poging, van 0 tot 1.

## NORM.DIST

`NORM.DIST(x; gemiddelde; standaarddeviatie; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de normaalcurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## NORM.INV

`NORM.INV(kans; gemiddelde; standaarddeviatie)`

Geeft de x-waarde voor een aandeel van een normaalcurve.

- `kans`: Een kans, van 0 tot 1.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.

## NORM.S.DIST

`NORM.S.DIST(z; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de standaardnormale curve bij z.

- `z`: De z-waarde.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## NORM.S.INV

`NORM.S.INV(kans)`

Geeft de z-waarde voor een aandeel van de standaardnormale curve.

- `kans`: Een kans, van 0 tot 1.

## NORMDIST

`NORMDIST(x; gemiddelde; standaarddeviatie; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de normaalcurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## NORMINV

`NORMINV(kans; gemiddelde; standaarddeviatie)`

Geeft de x-waarde voor een aandeel van een normaalcurve.

- `kans`: Een kans, van 0 tot 1.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.

## NORMSDIST

`NORMSDIST(z)`

Geeft het cumulatieve aandeel van de standaardnormale curve tot z.

- `z`: De z-waarde.

## NORMSINV

`NORMSINV(kans)`

Geeft de z-waarde voor een aandeel van de standaardnormale curve.

- `kans`: Een kans, van 0 tot 1.

## PEARSON

`PEARSON(matrix1; matrix2)`

Geeft hoe sterk twee reeksen getallen samen bewegen, van -1 tot 1.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.

## PERCENTILE

`PERCENTILE(matrix; k)`

Geeft de waarde waaronder een aandeel k van de getallen valt.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `k`: Het aandeel van de waarden, van 0 tot 1, waarvoor de waarde moet worden gegeven.

## PERCENTILE.EXC

`PERCENTILE.EXC(matrix; k)`

Geeft de waarde waaronder een aandeel k van de getallen valt, met uitsluiting van 0 en 1.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `k`: De plaats in de volgorde, geteld vanaf 1.

## PERCENTILE.INC

`PERCENTILE.INC(matrix; k)`

Geeft de waarde waaronder een aandeel k van de getallen valt, van 0 tot 1.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `k`: De plaats in de volgorde, geteld vanaf 1.

## PERCENTRANK

`PERCENTRANK(matrix; x; [significantie])`

Geeft de rangorde van een waarde in een reeks getallen als aandeel van 0 tot 1.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `x`: De x-waarde waarmee moet worden gewerkt.
- `significantie` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.

## PERCENTRANK.EXC

`PERCENTRANK.EXC(matrix; x; [significantie])`

Geeft de rangorde van een waarde in een reeks getallen als aandeel, met uitsluiting van 0 en 1.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `x`: De x-waarde waarmee moet worden gewerkt.
- `significantie` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.

## PERCENTRANK.INC

`PERCENTRANK.INC(matrix; x; [significantie])`

Geeft de rangorde van een waarde in een reeks getallen als aandeel van 0 tot 1.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `x`: De x-waarde waarmee moet worden gewerkt.
- `significantie` (optioneel): Het getal naar het veelvoud waarvan wordt afgerond.

## PERMUTATIONA

`PERMUTATIONA(getal; gekozen_aantal)`

Geeft het aantal geordende keuzes wanneer een item meer dan één keer mag worden gekozen.

- `getal`: Het getal waarmee moet worden gewerkt.
- `gekozen_aantal`: Het aantal items dat in elke groep wordt gekozen.

## PHI

`PHI(x)`

Geeft de hoogte van de standaardnormale curve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.

## POISSON

`POISSON(x; gemiddelde; cumulatief)`

Geeft de kans op een aantal gebeurtenissen, op basis van het aantal dat u verwacht.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## POISSON.DIST

`POISSON.DIST(x; gemiddelde; cumulatief)`

Geeft de kans op een aantal gebeurtenissen, op basis van het aantal dat u verwacht.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## PROB

`PROB(x_bereik; kans_bereik; ondergrens; [bovengrens])`

Geeft de kans dat waarden tussen twee grenzen vallen.

- `x_bereik`: Het celbereik met de x-waarden.
- `kans_bereik`: De kansen die bij elke x-waarde horen, elk van 0 tot 1, samen 1.
- `ondergrens`: De ondergrens van het bereik.
- `bovengrens` (optioneel): De bovengrens van het bereik (alleen de ondergrens als dit wordt weggelaten).

## QUARTILE

`QUARTILE(matrix; kwartiel)`

Geeft de kwartielwaarden van een reeks getallen.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `kwartiel`: Welk kwartiel: 0 het kleinste, 1 het eerste kwartiel, 2 de mediaan, 3 het derde kwartiel, 4 het grootste.

## QUARTILE.EXC

`QUARTILE.EXC(matrix; kwartiel)`

Geeft de kwartielwaarden van een reeks getallen, met uitsluiting van de uiteinden.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `kwartiel`: Welk kwartiel: 0 het kleinste, 1 het eerste kwartiel, 2 de mediaan, 3 het derde kwartiel, 4 het grootste.

## QUARTILE.INC

`QUARTILE.INC(matrix; kwartiel)`

Geeft de kwartielwaarden van een reeks getallen, van de laagste tot de hoogste waarde.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `kwartiel`: Welk kwartiel: 0 het kleinste, 1 het eerste kwartiel, 2 de mediaan, 3 het derde kwartiel, 4 het grootste.

## RANK

`RANK(getal; verwijzing; [volgorde])`

Geeft de plaats van een getal in een lijst wanneer de lijst wordt geordend.

- `getal`: Het getal waarmee moet worden gewerkt.
- `verwijzing`: Een cel of een celbereik.
- `volgorde` (optioneel): 0 of weggelaten zet het grootste eerst; elk ander getal zet het kleinste eerst.

## RANK.AVG

`RANK.AVG(getal; verwijzing; [volgorde])`

Geeft de rangorde van een getal in een lijst, en de gemiddelde rangorde als er gelijke waarden zijn.

- `getal`: Het getal waarmee moet worden gewerkt.
- `verwijzing`: Een cel of een celbereik.
- `volgorde` (optioneel): 0 of weggelaten zet het grootste eerst; elk ander getal zet het kleinste eerst.

## RANK.EQ

`RANK.EQ(getal; verwijzing; [volgorde])`

Geeft de rangorde van een getal in een lijst, en de hoogste rangorde als er gelijke waarden zijn.

- `getal`: Het getal waarmee moet worden gewerkt.
- `verwijzing`: Een cel of een celbereik.
- `volgorde` (optioneel): 0 of weggelaten zet het grootste eerst; elk ander getal zet het kleinste eerst.

## RSQ

`RSQ(bekende_y's; bekende_x's)`

Geeft hoe goed een rechte lijn past bij twee reeksen getallen.

- `bekende_y's`: Het celbereik met de y-waarden die u al hebt.
- `bekende_x's`: Het celbereik met de x-waarden die u al hebt.

## SKEW

`SKEW(getal1; ...)`

Geeft hoe scheef een reeks getallen is rond het gemiddelde, voor een steekproef.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## SKEW.P

`SKEW.P(getal1; ...)`

Geeft hoe scheef een reeks getallen is rond het gemiddelde, voor een hele populatie.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## SLOPE

`SLOPE(bekende_y's; bekende_x's)`

Geeft de helling van de rechte lijn die het best past bij twee reeksen getallen.

- `bekende_y's`: Het celbereik met de y-waarden die u al hebt.
- `bekende_x's`: Het celbereik met de x-waarden die u al hebt.

## SMALL

`SMALL(matrix; k)`

Geeft het k-de kleinste getal van een reeks.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `k`: Welke plaats moet worden gegeven, geteld vanaf het kleinste: 1 is het kleinste.

## STANDARDIZE

`STANDARDIZE(x; gemiddelde; standaarddeviatie)`

Geeft hoeveel standaarddeviaties een waarde van het gemiddelde afligt.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `gemiddelde`: Het gemiddelde van de verdeling.
- `standaarddeviatie`: De standaarddeviatie van de verdeling.

## STDEV

`STDEV(getal1; ...)`

Schat de spreiding van een hele groep op basis van een steekproef daaruit.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STDEV.P

`STDEV.P(getal1; ...)`

Geeft de spreiding van een hele groep.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STDEV.S

`STDEV.S(getal1; ...)`

Schat de spreiding van een hele groep op basis van een steekproef daaruit.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STDEVA

`STDEVA(waarde1; ...)`

Geeft de standaarddeviatie van een steekproef, waarbij tekst als 0 en waar als 1 wordt geteld.

- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STDEVP

`STDEVP(getal1; ...)`

Geeft de spreiding van een hele groep.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STDEVPA

`STDEVPA(waarde1; ...)`

Geeft de standaarddeviatie van een populatie, waarbij tekst als 0 en waar als 1 wordt geteld.

- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## STEYX

`STEYX(bekende_y's; bekende_x's)`

Geeft de standaardfout van de voorspelde y-waarden van een gevonden lijn.

- `bekende_y's`: Het celbereik met de y-waarden die u al hebt.
- `bekende_x's`: Het celbereik met de x-waarden die u al hebt.

## SUMIF

`SUMIF(bereik; criterium; [somebereik])`

Telt de waarden op van de cellen die aan een voorwaarde voldoen.

- `bereik`: Het celbereik dat wordt getest.
- `criterium`: Waarnaar moet worden gezocht: een getal, een tekst of een test zoals ">5".
- `somebereik` (optioneel): De cellen die worden opgeteld (de cellen die worden getest als dit wordt weggelaten).

## SUMIFS

`SUMIFS(somebereik; criteriumbereik1; criterium1; ...)`

Telt de waarden op van de cellen die aan meerdere voorwaarden voldoen.

- `somebereik`: De cellen die worden opgeteld (de cellen die worden getest als dit wordt weggelaten).
- `criteriumbereik1`: Het eerste celbereik dat wordt getest.
- `criterium1`: Waarnaar moet worden gezocht in het eerste celbereik: een getal, een tekst of een test zoals ">5".
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## T.DIST

`T.DIST(x; vrijheidsgraden; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de t-curve van Student bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## T.DIST.2T

`T.DIST.2T(x; vrijheidsgraden)`

Geeft het aandeel van de t-curve van Student buiten -x en x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.

## T.DIST.RT

`T.DIST.RT(x; vrijheidsgraden)`

Geeft het aandeel van de t-curve van Student boven x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.

## T.INV

`T.INV(kans; vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de t-curve van Student, vanaf links geteld.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.

## T.INV.2T

`T.INV.2T(kans; vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de t-curve van Student, verdeeld over beide staarten.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.

## T.TEST

`T.TEST(matrix1; matrix2; staarten; type)`

Geeft de kans dat de gemiddelden van twee reeksen getallen gelijk zijn.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.
- `staarten`: 1 voor één staart van de curve, 2 voor beide.
- `type`: Wanneer betalingen worden gedaan: 0 of weggelaten aan het eind van elke periode, 1 aan het begin.

## TDIST

`TDIST(x; vrijheidsgraden; staarten)`

Geeft het aandeel van de t-curve van Student in één staart of in beide.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.
- `staarten`: 1 voor één staart van de curve, 2 voor beide.

## TINV

`TINV(kans; vrijheidsgraden)`

Geeft de x-waarde voor een aandeel van de t-curve van Student, verdeeld over beide staarten.

- `kans`: Een kans, van 0 tot 1.
- `vrijheidsgraden`: De vrijheidsgraden, een geheel getal van 1 of hoger.

## TREND

`TREND(bekende_y's; [bekende_x's]; [nieuwe_x's]; [const])`

Geeft de waarden langs de best passende rechte lijn voor nieuwe x-waarden.

- `bekende_y's`: Het celbereik met de y-waarden die u al hebt.
- `bekende_x's` (optioneel): Het celbereik met de x-waarden die u al hebt.
- `nieuwe_x's` (optioneel): De x-waarden waarvoor de y-waarden moeten worden gegeven (de bekende x-waarden als dit wordt weggelaten).
- `const` (optioneel): Waar of weggelaten om een lijn te laten passen die niet door nul hoeft te gaan; onwaar om de lijn door nul te laten gaan.

## TRIMMEAN

`TRIMMEAN(matrix; percentage)`

Geeft het gemiddelde van een reeks getallen nadat een aandeel van de hoogste en laagste waarden is weggelaten.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `percentage`: Het aandeel van de waarden, van 0 tot maar niet met 1, dat aan beide uiteinden wordt weggelaten.

## TTEST

`TTEST(matrix1; matrix2; staarten; type)`

Geeft de kans dat de gemiddelden van twee reeksen getallen gelijk zijn.

- `matrix1`: Het eerste celbereik of de eerste lijst met waarden.
- `matrix2`: Het tweede celbereik of de tweede lijst met waarden, even groot als de eerste.
- `staarten`: 1 voor één staart van de curve, 2 voor beide.
- `type`: Wanneer betalingen worden gedaan: 0 of weggelaten aan het eind van elke periode, 1 aan het begin.

## VAR

`VAR(getal1; ...)`

Schat de variantie van een hele groep op basis van een steekproef daaruit.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## VAR.P

`VAR.P(getal1; ...)`

Geeft de variantie van een hele groep.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## VAR.S

`VAR.S(getal1; ...)`

Schat de variantie van een hele groep op basis van een steekproef daaruit.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## VARA

`VARA(waarde1; ...)`

Geeft de variantie van een steekproef, waarbij tekst als 0 en waar als 1 wordt geteld.

- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## VARP

`VARP(getal1; ...)`

Geeft de variantie van een hele groep.

- `getal1`: Het eerste getal of celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## VARPA

`VARPA(waarde1; ...)`

Geeft de variantie van een populatie, waarbij tekst als 0 en waar als 1 wordt geteld.

- `waarde1`: De eerste waarde of het eerste celbereik.
- `...`: Zoveel extra als nodig, van dezelfde soort als het argument ervoor.

## WEIBULL

`WEIBULL(x; alfa; bèta; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de Weibullcurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de vorm van de curve bepaalt (de schaal van de curve bij de gammacurve en de Weibullcurve).
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## WEIBULL.DIST

`WEIBULL.DIST(x; alfa; bèta; cumulatief)`

Geeft de waarde of het cumulatieve aandeel van de Weibullcurve bij x.

- `x`: De x-waarde waarmee moet worden gewerkt.
- `alfa`: De kans op een fout die u accepteert, bijvoorbeeld 0,05 voor een betrouwbaarheidsniveau van 95 procent.
- `bèta`: Een getal dat de vorm van de curve bepaalt (de schaal van de curve bij de gammacurve en de Weibullcurve).
- `cumulatief`: Waar om het lopende totaal tot dit punt te geven; onwaar om alleen de waarde op dit punt te geven.

## Z.TEST

`Z.TEST(matrix; x; [sigma])`

Geeft de eenzijdige kans dat het gemiddelde van een steekproef boven een waarde ligt.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `x`: De x-waarde waarmee moet worden gewerkt.
- `sigma` (optioneel): De standaarddeviatie van de hele populatie (die van de steekproef als dit wordt weggelaten).

## ZTEST

`ZTEST(matrix; x; [sigma])`

Geeft de eenzijdige kans dat het gemiddelde van een steekproef boven een waarde ligt.

- `matrix`: Een celbereik, of een lijst met waarden tussen accolades.
- `x`: De x-waarde waarmee moet worden gewerkt.
- `sigma` (optioneel): De standaarddeviatie van de hele populatie (die van de steekproef als dit wordt weggelaten).

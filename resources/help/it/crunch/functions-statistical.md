# Funzioni statistiche
<!-- keywords: funzioni, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST -->

Queste sono le funzioni statistiche che JeffCrunch può calcolare. Ogni funzione mostra come si scrive, che cosa fa e quali sono i suoi argomenti. Un argomento tra parentesi quadre è facoltativo, e "..." indica che puoi aggiungere quanti altri ne servono dell'ultimo tipo. Per provarne una, vedi [Scegli una funzione](insert-function). Mentre digiti una funzione in una cella, un piccolo suggerimento sotto la cella mostra i suoi argomenti; se fai clic sul nome della funzione nel suggerimento, si apre la sua scheda in questa pagina.

## AVEDEV

`AVEDEV(number1, ...)`

Restituisce la media degli scarti assoluti dei numeri dalla loro media.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## AVERAGE

`AVERAGE(number1, ...)`

Restituisce la media dei numeri.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## AVERAGEA

`AVERAGEA(value1, ...)`

Restituisce la media dei valori, considerando il testo come zero e il valore vero come uno.

- `value1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## AVERAGEIF

`AVERAGEIF(range, criteria, [average_range])`

Calcola la media delle celle che soddisfano una condizione.

- `range`: L'intervallo di celle da verificare.
- `criteria`: Che cosa cercare: un numero, un testo o un confronto come ">5".
- `average_range` (facoltativo): Le celle di cui calcolare la media (se omesso, quelle in cui si cerca).

## AVERAGEIFS

`AVERAGEIFS(average_range, criteria_range1, criteria1, ...)`

Calcola la media delle celle che soddisfano più condizioni.

- `average_range`: Le celle di cui calcolare la media (se omesso, quelle in cui si cerca).
- `criteria_range1`: Il primo intervallo di celle da verificare.
- `criteria1`: Che cosa cercare nel primo intervallo di celle: un numero, un testo o un confronto come ">5".
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## BETA.DIST

`BETA.DIST(x, alpha, beta, cumulative, [A], [B])`

Restituisce il valore della curva beta in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `beta`: Un numero che determina la forma della curva (la scala della curva per le curve gamma e Weibull).
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.
- `A` (facoltativo): Il limite inferiore di x (0 se omesso).
- `B` (facoltativo): Il limite superiore di x (1 se omesso).

## BETA.INV

`BETA.INV(probability, alpha, beta, [A], [B])`

Restituisce il valore x corrispondente a una probabilità della curva beta.

- `probability`: Una probabilità, da 0 a 1.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `beta`: Un numero che determina la forma della curva (la scala della curva per le curve gamma e Weibull).
- `A` (facoltativo): Il limite inferiore di x (0 se omesso).
- `B` (facoltativo): Il limite superiore di x (1 se omesso).

## BETADIST

`BETADIST(x, alpha, beta, [A], [B])`

Restituisce la probabilità cumulata della curva beta fino a x.

- `x`: Il valore x da elaborare.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `beta`: Un numero che determina la forma della curva (la scala della curva per le curve gamma e Weibull).
- `A` (facoltativo): Il limite inferiore di x (0 se omesso).
- `B` (facoltativo): Il limite superiore di x (1 se omesso).

## BETAINV

`BETAINV(probability, alpha, beta, [A], [B])`

Restituisce il valore x corrispondente a una probabilità della curva beta.

- `probability`: Una probabilità, da 0 a 1.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `beta`: Un numero che determina la forma della curva (la scala della curva per le curve gamma e Weibull).
- `A` (facoltativo): Il limite inferiore di x (0 se omesso).
- `B` (facoltativo): Il limite superiore di x (1 se omesso).

## BINOM.DIST

`BINOM.DIST(number_s, trials, probability_s, cumulative)`

Restituisce la probabilità di un certo numero di successi in un certo numero di prove.

- `number_s`: Il numero di successi.
- `trials`: Il numero di prove.
- `probability_s`: La probabilità di successo in ogni prova, da 0 a 1.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(trials, probability_s, number_s, [number_s2])`

Restituisce la probabilità che il numero di successi rientri in un intervallo.

- `trials`: Il numero di prove.
- `probability_s`: La probabilità di successo in ogni prova, da 0 a 1.
- `number_s`: Il numero di successi.
- `number_s2` (facoltativo): Il numero massimo di successi da contare (uguale al minimo se omesso).

## BINOM.INV

`BINOM.INV(trials, probability_s, alpha)`

Restituisce il minor numero di successi la cui probabilità cumulata raggiunge un valore.

- `trials`: Il numero di prove.
- `probability_s`: La probabilità di successo in ogni prova, da 0 a 1.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.

## BINOMDIST

`BINOMDIST(number_s, trials, probability_s, cumulative)`

Restituisce la probabilità di un certo numero di successi in un certo numero di prove.

- `number_s`: Il numero di successi.
- `trials`: Il numero di prove.
- `probability_s`: La probabilità di successo in ogni prova, da 0 a 1.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## CHIDIST

`CHIDIST(x, deg_freedom)`

Restituisce la porzione della curva chi quadrato oltre x.

- `x`: Il valore x da elaborare.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.

## CHIINV

`CHIINV(probability, deg_freedom)`

Restituisce il valore x corrispondente a una probabilità della curva chi quadrato, contata da destra.

- `probability`: Una probabilità, da 0 a 1.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.

## CHISQ.DIST

`CHISQ.DIST(x, deg_freedom, cumulative)`

Restituisce il valore della curva chi quadrato in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x, deg_freedom)`

Restituisce la porzione della curva chi quadrato oltre x.

- `x`: Il valore x da elaborare.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.

## CHISQ.INV

`CHISQ.INV(probability, deg_freedom)`

Restituisce il valore x corrispondente a una probabilità della curva chi quadrato, contata da sinistra.

- `probability`: Una probabilità, da 0 a 1.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.

## CHISQ.INV.RT

`CHISQ.INV.RT(probability, deg_freedom)`

Restituisce il valore x corrispondente a una probabilità della curva chi quadrato, contata da destra.

- `probability`: Una probabilità, da 0 a 1.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.

## CHISQ.TEST

`CHISQ.TEST(actual_range, expected_range)`

Restituisce la probabilità che le frequenze osservate differiscano da quelle attese solo per effetto del caso.

- `actual_range`: L'intervallo di celle con le frequenze osservate.
- `expected_range`: L'intervallo di celle con le frequenze attese, delle stesse dimensioni di quello delle frequenze osservate.

## CHITEST

`CHITEST(actual_range, expected_range)`

Restituisce la probabilità che le frequenze osservate differiscano da quelle attese solo per effetto del caso.

- `actual_range`: L'intervallo di celle con le frequenze osservate.
- `expected_range`: L'intervallo di celle con le frequenze attese, delle stesse dimensioni di quello delle frequenze osservate.

## CONFIDENCE

`CONFIDENCE(alpha, standard_dev, size)`

Restituisce il margine di un intervallo di confidenza per una media, usando la curva normale.

- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `standard_dev`: La deviazione standard della distribuzione.
- `size`: La dimensione del campione, un numero intero pari o superiore a 1.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(alpha, standard_dev, size)`

Restituisce il margine di un intervallo di confidenza per una media, usando la curva normale.

- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `standard_dev`: La deviazione standard della distribuzione.
- `size`: La dimensione del campione, un numero intero pari o superiore a 1.

## CONFIDENCE.T

`CONFIDENCE.T(alpha, standard_dev, size)`

Restituisce il margine di un intervallo di confidenza per una media, usando la curva t di Student.

- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `standard_dev`: La deviazione standard della distribuzione.
- `size`: La dimensione del campione, un numero intero pari o superiore a 1.

## CORREL

`CORREL(array1, array2)`

Restituisce quanto due serie di numeri variano in modo concorde, da -1 a 1.

- `array1`: Il primo intervallo di celle o elenco di valori.
- `array2`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.

## COUNT

`COUNT(value1, ...)`

Conta i numeri in un elenco o in un intervallo di celle.

- `value1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## COUNTA

`COUNTA(value1, ...)`

Conta le celle che non sono vuote.

- `value1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## COUNTBLANK

`COUNTBLANK(range)`

Conta le celle vuote in un intervallo.

- `range`: L'intervallo di celle da verificare.

## COUNTIF

`COUNTIF(range, criteria)`

Conta le celle che soddisfano una condizione.

- `range`: L'intervallo di celle da verificare.
- `criteria`: Che cosa cercare: un numero, un testo o un confronto come ">5".

## COUNTIFS

`COUNTIFS(criteria_range1, criteria1, ...)`

Conta le righe in cui sono soddisfatte contemporaneamente più condizioni.

- `criteria_range1`: Il primo intervallo di celle da verificare.
- `criteria1`: Che cosa cercare nel primo intervallo di celle: un numero, un testo o un confronto come ">5".
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## COVAR

`COVAR(array1, array2)`

Restituisce quanto due serie di numeri variano insieme.

- `array1`: Il primo intervallo di celle o elenco di valori.
- `array2`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.

## COVARIANCE.P

`COVARIANCE.P(array1, array2)`

Restituisce quanto due serie di numeri variano insieme, per una popolazione.

- `array1`: Il primo intervallo di celle o elenco di valori.
- `array2`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.

## COVARIANCE.S

`COVARIANCE.S(array1, array2)`

Restituisce quanto due serie di numeri variano insieme, per un campione.

- `array1`: Il primo intervallo di celle o elenco di valori.
- `array2`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.

## CRITBINOM

`CRITBINOM(trials, probability_s, alpha)`

Restituisce il minor numero di successi la cui probabilità cumulata raggiunge un valore.

- `trials`: Il numero di prove.
- `probability_s`: La probabilità di successo in ogni prova, da 0 a 1.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.

## DEVSQ

`DEVSQ(number1, ...)`

Restituisce la somma dei quadrati degli scarti dei numeri dalla loro media.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## EXPON.DIST

`EXPON.DIST(x, lambda, cumulative)`

Restituisce il valore della curva esponenziale in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `lambda`: Il tasso della curva, maggiore di 0.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## EXPONDIST

`EXPONDIST(x, lambda, cumulative)`

Restituisce il valore della curva esponenziale in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `lambda`: Il tasso della curva, maggiore di 0.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## F.DIST

`F.DIST(x, deg_freedom1, deg_freedom2, cumulative)`

Restituisce il valore della curva F in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `deg_freedom1`: I gradi di libertà del numeratore, un numero intero pari o superiore a 1.
- `deg_freedom2`: I gradi di libertà del denominatore, un numero intero pari o superiore a 1.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## F.DIST.RT

`F.DIST.RT(x, deg_freedom1, deg_freedom2)`

Restituisce la porzione della curva F oltre x.

- `x`: Il valore x da elaborare.
- `deg_freedom1`: I gradi di libertà del numeratore, un numero intero pari o superiore a 1.
- `deg_freedom2`: I gradi di libertà del denominatore, un numero intero pari o superiore a 1.

## F.INV

`F.INV(probability, deg_freedom1, deg_freedom2)`

Restituisce il valore x corrispondente a una probabilità della curva F, contata da sinistra.

- `probability`: Una probabilità, da 0 a 1.
- `deg_freedom1`: I gradi di libertà del numeratore, un numero intero pari o superiore a 1.
- `deg_freedom2`: I gradi di libertà del denominatore, un numero intero pari o superiore a 1.

## F.INV.RT

`F.INV.RT(probability, deg_freedom1, deg_freedom2)`

Restituisce il valore x corrispondente a una probabilità della curva F, contata da destra.

- `probability`: Una probabilità, da 0 a 1.
- `deg_freedom1`: I gradi di libertà del numeratore, un numero intero pari o superiore a 1.
- `deg_freedom2`: I gradi di libertà del denominatore, un numero intero pari o superiore a 1.

## F.TEST

`F.TEST(array1, array2)`

Restituisce la probabilità che due serie di numeri abbiano la stessa varianza.

- `array1`: Il primo intervallo di celle o elenco di valori.
- `array2`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.

## FDIST

`FDIST(x, deg_freedom1, deg_freedom2)`

Restituisce la porzione della curva F oltre x.

- `x`: Il valore x da elaborare.
- `deg_freedom1`: I gradi di libertà del numeratore, un numero intero pari o superiore a 1.
- `deg_freedom2`: I gradi di libertà del denominatore, un numero intero pari o superiore a 1.

## FINV

`FINV(probability, deg_freedom1, deg_freedom2)`

Restituisce il valore x corrispondente a una probabilità della curva F, contata da destra.

- `probability`: Una probabilità, da 0 a 1.
- `deg_freedom1`: I gradi di libertà del numeratore, un numero intero pari o superiore a 1.
- `deg_freedom2`: I gradi di libertà del denominatore, un numero intero pari o superiore a 1.

## FISHER

`FISHER(x)`

Restituisce la trasformata di Fisher di un numero compreso tra -1 e 1.

- `x`: Il valore x da elaborare.

## FISHERINV

`FISHERINV(y)`

Restituisce l'inversa della trasformata di Fisher.

- `y`: Il numero di cui calcolare l'inversa della trasformata di Fisher.

## FORECAST

`FORECAST(x, known_y's, known_x's)`

Prevede un valore lungo la retta che meglio approssima i dati.

- `x`: Il valore x per cui prevedere un valore y.
- `known_y's`: L'intervallo di celle che contiene i valori y già noti.
- `known_x's`: L'intervallo di celle che contiene i valori x già noti.

## FORECAST.LINEAR

`FORECAST.LINEAR(x, known_y's, known_x's)`

Prevede un valore lungo la retta che meglio approssima i dati.

- `x`: Il valore x da elaborare.
- `known_y's`: L'intervallo di celle che contiene i valori y già noti.
- `known_x's`: L'intervallo di celle che contiene i valori x già noti.

## FREQUENCY

`FREQUENCY(data_array, bins_array)`

Conta quanti valori rientrano in ciascun intervallo e restituisce i conteggi come colonna.

- `data_array`: L'intervallo di celle con i valori da contare.
- `bins_array`: L'intervallo di celle con i limiti degli intervalli in cui contare.

## FTEST

`FTEST(array1, array2)`

Restituisce la probabilità che due serie di numeri abbiano la stessa varianza.

- `array1`: Il primo intervallo di celle o elenco di valori.
- `array2`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.

## GAMMA

`GAMMA(x)`

Restituisce la funzione gamma di un numero.

- `x`: Il valore x da elaborare.

## GAMMA.DIST

`GAMMA.DIST(x, alpha, beta, cumulative)`

Restituisce il valore della curva gamma in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `beta`: Un numero che determina la forma della curva (la scala della curva per le curve gamma e Weibull).
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## GAMMA.INV

`GAMMA.INV(probability, alpha, beta)`

Restituisce il valore x corrispondente a una probabilità della curva gamma.

- `probability`: Una probabilità, da 0 a 1.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `beta`: Un numero che determina la forma della curva (la scala della curva per le curve gamma e Weibull).

## GAMMADIST

`GAMMADIST(x, alpha, beta, cumulative)`

Restituisce il valore della curva gamma in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `beta`: Un numero che determina la forma della curva (la scala della curva per le curve gamma e Weibull).
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## GAMMAINV

`GAMMAINV(probability, alpha, beta)`

Restituisce il valore x corrispondente a una probabilità della curva gamma.

- `probability`: Una probabilità, da 0 a 1.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `beta`: Un numero che determina la forma della curva (la scala della curva per le curve gamma e Weibull).

## GAMMALN

`GAMMALN(x)`

Restituisce il logaritmo naturale della funzione gamma.

- `x`: Il valore x da elaborare.

## GAMMALN.PRECISE

`GAMMALN.PRECISE(x)`

Restituisce il logaritmo naturale della funzione gamma.

- `x`: Il valore x da elaborare.

## GAUSS

`GAUSS(z)`

Restituisce la porzione della curva normale standard compresa tra la media e z.

- `z`: Il valore z.

## GEOMEAN

`GEOMEAN(number1, ...)`

Restituisce la media geometrica di numeri positivi.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## GROWTH

`GROWTH(known_y's, [known_x's], [new_x's], [const])`

Restituisce i valori lungo la curva esponenziale che meglio approssima i dati, per nuovi valori x.

- `known_y's`: L'intervallo di celle che contiene i valori y già noti.
- `known_x's` (facoltativo): L'intervallo di celle che contiene i valori x già noti.
- `new_x's` (facoltativo): I valori x per cui calcolare i valori y (i valori x noti se omesso).
- `const` (facoltativo): Vero o omesso per adattare una curva che non deve passare necessariamente per zero; falso per farla passare per zero.

## HARMEAN

`HARMEAN(number1, ...)`

Restituisce la media armonica di numeri positivi.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## HYPGEOM.DIST

`HYPGEOM.DIST(sample_s, number_sample, population_s, number_pop, cumulative)`

Restituisce la probabilità di estrarre un certo numero di successi senza rimettere gli elementi nel gruppo.

- `sample_s`: Il numero di successi nel campione.
- `number_sample`: Il numero di elementi estratti.
- `population_s`: Il numero di successi nell'intera popolazione.
- `number_pop`: Il numero di elementi dell'intera popolazione.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## HYPGEOMDIST

`HYPGEOMDIST(sample_s, number_sample, population_s, number_pop)`

Restituisce la probabilità di estrarre un certo numero di successi senza rimettere gli elementi nel gruppo.

- `sample_s`: Il numero di successi nel campione.
- `number_sample`: Il numero di elementi estratti.
- `population_s`: Il numero di successi nell'intera popolazione.
- `number_pop`: Il numero di elementi dell'intera popolazione.

## INTERCEPT

`INTERCEPT(known_y's, known_x's)`

Restituisce il punto in cui la retta che meglio approssima i dati interseca l'asse verticale.

- `known_y's`: L'intervallo di celle che contiene i valori y già noti.
- `known_x's`: L'intervallo di celle che contiene i valori x già noti.

## KURT

`KURT(number1, ...)`

Restituisce quanto un insieme di numeri è appuntito o piatto rispetto a una curva normale.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## LARGE

`LARGE(array, k)`

Restituisce il k-esimo numero più grande di un insieme.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `k`: Il posto da restituire, contando dal più grande: 1 è il più grande.

## LINEST

`LINEST(known_y's, [known_x's], [const], [stats])`

Adatta una retta o un piano ai dati con il metodo dei minimi quadrati e restituisce i suoi valori come riga.

- `known_y's`: L'intervallo di celle che contiene i valori y già noti.
- `known_x's` (facoltativo): L'intervallo di celle che contiene i valori x già noti.
- `const` (facoltativo): Vero o omesso per adattare una retta che non deve passare necessariamente per zero; falso per farla passare per zero.
- `stats` (facoltativo): Vero per ottenere anche altri valori sull'adattamento; falso o omesso per ottenere solo la retta.

## LOGEST

`LOGEST(known_y's, [known_x's], [const], [stats])`

Adatta una curva esponenziale ai dati e restituisce i suoi valori come riga.

- `known_y's`: L'intervallo di celle che contiene i valori y già noti.
- `known_x's` (facoltativo): L'intervallo di celle che contiene i valori x già noti.
- `const` (facoltativo): Vero o omesso per adattare una curva che non deve passare necessariamente per zero; falso per farla passare per zero.
- `stats` (facoltativo): Vero per ottenere anche altri valori sull'adattamento; falso o omesso per ottenere solo la curva.

## LOGINV

`LOGINV(probability, mean, standard_dev)`

Restituisce il valore x corrispondente a una probabilità della curva log-normale.

- `probability`: Una probabilità, da 0 a 1.
- `mean`: La media della distribuzione.
- `standard_dev`: La deviazione standard della distribuzione.

## LOGNORM.DIST

`LOGNORM.DIST(x, mean, standard_dev, cumulative)`

Restituisce il valore della curva log-normale in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `mean`: La media della distribuzione.
- `standard_dev`: La deviazione standard della distribuzione.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## LOGNORM.INV

`LOGNORM.INV(probability, mean, standard_dev)`

Restituisce il valore x corrispondente a una probabilità della curva log-normale.

- `probability`: Una probabilità, da 0 a 1.
- `mean`: La media della distribuzione.
- `standard_dev`: La deviazione standard della distribuzione.

## LOGNORMDIST

`LOGNORMDIST(x, mean, standard_dev)`

Restituisce la probabilità cumulata della curva log-normale fino a x.

- `x`: Il valore x da elaborare.
- `mean`: La media della distribuzione.
- `standard_dev`: La deviazione standard della distribuzione.

## MAXA

`MAXA(value1, ...)`

Restituisce il valore più grande, considerando il testo come zero e il valore vero come uno.

- `value1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## MAXIFS

`MAXIFS(max_range, criteria_range1, criteria1, ...)`

Restituisce il numero più grande tra le celle che soddisfano più condizioni.

- `max_range`: Le celle da cui prendere il valore più grande.
- `criteria_range1`: Il primo intervallo di celle da verificare.
- `criteria1`: Che cosa cercare nel primo intervallo di celle: un numero, un testo o un confronto come ">5".
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## MEDIAN

`MEDIAN(number1, ...)`

Restituisce il valore centrale di un insieme di numeri.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## MINA

`MINA(value1, ...)`

Restituisce il valore più piccolo, considerando il testo come zero e il valore vero come uno.

- `value1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## MINIFS

`MINIFS(min_range, criteria_range1, criteria1, ...)`

Restituisce il numero più piccolo tra le celle che soddisfano più condizioni.

- `min_range`: Le celle da cui prendere il valore più piccolo.
- `criteria_range1`: Il primo intervallo di celle da verificare.
- `criteria1`: Che cosa cercare nel primo intervallo di celle: un numero, un testo o un confronto come ">5".
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## MODE

`MODE(number1, ...)`

Restituisce il numero che compare più spesso.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## MODE.MULT

`MODE.MULT(number1, ...)`

Restituisce una colonna con i numeri che compaiono più spesso in un insieme.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## MODE.SNGL

`MODE.SNGL(number1, ...)`

Restituisce il numero che compare più spesso in un insieme.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## NEGBINOM.DIST

`NEGBINOM.DIST(number_f, number_s, probability_s, cumulative)`

Restituisce la probabilità di un certo numero di insuccessi prima di un certo numero di successi.

- `number_f`: Il numero di insuccessi.
- `number_s`: Il numero di successi.
- `probability_s`: La probabilità di successo in ogni prova, da 0 a 1.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## NEGBINOMDIST

`NEGBINOMDIST(number_f, number_s, probability_s)`

Restituisce la probabilità di un certo numero di insuccessi prima di un certo numero di successi.

- `number_f`: Il numero di insuccessi.
- `number_s`: Il numero di successi.
- `probability_s`: La probabilità di successo in ogni prova, da 0 a 1.

## NORM.DIST

`NORM.DIST(x, mean, standard_dev, cumulative)`

Restituisce il valore della curva normale in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `mean`: La media della distribuzione.
- `standard_dev`: La deviazione standard della distribuzione.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## NORM.INV

`NORM.INV(probability, mean, standard_dev)`

Restituisce il valore x corrispondente a una probabilità della curva normale.

- `probability`: Una probabilità, da 0 a 1.
- `mean`: La media della distribuzione.
- `standard_dev`: La deviazione standard della distribuzione.

## NORM.S.DIST

`NORM.S.DIST(z, cumulative)`

Restituisce il valore della curva normale standard in z o la sua probabilità cumulata.

- `z`: Il valore z.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## NORM.S.INV

`NORM.S.INV(probability)`

Restituisce il valore z corrispondente a una probabilità della curva normale standard.

- `probability`: Una probabilità, da 0 a 1.

## NORMDIST

`NORMDIST(x, mean, standard_dev, cumulative)`

Restituisce il valore della curva normale in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `mean`: La media della distribuzione.
- `standard_dev`: La deviazione standard della distribuzione.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## NORMINV

`NORMINV(probability, mean, standard_dev)`

Restituisce il valore x corrispondente a una probabilità della curva normale.

- `probability`: Una probabilità, da 0 a 1.
- `mean`: La media della distribuzione.
- `standard_dev`: La deviazione standard della distribuzione.

## NORMSDIST

`NORMSDIST(z)`

Restituisce la probabilità cumulata della curva normale standard fino a z.

- `z`: Il valore z.

## NORMSINV

`NORMSINV(probability)`

Restituisce il valore z corrispondente a una probabilità della curva normale standard.

- `probability`: Una probabilità, da 0 a 1.

## PEARSON

`PEARSON(array1, array2)`

Restituisce quanto due serie di numeri variano in modo concorde, da -1 a 1.

- `array1`: Il primo intervallo di celle o elenco di valori.
- `array2`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.

## PERCENTILE

`PERCENTILE(array, k)`

Restituisce il valore al di sotto del quale cade una quota k dei numeri.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `k`: La quota dei valori, da 0 a 1, per cui restituire il valore.

## PERCENTILE.EXC

`PERCENTILE.EXC(array, k)`

Restituisce il valore al di sotto del quale cade una quota k dei numeri, escludendo 0 e 1.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `k`: Il posto nell'ordine, contando da 1.

## PERCENTILE.INC

`PERCENTILE.INC(array, k)`

Restituisce il valore al di sotto del quale cade una quota k dei numeri, da 0 a 1.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `k`: Il posto nell'ordine, contando da 1.

## PERCENTRANK

`PERCENTRANK(array, x, [significance])`

Restituisce la posizione di un valore in un insieme di numeri, come quota da 0 a 1.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `x`: Il valore x da elaborare.
- `significance` (facoltativo): Il numero ai cui multipli arrotondare.

## PERCENTRANK.EXC

`PERCENTRANK.EXC(array, x, [significance])`

Restituisce la posizione di un valore in un insieme di numeri, come quota, escludendo 0 e 1.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `x`: Il valore x da elaborare.
- `significance` (facoltativo): Il numero ai cui multipli arrotondare.

## PERCENTRANK.INC

`PERCENTRANK.INC(array, x, [significance])`

Restituisce la posizione di un valore in un insieme di numeri, come quota da 0 a 1.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `x`: Il valore x da elaborare.
- `significance` (facoltativo): Il numero ai cui multipli arrotondare.

## PERMUTATIONA

`PERMUTATIONA(number, number_chosen)`

Restituisce il numero di combinazioni ordinate quando un elemento può essere scelto più di una volta.

- `number`: Il numero da elaborare.
- `number_chosen`: Quanti elementi scegliere in ogni gruppo.

## PHI

`PHI(x)`

Restituisce l'altezza della curva normale standard in x.

- `x`: Il valore x da elaborare.

## POISSON

`POISSON(x, mean, cumulative)`

Restituisce la probabilità di un certo numero di eventi, a partire da quanti se ne prevedono.

- `x`: Il valore x da elaborare.
- `mean`: La media della distribuzione.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## POISSON.DIST

`POISSON.DIST(x, mean, cumulative)`

Restituisce la probabilità di un certo numero di eventi, a partire da quanti se ne prevedono.

- `x`: Il valore x da elaborare.
- `mean`: La media della distribuzione.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## PROB

`PROB(x_range, prob_range, lower_limit, [upper_limit])`

Restituisce la probabilità che i valori cadano tra due limiti.

- `x_range`: L'intervallo di celle con i valori x.
- `prob_range`: Le probabilità corrispondenti a ciascun valore x, ciascuna da 0 a 1, la cui somma è 1.
- `lower_limit`: Il limite inferiore dell'intervallo.
- `upper_limit` (facoltativo): Il limite superiore dell'intervallo (solo il limite inferiore se omesso).

## QUARTILE

`QUARTILE(array, quart)`

Restituisce i valori dei quartili di un insieme di numeri.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `quart`: Quale quartile: 0 il più piccolo, 1 primo quartile, 2 mediana, 3 terzo quartile, 4 il più grande.

## QUARTILE.EXC

`QUARTILE.EXC(array, quart)`

Restituisce i valori dei quartili di un insieme di numeri, escludendo gli estremi.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `quart`: Quale quartile: 0 il più piccolo, 1 primo quartile, 2 mediana, 3 terzo quartile, 4 il più grande.

## QUARTILE.INC

`QUARTILE.INC(array, quart)`

Restituisce i valori dei quartili di un insieme di numeri, dal più basso al più alto.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `quart`: Quale quartile: 0 il più piccolo, 1 primo quartile, 2 mediana, 3 terzo quartile, 4 il più grande.

## RANK

`RANK(number, ref, [order])`

Restituisce la posizione di un numero in un elenco ordinato.

- `number`: Il numero da elaborare.
- `ref`: Una cella o un intervallo di celle.
- `order` (facoltativo): 0 o omesso mette il più grande per primo; qualsiasi altro numero mette per primo il più piccolo.

## RANK.AVG

`RANK.AVG(number, ref, [order])`

Restituisce la posizione di un numero in un elenco e, in caso di pari merito, la posizione media.

- `number`: Il numero da elaborare.
- `ref`: Una cella o un intervallo di celle.
- `order` (facoltativo): 0 o omesso mette il più grande per primo; qualsiasi altro numero mette per primo il più piccolo.

## RANK.EQ

`RANK.EQ(number, ref, [order])`

Restituisce la posizione di un numero in un elenco e, in caso di pari merito, la posizione più alta.

- `number`: Il numero da elaborare.
- `ref`: Una cella o un intervallo di celle.
- `order` (facoltativo): 0 o omesso mette il più grande per primo; qualsiasi altro numero mette per primo il più piccolo.

## RSQ

`RSQ(known_y's, known_x's)`

Restituisce quanto bene una retta si adatta a due serie di numeri.

- `known_y's`: L'intervallo di celle che contiene i valori y già noti.
- `known_x's`: L'intervallo di celle che contiene i valori x già noti.

## SKEW

`SKEW(number1, ...)`

Restituisce quanto è asimmetrico un insieme di numeri rispetto alla media, per un campione.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## SKEW.P

`SKEW.P(number1, ...)`

Restituisce quanto è asimmetrico un insieme di numeri rispetto alla media, per un'intera popolazione.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## SLOPE

`SLOPE(known_y's, known_x's)`

Restituisce la pendenza della retta che meglio approssima due serie di numeri.

- `known_y's`: L'intervallo di celle che contiene i valori y già noti.
- `known_x's`: L'intervallo di celle che contiene i valori x già noti.

## SMALL

`SMALL(array, k)`

Restituisce il k-esimo numero più piccolo di un insieme.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `k`: Il posto da restituire, contando dal più piccolo: 1 è il più piccolo.

## STANDARDIZE

`STANDARDIZE(x, mean, standard_dev)`

Restituisce di quante deviazioni standard un valore si discosta dalla media.

- `x`: Il valore x da elaborare.
- `mean`: La media della distribuzione.
- `standard_dev`: La deviazione standard della distribuzione.

## STDEV

`STDEV(number1, ...)`

Stima la dispersione di un intero gruppo a partire da un campione di esso.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## STDEV.P

`STDEV.P(number1, ...)`

Restituisce la dispersione di un intero gruppo.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## STDEV.S

`STDEV.S(number1, ...)`

Stima la dispersione di un intero gruppo a partire da un campione di esso.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## STDEVA

`STDEVA(value1, ...)`

Restituisce la deviazione standard di un campione, considerando il testo come 0 e il valore vero come 1.

- `value1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## STDEVP

`STDEVP(number1, ...)`

Restituisce la dispersione di un intero gruppo.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## STDEVPA

`STDEVPA(value1, ...)`

Restituisce la deviazione standard di una popolazione, considerando il testo come 0 e il valore vero come 1.

- `value1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## STEYX

`STEYX(known_y's, known_x's)`

Restituisce l'errore standard dei valori y previsti dalla retta adattata.

- `known_y's`: L'intervallo di celle che contiene i valori y già noti.
- `known_x's`: L'intervallo di celle che contiene i valori x già noti.

## SUMIF

`SUMIF(range, criteria, [sum_range])`

Somma le celle che soddisfano una condizione.

- `range`: L'intervallo di celle da verificare.
- `criteria`: Che cosa cercare: un numero, un testo o un confronto come ">5".
- `sum_range` (facoltativo): Le celle da sommare (quelle da verificare, se omesso).

## SUMIFS

`SUMIFS(sum_range, criteria_range1, criteria1, ...)`

Somma le celle che soddisfano più condizioni.

- `sum_range`: Le celle da sommare (quelle da verificare, se omesso).
- `criteria_range1`: Il primo intervallo di celle da verificare.
- `criteria1`: Che cosa cercare nel primo intervallo di celle: un numero, un testo o un confronto come ">5".
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## T.DIST

`T.DIST(x, deg_freedom, cumulative)`

Restituisce il valore della curva t di Student in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## T.DIST.2T

`T.DIST.2T(x, deg_freedom)`

Restituisce la porzione della curva t di Student esterna all'intervallo da -x a x.

- `x`: Il valore x da elaborare.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.

## T.DIST.RT

`T.DIST.RT(x, deg_freedom)`

Restituisce la porzione della curva t di Student oltre x.

- `x`: Il valore x da elaborare.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.

## T.INV

`T.INV(probability, deg_freedom)`

Restituisce il valore x corrispondente a una probabilità della curva t di Student, contata da sinistra.

- `probability`: Una probabilità, da 0 a 1.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.

## T.INV.2T

`T.INV.2T(probability, deg_freedom)`

Restituisce il valore x corrispondente a una probabilità della curva t di Student, ripartita tra entrambe le code.

- `probability`: Una probabilità, da 0 a 1.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.

## T.TEST

`T.TEST(array1, array2, tails, type)`

Restituisce la probabilità che le medie di due serie di numeri siano uguali.

- `array1`: Il primo intervallo di celle o elenco di valori.
- `array2`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.
- `tails`: 1 per una coda della curva, 2 per entrambe.
- `type`: Quando vengono effettuati i pagamenti: 0 o omesso alla fine di ogni periodo, 1 all'inizio.

## TDIST

`TDIST(x, deg_freedom, tails)`

Restituisce la porzione della curva t di Student in una coda o in entrambe.

- `x`: Il valore x da elaborare.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.
- `tails`: 1 per una coda della curva, 2 per entrambe.

## TINV

`TINV(probability, deg_freedom)`

Restituisce il valore x corrispondente a una probabilità della curva t di Student, ripartita tra entrambe le code.

- `probability`: Una probabilità, da 0 a 1.
- `deg_freedom`: I gradi di libertà, un numero intero pari o superiore a 1.

## TREND

`TREND(known_y's, [known_x's], [new_x's], [const])`

Restituisce i valori lungo la retta che meglio approssima i dati, per nuovi valori x.

- `known_y's`: L'intervallo di celle che contiene i valori y già noti.
- `known_x's` (facoltativo): L'intervallo di celle che contiene i valori x già noti.
- `new_x's` (facoltativo): I valori x per cui calcolare i valori y (i valori x noti se omesso).
- `const` (facoltativo): Vero o omesso per adattare una retta che non deve passare necessariamente per zero; falso per farla passare per zero.

## TRIMMEAN

`TRIMMEAN(array, percent)`

Restituisce la media di un insieme di numeri dopo aver eliminato una quota dei valori più alti e più bassi.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `percent`: La quota dei valori, da 0 a meno di 1, da escludere alle due estremità.

## TTEST

`TTEST(array1, array2, tails, type)`

Restituisce la probabilità che le medie di due serie di numeri siano uguali.

- `array1`: Il primo intervallo di celle o elenco di valori.
- `array2`: Il secondo intervallo di celle o elenco di valori, delle stesse dimensioni del primo.
- `tails`: 1 per una coda della curva, 2 per entrambe.
- `type`: Quando vengono effettuati i pagamenti: 0 o omesso alla fine di ogni periodo, 1 all'inizio.

## VAR

`VAR(number1, ...)`

Stima la varianza di un intero gruppo a partire da un campione di esso.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## VAR.P

`VAR.P(number1, ...)`

Restituisce la varianza di un intero gruppo.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## VAR.S

`VAR.S(number1, ...)`

Stima la varianza di un intero gruppo a partire da un campione di esso.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## VARA

`VARA(value1, ...)`

Restituisce la varianza di un campione, considerando il testo come 0 e il valore vero come 1.

- `value1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## VARP

`VARP(number1, ...)`

Restituisce la varianza di un intero gruppo.

- `number1`: Il primo numero o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## VARPA

`VARPA(value1, ...)`

Restituisce la varianza di una popolazione, considerando il testo come 0 e il valore vero come 1.

- `value1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## WEIBULL

`WEIBULL(x, alpha, beta, cumulative)`

Restituisce il valore della curva di Weibull in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `beta`: Un numero che determina la forma della curva (la scala della curva per le curve gamma e Weibull).
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## WEIBULL.DIST

`WEIBULL.DIST(x, alpha, beta, cumulative)`

Restituisce il valore della curva di Weibull in x o la sua probabilità cumulata.

- `x`: Il valore x da elaborare.
- `alpha`: La probabilità di errore che accetti, ad esempio 0,05 per un livello di confidenza del 95 per cento.
- `beta`: Un numero che determina la forma della curva (la scala della curva per le curve gamma e Weibull).
- `cumulative`: Vero per ottenere la probabilità cumulata fino a questo punto; falso per ottenere il valore solo in questo punto.

## Z.TEST

`Z.TEST(array, x, [sigma])`

Restituisce la probabilità a una coda che la media di un campione sia superiore a un valore.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `x`: Il valore x da elaborare.
- `sigma` (facoltativo): La deviazione standard dell'intera popolazione (quella del campione se omesso).

## ZTEST

`ZTEST(array, x, [sigma])`

Restituisce la probabilità a una coda che la media di un campione sia superiore a un valore.

- `array`: Un intervallo di celle, oppure un elenco di valori scritto tra parentesi graffe.
- `x`: Il valore x da elaborare.
- `sigma` (facoltativo): La deviazione standard dell'intera popolazione (quella del campione se omesso).

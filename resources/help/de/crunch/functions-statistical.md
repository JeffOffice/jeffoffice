# Statistische Funktionen
<!-- keywords: Funktionen, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST -->

Dies sind die statistischen Funktionen, die JeffCrunch berechnen kann. Jede zeigt, wie sie geschrieben wird, was sie tut und was jedes ihrer Argumente bedeutet. Ein Argument in eckigen Klammern ist optional, und „...“ bedeutet, dass Sie beliebig viele weitere des letzten Arguments angeben können. Zum Ausprobieren siehe [Funktion auswählen](insert-function). Während Sie in einer Zelle eine Funktion eingeben, zeigt ein kleiner Tipp unter der Zelle ihre Argumente an. Ein Klick auf den Funktionsnamen im Tipp öffnet ihren Eintrag hier.

## AVEDEV

`AVEDEV(number1, ...)`

Gibt den Mittelwert der Abstände der Zahlen von ihrem Mittelwert zurück.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## AVERAGE

`AVERAGE(number1, ...)`

Gibt den Mittelwert von Zahlen zurück.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## AVERAGEA

`AVERAGEA(value1, ...)`

Gibt den Mittelwert von Werten zurück, wobei Text als null und WAHR als eins gezählt wird.

- `value1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## AVERAGEIF

`AVERAGEIF(range, criteria, [average_range])`

Bildet den Mittelwert der Zellen, die eine Bedingung erfüllen.

- `range`: Der zu prüfende Zellbereich.
- `criteria`: Wonach gesucht wird: eine Zahl, ein Text oder eine Prüfung wie ">5".
- `average_range` (optional): Die Zellen, deren Mittelwert gebildet wird (wenn weggelassen, die Zellen, in denen gesucht wird).

## AVERAGEIFS

`AVERAGEIFS(average_range, criteria_range1, criteria1, ...)`

Bildet den Mittelwert der Zellen, die mehrere Bedingungen erfüllen.

- `average_range`: Die Zellen, deren Mittelwert gebildet wird (wenn weggelassen, die Zellen, in denen gesucht wird).
- `criteria_range1`: Der erste zu prüfende Zellbereich.
- `criteria1`: Wonach im ersten Zellbereich gesucht wird: eine Zahl, ein Text oder eine Prüfung wie ">5".
- `...`: Beliebig viele weitere derselben Art wie die davor.

## BETA.DIST

`BETA.DIST(x, alpha, beta, cumulative, [A], [B])`

Gibt den Wert oder den kumulierten Anteil der Beta-Kurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `beta`: Eine Zahl, die die Form der Kurve bestimmt (bei der Gamma- und der Weibull-Kurve der Maßstab der Kurve).
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.
- `A` (optional): Die untere Grenze von x (wenn weggelassen, 0).
- `B` (optional): Die obere Grenze von x (wenn weggelassen, 1).

## BETA.INV

`BETA.INV(probability, alpha, beta, [A], [B])`

Gibt den x-Wert für einen Anteil der Beta-Kurve zurück.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `beta`: Eine Zahl, die die Form der Kurve bestimmt (bei der Gamma- und der Weibull-Kurve der Maßstab der Kurve).
- `A` (optional): Die untere Grenze von x (wenn weggelassen, 0).
- `B` (optional): Die obere Grenze von x (wenn weggelassen, 1).

## BETADIST

`BETADIST(x, alpha, beta, [A], [B])`

Gibt den kumulierten Anteil der Beta-Kurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `beta`: Eine Zahl, die die Form der Kurve bestimmt (bei der Gamma- und der Weibull-Kurve der Maßstab der Kurve).
- `A` (optional): Die untere Grenze von x (wenn weggelassen, 0).
- `B` (optional): Die obere Grenze von x (wenn weggelassen, 1).

## BETAINV

`BETAINV(probability, alpha, beta, [A], [B])`

Gibt den x-Wert für einen Anteil der Beta-Kurve zurück.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `beta`: Eine Zahl, die die Form der Kurve bestimmt (bei der Gamma- und der Weibull-Kurve der Maßstab der Kurve).
- `A` (optional): Die untere Grenze von x (wenn weggelassen, 0).
- `B` (optional): Die obere Grenze von x (wenn weggelassen, 1).

## BINOM.DIST

`BINOM.DIST(number_s, trials, probability_s, cumulative)`

Gibt die Wahrscheinlichkeit einer Anzahl von Erfolgen bei einer Anzahl von Versuchen zurück.

- `number_s`: Die Anzahl der Erfolge.
- `trials`: Die Anzahl der Versuche.
- `probability_s`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(trials, probability_s, number_s, [number_s2])`

Gibt die Wahrscheinlichkeit zurück, dass die Erfolge in einen Bereich fallen.

- `trials`: Die Anzahl der Versuche.
- `probability_s`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `number_s`: Die Anzahl der Erfolge.
- `number_s2` (optional): Die höchste zu zählende Anzahl an Erfolgen (wenn weggelassen, dieselbe wie die niedrigste).

## BINOM.INV

`BINOM.INV(trials, probability_s, alpha)`

Gibt die kleinste Anzahl von Erfolgen zurück, deren kumulierte Wahrscheinlichkeit einen Wert erreicht.

- `trials`: Die Anzahl der Versuche.
- `probability_s`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.

## BINOMDIST

`BINOMDIST(number_s, trials, probability_s, cumulative)`

Gibt die Wahrscheinlichkeit einer Anzahl von Erfolgen bei einer Anzahl von Versuchen zurück.

- `number_s`: Die Anzahl der Erfolge.
- `trials`: Die Anzahl der Versuche.
- `probability_s`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## CHIDIST

`CHIDIST(x, deg_freedom)`

Gibt den Anteil der Chi-Quadrat-Kurve oberhalb von x zurück.

- `x`: Der zu verwendende x-Wert.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## CHIINV

`CHIINV(probability, deg_freedom)`

Gibt den x-Wert für einen Anteil der Chi-Quadrat-Kurve zurück, von rechts gezählt.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## CHISQ.DIST

`CHISQ.DIST(x, deg_freedom, cumulative)`

Gibt den Wert oder den kumulierten Anteil der Chi-Quadrat-Kurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x, deg_freedom)`

Gibt den Anteil der Chi-Quadrat-Kurve oberhalb von x zurück.

- `x`: Der zu verwendende x-Wert.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## CHISQ.INV

`CHISQ.INV(probability, deg_freedom)`

Gibt den x-Wert für einen Anteil der Chi-Quadrat-Kurve zurück, von links gezählt.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## CHISQ.INV.RT

`CHISQ.INV.RT(probability, deg_freedom)`

Gibt den x-Wert für einen Anteil der Chi-Quadrat-Kurve zurück, von rechts gezählt.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## CHISQ.TEST

`CHISQ.TEST(actual_range, expected_range)`

Gibt die Wahrscheinlichkeit zurück, dass die Anzahlen nur zufällig von den erwarteten Anzahlen abweichen.

- `actual_range`: Der Zellbereich mit den beobachteten Anzahlen.
- `expected_range`: Der Zellbereich mit den erwarteten Anzahlen, genauso groß wie der Bereich mit den beobachteten Anzahlen.

## CHITEST

`CHITEST(actual_range, expected_range)`

Gibt die Wahrscheinlichkeit zurück, dass die Anzahlen nur zufällig von den erwarteten Anzahlen abweichen.

- `actual_range`: Der Zellbereich mit den beobachteten Anzahlen.
- `expected_range`: Der Zellbereich mit den erwarteten Anzahlen, genauso groß wie der Bereich mit den beobachteten Anzahlen.

## CONFIDENCE

`CONFIDENCE(alpha, standard_dev, size)`

Gibt die Spanne eines Konfidenzintervalls für einen Mittelwert zurück, mit der Normalverteilungskurve.

- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `standard_dev`: Die Standardabweichung der Verteilung.
- `size`: Der Umfang der Stichprobe, eine ganze Zahl von 1 oder mehr.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(alpha, standard_dev, size)`

Gibt die Spanne eines Konfidenzintervalls für einen Mittelwert zurück, mit der Normalverteilungskurve.

- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `standard_dev`: Die Standardabweichung der Verteilung.
- `size`: Der Umfang der Stichprobe, eine ganze Zahl von 1 oder mehr.

## CONFIDENCE.T

`CONFIDENCE.T(alpha, standard_dev, size)`

Gibt die Spanne eines Konfidenzintervalls für einen Mittelwert zurück, mit der t-Verteilung nach Student.

- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `standard_dev`: Die Standardabweichung der Verteilung.
- `size`: Der Umfang der Stichprobe, eine ganze Zahl von 1 oder mehr.

## CORREL

`CORREL(array1, array2)`

Gibt zurück, wie stark sich zwei Zahlenreihen gemeinsam bewegen, von -1 bis 1.

- `array1`: Der erste Zellbereich oder die erste Werteliste.
- `array2`: Der zweite Zellbereich oder die zweite Werteliste, genauso groß wie die erste.

## COUNT

`COUNT(value1, ...)`

Zählt die Zahlen in einer Liste oder einem Zellbereich.

- `value1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## COUNTA

`COUNTA(value1, ...)`

Zählt die Zellen, die nicht leer sind.

- `value1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## COUNTBLANK

`COUNTBLANK(range)`

Zählt die leeren Zellen in einem Bereich.

- `range`: Der zu prüfende Zellbereich.

## COUNTIF

`COUNTIF(range, criteria)`

Zählt die Zellen, die eine Bedingung erfüllen.

- `range`: Der zu prüfende Zellbereich.
- `criteria`: Wonach gesucht wird: eine Zahl, ein Text oder eine Prüfung wie ">5".

## COUNTIFS

`COUNTIFS(criteria_range1, criteria1, ...)`

Zählt die Zeilen, in denen mehrere Bedingungen gleichzeitig erfüllt sind.

- `criteria_range1`: Der erste zu prüfende Zellbereich.
- `criteria1`: Wonach im ersten Zellbereich gesucht wird: eine Zahl, ein Text oder eine Prüfung wie ">5".
- `...`: Beliebig viele weitere derselben Art wie die davor.

## COVAR

`COVAR(array1, array2)`

Gibt zurück, wie stark sich zwei Zahlenreihen gemeinsam verändern.

- `array1`: Der erste Zellbereich oder die erste Werteliste.
- `array2`: Der zweite Zellbereich oder die zweite Werteliste, genauso groß wie die erste.

## COVARIANCE.P

`COVARIANCE.P(array1, array2)`

Gibt zurück, wie stark sich zwei Zahlenreihen für eine Grundgesamtheit gemeinsam verändern.

- `array1`: Der erste Zellbereich oder die erste Werteliste.
- `array2`: Der zweite Zellbereich oder die zweite Werteliste, genauso groß wie die erste.

## COVARIANCE.S

`COVARIANCE.S(array1, array2)`

Gibt zurück, wie stark sich zwei Zahlenreihen für eine Stichprobe gemeinsam verändern.

- `array1`: Der erste Zellbereich oder die erste Werteliste.
- `array2`: Der zweite Zellbereich oder die zweite Werteliste, genauso groß wie die erste.

## CRITBINOM

`CRITBINOM(trials, probability_s, alpha)`

Gibt die kleinste Anzahl von Erfolgen zurück, deren kumulierte Wahrscheinlichkeit einen Wert erreicht.

- `trials`: Die Anzahl der Versuche.
- `probability_s`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.

## DEVSQ

`DEVSQ(number1, ...)`

Gibt die Summe der Quadrate der Abstände der Zahlen von ihrem Mittelwert zurück.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## EXPON.DIST

`EXPON.DIST(x, lambda, cumulative)`

Gibt den Wert oder den kumulierten Anteil der Exponentialkurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `lambda`: Die Rate der Kurve, größer als 0.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## EXPONDIST

`EXPONDIST(x, lambda, cumulative)`

Gibt den Wert oder den kumulierten Anteil der Exponentialkurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `lambda`: Die Rate der Kurve, größer als 0.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## F.DIST

`F.DIST(x, deg_freedom1, deg_freedom2, cumulative)`

Gibt den Wert oder den kumulierten Anteil der F-Kurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `deg_freedom1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `deg_freedom2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## F.DIST.RT

`F.DIST.RT(x, deg_freedom1, deg_freedom2)`

Gibt den Anteil der F-Kurve oberhalb von x zurück.

- `x`: Der zu verwendende x-Wert.
- `deg_freedom1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `deg_freedom2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.

## F.INV

`F.INV(probability, deg_freedom1, deg_freedom2)`

Gibt den x-Wert für einen Anteil der F-Kurve zurück, von links gezählt.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `deg_freedom1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `deg_freedom2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.

## F.INV.RT

`F.INV.RT(probability, deg_freedom1, deg_freedom2)`

Gibt den x-Wert für einen Anteil der F-Kurve zurück, von rechts gezählt.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `deg_freedom1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `deg_freedom2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.

## F.TEST

`F.TEST(array1, array2)`

Gibt die Wahrscheinlichkeit zurück, dass zwei Zahlenreihen dieselbe Varianz haben.

- `array1`: Der erste Zellbereich oder die erste Werteliste.
- `array2`: Der zweite Zellbereich oder die zweite Werteliste, genauso groß wie die erste.

## FDIST

`FDIST(x, deg_freedom1, deg_freedom2)`

Gibt den Anteil der F-Kurve oberhalb von x zurück.

- `x`: Der zu verwendende x-Wert.
- `deg_freedom1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `deg_freedom2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.

## FINV

`FINV(probability, deg_freedom1, deg_freedom2)`

Gibt den x-Wert für einen Anteil der F-Kurve zurück, von rechts gezählt.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `deg_freedom1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `deg_freedom2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.

## FISHER

`FISHER(x)`

Gibt die Fisher-Transformation einer Zahl zwischen -1 und 1 zurück.

- `x`: Der zu verwendende x-Wert.

## FISHERINV

`FISHERINV(y)`

Gibt die Umkehrung der Fisher-Transformation zurück.

- `y`: Die Zahl, zu der die Umkehrung der Fisher-Transformation gesucht wird.

## FORECAST

`FORECAST(x, known_y's, known_x's)`

Sagt einen Wert entlang der am besten passenden Geraden voraus.

- `x`: Der x-Wert, für den ein y-Wert vorhergesagt werden soll.
- `known_y's`: Der Zellbereich, der die y-Werte enthält, die Sie bereits haben.
- `known_x's`: Der Zellbereich, der die x-Werte enthält, die Sie bereits haben.

## FORECAST.LINEAR

`FORECAST.LINEAR(x, known_y's, known_x's)`

Sagt einen Wert entlang der am besten passenden Geraden voraus.

- `x`: Der zu verwendende x-Wert.
- `known_y's`: Der Zellbereich, der die y-Werte enthält, die Sie bereits haben.
- `known_x's`: Der Zellbereich, der die x-Werte enthält, die Sie bereits haben.

## FREQUENCY

`FREQUENCY(data_array, bins_array)`

Zählt, wie viele Werte in jeden Bereich fallen, und gibt die Anzahlen als Spalte zurück.

- `data_array`: Der Zellbereich mit den zu zählenden Werten.
- `bins_array`: Der Zellbereich mit den Grenzen der Bereiche, in denen gezählt wird.

## FTEST

`FTEST(array1, array2)`

Gibt die Wahrscheinlichkeit zurück, dass zwei Zahlenreihen dieselbe Varianz haben.

- `array1`: Der erste Zellbereich oder die erste Werteliste.
- `array2`: Der zweite Zellbereich oder die zweite Werteliste, genauso groß wie die erste.

## GAMMA

`GAMMA(x)`

Gibt die Gammafunktion einer Zahl zurück.

- `x`: Der zu verwendende x-Wert.

## GAMMA.DIST

`GAMMA.DIST(x, alpha, beta, cumulative)`

Gibt den Wert oder den kumulierten Anteil der Gamma-Kurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `beta`: Eine Zahl, die die Form der Kurve bestimmt (bei der Gamma- und der Weibull-Kurve der Maßstab der Kurve).
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## GAMMA.INV

`GAMMA.INV(probability, alpha, beta)`

Gibt den x-Wert für einen Anteil der Gamma-Kurve zurück.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `beta`: Eine Zahl, die die Form der Kurve bestimmt (bei der Gamma- und der Weibull-Kurve der Maßstab der Kurve).

## GAMMADIST

`GAMMADIST(x, alpha, beta, cumulative)`

Gibt den Wert oder den kumulierten Anteil der Gamma-Kurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `beta`: Eine Zahl, die die Form der Kurve bestimmt (bei der Gamma- und der Weibull-Kurve der Maßstab der Kurve).
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## GAMMAINV

`GAMMAINV(probability, alpha, beta)`

Gibt den x-Wert für einen Anteil der Gamma-Kurve zurück.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `beta`: Eine Zahl, die die Form der Kurve bestimmt (bei der Gamma- und der Weibull-Kurve der Maßstab der Kurve).

## GAMMALN

`GAMMALN(x)`

Gibt den natürlichen Logarithmus der Gammafunktion zurück.

- `x`: Der zu verwendende x-Wert.

## GAMMALN.PRECISE

`GAMMALN.PRECISE(x)`

Gibt den natürlichen Logarithmus der Gammafunktion zurück.

- `x`: Der zu verwendende x-Wert.

## GAUSS

`GAUSS(z)`

Gibt den Anteil der Standardnormalverteilungskurve zwischen dem Mittelwert und z zurück.

- `z`: Der z-Wert.

## GEOMEAN

`GEOMEAN(number1, ...)`

Gibt das geometrische Mittel positiver Zahlen zurück.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## GROWTH

`GROWTH(known_y's, [known_x's], [new_x's], [const])`

Gibt die Werte entlang der am besten passenden Exponentialkurve für neue x-Werte zurück.

- `known_y's`: Der Zellbereich, der die y-Werte enthält, die Sie bereits haben.
- `known_x's` (optional): Der Zellbereich, der die x-Werte enthält, die Sie bereits haben.
- `new_x's` (optional): Die x-Werte, für die die y-Werte ermittelt werden (wenn weggelassen, die bekannten x-Werte).
- `const` (optional): WAHR oder weggelassen, um eine Gerade anzupassen, die nicht durch null gehen muss; FALSCH, damit sie durch null geht.

## HARMEAN

`HARMEAN(number1, ...)`

Gibt das harmonische Mittel positiver Zahlen zurück.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## HYPGEOM.DIST

`HYPGEOM.DIST(sample_s, number_sample, population_s, number_pop, cumulative)`

Gibt die Wahrscheinlichkeit zurück, eine Anzahl von Erfolgen zu ziehen, ohne die Elemente zurückzulegen.

- `sample_s`: Die Anzahl der Erfolge in der Stichprobe.
- `number_sample`: Die Anzahl der gezogenen Elemente.
- `population_s`: Die Anzahl der Erfolge in der gesamten Grundgesamtheit.
- `number_pop`: Die Anzahl der Elemente in der gesamten Grundgesamtheit.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## HYPGEOMDIST

`HYPGEOMDIST(sample_s, number_sample, population_s, number_pop)`

Gibt die Wahrscheinlichkeit zurück, eine Anzahl von Erfolgen zu ziehen, ohne die Elemente zurückzulegen.

- `sample_s`: Die Anzahl der Erfolge in der Stichprobe.
- `number_sample`: Die Anzahl der gezogenen Elemente.
- `population_s`: Die Anzahl der Erfolge in der gesamten Grundgesamtheit.
- `number_pop`: Die Anzahl der Elemente in der gesamten Grundgesamtheit.

## INTERCEPT

`INTERCEPT(known_y's, known_x's)`

Gibt an, wo die am besten passende Gerade die senkrechte Achse schneidet.

- `known_y's`: Der Zellbereich, der die y-Werte enthält, die Sie bereits haben.
- `known_x's`: Der Zellbereich, der die x-Werte enthält, die Sie bereits haben.

## KURT

`KURT(number1, ...)`

Gibt zurück, wie spitz oder flach eine Zahlenreihe im Vergleich zu einer Normalverteilungskurve ist.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## LARGE

`LARGE(array, k)`

Gibt die k-größte Zahl einer Menge zurück.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `k`: Welcher Rangplatz ausgegeben wird, von der größten Zahl aus gezählt: 1 ist die größte.

## LINEST

`LINEST(known_y's, [known_x's], [const], [stats])`

Passt eine Gerade oder Ebene nach der Methode der kleinsten Quadrate an die Daten an und gibt deren Kennzahlen als Zeile zurück.

- `known_y's`: Der Zellbereich, der die y-Werte enthält, die Sie bereits haben.
- `known_x's` (optional): Der Zellbereich, der die x-Werte enthält, die Sie bereits haben.
- `const` (optional): WAHR oder weggelassen, um eine Gerade anzupassen, die nicht durch null gehen muss; FALSCH, damit sie durch null geht.
- `stats` (optional): WAHR, um zusätzlich weitere Kennzahlen zur Anpassung zu erhalten; FALSCH oder weggelassen, um nur die Gerade zu erhalten.

## LOGEST

`LOGEST(known_y's, [known_x's], [const], [stats])`

Passt eine Exponentialkurve an die Daten an und gibt deren Kennzahlen als Zeile zurück.

- `known_y's`: Der Zellbereich, der die y-Werte enthält, die Sie bereits haben.
- `known_x's` (optional): Der Zellbereich, der die x-Werte enthält, die Sie bereits haben.
- `const` (optional): WAHR oder weggelassen, um eine Gerade anzupassen, die nicht durch null gehen muss; FALSCH, damit sie durch null geht.
- `stats` (optional): WAHR, um zusätzlich weitere Kennzahlen zur Anpassung zu erhalten; FALSCH oder weggelassen, um nur die Gerade zu erhalten.

## LOGINV

`LOGINV(probability, mean, standard_dev)`

Gibt den x-Wert für einen Anteil der logarithmischen Normalverteilungskurve zurück.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `mean`: Der Mittelwert der Verteilung.
- `standard_dev`: Die Standardabweichung der Verteilung.

## LOGNORM.DIST

`LOGNORM.DIST(x, mean, standard_dev, cumulative)`

Gibt den Wert oder den kumulierten Anteil der logarithmischen Normalverteilungskurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `mean`: Der Mittelwert der Verteilung.
- `standard_dev`: Die Standardabweichung der Verteilung.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## LOGNORM.INV

`LOGNORM.INV(probability, mean, standard_dev)`

Gibt den x-Wert für einen Anteil der logarithmischen Normalverteilungskurve zurück.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `mean`: Der Mittelwert der Verteilung.
- `standard_dev`: Die Standardabweichung der Verteilung.

## LOGNORMDIST

`LOGNORMDIST(x, mean, standard_dev)`

Gibt den kumulierten Anteil der logarithmischen Normalverteilungskurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `mean`: Der Mittelwert der Verteilung.
- `standard_dev`: Die Standardabweichung der Verteilung.

## MAXA

`MAXA(value1, ...)`

Gibt den größten Wert zurück, wobei Text als null und WAHR als eins gezählt wird.

- `value1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## MAXIFS

`MAXIFS(max_range, criteria_range1, criteria1, ...)`

Gibt die größte Zahl unter den Zellen zurück, die mehrere Bedingungen erfüllen.

- `max_range`: Die Zellen, aus denen der größte Wert ermittelt wird.
- `criteria_range1`: Der erste zu prüfende Zellbereich.
- `criteria1`: Wonach im ersten Zellbereich gesucht wird: eine Zahl, ein Text oder eine Prüfung wie ">5".
- `...`: Beliebig viele weitere derselben Art wie die davor.

## MEDIAN

`MEDIAN(number1, ...)`

Gibt den Median einer Zahlenreihe zurück.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## MINA

`MINA(value1, ...)`

Gibt den kleinsten Wert zurück, wobei Text als null und WAHR als eins gezählt wird.

- `value1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## MINIFS

`MINIFS(min_range, criteria_range1, criteria1, ...)`

Gibt die kleinste Zahl unter den Zellen zurück, die mehrere Bedingungen erfüllen.

- `min_range`: Die Zellen, aus denen der kleinste Wert ermittelt wird.
- `criteria_range1`: Der erste zu prüfende Zellbereich.
- `criteria1`: Wonach im ersten Zellbereich gesucht wird: eine Zahl, ein Text oder eine Prüfung wie ">5".
- `...`: Beliebig viele weitere derselben Art wie die davor.

## MODE

`MODE(number1, ...)`

Gibt die Zahl zurück, die am häufigsten vorkommt.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## MODE.MULT

`MODE.MULT(number1, ...)`

Gibt eine Spalte mit den Zahlen zurück, die in einer Reihe am häufigsten vorkommen.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## MODE.SNGL

`MODE.SNGL(number1, ...)`

Gibt die Zahl zurück, die in einer Reihe am häufigsten vorkommt.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## NEGBINOM.DIST

`NEGBINOM.DIST(number_f, number_s, probability_s, cumulative)`

Gibt die Wahrscheinlichkeit einer Anzahl von Misserfolgen vor einer Anzahl von Erfolgen zurück.

- `number_f`: Die Anzahl der Misserfolge.
- `number_s`: Die Anzahl der Erfolge.
- `probability_s`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## NEGBINOMDIST

`NEGBINOMDIST(number_f, number_s, probability_s)`

Gibt die Wahrscheinlichkeit einer Anzahl von Misserfolgen vor einer Anzahl von Erfolgen zurück.

- `number_f`: Die Anzahl der Misserfolge.
- `number_s`: Die Anzahl der Erfolge.
- `probability_s`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.

## NORM.DIST

`NORM.DIST(x, mean, standard_dev, cumulative)`

Gibt den Wert oder den kumulierten Anteil der Normalverteilungskurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `mean`: Der Mittelwert der Verteilung.
- `standard_dev`: Die Standardabweichung der Verteilung.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## NORM.INV

`NORM.INV(probability, mean, standard_dev)`

Gibt den x-Wert für einen Anteil einer Normalverteilungskurve zurück.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `mean`: Der Mittelwert der Verteilung.
- `standard_dev`: Die Standardabweichung der Verteilung.

## NORM.S.DIST

`NORM.S.DIST(z, cumulative)`

Gibt den Wert oder den kumulierten Anteil der Standardnormalverteilungskurve an z zurück.

- `z`: Der z-Wert.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## NORM.S.INV

`NORM.S.INV(probability)`

Gibt den z-Wert für einen Anteil der Standardnormalverteilungskurve zurück.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.

## NORMDIST

`NORMDIST(x, mean, standard_dev, cumulative)`

Gibt den Wert oder den kumulierten Anteil der Normalverteilungskurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `mean`: Der Mittelwert der Verteilung.
- `standard_dev`: Die Standardabweichung der Verteilung.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## NORMINV

`NORMINV(probability, mean, standard_dev)`

Gibt den x-Wert für einen Anteil einer Normalverteilungskurve zurück.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `mean`: Der Mittelwert der Verteilung.
- `standard_dev`: Die Standardabweichung der Verteilung.

## NORMSDIST

`NORMSDIST(z)`

Gibt den kumulierten Anteil der Standardnormalverteilungskurve bis z zurück.

- `z`: Der z-Wert.

## NORMSINV

`NORMSINV(probability)`

Gibt den z-Wert für einen Anteil der Standardnormalverteilungskurve zurück.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.

## PEARSON

`PEARSON(array1, array2)`

Gibt zurück, wie stark sich zwei Zahlenreihen gemeinsam bewegen, von -1 bis 1.

- `array1`: Der erste Zellbereich oder die erste Werteliste.
- `array2`: Der zweite Zellbereich oder die zweite Werteliste, genauso groß wie die erste.

## PERCENTILE

`PERCENTILE(array, k)`

Gibt den Wert zurück, unterhalb dessen ein Anteil k der Zahlen liegt, von 0 bis 1.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `k`: Der Anteil der Werte, von 0 bis 1, für den der Wert ausgegeben wird.

## PERCENTILE.EXC

`PERCENTILE.EXC(array, k)`

Gibt den Wert zurück, unterhalb dessen ein Anteil k der Zahlen liegt, ohne 0 und 1.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `k`: Der Rangplatz in der Reihenfolge, von 1 an gezählt.

## PERCENTILE.INC

`PERCENTILE.INC(array, k)`

Gibt den Wert zurück, unterhalb dessen ein Anteil k der Zahlen liegt, von 0 bis 1.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `k`: Der Rangplatz in der Reihenfolge, von 1 an gezählt.

## PERCENTRANK

`PERCENTRANK(array, x, [significance])`

Gibt den Rang eines Wertes in einer Zahlenreihe als Anteil von 0 bis 1 zurück.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `x`: Der zu verwendende x-Wert.
- `significance` (optional): Die Zahl, auf deren Vielfache gerundet wird.

## PERCENTRANK.EXC

`PERCENTRANK.EXC(array, x, [significance])`

Gibt den Rang eines Wertes in einer Zahlenreihe als Anteil zurück, ohne 0 und 1.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `x`: Der zu verwendende x-Wert.
- `significance` (optional): Die Zahl, auf deren Vielfache gerundet wird.

## PERCENTRANK.INC

`PERCENTRANK.INC(array, x, [significance])`

Gibt den Rang eines Wertes in einer Zahlenreihe als Anteil von 0 bis 1 zurück.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `x`: Der zu verwendende x-Wert.
- `significance` (optional): Die Zahl, auf deren Vielfache gerundet wird.

## PERMUTATIONA

`PERMUTATIONA(number, number_chosen)`

Gibt die Anzahl der geordneten Auswahlen zurück, wenn ein Element mehrmals gewählt werden kann.

- `number`: Die zu verarbeitende Zahl.
- `number_chosen`: Wie viele Elemente in jeder Gruppe ausgewählt werden.

## PHI

`PHI(x)`

Gibt die Höhe der Standardnormalverteilungskurve an x zurück.

- `x`: Der zu verwendende x-Wert.

## POISSON

`POISSON(x, mean, cumulative)`

Gibt die Wahrscheinlichkeit einer Anzahl von Ereignissen zurück, ausgehend von der erwarteten Anzahl.

- `x`: Der zu verwendende x-Wert.
- `mean`: Der Mittelwert der Verteilung.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## POISSON.DIST

`POISSON.DIST(x, mean, cumulative)`

Gibt die Wahrscheinlichkeit einer Anzahl von Ereignissen zurück, ausgehend von der erwarteten Anzahl.

- `x`: Der zu verwendende x-Wert.
- `mean`: Der Mittelwert der Verteilung.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## PROB

`PROB(x_range, prob_range, lower_limit, [upper_limit])`

Gibt die Wahrscheinlichkeit zurück, dass Werte zwischen zwei Grenzen liegen.

- `x_range`: Der Zellbereich mit den x-Werten.
- `prob_range`: Die Wahrscheinlichkeiten zu jedem x-Wert, jeweils von 0 bis 1, zusammen 1.
- `lower_limit`: Die untere Grenze des Bereichs.
- `upper_limit` (optional): Die obere Grenze des Bereichs (wenn weggelassen, nur die untere Grenze).

## QUARTILE

`QUARTILE(array, quart)`

Gibt die Quartilswerte einer Zahlenreihe zurück.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `quart`: Welches Quartil: 0 kleinster Wert, 1 erstes Quartil, 2 Median, 3 drittes Quartil, 4 größter Wert.

## QUARTILE.EXC

`QUARTILE.EXC(array, quart)`

Gibt die Quartilswerte einer Zahlenreihe zurück, ohne die Enden.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `quart`: Welches Quartil: 0 kleinster Wert, 1 erstes Quartil, 2 Median, 3 drittes Quartil, 4 größter Wert.

## QUARTILE.INC

`QUARTILE.INC(array, quart)`

Gibt die Quartilswerte einer Zahlenreihe zurück, vom niedrigsten bis zum höchsten Wert.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `quart`: Welches Quartil: 0 kleinster Wert, 1 erstes Quartil, 2 Median, 3 drittes Quartil, 4 größter Wert.

## RANK

`RANK(number, ref, [order])`

Gibt den Rangplatz einer Zahl in einer Liste zurück, wenn die Liste sortiert wird.

- `number`: Die zu verarbeitende Zahl.
- `ref`: Eine Zelle oder ein Zellbereich.
- `order` (optional): 0 oder weggelassen setzt die größte Zahl zuerst; jede andere Zahl setzt die kleinste zuerst.

## RANK.AVG

`RANK.AVG(number, ref, [order])`

Gibt den Rang einer Zahl in einer Liste zurück, bei Gleichstand den mittleren Rang.

- `number`: Die zu verarbeitende Zahl.
- `ref`: Eine Zelle oder ein Zellbereich.
- `order` (optional): 0 oder weggelassen setzt die größte Zahl zuerst; jede andere Zahl setzt die kleinste zuerst.

## RANK.EQ

`RANK.EQ(number, ref, [order])`

Gibt den Rang einer Zahl in einer Liste zurück, bei Gleichstand den höchsten Rang.

- `number`: Die zu verarbeitende Zahl.
- `ref`: Eine Zelle oder ein Zellbereich.
- `order` (optional): 0 oder weggelassen setzt die größte Zahl zuerst; jede andere Zahl setzt die kleinste zuerst.

## RSQ

`RSQ(known_y's, known_x's)`

Gibt an, wie gut eine Gerade zwei Zahlenreihen anpasst.

- `known_y's`: Der Zellbereich, der die y-Werte enthält, die Sie bereits haben.
- `known_x's`: Der Zellbereich, der die x-Werte enthält, die Sie bereits haben.

## SKEW

`SKEW(number1, ...)`

Gibt zurück, wie unsymmetrisch eine Zahlenreihe um ihren Mittelwert verteilt ist, für eine Stichprobe.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## SKEW.P

`SKEW.P(number1, ...)`

Gibt zurück, wie unsymmetrisch eine Zahlenreihe um ihren Mittelwert verteilt ist, für eine ganze Grundgesamtheit.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## SLOPE

`SLOPE(known_y's, known_x's)`

Gibt die Steigung der Geraden zurück, die zwei Zahlenreihen am besten anpasst.

- `known_y's`: Der Zellbereich, der die y-Werte enthält, die Sie bereits haben.
- `known_x's`: Der Zellbereich, der die x-Werte enthält, die Sie bereits haben.

## SMALL

`SMALL(array, k)`

Gibt die k-kleinste Zahl einer Menge zurück.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `k`: Welcher Rangplatz ausgegeben wird, von der kleinsten Zahl aus gezählt: 1 ist die kleinste.

## STANDARDIZE

`STANDARDIZE(x, mean, standard_dev)`

Gibt zurück, wie viele Standardabweichungen ein Wert vom Mittelwert entfernt ist.

- `x`: Der zu verwendende x-Wert.
- `mean`: Der Mittelwert der Verteilung.
- `standard_dev`: Die Standardabweichung der Verteilung.

## STDEV

`STDEV(number1, ...)`

Schätzt die Streuung einer Grundgesamtheit anhand einer Stichprobe daraus.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## STDEV.P

`STDEV.P(number1, ...)`

Gibt die Streuung einer ganzen Grundgesamtheit zurück.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## STDEV.S

`STDEV.S(number1, ...)`

Schätzt die Streuung einer Grundgesamtheit anhand einer Stichprobe daraus.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## STDEVA

`STDEVA(value1, ...)`

Gibt die Standardabweichung einer Stichprobe zurück, wobei Text als 0 und WAHR als 1 gezählt wird.

- `value1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## STDEVP

`STDEVP(number1, ...)`

Gibt die Streuung einer ganzen Grundgesamtheit zurück.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## STDEVPA

`STDEVPA(value1, ...)`

Gibt die Standardabweichung einer Grundgesamtheit zurück, wobei Text als 0 und WAHR als 1 gezählt wird.

- `value1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## STEYX

`STEYX(known_y's, known_x's)`

Gibt den Standardfehler der geschätzten y-Werte einer angepassten Geraden zurück.

- `known_y's`: Der Zellbereich, der die y-Werte enthält, die Sie bereits haben.
- `known_x's`: Der Zellbereich, der die x-Werte enthält, die Sie bereits haben.

## SUMIF

`SUMIF(range, criteria, [sum_range])`

Addiert die Zellen, die eine Bedingung erfüllen.

- `range`: Der zu prüfende Zellbereich.
- `criteria`: Wonach gesucht wird: eine Zahl, ein Text oder eine Prüfung wie ">5".
- `sum_range` (optional): Die Zellen, die addiert werden (wenn weggelassen, die Zellen, die geprüft werden).

## SUMIFS

`SUMIFS(sum_range, criteria_range1, criteria1, ...)`

Addiert die Zellen, die mehrere Bedingungen erfüllen.

- `sum_range`: Die Zellen, die addiert werden (wenn weggelassen, die Zellen, die geprüft werden).
- `criteria_range1`: Der erste zu prüfende Zellbereich.
- `criteria1`: Wonach im ersten Zellbereich gesucht wird: eine Zahl, ein Text oder eine Prüfung wie ">5".
- `...`: Beliebig viele weitere derselben Art wie die davor.

## T.DIST

`T.DIST(x, deg_freedom, cumulative)`

Gibt den Wert oder den kumulierten Anteil der t-Verteilung nach Student an x zurück.

- `x`: Der zu verwendende x-Wert.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## T.DIST.2T

`T.DIST.2T(x, deg_freedom)`

Gibt den Anteil der t-Verteilung nach Student außerhalb von minus x und x zurück.

- `x`: Der zu verwendende x-Wert.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## T.DIST.RT

`T.DIST.RT(x, deg_freedom)`

Gibt den Anteil der t-Verteilung nach Student oberhalb von x zurück.

- `x`: Der zu verwendende x-Wert.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## T.INV

`T.INV(probability, deg_freedom)`

Gibt den x-Wert für einen Anteil der t-Verteilung nach Student zurück, von links gezählt.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## T.INV.2T

`T.INV.2T(probability, deg_freedom)`

Gibt den x-Wert für einen Anteil der t-Verteilung nach Student zurück, auf beide Enden verteilt.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## T.TEST

`T.TEST(array1, array2, tails, type)`

Gibt die Wahrscheinlichkeit zurück, dass die Mittelwerte zweier Zahlenreihen gleich sind.

- `array1`: Der erste Zellbereich oder die erste Werteliste.
- `array2`: Der zweite Zellbereich oder die zweite Werteliste, genauso groß wie die erste.
- `tails`: 1 für eine Seite der Kurve, 2 für beide.
- `type`: Wann Zahlungen geleistet werden: 0 oder weggelassen am Ende jeder Periode, 1 am Anfang.

## TDIST

`TDIST(x, deg_freedom, tails)`

Gibt den Anteil der t-Verteilung nach Student in einer Seite oder in beiden Seiten zurück.

- `x`: Der zu verwendende x-Wert.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.
- `tails`: 1 für eine Seite der Kurve, 2 für beide.

## TINV

`TINV(probability, deg_freedom)`

Gibt den x-Wert für einen Anteil der t-Verteilung nach Student zurück, auf beide Enden verteilt.

- `probability`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `deg_freedom`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## TREND

`TREND(known_y's, [known_x's], [new_x's], [const])`

Gibt die Werte entlang der am besten passenden Geraden für neue x-Werte zurück.

- `known_y's`: Der Zellbereich, der die y-Werte enthält, die Sie bereits haben.
- `known_x's` (optional): Der Zellbereich, der die x-Werte enthält, die Sie bereits haben.
- `new_x's` (optional): Die x-Werte, für die die y-Werte ermittelt werden (wenn weggelassen, die bekannten x-Werte).
- `const` (optional): WAHR oder weggelassen, um eine Gerade anzupassen, die nicht durch null gehen muss; FALSCH, damit sie durch null geht.

## TRIMMEAN

`TRIMMEAN(array, percent)`

Gibt den Mittelwert einer Zahlenreihe zurück, nachdem ein Anteil der höchsten und niedrigsten Werte entfernt wurde.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `percent`: Der Anteil der Werte, von 0 bis unter 1, der an beiden Enden weggelassen wird.

## TTEST

`TTEST(array1, array2, tails, type)`

Gibt die Wahrscheinlichkeit zurück, dass die Mittelwerte zweier Zahlenreihen gleich sind.

- `array1`: Der erste Zellbereich oder die erste Werteliste.
- `array2`: Der zweite Zellbereich oder die zweite Werteliste, genauso groß wie die erste.
- `tails`: 1 für eine Seite der Kurve, 2 für beide.
- `type`: Wann Zahlungen geleistet werden: 0 oder weggelassen am Ende jeder Periode, 1 am Anfang.

## VAR

`VAR(number1, ...)`

Schätzt die Varianz einer Grundgesamtheit anhand einer Stichprobe daraus.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## VAR.P

`VAR.P(number1, ...)`

Gibt die Varianz einer ganzen Grundgesamtheit zurück.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## VAR.S

`VAR.S(number1, ...)`

Schätzt die Varianz einer Grundgesamtheit anhand einer Stichprobe daraus.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## VARA

`VARA(value1, ...)`

Gibt die Varianz einer Stichprobe zurück, wobei Text als 0 und WAHR als 1 gezählt wird.

- `value1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## VARP

`VARP(number1, ...)`

Gibt die Varianz einer ganzen Grundgesamtheit zurück.

- `number1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## VARPA

`VARPA(value1, ...)`

Gibt die Varianz einer Grundgesamtheit zurück, wobei Text als 0 und WAHR als 1 gezählt wird.

- `value1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie die davor.

## WEIBULL

`WEIBULL(x, alpha, beta, cumulative)`

Gibt den Wert oder den kumulierten Anteil der Weibull-Kurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `beta`: Eine Zahl, die die Form der Kurve bestimmt (bei der Gamma- und der Weibull-Kurve der Maßstab der Kurve).
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## WEIBULL.DIST

`WEIBULL.DIST(x, alpha, beta, cumulative)`

Gibt den Wert oder den kumulierten Anteil der Weibull-Kurve an x zurück.

- `x`: Der zu verwendende x-Wert.
- `alpha`: Die akzeptierte Irrtumswahrscheinlichkeit, etwa 0,05 für ein Konfidenzniveau von 95 Prozent.
- `beta`: Eine Zahl, die die Form der Kurve bestimmt (bei der Gamma- und der Weibull-Kurve der Maßstab der Kurve).
- `cumulative`: WAHR, um die laufende Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## Z.TEST

`Z.TEST(array, x, [sigma])`

Gibt die einseitige Wahrscheinlichkeit zurück, dass der Mittelwert einer Stichprobe über einem Wert liegt.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `x`: Der zu verwendende x-Wert.
- `sigma` (optional): Die Standardabweichung der gesamten Grundgesamtheit (wenn weggelassen, die der Stichprobe).

## ZTEST

`ZTEST(array, x, [sigma])`

Gibt die einseitige Wahrscheinlichkeit zurück, dass der Mittelwert einer Stichprobe über einem Wert liegt.

- `array`: Ein Zellbereich oder eine in geschweiften Klammern geschriebene Werteliste.
- `x`: Der zu verwendende x-Wert.
- `sigma` (optional): Die Standardabweichung der gesamten Grundgesamtheit (wenn weggelassen, die der Stichprobe).

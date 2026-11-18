# Statistische Funktionen
<!-- keywords: Funktionen, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.ETS, FORECAST.ETS.CONFINT, FORECAST.ETS.SEASONALITY, FORECAST.ETS.STAT, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST -->

Dies sind die statistischen Funktionen, die JeffCrunch berechnen kann. Jede zeigt, wie sie geschrieben wird, was sie tut und was jedes ihrer Argumente bedeutet. Ein Argument in eckigen Klammern ist optional, und „...“ bedeutet, dass Sie beliebig viele weitere des letzten Arguments angeben können. Um eine Funktion auszuprobieren, siehe [Funktion auswählen](insert-function). Während Sie in einer Zelle eine Funktion eingeben, zeigt ein kleiner Tipp unter der Zelle ihre Argumente an. Ein Klick auf den Namen der Funktion im Tipp öffnet ihren Eintrag hier.

## AVEDEV

`AVEDEV(Zahl1, ...)`

Gibt den Mittelwert der Abstände an, die Zahlen von ihrem Mittelwert haben.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## AVERAGE

`AVERAGE(Zahl1, ...)`

Gibt den Mittelwert von Zahlen an.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## AVERAGEA

`AVERAGEA(Wert1, ...)`

Gibt den Mittelwert von Werten an; Text zählt als 0 und WAHR als 1.

- `Wert1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## AVERAGEIF

`AVERAGEIF(Bereich, Kriterien, [Mittelwertbereich])`

Bildet den Mittelwert der Zellen, die eine Bedingung erfüllen.

- `Bereich`: Der zu prüfende Zellbereich.
- `Kriterien`: Wonach gesucht wird: eine Zahl, ein Text oder eine Vergleichsangabe wie ">5".
- `Mittelwertbereich` (optional): Die Zellen, deren Mittelwert gebildet wird (ohne Angabe die Zellen, die geprüft werden).

## AVERAGEIFS

`AVERAGEIFS(Mittelwertbereich, Kriterienbereich1, Kriterien1, ...)`

Bildet den Mittelwert der Zellen, die mehrere Bedingungen erfüllen.

- `Mittelwertbereich`: Die Zellen, deren Mittelwert gebildet wird (ohne Angabe die Zellen, die geprüft werden).
- `Kriterienbereich1`: Der erste zu prüfende Zellbereich.
- `Kriterien1`: Wonach im ersten Zellbereich gesucht wird: eine Zahl, ein Text oder eine Vergleichsangabe wie ">5".
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## BETA.DIST

`BETA.DIST(x, Alpha, Beta, Kumuliert, [A], [B])`

Gibt den Wert oder den kumulierten Anteil der Betaverteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Beta`: Eine Zahl, die die Kurve formt (bei der Gamma- und der Weibull-Verteilung der Maßstab der Kurve).
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.
- `A` (optional): Die untere Grenze von x (ohne Angabe 0).
- `B` (optional): Die obere Grenze von x (ohne Angabe 1).

## BETA.INV

`BETA.INV(Wahrscheinlichkeit, Alpha, Beta, [A], [B])`

Gibt den x-Wert für einen Anteil einer Betaverteilung an.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Beta`: Eine Zahl, die die Kurve formt (bei der Gamma- und der Weibull-Verteilung der Maßstab der Kurve).
- `A` (optional): Die untere Grenze von x (ohne Angabe 0).
- `B` (optional): Die obere Grenze von x (ohne Angabe 1).

## BETADIST

`BETADIST(x, Alpha, Beta, [A], [B])`

Gibt den kumulierten Anteil der Betaverteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Beta`: Eine Zahl, die die Kurve formt (bei der Gamma- und der Weibull-Verteilung der Maßstab der Kurve).
- `A` (optional): Die untere Grenze von x (ohne Angabe 0).
- `B` (optional): Die obere Grenze von x (ohne Angabe 1).

## BETAINV

`BETAINV(Wahrscheinlichkeit, Alpha, Beta, [A], [B])`

Gibt den x-Wert für einen Anteil einer Betaverteilung an.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Beta`: Eine Zahl, die die Kurve formt (bei der Gamma- und der Weibull-Verteilung der Maßstab der Kurve).
- `A` (optional): Die untere Grenze von x (ohne Angabe 0).
- `B` (optional): Die obere Grenze von x (ohne Angabe 1).

## BINOM.DIST

`BINOM.DIST(Anzahl_Erfolge, Versuche, Wahrscheinlichkeit_Erfolg, Kumuliert)`

Gibt die Wahrscheinlichkeit für eine Anzahl von Erfolgen bei einer Anzahl von Versuchen an.

- `Anzahl_Erfolge`: Die Anzahl der Erfolge.
- `Versuche`: Die Anzahl der Versuche.
- `Wahrscheinlichkeit_Erfolg`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(Versuche, Wahrscheinlichkeit_Erfolg, Anzahl_Erfolge, [Anzahl_Erfolge2])`

Gibt die Wahrscheinlichkeit an, dass die Erfolge in einen Bereich fallen.

- `Versuche`: Die Anzahl der Versuche.
- `Wahrscheinlichkeit_Erfolg`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `Anzahl_Erfolge`: Die Anzahl der Erfolge.
- `Anzahl_Erfolge2` (optional): Die höchste zu zählende Anzahl an Erfolgen (ohne Angabe dieselbe wie die niedrigste).

## BINOM.INV

`BINOM.INV(Versuche, Wahrscheinlichkeit_Erfolg, Alpha)`

Gibt die kleinste Anzahl von Erfolgen an, deren kumulierte Wahrscheinlichkeit einen Wert erreicht.

- `Versuche`: Die Anzahl der Versuche.
- `Wahrscheinlichkeit_Erfolg`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.

## BINOMDIST

`BINOMDIST(Anzahl_Erfolge, Versuche, Wahrscheinlichkeit_Erfolg, Kumuliert)`

Gibt die Wahrscheinlichkeit für eine Anzahl von Erfolgen bei einer Anzahl von Versuchen an.

- `Anzahl_Erfolge`: Die Anzahl der Erfolge.
- `Versuche`: Die Anzahl der Versuche.
- `Wahrscheinlichkeit_Erfolg`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## CHIDIST

`CHIDIST(x, Freiheitsgrade)`

Gibt den Anteil der Chi-Quadrat-Verteilung oberhalb von x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## CHIINV

`CHIINV(Wahrscheinlichkeit, Freiheitsgrade)`

Gibt den x-Wert für einen Anteil der Chi-Quadrat-Verteilung an, von rechts gezählt.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## CHISQ.DIST

`CHISQ.DIST(x, Freiheitsgrade, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der Chi-Quadrat-Verteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x, Freiheitsgrade)`

Gibt den Anteil der Chi-Quadrat-Verteilung oberhalb von x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## CHISQ.INV

`CHISQ.INV(Wahrscheinlichkeit, Freiheitsgrade)`

Gibt den x-Wert für einen Anteil der Chi-Quadrat-Verteilung an, von links gezählt.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## CHISQ.INV.RT

`CHISQ.INV.RT(Wahrscheinlichkeit, Freiheitsgrade)`

Gibt den x-Wert für einen Anteil der Chi-Quadrat-Verteilung an, von rechts gezählt.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## CHISQ.TEST

`CHISQ.TEST(Istbereich, Erwartungsbereich)`

Gibt die Wahrscheinlichkeit an, dass die Anzahlen von den erwarteten Anzahlen nur durch Zufall abweichen.

- `Istbereich`: Der Zellbereich mit den tatsächlich erhaltenen Anzahlen.
- `Erwartungsbereich`: Der Zellbereich mit den erwarteten Anzahlen, in derselben Größe wie die tatsächlich erhaltenen Anzahlen.

## CHITEST

`CHITEST(Istbereich, Erwartungsbereich)`

Gibt die Wahrscheinlichkeit an, dass die Anzahlen von den erwarteten Anzahlen nur durch Zufall abweichen.

- `Istbereich`: Der Zellbereich mit den tatsächlich erhaltenen Anzahlen.
- `Erwartungsbereich`: Der Zellbereich mit den erwarteten Anzahlen, in derselben Größe wie die tatsächlich erhaltenen Anzahlen.

## CONFIDENCE

`CONFIDENCE(Alpha, Standardabw, Umfang)`

Gibt die Spanne eines Konfidenzintervalls für einen Mittelwert an, mit der Normalverteilung.

- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Standardabw`: Die Standardabweichung der Verteilung.
- `Umfang`: Der Umfang der Stichprobe, eine ganze Zahl von 1 oder mehr.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(Alpha, Standardabw, Umfang)`

Gibt die Spanne eines Konfidenzintervalls für einen Mittelwert an, mit der Normalverteilung.

- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Standardabw`: Die Standardabweichung der Verteilung.
- `Umfang`: Der Umfang der Stichprobe, eine ganze Zahl von 1 oder mehr.

## CONFIDENCE.T

`CONFIDENCE.T(Alpha, Standardabw, Umfang)`

Gibt die Spanne eines Konfidenzintervalls für einen Mittelwert an, mit der t-Verteilung nach Student.

- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Standardabw`: Die Standardabweichung der Verteilung.
- `Umfang`: Der Umfang der Stichprobe, eine ganze Zahl von 1 oder mehr.

## CORREL

`CORREL(Matrix1, Matrix2)`

Gibt an, wie eng sich zwei Zahlenreihen gemeinsam bewegen, von -1 bis 1.

- `Matrix1`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix2`: Der zweite Zellbereich oder die zweite Werteliste, in derselben Größe wie der erste.

## COUNT

`COUNT(Wert1, ...)`

Zählt die Zahlen in einer Liste oder einem Zellbereich.

- `Wert1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## COUNTA

`COUNTA(Wert1, ...)`

Zählt die Zellen, die nicht leer sind.

- `Wert1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## COUNTBLANK

`COUNTBLANK(Bereich)`

Zählt die leeren Zellen in einem Bereich.

- `Bereich`: Der zu prüfende Zellbereich.

## COUNTIF

`COUNTIF(Bereich, Kriterien)`

Zählt die Zellen, die eine Bedingung erfüllen.

- `Bereich`: Der zu prüfende Zellbereich.
- `Kriterien`: Wonach gesucht wird: eine Zahl, ein Text oder eine Vergleichsangabe wie ">5".

## COUNTIFS

`COUNTIFS(Kriterienbereich1, Kriterien1, ...)`

Zählt die Zeilen, in denen mehrere Bedingungen gleichzeitig erfüllt sind.

- `Kriterienbereich1`: Der erste zu prüfende Zellbereich.
- `Kriterien1`: Wonach im ersten Zellbereich gesucht wird: eine Zahl, ein Text oder eine Vergleichsangabe wie ">5".
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## COVAR

`COVAR(Matrix1, Matrix2)`

Gibt an, wie stark zwei Zahlenreihen gemeinsam variieren.

- `Matrix1`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix2`: Der zweite Zellbereich oder die zweite Werteliste, in derselben Größe wie der erste.

## COVARIANCE.P

`COVARIANCE.P(Matrix1, Matrix2)`

Gibt an, wie stark zwei Zahlenreihen gemeinsam variieren, für eine Grundgesamtheit.

- `Matrix1`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix2`: Der zweite Zellbereich oder die zweite Werteliste, in derselben Größe wie der erste.

## COVARIANCE.S

`COVARIANCE.S(Matrix1, Matrix2)`

Gibt an, wie stark zwei Zahlenreihen gemeinsam variieren, für eine Stichprobe.

- `Matrix1`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix2`: Der zweite Zellbereich oder die zweite Werteliste, in derselben Größe wie der erste.

## CRITBINOM

`CRITBINOM(Versuche, Wahrscheinlichkeit_Erfolg, Alpha)`

Gibt die kleinste Anzahl von Erfolgen an, deren kumulierte Wahrscheinlichkeit einen Wert erreicht.

- `Versuche`: Die Anzahl der Versuche.
- `Wahrscheinlichkeit_Erfolg`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.

## DEVSQ

`DEVSQ(Zahl1, ...)`

Gibt die Summe der Quadrate der Abstände an, die Zahlen von ihrem Mittelwert haben.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## EXPON.DIST

`EXPON.DIST(x, Lambda, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der Exponentialverteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Lambda`: Die Rate der Kurve, größer als 0.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## EXPONDIST

`EXPONDIST(x, Lambda, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der Exponentialverteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Lambda`: Die Rate der Kurve, größer als 0.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## F.DIST

`F.DIST(x, Freiheitsgrad1, Freiheitsgrad2, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der F-Verteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Freiheitsgrad1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `Freiheitsgrad2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## F.DIST.RT

`F.DIST.RT(x, Freiheitsgrad1, Freiheitsgrad2)`

Gibt den Anteil der F-Verteilung oberhalb von x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Freiheitsgrad1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `Freiheitsgrad2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.

## F.INV

`F.INV(Wahrscheinlichkeit, Freiheitsgrad1, Freiheitsgrad2)`

Gibt den x-Wert für einen Anteil der F-Verteilung an, von links gezählt.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Freiheitsgrad1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `Freiheitsgrad2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.

## F.INV.RT

`F.INV.RT(Wahrscheinlichkeit, Freiheitsgrad1, Freiheitsgrad2)`

Gibt den x-Wert für einen Anteil der F-Verteilung an, von rechts gezählt.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Freiheitsgrad1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `Freiheitsgrad2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.

## F.TEST

`F.TEST(Matrix1, Matrix2)`

Gibt die Wahrscheinlichkeit an, dass zwei Zahlenreihen dieselbe Varianz haben.

- `Matrix1`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix2`: Der zweite Zellbereich oder die zweite Werteliste, in derselben Größe wie der erste.

## FDIST

`FDIST(x, Freiheitsgrad1, Freiheitsgrad2)`

Gibt den Anteil der F-Verteilung oberhalb von x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Freiheitsgrad1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `Freiheitsgrad2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.

## FINV

`FINV(Wahrscheinlichkeit, Freiheitsgrad1, Freiheitsgrad2)`

Gibt den x-Wert für einen Anteil der F-Verteilung an, von rechts gezählt.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Freiheitsgrad1`: Die Freiheitsgrade des Zählers, eine ganze Zahl von 1 oder mehr.
- `Freiheitsgrad2`: Die Freiheitsgrade des Nenners, eine ganze Zahl von 1 oder mehr.

## FISHER

`FISHER(x)`

Gibt die Fisher-Transformation einer Zahl zwischen -1 und 1 an.

- `x`: Der x-Wert, mit dem gerechnet wird.

## FISHERINV

`FISHERINV(y)`

Gibt die Umkehrung der Fisher-Transformation an.

- `y`: Die Zahl, zu der die Umkehrung der Fisher-Transformation gesucht wird.

## FORECAST

`FORECAST(x, Bekannte_y-Werte, Bekannte_x-Werte)`

Sagt einen Wert entlang der am besten passenden Geraden voraus.

- `x`: Der x-Wert, für den ein y-Wert vorhergesagt werden soll.
- `Bekannte_y-Werte`: Der Zellbereich mit den y-Werten, die Sie bereits haben.
- `Bekannte_x-Werte`: Der Zellbereich mit den x-Werten, die Sie bereits haben.

## FORECAST.ETS

`FORECAST.ETS(Zieldatum, Werte, Zeitachse, [Saisonalität], [Datenergänzung], [Aggregation])`

Sagt einen künftigen Wert einer Reihe mit Trend und Saison voraus, durch exponentielle Glättung.

- `Zieldatum`: Der Zeitpunkt, für den ein Wert vorhergesagt werden soll.
- `Werte`: Die Werte, die Sie bereits haben.
- `Zeitachse`: Die Datumsangaben oder Zahlen der Werte, in gleichen Abständen.
- `Saisonalität`: 1, um die Saison aus den Werten zu ermitteln, 0 für keine, oder die Länge der Saison.
- `Datenergänzung`: 1, um einen fehlenden Punkt durch eine Linie zwischen seinen Nachbarn zu füllen, 0, um ihn mit null zu füllen.
- `Aggregation`: Wie Werte zum selben Zeitpunkt zusammengefasst werden: 1 Mittelwert, 2 Anzahl, 3 Anzahl (nicht leer), 4 Maximum, 5 Median, 6 Minimum, 7 Summe.

## FORECAST.ETS.CONFINT

`FORECAST.ETS.CONFINT(Zieldatum, Werte, Zeitachse, [Konfidenzniveau], [Saisonalität], [Datenergänzung], [Aggregation])`

Gibt an, wie weit der Wert über und unter einer geglätteten Prognose wahrscheinlich liegt.

- `Konfidenzniveau`: Eine Zahl über 0 und unter 1, ohne Angabe 0,95.

## FORECAST.ETS.SEASONALITY

`FORECAST.ETS.SEASONALITY(Werte, Zeitachse, [Datenergänzung], [Aggregation])`

Gibt die Länge der sich wiederholenden Saison an, die in einer Reihe gefunden wird, oder 1, wenn es keine gibt.

## FORECAST.ETS.STAT

`FORECAST.ETS.STAT(Werte, Zeitachse, Statistiktyp, [Saisonalität], [Datenergänzung], [Aggregation])`

Gibt eine der Glättungskonstanten oder Anpassungsmaße einer geglätteten Prognose an.

- `Statistiktyp`: 1 Alpha, 2 Beta, 3 Gamma, 4 MASE, 5 SMAPE, 6 MAE, 7 RMSE, 8 der Schritt der Zeitachse.

## FORECAST.LINEAR

`FORECAST.LINEAR(x, Bekannte_y-Werte, Bekannte_x-Werte)`

Sagt einen Wert entlang der am besten passenden Geraden voraus.

- `x`: Der zu verwendende x-Wert.
- `Bekannte_y-Werte`: Der Zellbereich mit den y-Werten, die Sie bereits haben.
- `Bekannte_x-Werte`: Der Zellbereich mit den x-Werten, die Sie bereits haben.

## FREQUENCY

`FREQUENCY(Datenfeld, Klassenfeld)`

Zählt, wie viele Werte in jeden Bereich fallen, und gibt die Anzahlen als Spalte aus.

- `Datenfeld`: Der Zellbereich mit den zu zählenden Werten.
- `Klassenfeld`: Der Zellbereich mit den Grenzen der Bereiche, in denen gezählt wird.

## FTEST

`FTEST(Matrix1, Matrix2)`

Gibt die Wahrscheinlichkeit an, dass zwei Zahlenreihen dieselbe Varianz haben.

- `Matrix1`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix2`: Der zweite Zellbereich oder die zweite Werteliste, in derselben Größe wie der erste.

## GAMMA

`GAMMA(x)`

Gibt die Gammafunktion einer Zahl an.

- `x`: Der x-Wert, mit dem gerechnet wird.

## GAMMA.DIST

`GAMMA.DIST(x, Alpha, Beta, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der Gammaverteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Beta`: Eine Zahl, die die Kurve formt (bei der Gamma- und der Weibull-Verteilung der Maßstab der Kurve).
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## GAMMA.INV

`GAMMA.INV(Wahrscheinlichkeit, Alpha, Beta)`

Gibt den x-Wert für einen Anteil einer Gammaverteilung an.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Beta`: Eine Zahl, die die Kurve formt (bei der Gamma- und der Weibull-Verteilung der Maßstab der Kurve).

## GAMMADIST

`GAMMADIST(x, Alpha, Beta, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der Gammaverteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Beta`: Eine Zahl, die die Kurve formt (bei der Gamma- und der Weibull-Verteilung der Maßstab der Kurve).
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## GAMMAINV

`GAMMAINV(Wahrscheinlichkeit, Alpha, Beta)`

Gibt den x-Wert für einen Anteil einer Gammaverteilung an.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Beta`: Eine Zahl, die die Kurve formt (bei der Gamma- und der Weibull-Verteilung der Maßstab der Kurve).

## GAMMALN

`GAMMALN(x)`

Gibt den natürlichen Logarithmus der Gammafunktion an.

- `x`: Der x-Wert, mit dem gerechnet wird.

## GAMMALN.PRECISE

`GAMMALN.PRECISE(x)`

Gibt den natürlichen Logarithmus der Gammafunktion an.

- `x`: Der x-Wert, mit dem gerechnet wird.

## GAUSS

`GAUSS(z)`

Gibt den Anteil der Standardnormalverteilung zwischen dem Mittelwert und z an.

- `z`: Der z-Wert.

## GEOMEAN

`GEOMEAN(Zahl1, ...)`

Gibt das geometrische Mittel positiver Zahlen an.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## GROWTH

`GROWTH(Bekannte_y-Werte, [Bekannte_x-Werte], [Neue_x-Werte], [Konstante])`

Gibt die Werte entlang der am besten passenden Exponentialkurve für neue x-Werte an.

- `Bekannte_y-Werte`: Der Zellbereich mit den y-Werten, die Sie bereits haben.
- `Bekannte_x-Werte` (optional): Der Zellbereich mit den x-Werten, die Sie bereits haben.
- `Neue_x-Werte` (optional): Die x-Werte, für die die y-Werte angegeben werden (ohne Angabe die bekannten x-Werte).
- `Konstante` (optional): WAHR oder keine Angabe, um eine Linie anzupassen, die nicht durch null gehen muss; FALSCH, damit sie durch null geht.

## HARMEAN

`HARMEAN(Zahl1, ...)`

Gibt das harmonische Mittel positiver Zahlen an.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## HYPGEOM.DIST

`HYPGEOM.DIST(Stichprobe_Erfolge, Anzahl_Stichprobe, Grundgesamtheit_Erfolge, Anzahl_Grundgesamtheit, Kumuliert)`

Gibt die Wahrscheinlichkeit an, eine Anzahl von Erfolgen zu ziehen, ohne die Elemente zurückzulegen.

- `Stichprobe_Erfolge`: Die Anzahl der Erfolge in der Stichprobe.
- `Anzahl_Stichprobe`: Die Anzahl der gezogenen Elemente.
- `Grundgesamtheit_Erfolge`: Die Anzahl der Erfolge in der gesamten Grundgesamtheit.
- `Anzahl_Grundgesamtheit`: Die Anzahl der Elemente in der gesamten Grundgesamtheit.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## HYPGEOMDIST

`HYPGEOMDIST(Stichprobe_Erfolge, Anzahl_Stichprobe, Grundgesamtheit_Erfolge, Anzahl_Grundgesamtheit)`

Gibt die Wahrscheinlichkeit an, eine Anzahl von Erfolgen zu ziehen, ohne die Elemente zurückzulegen.

- `Stichprobe_Erfolge`: Die Anzahl der Erfolge in der Stichprobe.
- `Anzahl_Stichprobe`: Die Anzahl der gezogenen Elemente.
- `Grundgesamtheit_Erfolge`: Die Anzahl der Erfolge in der gesamten Grundgesamtheit.
- `Anzahl_Grundgesamtheit`: Die Anzahl der Elemente in der gesamten Grundgesamtheit.

## INTERCEPT

`INTERCEPT(Bekannte_y-Werte, Bekannte_x-Werte)`

Gibt an, wo die am besten passende Gerade die senkrechte Achse schneidet.

- `Bekannte_y-Werte`: Der Zellbereich mit den y-Werten, die Sie bereits haben.
- `Bekannte_x-Werte`: Der Zellbereich mit den x-Werten, die Sie bereits haben.

## KURT

`KURT(Zahl1, ...)`

Gibt an, wie spitz oder flach eine Zahlenreihe im Vergleich zu einer Normalverteilung ist.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## LARGE

`LARGE(Matrix, K)`

Gibt die k-größte Zahl einer Reihe an.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `K`: Welcher Platz ausgegeben wird, von der größten Zahl an gezählt: 1 ist die größte.

## LINEST

`LINEST(Bekannte_y-Werte, [Bekannte_x-Werte], [Konstante], [Statistik])`

Passt der Daten eine Gerade oder Ebene nach der Methode der kleinsten Quadrate an und gibt ihre Zahlen als Zeile aus.

- `Bekannte_y-Werte`: Der Zellbereich mit den y-Werten, die Sie bereits haben.
- `Bekannte_x-Werte` (optional): Der Zellbereich mit den x-Werten, die Sie bereits haben.
- `Konstante` (optional): WAHR oder keine Angabe, um eine Linie anzupassen, die nicht durch null gehen muss; FALSCH, damit sie durch null geht.
- `Statistik` (optional): WAHR, um zusätzlich weitere Zahlen zur Anpassung auszugeben; FALSCH oder keine Angabe, um nur die Gerade auszugeben.

## LOGEST

`LOGEST(Bekannte_y-Werte, [Bekannte_x-Werte], [Konstante], [Statistik])`

Passt den Daten eine Exponentialkurve an und gibt ihre Zahlen als Zeile aus.

- `Bekannte_y-Werte`: Der Zellbereich mit den y-Werten, die Sie bereits haben.
- `Bekannte_x-Werte` (optional): Der Zellbereich mit den x-Werten, die Sie bereits haben.
- `Konstante` (optional): WAHR oder keine Angabe, um eine Linie anzupassen, die nicht durch null gehen muss; FALSCH, damit sie durch null geht.
- `Statistik` (optional): WAHR, um zusätzlich weitere Zahlen zur Anpassung auszugeben; FALSCH oder keine Angabe, um nur die Linie auszugeben.

## LOGINV

`LOGINV(Wahrscheinlichkeit, Mittelwert, Standardabw)`

Gibt den x-Wert für einen Anteil einer logarithmischen Normalverteilung an.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Mittelwert`: Der Mittelwert der Verteilung.
- `Standardabw`: Die Standardabweichung der Verteilung.

## LOGNORM.DIST

`LOGNORM.DIST(x, Mittelwert, Standardabw, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der logarithmischen Normalverteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Mittelwert`: Der Mittelwert der Verteilung.
- `Standardabw`: Die Standardabweichung der Verteilung.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## LOGNORM.INV

`LOGNORM.INV(Wahrscheinlichkeit, Mittelwert, Standardabw)`

Gibt den x-Wert für einen Anteil einer logarithmischen Normalverteilung an.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Mittelwert`: Der Mittelwert der Verteilung.
- `Standardabw`: Die Standardabweichung der Verteilung.

## LOGNORMDIST

`LOGNORMDIST(x, Mittelwert, Standardabw)`

Gibt den kumulierten Anteil der logarithmischen Normalverteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Mittelwert`: Der Mittelwert der Verteilung.
- `Standardabw`: Die Standardabweichung der Verteilung.

## MAXA

`MAXA(Wert1, ...)`

Gibt den größten Wert an; Text zählt als 0 und WAHR als 1.

- `Wert1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## MAXIFS

`MAXIFS(Maximalbereich, Kriterienbereich1, Kriterien1, ...)`

Gibt die größte Zahl unter den Zellen an, die mehrere Bedingungen erfüllen.

- `Maximalbereich`: Die Zellen, aus denen der größte Wert ermittelt wird.
- `Kriterienbereich1`: Der erste zu prüfende Zellbereich.
- `Kriterien1`: Wonach im ersten Zellbereich gesucht wird: eine Zahl, ein Text oder eine Vergleichsangabe wie ">5".
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## MEDIAN

`MEDIAN(Zahl1, ...)`

Gibt die mittlere Zahl einer Reihe an.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## MINA

`MINA(Wert1, ...)`

Gibt den kleinsten Wert an; Text zählt als 0 und WAHR als 1.

- `Wert1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## MINIFS

`MINIFS(Minimalbereich, Kriterienbereich1, Kriterien1, ...)`

Gibt die kleinste Zahl unter den Zellen an, die mehrere Bedingungen erfüllen.

- `Minimalbereich`: Die Zellen, aus denen der kleinste Wert ermittelt wird.
- `Kriterienbereich1`: Der erste zu prüfende Zellbereich.
- `Kriterien1`: Wonach im ersten Zellbereich gesucht wird: eine Zahl, ein Text oder eine Vergleichsangabe wie ">5".
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## MODE

`MODE(Zahl1, ...)`

Gibt die Zahl an, die am häufigsten vorkommt.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## MODE.MULT

`MODE.MULT(Zahl1, ...)`

Gibt eine Spalte mit den Zahlen aus, die in einer Reihe am häufigsten vorkommen.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## MODE.SNGL

`MODE.SNGL(Zahl1, ...)`

Gibt die Zahl an, die in einer Reihe am häufigsten vorkommt.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## NEGBINOM.DIST

`NEGBINOM.DIST(Anzahl_Misserfolge, Anzahl_Erfolge, Wahrscheinlichkeit_Erfolg, Kumuliert)`

Gibt die Wahrscheinlichkeit für eine Anzahl von Misserfolgen vor einer Anzahl von Erfolgen an.

- `Anzahl_Misserfolge`: Die Anzahl der Misserfolge.
- `Anzahl_Erfolge`: Die Anzahl der Erfolge.
- `Wahrscheinlichkeit_Erfolg`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## NEGBINOMDIST

`NEGBINOMDIST(Anzahl_Misserfolge, Anzahl_Erfolge, Wahrscheinlichkeit_Erfolg)`

Gibt die Wahrscheinlichkeit für eine Anzahl von Misserfolgen vor einer Anzahl von Erfolgen an.

- `Anzahl_Misserfolge`: Die Anzahl der Misserfolge.
- `Anzahl_Erfolge`: Die Anzahl der Erfolge.
- `Wahrscheinlichkeit_Erfolg`: Die Wahrscheinlichkeit eines Erfolgs bei jedem Versuch, von 0 bis 1.

## NORM.DIST

`NORM.DIST(x, Mittelwert, Standardabw, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der Normalverteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Mittelwert`: Der Mittelwert der Verteilung.
- `Standardabw`: Die Standardabweichung der Verteilung.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## NORM.INV

`NORM.INV(Wahrscheinlichkeit, Mittelwert, Standardabw)`

Gibt den x-Wert für einen Anteil einer Normalverteilung an.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Mittelwert`: Der Mittelwert der Verteilung.
- `Standardabw`: Die Standardabweichung der Verteilung.

## NORM.S.DIST

`NORM.S.DIST(z, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der Standardnormalverteilung an der Stelle z an.

- `z`: Der z-Wert.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## NORM.S.INV

`NORM.S.INV(Wahrscheinlichkeit)`

Gibt den z-Wert für einen Anteil der Standardnormalverteilung an.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.

## NORMDIST

`NORMDIST(x, Mittelwert, Standardabw, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der Normalverteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Mittelwert`: Der Mittelwert der Verteilung.
- `Standardabw`: Die Standardabweichung der Verteilung.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## NORMINV

`NORMINV(Wahrscheinlichkeit, Mittelwert, Standardabw)`

Gibt den x-Wert für einen Anteil einer Normalverteilung an.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Mittelwert`: Der Mittelwert der Verteilung.
- `Standardabw`: Die Standardabweichung der Verteilung.

## NORMSDIST

`NORMSDIST(z)`

Gibt den kumulierten Anteil der Standardnormalverteilung bis z an.

- `z`: Der z-Wert.

## NORMSINV

`NORMSINV(Wahrscheinlichkeit)`

Gibt den z-Wert für einen Anteil der Standardnormalverteilung an.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.

## PEARSON

`PEARSON(Matrix1, Matrix2)`

Gibt an, wie eng sich zwei Zahlenreihen gemeinsam bewegen, von -1 bis 1.

- `Matrix1`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix2`: Der zweite Zellbereich oder die zweite Werteliste, in derselben Größe wie der erste.

## PERCENTILE

`PERCENTILE(Matrix, K)`

Gibt den Wert an, unterhalb dessen ein Anteil k der Zahlen liegt.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `K`: Der Anteil der Werte, von 0 bis 1, für den der Wert ausgegeben wird.

## PERCENTILE.EXC

`PERCENTILE.EXC(Matrix, K)`

Gibt den Wert an, unterhalb dessen ein Anteil k der Zahlen liegt, wobei 0 und 1 ausgeschlossen sind.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `K`: Die Position in der Reihenfolge, von 1 an gezählt.

## PERCENTILE.INC

`PERCENTILE.INC(Matrix, K)`

Gibt den Wert an, unterhalb dessen ein Anteil k der Zahlen liegt, von 0 bis 1.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `K`: Die Position in der Reihenfolge, von 1 an gezählt.

## PERCENTRANK

`PERCENTRANK(Matrix, x, [Stellen])`

Gibt den Rang eines Werts in einer Reihe von Zahlen als Anteil von 0 bis 1 an.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `x`: Der x-Wert, mit dem gerechnet wird.
- `Stellen` (optional): Die Anzahl der Nachkommastellen des Ergebnisses (ohne Angabe 3).

## PERCENTRANK.EXC

`PERCENTRANK.EXC(Matrix, x, [Stellen])`

Gibt den Rang eines Werts in einer Reihe von Zahlen als Anteil an, wobei 0 und 1 ausgeschlossen sind.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `x`: Der x-Wert, mit dem gerechnet wird.
- `Stellen` (optional): Die Anzahl der Nachkommastellen des Ergebnisses (ohne Angabe 3).

## PERCENTRANK.INC

`PERCENTRANK.INC(Matrix, x, [Stellen])`

Gibt den Rang eines Werts in einer Reihe von Zahlen als Anteil von 0 bis 1 an.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `x`: Der x-Wert, mit dem gerechnet wird.
- `Stellen` (optional): Die Anzahl der Nachkommastellen des Ergebnisses (ohne Angabe 3).

## PERMUTATIONA

`PERMUTATIONA(Anzahl, Anzahl_Auswahl)`

Gibt die Anzahl der geordneten Auswahlen an, wenn ein Element mehrfach gewählt werden kann.

- `Anzahl`: Die zu verarbeitende Zahl.
- `Anzahl_Auswahl`: Wie viele Elemente in jeder Gruppe ausgewählt werden.

## PHI

`PHI(x)`

Gibt die Höhe der Standardnormalverteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.

## POISSON

`POISSON(x, Mittelwert, Kumuliert)`

Gibt die Wahrscheinlichkeit für eine Anzahl von Ereignissen an, ausgehend von der erwarteten Anzahl.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Mittelwert`: Der Mittelwert der Verteilung.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## POISSON.DIST

`POISSON.DIST(x, Mittelwert, Kumuliert)`

Gibt die Wahrscheinlichkeit für eine Anzahl von Ereignissen an, ausgehend von der erwarteten Anzahl.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Mittelwert`: Der Mittelwert der Verteilung.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## PROB

`PROB(x_Bereich, Wahrscheinlichkeitsbereich, Untergrenze, [Obergrenze])`

Gibt die Wahrscheinlichkeit an, dass Werte zwischen zwei Grenzen liegen.

- `x_Bereich`: Der Zellbereich mit den x-Werten.
- `Wahrscheinlichkeitsbereich`: Die Wahrscheinlichkeiten zu jedem x-Wert, jeweils von 0 bis 1, die zusammen 1 ergeben.
- `Untergrenze`: Die untere Grenze des Bereichs.
- `Obergrenze` (optional): Die obere Grenze des Bereichs (ohne Angabe nur die Untergrenze).

## QUARTILE

`QUARTILE(Matrix, Quartil)`

Gibt die Quartilswerte einer Reihe von Zahlen an.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `Quartil`: Welches Quartil: 0 kleinster Wert, 1 erstes Quartil, 2 Median, 3 drittes Quartil, 4 größter Wert.

## QUARTILE.EXC

`QUARTILE.EXC(Matrix, Quartil)`

Gibt die Quartilswerte einer Reihe von Zahlen an, ohne die Enden.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `Quartil`: Welches Quartil: 0 kleinster Wert, 1 erstes Quartil, 2 Median, 3 drittes Quartil, 4 größter Wert.

## QUARTILE.INC

`QUARTILE.INC(Matrix, Quartil)`

Gibt die Quartilswerte einer Reihe von Zahlen an, vom niedrigsten bis zum höchsten Wert.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `Quartil`: Welches Quartil: 0 kleinster Wert, 1 erstes Quartil, 2 Median, 3 drittes Quartil, 4 größter Wert.

## RANK

`RANK(Zahl, Bezug, [Reihenfolge])`

Gibt den Platz einer Zahl in einer Liste an, wenn die Liste sortiert wird.

- `Zahl`: Die zu verarbeitende Zahl.
- `Bezug`: Eine Zelle oder ein Zellbereich.
- `Reihenfolge` (optional): 0 oder keine Angabe setzt die größte zuerst; jede andere Zahl setzt die kleinste zuerst.

## RANK.AVG

`RANK.AVG(Zahl, Bezug, [Reihenfolge])`

Gibt den Rang einer Zahl in einer Liste an und bei Gleichstand den mittleren Rang.

- `Zahl`: Die zu verarbeitende Zahl.
- `Bezug`: Eine Zelle oder ein Zellbereich.
- `Reihenfolge` (optional): 0 oder keine Angabe setzt die größte zuerst; jede andere Zahl setzt die kleinste zuerst.

## RANK.EQ

`RANK.EQ(Zahl, Bezug, [Reihenfolge])`

Gibt den Rang einer Zahl in einer Liste an und bei Gleichstand den oberen Rang.

- `Zahl`: Die zu verarbeitende Zahl.
- `Bezug`: Eine Zelle oder ein Zellbereich.
- `Reihenfolge` (optional): 0 oder keine Angabe setzt die größte zuerst; jede andere Zahl setzt die kleinste zuerst.

## RSQ

`RSQ(Bekannte_y-Werte, Bekannte_x-Werte)`

Gibt an, wie gut eine Gerade zwei Zahlenreihen anpasst.

- `Bekannte_y-Werte`: Der Zellbereich mit den y-Werten, die Sie bereits haben.
- `Bekannte_x-Werte`: Der Zellbereich mit den x-Werten, die Sie bereits haben.

## SKEW

`SKEW(Zahl1, ...)`

Gibt an, wie schief eine Zahlenreihe um ihren Mittelwert verteilt ist, für eine Stichprobe.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## SKEW.P

`SKEW.P(Zahl1, ...)`

Gibt an, wie schief eine Zahlenreihe um ihren Mittelwert verteilt ist, für eine ganze Grundgesamtheit.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## SLOPE

`SLOPE(Bekannte_y-Werte, Bekannte_x-Werte)`

Gibt die Steigung der Geraden an, die zwei Zahlenreihen am besten anpasst.

- `Bekannte_y-Werte`: Der Zellbereich mit den y-Werten, die Sie bereits haben.
- `Bekannte_x-Werte`: Der Zellbereich mit den x-Werten, die Sie bereits haben.

## SMALL

`SMALL(Matrix, K)`

Gibt die k-kleinste Zahl einer Reihe an.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `K`: Welcher Platz ausgegeben wird, vom kleinsten an gezählt: 1 ist der kleinste.

## STANDARDIZE

`STANDARDIZE(x, Mittelwert, Standardabw)`

Gibt an, wie viele Standardabweichungen ein Wert vom Mittelwert entfernt ist.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Mittelwert`: Der Mittelwert der Verteilung.
- `Standardabw`: Die Standardabweichung der Verteilung.

## STDEV

`STDEV(Zahl1, ...)`

Schätzt die Streuung einer Grundgesamtheit anhand einer Stichprobe daraus.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## STDEV.P

`STDEV.P(Zahl1, ...)`

Gibt die Streuung einer ganzen Grundgesamtheit an.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## STDEV.S

`STDEV.S(Zahl1, ...)`

Schätzt die Streuung einer Grundgesamtheit anhand einer Stichprobe daraus.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## STDEVA

`STDEVA(Wert1, ...)`

Gibt die Standardabweichung einer Stichprobe an; Text zählt als 0 und WAHR als 1.

- `Wert1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## STDEVP

`STDEVP(Zahl1, ...)`

Gibt die Streuung einer ganzen Grundgesamtheit an.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## STDEVPA

`STDEVPA(Wert1, ...)`

Gibt die Standardabweichung einer Grundgesamtheit an; Text zählt als 0 und WAHR als 1.

- `Wert1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## STEYX

`STEYX(Bekannte_y-Werte, Bekannte_x-Werte)`

Gibt den Standardfehler der vorhergesagten y-Werte einer angepassten Geraden an.

- `Bekannte_y-Werte`: Der Zellbereich mit den y-Werten, die Sie bereits haben.
- `Bekannte_x-Werte`: Der Zellbereich mit den x-Werten, die Sie bereits haben.

## SUMIF

`SUMIF(Bereich, Kriterien, [Summenbereich])`

Addiert die Zellen, die eine Bedingung erfüllen.

- `Bereich`: Der zu prüfende Zellbereich.
- `Kriterien`: Wonach gesucht wird: eine Zahl, ein Text oder eine Vergleichsangabe wie ">5".
- `Summenbereich` (optional): Die Zellen, die addiert werden (ohne Angabe die Zellen, die geprüft werden).

## SUMIFS

`SUMIFS(Summenbereich, Kriterienbereich1, Kriterien1, ...)`

Addiert die Zellen, die mehrere Bedingungen erfüllen.

- `Summenbereich`: Die Zellen, die addiert werden (ohne Angabe die Zellen, die geprüft werden).
- `Kriterienbereich1`: Der erste zu prüfende Zellbereich.
- `Kriterien1`: Wonach im ersten Zellbereich gesucht wird: eine Zahl, ein Text oder eine Vergleichsangabe wie ">5".
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## T.DIST

`T.DIST(x, Freiheitsgrade, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der t-Verteilung nach Student an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## T.DIST.2T

`T.DIST.2T(x, Freiheitsgrade)`

Gibt den Anteil der t-Verteilung nach Student außerhalb von minus x und x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## T.DIST.RT

`T.DIST.RT(x, Freiheitsgrade)`

Gibt den Anteil der t-Verteilung nach Student oberhalb von x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## T.INV

`T.INV(Wahrscheinlichkeit, Freiheitsgrade)`

Gibt den x-Wert für einen Anteil der t-Verteilung nach Student an, von links gezählt.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## T.INV.2T

`T.INV.2T(Wahrscheinlichkeit, Freiheitsgrade)`

Gibt den x-Wert für einen Anteil der t-Verteilung nach Student an, aufgeteilt auf beide Enden.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## T.TEST

`T.TEST(Matrix1, Matrix2, Seiten, Typ)`

Gibt die Wahrscheinlichkeit an, dass die Mittelwerte zweier Zahlenreihen gleich sind.

- `Matrix1`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix2`: Der zweite Zellbereich oder die zweite Werteliste, in derselben Größe wie der erste.
- `Seiten`: 1 für eine Seite der Kurve, 2 für beide.
- `Typ`: Wann Zahlungen geleistet werden: 0 oder keine Angabe am Ende jeder Periode, 1 am Anfang.

## TDIST

`TDIST(x, Freiheitsgrade, Seiten)`

Gibt den Anteil der t-Verteilung nach Student in einer Seite oder in beiden an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.
- `Seiten`: 1 für eine Seite der Kurve, 2 für beide.

## TINV

`TINV(Wahrscheinlichkeit, Freiheitsgrade)`

Gibt den x-Wert für einen Anteil der t-Verteilung nach Student an, aufgeteilt auf beide Enden.

- `Wahrscheinlichkeit`: Eine Wahrscheinlichkeit, von 0 bis 1.
- `Freiheitsgrade`: Die Freiheitsgrade, eine ganze Zahl von 1 oder mehr.

## TREND

`TREND(Bekannte_y-Werte, [Bekannte_x-Werte], [Neue_x-Werte], [Konstante])`

Gibt die Werte entlang der am besten passenden Geraden für neue x-Werte an.

- `Bekannte_y-Werte`: Der Zellbereich mit den y-Werten, die Sie bereits haben.
- `Bekannte_x-Werte` (optional): Der Zellbereich mit den x-Werten, die Sie bereits haben.
- `Neue_x-Werte` (optional): Die x-Werte, für die die y-Werte angegeben werden (ohne Angabe die bekannten x-Werte).
- `Konstante` (optional): WAHR oder keine Angabe, um eine Linie anzupassen, die nicht durch null gehen muss; FALSCH, damit sie durch null geht.

## TRIMMEAN

`TRIMMEAN(Matrix, Anteil)`

Gibt den Mittelwert einer Reihe von Zahlen an, nachdem ein Anteil der höchsten und niedrigsten Werte entfernt wurde.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `Anteil`: Der Anteil der Werte, von 0 bis unter 1, der an beiden Enden ausgelassen wird.

## TTEST

`TTEST(Matrix1, Matrix2, Seiten, Typ)`

Gibt die Wahrscheinlichkeit an, dass die Mittelwerte zweier Zahlenreihen gleich sind.

- `Matrix1`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix2`: Der zweite Zellbereich oder die zweite Werteliste, in derselben Größe wie der erste.
- `Seiten`: 1 für eine Seite der Kurve, 2 für beide.
- `Typ`: Wann Zahlungen geleistet werden: 0 oder keine Angabe am Ende jeder Periode, 1 am Anfang.

## VAR

`VAR(Zahl1, ...)`

Schätzt die Varianz einer Grundgesamtheit anhand einer Stichprobe daraus.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## VAR.P

`VAR.P(Zahl1, ...)`

Gibt die Varianz einer ganzen Grundgesamtheit an.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## VAR.S

`VAR.S(Zahl1, ...)`

Schätzt die Varianz einer Grundgesamtheit anhand einer Stichprobe daraus.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## VARA

`VARA(Wert1, ...)`

Gibt die Varianz einer Stichprobe an; Text zählt als 0 und WAHR als 1.

- `Wert1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## VARP

`VARP(Zahl1, ...)`

Gibt die Varianz einer ganzen Grundgesamtheit an.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## VARPA

`VARPA(Wert1, ...)`

Gibt die Varianz einer Grundgesamtheit an; Text zählt als 0 und WAHR als 1.

- `Wert1`: Der erste Wert oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorangehende Argument.

## WEIBULL

`WEIBULL(x, Alpha, Beta, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der Weibull-Verteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Beta`: Eine Zahl, die die Kurve formt (bei der Gamma- und der Weibull-Verteilung der Maßstab der Kurve).
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## WEIBULL.DIST

`WEIBULL.DIST(x, Alpha, Beta, Kumuliert)`

Gibt den Wert oder den kumulierten Anteil der Weibull-Verteilung an der Stelle x an.

- `x`: Der x-Wert, mit dem gerechnet wird.
- `Alpha`: Die Irrtumswahrscheinlichkeit, die Sie akzeptieren, z. B. 0,05 für ein Konfidenzniveau von 95 Prozent.
- `Beta`: Eine Zahl, die die Kurve formt (bei der Gamma- und der Weibull-Verteilung der Maßstab der Kurve).
- `Kumuliert`: WAHR, um die Summe bis zu diesem Punkt zu erhalten; FALSCH, um nur den Wert an diesem Punkt zu erhalten.

## Z.TEST

`Z.TEST(Matrix, x, [Sigma])`

Gibt die einseitige Wahrscheinlichkeit an, dass der Mittelwert einer Stichprobe über einem Wert liegt.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `x`: Der x-Wert, mit dem gerechnet wird.
- `Sigma` (optional): Die Standardabweichung der gesamten Grundgesamtheit (ohne Angabe die der Stichprobe).

## ZTEST

`ZTEST(Matrix, x, [Sigma])`

Gibt die einseitige Wahrscheinlichkeit an, dass der Mittelwert einer Stichprobe über einem Wert liegt.

- `Matrix`: Ein Zellbereich oder eine in geschweifte Klammern geschriebene Werteliste.
- `x`: Der x-Wert, mit dem gerechnet wird.
- `Sigma` (optional): Die Standardabweichung der gesamten Grundgesamtheit (ohne Angabe die der Stichprobe).

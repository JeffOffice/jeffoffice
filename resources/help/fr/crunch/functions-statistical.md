# Fonctions statistiques
<!-- keywords: functions, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.ETS, FORECAST.ETS.CONFINT, FORECAST.ETS.SEASONALITY, FORECAST.ETS.STAT, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST -->

Voici les fonctions statistiques que JeffCrunch peut calculer. Chacune indique son écriture, ce qu'elle fait et le rôle de chacun de ses arguments. Un argument entre crochets est facultatif, et « ... » signifie que vous pouvez en ajouter autant que nécessaire après le dernier. Pour essayer une fonction, consultez [Choisir une fonction](insert-function). Pendant que vous saisissez une fonction dans une cellule, une petite info-bulle sous la cellule affiche ses arguments ; un clic sur le nom de la fonction dans l'info-bulle ouvre sa fiche ici.

## AVEDEV

`AVEDEV(number1, ...)`

Renvoie la moyenne des écarts absolus des nombres par rapport à leur moyenne.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## AVERAGE

`AVERAGE(number1, ...)`

Renvoie la moyenne des nombres.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## AVERAGEA

`AVERAGEA(value1, ...)`

Renvoie la moyenne des valeurs, en comptant le texte pour zéro et VRAI pour un.

- `value1` : La première valeur, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## AVERAGEIF

`AVERAGEIF(range, criteria, [average_range])`

Calcule la moyenne des cellules qui répondent à une condition.

- `range` : La plage de cellules à tester.
- `criteria` : Ce qu'il faut rechercher : un nombre, un texte, ou un test tel que ">5".
- `average_range` (facultatif) : Les cellules dont calculer la moyenne (celles de la plage testée si l'argument est omis).

## AVERAGEIFS

`AVERAGEIFS(average_range, criteria_range1, criteria1, ...)`

Calcule la moyenne des cellules qui répondent à plusieurs conditions.

- `average_range` : Les cellules dont calculer la moyenne (celles de la première plage testée si l'argument est omis).
- `criteria_range1` : La première plage de cellules à tester.
- `criteria1` : Ce qu'il faut rechercher dans la première plage de cellules : un nombre, un texte, ou un test tel que ">5".
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## BETA.DIST

`BETA.DIST(x, alpha, beta, cumulative, [A], [B])`

Renvoie la valeur ou la proportion cumulée de la courbe bêta en x.

- `x` : La valeur x à traiter.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `beta` : Un nombre qui façonne la courbe (l'échelle de la courbe pour les courbes gamma et de Weibull).
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.
- `A` (facultatif) : La borne inférieure de x (0 si l'argument est omis).
- `B` (facultatif) : La borne supérieure de x (1 si l'argument est omis).

## BETA.INV

`BETA.INV(probability, alpha, beta, [A], [B])`

Renvoie la valeur x correspondant à une proportion de la courbe bêta.

- `probability` : Une probabilité, de 0 à 1.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `beta` : Un nombre qui façonne la courbe (l'échelle de la courbe pour les courbes gamma et de Weibull).
- `A` (facultatif) : La borne inférieure de x (0 si l'argument est omis).
- `B` (facultatif) : La borne supérieure de x (1 si l'argument est omis).

## BETADIST

`BETADIST(x, alpha, beta, [A], [B])`

Renvoie la proportion cumulée de la courbe bêta en x.

- `x` : La valeur x à traiter.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `beta` : Un nombre qui façonne la courbe (l'échelle de la courbe pour les courbes gamma et de Weibull).
- `A` (facultatif) : La borne inférieure de x (0 si l'argument est omis).
- `B` (facultatif) : La borne supérieure de x (1 si l'argument est omis).

## BETAINV

`BETAINV(probability, alpha, beta, [A], [B])`

Renvoie la valeur x correspondant à une proportion de la courbe bêta.

- `probability` : Une probabilité, de 0 à 1.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `beta` : Un nombre qui façonne la courbe (l'échelle de la courbe pour les courbes gamma et de Weibull).
- `A` (facultatif) : La borne inférieure de x (0 si l'argument est omis).
- `B` (facultatif) : La borne supérieure de x (1 si l'argument est omis).

## BINOM.DIST

`BINOM.DIST(number_s, trials, probability_s, cumulative)`

Renvoie la probabilité d'un nombre de succès sur un nombre d'essais.

- `number_s` : Le nombre de succès.
- `trials` : Le nombre d'essais.
- `probability_s` : La probabilité de succès à chaque essai, de 0 à 1.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(trials, probability_s, number_s, [number_s2])`

Renvoie la probabilité que le nombre de succès se situe dans un intervalle.

- `trials` : Le nombre d'essais.
- `probability_s` : La probabilité de succès à chaque essai, de 0 à 1.
- `number_s` : Le nombre de succès.
- `number_s2` (facultatif) : Le nombre maximal de succès à compter (égal au nombre minimal si l'argument est omis).

## BINOM.INV

`BINOM.INV(trials, probability_s, alpha)`

Renvoie le plus petit nombre de succès dont la probabilité cumulée atteint une valeur donnée.

- `trials` : Le nombre d'essais.
- `probability_s` : La probabilité de succès à chaque essai, de 0 à 1.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.

## BINOMDIST

`BINOMDIST(number_s, trials, probability_s, cumulative)`

Renvoie la probabilité d'un nombre de succès sur un nombre d'essais.

- `number_s` : Le nombre de succès.
- `trials` : Le nombre d'essais.
- `probability_s` : La probabilité de succès à chaque essai, de 0 à 1.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## CHIDIST

`CHIDIST(x, deg_freedom)`

Renvoie la proportion de la courbe du khi-deux au-delà de x.

- `x` : La valeur x à traiter.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.

## CHIINV

`CHIINV(probability, deg_freedom)`

Renvoie la valeur x correspondant à une proportion de la courbe du khi-deux, comptée depuis la droite.

- `probability` : Une probabilité, de 0 à 1.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.

## CHISQ.DIST

`CHISQ.DIST(x, deg_freedom, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe du khi-deux en x.

- `x` : La valeur x à traiter.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x, deg_freedom)`

Renvoie la proportion de la courbe du khi-deux au-delà de x.

- `x` : La valeur x à traiter.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.

## CHISQ.INV

`CHISQ.INV(probability, deg_freedom)`

Renvoie la valeur x correspondant à une proportion de la courbe du khi-deux, comptée depuis la gauche.

- `probability` : Une probabilité, de 0 à 1.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.

## CHISQ.INV.RT

`CHISQ.INV.RT(probability, deg_freedom)`

Renvoie la valeur x correspondant à une proportion de la courbe du khi-deux, comptée depuis la droite.

- `probability` : Une probabilité, de 0 à 1.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.

## CHISQ.TEST

`CHISQ.TEST(actual_range, expected_range)`

Renvoie la probabilité que les effectifs ne diffèrent des effectifs attendus que par hasard.

- `actual_range` : La plage de cellules contenant les effectifs observés.
- `expected_range` : La plage de cellules contenant les effectifs attendus, de même taille que les effectifs observés.

## CHITEST

`CHITEST(actual_range, expected_range)`

Renvoie la probabilité que les effectifs ne diffèrent des effectifs attendus que par hasard.

- `actual_range` : La plage de cellules contenant les effectifs observés.
- `expected_range` : La plage de cellules contenant les effectifs attendus, de même taille que les effectifs observés.

## CONFIDENCE

`CONFIDENCE(alpha, standard_dev, size)`

Renvoie la marge d'un intervalle de confiance pour une moyenne, à partir de la loi normale.

- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `standard_dev` : L'écart type de la distribution.
- `size` : La taille de l'échantillon, un nombre entier égal à 1 ou plus.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(alpha, standard_dev, size)`

Renvoie la marge d'un intervalle de confiance pour une moyenne, à partir de la loi normale.

- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `standard_dev` : L'écart type de la distribution.
- `size` : La taille de l'échantillon, un nombre entier égal à 1 ou plus.

## CONFIDENCE.T

`CONFIDENCE.T(alpha, standard_dev, size)`

Renvoie la marge d'un intervalle de confiance pour une moyenne, à partir de la loi de Student.

- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `standard_dev` : L'écart type de la distribution.
- `size` : La taille de l'échantillon, un nombre entier égal à 1 ou plus.

## CORREL

`CORREL(array1, array2)`

Renvoie le degré de variation conjointe de deux séries de nombres, de -1 à 1.

- `array1` : La première plage de cellules ou liste de valeurs.
- `array2` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.

## COUNT

`COUNT(value1, ...)`

Compte les nombres d'une liste ou d'une plage de cellules.

- `value1` : La première valeur, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## COUNTA

`COUNTA(value1, ...)`

Compte les cellules qui ne sont pas vides.

- `value1` : La première valeur, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## COUNTBLANK

`COUNTBLANK(range)`

Compte les cellules vides d'une plage.

- `range` : La plage de cellules à tester.

## COUNTIF

`COUNTIF(range, criteria)`

Compte les cellules qui répondent à une condition.

- `range` : La plage de cellules à tester.
- `criteria` : Ce qu'il faut rechercher : un nombre, un texte, ou un test tel que ">5".

## COUNTIFS

`COUNTIFS(criteria_range1, criteria1, ...)`

Compte les lignes qui répondent à toutes les conditions.

- `criteria_range1` : La première plage de cellules à tester.
- `criteria1` : Ce qu'il faut rechercher dans la première plage de cellules : un nombre, un texte, ou un test tel que ">5".
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## COVAR

`COVAR(array1, array2)`

Renvoie la covariance de deux séries de nombres.

- `array1` : La première plage de cellules ou liste de valeurs.
- `array2` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.

## COVARIANCE.P

`COVARIANCE.P(array1, array2)`

Renvoie la covariance de deux séries de nombres, pour une population.

- `array1` : La première plage de cellules ou liste de valeurs.
- `array2` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.

## COVARIANCE.S

`COVARIANCE.S(array1, array2)`

Renvoie la covariance de deux séries de nombres, pour un échantillon.

- `array1` : La première plage de cellules ou liste de valeurs.
- `array2` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.

## CRITBINOM

`CRITBINOM(trials, probability_s, alpha)`

Renvoie le plus petit nombre de succès dont la probabilité cumulée atteint une valeur donnée.

- `trials` : Le nombre d'essais.
- `probability_s` : La probabilité de succès à chaque essai, de 0 à 1.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.

## DEVSQ

`DEVSQ(number1, ...)`

Renvoie la somme des carrés des écarts des nombres par rapport à leur moyenne.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## EXPON.DIST

`EXPON.DIST(x, lambda, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe exponentielle en x.

- `x` : La valeur x à traiter.
- `lambda` : Le taux de la courbe, supérieur à 0.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## EXPONDIST

`EXPONDIST(x, lambda, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe exponentielle en x.

- `x` : La valeur x à traiter.
- `lambda` : Le taux de la courbe, supérieur à 0.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## F.DIST

`F.DIST(x, deg_freedom1, deg_freedom2, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe F en x.

- `x` : La valeur x à traiter.
- `deg_freedom1` : Le nombre de degrés de liberté du numérateur, un nombre entier égal à 1 ou plus.
- `deg_freedom2` : Le nombre de degrés de liberté du dénominateur, un nombre entier égal à 1 ou plus.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## F.DIST.RT

`F.DIST.RT(x, deg_freedom1, deg_freedom2)`

Renvoie la proportion de la courbe F au-delà de x.

- `x` : La valeur x à traiter.
- `deg_freedom1` : Le nombre de degrés de liberté du numérateur, un nombre entier égal à 1 ou plus.
- `deg_freedom2` : Le nombre de degrés de liberté du dénominateur, un nombre entier égal à 1 ou plus.

## F.INV

`F.INV(probability, deg_freedom1, deg_freedom2)`

Renvoie la valeur x correspondant à une proportion de la courbe F, comptée depuis la gauche.

- `probability` : Une probabilité, de 0 à 1.
- `deg_freedom1` : Le nombre de degrés de liberté du numérateur, un nombre entier égal à 1 ou plus.
- `deg_freedom2` : Le nombre de degrés de liberté du dénominateur, un nombre entier égal à 1 ou plus.

## F.INV.RT

`F.INV.RT(probability, deg_freedom1, deg_freedom2)`

Renvoie la valeur x correspondant à une proportion de la courbe F, comptée depuis la droite.

- `probability` : Une probabilité, de 0 à 1.
- `deg_freedom1` : Le nombre de degrés de liberté du numérateur, un nombre entier égal à 1 ou plus.
- `deg_freedom2` : Le nombre de degrés de liberté du dénominateur, un nombre entier égal à 1 ou plus.

## F.TEST

`F.TEST(array1, array2)`

Renvoie la probabilité que deux séries de nombres aient la même variance.

- `array1` : La première plage de cellules ou liste de valeurs.
- `array2` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.

## FDIST

`FDIST(x, deg_freedom1, deg_freedom2)`

Renvoie la proportion de la courbe F au-delà de x.

- `x` : La valeur x à traiter.
- `deg_freedom1` : Le nombre de degrés de liberté du numérateur, un nombre entier égal à 1 ou plus.
- `deg_freedom2` : Le nombre de degrés de liberté du dénominateur, un nombre entier égal à 1 ou plus.

## FINV

`FINV(probability, deg_freedom1, deg_freedom2)`

Renvoie la valeur x correspondant à une proportion de la courbe F, comptée depuis la droite.

- `probability` : Une probabilité, de 0 à 1.
- `deg_freedom1` : Le nombre de degrés de liberté du numérateur, un nombre entier égal à 1 ou plus.
- `deg_freedom2` : Le nombre de degrés de liberté du dénominateur, un nombre entier égal à 1 ou plus.

## FISHER

`FISHER(x)`

Renvoie la transformation de Fisher d'un nombre compris entre -1 et 1.

- `x` : La valeur x à traiter.

## FISHERINV

`FISHERINV(y)`

Renvoie l'inverse de la transformation de Fisher.

- `y` : Le nombre dont vous voulez trouver l'inverse de la transformation de Fisher.

## FORECAST

`FORECAST(x, known_y's, known_x's)`

Prévoit une valeur sur la droite d'ajustement optimal.

- `x` : La valeur x pour laquelle prévoir une valeur y.
- `known_y's` : La plage de cellules qui contient les valeurs y dont vous disposez déjà.
- `known_x's` : La plage de cellules qui contient les valeurs x dont vous disposez déjà.

## FORECAST.ETS

`FORECAST.ETS(target_date, values, timeline, [seasonality], [data_completion], [aggregation])`

Prévoit une valeur future d'une série qui présente une tendance et une saisonnalité, par lissage exponentiel.

- `target_date` : Le moment pour lequel prévoir une valeur.
- `values` : Les valeurs dont vous disposez déjà.
- `timeline` : Les dates ou nombres des valeurs, à intervalles réguliers.
- `seasonality` : 1 pour détecter la saisonnalité à partir des valeurs, 0 pour aucune, ou la durée de la saison.
- `data_completion` : 1 pour combler un point manquant par une droite entre ses voisins, 0 pour le combler avec zéro.
- `aggregation` : Mode de combinaison des valeurs de même date : 1 moyenne, 2 nombre, 3 nbval, 4 max, 5 médiane, 6 min, 7 somme.

## FORECAST.ETS.CONFINT

`FORECAST.ETS.CONFINT(target_date, values, timeline, [confidence_level], [seasonality], [data_completion], [aggregation])`

Renvoie l'écart au-dessus et en dessous d'une prévision lissée dans lequel la valeur est susceptible de se trouver.

- `confidence_level` : Un nombre supérieur à 0 et inférieur à 1, 0,95 si l'argument est omis.

## FORECAST.ETS.SEASONALITY

`FORECAST.ETS.SEASONALITY(values, timeline, [data_completion], [aggregation])`

Renvoie la durée de la saison qui se répète dans une série, ou 1 s'il n'y en a pas.

## FORECAST.ETS.STAT

`FORECAST.ETS.STAT(values, timeline, statistic_type, [seasonality], [data_completion], [aggregation])`

Renvoie l'une des constantes de lissage ou des mesures d'ajustement d'une prévision lissée.

- `statistic_type` : 1 Alpha, 2 Bêta, 3 Gamma, 4 MASE, 5 SMAPE, 6 MAE, 7 RMSE, 8 le pas de la chronologie.

## FORECAST.LINEAR

`FORECAST.LINEAR(x, known_y's, known_x's)`

Prévoit une valeur sur la droite d'ajustement optimal.

- `x` : La valeur x à traiter.
- `known_y's` : La plage de cellules qui contient les valeurs y dont vous disposez déjà.
- `known_x's` : La plage de cellules qui contient les valeurs x dont vous disposez déjà.

## FREQUENCY

`FREQUENCY(data_array, bins_array)`

Compte combien de valeurs tombent dans chaque intervalle, et affiche les effectifs sous forme de colonne.

- `data_array` : La plage de cellules contenant les valeurs à compter.
- `bins_array` : La plage de cellules contenant les limites des intervalles de comptage.

## FTEST

`FTEST(array1, array2)`

Renvoie la probabilité que deux séries de nombres aient la même variance.

- `array1` : La première plage de cellules ou liste de valeurs.
- `array2` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.

## GAMMA

`GAMMA(x)`

Renvoie la fonction gamma d'un nombre.

- `x` : La valeur x à traiter.

## GAMMA.DIST

`GAMMA.DIST(x, alpha, beta, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe gamma en x.

- `x` : La valeur x à traiter.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `beta` : Un nombre qui façonne la courbe (l'échelle de la courbe pour les courbes gamma et de Weibull).
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## GAMMA.INV

`GAMMA.INV(probability, alpha, beta)`

Renvoie la valeur x correspondant à une proportion de la courbe gamma.

- `probability` : Une probabilité, de 0 à 1.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `beta` : Un nombre qui façonne la courbe (l'échelle de la courbe pour les courbes gamma et de Weibull).

## GAMMADIST

`GAMMADIST(x, alpha, beta, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe gamma en x.

- `x` : La valeur x à traiter.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `beta` : Un nombre qui façonne la courbe (l'échelle de la courbe pour les courbes gamma et de Weibull).
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## GAMMAINV

`GAMMAINV(probability, alpha, beta)`

Renvoie la valeur x correspondant à une proportion de la courbe gamma.

- `probability` : Une probabilité, de 0 à 1.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `beta` : Un nombre qui façonne la courbe (l'échelle de la courbe pour les courbes gamma et de Weibull).

## GAMMALN

`GAMMALN(x)`

Renvoie le logarithme népérien de la fonction gamma.

- `x` : La valeur x à traiter.

## GAMMALN.PRECISE

`GAMMALN.PRECISE(x)`

Renvoie le logarithme népérien de la fonction gamma.

- `x` : La valeur x à traiter.

## GAUSS

`GAUSS(z)`

Renvoie la proportion de la courbe normale centrée réduite comprise entre la moyenne et z.

- `z` : La valeur z.

## GEOMEAN

`GEOMEAN(number1, ...)`

Renvoie la moyenne géométrique de nombres positifs.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## GROWTH

`GROWTH(known_y's, [known_x's], [new_x's], [const])`

Renvoie les valeurs de la courbe exponentielle d'ajustement optimal pour de nouvelles valeurs x.

- `known_y's` : La plage de cellules qui contient les valeurs y dont vous disposez déjà.
- `known_x's` (facultatif) : La plage de cellules qui contient les valeurs x dont vous disposez déjà.
- `new_x's` (facultatif) : Les valeurs x pour lesquelles calculer les valeurs y (les valeurs x connues si l'argument est omis).
- `const` (facultatif) : VRAI ou omis pour ajuster une droite qui ne doit pas nécessairement passer par zéro ; FAUX pour la faire passer par zéro.

## HARMEAN

`HARMEAN(number1, ...)`

Renvoie la moyenne harmonique de nombres positifs.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## HYPGEOM.DIST

`HYPGEOM.DIST(sample_s, number_sample, population_s, number_pop, cumulative)`

Renvoie la probabilité de tirer un nombre de succès sans remise des éléments.

- `sample_s` : Le nombre de succès dans l'échantillon.
- `number_sample` : Le nombre d'éléments tirés.
- `population_s` : Le nombre de succès dans la population totale.
- `number_pop` : Le nombre d'éléments de la population totale.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## HYPGEOMDIST

`HYPGEOMDIST(sample_s, number_sample, population_s, number_pop)`

Renvoie la probabilité de tirer un nombre de succès sans remise des éléments.

- `sample_s` : Le nombre de succès dans l'échantillon.
- `number_sample` : Le nombre d'éléments tirés.
- `population_s` : Le nombre de succès dans la population totale.
- `number_pop` : Le nombre d'éléments de la population totale.

## INTERCEPT

`INTERCEPT(known_y's, known_x's)`

Renvoie le point où la droite d'ajustement optimal coupe l'axe vertical.

- `known_y's` : La plage de cellules qui contient les valeurs y dont vous disposez déjà.
- `known_x's` : La plage de cellules qui contient les valeurs x dont vous disposez déjà.

## KURT

`KURT(number1, ...)`

Renvoie l'aplatissement d'une série de nombres par rapport à une loi normale.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## LARGE

`LARGE(array, k)`

Renvoie la k-ième plus grande valeur d'une série.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `k` : Le rang à renvoyer, en comptant depuis la plus grande valeur : 1 correspond à la plus grande.

## LINEST

`LINEST(known_y's, [known_x's], [const], [stats])`

Ajuste une droite ou un plan aux données par la méthode des moindres carrés, et renvoie ses valeurs sous forme de ligne.

- `known_y's` : La plage de cellules qui contient les valeurs y dont vous disposez déjà.
- `known_x's` (facultatif) : La plage de cellules qui contient les valeurs x dont vous disposez déjà.
- `const` (facultatif) : VRAI ou omis pour ajuster une droite qui ne doit pas nécessairement passer par zéro ; FAUX pour la faire passer par zéro.
- `stats` (facultatif) : VRAI pour obtenir en plus d'autres valeurs sur l'ajustement ; FAUX ou omis pour obtenir seulement la droite.

## LOGEST

`LOGEST(known_y's, [known_x's], [const], [stats])`

Ajuste une courbe exponentielle aux données, et renvoie ses valeurs sous forme de ligne.

- `known_y's` : La plage de cellules qui contient les valeurs y dont vous disposez déjà.
- `known_x's` (facultatif) : La plage de cellules qui contient les valeurs x dont vous disposez déjà.
- `const` (facultatif) : VRAI ou omis pour ajuster une courbe qui ne doit pas nécessairement passer par zéro ; FAUX pour la faire passer par zéro.
- `stats` (facultatif) : VRAI pour obtenir en plus d'autres valeurs sur l'ajustement ; FAUX ou omis pour obtenir seulement la courbe.

## LOGINV

`LOGINV(probability, mean, standard_dev)`

Renvoie la valeur x correspondant à une proportion de la courbe log-normale.

- `probability` : Une probabilité, de 0 à 1.
- `mean` : La moyenne de la distribution.
- `standard_dev` : L'écart type de la distribution.

## LOGNORM.DIST

`LOGNORM.DIST(x, mean, standard_dev, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe log-normale en x.

- `x` : La valeur x à traiter.
- `mean` : La moyenne de la distribution.
- `standard_dev` : L'écart type de la distribution.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## LOGNORM.INV

`LOGNORM.INV(probability, mean, standard_dev)`

Renvoie la valeur x correspondant à une proportion de la courbe log-normale.

- `probability` : Une probabilité, de 0 à 1.
- `mean` : La moyenne de la distribution.
- `standard_dev` : L'écart type de la distribution.

## LOGNORMDIST

`LOGNORMDIST(x, mean, standard_dev)`

Renvoie la proportion cumulée de la courbe log-normale en x.

- `x` : La valeur x à traiter.
- `mean` : La moyenne de la distribution.
- `standard_dev` : L'écart type de la distribution.

## MAXA

`MAXA(value1, ...)`

Renvoie la plus grande valeur, en comptant le texte pour zéro et VRAI pour un.

- `value1` : La première valeur, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## MAXIFS

`MAXIFS(max_range, criteria_range1, criteria1, ...)`

Renvoie le plus grand nombre parmi les cellules qui répondent à plusieurs conditions.

- `max_range` : Les cellules dont prendre la plus grande valeur.
- `criteria_range1` : La première plage de cellules à tester.
- `criteria1` : Ce qu'il faut rechercher dans la première plage de cellules : un nombre, un texte, ou un test tel que ">5".
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## MEDIAN

`MEDIAN(number1, ...)`

Renvoie la valeur médiane d'une série.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## MINA

`MINA(value1, ...)`

Renvoie la plus petite valeur, en comptant le texte pour zéro et VRAI pour un.

- `value1` : La première valeur, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## MINIFS

`MINIFS(min_range, criteria_range1, criteria1, ...)`

Renvoie le plus petit nombre parmi les cellules qui répondent à plusieurs conditions.

- `min_range` : Les cellules dont prendre la plus petite valeur.
- `criteria_range1` : La première plage de cellules à tester.
- `criteria1` : Ce qu'il faut rechercher dans la première plage de cellules : un nombre, un texte, ou un test tel que ">5".
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## MODE

`MODE(number1, ...)`

Renvoie la valeur la plus fréquente.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## MODE.MULT

`MODE.MULT(number1, ...)`

Renvoie une colonne contenant les nombres les plus fréquents d'une série.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## MODE.SNGL

`MODE.SNGL(number1, ...)`

Renvoie le nombre le plus fréquent d'une série.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## NEGBINOM.DIST

`NEGBINOM.DIST(number_f, number_s, probability_s, cumulative)`

Renvoie la probabilité d'un nombre d'échecs avant un nombre de succès.

- `number_f` : Le nombre d'échecs.
- `number_s` : Le nombre de succès.
- `probability_s` : La probabilité de succès à chaque essai, de 0 à 1.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## NEGBINOMDIST

`NEGBINOMDIST(number_f, number_s, probability_s)`

Renvoie la probabilité d'un nombre d'échecs avant un nombre de succès.

- `number_f` : Le nombre d'échecs.
- `number_s` : Le nombre de succès.
- `probability_s` : La probabilité de succès à chaque essai, de 0 à 1.

## NORM.DIST

`NORM.DIST(x, mean, standard_dev, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe normale en x.

- `x` : La valeur x à traiter.
- `mean` : La moyenne de la distribution.
- `standard_dev` : L'écart type de la distribution.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## NORM.INV

`NORM.INV(probability, mean, standard_dev)`

Renvoie la valeur x correspondant à une proportion de la courbe normale.

- `probability` : Une probabilité, de 0 à 1.
- `mean` : La moyenne de la distribution.
- `standard_dev` : L'écart type de la distribution.

## NORM.S.DIST

`NORM.S.DIST(z, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe normale centrée réduite en z.

- `z` : La valeur z.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## NORM.S.INV

`NORM.S.INV(probability)`

Renvoie la valeur z correspondant à une proportion de la courbe normale centrée réduite.

- `probability` : Une probabilité, de 0 à 1.

## NORMDIST

`NORMDIST(x, mean, standard_dev, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe normale en x.

- `x` : La valeur x à traiter.
- `mean` : La moyenne de la distribution.
- `standard_dev` : L'écart type de la distribution.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## NORMINV

`NORMINV(probability, mean, standard_dev)`

Renvoie la valeur x correspondant à une proportion de la courbe normale.

- `probability` : Une probabilité, de 0 à 1.
- `mean` : La moyenne de la distribution.
- `standard_dev` : L'écart type de la distribution.

## NORMSDIST

`NORMSDIST(z)`

Renvoie la proportion cumulée de la courbe normale centrée réduite jusqu'à z.

- `z` : La valeur z.

## NORMSINV

`NORMSINV(probability)`

Renvoie la valeur z correspondant à une proportion de la courbe normale centrée réduite.

- `probability` : Une probabilité, de 0 à 1.

## PEARSON

`PEARSON(array1, array2)`

Renvoie le degré de variation conjointe de deux séries de nombres, de -1 à 1.

- `array1` : La première plage de cellules ou liste de valeurs.
- `array2` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.

## PERCENTILE

`PERCENTILE(array, k)`

Renvoie la valeur en dessous de laquelle se situe une proportion k des nombres.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `k` : La proportion des valeurs, de 0 à 1, dont renvoyer la valeur.

## PERCENTILE.EXC

`PERCENTILE.EXC(array, k)`

Renvoie la valeur en dessous de laquelle se situe une proportion k des nombres, à l'exclusion de 0 et 1.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `k` : Le rang dans l'ordre, en comptant à partir de 1.

## PERCENTILE.INC

`PERCENTILE.INC(array, k)`

Renvoie la valeur en dessous de laquelle se situe une proportion k des nombres, de 0 à 1.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `k` : Le rang dans l'ordre, en comptant à partir de 1.

## PERCENTRANK

`PERCENTRANK(array, x, [significance])`

Renvoie le rang d'une valeur dans une série de nombres, sous forme de proportion de 0 à 1.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `x` : La valeur x à traiter.
- `significance` (facultatif) : Le nombre dont les multiples servent à l'arrondi.

## PERCENTRANK.EXC

`PERCENTRANK.EXC(array, x, [significance])`

Renvoie le rang d'une valeur dans une série de nombres, sous forme de proportion, à l'exclusion de 0 et 1.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `x` : La valeur x à traiter.
- `significance` (facultatif) : Le nombre dont les multiples servent à l'arrondi.

## PERCENTRANK.INC

`PERCENTRANK.INC(array, x, [significance])`

Renvoie le rang d'une valeur dans une série de nombres, sous forme de proportion de 0 à 1.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `x` : La valeur x à traiter.
- `significance` (facultatif) : Le nombre dont les multiples servent à l'arrondi.

## PERMUTATIONA

`PERMUTATIONA(number, number_chosen)`

Renvoie le nombre de choix ordonnés lorsqu'un élément peut être choisi plusieurs fois.

- `number` : Le nombre à traiter.
- `number_chosen` : Le nombre d'éléments à choisir dans chaque groupe.

## PHI

`PHI(x)`

Renvoie la hauteur de la courbe normale centrée réduite en x.

- `x` : La valeur x à traiter.

## POISSON

`POISSON(x, mean, cumulative)`

Renvoie la probabilité d'un nombre d'événements, à partir du nombre attendu.

- `x` : La valeur x à traiter.
- `mean` : La moyenne de la distribution.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## POISSON.DIST

`POISSON.DIST(x, mean, cumulative)`

Renvoie la probabilité d'un nombre d'événements, à partir du nombre attendu.

- `x` : La valeur x à traiter.
- `mean` : La moyenne de la distribution.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## PROB

`PROB(x_range, prob_range, lower_limit, [upper_limit])`

Renvoie la probabilité que des valeurs se situent entre deux limites.

- `x_range` : La plage de cellules contenant les valeurs x.
- `prob_range` : Les probabilités correspondant à chaque valeur x, comprises chacune entre 0 et 1, et dont la somme vaut 1.
- `lower_limit` : La limite inférieure de l'intervalle.
- `upper_limit` (facultatif) : La limite supérieure de l'intervalle (la limite inférieure seule si l'argument est omis).

## QUARTILE

`QUARTILE(array, quart)`

Renvoie les quartiles d'une série de nombres.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `quart` : Le quartile demandé : 0 pour le plus petit, 1 pour le premier quartile, 2 pour la médiane, 3 pour le troisième quartile, 4 pour le plus grand.

## QUARTILE.EXC

`QUARTILE.EXC(array, quart)`

Renvoie les quartiles d'une série de nombres, en excluant les extrémités.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `quart` : Le quartile demandé : 0 pour le plus petit, 1 pour le premier quartile, 2 pour la médiane, 3 pour le troisième quartile, 4 pour le plus grand.

## QUARTILE.INC

`QUARTILE.INC(array, quart)`

Renvoie les quartiles d'une série de nombres, du plus petit au plus grand.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `quart` : Le quartile demandé : 0 pour le plus petit, 1 pour le premier quartile, 2 pour la médiane, 3 pour le troisième quartile, 4 pour le plus grand.

## RANK

`RANK(number, ref, [order])`

Renvoie le rang d'un nombre dans une liste triée.

- `number` : Le nombre à traiter.
- `ref` : Une cellule ou une plage de cellules.
- `order` (facultatif) : 0 ou omis classe la plus grande valeur en premier ; tout autre nombre classe la plus petite en premier.

## RANK.AVG

`RANK.AVG(number, ref, [order])`

Renvoie le rang d'un nombre dans une liste, et le rang moyen lorsque des valeurs sont égales.

- `number` : Le nombre à traiter.
- `ref` : Une cellule ou une plage de cellules.
- `order` (facultatif) : 0 ou omis classe la plus grande valeur en premier ; tout autre nombre classe la plus petite en premier.

## RANK.EQ

`RANK.EQ(number, ref, [order])`

Renvoie le rang d'un nombre dans une liste, et le rang le plus élevé lorsque des valeurs sont égales.

- `number` : Le nombre à traiter.
- `ref` : Une cellule ou une plage de cellules.
- `order` (facultatif) : 0 ou omis classe la plus grande valeur en premier ; tout autre nombre classe la plus petite en premier.

## RSQ

`RSQ(known_y's, known_x's)`

Indique dans quelle mesure une droite s'ajuste à deux séries de nombres.

- `known_y's` : La plage de cellules qui contient les valeurs y dont vous disposez déjà.
- `known_x's` : La plage de cellules qui contient les valeurs x dont vous disposez déjà.

## SKEW

`SKEW(number1, ...)`

Renvoie le degré de dissymétrie d'une série de nombres autour de sa moyenne, pour un échantillon.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## SKEW.P

`SKEW.P(number1, ...)`

Renvoie le degré de dissymétrie d'une série de nombres autour de sa moyenne, pour une population entière.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## SLOPE

`SLOPE(known_y's, known_x's)`

Renvoie la pente de la droite qui s'ajuste le mieux à deux séries de nombres.

- `known_y's` : La plage de cellules qui contient les valeurs y dont vous disposez déjà.
- `known_x's` : La plage de cellules qui contient les valeurs x dont vous disposez déjà.

## SMALL

`SMALL(array, k)`

Renvoie la k-ième plus petite valeur d'une série.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `k` : Le rang à renvoyer, en comptant depuis la plus petite valeur : 1 correspond à la plus petite.

## STANDARDIZE

`STANDARDIZE(x, mean, standard_dev)`

Renvoie le nombre d'écarts types qui séparent une valeur de la moyenne.

- `x` : La valeur x à traiter.
- `mean` : La moyenne de la distribution.
- `standard_dev` : L'écart type de la distribution.

## STDEV

`STDEV(number1, ...)`

Estime l'écart type d'une population entière à partir d'un échantillon.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## STDEV.P

`STDEV.P(number1, ...)`

Renvoie l'écart type d'une population entière.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## STDEV.S

`STDEV.S(number1, ...)`

Estime l'écart type d'une population entière à partir d'un échantillon.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## STDEVA

`STDEVA(value1, ...)`

Renvoie l'écart type d'un échantillon, en comptant le texte pour zéro et VRAI pour un.

- `value1` : La première valeur, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## STDEVP

`STDEVP(number1, ...)`

Renvoie l'écart type d'une population entière.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## STDEVPA

`STDEVPA(value1, ...)`

Renvoie l'écart type d'une population, en comptant le texte pour zéro et VRAI pour un.

- `value1` : La première valeur, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## STEYX

`STEYX(known_y's, known_x's)`

Renvoie l'erreur type des valeurs y prévues par une droite d'ajustement.

- `known_y's` : La plage de cellules qui contient les valeurs y dont vous disposez déjà.
- `known_x's` : La plage de cellules qui contient les valeurs x dont vous disposez déjà.

## SUMIF

`SUMIF(range, criteria, [sum_range])`

Additionne les cellules qui répondent à une condition.

- `range` : La plage de cellules à tester.
- `criteria` : Ce qu'il faut rechercher : un nombre, un texte, ou un test tel que ">5".
- `sum_range` (facultatif) : Les cellules à additionner (celles de la plage testée si l'argument est omis).

## SUMIFS

`SUMIFS(sum_range, criteria_range1, criteria1, ...)`

Additionne les cellules qui répondent à plusieurs conditions.

- `sum_range` : Les cellules à additionner (celles de la plage testée si l'argument est omis).
- `criteria_range1` : La première plage de cellules à tester.
- `criteria1` : Ce qu'il faut rechercher dans la première plage de cellules : un nombre, un texte, ou un test tel que ">5".
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## T.DIST

`T.DIST(x, deg_freedom, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe de Student en x.

- `x` : La valeur x à traiter.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## T.DIST.2T

`T.DIST.2T(x, deg_freedom)`

Renvoie la proportion de la courbe de Student située en dehors de -x et de x.

- `x` : La valeur x à traiter.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.

## T.DIST.RT

`T.DIST.RT(x, deg_freedom)`

Renvoie la proportion de la courbe de Student au-delà de x.

- `x` : La valeur x à traiter.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.

## T.INV

`T.INV(probability, deg_freedom)`

Renvoie la valeur x correspondant à une proportion de la courbe de Student, comptée depuis la gauche.

- `probability` : Une probabilité, de 0 à 1.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.

## T.INV.2T

`T.INV.2T(probability, deg_freedom)`

Renvoie la valeur x correspondant à une proportion de la courbe de Student, répartie entre les deux queues.

- `probability` : Une probabilité, de 0 à 1.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.

## T.TEST

`T.TEST(array1, array2, tails, type)`

Renvoie la probabilité que les moyennes de deux séries de nombres soient identiques.

- `array1` : La première plage de cellules ou liste de valeurs.
- `array2` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.
- `tails` : 1 pour une queue de la courbe, 2 pour les deux.
- `type` : Moment des paiements : 0 ou omis à la fin de chaque période, 1 au début.

## TDIST

`TDIST(x, deg_freedom, tails)`

Renvoie la proportion de la courbe de Student dans une queue ou dans les deux.

- `x` : La valeur x à traiter.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.
- `tails` : 1 pour une queue de la courbe, 2 pour les deux.

## TINV

`TINV(probability, deg_freedom)`

Renvoie la valeur x correspondant à une proportion de la courbe de Student, répartie entre les deux queues.

- `probability` : Une probabilité, de 0 à 1.
- `deg_freedom` : Le nombre de degrés de liberté, un nombre entier égal à 1 ou plus.

## TREND

`TREND(known_y's, [known_x's], [new_x's], [const])`

Renvoie les valeurs de la droite d'ajustement optimal pour de nouvelles valeurs x.

- `known_y's` : La plage de cellules qui contient les valeurs y dont vous disposez déjà.
- `known_x's` (facultatif) : La plage de cellules qui contient les valeurs x dont vous disposez déjà.
- `new_x's` (facultatif) : Les valeurs x pour lesquelles calculer les valeurs y (les valeurs x connues si l'argument est omis).
- `const` (facultatif) : VRAI ou omis pour ajuster une droite qui ne doit pas nécessairement passer par zéro ; FAUX pour la faire passer par zéro.

## TRIMMEAN

`TRIMMEAN(array, percent)`

Renvoie la moyenne d'une série de nombres après avoir retiré une proportion des valeurs les plus hautes et les plus basses.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `percent` : La proportion des valeurs, de 0 à moins de 1, à exclure aux deux extrémités.

## TTEST

`TTEST(array1, array2, tails, type)`

Renvoie la probabilité que les moyennes de deux séries de nombres soient identiques.

- `array1` : La première plage de cellules ou liste de valeurs.
- `array2` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.
- `tails` : 1 pour une queue de la courbe, 2 pour les deux.
- `type` : Moment des paiements : 0 ou omis à la fin de chaque période, 1 au début.

## VAR

`VAR(number1, ...)`

Estime la variance d'une population entière à partir d'un échantillon.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## VAR.P

`VAR.P(number1, ...)`

Renvoie la variance d'une population entière.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## VAR.S

`VAR.S(number1, ...)`

Estime la variance d'une population entière à partir d'un échantillon.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## VARA

`VARA(value1, ...)`

Renvoie la variance d'un échantillon, en comptant le texte pour zéro et VRAI pour un.

- `value1` : La première valeur, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## VARP

`VARP(number1, ...)`

Renvoie la variance d'une population entière.

- `number1` : Le premier nombre, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## VARPA

`VARPA(value1, ...)`

Renvoie la variance d'une population, en comptant le texte pour zéro et VRAI pour un.

- `value1` : La première valeur, ou la plage de cellules.
- `...` : Autant d'autres arguments de même nature que le précédent que nécessaire.

## WEIBULL

`WEIBULL(x, alpha, beta, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe de Weibull en x.

- `x` : La valeur x à traiter.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `beta` : Un nombre qui façonne la courbe (l'échelle de la courbe pour les courbes gamma et de Weibull).
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## WEIBULL.DIST

`WEIBULL.DIST(x, alpha, beta, cumulative)`

Renvoie la valeur ou la proportion cumulée de la courbe de Weibull en x.

- `x` : La valeur x à traiter.
- `alpha` : Le risque d'erreur que vous acceptez, par exemple 0,05 pour un niveau de confiance de 95 %.
- `beta` : Un nombre qui façonne la courbe (l'échelle de la courbe pour les courbes gamma et de Weibull).
- `cumulative` : VRAI pour obtenir le total cumulé jusqu'à ce point ; FAUX pour obtenir la valeur en ce point seul.

## Z.TEST

`Z.TEST(array, x, [sigma])`

Renvoie la probabilité unilatérale que la moyenne d'un échantillon soit supérieure à une valeur.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `x` : La valeur x à traiter.
- `sigma` (facultatif) : L'écart type de la population entière (celui de l'échantillon si l'argument est omis).

## ZTEST

`ZTEST(array, x, [sigma])`

Renvoie la probabilité unilatérale que la moyenne d'un échantillon soit supérieure à une valeur.

- `array` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `x` : La valeur x à traiter.
- `sigma` (facultatif) : L'écart type de la population entière (celui de l'échantillon si l'argument est omis).

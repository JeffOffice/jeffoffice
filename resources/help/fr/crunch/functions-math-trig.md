# Fonctions mathématiques et trigonométriques
<!-- keywords: fonctions, ABS, ACOS, ACOSH, ACOT, ACOTH, AGGREGATE, ARABIC, ASIN, ASINH, ATAN, ATAN2, ATANH, BASE, CEILING, CEILING.MATH, CEILING.PRECISE, COMBIN, COMBINA, COS, COSH, COT, COTH, CSC, CSCH, DECIMAL, DEGREES, EVEN, EXP, FACT, FACTDOUBLE, FLOOR, FLOOR.MATH, FLOOR.PRECISE, GCD, INT, ISO.CEILING, LCM, LN, LOG, LOG10, MAX, MDETERM, MIN, MINVERSE, MMULT, MOD, MROUND, MULTINOMIAL, MUNIT, ODD, PERMUT, PI, POWER, PRODUCT, QUOTIENT, RADIANS, RAND, RANDARRAY, RANDBETWEEN, ROMAN, ROUND, ROUNDDOWN, ROUNDUP, SEC, SECH, SEQUENCE, SERIESSUM, SIGN, SIN, SINH, SQRT, SQRTPI, SUBTOTAL, SUM, SUMPRODUCT, SUMSQ, SUMX2MY2, SUMX2PY2, SUMXMY2, TAN, TANH, TRUNC -->

Voici les fonctions mathématiques et trigonométriques que JeffCrunch peut calculer. Chacune indique sa syntaxe, ce qu'elle fait et le rôle de chacun de ses arguments. Un argument entre crochets est facultatif, et « ... » signifie que vous pouvez ajouter autant d'arguments supplémentaires que nécessaire après le dernier. Pour essayer l'une d'elles, reportez-vous à [Choisir une fonction](insert-function). Pendant la saisie d'une fonction dans une cellule, une petite info-bulle sous la cellule affiche ses arguments, et un clic sur le nom de la fonction dans cette info-bulle ouvre sa fiche ici.

## ABS

`ABS(nombre)`

Renvoie un nombre sans son signe.

- `nombre` : Le nombre à traiter.

## ACOS

`ACOS(nombre)`

Renvoie l'angle, en radians, dont le cosinus correspond à un nombre donné.

- `nombre` : Le nombre à traiter.

## ACOSH

`ACOSH(nombre)`

Renvoie le cosinus hyperbolique inverse d'un nombre.

- `nombre` : Le nombre à traiter.

## ACOT

`ACOT(nombre)`

Renvoie la cotangente inverse d'un nombre, sous forme d'angle en radians.

- `nombre` : Le nombre à traiter.

## ACOTH

`ACOTH(nombre)`

Renvoie la cotangente hyperbolique inverse d'un nombre.

- `nombre` : Le nombre à traiter.

## AGGREGATE

`AGGREGATE(no_fonction, options, réf1, ...)`

Effectue un calcul sur une liste, comme une somme ou une médiane, et peut ignorer les lignes masquées et les erreurs.

- `no_fonction` : Le calcul à effectuer : 1 moyenne, 2 nombre, 3 nombre de valeurs non vides, 4 maximum, 5 minimum, 6 produit, 7 et 8 écart type, 9 somme, 10 et 11 variance.
- `options` : Ce qu'il faut ignorer : 0 totaux imbriqués, 1 également les lignes masquées, 2 également les erreurs, 3 les deux, 4 à 7 pour conserver les totaux imbriqués.
- `réf1` : La première cellule ou plage de cellules.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## ARABIC

`ARABIC(texte)`

Lit des chiffres romains dans un texte et donne le nombre correspondant.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## ASIN

`ASIN(nombre)`

Renvoie l'angle, en radians, dont le sinus correspond à un nombre donné.

- `nombre` : Le nombre à traiter.

## ASINH

`ASINH(nombre)`

Renvoie le sinus hyperbolique inverse d'un nombre.

- `nombre` : Le nombre à traiter.

## ATAN

`ATAN(nombre)`

Renvoie l'angle, en radians, dont la tangente correspond à un nombre donné.

- `nombre` : Le nombre à traiter.

## ATAN2

`ATAN2(x_num, y_num)`

Renvoie l'angle, en radians, du point de coordonnées x et y.

- `x_num` : La valeur de x.
- `y_num` : La valeur de y.

## ATANH

`ATANH(nombre)`

Renvoie la tangente hyperbolique inverse d'un nombre.

- `nombre` : Le nombre à traiter.

## BASE

`BASE(nombre, base, [longueur_min])`

Écrit un nombre dans une autre base, de 2 à 36, sous forme de texte.

- `nombre` : Le nombre à traiter.
- `base` : La base du nombre : de 2 à 36.
- `longueur_min` (facultatif) : Le nombre minimal de caractères du résultat ; les résultats plus courts sont complétés par des zéros à gauche.

## CEILING

`CEILING(nombre, [précision])`

Arrondit un nombre au multiple supérieur le plus proche.

- `nombre` : Le nombre à traiter.
- `précision` (facultatif) : Le nombre dont les multiples servent à l'arrondi.

## CEILING.MATH

`CEILING.MATH(nombre, [précision], [mode])`

Arrondit un nombre au multiple supérieur le plus proche, avec un choix pour les nombres négatifs.

- `nombre` : Le nombre à traiter.
- `précision` (facultatif) : Le nombre dont les multiples servent à l'arrondi.
- `mode` (facultatif) : La manière d'arrondir les nombres négatifs : 0 ou omis pour le cas habituel, toute autre valeur l'inverse.

## CEILING.PRECISE

`CEILING.PRECISE(nombre, [précision])`

Arrondit un nombre au multiple supérieur le plus proche, quel que soit le signe du multiple.

- `nombre` : Le nombre à traiter.
- `précision` (facultatif) : Le nombre dont les multiples servent à l'arrondi.

## COMBIN

`COMBIN(nombre, nombre_choisi)`

Compte les façons de choisir des éléments dans un groupe lorsque l'ordre n'a pas d'importance.

- `nombre` : Le nombre à traiter.
- `nombre_choisi` : Le nombre d'éléments à prélever dans chaque groupe.

## COMBINA

`COMBINA(nombre, nombre_choisi)`

Donne le nombre de façons de choisir des éléments lorsqu'un élément peut être choisi plusieurs fois.

- `nombre` : Le nombre à traiter.
- `nombre_choisi` : Le nombre d'éléments à prélever dans chaque groupe.

## COS

`COS(nombre)`

Renvoie le cosinus d'un angle exprimé en radians.

- `nombre` : Le nombre à traiter.

## COSH

`COSH(nombre)`

Renvoie le cosinus hyperbolique d'un nombre.

- `nombre` : Le nombre à traiter.

## COT

`COT(angle)`

Renvoie la cotangente d'un angle exprimé en radians.

- `angle` : L'angle, en radians.

## COTH

`COTH(nombre)`

Renvoie la cotangente hyperbolique d'un nombre.

- `nombre` : Le nombre à traiter.

## CSC

`CSC(angle)`

Renvoie la cosécante d'un angle exprimé en radians.

- `angle` : L'angle, en radians.

## CSCH

`CSCH(nombre)`

Renvoie la cosécante hyperbolique d'un nombre.

- `nombre` : Le nombre à traiter.

## DECIMAL

`DECIMAL(texte, base)`

Lit un texte écrit dans une base de 2 à 36 et le convertit en nombre.

- `texte` : Le texte à traiter, ou une cellule qui le contient.
- `base` : La base du nombre : de 2 à 36.

## DEGREES

`DEGREES(angle)`

Convertit des radians en degrés.

- `angle` : L'angle, en radians.

## EVEN

`EVEN(nombre)`

Arrondit un nombre au nombre entier pair supérieur le plus proche.

- `nombre` : Le nombre à traiter.

## EXP

`EXP(nombre)`

Renvoie e élevé à une puissance.

- `nombre` : Le nombre à traiter.

## FACT

`FACT(nombre)`

Renvoie la factorielle d'un nombre.

- `nombre` : Le nombre à traiter.

## FACTDOUBLE

`FACTDOUBLE(nombre)`

Renvoie la double factorielle d'un nombre : le produit de chaque deuxième nombre en descendant jusqu'à 1 ou 2.

- `nombre` : Le nombre à traiter.

## FLOOR

`FLOOR(nombre, [précision])`

Arrondit un nombre au multiple inférieur le plus proche.

- `nombre` : Le nombre à traiter.
- `précision` (facultatif) : Le nombre dont les multiples servent à l'arrondi.

## FLOOR.MATH

`FLOOR.MATH(nombre, [précision], [mode])`

Arrondit un nombre au multiple inférieur le plus proche, avec un choix pour les nombres négatifs.

- `nombre` : Le nombre à traiter.
- `précision` (facultatif) : Le nombre dont les multiples servent à l'arrondi.
- `mode` (facultatif) : La manière d'arrondir les nombres négatifs : 0 ou omis pour le cas habituel, toute autre valeur l'inverse.

## FLOOR.PRECISE

`FLOOR.PRECISE(nombre, [précision])`

Arrondit un nombre au multiple inférieur le plus proche, quel que soit le signe du multiple.

- `nombre` : Le nombre à traiter.
- `précision` (facultatif) : Le nombre dont les multiples servent à l'arrondi.

## GCD

`GCD(nombre1, ...)`

Renvoie le plus grand nombre entier qui divise tous les nombres.

- `nombre1` : Le premier nombre, ou la première plage de cellules.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## INT

`INT(nombre)`

Arrondit un nombre à l'entier inférieur le plus proche.

- `nombre` : Le nombre à traiter.

## ISO.CEILING

`ISO.CEILING(nombre, [précision])`

Arrondit un nombre au multiple supérieur le plus proche, quel que soit le signe du multiple.

- `nombre` : Le nombre à traiter.
- `précision` (facultatif) : Le nombre dont les multiples servent à l'arrondi.

## LCM

`LCM(nombre1, ...)`

Renvoie le plus petit nombre entier que tous les nombres divisent.

- `nombre1` : Le premier nombre, ou la première plage de cellules.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## LN

`LN(nombre)`

Renvoie le logarithme népérien d'un nombre.

- `nombre` : Le nombre à traiter.

## LOG

`LOG(nombre, [base])`

Renvoie le logarithme d'un nombre dans une base donnée (base 10 si elle est omise).

- `nombre` : Le nombre à traiter.
- `base` (facultatif) : La base du logarithme (10 si elle est omise).

## LOG10

`LOG10(nombre)`

Renvoie le logarithme en base 10 d'un nombre.

- `nombre` : Le nombre à traiter.

## MAX

`MAX(nombre1, ...)`

Renvoie le plus grand nombre.

- `nombre1` : Le premier nombre, ou la première plage de cellules.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## MDETERM

`MDETERM(matrice)`

Renvoie le déterminant d'une plage carrée de nombres.

- `matrice` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.

## MIN

`MIN(nombre1, ...)`

Renvoie le plus petit nombre.

- `nombre1` : Le premier nombre, ou la première plage de cellules.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## MINVERSE

`MINVERSE(matrice)`

Renvoie la matrice inverse d'une plage carrée de nombres.

- `matrice` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.

## MMULT

`MMULT(matrice1, matrice2)`

Renvoie le produit de deux plages de nombres, traitées comme des matrices.

- `matrice1` : La première plage de cellules ou liste de valeurs.
- `matrice2` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.

## MOD

`MOD(nombre, diviseur)`

Renvoie le reste de la division d'un nombre par un autre.

- `nombre` : Le nombre à traiter.
- `diviseur` : Le nombre par lequel diviser.

## MROUND

`MROUND(nombre, multiple)`

Arrondit un nombre au multiple le plus proche.

- `nombre` : Le nombre à traiter.
- `multiple` : Le nombre dont les multiples servent à l'arrondi.

## MULTINOMIAL

`MULTINOMIAL(nombre1, ...)`

Renvoie la factorielle de la somme des nombres divisée par le produit de leurs factorielles.

- `nombre1` : Le premier nombre, ou la première plage de cellules.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## MUNIT

`MUNIT(dimension)`

Renvoie la matrice identité d'une taille donnée.

- `dimension` : La taille de la plage carrée : nombre de lignes et de colonnes.

## ODD

`ODD(nombre)`

Arrondit un nombre au nombre entier impair supérieur le plus proche.

- `nombre` : Le nombre à traiter.

## PERMUT

`PERMUT(nombre, nombre_choisi)`

Compte les façons de choisir des éléments dans un groupe lorsque l'ordre a de l'importance.

- `nombre` : Le nombre à traiter.
- `nombre_choisi` : Le nombre d'éléments à prélever dans chaque groupe.

## PI

`PI()`

Renvoie le nombre pi.

## POWER

`POWER(nombre, puissance)`

Élève un nombre à une puissance.

- `nombre` : Le nombre à traiter.
- `puissance` : La puissance à laquelle élever le nombre.

## PRODUCT

`PRODUCT(nombre1, ...)`

Multiplie des nombres entre eux.

- `nombre1` : Le premier nombre, ou la première plage de cellules.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## QUOTIENT

`QUOTIENT(numérateur, dénominateur)`

Renvoie la partie entière d'une division.

- `numérateur` : Le nombre à diviser.
- `dénominateur` : Le nombre par lequel diviser.

## RADIANS

`RADIANS(angle)`

Convertit des degrés en radians.

- `angle` : L'angle, en degrés.

## RAND

`RAND()`

Renvoie un nombre aléatoire supérieur ou égal à 0 et strictement inférieur à 1.

## RANDARRAY

`RANDARRAY([lignes], [colonnes], [min], [max], [nombre_entier])`

Donne une plage de nombres aléatoires.

- `lignes` (facultatif) : Le nombre de lignes à parcourir : positif vers le bas, négatif vers le haut.
- `colonnes` (facultatif) : Le nombre de colonnes du résultat (1 si omis).
- `min` (facultatif) : La plus petite valeur possible du résultat (0 si omis).
- `max` (facultatif) : La plus grande valeur possible du résultat (1 si omis).
- `nombre_entier` (facultatif) : VRAI pour des nombres entiers ; FAUX ou omis pour des nombres décimaux.

## RANDBETWEEN

`RANDBETWEEN(minimum, maximum)`

Renvoie un nombre entier aléatoire compris entre deux nombres.

- `minimum` : La plus petite valeur possible du résultat.
- `maximum` : La plus grande valeur possible du résultat.

## ROMAN

`ROMAN(nombre, [forme])`

Écrit un nombre entier de 0 à 3999 en chiffres romains, sous forme de texte.

- `nombre` : Le nombre à traiter.
- `forme` (facultatif) : La longueur des chiffres romains : 0 ou omis pour la forme classique, jusqu'à 4 pour la forme la plus courte.

## ROUND

`ROUND(nombre, nombre_chiffres)`

Arrondit un nombre au nombre de chiffres spécifié.

- `nombre` : Le nombre à traiter.
- `nombre_chiffres` : Le nombre de chiffres de l'arrondi : positif pour les décimales à droite de la virgule, 0 pour un nombre entier, négatif pour la gauche.

## ROUNDDOWN

`ROUNDDOWN(nombre, nombre_chiffres)`

Arrondit un nombre par défaut, vers zéro, au nombre de chiffres spécifié.

- `nombre` : Le nombre à traiter.
- `nombre_chiffres` : Le nombre de chiffres de l'arrondi : positif pour les décimales à droite de la virgule, 0 pour un nombre entier, négatif pour la gauche.

## ROUNDUP

`ROUNDUP(nombre, nombre_chiffres)`

Arrondit un nombre par excès, en s'éloignant de zéro, au nombre de chiffres spécifié.

- `nombre` : Le nombre à traiter.
- `nombre_chiffres` : Le nombre de chiffres de l'arrondi : positif pour les décimales à droite de la virgule, 0 pour un nombre entier, négatif pour la gauche.

## SEC

`SEC(angle)`

Renvoie la sécante d'un angle exprimé en radians.

- `angle` : L'angle, en radians.

## SECH

`SECH(nombre)`

Renvoie la sécante hyperbolique d'un nombre.

- `nombre` : Le nombre à traiter.

## SEQUENCE

`SEQUENCE(lignes, [colonnes], [début], [pas])`

Donne une plage de nombres qui s'incrémentent à partir d'un début, d'un pas donné.

- `lignes` : Le nombre de lignes à parcourir : positif vers le bas, négatif vers le haut.
- `colonnes` (facultatif) : Le nombre de colonnes du résultat (1 si omis).
- `début` (facultatif) : Le premier nombre (1 si omis).
- `pas` (facultatif) : La valeur ajoutée à chaque nombre (1 si omis ; pour GESTEP, le nombre de comparaison, 0 si omis).

## SERIESSUM

`SERIESSUM(x, n, m, coefficients)`

Renvoie la somme d'une série de puissances.

- `x` : La valeur de x à utiliser.
- `n` : La première puissance de x, ou l'ordre de la fonction, un nombre entier égal ou supérieur à 0.
- `m` : Le pas ajouté à la puissance pour chaque coefficient.
- `coefficients` : Les nombres par lesquels multiplier chaque puissance de x.

## SIGN

`SIGN(nombre)`

Renvoie 1 pour un nombre positif, 0 pour zéro, et -1 pour un nombre négatif.

- `nombre` : Le nombre à traiter.

## SIN

`SIN(nombre)`

Renvoie le sinus d'un angle exprimé en radians.

- `nombre` : Le nombre à traiter.

## SINH

`SINH(nombre)`

Renvoie le sinus hyperbolique d'un nombre.

- `nombre` : Le nombre à traiter.

## SQRT

`SQRT(nombre)`

Renvoie la racine carrée d'un nombre.

- `nombre` : Le nombre à traiter.

## SQRTPI

`SQRTPI(nombre)`

Renvoie la racine carrée d'un nombre multiplié par pi.

- `nombre` : Le nombre à traiter.

## SUBTOTAL

`SUBTOTAL(no_fonction, réf1, ...)`

Renvoie un total, une moyenne, un nombre ou une autre valeur pour une plage de cellules.

- `no_fonction` : Le calcul à effectuer : 1 moyenne, 2 nombre, 3 nombre de valeurs non vides, 4 maximum, 5 minimum, 6 produit, 7 et 8 écart type, 9 somme, 10 et 11 variance.
- `réf1` : La première cellule ou plage de cellules.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## SUM

`SUM(nombre1, ...)`

Additionne des nombres et les nombres contenus dans des plages de cellules.

- `nombre1` : Le premier nombre, ou la première plage de cellules.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## SUMPRODUCT

`SUMPRODUCT(matrice1, ...)`

Multiplie les nombres de plages de même taille, case par case, puis additionne les produits.

- `matrice1` : La première plage de cellules ou liste de valeurs.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## SUMSQ

`SUMSQ(nombre1, ...)`

Additionne les carrés des nombres.

- `nombre1` : Le premier nombre, ou la première plage de cellules.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## SUMX2MY2

`SUMX2MY2(matrice_x, matrice_y)`

Additionne les différences des carrés de deux séries de nombres.

- `matrice_x` : La première plage de cellules ou liste de valeurs.
- `matrice_y` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.

## SUMX2PY2

`SUMX2PY2(matrice_x, matrice_y)`

Additionne les sommes des carrés de deux séries de nombres.

- `matrice_x` : La première plage de cellules ou liste de valeurs.
- `matrice_y` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.

## SUMXMY2

`SUMXMY2(matrice_x, matrice_y)`

Additionne les carrés des différences de deux séries de nombres.

- `matrice_x` : La première plage de cellules ou liste de valeurs.
- `matrice_y` : La deuxième plage de cellules ou liste de valeurs, de même taille que la première.

## TAN

`TAN(nombre)`

Renvoie la tangente d'un angle exprimé en radians.

- `nombre` : Le nombre à traiter.

## TANH

`TANH(nombre)`

Renvoie la tangente hyperbolique d'un nombre.

- `nombre` : Le nombre à traiter.

## TRUNC

`TRUNC(nombre, [nombre_chiffres])`

Tronque un nombre après un certain nombre de chiffres, sans arrondir.

- `nombre` : Le nombre à traiter.
- `nombre_chiffres` (facultatif) : Le nombre de chiffres de l'arrondi : positif pour les décimales à droite de la virgule, 0 pour un nombre entier, négatif pour la gauche.

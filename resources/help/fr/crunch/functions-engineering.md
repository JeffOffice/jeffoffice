# Fonctions d'ingénierie
<!-- keywords: fonctions, BESSELI, BESSELJ, BESSELK, BESSELY, BIN2DEC, BIN2HEX, BIN2OCT, BITAND, BITLSHIFT, BITOR, BITRSHIFT, BITXOR, COMPLEX, CONVERT, DEC2BIN, DEC2HEX, DEC2OCT, DELTA, ERF, ERF.PRECISE, ERFC, ERFC.PRECISE, GESTEP, HEX2BIN, HEX2DEC, HEX2OCT, IMABS, IMAGINARY, IMARGUMENT, IMCONJUGATE, IMCOS, IMCOSH, IMCOT, IMCSC, IMCSCH, IMDIV, IMEXP, IMLN, IMLOG10, IMLOG2, IMPOWER, IMPRODUCT, IMREAL, IMSEC, IMSECH, IMSIN, IMSINH, IMSQRT, IMSUB, IMSUM, IMTAN, OCT2BIN, OCT2DEC, OCT2HEX, fonctions d'ingénierie, fonction, nombre complexe, binaire, hexadécimal, octal, décimal, conversion, bits, Bessel, fonction d'erreur, logarithme, trigonométrie -->

Voici les fonctions d'ingénierie que JeffCrunch peut calculer. Pour chacune, vous voyez sa syntaxe, ce qu'elle fait et le rôle de chacun de ses arguments. Un argument entre crochets est facultatif, et « ... » signifie que vous pouvez ajouter autant d'arguments supplémentaires que nécessaire après le dernier. Pour essayer une fonction, consultez [Choisir une fonction](insert-function). Pendant que vous saisissez une fonction dans une cellule, une petite info-bulle sous la cellule affiche ses arguments, et un clic sur le nom de la fonction dans l'info-bulle ouvre sa fiche ici.

## BESSELI

`BESSELI(x, n)`

Donne la fonction de Bessel modifiée de première espèce.

- `x` : La valeur x à utiliser.
- `n` : La première puissance de x, ou l'ordre de la fonction, un nombre entier égal ou supérieur à 0.

## BESSELJ

`BESSELJ(x, n)`

Donne la fonction de Bessel de première espèce.

- `x` : La valeur x à utiliser.
- `n` : La première puissance de x, ou l'ordre de la fonction, un nombre entier égal ou supérieur à 0.

## BESSELK

`BESSELK(x, n)`

Donne la fonction de Bessel modifiée de seconde espèce.

- `x` : La valeur x à utiliser.
- `n` : La première puissance de x, ou l'ordre de la fonction, un nombre entier égal ou supérieur à 0.

## BESSELY

`BESSELY(x, n)`

Donne la fonction de Bessel de seconde espèce.

- `x` : La valeur x à utiliser.
- `n` : La première puissance de x, ou l'ordre de la fonction, un nombre entier égal ou supérieur à 0.

## BIN2DEC

`BIN2DEC(number)`

Convertit un nombre binaire en nombre décimal.

- `number` : Nombre à traiter.

## BIN2HEX

`BIN2HEX(number, [places])`

Convertit un nombre binaire en nombre hexadécimal.

- `number` : Nombre à traiter.
- `places` (facultatif) : Nombre de caractères jusqu'auquel compléter le résultat par des zéros à gauche.

## BIN2OCT

`BIN2OCT(number, [places])`

Convertit un nombre binaire en nombre octal.

- `number` : Nombre à traiter.
- `places` (facultatif) : Nombre de caractères jusqu'auquel compléter le résultat par des zéros à gauche.

## BITAND

`BITAND(number1, number2)`

Donne les bits qui sont définis dans les deux nombres entiers.

- `number1` : Premier nombre ou plage de cellules.
- `number2` : Second nombre.

## BITLSHIFT

`BITLSHIFT(number, shift_amount)`

Décale les bits d'un nombre entier vers la gauche.

- `number` : Nombre à traiter.
- `shift_amount` : Nombre de bits à décaler : les nombres positifs et négatifs vont dans des sens opposés.

## BITOR

`BITOR(number1, number2)`

Donne les bits définis dans l'un ou l'autre des deux nombres entiers.

- `number1` : Premier nombre ou plage de cellules.
- `number2` : Second nombre.

## BITRSHIFT

`BITRSHIFT(number, shift_amount)`

Décale les bits d'un nombre entier vers la droite.

- `number` : Nombre à traiter.
- `shift_amount` : Nombre de bits à décaler : les nombres positifs et négatifs vont dans des sens opposés.

## BITXOR

`BITXOR(number1, number2)`

Donne les bits définis dans l'un des deux nombres entiers, mais pas dans les deux.

- `number1` : Premier nombre ou plage de cellules.
- `number2` : Second nombre.

## COMPLEX

`COMPLEX(real_num, i_num, [suffix])`

Crée un nombre complexe à partir de ses parties réelle et imaginaire.

- `real_num` : La partie réelle du nombre complexe.
- `i_num` : La partie imaginaire du nombre complexe.
- `suffix` (facultatif) : i ou j, pour marquer la partie imaginaire (i si omis).

## CONVERT

`CONVERT(number, from_unit, to_unit)`

Convertit un nombre d'une unité de mesure à une autre.

- `number` : Nombre à traiter.
- `from_unit` : L'unité du nombre, sous forme de texte, par exemple « mi » ou « lbm ».
- `to_unit` : L'unité vers laquelle convertir, sous forme de texte, par exemple « km » ou « kg ».

## DEC2BIN

`DEC2BIN(number, [places])`

Convertit un nombre décimal en nombre binaire.

- `number` : Nombre à traiter.
- `places` (facultatif) : Nombre de caractères jusqu'auquel compléter le résultat par des zéros à gauche.

## DEC2HEX

`DEC2HEX(number, [places])`

Convertit un nombre décimal en nombre hexadécimal.

- `number` : Nombre à traiter.
- `places` (facultatif) : Nombre de caractères jusqu'auquel compléter le résultat par des zéros à gauche.

## DEC2OCT

`DEC2OCT(number, [places])`

Convertit un nombre décimal en nombre octal.

- `number` : Nombre à traiter.
- `places` (facultatif) : Nombre de caractères jusqu'auquel compléter le résultat par des zéros à gauche.

## DELTA

`DELTA(number1, [number2])`

Indique si deux nombres sont égaux, avec 1 pour oui et 0 pour non.

- `number1` : Premier nombre ou plage de cellules.
- `number2` (facultatif) : Second nombre.

## ERF

`ERF(lower_limit, [upper_limit])`

Donne la fonction d'erreur, entre deux bornes ou à partir de 0.

- `lower_limit` : La borne inférieure de l'intervalle.
- `upper_limit` (facultatif) : La borne supérieure de l'intervalle (la borne inférieure seule si omise).

## ERF.PRECISE

`ERF.PRECISE(x)`

Donne la fonction d'erreur de 0 à une borne.

- `x` : La valeur x à utiliser.

## ERFC

`ERFC(x)`

Donne la fonction d'erreur complémentaire, d'une borne à l'infini.

- `x` : La valeur x à utiliser.

## ERFC.PRECISE

`ERFC.PRECISE(x)`

Donne la fonction d'erreur complémentaire, d'une borne à l'infini.

- `x` : La valeur x à utiliser.

## GESTEP

`GESTEP(number, [step])`

Indique si un nombre est au moins égal à un seuil, avec 1 pour oui et 0 pour non.

- `number` : Nombre à traiter.
- `step` (facultatif) : Valeur à ajouter pour chaque nombre (1 si omise ; pour GESTEP, le nombre auquel comparer, 0 si omis).

## HEX2BIN

`HEX2BIN(number, [places])`

Convertit un nombre hexadécimal en nombre binaire.

- `number` : Nombre à traiter.
- `places` (facultatif) : Nombre de caractères jusqu'auquel compléter le résultat par des zéros à gauche.

## HEX2DEC

`HEX2DEC(number)`

Convertit un nombre hexadécimal en nombre décimal.

- `number` : Nombre à traiter.

## HEX2OCT

`HEX2OCT(number, [places])`

Convertit un nombre hexadécimal en nombre octal.

- `number` : Nombre à traiter.
- `places` (facultatif) : Nombre de caractères jusqu'auquel compléter le résultat par des zéros à gauche.

## IMABS

`IMABS(inumber)`

Donne le module d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMAGINARY

`IMAGINARY(inumber)`

Donne la partie imaginaire d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMARGUMENT

`IMARGUMENT(inumber)`

Donne l'argument d'un nombre complexe, en radians.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMCONJUGATE

`IMCONJUGATE(inumber)`

Donne le conjugué d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMCOS

`IMCOS(inumber)`

Donne le cosinus d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMCOSH

`IMCOSH(inumber)`

Donne le cosinus hyperbolique d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMCOT

`IMCOT(inumber)`

Donne la cotangente d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMCSC

`IMCSC(inumber)`

Donne la cosécante d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMCSCH

`IMCSCH(inumber)`

Donne la cosécante hyperbolique d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMDIV

`IMDIV(inumber1, inumber2)`

Donne le quotient de deux nombres complexes.

- `inumber1` : Le premier nombre complexe, écrit sous forme de texte, par exemple « 3+4i ».
- `inumber2` : Le second nombre complexe, écrit sous forme de texte.

## IMEXP

`IMEXP(inumber)`

Donne e élevé à la puissance d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMLN

`IMLN(inumber)`

Donne le logarithme népérien d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMLOG10

`IMLOG10(inumber)`

Donne le logarithme en base 10 d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMLOG2

`IMLOG2(inumber)`

Donne le logarithme en base 2 d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMPOWER

`IMPOWER(inumber, number)`

Donne un nombre complexe élevé à une puissance.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».
- `number` : Nombre à traiter.

## IMPRODUCT

`IMPRODUCT(inumber1, ...)`

Donne le produit de nombres complexes.

- `inumber1` : Le premier nombre complexe, écrit sous forme de texte, par exemple « 3+4i ».
- `...` : Autant d'autres que nécessaire, du même type que le précédent.

## IMREAL

`IMREAL(inumber)`

Donne la partie réelle d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMSEC

`IMSEC(inumber)`

Donne la sécante d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMSECH

`IMSECH(inumber)`

Donne la sécante hyperbolique d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMSIN

`IMSIN(inumber)`

Donne le sinus d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMSINH

`IMSINH(inumber)`

Donne le sinus hyperbolique d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMSQRT

`IMSQRT(inumber)`

Donne la racine carrée d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## IMSUB

`IMSUB(inumber1, inumber2)`

Donne la différence de deux nombres complexes.

- `inumber1` : Le premier nombre complexe, écrit sous forme de texte, par exemple « 3+4i ».
- `inumber2` : Le second nombre complexe, écrit sous forme de texte.

## IMSUM

`IMSUM(inumber1, ...)`

Donne la somme de nombres complexes.

- `inumber1` : Le premier nombre complexe, écrit sous forme de texte, par exemple « 3+4i ».
- `...` : Autant d'autres que nécessaire, du même type que le précédent.

## IMTAN

`IMTAN(inumber)`

Donne la tangente d'un nombre complexe.

- `inumber` : Nombre complexe écrit sous forme de texte, par exemple « 3+4i ».

## OCT2BIN

`OCT2BIN(number, [places])`

Convertit un nombre octal en nombre binaire.

- `number` : Nombre à traiter.
- `places` (facultatif) : Nombre de caractères jusqu'auquel compléter le résultat par des zéros à gauche.

## OCT2DEC

`OCT2DEC(number)`

Convertit un nombre octal en nombre décimal.

- `number` : Nombre à traiter.

## OCT2HEX

`OCT2HEX(number, [places])`

Convertit un nombre octal en nombre hexadécimal.

- `number` : Nombre à traiter.
- `places` (facultatif) : Nombre de caractères jusqu'auquel compléter le résultat par des zéros à gauche.

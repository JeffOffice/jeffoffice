# Fonctions logiques
<!-- keywords: fonctions, ET, CHOISIR, FAUX, SI, SIERREUR, SI.NON.DISP, SI.CONDITIONS, LET, NON, OU, SI.MULTIPLE, VRAI, OU.EXCLUSIF -->

Voici les fonctions logiques que JeffCrunch peut calculer. Chacune indique sa syntaxe, ce qu'elle fait et le rôle de chacun de ses arguments. Un argument entre crochets est facultatif, et « ... » signifie que vous pouvez ajouter autant d'arguments supplémentaires du dernier type que nécessaire. Pour en essayer une, reportez-vous à [Choisir une fonction](insert-function). Pendant que vous saisissez une fonction dans une cellule, une petite info-bulle sous la cellule affiche ses arguments ; un clic sur le nom de la fonction dans cette info-bulle ouvre sa fiche ici.

## ET

`ET(logique1, ...)`

Vrai lorsque toutes les conditions sont vraies.

- `logique1` : La première valeur ou le premier test, vrai ou faux.
- `...` : Autant d'autres que nécessaire, du même type que le précédent.

## CHOISIR

`CHOISIR(index, valeur1, ...)`

Choisit dans une liste la valeur située à la position indiquée par l'index.

- `index` : Numéro de la valeur à renvoyer, le premier étant 1.
- `valeur1` : La première valeur, ou plage de cellules.
- `...` : Autant d'autres que nécessaire, du même type que le précédent.

## FAUX

`FAUX()`

La valeur logique faux.

## SI

`SI(condition, valeur_si_vrai, [valeur_si_faux])`

Renvoie une valeur lorsqu'une condition est remplie, et une autre lorsqu'elle ne l'est pas.

- `condition` : Une valeur ou un test, vrai ou faux.
- `valeur_si_vrai` : Ce qu'il faut renvoyer si la condition est vraie.
- `valeur_si_faux` (facultatif) : Ce qu'il faut renvoyer si la condition est fausse (FAUX si cet argument est omis).

## SIERREUR

`SIERREUR(valeur, valeur_si_erreur)`

Renvoie une valeur, ou un remplacement lorsque cette valeur est une erreur.

- `valeur` : La valeur à traiter, ou une cellule qui la contient.
- `valeur_si_erreur` : Ce qu'il faut renvoyer si la valeur est une erreur.

## SI.NON.DISP

`SI.NON.DISP(valeur, valeur_si_non_disp)`

Renvoie une valeur, ou un remplacement lorsque cette valeur est l'erreur « non disponible ».

- `valeur` : La valeur à traiter, ou une cellule qui la contient.
- `valeur_si_non_disp` : Ce qu'il faut renvoyer si la valeur est l'erreur « non disponible ».

## SI.CONDITIONS

`SI.CONDITIONS(condition1, valeur1, ...)`

Teste les conditions l'une après l'autre et renvoie la valeur associée à la première qui est remplie.

- `condition1` : Le premier test, vrai ou faux.
- `valeur1` : La première valeur, ou plage de cellules.
- `...` : Autant d'autres que nécessaire, du même type que le précédent.

## LET

`LET(nom1, valeur1, ...)`

Donne des noms à des valeurs dans une même formule, puis calcule un résultat à partir de ces noms.

- `nom1` : Un nom à donner à la valeur qui suit. Il peut être utilisé dans les valeurs suivantes et dans le calcul.
- `valeur1` : La première valeur, ou plage de cellules.
- `...` : Autant d'autres que nécessaire, du même type que le précédent.

## NON

`NON(logique)`

Transforme vrai en faux, et faux en vrai.

- `logique` : Une valeur ou un test, vrai ou faux.

## OU

`OU(logique1, ...)`

Vrai lorsqu'au moins une des conditions est vraie.

- `logique1` : La première valeur ou le premier test, vrai ou faux.
- `...` : Autant d'autres que nécessaire, du même type que le précédent.

## SI.MULTIPLE

`SI.MULTIPLE(expression, valeur1, résultat1, ...)`

Compare une valeur à une liste de valeurs et renvoie le résultat associé à la première correspondance.

- `expression` : La valeur à comparer aux valeurs qui suivent.
- `valeur1` : La première valeur, ou plage de cellules.
- `résultat1` : Le résultat à renvoyer lorsque la valeur qui le précède correspond.
- `...` : Autant d'autres que nécessaire, du même type que le précédent.

## VRAI

`VRAI()`

La valeur logique vrai.

## OU.EXCLUSIF

`OU.EXCLUSIF(logique1, ...)`

Vrai lorsqu'un nombre impair de conditions sont vraies.

- `logique1` : La première valeur ou le premier test, vrai ou faux.
- `...` : Autant d'autres que nécessaire, du même type que le précédent.

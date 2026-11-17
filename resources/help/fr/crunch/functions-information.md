# Fonctions d'information
<!-- keywords: fonctions, CELLULE, TYPE.ERREUR, INFO, ESTVIDE, ESTERR, ESTERREUR, ESTPAIR, ESTFORMULE, ESTLOGIQUE, ESTNA, ESTNONTEXTE, ESTNUM, ESTIMPAIR, ESTREF, ESTTEXTE, N, NA, FEUILLE, FEUILLES, TYPE, fonction d'information, fonctions d'information, ISBLANK, ISERR, ISERROR, ISEVEN, ISFORMULA, ISLOGICAL, ISNA, ISNONTEXT, ISNUMBER, ISODD, ISREF, ISTEXT, SHEET, SHEETS, INFORMATION -->

Voici les fonctions d'information que JeffCrunch peut calculer. Chacune indique sa syntaxe, ce qu'elle fait et le rôle de chacun de ses arguments. Un argument entre crochets est facultatif, et « ... » signifie que vous pouvez en ajouter autant que nécessaire après le dernier. Pour en essayer une, reportez-vous à [Choisir une fonction](insert-function). Pendant que vous saisissez une fonction dans une cellule, une petite info-bulle sous la cellule affiche ses arguments ; un clic sur le nom de la fonction dans l'info-bulle ouvre sa fiche dans cette rubrique.

## CELLULE

`CELLULE(type_info, [référence])`

Donne une information sur une cellule, par exemple son adresse ou son format.

- `type_info` : L'information à donner sur la cellule, sous forme de texte, par exemple « adresse », « ligne », « col » ou « format ».
- `référence` (facultatif) : La cellule concernée (la cellule où se trouve le curseur si cet argument est omis).

## TYPE.ERREUR

`TYPE.ERREUR(valeur_erreur)`

Donne le numéro qui correspond à un type d'erreur.

- `valeur_erreur` : L'erreur à identifier, ou une cellule qui la contient.

## INFO

`INFO(type_info)`

Donne une information sur le programme ou le classeur, par exemple son dossier ou son mode de calcul.

- `type_info` : L'information à donner, sous forme de texte, par exemple « répertoire », « ligne » ou « calcul ».

## ESTVIDE

`ESTVIDE(valeur)`

Vrai si la cellule est vide.

- `valeur` : La valeur à traiter, ou une cellule qui la contient.

## ESTERR

`ESTERR(valeur)`

Vrai si la valeur est une erreur autre que « non disponible ».

- `valeur` : La valeur à traiter, ou une cellule qui la contient.

## ESTERREUR

`ESTERREUR(valeur)`

Vrai si la valeur est une erreur quelconque.

- `valeur` : La valeur à traiter, ou une cellule qui la contient.

## ESTPAIR

`ESTPAIR(nombre)`

Vrai si un nombre entier est pair.

- `nombre` : Le nombre à traiter.

## ESTFORMULE

`ESTFORMULE(référence)`

Vrai lorsque la cellule contient une formule.

- `référence` : Une cellule ou une plage de cellules.

## ESTLOGIQUE

`ESTLOGIQUE(valeur)`

Vrai si la valeur est vrai ou faux.

- `valeur` : La valeur à traiter, ou une cellule qui la contient.

## ESTNA

`ESTNA(valeur)`

Vrai si la valeur est l'erreur « non disponible ».

- `valeur` : La valeur à traiter, ou une cellule qui la contient.

## ESTNONTEXTE

`ESTNONTEXTE(valeur)`

Vrai si la valeur est autre chose que du texte.

- `valeur` : La valeur à traiter, ou une cellule qui la contient.

## ESTNUM

`ESTNUM(valeur)`

Vrai si la valeur est un nombre.

- `valeur` : La valeur à traiter, ou une cellule qui la contient.

## ESTIMPAIR

`ESTIMPAIR(nombre)`

Vrai si un nombre entier est impair.

- `nombre` : Le nombre à traiter.

## ESTREF

`ESTREF(valeur)`

Vrai si la valeur est une référence à des cellules.

- `valeur` : La valeur à traiter, ou une cellule qui la contient.

## ESTTEXTE

`ESTTEXTE(valeur)`

Vrai si la valeur est du texte.

- `valeur` : La valeur à traiter, ou une cellule qui la contient.

## N

`N(valeur)`

Convertit une valeur en nombre : les dates et les valeurs vrai deviennent des nombres, le texte devient zéro.

- `valeur` : La valeur à convertir, ou une cellule qui la contient.

## NA

`NA()`

Donne l'erreur « non disponible », pour signaler une valeur manquante.

## FEUILLE

`FEUILLE([valeur])`

Donne le numéro d'une feuille, la première feuille étant numérotée 1.

- `valeur` (facultatif) : La valeur à traiter, ou une cellule qui la contient.

## FEUILLES

`FEUILLES([référence])`

Donne le nombre de feuilles dans une référence, ou dans le classeur.

- `référence` (facultatif) : Une cellule ou une plage de cellules.

## TYPE

`TYPE(valeur)`

Donne un nombre qui indique le type de valeur : nombre, texte, logique, erreur ou matrice.

- `valeur` : La valeur à traiter, ou une cellule qui la contient.

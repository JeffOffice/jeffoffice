# Fonctions de texte
<!-- keywords: fonctions, ASC, BAHTTEXT, CAR, CONCAT, CONCATENER, CHERCHE, CODE, DBCS, DEVISE, ENCODEURL, EXACT, FILTERXML, TROUVE, TROUVERB, FIXE, GAUCHE, GAUCHEB, NBCAR, NBCARB, MINUSCULE, STXT, STXTB, VALEURNUM, PHONETIQUE, NOMPROPRE, REMPLACER, REMPLACERB, REPT, DROITE, DROITEB, SUBSTITUE, T, TEXTE, JOINDRE.TEXTE, EPURAGE, UNICAR, UNICODE, MAJUSCULE, CNUM -->

Voici les fonctions de texte que JeffCrunch peut calculer. Chacune indique sa syntaxe, ce qu'elle fait et le rôle de chacun de ses arguments. Un argument entre crochets est facultatif, et « ... » signifie que vous pouvez ajouter autant d'arguments supplémentaires que nécessaire après le dernier. Pour essayer l'une d'elles, voir [Choisir une fonction](insert-function). Pendant que vous saisissez une fonction dans une cellule, une petite info-bulle sous la cellule affiche ses arguments ; un clic sur le nom de la fonction dans cette info-bulle ouvre sa rubrique ici.

## ASC

`ASC(texte)`

Remplace dans un texte les lettres et les chiffres pleine chasse par leurs équivalents demi-chasse.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## BAHTTEXT

`BAHTTEXT(nombre)`

Écrit un nombre en mots thaïlandais, avec le mot pour le baht.

- `nombre` : Le nombre à traiter.

## CAR

`CAR(nombre)`

Donne le caractère correspondant à un code numérique.

- `nombre` : Le nombre à traiter.

## EPURAGE

`EPURAGE(texte)`

Supprime d'un texte les caractères qui ne s'impriment pas.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## CODE

`CODE(texte)`

Donne le code numérique du premier caractère d'un texte.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## CONCAT

`CONCAT(texte1, ...)`

Assemble en un seul texte des textes et des textes contenus dans des plages de cellules.

- `texte1` : Le premier texte, ou une plage de cellules contenant du texte.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## CONCATENER

`CONCATENER(texte1, ...)`

Assemble des textes en un seul.

- `texte1` : Le premier texte, ou une plage de cellules contenant du texte.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## DBCS

`DBCS(texte)`

Remplace dans un texte les lettres et les chiffres demi-chasse par leurs équivalents pleine chasse.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## DEVISE

`DEVISE(nombre, [décimales])`

Transforme un nombre en texte, sous forme de montant monétaire.

- `nombre` : Le nombre à traiter.
- `décimales` (facultatif) : Le nombre de chiffres à conserver après la virgule.

## ENCODEURL

`ENCODEURL(texte)`

Modifie un texte pour qu'il puisse être utilisé dans une adresse web.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## EXACT

`EXACT(texte1, texte2)`

Vrai si deux textes sont strictement identiques, majuscules comprises.

- `texte1` : Le premier texte, ou une plage de cellules contenant du texte.
- `texte2` : Le second texte.

## FILTERXML

`FILTERXML(xml, xpath)`

Extrait des valeurs d'un texte contenant du XML, à l'aide d'une expression XPath.

- `xml` : Le texte qui contient le XML.
- `xpath` : Le XPath qui indique les parties à donner, par exemple //article/prix.

## TROUVE

`TROUVE(texte_cherché, dans_le_texte, [no_départ])`

Donne la position à laquelle un texte commence dans un autre, en distinguant majuscules et minuscules.

- `texte_cherché` : Le texte à rechercher.
- `dans_le_texte` : Le texte dans lequel chercher.
- `no_départ` (facultatif) : La position dans le texte où commencer la recherche, le premier caractère étant compté comme 1.

## TROUVERB

`TROUVERB(texte_cherché, dans_le_texte, [no_départ])`

Donne la position à laquelle un texte apparaît pour la première fois dans un autre texte, en distinguant majuscules et minuscules.

- `texte_cherché` : Le texte à rechercher.
- `dans_le_texte` : Le texte dans lequel chercher.
- `no_départ` (facultatif) : La position dans le texte où commencer la recherche, le premier caractère étant compté comme 1.

## FIXE

`FIXE(nombre, [décimales], [sans_séparateurs])`

Transforme un nombre en texte avec un nombre fixe de décimales.

- `nombre` : Le nombre à traiter.
- `décimales` (facultatif) : Le nombre de chiffres à conserver après la virgule.
- `sans_séparateurs` (facultatif) : Vrai pour ne pas afficher les virgules qui regroupent les milliers.

## GAUCHE

`GAUCHE(texte, [nb_caractères])`

Donne les premiers caractères d'un texte.

- `texte` : Le texte à traiter, ou une cellule qui le contient.
- `nb_caractères` (facultatif) : Le nombre de caractères à prendre (1 si omis).

## GAUCHEB

`GAUCHEB(texte, [nb_caractères])`

Donne les premiers caractères d'un texte.

- `texte` : Le texte à traiter, ou une cellule qui le contient.
- `nb_caractères` (facultatif) : Le nombre de caractères à prendre (1 si omis).

## NBCAR

`NBCAR(texte)`

Compte les caractères d'un texte.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## NBCARB

`NBCARB(texte)`

Donne le nombre de caractères d'un texte.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## MINUSCULE

`MINUSCULE(texte)`

Convertit un texte en minuscules.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## STXT

`STXT(texte, no_départ, nb_caractères)`

Donne des caractères situés au milieu d'un texte.

- `texte` : Le texte à traiter, ou une cellule qui le contient.
- `no_départ` : La position dans le texte où commencer, le premier caractère étant compté comme 1.
- `nb_caractères` : Le nombre de caractères à prendre (1 si omis).

## STXTB

`STXTB(texte, no_départ, nb_caractères)`

Donne des caractères au milieu d'un texte.

- `texte` : Le texte à traiter, ou une cellule qui le contient.
- `no_départ` : La position dans le texte où commencer, le premier caractère étant compté comme 1.
- `nb_caractères` : Le nombre de caractères à prendre (1 si omis).

## VALEURNUM

`VALEURNUM(texte, [séparateur_décimal], [séparateur_groupes])`

Lit un texte comme un nombre, avec les séparateurs que vous indiquez.

- `texte` : Le texte à traiter, ou une cellule qui le contient.
- `séparateur_décimal` (facultatif) : Le caractère qui marque les décimales dans le texte (le point si omis).
- `séparateur_groupes` (facultatif) : Le caractère qui regroupe les chiffres dans le texte (la virgule si omis).

## PHONETIQUE

`PHONETIQUE(référence)`

Donne le texte d'une cellule, pour les lectures du texte japonais.

- `référence` : Une cellule ou une plage de cellules.

## NOMPROPRE

`NOMPROPRE(texte)`

Met en majuscule la première lettre de chaque mot d'un texte.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## REMPLACER

`REMPLACER(ancien_texte, no_départ, nb_caractères, nouveau_texte)`

Insère un nouveau texte à la place de certains caractères d'un texte.

- `ancien_texte` : Le texte à modifier.
- `no_départ` : La position dans le texte où commencer, le premier caractère étant compté comme 1.
- `nb_caractères` : Le nombre de caractères à remplacer (1 si omis).
- `nouveau_texte` : Le texte à insérer.

## REMPLACERB

`REMPLACERB(ancien_texte, no_départ, nb_caractères, nouveau_texte)`

Remplace une partie d'un texte par un autre texte.

- `ancien_texte` : Le texte à modifier.
- `no_départ` : La position dans le texte où commencer, le premier caractère étant compté comme 1.
- `nb_caractères` : Le nombre de caractères à remplacer (1 si omis).
- `nouveau_texte` : Le texte à insérer.

## REPT

`REPT(texte, nombre_fois)`

Répète un texte un certain nombre de fois.

- `texte` : Le texte à traiter, ou une cellule qui le contient.
- `nombre_fois` : Le nombre de répétitions du texte.

## DROITE

`DROITE(texte, [nb_caractères])`

Donne les derniers caractères d'un texte.

- `texte` : Le texte à traiter, ou une cellule qui le contient.
- `nb_caractères` (facultatif) : Le nombre de caractères à prendre (1 si omis).

## DROITEB

`DROITEB(texte, [nb_caractères])`

Donne les derniers caractères d'un texte.

- `texte` : Le texte à traiter, ou une cellule qui le contient.
- `nb_caractères` (facultatif) : Le nombre de caractères à prendre (1 si omis).

## CHERCHE

`CHERCHE(texte_cherché, dans_le_texte, [no_départ])`

Donne la position à laquelle un texte commence dans un autre, sans tenir compte des majuscules, et en acceptant ? et * dans le texte recherché.

- `texte_cherché` : Le texte à rechercher.
- `dans_le_texte` : Le texte dans lequel chercher.
- `no_départ` (facultatif) : La position dans le texte où commencer la recherche, le premier caractère étant compté comme 1.

## CHERCHEB

`CHERCHEB(texte_cherché, dans_le_texte, [no_départ])`

Donne la position à laquelle un texte apparaît pour la première fois dans un autre texte, sans tenir compte des majuscules.

- `texte_cherché` : Le texte à rechercher.
- `dans_le_texte` : Le texte dans lequel chercher.
- `no_départ` (facultatif) : La position dans le texte où commencer la recherche, le premier caractère étant compté comme 1.

## SUBSTITUE

`SUBSTITUE(texte, ancien_texte, nouveau_texte, [no_occurrence])`

Remplace un morceau de texte par un autre à chaque endroit où il apparaît.

- `texte` : Le texte à traiter, ou une cellule qui le contient.
- `ancien_texte` : Le texte à rechercher.
- `nouveau_texte` : Le texte à insérer.
- `no_occurrence` (facultatif) : L'occurrence à remplacer (toutes si omis).

## T

`T(valeur)`

Donne le texte lui-même, et rien pour une valeur qui n'est pas du texte.

- `valeur` : La valeur à traiter, ou une cellule qui la contient.

## TEXTE

`TEXTE(valeur, format_texte)`

Transforme un nombre en texte selon un format de votre choix.

- `valeur` : La valeur à traiter, ou une cellule qui la contient.
- `format_texte` : Le format de nombre dans lequel afficher la valeur, par exemple « 0.00 » ou « jj/mm/aaaa ».

## JOINDRE.TEXTE

`JOINDRE.TEXTE(séparateur, ignorer_vides, texte1, ...)`

Assemble des textes en insérant un séparateur entre eux.

- `séparateur` : Le texte à insérer entre les éléments.
- `ignorer_vides` : Vrai pour ignorer les éléments vides ; faux pour les conserver.
- `texte1` : Le premier texte, ou une plage de cellules contenant du texte.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## SUPPRESPACE

`SUPPRESPACE(texte)`

Supprime les espaces superflus d'un texte, en ne laissant qu'une seule espace entre les mots.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## UNICAR

`UNICAR(nombre)`

Donne le caractère correspondant à un nombre Unicode.

- `nombre` : Le nombre à traiter.

## UNICODE

`UNICODE(texte)`

Donne le nombre Unicode du premier caractère d'un texte.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## MAJUSCULE

`MAJUSCULE(texte)`

Convertit un texte en majuscules.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

## CNUM

`CNUM(texte)`

Transforme un texte qui ressemble à un nombre en nombre.

- `texte` : Le texte à traiter, ou une cellule qui le contient.

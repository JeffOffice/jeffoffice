# Rechercher, remplacer et atteindre
<!-- keywords: rechercher, trouver, remplacer, modifier, recherche, caractère générique, joker, astérisque, étoile, point d'interrogation, tilde, respecter la casse, majuscules, totalité du contenu de la cellule, atteindre, aller à, référence, nom, sélectionner par type, cellules vides, constantes, formules, erreurs, zone en cours, dernière cellule, cellules visibles, mise en forme conditionnelle, validation des données, Ctrl+F, Ctrl+H, F5, notes -->

## Rechercher

Appuyez sur **Ctrl+F** (ou choisissez **Rechercher** dans l'onglet Accueil, puis de nouveau **Rechercher**). Tapez le texte et appuyez sur **Entrée** pour atteindre la cellule suivante qui le contient, ou choisissez **Rechercher tout** pour obtenir la liste de toutes les cellules correspondantes. Cliquez sur une ligne de la liste pour atteindre cette cellule.

| Option | Rôle |
| --- | --- |
| **Dans** | Recherche dans cette feuille, ou dans toutes les feuilles affichées. Si plusieurs cellules sont sélectionnées, la recherche dans une feuille porte uniquement sur ces cellules. |
| **Sens** | Parcourt les cellules par ligne (de gauche à droite, puis vers le bas) ou par colonne (de haut en bas, puis vers la droite). **Suivant** s'arrête sur la première correspondance dans cet ordre. |
| **Regarder dans** | **Formules** recherche dans ce qui est saisi dans chaque cellule, **Valeurs** dans ce qu'elle affiche, et **Notes** dans les notes des cellules. |
| **Respecter la casse** | Les majuscules et les minuscules doivent être identiques. |
| **Totalité du contenu de la cellule** | La cellule doit contenir exactement le texte, et pas seulement une partie de celui-ci. |

Dans le texte recherché, **?** remplace un caractère quelconque et **\*** une suite quelconque de caractères (y compris aucun). Tapez **~** devant ?, \* ou ~ pour rechercher le caractère lui-même : **a~\*b** correspond à a\*b.

La recherche s'effectue petit à petit, ce qui vous permet de continuer à travailler. **Échap** (ou **Annuler**) l'interrompt, de même que la modification d'une cellule. Un texte très long occupe une plus grande part de chaque petite étape qu'un texte court : ainsi, même une feuille contenant de longs textes laisse la fenêtre disponible.

Les notes suivent les cellules : après un tri, ou après l'insertion ou la suppression de lignes ou de colonnes, la recherche dans les notes trouve chaque note dans la cellule où elle se trouve désormais. La note d'une cellule supprimée n'est plus trouvée par la recherche (elle l'est de nouveau après Annuler). Si un classeur a subi tant de modifications que les notes ne peuvent pas être suivies à travers toutes ces modifications, vous êtes invité à l'enregistrer d'abord. Dans un classeur ouvert à partir d'un ancien type de fichier ou d'un fichier texte, les notes sont dans un format dans lequel JeffCrunch n'effectue pas de recherche, et un message vous en informe.

## Remplacer

Appuyez sur **Ctrl+H**. **Remplacer** insère le nouveau texte dans la cellule contenant le curseur si elle contient le texte recherché, puis passe à la suivante. **Remplacer tout** effectue le remplacement dans toutes les cellules qui contiennent le texte, en une seule opération que vous pouvez annuler ; un remplacement important s'effectue petit à petit. Le nouveau texte est entré comme si vous l'aviez tapé : un nombre devient un nombre, et une formule modifiée reste une formule.

Rien n'est modifié, et un message vous en indique la raison, si une cellule à modifier est verrouillée dans une feuille protégée, fait partie d'une matrice ou d'une table de données, se trouve dans la première ligne d'un tableau, ou ne serait plus une formule.

## Atteindre

Appuyez sur **F5** (ou **Ctrl+G**), ou tapez dans la zone Nom. Vous pouvez indiquer une cellule comme B7, une plage comme B3:D9, des colonnes ou des lignes entières (C:E, 4:9), une cellule d'une autre feuille (Feuil2!A1, ou 'Ma feuille'!A1), ou le nom d'une plage.

## Sélectionner par type

Choisissez **Sélectionner par type** dans le menu **Rechercher** pour sélectionner des cellules selon leur nature :

- **Cellules vides**, **Constantes** (saisies), **Formules** et **Cellules contenant une erreur**. Pour les constantes et les formules, vous pouvez exclure les nombres, le texte, VRAI et FAUX, ou les erreurs.
- **Le bloc autour de la cellule contenant le curseur** et **La dernière cellule utilisée**.
- **Cellules affichées** (hors lignes ou colonnes masquées), **Cellules avec mise en forme conditionnelle** et **Cellules avec une règle pour les valeurs autorisées** (toutes, ou seulement celles qui ont la même règle que la cellule contenant le curseur).

Le premier bloc trouvé est sélectionné et les autres sont teintés ; la teinte disparaît lorsque vous cliquez sur une cellule.

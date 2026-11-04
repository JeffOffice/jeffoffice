# Rechercher, remplacer et atteindre

<!-- keywords: rechercher, remplacer, modifier, chercher, caractère générique, astérisque, point d'interrogation, tilde, même casse, cellule entière, atteindre, référence, nom, sélectionner par type, cellules vides, constantes, formules, erreurs, zone active, matrice active, différences de ligne, différences de colonne, dernière cellule, cellules visibles, mises en forme conditionnelles, validation, Ctrl+F, Ctrl+H, F5, notes -->

## Rechercher

Appuyez sur **Ctrl+F** (ou choisissez **Rechercher** dans l'onglet Accueil, puis de nouveau **Rechercher**). Tapez le texte, puis appuyez sur **Entrée** pour atteindre la cellule suivante qui le contient, ou choisissez **Rechercher tout** pour obtenir la liste de toutes les cellules correspondantes. Cliquez sur une ligne de la liste pour atteindre cette cellule.

| Option | Ce qu'elle fait |
| --- | --- |
| **Où** | Rechercher dans cette feuille, ou dans toutes les feuilles affichées. Si plusieurs cellules sont sélectionnées, une recherche dans une feuille ne porte que sur elles. |
| **Ordre** | Parcourir les cellules par lignes (de gauche à droite, puis vers le bas) ou par colonnes (de haut en bas, puis vers la droite). **Suivant** s'arrête à la première occurrence dans cet ordre. |
| **Rechercher dans** | **Formules** recherche dans ce qui est saisi dans chaque cellule, **Valeurs** dans ce qui est affiché, et **Notes** dans les notes des cellules. |
| **Respecter la casse** | Les majuscules et les minuscules doivent être identiques. |
| **Cellule entière** | La cellule entière doit contenir le texte, et non seulement une partie de celui-ci. |

Dans le texte recherché, **?** remplace un caractère quelconque et **\*** remplace une suite quelconque de caractères (y compris aucun caractère). Tapez **~** devant ?, \* ou ~ pour rechercher le signe lui-même : **a~\*b** trouve a\*b.

La recherche s'effectue par petites étapes, afin que vous puissiez continuer à travailler. **Échap** (ou **Annuler**) l'arrête, et la modification d'une cellule l'arrête également. Un texte très long coûte davantage à chaque étape qu'un texte court : une feuille contenant de longs textes laisse donc tout de même la fenêtre disponible.

Les notes des cellules suivent les cellules : après un tri, ou après l'insertion ou la suppression de lignes ou de colonnes, une recherche dans les notes trouve chaque note à la cellule où elle se trouve désormais. Une note placée sur une cellule supprimée n'est plus prise en compte dans la recherche (elle réapparaît après Annuler). Si un classeur a été modifié de tant de façons que les notes ne peuvent plus être suivies, il vous est demandé de l'enregistrer d'abord ; un classeur ouvert depuis un ancien type de fichier ou depuis un fichier texte conserve ses notes sous une forme que JeffCrunch ne peut pas rechercher, et vous en êtes averti.

## Remplacer

Appuyez sur **Ctrl+H**. **Remplacer** place le nouveau texte dans la cellule active si elle contient le texte recherché, puis passe à la suivante. **Tout remplacer** effectue le remplacement dans toutes les cellules concernées, en une seule étape que vous pouvez annuler ; un remplacement important est appliqué progressivement. Le nouveau texte est saisi comme si vous l'aviez tapé : un nombre devient un nombre, et une formule modifiée reste une formule.

Rien n'est modifié, et un message en indique la raison, si une cellule qui devrait être modifiée est verrouillée sur une feuille protégée, fait partie d'une matrice ou d'une table de données, se trouve dans la première ligne d'un tableau, ou cesserait d'être une formule.

## Atteindre

Appuyez sur **F5** (ou **Ctrl+G**), ou saisissez une adresse dans la zone Nom. Vous pouvez indiquer une cellule telle que B7, une plage telle que B3:D9, des colonnes ou des lignes entières (C:E, 4:9), une cellule d'une autre feuille (Feuille2!A1, ou 'Ma feuille'!A1), ou le nom d'une plage.

## Sélectionner par type

Choisissez **Sélectionner par type** dans le menu **Rechercher** pour sélectionner des cellules selon leur nature :

- **Cellules vides**, **Constantes** (saisies), **Formules** et **Cellules contenant une erreur**. Pour les constantes et les formules, vous pouvez exclure les nombres, le texte, VRAI et FAUX, ou les erreurs.
- **La zone autour de la cellule active** et **La dernière cellule utilisée**.
- **La formule matricielle où se trouve le curseur** sélectionne toutes les cellules de cette formule matricielle.
- **Cellules dont le contenu diffère de celui de la cellule active dans leur ligne** et **dans leur colonne** trouvent les cellules qui ne ressemblent pas à la cellule active. Sélectionnez d'abord une plage, puis placez le curseur sur la cellule à laquelle les autres doivent correspondre. Dans chaque ligne (ou colonne) de la plage, une cellule est retenue si elle contient un nombre ou un texte différent, ou si elle contient une formule qui n'est pas identique une fois ses noms de cellules décalés vers l'emplacement de cette cellule. Une cellule contenant une formule est aussi retenue lorsque la cellule active n'en contient pas. Si vous ne sélectionnez qu'une seule cellule, JeffCrunch examine la plage qui l'entoure.
- **Cellules visibles** (hors lignes ou colonnes masquées), **Cellules avec mise en forme conditionnelle**, et **Cellules avec une règle de saisie** (toutes, ou seulement celles qui ont la même règle que la cellule active).

Le premier bloc trouvé est sélectionné et les autres sont teintés ; la teinte disparaît lorsque vous cliquez sur une cellule.

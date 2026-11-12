# Rechercher, remplacer et atteindre
<!-- keywords: rechercher, remplacer, modifier, chercher, rechercher ce, remplacer par, remplacer tout, rechercher suivant, rechercher, changer en, modifier tout, même casse, caractère générique, astérisque, point d'interrogation, tilde, respecter la casse, cellule entière, atteindre, référence, nom, sélectionner par type, cellules vides, constantes, formules, erreurs, région active, matrice active, différences de ligne, différences de colonne, dernière cellule, cellules visibles, mises en forme conditionnelles, validation, Ctrl+F, Ctrl+H, F5, notes -->

## Rechercher

Appuyez sur **Ctrl+F** (ou choisissez **Rechercher** dans l'onglet Accueil, puis de nouveau **Rechercher**). Saisissez le texte dans **Rechercher** et appuyez sur **Entrée** pour atteindre la cellule suivante qui le contient, ou choisissez **Rechercher tout** pour obtenir la liste de toutes les cellules correspondantes. Cliquez sur une ligne de la liste pour atteindre cette cellule.

| Option | Ce qu'elle fait |
| --- | --- |
| **Dans** | Recherche dans cette feuille, ou dans toutes les feuilles affichées. Si plusieurs cellules sont sélectionnées, une recherche sur une seule feuille ne porte que sur elles. |
| **Sens** | Parcourt les cellules par lignes (de gauche à droite, puis vers le bas) ou par colonnes (de haut en bas, puis vers la droite). **Rechercher suivant** s'arrête à la première occurrence dans cet ordre. |
| **Regarder dans** | **Formules** examine ce qui est saisi dans chaque cellule, **Valeurs** ce qu'elle affiche, et **Notes** les notes des cellules. |
| **Respecter la casse** | Les majuscules et les minuscules doivent être identiques. |
| **Totalité du contenu de la cellule** | La cellule entière doit correspondre au texte, et non une simple partie de celle-ci. |

Dans le texte saisi dans **Rechercher**, **?** remplace un caractère quelconque et **\*** remplace une suite quelconque de caractères (y compris aucun). Saisissez **~** devant ?, \* ou ~ pour rechercher le signe lui-même : **a~\*b** correspond à a\*b.

La recherche s'exécute par petites étapes, ce qui vous permet de continuer à travailler. **Échap** (ou **Annuler**) l'interrompt, et la modification d'une cellule l'interrompt également. Un texte très long coûte davantage à chaque étape qu'un texte court : une feuille contenant de longs textes laisse donc tout de même la fenêtre disponible.

Les notes des cellules suivent les cellules : après un tri, ou après l'insertion ou la suppression de lignes ou de colonnes, une recherche dans les notes trouve chaque note à la cellule où elle se trouve désormais. Une note placée sur une cellule supprimée n'apparaît plus dans la recherche (elle revient après Annuler). Si un classeur a été modifié de tant de façons que les notes ne peuvent pas être suivies dans toutes, il vous est demandé de l'enregistrer d'abord ; un classeur ouvert à partir d'un ancien type de fichier ou d'un fichier texte conserve ses notes sous une forme que JeffCrunch ne peut pas rechercher, et vous en êtes informé.

## Remplacer

Appuyez sur **Ctrl+H**, saisissez le nouveau texte dans **Remplacer par**, puis choisissez un bouton. **Remplacer** place le nouveau texte dans la cellule où se trouve le curseur si elle contient le texte, puis passe à la suivante. **Tout remplacer** effectue la modification dans toutes les cellules qui la contiennent, en une seule étape que vous pouvez annuler ; une modification importante est appliquée par petites portions. Le nouveau texte est saisi comme si vous le tapiez : un nombre reste un nombre, et une formule modifiée reste une formule.

Rien n'est modifié, et un message vous en indique la raison, si une cellule à modifier est verrouillée sur une feuille protégée, fait partie d'une matrice ou d'un tableau de données, se trouve dans la première ligne d'un tableau, ou ne serait plus une formule.

## Atteindre

Appuyez sur **F5** (ou **Ctrl+G**), ou saisissez directement dans la **Zone Nom**. Vous pouvez indiquer une cellule telle que B7, une plage telle que B3:D9, des colonnes ou des lignes entières (C:E, 4:9), une cellule d'une autre feuille (Feuil2!A1, ou 'Ma feuille'!A1), ou le nom d'une plage.

## Sélectionner par type

Choisissez **Sélectionner par type** dans le menu **Rechercher** pour sélectionner des cellules selon leur nature :

- **Cellules vides**, **Constantes** (saisies), **Formules** et **Cellules contenant une erreur**. Pour les constantes et les formules, vous pouvez exclure les nombres, le texte, VRAI et FAUX, ou les erreurs.
- **Le bloc autour de la cellule contenant le curseur** et **La dernière cellule utilisée**.
- **La formule matricielle où se trouve le curseur** sélectionne toutes les cellules de cette formule matricielle.
- **Cellules différentes de la cellule active dans leur ligne** et **dans leur colonne** trouvent les cellules qui ne ressemblent pas à la cellule contenant le curseur. Sélectionnez d'abord un bloc, puis placez le curseur sur la cellule à laquelle les autres doivent correspondre. Dans chaque ligne (ou colonne) du bloc, une cellule est retenue lorsqu'elle contient un nombre ou un texte différent, ou une formule qui n'est pas identique une fois que ses références de cellules sont décalées vers l'emplacement de cette cellule. Une cellule contenant une formule est aussi retenue lorsque la cellule active n'en contient pas. Si vous ne sélectionnez qu'une seule cellule, JeffCrunch examine le bloc qui l'entoure.
- **Cellules affichées** (hors lignes ou colonnes masquées), **Cellules avec mise en forme conditionnelle** et **Cellules avec une règle de saisie** (toutes, ou seulement celles ayant la même règle que la cellule contenant le curseur).

Le premier bloc trouvé est sélectionné et les autres sont teintés ; la teinte disparaît lorsque vous cliquez sur une cellule.

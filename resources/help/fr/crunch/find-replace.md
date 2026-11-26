# Rechercher, remplacer et atteindre
<!-- keywords: rechercher, trouver, remplacer, modifier, chercher, rechercher quoi, remplacer par, remplacer tout, rechercher suivant, rechercher des mots, remplacer par, modifier tout, même majuscules, caractère générique, astérisque, point d'interrogation, tilde, respecter la casse, cellule entière, rechercher par format, format, remplacer dans les notes, atteindre, référence, nom, sélectionner par type, cellules vides, constantes, formules, erreurs, région active, matrice active, différences de ligne, différences de colonne, dernière cellule, cellules visibles, mises en forme conditionnelles, validation, Ctrl+F, Ctrl+H, F5, notes -->

## Rechercher

Appuyez sur **Ctrl+F** (ou choisissez **Rechercher** dans l'onglet Accueil, puis de nouveau **Rechercher**). Saisissez le texte dans **Rechercher quoi**, puis appuyez sur **Entrée** pour atteindre la cellule suivante qui le contient, ou choisissez **Rechercher tout** pour obtenir la liste de toutes les cellules correspondantes. Cliquez sur une ligne de la liste pour atteindre cette cellule.

| Option | Ce qu'elle fait |
| --- | --- |
| **Où** | Recherche dans cette feuille, ou dans toutes les feuilles affichées. Si plusieurs cellules sont sélectionnées, une recherche dans une seule feuille se limite à elles. |
| **Ordre** | Parcourt les cellules par lignes (de gauche à droite, puis vers le bas) ou par colonnes (de haut en bas, puis vers la droite). **Rechercher suivant** s'arrête à la première correspondance dans cet ordre. |
| **Rechercher dans** | **Formules** regarde ce qui est saisi dans chaque cellule, **Valeurs** ce qu'elle affiche, et **Notes** les notes attachées aux cellules. |
| **Format…** | Ne trouve que les cellules qui ont une mise en forme donnée. **Format…** ouvre la boîte de dialogue Format de cellule, lancée depuis la cellule où se trouve le curseur : ce que vous modifiez là est ce qui est recherché (une police en gras, une couleur de remplissage, un format de nombre). **Format à partir de la cellule** reprend toute la mise en forme de la cellule où se trouve le curseur, et **Effacer le format** revient à une recherche portant sur le texte seul. La boîte de dialogue indique ce qui est recherché, et le signale si la mise en forme choisie ne contient rien. L'apparence qu'une mise en forme conditionnelle donne à une cellule n'est pas recherchée, seule la mise en forme propre à la cellule l'est. Si **Rechercher quoi** est vide, **Rechercher tout** et **Rechercher suivant** trouvent les cellules qui ont la mise en forme ; si du texte est saisi, une cellule doit avoir les deux. |
| **Respecter la casse** | Les majuscules et les minuscules doivent être identiques. |
| **Cellule entière** | La cellule entière doit correspondre au texte, et non seulement une partie de celle-ci. |

Dans le texte saisi dans **Rechercher quoi**, **?** remplace un caractère quelconque, et **\*** remplace une suite de caractères quelconque (y compris aucun). Saisissez **~** devant ?, \* ou ~ pour rechercher le signe lui-même : **a~\*b** correspond à a\*b.

La recherche s'effectue par petites étapes, ce qui vous permet de continuer à travailler. **Échap** (ou **Annuler**) l'arrête, et la modification d'une cellule l'arrête également. Un texte très long coûte davantage à chacune de ces étapes qu'un texte court : une feuille contenant de longs textes laisse donc tout de même la fenêtre disponible.

Les notes suivent les cellules : après un tri, ou après l'insertion ou la suppression de lignes ou de colonnes, une recherche dans les notes retrouve chaque note sur la cellule où elle se trouve désormais. Une note attachée à une cellule supprimée n'est plus recherchée (elle réapparaît après Annuler). Si un classeur a été modifié de tant de manières que les notes ne peuvent plus être suivies, il vous est demandé de l'enregistrer d'abord ; un classeur ouvert à partir d'un ancien type de fichier ou d'un fichier texte conserve ses notes sous une forme que JeffCrunch ne recherche pas, et vous en êtes averti.

## Remplacer

Appuyez sur **Ctrl+H**, saisissez le nouveau texte dans **Remplacer par**, puis choisissez un bouton. **Remplacer** met le nouveau texte dans la cellule où se trouve le curseur si elle contient le texte, puis passe à la suivante. **Remplacer tout** effectue l'opération dans toutes les cellules qui le contiennent, en une seule étape que vous pouvez annuler ; une modification importante est appliquée par petites portions. Le nouveau texte est traité comme si vous l'aviez saisi : un nombre devient un nombre, et une formule modifiée reste une formule.

Pour remplacer dans les notes, choisissez **Notes** dans **Rechercher dans** sur la page Remplacer. **Remplacer tout** modifie alors les mots de toutes les notes qui contiennent le texte, sur cette feuille ou sur toutes les feuilles, en une seule étape que vous pouvez annuler ; **Remplacer** modifie la note de la cellule où se trouve le curseur, puis passe à la suivante. Une note qui est une conversation de commentaires n'est pas modifiée (modifiez-les dans l'onglet Révision), et le nombre de notes laissées en l'état vous est indiqué. Rien n'est modifié, et le motif vous est indiqué, si une feuille contenant une note à modifier est protégée, ou si une note ne contiendrait plus aucun mot ou plus de 32 767 caractères. Une note dont les mots sont remplacés perd la date à laquelle elle a été écrite, comme c'est le cas lorsque vous modifiez ses mots. Les notes sont enregistrées avec le classeur, dans toutes les formes qui en contiennent. **Valeurs** peut être recherché mais pas remplacé, car une valeur est le résultat d'un calcul de formule.

Rien n'est modifié, et le motif vous est indiqué, si une cellule qui serait modifiée est verrouillée sur une feuille protégée, fait partie d'une matrice ou d'une table de données, se trouve dans la première ligne d'un tableau, ou ne serait plus une formule.

## Atteindre

Appuyez sur **F5** (ou **Ctrl+G**), ou saisissez directement dans la zone Nom. Vous pouvez indiquer une cellule, comme B7, une plage, comme B3:D9, des colonnes ou des lignes entières (C:E, 4:9), une cellule d'une autre feuille (Feuille2!A1, ou 'Ma feuille'!A1), ou le nom d'une plage. Un nom désignant les mêmes cellules sur un ensemble de feuilles, comme Jan:Déc!B2, vous conduit à la première feuille de cet ensemble.

## Sélectionner par type

Choisissez **Sélectionner par type** dans le menu **Rechercher** pour sélectionner des cellules selon leur nature :

- **Cellules vides**, **Constantes** (saisies), **Formules**, et **Cellules contenant une erreur**. Pour les constantes et les formules, vous pouvez exclure les nombres, le texte, VRAI et FAUX, ou les erreurs.
- **La région active autour de la cellule active**, et **La dernière cellule utilisée**.
- **La matrice active dans laquelle se trouve le curseur** sélectionne toutes les cellules de cette formule matricielle.
- **Cellules différentes de la cellule active dans leur ligne** et **dans leur colonne** trouvent les cellules qui ne ressemblent pas à la cellule active. Sélectionnez d'abord une plage, puis placez le curseur sur la cellule à laquelle les autres doivent correspondre. Dans chaque ligne (ou colonne) de la plage, une cellule est retenue lorsqu'elle contient un nombre ou un texte différent, ou une formule qui n'est pas identique une fois ses noms de cellules décalés vers l'emplacement de cette cellule. Une cellule contenant une formule est également retenue lorsque la cellule active n'en contient pas. Si vous ne sélectionnez qu'une cellule, JeffCrunch examine la région active autour d'elle.
- **Cellules utilisées par les formules de la sélection** et **Cellules contenant des formules qui utilisent la sélection** sélectionnent les cellules de cette feuille utilisées par les formules des cellules sélectionnées, ou les cellules dont les formules utilisent les cellules sélectionnées. Si une seule cellule est sélectionnée, la recherche part de celle-ci. Cochez **suivre sur tous les niveaux** pour poursuivre à partir de chaque cellule trouvée (les cellules utilisées par celles-ci, et ainsi de suite) ; sans cette option, seul le premier niveau est sélectionné. La recherche part d'au plus 1 000 cellules sélectionnées, et la barre d'état indique si elle s'est arrêtée à cette limite.
- **Cellules visibles** (hors lignes ou colonnes masquées), **Cellules avec mise en forme conditionnelle**, et **Cellules avec une règle de saisie** (toutes, ou seulement celles ayant la même règle que la cellule active).

Le premier bloc trouvé est sélectionné, et les autres sont teintés ; la teinte disparaît lorsque vous cliquez sur une cellule.

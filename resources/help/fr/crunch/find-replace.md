# Rechercher, remplacer et atteindre
<!-- keywords: rechercher, remplacer, modifier, chercher, rechercher, rechercher quoi, remplacer par, remplacer tout, rechercher suivant, rechercher, modifier en, modifier tout, mêmes majuscules, joker, astérisque, point d'interrogation, tilde, respecter la casse, cellule entière, rechercher par format, format, remplacer dans les notes, atteindre, référence, nom, sélectionner par type, cellules vides, constantes, formules, erreurs, région active, formule matricielle active, différences de ligne, différences de colonne, dernière cellule, cellules visibles, mises en forme conditionnelles, validation, Ctrl+F, Ctrl+H, F5, notes -->

## Rechercher

Appuyez sur **Ctrl+F** (ou choisissez **Rechercher** dans l'onglet Accueil, puis de nouveau **Rechercher**). Saisissez le texte dans **Rechercher** et appuyez sur **Entrée** pour atteindre la cellule suivante qui le contient, ou choisissez **Rechercher tout** pour obtenir la liste de toutes les cellules correspondantes. Cliquez sur une ligne de la liste pour atteindre cette cellule.

| Option | Ce qu'elle fait |
| --- | --- |
| **Où** | Rechercher dans cette feuille, ou dans toutes les feuilles affichées. Si plusieurs cellules sont sélectionnées, une recherche sur une feuille se limite à elles. |
| **Ordre** | Parcourir les cellules par lignes (de gauche à droite, puis vers le bas) ou par colonnes (de haut en bas, puis vers la droite). **Rechercher suivant** s'arrête à la première occurrence dans cet ordre. |
| **Rechercher dans** | **Formules** recherche dans ce qui est saisi dans chaque cellule, **Valeurs** dans ce qui est affiché, et **Notes** dans les notes des cellules. |
| **Format…** | Ne trouve que les cellules qui ont une mise en forme donnée. **Format…** ouvre la boîte de dialogue Format de cellule, initialisée à partir de la cellule où se trouve le curseur : ce que vous modifiez là est ce qui est recherché (une police en gras, une couleur de remplissage, un format de nombre). **Format à partir de la cellule** reprend toute la mise en forme de la cellule où se trouve le curseur, et **Effacer le format** revient à une recherche portant uniquement sur le texte. La boîte de dialogue indique ce qui est recherché, et le signale si la mise en forme choisie ne contient rien. L'apparence donnée à une cellule par une mise en forme conditionnelle n'est pas recherchée, seule la mise en forme propre à la cellule l'est. Si **Rechercher** est vide, **Rechercher tout** et **Rechercher suivant** trouvent les cellules ayant la mise en forme ; si du texte est saisi, une cellule doit avoir les deux. |
| **Respecter la casse** | Les majuscules et les minuscules doivent être identiques. |
| **Cellule entière** | La cellule entière doit correspondre au texte, et non seulement une partie de celui-ci. |

Dans le texte saisi dans **Rechercher**, **?** remplace un caractère quelconque et **\*** remplace une suite quelconque de caractères (y compris aucun caractère). Saisissez **~** devant ?, \*, ou ~ pour rechercher ce signe lui-même : **a~\*b** correspond à a\*b.

Une recherche s'effectue par petites étapes, vous pouvez donc continuer à travailler. **Échap** (ou **Annuler**) l'interrompt, de même que la modification d'une cellule. Un texte très long coûte davantage à chaque étape qu'un texte court : une feuille contenant de longs textes laisse donc la fenêtre disponible.

Les notes des cellules suivent les cellules : après un tri, ou après l'insertion ou la suppression de lignes ou de colonnes, une recherche dans les notes trouve chaque note à la cellule où elle se trouve désormais. Une note placée sur une cellule supprimée n'est plus recherchée (elle réapparaît après Annuler). Si un classeur a été modifié de tant de façons que les notes ne peuvent plus être suivies, il vous est demandé de l'enregistrer d'abord ; un classeur ouvert à partir d'un ancien type de fichier ou d'un fichier texte conserve ses notes sous une forme que JeffCrunch ne recherche pas, et vous en êtes averti.

## Remplacer

Appuyez sur **Ctrl+H**, saisissez le nouveau texte dans **Remplacer par**, puis choisissez un bouton. **Remplacer** place le nouveau texte dans la cellule où se trouve le curseur si elle contient le texte, puis passe à la suivante. **Remplacer tout** le fait dans toutes les cellules qui le contiennent, en une seule étape que vous pouvez annuler ; une modification importante est effectuée par petites portions. Le nouveau texte est saisi comme si vous l'aviez tapé : un nombre devient un nombre, et une formule modifiée reste une formule.

Pour remplacer dans les notes, choisissez **Notes** dans **Rechercher dans** sous l'onglet Remplacer. **Remplacer tout** modifie alors les mots de chaque note qui contient le texte, sur cette feuille ou sur toutes les feuilles, en une seule étape que vous pouvez annuler, et **Remplacer** modifie la note de la cellule où se trouve le curseur, puis passe à la suivante. Une note qui est une conversation de commentaires est laissée telle quelle (modifiez-les dans l'onglet Révision), et le nombre de notes ignorées vous est indiqué. Rien n'est modifié, et la raison vous est indiquée, si une feuille contenant une note à modifier est protégée, ou si une note se retrouverait sans mot ou avec plus de 32 767 caractères. Une note dont les mots sont remplacés perd la date à laquelle elle a été écrite, comme c'est le cas lorsque vous modifiez ses mots. Les notes sont enregistrées avec le classeur, sous toutes les formes qui en contiennent. Les **valeurs** peuvent être recherchées mais non remplacées, car une valeur est le résultat d'un calcul de formule.

Rien n'est modifié, et la raison vous est indiquée, si une cellule qui devrait être modifiée est verrouillée sur une feuille protégée, fait partie d'une formule matricielle ou d'une table de données, se trouve dans la première ligne d'un tableau, ou ne serait plus une formule.

## Atteindre

Appuyez sur **F5** (ou **Ctrl+G**), ou saisissez directement dans la zone Nom. Vous pouvez indiquer une cellule, comme B7, une plage, comme B3:D9, des colonnes ou des lignes entières (C:E, 4:9), une cellule d'une autre feuille (Feuil2!A1, ou 'Ma feuille'!A1), ou le nom d'une plage.

## Sélectionner par type

Choisissez **Sélectionner par type** dans le menu **Rechercher** pour sélectionner des cellules selon leur nature :

- **Cellules vides**, **Constantes** (saisies), **Formules** et **Cellules affichant une erreur**. Pour les constantes et les formules, vous pouvez exclure les nombres, le texte, VRAI et FAUX, ou les erreurs.
- **La zone entourant la cellule active** et **La dernière cellule utilisée**.
- **La formule matricielle où se trouve le curseur** sélectionne toutes les cellules de cette formule matricielle.
- **Cellules différentes de la cellule active dans leur ligne** et **dans leur colonne** trouvent les cellules qui ne ressemblent pas à la cellule où se trouve le curseur. Sélectionnez d'abord une plage, puis placez le curseur sur la cellule que les autres doivent reproduire. Dans chaque ligne (ou colonne) de la plage, une cellule est retenue si elle contient un nombre ou un texte différent, ou une formule qui n'est pas identique une fois ses noms de cellules décalés vers l'emplacement de cette cellule. Une cellule contenant une formule est aussi retenue lorsque la cellule active n'en contient pas. Si vous ne sélectionnez qu'une seule cellule, JeffCrunch examine la zone qui l'entoure.
- **Cellules affichées** (hors lignes ou colonnes masquées), **Cellules avec mise en forme conditionnelle**, et **Cellules avec une règle de saisie** (toutes, ou seulement celles ayant la même règle que la cellule où se trouve le curseur).

La première zone trouvée est sélectionnée et les autres sont teintées ; la teinte disparaît lorsque vous cliquez sur une cellule.

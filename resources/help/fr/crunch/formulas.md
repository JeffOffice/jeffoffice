# Affichage et calcul des formules
<!-- keywords: formule, formules, calculer, calcul, recalculer, fonction, fonctions, valeur, résultat, enregistré, F9, circulaire, référence circulaire, afficher les formules, barre de formule, erreur, pointer, pointage, cliquer sur une cellule, référence, référence absolue, F4 -->

Une **formule** est une cellule qui calcule sa valeur à partir d'autres cellules, par exemple =SOMME(B3:B8). La cellule affiche la valeur, et la **barre de formule** affiche la formule.

## Ce que fait JeffCrunch des formules

Lorsque vous ouvrez un classeur, le fichier contient la dernière valeur de chaque formule, ce qui permet d'afficher la feuille immédiatement. JeffCrunch recalcule ensuite toutes les formules, quelques-unes à la fois, afin que la fenêtre reste réactive. Pendant ce temps, la barre d'état indique **Calcul** avec un pourcentage. Une fois le calcul terminé, les cellules contiennent les résultats calculés par JeffCrunch. Pour tout recalculer, cliquez sur **Tout recalculer** dans l'onglet **Formules**, ou appuyez sur **F9**.

## Lorsque JeffCrunch ne peut pas calculer une formule

JeffCrunch propose plus de 200 fonctions, et ce nombre continue d'augmenter. Une formule qui utilise une fonction que JeffCrunch ne connaît pas encore conserve la valeur enregistrée pour elle dans le fichier. Il en va de même pour une formule qui fait référence à un autre classeur. Les cellules qui en dépendent sont calculées à partir des valeurs enregistrées.

## MOD : le reste

**MOD(n, d)** renvoie ce qui reste de **n** après en avoir retiré **d** le plus grand nombre entier de fois possible : le résultat vaut n - d * ENT(n / d). Exemple : **=MOD(10, 3)** vaut 1, et **=MOD(-10, 3)** vaut 2, car ENT(-10 / 3) vaut -4 et -10 - 3 * -4 vaut 2. Le résultat a le signe de **d**. Si **d** vaut 0, le résultat est #DIV/0!.

## Afficher les formules

Cliquez sur **Afficher les formules** dans l'onglet **Formules**, ou appuyez sur **Ctrl+`** (sur Mac, la touche Contrôle et non la touche Commande), pour afficher la formule de chaque cellule à la place de sa valeur. Les colonnes qui contiennent des formules sont élargies, dans une certaine limite, afin que les formules s'affichent en entier ; elles reprennent leur largeur lorsque vous désactivez cet affichage, et les largeurs enregistrées dans votre fichier ne sont pas modifiées.

## Modifier des formules

Pour modifier une formule, sélectionnez sa cellule, appuyez sur **F2**, modifiez-la, puis appuyez sur **Entrée**. Vous pouvez aussi la modifier dans la barre de formule. Lorsque vous modifiez une cellule, les formules qui en dépendent sont recalculées en arrière-plan. Voir [Saisir du texte dans les cellules](edit-cells).

## Écrire une formule

Tapez **=** suivi des premières lettres d'une fonction : une courte liste affiche les fonctions et les [noms](names) qui commencent ainsi, chacun accompagné d'une phrase d'explication. Appuyez sur **Tab** pour choisir l'élément mis en évidence. Utilisez les touches **Flèche bas** et **Flèche haut** pour en mettre un autre en évidence, puis appuyez sur **Tab** ou **Entrée**. **Échap** ferme la liste. Dès que vous êtes entre les parenthèses d'une fonction, ses arguments s'affichent sous la cellule, celui que vous êtes en train de taper apparaissant en gras. La liste et les arguments sont les mêmes que ceux qu'affiche **Choisir une fonction**. Les arguments sont séparés par des virgules, ou par des points-virgules si votre langue utilise la virgule comme séparateur décimal.

## Pointer sur des cellules

Au lieu de taper l'adresse d'une cellule, vous pouvez la pointer. Pendant la saisie d'une formule, placez le curseur à un endroit où une cellule peut figurer (juste après **=**, un opérateur comme **+**, une parenthèse ouvrante ou une virgule) et cliquez sur la cellule. Son adresse est écrite automatiquement. Faites glisser la souris sur des cellules pour écrire une plage comme B2:D5. Si vous cliquez à nouveau avant de taper autre chose, la nouvelle cellule remplace celle qui vient d'être écrite. Maintenez la touche **Ctrl** enfoncée et cliquez pour ajouter une autre adresse après une virgule. Cliquez sur une lettre de colonne pour écrire une colonne entière comme A:A, ou sur un numéro de ligne pour écrire une ligne entière comme 3:3, et faites glisser le long des lettres ou des numéros pour obtenir B:D ou 2:5.

Les touches de direction fonctionnent aussi. Appuyez sur une touche de direction : un pointeur s'éloigne de la cellule dans laquelle vous tapez, et son adresse est écrite ; appuyez à nouveau sur les touches de direction pour le déplacer, ou maintenez la touche **Maj** enfoncée pour étendre le pointeur à une plage. La barre d'état indique **Pointer** lorsque les touches de direction servent à pointer, **Entrer** pendant la saisie, et **Modifier** lorsque les touches de direction déplacent le curseur dans le texte. Appuyez sur **F2** pour basculer entre le pointage et la modification du texte.

Pour pointer sur une cellule d'une autre feuille, cliquez sur l'onglet de cette feuille alors que le curseur se trouve à un endroit où une cellule peut figurer, puis cliquez sur la cellule. Le nom de la feuille est écrit avec l'adresse, entre apostrophes si nécessaire, comme dans 'Ma feuille'!B3. **Entrée** vous ramène à la feuille de départ et place la formule dans sa cellule. **Échap** annule tout, y compris le changement de feuille.

Vous pouvez aussi faire glisser la bordure d'une plage encadrée sur la feuille pour déplacer cette adresse, ou faire glisser son petit carré d'angle pour l'étendre ; la formule est modifiée au fur et à mesure.

Appuyez sur **F4** lorsque le curseur se trouve dans une adresse ou juste après pour en figer certaines parties à l'aide du signe dollar : A1 devient $A$1, puis A$1, puis $A1, puis de nouveau A1.

Chaque adresse de la formule est colorée dans le texte, et les cellules qu'elle désigne sont encadrées de la même couleur sur la feuille : vous voyez ainsi d'un coup d'œil ce que lit la formule. Un clic après une adresse complète, par exemple =A1, termine la saisie et sélectionne la cellule sur laquelle vous avez cliqué, comme pour tout autre texte.

## Tables de données

Une table de données (table d'hypothèses) montre le résultat d'une formule pour de nombreuses valeurs d'une ou deux cellules. Ouvrez un classeur qui en contient une, et JeffCrunch calcule ses cellules : pour une table à deux entrées, la formule se trouve dans la cellule située dans le coin supérieur gauche de la table, les valeurs à tester en ligne sont placées dans la cellule d'entrée en ligne, et celles en colonne dans la cellule d'entrée en colonne. Pour une table à une entrée, les valeurs sont placées soit dans la colonne de gauche (les formules se trouvant alors dans la ligne au-dessus), soit dans la ligne du haut (les formules se trouvant alors dans la colonne de gauche). Chaque cellule de la table contient le résultat de la formule, et de toutes les formules qui dépendent de la cellule d'entrée, lorsque l'entrée prend cette valeur ; rien d'autre n'est modifié dans le classeur. En mode **Automatique**, les tables sont calculées avec les formules, puis les formules qui lisent les cellules d'une table sont mises à jour. La création d'une nouvelle table de données n'est pas encore proposée. Une table qui ne peut pas être calculée (parce qu'une formule dépendant de l'entrée utilise une fonction que JeffCrunch ne connaît pas, ou change d'elle-même, comme ALEA ou MAINTENANT) conserve les valeurs enregistrées dans son fichier.

## Calcul manuel

Cliquez sur **Mode de calcul** dans l'onglet **Formules** pour choisir comment le classeur calcule ses formules :

- **Automatique** calcule les formules à chaque modification de cellule.
- **Automatique, sauf les tableaux** fait de même, mais laisse les cellules des tables de données inchangées jusqu'à ce que vous appuyiez sur **F9**.
- **Manuel** ne calcule rien, tables de données comprises, tant que vous ne le demandez pas. Après une modification, la barre d'état indique **Non à jour**. Appuyez sur **F9** (**Tout recalculer**) pour tout calculer, ou sur **Maj+F9** (**Recalculer cette feuille**) pour calculer les formules de la feuille active, ainsi que tout ce qui était en attente. Un classeur réglé sur Manuel s'ouvre avec les résultats enregistrés dans son fichier.

Ce choix est enregistré avec le classeur, et il constitue une étape que vous pouvez annuler.

## Formules qui font référence à elles-mêmes

Une formule qui lit son propre résultat, directement ou par l'intermédiaire d'autres cellules, ne peut pas être calculée normalement. JeffCrunch vous en avertit une fois, indique les cellules concernées et les encadre sur la feuille ; elles conservent leur valeur précédente. Cliquez sur **Références circulaires** dans l'onglet **Formules** pour en afficher la liste, puis double-cliquez sur une cellule de la liste pour l'atteindre.

Certains modèles ont besoin d'une telle boucle volontairement. Choisissez **Calcul itératif des formules circulaires** dans le menu **Mode de calcul** : les formules sont alors recalculées de façon répétée, chacune lisant les dernières valeurs des autres, jusqu'à ce qu'aucune valeur ne varie plus que l'écart que vous indiquez ou que le nombre d'itérations autorisé soit atteint.

## Voir ce que lit une formule

Sélectionnez une cellule et cliquez sur **Repérer les antécédents** pour tracer des flèches depuis les cellules que lit sa formule. Cliquez à nouveau pour remonter d'un niveau supplémentaire. **Repérer les dépendants** trace des flèches vers les formules qui lisent la cellule, chaque clic avançant d'un niveau. Une petite page représente une cellule située sur une autre feuille. Au maximum 2 000 flèches sont tracées, et la barre d'état signale s'il y en avait davantage. **Supprimer les flèches** les efface, de même que toute modification du classeur.

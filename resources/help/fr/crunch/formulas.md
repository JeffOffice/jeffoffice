# Affichage et calcul des formules
<!-- keywords: formule, formules, calculer, calcul, recalculer, recalcul, fonction, fonctions, valeur, résultat, enregistré, F9, circulaire, référence circulaire, afficher les formules, barre de formule, erreur -->

Une **formule** est une cellule qui calcule sa valeur à partir d'autres cellules, par exemple =SOMME(B3:B8). La cellule affiche la valeur, et la **barre de formule** affiche la formule.

## Traitement des formules par JeffCrunch

Lorsque vous ouvrez un classeur, le fichier contient la dernière valeur de chaque formule, ce qui permet d'afficher immédiatement la feuille. JeffCrunch recalcule ensuite toutes les formules, quelques-unes à la fois, pour que la fenêtre reste réactive. Pendant ce temps, la barre d'état affiche **Calcul en cours** avec un pourcentage. Une fois le calcul terminé, les cellules contiennent les résultats calculés par JeffCrunch. Pour tout recalculer, cliquez sur **Tout recalculer** dans l'onglet **Formules**, ou appuyez sur **F9**.

## Formules que JeffCrunch ne peut pas calculer

JeffCrunch propose plus de 200 fonctions, et leur nombre continue d'augmenter. Une formule qui utilise une fonction dont JeffCrunch ne dispose pas encore conserve la valeur enregistrée dans le fichier. Il en va de même pour une formule qui fait référence à un autre classeur. Les cellules qui en dépendent sont calculées à partir des valeurs enregistrées.

## Afficher les formules

Cliquez sur **Afficher les formules** dans l'onglet **Formules**, ou appuyez sur **Ctrl+`** (sur Mac, la touche Contrôle et non la touche Commande), pour voir la formule de chaque cellule à la place de sa valeur. Les colonnes qui contiennent des formules sont élargies, dans une certaine limite, pour que les formules s'affichent en entier ; elles retrouvent leur largeur lorsque vous désactivez l'option, et les largeurs enregistrées dans votre fichier ne sont pas modifiées.

## Modifier des formules

Pour modifier une formule, sélectionnez sa cellule, appuyez sur **F2**, modifiez-la, puis appuyez sur **Entrée**. Vous pouvez aussi la modifier dans la barre de formule. Lorsque vous modifiez une cellule, les formules qui en dépendent sont recalculées en arrière-plan. Voir [Saisir dans les cellules](edit-cells).

## Écrire une formule

Tapez **=** et les premières lettres d'une fonction : une courte liste affiche les fonctions et les [noms](names) qui commencent ainsi, accompagnés d'une phrase de description. Appuyez sur **Tab** pour choisir celui qui est en surbrillance. Utilisez les touches **Bas** et **Haut** pour en mettre un autre en surbrillance, puis appuyez sur **Tab** ou **Entrée**. **Échap** ferme la liste. Une fois à l'intérieur des parenthèses d'une fonction, ses arguments s'affichent sous la cellule, celui que vous saisissez apparaissant en gras. La liste et les arguments sont les mêmes que ceux qu'affiche **Choisir une fonction**. Les arguments sont séparés par des virgules, ou par des points-virgules si votre langue utilise la virgule comme séparateur décimal.

## Tables de données

Une table de données (table d'hypothèses) montre le résultat d'une formule pour de nombreuses valeurs d'une ou deux cellules. Ouvrez un classeur qui en contient une, et JeffCrunch calcule ses cellules : pour une table à deux entrées, la formule se trouve dans la cellule située dans l'angle supérieur gauche de la table, les valeurs à tester en ligne sont placées dans la cellule d'entrée de ligne, et celles en colonne dans la cellule d'entrée de colonne. Pour une table à une entrée, les valeurs sont placées dans la colonne située à sa gauche (les formules étant dans la ligne au-dessus) ou dans la ligne située au-dessus (les formules étant dans la colonne à sa gauche). Chaque cellule de la table contient le résultat de la formule, et de toutes les formules qui dépendent de la cellule d'entrée, lorsque l'entrée prend cette valeur ; rien d'autre n'est modifié dans le classeur. En mode **Automatique**, les tables sont calculées avec les formules, puis les formules qui lisent les cellules d'une table sont calculées à leur tour. La création d'une nouvelle table de données n'est pas encore proposée. Une table qui ne peut pas être calculée (parce qu'une formule dépendant de l'entrée utilise une fonction dont JeffCrunch ne dispose pas, ou change d'elle-même, comme ALEA ou MAINTENANT) conserve les valeurs enregistrées dans son fichier.

## Calcul manuel

Cliquez sur **Mode de calcul** dans l'onglet **Formules** pour choisir comment le classeur calcule ses formules :

- **Automatique** les calcule à mesure que vous modifiez les cellules.
- **Automatique, sauf les tableaux** fait de même, mais laisse les cellules des tables de données inchangées jusqu'à ce que vous appuyiez sur **F9**.
- **Manuel** ne calcule rien, tables de données comprises, tant que vous ne le demandez pas. Après une modification, la barre d'état affiche **Non à jour**. Appuyez sur **F9** (**Tout recalculer**) pour tout calculer, ou sur **Maj+F9** (**Recalculer cette feuille**) pour calculer les formules de la feuille active, ainsi que tout ce qui était en attente. Un classeur réglé sur Manuel s'ouvre avec les résultats enregistrés dans son fichier.

Ce choix est enregistré avec le classeur, et il peut être annulé en une seule étape.

## Formules qui font référence à elles-mêmes

Une formule qui lit son propre résultat, directement ou par l'intermédiaire d'autres cellules, ne peut pas être calculée de façon normale. JeffCrunch vous en avertit une fois, indique les cellules concernées et les entoure sur la feuille ; elles conservent leurs valeurs précédentes. Cliquez sur **Références circulaires** dans l'onglet **Formules** pour afficher la liste, puis double-cliquez sur une cellule de la liste pour y accéder.

Certains modèles nécessitent volontairement une telle boucle. Choisissez **Calcul itératif des formules circulaires** dans le menu **Mode de calcul** : les formules sont alors recalculées à plusieurs reprises, chacune lisant les dernières valeurs des autres, jusqu'à ce qu'aucune valeur ne varie de plus de l'écart indiqué ou que le nombre d'itérations autorisé soit atteint.

## Voir ce que lit une formule

Sélectionnez une cellule et cliquez sur **Repérer les antécédents** pour tracer des flèches depuis les cellules lues par sa formule. Cliquez de nouveau pour remonter d'un niveau supplémentaire. **Repérer les dépendants** trace des flèches vers les formules qui lisent la cellule, chaque clic avançant d'un niveau. Une petite page représente une cellule située sur une autre feuille. Au maximum 2 000 flèches sont tracées, et la barre d'état signale s'il y en avait davantage. **Supprimer les flèches** les efface, de même que toute modification du classeur.

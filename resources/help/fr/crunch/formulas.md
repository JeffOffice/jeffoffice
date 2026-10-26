# Affichage et calcul des formules
<!-- keywords: formule, formules, calculer, calcul, recalculer, recalcul, fonction, fonctions, valeur, résultat, valeur stockée, F9, circulaire, référence circulaire, afficher les formules, barre de formule, erreur -->

Une **formule** est une cellule qui calcule sa valeur à partir d'autres cellules, par exemple =SOMME(B3:B8). La cellule affiche la valeur, et la **barre de formule** affiche la formule.

## Ce que JeffCrunch en fait

Lorsque vous ouvrez un classeur, le fichier contient la dernière valeur de chaque formule, ce qui permet d'afficher la feuille immédiatement. JeffCrunch recalcule ensuite chaque formule, quelques-unes à la fois, afin que la fenêtre reste réactive. Pendant ce temps, la barre d'état affiche **Calcul en cours** avec un pourcentage. Une fois le calcul terminé, les cellules contiennent les résultats calculés par JeffCrunch. Pour tout recalculer, cliquez sur **Tout recalculer** sous l'onglet **Formules**, ou appuyez sur **F9**.

## Quand JeffCrunch ne peut pas calculer une formule

JeffCrunch propose plus de 200 fonctions, et ce nombre continue d'augmenter. Une formule qui utilise une fonction dont JeffCrunch ne dispose pas encore conserve la valeur enregistrée dans le fichier. Il en va de même pour une formule qui fait référence à un autre classeur. Les cellules qui en dépendent sont calculées à partir des valeurs enregistrées.

## Afficher les formules

Cliquez sur **Afficher les formules** sous l'onglet **Formules**, ou appuyez sur **Ctrl+`** (sur Mac, la touche Contrôle et non la touche Commande), pour voir la formule de chaque cellule à la place de sa valeur.

## Modifier des formules

Pour modifier une formule, sélectionnez sa cellule, appuyez sur **F2**, modifiez-la, puis appuyez sur **Entrée**. Vous pouvez aussi la modifier dans la barre de formule. Lorsque vous modifiez une cellule, les formules qui en dépendent sont recalculées en arrière-plan. Voir [Saisir dans les cellules](edit-cells).

## Écrire une formule

Tapez **=** suivi des premières lettres d'une fonction : une courte liste affiche les fonctions et les [noms](names) qui commencent ainsi, avec une phrase de description pour chacun. Appuyez sur **Tab** pour choisir l'élément mis en évidence. Utilisez les touches **Flèche bas** et **Flèche haut** pour en mettre un autre en évidence, puis appuyez sur **Tab** ou **Entrée**. **Échap** ferme la liste. Une fois entre les parenthèses d'une fonction, ses arguments s'affichent sous la cellule, celui que vous saisissez apparaissant en gras. La liste et les arguments sont les mêmes que ceux qu'affiche **Choisir une fonction**. Les arguments sont séparés par des virgules, ou par des points-virgules si votre langue utilise la virgule comme séparateur décimal.

## Calculer manuellement

Cliquez sur **Mode de calcul** sous l'onglet **Formules** pour choisir la façon dont le classeur calcule ses formules :

- **Automatique** les calcule à mesure que vous modifiez les cellules.
- **Automatique, sauf les tableaux** fait de même, et est conservé dans le fichier pour les programmes qui traitent les tables de données à part.
- **Manuel** ne calcule rien tant que vous ne le demandez pas. Après une modification, la barre d'état affiche **Non à jour**. Appuyez sur **F9** (**Tout recalculer**) pour tout calculer, ou sur **Maj+F9** (**Recalculer cette feuille**) pour calculer les formules de la feuille active ainsi que tout ce qui était en attente. Un classeur réglé sur Manuel s'ouvre avec les résultats contenus dans son fichier.

Ce choix est enregistré avec le classeur, et il peut être annulé en une seule étape.

## Formules qui font référence à elles-mêmes

Une formule qui fait référence à son propre résultat, directement ou par l'intermédiaire d'autres cellules, ne peut pas être calculée de la manière habituelle. JeffCrunch vous en avertit une fois, indique les cellules concernées et les entoure sur la feuille ; elles conservent leurs valeurs précédentes. Cliquez sur **Références circulaires** sous l'onglet **Formules** pour en afficher la liste, puis double-cliquez sur une cellule de la liste pour y accéder.

Certains modèles nécessitent volontairement une telle boucle. Choisissez **Calcul itératif des formules circulaires** dans le menu **Mode de calcul** : les formules sont alors recalculées à plusieurs reprises, chacune lisant les dernières valeurs des autres, jusqu'à ce qu'aucune valeur ne varie plus que l'écart indiqué ou que le nombre d'itérations autorisé soit atteint.

## Voir ce que lit une formule

Sélectionnez une cellule et cliquez sur **Repérer les antécédents** pour tracer des flèches depuis les cellules que lit sa formule. Cliquez de nouveau pour remonter d'un niveau supplémentaire. **Repérer les dépendants** trace des flèches vers les formules qui lisent la cellule, et chaque clic avance d'un niveau. Une petite page représente une cellule située sur une autre feuille. Au maximum 2 000 flèches sont tracées, et la barre d'état signale s'il y en avait davantage. **Supprimer les flèches** les efface, de même que toute modification du classeur.

# Comment les formules sont affichées et calculées
<!-- keywords: formule, formules, calculer, calcul, recalculer, fonction, fonctions, valeur, résultat, enregistré, F9, circulaire, afficher les formules, barre de formule, erreur, pointer, pointage, cliquer sur une cellule, référence, F4 -->

Une **formule** est une cellule qui calcule sa valeur à partir d'autres cellules, par exemple =SOMME(B3:B8). La cellule affiche la valeur, et la **barre de formule** affiche la formule.

## Ce que fait JeffCrunch avec les formules

Lorsque vous ouvrez un classeur, le fichier contient la dernière valeur de chaque formule, si bien que la feuille peut s'afficher immédiatement. Ensuite, JeffCrunch recalcule chaque formule, quelques-unes à la fois, afin que la fenêtre reste fluide. La barre d'état affiche **Calcul en cours** avec un pourcentage pendant ce travail. Une fois le calcul terminé, les cellules contiennent les résultats de JeffCrunch. Pour tout recalculer, cliquez sur **Recalculer tout** dans l'onglet **Formules**, ou appuyez sur **F9**.

## Lorsque JeffCrunch ne peut pas calculer une formule

JeffCrunch dispose de plus de 400 fonctions, et ce nombre continue d'augmenter. Une formule qui utilise une fonction que JeffCrunch ne connaît pas encore conserve la valeur enregistrée dans le fichier. Il en va de même pour une formule qui fait référence à un autre classeur. Les cellules qui dépendent de ces formules sont calculées à partir des valeurs enregistrées.

## Fonctions qui demandent quelques précisions

- **Obligations et emprunts.** PRIX, RENDEMENT, DURATION, PRIX.PCOUPON.IMPAIR et les autres fonctions d'obligations comptent les jours selon la **base** que vous indiquez (0 : 30 jours par mois sur une année de 360 jours ; 1 : jours réels ; et ainsi de suite). RENDEMENT, RENDEMENT.PCOUPON.IMPAIR, RENDEMENT.DCOUPON.IMPAIR et TAUX trouvent leur résultat en testant des valeurs jusqu'à ce que le prix ou le paiement corresponde. Elles s'arrêtent lorsque le résultat est exact à environ 15 chiffres près, si bien que les derniers chiffres peuvent différer de ceux d'un autre programme.
- **Lois de probabilité et tests.** Les fonctions qui se terminent par .LOI, .INVERSE, .TEST, ainsi que les anciens noms comme LOI.NORMALE et LOI.STUDENT, donnent les mêmes résultats : un ancien nom est conservé pour que les anciens fichiers continuent de fonctionner. Les fonctions *.INVERSE sont également déterminées en testant des valeurs.
- **Listes qui remplissent plusieurs cellules.** TRI, TRIERPAR, FILTRE, UNIQUE, SEQUENCE, ALEA.MATRICE, RECHERCHEX, MMULT, INVERSEMAT et DROITEREG renvoient un bloc de valeurs. Saisissez-les comme formule matricielle (**Ctrl+Maj+Entrée**) sur les cellules qu'elles doivent remplir. Il n'est pas encore possible qu'une formule déborde dans les cellules voisines.
- **Textes dans d'autres écritures.** NBCARB, GAUCHEB, STXTB, DROITEB, TROUVERB, CHERCHERB et REMPLACERB comptent chaque caractère pour un seul, comme sur un ordinateur qui n'est pas configuré pour le texte à deux octets. PHONETIQUE donne le texte de la cellule, car JeffCrunch ne conserve pas les lectures du texte japonais.
- **CONVERT.** Les unités s'écrivent sous forme de texte, avec des majuscules et des minuscules telles qu'indiquées dans la liste de la fonction : « kg », « lbm », « mi », « F ». Un préfixe métrique peut précéder une unité qui en accepte un, comme « km » ou « mL ».

## MOD : le reste

**MOD(n ; d)** donne ce qui reste de **n** après que le plus grand nombre entier de fois **d** a été retiré : c'est n - d * ENT(n / d). Exemple : **=MOD(10 ; 3)** vaut 1, et **=MOD(-10 ; 3)** vaut 2, car ENT(-10 / 3) vaut -4 et -10 - 3 * -4 vaut 2. Le résultat a le signe de **d**. Si **d** vaut 0, le résultat est #DIV/0!.

*Remarque : dans les exemples, le séparateur d'arguments est le point-virgule, comme dans la version française de l'application.*

## Afficher les formules

Cliquez sur **Afficher les formules** dans l'onglet **Formules**, ou appuyez sur **Ctrl+`** (sur Mac, la touche Contrôle et non la touche Commande), pour voir la formule de chaque cellule à la place de sa valeur. Les colonnes qui contiennent des formules sont élargies, dans une certaine limite, afin que les formules s'affichent en entier ; elles reprennent leur largeur initiale lorsque vous désactivez l'option, et les largeurs enregistrées dans votre fichier ne sont pas modifiées.

## Modifier une formule

Pour modifier une formule, sélectionnez sa cellule, appuyez sur **F2**, modifiez-la, puis appuyez sur **Entrée**. Vous pouvez aussi la modifier dans la barre de formule. Lorsque vous changez une cellule, les formules qui en dépendent sont recalculées en arrière-plan. Voir [Saisir dans les cellules](edit-cells).

## Écrire une formule

Tapez **=** puis les premières lettres d'une fonction : une courte liste affiche les fonctions et les [noms](names) qui commencent ainsi, accompagnés d'une phrase les décrivant. Appuyez sur **Tab** pour choisir celle qui est sélectionnée. Utilisez les flèches **Bas** et **Haut** pour en sélectionner une autre, puis **Tab** ou **Entrée**. **Échap** ferme la liste. Une fois à l'intérieur des parenthèses d'une fonction, ses arguments s'affichent sous la cellule, avec celui que vous saisissez en gras. La liste et les arguments sont les mêmes que ceux qu'affiche **Choisir une fonction**. Les arguments sont séparés par des virgules, ou par des points-virgules lorsque votre langue utilise la virgule comme séparateur décimal.

## Pointer les cellules

Au lieu de saisir l'adresse d'une cellule, vous pouvez la pointer. Pendant la saisie d'une formule, placez le curseur à un endroit où une cellule peut être insérée (juste après **=**, après un opérateur tel que **+**, après une parenthèse ouvrante ou une virgule), puis cliquez sur la cellule. Son adresse est écrite pour vous. Faites glisser sur des cellules pour écrire un bloc, par exemple B2:D5. Cliquez de nouveau avant de saisir autre chose, et la nouvelle cellule remplace celle qui vient d'être écrite. Maintenez **Ctrl** et cliquez pour ajouter une autre adresse après une virgule. Cliquez sur une lettre de colonne pour écrire une colonne entière, par exemple A:A, ou sur un numéro de ligne pour écrire une ligne entière, par exemple 3:3 ; faites glisser le long des lettres ou des numéros pour écrire B:D ou 2:5.

Les touches fléchées fonctionnent aussi. Appuyez sur une flèche : un pointeur se déplace à partir de la cellule dans laquelle vous saisissez, et son adresse est écrite. Appuyez sur d'autres flèches pour poursuivre, ou maintenez **Maj** pour étendre le pointeur sur un bloc. La barre d'état affiche **Pointer** lorsque les touches fléchées pointent, **Entrée** pendant la saisie, et **Modification** lorsque les touches fléchées déplacent le curseur dans le texte. Appuyez sur **F2** pour passer du mode pointage au mode modification du texte.

Pour pointer une cellule d'une autre feuille, cliquez sur l'onglet de cette feuille pendant que le curseur se trouve à un endroit où une cellule peut être insérée, puis cliquez sur la cellule. Le nom de la feuille est écrit avec l'adresse, entre guillemets si nécessaire, comme dans 'Ma feuille'!B3. **Entrée** vous ramène à la feuille de départ et place la formule dans sa cellule. **Échap** annule tout, y compris le changement de feuille.

Vous pouvez aussi faire glisser la bordure d'un bloc encadré sur la feuille pour déplacer cette adresse, ou faire glisser son carré d'angle pour l'étendre ; la formule change pendant que vous faites glisser.

Appuyez sur **F4** lorsque le curseur se trouve dans une adresse ou juste après elle, pour fixer certaines parties de l'adresse avec un signe dollar : A1 devient $A$1, puis A$1, puis $A1, puis de nouveau A1.

Chaque adresse de la formule est colorée dans le texte, et les cellules qu'elle désigne sont encadrées sur la feuille dans la même couleur, si bien que vous voyez d'un coup d'œil ce que lit la formule. Un clic après une adresse complète, comme dans =A1, termine la saisie et sélectionne la cellule sur laquelle vous avez cliqué, comme pour tout autre texte.

## Tables de données

Une table de données (table d'hypothèses) montre le résultat d'une formule pour de nombreuses valeurs d'une ou de deux cellules. Lorsque vous ouvrez un classeur qui en contient une, JeffCrunch calcule ses cellules : pour une table à deux variables, la formule se trouve dans la cellule située en haut à gauche de la table, les valeurs à tester en ligne sont placées dans la cellule de saisie en ligne, et celles en colonne dans la cellule de saisie en colonne. Pour une table à une variable, les valeurs sont placées dans la colonne à gauche de la table (et les formules se trouvent dans la ligne au-dessus), ou en ligne au-dessus de la table (et les formules se trouvent dans la colonne à gauche). Chaque cellule de la table est le résultat de la formule, et de toutes les formules qui dépendent de la cellule de saisie, avec cette cellule réglée sur la valeur correspondante ; rien d'autre dans le classeur n'est modifié. En mode **Automatique**, les tables sont calculées avec les formules, et les formules qui lisent les cellules d'une table les suivent. La création d'une nouvelle table de données n'est pas encore proposée. Une table qui ne peut pas être calculée (une formule qui dépend de la saisie utilise une fonction que JeffCrunch ne connaît pas, ou change d'elle-même, comme ALEA ou MAINTENANT) conserve les valeurs contenues dans son fichier.

## Calculer manuellement

Cliquez sur **Mode de calcul** dans l'onglet **Formules** pour choisir la façon dont le classeur calcule ses formules :

- **Automatique** calcule les formules au fur et à mesure que vous modifiez les cellules.
- **Automatique, sauf les tableaux** fait de même, mais laisse les cellules des tables de données telles quelles jusqu'à ce que vous appuyiez sur **F9**.
- **Manuel** ne calcule rien, tables de données comprises, tant que vous ne le demandez pas. La barre d'état affiche **Non à jour** après une modification. Appuyez sur **F9** (**Recalculer tout**) pour tout calculer, ou sur **Maj+F9** (**Recalculer cette feuille**) pour calculer les formules de la feuille active, ainsi que tout ce qui attendait. Un classeur réglé sur Manuel s'ouvre avec les résultats contenus dans son fichier.

Le choix est enregistré avec le classeur, et il s'agit d'une étape que vous pouvez annuler.

## Formules qui se lisent elles-mêmes

Une formule qui lit son propre résultat, directement ou par l'intermédiaire d'autres cellules, ne peut pas être calculée de la façon habituelle. JeffCrunch vous le signale une fois, indique les cellules concernées et les encadre sur la feuille ; ces cellules conservent les valeurs qu'elles avaient. Cliquez sur **Références circulaires** dans l'onglet **Formules** pour afficher la liste, puis double-cliquez sur une cellule de cette liste pour vous y rendre.

Certains modèles ont besoin d'une telle boucle volontairement. Choisissez **Calcul itératif des formules circulaires** dans le menu **Mode de calcul** : les formules sont recalculées à plusieurs reprises, chacune lisant les dernières valeurs des autres, jusqu'à ce qu'aucune valeur ne change de plus que la quantité que vous indiquez, ou jusqu'à ce que le nombre de passages autorisé soit atteint.

## Voir ce que lit une formule

Sélectionnez une cellule, puis cliquez sur **Repérer les antécédents** pour tracer des flèches vers les cellules que lit sa formule. Cliquez de nouveau pour remonter d'un niveau. **Repérer les dépendants** trace des flèches vers les formules qui lisent la cellule, et chaque clic avance d'un niveau. Une petite page représente une cellule d'une autre feuille. Au maximum 2 000 flèches sont tracées, et la barre d'état l'indique lorsqu'il y en avait davantage. **Supprimer les flèches** les efface, et toute modification du classeur les efface également.

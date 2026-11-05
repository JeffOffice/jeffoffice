# Comment les formules sont affichées et calculées
<!-- keywords: formule, formules, calculer, calcul, recalculer, fonction, fonctions, valeur, résultat, enregistré, F9, circulaire, afficher les formules, barre de formule, erreur, pointer, pointage, cliquer sur une cellule, référence, F4, vérification des erreurs, ignorer l'erreur, évaluer la formule, étape, fenêtre espion, espion -->

Une **formule** est une cellule qui calcule sa valeur à partir d'autres cellules, par exemple =SOMME(B3:B8). La cellule affiche la valeur, et la **barre de formule** affiche la formule.

## Ce que fait JeffCrunch avec les formules

Lorsque vous ouvrez un classeur, le fichier contient la dernière valeur de chaque formule, si bien que la feuille peut s'afficher immédiatement. JeffCrunch recalcule ensuite chaque formule, par petits groupes, pour que la fenêtre reste fluide. La barre d'état affiche **Calcul en cours** avec un pourcentage pendant ce travail. Une fois le calcul terminé, les cellules contiennent les résultats de JeffCrunch. Pour tout recalculer, cliquez sur **Tout recalculer** dans l'onglet **Formules**, ou appuyez sur **F9**.

## Lorsque JeffCrunch ne peut pas calculer une formule

JeffCrunch dispose de plus de 400 fonctions, et ce nombre continue d'augmenter. Une formule qui utilise une fonction que JeffCrunch ne connaît pas encore conserve la valeur enregistrée dans le fichier. C'est aussi le cas d'une formule qui fait référence à un autre classeur. Les cellules qui en dépendent sont calculées à partir des valeurs enregistrées.

## Fonctions qui demandent quelques précisions

- **Obligations et emprunts.** PRIX, RENDEMENT, DURATION, PRIXPER, et les autres fonctions d'obligations comptent les jours selon la **base** que vous indiquez (0 correspond à 30 jours par mois dans une année de 360 jours, 1 aux jours réels, et ainsi de suite). RENDEMENT, RENDEMENTPER, RENDEMENTDERNIER et TAUX déterminent leur résultat par essais successifs de valeurs, jusqu'à ce que le prix ou le paiement corresponde. Elles s'arrêtent lorsque le résultat est exact à environ 15 chiffres significatifs ; les derniers chiffres peuvent donc différer d'un autre logiciel.
- **Lois de probabilité et tests.** Les fonctions qui se terminent par .LOI, .INVERSE, .TEST, ainsi que les anciens noms comme LOI.NORMALE et LOI.STUDENT, donnent les mêmes résultats : un ancien nom est conservé pour que les anciens fichiers continuent de fonctionner. Les fonctions *.INVERSE sont également déterminées par essais successifs de valeurs.
- **Listes qui remplissent plusieurs cellules.** TRIER, TRIERPAR, FILTRE, UNIQUE, SEQUENCE, ALEA.MATRICE, RECHERCHEX, MMULT, INVERSEMAT et DROITEREG donnent un bloc de valeurs. Saisissez-les comme formule matricielle (**Ctrl+Maj+Entrée**) sur les cellules à remplir. L'extension automatique d'une formule dans les cellules voisines n'est pas encore prise en charge.
- **Textes dans d'autres écritures.** LENB, GAUCHEB, STXT, DROITEB, TROUVERB, CHERCHERB et REMPLACERB comptent chaque caractère pour un seul, comme sur un ordinateur qui n'est pas configuré pour les textes à double octet. PHONETIQUE donne le texte de la cellule, car JeffCrunch ne conserve pas les lectures des textes japonais.
- **CONVERT.** Les unités s'écrivent sous forme de texte, avec les majuscules et les minuscules indiquées dans la liste de la fonction : « kg », « lbm », « mi », « F ». Un préfixe métrique peut être placé devant une unité qui l'accepte, comme « km » ou « mL ».

## MOD : le reste

**MOD(n ; d)** donne le reste de **n** après que le plus grand nombre entier de fois **d** en a été retiré : il est égal à n - d * ENT(n / d). Exemple : **=MOD(10 ; 3)** donne 1, et **=MOD(-10 ; 3)** donne 2, car ENT(-10 / 3) vaut -4 et -10 - 3 * -4 vaut 2. Le résultat a le signe de **d**. Si **d** vaut 0, le résultat est #DIV/0!.

## Afficher les formules

Cliquez sur **Afficher les formules** dans l'onglet **Formules**, ou appuyez sur **Ctrl+`** (sur un Mac, la touche Contrôle et non la touche Commande), pour voir la formule de chaque cellule à la place de sa valeur. Les colonnes qui contiennent des formules sont élargies, jusqu'à une certaine limite, afin que les formules s'affichent en entier ; elles reprennent leur largeur d'origine lorsque vous désactivez cette option, et les largeurs enregistrées dans votre fichier ne sont pas modifiées.

## Modifier une formule

Pour modifier une formule, sélectionnez sa cellule, appuyez sur **F2**, modifiez-la, puis appuyez sur **Entrée**. Vous pouvez aussi la modifier dans la barre de formule. Lorsque vous modifiez une cellule, les formules qui en dépendent sont recalculées en arrière-plan. Voir [Saisir dans les cellules](edit-cells).

## Écrire une formule

Tapez **=** puis les premières lettres d'une fonction : une courte liste affiche les fonctions et les [noms](names) qui commencent ainsi, accompagnés d'une phrase pour chacun. Appuyez sur **Tab** pour choisir celle qui est sélectionnée. Utilisez les flèches **Bas** et **Haut** pour en sélectionner une autre, puis **Tab** ou **Entrée**. **Échap** ferme la liste. Une fois entre les parenthèses d'une fonction, ses arguments s'affichent sous la cellule, celui que vous saisissez étant en gras. La liste et les arguments sont les mêmes que ceux qu'affiche **Insérer une fonction**. Les arguments sont séparés par des points-virgules, ou par des virgules lorsque votre langue utilise la virgule comme séparateur décimal.

## Pointer les cellules

Au lieu de taper l'adresse d'une cellule, vous pouvez la pointer. Pendant la saisie d'une formule, placez le curseur à un endroit où une cellule peut figurer (juste après **=**, un opérateur comme **+**, une parenthèse ouvrante ou une virgule), puis cliquez sur la cellule. Son adresse est écrite à votre place. Faites glisser sur des cellules pour écrire un bloc, par exemple B2:D5. Cliquez à nouveau avant de saisir autre chose, et la nouvelle cellule remplace celle qui vient d'être écrite. Maintenez **Ctrl** et cliquez pour ajouter une autre adresse après une virgule. Cliquez sur une lettre de colonne pour écrire une colonne entière, par exemple A:A, ou sur un numéro de ligne pour écrire une ligne entière, par exemple 3:3 ; faites glisser le long des lettres ou des numéros pour obtenir B:D ou 2:5.

Les touches fléchées fonctionnent aussi. Appuyez sur une flèche : un pointeur s'éloigne de la cellule en cours de saisie et son adresse est écrite ; appuyez sur d'autres flèches pour continuer, ou maintenez **Maj** pour étendre le pointeur sur un bloc. La barre d'état affiche **Pointage** pendant que les touches fléchées pointent, **Entrée** pendant la saisie, et **Modification** lorsque les touches fléchées déplacent le curseur dans le texte. Appuyez sur **F2** pour passer du pointage à la modification du texte.

Pour pointer une cellule d'une autre feuille, cliquez sur l'onglet de cette feuille pendant que le curseur se trouve à un endroit où une cellule peut figurer, puis cliquez sur la cellule. Le nom de la feuille est écrit avec l'adresse, entre guillemets simples si nécessaire, comme dans 'Ma feuille'!B3. **Entrée** vous ramène sur la feuille de départ et place la formule dans sa cellule. **Échap** annule tout, y compris le changement de feuille.

Vous pouvez aussi faire glisser la bordure d'un bloc encadré sur la feuille pour déplacer cette adresse, ou faire glisser son carré d'angle pour l'étendre ; la formule change pendant le glissement.

Appuyez sur **F4** avec le curseur dans une adresse ou juste après elle pour fixer certaines parties avec le signe dollar : A1 devient $A$1, puis A$1, puis $A1, puis de nouveau A1.

Chaque adresse de la formule est colorée dans le texte, et les cellules qu'elle désigne sont encadrées sur la feuille de la même couleur, ce qui permet de voir d'un coup d'œil ce que la formule lit. Un clic après une adresse complète, comme =A1, termine la saisie et sélectionne la cellule sur laquelle vous avez cliqué, comme pour tout autre texte.

## Tables de données

Une table de données (table de simulation) montre le résultat d'une formule pour de nombreuses valeurs d'une ou de deux cellules. Lorsque vous ouvrez un classeur qui en contient une, JeffCrunch calcule ses cellules : pour une table à deux variables, la formule se trouve dans la cellule en haut à gauche de la table, les valeurs à essayer en ligne sont placées dans la cellule de saisie en ligne, et celles en colonne dans la cellule de saisie en colonne. Pour une table à une variable, les valeurs sont placées dans la colonne à gauche de la table (et les formules se trouvent dans la ligne au-dessus) ou dans la ligne au-dessus de la table (et les formules se trouvent dans la colonne à gauche). Chaque cellule de la table est le résultat de la formule, ainsi que de chaque formule qui dépend de la cellule de saisie, celle-ci prenant la valeur correspondante ; rien d'autre dans le classeur n'est modifié. En mode **Automatique**, les tables sont calculées avec les formules, et les formules qui lisent les cellules d'une table les suivent. La création d'une nouvelle table de données n'est pas encore proposée. Une table qui ne peut pas être calculée (une formule qui dépend de la variable utilise une fonction que JeffCrunch ne connaît pas, ou change d'elle-même, comme ALEA ou MAINTENANT) conserve les valeurs de son fichier.

## Calculer manuellement

Cliquez sur **Mode de calcul** dans l'onglet **Formules** pour choisir la façon dont le classeur calcule ses formules :

- **Automatique** calcule les formules au fur et à mesure que vous modifiez les cellules.
- **Automatique, sauf les tableaux** fait de même, mais laisse les cellules des tables de données telles quelles jusqu'à ce que vous appuyiez sur **F9**.
- **Manuel** ne calcule rien, tables de données comprises, tant que vous ne le demandez pas. La barre d'état affiche **Non à jour** après une modification. Appuyez sur **F9** (**Tout recalculer**) pour tout calculer, ou sur **Maj+F9** (**Recalculer cette feuille**) pour calculer les formules de la feuille active, ainsi que tout ce qui attendait. Un classeur réglé sur Manuel s'ouvre avec les résultats contenus dans son fichier.

Le choix est enregistré avec le classeur, et c'est une étape que vous pouvez annuler.

## Formules qui se lisent elles-mêmes

Une formule qui lit son propre résultat, directement ou par l'intermédiaire d'autres cellules, ne peut pas être calculée de la façon habituelle. JeffCrunch vous le signale une fois, indique les cellules concernées et les encadre sur la feuille ; elles conservent les valeurs qu'elles avaient. Cliquez sur **Références circulaires** dans l'onglet **Formules** pour afficher la liste, et double-cliquez sur une cellule de la liste pour vous y rendre.

Certains modèles nécessitent volontairement une telle boucle. Choisissez **Calcul itératif des formules circulaires** dans le menu **Mode de calcul** : les formules sont recalculées à plusieurs reprises, chacune lisant les dernières valeurs des autres, jusqu'à ce qu'aucune valeur ne varie de plus de la quantité indiquée ou que le nombre maximal d'itérations soit atteint.

## Voir ce que lit une formule

Sélectionnez une cellule et cliquez sur **Repérer les antécédents** pour tracer des flèches depuis les cellules que lit sa formule. Cliquez de nouveau pour remonter d'un niveau. **Repérer les dépendants** trace des flèches vers les formules qui lisent la cellule, et chaque clic ajoute un niveau. Une petite page représente une cellule d'une autre feuille. Au maximum 2 000 flèches sont tracées, et la barre d'état l'indique lorsqu'il y en avait davantage. **Effacer les flèches** les supprime, et toute modification du classeur les efface aussi.

## Vérification des erreurs

Choisissez **Vérification des erreurs** dans le groupe **Vérifier** de l'onglet Formules pour parcourir la feuille à la recherche de cellules potentiellement incorrectes. Elle recherche deux types de problèmes : une formule qui aboutit à une erreur (comme #DIV/0!), et un nombre enregistré comme texte, que les formules peuvent ignorer.

La boîte de dialogue affiche une cellule à la fois, la montre sur la feuille et indique en une phrase le problème. **Suivant** et **Précédent** parcourent les cellules. Un petit triangle vert dans l'angle supérieur gauche d'une cellule la signale sur la feuille. **Ignorer l'erreur** retire la cellule de la liste, supprime son triangle et est enregistrée dans le fichier. Une étape d'annulation la rétablit. Si rien n'est à signaler, un message le indique.

## Évaluer la formule

Utilisez cette fonction pour voir comment une formule aboutit à sa valeur. Sélectionnez une cellule qui contient une formule et choisissez **Évaluer la formule**. La formule s'affiche avec la partie suivante soulignée. Appuyez sur **Évaluer** pour calculer cette partie ; sa valeur prend sa place. Les parties sont traitées dans cet ordre : chaque cellule isolée de gauche à droite, puis chaque fonction avec ses parenthèses, en commençant par la plus interne, puis le reste. **Recommencer** reprend depuis le début. Si une partie ne peut pas être calculée, la boîte de dialogue en indique la raison et laisse la formule inchangée.

## Fenêtre Espion

La **fenêtre Espion** est une petite fenêtre qui reste visible pendant votre travail. Elle liste les cellules que vous voulez surveiller, avec leur feuille, leur adresse, leur valeur et leur formule, même lorsqu'elles se trouvent sur une autre feuille. Sélectionnez des cellules et appuyez sur **Ajouter un espion**. Sélectionnez des lignes de la liste et appuyez sur **Supprimer l'espion**. Double-cliquez sur une ligne pour aller à la cellule. Les valeurs changent lorsque la feuille est recalculée. La liste est conservée tant que le classeur reste ouvert.

# Comment les formules sont affichées et calculées
<!-- keywords: débordement, déversement, #SPILL!, matrice dynamique, plage de débordement, A1#, erreur suivante, erreur précédente, parcourir les erreurs, tracer l'erreur, calculer avant l'enregistrement, formule, formules, calculer, calcul, recalculer, fonction, fonctions, valeur, résultat, enregistré, F9, circulaire, afficher les formules, barre de formule, erreur, pointer, pointage, cliquer sur une cellule, référence, F4, vérification des erreurs, ignorer l'erreur, évaluer la formule, pas à pas, espion, fenêtre espion, surveiller, LET, nommer une valeur, fonction cellule, fonction info, FILTRE.XML, xml, xpath-->

Une **formule** est une cellule qui calcule sa valeur à partir d'autres cellules, par exemple =SOMME(B3:B8). La cellule affiche la valeur, et la **barre de formule** affiche la formule.

## Ce que fait JeffCrunch des formules

Quand vous ouvrez un classeur, le fichier contient la dernière valeur de chaque formule, si bien que la feuille peut s'afficher tout de suite. Ensuite, JeffCrunch recalcule chaque formule, quelques-unes à la fois, afin que la fenêtre reste fluide. La barre d'état affiche **Calcul en cours** avec un pourcentage pendant ce travail. Une fois terminé, les cellules contiennent les résultats de JeffCrunch. Pour tout recalculer, cliquez sur **Tout recalculer** dans l'onglet **Formules**, ou appuyez sur **F9**.

## Quand JeffCrunch ne peut pas calculer une formule

JeffCrunch dispose de plus de 400 fonctions, et ce nombre augmente encore. Une formule qui utilise une fonction que JeffCrunch ne connaît pas encore conserve la valeur enregistrée dans le fichier. Il en va de même pour une formule qui fait référence à un autre classeur. Les cellules qui dépendent de ces formules sont calculées à partir des valeurs enregistrées.

## Fonctions qui demandent quelques précisions

- **Obligations et emprunts.** PRIX, RENDEMENT, DURATION, PRIX.PCAL.PREMIERE.PER et les autres fonctions d'obligations comptent les jours selon la **base** indiquée (0 correspond à 30 jours par mois sur une année de 360 jours, 1 aux jours réels, et ainsi de suite). RENDEMENT, RENDEMENT.PCAL.PREMIERE.PER, RENDEMENT.PCAL.DERNIERE.PER et TAUX trouvent leur résultat par essais successifs jusqu'à ce que le prix ou le versement corresponde. Ils s'arrêtent lorsque le résultat est juste à environ 15 chiffres près, si bien que les derniers chiffres peuvent différer d'un autre logiciel.
- **Lois de probabilité et tests.** Les fonctions qui se terminent par .LOI, .INVERSE, .TEST, ainsi que les anciens noms comme LOI.NORMALE et LOI.STUDENT, donnent les mêmes résultats : un ancien nom est conservé pour que les anciens fichiers continuent de fonctionner. Les fonctions *.INVERSE sont aussi calculées par essais successifs.
- **Listes qui remplissent plusieurs cellules.** TRIER, TRIERPAR, FILTRE, UNIQUE, SEQUENCE, MATRICE.ALEA, RECHERCHEX, MMULT, MINVERSE et DROITEREG donnent un bloc de valeurs. Saisissez-en une dans une seule cellule : le bloc se déverse dans les cellules situées en dessous et à côté (voir **Formules qui remplissent les cellules voisines**), ou saisissez une formule matricielle (**Ctrl+Maj+Entrée**) sur les cellules à remplir.
- **Textes dans d'autres écritures.** LENB, GAUCHEB, STXT, DROITEB, TROUVEB, CHERCHERB et REMPLACERB comptent chaque caractère pour un seul, comme sur un ordinateur qui n'est pas configuré pour les textes à deux octets. PHONETIQUE donne le texte de la cellule, car JeffCrunch ne conserve pas les lectures du texte japonais.
- **CONVERT.** Les unités s'écrivent sous forme de texte, avec les majuscules et les minuscules indiquées dans la liste de la fonction : « kg », « lbm », « mi », « F ». Un préfixe métrique peut précéder une unité qui en accepte un, comme « km » ou « mL ».

## MOD : le reste

**MOD(n ; d)** donne le reste de **n** après que le plus grand nombre entier de fois **d** a été retiré : c'est n - d * ENT(n / d). Exemple : **=MOD(10 ; 3)** donne 1, et **=MOD(-10 ; 3)** donne 2, car ENT(-10 / 3) vaut -4 et -10 - 3 * -4 vaut 2. Le résultat a le signe de **d**. Si **d** vaut 0, le résultat est #DIV/0!.

## Afficher les formules

Cliquez sur **Afficher les formules** dans l'onglet **Formules**, ou appuyez sur **Ctrl+'** (sur Mac, la touche Contrôle et non la touche Commande), pour voir la formule de chaque cellule à la place de sa valeur. Les colonnes qui contiennent des formules sont élargies, jusqu'à une certaine limite, afin que les formules s'affichent en entier ; elles reprennent leur largeur initiale lorsque vous désactivez cette option, et les largeurs du fichier ne sont pas modifiées.

## Modifier une formule

Pour modifier une formule, sélectionnez sa cellule, appuyez sur **F2**, modifiez-la, puis appuyez sur **Entrée**. Vous pouvez aussi la modifier dans la barre de formule. Lorsque vous modifiez une cellule, les formules qui en dépendent sont recalculées en arrière-plan. Voir [Saisir dans les cellules](edit-cells).

## Écrire une formule

Tapez **=** puis les premières lettres d'une fonction : une courte liste affiche les fonctions et les [noms](names) qui commencent ainsi, accompagnés d'une phrase pour chacun. Appuyez sur **Tab** pour choisir celui qui est sélectionné. Utilisez les flèches **Bas** et **Haut** pour en sélectionner un autre, puis **Tab** ou **Entrée**. **Échap** ferme la liste. Une fois à l'intérieur des parenthèses d'une fonction, ses arguments s'affichent sous la cellule, celui que vous saisissez en gras. La liste et les arguments sont les mêmes que ceux qu'affiche **Insérer une fonction**. Les arguments sont séparés par des points-virgules, ou par des virgules lorsque votre langue utilise la virgule comme séparateur décimal.

## Pointer les cellules

Au lieu de saisir l'adresse d'une cellule, vous pouvez la désigner. Pendant que vous tapez une formule, placez le curseur à un endroit où une cellule peut figurer (juste après **=**, après un opérateur comme **+**, une parenthèse ouvrante ou une virgule), puis cliquez sur la cellule. Son adresse est écrite à votre place. Faites glisser sur des cellules pour écrire une plage, comme B2:D5. Cliquez à nouveau avant de saisir autre chose, et la nouvelle cellule remplace celle que vous venez d'écrire. Maintenez **Ctrl** et cliquez pour ajouter une autre adresse après une virgule. Cliquez sur une lettre de colonne pour écrire une colonne entière, comme A:A, ou sur un numéro de ligne pour écrire une ligne entière, comme 3:3 ; faites glisser le long des lettres ou des numéros pour obtenir B:D ou 2:5.

Les touches fléchées fonctionnent aussi. Appuyez sur une flèche : un pointeur s'éloigne de la cellule que vous êtes en train de saisir, et son adresse est écrite ; appuyez sur d'autres flèches pour continuer, ou maintenez **Maj** pour étendre le pointeur sur une plage. La barre d'état affiche **Pointage** lorsque les touches fléchées pointent, **Entrée** pendant la saisie, et **Modification** lorsque les touches fléchées déplacent le curseur dans le texte. Appuyez sur **F2** pour basculer entre le pointage et la modification du texte.

Pour pointer une cellule d'une autre feuille, cliquez sur l'onglet de cette feuille pendant que le curseur se trouve à un endroit où une cellule peut figurer, puis cliquez sur la cellule. Le nom de la feuille est écrit avec elle, entre apostrophes si nécessaire, comme dans 'Ma feuille'!B3. **Entrée** vous ramène à la feuille de départ et place la formule dans sa cellule. **Échap** annule tout, y compris le changement de feuille.

Vous pouvez aussi faire glisser la bordure d'un bloc encadré sur la feuille pour déplacer cette adresse, ou faire glisser son carré d'angle pour l'étendre ; la formule change pendant le glissement.

Appuyez sur **F4** lorsque le curseur est dans une adresse ou juste après elle pour fixer certaines parties avec un signe dollar : A1 devient $A$1, puis A$1, puis $A1, puis de nouveau A1.

Chaque adresse de la formule est colorée dans le texte, et les cellules qu'elle désigne sont encadrées sur la feuille dans la même couleur, de sorte que vous voyez d'un coup d'œil ce que lit la formule. Un clic après une adresse complète, comme =A1, termine la saisie et sélectionne la cellule sur laquelle vous avez cliqué, comme pour tout autre texte.

## Tables de calcul à deux variables

Une table de calcul (table de simulation) indique le résultat d'une formule pour de nombreuses valeurs d'une ou deux cellules. Ouvrez un classeur qui en contient une, et JeffCrunch calcule ses cellules : pour une table à deux variables, la formule se trouve dans la cellule en haut à gauche de la table, les valeurs à tester en ligne vont dans la cellule de saisie de ligne, et celles en colonne dans la cellule de saisie de colonne. Pour une table à une variable, les valeurs sont placées dans la colonne à gauche de la table (et les formules se trouvent dans la ligne au-dessus) ou dans la ligne au-dessus de la table (et les formules se trouvent dans la colonne à gauche). Chaque cellule de la table est le résultat de la formule, et de chaque formule qui dépend de la cellule de saisie, pour cette valeur ; rien d'autre dans le classeur n'est modifié. En mode **Automatique**, les tables sont calculées avec les formules, et les formules qui lisent les cellules d'une table suivent. La création d'une nouvelle table de calcul n'est pas encore proposée. Une table qui ne peut pas être calculée (une formule qui dépend de la variable utilise une fonction que JeffCrunch ne connaît pas, ou change d'elle-même, comme ALEA ou MAINTENANT) conserve les valeurs de son fichier.

## Calculer manuellement

Cliquez sur **Mode de calcul** dans l'onglet **Formules** pour choisir la façon dont le classeur calcule ses formules :

- **Automatique** calcule les formules au fur et à mesure que vous modifiez les cellules.
- **Automatique, tables manuelles** fait de même, mais laisse les cellules des tables de calcul telles quelles jusqu'à ce que vous appuyiez sur **F9**.
- **Manuel** ne calcule rien, tables de calcul comprises, tant que vous ne le demandez pas. La barre d'état affiche **Non à jour** après une modification. Appuyez sur **F9** (**Tout recalculer**) pour tout calculer, ou sur **Maj+F9** (**Recalculer cette feuille**) pour calculer les formules de la feuille active, ainsi que tout ce qui attendait. Un classeur réglé sur Manuel s'ouvre avec les résultats enregistrés dans son fichier. **Calculer avant l'enregistrement**, également dans le menu **Mode de calcul**, est coché par défaut : lorsque vous enregistrez un classeur en mode Manuel qui n'est pas à jour, les formules sont d'abord calculées, de sorte que le fichier contient des nombres à jour. Désactivez cette option pour enregistrer les nombres tels qu'ils sont.

Le choix est enregistré avec le classeur, et il s'agit d'une étape que vous pouvez annuler.

## Formules qui se lisent elles-mêmes

Une formule qui lit son propre résultat, directement ou par l'intermédiaire d'autres cellules, ne peut pas être calculée de la façon habituelle. JeffCrunch vous en avertit une fois, indique les cellules concernées et les encadre sur la feuille ; elles conservent les valeurs qu'elles avaient. Cliquez sur **Références circulaires** dans l'onglet **Formules** pour afficher la liste, et double-cliquez sur une cellule de la liste pour vous y rendre.

Certains modèles ont besoin d'une telle boucle volontairement. Choisissez **Calcul itératif des formules circulaires** dans le menu **Mode de calcul** : les formules sont calculées à plusieurs reprises, chacune lisant les dernières valeurs des autres, jusqu'à ce qu'aucune valeur ne change de plus que la valeur indiquée, ou jusqu'à ce que le nombre maximal d'itérations autorisé soit atteint.

## Voir ce que lit une formule

Sélectionnez une cellule et cliquez sur **Repérer les antécédents** pour tracer des flèches depuis les cellules que lit sa formule. Cliquez à nouveau pour remonter d'un cran de plus. **Repérer les dépendants** trace des flèches vers les formules qui lisent la cellule, et chaque clic va d'un cran plus loin. Une petite page représente une cellule d'une autre feuille. Au maximum 2 000 flèches sont tracées, et la barre d'état l'indique s'il y en avait davantage. **Effacer les flèches** les supprime, et toute modification du classeur les efface également.

Lorsqu'une cellule affiche une erreur comme #DIV/0!, sélectionnez-la et cliquez sur **Tracer l'erreur**. Des flèches sont tracées vers les cellules qu'elle lit et qui affichent aussi une erreur. Cliquez à nouveau pour remonter d'un cran de plus. Lorsqu'aucune des cellules qu'elle lit n'affiche d'erreur, la barre d'état indique que cette cellule est à l'origine de l'erreur, et que c'est celle qu'il faut corriger.

Pour parcourir toutes les erreurs de la feuille, cliquez sur **Erreur suivante** ou **Erreur précédente**. Les cellules qui affichent une erreur sont visitées ligne après ligne ; après la dernière, le parcours recommence à la première. Chaque cellule est sélectionnée, et les flèches sont tracées jusqu'à la cellule qui produit l'erreur. La barre d'état indique l'erreur en cours et le nombre total d'erreurs.

## Vérification des erreurs

Choisissez **Vérification des erreurs** dans le groupe **Vérification** de l'onglet Formules pour rechercher dans la feuille les cellules qui pourraient être erronées. Elle recherche deux choses : une formule qui aboutit à une erreur (comme #DIV/0!), et un nombre enregistré sous forme de texte, que les formules peuvent ignorer.

La boîte de dialogue affiche une cellule à la fois, va à cette cellule dans la feuille et indique en une phrase le problème. **Suivant** et **Précédent** parcourent les cellules. Un petit triangle vert dans le coin supérieur gauche d'une cellule la signale sur la feuille. **Ignorer l'erreur** retire la cellule de la liste, supprime son triangle et enregistre ce choix dans le fichier. Une étape d'annulation rétablit l'erreur. Si rien n'est à signaler, un message vous l'indique.

## Évaluer la formule

Utilisez cette fonction pour voir comment une formule obtient sa valeur. Sélectionnez une cellule qui contient une formule et choisissez **Évaluer la formule**. La formule s'affiche avec la partie suivante soulignée. Appuyez sur **Évaluer** pour calculer cette partie ; sa valeur prend sa place. Les parties sont traitées dans cet ordre : chaque cellule isolée de gauche à droite, puis chaque fonction avec ses parenthèses, en commençant par la plus interne, puis le reste. **Recommencer** reprend depuis le début. Si une partie ne peut pas être calculée, la boîte de dialogue en indique la raison et laisse la formule inchangée.

## Fenêtre Espion

La **fenêtre Espion** est une petite fenêtre qui reste visible pendant votre travail. Elle liste les cellules que vous voulez surveiller, avec leur feuille, leur cellule, leur valeur et leur formule, même lorsqu'elles se trouvent sur une autre feuille. Sélectionnez des cellules et appuyez sur **Ajouter un espion**. Sélectionnez des lignes dans la liste et appuyez sur **Supprimer l'espion**. Double-cliquez sur une ligne pour aller à la cellule. Les valeurs changent lorsque la feuille est recalculée. La liste est conservée tant que le classeur est ouvert.

## Formules qui remplissent les cellules voisines

Lorsqu'une formule donne un bloc de valeurs, ce bloc remplit les cellules situées en dessous et à côté de la cellule où vous l'avez saisie. On parle de **débordement**. Essayez =SEQUENCE(3) en A1 : A1 affiche 1, et A2 et A3 affichent 2 et 3. Une formule déborde lorsqu'elle appelle une fonction qui donne un bloc (TRIER, TRIERPAR, FILTRE, UNIQUE, SEQUENCE, MATRICE.ALEA, TRANSPOSE, RECHERCHEX ou MMULT), qu'elle écrit un bloc entre accolades, qu'elle utilise une plage de débordement (voir ci-dessous) ou qu'elle effectue un calcul sur une plage, comme dans =A1:A3*2.

- **Les cellules remplies ne contiennent pas de formule à elles seules.** Elles affichent les valeurs. Lorsque vous en sélectionnez une, la barre de formule affiche en grisé la formule de la cellule qui la remplit. Appuyer sur Entrée sans modification ne change rien. Pour modifier le bloc, modifiez d'abord cette première cellule.
- **Le bloc suit le résultat.** Si la formule donne plus ou moins de valeurs après une modification des cellules qu'elle lit, le bloc s'agrandit ou se réduit, et les formules qui lisent ces cellules sont recalculées.
- **Une cellule gênante donne #SPILL!.** Si l'une des cellules où le bloc irait contient une valeur ou une formule, la première cellule affiche **#SPILL!** et rien n'est rempli. Videz la cellule gênante, et le bloc se remplit aussitôt. Saisir dans une cellule que le bloc remplit produit le même effet : la première cellule affiche #SPILL!.
- **Un bloc a une taille limite.** Un bloc plus grand que la feuille, ou de plus de 200 000 cellules, donne #SPILL!.
- **A1# désigne tout le bloc.** Écrivez la cellule qui contient la formule suivie d'un **#**, comme dans =SOMME(A1#), pour utiliser tout le bloc tel qu'il est actuellement, depuis cette feuille ou une autre (=SOMME(Feuil1!A1#)). Si A1 ne déborde pas, A1# donne #REF!.

Un débordement est enregistré avec le fichier sous forme de matrice dynamique, si bien que le programme qui a créé le fichier l'affiche aussi comme un débordement. Un enregistrement au format .xls ou .ods plus ancien conserve les valeurs, ainsi que la formule sous forme de formule matricielle sur le bloc, lorsque ce format plus ancien dispose des fonctions utilisées. La vérification de compatibilité signale ces formules.

## Donner des noms à des valeurs dans une formule

**LET** donne un nom à une valeur au milieu d'une formule, afin que vous l'écriviez une seule fois et que vous puissiez la réutiliser. Les couples nom-valeur viennent en premier, et le calcul en dernier. Dans =LET(prix; B2; taxe; prix*0,08; prix+taxe), le nom **prix** représente la valeur de B2, **taxe** est calculée à partir de celui-ci, et la dernière partie additionne les deux. Un nom peut utiliser les noms qui le précèdent, et un LET peut contenir jusqu'à 126 couples. Un nom ne fonctionne qu'à l'intérieur de son propre LET, et un LET imbriqué dans un autre peut réutiliser un nom. Un nom peut aussi représenter une plage de cellules, comme dans =LET(lignes; B2:B9; SOMME(lignes)/NBVAL(lignes)).

## Interroger une cellule ou le classeur

**CELLULE** donne des informations sur une cellule. Son premier argument indique lesquelles : « adresse », « ligne », « col », « contenu », « type », « format » (la présentation des nombres, comme « F2 » pour deux décimales, « P0 » pour un pourcentage, ou « D1 » pour une date), « couleur » (1 lorsque les nombres négatifs s'affichent en couleur), « parenthèses » (1 lorsque les nombres positifs s'affichent entre parenthèses), « préfixe » (le signe d'un texte aligné à gauche, à droite ou au centre), « protect » (1 lorsque la cellule est verrouillée), « largeur » (la largeur de la colonne en caractères), et « nomfichier » (le dossier, le fichier et la feuille, comme /home/moi/[Budget.xlsx]Feuil1 ; vide tant que le classeur n'est pas enregistré). **INFO** donne des informations sur le programme et le classeur : « répertoire », « numfichier » (le nombre de feuilles affichées), « origine », « version_os », « recalcul » (Automatique ou Manuel), « version » et « système ».

## Extraire des valeurs d'un XML

**FILTRE.XML** prend un texte qui contient du XML et un XPath, c'est-à-dire une façon courte d'indiquer les parties souhaitées, et donne les valeurs trouvées. =FILTRE.XML(A1 ; "//item/price") donne chaque prix du texte de A1, les uns sous les autres. Utilisez "/shop/item[2]/name" pour le deuxième article, "//item[@id='b']/name" pour l'article dont l'identifiant est b, ou "count(//item)" pour en connaître le nombre. Si le texte n'est pas du XML valide, ou si le chemin n'est pas valide, le résultat est #VALEUR! ; si rien ne correspond, il est #N/A. Un XML qui contient une déclaration de type de document est refusé.

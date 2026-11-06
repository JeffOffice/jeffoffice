# Comment les formules sont affichées et calculées
<!-- keywords: erreur de suivi, calculer avant l'enregistrement, formule, formules, calculer, calcul, recalculer, fonction, fonctions, valeur, résultat, enregistré, F9, circulaire, afficher les formules, barre de formule, erreur, pointer, pointage, cliquer sur une cellule, référence, F4, vérification des erreurs, ignorer l'erreur, évaluer la formule, étape, fenêtre espion, espion, soit, nommer une valeur, fonction cellule, fonction info, filterxml, xml, xpath-->

Une **formule** est une cellule qui calcule sa valeur à partir d'autres cellules, par exemple =SOMME(B3:B8). La cellule affiche la valeur, et la **barre de formule** affiche la formule.

## Ce que fait JeffCrunch avec les formules

Lorsque vous ouvrez un classeur, le fichier contient la dernière valeur de chaque formule, ce qui permet d'afficher la feuille immédiatement. JeffCrunch recalcule ensuite chaque formule, quelques-unes à la fois, afin que la fenêtre reste réactive. La barre d'état affiche **Calcul en cours** avec un pourcentage pendant ce travail. Une fois le calcul terminé, les cellules contiennent les résultats de JeffCrunch. Pour tout recalculer, cliquez sur **Tout recalculer** dans l'onglet **Formules**, ou appuyez sur **F9**.

## Lorsque JeffCrunch ne peut pas calculer une formule

JeffCrunch dispose de plus de 400 fonctions, et ce nombre continue de croître. Une formule qui utilise une fonction que JeffCrunch ne possède pas encore conserve la valeur enregistrée dans le fichier. C'est aussi le cas d'une formule qui fait référence à un autre classeur. Les cellules qui dépendent de ces formules sont calculées à partir des valeurs enregistrées.

## Fonctions qui demandent quelques précisions

- **Obligations et emprunts.** PRIX, RENDEMENT, DURATION, PRIX.PCOUPON.IRREG et les autres fonctions de titres comptent les jours selon la **base** indiquée (0 correspond à 30 jours par mois sur une année de 360 jours, 1 correspond aux jours réels, et ainsi de suite). RENDEMENT, RENDEMENT.PCOUPON.IRREG, RENDEMENT.DERNIER.IRREG et TAUX cherchent leur résultat en essayant des valeurs jusqu'à ce que le prix ou le versement corresponde. Ils s'arrêtent lorsque le résultat est exact à environ 15 chiffres significatifs ; les derniers chiffres peuvent donc différer d'un autre logiciel.
- **Lois de probabilité et tests.** Les fonctions dont le nom se termine par .LOI, .INVERSE, .TEST, ainsi que les anciens noms comme LOI.NORMALE et LOI.STUDENT, donnent les mêmes résultats : un ancien nom est conservé afin que les anciens fichiers continuent de fonctionner. Les fonctions *.INVERSE trouvent elles aussi leur résultat en essayant des valeurs.
- **Listes qui remplissent plusieurs cellules.** TRIER, TRIERPAR, FILTRE, UNIQUE, SEQUENCE, MALEA, RECHERCHEX, MMULT, INVERSEMAT et DROITEREG donnent un bloc de valeurs. Saisissez-les comme formule matricielle (**Ctrl+Maj+Entrée**) sur les cellules qu'elles doivent remplir. La propagation automatique d'une formule dans les cellules voisines n'est pas encore prise en charge.
- **Textes dans d'autres écritures.** LENB, GAUCHEB, STXTB, DROITEB, TROUVERB, CHERCHERB et REMPLACERB comptent chaque caractère pour un seul, comme sur un ordinateur qui n'est pas configuré pour les jeux de caractères à deux octets. PHONETIQUE donne le texte de la cellule, car JeffCrunch ne conserve pas les lectures des textes japonais.
- **CONVERT.** Les unités s'écrivent sous forme de texte, avec les majuscules et les minuscules indiquées dans la liste de la fonction : « kg », « lbm », « mi », « F ». Un préfixe métrique peut précéder une unité qui en accepte un, comme « km » ou « mL ».

## MOD : le reste

**MOD(n ; d)** donne ce qui reste de **n** après que le plus grand nombre entier de fois **d** en a été retiré : c'est n - d * ENT(n / d). Exemple : **=MOD(10 ; 3)** donne 1, et **=MOD(-10 ; 3)** donne 2, car ENT(-10 / 3) vaut -4 et -10 - 3 * -4 vaut 2. Le résultat a le signe de **d**. Si **d** vaut 0, le résultat est #DIV/0!.

## Afficher les formules

Cliquez sur **Afficher les formules** dans l'onglet **Formules**, ou appuyez sur **Ctrl+'** (sur un Mac, la touche Contrôle et non la touche Commande), pour voir la formule de chaque cellule à la place de sa valeur. Les colonnes qui contiennent des formules sont élargies, dans une certaine limite, afin que les formules s'affichent en entier ; elles reprennent leur largeur d'origine lorsque vous désactivez cette option, et les largeurs enregistrées dans votre fichier ne sont pas modifiées.

## Modifier les formules

Pour modifier une formule, sélectionnez sa cellule, appuyez sur **F2**, modifiez-la, puis appuyez sur **Entrée**. Vous pouvez aussi la modifier dans la barre de formule. Lorsque vous modifiez une cellule, les formules qui en dépendent sont recalculées en arrière-plan. Voir [Saisir dans les cellules](edit-cells).

## Écrire une formule

Tapez **=** puis les premières lettres d'une fonction : une courte liste affiche les fonctions et les [noms](names) qui commencent ainsi, avec une phrase descriptive pour chacun. Appuyez sur **Tab** pour choisir celle qui est sélectionnée. Utilisez les flèches **Bas** et **Haut** pour en sélectionner une autre, puis **Tab** ou **Entrée**. **Échap** ferme la liste. Une fois à l'intérieur des parenthèses d'une fonction, ses arguments s'affichent sous la cellule, l'argument en cours de saisie étant en gras. La liste et les arguments sont les mêmes que ceux qu'affiche **Choisir une fonction**. Les arguments sont séparés par des points-virgules, ou par des virgules lorsque votre langue utilise la virgule comme séparateur décimal.

## Pointer des cellules

Au lieu de taper l'adresse d'une cellule, vous pouvez la désigner. Pendant la saisie d'une formule, placez le curseur à un endroit où une cellule peut figurer (juste après **=**, un opérateur comme **+**, une parenthèse ouvrante ou une virgule), puis cliquez sur la cellule. Son adresse est saisie automatiquement. Faites glisser sur des cellules pour saisir un bloc tel que B2:D5. Cliquez à nouveau avant de taper autre chose, et la nouvelle cellule remplace celle qui vient d'être saisie. Maintenez **Ctrl** et cliquez pour ajouter une autre adresse après une virgule. Cliquez sur une lettre de colonne pour saisir une colonne entière, telle que A:A, ou sur un numéro de ligne pour saisir une ligne entière, telle que 3:3, et faites glisser sur les lettres ou les numéros pour obtenir B:D ou 2:5.

Les touches fléchées fonctionnent aussi. Appuyez sur une flèche : un pointeur s'éloigne de la cellule en cours de saisie, et son adresse est saisie ; appuyez sur d'autres flèches pour continuer, ou maintenez **Maj** pour étendre le pointeur sur un bloc. La barre d'état affiche **Pointage** pendant que les touches fléchées pointent, **Entrée** pendant la saisie, et **Modifier** lorsque les touches fléchées déplacent le curseur dans le texte. Appuyez sur **F2** pour basculer entre le pointage et la modification du texte.

Pour pointer une cellule d'une autre feuille, cliquez sur l'onglet de cette feuille pendant que le curseur se trouve à un endroit où une cellule peut figurer, puis cliquez sur la cellule. Le nom de la feuille est écrit avec l'adresse, entre guillemets si nécessaire, comme dans 'Ma feuille'!B3. **Entrée** vous ramène à la feuille de départ et place la formule dans sa cellule. **Échap** annule tout, y compris le changement de feuille.

Vous pouvez aussi faire glisser la bordure d'un bloc encadré sur la feuille pour déplacer cette adresse, ou faire glisser son carré d'angle pour l'agrandir ; la formule change pendant le glissement.

Appuyez sur **F4** lorsque le curseur est dans une adresse ou juste après elle pour fixer certaines parties de celle-ci avec un signe dollar : A1 devient $A$1, puis A$1, puis $A1, puis de nouveau A1.

Chaque adresse de la formule est colorée dans le texte, et les cellules qu'elle désigne sont encadrées sur la feuille dans la même couleur, ce qui permet de voir d'un coup d'œil ce que lit la formule. Un clic après une adresse complète, telle que =A1, termine la saisie et sélectionne la cellule sur laquelle vous avez cliqué, comme pour tout autre texte.

## Tableaux de données

Un tableau de données (tableau d'hypothèses) indique le résultat d'une formule pour de nombreuses valeurs d'une ou de deux cellules. Lorsque vous ouvrez un classeur qui en contient un, JeffCrunch calcule ses cellules : pour un tableau à deux variables, la formule se trouve dans la cellule en haut à gauche du tableau, les valeurs à essayer en ligne sont placées dans la cellule de saisie en ligne, et celles en colonne dans la cellule de saisie en colonne. Pour un tableau à une variable, les valeurs se placent dans la colonne à sa gauche (et les formules se trouvent dans la ligne au-dessus) ou dans la ligne au-dessus (et les formules se trouvent dans la colonne à sa gauche). Chaque cellule du tableau est le résultat de la formule, ainsi que de toutes les formules qui dépendent de la cellule de saisie, lorsque celle-ci prend cette valeur ; rien d'autre dans le classeur n'est modifié. En mode **Automatique**, les tableaux sont calculés avec les formules, et les formules qui lisent les cellules d'un tableau suivent. La création d'un nouveau tableau de données n'est pas encore proposée. Un tableau qui ne peut pas être calculé (une formule qui dépend de la saisie utilise une fonction que JeffCrunch ne possède pas, ou change d'elle-même, comme ALEA ou MAINTENANT) conserve les valeurs de son fichier.

## Calcul manuel

Cliquez sur **Mode de calcul** dans l'onglet **Formules** pour choisir la manière dont le classeur calcule ses formules :

- **Automatique** calcule les formules au fur et à mesure que vous modifiez les cellules.
- **Automatique sauf tables** fait de même, mais laisse les cellules des tableaux de données telles quelles jusqu'à ce que vous appuyiez sur **F9**.
- **Manuel** ne calcule rien, tableaux de données compris, tant que vous ne le demandez pas. La barre d'état affiche **Non à jour** après une modification. Appuyez sur **F9** (**Tout recalculer**) pour tout calculer, ou sur **Maj+F9** (**Recalculer cette feuille**) pour calculer les formules de la feuille active, ainsi que tout ce qui attendait. Un classeur réglé sur Manuel s'ouvre avec les résultats contenus dans son fichier. **Calculer avant l'enregistrement**, également dans le menu **Mode de calcul**, est coché par défaut : lorsque vous enregistrez un classeur en mode Manuel qui n'est pas à jour, les formules sont d'abord calculées, de sorte que le fichier contienne des nombres à jour. Désactivez cette option pour enregistrer les nombres tels qu'ils sont.

Ce choix est enregistré avec le classeur, et c'est une étape que vous pouvez annuler.

## Formules qui se lisent elles-mêmes

Une formule qui lit son propre résultat, directement ou par l'intermédiaire d'autres cellules, ne peut pas être calculée de la manière habituelle. JeffCrunch vous le signale une seule fois, désigne les cellules et les encadre sur la feuille ; celles-ci conservent les valeurs qu'elles avaient. Cliquez sur **Références circulaires** dans l'onglet **Formules** pour afficher la liste, puis double-cliquez sur une cellule de la liste pour vous y rendre.

Certains modèles ont besoin d'une telle boucle volontairement. Choisissez **Calcul itératif des formules circulaires** dans le menu **Mode de calcul** : les formules sont recalculées encore et encore, chacune lisant les dernières valeurs des autres, jusqu'à ce qu'aucune valeur ne change de plus que le seuil que vous indiquez, ou jusqu'à ce que le nombre de tours autorisé soit atteint.

## Voir ce que lit une formule

Sélectionnez une cellule et cliquez sur **Repérer les antécédents** pour tracer des flèches vers les cellules que sa formule lit. Cliquez à nouveau pour remonter d'un cran. **Repérer les dépendants** trace des flèches vers les formules qui lisent la cellule, et chaque clic avance d'un cran. Une petite page représente une cellule d'une autre feuille. Au plus 2 000 flèches sont tracées, et la barre d'état indique s'il y en avait davantage. **Effacer les flèches** les supprime, et toute modification du classeur les supprime aussi.

Lorsqu'une cellule affiche une erreur telle que #DIV/0!, sélectionnez-la et cliquez sur **Tracer l'erreur**. Des flèches sont tracées vers les cellules qu'elle lit et qui affichent elles aussi une erreur. Cliquez à nouveau pour remonter d'un cran. Lorsqu'aucune des cellules lues n'affiche d'erreur, la barre d'état indique que cette cellule est à l'origine de l'erreur, et c'est celle qu'il faut corriger.

## Vérification des erreurs

Choisissez **Vérification des erreurs** dans le groupe **Vérification** de l'onglet Formules pour rechercher dans la feuille les cellules qui pourraient être erronées. Elle recherche deux choses : une formule qui aboutit à une erreur (telle que #DIV/0!), et un nombre enregistré sous forme de texte, que les formules pourraient ignorer.

La boîte de dialogue affiche une cellule à la fois, accède à cette cellule dans la feuille et indique en une phrase le problème. **Suivant** et **Précédent** parcourent les cellules. Un petit triangle vert dans le coin supérieur gauche d'une cellule la signale sur la feuille. **Ignorer l'erreur** retire la cellule de la liste, supprime son triangle et est enregistré dans le fichier. Une étape d'annulation permet de le rétablir. Si rien n'est à signaler, un message vous l'indique.

## Évaluer la formule

Utilisez cette fonction pour voir comment une formule aboutit à sa valeur. Sélectionnez une cellule qui contient une formule et choisissez **Évaluer la formule**. La formule s'affiche avec la partie suivante soulignée. Appuyez sur **Évaluer** pour calculer cette partie ; sa valeur prend sa place. Les parties sont traitées dans cet ordre : chaque cellule isolée de gauche à droite, puis chaque fonction avec ses parenthèses, en commençant par la plus interne, puis le reste. **Recommencer** reprend depuis le début. Si une partie ne peut pas être calculée, la boîte de dialogue en indique la raison et laisse la formule telle quelle.

## Fenêtre Espion

La **fenêtre Espion** est une petite fenêtre qui reste visible pendant que vous travaillez. Elle répertorie les cellules que vous voulez surveiller, avec leur feuille, leur cellule, leur valeur et leur formule, même lorsqu'elles se trouvent sur une autre feuille. Sélectionnez des cellules et appuyez sur **Ajouter un espion**. Sélectionnez des lignes de la liste et appuyez sur **Supprimer l'espion**. Double-cliquez sur une ligne pour aller à la cellule. Les valeurs changent lorsque la feuille est recalculée. La liste est conservée tant que le classeur est ouvert.

## Donner des noms aux valeurs dans une formule

**LET** donne un nom à une valeur au milieu d'une formule, de sorte que vous l'écrivez une seule fois et que vous pouvez la réutiliser. Les couples formés d'un nom et d'une valeur viennent en premier, et le calcul vient en dernier. Dans =LET(prix; B2; taxe; prix*0,08; prix+taxe), le nom **prix** représente la valeur de B2, **taxe** est calculée à partir de celui-ci, et la dernière partie additionne les deux. Un nom peut utiliser les noms qui le précèdent. Un nom n'est valable qu'à l'intérieur de son propre LET, et un LET placé dans un autre peut réutiliser un nom. Un nom peut aussi représenter un bloc de cellules, comme dans =LET(lignes; B2:B9; SOMME(lignes)/NB(lignes)).

## Interroger une cellule ou le classeur

**CELLULE** fournit des informations sur une cellule. Son premier argument indique lesquelles : « adresse », « ligne », « col », « contenu », « type », « format » (la manière dont les nombres s'affichent, comme « F2 » pour deux décimales, « P0 » pour un pourcentage, ou « D1 » pour une date), « couleur » (1 lorsque les nombres négatifs s'affichent en couleur), « parenthèses » (1 lorsque les nombres positifs s'affichent entre parenthèses), « préfixe » (le repère d'un texte aligné à gauche, à droite ou au centre), « protection » (1 lorsque la cellule est verrouillée), « largeur » (la largeur de la colonne en caractères), et « nomfichier » (le dossier, le fichier et la feuille, tel que /home/moi/[Budget.xlsx]Feuil1 ; vide tant que le classeur n'est pas enregistré). **INFO** fournit des informations sur le programme et le classeur : « repertoire », « numfeuille » (le nombre de feuilles affichées), « origine », « version_os », « recalcul » (Automatique ou Manuel), « release » et « système ».

## Extraire des valeurs d'un XML

**FILTREXML** prend un texte qui contient du XML et un XPath, c'est-à-dire une façon abrégée d'indiquer les parties souhaitées, et donne les valeurs trouvées. =FILTREXML(A1; "//article/prix") donne chaque prix du texte de A1, les uns sous les autres. Utilisez « /magasin/article[2]/nom » pour le deuxième article, « //article[@id='b']/nom » pour l'article dont l'identifiant est b, ou « count(//article) » pour leur nombre. Si le texte n'est pas du XML valide, ou si le chemin n'est pas valide, le résultat est #VALEUR!. Si rien ne correspond, le résultat est #N/A. Le XML qui contient une déclaration de type de document est refusé.

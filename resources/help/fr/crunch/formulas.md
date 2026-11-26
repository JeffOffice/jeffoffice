# Comment les formules sont affichées et calculées
<!-- keywords: débordement, déversement, #SPILL!, matrice dynamique, plage de débordement, A1#, erreur suivante, erreur précédente, parcourir les erreurs, tracer l'erreur, calculer avant l'enregistrement, formule, formules, calculer, calcul, recalculer, fonction, fonctions, valeur, résultat, enregistré, F9, circulaire, afficher les formules, barre de formule, erreur, pointer, pointage, cliquer sur une cellule, référence, F4, vérification des erreurs, ignorer l'erreur, évaluer la formule, pas à pas, volet espion, espion, LET, nommer une valeur, fonction cellule, fonction info, FILTRE.XML, xml, xpath, précision telle qu'affichée, définir la précision comme affichée, arrondi, arrondir, décimales, chiffres-->

Une **formule** est une cellule qui calcule sa valeur à partir d'autres cellules, par exemple =SOMME(B3:B8). La cellule affiche la valeur, et la **barre de formule** affiche la formule.

## Ce que fait JeffCrunch avec les formules

Lorsque vous ouvrez un classeur, le fichier contient la dernière valeur de chaque formule, ce qui permet d'afficher la feuille immédiatement. JeffCrunch recalcule ensuite toutes les formules, quelques-unes à la fois, afin que la fenêtre reste fluide. La barre d'état affiche **Calcul en cours** avec un pourcentage pendant ce temps. Une fois le calcul terminé, les cellules contiennent les résultats de JeffCrunch. Pour tout recalculer, cliquez sur **Calculer tout** dans l'onglet **Formules**, ou appuyez sur **F9**.

## Lorsque JeffCrunch ne peut pas calculer une formule

JeffCrunch dispose de plus de 400 fonctions, et ce nombre continue d'augmenter. Une formule qui utilise une fonction que JeffCrunch ne connaît pas encore conserve la valeur enregistrée dans le fichier. Il en va de même pour une formule qui fait référence à un autre classeur. Les cellules qui dépendent de ces formules sont calculées à partir des valeurs enregistrées.

## Fonctions qui demandent quelques précisions

- **Obligations et emprunts.** PRIX, RENDEMENT, DURATION, PRIX.PCOUPON.IRREG et les autres fonctions d'obligations comptent les jours selon la **base** indiquée (0 correspond à 30 jours par mois sur une année de 360 jours, 1 correspond aux jours réels, et ainsi de suite). RENDEMENT, RENDEMENT.PCOUPON.IMPAIR, RENDEMENT.DER.IMPAIR et TAUX trouvent leur résultat en essayant des valeurs jusqu'à ce que le prix ou le paiement correspondent. Elles s'arrêtent lorsque le résultat est juste à environ 15 chiffres près, de sorte que les derniers chiffres peuvent différer de ceux d'un autre programme.
- **Lois de probabilité et tests.** Les fonctions dont le nom se termine par .DIST, .INVERSE, .TEST, ainsi que les anciens noms comme LOI.NORMALE et LOI.STUDENT, donnent les mêmes résultats : un ancien nom est conservé pour que les anciens fichiers continuent de fonctionner. Les fonctions *.INVERSE trouvent elles aussi leur résultat en essayant des valeurs.
- **Listes qui remplissent plusieurs cellules.** TRI, TRIERPAR, FILTRE, UNIQUE, SEQUENCE, ALEA.MATRICE, RECHERCHEX, MMULT, INVERSEMAT et DROITEREG donnent un bloc de valeurs. Saisissez-en une dans une seule cellule : le bloc déborde dans les cellules situées en dessous et à côté (voir **Formules qui remplissent les cellules voisines**), ou saisissez-la comme formule matricielle (**Ctrl+Maj+Entrée**) sur les cellules qu'elle doit remplir.
- **Textes dans d'autres écritures.** NBCAR.B, GAUCHE.B, STXT.B, DROITE.B, TROUVE.B, CHERCHE.B et REMPLACER.B comptent chaque caractère comme un seul, comme sur un ordinateur qui n'est pas configuré pour le texte sur deux octets. PHONETIQUE donne le texte de la cellule, car JeffCrunch ne conserve pas les lectures du texte japonais.
- **CONVERT.** Les unités s'écrivent sous forme de texte, avec les majuscules et les minuscules indiquées dans la liste de la fonction : « kg », « lbm », « mi », « F ». Un préfixe métrique peut précéder une unité qui en accepte un, par exemple « km » ou « mL ».

## MOD : le reste

**MOD(n ; d)** donne le reste de **n** après que les copies entières de **d** les plus nombreuses ont été retirées : c'est n - d * ENT(n / d). Exemple : **=MOD(10 ; 3)** vaut 1, et **=MOD(-10 ; 3)** vaut 2, car ENT(-10 / 3) vaut -4 et -10 - 3 * -4 vaut 2. Le résultat a le signe de **d**. Si **d** vaut 0, le résultat est #DIV/0!.

## Afficher les formules

Cliquez sur **Afficher les formules** dans l'onglet **Formules**, ou appuyez sur **Ctrl+'** (sur Mac, la touche Contrôle et non la touche Commande), pour voir la formule de chaque cellule à la place de sa valeur. Les colonnes qui contiennent des formules sont élargies, dans une certaine limite, afin que les formules s'affichent en entier ; elles reprennent leur largeur initiale lorsque vous désactivez cette option, et les largeurs de votre fichier ne sont pas modifiées.

## Modifier des formules

Pour modifier une formule, sélectionnez sa cellule, appuyez sur **F2**, modifiez-la, puis appuyez sur **Entrée**. Vous pouvez aussi la modifier dans la barre de formule. Lorsque vous modifiez une cellule, les formules qui en dépendent sont recalculées en arrière-plan. Voir [Saisir dans les cellules](edit-cells).

## Écrire une formule

Tapez **=** puis les premières lettres d'une fonction : une courte liste affiche les fonctions et les [noms](names) qui commencent ainsi, accompagnés d'une phrase pour chacun. Appuyez sur **Tab** pour retenir celle qui est mise en évidence. Utilisez les flèches **Bas** et **Haut** pour en mettre une autre en évidence, puis **Tab** ou **Entrée**. **Échap** ferme la liste. Une fois à l'intérieur des parenthèses d'une fonction, ses arguments s'affichent sous la cellule, celui que vous saisissez étant en gras. La liste et les arguments sont les mêmes que ceux qu'affiche **Choisir une fonction**. Les arguments sont séparés par des virgules, ou par des points-virgules lorsque votre langue utilise la virgule comme séparateur décimal.

## Noms de fonctions dans votre langue

Lorsque JeffCrunch s'affiche en allemand, en français, en espagnol, en italien, en néerlandais, en portugais ou en russe, vous pouvez saisir une formule avec les noms de fonctions utilisés dans votre langue, par exemple `=SUMME(A1;A2)` en allemand ou `=SOMME(A1;A2)` en français. Les noms anglais fonctionnent aussi : une formule copiée depuis le web n'a donc pas besoin d'être modifiée. La barre de formule, la cellule, la liste des fonctions et **Choisir une fonction** affichent les noms de votre langue.

Le classeur ne change pas lorsque vous changez de langue. Un fichier conserve toujours les noms anglais : un classeur créé dans une langue s'ouvre et se lit correctement dans une autre, ainsi que dans tout autre programme. Les noms locaux sont disponibles en allemand, en français, en espagnol, en italien, en néerlandais, en portugais (Brésil et Portugal) et en russe ; les noms anglais fonctionnent toujours, quelle que soit la langue. Les programmes en ukrainien et en indonésien conservent les noms anglais pour presque toutes les fonctions. Lorsqu'une langue possède deux noms courants pour une même fonction, les deux sont reconnus et le plus usuel est affiché. Une fonction qui n'a pas de nom usuel dans votre langue conserve son nom anglais. VRAI et FAUX suivent également votre langue. Les valeurs d'erreur de votre langue, comme `#N/A` en allemand, sont reconnues lorsque vous saisissez une formule et s'affichent dans la barre de formule, mais une cellule qui contient une erreur affiche toujours la forme standard, comme `#N/A`.

Lorsque les paramètres régionaux de votre ordinateur utilisent la virgule comme séparateur décimal, les arguments d'une fonction sont séparés par un point-virgule. Une matrice entre accolades comportant plusieurs lignes s'affiche avec un point-virgule entre les valeurs d'une ligne et une barre oblique inverse entre les lignes, par exemple `{1;2\3;4}`, et vous pouvez la saisir ainsi. Une matrice saisie sans barre oblique inverse est lue comme un fichier l'écrit : des virgules entre les valeurs d'une ligne, des points-virgules entre les lignes, et un point comme séparateur décimal.

Pour ne saisir que des noms anglais, cliquez sur **Fichier > Options > Formules** et cochez **Utiliser les noms de fonctions en anglais**.

## Pointer les cellules

Au lieu de saisir l'adresse d'une cellule, vous pouvez la pointer. Pendant la saisie d'une formule, placez le curseur à un endroit où une cellule peut figurer (juste après **=**, un opérateur comme **+**, une parenthèse ouvrante ou une virgule) et cliquez sur la cellule. Son adresse est écrite à votre place. Faites glisser sur des cellules pour écrire une plage telle que B2:D5. Cliquez à nouveau avant de saisir autre chose, et la nouvelle cellule remplace celle que vous venez d'écrire. Maintenez **Ctrl** enfoncée et cliquez pour ajouter une autre adresse après une virgule. Cliquez sur une lettre de colonne pour écrire une colonne entière telle que A:A, ou sur un numéro de ligne pour écrire une ligne entière telle que 3:3, et faites glisser le long des lettres ou des numéros pour B:D ou 2:5.

Les touches fléchées fonctionnent aussi. Appuyez sur une flèche : un pointeur s'éloigne de la cellule en cours de saisie et son adresse est écrite ; appuyez à nouveau sur les flèches pour continuer, ou maintenez **Maj** enfoncée pour étendre le pointeur sur une plage. La barre d'état affiche **Pointer** pendant que les touches fléchées pointent, **Entrée** pendant la saisie et **Modifier** lorsque les touches fléchées déplacent le curseur dans le texte. Appuyez sur **F2** pour basculer entre le pointage et la modification du texte.

Pour pointer une cellule d'une autre feuille, cliquez sur l'onglet de cette feuille pendant que le curseur se trouve à un endroit où une cellule peut figurer, puis cliquez sur la cellule. Le nom de la feuille est écrit avec elle, entre apostrophes lorsqu'il en faut, comme dans 'Ma feuille'!B3. **Entrée** vous ramène à la feuille de départ et place la formule dans sa cellule. **Échap** annule tout, y compris le changement de feuille.

Vous pouvez aussi faire glisser la bordure d'une plage encadrée sur la feuille pour déplacer cette adresse, ou faire glisser son carré de coin pour l'étendre ; la formule change pendant le glissement.

Appuyez sur **F4** lorsque le curseur se trouve dans une adresse ou juste après elle pour fixer certaines parties à l'aide du signe dollar : A1 devient $A$1, puis A$1, puis $A1, puis de nouveau A1.

Chaque adresse de la formule est colorée dans le texte, et les cellules qu'elle désigne sont encadrées sur la feuille dans la même couleur, ce qui permet de voir d'un coup d'œil ce que la formule lit. Un clic après une adresse complète, comme =A1, termine la saisie et sélectionne la cellule sur laquelle vous avez cliqué, comme pour tout autre texte.

## Tables de données

Une table de données (table de simulation) montre le résultat d'une formule pour de nombreuses valeurs d'une ou de deux cellules. Lorsque vous ouvrez un classeur qui en contient une, JeffCrunch calcule ses cellules : pour une table à deux entrées, la formule se trouve dans la cellule située en haut à gauche de la table, les valeurs à essayer en ligne sont placées dans la cellule d'entrée en ligne, et celles en colonne dans la cellule d'entrée en colonne. Pour une table à une entrée, les valeurs sont placées en colonne à gauche de la table (et les formules se trouvent dans la ligne au-dessus), ou en ligne au-dessus de la table (et les formules se trouvent dans la colonne à gauche). Chaque cellule de la table est le résultat de la formule, ainsi que de chaque formule qui dépend de la cellule d'entrée, avec cette entrée fixée à la valeur correspondante ; rien d'autre dans le classeur n'est modifié. En mode **Automatique**, les tables sont calculées avec les formules, et les formules qui lisent les cellules d'une table suivent. La création d'une nouvelle table de données n'est pas encore proposée. Une table qui ne peut pas être calculée (une formule qui dépend de l'entrée utilise une fonction que JeffCrunch ne connaît pas, ou change d'elle-même, comme ALEA ou MAINTENANT) conserve les valeurs de son fichier.

## Calculer manuellement

Cliquez sur **Mode de calcul** dans l'onglet **Formules** pour choisir la manière dont le classeur calcule ses formules :

- **Automatique** calcule les formules au fur et à mesure que vous modifiez les cellules.
- **Automatique sauf tables de données** fait de même, mais laisse les cellules des tables de données telles quelles jusqu'à ce que vous appuyiez sur **F9**.
- **Manuel** ne calcule rien, tables de données comprises, tant que vous ne le demandez pas. La barre d'état affiche **Non à jour** après une modification. Appuyez sur **F9** (**Calculer tout**) pour tout calculer, ou sur **Maj+F9** (**Recalculer cette feuille**) pour calculer les formules de la feuille active ainsi que tout ce qui attendait. Un classeur réglé sur Manuel s'ouvre avec les résultats enregistrés dans son fichier. **Calculer avant l'enregistrement**, également dans le menu **Mode de calcul**, est coché par défaut : lorsque vous enregistrez un classeur en mode Manuel qui n'est pas à jour, les formules sont calculées d'abord, de sorte que le fichier contient des nombres à jour. Désactivez cette option pour enregistrer les nombres tels quels.

Le choix est enregistré avec le classeur, et c'est une étape que vous pouvez annuler.

## Formules qui se lisent elles-mêmes

Une formule qui lit son propre résultat, directement ou par l'intermédiaire d'autres cellules, ne peut pas être calculée de la manière habituelle. JeffCrunch vous en avertit une fois, indique les cellules concernées et les encadre sur la feuille ; elles conservent les valeurs qu'elles avaient. Cliquez sur **Références circulaires** dans l'onglet **Formules** pour afficher la liste, et double-cliquez sur une cellule de la liste pour vous y rendre.

Certains modèles ont besoin d'une telle boucle volontairement. Choisissez **Calcul itératif des formules circulaires** dans le menu **Mode de calcul** : les formules sont recalculées encore et encore, chacune lisant les dernières valeurs des autres, jusqu'à ce qu'aucune valeur ne varie de plus de la quantité que vous indiquez, ou jusqu'à ce que le nombre de passages autorisé soit atteint.

## Garder les nombres tels qu'affichés dans les cellules

Une cellule peut afficher moins de chiffres qu'elle n'en contient. Si une cellule contient 2,675 et affiche 2,68, les formules qui la lisent utilisent toujours 2,675. Il arrive que vous souhaitiez qu'elles utilisent ce que vous voyez. Choisissez alors **Définir la précision comme affichée** dans le menu **Options de calcul** de l'onglet **Formules**.

JeffCrunch vous prévient d'abord, car les chiffres masqués seront perdus. Choisissez **Annuler** et rien ne change. Choisissez **OK** et chaque nombre du classeur est arrondi à ce que sa cellule affiche. Une cellule qui affiche deux décimales en conserve deux. Une cellule qui affiche un pourcentage conserve le pourcentage. Une cellule contenant une date conserve le jour, et une cellule contenant une heure conserve la plus petite unité de temps affichée, comme la minute. Une cellule qui affiche **Standard** conserve tous ses chiffres. Pour un gros classeur, une note accompagnée d'un bouton **Annuler** s'affiche pendant ce traitement. Cliquez sur **Annuler** pendant que la note est affichée, et rien n'est modifié.

Ensuite, chaque nombre que vous saisissez ou collez, et chaque résultat donné par une formule, est arrondi de la même manière. Les formules elles-mêmes ne sont pas modifiées. Le choix est enregistré avec le classeur.

L'arrondi est une étape que vous pouvez annuler : **Annuler** rétablit les nombres d'origine. Choisir de nouveau **Définir la précision comme affichée** désactive l'option. Cela empêche seulement l'arrondi à partir de ce moment : les nombres déjà arrondis restent arrondis.

## Voir ce que lit une formule

Sélectionnez une cellule et cliquez sur **Repérer les antécédents** pour tracer des flèches depuis les cellules que lit sa formule. Cliquez à nouveau pour remonter d'un cran. **Repérer les dépendants** trace des flèches vers les formules qui lisent la cellule, et chaque clic avance d'un cran. Une petite page représente une cellule d'une autre feuille. Au maximum 2 000 flèches sont tracées, et la barre d'état l'indique lorsqu'il y en avait davantage. **Effacer les flèches** les supprime, et toute modification du classeur les efface également.

Lorsqu'une cellule affiche une erreur telle que #DIV/0!, sélectionnez-la et cliquez sur **Tracer l'erreur**. Des flèches sont tracées vers les cellules qu'elle lit et qui affichent elles aussi une erreur. Cliquez à nouveau pour remonter d'un cran. Lorsqu'aucune cellule qu'elle lit n'affiche d'erreur, la barre d'état indique que cette cellule est à l'origine de l'erreur, et c'est celle qu'il faut corriger.

Pour parcourir toutes les erreurs de la feuille, cliquez sur **Erreur suivante** ou **Erreur précédente**. Les cellules qui affichent une erreur sont visitées ligne après ligne, et après la dernière on repart de la première. Chaque cellule est sélectionnée, et les flèches sont tracées jusqu'à la cellule qui est à l'origine de l'erreur. La barre d'état indique l'erreur en cours et le nombre total d'erreurs.

## Vérification des erreurs

Choisissez **Vérification des erreurs** dans le groupe **Vérifier** de l'onglet Formules pour rechercher dans la feuille les cellules qui pourraient être incorrectes. Elle recherche une formule qui aboutit à une erreur (comme #DIV/0!), un nombre enregistré sous forme de texte (que les formules peuvent ignorer), et d'autres cas : une formule différente de celles qui l'entourent, une date saisie sous forme de texte avec une année à deux chiffres, une formule dans une cellule qui n'est pas verrouillée, une formule qui omet un nombre à côté des cellules qu'elle utilise (uniquement pour une fonction qui additionne une plage, comme **SOMME**, **MOYENNE**, **NB**, **MIN** ou **MAX**, lorsque la plage constitue seule un argument), et une formule qui utilise une cellule vide.

La boîte de dialogue affiche une cellule à la fois, se rend à cette cellule sur la feuille et indique en une phrase le problème. **Suivant** et **Précédent** parcourent les cellules. Un petit triangle vert dans le coin supérieur gauche d'une cellule la signale sur la feuille. Cliquez sur le triangle lui-même pour ouvrir un menu à côté de la cellule : il indique le problème, propose la correction lorsqu'il en existe une, et comporte **Ignorer l'erreur** et **Vérification des erreurs…**. Lorsque la cellule peut être corrigée, un bouton portant la correction apparaît (pour une formule qui omet un nombre, **Inclure la cellule**) ; il modifie la formule, et une étape d'**Annuler** rétablit l'ancienne formule. **Ignorer l'erreur** retire la cellule de la liste, supprime son triangle et est enregistré dans le fichier. Une étape d'**Annuler** la rétablit. Si rien n'est incorrect, un message vous le signale. Pour choisir les contrôles à signaler, ouvrez **Options** puis la page **Formules** : une case à cocher existe pour chacun, et une case décochée empêche de signaler ce type de problème. Ce choix est celui du programme, et il vaut pour toutes les fenêtres ouvertes. Une cellule peut présenter plusieurs problèmes, et **Ignorer l'erreur** n'ignore que celui qui est affiché.

## Évaluer la formule

Utilisez cette fonction pour voir comment une formule obtient sa valeur. Sélectionnez une cellule qui contient une formule et choisissez **Évaluer la formule**. La formule s'affiche avec la partie suivante soulignée. Appuyez sur **Évaluer** pour calculer cette partie ; sa valeur prend sa place. Les parties sont traitées dans cet ordre : chaque cellule isolée de gauche à droite, puis chaque fonction avec ses parenthèses, en commençant par la plus imbriquée, puis le reste. **Recommencer** reprend depuis le début. Si une partie ne peut pas être calculée, la boîte de dialogue indique la raison et laisse la formule inchangée.

## Volet Espion

Le **Volet Espion** est une petite fenêtre qui reste visible pendant votre travail. Il liste les cellules que vous souhaitez surveiller, avec leur feuille, leur cellule, leur valeur et leur formule, même lorsqu'elles se trouvent sur une autre feuille. Sélectionnez des cellules et appuyez sur **Ajouter un espion**. Sélectionnez des lignes de la liste et appuyez sur **Supprimer l'espion**. Double-cliquez sur une ligne pour atteindre la cellule. Les valeurs changent lorsque la feuille est recalculée. La liste est conservée tant que le classeur est ouvert.

## Formules qui remplissent les cellules voisines

Lorsqu'une formule donne un bloc de valeurs, ce bloc remplit les cellules situées sous la cellule où vous l'avez saisie et à côté d'elle. C'est ce qu'on appelle un débordement. Essayez =SEQUENCE(3) en A1 : A1 affiche 1, et A2 et A3 affichent 2 et 3. Une formule déborde lorsqu'elle appelle une fonction qui donne un bloc (TRI, TRIERPAR, FILTRE, UNIQUE, SEQUENCE, ALEA.MATRICE, TRANSPOSE, RECHERCHEX ou MMULT), écrit un bloc entre accolades, utilise une plage de débordement (voir ci-dessous) ou effectue un calcul sur une plage, comme dans =A1:A3*2.

- **Les cellules qu'il remplit ne contiennent aucune formule propre.** Elles affichent les valeurs. Lorsque vous en sélectionnez une, la barre de formule affiche en grisé la formule de la cellule qui la remplit. Appuyer sur Entrée sans rien modifier ne change rien. Pour modifier le bloc, modifiez d'abord cette première cellule.
- **Le bloc suit le résultat.** Si la formule donne davantage ou moins de valeurs après une modification des cellules qu'elle lit, le bloc s'agrandit ou se réduit, et les formules qui lisent ces cellules sont recalculées.
- **Une cellule gênante donne #SPILL!.** Si une cellule où le bloc devrait s'étendre contient une valeur ou une formule, la première cellule affiche **#SPILL!** et rien n'est rempli. Videz la cellule gênante et le bloc se remplit immédiatement. Saisir du texte dans une cellule que le bloc doit remplir produit le même effet : la première cellule affiche #SPILL!.
- **Un bloc a une taille limite.** Un bloc plus grand que la feuille, ou de plus de 200 000 cellules, donne #SPILL!.
- **A1# désigne le bloc entier.** Écrivez la cellule qui contient la formule suivie d'un **#**, comme dans =SOMME(A1#), pour utiliser le bloc entier tel qu'il est actuellement, depuis cette feuille ou une autre (=SOMME(Feuil1!A1#)). Si A1 ne déborde pas, A1# donne #REF!.

Un débordement est enregistré avec le fichier sous forme de matrice dynamique, de sorte que le programme d'origine du fichier l'affiche lui aussi comme un débordement. L'enregistrement au format .xls ou .ods plus ancien conserve les valeurs, ainsi que la formule sous forme de formule matricielle sur le bloc lorsque le format ancien possède les fonctions utilisées. Vérifier la compatibilité signale ces formules.

## Donner des noms à des valeurs dans une formule

**LET** nomme une valeur au milieu d'une formule, de sorte que vous l'écrivez une fois et que vous pouvez la réutiliser. Les couples nom et valeur viennent d'abord, et le calcul en dernier. Dans =LET(prix; B2; taxe; prix*0,08; prix+taxe), le nom **prix** désigne la valeur de B2, **taxe** est calculé à partir de celui-ci, et la dernière partie les additionne. Un nom peut utiliser les noms qui le précèdent, et un LET peut contenir jusqu'à 126 couples. Un nom ne fonctionne qu'à l'intérieur de son propre LET, et un LET imbriqué dans un autre peut réutiliser un nom. Un nom peut aussi désigner une plage de cellules, comme dans =LET(lignes; B2:B9; SOMME(lignes)/NB(lignes)).

## Interroger une cellule ou le classeur

**CELLULE** fournit des informations sur une cellule. Son premier argument indique lesquelles : « adresse », « ligne », « col », « contenu », « type », « format » (la manière dont les nombres s'affichent, comme « F2 » pour deux décimales, « P0 » pour un pourcentage ou « D1 » pour une date), « couleur » (1 lorsque les nombres négatifs s'affichent en couleur), « parenthèses » (1 lorsque les nombres positifs s'affichent entre parenthèses), « préfixe » (le signe d'un texte aligné à gauche, à droite ou centré), « protection » (1 lorsque la cellule est verrouillée), « largeur » (la largeur de la colonne en caractères) et « nomfichier » (le dossier, le fichier et la feuille, comme /home/moi/[Budget.xlsx]Feuil1 ; vide tant que le classeur n'est pas enregistré). **INFO** fournit des renseignements sur le programme et le classeur : « répertoire », « numfichier » (le nombre de feuilles affichées), « origine », « versionos », « recalc » (Automatique ou Manuel), « release » et « système ».

## Extraire des valeurs d'un XML

**FILTRE.XML** prend un texte qui contient du XML et un XPath, c'est-à-dire une manière abrégée de désigner les parties souhaitées, et donne les valeurs trouvées. =FILTRE.XML(A1 ; "//article/prix") donne chaque prix du texte de A1, les uns sous les autres. Utilisez « /magasin/article[2]/nom » pour le deuxième article, « //article[@id='b']/nom » pour l'article dont l'identifiant est b, ou « count(//article) » pour leur nombre. Si le texte n'est pas du XML valide, ou si le chemin n'est pas valide, le résultat est #VALEUR!. Si rien ne correspond, il est #N/A. Un XML qui comporte une déclaration de type de document est refusé.

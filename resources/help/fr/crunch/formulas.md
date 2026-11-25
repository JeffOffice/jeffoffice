# Comment les formules sont affichées et calculées
<!-- keywords: débordement, déversement, #SPILL!, matrice dynamique, plage de déversement, A1#, erreur suivante, erreur précédente, parcourir les erreurs, tracer l'erreur, calculer avant l'enregistrement, formule, formules, calculer, calcul, recalculer, fonction, fonctions, valeur, résultat, enregistré, F9, circulaire, afficher les formules, barre de formule, erreur, pointer, pointage, cliquer sur une cellule, référence, F4, vérification des erreurs, ignorer l'erreur, évaluer la formule, pas à pas, fenêtre Espion, espion, LET, nommer une valeur, fonction CELLULE, fonction INFO, FILTRE.XML, xml, xpath, précision telle qu'affichée, définir la précision comme affichée, arrondi, arrondir, décimales, chiffres-->

Une **formule** est une cellule qui calcule sa valeur à partir d'autres cellules, par exemple =SOMME(B3:B8). La cellule affiche la valeur, et la **barre de formule** affiche la formule.

## Ce que fait JeffCrunch avec les formules

Lorsque vous ouvrez un classeur, le fichier contient la dernière valeur de chaque formule, de sorte que la feuille peut s'afficher immédiatement. JeffCrunch recalcule ensuite chaque formule, quelques-unes à la fois, afin que la fenêtre reste fluide. La barre d'état affiche **Calcul en cours** avec un pourcentage pendant ce travail. Une fois le calcul terminé, les cellules contiennent les résultats de JeffCrunch. Pour tout recalculer, cliquez sur **Calculer tout** dans l'onglet **Formules**, ou appuyez sur **F9**.

## Lorsque JeffCrunch ne peut pas calculer une formule

JeffCrunch dispose de plus de 400 fonctions, et leur nombre augmente encore. Une formule qui utilise une fonction que JeffCrunch ne connaît pas encore conserve la valeur enregistrée dans le fichier. C'est aussi le cas d'une formule qui fait référence à un autre classeur. Les cellules qui dépendent de ces formules sont calculées à partir des valeurs enregistrées.

## Fonctions qui demandent quelques précisions

- **Obligations et emprunts.** PRIX, RENDEMENT, DURATION, PRIX.PCOUPON.IMPAIR, et les autres fonctions d'obligations comptent les jours selon la **base** indiquée (0 correspond à 30 jours par mois sur une année de 360 jours, 1 correspond aux jours réels, et ainsi de suite). RENDEMENT, RENDEMENT.PCOUPON.IMPAIR, RENDEMENT.DCOUPON.IMPAIR et TAUX trouvent leur résultat en essayant des valeurs jusqu'à ce que le prix ou le versement corresponde. Ils s'arrêtent lorsque le résultat est juste à environ 15 chiffres près ; les derniers chiffres peuvent donc différer de ceux d'un autre logiciel.
- **Lois de probabilité et tests.** Les fonctions qui se terminent par .DIST, .INV, .TEST, ainsi que les anciens noms comme LOI.NORMALE et LOI.STUDENT, donnent les mêmes résultats : un ancien nom est conservé pour que les anciens fichiers continuent de fonctionner. Les fonctions *.INV trouvent elles aussi leur résultat en essayant des valeurs.
- **Listes qui remplissent plusieurs cellules.** TRIER, TRIERPAR, FILTRE, UNIQUE, SEQUENCE, MATRICE.ALEA, RECHERCHEX, MMULT, MINVERSE et DROITEREG donnent un bloc de valeurs. Saisissez-en une dans une seule cellule : le bloc se déverse dans les cellules situées en dessous et à côté (voir **Formules qui remplissent les cellules voisines**), ou saisissez une formule matricielle (**Ctrl+Maj+Entrée**) sur les cellules à remplir.
- **Textes dans d'autres écritures.** LENB, GAUCHEB, STXT, DROITEB, TROUVERB, CHERCHERB et REMPLACERB comptent chaque caractère pour un seul, comme sur un ordinateur qui n'est pas configuré pour le texte à deux octets. PHONETIQUE donne le texte de la cellule, car JeffCrunch ne conserve pas les lectures du texte japonais.
- **CONVERT.** Les unités s'écrivent sous forme de texte, avec les majuscules et les minuscules indiquées dans la liste de la fonction : « kg », « lbm », « mi », « F ». Un préfixe métrique peut précéder une unité qui l'accepte, comme « km » ou « mL ».

## MOD : le reste

**MOD(n ; d)** donne le reste de **n** après que le plus grand nombre entier de fois **d** a été retiré : il vaut n - d * ENT(n / d). Exemple : **=MOD(10 ; 3)** vaut 1, et **=MOD(-10 ; 3)** vaut 2, car ENT(-10 / 3) vaut -4 et -10 - 3 * -4 vaut 2. Le résultat a le signe de **d**. Si **d** vaut 0, le résultat est #DIV/0!.

## Afficher les formules

Cliquez sur **Afficher les formules** dans l'onglet **Formules**, ou appuyez sur **Ctrl+'** (sur Mac, la touche Contrôle et non la touche Commande), pour voir la formule de chaque cellule à la place de sa valeur. Les colonnes qui contiennent des formules sont élargies, jusqu'à une certaine limite, pour que les formules s'affichent en entier ; elles retrouvent leur largeur initiale lorsque vous désactivez cette option, et les largeurs de votre fichier ne sont pas modifiées.

## Modifier une formule

Pour modifier une formule, sélectionnez sa cellule, appuyez sur **F2**, modifiez-la, puis appuyez sur **Entrée**. Vous pouvez aussi la modifier dans la barre de formule. Lorsque vous changez une cellule, les formules qui en dépendent sont recalculées en arrière-plan. Voir [Saisir du texte dans les cellules](edit-cells).

## Écrire une formule

Tapez **=** puis les premières lettres d'une fonction : une courte liste affiche les fonctions et les [noms](names) qui commencent ainsi, accompagnés d'une phrase les décrivant. Appuyez sur **Tab** pour choisir celle qui est marquée. Utilisez les flèches **Bas** et **Haut** pour en marquer une autre, puis **Tab** ou **Entrée**. **Échap** ferme la liste. Une fois à l'intérieur des parenthèses d'une fonction, ses arguments s'affichent sous la cellule, celui que vous saisissez en gras. La liste et les arguments sont les mêmes que ceux affichés par **Insérer une fonction**. Les arguments sont séparés par des virgules, ou par des points-virgules lorsque votre langue utilise la virgule comme séparateur décimal.

## Noms de fonctions dans votre langue

Lorsque JeffCrunch s'affiche en allemand, français, espagnol, italien, néerlandais, portugais ou russe, vous pouvez saisir une formule avec les noms de fonctions de votre langue, par exemple `=SUMME(A1;A2)` en allemand ou `=SOMME(A1;A2)` en français. Les noms anglais fonctionnent aussi, de sorte qu'une formule copiée depuis le web n'a pas besoin d'être modifiée. La barre de formule, la cellule, la liste des fonctions et **Insérer une fonction** affichent les noms de votre langue.

Le classeur ne change pas lorsque vous changez de langue. Un fichier conserve toujours les noms anglais : un classeur créé dans une langue s'ouvre et se lit correctement dans une autre, et dans tout autre logiciel. Les noms locaux sont disponibles en allemand, français, espagnol, italien, néerlandais, portugais (Brésil et Portugal) et russe ; les noms anglais fonctionnent toujours, quelle que soit la langue. Les versions ukrainienne et indonésienne conservent les noms anglais pour presque toutes les fonctions. Lorsqu'une langue possède deux noms courants pour une fonction, les deux sont reconnus et le plus usuel est affiché. Une fonction qui n'a pas de nom usuel dans votre langue garde son nom anglais. VRAI et FAUX suivent aussi votre langue. Les valeurs d'erreur de votre langue, comme `#NV` en allemand, sont reconnues lorsque vous saisissez une formule et sont affichées dans la barre de formule, mais une cellule en erreur affiche toujours la forme standard, comme `#N/A`.

Lorsque les paramètres régionaux de votre ordinateur utilisent la virgule comme séparateur décimal, les arguments d'une fonction sont séparés par un point-virgule. Une matrice entre accolades comportant plusieurs lignes s'affiche avec un point-virgule entre les valeurs d'une ligne et une barre oblique inverse entre les lignes, comme `{1;2\3;4}`, et vous pouvez la saisir ainsi. Une matrice saisie sans barre oblique inverse est lue comme un fichier l'écrit : des virgules entre les valeurs d'une ligne, des points-virgules entre les lignes, et un point pour le séparateur décimal.

Pour ne saisir que des noms anglais, cliquez sur **Fichier > Options > Formules** et cochez **Utiliser les noms de fonctions anglais**.

## Pointer les cellules

Au lieu de saisir l'adresse d'une cellule, vous pouvez la désigner à la souris. Pendant la saisie d'une formule, placez le curseur à un endroit où une cellule peut être indiquée (juste après **=**, après un opérateur comme **+**, après une parenthèse ouvrante ou après une virgule), puis cliquez sur la cellule. Son adresse est écrite à votre place. Faites glisser sur des cellules pour écrire une plage, comme B2:D5. Cliquez de nouveau avant de saisir autre chose, et la nouvelle cellule remplace celle que vous venez d'écrire. Maintenez **Ctrl** et cliquez pour ajouter une autre adresse après une virgule. Cliquez sur une lettre de colonne pour écrire une colonne entière, comme A:A, ou sur un numéro de ligne pour écrire une ligne entière, comme 3:3 ; faites glisser le long des lettres ou des numéros pour obtenir B:D ou 2:5.

Les touches fléchées fonctionnent aussi. Appuyez sur une flèche : un pointeur s'éloigne de la cellule que vous êtes en train de saisir et son adresse est écrite ; appuyez à nouveau sur les flèches pour continuer, ou maintenez **Maj** pour étendre le pointeur sur une plage. La barre d'état affiche **Pointage** lorsque les touches fléchées désignent des cellules, **Entrée** pendant la saisie, et **Modification** lorsque les touches fléchées déplacent le curseur dans le texte. Appuyez sur **F2** pour basculer entre le pointage et la modification du texte.

Pour désigner une cellule d'une autre feuille, cliquez sur l'onglet de cette feuille pendant que le curseur se trouve à un endroit où une cellule peut être indiquée, puis cliquez sur la cellule. Le nom de la feuille est écrit avec elle, entre apostrophes lorsque c'est nécessaire, comme dans 'Ma feuille'!B3. **Entrée** vous ramène sur la feuille de départ et place la formule dans sa cellule. **Échap** annule tout, y compris le changement de feuille.

Vous pouvez aussi faire glisser le bord d'un bloc encadré sur la feuille pour déplacer cette adresse, ou faire glisser son carré d'angle pour l'étirer ; la formule change pendant le glissement.

Appuyez sur **F4** lorsque le curseur est dans une adresse, ou juste après elle, pour fixer certaines parties avec un signe dollar : A1 devient $A$1, puis A$1, puis $A1, puis de nouveau A1.

Chaque adresse de la formule est colorée dans le texte, et les cellules qu'elle désigne sont encadrées sur la feuille de la même couleur, afin de voir d'un coup d'œil ce que la formule lit. Un clic après une adresse complète, comme =A1, termine la saisie et sélectionne la cellule sur laquelle vous avez cliqué, comme pour tout autre texte.

## Tableaux de données

Un tableau de données (tableau d'hypothèses) montre le résultat d'une formule pour de nombreuses valeurs d'une ou de deux cellules. Lorsque vous ouvrez un classeur qui en contient un, JeffCrunch calcule ses cellules : pour un tableau à deux variables, la formule se trouve dans la cellule en haut à gauche du tableau, les valeurs à essayer en ligne sont placées dans la cellule de saisie en ligne, et celles en colonne dans la cellule de saisie en colonne. Pour un tableau à une variable, les valeurs sont placées dans la colonne à gauche (et les formules se trouvent dans la ligne au-dessus), ou en ligne au-dessus (et les formules se trouvent dans la colonne à gauche). Chaque cellule du tableau est le résultat de la formule, et de chaque formule qui dépend de la cellule de saisie, lorsque cette cellule prend la valeur donnée ; rien d'autre dans le classeur n'est modifié. En mode **Automatique**, les tableaux sont calculés avec les formules, et les formules qui lisent les cellules d'un tableau suivent. La création d'un nouveau tableau de données n'est pas encore proposée. Un tableau qui ne peut pas être calculé (une formule qui dépend de la saisie utilise une fonction que JeffCrunch ne connaît pas, ou change d'elle-même, comme ALEA ou MAINTENANT) conserve les valeurs de son fichier.

## Calcul manuel

Cliquez sur **Mode de calcul** dans l'onglet **Formules** pour choisir la manière dont le classeur calcule ses formules :

- **Automatique** calcule les formules au fur et à mesure que vous modifiez les cellules.
- **Automatique, tables en manuel** fait de même, mais laisse les cellules des tableaux de données telles quelles jusqu'à ce que vous appuyiez sur **F9**.
- **Manuel** ne calcule rien, tableaux de données compris, tant que vous ne le demandez pas. La barre d'état affiche **À recalculer** après une modification. Appuyez sur **F9** (**Calculer tout**) pour tout calculer, ou sur **Maj+F9** (**Calculer cette feuille**) pour calculer les formules de la feuille active, ainsi que tout ce qui attendait. Un classeur réglé sur Manuel s'ouvre avec les résultats enregistrés dans son fichier. **Calculer avant l'enregistrement**, également dans le menu **Mode de calcul**, est coché par défaut : lorsque vous enregistrez un classeur en mode Manuel qui n'est pas à jour, les formules sont calculées d'abord, de sorte que le fichier contienne des nombres à jour. Décochez cette option pour enregistrer les nombres tels qu'ils sont.

Le choix est enregistré avec le classeur, et il s'agit d'une étape que vous pouvez annuler.

## Formules qui lisent leur propre résultat

Une formule qui lit son propre résultat, directement ou par l'intermédiaire d'autres cellules, ne peut pas être calculée de la manière habituelle. JeffCrunch vous le signale une fois, indique les cellules concernées et les encadre sur la feuille ; elles conservent les valeurs qu'elles avaient. Cliquez sur **Références circulaires** dans l'onglet **Formules** pour afficher la liste, puis double-cliquez sur une cellule de la liste pour y aller.

Certains modèles nécessitent volontairement une telle boucle. Choisissez **Calcul itératif des formules circulaires** dans le menu **Mode de calcul** : les formules sont recalculées encore et encore, chacune lisant les dernières valeurs des autres, jusqu'à ce qu'aucune valeur ne varie de plus que la quantité indiquée ou jusqu'à ce que le nombre d'itérations autorisé soit atteint.

## Garder les nombres tels qu'affichés dans les cellules

Une cellule peut afficher moins de chiffres qu'elle n'en contient. Si une cellule contient 2,675 et affiche 2,68, les formules qui la lisent utilisent toujours 2,675. Il arrive que vous vouliez qu'elles utilisent ce que vous voyez. Choisissez alors **Définir la précision comme affichée** dans le menu **Options de calcul** de l'onglet **Formules**.

JeffCrunch vous prévient d'abord, car les chiffres masqués seront perdus. Choisissez **Annuler** pour que rien ne change. Choisissez **OK** pour que chaque nombre du classeur soit arrondi à ce qu'affiche sa cellule. Une cellule qui affiche deux décimales en conserve deux. Une cellule qui affiche un pourcentage conserve le pourcentage. Une cellule contenant une date conserve le jour, et une cellule contenant une heure conserve la plus petite unité de temps affichée, comme la minute. Une cellule qui affiche **Standard** conserve tous ses chiffres. Un classeur volumineux affiche pendant ce traitement une note accompagnée d'un bouton **Annuler** ; si vous annulez pendant que la note est affichée, rien n'est modifié.

Ensuite, chaque nombre que vous saisissez ou collez, et chaque résultat fourni par une formule, est arrondi de la même manière. Les formules elles-mêmes ne sont pas modifiées. Le choix est enregistré avec le classeur.

L'arrondi est une étape que vous pouvez annuler : **Annuler** rétablit les nombres d'origine. Choisir de nouveau **Définir la précision comme affichée** désactive cette option. Cela empêche seulement l'arrondi à partir de ce moment ; les nombres déjà arrondis restent arrondis.

## Voir ce qu'une formule lit

Sélectionnez une cellule et cliquez sur **Repérer les antécédents** pour tracer des flèches depuis les cellules que sa formule lit. Cliquez de nouveau pour remonter d'un cran. **Repérer les dépendants** trace des flèches vers les formules qui lisent la cellule, et chaque clic avance d'un cran. Une petite page représente une cellule située sur une autre feuille. Au maximum 2 000 flèches sont tracées, et la barre d'état indique s'il y en avait davantage. **Effacer les flèches** les supprime, et toute modification du classeur les supprime aussi.

Lorsqu'une cellule affiche une erreur comme #DIV/0!, sélectionnez-la et cliquez sur **Tracer l'erreur**. Des flèches sont tracées vers les cellules qu'elle lit et qui affichent aussi une erreur. Cliquez de nouveau pour remonter d'un cran. Lorsqu'aucune cellule qu'elle lit n'affiche d'erreur, la barre d'état indique que cette cellule est à l'origine de l'erreur, et c'est celle qu'il faut corriger.

Pour parcourir toutes les erreurs de la feuille, cliquez sur **Erreur suivante** ou **Erreur précédente**. Les cellules qui affichent une erreur sont parcourues ligne après ligne, et après la dernière on recommence à la première. Chaque cellule est sélectionnée, et les flèches sont tracées jusqu'à la cellule à l'origine de l'erreur. La barre d'état indique l'erreur en cours et le nombre total d'erreurs.

## Vérification des erreurs

Choisissez **Vérification des erreurs** dans le groupe **Vérification** de l'onglet Formules pour rechercher dans la feuille les cellules qui pourraient être incorrectes. Elle recherche une formule qui aboutit à une erreur (comme #DIV/0!), un nombre enregistré sous forme de texte (que les formules peuvent ignorer), et d'autres cas : une formule différente de celles qui l'entourent, une date saisie sous forme de texte avec une année à deux chiffres, une formule dans une cellule non verrouillée, une formule qui omet un nombre à côté des cellules qu'elle utilise (uniquement pour une fonction qui additionne une plage, comme **SOMME**, **MOYENNE**, **NB**, **MIN** ou **MAX**, lorsque la plage est seule en argument), et une formule qui utilise une cellule vide.

La boîte de dialogue affiche une cellule à la fois, va à cette cellule dans la feuille et explique en une phrase le problème. **Suivant** et **Précédent** parcourent les cellules. Un petit triangle vert dans le coin supérieur gauche d'une cellule la signale sur la feuille. Lorsque la cellule peut être corrigée, un bouton portant la correction apparaît (pour une formule qui omet un nombre, **Inclure la cellule**) ; il modifie la formule, et une étape d'**Annuler** rétablit l'ancienne formule. **Ignorer l'erreur** retire la cellule de la liste, supprime son triangle et est enregistré dans le fichier. Une étape d'**Annuler** le rétablit. Si rien n'est incorrect, un message vous le signale. Pour choisir les vérifications à effectuer, ouvrez **Options** puis la page **Formules** : une case à cocher existe pour chacune, et une case décochée empêche de signaler ce type de problème. Ce choix appartient au programme et s'applique à toutes les fenêtres ouvertes. Une cellule peut présenter plusieurs problèmes, et **Ignorer l'erreur** n'ignore que celui qui est affiché.

## Évaluer la formule

Utilisez cette fonction pour voir comment une formule obtient sa valeur. Sélectionnez une cellule qui contient une formule et choisissez **Évaluer la formule**. La formule s'affiche avec la partie suivante soulignée. Appuyez sur **Évaluer** pour calculer cette partie ; sa valeur prend sa place. Les parties sont traitées dans cet ordre : chaque cellule isolée de gauche à droite, puis chaque fonction avec ses parenthèses, en commençant par la plus interne, puis le reste. **Recommencer** reprend depuis le début. Si une partie ne peut pas être calculée, la boîte de dialogue en indique la raison et laisse la formule inchangée.

## Fenêtre Espion

La **fenêtre Espion** est une petite fenêtre qui reste visible pendant votre travail. Elle liste les cellules que vous voulez surveiller, avec leur feuille, leur cellule, leur valeur et leur formule, même lorsqu'elles se trouvent sur une autre feuille. Sélectionnez des cellules et appuyez sur **Ajouter un espion**. Sélectionnez des lignes de la liste et appuyez sur **Supprimer l'espion**. Double-cliquez sur une ligne pour aller à la cellule. Les valeurs changent lorsque la feuille est recalculée. La liste est conservée tant que le classeur est ouvert.

## Formules qui remplissent les cellules voisines

Lorsqu'une formule donne un bloc de valeurs, ce bloc remplit les cellules situées en dessous et à côté de la cellule dans laquelle vous l'avez saisie. On parle de déversement. Essayez =SEQUENCE(3) en A1 : A1 affiche 1, et A2 et A3 affichent 2 et 3. Une formule se déverse lorsqu'elle appelle une fonction qui donne un bloc (TRIER, TRIERPAR, FILTRE, UNIQUE, SEQUENCE, MATRICE.ALEA, TRANSPOSE, RECHERCHEX ou MMULT), écrit un bloc entre accolades, utilise une plage de déversement (voir ci-dessous), ou effectue un calcul sur une plage, comme =A1:A3*2.

- **Les cellules qu'il remplit ne contiennent pas de formule.** Elles affichent les valeurs. Lorsque vous en sélectionnez une, la barre de formule affiche en grisé la formule de la cellule qui la remplit. Appuyer sur Entrée sans rien modifier ne change rien. Pour modifier le bloc, modifiez d'abord cette première cellule.
- **Le bloc suit le résultat.** Si la formule donne davantage ou moins de valeurs après la modification de ce qu'elle lit, le bloc s'agrandit ou se réduit, et les formules qui lisent ces cellules sont recalculées.
- **Une cellule gênante donne #SPILL!.** Si l'une des cellules où le bloc devrait aller contient une valeur ou une formule, la première cellule affiche **#SPILL!** et rien n'est rempli. Effacez la cellule gênante, et le bloc se remplit aussitôt. Saisir du texte dans une cellule que le bloc remplit produit le même effet : la première cellule affiche #SPILL!.
- **Un bloc a une taille limite.** Un bloc plus grand que la feuille, ou de plus de 200 000 cellules, donne #SPILL!.
- **A1# désigne le bloc entier.** Écrivez la cellule qui contient la formule suivie d'un **#**, comme dans =SOMME(A1#), pour utiliser tout le bloc tel qu'il est actuellement, depuis cette feuille ou une autre (=SOMME(Feuil1!A1#)). Si A1 ne se déverse pas, A1# donne #REF!.

Un déversement est enregistré avec le fichier sous forme de matrice dynamique, de sorte que le logiciel d'origine du fichier l'affiche aussi comme un déversement. L'enregistrement au format .xls ou .ods plus ancien conserve les valeurs, et la formule sous forme de formule matricielle sur le bloc lorsque ce format ancien dispose des fonctions utilisées. La fonction Vérifier la compatibilité signale ces formules.

## Donner des noms à des valeurs dans une formule

**LET** attribue un nom à une valeur au milieu d'une formule, de sorte que vous l'écrivez une fois et la réutilisez. Les couples nom et valeur viennent en premier, et le calcul en dernier. Dans =LET(prix; B2; taxe; prix*0,08; prix+taxe), le nom **prix** représente la valeur de B2, **taxe** est calculée à partir de lui, et la dernière partie les additionne. Un nom peut utiliser les noms qui le précèdent, et un LET peut contenir jusqu'à 126 couples. Un nom ne fonctionne qu'à l'intérieur de son propre LET, et un LET placé dans un autre peut réutiliser un nom. Un nom peut aussi représenter une plage de cellules, comme dans =LET(lignes; B2:B9; SOMME(lignes)/NB(lignes)).

## Interroger une cellule ou le classeur

**CELLULE** donne des informations sur une cellule. Son premier argument indique lesquelles : « adresse », « ligne », « col », « contenu », « type », « format » (la manière dont les nombres s'affichent, comme « F2 » pour deux décimales, « P0 » pour un pourcentage, ou « D1 » pour une date), « couleur » (1 lorsque les nombres négatifs s'affichent en couleur), « parenthèses » (1 lorsque les nombres positifs s'affichent entre parenthèses), « prefixe » (le signe d'un texte aligné à gauche, à droite ou centré), « protection » (1 lorsque la cellule est verrouillée), « largeur » (la largeur de la colonne en caractères), et « nomfichier » (le dossier, le fichier et la feuille, comme /home/moi/[Budget.xlsx]Feuil1 ; vide tant que le classeur n'est pas enregistré). **INFO** renseigne sur le programme et le classeur : « repertoire », « numfichier » (le nombre de feuilles affichées), « origine », « osversion », « recalcul » (Automatique ou Manuel), « release » et « systeme ».

## Extraire des valeurs d'un XML

**FILTRE.XML** prend un texte contenant du XML et un XPath, c'est-à-dire une manière courte d'indiquer les parties souhaitées, et donne les valeurs trouvées. =FILTRE.XML(A1; "//article/prix") donne chaque prix du texte en A1, les uns sous les autres. Utilisez "/magasin/article[2]/nom" pour le deuxième article, "//article[@id='b']/nom" pour l'article dont l'identifiant est b, ou "count(//article)" pour connaître leur nombre. Si le texte n'est pas du XML valide, ou si le chemin n'en est pas un, le résultat est #VALEUR! Si rien ne correspond, le résultat est #N/A. Le XML qui contient une déclaration de type de document est refusé.

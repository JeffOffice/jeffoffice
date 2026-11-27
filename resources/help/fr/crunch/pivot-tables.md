# Tableaux croisés dynamiques
<!-- keywords: autre fichier, autre classeur, source externe, formules de liste, champ calculé, élément calculé, ordre de calcul, champ de formule, pages de filtre du rapport, tableau croisé dynamique, tableau croisé, explorer en détail, synthétiser, insérer un tableau croisé dynamique, créer, faire, sélectionner, déplacer le tableau croisé dynamique, LIREDONNEESTABCROISDYNAMIQUE, générer LIREDONNEESTABCROISDYNAMIQUE, actualiser, tout actualiser, modifier la source de données, liste des champs, champs, lignes, colonnes, valeurs, filtres, filtre de rapport, total général, sous-total, disposition, compact, plan, tabulaire, effacer, paramètres du champ de valeur, somme, nombre, moyenne, afficher les valeurs sous forme de, pourcentage du total, différence, total cumulé, format de nombre, paramètres du champ, trier, filtrer, premiers, regrouper, dissocier, dates, mois, années, développer, réduire, options, style, lignes à bandes, lignes vides, répéter les étiquettes, actualiser à l'ouverture du fichier, graphique croisé dynamique, segment, chronologie, afficher les détails, explorer en détail, lignes derrière une valeur -->

Un tableau croisé dynamique est un petit rapport établi à partir d'une longue liste. La liste compte une ligne par vente, par exemple ; le rapport additionne les ventes par région et par mois. Vous n'écrivez aucune formule. Vous indiquez quelles colonnes de la liste vont en lignes à gauche, lesquelles en colonnes en haut, et laquelle est additionnée : le rapport se remplit tout seul.

## Un exemple

Voici une liste dans les cellules A1 à C7. Chaque ligne correspond à une vente.

| Région | Mois | Montant |
|---|---|---|
| Est | Jan | 10 |
| Est | Jan | 5 |
| Est | Fév | 20 |
| Ouest | Jan | 7 |
| Ouest | Fév | 3 |
| Ouest | Fév | 4 |

Un tableau croisé dynamique avec **Région** en lignes, **Mois** en colonnes et la somme de **Montant** au centre ressemble à ceci.

| Somme de Montant | Mois | | |
|---|---|---|---|
| Région | Jan | Fév | Total général |
| Est | 15 | 20 | 35 |
| Ouest | 7 | 7 | 14 |
| Total général | 22 | 27 | 49 |

L'Est en janvier vaut 15, car les deux lignes Est de janvier, 10 et 5, sont additionnées. La colonne Total général additionne chaque ligne, la ligne Total général additionne chaque colonne, et la cellule du coin additionne tout.

Les mots employés ici :

- Un **champ** est une colonne de la liste. Région, Mois et Montant sont des champs.
- **Lignes**, **Colonnes** et **Valeurs** sont les trois emplacements où un champ peut être placé. Un champ placé dans Lignes donne une ligne pour chacune de ses valeurs différentes ; un champ placé dans Colonnes donne une colonne pour chacune ; un champ placé dans Valeurs est celui qui est additionné. Un quatrième emplacement, **Filtres**, contient un champ qui exclut les lignes que vous ne choisissez pas.
- Un **cache** (le cache du tableau croisé dynamique) est une copie de votre liste que le tableau conserve dans le fichier, afin de pouvoir afficher des totaux sans lire les cellules. Le rapport est calculé à partir du cache, et non à partir des cellules : il ne change donc pas lorsque les cellules changent. **Actualiser** recopie les cellules dans le cache.

## Créer un tableau croisé dynamique

1. Cliquez dans une cellule de la liste, ou sélectionnez les cellules à utiliser. La liste doit comporter une ligne de noms au-dessus des données.
2. Choisissez l'onglet **Insertion**, puis **Tableau croisé dynamique**.
3. La boîte indique les cellules contenant les données. Modifiez-les si elles ne conviennent pas. Vous pouvez aussi saisir le nom d'un tableau. Pour utiliser une liste située dans un autre fichier, choisissez **Une plage dans un autre fichier**, sélectionnez le fichier avec **Parcourir**, puis saisissez la plage, ou le nom d'un tableau ou d'un nom que vous avez défini dans ce fichier, par exemple `Feuil1!A1:D100`. JeffCrunch lit les classeurs (`.xlsx`, `.xlsm`, `.xls` et `.ods`) ainsi que les fichiers `.csv`.
4. Choisissez l'emplacement du tableau croisé dynamique : **Une nouvelle feuille** (par défaut), ou une cellule de cette feuille ou d'une autre. Choisissez un endroit où rien ne se trouve, car le tableau vide occupe une vingtaine de lignes et trois colonnes.
5. Choisissez **OK**. Le tableau démarre vide, et le volet **Champs de tableau croisé dynamique** s'ouvre. Cochez les champs souhaités : un champ de nombres est additionné, et tout autre champ devient une ligne. Placez ensuite les champs dans les zones, comme l'indiquent les sections suivantes.

Lorsque les données se trouvent dans un autre fichier, le tableau croisé dynamique conserve une copie de celles-ci ; le tableau fonctionne et s'enregistre donc sans ce fichier. Voir Données dans un autre fichier ci-dessous.

Une nouvelle feuille constitue sa propre étape d'Annuler, et il en va de même pour le tableau. Lorsque vous enregistrez au format .xlsx, le tableau est écrit comme le programme d'origine l'écrit, de sorte qu'il s'ouvre là-bas en tant que tableau croisé dynamique. Une copie enregistrée au format .xls ou .ods conserve dans ses cellules les nombres affichés par le tableau, mais pas le tableau lui-même, et l'enregistrement vous en avertit.

## LIREDONNEESTABCROISDYNAMIQUE : lire un nombre dans un tableau croisé dynamique

`LIREDONNEESTABCROISDYNAMIQUE` donne le nombre qu'un tableau croisé dynamique affiche pour les valeurs, les lignes et les colonnes que vous indiquez. Pour l'exemple ci-dessus :

`=LIREDONNEESTABCROISDYNAMIQUE("Somme de Montant";$E$3;"Région";"Est";"Mois";"Jan")` donne 15.

- Le premier argument est le nom de la valeur, tel que le tableau l'affiche (par exemple Somme de Montant). Le nom du champ additionné (Montant) convient également.
- Le deuxième argument est une cellule quelconque du tableau croisé dynamique.
- Viennent ensuite des couples formés d'un champ et de l'un de ses éléments. Si vous indiquez seulement Région et Est, vous obtenez le total de l'Est, soit 35. Si vous n'indiquez rien, vous obtenez le total général, soit 49. Les champs que vous indiquez doivent être les premiers champs des lignes, ou les premiers champs des colonnes, dans l'ordre.
- Si le tableau n'affiche pas le nombre, la cellule affiche `#RÉF!`.

Lorsque vous cliquez sur une valeur d'un tableau croisé dynamique pendant la saisie d'une formule, JeffCrunch écrit cette formule à votre place, afin qu'elle lise toujours le bon nombre après que le tableau a été réorganisé. Pour désactiver cette fonction, choisissez **Options** dans l'onglet **Tableau croisé dynamique**, puis décochez la case concernant LIREDONNEESTABCROISDYNAMIQUE.

## Ce que JeffCrunch fait d'un tableau croisé dynamique

Un tableau croisé dynamique présent dans un classeur est affiché tel qu'il a été enregistré. Si vous n'y modifiez rien, le fichier est enregistré avec le tableau exactement tel qu'il était. Lorsque vous l'actualisez ou que vous le modifiez, JeffCrunch recalcule le rapport, place les nouveaux nombres dans les cellules et réécrit le tableau croisé dynamique afin que le programme qui l'a créé l'ouvre sans message d'erreur.

JeffCrunch calcule les tableaux croisés dynamiques qui additionnent, comptent, font la moyenne, etc., qui regroupent des dates et des nombres, et qui affichent les valeurs sous forme de pourcentage, de différence ou de total cumulé. Un tableau croisé dynamique qui affiche une valeur sous forme de rang ou de pourcentage d'un parent, qui contient des regroupements d'éléments créés à la main, ou qui utilise un modèle de données est affiché et conservé tel qu'il a été enregistré, et les commandes vous indiquent qu'elles ne peuvent pas le modifier.

Cliquez sur une cellule quelconque d'un tableau croisé dynamique. L'onglet **Tableau croisé dynamique** apparaît.

## Actualiser

**Actualiser** lit les cellules à partir desquelles le tableau croisé dynamique est établi, puis recalcule le rapport. Les nouvelles entrées s'ajoutent à celles qui existent déjà ; les entrées qui ont disparu sont retirées. La largeur des colonnes et la disposition que vous avez choisie restent en place. **Tout actualiser**, dans l'onglet **Données** comme dans l'onglet **Tableau croisé dynamique**, fait de même pour tous les tableaux croisés dynamiques du classeur.

Si le rapport devait déborder sur des cellules qui contiennent quelque chose, JeffCrunch n'actualise pas le tableau. Il vous indique les cellules qui gênent. Effacez-les ou déplacez le tableau croisé dynamique, puis actualisez de nouveau.

**Annuler** annule une actualisation en une seule étape.

## Modifier la source de données

**Modifier la source de données** demande les cellules dans lesquelles se trouve la liste, y compris sa première ligne de noms. Saisissez-les sous la forme `Feuil1!A1:D100`, ou saisissez le nom d'un tableau ou d'un nom que vous avez défini. JeffCrunch les lit et recalcule le rapport. Si un champ de la liste porte le même nom dans les nouvelles cellules, sa place dans le rapport est conservée.

Vous pouvez aussi faire pointer le tableau vers une plage située dans un autre fichier. Dans la boîte, choisissez **Une plage dans un autre fichier**, sélectionnez le fichier, puis saisissez la plage qu'il contient. Choisir **Une plage dans ce classeur** fait pointer le tableau de nouveau vers des cellules de ce classeur. Chacune de ces modifications constitue une étape d'Annuler.

## Données dans un autre fichier

Un tableau croisé dynamique établi à partir d'un autre fichier conserve à l'intérieur de ce classeur les données dont il est issu. Vous pouvez donc ouvrir, modifier et enregistrer le classeur même lorsque l'autre fichier n'est plus là. **Actualiser** relit l'autre fichier. Si le fichier a été déplacé, est absent, est trop volumineux, n'est pas un classeur, ou ne contient plus la plage, le tableau conserve les données qu'il avait, et un message indique que le fichier n'a pas pu être lu. Un fichier situé sur un autre ordinateur du réseau n'est pas lu tant que vous ne l'avez pas choisi vous-même dans **Modifier la source de données**. Une actualisation lit chaque fichier une seule fois, et au maximum 32 fichiers, 1 Go et 30 secondes au total ; au-delà, un tableau conserve ses données.

Si un classeur indique que son tableau doit être relu à l'ouverture, et que les données du tableau se trouvent dans un autre fichier, JeffCrunch vous demande d'abord votre accord, de la même manière que pour les liens vers d'autres classeurs. Tant que vous n'avez pas répondu oui, aucun fichier n'est lu.

Un classeur enregistré au format .xlsx mémorise l'emplacement du fichier. Choisir le fichier dans la boîte mémorise son adresse complète. Un classeur créé par un autre programme, dont le tableau fait référence à un fichier par un chemin court tel que `Prix.xlsx`, conserve ce chemin, et JeffCrunch cherche le fichier dans le même dossier que le classeur. Lorsque vous utilisez **Enregistrer sous** pour enregistrer le classeur dans un autre dossier, un chemin de ce type est réécrit afin de désigner toujours le même fichier. L'actualisation ne suit pas les liens que l'autre fichier peut avoir vers d'autres fichiers.

## Effacer

**Effacer** propose deux choix. **Effacer les filtres** supprime tous les filtres et affiche de nouveau toutes les entrées. **Tout effacer** retire tous les champs du rapport et laisse celui-ci vide, prêt à être construit de nouveau.

## Explorer en détail et synthétiser

## Pages de filtre du rapport

Un filtre se place au-dessus d'un tableau croisé dynamique et permet de n'examiner qu'un élément à la fois, comme un mois. **Afficher les pages de filtre du rapport**, dans l'onglet **Tableau croisé dynamique**, crée une page pour chaque élément à la fois. Chaque page est une nouvelle feuille, nommée d'après l'élément, avec sa propre copie du tableau. Le filtre de chaque copie est réglé sur cet élément. Si le tableau comporte plusieurs filtres, vous choisissez celui à utiliser. Un seul **Annuler** supprime toutes les pages.

Lorsqu'un rapport comporte plusieurs champs dans ses lignes, un élément du premier champ peut être développé ou réduit. Cliquez sur la cellule contenant le nom de l'élément, puis choisissez **Synthétiser** pour le réduire : les lignes du champ suivant placé sous lui sont masquées, et seul son total reste visible. **Explorer en détail** le développe de nouveau. Un élément du dernier champ n'a rien en dessous de lui. Chacune de ces actions se défait d'un seul Annuler.

## Sélectionner et déplacer

**Sélectionner**, dans l'onglet **Tableau croisé dynamique**, sélectionne une partie du rapport : **Tout le tableau croisé dynamique**, **Valeurs** (les nombres) ou **Étiquettes** (les noms en lignes ; s'il n'y en a pas, les noms en colonnes). **Déplacer le tableau croisé dynamique…** demande une cellule de la même feuille et redessine le rapport à partir de cette cellule. Les cellules que le rapport quitte sont vidées. Les formules qui pointent vers les cellules du rapport les suivent, comme lors d'un couper-coller. Si le nouvel emplacement contient quelque chose, JeffCrunch ne déplace pas le rapport et vous en avertit. Un seul **Annuler** le ramène à sa place.

## Le volet des champs

**Liste des champs**, dans l'onglet **Tableau croisé dynamique**, ouvre le volet **Champs de tableau croisé dynamique**. Il liste les champs de la liste, avec une case à cocher à côté de chacun, puis, en dessous, les quatre emplacements : **Filtres**, **Colonnes**, **Lignes** et **Valeurs**.

- Cochez un champ pour l'ajouter. Un champ qui ne contient que des nombres va dans **Valeurs** et est additionné ; tout autre champ va dans **Lignes**.
- Décochez un champ pour le retirer de partout.
- Cliquez sur un champ dans l'un des quatre emplacements, puis choisissez **Déplacer vers** pour le placer dans un autre. Les commandes **Monter** et **Descendre** modifient son ordre au sein de son emplacement, et **Supprimer le champ** le retire.
- Avec plusieurs valeurs, une ligne nommée **Valeurs** apparaît dans **Colonnes**. Déplacez-la vers **Lignes** pour placer les valeurs en lignes à gauche.
- **Paramètres du champ de valeur** (un double-clic sur une valeur, ou le bouton) définit le nom que le rapport affiche pour la valeur et son mode de calcul : **Somme**, **Nombre**, **Nombre de valeurs**, **Moyenne**, **Max**, **Min**, **Produit**, **Écart type** et **Écart type p** (l'écart type d'un échantillon, ou de l'ensemble des valeurs), ainsi que **Var** et **Var p** (la variance des mêmes ensembles). Il définit aussi la manière dont la valeur est affichée et son format de nombre. Voir la section suivante.
- **Paramètres du champ** (un double-clic sur un champ des zones Filtres, Colonnes ou Lignes, ou le bouton) règle un seul champ. Voir « Paramètres du champ ».

Chaque modification constitue une étape d'**Annuler**.

## Afficher les valeurs sous forme de, et format de nombre d'une valeur

Dans **Paramètres du champ de valeur**, **Afficher les valeurs sous forme de** modifie ce qu'affiche une cellule du rapport.

- **% du total général** : la cellule divisée par le total de l'ensemble du rapport. Dans l'exemple, l'Est en janvier vaut 15 sur 49, soit 30,6 %.
- **% du total de colonne** et **% du total de ligne** : la cellule divisée par le total de sa colonne, ou de sa ligne.
- **Index** : la comparaison de la cellule avec ce que donneraient sa ligne et sa colonne si les données étaient réparties uniformément. Un nombre supérieur à 1 indique une part plus grande que la part égale.
- **Différence avec**, **% de** et **% de la différence avec** : la cellule comparée à un autre élément d'un champ. Choisissez le **Champ de base** (un champ placé dans Lignes ou Colonnes) et l'**Élément de base** (l'un de ses éléments, ou celui qui précède ou suit). Par exemple, avec Mois comme champ de base et Jan comme élément de base, février indique de combien il est supérieur ou inférieur à janvier. L'élément de base lui-même reste vide pour une différence, et vaut 100 % pour **% de**.
- **Total cumulé dans** : chaque cellule additionne les cellules qui la précèdent le long du champ de base.
- **Classement du plus petit au plus grand** et **Classement du plus grand au plus petit** : chaque cellule affiche son rang, 1, 2, 3 et ainsi de suite, parmi les cellules du champ de base dans la même ligne ou la même colonne. Le rang 1 correspond au plus petit nombre, ou au plus grand.
- **% du total parent en ligne** et **% du total parent en colonne** : chaque cellule affiche la part qu'elle représente dans son parent. Le parent d'un élément est l'élément situé un niveau au-dessus, comme l'année qui contient un mois. Un élément sans parent est affiché comme une part du total général.

**Format de nombre** accepte un code de format, tel que `0,00`, `# ##0` ou `0,0 %`. Laissez-le vide pour conserver le format des cellules. Une valeur affichée sous forme de pourcentage reçoit le format `0,00 %`, sauf si vous choisissez un autre format. Les cellules du rapport prennent le format immédiatement. **Annuler** le supprime en même temps que les nombres.

## Paramètres du champ

Sélectionnez un champ dans la zone **Filtres**, **Colonnes** ou **Lignes**, puis choisissez **Paramètres du champ** dans l'onglet **Tableau croisé dynamique** (ou double-cliquez dessus).

- **Nom personnalisé** modifie le nom que le rapport affiche pour le champ.
- **Sous-totaux** : **Automatique** affiche un total pour chaque groupe du champ, **Aucun** n'en affiche pas.
- **Afficher les étiquettes d'éléments en** : forme compacte, forme plan ou forme tabulaire, comme dans Disposition du rapport.
- **Insérer une ligne vide après chaque élément** ajoute une ligne vide après chaque groupe.
- **Répéter les étiquettes d'éléments** affiche le nom d'un élément parent sur chaque ligne de son groupe (formes plan et tabulaire).
- **Trier** : conserver l'ordre actuel des éléments (manuel), ou trier de A à Z ou de Z à A selon les étiquettes. **Du plus petit au plus grand selon une valeur** et **Du plus grand au plus petit selon une valeur** trient les éléments d'après les nombres d'un champ de valeurs. Le rapport conserve le tri lorsque les données changent.
- **Filtrer** exclut des éléments d'après leurs étiquettes (commence par, se termine par, contient, est égal à) ou d'après une valeur (supérieur à, inférieur à, compris entre, etc.), ou ne conserve que les quelques premiers ou derniers éléments selon une valeur. Le filtre est calculé lorsque vous choisissez OK. Il masque les éléments qu'il exclut ; les cases à cocher ci-dessous indiquent lesquels, et vous pouvez cocher un élément pour le faire réapparaître.
- **Afficher ces éléments** est la liste des éléments du champ, avec une case à cocher pour chacun. Décochez une case pour masquer l'élément, et les totaux ne le prennent plus en compte.

## Regrouper des dates et des nombres

Sélectionnez un champ de dates ou de nombres dans **Lignes** ou **Colonnes**, puis choisissez **Regrouper le champ** dans l'onglet **Tableau croisé dynamique**. Les groupes sont créés à partir des données : la colonne ne doit donc contenir que des nombres ou des dates, sans texte ni cellule vide.

- **Dates** : cochez le critère de regroupement : **Secondes**, **Minutes**, **Heures**, **Jours**, **Mois**, **Trimestres** ou **Années**. La part la plus fine que vous cochez reste dans le champ. Pour chaque part plus grossière, JeffCrunch ajoute un champ qui porte son nom (par exemple **Années**), et le place avant le champ de dates, au même endroit. **Commencer à** et **Terminer à** sont les premières et dernières dates ; laissez-les tels quels pour utiliser les dates la plus ancienne et la plus récente des données. Les dates antérieures au début ou postérieures à la fin vont dans deux groupes supplémentaires, dont les noms commencent par `<` et `>`.
- **Nombres** : **Commencer à**, **Terminer à** et **Par** créent des groupes tels que 1-10, 11-20, et ainsi de suite.

**Dissocier** supprime les groupes ainsi que les champs ajoutés pour eux. L'actualisation recrée les groupes à partir des nouvelles données. Lorsque deux tableaux croisés dynamiques sont établis à partir des mêmes données, il est impossible de créer des groupes, car un groupe fait partie des données qu'ils partagent.

## Champs calculés

Un champ calculé est un champ que vous créez à partir d'une formule. Il utilise d'autres champs de la liste. Par exemple, une prime égale à un dixième du montant s'écrit `=Montant*0,1`.

Choisissez **Champ calculé** dans l'onglet **Tableau croisé dynamique**. Saisissez un nom et une formule. Écrivez le nom d'un champ dans la formule, ou double-cliquez dessus dans la liste **Champs** pour l'y placer. Mettez entre apostrophes un nom comportant des espaces, comme dans `='Prix unitaire'*Quantité`. Vous pouvez utiliser des nombres, `+ - * / ^ %`, des comparaisons et quelques fonctions telles que `SI`, `ARRONDI`, `SOMME`, `MIN`, `MAX`, `MOYENNE` et `ABS`.

Choisissez **Ajouter**. Le nouveau champ est placé dans la zone **Valeurs**. Il apparaît aussi dans la liste des champs, où vous pouvez le cocher ou le décocher comme n'importe quel autre champ. Il ne peut pas être placé dans les lignes, les colonnes ou les filtres.

Un champ calculé est calculé dans chaque cellule du rapport. Le nom d'un champ représente le total de ce champ dans la cellule : `=Montant*Montant` élève donc au carré le total de la cellule. Il n'élève pas chaque ligne au carré avant de les additionner. Un total général utilise les totaux de la liste entière.

**Formules de liste** (à côté de **Champ calculé**) crée une nouvelle feuille qui indique chaque champ calculé et sa formule, ainsi que chaque élément calculé avec sa formule, son champ et son ordre de calcul. **Annuler** supprime les cellules, et un autre **Annuler** supprime la feuille.

Pour modifier un champ, choisissez son nom dans la zone **Nom**. Sa formule s'affiche. Modifiez-la, puis choisissez **Modifier**. **Supprimer** retire le champ et sa valeur. Chacune de ces actions constitue une étape d'**Annuler**. Un champ calculé est enregistré avec le fichier et reste en place lorsque le rapport est actualisé. Deux tableaux croisés dynamiques établis à partir des mêmes données ne peuvent pas en avoir un, car un champ calculé fait partie des données qu'ils partagent.

## Éléments calculés

Un élément calculé est un élément que vous créez à partir d'autres éléments du même champ. Par exemple, le champ Région a les éléments Est et Ouest. Un nouvel élément nommé Les deux peut valoir `=Est+Ouest`.

Cliquez dans une cellule du tableau croisé dynamique, puis choisissez **Élément calculé** dans l'onglet **Tableau croisé dynamique**. Choisissez le **Champ**. Il doit se trouver dans les lignes ou les colonnes. Saisissez un **Nom** et une **Formule**. Écrivez le nom d'un élément dans la formule, ou double-cliquez dessus dans la liste **Éléments** pour l'y placer. Mettez entre apostrophes un nom comportant des espaces, comme dans `='Grand Est'+Ouest`. Vous pouvez utiliser des nombres, `+ - * / ^ %` et des parenthèses.

Le nouvel élément reçoit une ligne dans le rapport. Dans chaque colonne, il affiche la formule calculée à partir des lignes des éléments qu'elle désigne. Le nouvel élément entre dans les totaux. Un total de sommes, tel qu'un total général ou un sous-total, ajoute la ligne du nouvel élément aux lignes des autres éléments. Ainsi, avec Est à 35, Ouest à 14 et Les deux (Est plus Ouest) à 49, le total général vaut 98. Si un élément désigné par la formule est masqué, la cellule affiche **#RÉF!**. Si la formule désigne un élément qui n'existe pas, la cellule affiche **#NOM?**.

Pour modifier un élément, choisissez son nom dans la zone **Nom**. Sa formule s'affiche. Modifiez-la, puis choisissez **Modifier**. **Supprimer** retire l'élément et sa ligne. Chacune de ces actions constitue une étape d'**Annuler**. Un élément calculé est enregistré avec le fichier et reste en place lorsque le rapport est actualisé. Deux tableaux croisés dynamiques établis à partir des mêmes données ne peuvent pas en avoir un, car un élément calculé fait partie des données qu'ils partagent.

### Ordre de calcul

Il arrive qu'une cellule comporte deux éléments calculés : l'un dans sa ligne et l'autre dans sa colonne. L'ordre dans lequel ils sont calculés peut modifier le résultat. Supposons que l'élément de ligne soit `=Est-Ouest` et l'élément de colonne `=Jan+1`. Calculée dans un ordre, la cellule vaut (Est - Ouest) + 1. Dans l'autre ordre, elle vaut (Est + 1) - (Ouest + 1), ce qui donne un autre nombre.

**Ordre de calcul**, dans l'onglet **Tableau croisé dynamique**, liste les éléments calculés dans l'ordre où ils sont calculés. L'élément situé plus bas dans la liste est calculé en dernier. Choisissez un élément, puis cliquez sur **Monter** ou **Descendre**. L'ordre est enregistré avec le fichier sous la forme de l'ordre des éléments, et les autres programmes le lisent de la même façon. **Formules de liste** indique la place de chaque élément dans cet ordre.

## Afficher les lignes derrière une valeur

Double-cliquez sur une valeur d'un tableau croisé dynamique, ou choisissez **Afficher les détails** dans l'onglet **Tableau croisé dynamique**. JeffCrunch crée une nouvelle feuille, nommée Détail 1, 2, et ainsi de suite, avec les lignes de la liste qui composent le nombre. Les noms des champs figurent dans la première ligne. F2 modifie toujours la cellule, si elle peut l'être. **Annuler** supprime les lignes, et un autre **Annuler** supprime la feuille.

## Développer et réduire

Un champ dont les éléments ont des éléments qui leur sont subordonnés peut afficher ou masquer les lignes situées sous ses éléments. **Développer tout le champ** et **Réduire tout le champ** le font pour chaque élément du champ que vous sélectionnez dans le volet.

## Options du tableau croisé dynamique

**Options**, dans l'onglet **Tableau croisé dynamique**, ouvre une boîte avec les choix du tableau.

- **Nom** est le nom du tableau croisé dynamique. Aucun autre tableau croisé dynamique du classeur ne peut porter ce nom.
- **Pour les valeurs d'erreur, afficher** et **Pour les cellules vides, afficher** placent un texte de votre choix dans ces cellules.
- **Fusionner et centrer les cellules avec étiquettes**, **Ajuster la largeur des colonnes lors de la mise à jour du tableau** et **Conserver la mise en forme des cellules lors de la mise à jour du tableau** sont enregistrés avec le tableau.
- **Afficher les totaux généraux pour les lignes** et **pour les colonnes** correspondent aux choix Totaux généraux.
- **Actualiser les données à l'ouverture du fichier** relit les cellules à chaque ouverture du classeur. Il s'agit d'une étape d'Annuler, comme toute actualisation.

## Style : en-têtes, bandes et galerie

Les **Options de style**, dans l'onglet **Tableau croisé dynamique**, activent ou désactivent la ligne d'en-tête, la première colonne (en-têtes de ligne) et les bandes de lignes et de colonnes. **Styles** propose une galerie d'aspects, et **Effacer le style** supprime l'aspect. Ces aspects sont dessinés comme ceux des tableaux. **Lignes vides** et **Répéter les étiquettes** définissent la ligne vide et les étiquettes répétées pour tous les champs des lignes à la fois.

## Conception : sous-totaux, totaux généraux et disposition

- **Sous-totaux** affiche ou masque le total de chaque groupe, en haut ou en bas du groupe. Lorsqu'il y a plusieurs champs dans Lignes, le total de chaque champ externe est un sous-total.
- **Totaux généraux** propose quatre choix : désactivés pour les lignes et les colonnes, activés pour les deux, activés pour les lignes seulement (une colonne de totaux à droite, un par ligne), et activés pour les colonnes seulement (une ligne de totaux en bas).
- **Disposition du rapport** détermine la forme des étiquettes. En **forme compacte**, tous les champs des lignes partagent une seule colonne, et un nouveau groupe est affiché un peu plus à droite. En **forme plan**, chaque champ a sa propre colonne, et chaque groupe a une ligne à lui au-dessus de ses éléments. En **forme tabulaire**, chaque champ a sa propre colonne, le nom du groupe figure sur sa première ligne, et le sous-total se place sous le groupe.

## La même liste, avec un filtre

Déplacez **Mois** de **Colonnes** vers **Filtres**. Le rapport perd ses colonnes, et une ligne au-dessus de lui affiche **Mois** et **(Tous)**. Les nombres sont désormais le total de chaque région sur tous les mois.

## Graphique croisé dynamique

Cliquez dans une cellule d'un tableau croisé dynamique, puis choisissez **Insertion**, puis **Graphique croisé dynamique**. JeffCrunch crée un graphique à colonnes des valeurs du tableau. Il exclut les totaux généraux. Le graphique est signalé dans le fichier comme le graphique du tableau croisé dynamique. Il lit les cellules du tableau, de sorte que ses nombres changent lorsque vous actualisez le tableau ou modifiez sa disposition. Si le tableau gagne des lignes ou des colonnes, recréez le graphique, car celui-ci conserve les cellules à partir desquelles il a été créé. Un graphique croisé dynamique provenant d'un fichier est affiché tel qu'il a été enregistré.

## Segments et chronologies

Un segment est un ensemble de boutons qui filtre un tableau croisé dynamique, et une chronologie est un curseur qui fait de même pour les dates.

Cliquez dans une cellule d'un tableau croisé dynamique, puis cliquez sur **Segments** dans l'onglet Tableau croisé dynamique. Le volet Segments s'ouvre et liste les segments et chronologies du classeur. Un segment comporte un bouton par élément de son champ. Les boutons bleus correspondent aux éléments affichés par le tableau.

Cliquez sur un bouton pour n'afficher que cet élément. Cliquez de nouveau pour afficher tous les éléments. Maintenez **Ctrl** enfoncée et cliquez pour ajouter un élément à ceux affichés, ou pour en retirer un. Chaque clic constitue une étape d'Annuler, et Annuler rétablit le tableau. Lorsque vous enregistrez, le fichier conserve le choix du segment.

Un segment qui accompagne le fichier est aussi dessiné par-dessus les cellules, à l'endroit où le fichier le place. Il affiche sa légende et un bouton par élément. Les boutons bleus correspondent aux éléments affichés par le tableau. Cliquez sur un bouton pour n'afficher que cet élément, maintenez **Ctrl** enfoncée et cliquez pour ajouter un élément ou en retirer un, et cliquez sur l'entonnoir en haut à droite pour afficher de nouveau tous les éléments. Chaque clic constitue une étape d'Annuler. Lorsque le tableau est filtré d'une autre manière, ou que vous cliquez sur Annuler, les boutons suivent cette modification. Lorsqu'un segment est trop petit pour afficher tous ses boutons, une barre de défilement apparaît à sa droite : faites tourner la molette de la souris au-dessus du segment, cliquez au-dessus ou en dessous du curseur de la barre, ou faites-le glisser pour accéder au reste. Maintenez **Maj** enfoncée et cliquez sur un deuxième bouton pour choisir toute la série d'éléments, du premier bouton cliqué jusqu'à celui-ci ; maintenez **Maj** et **Ctrl** enfoncées pour ajouter la série aux éléments déjà choisis. Un segment imprimé ne peut pas défiler : il imprime donc les boutons qui tiennent, et sa dernière ligne indique combien d'éléments supplémentaires existent. Un segment qui filtre un tableau autre qu'un tableau croisé dynamique, ou dont le tableau croisé dynamique ne se trouve pas dans le fichier, est dessiné avec sa légende et une phrase indiquant que JeffCrunch ne peut pas encore l'utiliser ; il est conservé dans le fichier tel quel. Un classeur ouvert en lecture seule, et une feuille protégée contre la modification des tableaux croisés dynamiques, ne permettent pas à un clic de filtrer le tableau. Un segment s'imprime avec la feuille, sauf si le fichier indique le contraire. Cliquer sur un bouton d'un segment le filtre, tandis que cliquer sur sa légende ou sur son bord le sélectionne comme un objet, à la manière d'une image. Un segment sélectionné affiche des poignées : faites-le glisser pour le déplacer, faites glisser une poignée pour le redimensionner, ou appuyez sur une touche fléchée pour le déplacer. Maintenez **Ctrl** ou **Maj** enfoncée et cliquez sur la légende pour le sélectionner avec d'autres objets. Faites un clic droit sur un segment pour afficher un menu contenant **Supprimer** et le volet **Segments**. Appuyez sur **Suppr** pour retirer un segment sélectionné : le fichier ne contient alors plus de segment, mais le filtre qu'il appliquait au tableau croisé dynamique reste en place jusqu'à ce que vous le modifiiez. Déplacer, redimensionner et supprimer constituent chacun une étape d'Annuler. Un classeur ouvert en lecture seule, ou une feuille protégée contre la modification des objets, ne permet pas ces opérations.

Pour utiliser un segment au clavier, appuyez sur **F6** pour aller au premier segment de la feuille (**Maj+F6** va au dernier). Utilisez ensuite les touches fléchées, **Début**, **Fin**, **Page précédente** et **Page suivante** pour vous déplacer parmi ses boutons, **Espace** ou **Entrée** pour choisir le bouton, **Ctrl+Espace** pour l'ajouter aux éléments choisis ou l'en retirer, **Maj+Espace** pour choisir la série depuis le dernier bouton choisi, et **Ctrl+Alt+C** pour afficher de nouveau tous les éléments. Appuyez sur **Échap** pour quitter les boutons (les touches fléchées déplacent alors le segment, et **Suppr** le retire ; tant que vous êtes parmi les boutons, **Suppr** ne fait rien), puis de nouveau sur **Échap** pour revenir aux cellules. **F6** passe au segment suivant, et après le dernier, revient aux cellules.

**Connexions du rapport** indique les tableaux croisés dynamiques filtrés par un segment. Une chronologie affiche la période de dates qu'elle a choisie. JeffCrunch ne dessine pas encore de chronologie par-dessus les cellules, et ne la modifie pas non plus. Les deux sont conservées dans le fichier tels qu'ils ont été enregistrés.

## Limites

Un tableau croisé dynamique peut comporter jusqu'à 2 000 champs, une liste de cinq millions de lignes au maximum, et un rapport de quatre millions de cellules au maximum. Un fichier qui dépasse ces limites est affiché tel qu'il a été enregistré et ne peut pas être modifié. La relecture de cent mille lignes de vingt colonnes et le calcul du rapport ont pris environ une demi-seconde sur l'ordinateur où cela a été mesuré ; un ordinateur plus lent met plus de temps.

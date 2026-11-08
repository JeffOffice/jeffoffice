# Tableaux croisés dynamiques

<!-- keywords: liste de formules, champ calculé, élément calculé, ordre de calcul, champ de formule, pages de filtre de rapport, tableau croisé dynamique, explorer en détail, synthétiser, insérer un tableau croisé dynamique, créer, faire, sélectionner, déplacer le tableau croisé dynamique, obtenir.données.tableau.croisé, GETPIVOTDATA, générer GETPIVOTDATA, actualiser, actualiser tout, modifier la source de données, liste des champs, champs, lignes, colonnes, valeurs, filtres, filtre de rapport, total général, sous-total, disposition, compact, plan, tabulaire, effacer, paramètres du champ de valeur, somme, nombre, moyenne, afficher les valeurs sous forme de, pourcentage du total, différence, total cumulé, format de nombre, paramètres du champ, trier, filtrer, premiers, grouper, dissocier, dates, mois, années, développer, réduire, options, style, lignes à bandes, lignes vides, répéter les étiquettes, actualiser à l'ouverture du fichier, graphique croisé dynamique, segment, chronologie, afficher les détails, explorer en détail, lignes derrière une valeur -->

Un tableau croisé dynamique est un petit rapport construit à partir d'une longue liste. La liste comporte une ligne par vente, par exemple ; le rapport additionne les ventes par région et par mois. Vous n'écrivez aucune formule. Vous indiquez quelles colonnes de la liste vont en colonne à gauche, lesquelles en haut, et laquelle est additionnée : le rapport se remplit tout seul.

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

Un tableau croisé dynamique avec **Région** en colonne à gauche, **Mois** en haut et la somme de **Montant** au centre ressemble à ceci.

| Somme de Montant | Mois | | |
|---|---|---|---|
| Région | Jan | Fév | Total général |
| Est | 15 | 20 | 35 |
| Ouest | 7 | 7 | 14 |
| Total général | 22 | 27 | 49 |

Est en janvier vaut 15, car les deux lignes Est de janvier, 10 et 5, sont additionnées. La colonne Total général additionne chaque ligne, la ligne Total général additionne chaque colonne, et la cellule du coin additionne tout.

Les termes utilisés ici :

- **Champ** désigne une colonne de la liste. Région, Mois et Montant sont des champs.
- **Lignes**, **Colonnes** et **Valeurs** sont les trois emplacements où un champ peut être placé. Un champ dans Lignes donne une ligne pour chacune de ses valeurs différentes ; un champ dans Colonnes donne une colonne pour chacune ; un champ dans Valeurs est celui qui est additionné. Un quatrième emplacement, **Filtres**, contient un champ qui écarte les lignes que vous ne choisissez pas.
- Un **cache** (le cache du tableau croisé dynamique) est une copie de votre liste que le tableau conserve dans le fichier, afin de pouvoir afficher les totaux sans lire les cellules. Le rapport est calculé à partir du cache, et non à partir des cellules : il ne change donc pas quand les cellules changent. **Actualiser** recopie à nouveau les cellules dans le cache.

## Créer un tableau croisé dynamique

1. Cliquez dans une cellule de la liste, ou sélectionnez les cellules à utiliser. La liste doit comporter une ligne de noms au-dessus des données.
2. Choisissez l'onglet **Insertion**, puis **Tableau croisé dynamique**.
3. La boîte affiche les cellules contenant les données. Modifiez-les si elles ne sont pas correctes. Vous pouvez aussi saisir le nom d'un tableau.
4. Choisissez où placer le tableau croisé dynamique : **Nouvelle feuille** (par défaut), ou une cellule de cette feuille ou d'une autre. Choisissez un emplacement libre, car le tableau vide occupe environ dix-huit lignes et trois colonnes.
5. Cliquez sur **OK**. Le tableau démarre vide, et le volet **Champs de tableau croisé dynamique** s'ouvre. Cochez les champs souhaités : un champ de nombres est additionné, tout autre champ devient une ligne. Déplacez ensuite les champs vers les zones, comme l'expliquent les sections suivantes.

Une nouvelle feuille constitue une étape distincte de **Annuler**, de même que le tableau. Lorsque vous enregistrez au format .xlsx, le tableau est écrit tel que le programme d'origine l'écrit, et il s'ouvre donc là-bas comme un tableau croisé dynamique. Une copie enregistrée au format .xls ou .ods conserve dans ses cellules les nombres affichés par le tableau, mais pas le tableau lui-même, et l'enregistrement vous en avertit.

## GETPIVOTDATA : lire un nombre dans un tableau croisé dynamique

`LIREDONNEESTABCROISDYNAMIQUE` donne le nombre affiché par un tableau croisé dynamique pour les valeurs, lignes et colonnes que vous indiquez. Pour l'exemple ci-dessus :

`=LIREDONNEESTABCROISDYNAMIQUE("Somme de Montant";$E$3;"Région";"Est";"Mois";"Jan")` donne 15.

- Le premier argument est le nom de la valeur, tel qu'il est affiché dans le tableau (par exemple Somme de Montant). Le nom du champ additionné (Montant) convient aussi.
- Le deuxième argument est une cellule quelconque du tableau croisé dynamique.
- Viennent ensuite des couples formés d'un champ et de l'un de ses éléments. Si vous ne nommez que Région et Est, vous obtenez le total d'Est, soit 35. Si vous ne nommez rien, vous obtenez le total général, soit 49. Les champs que vous nommez doivent être les premiers champs des lignes, ou les premiers champs des colonnes, dans l'ordre.
- Si le tableau n'affiche pas le nombre, la cellule affiche `#REF!`.

Lorsque vous cliquez sur une valeur d'un tableau croisé dynamique pendant la saisie d'une formule, JeffCrunch écrit cette formule à votre place, afin qu'elle lise toujours le bon nombre après la réorganisation du tableau. Pour désactiver cette fonction, choisissez **Options** dans l'onglet **Outils de tableau croisé dynamique**, puis décochez la case concernant GETPIVOTDATA.

## Ce que fait JeffCrunch avec un tableau croisé dynamique

Un tableau croisé dynamique présent dans un classeur est affiché tel qu'il a été enregistré. Si vous ne modifiez rien, le fichier est enregistré avec le tableau exactement comme il était. Lorsque vous l'actualisez ou que vous le modifiez, JeffCrunch recalcule le rapport, inscrit les nouveaux nombres dans les cellules, puis réécrit le tableau croisé dynamique afin que le programme qui l'a créé l'ouvre sans message d'erreur.

JeffCrunch calcule les tableaux croisés dynamiques qui font des sommes, des décomptes, des moyennes, etc., qui regroupent des dates et des nombres, et qui affichent les valeurs sous forme de pourcentage, d'écart ou de total cumulé. Un tableau croisé dynamique qui affiche une valeur sous forme de rang ou de pourcentage d'un élément parent, qui comporte des regroupements d'éléments créés à la main, ou qui utilise un modèle de données est affiché et conservé tel qu'il a été enregistré, et les commandes vous indiquent qu'elles ne peuvent pas le modifier.

Cliquez sur une cellule quelconque d'un tableau croisé dynamique. L'onglet **Outils de tableau croisé dynamique** apparaît.

## Actualiser

**Actualiser** lit les cellules à partir desquelles le tableau est construit, puis recalcule le rapport. Les nouvelles entrées viennent après celles déjà présentes ; les entrées disparues sont retirées. La largeur des colonnes et la disposition choisie sont conservées. **Actualiser tout**, dans l'onglet **Données** et dans l'onglet **Outils de tableau croisé dynamique**, fait de même pour tous les tableaux croisés dynamiques du classeur.

Si le rapport devait déborder sur des cellules non vides, JeffCrunch n'actualise pas le tableau et vous indique quelles cellules gênent. Effacez-les ou déplacez le tableau, puis actualisez à nouveau.

**Annuler** annule une actualisation en une seule étape.

## Modifier la source de données

**Modifier la source de données** demande les cellules où se trouve la liste, y compris sa première ligne de noms. Saisissez-les sous la forme `Feuil1!A1:D100`, ou saisissez le nom d'un tableau ou d'un nom que vous avez créé. JeffCrunch lit ces cellules et recalcule le rapport. Si un champ de la liste porte le même nom dans les nouvelles cellules, sa place dans le rapport est conservée.

## Effacer

**Effacer** propose deux choix. **Effacer les filtres** supprime tous les filtres et affiche à nouveau toutes les valeurs. **Tout effacer** retire tous les champs du rapport et laisse celui-ci vide, prêt à être construit à nouveau.

## Explorer en détail et synthétiser

## Pages de filtre de rapport

Un filtre se place au-dessus d'un tableau croisé dynamique et permet de n'afficher qu'un élément à la fois, comme un mois. **Afficher les pages de filtre du rapport** dans l'onglet **Outils de tableau croisé dynamique** crée une page pour chaque élément à la fois. Chaque page est une nouvelle feuille, nommée d'après l'élément, avec sa propre copie du tableau. Le filtre de chaque copie est réglé sur cet élément. Si le tableau comporte plusieurs filtres, vous choisissez celui à utiliser. Une seule commande **Annuler** supprime toutes les pages.

Lorsqu'un rapport comporte plusieurs champs dans ses lignes, un élément du premier champ peut être développé ou réduit. Cliquez sur la cellule contenant le nom de l'élément, puis choisissez **Synthétiser** pour le réduire : les lignes du champ suivant placé sous lui sont masquées, et seul son total reste visible. **Explorer en détail** le développe à nouveau. Un élément du dernier champ n'a rien en dessous de lui. Chacune de ces actions compte pour une étape d'**Annuler**.

## Sélectionner et déplacer

**Sélectionner** dans l'onglet **Outils de tableau croisé dynamique** sélectionne une partie du rapport : **Tout le tableau croisé dynamique**, **Valeurs** (les nombres) ou **Étiquettes** (les noms en colonne à gauche ; si le rapport n'en a pas, les noms en haut). **Déplacer le tableau croisé dynamique…** demande une cellule de la même feuille et redessine le rapport à partir de cette cellule. Les cellules qu'il quitte sont vidées. Les formules qui pointent vers les cellules du rapport les suivent, comme lors d'un couper-coller. Si le nouvel emplacement contient quelque chose, JeffCrunch ne déplace pas le rapport et vous en avertit. Une seule commande **Annuler** le ramène à sa place.

## Le volet des champs

**Liste des champs** dans l'onglet **Outils de tableau croisé dynamique** ouvre le volet **Champs de tableau croisé dynamique**. Il énumère les champs de la liste, avec une case à côté de chacun, et, en dessous, les quatre emplacements : **Filtres**, **Colonnes**, **Lignes** et **Valeurs**.

- Cochez un champ pour l'ajouter. Un champ qui ne contient que des nombres va dans **Valeurs** et est additionné ; tout autre champ va dans **Lignes**.
- Décochez un champ pour le retirer partout.
- Cliquez sur un champ dans l'un des quatre emplacements, puis choisissez **Déplacer vers** pour le placer dans un autre. Les commandes **Monter** et **Descendre** modifient son ordre dans son emplacement, et **Supprimer le champ** le retire.
- Lorsqu'il y a plusieurs valeurs, une ligne nommée **Valeurs** apparaît dans **Colonnes**. Déplacez-la vers **Lignes** pour placer les valeurs en colonne à gauche.
- **Paramètres du champ de valeur** (un double-clic sur une valeur, ou le bouton) définit le nom que le rapport affiche pour la valeur et son mode de calcul : **Somme**, **Nb**, **Nb (chiffres)**, **Moyenne**, **Max**, **Min**, **Produit**, **Écartype** et **Écartypep** (l'écart type d'un échantillon, ou de toutes les valeurs), ainsi que **Var** et **Varp** (la variance de ces mêmes ensembles). Il définit aussi la manière dont la valeur est affichée et son format de nombre. Voir la section suivante.
- **Paramètres du champ** (un double-clic sur un champ dans Filtres, Colonnes ou Lignes, ou le bouton) règle un seul champ. Voir « Paramètres du champ ».

Chaque modification constitue une étape d'**Annuler**.

## Afficher les valeurs sous forme de, et format de nombre d'une valeur

Dans **Paramètres du champ de valeur**, **Afficher les valeurs sous forme de** modifie ce qu'affiche une cellule du rapport.

- **% du total général** : la cellule divisée par le total de l'ensemble du rapport. Dans l'exemple, Est en janvier représente 15 sur 49, soit 30,6 %.
- **% du total des colonnes** et **% du total des lignes** : la cellule divisée par le total de sa colonne, ou de sa ligne.
- **Index** : la comparaison de la cellule avec ce que donneraient sa ligne et sa colonne si les données étaient réparties de façon égale. Un nombre supérieur à 1 signifie une part supérieure à la répartition égale.
- **Différence avec**, **% de** et **% de la différence avec** : la cellule comparée à un autre élément d'un champ. Choisissez le **Champ de base** (un champ des lignes ou des colonnes) et l'**Élément de base** (l'un de ses éléments, ou celui qui précède ou suit). Par exemple, avec Mois comme champ de base et Jan comme élément de base, février indique de combien il est supérieur ou inférieur à janvier. L'élément de base lui-même reste vide pour une différence, et vaut 100 % pour **% de**.
- **Total cumulé dans** : chaque cellule additionne les cellules qui la précèdent le long du champ de base.
- **Classement du plus petit au plus grand** et **Classement du plus grand au plus petit** : chaque cellule affiche son rang, 1, 2, 3, etc., parmi les cellules du champ de base de la même ligne ou colonne. Le rang 1 correspond au plus petit nombre, ou au plus grand.
- **% du total parent en ligne** et **% du total parent en colonne** : chaque cellule affiche la part qu'elle représente dans son parent. Le parent d'un élément est l'élément situé un niveau au-dessus, comme l'année qui contient un mois. Un élément sans parent est affiché comme une part du total général.

**Format de nombre** accepte un code de format, tel que `0,00`, `# ##0` ou `0,0 %`. Laissez le champ vide pour conserver le format que les cellules possèdent. Une valeur affichée sous forme de pourcentage reçoit `0,00 %`, sauf si vous choisissez un autre format. Les cellules du rapport prennent immédiatement le format. **Annuler** le retire avec les nombres.

## Paramètres du champ

Sélectionnez un champ dans la zone **Filtres**, **Colonnes** ou **Lignes**, puis choisissez **Paramètres du champ** dans l'onglet **Outils de tableau croisé dynamique** (ou double-cliquez dessus).

- **Nom personnalisé** modifie le nom que le rapport affiche pour le champ.
- **Sous-totaux** : **Automatiques** affiche un total pour chaque groupe du champ, **Aucun** n'en affiche pas.
- **Afficher les étiquettes d'éléments dans** : forme compacte, forme plan ou forme tabulaire, comme dans Disposition du rapport.
- **Insérer une ligne vide après chaque élément** place une ligne vide après chaque groupe.
- **Répéter les étiquettes d'éléments** affiche le nom d'un élément externe sur chaque ligne de son groupe (formes plan et tabulaire).
- **Trier** : conserver l'ordre des éléments (manuel), ou trier de A à Z ou de Z à A selon les étiquettes. **Du plus petit au plus grand selon une valeur** et **Du plus grand au plus petit selon une valeur** trient les éléments selon les nombres d'un champ de valeurs. Le rapport conserve le tri lorsque les données changent.
- **Filtrer** écarte des éléments selon leurs étiquettes (commence par, se termine par, contient, est égal à) ou selon une valeur (supérieur à, inférieur à, compris entre, etc.), ou ne conserve que les quelques éléments les plus hauts ou les plus bas selon une valeur. Le filtre est appliqué lorsque vous choisissez OK. Il masque les éléments qu'il écarte ; les cases ci-dessous indiquent lesquels, et vous pouvez cocher un élément pour l'afficher à nouveau.
- **Afficher ces éléments** est une liste des éléments du champ, avec une case à cocher pour chacun. Décochez une case pour masquer l'élément ; les totaux ne le prennent alors plus en compte.

## Grouper des dates et des nombres

Sélectionnez un champ de dates ou de nombres dans **Lignes** ou **Colonnes**, puis choisissez **Grouper le champ** dans l'onglet **Outils de tableau croisé dynamique**. Les groupes sont créés à partir des données ; la colonne ne doit donc contenir que des nombres ou que des dates, sans texte ni cellule vide.

- **Dates** : cochez le critère de regroupement : **Secondes**, **Minutes**, **Heures**, **Jours**, **Mois**, **Trimestres** ou **Années**. La partie la plus fine que vous cochez reste sur le champ. Pour chaque partie plus grossière, JeffCrunch ajoute un champ portant son nom (par exemple **Années**) et le place avant le champ de date, au même endroit. **Début** et **Fin** correspondent à la première et à la dernière date ; laissez-les tels quels pour utiliser la date la plus ancienne et la plus récente des données. Les dates antérieures au début ou postérieures à la fin sont regroupées dans deux groupes supplémentaires, dont les étiquettes commencent par `<` et `>`.
- **Nombres** : **Début**, **Fin** et **Pas** créent des groupes tels que 1-10, 11-20, et ainsi de suite.

**Dissocier** supprime les groupes ainsi que les champs ajoutés pour eux. L'actualisation recrée les groupes à partir des nouvelles données. Lorsque deux tableaux croisés dynamiques sont créés à partir des mêmes données, il est impossible de créer des groupes, car un groupe fait partie des données qu'ils partagent.

## Champs calculés

Un champ calculé est un champ que vous créez à partir d'une formule. Il utilise d'autres champs de la liste. Par exemple, une prime égale à un dixième du montant s'écrit `=Montant*0,1`.

Choisissez **Champ calculé** dans l'onglet **Outils de tableau croisé dynamique**. Saisissez un nom et une formule. Écrivez le nom d'un champ dans la formule, ou double-cliquez dessus dans la liste **Champs** pour le placer. Placez entre apostrophes un nom comportant des espaces, comme dans `='Prix unitaire'*Quantité`. Vous pouvez utiliser des nombres, `+ - * / ^ %`, des comparaisons et quelques fonctions telles que `SI`, `ARRONDI`, `SOMME`, `MIN`, `MAX`, `MOYENNE` et `ABS`.

Choisissez **Ajouter**. Le nouveau champ est placé dans la zone **Valeurs**. Il apparaît aussi dans la liste des champs, où vous pouvez le cocher ou le décocher comme tout autre champ. Il ne peut pas être placé dans les lignes, les colonnes ou les filtres.

Un champ calculé est calculé dans chaque cellule du rapport. Le nom d'un champ représente le total de ce champ dans la cellule : `=Montant*Montant` élève donc au carré le total de la cellule. Il ne calcule pas le carré de chaque ligne puis n'additionne pas ensuite ces carrés. Un total général utilise les totaux de l'ensemble de la liste.

**Liste des formules** (à côté de **Champ calculé**) crée une nouvelle feuille qui indique le nom de chaque champ calculé et sa formule, ainsi que chaque élément calculé avec sa formule, son champ et son ordre de calcul. **Annuler** supprime les cellules, et un autre **Annuler** supprime la feuille.

Pour modifier un champ, choisissez son nom dans la zone **Nom**. Sa formule s'affiche. Modifiez-la, puis choisissez **Modifier**. **Supprimer** retire le champ et sa valeur. Chacune de ces actions constitue une étape d'**Annuler**. Un champ calculé est enregistré avec le fichier et conservé lors de l'actualisation du rapport. Deux tableaux croisés dynamiques créés à partir des mêmes données ne peuvent pas en avoir un, car un champ calculé fait partie des données qu'ils partagent.

## Éléments calculés

Un élément calculé est un élément que vous créez à partir d'autres éléments du même champ. Par exemple, le champ Région comporte les éléments Est et Ouest. Un nouvel élément nommé Ensemble peut être `=Est+Ouest`.

Cliquez dans une cellule du tableau croisé dynamique, puis choisissez **Élément calculé** dans l'onglet **Outils de tableau croisé dynamique**. Choisissez le **Champ**. Il doit figurer dans les lignes ou les colonnes. Saisissez un **Nom** et une **Formule**. Écrivez le nom d'un élément dans la formule, ou double-cliquez dessus dans la liste **Éléments** pour le placer. Placez entre apostrophes un nom comportant des espaces, comme dans `='Grand Est'+Ouest`. Vous pouvez utiliser des nombres, `+ - * / ^ %` et des parenthèses.

Le nouvel élément reçoit une ligne dans le rapport. Dans chaque colonne, il affiche la formule calculée à partir des lignes des éléments qu'elle désigne. Le nouvel élément entre dans les totaux. Un total de sommes, comme un total général ou un sous-total, ajoute la ligne du nouvel élément aux lignes des autres éléments. Ainsi, avec Est à 35, Ouest à 14 et Ensemble (Est plus Ouest) à 49, le total général est de 98. Si un élément nommé par la formule est masqué, la cellule affiche **#REF!**. Si la formule nomme un élément qui n'existe pas, la cellule affiche **#NOM?**.

Pour modifier un élément, choisissez son nom dans la zone **Nom**. Sa formule s'affiche. Modifiez-la, puis choisissez **Modifier**. **Supprimer** retire l'élément et sa ligne. Chacune de ces actions constitue une étape d'**Annuler**. Un élément calculé est enregistré avec le fichier et conservé lors de l'actualisation du rapport. Deux tableaux croisés dynamiques créés à partir des mêmes données ne peuvent pas en avoir un, car un élément calculé fait partie des données qu'ils partagent.

### Ordre de calcul

Il arrive qu'une cellule comporte deux éléments calculés : l'un dans sa ligne et l'autre dans sa colonne. L'ordre dans lequel ils sont calculés peut modifier le résultat. Supposons que l'élément de ligne soit `=Est-Ouest` et l'élément de colonne `=Jan+1`. Calculée dans un ordre, la cellule vaut (Est - Ouest) + 1. Dans l'autre ordre, elle vaut (Est + 1) - (Ouest + 1), ce qui donne un nombre différent.

**Ordre de calcul** dans l'onglet **Outils de tableau croisé dynamique** affiche les éléments calculés dans l'ordre où ils sont calculés. L'élément situé plus bas dans la liste est calculé en dernier. Choisissez un élément, puis cliquez sur **Monter** ou **Descendre**. L'ordre est enregistré avec le fichier, comme ordre des éléments, et d'autres programmes le lisent de la même façon. **Liste des formules** indique la place de chaque élément dans cet ordre.

## Afficher les lignes derrière une valeur

Double-cliquez sur une valeur d'un tableau croisé dynamique, ou choisissez **Afficher les détails** dans l'onglet **Outils de tableau croisé dynamique**. JeffCrunch crée une nouvelle feuille, nommée Détail 1, 2, etc., qui contient les lignes de la liste formant ce nombre. Les noms des champs figurent dans la première ligne. F2 modifie toujours la cellule, si elle peut l'être. **Annuler** supprime les lignes, et un autre **Annuler** supprime la feuille.

## Développer et réduire

Un champ dont les éléments ont d'autres éléments en dessous peut afficher ou masquer les lignes situées sous ses éléments. **Développer tout le champ** et **Réduire tout le champ** agissent sur chaque élément du champ sélectionné dans le volet.

## Options du tableau croisé dynamique

**Options** dans l'onglet **Outils de tableau croisé dynamique** ouvre une boîte de dialogue avec les réglages du tableau.

- **Nom** est le nom du tableau croisé dynamique. Aucun autre tableau croisé dynamique du classeur ne peut le porter.
- **Pour les valeurs d'erreur, afficher** et **Pour les cellules vides, afficher** permettent d'y placer un texte de votre choix.
- **Fusionner et centrer les cellules avec étiquettes**, **Ajuster la largeur des colonnes lors de la mise à jour du tableau** et **Conserver la mise en forme des cellules lors de la mise à jour du tableau** sont enregistrés avec le tableau.
- **Afficher les totaux généraux pour les lignes** et **pour les colonnes** correspondent aux choix Totaux généraux.
- **Actualiser les données à l'ouverture du fichier** relit les cellules à chaque ouverture du classeur. Cette action constitue une étape d'Annuler, comme toute actualisation.

## Style : en-têtes, bandes et galerie

Les **Options de style** de l'onglet **Outils de tableau croisé dynamique** activent ou désactivent la ligne d'en-tête, la première colonne (en-têtes de ligne) et les bandes de lignes et de colonnes. **Styles** propose une galerie d'aspects, et **Effacer le style** supprime l'aspect. Ces aspects sont dessinés comme ceux des tableaux. **Lignes vides** et **Répéter les étiquettes** règlent d'un coup la ligne vide et les étiquettes répétées pour tous les champs des lignes.

## Mise en forme : sous-totaux, totaux généraux et disposition

- **Sous-totaux** affiche ou masque le total de chaque groupe, en haut ou en bas du groupe. Lorsque les lignes comportent plusieurs champs, le total de chaque champ externe est un sous-total.
- **Totaux généraux** propose quatre choix : désactivés pour les lignes et les colonnes, activés pour les deux, activés pour les lignes seulement (une colonne de totaux à droite, une par ligne), et activés pour les colonnes seulement (une ligne de totaux en bas).
- **Disposition du rapport** détermine la forme des étiquettes. En forme **compacte**, tous les champs des lignes partagent une seule colonne, et un nouveau groupe est affiché un peu plus à droite. En forme **plan**, chaque champ a sa propre colonne, et chaque groupe occupe une ligne à lui au-dessus de ses éléments. En forme **tabulaire**, chaque champ a sa propre colonne, le nom du groupe figure sur sa première ligne, et le sous-total se place sous le groupe.

## La même liste, avec un filtre

Déplacez **Mois** de **Colonnes** vers **Filtres**. Le rapport perd ses colonnes, et une ligne au-dessus de lui affiche **Mois** et **(Tout)**. Les nombres représentent désormais le total de chaque région sur tous les mois.

## Graphique croisé dynamique

Cliquez dans une cellule d'un tableau croisé dynamique, puis choisissez **Insertion**, puis **Graphique croisé dynamique**. JeffCrunch crée un graphique en colonnes des valeurs du tableau. Il exclut les totaux généraux. Le graphique est marqué dans le fichier comme graphique du tableau croisé dynamique. Il lit les cellules du tableau ; ses nombres changent donc lorsque vous actualisez le tableau ou modifiez sa disposition. Si le tableau gagne des lignes ou des colonnes, recréez le graphique, car celui-ci conserve les cellules à partir desquelles il a été créé. Un graphique croisé dynamique provenant d'un fichier est affiché tel qu'il a été enregistré.

## Segments et chronologies

Un segment est un ensemble de boutons qui filtre un tableau croisé dynamique, et une chronologie est un curseur qui fait de même pour les dates.

Cliquez dans une cellule d'un tableau croisé dynamique, puis cliquez sur **Segments** dans l'onglet Outils de tableau croisé dynamique. Le volet Segments s'ouvre et énumère les segments et les chronologies du classeur. Un segment comporte un bouton par élément de son champ. Les boutons bleus correspondent aux éléments affichés par le tableau.

Cliquez sur un bouton pour n'afficher que cet élément. Cliquez de nouveau dessus pour afficher tous les éléments. Maintenez **Ctrl** enfoncé et cliquez pour ajouter un élément à ceux qui sont affichés, ou pour en retirer un. Chaque clic constitue une étape d'Annuler, et Annuler remet le tableau dans son état précédent. Lors de l'enregistrement, le fichier conserve le choix du segment.

**Connexions de rapport** indique les tableaux croisés dynamiques filtrés par un segment. Une chronologie affiche la période de dates qu'elle a choisie. JeffCrunch affiche les segments dans ce volet, et non par-dessus les cellules, et il ne modifie pas encore une chronologie. Les deux sont conservés dans le fichier tels qu'ils ont été enregistrés.

## Limites

Un tableau croisé dynamique peut comporter jusqu'à 2 000 champs, une liste allant jusqu'à cinq millions de lignes, et un rapport allant jusqu'à quatre millions de cellules. Un fichier qui dépasse ces limites est affiché tel qu'il a été enregistré et ne peut pas être modifié. La relecture de cent mille lignes de vingt colonnes et le calcul du rapport ont pris environ une demi-seconde sur l'ordinateur où cette mesure a été faite ; un ordinateur plus lent met plus de temps.

# Tableaux croisés dynamiques
<!-- keywords: lister les formules, champ calculé, champ de formule, pages de filtre du rapport, tableau croisé dynamique, explorer en détail, synthétiser, explorer en synthèse, insérer un tableau croisé dynamique, créer, faire, sélectionner, déplacer le tableau croisé dynamique, obtenir.données.tableau.croisé, générer GETPIVOTDATA, actualiser, actualiser tout, modifier la source de données, liste des champs, champs, lignes, colonnes, valeurs, filtres, filtre de rapport, total général, sous-total, disposition, compact, plan, tabulaire, effacer, paramètres du champ de valeur, somme, nombre, moyenne, afficher les valeurs sous forme de, pourcentage du total, différence, total cumulé, format de nombre, paramètres du champ, trier, filtrer, premiers, grouper, dissocier, dates, mois, années, développer, réduire, options, style, lignes à bandes, lignes vides, répéter les étiquettes, actualiser à l'ouverture du fichier, graphique croisé dynamique, segment, chronologie, afficher les détails, explorer en détail, lignes derrière une valeur -->

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

- Un **champ** est une colonne de la liste. Région, Mois et Montant sont des champs.
- **Lignes**, **Colonnes** et **Valeurs** sont les trois emplacements où un champ peut être placé. Un champ dans Lignes donne une ligne pour chacune de ses valeurs différentes ; un champ dans Colonnes donne une colonne pour chacune ; un champ dans Valeurs est celui qui est additionné. Un quatrième emplacement, **Filtres**, contient un champ qui écarte les lignes que vous ne choisissez pas.
- Un **cache** (le cache du tableau croisé dynamique) est une copie de votre liste que le tableau conserve dans le fichier, afin de pouvoir afficher les totaux sans lire les cellules. Le rapport est calculé à partir du cache, et non des cellules ; il ne change donc pas lorsque les cellules changent. **Actualiser** recopie à nouveau les cellules dans le cache.

## Créer un tableau croisé dynamique

1. Cliquez dans une cellule de la liste, ou sélectionnez les cellules à utiliser. La liste doit comporter une ligne de noms au-dessus de ses données.
2. Choisissez **Insertion**, puis **Tableau croisé dynamique**.
3. La boîte affiche les cellules contenant les données. Modifiez-les si elles ne sont pas correctes. Vous pouvez aussi saisir le nom d'un tableau.
4. Choisissez où placer le tableau croisé dynamique : **Nouvelle feuille** (par défaut), ou une cellule de cette feuille ou d'une autre. Choisissez un emplacement où rien d'autre ne se trouve, car le tableau vide occupe environ dix-huit lignes et trois colonnes.
5. Choisissez **OK**. Le tableau commence vide, et le volet **Champs de tableau croisé dynamique** s'ouvre. Cochez les champs souhaités : un champ de nombres est additionné, tout autre champ devient une ligne. Ensuite, déplacez les champs vers les zones, comme l'expliquent les sections suivantes.

Une nouvelle feuille constitue une étape distincte d'Annuler, de même que le tableau. Lorsque vous enregistrez au format .xlsx, le tableau est écrit de la manière dont le programme d'origine l'écrit, de sorte qu'il s'ouvre là-bas comme tableau croisé dynamique. Une copie enregistrée au format .xls ou .ods conserve dans ses cellules les nombres affichés par le tableau, mais pas le tableau lui-même, et l'enregistrement vous en informe.

## GETPIVOTDATA : lire un nombre dans un tableau croisé dynamique

`LIREDONNEESTABCROISDYNAMIQUE` donne le nombre affiché par un tableau croisé dynamique pour les valeurs, les lignes et les colonnes que vous indiquez. Pour l'exemple ci-dessus :

`=LIREDONNEESTABCROISDYNAMIQUE("Somme de Montant";$E$3;"Région";"Est";"Mois";"Jan")` donne 15.

- Le premier argument est le nom de la valeur, tel qu'affiché par le tableau (par exemple Somme de Montant). Le nom du champ additionné (Montant) convient également.
- Le deuxième argument est une cellule quelconque du tableau croisé dynamique.
- Viennent ensuite des couples formés d'un champ et d'un de ses éléments. Si vous ne nommez que Région et Est, vous obtenez le total de Est, soit 35. Si vous ne nommez rien, vous obtenez le total général, soit 49. Les champs que vous nommez doivent être les premiers champs des lignes, ou les premiers champs des colonnes, dans l'ordre.
- Si le tableau n'affiche pas le nombre, la cellule affiche `#RÉF!`.

Lorsque vous cliquez sur une valeur d'un tableau croisé dynamique pendant la saisie d'une formule, JeffCrunch écrit cette formule à votre place, afin qu'elle lise toujours le bon nombre après la réorganisation du tableau. Pour désactiver cette fonction, choisissez **Options** dans l'onglet **Tableau croisé dynamique**, puis décochez la case concernant GETPIVOTDATA.

## Ce que JeffCrunch fait d'un tableau croisé dynamique

Un tableau croisé dynamique d'un classeur est affiché tel qu'il a été enregistré. Si vous n'y changez rien, le fichier est enregistré avec le tableau exactement tel qu'il était. Lorsque vous l'actualisez ou le modifiez, JeffCrunch recalcule le rapport, place les nouveaux nombres dans les cellules et réécrit le tableau croisé dynamique afin que le programme qui l'a créé l'ouvre sans message d'erreur.

JeffCrunch calcule les tableaux croisés dynamiques qui additionnent, comptent, font la moyenne, etc., qui regroupent des dates et des nombres, et qui affichent des valeurs sous forme de pourcentage, de différence ou de total cumulé. Un tableau croisé dynamique qui comporte un champ ou un élément calculé, qui affiche une valeur sous forme de rang ou de part d'un élément parent, qui contient des regroupements d'éléments établis à la main ou qui utilise un modèle de données est affiché et conservé tel qu'il a été enregistré, et les commandes vous indiquent qu'elles ne peuvent pas le modifier.

Cliquez sur une cellule quelconque d'un tableau croisé dynamique. L'onglet **Tableau croisé dynamique** apparaît.

## Actualiser

**Actualiser** lit les cellules à partir desquelles le tableau croisé dynamique est construit et recalcule le rapport. Les nouvelles entrées viennent après celles déjà présentes ; les entrées qui ont disparu sont retirées. La largeur des colonnes et la disposition que vous avez choisie sont conservées. **Actualiser tout**, dans l'onglet **Données** et dans l'onglet **Tableau croisé dynamique**, fait de même pour tous les tableaux croisés dynamiques du classeur.

Si le rapport devait déborder sur des cellules contenant quelque chose, JeffCrunch n'actualise pas. Il vous indique les cellules qui gênent. Effacez-les ou déplacez le tableau croisé dynamique, puis actualisez à nouveau.

**Annuler** annule une actualisation en une seule étape.

## Modifier la source de données

**Modifier la source de données** demande les cellules où se trouve la liste, y compris sa première ligne de noms. Saisissez-les sous la forme `Feuil1!A1:D100`, ou saisissez le nom d'un tableau ou d'un nom que vous avez créé. JeffCrunch lit ces cellules et recalcule le rapport. Si un champ de la liste porte le même nom dans les nouvelles cellules, sa place dans le rapport est conservée.

## Effacer

**Effacer** propose deux choix. **Effacer les filtres** supprime tous les filtres et affiche à nouveau toutes les entrées. **Effacer tout** retire tous les champs du rapport et le laisse vide, prêt à être reconstruit.

## Explorer en détail et synthétiser

## Pages de filtre du rapport

Un filtre se place au-dessus d'un tableau croisé dynamique et permet de n'afficher qu'un élément à la fois, comme un mois. **Afficher les pages de filtre du rapport**, dans l'onglet **Tableau croisé dynamique**, crée une page pour chaque élément à la fois. Chaque page est une nouvelle feuille, nommée d'après l'élément, avec sa propre copie du tableau. Le filtre de chaque copie est réglé sur cet élément. Si le tableau comporte plusieurs filtres, vous choisissez celui à utiliser. Un seul **Annuler** supprime toutes les pages.

Lorsqu'un rapport comporte plusieurs champs dans ses lignes, un élément du premier champ peut être développé ou réduit. Cliquez sur la cellule portant le nom de l'élément et choisissez **Synthétiser** pour le réduire : les lignes du champ suivant placé sous lui sont masquées, et seul son total reste visible. **Explorer en détail** le développe de nouveau. Un élément du dernier champ n'a rien en dessous de lui. Chacune de ces actions correspond à un **Annuler**.

## Sélectionner et déplacer

**Sélectionner**, dans l'onglet **Tableau croisé dynamique**, sélectionne une partie du rapport : **Tout le tableau croisé dynamique**, **Valeurs** (les nombres) ou **Étiquettes** (les noms en colonne à gauche ; s'il n'y en a pas, les noms en haut). **Déplacer le tableau croisé dynamique…** demande une cellule de la même feuille et redessine le rapport à partir de cette cellule. Les cellules que le rapport quitte sont vidées. Les formules qui pointent vers les cellules du rapport les suivent, comme lors d'un couper-coller. Si le nouvel emplacement contient quelque chose, JeffCrunch ne déplace pas le rapport et vous en informe. Un seul **Annuler** le ramène à sa place.

## Le volet des champs

**Liste des champs**, dans l'onglet **Tableau croisé dynamique**, ouvre le volet **Champs de tableau croisé dynamique**. Il affiche les champs de la liste, avec une case à côté de chacun, et, en dessous, les quatre emplacements : **Filtres**, **Colonnes**, **Lignes** et **Valeurs**.

- Cochez un champ pour l'ajouter. Un champ qui ne contient que des nombres va dans **Valeurs** et est additionné ; tout autre champ va dans **Lignes**.
- Décochez un champ pour le retirer partout.
- Cliquez sur un champ dans l'un des quatre emplacements, puis choisissez **Déplacer vers** pour le placer ailleurs. Les mots **Monter** et **Descendre** modifient son ordre à l'intérieur de son emplacement, et **Supprimer le champ** le retire.
- Lorsqu'il y a plusieurs valeurs, une ligne nommée **Valeurs** apparaît dans **Colonnes**. Déplacez-la vers **Lignes** pour placer les valeurs en colonne à gauche.
- **Paramètres du champ de valeur** (un double-clic sur une valeur, ou le bouton) définit le nom que le rapport affiche pour la valeur et la manière dont elle est totalisée : **Somme**, **Nombre**, **Nombre de valeurs numériques**, **Moyenne**, **Max**, **Min**, **Produit**, **Écart type** et **Écart type (population)** (l'écart type d'un échantillon, ou de toutes les valeurs), ainsi que **Var** et **Var (population)** (la variance des mêmes ensembles). Il définit également la manière dont la valeur est affichée et son format de nombre. Voir la section suivante.
- **Paramètres du champ** (un double-clic sur un champ dans Filtres, Colonnes ou Lignes, ou le bouton) définit un seul champ. Voir « Paramètres du champ ».

Chaque modification constitue une étape d'**Annuler**.

## Afficher les valeurs sous forme de, et format de nombre d'une valeur

Dans **Paramètres du champ de valeur**, **Afficher les valeurs sous forme de** modifie ce qu'affiche une cellule du rapport.

- **% du total général** : la cellule divisée par le total de tout le rapport. Dans l'exemple, Est en janvier vaut 15 sur 49, soit 30,6 %.
- **% du total de la colonne** et **% du total de la ligne** : la cellule divisée par le total de sa colonne, ou de sa ligne.
- **Index** : la comparaison de la cellule avec ce que donneraient sa ligne et sa colonne si les données étaient réparties uniformément. Un nombre supérieur à 1 signifie une part plus grande que la part égale.
- **Différence avec**, **% de** et **% de la différence avec** : la cellule comparée à un autre élément d'un champ. Choisissez le **Champ de base** (un champ de Lignes ou de Colonnes) et l'**Élément de base** (l'un de ses éléments, ou celui qui précède ou suit). Par exemple, avec Mois comme champ de base et Jan comme élément de base, février montre de combien il est supérieur ou inférieur à janvier. L'élément de base lui-même reste vide pour une différence, et vaut 100 % pour **% de**.
- **Total cumulé dans** : chaque cellule additionne les cellules qui la précèdent le long du champ de base.
- **Classement du plus petit au plus grand** et **Classement du plus grand au plus petit** : chaque cellule affiche son rang, 1, 2, 3, etc., parmi les cellules du champ de base de la même ligne ou colonne. Le rang 1 correspond au plus petit nombre, ou au plus grand.
- **% du total de la ligne parente** et **% du total de la colonne parente** : chaque cellule affiche la part qu'elle représente de son parent. Le parent d'un élément est l'élément situé un niveau au-dessus, comme l'année qui contient un mois. Un élément sans parent est affiché comme une part du total général.

**Format de nombre** accepte un code de format, tel que `0,00`, `# ##0` ou `0,0 %`. Laissez-le vide pour conserver le format des cellules. Une valeur affichée sous forme de pourcentage reçoit `0,00 %`, sauf si vous choisissez un autre format. Les cellules du rapport prennent le format immédiatement. **Annuler** le rétablit avec les nombres.

## Paramètres du champ

Sélectionnez un champ dans la zone **Filtres**, **Colonnes** ou **Lignes**, puis choisissez **Paramètres du champ** dans l'onglet **Tableau croisé dynamique** (ou double-cliquez dessus).

- **Nom personnalisé** modifie le nom que le rapport affiche pour le champ.
- **Sous-totaux** : **Automatique** affiche un total pour chaque groupe du champ, **Aucun** n'en affiche pas.
- **Afficher les étiquettes d'éléments dans** : forme compacte, forme plan ou forme tabulaire, comme dans Disposition du rapport.
- **Insérer une ligne vide après chaque élément** place une ligne vide après chaque groupe.
- **Répéter les étiquettes d'éléments** affiche le nom d'un élément parent sur chaque ligne de son groupe (formes plan et tabulaire).
- **Trier** : conserver l'ordre des éléments (manuel), ou trier de A à Z ou de Z à A selon les étiquettes. **Du plus petit au plus grand selon une valeur** et **Du plus grand au plus petit selon une valeur** trient les éléments selon les nombres d'un champ de valeurs. Le rapport conserve le tri lorsque les données changent.
- **Filtrer** écarte des éléments selon leurs étiquettes (commence par, se termine par, contient, est égal à) ou selon une valeur (supérieur à, inférieur à, compris entre, etc.), ou ne conserve que les premiers ou les derniers éléments selon une valeur. Le filtre est appliqué lorsque vous choisissez OK. Il masque les éléments écartés ; les cases ci-dessous indiquent lesquels, et vous pouvez cocher un élément pour le faire réapparaître.
- **Afficher ces éléments** est une liste des éléments du champ, avec une case à cocher pour chacun. Décochez une case pour masquer l'élément ; les totaux l'excluent alors.

## Grouper les dates et les nombres

Sélectionnez un champ de dates ou de nombres dans **Lignes** ou **Colonnes**, puis choisissez **Grouper le champ** dans l'onglet **Tableau croisé dynamique**. Les groupes sont créés à partir des données ; la colonne ne doit donc contenir que des nombres ou des dates, sans texte ni cellule vide.

- **Dates** : cochez le critère de regroupement : **Secondes**, **Minutes**, **Heures**, **Jours**, **Mois**, **Trimestres** ou **Années**. La partie la plus fine que vous cochez reste sur le champ. Pour chaque partie plus grossière, JeffCrunch ajoute un champ portant son nom (par exemple **Années**) et le place avant le champ de dates, au même endroit. **Date de début** et **Date de fin** sont les premières et dernières dates ; laissez-les telles quelles pour utiliser les dates les plus anciennes et les plus récentes des données. Les dates antérieures au début ou postérieures à la fin vont dans deux groupes supplémentaires, dont les noms commencent par `<` et `>`.
- **Nombres** : **Début**, **Fin** et **Pas** créent des groupes tels que 1 à 10, 11 à 20, et ainsi de suite.

**Dissocier** supprime les groupes ainsi que les champs ajoutés pour eux. **Actualiser** recrée les groupes à partir des nouvelles données. Lorsque deux tableaux croisés dynamiques sont construits à partir des mêmes données, il est impossible de créer des groupes, car un groupe fait partie des données qu'ils partagent.

## Champs calculés

Un champ calculé est un champ créé à partir d'une formule. Il utilise d'autres champs de la liste. Par exemple, une prime égale à un dixième du montant s'écrit `=Montant*0,1`.

Choisissez **Champ calculé** dans l'onglet **Tableau croisé dynamique**. Saisissez un nom et une formule. Écrivez le nom d'un champ dans la formule, ou double-cliquez dessus dans la liste **Champs** pour l'insérer. Placez entre apostrophes un nom comportant des espaces, comme dans `='Prix unitaire'*Quantité`. Vous pouvez utiliser des nombres, `+ - * / ^ %`, des comparaisons, et quelques fonctions telles que `SI`, `ARRONDI`, `SOMME`, `MIN`, `MAX`, `MOYENNE` et `ABS`.

Choisissez **Ajouter**. Le nouveau champ est placé dans la zone **Valeurs**. Il apparaît également dans la liste des champs, où vous pouvez le cocher ou le décocher comme les autres. Il ne peut pas être placé dans les lignes, les colonnes ou les filtres.

Un champ calculé est calculé dans chaque cellule du rapport. Le nom d'un champ représente le total de ce champ dans la cellule ; ainsi, `=Montant*Montant` élève au carré le total de la cellule. Il n'élève pas chaque ligne au carré avant de les additionner. Un total général utilise les totaux de toute la liste.

**Lister les formules** (à côté de **Champ calculé**) crée une nouvelle feuille qui indique le nom de chaque champ calculé et sa formule. **Annuler** supprime les cellules, et un second **Annuler** supprime la feuille.

Pour modifier un champ, choisissez son nom dans la zone **Nom**. Sa formule s'affiche. Modifiez-la et choisissez **Modifier**. **Supprimer** retire le champ et sa valeur. Chacune de ces actions constitue une étape d'**Annuler**. Un champ calculé est enregistré avec le fichier et conservé lors de l'actualisation du rapport. Deux tableaux croisés dynamiques construits à partir des mêmes données ne peuvent pas en comporter un, car un champ calculé fait partie des données qu'ils partagent.

## Afficher les lignes derrière une valeur

Double-cliquez sur une valeur d'un tableau croisé dynamique, ou choisissez **Afficher les détails** dans l'onglet **Tableau croisé dynamique**. JeffCrunch crée une nouvelle feuille, nommée Détail 1, 2, etc., avec les lignes de la liste qui composent le nombre. Les noms des champs figurent dans la première ligne. F2 modifie toujours la cellule, si elle peut être modifiée. **Annuler** supprime les lignes, et un second **Annuler** supprime la feuille.

## Développer et réduire

Un champ dont les éléments ont d'autres éléments en dessous peut afficher ou masquer les lignes situées sous ses éléments. **Développer tout le champ** et **Réduire tout le champ** le font pour chaque élément du champ sélectionné dans le volet.

## Options du tableau croisé dynamique

**Options**, dans l'onglet **Tableau croisé dynamique**, ouvre une boîte contenant les choix du tableau.

- **Nom** est le nom du tableau croisé dynamique. Aucun autre tableau croisé dynamique du classeur ne peut le porter.
- **Pour les valeurs d'erreur, afficher** et **Pour les cellules vides, afficher** placent un texte de votre choix dans ces cellules.
- **Fusionner et centrer les cellules avec étiquettes**, **Ajuster la largeur des colonnes lors de la mise à jour du tableau** et **Conserver la mise en forme des cellules lors de la mise à jour du tableau** sont enregistrés avec le tableau.
- **Afficher les totaux généraux pour les lignes** et **pour les colonnes** sont équivalents aux choix Totaux généraux.
- **Actualiser les données à l'ouverture du fichier** relit les cellules à chaque ouverture du classeur. Il s'agit d'une étape d'Annuler, comme toute actualisation.

## Style : en-têtes, bandes et galerie

Les **Options de style**, dans l'onglet **Tableau croisé dynamique**, activent ou désactivent la ligne d'en-tête, la première colonne (en-têtes de ligne) et les bandes de lignes et de colonnes. **Styles** propose une galerie d'aspects, et **Effacer le style** supprime l'aspect. Ces aspects sont dessinés de la même manière que ceux des tableaux. **Lignes vides** et **Répéter les étiquettes** définissent la ligne vide et les étiquettes répétées pour tous les champs des lignes à la fois.

## Conception : sous-totaux, totaux généraux et disposition

- **Sous-totaux** affiche ou masque le total de chaque groupe, en haut ou en bas du groupe. Lorsque Lignes comporte plusieurs champs, le total de chaque champ externe est un sous-total.
- **Totaux généraux** propose quatre choix : désactivés pour les lignes et les colonnes, activés pour les deux, activés pour les lignes seulement (une colonne de totaux à droite, un pour chaque ligne), et activés pour les colonnes seulement (une ligne de totaux en bas).
- **Disposition du rapport** détermine la forme des étiquettes. En forme **compacte**, tous les champs des lignes partagent une seule colonne, et un nouveau groupe est affiché un peu plus à droite. En forme **plan**, chaque champ a sa propre colonne, et chaque groupe a une ligne à lui au-dessus de ses éléments. En forme **tabulaire**, chaque champ a sa propre colonne et le nom du groupe figure sur sa première ligne, avec le sous-total sous le groupe.

## La même liste, avec un filtre

Déplacez **Mois** de **Colonnes** vers **Filtres**. Le rapport perd ses colonnes, et une ligne au-dessus de lui affiche **Mois** et **(Tous)**. Les nombres sont désormais le total de chaque région sur tous les mois.

## Graphique croisé dynamique

Cliquez dans une cellule d'un tableau croisé dynamique, puis choisissez **Insertion**, puis **Graphique croisé dynamique**. JeffCrunch crée un graphique en colonnes à partir des valeurs du tableau. Il exclut les totaux généraux. Le graphique est marqué dans le fichier comme graphique du tableau croisé dynamique. Il lit les cellules du tableau ; ses nombres changent donc lorsque vous actualisez le tableau ou modifiez sa disposition. Si le tableau gagne des lignes ou des colonnes, créez à nouveau le graphique, car celui-ci conserve les cellules à partir desquelles il a été créé. Un graphique croisé dynamique provenant d'un fichier est affiché tel qu'il a été enregistré.

## Segments et chronologies

Un segment est un ensemble de boutons qui filtre un tableau croisé dynamique ; une chronologie est un curseur qui fait de même pour les dates.

Cliquez dans une cellule d'un tableau croisé dynamique, puis cliquez sur **Segments** dans l'onglet Tableau croisé dynamique. Le volet Segments s'ouvre et affiche les segments et chronologies du classeur. Un segment comporte un bouton pour chaque élément de son champ. Les boutons en bleu correspondent aux éléments affichés par le tableau.

Cliquez sur un bouton pour n'afficher que cet élément. Cliquez à nouveau pour afficher tous les éléments. Maintenez **Ctrl** enfoncé et cliquez pour ajouter un élément à ceux affichés, ou pour en retirer un. Chaque clic constitue une étape d'Annuler, et Annuler rétablit le tableau. Lors de l'enregistrement, le fichier conserve le choix du segment.

**Connexions de rapport** indique les tableaux croisés dynamiques filtrés par un segment. Une chronologie affiche la période de dates qu'elle a choisie. JeffCrunch affiche les segments dans ce volet, et non au-dessus des cellules, et il ne modifie pas encore une chronologie. Les deux sont conservés dans le fichier tels qu'ils ont été enregistrés.

## Limites

Un tableau croisé dynamique peut comporter jusqu'à 2 000 champs, une liste allant jusqu'à cinq millions de lignes et un rapport allant jusqu'à quatre millions de cellules. Un fichier qui dépasse ces limites est affiché tel qu'il a été enregistré et ne peut pas être modifié. La relecture de cent mille lignes de vingt colonnes et le calcul du rapport ont pris environ une demi-seconde sur l'ordinateur où ces mesures ont été faites ; un ordinateur plus lent met plus de temps.

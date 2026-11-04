# Tableaux croisés dynamiques
<!-- keywords: tableau croisé dynamique, tableau croisé, forer, explorer en détail, synthétiser, insérer un tableau croisé dynamique, créer, faire, sélectionner, déplacer le tableau croisé dynamique, GETPIVOTDATA, générer GETPIVOTDATA, actualiser, actualiser tout, modifier la source de données, liste de champs, champs, lignes, colonnes, valeurs, filtres, filtre de rapport, total général, sous-total, disposition, forme compacte, forme plan, forme tabulaire, effacer, paramètres des champs de valeurs, somme, nombre, moyenne, afficher les valeurs comme, pourcentage du total, écart, total cumulé, format de nombre, paramètres de champ, trier, filtrer, premiers, regrouper, dissocier, dates, mois, années, développer, réduire, options, style, lignes à bandes, lignes vides, répéter les étiquettes, actualiser à l'ouverture du fichier, graphique croisé dynamique, segment, chronologie, afficher les détails, lignes derrière une valeur -->

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
- **Lignes**, **Colonnes** et **Valeurs** sont les trois emplacements où un champ peut être placé. Un champ dans Lignes donne une ligne pour chacune de ses valeurs différentes ; un champ dans Colonnes donne une colonne pour chacune ; un champ dans Valeurs est celui qui est additionné. Un quatrième emplacement, **Filtres**, contient un champ qui exclut les lignes que vous ne choisissez pas.
- Un **cache** (le cache du tableau croisé dynamique) est une copie de votre liste que le tableau conserve dans le fichier, afin de pouvoir afficher des totaux sans lire les cellules. Le rapport est calculé à partir du cache, et non à partir des cellules : il ne change donc pas quand les cellules changent. **Actualiser** recopie les cellules dans le cache.

## Créer un tableau croisé dynamique

1. Cliquez dans une cellule de la liste, ou sélectionnez les cellules à utiliser. La liste doit comporter une ligne de noms au-dessus de ses données.
2. Choisissez **Insertion**, puis **Tableau croisé dynamique**.
3. La boîte affiche les cellules contenant les données. Modifiez-les si elles ne sont pas correctes. Vous pouvez aussi saisir le nom d'un tableau.
4. Choisissez l'emplacement du tableau croisé dynamique : **Nouvelle feuille** (par défaut), ou une cellule de cette feuille ou d'une autre. Choisissez un emplacement libre, car le tableau vide occupe environ dix-huit lignes et trois colonnes.
5. Cliquez sur **OK**. Le tableau démarre vide, et le volet **Champs de tableau croisé dynamique** s'ouvre. Cochez les champs souhaités : un champ de nombres est additionné, tout autre champ devient une ligne. Déplacez ensuite les champs vers les zones, comme l'indiquent les sections suivantes.

Une nouvelle feuille constitue une étape distincte d'**Annuler**, de même que le tableau. Lorsque vous enregistrez au format .xlsx, le tableau est écrit comme le fait le programme d'origine, de sorte qu'il s'ouvre là-bas en tant que tableau croisé dynamique. Une copie enregistrée au format .xls ou .ods conserve dans ses cellules les nombres affichés par le tableau, mais pas le tableau lui-même, et l'enregistrement vous en informe.

## GETPIVOTDATA : lire un seul nombre dans un tableau croisé dynamique

`GETPIVOTDATA` donne le nombre affiché par un tableau croisé dynamique pour les valeurs, lignes et colonnes que vous indiquez. Pour l'exemple ci-dessus :

`=GETPIVOTDATA("Somme de Montant"; $E$3; "Région"; "Est"; "Mois"; "Jan")` donne 15.

- Le premier argument est le nom de la valeur, tel qu'affiché dans le tableau (par exemple Somme de Montant). Le nom du champ additionné (Montant) fonctionne aussi.
- Le deuxième argument est une cellule quelconque du tableau croisé dynamique.
- Viennent ensuite des couples formés d'un champ et de l'un de ses éléments. Si vous ne nommez que Région et Est, vous obtenez le total de Est, 35. Si vous ne nommez rien, vous obtenez le total général, 49. Les champs nommés doivent être les premiers champs des lignes, ou les premiers champs des colonnes, dans l'ordre.
- Si le tableau n'affiche pas le nombre demandé, la cellule affiche `#REF!`.

Lorsque vous cliquez sur une valeur d'un tableau croisé dynamique pendant la saisie d'une formule, JeffCrunch écrit cette formule à votre place, afin qu'elle lise toujours le bon nombre après un réaménagement du tableau. Pour désactiver cette fonction, choisissez **Options** dans l'onglet **Tableau croisé dynamique**, puis décochez la case relative à GETPIVOTDATA.

## Ce que JeffCrunch fait d'un tableau croisé dynamique

Un tableau croisé dynamique présent dans un classeur est affiché tel qu'il a été enregistré. Si vous ne modifiez rien, le fichier est enregistré avec le tableau exactement tel qu'il était. Lorsque vous l'actualisez ou le modifiez, JeffCrunch recalcule le rapport, place les nouveaux nombres dans les cellules et réécrit le tableau croisé dynamique afin que le programme qui l'a créé l'ouvre sans message d'erreur.

JeffCrunch calcule les tableaux croisés dynamiques qui additionnent, comptent, font la moyenne, etc., qui regroupent des dates et des nombres, et qui affichent des valeurs sous forme de part, d'écart ou de total cumulé. Un tableau croisé dynamique qui comporte un champ ou un élément calculé, qui affiche une valeur sous forme de rang ou de part d'un élément parent, qui contient des regroupements d'éléments créés à la main ou qui utilise un modèle de données est affiché et conservé tel qu'il a été enregistré, et les commandes vous indiquent qu'elles ne peuvent pas le modifier.

Cliquez sur n'importe quelle cellule d'un tableau croisé dynamique. L'onglet **Tableau croisé dynamique** apparaît.

## Actualiser

**Actualiser** lit les cellules à partir desquelles le tableau est construit et recalcule le rapport. Les nouvelles entrées s'ajoutent après celles qui existent déjà ; les entrées disparues sont retirées. Les largeurs de colonnes et la disposition que vous avez choisie sont conservées. **Actualiser tout**, dans l'onglet **Données** et dans l'onglet **Tableau croisé dynamique**, fait de même pour tous les tableaux croisés dynamiques du classeur.

Si le rapport devait s'étendre sur des cellules contenant déjà des données, JeffCrunch n'actualise pas le tableau et vous indique les cellules qui gênent. Effacez-les ou déplacez le tableau, puis actualisez de nouveau.

**Annuler** annule une actualisation en une seule étape.

## Modifier la source de données

**Modifier la source de données** vous demande les cellules où se trouve la liste, y compris sa première ligne de noms. Saisissez-les sous la forme `Feuil1!A1:D100`, ou saisissez le nom d'un tableau ou d'un nom que vous avez défini. JeffCrunch lit ces cellules et recalcule le rapport. Si un champ de la liste porte le même nom dans les nouvelles cellules, sa place dans le rapport est conservée.

## Effacer

**Effacer** propose deux choix. **Effacer les filtres** supprime tous les filtres et affiche de nouveau toutes les entrées. **Tout effacer** retire tous les champs du rapport et le laisse vide, prêt à être reconstruit.

## Explorer en détail et synthétiser

Lorsqu'un rapport comporte plusieurs champs dans ses lignes, un élément du premier champ peut être développé ou réduit. Cliquez sur la cellule portant le nom de l'élément et choisissez **Synthétiser** pour le réduire : les lignes du champ suivant placées sous lui sont masquées, et seul son total reste visible. **Explorer en détail** le développe de nouveau. Un élément du dernier champ n'a rien en dessous de lui. Chacune de ces actions correspond à une étape d'**Annuler**.

## Sélectionner et déplacer

**Sélectionner** dans l'onglet **Tableau croisé dynamique** sélectionne une partie du rapport : **Tout le tableau croisé dynamique**, **Valeurs** (les nombres), ou **Étiquettes** (les noms placés à gauche ; si le rapport n'en comporte pas à gauche, les noms placés en haut). **Déplacer le tableau croisé dynamique…** vous demande une cellule de la même feuille et redessine le rapport à partir de cette cellule. Les cellules que le rapport quitte sont vidées. Les formules qui pointent vers les cellules du rapport les suivent, comme lors d'un couper-coller. Si le nouvel emplacement contient déjà des données, JeffCrunch ne déplace pas le rapport et vous en informe. Un seul **Annuler** le ramène à sa place.

## Le volet des champs

**Liste de champs** dans l'onglet **Tableau croisé dynamique** ouvre le volet **Champs de tableau croisé dynamique**. Il liste les champs de la liste, avec une case à cocher à côté de chacun, puis, en dessous, les quatre emplacements : **Filtres**, **Colonnes**, **Lignes** et **Valeurs**.

- Cochez un champ pour l'ajouter. Un champ qui ne contient que des nombres va dans **Valeurs** et est additionné ; tout autre champ va dans **Lignes**.
- Décochez un champ pour le retirer de partout.
- Cliquez sur un champ dans l'un des quatre emplacements, puis choisissez **Déplacer vers** pour le placer ailleurs. Les commandes **Monter** et **Descendre** modifient son ordre à l'intérieur de son emplacement, et **Supprimer le champ** le retire.
- Lorsqu'il y a plusieurs valeurs, une ligne nommée **Valeurs** apparaît dans **Colonnes**. Déplacez-la vers **Lignes** pour placer les valeurs à gauche.
- **Paramètres des champs de valeurs** (double-clic sur une valeur, ou bouton) définit le nom affiché dans le rapport pour la valeur et la manière dont elle est totalisée : **Somme**, **Nombre**, **Nombres**, **Moyenne**, **Max**, **Min**, **Produit**, **Écartype** et **Ecartypep** (l'écart type d'un échantillon, ou de toutes les valeurs), ainsi que **Var** et **Varp** (la variance des mêmes ensembles). Il définit aussi la manière dont la valeur est affichée et son format de nombre. Voir la section suivante.
- **Paramètres de champ** (double-clic sur un champ dans Filtres, Colonnes ou Lignes, ou bouton) règle un seul champ. Voir « Paramètres de champ ».

Chaque modification correspond à une étape d'**Annuler**.

## Afficher les valeurs comme, et format de nombre d'une valeur

Dans **Paramètres des champs de valeurs**, **Afficher les valeurs comme** modifie ce qu'affiche une cellule du rapport.

- **% du total général** : la cellule divisée par le total de l'ensemble du rapport. Dans l'exemple, Est en janvier représente 15 sur 49, soit 30,6 %.
- **% du total des colonnes** et **% du total des lignes** : la cellule divisée par le total de sa colonne ou de sa ligne.
- **Index** : la comparaison de la cellule avec ce que donneraient sa ligne et sa colonne si les données étaient réparties uniformément. Un nombre supérieur à 1 indique une part supérieure à la répartition égale.
- **Écart par rapport à**, **% de** et **% d'écart par rapport à** : la cellule comparée à un autre élément d'un champ. Choisissez le **Champ de base** (un champ placé dans Lignes ou Colonnes) et l'**Élément de base** (l'un de ses éléments, ou celui qui précède ou suit). Par exemple, avec Mois comme champ de base et Jan comme élément de base, février indique de combien il est supérieur ou inférieur à janvier. L'élément de base lui-même reste vide pour un écart, et vaut 100 % pour **% de**.
- **Total cumulé dans** : chaque cellule additionne les cellules qui la précèdent le long du champ de base.

**Format de nombre** accepte un code de format, tel que `0,00`, `# ##0` ou `0,0%`. Laissez-le vide pour conserver le format des cellules. Une valeur affichée sous forme de part reçoit `0,00%`, sauf si vous choisissez un autre format. Les cellules du rapport adoptent le format immédiatement. **Annuler** le rétablit avec les nombres.

## Paramètres de champ

Sélectionnez un champ dans la zone **Filtres**, **Colonnes** ou **Lignes**, puis choisissez **Paramètres de champ** dans l'onglet **Tableau croisé dynamique** (ou double-cliquez dessus).

- **Nom personnalisé** modifie le nom affiché dans le rapport pour le champ.
- **Sous-totaux** : **Automatique** affiche un total pour chaque groupe du champ, **Aucun** n'en affiche pas.
- **Afficher les étiquettes d'éléments** : forme compacte, forme plan ou forme tabulaire, comme dans Disposition du rapport.
- **Insérer une ligne vide après chaque élément** ajoute une ligne vide après chaque groupe.
- **Répéter les étiquettes d'éléments** affiche le nom d'un élément externe sur chaque ligne de son groupe (formes plan et tabulaire).
- **Trier** : conserver l'ordre des éléments (manuel), ou trier de A à Z ou de Z à A selon les étiquettes. **Du plus petit au plus grand selon une valeur** et **Du plus grand au plus petit selon une valeur** trient les éléments selon les nombres d'un champ de valeurs. Le rapport conserve le tri lorsque les données changent.
- **Filtrer** exclut des éléments selon leurs étiquettes (commence par, se termine par, contient, égale) ou selon une valeur (supérieure à, inférieure à, comprise entre, etc.), ou ne conserve que les quelques premiers ou derniers éléments selon une valeur. Le filtre est calculé lorsque vous cliquez sur OK. Il masque les éléments exclus ; les cases ci-dessous indiquent lesquels, et vous pouvez cocher un élément pour de nouveau l'afficher.
- **Afficher ces éléments** : liste des éléments du champ, chacun avec une case à cocher. Décochez une case pour masquer l'élément ; les totaux l'excluent.

## Regrouper des dates et des nombres

Sélectionnez un champ de dates ou de nombres dans **Lignes** ou **Colonnes**, puis choisissez **Regrouper le champ** dans l'onglet **Tableau croisé dynamique**. Les groupes sont créés à partir des données : la colonne ne doit contenir que des nombres ou que des dates, sans texte ni cellule vide.

- **Dates** : cochez le critère de regroupement : **Secondes**, **Minutes**, **Heures**, **Jours**, **Mois**, **Trimestres** ou **Années**. La partie la plus fine que vous cochez reste sur le champ. Pour chaque partie plus grossière, JeffCrunch ajoute un champ portant son nom (par exemple **Années**) et le place avant le champ de dates, au même endroit. **Début** et **Fin** sont la première et la dernière dates ; laissez-les tels quels pour utiliser les dates minimale et maximale des données. Les dates antérieures au début ou postérieures à la fin forment deux groupes supplémentaires, commençant par `<` et `>`.
- **Nombres** : **Début**, **Fin** et **Par** créent des groupes tels que 1-10, 11-20, etc.

**Dissocier** supprime les groupes ainsi que les champs ajoutés pour eux. **Actualiser** recrée les groupes à partir des nouvelles données. Lorsque deux tableaux croisés dynamiques sont construits à partir des mêmes données, il est impossible de créer des groupes, car un groupe fait partie des données qu'ils partagent.

## Afficher les lignes derrière une valeur

Double-cliquez sur une valeur d'un tableau croisé dynamique, ou choisissez **Afficher les détails** dans l'onglet **Tableau croisé dynamique**. JeffCrunch crée une nouvelle feuille, nommée Détail 1, 2, etc., avec les lignes de la liste qui composent le nombre. Les noms des champs figurent dans la première ligne. F2 modifie toujours la cellule, si elle peut être modifiée. **Annuler** supprime les lignes, et un autre **Annuler** supprime la feuille.

## Développer et réduire

Un champ dont les éléments ont d'autres éléments sous eux peut afficher ou masquer les lignes placées sous ses éléments. **Développer tout le champ** et **Réduire tout le champ** le font pour tous les éléments du champ sélectionné dans le volet.

## Options du tableau croisé dynamique

**Options** dans l'onglet **Tableau croisé dynamique** ouvre une boîte contenant les choix du tableau.

- **Nom** est le nom du tableau croisé dynamique. Aucun autre tableau croisé dynamique du classeur ne peut le porter.
- **Pour les valeurs d'erreur, afficher** et **Pour les cellules vides, afficher** placent un texte de votre choix dans ces cellules.
- **Fusionner et centrer les cellules avec étiquettes**, **Ajuster la largeur des colonnes lors de la mise à jour du tableau** et **Conserver la mise en forme des cellules lors de la mise à jour du tableau** sont enregistrés avec le tableau.
- **Afficher les totaux généraux pour les lignes** et **pour les colonnes** correspondent aux choix Totaux généraux.
- **Actualiser les données à l'ouverture du fichier** relit les cellules à chaque ouverture du classeur. Il s'agit d'une étape d'Annuler, comme toute actualisation.

## Style : en-têtes, bandes et galerie

Les **Options de style** de l'onglet **Tableau croisé dynamique** activent ou désactivent la ligne d'en-tête, la première colonne (en-têtes de ligne) et les bandes de lignes et de colonnes. **Styles** propose une galerie d'aspects, et **Effacer le style** supprime l'aspect. Ces aspects sont dessinés de la même manière que ceux des tableaux. **Lignes vides** et **Répéter les étiquettes** définissent la ligne vide et les étiquettes répétées pour tous les champs des lignes à la fois.

## Conception : sous-totaux, totaux généraux et disposition

- **Sous-totaux** affiche ou masque le total de chaque groupe, en haut ou en bas du groupe. Lorsque Lignes comporte plusieurs champs, le total de chaque champ externe est un sous-total.
- **Totaux généraux** propose quatre choix : désactivés pour les lignes et les colonnes, activés pour les deux, activés pour les lignes seulement (une colonne de totaux à droite, un par ligne), et activés pour les colonnes seulement (une ligne de totaux en bas).
- **Disposition du rapport** détermine la forme des étiquettes. En forme **compacte**, tous les champs des lignes partagent une seule colonne, et un nouveau groupe est affiché un peu plus à droite. En forme **plan**, chaque champ a sa propre colonne, et chaque groupe occupe une ligne à lui au-dessus de ses éléments. En forme **tabulaire**, chaque champ a sa propre colonne, le nom du groupe figure sur sa première ligne, et le sous-total se trouve sous le groupe.

## La même liste, avec un filtre

Déplacez **Mois** de **Colonnes** vers **Filtres**. Le rapport perd ses colonnes, et une ligne au-dessus affiche **Mois** et **(Tout)**. Les nombres représentent désormais le total de chaque région sur tous les mois.

## Graphique croisé dynamique

Cliquez sur une cellule d'un tableau croisé dynamique, puis choisissez **Insertion**, puis **Graphique croisé dynamique**. JeffCrunch crée un histogramme des valeurs du tableau, sans les totaux généraux. Le graphique est marqué dans le fichier comme graphique du tableau croisé dynamique. Il lit les cellules du tableau : ses nombres changent lorsque vous actualisez le tableau ou modifiez sa disposition. Si le tableau gagne des lignes ou des colonnes, créez de nouveau le graphique, car il conserve les cellules à partir desquelles il a été créé. Un graphique croisé dynamique provenant d'un fichier est affiché tel qu'il a été enregistré.

## Segments et chronologies

Un segment est un ensemble de boutons qui filtre un tableau croisé dynamique, et une chronologie est un curseur qui fait de même pour les dates.

Cliquez sur une cellule d'un tableau croisé dynamique, puis cliquez sur **Segments** dans l'onglet Tableau croisé dynamique. Le volet Segments s'ouvre et liste les segments et chronologies du classeur. Un segment comporte un bouton pour chaque élément de son champ. Les boutons bleus correspondent aux éléments affichés par le tableau.

Cliquez sur un bouton pour n'afficher que cet élément. Cliquez à nouveau dessus pour afficher de nouveau tous les éléments. Maintenez **Ctrl** enfoncée et cliquez pour ajouter un élément à ceux qui sont affichés, ou pour en retirer un. Chaque clic correspond à une étape d'Annuler, et Annuler remet le tableau dans son état précédent. Lors de l'enregistrement, le fichier conserve le choix effectué dans le segment.

**Connexions de rapport** indique les tableaux croisés dynamiques filtrés par un segment. Une chronologie affiche la période de dates qu'elle a choisie. JeffCrunch affiche les segments dans ce volet, et non au-dessus des cellules, et ne modifie pas encore une chronologie. Les deux sont conservés dans le fichier tels qu'ils ont été enregistrés.

## Limites

Un tableau croisé dynamique peut comporter jusqu'à 2 000 champs, une liste allant jusqu'à cinq millions de lignes, et un rapport allant jusqu'à quatre millions de cellules. Un fichier qui dépasse ces limites est affiché tel qu'il a été enregistré et ne peut pas être modifié. La relecture de cent mille lignes de vingt colonnes et le calcul du rapport ont pris environ une demi-seconde sur l'ordinateur où ces mesures ont été faites ; un ordinateur plus lent met plus de temps.

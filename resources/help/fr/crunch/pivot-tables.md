# Tableaux croisés dynamiques
<!-- keywords: tableau croisé dynamique, TCD, insérer un tableau croisé dynamique, créer, LIREDONNEESTABCROISDYNAMIQUE, getpivotdata, générer LIREDONNEESTABCROISDYNAMIQUE, actualiser, actualiser tout, changer la source de données, liste des champs, champs, lignes, colonnes, valeurs, filtres, filtre du rapport, total général, sous-total, disposition, compacte, plan, tabulaire, effacer, paramètres des champs de valeurs, somme, nombre, moyenne, synthèse -->

Un tableau croisé dynamique est un petit rapport établi à partir d'une longue liste. La liste comporte par exemple une ligne par vente ; le rapport additionne les ventes par région et par mois. Vous n'écrivez aucune formule. Vous indiquez quelles colonnes de la liste vont sur le côté, lesquelles vont en haut et laquelle est additionnée, et le rapport se remplit tout seul.

## Un exemple

Voici une liste dans les cellules A1 à C7. Chaque ligne correspond à une vente.

| Région | Mois | Montant |
|---|---|---|
| Est | Janv | 10 |
| Est | Janv | 5 |
| Est | Févr | 20 |
| Ouest | Janv | 7 |
| Ouest | Févr | 3 |
| Ouest | Févr | 4 |

Un tableau croisé dynamique avec **Région** sur le côté, **Mois** en haut et la somme de **Montant** au centre se présente ainsi.

| Somme de Montant | Mois | | |
|---|---|---|---|
| Région | Janv | Févr | Total général |
| Est | 15 | 20 | 35 |
| Ouest | 7 | 7 | 14 |
| Total général | 22 | 27 | 49 |

Est en janvier vaut 15 parce que les deux lignes Est de janvier, 10 et 5, sont additionnées. La colonne Total général additionne chaque ligne, la ligne Total général additionne chaque colonne, et la cellule du coin additionne le tout.

Les termes employés ici :

- Un **champ** est une colonne de la liste. Région, Mois et Montant sont les champs.
- **Lignes**, **Colonnes** et **Valeurs** sont les trois emplacements où placer un champ. Un champ placé dans Lignes donne une ligne pour chacune de ses différentes entrées ; un champ placé dans Colonnes donne une colonne pour chacune ; un champ placé dans Valeurs est celui qui est additionné. Un quatrième emplacement, **Filtres**, contient un champ qui exclut les lignes que vous ne choisissez pas.
- Le **cache** (cache du tableau croisé dynamique) est une copie de votre liste que le tableau croisé dynamique conserve dans le fichier, afin d'afficher les totaux sans consulter les cellules. Le rapport est calculé à partir du cache, et non des cellules : il ne change donc pas lorsque les cellules changent. **Actualiser** recopie les cellules dans le cache.

## Créer un tableau croisé dynamique

1. Cliquez sur une cellule de la liste ou sélectionnez les cellules à utiliser. La liste doit comporter une ligne de noms au-dessus de ses données.
2. Choisissez **Insertion**, puis **Tableau croisé dynamique**.
3. La boîte de dialogue indique les cellules contenant les données. Modifiez-les si elles ne sont pas correctes. Vous pouvez aussi taper le nom d'un tableau.
4. Choisissez l'emplacement du tableau croisé dynamique : **Une nouvelle feuille** (par défaut), ou une cellule de cette feuille ou d'une autre. Choisissez un endroit vide, car le tableau vide occupe environ dix-huit lignes et trois colonnes.
5. Choisissez **OK**. Le tableau est vide au départ et le volet **Champs de tableau croisé dynamique** s'ouvre. Cochez les champs souhaités : un champ de nombres est additionné et tout autre champ devient une ligne. Déplacez ensuite les champs dans les zones comme l'expliquent les sections suivantes.

La nouvelle feuille constitue une étape d'annulation à elle seule, tout comme le tableau. Lorsque vous enregistrez au format .xlsx, le tableau est écrit comme le fait le programme d'origine, et il s'y ouvre donc en tant que tableau croisé dynamique. Une copie enregistrée au format .xls ou .ods conserve les nombres affichés dans les cellules du tableau, mais pas le tableau lui-même, et l'enregistrement vous en avertit.

## LIREDONNEESTABCROISDYNAMIQUE : lire un nombre dans un tableau croisé dynamique

`GETPIVOTDATA` renvoie le nombre qu'affiche un tableau croisé dynamique pour les valeurs, lignes et colonnes indiquées. Pour l'exemple ci-dessus :

`=GETPIVOTDATA("Sum of Amount", $E$3, "Region", "East", "Month", "Jan")` renvoie 15.

- Le premier argument est le nom de la valeur, tel que l'affiche le tableau (par exemple Somme de Montant). Le nom du champ additionné (Montant) fonctionne également.
- Le deuxième argument est une cellule quelconque du tableau croisé dynamique.
- Viennent ensuite des paires constituées d'un champ et de l'un de ses éléments. Si vous indiquez seulement Région, Est, vous obtenez le total de l'Est, 35. Si vous n'indiquez rien, vous obtenez le total général, 49. Les champs indiqués doivent être les premiers champs des lignes, ou les premiers champs des colonnes, dans l'ordre.
- Si le tableau n'affiche pas le nombre, la cellule affiche `#REF!`.

Lorsque vous cliquez sur une valeur d'un tableau croisé dynamique pendant la saisie d'une formule, JeffCrunch écrit cette formule pour vous, afin qu'elle lise toujours le bon nombre après réorganisation du tableau. Pour désactiver ce comportement, choisissez **Options** dans l'onglet **Tableau croisé dynamique** et décochez la case relative à GETPIVOTDATA.

## Traitement des tableaux croisés dynamiques par JeffCrunch

Un tableau croisé dynamique d'un classeur s'affiche tel qu'il a été enregistré. Si vous n'y modifiez rien, le fichier est enregistré avec le tableau croisé dynamique exactement tel qu'il était. Lorsque vous l'actualisez ou le modifiez, JeffCrunch recalcule le rapport, place les nouveaux nombres dans les cellules et réécrit le tableau croisé dynamique de sorte que le programme qui l'a créé l'ouvre sans problème.

JeffCrunch calcule les tableaux croisés dynamiques qui additionnent, comptent, calculent des moyennes, etc. Un tableau croisé dynamique qui groupe des dates ou des nombres, comporte un champ ou un élément calculé, affiche les valeurs sous forme de pourcentage, de différence ou de rang, ou utilise un modèle de données, est affiché et conservé tel qu'il a été enregistré, et les commandes vous indiquent qu'elles ne peuvent pas le modifier.

Cliquez sur une cellule quelconque d'un tableau croisé dynamique. L'onglet **Tableau croisé dynamique** apparaît.

## Actualiser

**Actualiser** lit les cellules à partir desquelles le tableau croisé dynamique est créé et recalcule le rapport. Les nouvelles entrées sont placées après celles déjà présentes ; les entrées disparues sont supprimées. Les largeurs de colonne et la disposition choisie sont conservées. **Actualiser tout**, dans l'onglet **Données** et dans l'onglet **Tableau croisé dynamique**, fait de même pour tous les tableaux croisés dynamiques du classeur.

Si le rapport devait s'étendre sur des cellules non vides, JeffCrunch n'actualise pas. Il indique les cellules qui gênent. Effacez-les ou déplacez le tableau croisé dynamique, puis actualisez de nouveau.

**Annuler** annule une actualisation en une seule étape.

## Changer la source de données

**Changer la source de données** demande les cellules contenant la liste, y compris sa première ligne de noms. Tapez-les sous la forme `Sheet1!A1:D100`, ou tapez le nom d'un tableau ou d'un nom que vous avez défini. JeffCrunch les lit et recalcule le rapport. Si un champ de la liste porte le même nom dans les nouvelles cellules, il garde sa place dans le rapport.

## Effacer

**Effacer** propose deux choix. **Effacer les filtres** supprime tous les filtres et affiche de nouveau toutes les entrées. **Effacer tout** retire tous les champs du rapport et le laisse vide, prêt à être reconstruit.

## Le volet des champs

**Liste des champs**, dans l'onglet **Tableau croisé dynamique**, ouvre le volet **Champs de tableau croisé dynamique**. Il répertorie les champs de la liste, chacun accompagné d'une case, et en dessous les quatre emplacements : **Filtres**, **Colonnes**, **Lignes** et **Valeurs**.

- Cochez un champ pour l'ajouter. Un champ ne contenant que des nombres va dans **Valeurs** et est additionné ; tout autre champ va dans **Lignes**.
- Décochez un champ pour le retirer partout.
- Cliquez sur un champ dans l'un des quatre emplacements et choisissez **Déplacer vers** pour le placer ailleurs. **Monter** et **Descendre** modifient son ordre dans son emplacement, et **Supprimer le champ** le retire.
- Avec plusieurs valeurs, une ligne nommée **Valeurs** apparaît dans **Colonnes**. Déplacez-la dans **Lignes** pour placer les valeurs sur le côté.
- **Paramètres des champs de valeurs** (double-clic sur une valeur, ou le bouton) définit le nom affiché par le rapport pour la valeur et son mode de calcul : **Somme**, **Nombre**, **Chiffres**, **Moyenne**, **Max**, **Min**, **Produit**, **Ecartype** et **Ecartypep** (écart type d'un échantillon ou de toutes les valeurs), et **Var** et **Varp** (variance de ces mêmes ensembles).

Chaque modification constitue une étape d'**Annuler**.

## Création : sous-totaux, totaux généraux et disposition

- **Sous-totaux** affiche ou masque le total de chaque groupe, en haut ou en bas du groupe. Lorsque Lignes contient plusieurs champs, le total de chaque champ extérieur est un sous-total.
- **Totaux généraux** propose quatre choix : désactivé pour les lignes et les colonnes, activé pour les deux, activé pour les lignes uniquement (une colonne de totaux à droite, un par ligne) et activé pour les colonnes uniquement (une ligne de totaux en bas).
- **Disposition du rapport** détermine la forme des étiquettes. Sous forme **compactée**, tous les champs de Lignes partagent une colonne, et chaque nouveau groupe est légèrement décalé vers la droite. En mode **Plan**, chaque champ a sa propre colonne et chaque groupe a sa propre ligne au-dessus de ses éléments. Sous forme **tabulaire**, chaque champ a sa propre colonne et le nom du groupe figure sur sa première ligne, avec le sous-total sous le groupe.

## La même liste, avec un filtre

Déplacez **Mois** de **Colonnes** vers **Filtres**. Le rapport perd ses colonnes, et une ligne au-dessus affiche **Mois** et **(Tout)**. Les nombres sont désormais le total de chaque région sur l'ensemble des mois.

## Limites

Un tableau croisé dynamique peut comporter jusqu'à 2 000 champs, une liste de cinq millions de lignes au maximum et un rapport de quatre millions de cellules au maximum. Un fichier qui dépasse ces limites est affiché tel qu'il a été enregistré et ne peut pas être modifié. Relire cent mille lignes de vingt colonnes et calculer le rapport a pris environ une demi-seconde sur l'ordinateur ayant servi à la mesure ; un ordinateur plus lent prend davantage de temps.

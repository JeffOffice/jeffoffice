# Remplir des cellules et les additionner
<!-- keywords: remplir, série, glisser, poignée, double-clic, liste personnalisée, recopie incrémentée, Ctrl+D, Ctrl+R, bas, droite, haut, gauche, nombre, date, jour ouvré, mois, année, pas, arrêter, copier, Somme, somme, moyenne, nombre, max, min, Alt+égal, total -->

## Remplir

La recopie prolonge ce que vous avez commencé. Sélectionnez les cellules que vous avez remplies, puis faites glisser le petit carré situé dans le coin de la sélection (la **poignée de recopie**) le long d'une colonne ou d'une ligne, et relâchez. Un cadre en pointillés indique jusqu'où la recopie ira.

| Vous commencez par | La recopie donne |
| --- | --- |
| 1, 2 | 3, 4, 5, et ainsi de suite : le pas entre les deux valeurs se poursuit |
| Un seul nombre, comme 5 | 5, 5, 5 : des copies (maintenez Ctrl pendant le glissement pour obtenir l'inverse) |
| Une date | Les jours suivants (utilisez le bouton **Options de recopie**, ou **Remplir > Série**, pour les jours ouvrés, les mois ou les années) |
| Lun, ou janvier | Mar, Mer, ... ou février, mars, ... dans votre langue |
| Élément 1 | Élément 2, Élément 3, ... |
| Un mot d'une liste personnalisée, comme Petit | Le mot suivant de la liste, puis le premier après le dernier |
| Une formule | La formule copiée, les références étant décalées comme lors d'un collage |

Double-cliquez sur la poignée de recopie pour remplir vers le bas jusqu'où vont les données voisines de la sélection. JeffCrunch examine d'abord la colonne de gauche, puis celle de droite. Il s'arrête à la première cellule vide de cette colonne.

## Listes personnalisées

Une **liste personnalisée** est une suite de mots que la recopie parcourt, comme les jours de la semaine. Pour en créer une, cliquez sur **Fichier > Options > Listes personnalisées**. Saisissez une liste par ligne, avec des virgules entre les mots, par exemple `Petit, Moyen, Grand`. Cliquez sur **OK**. Saisissez ensuite **Petit** dans une cellule et faites glisser la poignée de recopie : les cellules se remplissent avec Moyen, Grand, Petit, et ainsi de suite. Les majuscules ne comptent pas lorsque JeffCrunch recherche un mot dans une liste, et les nouvelles cellules reprennent les majuscules de la cellule de départ. Une liste doit comporter au moins deux mots. Les jours de la semaine et les mois sont intégrés.

**Remplir > Bas** (**Ctrl+D**) et **Remplir > Droite** (**Ctrl+R**) copient la première ligne ou colonne de la sélection sur le reste. **Remplir > Haut** et **Remplir > Gauche** travaillent à partir de la dernière. **Remplir > Série** demande le type (linéaire, croissance, date), l'unité d'une date, le pas et la valeur d'arrêt. Chaque cellule remplie prend la mise en forme de la cellule qui la précède. Une cellule qui ne peut pas être modifiée (partie d'une matrice, cellule verrouillée ou en-tête de tableau) interrompt la recopie, et rien n'est modifié.

Après avoir fait glisser la poignée de recopie, le petit bouton **Options de recopie** apparaît dans le coin de la zone remplie. Son menu refait la recopie d'une autre façon, et cette opération reste une seule étape d'Annuler : **Copier les cellules**, **Recopier la série**, **Recopier uniquement la mise en forme** (les cellules conservent leur contenu et prennent la mise en forme), **Recopier les valeurs sans la mise en forme** (les cellules conservent leur propre mise en forme), et, pour les dates, **Incrémenter les jours**, **Incrémenter les jours ouvrés**, **Incrémenter les mois** et **Incrémenter les années**. Le bouton disparaît dès que vous faites autre chose.

Une recopie ne va ni dans les cellules fusionnées ni n'en sort, car elle laisserait des valeurs dans des cellules masquées par la zone fusionnée. Annulez d'abord la fusion.

**Remplir > Justifier** répartit le texte des cellules sélectionnées d'une colonne en lignes de la largeur de la colonne, dans les cellules à partir de la première, vers le bas ; les cellules restantes de la sélection sont vidées. Si le texte demande plus de cellules que vous n'en avez sélectionné, JeffCrunch demande s'il faut utiliser les cellules situées en dessous. Cette commande ne fonctionne que sur du texte.

**Remplir > Recopier dans les feuilles de calcul** copie les cellules sélectionnées dans les mêmes cellules des feuilles cochées dans une liste : tout, seulement le contenu ou seulement les formats. Cette opération est une seule étape d'Annuler.

## Somme

**Accueil > Somme** (**Alt et le signe égal**) place un total dans la cellule sélectionnée. Il regarde les nombres situés au-dessus de la cellule ; s'il n'y en a pas, il regarde ceux situés à sa gauche. La formule est placée dans la cellule pour que vous la vérifiiez, et **Entrée** la conserve. La flèche à côté de Somme donne les autres totaux : **Moyenne**, **Nombre de valeurs numériques**, **Max** et **Min**.

Sélectionnez d'abord une plage : chaque colonne de la plage reçoit sa propre formule dans les cellules vides situées dessous (ou, pour une seule ligne, à sa droite).

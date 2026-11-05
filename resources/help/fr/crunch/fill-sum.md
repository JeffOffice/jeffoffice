# Remplir des cellules et les additionner
<!-- keywords: remplir, série, glisser, poignée, double-clic, liste personnalisée, recopie incrémentée, AutoFill, Ctrl+D, Ctrl+R, bas, droite, haut, gauche, nombre, date, jour ouvré, mois, année, pas, arrêter, copier, somme, additionner, moyenne, compter, max, min, Alt+=, total, remplissage instantané, Ctrl+E, motif, scinder les noms -->

## Remplir

Le remplissage poursuit ce que vous avez commencé. Sélectionnez les cellules que vous avez remplies, puis faites glisser le petit carré situé dans le coin de la sélection (la **poignée de recopie**) le long d'une colonne ou d'une ligne, et relâchez. Un cadre en pointillés indique jusqu'où la recopie ira.

| Vous saisissez | Le remplissage donne |
| --- | --- |
| 1, 2 | 3, 4, 5, et ainsi de suite : le pas entre les nombres se poursuit |
| Un seul nombre, comme 5 | 5, 5, 5 : copies (maintenez Ctrl pendant le glissement pour obtenir l'inverse) |
| Une date | Les jours suivants (utilisez le bouton **Options de recopie**, ou **Remplissage > Incrémenter une série**, pour les jours ouvrés, les mois ou les années) |
| Lun, ou Janvier | Mar, Mer, ... ou Février, Mars, ... dans votre langue |
| Élément 1 | Élément 2, Élément 3, ... |
| Un mot d'une liste personnalisée, comme Petit | Le mot suivant de la liste, puis le premier mot après le dernier |
| Une formule | La formule copiée, avec ses références déplacées comme lors d'un collage |

Double-cliquez sur la poignée de recopie pour remplir vers le bas jusqu'où vont les données voisines de la sélection. JeffCrunch regarde d'abord la colonne de gauche, puis celle de droite. Il s'arrête à la première cellule vide de cette colonne.

## Listes personnalisées

Une **liste personnalisée** est une suite de mots que le remplissage parcourt, comme les jours de la semaine. Pour en créer une, cliquez sur **Fichier > Options > Listes personnalisées**. Saisissez une liste par ligne, avec des virgules entre les mots, par exemple `Petit, Moyen, Grand`. Cliquez sur **OK**. Saisissez ensuite **Petit** dans une cellule et faites glisser la poignée de recopie : les cellules se remplissent avec Moyen, Grand, Petit, et ainsi de suite. Les majuscules n'ont pas d'importance lorsque JeffCrunch cherche un mot dans une liste, et les nouvelles cellules reprennent les majuscules de la cellule de départ. Une liste comporte au moins deux mots. Les jours de la semaine et les mois sont intégrés.

**Remplissage > Vers le bas** (**Ctrl+D**) et **Remplissage > Vers la droite** (**Ctrl+R**) copient la première ligne ou la première colonne de la sélection sur le reste. **Remplissage > Vers le haut** et **Remplissage > Vers la gauche** travaillent à partir de la dernière. **Remplissage > Incrémenter une série** demande le type (linéaire, croissance, date), l'unité d'une date, le pas et la valeur d'arrêt. Chaque cellule remplie prend la mise en forme de la cellule qui la précède. Une cellule qui ne peut pas être modifiée (partie d'une formule matricielle, cellule verrouillée ou en-tête de tableau) arrête le remplissage, et rien ne change.

Après avoir fait glisser la poignée de recopie, le petit bouton **Options de recopie** apparaît dans le coin de la zone remplie. Son menu refait le remplissage d'une autre manière, et le remplissage reste une seule étape d'Annuler : **Copier les cellules**, **Incrémenter une série**, **Recopier uniquement la mise en forme** (les cellules conservent leur contenu et prennent la mise en forme), **Recopier les valeurs sans la mise en forme** (les cellules conservent leur propre mise en forme), et, pour les dates, **Incrémenter les jours**, **Incrémenter les jours ouvrés**, **Incrémenter les mois** et **Incrémenter les années**. Le bouton disparaît lorsque vous effectuez une autre action.

Un remplissage ne va ni dans les cellules fusionnées ni à partir de celles-ci, car il laisserait des valeurs dans des cellules masquées par la zone fusionnée. Annulez d'abord la fusion.

**Remplissage > Justifier** répartit le texte des cellules sélectionnées d'une colonne en lignes aussi larges que la colonne, dans les cellules, à partir de la première vers le bas ; les cellules restantes de la sélection sont vidées. Si le texte nécessite plus de cellules que vous n'en avez sélectionné, JeffCrunch vous demande s'il faut utiliser les cellules situées en dessous. Cette fonction ne s'applique qu'au texte.

**Remplissage > Remplissage instantané** (**Ctrl+E**) remplit une colonne à votre place, à partir d'un motif que vous lui montrez. Saisissez à la main le résultat souhaité dans la ou les deux premières cellules d'une colonne, à côté des cellules dont il provient. Placez ensuite le curseur dans cette colonne et choisissez Remplissage instantané. JeffCrunch détermine comment vos exemples ont été obtenus à partir des cellules voisines, puis remplit les cellules vides de la même manière. Exemple : la colonne A contient des noms comme **Smith, Ann**. Dans B1, vous saisissez **Ann Smith**. Le remplissage instantané complète B2 et les cellules suivantes avec les autres noms, inversés de la même façon. Il peut reprendre un mot d'une cellule, la première lettre d'un mot, ou les premières ou dernières lettres, en majuscules, en minuscules ou tel quel, et il peut ajouter un texte identique dans tous les exemples, comme un espace ou un point. S'il ne trouve pas de motif, il vous le signale. Saisissez un ou deux exemples de plus, puis réessayez. Cette opération est une seule étape d'Annuler.

**Remplissage > Recopier dans les feuilles de calcul** copie les cellules sélectionnées dans les mêmes cellules des feuilles que vous cochez dans une liste : tout, uniquement le contenu, ou uniquement la mise en forme. Cette opération est une seule étape d'Annuler.

## Somme

**Accueil > Somme** (**Alt+=**) place un total dans la cellule sélectionnée. La fonction regarde les nombres situés au-dessus de la cellule ; s'il n'y en a pas, elle regarde ceux situés à sa gauche. La formule est placée dans la cellule pour que vous la vérifiez, et **Entrée** la conserve. La flèche à côté de Somme donne les autres totaux : **Moyenne**, **Nombre de valeurs numériques**, **Max** et **Min**.

Sélectionnez d'abord une plage : chaque colonne de la plage reçoit sa propre formule dans les cellules vides situées en dessous (ou, pour une seule ligne, à sa droite).

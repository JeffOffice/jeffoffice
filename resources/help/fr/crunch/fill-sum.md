# Recopier des cellules et calculer une somme
<!-- keywords: recopie, recopier, remplir, série, incrémenter, faire glisser, poignée de recopie, recopie incrémentée, Ctrl+B, Ctrl+D, bas, droite, haut, gauche, nombre, date, jour ouvré, mois, année, pas, dernière valeur, copier, Somme, somme, moyenne, nombre, max, min, Alt+égal, total -->

## Recopie

La recopie prolonge ce que vous avez commencé. Sélectionnez les cellules que vous avez remplies, puis faites glisser le petit carré situé à l'angle de la sélection (la **poignée de recopie**) le long d'une colonne ou d'une ligne, et relâchez. Une bordure en pointillés indique jusqu'où ira la recopie.

| Vous commencez par | La recopie donne |
| --- | --- |
| 1, 2 | 3, 4, 5, etc. : l'écart entre les valeurs se poursuit |
| Un nombre unique, par exemple 5 | 5, 5, 5 : des copies (maintenez la touche Ctrl enfoncée pendant le glissement pour obtenir l'inverse) |
| Une date | Les jours suivants (utilisez le bouton **Options de recopie**, ou **Recopier > Série**, pour les jours ouvrés, les mois ou les années) |
| Lun, ou janvier | Mar, Mer, ... ou février, mars, ... dans votre langue |
| Article 1 | Article 2, Article 3, ... |
| Une formule | La formule copiée, avec ses références décalées comme lors d'un collage |

**Recopier > En bas** (**Ctrl+B**) et **Recopier > À droite** (**Ctrl+D**) copient la première ligne ou colonne de la sélection sur le reste. **Recopier > En haut** et **Recopier > À gauche** partent de la dernière. **Recopier > Série** demande le type (linéaire, géométrique, chronologique), l'unité de temps, la valeur du pas et la dernière valeur. Chaque cellule recopiée prend le format de la cellule qu'elle prolonge. Une cellule qui ne peut pas être modifiée (partie d'une matrice, cellule verrouillée ou en-tête de tableau) arrête la recopie, et rien n'est modifié.

Après avoir fait glisser la poignée de recopie, le petit bouton **Options de recopie** apparaît à l'angle de la zone recopiée. Son menu refait la recopie d'une autre manière, et la recopie reste une seule étape d'annulation : **Copier les cellules**, **Incrémenter une série**, **Recopier uniquement la mise en forme** (les cellules conservent leur contenu et prennent le format), **Recopier les valeurs sans la mise en forme** (les cellules conservent leur propre format) et, pour les dates, **Incrémenter les jours**, **Incrémenter les jours ouvrés**, **Incrémenter les mois** et **Incrémenter les années**. Le bouton disparaît dès que vous effectuez une autre action.

Une recopie ne peut ni entrer dans des cellules fusionnées ni en sortir, car elle laisserait des valeurs dans des cellules masquées par la zone fusionnée. Annulez d'abord la fusion.

**Recopier > Justifier** répartit le texte des cellules sélectionnées d'une colonne en lignes aussi larges que la colonne, dans les cellules à partir de la première ; les cellules restantes de la sélection sont vidées. Si le texte nécessite plus de cellules que vous n'en avez sélectionné, JeffCrunch demande s'il faut utiliser les cellules situées en dessous. Cette commande ne fonctionne qu'avec du texte.

**Recopier > Dans toutes les feuilles de calcul** copie les cellules sélectionnées dans les mêmes cellules des feuilles que vous cochez dans une liste : tout, le contenu seul ou les formats seuls. Il s'agit d'une seule étape d'annulation.

## Somme

**Accueil > Somme** (**Alt et le signe égal**) insère un total dans la cellule sélectionnée. La commande examine les nombres situés au-dessus de la cellule ; s'il n'y en a pas, ceux situés à sa gauche. La formule est insérée dans la cellule pour que vous la vérifiiez, et **Entrée** la valide. La flèche à côté de Somme propose les autres totaux : **Moyenne**, **Nombre de valeurs numériques**, **Max** et **Min**.

Si vous sélectionnez d'abord une plage, chaque colonne de la plage reçoit sa propre formule dans les cellules vides situées en dessous (ou, pour une seule ligne, à sa droite).

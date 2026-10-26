# Recopier des cellules et calculer une somme
<!-- keywords: recopier, recopie, remplir, série, faire glisser, poignée, poignée de recopie, recopie incrémentée, Ctrl+D, Ctrl+R, bas, droite, haut, gauche, nombre, date, jour ouvré, mois, année, pas, dernière valeur, copier, somme automatique, somme, total, moyenne, nombre, max, min, Alt+égal -->

## Recopier

La recopie prolonge ce que vous avez commencé. Sélectionnez les cellules que vous avez remplies, puis faites glisser le petit carré situé dans l'angle de la sélection (la **poignée de recopie**) le long d'une colonne ou d'une ligne, et relâchez. Une bordure en pointillés indique jusqu'où va la recopie.

| Vous commencez par | La recopie donne |
| --- | --- |
| 1, 2 | 3, 4, 5, etc. : l'écart entre les valeurs se poursuit |
| Un seul nombre, par exemple 5 | 5, 5, 5 : des copies (maintenez Ctrl enfoncée pendant le glissement pour obtenir l'inverse) |
| Une date | Les jours suivants (utilisez **Recopier > Série** pour les jours ouvrés, les mois ou les années) |
| Lun, ou janvier | Mar, Mer, ... ou février, mars, ... dans votre langue |
| Article 1 | Article 2, Article 3, ... |
| Une formule | La formule copiée, avec ses références décalées comme lors d'un collage |

**Recopier > En bas** (**Ctrl+D**) et **Recopier > À droite** (**Ctrl+R**) copient la première ligne ou colonne de la sélection sur le reste. **Recopier > En haut** et **Recopier > À gauche** partent de la dernière. **Recopier > Série** demande le type (linéaire, géométrique, chronologique), l'unité de temps, la valeur du pas et la dernière valeur. Chaque cellule recopiée prend le format de la cellule qui la précède. Une cellule non modifiable (faisant partie d'une matrice, verrouillée, ou en-tête d'un tableau) arrête la recopie, et rien n'est modifié.

## Somme automatique

**Accueil > Somme automatique** (**Alt et le signe égal**) place un total dans la cellule sélectionnée. La commande examine les nombres situés au-dessus de la cellule ; s'il n'y en a pas, ceux situés à sa gauche. La formule est placée dans la cellule pour que vous la vérifiiez, et **Entrée** la valide. La flèche à côté de Somme automatique propose les autres totaux : **Moyenne**, **Nombre de valeurs numériques**, **Max** et **Min**.

Si vous sélectionnez d'abord une plage, chaque colonne de la plage reçoit sa propre formule dans les cellules vides situées en dessous (ou, pour une seule ligne, à sa droite).

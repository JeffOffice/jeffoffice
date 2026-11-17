# Graphiques

<!-- keywords: graphique, graphiques, diagramme, histogramme, graphique en colonnes, graphique à barres, graphique en courbes, graphique en secteurs, anneau, aires, nuage de points, insérer un graphique, données du graphique, modifier les données, titre du graphique, légende, étiquettes de données, type de graphique, format du graphique, axes, titre de l'axe, quadrillage, style de graphique, éléments du graphique, dispositions, couleurs, apparences, données du graphique, permuter les lignes et les colonnes, type de graphique, mettre en forme l'axe, barres d'erreur, courbe de tendance, filtres du graphique, éléments du graphique -->

Un **graphique** transforme des nombres en image, sous forme de colonnes, de courbe ou de secteurs de camembert. Cette rubrique explique comment insérer un graphique dans un document, saisir ses nombres et modifier son apparence.

## Insérer un graphique

1. Placez le point d'insertion à l'endroit où le graphique doit apparaître.
2. Cliquez sur **Insertion > Graphique**, puis choisissez un type : colonnes, colonnes empilées, colonnes empilées à 100 %, barres, barres empilées, courbes, courbes avec marqueurs, secteurs, anneau, aires, aires empilées ou nuage de points.

Un graphique **empilé** superpose les séries les unes sur les autres. Un graphique **empilé à 100 %** donne à chaque pile la même hauteur, ce qui permet de voir les proportions. Un graphique en **nuage de points** place chaque point à l'endroit indiqué par ses deux nombres.

Le graphique apparaît avec des valeurs d'exemple, aligné sur le texte, comme une grande lettre. L'onglet **Format du graphique** s'ouvre.

## Saisir vos propres nombres

Sélectionnez le graphique, puis cliquez sur **Format du graphique > Modifier les données**. Un tableau s'ouvre.

- La première colonne contient les mots placés sous les points (les **catégories**), par exemple les mois.
- La première ligne contient le nom de chaque **série**, c'est-à-dire un ensemble de nombres, par exemple un produit.
- Les autres cellules contiennent les nombres.

Cliquez sur **Ajouter une ligne** ou **Ajouter une colonne** pour en ajouter, ou sur **Supprimer la dernière ligne** ou **Supprimer la dernière colonne** pour en retirer. Cliquez sur **OK**. Si une cellule ne contient pas un nombre, le graphique n'est pas modifié, et la barre d'état indique la ligne concernée. Un graphique peut comporter jusqu'à 200 catégories et 24 séries.

Pour un nuage de points, la première colonne contient les nombres qui placent les points de gauche à droite.

## Graphiques enregistrés par un autre programme

Certains graphiques enregistrés par d'autres programmes conservent leurs nombres dans un petit **classeur** (un fichier de tableur) stocké dans votre fichier. Sélectionnez un tel graphique, puis cliquez sur **Format du graphique > Modifier les données**. Le tableau affiche les cellules de ce classeur que le graphique lit. Modifiez un nombre, un nom placé sous un point ou le nom d'une série, puis cliquez sur **OK**. La modification est enregistrée à la fois dans le classeur et dans le graphique, en une seule étape d'annulation, de sorte que les autres programmes affichent eux aussi les nouveaux nombres.

Vous ne pouvez pas ajouter ni supprimer une ligne ou une colonne de ce type de graphique, et **Données du graphique** ainsi que **Permuter les lignes et les colonnes** sont grisées, car le graphique est lié à ces cellules. Il en va de même lorsque le graphique lit des cellules d'un autre fichier, une plage qui n'est ni une colonne ni une ligne, ou des cellules contenant une formule, une date ou du texte là où il faudrait un nombre. La commande est alors grisée, et son info-bulle indique la raison.

## Modifier l'apparence

Ces commandes se trouvent dans l'onglet **Format du graphique**. Chacune constitue une seule étape d'annulation, et le graphique conserve ses couleurs ainsi que le reste de son apparence.

- **Modifier le type de graphique** transforme le graphique en un autre type, avec les mêmes nombres.
- **Titre du graphique** affiche ou masque le titre. **Modifier le titre du graphique** change son texte.
- **Légende** est la clé qui indique la couleur de chaque série. Placez-la à droite, en haut, à gauche ou en bas, ou supprimez-la.
- **Axes** affiche ou masque la ligne de mots sous le graphique (les **catégories**) et la ligne de nombres à côté (les **valeurs**), et définit un titre pour chacune. Un graphique en secteurs n'a pas d'axes.
- **Quadrillage** trace des lignes discrètes à travers le graphique pour faciliter la lecture des nombres. Les lignes **principales** correspondent aux grands nombres de l'échelle, et les lignes **secondaires** se situent entre elles.
- **Styles de graphique** permet de choisir parmi huit styles d'ensemble.
- **Étiquettes de données** inscrit chaque nombre sur sa colonne, son point ou son secteur.
- **Renvoyer à la ligne automatiquement**, **Texte de remplacement**, ainsi que **Hauteur** et **Largeur** fonctionnent comme pour les images. Voir [Ajouter et modifier des images](pictures).

## L'onglet Graphique

Sélectionnez un graphique : l'onglet **Graphique** s'ouvre à côté de l'onglet **Format du graphique**. Tout ce qui s'y trouve modifie le graphique en une seule étape que vous pouvez annuler.

- **Éléments du graphique** ajoute, retire ou place un élément du graphique : **Axes**, **Titres des axes**, **Titre du graphique**, **Étiquettes de données** (leur contenu, leur emplacement et le signe qui les sépare), **Quadrillage**, **Légende**, **Barres d'erreur** (petites barres qui indiquent de combien un nombre peut être inexact), **Traits** (lignes de chute, lignes haut-bas et barres haut-bas) , **Tableau de données** (une grille des nombres sous le graphique, avec une colonne par catégorie ; sa liste l'affiche ou le masque, et règle les traits entre les lignes, les traits entre les colonnes, le contour et les clés de légende, qui indiquent la couleur de chaque série à côté de son nom ; il est proposé pour les graphiques en colonnes, en courbes et en aires), et **Courbe de tendance** (une droite qui montre vers où tendent les nombres, avec son équation si vous le souhaitez).
- **Dispositions** définit plusieurs éléments à la fois, comme le titre, la légende, les étiquettes et le quadrillage.
- **Couleurs** attribue aux séries un nouveau jeu de couleurs. **Apparences** donne à l'ensemble du graphique un aspect, par exemple sobre, épuré, sombre ou doux.
- **Permuter les lignes et les colonnes** inverse les nombres : les séries deviennent les mots placés sous les points, et ces mots deviennent les séries.
- **Données du graphique** liste les séries. Cochez celles à tracer, modifiez leurs noms et déplacez-les vers le haut ou vers le bas.
- **Modifier les données** ouvre le tableau des nombres.
- **Type de graphique** affiche un aperçu de chaque type. Choisissez un type pour l'ensemble du graphique, ou un type pour chaque série, par exemple des colonnes avec une courbe par-dessus.
- **Mettre en forme l'élément du graphique** définit le remplissage et le trait de la zone du graphique, de la zone de traçage (le cadre dans lequel se trouvent les marques) ou d'une série. Pour une courbe, il permet aussi de définir si elle est lissée et quel repère se trouve à chaque point.
- **Mettre en forme l'axe horizontal** et **Mettre en forme l'axe vertical** définissent les valeurs minimale et maximale, la taille de chaque graduation, l'ordre, l'endroit où l'autre axe coupe, l'apparence des nombres et les graduations.

## Les boutons à côté d'un graphique

Trois petits boutons apparaissent à droite d'un graphique sélectionné.

- **Éléments du graphique** (le plus) ouvre le même menu que **Éléments du graphique** de l'onglet.
- **Styles et couleurs du graphique** (le pinceau) affiche les jeux de couleurs et les apparences.
- **Filtres du graphique** (l'entonnoir) liste les séries. Décochez une série pour la retirer de l'image. Le fichier la conserve tout de même, et elle réapparaît lorsque vous la cochez à nouveau. Un graphique affiche toujours au moins une série.

## Graphiques dans d'autres documents

JeffWrite affiche les graphiques qu'un document contient et les conserve tels quels. Il peut modifier le type, le titre, la légende et les étiquettes d'un graphique qu'il sait écrire entièrement. Si un graphique comporte des éléments que JeffWrite ne modifie pas, comme des effets spéciaux, le programme le signale et laisse le graphique tel quel. Si un graphique lit ses nombres dans une feuille de calcul conservée avec lui, vous pouvez tout modifier sauf les nombres.

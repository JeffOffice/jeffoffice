# Graphiques

<!-- keywords: graphique, graphiques, diagramme, histogramme, graphique en colonnes, graphique à barres, graphique en courbes, graphique circulaire, anneau, aires, nuage de points, insérer un graphique, données du graphique, modifier les données, titre du graphique, légende, étiquettes de données, type de graphique, format du graphique, axes, titre de l'axe, quadrillage, style de graphique, éléments du graphique, dispositions, couleurs, apparences, données du graphique, inverser les lignes et les colonnes, type de graphique, mettre en forme l'axe, barres d'erreur, courbe de tendance, filtres du graphique, éléments du graphique -->

Un **graphique** transforme des nombres en image : des colonnes, une courbe ou des parts de tarte. Cette rubrique explique comment insérer un graphique dans un document, saisir ses nombres et modifier son apparence.

## Insérer un graphique

1. Placez le curseur à l'endroit où le graphique doit apparaître.
2. Cliquez sur **Insertion > Graphique**, puis choisissez un type : colonnes, colonnes empilées, colonnes empilées à 100 %, barres, barres empilées, courbes, courbes avec marqueurs, secteurs, anneau, aires, aires empilées, nuage de points, radar, radar avec marqueurs, radar plein, plus haut-plus bas-fermeture, ouverture-plus haut-plus bas-fermeture, volume-plus haut-plus bas-fermeture ou volume-ouverture-plus haut-plus bas-fermeture.

Un graphique **boursier** montre des cours : pour chaque catégorie, un trait va du cours le plus haut au cours le plus bas, avec un petit point pour la clôture ou, lorsqu'il y a une ouverture, une barre entre l'ouverture et la clôture (claire en cas de hausse, foncée en cas de baisse). Le graphique est construit à partir de vos colonnes dans l'ordre exact où elles se présentent, de gauche à droite, et rien ne vérifie que les nombres correspondent à leur place : placez-les donc dans cet ordre, à savoir haut, bas et clôture ; ouverture, haut, bas et clôture ; volume, haut, bas et clôture ; ou volume, ouverture, haut, bas et clôture. Si les colonnes sont dans un autre ordre, le graphique reste dessiné, mais les repères ont une autre signification (le point de clôture se place sur la troisième colonne de cours, et la boîte va de la première à la dernière colonne de cours). Avec un volume, le volume est représenté par des colonnes lues sur l'axe de gauche, et les cours sont lus sur un second axe à droite. Si le nombre de colonnes ne correspond pas au type choisi, JeffWrite indique combien il en faut et ne crée pas de graphique.

Un graphique **empilé** superpose les séries les unes sur les autres. Un graphique **empilé à 100 %** donne à chaque pile la même hauteur, de sorte que l'on voit des proportions. Un graphique en **nuage de points** place chaque point là où ses deux nombres l'indiquent.

Le graphique apparaît avec des exemples de nombres, aligné sur le texte, comme une grande lettrine. L'onglet **Format du graphique** s'ouvre.

## Saisir vos propres nombres

Sélectionnez le graphique, puis cliquez sur **Format du graphique > Modifier les données**. Un tableau s'ouvre.

- La première colonne contient les mots placés sous les points (les **catégories**), par exemple les mois.
- La première ligne contient le nom de chaque **série**, c'est-à-dire un ensemble de nombres, par exemple un produit.
- Les autres cellules contiennent les nombres.

Cliquez sur **Ajouter une ligne** ou **Ajouter une colonne** pour en ajouter, ou sur **Supprimer la dernière ligne** ou **Supprimer la dernière colonne** pour en retirer. Cliquez sur **OK**. Si une cellule ne contient pas de nombre, le graphique n'est pas modifié et la barre d'état indique la ligne concernée. Un graphique peut comporter jusqu'à 200 catégories et 24 séries.

Pour un graphique en nuage de points, la première colonne contient les nombres qui placent les points de gauche à droite.

## Graphiques enregistrés par un autre logiciel

Certains graphiques enregistrés par d'autres logiciels conservent leurs nombres dans un petit **classeur** (un fichier de tableur) stocké dans votre fichier. Sélectionnez un tel graphique, puis cliquez sur **Format du graphique > Modifier les données**. Le tableau affiche les cellules de ce classeur que le graphique lit. Modifiez un nombre, un nom placé sous un point ou le nom d'une série, puis cliquez sur **OK**. La modification est appliquée au classeur et au graphique en même temps, en une seule étape annulable, de sorte que les autres logiciels affichent eux aussi les nouveaux nombres.

Vous ne pouvez pas ajouter ni supprimer une ligne ou une colonne d'un tel graphique, et **Données du graphique** ainsi que **Inverser les lignes et les colonnes** sont grisés, car le graphique est lié à ces cellules. Il en va de même lorsque le graphique lit des cellules d'un autre fichier, une plage qui n'est ni une colonne ni une ligne, ou des cellules contenant une formule, une date ou du texte là où un nombre est attendu. La commande est alors grisée, et son info-bulle indique la raison.

## Modifier l'apparence

Ces commandes se trouvent dans l'onglet **Format du graphique**. Chacune constitue une seule étape annulable, et le graphique conserve ses couleurs ainsi que le reste de son apparence.

- **Modifier le type de graphique** transforme le graphique en un autre type, avec les mêmes nombres. Pour passer à un type boursier, les séries du graphique doivent être dans l'ordre exigé par ce type ; sinon, JeffWrite l'indique et laisse le graphique tel quel.
- **Titre du graphique** affiche ou masque le titre. **Modifier le titre du graphique** change son texte.
- **Légende** est la clé qui indique la couleur de chaque série. Placez-la à droite, en haut, à gauche ou en bas, ou supprimez-la.
- **Axes** affiche ou masque la ligne de mots sous le graphique (les **catégories**) et la ligne de nombres à côté (les **valeurs**), et définit un titre pour chacune. Un graphique en secteurs n'a pas d'axes.
- **Quadrillage** trace des lignes discrètes à travers le graphique pour faciliter la lecture des nombres. Les lignes **principales** se trouvent aux grandes valeurs sur le côté, et les lignes **secondaires** se placent entre elles.
- **Styles de graphique** permet de choisir l'un des huit styles généraux.
- **Étiquettes de données** inscrit chaque nombre sur sa colonne, son point ou sa part.
- **Habillage du texte**, **Texte de remplacement**, ainsi que **Hauteur** et **Largeur** fonctionnent comme pour les images. Voir [Ajouter et modifier des images](pictures).

## L'onglet Graphique

Sélectionnez un graphique : l'onglet **Graphique** s'ouvre à côté de l'onglet **Format du graphique**. Toutes les commandes de cet onglet modifient le graphique en une seule étape annulable.

- **Éléments du graphique** ajoute, retire ou place une partie du graphique : **Axes**, **Titres des axes**, **Titre du graphique**, **Étiquettes de données** (ce qu'elles contiennent, leur emplacement et le signe qui les sépare), **Quadrillage**, **Légende**, **Barres d'erreur** (petites barres qui indiquent de combien un nombre peut être inexact), **Traits** (lignes de chute, lignes haut-bas et barres haut-bas) , **Tableau de données** (une grille des nombres sous le graphique, avec une colonne par catégorie ; sa liste active ou désactive le tableau et règle les traits entre les lignes, les traits entre les colonnes, le contour et les clés de légende, qui affichent la couleur de chaque série à côté de son nom ; il est proposé pour les graphiques en colonnes, en courbes et en aires) et **Courbe de tendance** (une droite qui indique la direction vers laquelle tendent les nombres, avec son équation si vous le souhaitez).
- **Dispositions** règle plusieurs parties à la fois, comme le titre, la légende, les étiquettes et le quadrillage.
- **Couleurs** donne aux séries un nouveau jeu de couleurs. **Apparences** donne au graphique entier une apparence, par exemple simple, épurée, sombre ou douce.
- **Inverser les lignes et les colonnes** retourne les nombres : les séries deviennent les mots placés sous les points, et ces mots deviennent les séries.
- **Données du graphique** liste les séries. Cochez celles à tracer, modifiez leurs noms et déplacez-les vers le haut ou vers le bas.
- **Modifier les données** ouvre le tableau des nombres.
- **Type de graphique** affiche un aperçu de chaque type. Choisissez un type pour l'ensemble du graphique, ou un type pour chaque série, par exemple des colonnes avec une courbe par-dessus.
- **Mettre en forme l'élément du graphique** définit le remplissage et le trait de la zone du graphique, de la zone de traçage (le cadre dans lequel se placent les repères) ou d'une série. Pour une courbe, il permet aussi de définir si elle est lissée et quel marqueur se place à chaque point.
- **Sélection actuelle** liste les parties du graphique : la zone du graphique, la zone de traçage et chaque série. Choisissez-en une, et **Mettre en forme la sélection** ouvre la fenêtre de mise en forme sur cette partie. **Rétablir selon le style** supprime les réglages de la partie choisie et lui redonne ceux que lui attribue le style du graphique. Pour la zone du graphique, il s'agit de son remplissage, de son trait et de la couleur du texte définie par une apparence (le texte clair de l'apparence sombre revient à la couleur de texte du thème). Pour la zone de traçage, il s'agit de son remplissage et de son trait. Pour une série, il s'agit de sa couleur (celle que donne le choix de **Couleurs** ou le style du graphique), de son contour, de ses marqueurs tels que le type de graphique les prévoit, ainsi que des couleurs de ses points ; une courbe lissée reste lissée si le type de graphique est un type lissé. Cette commande ne modifie ni les titres, ni la légende, ni les étiquettes, ni les axes, ni le quadrillage, ni les données, et elle n'agit pas sur une partie qui correspond déjà au style. Elle constitue une étape annulable.
- **Mettre en forme l'axe horizontal** et **Mettre en forme l'axe vertical** définissent les valeurs minimale et maximale, la taille de chaque graduation, l'ordre, le point où l'autre axe coupe, l'aspect des nombres et les repères de graduation.

## Les boutons à côté d'un graphique

Trois petits boutons se trouvent à droite d'un graphique sélectionné.

- **Éléments du graphique** (le plus) ouvre le même menu que **Éléments du graphique** de l'onglet.
- **Couleurs et styles du graphique** (le pinceau) affiche les jeux de couleurs et les apparences.
- **Filtres du graphique** (l'entonnoir) liste les séries. Décochez une série pour la retirer du graphique. Le fichier la conserve tout de même, de sorte qu'elle réapparaît lorsque vous la recochez. Un graphique affiche toujours au moins une série.

## Enregistrer un graphique dans un fichier OpenDocument

Si vous enregistrez le document au format **Texte OpenDocument** (`.odt`), chaque graphique est enregistré comme un objet distinct, et s'ouvre à nouveau comme un graphique que vous pouvez modifier. Le type, les nombres, le titre, la légende, les étiquettes, les couleurs, les axes, le quadrillage, jusqu'à quatre courbes de tendance par série et les barres d'erreur sont conservés. Certains éléments ne peuvent pas être enregistrés dans ce format, comme le tableau de données, les lignes de chute, les lignes haut-bas, les barres haut-bas, et les barres d'erreur dont vous avez saisi les valeurs. JeffWrite affiche la liste des éléments omis lors de l'enregistrement.

## Graphiques dans d'autres documents

JeffWrite affiche les graphiques contenus dans un document et les conserve tels quels. Il peut modifier le type, le titre, la légende et les étiquettes d'un graphique qu'il peut écrire en entier. Si un graphique comporte des éléments que JeffWrite ne modifie pas, comme des effets spéciaux, il le signale et laisse le graphique intact. Si un graphique lit ses nombres dans une feuille de calcul enregistrée avec lui, vous pouvez tout modifier, sauf les nombres.

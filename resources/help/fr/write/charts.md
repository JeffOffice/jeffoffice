# Graphiques

<!-- keywords: graphique, graphiques, diagramme, histogramme, graphique en colonnes, graphique en barres, graphique en courbes, graphique en secteurs, anneau, aires, nuage de points, insérer un graphique, données du graphique, modifier les données, titre du graphique, légende, étiquettes de données, type de graphique, format du graphique, axes, titre de l'axe, quadrillage, style de graphique, éléments du graphique, dispositions, couleurs, apparence, intervertir les lignes et les colonnes, type de graphique, mettre en forme l'axe, barres d'erreur, courbe de tendance, filtres du graphique, éléments du graphique -->

Un **graphique** transforme des nombres en image, sous forme de colonnes, de courbe ou de secteurs de camembert. Cette rubrique explique comment insérer un graphique dans un document, saisir ses nombres et modifier son apparence.

## Insérer un graphique

1. Placez le curseur à l'endroit où doit apparaître le graphique.
2. Cliquez sur **Insertion > Graphique**, puis choisissez un type : colonnes, colonnes empilées, colonnes empilées à 100 %, barres, barres empilées, courbes, courbes avec marqueurs, secteurs, anneau, aires, aires empilées, nuage de points, radar, radar avec marqueurs ou radar plein.

Un graphique **empilé** superpose les séries les unes sur les autres. Un graphique **empilé à 100 %** donne à chaque pile la même hauteur, ce qui permet de voir les proportions. Un graphique en **nuage de points** place chaque point selon ses deux nombres.

Le graphique apparaît avec des nombres d'exemple, aligné sur le texte, comme une grande lettre. L'onglet **Format du graphique** s'ouvre.

## Saisir vos propres nombres

Sélectionnez le graphique, puis cliquez sur **Format du graphique > Modifier les données**. Un tableau s'ouvre.

- La première colonne contient les mots placés sous les points (les **catégories**), par exemple les mois.
- La première ligne contient le nom de chaque **série**, c'est-à-dire un ensemble de nombres, par exemple un produit.
- Les autres cellules contiennent les nombres.

Cliquez sur **Ajouter une ligne** ou **Ajouter une colonne** pour en ajouter, ou sur **Supprimer la dernière ligne** ou **Supprimer la dernière colonne** pour en retirer. Cliquez sur **OK**. Si une cellule ne contient pas un nombre, le graphique n'est pas modifié et la barre d'état indique la ligne concernée. Un graphique peut comporter jusqu'à 200 catégories et 24 séries.

Pour un nuage de points, la première colonne contient les nombres qui placent les points de gauche à droite.

## Graphiques enregistrés par un autre logiciel

Certains graphiques enregistrés par d'autres logiciels conservent leurs nombres dans un petit **classeur** (un fichier de tableur) stocké à l'intérieur de votre fichier. Sélectionnez un tel graphique, puis cliquez sur **Format du graphique > Modifier les données**. Le tableau affiche les cellules de ce classeur que le graphique utilise. Modifiez un nombre, un nom placé sous un point ou le nom d'une série, puis cliquez sur **OK**. La modification est enregistrée à la fois dans le classeur et dans le graphique, en une seule étape annulable, de sorte que les autres logiciels affichent aussi les nouveaux nombres.

Vous ne pouvez pas ajouter ni supprimer une ligne ou une colonne de ce type de graphique. Les commandes **Données du graphique** et **Intervertir les lignes et les colonnes** sont alors grisées, car le graphique est lié à ces cellules. Il en va de même lorsque le graphique lit des cellules d'un autre fichier, une plage qui n'est ni une seule colonne ni une seule ligne, ou des cellules contenant une formule, une date ou du texte là où devrait figurer un nombre. La commande est alors grisée, et son info-bulle indique la raison.

## Modifier l'apparence

Ces commandes se trouvent dans l'onglet **Format du graphique**. Chacune constitue une étape annulable, et le graphique conserve ses couleurs et le reste de son apparence.

- **Modifier le type de graphique** transforme le graphique en un autre type, avec les mêmes nombres.
- **Titre du graphique** affiche ou masque le titre. **Modifier le titre du graphique** change son texte.
- **Légende** est la clé qui indique la couleur de chaque série. Placez-la à droite, en haut, à gauche ou en bas, ou supprimez-la.
- **Axes** affiche ou masque la ligne de mots sous le graphique (les **catégories**) et la ligne de nombres à côté (les **valeurs**), et définit un titre pour chacune. Un graphique en secteurs n'a pas d'axes.
- **Quadrillage** trace des lignes discrètes à travers le graphique pour faciliter la lecture des nombres. Les lignes **principales** correspondent aux grands nombres sur le côté, et les lignes **secondaires** se situent entre elles.
- **Styles de graphique** permet de choisir l'un des huit styles globaux.
- **Étiquettes de données** écrit chaque nombre sur sa colonne, son point ou sa part.
- **Renvoyer à la ligne automatiquement**, **Texte de remplacement**, ainsi que **Hauteur** et **Largeur** fonctionnent comme pour les images. Voir [Ajouter et modifier des images](pictures).

## L'onglet Graphique

Sélectionnez un graphique : l'onglet **Graphique** s'ouvre à côté de l'onglet **Format du graphique**. Toutes les commandes de cet onglet modifient le graphique en une seule étape annulable.

- **Éléments du graphique** ajoute, retire ou place un élément du graphique : **Axes**, **Titres des axes**, **Titre du graphique**, **Étiquettes de données** (ce qu'elles contiennent, leur emplacement et le signe qui les sépare), **Quadrillage**, **Légende**, **Barres d'erreur** (petites barres qui indiquent de combien un nombre peut varier), **Traits** (lignes de chute, lignes haut-bas et barres haut-bas), **Tableau de données** (une grille des nombres sous le graphique, avec une colonne par catégorie ; sa liste permet d'afficher ou de masquer le tableau, et de définir les traits entre les lignes, les traits entre les colonnes, le contour et les clés de légende, qui affichent la couleur de chaque série à côté de son nom ; il est proposé pour les graphiques en colonnes, en courbes et en aires), et **Courbe de tendance** (une droite qui indique la direction des nombres, avec son équation si vous le souhaitez).
- **Dispositions** définit plusieurs éléments à la fois, comme le titre, la légende, les étiquettes et le quadrillage.
- **Couleurs** attribue aux séries un nouveau jeu de couleurs. **Apparences** donne à l'ensemble du graphique un aspect, par exemple neutre, épuré, sombre ou doux.
- **Intervertir les lignes et les colonnes** inverse les nombres : les séries deviennent les mots placés sous les points, et ces mots deviennent les séries.
- **Données du graphique** affiche la liste des séries. Cochez celles à tracer, modifiez leurs noms et déplacez-les vers le haut ou vers le bas.
- **Modifier les données** ouvre le tableau des nombres.
- **Type de graphique** affiche un aperçu de chaque type. Choisissez un seul type pour l'ensemble du graphique, ou un type pour chaque série, par exemple des colonnes avec une courbe par-dessus.
- **Mettre en forme l'élément du graphique** définit le remplissage et le trait de la zone du graphique, du tracé (la zone où figurent les repères) ou d'une série. Pour une courbe, il définit également si elle est lissée et le repère placé à chaque point.
- **Sélection actuelle** répertorie les éléments du graphique : la zone du graphique, la zone de traçage et chaque série. Choisissez-en un, et **Mettre en forme la sélection** ouvre la fenêtre de mise en forme pour cet élément. **Rétablir selon le style** supprime les réglages appliqués à l'élément choisi et rétablit ceux que le style du graphique lui attribue. Pour la zone du graphique, il s'agit de son remplissage, de son trait et de la couleur du texte définie par une apparence (le texte clair de l'apparence sombre revient à la couleur de texte du thème). Pour la zone de traçage, il s'agit de son remplissage et de son trait. Pour une série, il s'agit de sa couleur (celle que lui attribue le choix de Couleurs ou le style du graphique), de son contour, de ses repères tels que le type de graphique les prévoit, et des couleurs de ses points ; une courbe lissée reste lissée si le type de graphique est un type lissé. Cette commande ne modifie ni les titres, ni la légende, ni les étiquettes, ni les axes, ni le quadrillage, ni les données, et elle ne fait rien à un élément qui correspond déjà au style. Elle constitue une étape annulable.
- **Mettre en forme l'axe horizontal** et **Mettre en forme l'axe vertical** définissent les nombres les plus bas et les plus élevés, la taille de chaque graduation, l'ordre, l'endroit où l'autre axe croise celui-ci, l'apparence des nombres et les repères de graduation.

## Les boutons à côté d'un graphique

Trois petits boutons apparaissent à droite d'un graphique sélectionné.

- **Éléments du graphique** (le signe plus) ouvre le même menu que **Éléments du graphique** de l'onglet Graphique.
- **Styles et couleurs du graphique** (le pinceau) répertorie les jeux de couleurs et les apparences.
- **Filtres du graphique** (l'entonnoir) répertorie les séries. Décochez une série pour la retirer du graphique. Le fichier la conserve, de sorte qu'elle réapparaît lorsque vous la cochez de nouveau. Un graphique affiche toujours au moins une série.

## Enregistrer un graphique dans un fichier OpenDocument

Si vous enregistrez le document au format **Texte OpenDocument** (`.odt`), chaque graphique est enregistré comme un objet distinct, et il s'ouvre de nouveau comme un graphique que vous pouvez modifier. Le type, les nombres, le titre, la légende, les étiquettes, les couleurs, les axes, le quadrillage, jusqu'à quatre courbes de tendance par série et les barres d'erreur sont conservés. Certains éléments ne peuvent pas être enregistrés dans ce format, comme le tableau de données, les lignes de chute, les lignes haut-bas, les barres haut-bas et les barres d'erreur dont vous avez saisi les valeurs. JeffWrite affiche la liste des éléments omis lors de l'enregistrement.

## Graphiques dans d'autres documents

JeffWrite affiche les graphiques contenus dans un document et les conserve tels quels. Il peut modifier le type, le titre, la légende et les étiquettes d'un graphique qu'il sait écrire dans son intégralité. Si un graphique comporte des éléments que JeffWrite ne modifie pas, comme des effets spéciaux, il l'indique et laisse le graphique inchangé. Si un graphique lit ses nombres dans une feuille de calcul enregistrée avec lui, vous pouvez tout modifier, sauf les nombres.

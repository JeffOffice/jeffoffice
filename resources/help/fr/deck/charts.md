# Graphiques

<!-- keywords: graphique, graphiques, diagramme, histogramme, graphique à colonnes, graphique à barres, graphique en courbes, graphique en secteurs, anneau, aires, nuage de points, insérer un graphique, données du graphique, modifier les données, titre du graphique, légende, étiquettes de données, type de graphique, format du graphique, axes, titre de l'axe, quadrillage, style de graphique, éléments du graphique, dispositions, couleurs, apparences, données du graphique, permuter les lignes et les colonnes, type de graphique, mettre en forme l'axe, barres d'erreur, courbe de tendance, filtres du graphique, éléments du graphique -->

Un **graphique** transforme des nombres en image, sous forme de colonnes, de courbe ou de secteurs de camembert. Cette rubrique explique comment insérer un graphique sur une diapositive, saisir ses nombres et modifier son apparence.

## Insérer un graphique sur une diapositive

1. Accédez à la diapositive où doit figurer le graphique.
2. Cliquez sur **Insertion > Graphique**, puis choisissez un type : colonnes, colonnes empilées, colonnes empilées à 100 %, barres, barres empilées, courbes, courbes avec marqueurs, secteurs, anneau, aires, aires empilées, nuage de points, radar, radar avec marqueurs, radar plein, haut-bas-fermeture, ouverture-haut-bas-fermeture, volume-haut-bas-fermeture ou volume-ouverture-haut-bas-fermeture.

Un graphique **boursier** représente des cours : une ligne par catégorie, allant du cours le plus haut au plus bas, avec un petit point pour la clôture, ou, lorsqu'il existe une ouverture, une barre entre l'ouverture et la clôture (claire pour une hausse, foncée pour une baisse). Le graphique est construit à partir de vos colonnes exactement dans l'ordre où elles se présentent, de gauche à droite, et rien ne vérifie que les nombres correspondent à leur place. Placez-les donc dans cet ordre : haut, bas et clôture ; ouverture, haut, bas et clôture ; volume, haut, bas et clôture ; ou volume, ouverture, haut, bas et clôture. Si les colonnes sont dans un autre ordre, le graphique est tout de même dessiné, mais les repères n'ont pas le sens attendu (le point de clôture se place sur la troisième colonne de cours, et la boîte va de la première colonne de cours à la dernière). Avec un volume, celui-ci est représenté par des colonnes lues sur l'axe de gauche, et les cours sont lus sur un second axe à droite. Si le nombre de colonnes ne correspond pas au type choisi, JeffDeck indique combien il en faut et ne crée aucun graphique.

Un graphique **empilé** superpose les séries les unes sur les autres. Un graphique **empilé à 100 %** donne à chaque pile la même hauteur, ce qui permet de voir les proportions. Un graphique en **nuage de points** place chaque point là où indiquent ses deux nombres.

Le graphique apparaît avec des exemples de nombres, au centre de la diapositive, et l'onglet **Format du graphique** s'ouvre. Un graphique est un objet comme une image ou une forme : faites-le glisser pour le déplacer, faites glisser une poignée pour modifier sa taille (le graphique est redessiné pour s'adapter, de sorte que ses textes gardent la bonne forme), et faites-le pivoter avec **Faire pivoter** dans le groupe **Dessin** de l'onglet **Accueil**.

## Saisir vos propres nombres

Sélectionnez le graphique, puis cliquez sur **Format du graphique > Modifier les données**. Un tableau s'ouvre.

- La première colonne contient les libellés placés sous les points (les **catégories**), par exemple les mois.
- La première ligne contient le nom de chaque **série**, c'est-à-dire un ensemble de nombres, par exemple un produit.
- Les autres cellules contiennent les nombres.

Cliquez sur **Ajouter une ligne** ou **Ajouter une colonne** pour en ajouter, ou sur **Supprimer la dernière ligne** ou **Supprimer la dernière colonne** pour en retirer. Cliquez sur **OK**. Si une cellule ne contient pas un nombre, le graphique n'est pas modifié, et la barre d'état indique la ligne concernée. Un graphique peut comporter jusqu'à 200 catégories et 24 séries.

Pour un nuage de points, la première colonne contient les nombres qui placent les points de gauche à droite.

## Graphiques enregistrés par un autre programme

Certains graphiques enregistrés par d'autres programmes conservent leurs nombres dans un petit **classeur** (un fichier de tableur) stocké dans votre fichier. Sélectionnez un tel graphique, puis cliquez sur **Format du graphique > Modifier les données**. Le tableau présente les cellules de ce classeur que le graphique lit. Modifiez un nombre, un libellé placé sous un point ou le nom d'une série, puis cliquez sur **OK**. La modification est enregistrée à la fois dans le classeur et dans le graphique, en une seule étape annulable, de sorte que les autres programmes affichent eux aussi les nouveaux nombres.

Vous ne pouvez pas ajouter ni supprimer une ligne ou une colonne de ce type de graphique, et **Données du graphique** ainsi que **Permuter les lignes et les colonnes** sont grisés, car le graphique est lié à ces cellules. Il en va de même lorsque le graphique lit des cellules d'un autre fichier, une plage qui n'est ni une seule colonne ni une seule ligne, ou des cellules contenant une formule, une date ou du texte là où un nombre est attendu. La commande est alors grisée, et son info-bulle indique la raison.

## Modifier l'apparence

Ces commandes se trouvent dans l'onglet **Format du graphique**. Chacune constitue une seule étape annulable, et le graphique conserve ses couleurs ainsi que le reste de son apparence.

- **Modifier le type de graphique** transforme le graphique en un autre type, avec les mêmes nombres. Pour passer à un type boursier, les séries du graphique doivent être dans l'ordre attendu par ce type ; sinon, JeffDeck l'indique et laisse le graphique tel quel.
- **Titre du graphique** affiche ou masque le titre. **Modifier le titre du graphique** change son texte.
- **Légende** est la clé qui indique la couleur de chaque série. Placez-la à droite, en haut, à gauche ou en bas, ou supprimez-la.
- **Axes** affiche ou masque la ligne de libellés sous le graphique (les **catégories**) et la ligne de nombres à côté (les **valeurs**), et définit un titre pour chacune. Un graphique en secteurs n'a pas d'axes.
- **Quadrillage** trace des lignes discrètes sur le graphique pour faciliter la lecture des nombres. Les lignes **principales** correspondent aux grands nombres de l'axe, et les lignes **secondaires** se situent entre elles.
- **Styles de graphique** choisit l'un des huit styles d'ensemble.
- **Étiquettes de données** inscrit chaque nombre sur sa colonne, son point ou sa part.

## L'onglet Graphique

Sélectionnez un graphique : l'onglet **Graphique** s'ouvre à côté de l'onglet **Format du graphique**. Toutes les commandes de cet onglet modifient le graphique en une seule étape annulable.

- **Éléments du graphique** ajoute, retire ou place un élément du graphique : **Axes**, **Titres des axes**, **Titre du graphique**, **Étiquettes de données** (leur contenu, leur emplacement et le signe qui les sépare), **Quadrillage**, **Légende**, **Barres d'erreur** (petites barres indiquant de combien un nombre peut varier), **Lignes** (lignes de chute, lignes haut-bas et barres haut-bas) et **Tableau de données** (une grille des nombres sous le graphique, avec une colonne par catégorie ; sa liste permet d'afficher ou de masquer le tableau, et de définir les traits entre les lignes, les traits entre les colonnes, le contour et les clés de légende, qui affichent la couleur de chaque série à côté de son nom ; il est proposé pour les graphiques en colonnes, en courbes et en aires), et **Courbe de tendance** (une ligne montrant vers où tendent les nombres, avec son équation si vous le souhaitez).
- **Dispositions** définit plusieurs éléments à la fois, tels que le titre, la légende, les étiquettes et le quadrillage.
- **Couleurs** attribue aux séries un nouveau jeu de couleurs. **Apparences** donne à l'ensemble du graphique un aspect, par exemple simple, épuré, sombre ou doux.
- **Permuter les lignes et les colonnes** inverse les nombres : les séries deviennent les libellés placés sous les points, et ces libellés deviennent les séries.
- **Données du graphique** liste les séries. Cochez celles à tracer, modifiez leurs noms et déplacez-les vers le haut ou vers le bas.
- **Modifier les données** ouvre le tableau de nombres.
- **Type de graphique** présente un aperçu de chaque type. Choisissez un type pour l'ensemble du graphique, ou un type pour chaque série, par exemple des colonnes avec une courbe par-dessus.
- **Mettre en forme l'élément du graphique** définit le remplissage et le contour de la zone du graphique, de la zone de traçage (le cadre dans lequel se placent les marques) ou d'une série. Pour une courbe, il permet aussi de choisir si elle est lissée et quel repère marque chaque point.
- **Sélection actuelle** liste les éléments du graphique : la zone du graphique, la zone de traçage et chaque série. Choisissez-en un, et **Mettre en forme la sélection** ouvre la fenêtre de mise en forme pour cet élément. **Réinitialiser selon le style** supprime les réglages appliqués à l'élément choisi et rétablit ceux que le style du graphique lui attribue. Pour la zone du graphique, il s'agit de son remplissage, de son contour et de la couleur du texte définie par une apparence (le texte clair de l'apparence sombre revient à la couleur de texte du thème). Pour la zone de traçage, il s'agit de son remplissage et de son contour. Pour une série, il s'agit de sa couleur (celle que lui attribuent le choix Couleurs ou le style du graphique), de son contour, de ses repères tels que le type de graphique les prévoit, et des couleurs de ses points ; une courbe lissée reste lissée si le type de graphique est un type lissé. Cette commande ne modifie ni les titres, ni la légende, ni les étiquettes, ni les axes, ni le quadrillage, ni les données, et elle ne touche pas à un élément qui correspond déjà au style. Elle constitue une étape annulable.
- **Mettre en forme l'axe horizontal** et **Mettre en forme l'axe vertical** définissent les nombres minimal et maximal, la taille de chaque graduation, l'ordre, l'endroit où l'autre axe coupe, l'aspect des nombres et les repères de graduation.

## Les boutons à côté d'un graphique

Trois petits boutons se trouvent à droite d'un graphique sélectionné.

- **Éléments du graphique** (le signe plus) ouvre le même menu que **Éléments du graphique** de l'onglet.
- **Styles et couleurs du graphique** (le pinceau) affiche les jeux de couleurs et les apparences.
- **Filtres du graphique** (l'entonnoir) liste les séries. Décochez une série pour l'exclure de l'image. Le fichier la conserve tout de même, de sorte qu'elle réapparaît lorsque vous la recochez. Un graphique garde toujours au moins une série visible.

## Graphiques dans d'autres présentations

Un graphique d'un fichier **.pptx** s'ouvre comme un graphique modifiable lorsqu'il contient ses nombres à l'intérieur du graphique. Si un graphique lit ses nombres dans une feuille de calcul conservée avec lui, vous pouvez modifier tout sauf les nombres. Un graphique comportant des éléments que JeffDeck ne modifie pas, tels que des effets spéciaux, est conservé tel quel.

## Enregistrement, impression et PDF

- Un fichier **.pptx** conserve le graphique en tant que graphique, de sorte qu'il s'ouvre comme graphique ici et dans d'autres programmes.
- Un fichier **.odp** ne peut pas contenir un graphique qui s'ouvre comme tel ici. JeffDeck enregistre alors le graphique sous forme d'image, et le message affiché après l'enregistrement le précise.
- Le **PDF** et l'impression dessinent le graphique sous forme de traits, de formes et de texte réel, et non d'image : il reste net quelle que soit sa taille.

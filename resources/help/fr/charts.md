# Graphiques

<!-- keywords: graphique, graphiques, diagramme, histogramme, graphique en colonnes, graphique en barres, graphique en courbes, graphique circulaire, secteurs, anneau, aires, nuage de points, insérer un graphique, données du graphique, modifier les données, titre du graphique, légende, étiquettes de données, type de graphique, format du graphique, axes, titre de l'axe, quadrillage, style de graphique, courbe de tendance, barres d'erreur, couleurs du graphique, disposition rapide, mettre en forme un élément du graphique -->

Un **graphique** transforme des nombres en image, sous forme de colonnes, de courbe ou de secteurs d'un camembert. Cette rubrique explique comment placer un graphique sur une page, comment saisir ses nombres et comment modifier son aspect.

## Placer un graphique sur une page

1. Allez à la page où doit se trouver le graphique.
2. Cliquez sur **Insertion > Graphique**, puis choisissez un type : colonnes, colonnes empilées, colonnes empilées à 100 %, barres, barres empilées, courbes, courbes avec marqueurs, secteurs, anneau, aires, aires empilées, nuage de points, radar, radar avec marqueurs, radar plein, haut-bas-fermeture, ouverture-haut-bas-fermeture, volume-haut-bas-fermeture ou volume-ouverture-haut-bas-fermeture.

Un graphique **boursier** montre des cours : une ligne pour chaque catégorie, du cours le plus haut au plus bas, avec un petit point pour la clôture ou, lorsqu'il existe une ouverture, une barre entre l'ouverture et la clôture (claire en cas de hausse, foncée en cas de baisse). Le graphique est tracé à partir de vos colonnes exactement dans l'ordre où elles se présentent, de gauche à droite, et rien ne vérifie que les nombres correspondent à leur place. Mettez donc vos colonnes dans l'un de ces ordres : haut, bas et clôture ; ouverture, haut, bas et clôture ; volume, haut, bas et clôture ; ou volume, ouverture, haut, bas et clôture. Si les colonnes sont dans un autre ordre, le graphique est tout de même tracé, mais les repères n'ont pas le même sens (le point de clôture se place sur la troisième colonne de cours, et la boîte va de la première à la dernière colonne de cours). Avec un volume, celui-ci est lu sur l'axe de gauche, et les cours sur un second axe, à droite. Si le nombre de colonnes ne correspond pas au type choisi, JeffOffice indique combien il en faut et ne crée aucun graphique.

Un graphique **empilé** superpose les séries les unes sur les autres. Un graphique **empilé à 100 %** donne à chaque pile la même hauteur, ce qui permet de voir des proportions. Un graphique en **nuage de points** place chaque point là où ses deux nombres l'indiquent.

Le graphique apparaît avec des valeurs d'exemple, au milieu de la page, et l'onglet **Format du graphique** s'ouvre. Un graphique est un objet comme une image ou une forme : faites-le glisser pour le déplacer, faites glisser une poignée pour changer sa taille (le graphique est retracé pour s'adapter, de sorte que ses mots gardent la bonne forme), et faites-le pivoter avec **Faire pivoter** dans le groupe **Organiser**.

## Saisir vos propres nombres

Sélectionnez le graphique, puis cliquez sur **Format du graphique > Modifier les données**. Un tableau s'ouvre.

- La première colonne contient les mots placés sous les points (les **catégories**), par exemple les mois.
- La première ligne contient le nom de chaque **série**, c'est-à-dire un ensemble de nombres, par exemple un produit.
- Les autres cellules contiennent les nombres.

Cliquez sur **Ajouter une ligne** ou **Ajouter une colonne** pour en ajouter, ou sur **Supprimer la dernière ligne** ou **Supprimer la dernière colonne** pour en retirer. Cliquez sur **OK**. Si une cellule ne contient pas un nombre, le graphique n'est pas modifié, et la barre d'état indique la ligne concernée. Un graphique peut comporter jusqu'à 200 catégories et 24 séries.

Pour un nuage de points, la première colonne contient les nombres qui placent les points de gauche à droite.

## Modifier l'aspect

Ces commandes se trouvent dans l'onglet **Format du graphique**. Chacune peut être annulée en une seule étape, et le graphique conserve ses couleurs ainsi que le reste de son aspect.

- **Modifier le type de graphique** transforme le graphique en un autre type, avec les mêmes nombres. Pour passer à un type boursier, les séries doivent être dans l'ordre que ce type demande ; sinon, le programme indique cet ordre et laisse le graphique tel quel.
- **Titre du graphique** affiche ou masque le titre. **Modifier le titre du graphique** change son texte.
- La **légende** est la clé qui indique la couleur de chaque série. Placez-la à droite, en haut, à gauche ou en bas, ou supprimez-la.
- **Axes** affiche ou masque la ligne de mots sous le graphique (les **catégories**) et la ligne de nombres à côté (les **valeurs**), et définit un titre pour chacune. Un graphique en secteurs n'a pas d'axes.
- **Quadrillage** trace des lignes discrètes à travers le graphique pour faciliter la lecture des nombres. Les lignes **principales** se placent aux grands nombres sur le côté, et les lignes **secondaires** se placent entre elles.
- **Styles de graphique** permet de choisir l'un des huit styles d'ensemble.
- **Étiquettes de données** inscrit chaque nombre sur sa colonne, son point ou son secteur.

## Ajouter des courbes de tendance, des barres d'erreur et des couleurs

Sélectionnez le graphique. Trois petits boutons apparaissent à côté, et les mêmes commandes se trouvent dans l'onglet **Format du graphique**.

- **Éléments du graphique** (le bouton plus) ajoute ou retire un élément du graphique : le titre, les étiquettes, un tableau des nombres, et d'autres. Son entrée **Courbe de tendance** trace une ligne à travers les points, qui montre vers où ils se dirigent (linéaire, exponentielle et autres types), et peut inscrire son équation sur le graphique. Son entrée **Barres d'erreur** trace une petite barre à chaque point, pour indiquer de combien le nombre réel peut s'écarter de celui affiché : une valeur fixe, un pourcentage ou un écart type des nombres.
- **Dispositions** permet de choisir l'une des nombreuses mises en place prédéfinies du titre, de la légende et des étiquettes.
- **Couleurs** modifie les couleurs des séries. **Styles** (le bouton pinceau) choisit un aspect général.
- **Filtres du graphique** (le bouton entonnoir) masque une série sans supprimer ses nombres.
- **Mettre en forme le graphique** ouvre une fenêtre pour l'aspect d'un élément : le graphique entier, la zone de traçage ou une série. **Mettre en forme l'axe horizontal** et **Mettre en forme l'axe vertical** définissent l'étendue, les intervalles et le format des nombres d'un axe.
- **Sélection actuelle** liste les éléments du graphique : la zone du graphique, la zone de traçage et chaque série. Choisissez-en un, et **Mettre en forme la sélection** ouvre la fenêtre de mise en forme pour cet élément. **Réinitialiser selon le style** annule ce qui a été défini sur l'élément choisi et lui rend ce que le style du graphique lui attribue. Pour la zone du graphique, il s'agit de son remplissage, de son trait et de la couleur de texte définie par un style (le texte clair du style sombre revient à la couleur de texte du thème). Pour la zone de traçage, il s'agit de son remplissage et de son trait. Pour une série, il s'agit de sa couleur (celle que lui donnent le choix Couleurs ou le style du graphique), de son contour, de ses marqueurs tels que les prévoit le type de graphique, et des couleurs de ses points ; une courbe lissée reste lissée lorsque le type de graphique est un type lissé. Cette commande ne modifie ni les titres, ni la légende, ni les étiquettes, ni les axes, ni le quadrillage, ni les données, et ne fait rien à un élément qui correspond déjà au style. Elle peut être annulée en une étape.
- **Modifier les données du graphique** et **Permuter les lignes et les colonnes** modifient les nombres qui forment les séries et ceux qui forment les catégories.

Chacune de ces commandes peut être annulée en une seule étape.

## Enregistrement, impression et PDF

- Un fichier **.jpub** conserve le graphique en tant que graphique, de sorte que vous pouvez l'ouvrir plus tard et modifier ses nombres.
- Un fichier **.pub** ne peut pas contenir un graphique que d'autres programmes ouvriraient comme graphique. JeffPub enregistre alors le graphique sous forme d'image et indique le nombre de graphiques enregistrés de cette façon. Conservez une copie **.jpub** si vous voulez modifier les nombres plus tard.
- Le **PDF** et l'impression dessinent le graphique sous forme de traits, de formes et de texte réel, et non d'image : il reste net à toute taille.

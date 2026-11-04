# Graphiques, sparklines, images et formes

<!-- keywords: graphique, diagramme, colonne, barre, ligne, secteur, aires, nuage de points, combiné, légende, titre, quadrillage, étiquettes de données, données du graphique, type de graphique, apparence, dispositions, déplacer, redimensionner, image, forme, objet, série, courbe de tendance, tendance, équation, R², deuxième axe, sparkline, sparklines, axe des dates, gain/perte, mini-graphique, marqueurs -->

Un graphique est une représentation de nombres qui reste liée à ceux-ci : si vous modifiez un nombre, le graphique change en conséquence. Les images et les formes se placent au-dessus des cellules de la même façon, mais elles ne sont liées à aucun nombre.

## Créer un graphique

1. Sélectionnez les nombres à représenter. Si la première ligne contient des noms et la première colonne des étiquettes, sélectionnez-les aussi. JeffCrunch examine la première ligne et la première colonne et les utilise comme noms et étiquettes lorsqu'il s'agit de mots (ou de dates, pour les étiquettes).
2. Dans l'onglet **Insertion**, dans **Graphiques**, choisissez un type : **Histogramme**, **Barres**, **Courbes**, **Secteurs**, **Aires**, **Nuage de points** ou **Graphique combiné**. La petite flèche à côté d'un type donne accès à ses variantes, par exemple les histogrammes empilés, une courbe avec marqueurs, ou un nuage de points avec des points, des points reliés ou des courbes lissées.
3. Pour voir d'abord chaque type, choisissez **Autres graphiques**. La liste à gauche affiche chaque type dessiné à partir de vos cellules. **Intervertir les lignes et les colonnes** change le sens des séries.

Si vous ne sélectionnez qu'une seule cellule, JeffCrunch représente le bloc de cellules autour d'elle qui contient des données.

Les séries sont disposées en colonnes lorsque la sélection compte au moins autant de lignes que de colonnes, et en lignes lorsqu'elle compte plus de colonnes que de lignes. Un secteur ne contient qu'une série. Un nuage de points prend ses valeurs horizontales dans la première colonne.

Exemple : vous sélectionnez A1:C4, où A2:A4 contiennent T1, T2 et T3, B1 et C1 contiennent Est et Ouest, et le reste sont des nombres. Le graphique comporte deux séries, Est et Ouest, et trois groupes de colonnes, T1 à T3.

## Déplacer et redimensionner

Cliquez sur un graphique, une image ou une forme pour la sélectionner. Faites-la glisser pour la déplacer, ou faites glisser l'un des huit carrés de son bord pour la redimensionner.

- Maintenez la touche **Alt** enfoncée pendant le glissement pour aligner un bord sur le bord d'une cellule.
- Maintenez la touche **Maj** enfoncée pendant que vous faites glisser un coin pour conserver les proportions.
- Les touches fléchées déplacent l'objet de quelques pixels à la fois. **Ctrl** avec une flèche le déplace d'un pixel, **Maj** avec une flèche l'agrandit ou le réduit, et **Alt** avec une flèche le place sur le bord suivant d'une cellule.
- **Échap** pendant le glissement le remet à sa place.

Chaque déplacement ou redimensionnement constitue une étape que vous pouvez annuler. Faites un clic droit sur l'objet et choisissez **Propriétés** pour saisir sa taille, et pour choisir son comportement lors de l'insertion, de la suppression ou du redimensionnement de cellules : se déplacer et se redimensionner avec les cellules, se déplacer avec elles tout en conservant sa taille, ou rester à sa place. Ce choix est enregistré dans le fichier. **Ne pas imprimer l'objet**, dans la même boîte, masque l'objet sur les pages imprimées (il reste visible à l'écran) ; le fichier conserve aussi ce choix.

## Modifier ce qu'affiche un graphique

Lorsqu'un graphique est sélectionné, deux boutons apparaissent à côté de lui et un onglet **Graphique** apparaît sur le ruban.

- Le bouton **+** (et **Éléments du graphique** dans l'onglet) active ou désactive le titre du graphique, les axes, les titres des axes, le quadrillage, la légende et les étiquettes de données, et indique où placer la légende et les étiquettes.
- Le bouton pinceau (et **Couleurs** et **Styles** dans l'onglet) applique au graphique l'un des jeux de couleurs et l'un des styles de JeffCrunch. **Dispositions** règle plusieurs éléments à la fois.
- **Données du graphique** affiche la liste des séries du graphique. Vous pouvez en ajouter, en modifier, en supprimer et en réordonner, changer les cellules que chacune lit, définir les cellules contenant les étiquettes, intervertir les lignes et les colonnes, et choisir la façon dont les cellules vides et les lignes et colonnes masquées sont représentées.
- **Courbe de tendance** (dans le menu **+**) trace une droite ajustée aux nombres de chaque série, afin de voir vers où ils se dirigent. Choisissez **Linéaire** pour une droite, **Exponentielle** pour des nombres qui augmentent d'un pourcentage constant, **Logarithmique** pour des nombres qui augmentent rapidement puis se stabilisent, **Polynomiale** pour une courbe avec des inflexions, **Puissance**, ou **Moyenne mobile**, qui trace la moyenne des derniers nombres. **Afficher l'équation** écrit la formule de la droite sur le graphique, et **Afficher le R²** indique la qualité de l'ajustement (1 est un ajustement parfait ; 0 n'indique aucun ajustement). La courbe de tendance a sa propre entrée dans la légende.
- Lorsqu'une série est représentée sur un second axe vertical, les menus **Axes** et **Titres des axes** proposent les entrées **Horizontal secondaire** et **Vertical secondaire** pour cette série. Le menu **Légende** propose **Superposé**, qui place la légende sur la zone de tracé au lieu de la placer à côté.
- **Étiquettes de données** propose aussi **Étiquette contient** (la valeur, le nom de la catégorie, le nom de la série et le pourcentage, combinés à volonté), **Séparateur** (ce qui les sépare : une virgule, un point-virgule, un point, un saut de ligne ou un espace) et **Format de nombre…** (un code de format comme 0,0 pour les nombres des étiquettes ; laissez ce champ vide pour afficher les nombres tels quels). Désactivez tous les contenus et les étiquettes disparaissent.
- **Format de l'axe horizontal…** et **Format de l'axe vertical…** dans l'onglet **Graphique** ouvrent une boîte pour un seul axe. Elle permet de définir les valeurs minimale et maximale, ainsi que les unités principale et secondaire (chacune automatique ou un nombre que vous saisissez), d'inverser l'ordre des valeurs, d'indiquer où l'autre axe coupe celui-ci, de choisir le format des nombres de l'axe, ses graduations, la position de ses étiquettes et leur inclinaison. Seul un axe numérique a des bornes et des unités.
- **Type de graphique** modifie le type pour l'ensemble du graphique ou pour chaque série : des histogrammes avec une courbe par-dessus, ou une série sur un second axe vertical.

Une référence à des cellules s'écrit par exemple **Feuil1!$B$2:$B$9**, avec le nom de la feuille entre apostrophes lorsqu'il contient des espaces.

## Sparklines

Une sparkline est un mini-graphique qui tient dans une seule cellule. Elle montre la tendance d'une ligne ou d'une colonne de nombres, directement à côté de ceux-ci.

1. Sélectionnez les nombres. Pour une sparkline par ligne, sélectionnez plusieurs lignes de nombres.
2. Dans l'onglet **Insertion**, dans **Sparklines**, choisissez **Courbe**, **Histogramme** ou **Gain/perte**. Une **Courbe** relie les nombres par un trait. Un **Histogramme** trace une barre pour chaque nombre. **Gain/perte** trace une barre vers le haut pour chaque nombre positif et une barre vers le bas pour chaque nombre négatif, toutes de même taille, ce qui convient aux résultats qui ne sont que bons ou mauvais.
3. Dans la boîte **Créer des sparklines**, vérifiez la **Plage de données** (les cellules que lisent les sparklines) et saisissez la **Plage d'emplacement** (les cellules qui les accueillent). Choisissez une cellule d'emplacement pour chaque ligne de données, ou pour chaque colonne de données. Si vous choisissez une seule cellule, la sparkline lit toutes les données.

Les sparklines créées en une seule opération forment un groupe et partagent le même style. Cliquez sur une cellule contenant une sparkline pour faire apparaître l'onglet **Sparkline**.

- **Modifier les données** modifie les données et l'emplacement de tout le groupe, ou les données de la seule sparkline sélectionnée. Elle indique aussi quoi faire des cellules vides (laisser un intervalle, les traiter comme zéro ou relier les points de part et d'autre) et si les cellules des lignes et colonnes masquées doivent être lues.
- **Type** permet de passer de courbe à histogramme, puis à gain/perte.
- **Afficher** met en évidence des points : le **Point haut**, le **Point bas**, les **Points négatifs**, le **Premier point**, le **Dernier point» et, pour une courbe, les **Marqueurs** sur chaque point.
- **Styles** applique au groupe un jeu de couleurs. **Couleur de la sparkline** définit la couleur de la courbe ou des barres, et l'**Épaisseur** (l'épaisseur du trait, en points). **Couleur du marqueur** définit la couleur de chaque type de point mis en évidence.
- **Axe** affiche une ligne à zéro, trace les nombres de droite à gauche et définit les valeurs minimale et maximale de l'axe vertical. **Automatique pour chaque sparkline** ajuste chacune à ses propres nombres. **Identique pour toutes les sparklines** donne une même échelle à tout le groupe, pour pouvoir les comparer. **Valeur personnalisée** vous permet de saisir un nombre. **Axe des dates…** espace les points selon les dates : saisissez les cellules qui contiennent la date de chaque point, et un point dont la date est plus récente se place plus loin. Laissez ce champ vide pour répartir de nouveau les points de façon régulière.
- **Grouper** place les sparklines sélectionnées dans un même groupe. **Dissocier** donne à chacune son propre style. **Effacer** supprime les sparklines sélectionnées ou leurs groupes entiers.

Une sparkline suit ses cellules : si vous insérez une ligne au-dessus des données, elle lit toujours les mêmes nombres. Chaque modification constitue une étape que vous pouvez annuler. Les sparklines sont enregistrées dans le fichier sous une forme que les autres tableurs peuvent lire.

## Couper, copier et coller un objet

Une image ou une forme se copie et se colle de la même façon qu'un graphique. Une forme qui utilise autre chose qu'une image du classeur, comme un lien vers un fichier, ne peut pas être copiée, et JeffCrunch vous indique la raison.

Copiez un graphique et collez-le dans le même classeur, sur n'importe quelle feuille : la copie lit les mêmes cellules que l'original. Si vous le collez dans un autre classeur, il conserve les nombres qu'il affichait mais ne lit plus de cellules, car les cellules qu'il lirait à cet endroit ne sont pas celles à partir desquelles il a été créé.

## Nommer un objet

Lorsqu'un objet est sélectionné, la zone Nom affiche son nom. Saisissez-y un nouveau nom et appuyez sur **Entrée**. Annuler rétablit l'ancien nom, et le nouveau nom est enregistré dans le fichier.

## Supprimer un objet

Appuyez sur **Suppr** lorsque l'objet est sélectionné. Annuler le restaure.

## Ce qui est enregistré

Tout ce qui précède est enregistré dans le fichier sous une forme que les autres tableurs peuvent lire, de sorte qu'un graphique créé ici s'ouvre de la même façon ailleurs. Un graphique provenant d'un autre logiciel, comportant des éléments que JeffCrunch ne peut pas conserver lorsqu'il réécrit un graphique (barres d'erreur, graphique en trois dimensions), peut tout de même être déplacé, redimensionné et supprimé, mais ses éléments et ses données ne sont pas modifiés ici.

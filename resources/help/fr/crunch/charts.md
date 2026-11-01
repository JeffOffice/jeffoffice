# Graphiques, images et formes
<!-- keywords: graphique, diagramme, histogramme, colonne, barres, courbe, secteurs, camembert, aires, nuage de points, graphique combiné, légende, titre, quadrillage, étiquettes de données, données du graphique, type de graphique, apparence, dispositions, déplacer, redimensionner, image, forme, objet, série -->

Un graphique est une représentation de nombres qui reste liée à ceux-ci : modifiez un nombre et le graphique change avec lui. Les images et les formes se placent de la même façon au-dessus des cellules, mais elles ne sont liées à aucun nombre.

## Créer un graphique

1. Sélectionnez les nombres à représenter. Si la première ligne contient des noms et la première colonne des étiquettes, sélectionnez-les aussi. JeffCrunch examine la première ligne et la première colonne et les utilise comme noms et comme étiquettes lorsqu'elles contiennent des mots (ou des dates, pour les étiquettes).
2. Sous l'onglet **Insertion**, dans le groupe **Graphiques**, choisissez un type : **Histogramme**, **Barres**, **Courbes**, **Secteurs**, **Aires**, **Nuage de points** ou **Graphique combiné**. La petite flèche à côté d'un type propose ses variantes, par exemple un histogramme empilé, des courbes avec marqueurs, ou un nuage de points simple, à points reliés ou avec courbes lissées.
3. Pour voir d'abord tous les types, choisissez **Autres graphiques**. La liste de gauche montre chaque type dessiné à partir de vos cellules. **Intervertir les lignes et les colonnes** oriente les séries dans l'autre sens.

Si vous ne sélectionnez qu'une cellule, JeffCrunch représente le bloc de cellules non vides qui l'entoure.

Les séries suivent les colonnes lorsque la sélection compte au moins autant de lignes que de colonnes, et les lignes lorsqu'elle compte plus de colonnes que de lignes. Un graphique en secteurs contient une seule série. Un nuage de points prend ses valeurs horizontales dans la première colonne.

Exemple : vous sélectionnez A1:C4, où A2:A4 contiennent T1, T2 et T3, B1 et C1 contiennent Est et Ouest, et le reste des nombres. Le graphique comporte deux séries, Est et Ouest, et trois groupes de colonnes, de T1 à T3.

## Le déplacer et le redimensionner

Cliquez sur un graphique, une image ou une forme pour la sélectionner. Faites-la glisser pour la déplacer, ou faites glisser l'un des huit carrés de son contour pour la redimensionner.

- Maintenez la touche **Alt** enfoncée pendant le glissement pour aligner un bord sur le bord d'une cellule.
- Maintenez la touche **Maj** enfoncée pendant le glissement d'un angle pour conserver les proportions.
- Les touches de direction déplacent l'objet de quelques pixels à la fois. **Ctrl** avec une touche de direction le déplace d'un pixel, **Maj** avec une touche de direction l'agrandit ou le réduit, et **Alt** avec une touche de direction l'amène au bord de cellule suivant.
- **Échap** pendant le glissement le remet à sa place.

Chaque déplacement ou redimensionnement constitue une étape que vous pouvez annuler. Cliquez avec le bouton droit sur l'objet et choisissez **Propriétés** pour saisir sa taille et choisir son comportement lorsque vous insérez, supprimez ou redimensionnez des cellules : se déplacer et se dimensionner avec les cellules, se déplacer avec elles sans changer de taille, ou ne pas bouger. Ce choix est enregistré dans le fichier.

## Modifier ce qu'affiche un graphique

Lorsqu'un graphique est sélectionné, deux boutons apparaissent à côté et un onglet **Graphique** apparaît sur le ruban.

- Le bouton **+** (et **Éléments du graphique** sous l'onglet) affiche ou masque le titre, les axes, les titres des axes, le quadrillage, la légende et les étiquettes de données du graphique, et indique où placer la légende et les étiquettes.
- Le bouton pinceau (et **Couleurs** et **Apparences** sous l'onglet) applique au graphique l'un des jeux de couleurs et l'une des apparences de JeffCrunch. **Dispositions** définit plusieurs éléments à la fois.
- **Données du graphique** répertorie les séries du graphique. Vous pouvez les ajouter, les modifier, les supprimer et les réorganiser, changer les cellules lues par chacune, définir les cellules contenant les étiquettes, intervertir les lignes et les colonnes, et choisir comment représenter les cellules vides et les lignes et colonnes masquées.
- **Type de graphique** change le type de l'ensemble du graphique, ou de chaque série : un histogramme surmonté d'une courbe, ou une série sur un axe vertical secondaire.

Une référence à des cellules s'écrit sous la forme **Feuil1!$B$2:$B$9**, avec le nom de la feuille entre guillemets simples s'il contient des espaces.

## Couper, copier et coller un objet

Une image ou une forme se copie et se colle de la même façon qu'un graphique. Une forme qui utilise autre chose qu'une image du classeur, comme un lien vers un fichier, ne peut pas être copiée, et JeffCrunch vous en indique la raison.


Si vous copiez un graphique et le collez dans le même classeur, sur n'importe quelle feuille, la copie lit les mêmes cellules que l'original. Si vous le collez dans un autre classeur, il conserve les nombres qu'il affichait mais ne lit plus de cellules, car les cellules qu'il y lirait ne sont pas celles à partir desquelles il a été créé.

## Le nommer

Lorsqu'un objet est sélectionné, la zone Nom affiche son nom. Tapez-y un nouveau nom et appuyez sur **Entrée**. Annuler rétablit l'ancien nom, et le nouveau nom est enregistré dans le fichier.

## Le supprimer

Appuyez sur **Suppr** lorsque l'objet est sélectionné. Annuler le rétablit.

## Ce qui est enregistré

Tout ce qui précède est enregistré dans le fichier sous la forme que lisent les autres tableurs ; un graphique créé ici s'y ouvre donc de la même façon. Un graphique provenant d'un autre programme et contenant un élément que JeffCrunch ne peut pas conserver lorsqu'il réécrit un graphique (un remplissage dégradé, une courbe de tendance, un graphique en trois dimensions) peut toujours être déplacé, redimensionné et supprimé, mais ses éléments et ses données ne sont pas modifiés ici.

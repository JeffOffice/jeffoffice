# Graphiques, mini-graphiques, images et formes

<!-- keywords: graphique, diagramme, histogramme, colonne, barre, courbe, secteurs, aires, nuage de points, combiné, légende, titre, quadrillage, étiquettes de données, données du graphique, type de graphique, apparence, dispositions, déplacer, redimensionner, image, forme, objet, série, courbe de tendance, tendance, équation, R², deuxième axe, sparkline, sparklines, axe des dates, gain/perte, mini-graphique, marqueurs, graphique sparkline -->

Un graphique est une représentation de nombres qui reste liée à ces nombres : si vous modifiez un nombre, le graphique change en conséquence. Les images et les formes se placent au-dessus des cellules de la même manière, mais elles ne sont liées à aucun nombre.

## Créer un graphique

1. Sélectionnez les nombres à représenter. Si la première ligne contient des noms et la première colonne des étiquettes, sélectionnez-les aussi. JeffCrunch examine la première ligne et la première colonne : il les utilise comme noms et comme étiquettes lorsqu'il s'agit de mots (ou de dates, pour les étiquettes).
2. Dans l'onglet **Insertion**, dans le groupe **Graphiques**, choisissez un type : **Histogramme**, **Barres**, **Courbes**, **Secteurs**, **Aires**, **Nuage de points** ou **Graphique combiné**. La petite flèche à côté d'un type donne ses variantes, comme les histogrammes empilés, les courbes avec marqueurs ou les nuages de points avec points, avec points reliés ou avec courbes lissées.
3. Pour voir d'abord chaque type, choisissez **Autres graphiques**. La liste à gauche affiche chaque type dessiné à partir de vos cellules. **Inverser lignes et colonnes** change le sens des séries.

Si vous ne sélectionnez qu'une seule cellule, JeffCrunch crée le graphique à partir du bloc de cellules qui l'entoure et qui contient des données.

Les séries sont placées en colonnes lorsque la sélection compte au moins autant de lignes que de colonnes, et en lignes lorsqu'elle compte plus de colonnes que de lignes. Un graphique en secteurs ne comporte qu'une série. Un nuage de points prend ses valeurs horizontales dans la première colonne.

Exemple : vous sélectionnez A1:C4, où A2:A4 contiennent T1, T2 et T3, où B1 et C1 contiennent Est et Ouest, et où le reste correspond à des nombres. Le graphique comporte deux séries, Est et Ouest, et trois groupes de colonnes, de T1 à T3.

## Déplacer et redimensionner

Cliquez sur un graphique, une image ou une forme pour la sélectionner. Faites-la glisser pour la déplacer, ou faites glisser l'un des huit carrés de son contour pour la redimensionner.

- Maintenez la touche **Alt** enfoncée pendant le glissement pour aligner un bord sur le bord d'une cellule.
- Maintenez la touche **Maj** enfoncée pendant le glissement d'un coin pour conserver les proportions.
- Les touches fléchées déplacent l'objet de quelques pixels à la fois. **Ctrl** avec une touche fléchée le déplace d'un pixel, **Maj** avec une touche fléchée l'agrandit ou le réduit, et **Alt** avec une touche fléchée le fait aller jusqu'au bord de cellule suivant.
- **Échap** pendant le glissement le remet à sa place.

Chaque déplacement ou redimensionnement correspond à une étape que vous pouvez annuler. Faites un clic droit sur l'objet et choisissez **Propriétés** pour saisir sa taille, et pour choisir son comportement lors de l'insertion, de la suppression ou du redimensionnement de cellules : se déplacer et se redimensionner avec les cellules, se déplacer avec elles tout en conservant sa taille, ou rester en place. Ce choix est enregistré dans le fichier. **Imprimer cet objet**, dans la même boîte, masque l'objet sur les pages imprimées (il reste visible à l'écran) ; le fichier conserve aussi ce choix.

## Modifier ce qu'affiche un graphique

Lorsqu'un graphique est sélectionné, deux boutons apparaissent à côté et un onglet **Graphique** s'affiche dans le ruban.

- Le bouton **+** (et **Éléments du graphique** dans l'onglet) active ou désactive le titre du graphique, les axes, les titres des axes, le quadrillage, la légende et les étiquettes de données, et indique où placer la légende et les étiquettes.
- Le bouton pinceau (et **Couleurs** et **Styles** dans l'onglet) applique au graphique un des jeux de couleurs et un des styles de JeffCrunch. **Dispositions** règle plusieurs éléments à la fois.
- **Données du graphique** affiche la liste des séries du graphique. Vous pouvez en ajouter, en modifier, en supprimer et en réordonner, changer les cellules que chacune lit, définir les cellules contenant les étiquettes, inverser lignes et colonnes, et choisir la façon dont les cellules vides et les lignes et colonnes masquées sont représentées.
- **Courbe de tendance** (dans le menu **+**) trace une droite ajustée aux nombres de chaque série, pour voir vers où ils tendent. Choisissez **Linéaire** pour une droite, **Exponentielle** pour des nombres qui augmentent d'un même pourcentage, **Logarithmique** pour des nombres qui montent rapidement puis se stabilisent, **Polynomiale** pour une courbe à plusieurs inflexions, **Puissance**, ou **Moyenne mobile**, qui trace la moyenne des derniers nombres. **Afficher l'équation** inscrit la formule de la courbe sur le graphique, et **Afficher le R²** indique la qualité de l'ajustement (1 correspond à un ajustement parfait ; 0, à aucun ajustement). La courbe de tendance a sa propre entrée dans la légende.
- Lorsqu'une série est tracée par rapport à un second axe vertical, les menus **Axes** et **Titres des axes** proposent les entrées **Horizontal secondaire** et **Vertical secondaire** pour cette série. Le menu **Légende** propose **Au-dessus du graphique**, qui place la légende sur la zone de tracé au lieu de la placer à côté.
- **Étiquettes de données** propose aussi **Contenu de l'étiquette** (la valeur, le nom de la catégorie, le nom de la série et le pourcentage, combinés à volonté), **Séparateur** (ce qui les sépare : une virgule, un point-virgule, un point, un saut de ligne ou un espace) et **Format de nombre…** (un code de format tel que 0,0 pour les nombres des étiquettes ; laissez-le vide pour afficher les nombres tels quels). Si vous désactivez tous les contenus, les étiquettes disparaissent.
- **Mettre en forme l'axe horizontal…** et **Mettre en forme l'axe vertical…** dans l'onglet **Graphique** ouvrent une boîte pour un seul axe. Elle permet de définir les valeurs minimale et maximale ainsi que les unités principale et secondaire (chacune automatique ou un nombre que vous saisissez), d'inverser l'ordre des valeurs, d'indiquer où l'autre axe coupe celui-ci, de choisir le format des nombres de l'axe, ses graduations, la position de ses étiquettes et leur inclinaison. Seul un axe numérique possède des bornes et des unités.
- **Déplacer le graphique…** dans l'onglet **Graphique** demande une autre feuille de calcul et y place le graphique, dans les mêmes cellules. Il lit toujours les cellules qu'il lisait, conserve sa taille et sa position sur la feuille, même lorsque les lignes et les colonnes sont plus hautes ou plus larges, et se déplace et se redimensionne avec les cellules comme auparavant. Les feuilles masquées ne sont pas proposées. Cette opération correspond à une seule étape d'**Annuler**, même après un enregistrement. Une feuille graphique (feuille qui ne contient qu'un graphique) ne peut pas encore être créée, et un graphique qui se trouve sur une telle feuille ne peut pas être déplacé.
- **Type de graphique** modifie le type pour l'ensemble du graphique, ou pour chaque série : colonnes avec une courbe par-dessus, ou une série par rapport à un second axe vertical.

Une référence à des cellules s'écrit sous la forme **Feuil1!$B$2:$B$9**, le nom de la feuille étant placé entre apostrophes lorsqu'il contient des espaces.

## Mini-graphiques (sparklines)

Un mini-graphique est un petit graphique qui tient dans une seule cellule. Il montre la tendance d'une ligne ou d'une colonne de nombres, juste à côté de ceux-ci.

1. Sélectionnez les nombres. Pour un mini-graphique par ligne, sélectionnez plusieurs lignes de nombres.
2. Dans l'onglet **Insertion**, dans le groupe **Mini-graphiques**, choisissez **Courbe**, **Histogramme** ou **Gain/perte**. **Courbe** relie les nombres par un trait. **Histogramme** trace une barre pour chaque nombre. **Gain/perte** trace une barre vers le haut pour chaque nombre positif et une barre vers le bas pour chaque nombre négatif, toutes de même hauteur, ce qui convient aux résultats qui ne sont que bons ou mauvais.
3. Dans la boîte **Créer des mini-graphiques**, vérifiez la **Plage de données** (les cellules lues par les mini-graphiques) et saisissez la **Plage d'emplacement** (les cellules qui les accueillent). Choisissez une cellule d'emplacement pour chaque ligne de données, ou pour chaque colonne de données. Si vous choisissez une seule cellule, le mini-graphique lit l'ensemble des données.

Les mini-graphiques créés en une seule opération forment un groupe et partagent le même style. Cliquez sur une cellule contenant un mini-graphique : l'onglet **Mini-graphique** apparaît.

- **Modifier les données** modifie les données et l'emplacement de tout le groupe, ou les données du seul mini-graphique sélectionné. Elle indique aussi quoi faire des cellules vides (laisser un espace, les traiter comme zéro ou relier les points de part et d'autre) et si les cellules des lignes et colonnes masquées doivent être lues.
- **Type** permet de passer de courbe à histogramme ou gain/perte.
- **Afficher** marque certains points : le **Point haut**, le **Point bas**, les **Points négatifs**, le **Premier point**, le **Dernier point» et, pour une courbe, les **Marqueurs** sur chaque point.
- **Styles** applique au groupe un jeu de couleurs. **Couleur du mini-graphique** définit la couleur de la courbe ou des barres, et l'**Épaisseur** (l'épaisseur du trait, en points). **Couleur du marqueur** définit la couleur de chaque type de point marqué.
- **Axe** affiche un trait à zéro, trace les nombres de droite à gauche et définit les valeurs minimale et maximale de l'axe vertical. **Automatique pour chaque mini-graphique** adapte chacun à ses propres nombres. **Identique pour tous les mini-graphiques** donne au groupe une seule échelle, pour pouvoir les comparer. **Valeur personnalisée** vous permet de saisir un nombre. **Axe des dates…** espace les points selon leurs dates : saisissez les cellules contenant la date de chaque point, et un point dont la date est plus tardive se place plus loin. Laissez le champ vide pour répartir de nouveau les points uniformément.
- **Grouper** place les mini-graphiques sélectionnés dans un même groupe. **Dissocier** donne à chacun son propre aspect. **Effacer** supprime les mini-graphiques sélectionnés, ou leurs groupes entiers.

Un mini-graphique suit ses cellules : si vous insérez une ligne au-dessus des données, il lit toujours les mêmes nombres. Chaque modification correspond à une étape que vous pouvez annuler. Les mini-graphiques sont enregistrés dans le fichier sous une forme que les autres tableurs peuvent lire.

## Couper, copier et coller un objet

Une image ou une forme se copie et se colle de la même manière qu'un graphique. Une forme qui utilise autre chose qu'une image du classeur, par exemple un lien vers un fichier, ne peut pas être copiée, et JeffCrunch vous indique pourquoi.

Copiez un graphique et collez-le dans le même classeur, sur n'importe quelle feuille : la copie lit les mêmes cellules que l'original. Si vous le collez dans un autre classeur, il conserve les nombres affichés mais ne lit plus de cellules, car les cellules qu'il lirait là-bas ne sont pas celles à partir desquelles il a été créé.

## Nommer un objet

Lorsqu'un objet est sélectionné, la zone Nom affiche son nom. Saisissez-y un nouveau nom et appuyez sur **Entrée**. **Annuler** rétablit l'ancien nom, et le nouveau nom est enregistré dans le fichier.

## Supprimer un objet

Appuyez sur **Suppr** lorsque l'objet est sélectionné. **Annuler** le rétablit.

## Ce qui est enregistré

Tout ce qui précède est enregistré dans le fichier sous une forme que les autres tableurs peuvent lire : un graphique créé ici s'ouvre donc de la même manière ailleurs. Un graphique provenant d'un autre logiciel et contenant des éléments que JeffCrunch ne peut pas conserver lorsqu'il réécrit un graphique (barres d'erreur, graphique en trois dimensions) peut tout de même être déplacé, redimensionné et supprimé, mais ses éléments et ses données ne sont pas modifiés ici.

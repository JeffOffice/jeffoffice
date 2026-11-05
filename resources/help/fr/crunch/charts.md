# Graphiques, sparklines, images et formes

<!-- keywords: graphique, diagramme, colonne, barre, courbe, secteurs, aires, nuage de points, combiné, légende, titre, quadrillage, étiquettes de données, données du graphique, type de graphique, apparence, dispositions, déplacer, redimensionner, image, forme, objet, série, courbe de tendance, tendance, équation, R², second axe, sparkline, sparklines, axe des dates, gain/perte, mini graphique, marqueurs, nom, texte de remplacement, texte alternatif, logarithmique, échelle logarithmique, enregistrer comme image, modèle, crtx, texte du titre, repère de légende -->

Un graphique est une image de nombres qui reste liée à eux : si vous modifiez un nombre, le graphique change en conséquence. Les images et les formes se placent au-dessus des cellules de la même manière, mais elles ne sont liées à aucun nombre.

## Créer un graphique

1. Sélectionnez les nombres à représenter. Si la première ligne contient des noms et la première colonne des étiquettes, sélectionnez-les aussi. JeffCrunch examine la première ligne et la première colonne et les utilise comme noms et comme étiquettes lorsqu'il s'agit de mots (ou de dates, pour les étiquettes).
2. Dans l'onglet **Insertion**, dans le groupe **Graphiques**, choisissez un type : **Colonne**, **Barre**, **Courbe**, **Secteurs**, **Aires**, **Nuage de points** ou **Combiné**. La petite flèche à côté d'un type donne ses variantes, par exemple des colonnes empilées, une courbe avec marqueurs, ou un nuage de points avec des points isolés, des points reliés ou des courbes lissées.
3. Pour voir d'abord chaque type, choisissez **Autres graphiques**. La liste à gauche affiche chaque type tracé à partir de vos cellules. **Intervertir les lignes et les colonnes** change le sens des séries.

Si vous ne sélectionnez qu'une seule cellule, JeffCrunch représente le bloc de cellules qui l'entoure et qui contient des données.

Les séries sont disposées en colonnes lorsque la sélection compte au moins autant de lignes que de colonnes, et en lignes lorsqu'elle compte plus de colonnes que de lignes. Un graphique en secteurs ne contient qu'une série. Un nuage de points prend ses valeurs horizontales dans la première colonne.

Exemple : vous sélectionnez A1:C4, où A2:A4 contiennent T1, T2 et T3, où B1 et C1 contiennent Est et Ouest, et où le reste est constitué de nombres. Le graphique comporte deux séries, Est et Ouest, et trois groupes de colonnes, T1 à T3.

## Déplacer et redimensionner

Cliquez sur un graphique, une image ou une forme pour le sélectionner. Faites-le glisser pour le déplacer, ou faites glisser l'un des huit carrés de son bord pour le redimensionner.

- Maintenez la touche **Alt** enfoncée pendant que vous faites glisser pour aligner un bord sur le bord d'une cellule.
- Maintenez la touche **Maj** enfoncée pendant que vous faites glisser un coin pour conserver les proportions.
- Les touches fléchées le déplacent de quelques pixels à la fois. **Ctrl** avec une flèche le déplace d'un pixel, **Maj** avec une flèche l'agrandit ou le réduit, et **Alt** avec une flèche le place sur le bord suivant d'une cellule.
- **Échap** pendant un glissement le remet à sa place d'origine.

Chaque déplacement ou redimensionnement constitue une étape que vous pouvez annuler. Faites un clic droit sur l'objet et choisissez **Propriétés** pour saisir son **Nom** (chaque objet d'une feuille doit avoir le sien) et son **Texte de remplacement** (une courte phrase qui indique à une personne ne pouvant pas voir l'objet ce qu'il représente), pour saisir sa taille, et pour choisir son comportement lorsque vous insérez, supprimez ou redimensionnez des cellules : se déplacer et se redimensionner avec les cellules, se déplacer avec elles tout en conservant sa taille, ou rester en place. Ce choix est enregistré dans le fichier. **Ne pas imprimer l'objet**, dans la même boîte, masque l'objet sur les pages imprimées (il reste affiché à l'écran) ; le fichier conserve aussi ce choix.

## Modifier ce qu'affiche un graphique

Lorsqu'un graphique est sélectionné, deux boutons apparaissent à côté de lui et un onglet **Graphique** s'affiche dans le ruban.

- Le bouton **+** (et **Éléments du graphique** dans l'onglet) active ou désactive le titre du graphique, les axes, les titres des axes, le quadrillage, la légende et les étiquettes de données, et indique où placer la légende et les étiquettes.
- Le bouton pinceau (et **Couleurs** et **Styles** dans l'onglet) applique au graphique l'un des jeux de couleurs et l'un des styles de JeffCrunch. **Dispositions** règle plusieurs éléments à la fois.
- **Données du graphique** affiche la liste des séries du graphique. Vous pouvez ajouter, modifier, supprimer et réordonner les séries, changer les cellules que chacune lit, définir les cellules contenant les étiquettes, intervertir les lignes et les colonnes, et choisir la manière de représenter les cellules vides et les lignes et colonnes masquées.
- **Courbe de tendance** (dans le menu **+**) trace une droite ajustée aux nombres de chaque série, afin de voir où ils se dirigent. Choisissez **Linéaire** pour une droite, **Exponentielle** pour des nombres qui augmentent d'un pourcentage constant, **Logarithmique** pour des nombres qui augmentent rapidement puis se stabilisent, **Polynomiale** pour une courbe avec des inflexions, **Puissance**, ou **Moyenne mobile**, qui trace la moyenne des derniers nombres. **Afficher l'équation** inscrit la formule de la courbe sur le graphique, et **Afficher le R²** indique la qualité de l'ajustement (1 correspond à un ajustement parfait ; 0, à aucun ajustement). La courbe de tendance reçoit sa propre entrée dans la légende.
- Lorsqu'une série est tracée par rapport à un second axe vertical, les menus **Axes** et **Titres des axes** proposent les entrées **Second horizontal** et **Second vertical** pour elle. Le menu **Légende** propose **Au-dessus du graphique**, qui place la légende sur la zone de tracé au lieu de la placer à côté.
- **Étiquettes de données** propose aussi **Contenu de l'étiquette** (la valeur, le nom de la catégorie, le nom de la série, le pourcentage et le repère de légende, c'est-à-dire un petit carré de la couleur de la série, dans n'importe quelle combinaison), **Séparateur** (ce qui les sépare : virgule, point-virgule, point, saut de ligne ou espace) et **Format de nombre…** (un code de format tel que 0,0 pour les nombres des étiquettes ; laissez ce champ vide pour afficher les nombres tels quels). Si vous désactivez tous les contenus, les étiquettes disparaissent.
- **Format de l'axe horizontal…** et **Format de l'axe vertical…** dans l'onglet **Graphique** ouvrent une boîte pour un axe. Elle définit les valeurs minimale et maximale ainsi que les unités principale et secondaire (chacune automatique ou un nombre que vous saisissez), indique si les valeurs sont dans l'ordre inverse, si l'échelle est logarithmique (chaque graduation est une base multipliée par la précédente, par exemple 1, 10, 100 et 1000, ce qui convient aux nombres de tailles très différentes, à condition qu'ils soient tous supérieurs à 0), l'endroit où l'autre axe le coupe, le format de nombre de ses étiquettes, ses graduations, la position de ses étiquettes et leur angle de rotation. Seul un axe numérique possède des bornes et des unités.
- **Déplacer le graphique…** dans l'onglet **Graphique** demande une autre feuille de calcul et y place le graphique, aux mêmes cellules. Il continue de lire les cellules qu'il lisait, et conserve sa taille et sa place sur la feuille, même lorsque les lignes et les colonnes sont plus hautes ou plus larges, et il se déplace et se redimensionne avec les cellules comme auparavant. Les feuilles masquées ne sont pas proposées. Cette opération constitue une seule étape d'**Annuler**, même après un enregistrement. Une feuille de graphique (feuille qui ne contient qu'un graphique) ne peut pas encore être créée, et un graphique qui s'y trouve ne peut pas être déplacé.
- **Titre du graphique** et **Titres des axes** (dans le menu **+**) proposent aussi **Texte du titre…**. Saisissez le texte souhaité. Ou saisissez un signe égal suivi d'une cellule, par exemple **=Feuil1!A1**, et le titre affiche le contenu de cette cellule. Lorsque la cellule change, le titre change aussi. Le fichier conserve le lien.
- **Enregistrer comme image…** dans l'onglet **Graphique** (et dans le menu contextuel) enregistre le graphique sélectionné dans un fichier image **PNG** ou **JPEG**, deux fois plus grand que sur la feuille afin de rester net.
- **Enregistrer comme modèle…** et **Appliquer le modèle…** (dans l'onglet **Graphique**, dans le groupe **Type**) conservent l'apparence d'un graphique dans un fichier **.crtx** et la transmettent à un autre graphique. L'apparence comprend le type, les couleurs, les réglages du titre et de la légende, les axes et les polices. Le graphique auquel vous appliquez le modèle conserve ses propres nombres et les cellules qu'il lit. Une étape **Annuler** supprime à nouveau cette apparence.
- **Type de graphique** modifie le type de l'ensemble du graphique, ou de chaque série : colonnes avec une courbe par-dessus, ou série sur un second axe vertical.

Une référence à des cellules s'écrit par exemple **Feuil1!$B$2:$B$9**, le nom de la feuille étant placé entre apostrophes lorsqu'il contient des espaces.

## Sparklines

Une sparkline est un mini graphique qui tient dans une seule cellule. Elle montre la tendance d'une ligne ou d'une colonne de nombres, directement à côté d'eux.

1. Sélectionnez les nombres. Pour une sparkline par ligne, sélectionnez plusieurs lignes de nombres.
2. Dans l'onglet **Insertion**, dans le groupe **Sparklines**, choisissez **Courbe**, **Histogramme** ou **Gain/perte**. Une **courbe** relie les nombres par un trait. Un **histogramme** dessine une barre pour chaque nombre. **Gain/perte** dessine une barre vers le haut pour chaque nombre supérieur à zéro et une barre vers le bas pour chaque nombre inférieur à zéro, toutes de la même taille, ce qui convient aux résultats qui ne sont que bons ou mauvais.
3. Dans la boîte **Créer des sparklines**, vérifiez la **Plage de données** (les cellules que lisent les sparklines) et saisissez la **Plage d'emplacement** (les cellules où elles sont placées). Choisissez une cellule d'emplacement pour chaque ligne de données, ou pour chaque colonne de données. Si vous choisissez une seule cellule, la sparkline lit toutes les données.

Les sparklines créées en une seule opération forment un groupe et partagent le même style. Cliquez sur une cellule contenant une sparkline : l'onglet **Sparkline** apparaît.

- **Modifier les données** modifie les données et l'emplacement de tout le groupe, ou les données de celle sur laquelle vous avez cliqué. Elle indique aussi quoi faire des cellules vides (laisser un espace, les traiter comme zéro ou relier les points de part et d'autre) et si les cellules des lignes et colonnes masquées doivent être lues.
- **Type** permet de passer de la courbe à l'histogramme ou à Gain/perte.
- **Afficher** met en évidence des points : le **Point haut**, le **Point bas**, les **Points négatifs**, le **Premier point**, le **Dernier point» et, pour une courbe, les **Marqueurs** sur chaque point.
- **Styles** applique au groupe un jeu de couleurs. **Couleur de la sparkline** définit la couleur de la courbe ou des barres, ainsi que l'**Épaisseur** (l'épaisseur du trait, en points). **Couleur du marqueur** définit la couleur de chaque type de point mis en évidence.
- **Axe** affiche un trait à zéro, trace les nombres de droite à gauche et définit les valeurs minimale et maximale de l'axe vertical. **Automatique pour chaque sparkline** ajuste chacune à ses propres nombres. **Identique pour toutes les sparklines** donne au groupe entier une même échelle, pour pouvoir les comparer. **Valeur personnalisée** vous permet de saisir un nombre. **Axe des dates…** espace les points selon leurs dates : saisissez les cellules qui contiennent la date de chaque point, et un point dont la date est plus récente se place plus loin. Laissez ce champ vide pour répartir de nouveau les points de façon régulière.
- **Grouper** place les sparklines sélectionnées dans un même groupe. **Dissocier** donne à chacune son propre style. **Effacer** supprime les sparklines sélectionnées, ou leurs groupes entiers.

Une sparkline suit ses cellules : si vous insérez une ligne au-dessus des données, elle lit toujours les mêmes nombres. Chaque modification constitue une étape que vous pouvez annuler. Les sparklines sont enregistrées dans le fichier sous une forme que lisent les autres logiciels de feuilles de calcul.

## Couper, copier et coller un objet

Une image ou une forme se copie et se colle de la même manière qu'un graphique. Une forme qui utilise autre chose qu'une image du classeur, comme un lien vers un fichier, ne peut pas être copiée, et JeffCrunch vous en indique la raison.

Copiez un graphique et collez-le dans le même classeur, sur n'importe quelle feuille : la copie lit les mêmes cellules que l'original. Si vous le collez dans un autre classeur, il conserve les nombres affichés mais ne lit plus de cellules, car les cellules qu'il lirait dans ce classeur ne sont pas celles à partir desquelles il a été créé.

## Nommer un objet

Lorsqu'un objet est sélectionné, la zone Nom affiche son nom. Saisissez-y un nouveau nom et appuyez sur **Entrée**. **Annuler** rétablit l'ancien nom, et le nouveau nom est enregistré dans le fichier.

## Supprimer un objet

Appuyez sur **Suppr** lorsque l'objet est sélectionné. **Annuler** le restaure.

## Ce qui est enregistré

Tout ce qui précède est enregistré dans le fichier sous une forme que lisent les autres logiciels de feuilles de calcul, de sorte qu'un graphique créé ici s'ouvre de la même manière ailleurs. Un graphique provenant d'un autre logiciel et contenant des éléments que JeffCrunch ne peut pas conserver lorsqu'il réécrit un graphique (barres d'erreur, graphique en trois dimensions) peut tout de même être déplacé, redimensionné et supprimé, mais ses éléments et ses données ne sont pas modifiés ici.

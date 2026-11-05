# Graphiques, sparklines, images et formes
<!-- keywords: filtres de graphique, masquer une série, graphiques recommandés, suggestions de graphique, graphique, diagramme, colonne, barre, courbe, secteurs, aires, nuage de points, combiné, légende, titre, quadrillage, étiquettes de données, données du graphique, type de graphique, apparence, dispositions, déplacer, redimensionner, image, forme, objet, série, courbe de tendance, tendance, équation, R², second axe, sparkline, sparklines, axe des dates, gain/perte, mini-graphique, marqueurs, nom, texte de remplacement, échelle logarithmique, enregistrer comme image, modèle, crtx, texte du titre, repère de légende, volet Sélection, remplissage de forme, contour de forme, pivoter, retourner, rogner, modifier le texte, bordure de l'image, insérer une image, insérer une forme, zone de texte, formes, flèche, ovale, rectangle, étoile -->

Un graphique est une représentation de nombres qui reste liée à ces nombres : si vous modifiez un nombre, le graphique change avec lui. Les images et les formes se placent au-dessus des cellules de la même façon, mais elles ne sont liées à aucun nombre.

## Créer un graphique

1. Sélectionnez les nombres à représenter. Si la première ligne contient des noms et la première colonne des étiquettes, sélectionnez-les aussi. JeffCrunch examine la première ligne et la première colonne et les utilise comme noms et comme étiquettes lorsqu'il s'agit de mots (ou de dates, pour les étiquettes).
2. Dans l'onglet **Insertion**, dans le groupe **Graphiques**, choisissez un type : **Colonne**, **Barre**, **Courbe**, **Secteurs**, **Aires**, **Nuage de points** ou **Combiné**. La petite flèche à côté d'un type donne ses variantes, comme les colonnes empilées, une courbe avec marqueurs, ou un nuage de points avec des points, des points reliés ou des courbes lissées.
3. Pour voir d'abord tous les types, choisissez **Autres graphiques**. La liste à gauche affiche chaque type tracé à partir de vos cellules. **Intervertir les lignes et les colonnes** inverse le sens des séries.

Vous ne savez pas quel type choisir ? Sélectionnez vos cellules et choisissez **Graphiques recommandés** dans l'onglet **Insertion**. JeffCrunch examine les cellules et propose quelques types adaptés, du plus pertinent au moins pertinent. Par exemple, des dates en colonne suggèrent une courbe, et une seule colonne de quelques nombres positifs accompagnés d'étiquettes suggère des secteurs. Chaque type est tracé à partir de vos cellules. Choisissez-en un, puis cliquez sur **OK**.

Si vous ne sélectionnez qu'une seule cellule, JeffCrunch représente le bloc de cellules qui l'entoure et qui contient des données.

Les séries sont disposées en colonnes lorsque la sélection compte au moins autant de lignes que de colonnes, et en lignes lorsqu'elle compte plus de colonnes que de lignes. Un graphique en secteurs ne contient qu'une série. Un nuage de points prend ses valeurs horizontales dans la première colonne.

Exemple : vous sélectionnez A1:C4, où A2:A4 contiennent T1, T2 et T3, B1 et C1 contiennent Est et Ouest, et le reste sont des nombres. Le graphique comporte deux séries, Est et Ouest, et trois groupes de colonnes, T1 à T3.

## Masquer une série temporairement

Sélectionnez un graphique. Trois petits boutons apparaissent à côté. Celui qui représente un entonnoir est **Filtres de graphique**. Il affiche les séries du graphique, chacune avec une case à cocher. Décochez une case pour retirer cette série du graphique. Cochez-la de nouveau pour la faire réapparaître à la même place. **Annuler** fonctionne aussi. Un graphique conserve toujours au moins une série. Une série que vous avez retirée n'est pas conservée lorsque vous enregistrez et fermez le fichier : cochez-la de nouveau au préalable si vous voulez la garder.

## Déplacer et redimensionner

Cliquez sur un graphique, une image ou une forme pour la sélectionner. Faites-la glisser pour la déplacer, ou faites glisser l'un des huit carrés de son bord pour la redimensionner.

- Maintenez **Alt** enfoncée pendant le glissement pour aligner un bord sur le bord d'une cellule.
- Maintenez **Maj** enfoncée pendant que vous faites glisser un coin pour conserver les proportions.
- Les touches fléchées déplacent l'objet de quelques pixels à la fois. **Ctrl** avec une flèche le déplace d'un pixel, **Maj** avec une flèche l'agrandit ou le réduit, et **Alt** avec une flèche le place jusqu'au bord suivant d'une cellule.
- **Échap** pendant le glissement remet l'objet en place.

Chaque déplacement ou redimensionnement constitue une étape que vous pouvez annuler. Faites un clic droit sur l'objet et choisissez **Propriétés** pour saisir son **Nom** (chaque objet d'une feuille doit avoir le sien) et son **Texte de remplacement** (une courte phrase décrivant l'objet pour une personne qui ne peut pas le voir), pour saisir sa taille, et pour choisir son comportement lorsque vous insérez, supprimez ou redimensionnez des cellules : se déplacer et se redimensionner avec les cellules, se déplacer avec elles mais garder sa taille, ou rester en place. Ce choix est enregistré dans le fichier. **Ne pas imprimer l'objet**, dans la même boîte, masque l'objet sur les pages imprimées (il reste affiché à l'écran) ; le fichier conserve aussi ce choix.

## Modifier ce qu'affiche un graphique

Lorsqu'un graphique est sélectionné, deux boutons apparaissent à côté et un onglet **Graphique** apparaît sur le ruban.

- Le bouton **+** (et **Éléments du graphique** dans l'onglet) active ou désactive le titre du graphique, les axes, les titres des axes, le quadrillage, la légende et les étiquettes de données, et indique où placer la légende et les étiquettes.
- Le bouton en forme de pinceau (et **Couleurs** et **Apparence** dans l'onglet) donne au graphique l'un des jeux de couleurs et l'une des apparences de JeffCrunch. **Dispositions** règle plusieurs éléments à la fois.
- **Données du graphique** affiche les séries du graphique. Vous pouvez en ajouter, les modifier, les supprimer et les réordonner, changer les cellules que chacune lit, définir les cellules contenant les étiquettes, intervertir les lignes et les colonnes, et choisir la manière dont les cellules vides et les lignes et colonnes masquées sont représentées.
- **Courbe de tendance** (dans le menu **+**) trace une droite ajustée aux nombres de chaque série, pour voir vers où ils tendent. Choisissez **Linéaire** pour une droite, **Exponentielle** pour des nombres qui augmentent d'un pourcentage constant, **Logarithmique** pour des nombres qui augmentent rapidement puis se stabilisent, **Polynomiale** pour une courbe avec des inflexions, **Puissance**, ou **Moyenne mobile**, qui trace la moyenne des derniers nombres. **Afficher l'équation** écrit la formule de la droite sur le graphique, et **Afficher le R²** indique la qualité de l'ajustement (1 correspond à un ajustement parfait, 0 à aucun ajustement). La courbe de tendance a sa propre entrée dans la légende.
- Lorsqu'une série est tracée sur un second axe vertical, les menus **Axes** et **Titres des axes** proposent les entrées **Second horizontal** et **Second vertical** pour elle. Le menu **Légende** propose **Au-dessus du graphique**, qui place la légende sur la zone de tracé au lieu de la placer à côté.
- **Étiquettes de données** propose aussi **Contenu de l'étiquette** (la valeur, le nom de la catégorie, le nom de la série, le pourcentage et le repère de légende, c'est-à-dire un petit carré de la couleur de la série, combinables à volonté), **Séparateur** (ce qui les sépare : virgule, point-virgule, point, saut de ligne ou espace) et **Format de nombre…** (un code de format comme 0,0 pour les nombres des étiquettes ; laissez-le vide pour afficher les nombres tels quels). Si vous désactivez tous les contenus, les étiquettes disparaissent.
- **Mettre en forme l'axe horizontal…** et **Mettre en forme l'axe vertical…** dans l'onglet **Graphique** ouvrent une boîte pour un seul axe. Elle permet de définir les valeurs minimale et maximale, ainsi que les unités principale et secondaire (chacune automatique ou un nombre que vous saisissez), d'inverser l'ordre des valeurs, d'activer l'échelle logarithmique (chaque graduation vaut la base multipliée par la précédente, par exemple 1, 10, 100 et 1 000, ce qui convient aux nombres de tailles très différentes et qui doivent tous être supérieurs à 0), de choisir l'endroit où l'autre axe le croise, le format des nombres de ses étiquettes, ses graduations, la position de ses étiquettes et leur angle de rotation. Seul un axe de nombres a des bornes et des unités.
- **Déplacer le graphique…** dans l'onglet **Graphique** demande une autre feuille de calcul et y place le graphique, aux mêmes cellules. Il lit toujours les cellules qu'il lisait et conserve sa taille et sa place sur la feuille, même lorsque les lignes et les colonnes sont plus hautes ou plus larges, et il se déplace et se redimensionne avec les cellules comme auparavant. Les feuilles masquées ne sont pas proposées. Cette opération constitue une seule étape d'**Annuler**, même après un enregistrement. Une feuille de graphique (feuille qui ne contient qu'un graphique) ne peut pas encore être créée, et un graphique situé sur une telle feuille ne peut pas être déplacé.
- **Titre du graphique** et **Titres des axes** (dans le menu **+**) proposent aussi **Texte du titre…**. Saisissez le texte souhaité. Ou saisissez un signe égal suivi d'une cellule, par exemple **=Feuil1!A1**, et le titre affiche le contenu de cette cellule. Lorsque la cellule change, le titre change aussi. Le fichier conserve le lien.
- **Enregistrer comme image…** dans l'onglet **Graphique** (et dans le menu contextuel) enregistre le graphique sélectionné dans un fichier image **PNG** ou **JPEG**, deux fois plus grand que sur la feuille afin de rester net.
- **Enregistrer comme modèle…** et **Appliquer le modèle…** (dans l'onglet **Graphique**, groupe **Type**) conservent l'apparence d'un graphique dans un fichier **.crtx** et la transmettent à un autre graphique. L'apparence comprend le type, les couleurs, les paramètres du titre et de la légende, les axes et les polices. Le graphique auquel vous appliquez le modèle conserve ses propres nombres et les cellules qu'il lit. Une seule étape d'**Annuler** retire l'apparence.
- **Modifier le type de graphique** change le type de l'ensemble du graphique, ou celui de chaque série : colonnes avec une courbe par-dessus, ou une série sur un second axe vertical.

Une référence à des cellules s'écrit par exemple **Feuil1!$B$2:$B$9**, le nom de la feuille étant placé entre apostrophes lorsqu'il contient des espaces.

## Sparklines

Une sparkline est un mini-graphique qui tient dans une seule cellule. Elle montre la tendance d'une ligne ou d'une colonne de nombres, juste à côté de ceux-ci.

1. Sélectionnez les nombres. Pour obtenir une sparkline par ligne, sélectionnez plusieurs lignes de nombres.
2. Dans l'onglet **Insertion**, dans le groupe **Sparklines**, choisissez **Courbe**, **Histogramme** ou **Gain/perte**. Une **Courbe** relie les nombres par un trait. Un **Histogramme** trace une barre pour chaque nombre. **Gain/perte** trace une barre vers le haut pour chaque nombre supérieur à zéro et une barre vers le bas pour chaque nombre inférieur à zéro, toutes de même hauteur, ce qui convient aux résultats qui ne sont que bons ou mauvais.
3. Dans la boîte **Créer des sparklines**, vérifiez la **Plage de données** (les cellules que lisent les sparklines) et saisissez la **Plage d'emplacement** (les cellules qui les accueillent). Choisissez une cellule d'emplacement pour chaque ligne de données, ou pour chaque colonne de données. Si vous choisissez une seule cellule, la sparkline lit l'ensemble des données.

Les sparklines créées en une seule étape forment un groupe, et elles partagent une même apparence. Cliquez sur une cellule contenant une sparkline pour faire apparaître l'onglet **Sparkline**.

- **Modifier les données** change les données et l'emplacement de l'ensemble du groupe, ou les données de la seule sparkline sélectionnée. Elle indique aussi quoi faire des cellules vides (laisser un intervalle, les traiter comme zéro, ou relier les points de part et d'autre) et si les cellules des lignes et colonnes masquées doivent être lues.
- **Type** permet de passer de la courbe à l'histogramme ou à gain/perte.
- **Afficher** met en évidence des points : le **Point haut**, le **Point bas**, les **Points négatifs**, le **Premier point**, le **Dernier point» et, pour une courbe, les **Marqueurs** sur chaque point.
- **Styles** donne au groupe un jeu de couleurs. **Couleur de sparkline** définit la couleur de la ligne ou des barres, et l'**Épaisseur** (l'épaisseur du trait, en points). **Couleur du marqueur** définit la couleur de chaque type de point mis en évidence.
- **Axe** affiche un trait à zéro, trace les nombres de droite à gauche et définit les valeurs minimale et maximale de l'axe vertical. **Automatique pour chaque sparkline** adapte chacune à ses propres nombres. **Identique pour toutes les sparklines** donne au groupe une échelle unique, pour pouvoir les comparer. **Valeur personnalisée** permet de saisir un nombre. **Axe des dates…** espace les points selon leurs dates : saisissez les cellules qui contiennent la date de chaque point, et un point dont la date est plus tardive se place plus loin. Laissez le champ vide pour répartir de nouveau les points de façon régulière.
- **Grouper** place les sparklines sélectionnées dans un même groupe. **Dissocier** donne à chacune sa propre apparence. **Effacer** supprime les sparklines sélectionnées, ou les groupes entiers auxquels elles appartiennent.

Une sparkline suit ses cellules : si vous insérez une ligne au-dessus des données, elle lit toujours les mêmes nombres. Chaque modification constitue une étape que vous pouvez annuler. Les sparklines sont enregistrées dans le fichier sous une forme que les autres tableurs peuvent lire.

## Insérer une image, une forme ou une zone de texte

Dans l'onglet **Insertion**, dans le groupe **Illustrations**, **Image** ouvre un sélecteur de fichiers. Choisissez un fichier image (PNG, JPEG, GIF, BMP, TIFF ou SVG) : il est placé sur la feuille, dans la cellule sélectionnée. Une image dont la largeur dépasse 480 pixels est réduite pour tenir, en conservant ses proportions. Le classeur conserve l'image elle-même. Une image d'un type que le classeur ne peut pas conserver tel quel (un SVG, par exemple) est enregistrée au format PNG, une image ordinaire composée de points. Un fichier qui n'est pas une image, ou dont la taille dépasse 16 Mo, est refusé avec un message.

**Formes** ouvre une liste : rectangle, rectangle à coins arrondis, ovale, triangle, losange, flèches, étoile, cœur, nuage, bulle de dialogue et trait. Choisissez-en une : elle est placée sur la feuille en bleu clair, avec un contour plus foncé. **Zone de texte**, dans le groupe **Texte**, insère une zone vide, sans remplissage ni contour.

Chacune de ces opérations constitue une étape unique que vous pouvez annuler. Le nouvel objet est sélectionné : vous pouvez le faire glisser, le redimensionner, le faire passer devant ou derrière les autres, lui donner un nom et un texte de remplacement dans **Propriétés**, ou appuyer sur **Suppr**.

## Modifier l'apparence d'une image, d'une forme ou d'une zone de texte

Lorsque vous sélectionnez une image, l'onglet **Format de l'image** apparaît. Lorsque vous sélectionnez une forme, un trait ou une zone de texte, l'onglet **Format de la forme** apparaît. Chaque modification constitue une étape que vous pouvez annuler, et l'objet conserve son nom, son texte de remplacement, sa place et son ordre parmi les autres objets.

- **Remplissage de forme** et **Contour de forme** choisissent une couleur, ou aucune. **Style de trait** définit l'épaisseur (l'épaisseur du trait) et les tirets (continu, en tirets ou pointillé). Une image dispose de **Bordure de l'image**, qui est son contour.
- **Pivoter** fait tourner l'objet d'un quart de tour vers la droite ou vers la gauche, ou le retourne horizontalement ou verticalement.
- **Rogner**, pour une image, la découpe selon une forme de fenêtre : carré, 4 de large sur 3 de haut, 3 de large sur 4 de haut, 16 sur 9, ou 9 sur 16. La découpe est faite également à partir du centre de la partie visible, et l'image est agrandie autant que possible à partir de ce qui reste. **Afficher l'image entière** rétablit ce qui a été coupé. Le fichier de l'image n'est pas modifié : la partie rognée est seulement masquée.
- **Modifier le texte** concerne les formes et les zones de texte. Saisissez le texte, puis choisissez son alignement horizontal et vertical, sa taille, le gras ou le normal, et s'il passe à la ligne dans la forme ou reste sur une seule ligne.
- **Volet Sélection** ouvre la liste de tous les objets de la feuille par leur nom, celui qui est au premier plan en premier. Cliquez sur un nom pour sélectionner cet objet, ce qui est utile lorsque des objets se masquent les uns les autres. **Avancer** et **Reculer** le déplacent dans la liste.
- **Propriétés** définit la taille, le nom, le texte de remplacement et le comportement de l'objet lorsque les cellules sont déplacées ou agrandies.

## Couper, copier et coller un objet

Une image ou une forme se copie et se colle comme un graphique. Une forme qui utilise un élément autre qu'une image du classeur, comme un lien vers un fichier, ne peut pas être copiée, et JeffCrunch vous indique pourquoi.

Copiez un graphique et collez-le dans le même classeur, sur n'importe quelle feuille : la copie lit les mêmes cellules que l'original. Collé dans un autre classeur, il conserve les nombres qu'il affichait mais ne lit plus de cellules, car les cellules qu'il lirait dans ce classeur ne sont pas celles à partir desquelles il a été créé.

## Nommer un objet

Lorsqu'un objet est sélectionné, la zone Nom affiche son nom. Saisissez-y un nouveau nom et appuyez sur **Entrée**. **Annuler** rétablit l'ancien nom, et le nouveau nom est enregistré dans le fichier.

## Supprimer un objet

Appuyez sur **Suppr** lorsque l'objet est sélectionné. **Annuler** le rétablit.

## Ce qui est enregistré

Tout ce qui précède est enregistré dans le fichier sous une forme que les autres tableurs peuvent lire, de sorte qu'un graphique créé ici s'ouvre de la même façon ailleurs. Un graphique provenant d'un autre logiciel et contenant un élément que JeffCrunch ne peut pas conserver lorsqu'il réécrit un graphique (barres d'erreur, graphique en trois dimensions) peut tout de même être déplacé, redimensionné et supprimé, mais ses éléments et ses données ne sont pas modifiés ici.

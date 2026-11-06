# Graphiques, sparklines, images et formes

<!-- keywords: grouper, dissocier, aligner, aligner à gauche, répartir, sélectionner des objets, plusieurs formes, sélectionner plusieurs, cadre de sélection, filtres de graphique, masquer une série, graphiques recommandés, suggestions de graphiques, graphique, diagramme, colonne, barre, courbe, secteurs, aires, nuage de points, combiné, légende, titre, quadrillage, étiquettes de données, données du graphique, type de graphique, apparence, dispositions, déplacer, redimensionner, image, forme, objet, série, courbe de tendance, tendance, équation, R², deuxième axe, sparkline, sparklines, axe des dates, gain/perte, mini-graphique, marqueurs, nom, texte de remplacement, échelle logarithmique, enregistrer comme image, modèle, crtx, texte du titre, repère de légende, volet Sélection, remplissage de forme, contour de forme, pivoter, retourner, rogner, modifier le texte, bordure d'image, insérer une image, icônes, icône, wmf, emf, styles de forme, styles, insérer une forme, zone de texte, formes, flèche, ovale, rectangle, étoile -->

Un graphique est une représentation de nombres qui reste liée à eux : si vous modifiez un nombre, le graphique change en conséquence. Les images et les formes se placent au-dessus des cellules de la même façon, mais elles ne sont liées à aucun nombre.

## Créer un graphique

1. Sélectionnez les nombres à représenter. Si la première ligne contient des noms et la première colonne des étiquettes, sélectionnez-les aussi. JeffCrunch examine la première ligne et la première colonne et les utilise comme noms et comme étiquettes lorsqu'il s'agit de mots (ou de dates, pour les étiquettes).
2. Dans l'onglet **Insertion**, dans le groupe **Graphiques**, choisissez un type : **Histogramme**, **Barres**, **Courbes**, **Secteurs**, **Aires**, **Nuage de points** ou **Graphique combiné**. La petite flèche à côté d'un type donne ses variantes, comme les colonnes empilées, une courbe avec marqueurs, ou un nuage de points, de points reliés ou de courbes lissées.
3. Pour voir d'abord tous les types, choisissez **Autres graphiques**. La liste de gauche affiche chaque type tracé à partir de vos cellules. **Intervertir les lignes et les colonnes** inverse le sens des séries.

Vous ne savez pas quel type choisir ? Sélectionnez vos cellules et choisissez **Graphiques recommandés** dans l'onglet **Insertion**. JeffCrunch examine les cellules et propose quelques types qui leur conviennent, du plus adapté au moins adapté. Par exemple, des dates en colonne suggèrent une courbe, et une seule colonne de quelques nombres positifs avec des étiquettes suggère des secteurs. Chaque type est tracé à partir de vos cellules. Choisissez-en un, puis cliquez sur **OK**.

Si vous ne sélectionnez qu'une seule cellule, JeffCrunch représente le bloc de cellules qui l'entoure et qui contient des données.

Les séries sont disposées en colonnes lorsque la sélection compte au moins autant de lignes que de colonnes, et en lignes lorsqu'elle compte plus de colonnes que de lignes. Un graphique en secteurs ne comporte qu'une série. Un nuage de points prend ses valeurs horizontales dans la première colonne.

Exemple : vous sélectionnez A1:C4, où A2:A4 contiennent T1, T2 et T3, B1 et C1 contiennent Est et Ouest, et le reste sont des nombres. Le graphique comporte deux séries, Est et Ouest, et trois groupes de colonnes, de T1 à T3.

## Masquer une série provisoirement

Sélectionnez un graphique. Trois petits boutons apparaissent à côté. Celui qui représente un entonnoir correspond aux **Filtres de graphique**. Il affiche la liste des séries du graphique, chacune accompagnée d'une case à cocher. Décochez une case pour retirer la série du graphique. Cochez-la de nouveau pour la faire réapparaître au même endroit. **Annuler** fonctionne également. Un graphique conserve toujours au moins une série. Une série que vous avez retirée reste dans le fichier lorsque vous l'enregistrez, et elle reste masquée à l'ouverture du fichier : rien n'est perdu. Cochez-la de nouveau pour la faire réapparaître. (Un graphique en anneau affiche toutes les séries à son ouverture.)

## Déplacer et redimensionner

Cliquez sur un graphique, une image ou une forme pour le sélectionner. Faites-le glisser pour le déplacer, ou faites glisser l'un des huit carrés de son contour pour le redimensionner.

- Maintenez **Alt** enfoncée pendant le glissement pour aligner un bord sur le bord d'une cellule.
- Maintenez **Maj** enfoncée pendant que vous faites glisser un coin pour conserver les proportions.
- Les touches fléchées le déplacent de quelques pixels à la fois. **Ctrl** avec une flèche le déplace d'un pixel, **Maj** avec une flèche l'agrandit ou le réduit, et **Alt** avec une flèche va jusqu'au bord suivant d'une cellule.
- **Échap** pendant le glissement le remet à sa place.

Chaque déplacement ou redimensionnement constitue une étape que vous pouvez annuler. Faites un clic droit sur l'objet et choisissez **Propriétés** pour saisir son **Nom** (chaque objet d'une feuille doit en avoir un qui lui est propre) et son **Texte de remplacement** (une courte phrase qui indique à une personne qui ne peut pas voir l'objet ce qu'il représente), pour saisir sa taille, et pour choisir son comportement lorsque vous insérez, supprimez ou redimensionnez des cellules : se déplacer et se redimensionner avec les cellules, se déplacer avec elles mais garder sa taille, ou rester en place. Ce choix est enregistré dans le fichier. **Imprimer cet objet**, dans la même boîte, désactive l'objet sur les pages imprimées (il reste visible à l'écran) ; le fichier conserve aussi ce choix.

## Modifier ce qu'affiche un graphique

Lorsqu'un graphique est sélectionné, deux boutons apparaissent à côté et un onglet **Création de graphique** s'affiche dans le ruban.

- Le bouton **+** (et **Éléments du graphique** dans l'onglet) active ou désactive le titre du graphique, les axes, les titres des axes, le quadrillage, la légende et les étiquettes de données, et indique où placer la légende et les étiquettes.
- Le bouton pinceau (et **Couleurs** et **Styles** dans l'onglet) applique au graphique l'un des jeux de couleurs et l'un des styles de JeffCrunch. **Dispositions** règle plusieurs éléments à la fois.
- **Données du graphique** affiche la liste des séries du graphique. Vous pouvez en ajouter, en modifier, en supprimer et en réordonner, changer les cellules que chacune lit, définir les cellules contenant les étiquettes, intervertir les lignes et les colonnes, et choisir la façon dont les cellules vides et les lignes et colonnes masquées sont représentées.
- **Barres d'erreur** (dans le menu **+**, pour un histogramme, une barre, une courbe ou un nuage de points) trace un petit trait au-dessus et au-dessous de chaque point pour montrer l'écart possible avec la valeur réelle. Choisissez **Erreur type**, **Pourcentage (5 %)**, **Écart type** ou **Valeur fixe (1)**. Un fichier qui contient ses propres montants pour chaque point les conserve.
- **Traits** (dans le menu **+**, pour un graphique en courbes) propose **Lignes de chute**, qui descendent du point le plus haut de chaque catégorie jusqu'à l'axe, **Lignes haut-bas**, qui relient le point le plus haut et le point le plus bas de chaque catégorie, et **Barres haut/bas**, qui sont des rectangles entre la première et la dernière série de chaque catégorie. Une barre montante indique que la dernière série est plus élevée. Les lignes haut-bas et les barres haut/bas exigent deux séries ou plus. Les graphiques en aires disposent des lignes de chute.
- **Courbe de tendance** (dans le menu **+**) trace une droite ajustée aux nombres de chaque série, pour que vous voyiez vers où ils tendent. Choisissez **Linéaire** pour une droite, **Exponentielle** pour des nombres qui croissent d'un pourcentage constant, **Logarithmique** pour des nombres qui augmentent vite puis se stabilisent, **Polynomiale** pour une courbe à plusieurs inflexions, **Puissance**, ou **Moyenne mobile**, qui trace la moyenne des derniers nombres. **Afficher l'équation** inscrit la formule de la droite sur le graphique, et **Afficher le R²** indique la qualité de l'ajustement (1 correspond à un ajustement parfait ; 0, à aucun ajustement). La courbe de tendance reçoit sa propre entrée dans la légende.
- Lorsqu'une série est tracée sur un second axe vertical, les menus **Axes** et **Titres des axes** proposent les entrées **Horizontal secondaire** et **Vertical secondaire** pour elle. Le menu **Légende** propose **Superposée**, qui place la légende sur la zone du graphique au lieu de la placer à côté.
- **Étiquettes de données** propose aussi **Contenu de l'étiquette** (la valeur, le nom de la catégorie, le nom de la série, le pourcentage et le repère de légende, c'est-à-dire un petit carré de la couleur de la série, dans n'importe quelle combinaison), **Séparateur** (ce qui les sépare : virgule, point-virgule, point, saut de ligne ou espace) et **Format de nombre…** (un code de format, comme 0,0, pour les nombres des étiquettes ; laissez-le vide pour afficher les nombres tels quels). **Valeur à partir des cellules…** affiche le texte de certaines cellules comme étiquettes, une cellule par point. Saisissez les cellules, par exemple =Feuil1!$D$2:$D$4. Les étiquettes changent lorsque les cellules changent. Laissez le champ vide pour les supprimer. Si vous désactivez tous les éléments de contenu, les étiquettes disparaissent.
- **Mettre en forme l'axe horizontal…** et **Mettre en forme l'axe vertical…** dans l'onglet **Création de graphique** ouvrent une boîte pour un seul axe. Elle permet de régler les valeurs minimale et maximale, ainsi que les unités principale et secondaire (chacune automatique ou un nombre que vous saisissez), d'inverser l'ordre des valeurs, de définir les unités d'affichage (les étiquettes affichent les nombres divisés, par exemple par 1 000 pour des milliers, de sorte que 30 000 s'affiche 30 ; cochez **Afficher le nom des unités sur le graphique** pour imprimer le nom, comme Milliers, avec le titre de l'axe), de passer à une échelle logarithmique (chaque graduation est une base multipliée par la précédente, comme 1, 10, 100 et 1 000, ce qui convient aux nombres de tailles très différentes et qui doivent tous être supérieurs à 0), de choisir l'endroit où l'autre axe le coupe, le format de nombre de ses étiquettes, ses graduations, la position de ses étiquettes et leur rotation. Seul un axe numérique possède des bornes et des unités.
- **Déplacer le graphique…** dans l'onglet **Création de graphique** demande une autre feuille de calcul et y place le graphique, aux mêmes cellules. Il lit toujours les cellules qu'il lisait, conserve sa taille et sa position sur la feuille, même lorsque les lignes et les colonnes sont plus hautes ou plus larges, et se déplace et se redimensionne avec les cellules de la même façon qu'auparavant. Les feuilles masquées ne sont pas proposées. Il s'agit d'une seule étape d'**Annuler**, même après un enregistrement. Une feuille de graphique (feuille qui ne contient qu'un graphique) ne peut pas encore être créée, et un graphique qui se trouve sur une telle feuille ne peut pas être déplacé.
- **Titre du graphique** et **Titres des axes** (dans le menu **+**) proposent aussi **Texte du titre…**. Saisissez le texte souhaité. Ou saisissez un signe égal suivi d'une cellule, comme **=Feuil1!A1**, et le titre affiche le contenu de cette cellule. Lorsque la cellule change, le titre change aussi. Le fichier conserve le lien.
- **Enregistrer comme image…** dans l'onglet **Création de graphique** (et dans le menu contextuel) enregistre le graphique sélectionné dans un fichier image **PNG** ou **JPEG**, deux fois plus grand qu'il ne l'est sur la feuille, afin de rester net.
- **Enregistrer comme modèle…** et **Appliquer le modèle…** (dans l'onglet **Création de graphique**, dans le groupe **Type**) conservent l'apparence d'un graphique dans un fichier **.crtx** et la transmettent à un autre graphique. L'apparence comprend le type, les couleurs, les paramètres du titre et de la légende, les axes et les polices. Le graphique auquel vous l'appliquez conserve ses propres nombres et les cellules qu'il lit. Une seule étape d'**Annuler** supprime de nouveau cette apparence.
- **Modifier le type de graphique** change le type de l'ensemble du graphique, ou celui de chaque série : des colonnes avec une courbe par-dessus, ou une série tracée sur un second axe vertical.

Une référence à des cellules s'écrit par exemple **Feuil1!$B$2:$B$9**, avec le nom de la feuille entre apostrophes lorsqu'il contient des espaces.

## Sparklines

Une sparkline est un mini-graphique qui tient dans une seule cellule. Elle montre la tendance d'une ligne ou d'une colonne de nombres, juste à côté d'eux.

1. Sélectionnez les nombres. Pour obtenir une sparkline par ligne, sélectionnez plusieurs lignes de nombres.
2. Dans l'onglet **Insertion**, dans le groupe **Sparklines**, choisissez **Courbe**, **Histogramme** ou **Gain/perte**. Une **Courbe** relie les nombres par un trait. Un **Histogramme** trace une barre pour chaque nombre. Un **Gain/perte** trace une barre vers le haut pour chaque nombre supérieur à zéro et une barre vers le bas pour chaque nombre inférieur à zéro, toutes de la même taille, ce qui convient aux résultats qui ne sont que bons ou mauvais.
3. Dans la boîte **Créer des sparklines**, vérifiez la **Plage de données** (les cellules que lisent les sparklines) et saisissez la **Plage d'emplacement** (les cellules qui les accueillent). Choisissez une cellule d'emplacement pour chaque ligne de données, ou pour chaque colonne de données. Si vous choisissez une seule cellule, la sparkline lit toutes les données.

Les sparklines créées en une seule opération forment un groupe et partagent le même style. Cliquez sur une cellule contenant une sparkline pour afficher l'onglet **Outils de sparkline**.

- **Modifier les données** modifie les données et l'emplacement de tout le groupe, ou les données de la seule sparkline sur laquelle vous avez cliqué. Elle permet aussi de définir le traitement des cellules vides (laisser un espace, les considérer comme zéro, ou relier les points de part et d'autre) et de choisir si les cellules des lignes et colonnes masquées sont lues.
- **Type** permet de passer de courbe à histogramme ou gain/perte.
- **Afficher** met en évidence des points : le **Point haut**, le **Point bas**, les **Points négatifs**, le **Premier point**, le **Dernier point» et, pour une courbe, les **Marqueurs** sur chaque point.
- **Styles** attribue au groupe un jeu de couleurs. **Couleur de sparkline** définit la couleur de la courbe ou des barres, et l'**Épaisseur** (l'épaisseur du trait, en points). **Couleur du marqueur** définit la couleur de chaque type de point mis en évidence.
- **Axe** affiche une ligne à zéro, trace les nombres de droite à gauche et définit les valeurs minimale et maximale de l'axe vertical. **Automatique pour chaque sparkline** ajuste chacune à ses propres nombres. **Identique pour toutes les sparklines** donne une seule échelle à tout le groupe, pour que vous puissiez les comparer. **Valeur personnalisée** vous permet de saisir un nombre. **Axe des dates…** espace les points selon leurs dates : saisissez les cellules qui contiennent la date de chaque point, et un point dont la date est plus tardive se place plus loin. Laissez le champ vide pour répartir de nouveau les points de façon régulière.
- **Grouper** place les sparklines sélectionnées dans un même groupe. **Dissocier** donne à chacune son propre style. **Effacer** supprime les sparklines sélectionnées, ou leurs groupes entiers.

Une sparkline suit ses cellules : si vous insérez une ligne au-dessus des données, elle lit toujours les mêmes nombres. Chaque modification constitue une étape que vous pouvez annuler. Les sparklines sont enregistrées dans le fichier sous une forme que d'autres tableurs peuvent lire.

## Insérer une image, une forme ou une zone de texte

Dans l'onglet **Insertion**, dans le groupe **Illustrations**, **Image** ouvre une boîte de sélection de fichier. Choisissez un fichier image (PNG, JPEG, GIF, BMP, TIFF ou SVG) : il s'insère dans la feuille, à la cellule sélectionnée. Une image dont la largeur dépasse 480 pixels est réduite pour tenir, et conserve ses proportions. Le classeur conserve l'image elle-même. Une image d'un type que le classeur ne peut pas conserver tel quel (un SVG, par exemple) est enregistrée au format PNG, une image ordinaire faite de points. Un fichier qui n'est pas une image, ou qui dépasse 16 Mo, est refusé avec un message. Un fichier WMF ou EMF (les formats d'image que certains anciens programmes créent à partir de traits et de formes) est également dessiné et conservé au format PNG.

**Icônes** affiche la même fenêtre de petites images au trait que celle de JeffPub et de JeffDeck, avec une zone de recherche par nom. Choisissez-en une ou plusieurs : chacune s'insère dans la feuille sous forme d'image, large et haute de 48 points. Elle est dessinée en gris foncé, à une taille qui reste nette lorsque vous l'agrandissez. Chacune constitue une seule étape que vous pouvez annuler.

**Formes** ouvre une liste : rectangle, rectangle à coins arrondis, ovale, triangle, losange, flèches, étoile, cœur, nuage, bulle de dialogue et ligne. Choisissez-en une : elle s'insère dans la feuille en bleu clair, avec un contour bleu plus foncé. **Zone de texte**, dans le groupe **Texte**, insère une zone vide sans remplissage ni bordure.

Lorsqu'une forme est sélectionnée, **Styles** dans l'onglet **Format de la forme** lui applique un aspect prédéfini : un remplissage et un contour assortis, par exemple bleu clair avec un contour bleu plus foncé. Les deux derniers ne conservent que le contour ou que le remplissage. Un clic équivaut à une étape que vous pouvez annuler. **Remplissage de forme** et **Contour de forme** modifient chaque partie séparément.

Chacune de ces opérations constitue une seule étape que vous pouvez annuler. Le nouvel objet est sélectionné : vous pouvez le faire glisser, le redimensionner, le faire passer devant ou derrière, lui donner un nom et un texte de remplacement dans **Propriétés**, ou appuyer sur **Supprimer**.

## Modifier l'aspect d'une image, d'une forme ou d'une zone de texte

Sélectionnez une image pour afficher l'onglet **Format de l'image**. Sélectionnez une forme, une ligne ou une zone de texte pour afficher l'onglet **Format de la forme**. Chaque modification constitue une étape que vous pouvez annuler, et l'objet conserve son nom, son texte de remplacement, sa place et son ordre parmi les autres objets.

- **Remplissage de forme** et **Contour de forme** choisissent une couleur, ou aucune. **Style de trait** définit l'épaisseur (l'épaisseur du trait) et les tirets (continu, tireté ou pointillé). Une image dispose de **Bordure de l'image**, qui correspond à son contour.
- **Pivoter** fait tourner l'objet d'un quart de tour vers la droite ou vers la gauche, ou le retourne horizontalement ou verticalement.
- **Rogner**, pour une image, la découpe selon une forme de fenêtre : carré, 4 de large sur 3 de haut, 3 de large sur 4 de haut, 16 sur 9, ou 9 sur 16. La découpe est répartie uniformément à partir du centre de ce qui est visible, et l'image est agrandie autant que possible à partir de ce qui reste. **Afficher l'image entière** rétablit ce qui a été rogné. Le fichier image n'est pas modifié : la partie rognée est seulement masquée.
- **Modifier le texte** concerne les formes et les zones de texte. Saisissez le texte, puis choisissez sa position horizontale et verticale, sa taille, le gras ou le normal, et s'il passe à la ligne dans la forme ou reste sur une seule ligne.
- **Volet Sélection** ouvre la liste de tous les objets de la feuille, par nom, celui qui est au premier plan en tête. Cliquez sur un nom pour sélectionner cet objet, ce qui est utile lorsque des objets se masquent les uns les autres. **Avancer** et **Reculer** le déplacent dans la liste.
- **Propriétés** définit la taille, le nom, le texte de remplacement et le comportement de l'objet lorsque les cellules sont déplacées ou agrandies.

## Couper, copier et coller un objet

Une image ou une forme se copie et se colle de la même façon qu'un graphique. Une forme qui utilise un élément du classeur autre qu'une image, comme un lien vers un fichier, ne peut pas être copiée, et JeffCrunch vous indique pourquoi.

Copiez un graphique et collez-le dans le même classeur, sur n'importe quelle feuille : la copie lit les mêmes cellules que l'original. Si vous le collez dans un autre classeur, il conserve les nombres qu'il affichait mais ne lit plus de cellules, car les cellules qu'il lirait là-bas ne sont pas celles à partir desquelles il a été créé.

## Sélectionner plusieurs objets, les grouper et les aligner

Cliquez sur un objet, puis maintenez **Maj** ou **Ctrl** enfoncée et cliquez sur un autre pour le sélectionner également. Cliquez de nouveau sur un objet sélectionné en maintenant **Maj** ou **Ctrl** enfoncée pour le désélectionner. Les objets sélectionnés affichent chacun un cadre en pointillés. Vous pouvez aussi choisir **Sélectionner des objets** dans l'onglet **Format de la forme**, puis faire glisser un cadre sur des cellules vides : tous les objets entièrement contenus dans le cadre sont sélectionnés. Appuyez sur Échap, ou choisissez de nouveau **Sélectionner des objets**, pour revenir à la sélection de cellules. Appuyez sur **Supprimer** pour retirer tous les objets sélectionnés en une seule étape.

Lorsque deux objets ou plus sont sélectionnés, l'onglet **Format de la forme** propose ces commandes dans le groupe **Organiser** :

- **Grouper** réunit les formes, images et lignes sélectionnées en un seul objet. Il se déplace et se redimensionne d'un seul bloc, et le fichier le conserve sous forme de groupe. Un graphique ne peut pas être placé dans un groupe.
- **Dissocier** sépare un groupe sélectionné. Chaque élément retrouve sa place sur la feuille.
- **Aligner** aligne les objets sélectionnés : **Aligner à gauche**, **Centrer**, **Aligner à droite**, **Aligner en haut**, **Aligner au milieu** et **Aligner en bas**. Ils s'alignent sur le bord ou sur le centre de l'ensemble. **Répartir horizontalement** et **Répartir verticalement** répartissent uniformément trois objets ou plus entre les deux situés aux extrémités.

Grouper, Dissocier et chaque choix d'alignement constituent une étape que vous pouvez annuler.

## Nommer un objet

Lorsqu'un objet est sélectionné, la zone Nom affiche son nom. Saisissez-y un nouveau nom et appuyez sur **Entrée**. **Annuler** rétablit l'ancien nom, et le nouveau nom est enregistré dans le fichier.

## Supprimer un objet

Appuyez sur **Supprimer** lorsque l'objet est sélectionné. **Annuler** le rétablit.

## Ce qui est enregistré

Tout ce qui précède est enregistré dans le fichier sous une forme que d'autres tableurs peuvent lire : un graphique créé ici s'ouvre donc de la même manière ailleurs. Un graphique provenant d'un autre programme et contenant des éléments que JeffCrunch ne peut pas conserver lorsqu'il réécrit un graphique (barres d'erreur, graphique en trois dimensions) peut toujours être déplacé, redimensionné et supprimé, mais ses éléments et ses données ne sont pas modifiés ici.

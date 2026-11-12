# Graphiques, sparklines, images et formes

<!-- keywords: boutons de champ, boutons de champ de graphique croisé dynamique, grouper, dissocier, aligner, aligner à gauche, distribuer, sélectionner des objets, plusieurs formes, sélection multiple, cadre de sélection, filtres de graphique, masquer une série, graphiques recommandés, suggestions de graphiques, graphique, diagramme, histogramme, colonne, barre, courbe, ligne, secteurs, aires, nuage de points, combiné, légende, titre, quadrillage, étiquettes de données, données du graphique, type de graphique, apparence, dispositions, déplacer, redimensionner, image, forme, objet, série, courbe de tendance, tendance, équation, R², second axe, sparkline, sparklines, axe des dates, gain/perte, mini-graphique, marqueurs, nom, texte de remplacement, échelle logarithmique, enregistrer comme image, modèle, crtx, texte du titre, repère de légende, volet Sélection, remplissage de forme, contour de forme, pivoter, retourner, rogner, modifier le texte, bordure d'image, insérer une image, icônes, icône, wmf, emf, styles de forme, styles, insérer une forme, zone de texte, formes, flèche, ovale, rectangle, étoile, diagramme, diagrammes, images en ligne, images libres, openverse, wikimedia commons, licence, crédit, texte artistique, wordart, déformation, compresser les images, réduire la taille des images, taille du fichier, ppp, supprimer les zones rognées, code-barres, code QR -->

Un graphique est une image de nombres qui reste liée à eux : si vous modifiez un nombre, le graphique change avec lui. Les images et les formes se placent au-dessus des cellules de la même façon, mais elles ne sont liées à aucun nombre.

## Créer un graphique

1. Sélectionnez les nombres à représenter. Si la première ligne contient des noms et la première colonne des étiquettes, sélectionnez-les aussi. JeffCrunch examine la première ligne et la première colonne et les utilise comme noms et comme étiquettes lorsqu'il s'agit de mots (ou de dates, pour les étiquettes).
2. Dans l'onglet **Insertion**, dans le groupe **Graphiques**, choisissez un type : **Histogramme**, **Barres**, **Courbes**, **Secteurs**, **Aires**, **Nuage de points** ou **Graphique combiné**. La petite flèche à côté d'un type donne ses variantes, comme les histogrammes empilés, les courbes avec marqueurs, ou les nuages de points, reliés ou lissés.
3. Pour voir d'abord tous les types, choisissez **Autres graphiques**. La liste de gauche présente chaque type dessiné à partir de vos cellules. **Permuter les lignes et les colonnes** inverse le sens des séries.

Vous ne savez pas quel type choisir ? Sélectionnez vos cellules et choisissez **Graphiques recommandés** dans l'onglet **Insertion**. JeffCrunch examine les cellules et propose quelques types adaptés, du meilleur au moins bon. Par exemple, des dates en colonne suggèrent une courbe, et une colonne de quelques nombres positifs accompagnée d'étiquettes suggère des secteurs. Chaque type est dessiné à partir de vos cellules. Choisissez-en un, puis cliquez sur **OK**.

Si vous ne sélectionnez qu'une seule cellule, JeffCrunch représente le bloc de cellules autour d'elle qui contient des données.

Les séries sont disposées en colonnes lorsque la sélection compte au moins autant de lignes que de colonnes, et en lignes lorsqu'elle compte plus de colonnes que de lignes. Un secteur ne contient qu'une série. Un nuage de points prend ses valeurs horizontales dans la première colonne.

Exemple : vous sélectionnez A1:C4, où A2:A4 contiennent T1, T2 et T3, B1 et C1 contiennent Est et Ouest, et le reste sont des nombres. Le graphique comporte deux séries, Est et Ouest, et trois groupes de colonnes, T1 à T3.

## Masquer une série temporairement

Sélectionnez un graphique. Trois petits boutons apparaissent à côté de lui. Celui qui porte un entonnoir est **Filtres de graphique**. Il affiche les séries du graphique, chacune avec une case à cocher. Décochez une case pour retirer la série du graphique. Cochez-la de nouveau pour la faire réapparaître à la même place. **Annuler** fonctionne aussi. Un graphique conserve toujours au moins une série. Une série masquée reste présente dans le fichier lors de l'enregistrement, et reste masquée à l'ouverture du fichier : rien n'est perdu. Cochez-la pour la faire réapparaître. (Un graphique en anneau affiche toutes les séries à l'ouverture.)

## Déplacer et redimensionner

Cliquez sur un graphique, une image ou une forme pour le sélectionner. Faites-le glisser pour le déplacer, ou faites glisser l'un des huit carrés de son bord pour le redimensionner.

- Maintenez **Alt** pendant le glissement pour aligner un bord sur le bord d'une cellule.
- Maintenez **Maj** pendant le glissement d'un coin pour conserver les proportions.
- Les touches fléchées le déplacent de quelques pixels à la fois. **Ctrl** avec une flèche le déplace d'un pixel, **Maj** avec une flèche le agrandit ou le réduit, et **Alt** avec une flèche le place au bord de cellule suivant.
- **Échap** pendant le glissement le remet à sa place.

Chaque déplacement ou redimensionnement constitue une étape que vous pouvez annuler. Faites un clic droit sur l'objet et choisissez **Propriétés** pour saisir son **Nom** (chaque objet d'une feuille doit avoir le sien) et son **Texte de remplacement** (une courte phrase qui indique à une personne qui ne peut pas voir l'objet ce qu'il représente), pour saisir sa taille, et pour choisir son comportement lors de l'insertion, de la suppression ou du redimensionnement de cellules : se déplacer et se redimensionner avec les cellules, se déplacer avec elles sans changer de taille, ou rester en place. Ce choix est enregistré dans le fichier. **Imprimer cet objet**, dans la même boîte, masque l'objet sur les pages imprimées (il reste affiché à l'écran) ; le fichier conserve aussi ce choix.

## Modifier ce qu'affiche un graphique

Lorsqu'un graphique est sélectionné, deux boutons apparaissent à côté de lui et un onglet **Graphique** apparaît dans le ruban.

- Le bouton **+** (et **Éléments du graphique** dans l'onglet) active ou désactive le titre du graphique, les axes, les titres des axes, le quadrillage, la légende et les étiquettes de données, et indique où placer la légende et les étiquettes.
- Le bouton en forme de pinceau (et **Couleurs** et **Styles** dans l'onglet) donne au graphique l'un des jeux de couleurs et l'un des aspects de JeffCrunch. **Dispositions** règle plusieurs éléments à la fois.
- **Données du graphique** affiche les séries du graphique. Vous pouvez en ajouter, les modifier, les supprimer et les réordonner, changer les cellules que chacune lit, définir les cellules contenant les étiquettes, permuter les lignes et les colonnes, et choisir la façon de représenter les cellules vides et les lignes et colonnes masquées.
- **Boutons de champ** (dans le menu **+**, uniquement pour un graphique croisé dynamique) affiche de petits boutons portant les noms des champs du tableau croisé dynamique que suit le graphique : les filtres du rapport sous le titre, et les lignes, valeurs et colonnes le long du bas. Activez ou désactivez chaque type, ou choisissez **Tout afficher** ou **Tout masquer**. Un nouveau graphique croisé dynamique les affiche.
- **Barres d'erreur** (dans le menu **+**, pour un graphique en histogramme, barres, courbes, aires ou nuage de points) trace un petit trait au-dessus et au-dessous de chaque point pour montrer de combien la valeur réelle pourrait différer. Choisissez **Erreur type**, **Pourcentage (5 %)**, **Écart type** ou **Valeur fixe (1)**. Choisissez **Personnalisées** pour utiliser vos propres montants : saisissez les cellules qui contiennent les montants au-dessus de chaque point (par exemple `=Feuil1!$D$2:$D$4`), puis les cellules des montants en dessous. Laissez-en une vide pour ne tracer les barres que d'un côté. Les barres changent lorsque ces cellules changent. Un fichier qui contient ses propres montants pour chaque point les conserve.
- **Lignes** (dans le menu **+**, pour un graphique en courbes) propose **Lignes de projection**, qui descendent du point le plus haut de chaque catégorie jusqu'à l'axe, **Lignes haut-bas**, qui relient les points les plus haut et les plus bas de chaque catégorie, et **Barres haut-bas**, qui sont des rectangles entre la première et la dernière série de chaque catégorie. Une barre montante indique que la dernière série est plus élevée. Les lignes haut-bas et les barres haut-bas nécessitent au moins deux séries. Les graphiques en aires ont des lignes de projection.
- **Courbe de tendance** (dans le menu **+**) trace une droite ajustée aux nombres de chaque série, pour voir vers où ils se dirigent. Choisissez **Linéaire** pour une droite, **Exponentielle** pour des nombres qui augmentent d'un pourcentage constant, **Logarithmique** pour des nombres qui augmentent vite puis se stabilisent, **Polynomiale** pour une courbe avec des inflexions, **Puissance**, ou **Moyenne mobile**, qui trace la moyenne des derniers nombres. **Afficher l'équation** inscrit la formule de la droite sur le graphique, et **Afficher le R²** indique la qualité de l'ajustement (1 correspond à un ajustement parfait ; 0 à aucun ajustement). La courbe de tendance a sa propre entrée dans la légende.
- Lorsqu'une série est tracée sur un second axe vertical, les menus **Axes** et **Titres des axes** proposent les entrées **Second horizontal** et **Second vertical** pour elle. Le menu **Légende** propose **Au-dessus du graphique**, qui place la légende sur la zone de tracé au lieu de la placer à côté.
- **Étiquettes de données** propose aussi **Contenu de l'étiquette** (la valeur, le nom de la catégorie, le nom de la série, le pourcentage et le repère de légende, c'est-à-dire un petit carré de la couleur de la série, combinables à volonté), **Séparateur** (ce qui les sépare : virgule, point-virgule, point, saut de ligne ou espace), et **Format de nombre…** (un code de format comme 0,0 pour les nombres des étiquettes ; laissez-le vide pour afficher les nombres tels quels). **Valeur à partir des cellules…** affiche le texte de certaines cellules comme étiquettes, une cellule par point. Saisissez les cellules, par exemple =Feuil1!$D$2:$D$4. Les étiquettes changent lorsque les cellules changent. Laissez le champ vide pour les supprimer. Désactivez tous les contenus et les étiquettes disparaissent.
- **Mettre en forme l'axe horizontal…** et **Mettre en forme l'axe vertical…** dans l'onglet **Graphique** ouvrent une boîte pour un axe. Elle définit les valeurs minimale et maximale, ainsi que les unités principale et secondaire (chacune automatique ou un nombre que vous saisissez), si les valeurs suivent un ordre inverse, les unités d'affichage (les étiquettes affichent les nombres divisés, par exemple par 1 000 pour des milliers, de sorte que 30 000 s'affiche 30 ; cochez **Afficher le nom des unités sur le graphique** pour imprimer le nom, comme Milliers, avec le titre de l'axe), si l'échelle est logarithmique (chaque repère est une base multipliée par le précédent, comme 1, 10, 100 et 1 000, ce qui convient aux nombres très différents les uns des autres et qui doivent tous être supérieurs à 0), l'endroit où l'autre axe coupe celui-ci, le format de nombre de ses étiquettes, ses graduations, la position de ses étiquettes et l'angle de leur rotation. Seul un axe numérique possède des bornes et des unités.
- **Déplacer le graphique…** dans l'onglet **Graphique** demande où placer le graphique. Choisissez **Nouvelle feuille** et saisissez un nom : le graphique reçoit une feuille de graphique à lui, une feuille qui ne contient qu'un graphique, sans cellules. Choisissez **Objet dans** et sélectionnez une feuille de calcul : le graphique est placé sur cette feuille. Un graphique situé sur une feuille de graphique ne peut être placé que sur une feuille de calcul, et la feuille de graphique qu'il quitte est supprimée. Le graphique lit toujours les cellules qu'il lisait. Sur une feuille de calcul, il conserve sa taille et sa place sur la feuille, même lorsque les lignes et les colonnes sont plus hautes ou plus larges, et il se déplace et se redimensionne avec les cellules comme auparavant. Les feuilles masquées ne sont pas proposées. Cette opération constitue une seule étape d'**Annuler**, même après un enregistrement, sauf si une feuille a été supprimée par elle. Un fichier au format `.ods` ne peut pas contenir de feuille de graphique : enregistrez donc une feuille de graphique au format `.xlsx`.
- **Titre du graphique** et **Titres des axes** (dans le menu **+**) proposent aussi **Texte du titre…**. Saisissez le texte souhaité. Ou saisissez un signe égal suivi d'une cellule, par exemple **=Feuil1!A1**, et le titre affiche le contenu de cette cellule. Lorsque la cellule change, le titre change aussi. Le fichier conserve le lien.
- **Enregistrer comme image…** dans l'onglet **Graphique** (et dans le menu contextuel) enregistre le graphique sélectionné dans un fichier image **PNG** ou **JPEG**, deux fois plus grand que sur la feuille pour rester net.
- **Enregistrer comme modèle…** et **Appliquer le modèle…** (dans l'onglet **Graphique**, groupe **Type**) conservent l'aspect d'un graphique dans un fichier **.crtx** et l'appliquent à un autre graphique. L'aspect comprend le type, les couleurs, les réglages du titre et de la légende, les axes et les polices. Le graphique auquel vous l'appliquez conserve ses propres nombres et les cellules qu'il lit. Une seule étape d'**Annuler** supprime à nouveau cet aspect.
- **Modifier le type de graphique** change le type de tout le graphique, ou de chaque série : des colonnes avec une courbe par-dessus, ou une série sur un second axe vertical.

Une référence à des cellules s'écrit par exemple **Feuil1!$B$2:$B$9**, avec le nom de la feuille entre apostrophes lorsqu'il contient des espaces.

## Sparklines

Une sparkline est un mini-graphique qui tient dans une seule cellule. Elle montre la tendance d'une ligne ou d'une colonne de nombres, juste à côté d'eux.

1. Sélectionnez les nombres. Pour une sparkline par ligne, sélectionnez plusieurs lignes de nombres.
2. Dans l'onglet **Insertion**, dans le groupe **Sparklines**, choisissez **Courbe**, **Histogramme** ou **Gain/perte**. Une **courbe** relie les nombres par un trait. Un **histogramme** trace une barre pour chaque nombre. **Gain/perte** trace une barre vers le haut pour chaque nombre supérieur à zéro et une barre vers le bas pour chaque nombre inférieur à zéro, toutes de la même taille, ce qui convient aux résultats qui ne sont que bons ou mauvais.
3. Dans la boîte **Créer des sparklines**, vérifiez la **Plage de données** (les cellules que lisent les sparklines) et saisissez la **Plage d'emplacement** (les cellules qui les accueillent). Choisissez une cellule d'emplacement pour chaque ligne de données, ou pour chaque colonne de données. Si vous choisissez une seule cellule, la sparkline lit toutes les données.

Les sparklines créées en une seule fois forment un groupe et partagent le même aspect. Cliquez sur une cellule contenant une sparkline : l'onglet **Sparkline** apparaît.

- **Modifier les données** modifie les données et l'emplacement de tout le groupe, ou les données de celle sur laquelle vous avez cliqué. Il indique aussi quoi faire des cellules vides (laisser un espace, les traiter comme zéro, ou relier les points à travers elles) et s'il faut lire les cellules des lignes et colonnes masquées.
- **Type** alterne entre courbe, histogramme et gain/perte.
- **Afficher** met en évidence des points : le **Point haut**, le **Point bas**, les **Points négatifs**, le **Premier point**, le **Dernier point» et, pour une courbe, les **Marqueurs** sur chaque point.
- **Styles** donne au groupe un jeu de couleurs. **Couleur de la sparkline** définit la couleur du trait ou des barres, ainsi que l'**Épaisseur** (l'épaisseur du trait, en points). **Couleur du marqueur** définit la couleur de chaque type de point mis en évidence.
- **Axe** affiche un trait à zéro, trace les nombres de droite à gauche, et définit les valeurs minimale et maximale de l'axe vertical. **Automatique pour chaque sparkline** ajuste chacune à ses propres nombres. **Identique pour toutes les sparklines** donne au groupe une seule échelle, pour pouvoir les comparer. **Valeur personnalisée** vous permet de saisir un nombre. **Axe des dates…** espace les points selon les dates : saisissez les cellules qui contiennent la date de chaque point, et un point dont la date est plus tardive se place plus loin. Laissez le champ vide pour répartir de nouveau les points de façon régulière.
- **Grouper** place les sparklines sélectionnées dans un même groupe. **Dissocier** donne à chacune son propre aspect. **Effacer** supprime les sparklines sélectionnées, ou leurs groupes entiers.

Une sparkline suit ses cellules : si vous insérez une ligne au-dessus des données, elle lit toujours les mêmes nombres. Chaque modification constitue une étape que vous pouvez annuler. Les sparklines sont enregistrées dans le fichier sous une forme que les autres tableurs peuvent lire.

## Insérer une image, une forme ou une zone de texte

Dans l'onglet **Insertion**, dans le groupe **Illustrations**, **Image** ouvre un sélecteur de fichiers. Choisissez un fichier image (PNG, JPEG, GIF, BMP, TIFF ou SVG) : il apparaît sur la feuille, dans la cellule sélectionnée. Une image dont la largeur dépasse 480 points est réduite pour tenir, en conservant ses proportions. Le classeur conserve l'image elle-même. Une image d'un type que le classeur ne peut pas conserver tel quel (un SVG, par exemple) est enregistrée au format PNG, une image ordinaire composée de points. Un fichier qui n'est pas une image, ou qui dépasse 16 Mo, est refusé avec un message. Un fichier WMF ou EMF (les formats d'image que certains anciens programmes produisent à partir de traits et de formes) est également dessiné et enregistré au format PNG.

**Icônes** affiche la même fenêtre de petites images au trait que JeffPub et JeffDeck proposent, avec une zone de recherche par nom. Choisissez-en une ou plusieurs : chacune apparaît sur la feuille sous forme d'image, de 48 points de large et de haut. Elle est dessinée en gris foncé, à une taille qui reste nette lorsque vous l'agrandissez. Chacune constitue une étape que vous pouvez annuler.

**Code-barres** ouvre une fenêtre pour créer un code-barres : ISBN de livre (avec ou sans prix), EAN-13, UPC-A, EAN-8, Code 128, Code 39, ou code QR, qui peut contenir n'importe quel texte, comme une adresse web. Choisissez un **Type**, saisissez le texte ou les chiffres : la fenêtre affiche le code-barres au fur et à mesure de la saisie, et indique en rouge ce qui ne va pas s'il ne peut pas être créé. Cliquez sur **Insérer** : il apparaît sur la feuille sous forme d'image, en une étape que vous pouvez annuler. Déplacez-le et redimensionnez-le comme n'importe quelle image. Pour le modifier, sélectionnez-le et cliquez de nouveau sur **Code-barres**, changez le texte, puis cliquez sur **Mettre à jour** : le nouveau code-barres prend la place de l'ancien et sa largeur. À l'enregistrement, il est sauvegardé comme image accompagnée d'un SVG qui reste net, afin que tous les programmes l'affichent ; JeffCrunch conserve ses réglages dans le fichier, de sorte que vous pouvez encore le modifier lorsque vous rouvrez le classeur dans JeffCrunch.

**Formes** ouvre une liste : rectangle, rectangle à coins arrondis, ovale, triangle, losange, flèches, étoile, cœur, nuage, bulle de dialogue et trait. Choisissez-en une : elle apparaît sur la feuille en bleu clair, avec un contour bleu plus foncé. **Zone de texte**, dans le groupe **Texte**, insère une zone vide, sans remplissage ni bordure.

Lorsqu'une forme est sélectionnée, **Styles** dans l'onglet **Format de la forme** lui applique un aspect prêt à l'emploi : un remplissage et un contour assortis, comme du bleu clair avec un bord bleu plus foncé. Les deux derniers ne conservent que le contour ou que le remplissage. Un clic constitue une étape que vous pouvez annuler. **Remplissage de la forme** et **Contour de la forme** modifient chaque partie séparément.

## Trouver des images libres en ligne

**Images en ligne**, dans le groupe **Illustrations** de l'onglet **Insertion**, recherche des images libres. Elles proviennent de deux bibliothèques ouvertes : Openverse, qui interroge de nombreuses collections, et Wikimedia Commons, la bibliothèque des images de Wikipédia.

1. Cliquez sur **Images en ligne…**. Un volet s'ouvre à droite.
2. Choisissez une bibliothèque, saisissez ce que vous recherchez et cliquez sur **Rechercher** (ou appuyez sur Entrée).
3. Cliquez sur une image pour voir qui en est l'auteur et quelle **licence** elle possède. Une **licence** indique comment vous pouvez utiliser une image. Cliquez sur le nom de la licence pour la lire.
4. Cliquez sur **Insérer** (ou double-cliquez sur l'image).

**Ce que JeffCrunch envoie.** Les mots de votre recherche sont envoyés à la bibliothèque choisie, uniquement lorsque vous cliquez sur **Rechercher** ou appuyez sur Entrée. JeffCrunch demande ensuite à la bibliothèque les petits aperçus de ce qu'elle a trouvé, puis l'unique image que vous insérez. Comme tout site web, la bibliothèque peut voir l'adresse internet de votre ordinateur, et elle est informée que JeffCrunch fait cette demande. Rien de votre classeur n'est envoyé.

L'image apparaît sur la feuille comme une image provenant d'un fichier, et le classeur la conserve. Son titre est enregistré comme **texte de remplacement**. La plupart des licences demandent de citer l'auteur. Lorsque **Ajouter un crédit sous l'image** est coché, JeffCrunch ajoute sous l'image une petite zone de texte portant son titre, son auteur et sa licence. Vous pouvez la déplacer, la modifier ou la supprimer. L'image et son crédit sont insérés en une seule étape, de sorte qu'**Annuler** retire les deux. Vérifiez que la licence convient à l'usage que vous ferez de l'image.

## Ajouter du texte artistique

**Texte artistique** consiste en des mots soignés : lettres colorées, avec un contour, une ombre ou une forme courbe.

1. Cliquez sur **Insertion > Texte > Texte artistique**. Une galerie de styles s'ouvre.
2. Cliquez sur un style. Les mots « Votre texte ici » apparaissent sur la feuille dans ce style.
3. Pour modifier les mots, sélectionnez le texte artistique et cliquez sur **Modifier le texte…** dans l'onglet **Format de la forme**. Une boîte demande les mots, la police, la taille, le gras et l'italique. Une taille plus grande agrandit le texte artistique.
4. Dans l'onglet **Format de la forme**, **Styles de texte artistique** donne un autre style aux lettres, et **Déformation du texte** courbe les mots selon une forme, comme une vague ou un arc. « Normal » les laisse droits.

Le texte artistique est une forme du dessin de la feuille, avec sa déformation, son remplissage et son contour : il conserve son aspect lorsque vous enregistrez le classeur au format .xlsx et que vous le rouvrez, dans JeffCrunch ou dans un autre programme. Chaque modification constitue une étape que vous pouvez annuler. Un classeur enregistré au format .ods conserve aussi ses images, formes, zones de texte, groupes, traits et textes artistiques. Le texte artistique est enregistré sous la forme de texte d'habillage qu'OpenDocument prévoit, avec ses mots, sa police, son remplissage, son contour et sa déformation, et redevient du texte artistique lorsque vous rouvrez le fichier .ods. Quelques déformations qu'OpenDocument ne prévoit pas (Vague 2, Vague 4, et les formes basse et haute de Gonfler et Dégonfler) sont enregistrées comme des mots droits, avec les mêmes lettres et le même style. Les graphiques sont également conservés dans un fichier .ods : voir ci-dessous.

## Réduire la taille des images

Une photo prise avec un appareil peut être beaucoup plus grande que son affichage sur la feuille, et alourdir le fichier. Sélectionnez une image, puis cliquez sur **Compresser les images…** dans le groupe **Ajuster** de l'onglet **Format de l'image**. Choisissez l'usage prévu du classeur : **Impression commerciale (300 ppp)**, **Impression de bureau (220 ppp)** ou **Web (96 ppp)**. Laissez **Appliquer à toutes les images de cette feuille** coché pour réduire toutes les images de la feuille, ou décochez-le pour ne réduire que l'image sélectionnée. **Supprimer les zones rognées des images** retire aussi les parties masquées que vous avez rognées : le rognage disparaît. Une image n'est remplacée que si la nouvelle est plus petite. Sa taille, sa place, son nom et son texte de remplacement restent inchangés. **Annuler** (Ctrl+Z) rétablit les images d'origine, et le classeur enregistré ne contient que les images réduites. Les images des autres feuilles ne sont pas modifiées : allez sur chaque feuille et recommencez. Une image qui est un dessin (SVG, EMF ou WMF), ou une animation, reste inchangée.

Chaque opération constitue une étape que vous pouvez annuler. Le nouvel objet est sélectionné : vous pouvez le faire glisser, le redimensionner, le faire passer devant ou derrière, lui donner un nom et un texte de remplacement dans **Propriétés**, ou appuyer sur **Supprimer**.

## Modifier l'aspect d'une image, d'une forme ou d'une zone de texte

Sélectionnez une image : l'onglet **Format de l'image** apparaît. Sélectionnez une forme, un trait ou une zone de texte : l'onglet **Format de la forme** apparaît. Chaque modification constitue une étape que vous pouvez annuler, et l'objet conserve son nom, son texte de remplacement, sa place et son ordre parmi les autres objets.

- **Remplissage de la forme** et **Contour de la forme** choisissent une couleur, ou aucune. **Style de trait** définit l'épaisseur (l'épaisseur du trait) et les tirets (continu, en tirets ou pointillés). Une image dispose de **Bordure de l'image**, qui est son contour.
- **Pivoter** fait tourner l'objet d'un quart de tour vers la droite ou vers la gauche, ou le retourne horizontalement ou verticalement.
- **Rogner**, pour une image, la découpe selon une forme de fenêtre : carré, 4 de large sur 3 de haut, 3 de large sur 4 de haut, 16 sur 9, ou 9 sur 16. La découpe est faite également depuis le centre de ce qui est visible, et l'image est agrandie autant que possible à partir de ce qui reste. **Afficher l'image entière** rétablit ce qui a été rogné. Le fichier de l'image n'est pas modifié : la partie rognée est seulement masquée.
- **Modifier le texte** concerne les formes et les zones de texte. Saisissez le texte, puis choisissez comment il se place horizontalement, verticalement, sa taille, le gras ou non, et s'il s'enroule dans la forme ou reste sur une seule ligne. Pour un texte artistique, la boîte demande les mots, la police, la taille, le gras et l'italique.
- **Volet Sélection** ouvre la liste de tous les objets de la feuille, par nom, celui du premier plan en haut. Cliquez sur un nom pour sélectionner cet objet, ce qui est utile lorsque des objets se masquent les uns les autres. **Avancer** et **Reculer** le déplacent dans la liste.
- **Propriétés** définit la taille, le nom, le texte de remplacement et le comportement de l'objet lorsque les cellules sont déplacées ou agrandies.

## Couper, copier et coller un objet

Une image ou une forme se copie et se colle comme un graphique. Une forme qui utilise autre chose qu'une image du classeur, comme un lien vers un fichier, ne peut pas être copiée, et JeffCrunch vous indique pourquoi.

Copiez un graphique et collez-le dans le même classeur, sur n'importe quelle feuille : la copie lit les mêmes cellules que l'original. Collez-le dans un autre classeur : il conserve les nombres qu'il affichait, mais ne lit plus de cellules, car les cellules qu'il lirait là-bas ne sont pas celles d'où il provient.

## Sélectionner plusieurs objets, les grouper et les aligner

Cliquez sur un objet, puis maintenez Maj ou Ctrl et cliquez sur un autre pour le sélectionner aussi. Cliquez de nouveau sur un objet sélectionné en maintenant Maj ou Ctrl pour le désélectionner. Les objets sélectionnés reçoivent chacun un cadre en pointillés. Vous pouvez aussi choisir **Sélectionner des objets** dans l'onglet **Format de la forme**, puis faire glisser un cadre sur des cellules vides : chaque objet entièrement compris dans ce cadre est sélectionné. Appuyez sur Échap, ou choisissez de nouveau **Sélectionner des objets**, pour revenir à la sélection de cellules. Appuyez sur Supprimer pour retirer en une seule étape tous les objets sélectionnés.

Lorsque deux objets ou plus sont sélectionnés, l'onglet **Format de la forme** propose ces commandes dans le groupe **Organiser** :

- **Grouper** réunit les formes, images et traits sélectionnés en un seul objet. Il se déplace et se redimensionne d'un bloc, et le fichier le conserve comme groupe. Un graphique ne peut pas être placé dans un groupe.
- **Dissocier** sépare un groupe sélectionné. Chaque élément retrouve sa place sur la feuille.
- **Aligner** aligne les objets sélectionnés : **Aligner à gauche**, **Centrer**, **Aligner à droite**, **Aligner en haut**, **Aligner au milieu** et **Aligner en bas**. Ils s'alignent sur le bord ou le milieu de l'ensemble. **Distribuer horizontalement** et **Distribuer verticalement** répartissent régulièrement trois objets ou plus entre les deux situés aux extrémités.

Grouper, Dissocier et chaque choix d'alignement constituent une étape que vous pouvez annuler.

## Le nommer

Lorsqu'un objet est sélectionné, la zone Nom affiche son nom. Saisissez-y un nouveau nom et appuyez sur **Entrée**. Annuler rétablit l'ancien nom, et le nouveau nom est enregistré dans le fichier.

## Le supprimer

Appuyez sur **Supprimer** lorsque l'objet est sélectionné. Annuler le rétablit.

## Ce qui est enregistré

Tout ce qui précède est enregistré dans le fichier sous une forme que les autres tableurs peuvent lire : un graphique créé ici s'ouvre donc de la même façon ailleurs. Un graphique provenant d'un autre programme et contenant des éléments que JeffCrunch ne peut pas conserver lorsqu'il réécrit un graphique (barres d'erreur, graphique en trois dimensions) peut toujours être déplacé, redimensionné et supprimé, mais ses éléments et ses données ne sont pas modifiés ici.

## Diagrammes provenant d'autres programmes

Un **diagramme** est une image de boîtes et de flèches, par exemple un processus, réalisée par un autre programme. JeffCrunch affiche un diagramme à partir de l'image qu'en conserve le fichier à côté de lui. Cliquez dessus pour le sélectionner, et faites-le glisser pour le déplacer. Lors de l'enregistrement du fichier, le diagramme est réécrit exactement tel qu'il était, afin que le programme qui l'a créé puisse encore le modifier.

JeffCrunch ne peut pas encore modifier le contenu d'un diagramme, et un diagramme ne peut être ni copié ni placé dans un groupe. Si le fichier ne conserve aucune image d'un diagramme, JeffCrunch affiche une simple case grise indiquant qu'il s'agit d'un diagramme.

## Graphiques dans les fichiers .ods

Un classeur enregistré au format .ods conserve ses graphiques : histogrammes, barres, courbes, aires, secteurs, anneaux et nuages de points, chacun avec ses séries et les cellules qu'elles lisent (sur cette feuille ou une autre), ses catégories, son titre, sa légende, ses axes (titres, valeurs minimale et maximale, format de nombre), ses étiquettes de données, ses couleurs et sa place. Un fichier .ods créé par un autre programme s'ouvre avec ses graphiques, dessinés à partir des cellules qu'ils lisent. Ce qu'un graphique .ods ne peut pas contenir est omis et signalé lors de l'enregistrement : courbes de tendance, barres d'erreur, axes secondaires et les aspects particuliers d'un seul point. Les graphiques de types que JeffCrunch ne dessine pas (graphiques boursiers et radar) s'ouvrent sous forme d'histogrammes de leurs valeurs.

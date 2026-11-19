# Graphiques, sparklines, images et formes

<!-- keywords: objet incorporé, insérer un objet, objet à partir d'une plage, objet à partir d'un graphique, objet à partir d'une page, coller comme objet, liste des objets, modifier l'objet, boutons de champ, boutons de graphique croisé dynamique, grouper, dissocier, aligner, aligner à gauche, distribuer, sélectionner des objets, plusieurs formes, sélection multiple, cadre de sélection, filtres du graphique, masquer une série, graphiques recommandés, suggestions de graphiques, graphique, diagramme, colonne, barre, courbe, secteurs, aires, nuage de points, combiné, légende, titre, quadrillage, étiquettes de données, données du graphique, type de graphique, apparence, dispositions, déplacer, redimensionner, image, forme, objet, série, courbe de tendance, tendance, équation, R carré, second axe, sparkline, sparklines, axe des dates, gain/perte, mini graphique, marqueurs, nom, texte de remplacement, échelle logarithmique, enregistrer comme image, modèle, crtx, texte du titre, clé de légende, volet Sélection, remplissage de forme, contour de forme, pivoter, retourner, rogner, modifier le texte, bordure d'image, insérer une image, icônes, icône, wmf, emf, styles de forme, styles, insérer une forme, zone de texte, formes, flèche, ovale, rectangle, étoile, diagramme SmartArt, diagrammes, images en ligne, images libres, openverse, wikimedia commons, licence, crédit, texte artistique, wordart, déformation, compresser les images, réduire la taille des images, taille du fichier, ppp, supprimer les zones rognées, code-barres, code QR, lier une forme à une cellule, forme affichant une cellule, zone de texte affichant une cellule, valeur de cellule dans une forme -->

Un graphique est une image de nombres qui reste liée à eux : si vous modifiez un nombre, le graphique change avec lui. Les images et les formes se placent au-dessus des cellules de la même manière, mais elles ne sont liées à aucun nombre.

## Créer un graphique

1. Sélectionnez les nombres à représenter dans le graphique. Si la première ligne contient des noms et la première colonne des étiquettes, sélectionnez-les aussi. JeffCrunch examine la première ligne et la première colonne et les utilise comme noms et comme étiquettes lorsqu'il s'agit de mots (ou de dates, pour les étiquettes).
2. Dans l'onglet **Insertion**, dans le groupe **Graphiques**, choisissez un type : **Colonne**, **Barre**, **Courbe**, **Secteurs**, **Aires**, **Nuage de points** ou **Combiné**. La petite flèche à côté d'un type donne ses variantes, comme les colonnes empilées, une courbe avec marqueurs, ou un nuage de points, de points reliés ou de courbes lissées.
3. Pour voir d'abord tous les types, choisissez **Autres graphiques**. La liste de gauche présente chaque type dessiné à partir de vos cellules. **Permuter les lignes et les colonnes** inverse le sens des séries.

Vous ne savez pas quel type choisir ? Sélectionnez vos cellules et choisissez **Graphiques recommandés** dans l'onglet **Insertion**. JeffCrunch examine les cellules et propose quelques types qui leur conviennent, du meilleur au moins bon. Par exemple, des dates en colonne suggèrent une courbe, et une colonne de quelques nombres positifs accompagnés d'étiquettes suggère un secteur. Chaque type est dessiné à partir de vos cellules. Choisissez-en un, puis cliquez sur **OK**.

Si vous ne sélectionnez qu'une seule cellule, JeffCrunch représente le bloc de cellules qui l'entoure et qui contient des données.

Les séries sont disposées en colonnes lorsque la sélection compte au moins autant de lignes que de colonnes, et en lignes lorsqu'elle compte plus de colonnes que de lignes. Un secteur ne contient qu'une série. Un nuage de points prend ses valeurs horizontales dans la première colonne.

Exemple : vous sélectionnez A1:C4, où A2:A4 contiennent T1, T2 et T3, B1 et C1 contiennent Est et Ouest, et le reste sont des nombres. Le graphique comporte deux séries, Est et Ouest, et trois groupes de colonnes, T1 à T3.

## Masquer une série provisoirement

Sélectionnez un graphique. Trois petits boutons apparaissent à côté. Celui qui représente un entonnoir est **Filtres du graphique**. Il liste les séries du graphique, chacune avec une case à cocher. Décochez une case pour retirer cette série du graphique. Cochez-la de nouveau pour la faire réapparaître à sa place. **Annuler** fonctionne aussi. Un graphique conserve toujours au moins une série. Une série retirée reste dans le fichier lors de l'enregistrement, et reste masquée à l'ouverture du fichier : rien n'est perdu. Cochez-la de nouveau pour la rétablir. (Un graphique en anneau affiche toutes les séries à son ouverture.)

## Déplacer et redimensionner

Cliquez sur un graphique, une image ou une forme pour la sélectionner. Faites-la glisser pour la déplacer, ou faites glisser l'un des huit carrés de son bord pour la redimensionner.

- Maintenez **Alt** pendant le glissement pour aligner un bord sur le bord d'une cellule.
- Maintenez **Maj** pendant le glissement d'un coin pour conserver les proportions.
- Les touches fléchées la déplacent de quelques pixels à la fois. **Ctrl** avec une flèche la déplace d'un pixel, **Maj** avec une flèche la fait grandir ou rétrécir, et **Alt** avec une flèche la place au bord de cellule suivant.
- **Échap** pendant le glissement la remet à sa place.

Chaque déplacement ou redimensionnement constitue une étape que vous pouvez annuler. Faites un clic droit sur l'objet et choisissez **Propriétés** pour saisir son **nom** (chaque objet d'une feuille doit avoir le sien) et son **texte de remplacement** (une courte phrase qui indique à une personne qui ne peut pas voir l'objet ce qu'il montre), pour saisir sa taille, et pour choisir ce qu'il fait lorsque vous insérez, supprimez ou redimensionnez des cellules : se déplacer et se redimensionner avec les cellules, se déplacer avec elles sans changer de taille, ou rester en place. Ce choix est enregistré dans le fichier. **Imprimer cet objet**, dans la même boîte, désactive l'objet sur les pages imprimées (il reste affiché à l'écran) ; le fichier conserve aussi ce choix.

## Modifier ce que montre un graphique

Lorsqu'un graphique est sélectionné, deux boutons apparaissent à côté et un onglet **Création de graphique** s'affiche dans le ruban.

- Le bouton **+** (et **Éléments du graphique** dans l'onglet) active ou désactive le titre du graphique, les axes, les titres des axes, le quadrillage, la légende et les étiquettes de données, et indique où placer la légende et les étiquettes.
- Le bouton en forme de pinceau (et **Couleurs** et **Apparence** dans l'onglet) donne au graphique l'un des jeux de couleurs et l'une des apparences de JeffCrunch. **Dispositions** règle plusieurs éléments à la fois.
- **Données du graphique** liste les séries du graphique. Vous pouvez en ajouter, en modifier, en supprimer et en réordonner, changer les cellules que chacune lit, définir les cellules des étiquettes, permuter les lignes et les colonnes, et choisir comment les cellules vides et les lignes et colonnes masquées sont représentées.
- **Boutons de champ** (dans le menu **+**, uniquement pour un graphique croisé dynamique) affiche de petits boutons portant les noms des champs du tableau croisé dynamique que suit le graphique : les filtres de rapport sous le titre, et les lignes, valeurs et colonnes en bas. Activez ou désactivez chaque type, ou choisissez **Tout afficher** ou **Tout masquer**. Un nouveau graphique croisé dynamique les affiche.
- **Barres d'erreur** (dans le menu **+**, pour un graphique en colonnes, en barres, en courbes, en aires ou en nuage de points) trace un petit trait au-dessus et au-dessous de chaque point pour montrer de combien la valeur réelle pourrait différer. Choisissez **Erreur type**, **Pourcentage (5 %)**, **Écart type** ou **Valeur fixe (1)**. Choisissez **Personnalisé** pour utiliser vos propres montants : saisissez les cellules qui contiennent les montants au-dessus de chaque point (par exemple `=Feuil1!$D$2:$D$4`), puis les cellules des montants au-dessous. Laissez-en une vide pour ne tracer les barres que d'un seul côté. Les barres changent lorsque ces cellules changent. Un fichier qui possède ses propres montants pour chaque point les conserve.
- **Lignes** (dans le menu **+**, pour un graphique en courbes) propose **Lignes de projection**, qui descendent du point le plus haut de chaque catégorie jusqu'à l'axe, **Lignes haut-bas**, qui relient les points le plus haut et le plus bas de chaque catégorie, et **Barres haut-bas**, qui sont des rectangles entre la première et la dernière série de chaque catégorie. Une barre montante indique que la dernière série est plus élevée. Les lignes haut-bas et les barres haut-bas exigent au moins deux séries. Les graphiques en aires disposent des lignes de projection.
- **Tableau de données** (dans le menu **+**, pour un graphique en colonnes, en courbes ou en aires) place sous le graphique une grille des nombres, avec une colonne pour chaque catégorie et une ligne pour chaque série. Sa liste active ou désactive le tableau, et règle les traits entre les lignes, les traits entre les colonnes, le contour et les clés de légende (la couleur de chaque série à côté de son nom).
- **Courbe de tendance** (dans le menu **+**) trace une courbe ajustée aux nombres de chaque série, pour voir vers où ils se dirigent. Choisissez **Linéaire** pour une droite, **Exponentielle** pour des nombres qui augmentent d'un pourcentage constant, **Logarithmique** pour des nombres qui montent vite puis se stabilisent, **Polynomiale** pour une courbe avec des inflexions, **Puissance**, ou **Moyenne mobile**, qui trace la moyenne des derniers nombres. **Afficher l'équation** écrit la formule de la courbe sur le graphique, et **Afficher R carré** indique la qualité de l'ajustement (1 est un ajustement parfait ; 0, aucun ajustement). La courbe de tendance reçoit sa propre entrée dans la légende.
- Lorsqu'une série est tracée sur un second axe vertical, les menus **Axes** et **Titres des axes** proposent les entrées **Second horizontal** et **Second vertical** pour elle. Le menu **Légende** propose **Par-dessus le graphique**, qui place la légende sur la zone de tracé au lieu de la mettre à côté.
- **Étiquettes de données** propose aussi **Contenu de l'étiquette** (la valeur, le nom de la catégorie, le nom de la série, le pourcentage et la clé de légende, qui est un petit carré de la couleur de la série, dans n'importe quelle combinaison), **Séparateur** (ce qui les sépare : virgule, point-virgule, point, saut de ligne ou espace) et **Format de nombre…** (un code de format comme 0,0 pour les nombres des étiquettes ; laissez-le vide pour afficher les nombres tels quels). **Valeur à partir des cellules…** affiche le texte de certaines cellules comme étiquettes, une cellule pour chaque point. Saisissez les cellules, par exemple `=Feuil1!$D$2:$D$4`. Les étiquettes changent lorsque les cellules changent. Laissez le champ vide pour les supprimer. Désactivez tous les contenus et les étiquettes disparaissent.
- **Format de l'axe horizontal…** et **Format de l'axe vertical…** dans l'onglet **Création de graphique** ouvrent une boîte pour un seul axe. Elle règle les valeurs minimale et maximale, ainsi que les unités principale et secondaire (chacune automatique ou un nombre que vous saisissez), si les valeurs sont dans l'ordre inverse, les unités d'affichage (les étiquettes affichent les nombres divisés, par exemple par 1 000 pour des milliers, de sorte que 30 000 s'affiche 30 ; cochez **Afficher le nom des unités sur le graphique** pour imprimer le nom, comme Milliers, avec le titre de l'axe), si l'échelle est logarithmique (chaque graduation est une base multipliée par la précédente, comme 1, 10, 100 et 1 000, ce qui convient à des nombres de tailles très différentes et qui doivent tous être supérieurs à 0), l'endroit où l'autre axe coupe celui-ci, le format de nombre de ses étiquettes, ses graduations, la position de ses étiquettes et l'angle de rotation des étiquettes. Seul un axe numérique possède des bornes et des unités.
- **Déplacer le graphique…** dans l'onglet **Création de graphique** demande où le graphique doit aller. Choisissez **Nouvelle feuille** et saisissez un nom : le graphique reçoit une feuille de graphique qui lui est propre, une feuille qui ne contient qu'un graphique, sans cellules. Choisissez **Objet dans** et sélectionnez une feuille de calcul : le graphique est placé sur cette feuille. Un graphique situé sur une feuille de graphique ne peut être déplacé que vers une feuille de calcul, et la feuille de graphique qu'il quitte est supprimée. Le graphique lit toujours les cellules qu'il lisait. Sur une feuille de calcul, il conserve sa taille et sa place sur la feuille, même là où les lignes et les colonnes sont plus hautes ou plus larges, et il se déplace et se redimensionne avec les cellules comme auparavant. Les feuilles masquées ne sont pas proposées. Cette opération constitue une seule étape d'**Annuler**, même après un enregistrement, sauf si une feuille a été supprimée par elle. Un fichier au format `.ods` ne peut pas contenir de feuille de graphique : enregistrez donc une feuille de graphique au format `.xlsx`.
- **Titre du graphique** et **Titres des axes** (dans le menu **+**) proposent aussi **Texte du titre…**. Saisissez le texte souhaité. Ou saisissez un signe égal suivi d'une cellule, comme **=Feuil1!A1**, et le titre affiche le contenu de cette cellule. Lorsque la cellule change, le titre change aussi. Le fichier conserve le lien.
- **Enregistrer comme image…** dans l'onglet **Création de graphique** (et dans le menu du clic droit) enregistre le graphique sélectionné dans un fichier image **PNG** ou **JPEG**, deux fois plus grand qu'il ne l'est sur la feuille afin de rester net.
- **Enregistrer comme modèle…** et **Appliquer le modèle…** (dans l'onglet **Création de graphique**, groupe **Type**) conservent l'apparence d'un graphique dans un fichier **.crtx** et la transmettent à un autre graphique. L'apparence comprend le type, les couleurs, les réglages du titre et de la légende, les axes et les polices. Le graphique auquel vous l'appliquez conserve ses propres nombres et les cellules qu'il lit. Un seul **Annuler** supprime de nouveau l'apparence.
- **Modifier le type de graphique** change le type de tout le graphique, ou de chaque série : des colonnes avec une courbe par-dessus, ou une série sur un second axe vertical.

Une référence à des cellules s'écrit par exemple **Feuil1!$B$2:$B$9**, le nom de la feuille étant placé entre apostrophes lorsqu'il contient des espaces.

## Sparklines

Une sparkline est un mini graphique qui tient dans une seule cellule. Elle montre la tendance d'une ligne ou d'une colonne de nombres, juste à côté d'eux.

1. Sélectionnez les nombres. Pour une sparkline par ligne, sélectionnez plusieurs lignes de nombres.
2. Dans l'onglet **Insertion**, dans le groupe **Sparklines**, choisissez **Courbe**, **Colonne** ou **Gain/perte**. **Courbe** relie les nombres par une ligne. **Colonne** dessine une barre pour chaque nombre. **Gain/perte** dessine une barre vers le haut pour chaque nombre supérieur à zéro et une barre vers le bas pour chaque nombre inférieur, toutes de la même taille, ce qui convient aux résultats qui ne sont que bons ou mauvais.
3. Dans la boîte **Créer des sparklines**, vérifiez la **Plage de données** (les cellules que lisent les sparklines) et saisissez la **Plage d'emplacement** (les cellules où elles se placent). Choisissez une cellule d'emplacement pour chaque ligne de données, ou pour chaque colonne de données. Si vous choisissez une seule cellule, la sparkline lit toutes les données.

Les sparklines créées en une seule opération forment un groupe et partagent la même apparence. Cliquez sur une cellule contenant une sparkline : l'onglet **Outils de sparkline** apparaît.

- **Modifier les données** modifie les données et l'emplacement de tout le groupe, ou les données de celle sur laquelle vous avez cliqué. Il précise aussi quoi faire des cellules vides (laisser un espace, les traiter comme zéro ou relier les points de part et d'autre) et s'il faut lire les cellules des lignes et colonnes masquées.
- **Type** permet de passer de courbe à colonne et à gain/perte.
- **Affichage** met en évidence des points : le **Point haut**, le **Point bas**, les **Points négatifs**, le **Premier point**, le **Dernier point» et, pour une courbe, les **Marqueurs** sur chaque point.
- **Styles** donne au groupe un jeu de couleurs. **Couleur de sparkline** règle la couleur de la ligne ou des barres, et l'**Épaisseur** (l'épaisseur de la ligne, en points). **Couleur du marqueur** règle la couleur de chaque type de point mis en évidence.
- **Axe** affiche une ligne à zéro, trace les nombres de droite à gauche et règle les valeurs minimale et maximale de l'axe vertical. **Automatique pour chaque sparkline** ajuste chacune à ses propres nombres. **Identique pour toutes les sparklines** donne au groupe une seule échelle, pour pouvoir les comparer. **Valeur personnalisée** vous permet de saisir un nombre. **Axe des dates…** espace les points selon les dates : saisissez les cellules qui contiennent la date de chaque point, et un point dont la date est plus tardive se place plus loin. Laissez le champ vide pour répartir de nouveau les points uniformément.
- **Grouper** place les sparklines sélectionnées dans un même groupe. **Dissocier** donne à chacune sa propre apparence. **Effacer** supprime les sparklines sélectionnées, ou leurs groupes entiers.

Une sparkline suit ses cellules : si vous insérez une ligne au-dessus des données, elle lit toujours les mêmes nombres. Chaque modification constitue une étape que vous pouvez annuler. Les sparklines sont enregistrées dans le fichier sous une forme que lisent les autres tableurs.

## Insérer une image, une forme ou une zone de texte

Dans l'onglet **Insertion**, dans le groupe **Illustrations**, **Image** ouvre un sélecteur de fichiers. Choisissez un fichier image (PNG, JPEG, GIF, BMP, TIFF ou SVG) : il se place sur la feuille, à la cellule sélectionnée. Une image dont la largeur dépasse 480 points est réduite pour tenir, et conserve ses proportions. Le classeur conserve l'image elle-même. Une image d'un type que le classeur ne peut pas conserver tel quel (un SVG, par exemple) est enregistrée au format PNG, une image ordinaire faite de points. Un fichier qui n'est pas une image, ou qui dépasse 16 Mo, est refusé avec un message. Un fichier WMF ou EMF (les formats d'image que certains anciens logiciels créent à partir de traits et de formes) est dessiné et conservé également au format PNG.

**Icônes** affiche la même fenêtre de petites images au trait que JeffPub et JeffDeck proposent, avec une zone pour les rechercher par nom. Choisissez-en une ou plusieurs : chacune se place sur la feuille comme une image, large et haute de 48 points. Elle est dessinée en gris foncé, à une taille qui reste nette lorsque vous l'agrandissez. Chacune constitue une seule étape que vous pouvez annuler.

**Code-barres** ouvre un court menu à deux choix. **Code-barres** ouvre une fenêtre pour créer un seul code-barres (**Codes-barres à partir des cellules**, ci-dessous, en crée plusieurs) : ISBN de livre (avec ou sans prix), EAN-13, UPC-A, EAN-8, Code 128, Code 39, ou un code QR, qui peut contenir n'importe quel texte, comme une adresse web. Choisissez un **Type**, saisissez le texte ou les chiffres : la fenêtre affiche le code-barres au fur et à mesure de la saisie, et indique en rouge ce qui ne va pas s'il ne peut pas être créé. Cliquez sur **Insérer** : il se place sur la feuille comme une image, en une étape que vous pouvez annuler. Déplacez-le et redimensionnez-le comme n'importe quelle image. Pour le modifier, sélectionnez-le et cliquez de nouveau sur **Code-barres**, changez le texte, puis cliquez sur **Mettre à jour** : le nouveau code-barres prend la place de l'ancien et sa largeur. Lors de l'enregistrement, il est sauvegardé comme image avec un SVG qui reste net, afin que tous les programmes l'affichent ; JeffCrunch conserve ses réglages dans le fichier, de sorte que vous pouvez encore le modifier lorsque vous rouvrez le classeur dans JeffCrunch.

**Code-barres > Codes-barres à partir des cellules** (dans le même menu **Code-barres**) crée un code-barres pour chaque cellule remplie parmi les cellules sélectionnées, par exemple pour une planche d'étiquettes. Sélectionnez les cellules, cliquez sur cette commande, puis choisissez le type et l'apparence dans la même fenêtre. Le texte que vous saisissez là n'est qu'un exemple, car le texte de chaque cellule devient son propre code-barres. Les codes-barres se placent dans une colonne juste à droite des cellules sélectionnées, l'un sous l'autre, en une seule étape que vous pouvez annuler. Une cellule dont le texte ne peut pas être codé (une lettre dans un EAN-13, par exemple) est signalée dans un message et ne reçoit pas de code-barres. Au plus 500 cellules sont traitées à la fois ; sélectionnez-en moins si le programme indique qu'il y en a trop. Chaque code-barres garde la trace de la cellule dont il provient et la suit : lorsque vous modifiez cette cellule, le code-barres est recréé à partir du nouveau texte, à la même place et à la même taille, en une étape distincte que vous pouvez annuler. Le lien suit la cellule lorsque vous insérez ou supprimez des lignes ou des colonnes. Si vous supprimez la cellule, le code-barres conserve le dernier texte qu'il avait. Lors de l'enregistrement, l'image du fichier est créée à partir du texte actuel de la cellule, afin que les autres programmes affichent aussi la valeur courante.

**Formes** ouvre une liste : rectangle, rectangle à coins arrondis, ovale, triangle, losange, flèches, une étoile, un cœur, un nuage, une bulle de dialogue et une ligne. Choisissez-en une : elle se place sur la feuille en bleu clair avec un contour bleu plus foncé. **Zone de texte**, dans le groupe **Texte**, insère une zone vide, sans remplissage ni contour. Cliquez sur **Dessiner une zone de texte** (le bouton principal, ou le premier choix de son menu), puis faites glisser sur la feuille pour dessiner la zone. Un glissement crée une zone de la taille choisie, mais jamais inférieure à 18 points (0,6 cm) de côté, et un simple clic de moins de 4 points donne la taille habituelle. Maintenez **Maj** pour garder la zone carrée. Le pointeur reste une croix jusqu'à ce que vous relâchiez ; **Échap**, ou toute autre commande, repose l'outil. La zone constitue une étape que vous pouvez annuler.

Pour changer le sens du texte d'une zone de texte, sélectionnez-la et cliquez sur **Orientation du texte** dans l'onglet **Format de la forme** : **Horizontal**, **Faire pivoter tout le texte de 90°** ou **Faire pivoter tout le texte de 270°**. Le texte se saisit tourné, comme il s'affiche, et un classeur enregistré au format .xlsx ou .ods conserve ce sens.

Une forme étant sélectionnée, **Styles** dans l'onglet **Format de la forme** lui donne un aspect prêt à l'emploi : un remplissage et un contour assortis, comme le bleu clair avec un contour bleu plus foncé. Les deux dernières options ne conservent que le contour ou que le remplissage. Un clic constitue une étape que vous pouvez annuler. **Remplissage de forme** et **Contour de forme** modifient chaque partie séparément.

## Trouver des images libres en ligne

**Images en ligne**, dans le groupe **Illustrations** de l'onglet **Insertion**, trouve des images libres. Elles proviennent de deux bibliothèques ouvertes : Openverse, qui recherche dans de nombreuses collections, et Wikimedia Commons, la bibliothèque des images de Wikipédia.

1. Cliquez sur **Images en ligne…**. Un volet s'ouvre à droite.
2. Choisissez une bibliothèque, saisissez ce que vous cherchez, et cliquez sur **Rechercher** (ou appuyez sur Entrée).
3. Cliquez sur une image pour voir qui l'a réalisée et sous quelle licence elle est diffusée. Une **licence** indique comment vous pouvez utiliser une image. Cliquez sur le nom de la licence pour la lire.
4. Cliquez sur **Insérer** (ou double-cliquez sur l'image).

**Ce que JeffCrunch envoie.** Vos mots de recherche sont envoyés à la bibliothèque que vous choisissez, et seulement lorsque vous cliquez sur **Rechercher** ou appuyez sur Entrée. JeffCrunch demande ensuite à la bibliothèque les petits aperçus de ce qu'elle a trouvé, ainsi que l'image unique que vous insérez. Comme tout site web, la bibliothèque peut voir l'adresse internet de votre ordinateur, et elle est informée que JeffCrunch fait cette demande. Rien de votre classeur n'est envoyé.

L'image se place sur la feuille comme une image provenant d'un fichier, et le classeur la conserve. Son titre est enregistré comme **texte de remplacement**. La plupart des licences demandent de citer l'auteur. Lorsque **Ajouter un crédit sous l'image** est coché, JeffCrunch ajoute sous l'image une petite zone de texte avec son titre, son auteur et sa licence. Vous pouvez la déplacer, la modifier ou la supprimer. L'image et son crédit s'insèrent en une seule étape, de sorte que **Annuler** les retire ensemble. Vérifiez que la licence convient à l'usage que vous ferez de l'image.

## Ajouter du texte artistique

Le **texte artistique** consiste en mots décoratifs : des lettres colorées, avec un contour, une ombre ou une forme courbe.

1. Cliquez sur **Insertion > Texte > Texte artistique**. Une galerie de styles s'ouvre.
2. Cliquez sur un style. Les mots « Votre texte ici » se placent sur la feuille dans ce style.
3. Pour changer les mots, sélectionnez le texte artistique et cliquez sur **Modifier le texte…** dans l'onglet **Format de la forme**. Une boîte demande les mots, la police, la taille, le gras et l'italique. Une taille plus grande agrandit le texte artistique.
4. Dans l'onglet **Format de la forme**, **Styles de texte artistique** donne un autre style aux lettres, et **Déformation du texte** courbe les mots selon une forme, comme une vague ou une arche. « Simple » les garde droits.

Le texte artistique est une forme du dessin de la feuille, avec sa déformation, son remplissage et son contour : il conserve son aspect lorsque vous enregistrez le classeur au format .xlsx et le rouvrez, dans JeffCrunch ou dans un autre programme. Chaque modification constitue une étape que vous pouvez annuler. Un classeur enregistré au format .ods conserve aussi ses images, formes, zones de texte, groupes, traits et textes artistiques. Le texte artistique est enregistré sous la forme de texte d'habillage que possède OpenDocument, avec ses mots, sa police, son remplissage, son contour et sa déformation, et il redevient du texte artistique lorsque vous rouvrez le fichier .ods. Quelques déformations qu'OpenDocument ne possède pas (Vague 2, Vague 4, et les formes basse et haute de Gonfler et Dégonfler) sont enregistrées comme du texte droit avec les mêmes lettres et le même style. Les graphiques sont aussi conservés dans un fichier .ods : voir plus bas.

## Réduire la taille des images

Une photo prise avec un appareil peut être beaucoup plus grande que ce qu'elle montre sur la feuille, et elle alourdit le fichier. Sélectionnez une image, puis cliquez sur **Compresser les images…** dans le groupe **Ajuster** de l'onglet **Format de l'image**. Choisissez l'usage prévu du classeur : **Impression commerciale (300 ppp)**, **Impression de bureau (220 ppp)** ou **Web (96 ppp)**. Laissez **Appliquer à toutes les images de cette feuille** coché pour réduire toutes les images de la feuille, ou décochez-le pour ne réduire que l'image sélectionnée. **Supprimer les zones rognées des images** retire aussi les parties masquées que vous avez rognées : le rognage disparaît. Une image n'est remplacée que si la nouvelle est plus petite. Sa taille, son emplacement, son nom et son texte de remplacement restent inchangés. **Annuler** (Ctrl+Z) rétablit les images, et le classeur enregistré ne contient que les images réduites. Les images des autres feuilles ne sont pas modifiées : allez sur chaque feuille et recommencez. Une image qui est un dessin (SVG, EMF ou WMF), ou une animation, reste inchangée.

Chaque opération constitue une étape que vous pouvez annuler. Le nouvel objet est sélectionné, de sorte que vous pouvez le faire glisser, le redimensionner, le faire passer devant ou derrière, lui donner un nom et un texte de remplacement dans **Propriétés**, ou appuyer sur **Suppr**.

## Afficher le texte d'une cellule dans une forme ou une zone de texte

Une forme ou une zone de texte peut afficher ce qu'une cellule affiche, et suivre cette cellule. Sélectionnez la forme, cliquez dans la **barre de formule**, saisissez un signe égal suivi d'une cellule, puis appuyez sur **Entrée**.

- `=B2` lie la forme à B2 de cette feuille. `=Feuil2!B3` la lie à une cellule d'une autre feuille. `=Total` la lie à un nom qui désigne une seule cellule.
- La forme affiche alors le texte de la cellule tel que la cellule le montre, avec son format de nombre. Lorsque vous modifiez la cellule, son format, ou une formule dont elle dépend, la forme change aussi. Un texte plus long que ce qu'une cellule peut contenir (32 767 caractères) est tronqué. Toutes les formes liées d'un classeur affichent ensemble au plus 4 Mo de texte : au-delà, une forme affiche son texte tronqué.
- Lorsque la forme est sélectionnée, la barre de formule affiche son lien, par exemple `=Feuil1!$B$2`. Effacez la barre de formule et appuyez sur **Entrée** pour retirer le lien. La forme conserve alors les mots qu'elle affichait.
- Saisir des mots dans la forme retire aussi le lien.
- Un lien désigne une seule cellule. Une plage comme `=A1:B2`, une liste, un calcul ou un nom qui ne désigne pas une cellule est refusé avec un message, et la forme conserve son lien.
- Si la ligne ou la colonne de la cellule est supprimée, ou sa feuille, la forme affiche `#REF!` et conserve le lien, comme le ferait une formule. Annuler rétablit la cellule.
- Chaque lien, et chaque suppression de lien, constitue une étape que vous pouvez annuler.

Le lien est enregistré dans un fichier .xlsx avec le texte que la cellule affichait, de sorte qu'un programme qui ne suit pas les liens affiche tout de même les mots, et JeffCrunch rouvre le fichier avec le lien actif. OpenDocument ne permet pas de lier une zone de texte à une cellule : un classeur enregistré au format .ods conserve donc les mots affichés par la forme à ce moment-là, et non le lien.

## Modifier l'aspect d'une image, d'une forme ou d'une zone de texte

Sélectionnez une image et l'onglet **Format de l'image** apparaît. Sélectionnez une forme, un trait ou une zone de texte et l'onglet **Format de la forme** apparaît. Chaque modification constitue une étape que vous pouvez annuler, et l'objet conserve son nom, son texte de remplacement, son emplacement et son ordre parmi les autres objets.

- **Remplissage de forme** et **Contour de forme** choisissent une couleur, ou aucune. **Style de trait** règle l'épaisseur (l'épaisseur du trait) et les tirets (continu, en tirets ou en pointillés). Une image dispose de **Bordure de l'image**, qui est son contour.
- **Pivoter** fait tourner l'objet d'un quart de tour vers la droite ou vers la gauche, ou le retourne horizontalement ou verticalement.
- **Rogner**, pour une image, la découpe selon une forme de fenêtre : carré, 4 de large sur 3 de haut, 3 de large sur 4 de haut, 16 sur 9, ou 9 sur 16. La découpe est faite uniformément depuis le milieu de ce qui est visible, et l'image est agrandie autant que ce qui reste. **Afficher l'image entière** rétablit ce qui a été rogné. **Remplir** conserve le cadre et découpe l'image pour qu'elle le recouvre entièrement. **Ajuster** affiche l'image entière, et le cadre prend la forme de l'image à l'intérieur de l'ancien. **Rogner selon la forme** (la galerie à côté de **Rogner**) découpe l'image selon une forme comme un ovale, une étoile ou un rectangle à coins arrondis ; la forme est enregistrée avec l'image. Le fichier de l'image n'est pas modifié : la partie découpée est seulement masquée.
- **Modifier le texte** ne concerne que les formes et les zones de texte. Saisissez les mots, puis choisissez leur disposition horizontale et verticale, leur taille, le gras ou non, et s'ils se renvoient à la forme ou restent sur une seule ligne. Pour un texte artistique, la boîte demande les mots, la police, la taille, le gras et l'italique.
- **Volet Sélection** ouvre la liste de tous les objets de la feuille par leur nom, celui qui est au premier plan en premier. Cliquez sur un nom pour sélectionner cet objet, ce qui est utile lorsque des objets se masquent les uns les autres. **Avancer** et **Reculer** le déplacent dans la liste.
- **Propriétés** règle la taille, le nom, le texte de remplacement, et ce que l'objet fait lorsque les cellules sont déplacées ou agrandies.

## Couper, copier et coller un objet

Une image ou une forme se copie et se colle comme un graphique. Une forme qui utilise autre chose qu'une image du classeur, comme un lien vers un fichier, ne peut pas être copiée, et JeffCrunch vous indique pourquoi.

Copiez un graphique et collez-le dans le même classeur, sur n'importe quelle feuille : la copie lit les mêmes cellules que l'original. Collé dans un autre classeur, il conserve les nombres qu'il affichait, mais ne lit plus de cellules, car les cellules qu'il lirait là-bas ne sont pas celles d'où il provient.

## Sélectionner plusieurs objets, les grouper et les aligner

Cliquez sur un objet, puis maintenez Maj ou Ctrl et cliquez sur un autre pour le sélectionner aussi. Cliquez de nouveau sur un objet sélectionné en maintenant Maj ou Ctrl pour le désélectionner. Les objets sélectionnés reçoivent chacun un cadre en pointillés. Vous pouvez aussi choisir **Sélectionner des objets** dans l'onglet **Format de la forme**, puis faire glisser une boîte sur des cellules vides : chaque objet entièrement inclus dans la boîte est sélectionné. Appuyez sur Échap, ou choisissez de nouveau **Sélectionner des objets**, pour revenir à la sélection de cellules. Appuyez sur Suppr pour supprimer en une seule étape tous les objets sélectionnés.

Lorsque deux objets ou plus sont sélectionnés, l'onglet **Format de la forme** propose ces commandes dans le groupe **Organiser** :

- **Grouper** réunit les formes, images et traits sélectionnés en un seul objet. Il se déplace et se redimensionne d'un bloc, et le fichier le conserve comme un groupe. Un graphique ne peut pas être placé dans un groupe.
- **Dissocier** sépare un groupe sélectionné. Chaque élément retrouve sa place sur la feuille.
- **Aligner** aligne les objets sélectionnés : **Aligner à gauche**, **Centrer**, **Aligner à droite**, **Aligner en haut**, **Aligner au milieu** et **Aligner en bas**. Ils s'alignent sur le bord ou le centre de l'ensemble. **Distribuer horizontalement** et **Distribuer verticalement** répartissent uniformément trois objets ou plus entre les deux extrêmes.

Grouper, Dissocier et chaque choix d'alignement constituent une étape que vous pouvez annuler.

## Nommer un objet

Lorsqu'un objet est sélectionné, la zone Nom affiche son nom. Saisissez-y un nouveau nom et appuyez sur **Entrée**. **Annuler** rétablit l'ancien nom, et le nouveau nom est enregistré dans le fichier.

## Supprimer un objet

Appuyez sur **Suppr** lorsque l'objet est sélectionné. **Annuler** le rétablit.

## Ce qui est enregistré

Tout ce qui précède est enregistré dans le fichier sous une forme que lisent les autres tableurs : un graphique créé ici s'ouvre donc de la même manière ailleurs. Un graphique provenant d'un autre programme et comportant des éléments que JeffCrunch ne peut pas conserver lorsqu'il réécrit un graphique (barres d'erreur, graphique en trois dimensions) peut toujours être déplacé, redimensionné et supprimé, mais ses éléments et ses données ne sont pas modifiés ici.

## Insérer un objet

Un **objet** contient son propre fichier à l'intérieur du classeur, afin que l'application qui l'a créé puisse le rouvrir. Il affiche une image de lui-même, mais ce n'est pas une image : il reste un objet lorsque vous le déplacez, le redimensionnez, le copiez ou enregistrez le classeur.

1. Cliquez sur **Insertion**, puis sur **Objet**, et choisissez **À partir d'une plage JeffCrunch**, **À partir d'un graphique JeffCrunch** ou **À partir d'une page JeffWrite**.
2. Choisissez le fichier, puis la plage ou le numéro de page si la boîte en demande une. Si vous avez copié l'élément dans l'autre application, la boîte propose de l'utiliser.
3. L'objet se place à la cellule sélectionnée. Faites-le glisser pour le déplacer, et faites glisser une poignée pour le redimensionner. Une plage, un graphique ou une page que vous redimensionnez est redessiné à sa nouvelle taille, afin de rester net. Appuyez sur **Suppr** pour le retirer. Chacune de ces actions constitue une étape d'Annuler.

Pour insérer ce que vous avez copié dans une autre application, cliquez sur la flèche sous **Coller** et choisissez **En tant qu'objet**.

**Double-cliquez** sur un objet pour l'ouvrir dans l'application qui l'a créé. Enregistrez-le là-bas, et l'objet ici affiche la modification en une étape d'Annuler. Une cellule dans laquelle vous étiez en train de saisir reste inchangée. Une copie que vous collez est un objet indépendant : modifier l'une ne modifie pas l'autre. Il en va de même pour une feuille que vous copiez : ses objets sont des copies, et la copie de la feuille les conserve.

**Fichier**, puis **Infos**, puis **Objets** liste tous les objets du classeur. Sélectionnez-en un pour l'ouvrir, le transformer en simple image ou le supprimer.

Un objet créé par un autre programme apparaît comme l'image que son fichier conserve, quel que soit son type d'image. Vous pouvez le sélectionner, le déplacer, le redimensionner, l'aligner avec d'autres objets et le supprimer ; il se déplace avec les cellules comme vos propres objets. Vous ne pouvez pas le modifier ici, le placer dans un groupe, ni changer sa place dans l'ordre des objets. Il est enregistré tel que le fichier le contenait, seule sa position étant modifiée si vous l'avez déplacé. Un objet contenu dans un fichier .ods fonctionne de la même manière. Enregistrés au format .xlsx ou .ods, vos objets sont conformes aux formats standard que lisent les autres programmes. Lorsqu'un fichier .ods est écrit pour un objet dont l'image ne peut pas être dessinée, l'objet est conservé sans image.

## Diagrammes créés par d'autres programmes

Un **diagramme** est une image de boîtes et de flèches, par exemple un processus, créée par un autre programme. JeffCrunch affiche un diagramme à partir de l'image qu'en conserve le fichier. Cliquez dessus pour le sélectionner, et faites-le glisser pour le déplacer. Lorsque vous enregistrez le fichier, le diagramme est réécrit exactement tel qu'il était, afin que le programme qui l'a créé puisse toujours le modifier.

JeffCrunch ne peut pas encore modifier le contenu d'un diagramme, et un diagramme ne peut être ni copié ni placé dans un groupe. Si le fichier ne conserve aucune image d'un diagramme, JeffCrunch affiche une simple case grise indiquant qu'il s'agit d'un diagramme.

## Graphiques dans les fichiers .ods

Un classeur enregistré au format .ods conserve ses graphiques : colonnes, barres, courbes, aires, secteurs, anneaux et nuages de points, chacun avec ses séries et les cellules qu'elles lisent (sur cette feuille ou sur une autre), ses catégories, son titre, sa légende, ses axes (titres, valeurs minimale et maximale, format de nombre), ses étiquettes de données, ses couleurs et son emplacement. Un fichier .ods créé par un autre programme s'ouvre avec ses graphiques, dessinés à partir des cellules qu'ils lisent. Ce qu'un graphique .ods ne peut pas accueillir est omis et signalé lors de l'enregistrement : courbes de tendance, barres d'erreur, axes secondaires et apparences particulières d'un seul point. Les graphiques d'un type que JeffCrunch ne dessine pas (graphiques boursiers et radar) s'ouvrent sous forme de colonnes de leurs valeurs.

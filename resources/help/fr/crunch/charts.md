# Graphiques, sparklines, images et formes

<!-- keywords: boutons de champ, boutons de champ de graphique croisé dynamique, grouper, dissocier, aligner, aligner à gauche, distribuer, sélectionner des objets, plusieurs formes, sélectionner plusieurs, cadre de sélection, filtres de graphique, masquer une série, graphiques recommandés, suggestions de graphiques, graphique, diagramme, histogramme, colonne, barre, courbe, secteurs, aires, nuage de points, combiné, légende, titre, quadrillage, étiquettes de données, données du graphique, type de graphique, apparence, dispositions, déplacer, redimensionner, image, forme, objet, série, courbe de tendance, tendance, équation, R², second axe, sparkline, sparklines, axe des dates, gain/perte, mini-graphique, marqueurs, nom, texte de remplacement, échelle logarithmique, enregistrer comme image, modèle, crtx, texte du titre, repère de légende, volet Sélection, remplissage de forme, contour de forme, pivoter, retourner, rogner, modifier le texte, bordure d'image, insérer une image, icônes, icône, wmf, emf, styles de forme, styles, insérer une forme, zone de texte, formes, flèche, ovale, rectangle, étoile, diagramme, diagrammes, images en ligne, images libres, openverse, wikimedia commons, licence, crédit, texte artistique, wordart, déformation, compresser les images, réduire la taille des images, taille du fichier, ppp, supprimer les zones rognées, code-barres, code QR -->

Un graphique est une image de nombres qui reste liée à eux : si vous modifiez un nombre, le graphique change avec lui. Les images et les formes se placent au-dessus des cellules de la même manière, mais elles ne sont liées à aucun nombre.

## Créer un graphique

1. Sélectionnez les nombres à représenter. Si la première ligne contient des noms et la première colonne des libellés, sélectionnez-les aussi. JeffCrunch examine la première ligne et la première colonne et les utilise comme noms et libellés lorsqu'il s'agit de mots (ou de dates, pour les libellés).
2. Dans l'onglet **Insertion**, dans **Graphiques**, choisissez un type : **Histogramme**, **Barres**, **Courbes**, **Secteurs**, **Aires**, **Nuage de points** ou **Combiné**. La petite flèche à côté d'un type donne ses variantes, comme les histogrammes empilés, une courbe avec marqueurs, ou un nuage de points, de points reliés ou de courbes lissées.
3. Pour voir d'abord chaque type, choisissez **Autres graphiques**. La liste à gauche présente chaque type dessiné à partir de vos cellules. **Intervertir les lignes et les colonnes** inverse le sens des séries.

Vous ne savez pas quel type choisir ? Sélectionnez vos cellules et choisissez **Graphiques recommandés** dans l'onglet **Insertion**. JeffCrunch examine les cellules et propose quelques types adaptés, du meilleur au moins bon. Par exemple, des dates en colonne suggèrent une courbe, et une colonne de quelques nombres positifs accompagnés de libellés suggère des secteurs. Chaque type est dessiné à partir de vos cellules. Choisissez-en un, puis **OK**.

Si vous ne sélectionnez qu'une seule cellule, JeffCrunch représente le bloc de cellules autour d'elle qui contient des données.

Les séries sont disposées en colonnes lorsque la sélection compte au moins autant de lignes que de colonnes, et en lignes lorsqu'elle compte plus de colonnes que de lignes. Un graphique en secteurs ne contient qu'une série. Un nuage de points prend ses valeurs horizontales dans la première colonne.

Exemple : vous sélectionnez A1:C4, où A2:A4 contiennent T1, T2 et T3, B1 et C1 contiennent Est et Ouest, et le reste sont des nombres. Le graphique comporte deux séries, Est et Ouest, et trois groupes de colonnes, T1 à T3.

## Masquer une série provisoirement

Sélectionnez un graphique. Trois petits boutons apparaissent à côté. Celui qui représente un entonnoir est **Filtres de graphique**. Il affiche les séries du graphique, chacune avec une case à cocher. Décochez une case pour retirer cette série du graphique. Cochez-la de nouveau pour la faire réapparaître à sa place. **Annuler** fonctionne aussi. Un graphique conserve toujours au moins une série. Une série retirée reste dans le fichier lors de l'enregistrement et reste masquée à l'ouverture du fichier, rien n'est donc perdu. Cochez-la pour la faire réapparaître. (Un graphique en anneau affiche toutes les séries à son ouverture.)

## Déplacer et redimensionner

Cliquez sur un graphique, une image ou une forme pour le sélectionner. Faites-le glisser pour le déplacer, ou faites glisser l'un des huit carrés de son bord pour le redimensionner.

- Maintenez **Alt** pendant le glissement pour aligner un bord sur le bord d'une cellule.
- Maintenez **Maj** pendant que vous faites glisser un coin pour conserver les proportions.
- Les touches fléchées le déplacent de quelques pixels à la fois. **Ctrl** avec une flèche le déplace d'un pixel, **Maj** avec une flèche l'agrandit ou le réduit, et **Alt** avec une flèche va jusqu'au bord de cellule suivant.
- **Échap** pendant le glissement le remet à sa place.

Chaque déplacement ou redimensionnement constitue une étape que vous pouvez annuler. Faites un clic droit sur l'objet et choisissez **Propriétés** pour saisir son **Nom** (chaque objet d'une feuille doit en avoir un distinct) et son **Texte de remplacement** (une courte phrase qui indique à une personne qui ne voit pas l'objet ce qu'il représente), pour saisir sa taille, et pour choisir son comportement lorsque vous insérez, supprimez ou redimensionnez des cellules : se déplacer et se redimensionner avec les cellules, se déplacer avec elles sans changer de taille, ou rester en place. Ce choix est enregistré dans le fichier. **Imprimer cet objet**, dans la même boîte, masque l'objet sur les pages imprimées (il reste visible à l'écran) ; le fichier conserve aussi ce choix.

## Modifier ce qu'affiche un graphique

Lorsqu'un graphique est sélectionné, deux boutons apparaissent à côté et un onglet **Graphique** s'affiche dans le ruban.

- Le bouton **+** (et **Éléments du graphique** dans l'onglet) active ou désactive le titre du graphique, les axes, les titres des axes, le quadrillage, la légende et les étiquettes de données, et indique où placer la légende et les étiquettes.
- Le bouton pinceau (et **Couleurs** et **Styles** dans l'onglet) donne au graphique un des jeux de couleurs et un des styles de JeffCrunch. **Dispositions** règle plusieurs éléments à la fois.
- **Données du graphique** affiche les séries du graphique. Vous pouvez en ajouter, les modifier, les supprimer et les réordonner, changer les cellules que chacune lit, définir les cellules des libellés, intervertir les lignes et les colonnes, et choisir la façon de représenter les cellules vides et les lignes et colonnes masquées.
- **Boutons de champ** (dans le menu **+**, uniquement pour un graphique croisé dynamique) affiche de petits boutons portant les noms des champs du tableau croisé dynamique que suit le graphique : les filtres du rapport sous le titre, et les lignes, valeurs et colonnes en bas. Activez ou désactivez chaque type, ou choisissez **Tout afficher** ou **Tout masquer**. Un nouveau graphique croisé dynamique les affiche.
- **Barres d'erreur** (dans le menu **+**, pour un graphique en colonnes, barres, courbes, aires ou nuage de points) trace un petit trait au-dessus et au-dessous de chaque point pour montrer de combien la valeur réelle pourrait différer. Choisissez **Erreur type**, **Pourcentage (5 %)**, **Écart type** ou **Valeur fixe (1)**. Choisissez **Personnalisées** pour utiliser vos propres montants : saisissez les cellules contenant les montants au-dessus de chaque point (par exemple `=Feuil1!$D$2:$D$4`), puis les cellules des montants au-dessous. Laissez-en une vide pour ne tracer les barres que d'un seul côté. Les barres changent lorsque ces cellules changent. Un fichier qui contient ses propres montants pour chaque point les conserve.
- **Courbes** (dans le menu **+**, pour un graphique en courbes) propose **Lignes de chute**, qui descendent du point le plus haut de chaque catégorie jusqu'à l'axe, **Lignes haut-bas**, qui relient le point le plus haut et le point le plus bas de chaque catégorie, et **Barres haut/bas**, qui sont des rectangles entre la première et la dernière série de chaque catégorie. Une barre montante indique que la dernière série est plus haute. Les lignes haut-bas et les barres haut/bas nécessitent au moins deux séries. Les graphiques en aires ont des lignes de chute.
- **Courbe de tendance** (dans le menu **+**) trace une droite ajustée aux nombres de chaque série, pour voir vers où ils tendent. Choisissez **Linéaire** pour une droite, **Exponentielle** pour des nombres qui augmentent d'un pourcentage constant, **Logarithmique** pour des nombres qui montent vite puis se stabilisent, **Polynomiale** pour une courbe avec des inflexions, **Puissance**, ou **Moyenne mobile**, qui trace la moyenne des derniers nombres. **Afficher l'équation** inscrit la formule de la courbe sur le graphique, et **Afficher le R²** indique la qualité de l'ajustement (1 correspond à un ajustement parfait ; 0, à aucun ajustement). La courbe de tendance a sa propre entrée dans la légende.
- Lorsqu'une série est tracée sur un second axe vertical, les menus **Axes** et **Titres des axes** proposent les entrées **Second horizontal** et **Second vertical** pour cette série. Le menu **Légende** propose **Au-dessus du graphique**, qui place la légende sur la zone de tracé au lieu de la mettre à côté.
- **Étiquettes de données** propose aussi **Contenu de l'étiquette** (la valeur, le nom de la catégorie, le nom de la série, le pourcentage et le repère de légende, c'est-à-dire un petit carré de la couleur de la série, combinables à volonté), **Séparateur** (ce qui les sépare : virgule, point-virgule, point, saut de ligne ou espace) et **Format de nombre…** (un code de format, comme 0,0, pour les nombres des étiquettes ; laissez-le vide pour afficher les nombres tels quels). **Valeur à partir des cellules…** affiche le texte de certaines cellules comme étiquettes, une cellule par point. Saisissez les cellules, par exemple =Feuil1!$D$2:$D$4. Les étiquettes changent lorsque les cellules changent. Laissez le champ vide pour les supprimer. Décochez tous les contenus et les étiquettes disparaissent.
- **Format de l'axe horizontal…** et **Format de l'axe vertical…** dans l'onglet **Graphique** ouvrent une boîte pour un seul axe. Elle définit les valeurs minimale et maximale, les unités principales et secondaires (chacune automatique ou un nombre que vous saisissez), si les valeurs sont dans l'ordre inverse, les unités d'affichage (les étiquettes affichent les nombres divisés, par exemple par 1 000 pour des milliers, de sorte que 30 000 s'affiche 30 ; cochez **Afficher le nom des unités sur le graphique** pour imprimer le nom, par exemple Milliers, avec le titre de l'axe), si l'échelle est logarithmique (chaque graduation vaut une base fois la précédente, par exemple 1, 10, 100 et 1 000, ce qui convient à des nombres de tailles très différentes et qui doivent tous être supérieurs à 0), l'endroit où l'autre axe le coupe, le format de nombre de ses étiquettes, ses graduations, la position de ses étiquettes et leur inclinaison. Seul un axe numérique possède des bornes et des unités.
- **Déplacer le graphique…** dans l'onglet **Graphique** demande où placer le graphique. Choisissez **Nouvelle feuille** et saisissez un nom, et le graphique obtient une feuille de graphique qui lui est propre : une feuille qui ne contient que le graphique, sans cellules. Choisissez **Objet dans** et sélectionnez une feuille de calcul, et le graphique est placé sur cette feuille. Un graphique sur une feuille de graphique ne peut être placé que sur une feuille de calcul, et la feuille de graphique qu'il quitte est supprimée. Le graphique lit toujours les cellules qu'il lisait. Sur une feuille de calcul, il conserve sa taille et sa place sur la feuille, même là où les lignes et colonnes sont plus hautes ou plus larges, et se déplace et se redimensionne avec les cellules comme auparavant. Les feuilles masquées ne sont pas proposées. Il s'agit d'une seule étape **Annuler**, même après un enregistrement, sauf si une feuille a été supprimée par cette action. Un fichier au format `.ods` ne peut pas contenir de feuille de graphique : enregistrez donc une feuille de graphique au format `.xlsx`.
- **Titre du graphique** et **Titres des axes** (dans le menu **+**) proposent aussi **Texte du titre…**. Saisissez le texte souhaité. Ou saisissez un signe égal suivi d'une cellule, par exemple **=Feuil1!A1**, et le titre affiche le contenu de cette cellule. Lorsque la cellule change, le titre suit. Le fichier conserve le lien.
- **Enregistrer comme image…** dans l'onglet **Graphique** (et dans le menu contextuel) enregistre le graphique sélectionné dans un fichier image **PNG** ou **JPEG**, deux fois plus grand que sur la feuille afin de rester net.
- **Enregistrer comme modèle…** et **Appliquer le modèle…** (dans l'onglet **Graphique**, groupe **Type**) conservent l'apparence d'un graphique dans un fichier **.crtx** et l'appliquent à un autre graphique. L'apparence comprend le type, les couleurs, les réglages du titre et de la légende, les axes et les polices. Le graphique auquel vous appliquez le modèle conserve ses propres nombres et les cellules qu'il lit. Une seule étape **Annuler** retire de nouveau cette apparence.
- **Type de graphique** modifie le type de tout le graphique, ou de chaque série : des colonnes avec une courbe par-dessus, ou une série tracée sur un second axe vertical.

Une référence à des cellules s'écrit par exemple **Feuil1!$B$2:$B$9**, avec le nom de la feuille entre apostrophes lorsqu'il contient des espaces.

## Sparklines

Une sparkline est un mini-graphique qui tient dans une seule cellule. Elle montre la tendance d'une ligne ou d'une colonne de nombres, juste à côté de ceux-ci.

1. Sélectionnez les nombres. Pour une sparkline par ligne, sélectionnez plusieurs lignes de nombres.
2. Dans l'onglet **Insertion**, dans **Sparklines**, choisissez **Courbe**, **Histogramme** ou **Gain/perte**. Une **courbe** relie les nombres par un trait. Un **histogramme** dessine une barre pour chaque nombre. **Gain/perte** dessine une barre vers le haut pour chaque nombre supérieur à zéro et une barre vers le bas pour chaque nombre inférieur, toutes de même taille, ce qui convient aux résultats qui ne sont que bons ou mauvais.
3. Dans la boîte **Créer des sparklines**, vérifiez la **Plage de données** (les cellules que lisent les sparklines) et saisissez la **Plage d'emplacement** (les cellules qui les accueillent). Choisissez une cellule d'emplacement pour chaque ligne de données, ou pour chaque colonne de données. Si vous choisissez une seule cellule, la sparkline lit l'ensemble des données.

Les sparklines créées en une seule opération forment un groupe et partagent le même aspect. Cliquez sur une cellule contenant une sparkline pour afficher l'onglet **Sparkline**.

- **Modifier les données** modifie les données et l'emplacement de tout le groupe, ou les données de celle sur laquelle vous avez cliqué. Il définit aussi le traitement des cellules vides (laisser un espace, les traiter comme zéro, ou relier les points de part et d'autre) et si l'on lit les cellules des lignes et colonnes masquées.
- **Type** permet de passer de courbe à histogramme ou gain/perte.
- **Afficher** met en évidence des points : le **Point haut**, le **Point bas**, les **Points négatifs**, le **Premier point**, le **Dernier point** et, pour une courbe, des **Marqueurs** sur chaque point.
- **Styles** donne au groupe un jeu de couleurs. **Couleur de sparkline** règle la couleur du trait ou des barres, et l'**Épaisseur** (l'épaisseur du trait, en points). **Couleur du marqueur** règle la couleur de chaque type de point mis en évidence.
- **Axe** affiche un trait à zéro, trace les nombres de droite à gauche, et règle les valeurs minimale et maximale de l'axe vertical. **Automatique pour chaque sparkline** ajuste chacune à ses propres nombres. **Identique pour toutes les sparklines** donne au groupe une seule échelle, pour les comparer. **Valeur personnalisée** vous permet de saisir un nombre. **Axe des dates…** espace les points selon les dates : saisissez les cellules qui contiennent la date de chaque point, et un point dont la date est plus tardive se place plus loin. Laissez le champ vide pour répartir de nouveau les points uniformément.
- **Grouper** place les sparklines sélectionnées dans un même groupe. **Dissocier** donne à chacune son propre aspect. **Effacer** supprime les sparklines sélectionnées, ou leurs groupes entiers.

Une sparkline suit ses cellules : si vous insérez une ligne au-dessus des données, elle lit toujours les mêmes nombres. Chaque modification constitue une étape que vous pouvez annuler. Les sparklines sont enregistrées dans le fichier sous une forme que lisent les autres logiciels de feuille de calcul.

## Insérer une image, une forme ou une zone de texte

Dans l'onglet **Insertion**, dans **Illustrations**, **Image** ouvre un sélecteur de fichiers. Choisissez un fichier image (PNG, JPEG, GIF, BMP, TIFF ou SVG) et il est placé sur la feuille, dans la cellule sélectionnée. Une image dont la largeur dépasse 480 pixels est réduite pour tenir et conserve ses proportions. Le classeur conserve l'image elle-même. Une image d'un type que le classeur ne peut pas conserver tel quel (un SVG, par exemple) est conservée au format PNG, une image ordinaire faite de points. Un fichier qui n'est pas une image, ou dont la taille dépasse 16 Mo, est refusé avec un message. Un fichier WMF ou EMF (les formats d'image que certains anciens logiciels créent à partir de traits et de formes) est également dessiné et conservé au format PNG.

**Icônes** affiche la même fenêtre de petites images au trait que JeffPub et JeffDeck, avec une zone de recherche par nom. Choisissez-en une ou plusieurs : chacune est placée sur la feuille sous forme d'image, de 48 points de large et de haut. Elle est dessinée en gris foncé, à une taille qui reste nette lorsque vous l'agrandissez. Chacune constitue une seule étape que vous pouvez annuler.

**Code-barres** ouvre un court menu avec deux choix. **Code-barres** ouvre une fenêtre pour créer un seul code-barres (**Codes-barres à partir des cellules**, ci-dessous, en crée plusieurs) : ISBN de livre (avec ou sans prix), EAN-13, UPC-A, EAN-8, Code 128, Code 39, ou un code QR, qui peut contenir n'importe quel texte, comme une adresse web. Choisissez un **Type**, saisissez le texte ou les chiffres, et la fenêtre affiche le code-barres au fur et à mesure de la saisie ; si celui-ci ne peut pas être créé, elle indique en rouge le problème. Cliquez sur **Insérer** : il est placé sur la feuille sous forme d'image, en une étape que vous pouvez annuler. Déplacez-le et redimensionnez-le comme toute image. Pour le modifier, sélectionnez-le et cliquez de nouveau sur **Code-barres**, changez le texte, puis cliquez sur **Mettre à jour** ; le nouveau code-barres prend la place et la largeur de l'ancien. Lors de l'enregistrement, il est conservé sous forme d'image accompagnée d'un SVG qui reste net, de sorte que tous les logiciels l'affichent ; JeffCrunch conserve ses réglages dans le fichier, si bien qu'il reste modifiable lorsque vous rouvrez le classeur dans JeffCrunch.

**Code-barres > Codes-barres à partir des cellules** (dans le même menu **Code-barres**) crée un code-barres pour chaque cellule remplie des cellules sélectionnées, pour une planche d'étiquettes. Sélectionnez les cellules, cliquez sur cette commande, puis choisissez le type et l'apparence dans la même fenêtre. Ce que vous saisissez là n'est qu'un exemple, car le texte de chaque cellule devient son propre code-barres. Les codes-barres sont placés dans une colonne juste à droite des cellules sélectionnées, l'un sous l'autre, en une seule étape que vous pouvez annuler. Une cellule dont le texte ne peut pas être codé (une lettre dans un EAN-13, par exemple) est signalée dans un message et ne reçoit aucun code-barres. Au plus 500 cellules sont traitées à la fois ; sélectionnez-en moins si le logiciel indique qu'il y en a trop. Chaque code-barres mémorise la cellule dont il provient et la suit : lorsque vous modifiez cette cellule, le code-barres est recréé à partir du nouveau texte, au même endroit et à la même taille, en une étape distincte que vous pouvez annuler. Le lien suit la cellule lorsque vous insérez ou supprimez des lignes ou des colonnes. Si vous supprimez la cellule, le code-barres conserve le dernier texte qu'il avait. Lors de l'enregistrement, l'image du fichier est créée à partir du texte actuel de la cellule, afin que les autres logiciels affichent aussi la valeur à jour.

**Formes** ouvre une liste : rectangle, rectangle à coins arrondis, ovale, triangle, losange, flèches, une étoile, un cœur, un nuage, une bulle de dialogue et une ligne. Choisissez-en une : elle est placée sur la feuille en bleu clair, avec un contour bleu plus foncé. **Zone de texte**, dans le groupe **Texte**, insère une zone vide, sans remplissage ni bordure.

Une forme étant sélectionnée, **Styles** dans l'onglet **Format de la forme** lui donne un aspect prêt à l'emploi : un remplissage et un contour assortis, comme un bleu clair avec un contour bleu foncé. Les deux derniers ne conservent que le contour ou que le remplissage. Un clic équivaut à une étape que vous pouvez annuler. **Remplissage de forme** et **Contour de forme** modifient chaque partie séparément.

## Trouver des images libres en ligne

**Images en ligne**, dans le groupe **Illustrations** de l'onglet **Insertion**, trouve des images libres. Elles proviennent de deux bibliothèques ouvertes : Openverse, qui recherche dans de nombreuses collections, et Wikimedia Commons, la bibliothèque des images de Wikipédia.

1. Cliquez sur **Images en ligne…**. Un volet s'ouvre à droite.
2. Choisissez une bibliothèque, saisissez ce que vous cherchez, puis cliquez sur **Rechercher** (ou appuyez sur Entrée).
3. Cliquez sur une image pour voir qui en est l'auteur et sous quelle licence elle est diffusée. Une **licence** indique comment vous pouvez utiliser une image. Cliquez sur le nom de la licence pour la lire.
4. Cliquez sur **Insérer** (ou double-cliquez sur l'image).

**Ce que JeffCrunch envoie.** Les mots de votre recherche sont envoyés à la bibliothèque que vous avez choisie, et seulement lorsque vous cliquez sur **Rechercher** ou appuyez sur Entrée. JeffCrunch demande ensuite à la bibliothèque les petits aperçus de ce qu'elle a trouvé, puis l'image unique que vous insérez. Comme toute page web, la bibliothèque peut voir l'adresse internet de votre ordinateur, et elle est informée que JeffCrunch fait la demande. Rien du contenu de votre classeur n'est envoyé.

L'image est placée sur la feuille comme une image provenant d'un fichier, et le classeur la conserve. Son titre est enregistré comme **texte de remplacement**. La plupart des licences demandent de citer l'auteur. Lorsque **Ajouter un crédit sous l'image** est coché, JeffCrunch ajoute sous l'image une petite zone de texte portant son titre, son auteur et sa licence. Vous pouvez la déplacer, la modifier ou la supprimer. L'image et son crédit sont insérés en une seule étape, de sorte qu'**Annuler** retire les deux. Vérifiez que la licence convient à l'usage que vous ferez de l'image.

## Ajouter du texte artistique

**Texte artistique** désigne des mots décorés : des lettres colorées, avec un contour, une ombre ou une forme courbe.

1. Cliquez sur **Insertion > Texte > Texte artistique**. Une galerie de styles s'ouvre.
2. Cliquez sur un style. Les mots « Votre texte ici » apparaissent sur la feuille dans ce style.
3. Pour modifier le texte, sélectionnez le texte artistique et cliquez sur **Modifier le texte…** dans l'onglet **Format de la forme**. Une boîte demande les mots, la police, la taille, le gras et l'italique. Une taille plus grande agrandit le texte artistique.
4. Dans l'onglet **Format de la forme**, **Styles de texte artistique** donne un autre style aux lettres, et **Déformation du texte** courbe les mots selon une forme, comme une vague ou une arche. « Aucune » les laisse droits.

Le texte artistique est une forme du dessin de la feuille, avec sa déformation, son remplissage et son contour : il garde son aspect lorsque vous enregistrez le classeur au format .xlsx puis le rouvrez, dans JeffCrunch ou dans un autre logiciel. Chaque modification constitue une étape que vous pouvez annuler. Un classeur enregistré au format .ods conserve aussi ses images, formes, zones de texte, groupes, traits et textes artistiques. Le texte artistique est enregistré sous la forme de texte d'effet que possède OpenDocument, avec ses mots, sa police, son remplissage, son contour et sa déformation, et redevient du texte artistique lorsque vous rouvrez le fichier .ods. Quelques déformations pour lesquelles OpenDocument n'a pas de forme (Vague 2, Vague 4, ainsi que les formes basse et haute de Gonfler et Dégonfler) sont enregistrées sous forme de mots droits, avec les mêmes lettres et le même style. Les graphiques sont aussi conservés dans un fichier .ods : voir ci-dessous.

## Réduire la taille des images

Une photo prise avec un appareil peut être beaucoup plus grande que l'affichage sur la feuille, et elle alourdit le fichier. Cliquez sur une image, puis sur **Compresser les images…** dans le groupe **Ajuster** de l'onglet **Format de l'image**. Choisissez l'usage prévu du classeur : **Impression commerciale (300 ppp)**, **Impression de bureau (220 ppp)** ou **Web (96 ppp)**. Laissez **Appliquer à toutes les images de cette feuille** coché pour réduire toutes les images de la feuille, ou décochez-le pour ne réduire que l'image sélectionnée. **Supprimer les zones rognées des images** retire aussi les parties masquées que vous avez rognées : le rognage disparaît. Une image n'est remplacée que si la nouvelle est plus petite. Sa taille, son emplacement, son nom et son texte de remplacement restent inchangés. **Annuler** (Ctrl+Z) restitue les images, et le classeur enregistré ne contient que les images réduites. Les images des autres feuilles ne sont pas modifiées : passez sur chaque feuille et recommencez. Une image qui est un dessin (SVG, EMF ou WMF), ou une animation, est laissée telle quelle.

Chaque opération constitue une étape que vous pouvez annuler. Le nouvel objet est sélectionné, de sorte que vous pouvez le faire glisser, le redimensionner, le faire avancer ou reculer, lui donner un nom et un texte de remplacement dans **Propriétés**, ou appuyer sur **Suppr**.

## Modifier l'apparence d'une image, d'une forme ou d'une zone de texte

Sélectionnez une image pour afficher l'onglet **Format de l'image**. Sélectionnez une forme, un trait ou une zone de texte pour afficher l'onglet **Format de la forme**. Chaque modification constitue une étape que vous pouvez annuler, et l'objet conserve son nom, son texte de remplacement, sa place et son ordre parmi les autres objets.

- **Remplissage de forme** et **Contour de forme** choisissent une couleur, ou aucune. **Style de trait** règle l'épaisseur (l'épaisseur du trait) et les tirets (continu, tireté ou pointillé). Une image dispose d'une **Bordure de l'image**, qui est son contour.
- **Pivoter** fait tourner l'objet d'un quart de tour vers la droite ou vers la gauche, ou le retourne horizontalement ou verticalement.
- **Rogner**, pour une image, la découpe selon une forme de fenêtre : carré, 4 de large sur 3 de haut, 3 de large sur 4 de haut, 16 sur 9, ou 9 sur 16. La découpe est faite uniformément à partir du centre de ce qui est visible, et l'image est agrandie autant que possible pour occuper ce qui reste. **Afficher l'image entière** rétablit ce qui a été coupé. Le fichier image n'est pas modifié : la partie rognée est seulement masquée.
- **Modifier le texte** concerne les formes et les zones de texte. Saisissez le texte, puis choisissez sa disposition horizontale et verticale, sa taille, le gras ou le normal, et s'il se renvoie à la ligne dans la forme ou reste sur une seule ligne. Pour un texte artistique, la boîte demande plutôt les mots, la police, la taille, le gras et l'italique.
- **Volet Sélection** ouvre la liste de tous les objets de la feuille par leur nom, celui du premier plan en tête. Cliquez sur un nom pour sélectionner cet objet, ce qui est utile lorsque des objets se cachent les uns les autres. **Avancer** et **Reculer** le déplacent dans la liste.
- **Propriétés** définit la taille, le nom, le texte de remplacement et le comportement de l'objet lorsque les cellules sont déplacées ou agrandies.

## Couper, copier et coller un objet

Une image ou une forme se copie et se colle comme un graphique. Une forme qui utilise autre chose qu'une image du classeur, comme un lien vers un fichier, ne peut pas être copiée, et JeffCrunch indique la raison.

Copiez un graphique et collez-le dans le même classeur, sur n'importe quelle feuille : la copie lit les mêmes cellules que l'original. Collé dans un autre classeur, il conserve les nombres qu'il affichait mais ne lit plus de cellules, car les cellules qu'il lirait là-bas ne sont pas celles d'origine.

## Sélectionner plusieurs objets, les grouper et les aligner

Cliquez sur un objet, puis maintenez Maj ou Ctrl et cliquez sur un autre pour le sélectionner aussi. Cliquez de nouveau sur un objet sélectionné avec Maj ou Ctrl pour le désélectionner. Les objets sélectionnés reçoivent chacun un cadre en pointillés. Vous pouvez aussi choisir **Sélectionner des objets** dans l'onglet **Format de la forme** et faire glisser une zone sur des cellules vides : chaque objet entièrement contenu dans la zone est sélectionné. Appuyez sur Échap, ou choisissez de nouveau **Sélectionner des objets**, pour revenir à la sélection de cellules. Appuyez sur Suppr pour supprimer en une seule étape tous les objets sélectionnés.

Lorsque deux objets ou plus sont sélectionnés, l'onglet **Format de la forme** propose les commandes suivantes dans le groupe **Organiser** :

- **Grouper** réunit les formes, images et traits sélectionnés en un seul objet. Il se déplace et se redimensionne d'un bloc, et le fichier le conserve comme groupe. Un graphique ne peut pas être placé dans un groupe.
- **Dissocier** sépare un groupe sélectionné. Chaque élément retrouve sa place sur la feuille.
- **Aligner** aligne les objets sélectionnés : **Aligner à gauche**, **Centrer**, **Aligner à droite**, **Aligner en haut**, **Aligner au milieu** et **Aligner en bas**. Ils s'alignent sur le bord ou le milieu de l'ensemble. **Distribuer horizontalement** et **Distribuer verticalement** répartissent uniformément trois objets ou plus entre les deux situés aux extrémités.

Grouper, dissocier et chaque choix d'alignement constituent une étape que vous pouvez annuler.

## Le nommer

Lorsqu'un objet est sélectionné, la zone Nom affiche son nom. Saisissez-y un nouveau nom et appuyez sur **Entrée**. Annuler rétablit l'ancien nom, et le nouveau nom est enregistré dans le fichier.

## Le supprimer

Appuyez sur **Suppr** lorsque l'objet est sélectionné. Annuler le restitue.

## Ce qui est enregistré

Tout ce qui précède est enregistré dans le fichier sous une forme que lisent les autres logiciels de feuille de calcul : un graphique créé ici s'ouvre donc de la même manière ailleurs. Un graphique provenant d'un autre logiciel et contenant des éléments que JeffCrunch ne peut pas conserver lorsqu'il réécrit un graphique (barres d'erreur, graphique en trois dimensions) peut encore être déplacé, redimensionné et supprimé, mais ses éléments et ses données ne sont pas modifiés ici.

## Schémas provenant d'autres logiciels

Un **schéma** est une image de boîtes et de flèches, par exemple un processus, créée par un autre logiciel. JeffCrunch affiche un schéma à partir de l'image qu'en conserve le fichier à côté de lui. Cliquez dessus pour le sélectionner, et faites-le glisser pour le déplacer. Lors de l'enregistrement du fichier, le schéma est réécrit exactement tel quel, de sorte que le logiciel qui l'a créé peut encore le modifier.

JeffCrunch ne peut pas encore modifier le contenu d'un schéma, et un schéma ne peut être ni copié ni placé dans un groupe. Si le fichier ne conserve aucune image d'un schéma, JeffCrunch affiche une simple case grise indiquant qu'il s'agit d'un schéma.

## Graphiques dans les fichiers .ods

Un classeur enregistré au format .ods conserve ses graphiques : histogrammes, barres, courbes, aires, secteurs, anneaux et nuages de points, chacun avec ses séries et les cellules qu'elles lisent (sur cette feuille ou une autre), ses catégories, son titre, sa légende, ses axes (titres, valeurs minimale et maximale, format de nombre), ses étiquettes de données, ses couleurs et son emplacement. Un fichier .ods créé par un autre logiciel s'ouvre avec ses graphiques, dessinés à partir des cellules qu'ils lisent. Ce qu'un graphique .ods ne peut pas contenir est omis et signalé lors de l'enregistrement : courbes de tendance, barres d'erreur, axes secondaires et les aspects particuliers d'un seul point. Les graphiques de types que JeffCrunch ne dessine pas (graphiques boursiers et radar) s'ouvrent sous forme de colonnes de leurs valeurs.

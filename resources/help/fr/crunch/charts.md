# Graphiques, sparklines, images et formes

<!-- keywords: boutons de champ, boutons de champ de graphique croisé dynamique, grouper, dissocier, aligner, aligner à gauche, distribuer, sélectionner des objets, plusieurs formes, choisir plusieurs objets, zone de sélection, filtres du graphique, masquer une série, graphiques recommandés, suggestions de graphiques, graphique, diagramme, colonne, barre, courbe, secteurs, aires, nuage de points, combiné, légende, titre, quadrillage, étiquettes de données, données du graphique, type de graphique, styles, dispositions, déplacer, redimensionner, image, forme, objet, série, courbe de tendance, équation, R carré, deuxième axe, sparkline, sparklines, axe des dates, gain/perte, mini-graphique, marqueurs, nom, texte de remplacement, échelle logarithmique, enregistrer comme image, modèle, crtx, texte du titre, clé de légende, volet Sélection, remplissage de forme, contour de forme, pivoter, retourner, rogner, modifier le texte, contour d'image, insérer une image, icônes, icône, wmf, emf, styles de forme, insérer une forme, zone de texte, formes, flèche, ovale, rectangle, étoile, diagramme SmartArt, images en ligne, images libres, openverse, wikimedia commons, licence, crédit, texte artistique, wordart, déformation, compresser les images, réduire la taille des images, taille du fichier, ppp, supprimer les zones rognées, code-barres, code QR, lier une forme à une cellule, forme affichant une cellule, zone de texte affichant une cellule, valeur de cellule dans une forme -->

Un graphique est une représentation des nombres qui reste liée à ceux-ci : si vous modifiez un nombre, le graphique change avec lui. Les images et les formes se placent au-dessus des cellules de la même façon, mais elles ne sont liées à aucun nombre.

## Créer un graphique

1. Sélectionnez les nombres à représenter dans le graphique. Si la première ligne contient des noms et la première colonne des étiquettes, sélectionnez-les aussi. JeffCrunch examine la première ligne et la première colonne et les utilise comme noms et étiquettes lorsqu'il s'agit de mots (ou de dates, pour les étiquettes).
2. Dans l'onglet **Insertion**, dans le groupe **Graphiques**, choisissez un type : **Colonne**, **Barre**, **Courbe**, **Secteurs**, **Aires**, **Nuage de points** ou **Combiné**. La petite flèche à côté d'un type donne ses variantes, comme les colonnes empilées, une courbe avec marqueurs, ou un nuage de points avec des points isolés, des points reliés ou des courbes lissées.
3. Pour voir d'abord tous les types, choisissez **Autres graphiques**. La liste à gauche montre chaque type dessiné à partir de vos cellules. **Permuter les lignes et les colonnes** inverse le sens des séries.

Vous ne savez pas quel type choisir ? Sélectionnez vos cellules et choisissez **Graphiques recommandés** dans l'onglet **Insertion**. JeffCrunch examine les cellules et propose quelques types qui leur conviennent, du plus adapté au moins adapté. Par exemple, des dates en colonne suggèrent une courbe, et une colonne de quelques nombres positifs accompagnés d'étiquettes suggère un secteur. Chaque type est dessiné à partir de vos cellules. Choisissez-en un, puis cliquez sur **OK**.

Si vous ne sélectionnez qu'une seule cellule, JeffCrunch représente le bloc de cellules qui l'entoure et qui contient des données.

Les séries sont placées en colonnes lorsque la sélection compte au moins autant de lignes que de colonnes, et en lignes lorsqu'elle compte plus de colonnes que de lignes. Un secteur ne contient qu'une série. Un nuage de points prend ses valeurs horizontales dans la première colonne.

Exemple : vous sélectionnez A1:C4, où A2:A4 contiennent T1, T2 et T3, où B1 et C1 contiennent Est et Ouest, et où le reste est constitué de nombres. Le graphique comporte deux séries, Est et Ouest, et trois groupes de colonnes, de T1 à T3.

## Masquer une série provisoirement

Sélectionnez un graphique. Trois petits boutons apparaissent à côté de lui. Celui qui porte un entonnoir est **Filtres du graphique**. Il affiche les séries du graphique, chacune avec une case à cocher. Décochez une case pour retirer cette série du graphique. Cochez-la de nouveau pour la faire réapparaître à la même place. **Annuler** fonctionne aussi. Un graphique conserve toujours au moins une série. Une série retirée reste dans le fichier lorsque vous l'enregistrez, et elle reste masquée à l'ouverture du fichier, de sorte que rien n'est perdu. Cochez-la de nouveau pour la faire réapparaître. (Un graphique en anneau affiche toutes les séries à son ouverture.)

## Déplacer et redimensionner

Cliquez sur un graphique, une image ou une forme pour la sélectionner. Faites-la glisser pour la déplacer, ou faites glisser l'une des huit poignées de son contour pour la redimensionner.

- Maintenez la touche **Alt** enfoncée pendant le glissement pour aligner un bord sur le bord d'une cellule.
- Maintenez la touche **Maj** enfoncée pendant que vous faites glisser un coin pour conserver les proportions.
- Les touches fléchées le déplacent de quelques pixels à la fois. **Ctrl** avec une flèche le déplace d'un pixel, **Maj** avec une flèche l'agrandit ou la réduit, et **Alt** avec une flèche va jusqu'au bord de cellule suivant.
- **Échap** pendant le glissement la remet à sa place.

Chaque déplacement ou redimensionnement constitue une étape que vous pouvez annuler. Faites un clic droit sur l'objet et choisissez **Propriétés** pour saisir son **Nom** (chaque objet d'une feuille doit avoir le sien) et son **Texte de remplacement** (une courte phrase qui indique à une personne ne pouvant pas voir l'objet ce qu'il représente), pour saisir sa taille, et pour choisir son comportement lorsque vous insérez, supprimez ou redimensionnez des cellules : se déplacer et se redimensionner avec les cellules, se déplacer avec elles tout en conservant sa taille, ou rester en place. Ce choix est enregistré dans le fichier. **Imprimer cet objet**, dans la même boîte, masque l'objet sur les pages imprimées (il reste visible à l'écran) ; le fichier conserve aussi ce choix.

## Modifier ce qu'affiche un graphique

Lorsqu'un graphique est sélectionné, deux boutons apparaissent à côté de lui et un onglet **Création de graphique** apparaît sur le ruban.

- Le bouton **+** (et **Éléments du graphique** dans l'onglet) active ou désactive le titre du graphique, les axes, les titres des axes, le quadrillage, la légende et les étiquettes de données, et indique où placer la légende et les étiquettes.
- Le bouton pinceau (et **Couleurs** et **Styles** dans l'onglet) donne au graphique l'un des jeux de couleurs et l'un des styles de JeffCrunch. **Dispositions** règle plusieurs éléments à la fois.
- **Données du graphique** affiche les séries du graphique. Vous pouvez en ajouter, les modifier, les supprimer et les réordonner, modifier les cellules que chacune lit, définir les cellules contenant les étiquettes, permuter les lignes et les colonnes, et choisir la façon dont les cellules vides et les lignes et colonnes masquées sont représentées.
- **Boutons de champ** (dans le menu **+**, uniquement pour un graphique croisé dynamique) affiche de petits boutons portant les noms des champs du tableau croisé dynamique que suit le graphique : les filtres du rapport sous le titre, et les lignes, les valeurs et les colonnes le long du bas. Activez ou désactivez chaque type, ou choisissez **Tout afficher** ou **Tout masquer**. Un nouveau graphique croisé dynamique les affiche.
- **Barres d'erreur** (dans le menu **+**, pour un graphique en colonnes, en barres, en courbes, en aires ou en nuage de points) trace un petit trait au-dessus et au-dessous de chaque point pour montrer de combien la valeur réelle pourrait varier. Choisissez **Erreur type**, **Pourcentage (5 %)**, **Écart type** ou **Valeur fixe (1)**. Choisissez **Personnalisé** pour utiliser vos propres montants : saisissez les cellules qui contiennent les montants au-dessus de chaque point (par exemple `=Feuil1!$D$2:$D$4`), puis les cellules des montants au-dessous. Laissez une zone vide pour ne tracer les barres que d'un seul côté. Les barres changent lorsque ces cellules changent. Un fichier qui contient ses propres montants pour chaque point les conserve.
- **Lignes** (dans le menu **+**, pour un graphique en courbes) propose **Lignes de chute**, qui descendent du point le plus haut de chaque catégorie jusqu'à l'axe, **Lignes haut-bas**, qui relient les points les plus haut et les plus bas de chaque catégorie, et **Barres haut-bas**, qui sont des cadres entre la première et la dernière série de chaque catégorie. Une barre montante indique que la dernière série est plus élevée. Les lignes haut-bas et les barres haut-bas exigent au moins deux séries. Les graphiques en aires disposent des lignes de chute.
- **Tableau de données** (dans le menu **+**, pour un graphique en colonnes, en courbes ou en aires) place sous le graphique une grille des nombres, avec une colonne pour chaque catégorie et une ligne pour chaque série. Sa liste active ou désactive le tableau, et définit les traits entre les lignes, les traits entre les colonnes, le contour et les clés de légende (la couleur de chaque série à côté de son nom).
- **Courbe de tendance** (dans le menu **+**) trace une droite ajustée aux nombres de chaque série, pour voir vers où ils se dirigent. Choisissez **Linéaire** pour une droite, **Exponentielle** pour des nombres qui augmentent d'un pourcentage constant, **Logarithmique** pour des nombres qui montent vite puis se stabilisent, **Polynomiale** pour une courbe à plusieurs inflexions, **Puissance**, ou **Moyenne mobile**, qui trace la moyenne des derniers nombres. **Afficher l'équation** écrit la formule de la droite sur le graphique, et **Afficher R carré** indique la qualité de l'ajustement (1 correspond à un ajustement parfait ; 0 à aucun ajustement). La courbe de tendance a sa propre entrée dans la légende.
- Lorsqu'une série est tracée sur un second axe vertical, les menus **Axes** et **Titres des axes** proposent les entrées **Deuxième horizontal** et **Deuxième vertical** pour cette série. Le menu **Légende** propose **Par-dessus le graphique**, qui place la légende sur la zone de tracé au lieu de la placer à côté.
- **Étiquettes de données** propose aussi **Contenu de l'étiquette** (la valeur, le nom de la catégorie, le nom de la série, le pourcentage et la clé de légende, qui est un petit carré de la couleur de la série, dans n'importe quelle combinaison), **Séparateur** (ce qui les sépare : virgule, point-virgule, point, saut de ligne ou espace) et **Format de nombre…** (un code de format comme 0,0 pour les nombres des étiquettes ; laissez ce champ vide pour afficher les nombres tels quels). **Valeur à partir des cellules…** affiche le texte de certaines cellules comme étiquettes, une cellule par point. Saisissez les cellules, par exemple `=Feuil1!$D$2:$D$4`. Les étiquettes changent lorsque les cellules changent. Laissez le champ vide pour les supprimer. Si vous désactivez tous les contenus, les étiquettes disparaissent.
- **Format de l'axe horizontal…** et **Format de l'axe vertical…** dans l'onglet **Création de graphique** ouvrent une boîte pour un seul axe. Elle définit les valeurs minimale et maximale, les unités principale et secondaire (chacune automatique ou un nombre que vous saisissez), si les valeurs sont classées dans l'ordre inverse, les unités d'affichage (les étiquettes affichent les nombres divisés, par exemple par 1 000 pour des milliers, de sorte que 30 000 s'affiche 30 ; cochez **Afficher le nom des unités sur le graphique** pour imprimer le nom, comme Milliers, avec le titre de l'axe), si l'échelle est logarithmique (chaque repère vaut une base multipliée par le précédent, par exemple 1, 10, 100 et 1 000, ce qui convient aux nombres de tailles très différentes et qui doivent tous être supérieurs à 0), l'endroit où l'autre axe coupe celui-ci, le format de nombre de ses étiquettes, ses graduations, la position de ses étiquettes et l'angle de rotation de celles-ci. Seul un axe numérique possède des bornes et des unités.
- **Déplacer le graphique…** dans l'onglet **Création de graphique** demande où placer le graphique. Choisissez **Nouvelle feuille** et saisissez un nom : le graphique reçoit une feuille de graphique qui lui est propre, une feuille ne contenant qu'un graphique, sans cellules. Choisissez **Objet dans** et sélectionnez une feuille de calcul : le graphique est placé sur cette feuille. Un graphique placé sur une feuille de graphique ne peut être déplacé que vers une feuille de calcul, et la feuille de graphique qu'il quitte est supprimée. Le graphique lit toujours les cellules qu'il lisait. Sur une feuille de calcul, il conserve sa taille et sa place sur la feuille, même lorsque les lignes et les colonnes sont plus hautes ou plus larges, et il se déplace et se redimensionne avec les cellules de cet endroit, comme auparavant. Les feuilles masquées ne sont pas proposées. Cette opération correspond à une seule étape d'**Annuler**, même après un enregistrement, sauf si une feuille a été supprimée par celle-ci. Un fichier au format `.ods` ne peut pas contenir de feuille de graphique : enregistrez donc une feuille de graphique au format `.xlsx`.
- **Titre du graphique** et **Titres des axes** (dans le menu **+**) proposent aussi **Texte du titre…**. Saisissez le texte souhaité. Ou saisissez un signe égal suivi d'une cellule, par exemple **=Feuil1!A1**, et le titre affiche le contenu de cette cellule. Lorsque la cellule change, le titre change aussi. Le fichier conserve le lien.
- **Enregistrer comme image…** dans l'onglet **Création de graphique** (et dans le menu contextuel) enregistre le graphique sélectionné dans un fichier image **PNG** ou **JPEG**, deux fois plus grand que sur la feuille afin de rester net.
- **Enregistrer comme modèle…** et **Appliquer le modèle…** (dans l'onglet **Création de graphique**, dans le groupe **Type**) conservent l'apparence d'un graphique dans un fichier **.crtx** et l'appliquent à un autre graphique. L'apparence comprend le type, les couleurs, les réglages du titre et de la légende, les axes et les polices. Le graphique auquel vous appliquez le modèle conserve ses propres nombres et les cellules qu'il lit. Une seule étape d'**Annuler** retire de nouveau l'apparence.
- **Modifier le type de graphique** change le type de l'ensemble du graphique, ou de chaque série : des colonnes avec une courbe par-dessus, ou une série sur un second axe vertical.

Une référence à des cellules s'écrit par exemple **Feuil1!$B$2:$B$9**, avec le nom de la feuille entre apostrophes lorsqu'il contient des espaces.

## Sparklines

Une sparkline est un mini-graphique qui tient dans une seule cellule. Elle montre la tendance d'une ligne ou d'une colonne de nombres, juste à côté d'eux.

1. Sélectionnez les nombres. Pour une sparkline par ligne, sélectionnez plusieurs lignes de nombres.
2. Dans l'onglet **Insertion**, dans le groupe **Sparklines**, choisissez **Courbe**, **Colonne** ou **Gain/perte**. Une **Courbe** relie les nombres par un trait. Une **Colonne** dessine une barre pour chaque nombre. Un **Gain/perte** dessine une barre vers le haut pour chaque nombre supérieur à zéro et une barre vers le bas pour chaque nombre inférieur, toutes de la même hauteur, ce qui convient aux résultats qui ne peuvent être que bons ou mauvais.
3. Dans la boîte **Créer des sparklines**, vérifiez la **Plage de données** (les cellules que lisent les sparklines) et saisissez la **Plage d'emplacement** (les cellules qui les accueillent). Choisissez une cellule d'emplacement pour chaque ligne de données, ou pour chaque colonne de données. Si vous choisissez une seule cellule, la sparkline lit toutes les données.

Les sparklines créées en une seule opération forment un groupe et partagent le même style. Cliquez sur une cellule contenant une sparkline : l'onglet **Sparkline** apparaît.

- **Modifier les données** modifie les données et l'emplacement de l'ensemble du groupe, ou les données de la seule sparkline sélectionnée. Elle indique aussi quoi faire des cellules vides (laisser un espace, les traiter comme zéro ou relier les points de part et d'autre) et s'il faut lire les cellules des lignes et colonnes masquées.
- **Type** permet de passer de courbe à colonne et à gain/perte.
- **Afficher** met en évidence des points : le **Point haut**, le **Point bas**, les **Points négatifs**, le **Premier point**, le **Dernier point» et, pour une courbe, les **Marqueurs** sur chaque point.
- **Styles** donne au groupe un jeu de couleurs. **Couleur de la sparkline** règle la couleur de la courbe ou des barres, et l'**Épaisseur** (l'épaisseur du trait, en points). **Couleur du marqueur** règle la couleur de chaque type de point mis en évidence.
- **Axe** affiche un trait à zéro, trace les nombres de droite à gauche et définit les valeurs minimale et maximale de l'axe vertical. **Automatique pour chaque sparkline** adapte chacune à ses propres nombres. **Même pour toutes les sparklines** donne au groupe une seule échelle, pour pouvoir les comparer. **Valeur personnalisée** permet de saisir un nombre. **Axe des dates…** espace les points selon leurs dates : saisissez les cellules contenant la date de chaque point, et un point dont la date est plus tardive se place plus loin. Laissez ce champ vide pour répartir de nouveau les points de manière régulière.
- **Grouper** réunit les sparklines sélectionnées dans un même groupe. **Dissocier** donne à chacune son propre style. **Effacer** supprime les sparklines sélectionnées, ou leurs groupes entiers.

Une sparkline suit ses cellules : si vous insérez une ligne au-dessus des données, elle lit toujours les mêmes nombres. Chaque modification constitue une étape que vous pouvez annuler. Les sparklines sont enregistrées dans le fichier dans un format que lisent les autres tableurs.

## Insérer une image, une forme ou une zone de texte

Dans l'onglet **Insertion**, dans le groupe **Illustrations**, **Image** ouvre une fenêtre de sélection de fichier. Choisissez un fichier image (PNG, JPEG, GIF, BMP, TIFF ou SVG) : il est placé sur la feuille, dans la cellule sélectionnée. Une image dont la largeur dépasse 480 pixels est réduite pour tenir, en conservant ses proportions. Le classeur conserve l'image elle-même. Une image d'un type que le classeur ne peut pas conserver tel quel (un SVG, par exemple) est conservée au format PNG, une image ordinaire faite de points. Un fichier qui n'est pas une image, ou dont la taille dépasse 16 Mo, est refusé avec un message. Un fichier WMF ou EMF (les types d'images que certains anciens programmes créent à partir de traits et de formes) est dessiné et conservé également au format PNG.

**Icônes** affiche la même fenêtre de petites images en traits que JeffPub et JeffDeck, avec une zone de recherche par nom. Choisissez-en une ou plusieurs : chacune est placée sur la feuille sous forme d'image, de 48 points de large et de haut. Elle est dessinée en gris foncé, à une taille qui reste nette lorsque vous l'agrandissez. Chacune correspond à une seule étape que vous pouvez annuler.

**Code-barres** ouvre un court menu à deux choix. **Code-barres** ouvre une fenêtre pour créer un seul code-barres (**Codes-barres à partir des cellules**, ci-dessous, en crée plusieurs) : ISBN de livre (avec ou sans prix), EAN-13, UPC-A, EAN-8, Code 128, Code 39, ou un code QR, qui peut contenir n'importe quel texte, comme une adresse web. Choisissez un **Type**, saisissez le texte ou les chiffres : la fenêtre affiche le code-barres au fur et à mesure de votre saisie, et indique en rouge ce qui ne va pas s'il ne peut pas être créé. Cliquez sur **Insérer** : il est placé sur la feuille sous forme d'image, en une étape que vous pouvez annuler. Déplacez-le et redimensionnez-le comme n'importe quelle image. Pour le modifier, sélectionnez-le et cliquez de nouveau sur **Code-barres**, changez le texte, puis cliquez sur **Mettre à jour** ; le nouveau code-barres prend la place et la largeur de l'ancien. À l'enregistrement, il est sauvegardé comme image avec un SVG qui reste net, afin que tous les programmes l'affichent. JeffCrunch conserve ses réglages dans le fichier : c'est donc un code-barres que vous pouvez encore modifier lorsque vous rouvrez le classeur dans JeffCrunch.

**Code-barres > Codes-barres à partir des cellules** (dans le même menu **Code-barres**) crée un code-barres pour chaque cellule remplie des cellules sélectionnées, pour une planche d'étiquettes. Sélectionnez les cellules, cliquez sur cette commande, puis choisissez le type et l'aspect dans la même fenêtre. Ce que vous saisissez là n'est qu'un exemple, car le texte de chaque cellule devient son propre code-barres. Les codes-barres sont placés dans une colonne, juste à droite des cellules sélectionnées, l'un sous l'autre, en une seule étape que vous pouvez annuler. Une cellule dont le texte ne peut pas être codé (une lettre dans un EAN-13, par exemple) est signalée dans un message et ne reçoit aucun code-barres. Au plus 500 cellules sont traitées à la fois ; sélectionnez moins de cellules si le programme indique qu'il y en a trop. Chaque code-barres garde la trace de la cellule dont il provient et la suit : lorsque vous modifiez cette cellule, le code-barres est recréé à partir du nouveau texte, à la même place et à la même taille, comme une étape distincte que vous pouvez annuler. Le lien suit la cellule lorsque vous insérez ou supprimez des lignes ou des colonnes. Si vous supprimez la cellule, le code-barres conserve le dernier texte qu'il avait. À l'enregistrement, l'image du fichier est créée à partir du texte de la cellule tel qu'il est alors, de sorte que les autres programmes affichent aussi la valeur actuelle.

**Formes** ouvre une liste : rectangle, rectangle à coins arrondis, ovale, triangle, losange, flèches, étoile, cœur, nuage, bulle de dialogue et trait. Choisissez-en une : elle est placée sur la feuille en bleu clair, avec un bord plus foncé. **Zone de texte**, dans le groupe **Texte**, place une zone vide, sans remplissage ni bordure.

Lorsqu'une forme est sélectionnée, **Styles** dans l'onglet **Format de la forme** lui donne un aspect prêt à l'emploi : un remplissage et un contour assortis, par exemple bleu clair avec un bord bleu foncé. Les deux derniers ne conservent que le contour ou que le remplissage. Un clic correspond à une étape que vous pouvez annuler. **Remplissage de forme** et **Contour de forme** modifient chaque partie séparément.

## Rechercher des images libres en ligne

**Images en ligne**, dans le groupe **Illustrations** de l'onglet **Insertion**, trouve des images libres. Elles proviennent de deux bibliothèques ouvertes : Openverse, qui recherche dans de nombreuses collections, et Wikimedia Commons, la bibliothèque des images de Wikipédia.

1. Cliquez sur **Images en ligne…**. Un volet s'ouvre à droite.
2. Choisissez une bibliothèque, saisissez ce que vous cherchez, puis cliquez sur **Rechercher** (ou appuyez sur Entrée).
3. Cliquez sur une image pour voir qui l'a réalisée et sous quelle **licence** elle est diffusée. Une **licence** indique comment vous pouvez utiliser une image. Cliquez sur le nom de la licence pour en lire le texte.
4. Cliquez sur **Insérer** (ou double-cliquez sur l'image).

**Ce que JeffCrunch envoie.** Vos mots de recherche sont envoyés à la bibliothèque que vous avez choisie, et seulement lorsque vous cliquez sur **Rechercher** ou appuyez sur Entrée. JeffCrunch demande ensuite à la bibliothèque les petits aperçus de ce qu'elle a trouvé, puis l'image unique que vous insérez. Comme sur toute page web, la bibliothèque peut voir l'adresse internet de votre ordinateur, et elle est informée que JeffCrunch fait cette demande. Aucun contenu de votre classeur n'est envoyé.

L'image est placée sur la feuille comme une image provenant d'un fichier, et le classeur la conserve. Son titre est enregistré comme **texte de remplacement**. La plupart des licences demandent de citer l'auteur. Lorsque **Ajouter un crédit sous l'image** est coché, JeffCrunch ajoute sous l'image une petite zone de texte avec son titre, son auteur et sa licence. Vous pouvez la déplacer, la modifier ou la supprimer. L'image et son crédit sont insérés en une seule étape, de sorte qu'**Annuler** retire les deux. Vérifiez que la licence convient à l'usage que vous ferez de l'image.

## Ajouter du texte artistique

**Texte artistique** désigne des mots décorés : des lettres colorées, avec un contour, une ombre ou une forme courbe.

1. Cliquez sur **Insertion > Texte > Texte artistique**. Une galerie de styles s'ouvre.
2. Cliquez sur un style. Les mots « Votre texte ici » apparaissent sur la feuille dans ce style.
3. Pour modifier les mots, sélectionnez le texte artistique et cliquez sur **Modifier le texte…** dans l'onglet **Format de la forme**. Une boîte demande les mots, la police, la taille, le gras et l'italique. Une taille plus grande agrandit le texte artistique.
4. Dans l'onglet **Format de la forme**, **Styles de texte artistique** donne aux lettres un autre style, et **Déformation du texte** courbe les mots selon une forme, comme une vague ou une arche. « Normal » les remet droits.

Le texte artistique est une forme du dessin de la feuille, avec sa déformation, son remplissage et son contour : il conserve son aspect lorsque vous enregistrez le classeur au format .xlsx puis le rouvrez, dans JeffCrunch ou dans un autre programme. Chaque modification constitue une étape que vous pouvez annuler. Un classeur enregistré au format .ods conserve aussi ses images, formes, zones de texte, groupes, traits et textes artistiques. Le texte artistique est enregistré sous la forme de texte d'effet que prévoit OpenDocument, avec ses mots, sa police, son remplissage, son contour et sa déformation, et il redevient du texte artistique lorsque vous rouvrez le fichier .ods. Quelques déformations pour lesquelles OpenDocument n'a pas de forme (Vague 2, Vague 4, et les formes basse et haute de Gonfler et Dégonfler) sont enregistrées sous forme de mots droits, avec les mêmes lettres et le même style. Les graphiques sont aussi conservés dans un fichier .ods : voir ci-dessous.

## Réduire la taille des images

Une photo prise avec un appareil peut être bien plus grande que son affichage sur la feuille, ce qui alourdit le fichier. Sélectionnez une image, puis cliquez sur **Compresser les images…** dans le groupe **Ajuster** de l'onglet **Format de l'image**. Choisissez l'usage prévu du classeur : **Impression commerciale (300 ppp)**, **Impression de bureau (220 ppp)** ou **Web (96 ppp)**. Laissez cochée l'option **Appliquer à toutes les images de cette feuille** pour réduire toutes les images de la feuille, ou décochez-la pour ne réduire que l'image sélectionnée. **Supprimer les zones rognées des images** supprime aussi les parties masquées que vous avez rognées : le rognage disparaît. Une image n'est remplacée que si la nouvelle est plus petite. Sa taille, sa place, son nom et son texte de remplacement restent inchangés. **Annuler** (Ctrl+Z) restitue les images d'origine, et le classeur enregistré ne contient que les images réduites. Les images des autres feuilles ne sont pas modifiées : passez à chaque feuille et recommencez. Une image qui est un dessin (SVG, EMF ou WMF), ou une animation, est laissée telle quelle.

Chaque action correspond à une seule étape que vous pouvez annuler. L'objet nouveau est sélectionné, de sorte que vous pouvez le déplacer, le redimensionner, le faire passer devant ou derrière, lui donner un nom et un texte de remplacement dans **Propriétés**, ou appuyer sur **Suppr**.

## Afficher le texte d'une cellule dans une forme ou une zone de texte

Une forme ou une zone de texte peut afficher ce qu'affiche une cellule, et suivre cette cellule. Sélectionnez la forme, cliquez dans la **barre de formule**, saisissez un signe égal suivi d'une cellule, puis appuyez sur **Entrée**.

- `=B2` lie la forme à B2 de cette feuille. `=Feuil2!B3` la lie à une cellule d'une autre feuille. `=Total` la lie à un nom qui désigne une seule cellule.
- La forme affiche alors le texte de la cellule tel que la cellule l'affiche, avec son format de nombre. Lorsque vous modifiez la cellule, son format, ou une formule dont elle dépend, la forme change aussi. Un texte plus long que ce qu'une cellule peut contenir (32 767 caractères) est coupé. Toutes les formes liées d'un classeur affichent ensemble au plus 4 Mo de texte : au-delà, une forme n'affiche qu'un texte tronqué.
- Lorsque la forme est sélectionnée, la barre de formule affiche son lien, par exemple `=Feuil1!$B$2`. Effacez la barre de formule et appuyez sur **Entrée** pour supprimer le lien. La forme conserve alors le texte qu'elle affiche.
- Saisir du texte dans la forme supprime aussi le lien.
- Un lien désigne une seule cellule. Une plage comme `=A1:B2`, une liste, un calcul, ou un nom qui ne désigne pas une cellule est refusé avec un message, et la forme conserve son lien.
- Si la ligne ou la colonne de la cellule est supprimée, ou si sa feuille l'est, la forme affiche `#REF!` et conserve son lien, comme le ferait une formule. Annuler rétablit la cellule.
- Chaque lien, et chaque suppression de lien, constitue une étape que vous pouvez annuler.

Le lien est enregistré dans un fichier .xlsx avec le texte que la cellule affichait : un programme qui ne suit pas les liens affiche tout de même les mots, et JeffCrunch ouvre le fichier avec le lien fonctionnel. OpenDocument ne permet pas de lier une zone de texte à une cellule : un classeur enregistré au format .ods conserve donc les mots affichés par la forme à ce moment-là, et non le lien.

## Modifier l'aspect d'une image, d'une forme ou d'une zone de texte

Sélectionnez une image : l'onglet **Format de l'image** apparaît. Sélectionnez une forme, un trait ou une zone de texte : l'onglet **Format de la forme** apparaît. Chaque modification constitue une étape que vous pouvez annuler, et l'objet conserve son nom, son texte de remplacement, sa place et son ordre parmi les autres objets.

- **Remplissage de forme** et **Contour de forme** choisissent une couleur, ou aucune. **Style de trait** règle l'épaisseur du trait et les tirets (continu, tireté ou pointillé). Une image dispose de **Bordure de l'image**, qui est son contour.
- **Pivoter** fait tourner l'objet d'un quart de tour vers la droite ou vers la gauche, ou le retourne horizontalement ou verticalement.
- **Rogner**, pour une image, la découpe selon une forme de fenêtre : carré, 4 de large sur 3 de haut, 3 de large sur 4 de haut, 16 sur 9, ou 9 sur 16. La découpe est faite à parts égales depuis le milieu de ce qui est visible, et l'image est agrandie au maximum de ce qui reste. **Afficher l'image entière** rétablit ce qui a été rogné. **Remplir** conserve le cadre et découpe l'image de façon qu'elle le recouvre entièrement. **Ajuster** affiche l'image entière, et le cadre prend la forme de l'image à l'intérieur de l'ancien. **Rogner selon la forme** (la galerie à côté de **Rogner**) découpe l'image selon une forme telle qu'un ovale, une étoile ou un rectangle à coins arrondis ; la forme est enregistrée avec l'image. Le fichier de l'image n'est pas modifié : la partie rognée est seulement masquée.
- **Modifier le texte** concerne les formes et les zones de texte. Saisissez le texte, puis choisissez sa position horizontale et verticale, sa taille, le gras ou non, et s'il se renvoie à la forme ou reste sur une seule ligne. Pour un texte artistique, la boîte demande les mots, la police, la taille, le gras et l'italique.
- **Volet Sélection** ouvre la liste de tous les objets de la feuille par nom, celui du premier plan en tête. Cliquez sur un nom pour sélectionner cet objet, ce qui est utile lorsque des objets se masquent les uns les autres. **Avancer** et **Reculer** le déplacent dans la liste.
- **Propriétés** définit la taille, le nom, le texte de remplacement et le comportement de l'objet lorsque les cellules sont déplacées ou agrandies.

## Couper, copier et coller un objet

Une image ou une forme se copie et se colle comme un graphique. Une forme qui utilise autre chose qu'une image du classeur, comme un lien vers un fichier, ne peut pas être copiée, et JeffCrunch vous indique pourquoi.

Copiez un graphique puis collez-le dans le même classeur, sur n'importe quelle feuille : la copie lit les mêmes cellules que l'original. Si vous le collez dans un autre classeur, il conserve les nombres qu'il affichait mais ne lit plus de cellules, car les cellules qu'il lirait là-bas ne sont pas celles à partir desquelles il a été créé.

## Sélectionner plusieurs objets, les grouper et les aligner

Cliquez sur un objet, puis maintenez Maj ou Ctrl et cliquez sur un autre pour le sélectionner aussi. Cliquez de nouveau sur un objet sélectionné en maintenant Maj ou Ctrl pour le désélectionner. Les objets sélectionnés affichent chacun un cadre en pointillés. Vous pouvez aussi choisir **Sélectionner des objets** dans l'onglet **Format de la forme**, puis faire glisser un cadre sur des cellules vides : tous les objets entièrement compris dans ce cadre sont sélectionnés. Appuyez sur Échap, ou choisissez de nouveau **Sélectionner des objets**, pour revenir à la sélection de cellules. Appuyez sur Suppr pour supprimer tous les objets sélectionnés en une seule étape.

Lorsque deux objets ou plus sont sélectionnés, l'onglet **Format de la forme** propose ces commandes dans le groupe **Organiser** :

- **Grouper** réunit les formes, images et traits sélectionnés en un seul objet. Il se déplace et se redimensionne d'un seul tenant, et le fichier le conserve comme groupe. Un graphique ne peut pas être placé dans un groupe.
- **Dissocier** sépare un groupe sélectionné. Chaque élément retrouve sa place sur la feuille.
- **Aligner** aligne les objets sélectionnés : **Aligner à gauche**, **Centrer**, **Aligner à droite**, **Aligner en haut**, **Aligner au milieu** et **Aligner en bas**. Ils s'alignent sur le bord ou le milieu de l'ensemble. **Distribuer horizontalement** et **Distribuer verticalement** répartissent régulièrement trois objets ou plus entre les deux situés aux extrémités.

Grouper, Dissocier et chaque choix d'alignement constituent une étape que vous pouvez annuler.

## Nommer un objet

Lorsqu'un objet est sélectionné, la zone Nom affiche son nom. Saisissez-y un nouveau nom et appuyez sur **Entrée**. **Annuler** rétablit l'ancien nom, et le nouveau nom est enregistré dans le fichier.

## Supprimer un objet

Appuyez sur **Suppr** lorsque l'objet est sélectionné. **Annuler** le restitue.

## Ce qui est enregistré

Tout ce qui précède est enregistré dans le fichier, dans un format que lisent les autres tableurs : un graphique créé ici s'ouvre donc de la même façon ailleurs. Un graphique provenant d'un autre programme et comportant des éléments que JeffCrunch ne peut pas conserver lorsqu'il réécrit un graphique (barres d'erreur, graphique en trois dimensions) peut tout de même être déplacé, redimensionné et supprimé, mais ses éléments et ses données ne sont pas modifiés ici.

## Diagrammes créés par d'autres programmes

Un **diagramme** est une image de boîtes et de flèches, comme un processus, créée par un autre programme. JeffCrunch affiche un diagramme à partir de l'image que le fichier conserve à côté de lui. Cliquez dessus pour le sélectionner, et faites-le glisser pour le déplacer. Lorsque vous enregistrez le fichier, le diagramme est réécrit exactement tel qu'il était, de sorte que le programme qui l'a créé peut encore le modifier.

JeffCrunch ne peut pas encore modifier le contenu d'un diagramme, et un diagramme ne peut être ni copié ni placé dans un groupe. Si le fichier ne conserve aucune image d'un diagramme, JeffCrunch affiche une simple case grise indiquant qu'il s'agit d'un diagramme.

## Graphiques dans les fichiers .ods

Un classeur enregistré au format .ods conserve ses graphiques : colonnes, barres, courbes, aires, secteurs, anneaux et nuages de points, chacun avec ses séries et les cellules qu'elles lisent (sur cette feuille ou une autre), ses catégories, son titre, sa légende, ses axes (titres, valeurs minimale et maximale, format de nombre), ses étiquettes de données, ses couleurs et sa place. Un fichier .ods créé par un autre programme s'ouvre avec ses graphiques, dessinés à partir des cellules qu'ils lisent. Ce qu'un graphique .ods ne peut pas contenir est omis et signalé lors de l'enregistrement : courbes de tendance, barres d'erreur, axes secondaires et aspects particuliers d'un seul point. Les graphiques d'un type que JeffCrunch ne dessine pas (graphiques boursiers et radar) s'ouvrent sous forme de colonnes de leurs valeurs.

# Le volet Format de la forme et l'onglet Format de la forme
<!-- keywords: format de la forme, mise en forme de la forme, format de l'image, volet format, volet, ctrl+1, remplissage, remplissage uni, dégradé, points de dégradé, point, remplissage avec image, texture, mosaïque, motif, trait, contour, bordure, épaisseur, largeur, type composé, type de tiret, pointillé, type d'extrémité, type de jointure, flèches, flèche de début, flèche de fin, effets, ombre, réflexion, lumière, halo, bordures adoucies, taille, hauteur, rotation, échelle, conserver les proportions, position, zone de texte, marges, alignement vertical, orientation du texte, ajustement automatique, réduire le texte, redimensionner la forme, renvoi à la ligne, colonnes, texte de remplacement, texte alternatif, titre, description, décoratif, corrections de l'image, luminosité, contraste, recolorier, noir et blanc, nuances de gris, transparence, rogner, recadrer, remplissage de la forme, contour de la forme, effets de forme, outils de dessin, modifier la forme, styles de forme, styles rapides, style, couleurs du thème, accent, styles de texte artistique, remplissage du texte, contour du texte, effets du texte, transformer, lettres -->

Le **volet Format de la forme** est un panneau situé à droite de la fenêtre. Il indique l'apparence des objets sélectionnés et permet de la modifier. Les **formes**, les **zones de texte**, les **traits** et les **images** l'utilisent tous. Il comporte une page pour le remplissage et le trait, une pour les effets, une pour la taille et la position et, pour les images, une pour les couleurs de l'image.

## Ouvrir et fermer le volet

- Sélectionnez un objet, puis appuyez sur **Ctrl+1**.
- Ou cliquez avec le bouton droit sur l'objet et choisissez **Format de la forme…**. (Pour une image, la commande s'appelle **Format de l'image…**.)
- Ou cliquez sur **Volet Format** sous l'onglet **Format de la forme**. Un second clic le ferme.
- Ou cliquez sur le petit bouton situé dans l'angle d'un groupe de l'onglet **Format de la forme**. L'angle de **Styles de forme** ouvre la page **Remplissage et trait**, et l'angle de **Taille** ouvre la page **Taille et propriétés**.

Pour fermer le volet, cliquez sur le **×** en haut à droite, ou cliquez de nouveau sur **Volet Format**. JeffDeck mémorise si le volet était ouvert : il le retrouve dans le même état au prochain démarrage.

## Fonctionnement du volet

- **Il suit la sélection.** Cliquez sur un autre objet : le volet affiche cet objet. Lorsque rien n'est sélectionné, il l'indique.
- **Plusieurs objets à la fois.** Si vous en sélectionnez plusieurs, une modification s'applique à tous. Lorsque leurs valeurs diffèrent, la zone est vide. Modifiez-la et tous les objets prennent votre valeur. Chaque objet conserve tout ce que vous n'avez pas modifié. Par exemple, trois formes de trois couleurs différentes gardent leur couleur lorsque vous modifiez leur transparence.
- **Chaque modification compte pour une étape.** **Ctrl+Z** annule une modification. Lorsque vous faites glisser un curseur, les objets suivent le pointeur, et l'ensemble du glissement compte pour une étape, enregistrée au moment où vous relâchez. Il en va de même pour un nombre que vous modifiez en maintenant une touche de direction enfoncée.
- **Un groupe** applique un remplissage, un trait ou un effet à tout son contenu. Sa taille et sa position lui sont propres.
- **Les objets verrouillés** conservent leur taille et leur position.

Les onglets sont des icônes en haut du volet : **Remplissage et trait**, **Effets**, **Taille et propriétés** et, lorsque seules des images sont sélectionnées, **Image**. Placez le pointeur sur une icône pour afficher son nom. Chaque page est une colonne de **sections**. Cliquez sur le nom d'une section pour la réduire ou la développer.

## Remplissage et trait

Le **remplissage** est la couleur ou l'image à l'intérieur d'une forme. Choisissez un type :

- **Aucun remplissage** laisse l'intérieur transparent.
- **Remplissage uni** applique une seule couleur. **Couleur** ouvre les couleurs du thème. **Transparence** indique dans quelle mesure l'arrière-plan apparaît : 0 % masque ce qui se trouve derrière, 100 % le laisse entièrement visible.
- **Remplissage dégradé** fond plusieurs couleurs. **Type** peut être **Linéaire** (le long d'une ligne), **Radial** (à partir du centre), **Rectangulaire** ou **Trajectoire** (le long du contour). Un dégradé linéaire possède aussi un **Angle** : 0° va de gauche à droite, 90° de haut en bas.
  - La barre affiche les couleurs. Chaque petit repère situé dessous est un **point** de dégradé, un endroit où la couleur est exactement celle que vous avez choisie. Cliquez sur un repère pour le sélectionner, puis définissez sa **Couleur**, sa **Position** de 0 % à 100 % et sa **Transparence**.
  - Faites glisser un repère, ou appuyez sur les touches **Flèche gauche** et **Flèche droite**, pour le déplacer. Appuyez sur **Flèche haut** et **Flèche bas** pour en sélectionner un autre. **Ajouter un point** en place un nouveau à mi-chemin du suivant. **Supprimer le point**, ou la touche **Suppr**, retire le point sélectionné. Un dégradé en conserve au moins deux. Vous pouvez aussi double-cliquer sur la barre pour ajouter un point à l'endroit du clic.
- **Remplissage avec image ou texture** couvre l'intérieur avec une image provenant d'un fichier. Cliquez sur **Insérer…** et choisissez un fichier. Cochez **Utiliser l'image comme texture en mosaïque** pour répéter l'image sur toute la forme, à l'**Échelle** définie. Laissez la case décochée pour étirer l'image à la taille de la forme. **Transparence** fonctionne également ici.
- **Remplissage à motif** applique une petite image répétée, comme des rayures ou des points. Cliquez sur l'un des 13 motifs, puis choisissez sa couleur de **Premier plan** (les traits) et sa couleur d'**Arrière-plan** (l'espace entre eux).

Le **trait** est le bord d'une forme, ou un trait lui-même. Choisissez **Aucun trait**, **Trait plein** ou **Trait dégradé**. Un trait dégradé fond les couleurs sur sa longueur. Définissez ensuite :

- **Couleur** et **Transparence**, ou les points du dégradé.
- **Largeur**, en points. Un point vaut 1/72 de pouce.
- **Type composé** : un trait simple, ou deux ou trois traits côte à côte, épais et fins.
- **Type de tiret** : continu, pointillé ou en tirets.
- **Type d'extrémité** : l'aspect de l'extrémité d'un trait. **Plat** s'arrête à l'extrémité, **Arrondi** ajoute un demi-cercle et **Carré** ajoute un demi-carré.
- **Type de jointure** : l'aspect d'un angle. **Arrondi**, **Biseau** (coupé) ou **Angle droit** (pointu).
- **Type de flèche de début**, **Taille de flèche de début**, **Type de flèche de fin** et **Taille de flèche de fin**, pour un trait qui a des extrémités. Le bord d'une forme n'en a pas : ces options n'apparaissent donc que lorsqu'un trait est sélectionné.

## Effets

Un **effet** est un aspect ajouté autour d'un objet. Chaque section comporte une case pour l'activer, une liste **Présélections** comme point de départ et des zones pour le modifier. Une présélection ne fait que remplir les zones : vous pouvez ensuite modifier chacune d'elles.

- **Ombre** : une copie sombre derrière l'objet. **Couleur** et **Transparence** fonctionnent comme pour un remplissage. **Taille** rend l'ombre plus grande ou plus petite que l'objet, à partir du centre de l'objet. **Flou** adoucit son bord. **Angle** indique sa direction (0° vers la droite, 90° vers le bas) et **Distance** son éloignement.
- **Réflexion** : une copie estompée et renversée sous l'objet. **Transparence** indique son intensité de départ. **Taille** indique jusqu'où elle s'estompe vers le bas. **Distance** est l'écart entre l'objet et sa réflexion. **Flou** l'adoucit.
- **Lumière** : un halo de couleur doux autour de l'objet. **Couleur**, **Taille** et **Transparence**.
- **Bordures adoucies** : fond le bord de l'objet dans ce qui se trouve derrière. **Taille** est la largeur du fondu.

## Taille et propriétés

**Taille** :

- **Hauteur** et **Largeur** sont exprimées dans l'unité choisie dans les options (les pouces, par exemple). Vous pouvez taper une autre unité, par exemple **2 cm**.
- **Rotation** fait pivoter l'objet, en degrés.
- **Échelle de hauteur** et **Échelle de largeur** sont des pourcentages. Pour une image, 100 % correspond à la taille de l'image dans son fichier. Pour les autres objets, elles indiquent 100 % et agrandissent ou réduisent l'objet du pourcentage saisi.
- **Conserver les proportions** maintient les proportions : une nouvelle largeur modifie la hauteur en conséquence, et inversement. Cette option est activée au départ pour une image. Elle détermine aussi si le glissement d'un angle conserve les proportions. Maintenez la touche **Maj** enfoncée pendant le glissement pour obtenir l'effet inverse.

**Position** : **Position horizontale** et **Position verticale** indiquent l'emplacement de l'angle supérieur gauche de l'objet, mesuré à partir de l'angle supérieur gauche de la diapositive.

**Zone de texte** apparaît pour les zones de texte et les formes :

- **Alignement vertical** place le texte en haut, au milieu ou en bas. Les choix **Centré** centrent aussi le bloc de texte dans la largeur de la zone.
- **Orientation du texte** fait pivoter tout le texte d'un quart de tour. Cette option concerne les zones de texte.
- **Ajustement automatique** détermine ce qui se passe lorsque le texte dépasse l'espace disponible. **Ne pas ajuster automatiquement** le laisse déborder. **Réduire le texte dans la zone de texte** réduit la taille des caractères pour qu'il tienne. **Redimensionner la forme pour contenir le texte** agrandit ou réduit la hauteur de la zone. Ces deux options concernent les zones de texte. Le texte d'une forme conserve sa taille.
- **Marge de gauche**, **Marge de droite**, **Marge du haut** et **Marge du bas** définissent l'espace entre le bord et le texte.
- **Renvoyer à la ligne automatiquement le texte dans la forme** : lorsque cette option est activée, une ligne de texte passe à la ligne au bord de la zone. Lorsqu'elle est désactivée, la ligne continue jusqu'à ce que vous appuyiez sur Entrée.
- **Colonnes** et **Espacement** définissent le nombre de colonnes de texte et l'écart entre elles.

**Texte de remplacement** est destiné aux personnes qui ne voient pas la diapositive. Un lecteur d'écran le lit à voix haute.

- **Titre** est un nom court.
- **Description** indique ce que montre l'objet ou pourquoi il est là. Appuyez sur **Tab** ou cliquez ailleurs pour conserver votre saisie.
- **Marquer comme décoratif** s'applique à un objet purement esthétique, comme un trait sous un titre. Un lecteur d'écran l'ignore, et JeffDeck ne demande pas de description pour cet objet.

## Image

Lorsque seules des images sont sélectionnées, la page **Image** propose :

- **Luminosité** et **Contraste** éclaircissent ou assombrissent l'image, et accentuent ou atténuent l'écart entre ses parties claires et sombres.
- **Recolorier** permet de passer l'image en **Nuances de gris** ou en **Noir et blanc**. **Niveau de noir et blanc** indique à partir de quelle clarté un point devient blanc.
- **Transparence** laisse apparaître ce qui se trouve derrière l'image.
- **Rogner** indique où se trouve l'image entière dans son cadre. **Largeur de l'image** et **Hauteur de l'image** sont la taille de l'image entière. **Décalage X** et **Décalage Y** la déplacent par rapport au centre du cadre. Ce qui dépasse du cadre est masqué, mais conservé. Pour disposer de poignées à faire glisser, utilisez **Rogner** sous l'onglet **Format de l'image**.

Ces modifications ne touchent jamais aux données de l'image : elles sont enregistrées à côté.

## L'onglet Format de la forme

L'onglet **Format de la forme** apparaît lorsqu'une forme, une zone de texte, un trait ou un groupe de ces objets est sélectionné. Il disparaît lorsque vous sélectionnez une image, un tableau ou rien.

- **Formes** contient **Modifier la forme**. Cette commande remplace la forme des objets sélectionnés par une autre forme de la liste **Insertion > Formes**. Leur texte, leur remplissage, leur trait, leurs effets, leur position et leur taille restent inchangés. Les poignées jaunes d'une forme reprennent les valeurs propres à la nouvelle forme, et les points déplacés à la main sont remplacés. Cette commande concerne uniquement les formes, pas les zones de texte, les traits ni les images. Elle compte pour une étape : **Ctrl+Z** rétablit l'ancienne forme.
- **Styles de forme** commence par une galerie d'aspects prédéfinis, et contient **Remplissage de la forme**, **Contour de la forme** et **Effets de forme**.
  - **La galerie** comporte quatre lignes : **Contour uniquement** (un trait, sans remplissage), **Effet discret**, **Effet modéré** et **Effet intense** (chacun avec un remplissage et un trait, les deux derniers avec une ombre, de plus en plus marquée vers le bas). Chaque ligne compte six vignettes, une pour chacune des six couleurs d'accent du thème. Placez le pointeur sur une vignette pour afficher son nom, et cliquez dessus pour appliquer cet aspect à toutes les formes sélectionnées. Cela compte pour une étape.
  - Un style provient du thème : les remplissages, les traits et les ombres sont ceux du thème, dans l'accent choisi. La même vignette a un aspect différent dans un autre thème, et une vignette montre le résultat dans le thème actuel. La forme mémorise son style. Un style définit aussi la couleur du texte de la forme, sauf pour le texte que vous avez coloré vous-même.
  - **Remplissage de la forme**, **Contour de la forme** et **Effets de forme** ouvrent chacun un menu reprenant les choix ci-dessus, ce qui évite de passer par le volet pour les plus courants. Ils modifient une partie d'un style et laissent le reste intact. Le dernier choix de chaque menu, par exemple **Autres options de remplissage…**, ouvre le volet sur la page correspondante.
  - **Remplissage de la forme** : les couleurs du thème, **Aucun remplissage**, **Autres couleurs de remplissage…**, **Image…** et **Dégradé** (six aspects de départ).
  - **Contour de la forme** : les couleurs, **Aucun contour**, **Épaisseur**, **Tirets** et **Flèches**.
  - **Effets de forme** : **Ombre**, **Réflexion**, **Lumière** et **Bordures adoucies**, chacun avec ses présélections.
- **Styles de texte artistique** modifie les caractères du texte d'une forme ou d'une zone de texte, ainsi que le texte artistique. Pendant la saisie dans une zone, il modifie le texte sélectionné. Lorsque la zone elle-même est sélectionnée, il modifie tout son texte. Lorsque du texte artistique est sélectionné, il modifie ce texte artistique. Il est désactivé pour un objet sans texte.
  - **La galerie** propose les mêmes styles que **Insertion > Texte artistique**. Sur le texte d'une forme, un style applique aux caractères son remplissage, son contour et son ombre, sans modifier la police. Sur du texte artistique, il applique le style complet (police et courbure comprises), tout en conservant le texte et la taille.
  - **Remplissage du texte** : les couleurs du thème, **Aucun remplissage** (caractères transparents, seul le contour apparaît), **Autres couleurs de remplissage…** et **Dégradé** (six aspects de départ, à partir de la couleur actuelle des caractères).
  - **Contour du texte** : les couleurs, **Aucun contour**, **Autres couleurs de contour…** et **Épaisseur**. Un contour a au départ une épaisseur d'un point.
  - **Effets du texte** : **Ombre**, **Réflexion** et **Lumière**, chacun avec ses présélections, ainsi que **Transformer**. Chaque effet ajouté s'additionne aux autres. **Transformer** courbe le texte artistique en arc, en cercle, en vague, etc. Le texte d'une forme n'est pas courbé : la commande est donc désactivée pour lui.
- **Accessibilité** contient **Texte de remplacement**. Cette commande ouvre le volet sur la section **Texte de remplacement**, avec le curseur dans la description.
- **Taille** contient les zones **Hauteur** et **Largeur**. Tapez un nombre pour modifier la taille de tous les objets sélectionnés.
- **Format** contient **Volet Format**, qui ouvre et ferme le volet.

Chaque choix d'un menu compte pour une étape, comme une modification dans le volet.

## Utiliser le volet au clavier

- **Ctrl+1** ouvre le volet et place le focus sur ses onglets. Utilisez les touches **Flèche gauche** et **Flèche droite** pour passer d'un onglet à l'autre, et **Tab** pour atteindre les sections et les zones.
- **Espace** ou **Entrée** sur le nom d'une section la réduit ou la développe.
- Dans une zone numérique, **Flèche haut** et **Flèche bas** modifient légèrement la valeur. Tapez un nombre et appuyez sur **Entrée** pour le valider.
- **Échap** renvoie le focus sur la diapositive.

## Dans les fichiers

Un fichier .pptx conserve tous les paramètres ci-dessus. Un fichier .odp les conserve aussi, mais certains n'y ont pas d'emplacement prévu : JeffDeck les enregistre alors d'une manière que seul JeffDeck sait lire. Il s'agit du trait dégradé, du trait à deux ou trois traits, de la réflexion, de la taille de l'ombre, du verrouillage des proportions, des colonnes, du marquage décoratif et du style de la forme. Pour les caractères, il s'agit du remplissage dégradé, du contour, de la lumière, de la réflexion, du flou de l'ombre et des caractères transparents. Un autre programme qui ouvre ce fichier .odp affiche le trait dans sa première couleur, en trait simple et sans réflexion, et affiche les caractères sans effet, d'une seule couleur, avec une ombre sans flou. JeffDeck les restitue tous. Un remplissage à motif est enregistré sous la forme de la hachure ou de la couleur unie la plus proche, et le niveau de noir et blanc d'une image n'est pas enregistré dans un fichier .odp.

Un style est enregistré dans un fichier .pptx de la même manière que dans les autres programmes. La forme indique quels aspects du thème elle utilise (un trait, un remplissage, une ombre et une couleur de texte, chacun dans une couleur d'accent), et ses propres paramètres n'indiquent que ce que vous avez modifié ensuite. Ainsi, une forme modifiée conserve votre modification, et une forme non modifiée garde l'aspect défini par le thème, quel que soit l'endroit où le fichier est ouvert. Une forme d'un fichier que vous ouvrez conserve son style de la même façon. Le contour et le remplissage dégradé des caractères sont également enregistrés dans un fichier .pptx.

## Rubriques connexes

- [Formes, images, tableaux et texte artistique](objects)
- [Saisir et mettre en forme du texte](text)
- [Onglet Accueil](tab-home)
- [Utiliser JeffDeck au clavier](keyboard)

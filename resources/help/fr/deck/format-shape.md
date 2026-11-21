# Le volet Format de forme et l'onglet Format de forme
<!-- keywords: format de forme, format de l'image, volet format, volet, format de forme, ctrl+1, remplissage, remplissage uni, dégradé, points d'arrêt de dégradé, arrêt, remplissage avec image, texture, mosaïque, motif, trait, contour, épaisseur, type composé, type de tiret, type d'extrémité, type de jonction, flèches, flèche de début, flèche de fin, effets, ombre, reflet, lueur, bordures adoucies, taille, hauteur, rotation, échelle, verrouiller les proportions, position, zone de texte, marges, alignement vertical, orientation du texte, ajuster, réduire le texte en cas de dépassement, redimensionner la forme pour ajuster le texte, renvoyer à la ligne, colonnes, texte de remplacement, titre, description, décoratif, corrections d'image, luminosité, contraste, recolorier, noir et blanc, transparence, rogner, remplissage de forme, contour de forme, effets de forme, outils de dessin, modifier la forme, styles de forme, styles rapides, style, couleurs du thème, accent, styles de texte artistique, remplissage du texte, contour du texte, effets de texte, transformer, lettres -->

Le **volet Format de forme** est un panneau situé à droite de la fenêtre. Il affiche l'aspect des objets sélectionnés et permet de le modifier. Une **forme**, une **zone de texte**, un **trait** et une **image** utilisent tous ce volet. Il comporte une page pour le remplissage et le contour, une pour les effets, une pour la taille et la position et, pour les images, une pour les couleurs de l'image.

## Ouvrir et fermer le volet

- Sélectionnez un objet, puis appuyez sur **Ctrl+1**.
- Ou faites un clic droit sur l'objet et choisissez **Format de la forme…**. (Pour une image, la commande s'appelle **Format de l'image…**.)
- Ou cliquez sur **Volet Format** dans l'onglet **Format de forme**. Un second clic le ferme.
- Ou cliquez sur le petit bouton situé dans le coin d'un groupe de l'onglet **Format de forme**. Le coin du groupe **Styles de forme** ouvre la page **Remplissage et trait**, et le coin du groupe **Taille** ouvre la page **Taille et propriétés**.

Pour fermer le volet, cliquez sur le **×** en haut à droite, ou cliquez à nouveau sur **Volet Format**. JeffDeck se souvient si le volet était ouvert et l'affiche de la même façon au prochain démarrage.

## Fonctionnement du volet

- **Il suit votre sélection.** Cliquez sur un autre objet et le volet affiche celui-ci. Lorsque rien n'est sélectionné, il l'indique.
- **Plusieurs objets à la fois.** Si vous sélectionnez plusieurs objets, une modification s'applique à tous. Lorsque leurs valeurs diffèrent, la case reste vide. Si vous modifiez la case, chaque objet reçoit votre valeur. Chaque objet conserve ce que vous n'avez pas touché. Par exemple, trois formes de trois couleurs différentes gardent leurs couleurs lorsque vous modifiez leur transparence.
- **Chaque modification est une étape.** **Ctrl+Z** annule une modification. Lorsque vous faites glisser un curseur, les objets suivent le pointeur, et tout le glissement compte pour une seule étape. Elle est comptabilisée au moment où vous relâchez le bouton. Un nombre que vous modifiez en maintenant une touche fléchée enfoncée fonctionne de la même manière.
- **Un groupe** applique un remplissage, un trait ou un effet à tout ce qu'il contient. Sa taille et sa position lui sont propres.
- **Les objets verrouillés** conservent leur taille et leur position.

Les onglets sont représentés par des icônes en haut du volet : **Remplissage et trait**, **Effets**, **Taille et propriétés** et, lorsque vous n'avez sélectionné que des images, **Image**. Pointez une icône pour afficher son nom. Chaque page est une colonne de **sections**. Cliquez sur le nom d'une section pour la replier ou la déplier.

## Remplissage et trait

Le **remplissage** est la couleur ou l'image à l'intérieur d'une forme. Choisissez un type :

- **Aucun remplissage** laisse l'intérieur transparent.
- **Remplissage uni** est une couleur unique. **Couleur** ouvre les couleurs du thème. **Transparence** indique la quantité d'arrière-plan visible à travers la forme : 0 % masque ce qui se trouve derrière, et 100 % le laisse entièrement visible.
- **Remplissage en dégradé** fait se succéder plusieurs couleurs. **Type** peut être **Linéaire** (le long d'une ligne), **Radial** (depuis le centre), **Rectangulaire** ou **Trajet** (le long du contour). Un dégradé linéaire dispose aussi d'un **Angle** : 0° va de gauche à droite, et 90° de haut en bas.
  - La barre montre les couleurs. Chaque petit repère situé sous la barre est un **point d'arrêt**, un endroit où la couleur est exactement celle que vous avez choisie. Cliquez sur un repère pour le sélectionner, puis réglez sa **Couleur**, sa **Position** de 0 % à 100 % et sa **Transparence**.
  - Faites glisser un repère, ou utilisez les touches **Gauche** et **Droite**, pour le déplacer. Appuyez sur **Haut** et **Bas** pour en sélectionner un autre. **Ajouter un arrêt** crée un nouveau point à mi-chemin du suivant. **Supprimer l'arrêt**, ou la touche **Suppr**, supprime le point sélectionné. Un dégradé comporte au moins deux points. Vous pouvez aussi double-cliquer sur la barre pour ajouter un point à l'endroit où vous cliquez.
- **Remplissage avec image ou texture** recouvre l'intérieur avec une image provenant d'un fichier. Cliquez sur **Insérer…** et choisissez un fichier. Cochez **Afficher l'image en mosaïque comme texture** pour répéter l'image sur toute la forme, à l'**échelle** que vous avez définie. Si vous ne la cochez pas, l'image est étirée pour remplir la forme. **La transparence** fonctionne aussi ici.
- **Remplissage à motif** utilise une petite image répétée, comme des rayures ou des pois. Cliquez sur l'un des 13 motifs, puis choisissez sa **Couleur de premier plan** (les traits) et sa **Couleur d'arrière-plan** (ce qui se trouve entre eux).

Le **trait** est le contour d'une forme, ou la ligne elle-même. Choisissez **Aucun trait**, **Trait plein** ou **Trait en dégradé**. Un trait en dégradé combine des couleurs sur toute sa longueur. Réglez ensuite :

- **Couleur** et **Transparence**, ou les points d'arrêt du dégradé.
- **Épaisseur**, en points. Un point équivaut à 1/72 de pouce.
- **Type composé** : un trait simple, ou deux ou trois traits côte à côte, épais et fins.
- **Type de tiret** : plein, pointillé ou tirets.
- **Type d'extrémité** : l'aspect de la fin d'une ligne. **Plat** s'arrête net à l'extrémité, **Rond** ajoute un demi-cercle, et **Carré** ajoute un demi-carré.
- **Type de jonction** : l'aspect d'un angle. **Arrondi**, **Biseauté** (coupé) ou **Angle** (pointu).
- **Type de flèche de début**, **Taille de flèche de début**, **Type de flèche de fin** et **Taille de flèche de fin**, pour une ligne qui a des extrémités. Le contour d'une forme n'a pas d'extrémités : ces options n'apparaissent que lorsqu'une ligne est sélectionnée.

## Effets

Un **effet** est un aspect ajouté autour d'un objet. Chaque section comporte une case pour l'activer, une liste de **Préréglages** pour démarrer, et des cases pour le modifier. Un préréglage ne fait que régler les cases. Vous pouvez modifier chacune d'elles ensuite.

- **Ombre** : une copie sombre placée derrière l'objet. **Couleur** et **Transparence** s'utilisent comme pour un remplissage. **Taille** agrandit ou réduit l'ombre par rapport à l'objet, à partir de son milieu. **Flou** adoucit son bord. **Angle** indique la direction de l'ombre (0° est à droite, 90° est en bas), et **Distance** indique son éloignement.
- **Reflet** : une copie estompée et retournée, placée sous l'objet. **Transparence** indique le degré de faiblesse du reflet au départ. **Taille** indique jusqu'où le reflet s'estompe vers le bas. **Distance** est l'écart entre l'objet et son reflet. **Flou** adoucit le reflet.
- **Lueur** : un halo de couleur doux autour de l'objet. **Couleur**, **Taille** et **Transparence**.
- **Bordures adoucies** : estompe le bord de l'objet pour le fondre dans l'arrière-plan. **Taille** indique la largeur du fondu.

## Taille et propriétés

**Taille** :

- **Hauteur** et **Largeur** s'expriment dans l'unité choisie dans les options (en centimètres, par exemple). Vous pouvez saisir une autre unité, comme **2 cm**.
- **Rotation** fait pivoter l'objet, en degrés. **Autres options de rotation**, dans le menu **Organiser**, ouvre cette page.
- **Hauteur de l'échelle** et **Largeur de l'échelle** sont exprimées en pourcentages. Pour une image, 100 % correspond à la taille qu'elle a dans son fichier. Pour les autres objets, la valeur est 100 % et l'objet est agrandi ou réduit selon le pourcentage que vous saisissez.
- **Verrouiller les proportions** conserve les proportions : une nouvelle largeur modifie la hauteur en conséquence, et inversement. Une image est d'abord créée avec cette option activée. Elle détermine aussi si le glissement d'un coin conserve les proportions. Maintenez **Maj** pendant le glissement pour faire l'inverse.

**Position** : **Position horizontale** et **Position verticale** indiquent où se trouve l'objet, mesuré depuis le coin supérieur gauche de la diapositive. **Position à partir de** choisit le point de mesure : le **coin supérieur gauche** de l'objet ou son **centre**. Un nombre que vous saisissez place ce point à l'endroit indiqué. Ce choix modifie seulement les valeurs affichées dans les cases ; l'objet et le fichier enregistré restent identiques dans les deux cas.

**Zone de texte** s'affiche pour les zones de texte et les formes :

- **Alignement vertical** place le texte en haut, au milieu ou en bas. Les choix **Centré** placent aussi le bloc de texte au milieu de la zone, dans le sens horizontal.
- **Orientation du texte** fait pivoter tout le texte d'un quart de tour. Cette option concerne les zones de texte.
- **Ajuster** détermine ce qui se passe lorsqu'il y a plus de texte que de place. **Ne pas ajuster** laisse le texte déborder. **Réduire le texte dans la zone de texte** diminue la taille des caractères pour les faire tenir. **Redimensionner la forme pour ajuster le texte** agrandit ou réduit la hauteur de la zone pour l'adapter au texte. Ces deux dernières options concernent les zones de texte. Le texte d'une forme conserve sa taille.
- **Marge gauche**, **Marge droite**, **Marge supérieure** et **Marge inférieure** indiquent l'espace entre le bord et le texte.
- **Renvoyer à la ligne automatiquement le texte dans la forme** : lorsque cette option est activée, une ligne de texte passe à la ligne au bord de la forme. Lorsqu'elle est désactivée, la ligne se prolonge jusqu'à ce que vous appuyiez sur Entrée.
- **Colonnes** et **Espacement** définissent le nombre de colonnes de texte et l'espace qui les sépare.

**Texte de remplacement** s'adresse aux personnes qui ne peuvent pas voir la diapositive. Un lecteur d'écran le lit à voix haute.

- **Titre** est un nom court.
- **Description** indique ce que l'objet montre ou pourquoi il est là. Appuyez sur **Tab** ou cliquez ailleurs pour conserver ce que vous avez saisi.
- **Marquer comme décoratif** s'applique à un objet qui ne fait qu'ajouter un aspect graphique, comme une ligne sous un titre. Un lecteur d'écran l'ignore. JeffDeck ne demande pas de description pour cet objet.

## Image

Lorsque seules des images sont sélectionnées, la page **Image** affiche :

- **Luminosité** et **Contraste** éclaircissent ou assombrissent l'image, et accentuent ou atténuent la différence entre les zones claires et sombres.
- **Recolorier** permet de passer l'image en **Nuances de gris** ou en **Noir et blanc**. **Niveau de noir et blanc** indique le degré de clarté qu'une zone doit avoir pour devenir blanche.
- **Saturation** rend les couleurs plus ou moins vives : -100 % donne du gris, 0 % laisse les couleurs telles quelles, et 100 % les rend au maximum de leur intensité.
- **Transparence** laisse voir à travers l'image ce qui se trouve derrière.
- **Rogner** indique où se situe l'image entière dans son cadre. **Largeur de l'image** et **Hauteur de l'image** correspondent à la taille de l'image complète. **Décalage X** et **Décalage Y** la déplacent par rapport au centre du cadre. Ce qui se trouve en dehors du cadre est masqué, mais conservé. Pour utiliser des poignées que vous pouvez faire glisser, utilisez **Rogner** dans l'onglet **Format de l'image**.

Ces modifications ne touchent jamais aux données de l'image. Elles sont enregistrées à côté d'elle.

## L'onglet Format de forme

L'onglet **Format de forme** apparaît lorsqu'une forme, une zone de texte, un trait ou un groupe de ces objets est sélectionné. Il disparaît lorsque vous sélectionnez une image, un tableau ou rien.

- **Formes** comporte **Modifier la forme**. Elle remplace les formes sélectionnées par une autre forme de la liste disponible dans **Insertion > Formes**. Leur texte, leur remplissage, leur contour, leurs effets, leur position et leur taille restent identiques. Les poignées jaunes de la forme reprennent celles de la nouvelle forme, et les points que vous avez déplacés à la main sont remplacés. Cette option ne concerne que les formes, et non les zones de texte, les traits ou les images. Elle constitue une seule étape : **Ctrl+Z** rétablit l'ancienne forme.
- **Styles de forme** commence par une galerie de présentations prêtes à l'emploi, et comporte **Remplissage de forme**, **Contour de forme** et **Effets de forme**.
  - **La galerie** comporte quatre lignes : **Contour uniquement** (un trait, sans remplissage), **Effet discret**, **Effet modéré** et **Effet intense** (chacun associe un remplissage à un trait, et les deux derniers à une ombre, de plus en plus marquée en descendant). Chaque ligne comprend six vignettes, une par couleur d'accentuation de votre thème. Pointez une vignette pour voir son nom, et cliquez dessus pour appliquer ce style à toutes les formes sélectionnées. Cette opération constitue une seule étape.
  - Un style provient du thème : les remplissages, les traits et les ombres sont ceux du thème, dans la couleur d'accentuation que vous avez choisie. La même vignette a un aspect différent dans un autre thème, et chaque vignette montre ce qu'elle donne dans le thème actuel. La forme mémorise le style qu'elle utilise. Un style définit aussi la couleur du texte de la forme, sauf les mots auxquels vous avez appliqué votre propre couleur.
  - **Remplissage de forme**, **Contour de forme** et **Effets de forme** ouvrent chacun un menu des choix ci-dessus, pour que vous n'ayez pas besoin du volet pour les options courantes. Ils modifient une seule partie d'un style et laissent le reste intact. Le dernier choix de chacun, comme **Autres options de remplissage…**, ouvre le volet à la page correspondante.
  - **Remplissage de forme** : les couleurs du thème, **Aucun remplissage**, **Autres couleurs de remplissage…**, **Image…** et **Dégradé** (six présentations de départ).
  - **Contour de forme** : les couleurs, **Aucun contour**, **Épaisseur**, **Tirets** et **Flèches**.
  - **Effets de forme** : **Ombre**, **Reflet**, **Lueur** et **Bordures adoucies**, chacun avec ses préréglages.
- **Styles de texte artistique** modifie les lettres des mots d'une forme ou d'une zone de texte, ainsi que du texte artistique. Pendant la saisie dans une zone, il modifie les mots sélectionnés. Lorsque vous sélectionnez la zone elle-même, il modifie tous ses mots. Lorsque vous sélectionnez le texte artistique, il modifie ce dernier. Cette option est désactivée pour un objet qui ne contient pas de mots.
  - **La galerie** propose les mêmes styles que **Insertion > Texte artistique**. Sur les mots d'une forme, un style applique aux lettres son remplissage, son contour et son ombre, et ne modifie pas la police. Sur le texte artistique, il applique le style complet (la police et la courbure aussi), et conserve les mots et la taille.
  - **Remplissage du texte** : les couleurs du thème, **Aucun remplissage** (lettres transparentes, de sorte que seul le contour apparaît), **Autres couleurs de remplissage…** et **Dégradé** (six présentations de départ, à partir de la couleur des lettres).
  - **Contour du texte** : les couleurs, **Aucun contour**, **Autres couleurs de contour…** et **Épaisseur**. Un contour commence avec une épaisseur d'un point.
  - **Effets sur le texte** : **Ombre**, **Reflet** et **Lueur**, chacun avec ses préréglages, et **Transformer**. Chaque effet ajouté reste en place à côté des autres. **Transformer** courbe le texte artistique en arc, en cercle, en vague et selon d'autres formes. Les mots d'une forme ne sont pas courbés : cette option est donc désactivée pour eux.
- **Accessibilité** comporte **Texte de remplacement**. Elle ouvre le volet à la section **Texte de remplacement**, avec le curseur placé dans la description.
- **Taille** comporte les cases **Hauteur** et **Largeur**. Saisissez un nombre pour modifier la taille de tous les objets sélectionnés.
- **Format** comporte **Volet Format**, qui ouvre et ferme le volet.

Chaque choix d'un menu constitue une seule étape, comme une modification effectuée dans le volet.

## Utiliser le clavier

- **Ctrl+1** ouvre le volet, et le clavier passe à ses onglets. Utilisez les touches **Gauche** et **Droite** pour passer d'un onglet à l'autre, et **Tab** pour accéder aux sections et aux cases.
- **Espace** ou **Entrée** sur le nom d'une section la replie ou la déplie.
- Dans une case numérique, **Haut** et **Bas** modifient légèrement le nombre. Saisissez un nombre et appuyez sur **Entrée** pour l'appliquer.
- **Échap** rend le clavier à la diapositive.

## Dans les fichiers

Un fichier .pptx conserve tous les réglages décrits ci-dessus. Un fichier .odp les conserve aussi, mais il ne dispose pas d'emplacement pour certains d'entre eux. JeffDeck les écrit donc d'une façon que seul JeffDeck lit : un trait en dégradé, un trait composé de deux ou trois lignes, un reflet, la taille d'une ombre, le verrouillage des proportions, les colonnes, la mention de décoration et le style d'une forme. Pour les lettres, il s'agit du remplissage en dégradé, du contour, de la lueur, du reflet, du flou d'une ombre et des lettres transparentes. Un autre programme qui ouvre ce fichier .odp affiche le trait dans sa première couleur, comme une ligne simple, sans reflet, et affiche les lettres sans mise en forme, dans une seule couleur, avec une ombre sans flou. JeffDeck restaure tous ces éléments. Un remplissage à motif est enregistré sous la trame ou la couleur unie la plus proche, et le niveau de noir et blanc d'une image n'est pas enregistré dans un fichier .odp.

Un style est enregistré dans un fichier .pptx de la même manière que les autres programmes. La forme indique quel aspect du thème elle utilise (un trait, un remplissage, une ombre et une couleur de texte, chacun dans une couleur d'accentuation), et ses propres réglages ne décrivent que ce que vous avez modifié ensuite. Ainsi, une forme que vous avez modifiée conserve vos changements, et une forme que vous n'avez pas modifiée s'affiche comme le thème la prévoit, quel que soit l'endroit où le fichier est ouvert. Une forme dans un fichier que vous ouvrez conserve son style de la même façon. Le contour et le remplissage en dégradé des lettres sont également enregistrés dans un fichier .pptx.

## Voir aussi

- [Formes, images, tableaux et texte artistique](objects)
- [Mettre en forme le texte](text)
- [Onglet Accueil](tab-home)
- [Utiliser JeffDeck au clavier](keyboard)

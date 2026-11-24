# Le volet Format de forme et l'onglet Format de forme
<!-- keywords: format de forme, format de l'image, volet format, volet, format de la forme, ctrl+1, remplissage, remplissage uni, dégradé, arrêts de dégradé, arrêt, remplissage avec image, texture, mosaïque, motif, trait, contour, épaisseur, type composé, type de tiret, type d'extrémité, type de jointure, flèches, flèche de début, flèche de fin, effets, ombre, reflet, lueur, bordures adoucies, taille, hauteur, rotation, échelle, verrouiller les proportions, position, zone de texte, marges, alignement vertical, orientation du texte, ajuster, réduire le texte en cas de dépassement, redimensionner la forme pour ajuster le texte, renvoi à la ligne automatique, colonnes, texte de remplacement, titre, description, décoratif, corrections de l'image, luminosité, contraste, recolorier, noir et blanc, transparence, rogner, remplissage de forme, contour de forme, effets de forme, outils de dessin, modifier la forme, modifier les points, fusionner les formes, union, combiner, soustraire, intersection, fragmentation, styles de forme, styles rapides, style, couleurs du thème, accent, styles de texte artistique, remplissage du texte, contour du texte, effets de texte, transformation, lettres -->

Le **volet Format de la forme** est un panneau situé à droite de la fenêtre. Il affiche l'apparence des objets sélectionnés et permet de la modifier. Une **forme**, une **zone de texte**, un **trait** et une **image** utilisent tous ce volet. Il comporte une page pour le remplissage et le contour, une pour les effets, une pour la taille et la position et, pour les images, une pour les couleurs de l'image.

## Ouvrir et fermer le volet

- Sélectionnez un objet, puis appuyez sur **Ctrl+1**.
- Ou faites un clic droit sur l'objet et choisissez **Format de la forme…**. (Pour une image, le menu indique **Format de l'image…**.)
- Ou cliquez sur **Volet Format** dans l'onglet **Format de forme**. Un second clic le ferme.
- Ou cliquez sur le petit bouton situé dans le coin d'un groupe de l'onglet **Format de forme**. Le coin du groupe **Styles de forme** ouvre la page **Remplissage et trait**, et le coin du groupe **Taille** ouvre la page **Taille et propriétés**.

Pour fermer le volet, cliquez sur le **×** en haut à droite, ou cliquez à nouveau sur **Volet Format**. JeffDeck mémorise si le volet était ouvert : il s'ouvre donc de la même façon la prochaine fois que vous démarrez le programme.

## Fonctionnement du volet

- **Il suit votre sélection.** Cliquez sur un autre objet et le volet affiche celui-ci. Lorsque rien n'est sélectionné, le volet l'indique.
- **Plusieurs objets à la fois.** Si vous sélectionnez plusieurs objets, une modification s'applique à tous. Lorsque leurs valeurs diffèrent, la zone reste vide. Si vous modifiez la zone, chaque objet reçoit votre valeur. Chaque objet conserve ce que vous n'avez pas modifié. Par exemple, trois formes de trois couleurs différentes gardent leurs couleurs lorsque vous modifiez leur transparence.
- **Chaque modification constitue une étape.** **Ctrl+Z** annule une modification. Lorsque vous faites glisser un curseur, les objets suivent le pointeur, et le glissement entier constitue une seule étape. Elle est enregistrée lorsque vous relâchez le bouton. Un nombre que vous augmentez ou diminuez en maintenant une touche fléchée fonctionne de la même façon.
- **Un groupe** applique un remplissage, un trait ou un effet à tous les objets qu'il contient. Sa taille et sa position lui sont propres.
- **Les objets verrouillés** conservent leur taille et leur position.

Les onglets sont représentés par des icônes en haut du volet : **Remplissage et trait**, **Effets**, **Taille et propriétés** et, lorsque vous avez sélectionné uniquement des images, **Image**. Pointez une icône pour voir son nom. Chaque page se compose d'une colonne de **sections**. Cliquez sur le nom d'une section pour la replier ou la déplier.

## Remplissage et trait

Le **remplissage** est la couleur ou l'image à l'intérieur d'une forme. Choisissez un type :

- **Aucun remplissage** laisse l'intérieur transparent.
- **Remplissage uni** correspond à une seule couleur. **Couleur** ouvre les couleurs du thème. **Transparence** indique la quantité de ce qui est visible derrière : 0 % masque ce qui se trouve derrière, et 100 % le laisse entièrement visible.
- **Remplissage en dégradé** fait se fondre plusieurs couleurs. **Type** propose **Linéaire** (le long d'une ligne), **Radial** (depuis le centre), **Rectangulaire** ou **Trajectoire** (le long du contour). Un dégradé linéaire possède aussi un **Angle** : 0° va de gauche à droite, et 90° de haut en bas.
  - La barre affiche les couleurs. Chaque petit repère situé sous celle-ci est un **arrêt**, un endroit où la couleur est exactement celle que vous avez choisie. Cliquez sur un repère pour le sélectionner, puis définissez sa **Couleur**, sa **Position** de 0 % à 100 % et sa **Transparence**.
  - Faites glisser un repère, ou appuyez sur les touches **Gauche** et **Droite**, pour le déplacer. Appuyez sur **Haut** et **Bas** pour en sélectionner un autre. **Ajouter un arrêt** en place un nouveau à mi-chemin du suivant. **Supprimer l'arrêt**, ou la touche **Suppr**, supprime l'arrêt sélectionné. Un dégradé conserve au moins deux arrêts. Vous pouvez aussi double-cliquer sur la barre pour ajouter un arrêt à l'endroit où vous cliquez.
- **Remplissage avec image ou texture** recouvre l'intérieur avec une image provenant d'un fichier. Cliquez sur **Insérer…** et choisissez un fichier. Cochez **Utiliser l'image comme texture en mosaïque** pour répéter l'image dans la forme, à l'**échelle** que vous avez définie. Si la case reste décochée, l'image est étirée pour remplir la forme. **La transparence** fonctionne aussi ici.
- **Remplissage à motif** utilise une petite image répétée, comme des rayures ou des points. Cliquez sur l'un des 13 motifs, puis choisissez sa **couleur de premier plan** (les traits) et sa **couleur d'arrière-plan** (ce qui se trouve entre eux).

Le **trait** est le contour d'une forme, ou la ligne elle-même. Choisissez **Aucun trait**, **Trait plein** ou **Trait dégradé**. Un trait dégradé fait se fondre des couleurs sur toute sa longueur. Définissez ensuite :

- la **Couleur** et la **Transparence**, ou les arrêts du dégradé ;
- l'**Épaisseur**, en points. Un point équivaut à 1/72 de pouce, soit environ 0,35 mm ;
- le **Type composé** : un trait simple, ou deux ou trois traits côte à côte, épais et fins ;
- le **Type de tiret** : plein, pointillé ou tireté ;
- le **Type d'extrémité** : l'aspect de la fin d'un trait. **Plat** s'arrête net à l'extrémité, **Rond** ajoute un demi-cercle, et **Carré** ajoute un demi-carré ;
- le **Type de jointure** : l'aspect d'un angle. **Arrondie**, **Biseautée** (coupée) ou **Onglet** (pointue) ;
- le **Type de flèche de début**, la **Taille de flèche de début**, le **Type de flèche de fin** et la **Taille de flèche de fin**, pour un trait qui possède des extrémités. Le contour d'une forme n'a pas d'extrémités : ces options n'apparaissent que lorsqu'un trait est sélectionné.

## Effets

Un **effet** est un aspect ajouté autour d'un objet. Chaque section comporte une case pour l'activer, une liste de **Préréglages** pour démarrer, et des zones pour le modifier. Un préréglage ne fait que remplir ces zones. Vous pouvez ensuite modifier chacune d'elles.

- **Ombre** : une copie sombre placée derrière l'objet. **Couleur** et **Transparence** fonctionnent comme pour un remplissage. **Taille** agrandit ou réduit l'ombre par rapport à l'objet, à partir de son centre. **Flou** adoucit son bord. **Angle** indique le côté où elle tombe (0° est à droite, 90° est en bas), et **Distance** indique son éloignement.
- **Reflet** : une copie estompée et retournée, placée sous l'objet. **Transparence** indique le niveau de départ de l'estompage. **Taille** indique la distance sur laquelle le reflet s'estompe. **Distance** est l'écart entre l'objet et son reflet. **Flou** adoucit le reflet.
- **Lueur** : un halo de couleur doux autour de l'objet. **Couleur**, **Taille** et **Transparence**.
- **Bordures adoucies** : estompe le bord de l'objet pour le fondre dans ce qui se trouve derrière. **Taille** indique la largeur du fondu.

## Taille et propriétés

**Taille** :

- **Hauteur** et **Largeur** s'expriment dans l'unité choisie dans les options (par exemple en centimètres). Vous pouvez saisir une autre unité, comme **2 po**.
- **Rotation** fait pivoter l'objet, en degrés. **Autres options de rotation**, dans le menu **Organiser**, ouvre cette page.
- **Échelle de hauteur** et **Échelle de largeur** s'expriment en pourcentage. Pour une image, 100 % correspond à la taille qu'elle a dans son fichier. Pour les autres objets, la valeur est de 100 % et l'objet est agrandi ou réduit selon le pourcentage saisi.
- **Conserver les proportions** garde le rapport entre hauteur et largeur : une nouvelle largeur modifie la hauteur en conséquence, et inversement. Une image est créée avec cette option activée. Elle détermine aussi si le fait de faire glisser un coin conserve les proportions. Maintenez **Maj** pendant le glissement pour faire l'inverse.

**Position** : **Position horizontale** et **Position verticale** indiquent l'emplacement de l'objet, mesuré depuis le coin supérieur gauche de la diapositive. **Position à partir de** choisit le point de mesure : le **coin supérieur gauche** de l'objet ou son **centre**. Un nombre que vous saisissez place ce point à l'endroit indiqué. Ce choix ne modifie que les valeurs affichées dans les zones : l'objet et le fichier enregistré restent identiques.

**Zone de texte** apparaît pour les zones de texte et les formes :

- **Alignement vertical** place le texte en haut, au milieu ou en bas. Les choix **Centré** placent aussi le bloc de texte au milieu de la zone, dans les deux sens.
- **Orientation du texte** fait pivoter tout le texte d'un quart de tour. Cette option concerne les zones de texte.
- **Ajuster** indique ce qui se passe lorsque le texte dépasse la place disponible. **Ne pas ajuster** laisse le texte déborder. **Réduire le texte en cas de dépassement** diminue la taille des caractères pour qu'il tienne. **Redimensionner la forme pour ajuster le texte** agrandit ou réduit la hauteur de la zone pour qu'elle contienne le texte. Ces deux dernières options concernent les zones de texte. Le texte d'une forme conserve sa taille.
- **Marge gauche**, **Marge droite**, **Marge supérieure** et **Marge inférieure** indiquent l'espace entre le bord et le texte.
- **Renvoyer à la ligne automatiquement le texte dans la forme** : lorsque cette option est activée, une ligne de texte passe à la ligne au bord de la zone. Lorsqu'elle est désactivée, la ligne se poursuit jusqu'à ce que vous appuyiez sur Entrée.
- **Colonnes** et **Espacement** définissent le nombre de colonnes de texte et l'espace qui les sépare.

**Texte de remplacement** s'adresse aux personnes qui ne voient pas la diapositive. Un lecteur d'écran le lit à voix haute.

- **Titre** est un nom court.
- **Description** indique ce que montre l'objet ou pourquoi il est là. Appuyez sur **Tab** ou cliquez ailleurs pour conserver ce que vous avez saisi.
- **Marquer comme décoratif** convient à un objet qui ne sert qu'à l'esthétique, comme un trait sous un titre. Un lecteur d'écran l'ignore. JeffDeck ne demande alors aucune description.

## Image

Lorsque seules des images sont sélectionnées, la page **Image** affiche :

- **Luminosité** et **Contraste** éclaircissent ou assombrissent l'image, et accentuent ou atténuent la différence entre les zones claires et sombres.
- **Recolorier** peut transformer l'image en **Niveaux de gris** ou en **Noir et blanc**. **Niveau de noir et blanc** indique le degré de clarté qu'une zone doit avoir pour devenir blanche.
- **Saturation** rend les couleurs plus ou moins vives : -100 % donne du gris, 0 % laisse les couleurs telles quelles, et 100 % les rend aussi vives que possible.
- **Transparence** permet de voir ce qui se trouve derrière l'image.
- **Rogner** indique où se trouve l'image entière dans son cadre. **Largeur de l'image** et **Hauteur de l'image** correspondent à la taille de l'image complète. **Décalage X** et **Décalage Y** la déplacent par rapport au centre du cadre. Ce qui se trouve en dehors du cadre est masqué, mais conservé. Pour utiliser les poignées que vous pouvez faire glisser, utilisez **Rogner** dans l'onglet **Format de l'image**.

Ces modifications ne touchent jamais aux données de l'image. Elles sont enregistrées à côté de celle-ci.

## L'onglet Format de forme

L'onglet **Format de forme** apparaît lorsqu'une forme, une zone de texte, un trait ou un groupe de ces éléments est sélectionné. Il disparaît lorsque vous sélectionnez une image, un tableau ou rien du tout.

- **Formes** contient **Modifier la forme**. Cette commande remplace les formes sélectionnées par une autre forme de la liste d'**Insertion > Formes**. Leur texte, leur remplissage, leur trait, leurs effets, leur position et leur taille restent inchangés. Les poignées jaunes d'une forme reprennent celles de la nouvelle forme, et les points déplacés à la main sont remplacés. Cette commande ne concerne que les formes, et non les zones de texte, les traits ou les images. Elle constitue une seule étape : **Ctrl+Z** rétablit l'ancienne forme.
- **Styles de forme** affiche d'abord une galerie de styles prêts à l'emploi, puis propose **Remplissage de forme**, **Contour de forme** et **Effets de forme**.
  - **La galerie** comporte quatre rangées : **Contour seul** (un trait, sans remplissage), **Effet discret**, **Effet modéré** et **Effet intense** (chacun associe un remplissage à un trait, et les deux derniers ajoutent une ombre, de plus en plus marquée en descendant). Chaque rangée compte six vignettes, une pour chacune des six couleurs d'accent de votre thème. Pointez une vignette pour voir son nom, puis cliquez dessus pour appliquer ce style à toutes les formes sélectionnées. Cette action constitue une seule étape.
  - Un style provient du thème : les remplissages, les traits et les ombres sont ceux du thème, dans la couleur d'accent que vous avez choisie. La même vignette peut donc avoir un aspect différent dans un autre thème, et une vignette indique ce qu'elle donnera dans le thème actuel. La forme mémorise le style qu'elle utilise. Un style définit aussi la couleur des mots de la forme, sauf les mots auxquels vous avez appliqué une couleur vous-même.
  - **Remplissage de forme**, **Contour de forme** et **Effets de forme** ouvrent chacun un menu contenant les choix décrits ci-dessus, ce qui évite d'ouvrir le volet pour les options courantes. Ces menus modifient une seule partie d'un style et laissent le reste inchangé. Le dernier choix de chaque menu, comme **Autres options de remplissage…**, ouvre le volet à la page correspondante.
  - **Remplissage de forme** : les couleurs du thème, **Aucun remplissage**, **Autres couleurs de remplissage…**, **Image…** et **Dégradé** (six modèles de départ).
  - **Contour de forme** : les couleurs, **Aucun contour**, **Épaisseur**, **Tirets** et **Flèches**.
  - **Effets de forme** : **Ombre**, **Reflet**, **Lueur** et **Bordures adoucies**, chacun avec ses préréglages.
- **Styles de texte artistique** modifie l'apparence des lettres d'une forme, d'une zone de texte ou d'un texte artistique. Lorsque vous saisissez du texte dans une zone, la commande s'applique aux mots sélectionnés. Lorsque vous sélectionnez la zone elle-même, elle s'applique à tous ses mots. Lorsque vous sélectionnez un texte artistique, elle s'applique à celui-ci. Elle est désactivée pour un objet qui ne contient aucun mot.
  - **La galerie** comporte les mêmes styles que **Insertion > Texte artistique**. Sur les mots d'une forme, un style donne aux lettres leur remplissage, leur contour et leur ombre, et laisse la police inchangée. Sur un texte artistique, il applique le style complet (la police et la courbure aussi) et conserve les mots et la taille.
  - **Remplissage du texte** : les couleurs du thème, **Aucun remplissage** (lettres transparentes, de sorte que seul le contour est visible), **Autres couleurs de remplissage…**, et **Dégradé** (six modèles de départ, à partir de la couleur des lettres).
  - **Contour du texte** : les couleurs, **Aucun contour**, **Autres couleurs de contour…** et **Épaisseur**. Un contour fait d'abord un point d'épaisseur.
  - **Effets de texte** : **Ombre**, **Reflet** et **Lueur**, chacun avec ses préréglages, ainsi que **Transformation**. Chaque effet ajouté se cumule avec les autres. **Transformation** courbe le texte artistique en arc, en cercle, en vague et selon d'autres formes. Les mots d'une forme ne peuvent pas être courbés : cette option est donc désactivée pour eux.
- **Accessibilité** contient **Texte de remplacement**. Cette commande ouvre le volet sur la section **Texte de remplacement**, avec le curseur placé dans la description.
- **Taille** contient les zones **Hauteur** et **Largeur**. Saisissez un nombre pour modifier la taille de tous les objets sélectionnés.
- **Format** contient **Volet Format**, qui ouvre et ferme le volet.

Chaque choix d'un menu constitue une étape, comme une modification effectuée dans le volet.

## Modifier la forme et fusionner des formes

- **Modifier les points** (dans **Formes**) affiche un carré blanc sur chaque point du contour de la forme sélectionnée. Faites glisser un point pour le déplacer. **Ctrl**+clic sur un point le supprime, et **Ctrl**+clic sur un bord en ajoute un. **Maj**+clic place un point personnel sur la forme, auquel les connecteurs peuvent se fixer, ou le retire. Une fois terminé, appuyez sur **Échap** ou cliquez à côté de la forme ; le lancement d'un diaporama ou du mode Lecture met aussi fin à cette action. La première modification transforme la forme en forme libre. Chaque modification constitue une étape d'annulation, et un trou dans la forme reste un trou.
- **Fusionner les formes** (dans **Formes**) transforme deux formes ou plus de la diapositive en de nouvelles formes. **Union** crée une seule forme couvrant tout ce qu'elles recouvrent. **Combiner** conserve ce qui n'est couvert que par une seule d'entre elles et découpe les zones de chevauchement. **Soustraire** retire les autres formes de la forme que vous avez sélectionnée en premier. **Intersection** ne conserve que ce qui est couvert par toutes les formes. **Fragmentation** les découpe en toutes les pièces qu'elles forment.

La nouvelle forme reprend le remplissage, le trait, les effets, le lien et les animations de la forme sélectionnée en premier, et se place à l'emplacement des anciennes formes. Ces dernières sont supprimées, ainsi que leurs animations. Le texte des formes est supprimé ; si plusieurs formes contiennent du texte, JeffDeck vous demande confirmation au préalable. Vous pouvez fusionner jusqu'à 64 formes à la fois. Ces commandes sont désactivées pour les formes situées dans un groupe (sélectionnez alors le groupe entier) et pour plus de 64 formes. Une fusion qui produirait plus de 256 morceaux, ou qui prendrait trop de temps, est refusée avec un message. Une fusion longue s'exécute en arrière-plan, avec une fenêtre dans laquelle vous pouvez cliquer sur **Annuler**. Une forme fusionnée est une forme libre : **Modifier les points** permet donc de la remodeler, et elle est enregistrée dans les fichiers `.pptx` et `.odp` comme toute autre forme.

## Utiliser le clavier

- **Ctrl+1** ouvre le volet, et le clavier passe à ses onglets. Utilisez les touches **Gauche** et **Droite** pour passer d'un onglet à l'autre, et **Tab** pour accéder aux sections et aux zones.
- **Espace** ou **Entrée** sur le nom d'une section la replie ou la déplie.
- Dans une zone numérique, **Haut** et **Bas** modifient légèrement le nombre. Saisissez un nombre puis appuyez sur **Entrée** pour le valider.
- **Échap** rend le clavier à la diapositive.

## Dans les fichiers

Un fichier .pptx conserve tous les réglages décrits ci-dessus. Un fichier .odp les conserve aussi, mais il ne prévoit pas d'emplacement pour certains d'entre eux. JeffDeck les enregistre donc d'une façon que lui seul sait lire : un trait en dégradé, un trait composé de deux ou trois lignes, un reflet, la taille d'une ombre, le verrouillage des proportions, les colonnes, le marquage décoratif et le style appliqué à une forme. Pour les lettres, il s'agit du remplissage en dégradé, du contour, de la lueur, du reflet, du flou d'une ombre et des lettres transparentes. Un autre programme qui ouvre ce fichier .odp affiche le trait dans sa première couleur, sous forme de ligne simple, sans reflet ; il affiche les lettres de façon simple, dans une seule couleur, avec une ombre sans flou. JeffDeck restaure tous ces éléments. Un remplissage à motif est enregistré sous la trame ou la couleur unie la plus proche, et le niveau de noir et blanc d'une image n'est pas enregistré dans un fichier .odp.

Un style est enregistré dans un fichier .pptx de la même manière que les autres programmes. La forme indique quel aspect du thème elle utilise (un trait, un remplissage, une ombre et une couleur de texte, chacun dans une couleur d'accent), et ses propres réglages ne décrivent que ce que vous avez modifié ensuite. Ainsi, une forme que vous avez modifiée conserve votre modification, et une forme que vous n'avez pas modifiée apparaît telle que le thème la définit, quel que soit l'endroit où le fichier est ouvert. Une forme d'un fichier que vous ouvrez conserve son style de la même manière. Le contour et le remplissage en dégradé des lettres sont également enregistrés dans un fichier .pptx.

## Voir aussi

- [Formes, images, tableaux et texte artistique](objects)
- [Mettre en forme le texte](text)
- [Onglet Accueil](tab-home)
- [Utiliser JeffDeck au clavier](keyboard)

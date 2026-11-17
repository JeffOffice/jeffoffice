# Créer et modifier des tableaux
<!-- keywords: tableau, grille, lignes, colonnes, cellules, sélectionner des cellules, sélectionner une ligne, sélectionner une colonne, plusieurs cellules, fusionner les cellules, fractionner les cellules, bordures, remplissage de cellule, format de tableau, style de tableau, insérer un tableau, dessiner un tableau, supprimer une ligne, quadrillage, orientation du texte, coupure de mots -->

Un tableau est une grille de lignes et de colonnes. Chaque case de la grille est une cellule, et chaque cellule contient son propre texte.

## Insérer un tableau

- Cliquez sur **Accueil > Objets > Tableau** (ou **Insertion > Tableaux > Tableau**). Faites glisser le pointeur sur la grille pour choisir le nombre de lignes et de colonnes, puis cliquez.
- Ou choisissez **Insérer un tableau…** en bas de ce menu. Dans la boîte de dialogue **Créer un tableau**, définissez **Nombre de lignes**, **Nombre de colonnes** et **Formats de tableau**.
- Ou choisissez **Dessiner un tableau** dans le menu **Tableau** de l'onglet **Accueil**, puis faites glisser un rectangle. Vous obtenez 3 lignes et 3 colonnes.

Le tableau apparaît au milieu de la page. Faites glisser son bord pour le déplacer.

## Coller des cellules sous forme de tableau

Copiez des cellules dans JeffCrunch, ou un tableau d'un document ou d'une page web, puis appuyez sur **Ctrl+V**. Les cellules s'insèrent sous forme de tableau dans la partie centrale supérieure de la page. Chaque cellule conserve sa police, sa taille, son gras, son italique, son soulignement et sa couleur de texte, son remplissage, ses bordures et la position de son texte. Les cellules fusionnées restent fusionnées, et le contenu correspond à ce qu'affichaient les cellules : un nombre apparaît donc tel qu'il était affiché. Le tableau n'est jamais plus large que la page, et un seul **Ctrl+Z** l'annule.

Si vous êtes en train de saisir du texte dans une cellule de tableau au moment du collage, les cellules remplissent le tableau à partir de cette cellule. Chaque cellule adopte la présentation de la cellule d'origine : une cellule sans remplissage ni bordure en arrive donc sans, et la cellule remplacée perd les siennes. Les cellules fusionnées et les largeurs de colonnes ne sont pas reprises dans un tableau déjà existant. Les cellules qui dépassent le bord du tableau sont omises, et la barre d'état l'indique. **Conserver la mise en forme source** crée aussi le tableau. **Conserver uniquement le texte** ne colle que les mots, et **Collage spécial** propose toujours ces options ainsi que les autres formes de collage. Une page qui contient autre chose qu'un tableau est collée comme auparavant. Un tableau de plus de 1 000 lignes, 64 colonnes ou 100 000 cellules, ou de plus de 256 lignes (le maximum qu'un tableau peut contenir), est collé en texte brut, et la barre d'état indique la raison.

## Saisir dans un tableau

Cliquez dans une cellule et saisissez du texte. Appuyez sur Tab pour passer à la cellule suivante et sur Maj+Tab pour revenir à la précédente. Appuyez sur Tab dans la dernière cellule pour ajouter une ligne. Une ligne s'agrandit pour s'adapter à son texte.

## Sélectionner des cellules, des lignes, des colonnes ou le tableau entier

- Cliquez dans une cellule pour travailler sur cette cellule.
- Pour sélectionner plusieurs cellules à la fois, faites glisser le pointeur d'une cellule à une autre. Vous pouvez aussi cliquer dans une cellule, puis maintenir Maj et cliquer dans une autre. Les cellules situées dans le rectangle délimité par les deux sont grisées, et un nouveau Maj+clic déplace l'angle opposé. Une cellule fusionnée est toujours sélectionnée en entier.
- Cliquez sur **Disposition du tableau > Tableau > Sélectionner** pour **Sélectionner la cellule**, **Sélectionner la ligne**, **Sélectionner la colonne** et **Sélectionner le tableau**. **Sélectionner la cellule** sélectionne tout le texte de la cellule où vous vous trouvez. **Sélectionner la ligne** et **Sélectionner la colonne** sélectionnent chaque cellule de la ligne ou de la colonne où vous vous trouvez. Si vous avez déjà sélectionné plusieurs cellules, elles sélectionnent chaque ligne ou colonne que ces cellules atteignent.
- Cliquez sur le bord extérieur du tableau, ou appuyez sur Échap pendant la saisie, pour sélectionner le tableau entier. Appuyez sur Échap lorsque des cellules sont sélectionnées pour revenir à la sélection de tout le tableau.

Vous pouvez tout faire au clavier. Appuyez sur Alt, puis sur J et L pour ouvrir **Disposition du tableau**. Appuyez ensuite sur S et L pour ouvrir le menu **Sélectionner**, utilisez les touches fléchées pour choisir une commande, et appuyez sur Entrée. Lorsque des cellules sont sélectionnées, Entrée ou F2 commence la saisie dans la première d'entre elles, et Suppr efface leur texte tout en conservant le tableau.

Les remplissages, les bordures, les diagonales, l'alignement, les marges de cellule et la mise en forme du texte (comme la police, la taille, le gras, l'italique, le soulignement, la couleur et l'alignement) s'appliquent à chaque cellule sélectionnée, et un seul Annuler les annule toutes. Ils s'appliquent à la cellule où vous vous trouvez si vous venez de cliquer dans une cellule, et à toutes les cellules lorsque le tableau entier est sélectionné. Les diagonales nécessitent que vous soyez dans une cellule ou que des cellules soient sélectionnées.

## Ajouter et supprimer des lignes et des colonnes

1. Cliquez dans une cellule voisine de l'emplacement souhaité, ou sélectionnez plusieurs cellules.
2. Dans **Disposition du tableau > Lignes et colonnes**, cliquez sur **Insérer au-dessus**, **Insérer en dessous**, **Insérer à gauche** ou **Insérer à droite**. Si des cellules sont sélectionnées, la nouvelle ligne ou colonne est insérée au-dessus, en dessous, à gauche ou à droite de toutes ces cellules.
3. Pour supprimer des lignes ou des colonnes, cliquez sur **Supprimer**, puis choisissez **Supprimer les lignes**, **Supprimer les colonnes** ou **Supprimer le tableau**. **Supprimer les lignes** retire toutes les lignes atteintes par la cellule où vous vous trouvez ou par vos cellules sélectionnées, et **Supprimer les colonnes** fait de même pour les colonnes.

Lorsque le tableau entier est sélectionné, **Insérer au-dessus** et **Insérer à gauche** ajoutent au début du tableau, et **Insérer en dessous** et **Insérer à droite** à la fin. Si une nouvelle ligne ou colonne traverse une cellule fusionnée, la cellule fusionnée s'agrandit pour l'intégrer. Pour supprimer une ligne ou une colonne, cliquez d'abord dans celle-ci. Un tableau conserve au moins une ligne et une colonne, et compte au maximum 256 lignes et 256 colonnes.

## Modifier les dimensions

- Saisissez un nombre dans **Disposition du tableau > Taille > Hauteur** ou **Largeur** pour redimensionner le tableau entier. Une nouvelle largeur met à l'échelle chaque colonne proportionnellement, et une nouvelle hauteur fait de même pour chaque ligne. Une ligne ne devient jamais plus basse que ne l'exige son texte lorsque **Agrandir pour ajuster le texte** est coché.
- Faites glisser un trait entre deux colonnes pour modifier leur largeur. Le tableau conserve la même largeur.
- Faites glisser un trait entre deux lignes pour modifier la hauteur d'une ligne.
- Cliquez sur **Disposition du tableau > Taille > Répartir les lignes** ou **Répartir les colonnes** pour les rendre égales.

## Fusionner et fractionner des cellules

Sélectionnez les cellules à réunir, puis choisissez **Disposition du tableau > Fusionner > Fusionner les cellules**. Elles deviennent une seule cellule, et leur texte se combine : un paragraphe par cellule, de gauche à droite puis de haut en bas. Si vous cliquez simplement dans une cellule, **Fusionner les cellules** la réunit avec la cellule située à sa droite. **Fractionner les cellules** transforme les cellules fusionnées en cellules distinctes.

## Couleurs et bordures

Utilisez l'onglet **Création de tableau**.

- **Formats de tableau** est une galerie de présentations prêtes à l'emploi. Un format définit les remplissages, les bordures et la mise en forme du texte des en-têtes.
- **Remplissage de la cellule** colore les cellules sélectionnées, la cellule où vous vous trouvez, ou tout le tableau.
- Pour les bordures, choisissez d'abord une **Couleur du trait** et une **Épaisseur du trait**. Cliquez ensuite sur **Bordures**, puis choisissez **Toutes les bordures**, **Bordures extérieures**, **Bordures intérieures**, **Aucune bordure** ou un seul côté, par exemple **Bordure supérieure**.
- **Diagonales** divise une cellule par une ligne allant d'un coin à l'autre. Choisissez **Diviser vers le bas** ou **Diviser vers le haut**. Chaque moitié a son propre texte : le texte de la cellule reste dans la première moitié (en bas à gauche d'un trait descendant, en haut à gauche d'un trait montant), et la seconde moitié commence vide. Cliquez dans une moitié pour y saisir du texte, ou appuyez sur Tab pour passer d'une moitié à l'autre, puis à la cellule suivante. Le texte se retourne à l'intérieur de sa moitié, si bien que ses lignes se raccourcissent vers la pointe du triangle. La ligne ne s'agrandit pas pour cela : agrandissez-la si du texte est coupé. Choisir **Diviser vers le bas** ou **Diviser vers le haut** pour une cellule déjà divisée inverse le sens du trait, et conserve sa couleur et son épaisseur. **Aucune division** supprime le trait et place le texte de la seconde moitié après celui de la première ; la ligne s'agrandit si le texte réuni a besoin de plus de place.

Pour placer le texte dans une cellule, utilisez les boutons de **Disposition du tableau > Alignement** : en haut, au milieu ou en bas, et à gauche, centré, à droite ou justifié. **Marges de cellule**, situé au même endroit, définit l'espace entre les bords d'une cellule et son texte : **Aucune**, **Étroites**, **Moyennes** ou **Larges**. Les boutons haut, milieu et bas ainsi que **Marges de cellule** modifient les cellules sélectionnées, la cellule où vous vous trouvez, ou toutes les cellules lorsque le tableau entier est sélectionné.

**Orientation du texte**, également dans **Disposition du tableau > Alignement**, fait pivoter le texte d'une cellule d'un quart de tour, si bien que ses lignes se lisent de haut en bas. Cette option convient à un titre au-dessus d'une colonne étroite. Cliquez de nouveau dessus pour remettre le texte à l'horizontale. La ligne ne s'agrandit pas pour s'adapter au texte pivoté : faites glisser le trait situé sous la ligne vers le bas jusqu'à ce que le texte tienne.

**Coupure de mots**, à côté, ouvre la boîte de dialogue **Coupure de mots**. Vous pouvez y activer ou désactiver **Coupure automatique des mots dans cet article**, ce qui détermine si les mots longs se coupent avec un trait d'union en fin de ligne, et vous définissez la **Zone de coupure**. Les mots des cellules sont coupés sauf si vous désactivez cette option. Voir [Orthographe, dictionnaire des synonymes, recherche et coupure de mots](proofing).

**Orientation du texte** modifie les cellules sélectionnées, la cellule où vous vous trouvez, ou toutes les cellules lorsque le tableau entier est sélectionné, et chaque cellule conserve son propre réglage. La coupure de mots est différente : le texte d'un tableau forme un seul article, si bien que ce réglage concerne l'ensemble du tableau, quelles que soient les cellules sélectionnées, et un fichier .pub le conserve ainsi. **Ajouter des traits d'union conditionnels** dans la boîte de dialogue agit sur le texte des cellules sélectionnées, de la cellule où vous vous trouvez, ou de toutes les cellules lorsque le tableau entier est sélectionné.

**Afficher le quadrillage** dans **Disposition du tableau > Tableau** affiche à l'écran des lignes pointillées autour de chaque cellule. Elles ne sont pas imprimées.

## Mettre en forme le tableau

Faites un clic droit sur le tableau et choisissez **Mettre en forme le tableau**. Vous pouvez aussi cliquer sur la petite flèche dans le coin de **Disposition du tableau > Alignement**, ce qui ouvre la boîte de dialogue sur **Propriétés de la cellule**. La boîte de dialogue comprend ces onglets :

- **Couleurs et traits** définit le remplissage derrière les cellules et le trait autour du tableau.
- **Taille** définit la hauteur, la largeur et la rotation du tableau. Cochez **Conserver les proportions** pour conserver les proportions du tableau lorsque vous faites glisser une poignée d'angle.
- **Disposition** définit l'emplacement du tableau sur la page et la manière dont le texte s'enroule autour de lui.
- **Propriétés de la cellule** définit l'alignement vertical, les quatre marges et **Faire pivoter le texte de 90°** (identique à **Orientation du texte**). Ces réglages s'appliquent aux cellules sélectionnées, à la cellule où vous vous trouvez, ou à toutes les cellules lorsque le tableau entier est sélectionné. Seuls les réglages que vous modifiez sont appliqués.
- **Texte de remplacement** décrit le tableau pour les personnes qui ne peuvent pas le voir.

## Voir aussi

- [Les onglets Tableau](tab-table)
- [Mettre en forme le texte](formatting-text)
- [Puces, numérotation et tabulations](lists-tabs)

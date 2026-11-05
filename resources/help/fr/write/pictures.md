# Ajouter et modifier des images

<!-- keywords: image, images, picture, insérer une image, photo, presse-papiers, coller une image, capture d'écran, habillage, habillage du texte, aligné sur le texte, carré, rapproché, haut et bas, derrière le texte, devant le texte, texte de remplacement, description, redimensionner, taille, poignées, déplacer une image, glisser une image, glisser-déposer, format de l'image, verrouiller les proportions -->

Une **image** peut être placée dans une ligne de texte, comme une lettre, ou flotter sur la page, le texte s'enroulant autour d'elle. Cette rubrique explique comment insérer une image dans un document, modifier sa taille, son emplacement et la façon dont le texte l'entoure, et la décrire pour les personnes qui ne peuvent pas la voir.

## Insérer une image

- Cliquez sur **Insertion > Images**, puis sur **Image**, choisissez un fichier et cliquez sur **Ouvrir**. JeffWrite lit les fichiers PNG, JPEG, GIF, BMP, TIFF, WebP et SVG. Une image provenant d'un fichier est conservée dans le document sous la forme de ce fichier (une image WebP ou SVG est conservée au format PNG).
- Ou cliquez sur **Insertion > Images > Image du presse-papiers**, ou appuyez sur **Ctrl+V**, lorsque vous avez copié une image (une capture d'écran, par exemple). L'image est conservée dans le document, elle reste donc présente lorsque vous enregistrez le fichier puis le rouvrez.
- Ou faites glisser un fichier image depuis l'explorateur de fichiers de votre ordinateur vers une page. L'image est placée à l'endroit où vous la déposez.

L'image est placée à l'emplacement du point d'insertion, à la place de tout ce que vous avez sélectionné. Elle prend la taille demandée, sans dépasser la largeur de la colonne ni la hauteur du texte de la page, et ses proportions sont conservées. L'image est ensuite sélectionnée et l'onglet **Format de l'image** apparaît à droite du ruban.

Une image trop grande (plus de 40 millions de pixels, un côté de plus de 30 000 pixels, ou un fichier de plus de 48 Mo) n'est pas insérée, et la barre d'état l'indique. Il en va de même pour un fichier qui n'est pas une image.

## Modifier la taille

Cliquez sur l'image pour la sélectionner. Huit petits carrés, les **poignées**, apparaissent sur ses coins et sur ses côtés.

- Faites glisser une poignée d'**angle** pour agrandir ou réduire l'image dans les mêmes proportions.
- Faites glisser une poignée de **côté** pour modifier un seul côté.
- Ou saisissez une valeur dans **Format de l'image > Hauteur** ou **Largeur**. Si **Conserver les proportions** est activé, l'autre valeur suit. Les images sont limitées entre 2 points et 2 000 points sur un côté.

Chaque modification correspond à une étape annulable.

## Habiller le texte autour d'une image

**Format de l'image > Habillage du texte** définit la façon dont le texte entoure l'image. Vous pouvez aussi faire un clic droit sur l'image et choisir **Habillage du texte**.

- **Aligné sur le texte** : l'image est un caractère de sa ligne et se déplace avec le texte qui l'entoure.
- **Carré** : le texte contourne un rectangle qui contient l'image.
- **Rapproché** : le texte suit l'image d'aussi près que le permet un rectangle (JeffWrite le traite comme Carré).
- **Haut et bas** : le texte s'arrête au-dessus de l'image et reprend en dessous.
- **Derrière le texte** : l'image se trouve sous le texte, qui passe directement par-dessus.
- **Devant le texte** : l'image se trouve par-dessus le texte, qui passe directement dessous.

## Déplacer une image

Faites glisser l'image. Une image **alignée sur le texte** se place à l'endroit du texte où vous relâchez le bouton. Une image avec un autre habillage se place à l'endroit de la page où vous relâchez le bouton et reste à cet endroit lorsque le texte qui l'entoure change (elle reste liée au paragraphe auquel elle appartient, sur la page de ce paragraphe).

Faire glisser une image alignée sur le texte ne modifie pas son habillage ; pour qu'elle flotte, choisissez un habillage tel que **Carré**.

## Décrire une image

Une image qui ne sert qu'à décorer la page, comme un filet ou une bordure, n'a pas besoin de description. Sélectionnez-la, puis cliquez sur **Format de l'image > Marquer comme décorative** (ou faites un clic droit sur l'image). Les lecteurs d'écran l'ignorent alors. Cliquez de nouveau dessus pour retirer ce marquage.

Le **texte de remplacement** est une courte description que les lecteurs d'écran lisent à voix haute, et qui s'affiche lorsque l'image ne peut pas l'être. Sélectionnez l'image, cliquez sur **Format de l'image > Texte de remplacement** (ou faites un clic droit sur l'image et choisissez **Texte de remplacement**), saisissez la description, puis cliquez sur **OK**.

## Images provenant d'autres documents

JeffWrite affiche et conserve les images des documents qu'il ouvre. Il peut modifier la taille, l'habillage, l'emplacement et le texte de remplacement d'une image que le document enregistre comme une image simple. Une image enregistrée sous une forme que JeffWrite ne modifie pas (un groupe de dessins, par exemple) est sélectionnée et conservée telle quelle ; ses commandes sont désactivées.

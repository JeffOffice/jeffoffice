# Impression, PDF et images des diapositives

<!-- keywords: imprimer, impression, imprimante, copies, assembler, assemblés, sélection, dernière page en premier, inverser, zoom, diapositives, pages de commentaires, documents, plan, pdf, exporter, image, png, jpeg, jpg, gif, tiff, tif, bmp, svg, vectoriel, image, masque des notes, masque du document, plage, diapositives masquées, cadre, niveaux de gris, noir et blanc, aperçu avant impression, papier, lettre, a4, mot de passe, chiffrer, protéger par un mot de passe, enregistrer en pdf, format pdf -->

Vous pouvez imprimer votre présentation, l'enregistrer au format PDF ou enregistrer une diapositive sous forme d'image. Ces trois opérations se trouvent dans l'écran Fichier.

## Imprimer

1. Appuyez sur Ctrl+P, ou cliquez sur **Fichier > Imprimer**.
2. À côté du bouton **Imprimer**, réglez **Copies**. Choisissez une **Imprimante** : la liste se termine par **Enregistrer au format PDF**, qui crée un fichier PDF à la place d'une impression sur papier. Avec plusieurs copies, **Assembler** imprime l'ensemble complet pour chaque copie (1, 2, 3, puis 1, 2, 3), et lorsque l'option est désactivée, les copies de chaque page sortent ensemble (1, 1, 2, 2, 3, 3). Un fichier PDF n'est créé qu'une seule fois, quel que soit le nombre de copies. **Propriétés de l'imprimante…** ouvre la fenêtre d'impression du système pour cette imprimante, avec ses propres options (le bac, la qualité) ; ce que vous y choisissez est conservé pour le travail, tandis que les copies, l'assemblage, le papier et les niveaux de gris reviennent sur cette page. **Papier** définit le format de la feuille ; **Comme indiqué dans les diapositives** est le choix habituel.
3. Sous **Imprimer**, choisissez ce qui doit sortir : **Diapositives**, **Pages de commentaires**, **Plan** ou **Documents** avec 1, 2, 3, 4, 6 ou 9 diapositives par page.
4. Pour les documents de 4 diapositives ou plus, **Ordre** indique si les diapositives se placent d'abord en largeur, puis en hauteur (**Horizontal**), ou d'abord en hauteur, puis en largeur (**Vertical**).
5. Sous **Plage**, choisissez **Toutes les diapositives**, **Diapositive active** ou **Sélection** (les diapositives sélectionnées dans la trieuse de diapositives, ou la diapositive affichée si vous n'êtes pas dans la trieuse), ou choisissez **Diapositives** et saisissez les numéros dans la zone : numéros de diapositives et plages séparés par des virgules, par exemple 1-3, 5.
6. Cochez ce dont vous avez besoin : **Dernière page en premier** (les feuilles sortent dans l'ordre inverse, si bien qu'une pile sort dans l'ordre ; en recto verso, un recto reste avec son verso, de sorte que les diapositives 1 à 4 sortent comme 3 et 4, puis 1 et 2 ; l'option est grisée lorsque vous imprimez recto verso manuellement), **Imprimer les diapositives masquées**, **Encadrer les diapositives** (un fin trait autour de chaque diapositive) et **Nuances de gris**.
7. Sous **Faces**, choisissez **Imprimer en recto**, **Imprimer recto verso, retournement sur le grand côté** (pages qui se tournent comme un livre), **Imprimer recto verso, retournement sur le petit côté** (pages qui se tournent comme un bloc-notes) ou **Imprimer recto verso manuellement**. Les deux premières options sont grisées si votre imprimante ne peut pas imprimer recto verso ; pointez l'une d'elles pour voir pourquoi. **Manuellement** est prévu pour une telle imprimante : JeffDeck imprime les pages 1, 3, 5 et ainsi de suite, puis un message vous demande de retourner la pile et de la replacer dans l'imprimante. Lorsque vous cliquez sur **OK**, il imprime les autres pages, la dernière en premier. Avec une seule page, il n'y a rien à retourner. JeffDeck mémorise votre choix.
8. Cliquez sur **Imprimer**. Pour un fichier PDF, JeffDeck demande un nom. Un fichier PDF n'a pas de faces.

L'aperçu à droite montre chaque page qui sortira, avec des flèches pour passer d'une page à l'autre. **Zoom**, au-dessus, agrandit la page au-delà de la taille qui tient à l'écran ; faites défiler pour voir le reste. L'aperçu est dessiné de la même manière que la page imprimée. JeffDeck mémorise vos choix pour la prochaine fois.

## Ce que chaque choix imprime

- **Diapositives** imprime une diapositive par page, dans sa propre forme. La feuille est placée en paysage pour une diapositive large.
- **Pages de commentaires** place une diapositive en haut de la page, avec ses notes du présentateur en dessous.
- **Documents** placent une grille de diapositives sur chaque page, avec un en-tête et un pied de page qui contiennent le titre, la date et le numéro de page. Avec 3 diapositives par page, des lignes sont prévues à côté de chaque diapositive pour écrire.
- **Plan** imprime le numéro et le titre de chaque diapositive, puis son texte, en retrait selon le niveau.

Les pages de commentaires, les documents et le plan s'impriment sur du papier Lettre ou A4, selon le format utilisé dans votre région, en position verticale. Un document s'imprime en paysage si son masque du document indique Paysage. La place de la diapositive, des notes, de l'en-tête, de la date, du pied de page et du numéro de page provient du masque des notes et du masque du document, que vous modifiez dans **Affichage > Modes Masque**. Les diapositives masquées ne sont pas imprimées, sauf si vous cochez **Imprimer les diapositives masquées**.

## Enregistrer au format PDF

Cliquez sur **Fichier > Exporter > Exporter au format PDF**. Une fenêtre demande ce qu'il faut inclure : les mêmes choix **Imprimer**, **Ordre** et **Plage** que pour l'impression, ainsi que les mêmes trois cases à cocher. Elle indique le nombre de pages que le fichier comptera. Cliquez sur **Exporter…**, puis choisissez un nom et un emplacement.

Le PDF contient du texte réel, avec les polices incorporées, de sorte qu'il s'affiche de la même façon sur tous les ordinateurs. Son titre est celui de la présentation, ou le nom du fichier.

**Balises de structure du document pour l'accessibilité** (activé par défaut) crée un PDF *balisé* : le titre de chaque diapositive devient un titre, ses objets sont placés dans l'ordre de lecture d'un lecteur d'écran (l'ordre de lecture de la diapositive), les images et les formes ont leur texte de remplacement, et un tableau reste un tableau. **Créer des signets** (activé par défaut) ajoute au PDF un signet pour chaque diapositive, portant le titre de la diapositive, ou « Diapositive 3 » si elle n'en a pas. La disposition Plan n'en comporte aucun.

Cochez **PDF/A, pour une conservation à long terme** pour créer un fichier PDF/A-1b. Toutes les polices sont incorporées au fichier, et les effets de transparence sont aplatis, de sorte que les diapositives restent identiques des années plus tard. Les balises et les signets sont conservés. Un PDF protégé par un mot de passe ne peut pas être un fichier PDF/A : choisir un mot de passe désactive donc PDF/A.

## Envoyer une présentation par courriel

Cliquez sur **Fichier > Exporter > Créer un courriel**. Choisissez la présentation sous forme de fichier ou de PDF. JeffDeck crée un fichier de message (.eml) qui la contient en pièce jointe, puis demande où l'enregistrer. Rien n'est envoyé. Ouvrez le fichier dans votre logiciel de messagerie, ajoutez les destinataires, puis envoyez-le. Une présentation protégée par un mot de passe est jointe avec ce mot de passe toujours en place. Un PDF peut avoir son propre mot de passe.

## Protéger un PDF par un mot de passe

Dans la fenêtre **Exporter au format PDF**, cliquez sur **Chiffrer avec un mot de passe…**. Saisissez le mot de passe deux fois. Toute personne qui ouvre le PDF devra alors le saisir. Le mot de passe protège réellement le fichier : les pages sont brouillées par un chiffrement fort (AES-256), et personne ne peut les lire sans lui.

Vous pouvez aussi cocher **Restreindre l'impression, la copie et la modification**, saisir un second mot de passe et choisir ce qui est autorisé. Ces limites ne sont qu'une demande. La plupart des lecteurs PDF les respectent, mais elles n'empêchent pas une personne déterminée de les contourner ; ne comptez donc pas sur elles pour garder un fichier privé.

Un mot de passe oublié ne peut pas être récupéré : conservez-le en lieu sûr. Le mot de passe sert pour ce seul PDF et n'est jamais enregistré.

Lorsque vous insérez comme image un PDF protégé par un mot de passe, JeffDeck demande ce mot de passe. S'il est erroné, il le redemande ; cliquez sur **Annuler** pour arrêter. La diapositive reçoit une copie de la page, qui ne nécessite pas de mot de passe. Si le propriétaire du PDF demande qu'il ne soit pas copié, JeffDeck l'insère tout de même et affiche une note qui le signale ; cette demande n'est pas un verrou.

## Enregistrer une diapositive sous forme d'image

Cliquez sur **Fichier > Exporter > Exporter la diapositive en tant qu'image**. JeffDeck enregistre la diapositive affichée au format image PNG, JPEG, GIF, TIFF, BMP ou SVG. Les formats à pixels (tous sauf SVG) font 1920 pixels de large, sur fond blanc. Un GIF comporte au maximum 256 couleurs : une photo peut donc paraître plus granuleuse.

Une image **SVG** est un dessin, et non une grille de pixels. Elle reste nette à toute taille, les lettres sont enregistrées sous forme de formes, de sorte que tous les ordinateurs affichent les mêmes, et les images de la diapositive sont incluses dans le fichier. Une diapositive qui utilise quelques effets, comme une ombre douce, peut paraître légèrement différente en SVG.

JeffDeck propose comme nom de fichier le nom de la présentation et le numéro de la diapositive.

Pour enregistrer toutes les diapositives, cliquez sur **Enregistrer les diapositives en tant qu'images** à côté de cette option. Choisissez le type d'image (PNG, JPEG, GIF, TIFF, BMP ou SVG) et un dossier. JeffDeck crée un fichier par diapositive, nommé **Diapositive 1**, **Diapositive 2**, et ainsi de suite. La zone **Largeur** définit le nombre de pixels en largeur et n'est pas utilisée pour SVG.

## Voir aussi

- [L'écran Fichier](file-screen)
- [Notes du présentateur](notes)
- [Ajouter, modifier et supprimer des diapositives](slides)

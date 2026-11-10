# Impression, PDF et images de diapositives

<!-- keywords: imprimer, impression, imprimante, copies, diapositives, pages de commentaires, documents, plan, pdf, exporter, export, image, png, jpeg, jpg, illustration, plage, diapositives masquées, encadrer, nuances de gris, noir et blanc, aperçu avant impression, papier, lettre, a4, mot de passe, chiffrer, protéger par un mot de passe -->

Vous pouvez imprimer votre présentation, l'enregistrer au format PDF ou enregistrer une diapositive sous forme d'image. Ces trois options se trouvent dans l'écran Fichier.

## Imprimer

1. Appuyez sur Ctrl+P, ou cliquez sur **Fichier > Imprimer**.
2. Sous **Imprimer**, choisissez ce qui sera imprimé : **Diapositives**, **Pages de commentaires**, **Plan** ou **Documents** avec 1, 2, 3, 4, 6 ou 9 diapositives par page.
3. Pour des documents de 4 diapositives ou plus, **Ordre** indique si les diapositives se suivent d'abord sur la largeur de la page (**Horizontal**) puis en descendant, ou d'abord en descendant (**Vertical**) puis sur la largeur.
4. Sous **Plage**, choisissez **Toutes les diapositives** ou **Diapositive actuelle**, ou choisissez **Diapositives** et saisissez les numéros dans la zone : numéros de diapositives et plages séparés par des virgules, par exemple 1-3, 5.
5. Cochez ce dont vous avez besoin : **Imprimer les diapositives masquées**, **Encadrer les diapositives** (un filet fin autour de chaque diapositive) et **Nuances de gris**.
6. Sous **Faces**, choisissez **Imprimer en recto**, **Imprimer recto verso, retournement sur le grand côté** (pages qui se tournent comme un livre), **Imprimer recto verso, retournement sur le petit côté** (pages qui se tournent comme un bloc-notes) ou **Imprimer recto verso manuellement**. Les deux premières options sont grisées si votre imprimante ne peut pas imprimer recto verso. Pointez l'une d'elles pour voir la raison. **Manuellement** convient à une telle imprimante : JeffDeck imprime les pages 1, 3, 5 et ainsi de suite, puis un message vous demande de retourner la pile et de la replacer dans l'imprimante. Lorsque vous cliquez sur **OK**, il imprime les autres pages, la dernière en premier. Avec une seule page, il n'y a rien à retourner. JeffDeck retient votre choix.
7. Cliquez sur **Imprimer**. JeffDeck ouvre la boîte d'impression de votre système, où vous choisissez l'imprimante et le nombre de copies. La boîte s'ouvre avec le choix de faces que vous avez fait. Un fichier PDF créé depuis cette boîte n'a pas de faces.

L'aperçu à droite montre chaque page qui sera imprimée, avec des flèches pour passer de l'une à l'autre. Il est dessiné de la même façon que la page sera imprimée. JeffDeck retient vos choix pour la prochaine fois.

## Ce que produit chaque choix

- **Diapositives** imprime une diapositive par page, dans la forme propre à la diapositive. Le papier est placé à l'horizontale pour une diapositive large.
- **Pages de commentaires** place une diapositive en haut de la page, avec ses commentaires du présentateur en dessous.
- **Documents** placent une grille de diapositives sur chaque page, avec un en-tête et un pied de page qui contiennent le titre, la date et le numéro de page. Avec 3 par page, des lignes sont prévues à côté de chaque diapositive pour écrire.
- **Plan** imprime le numéro et le titre de chaque diapositive, puis son texte, en retrait selon le niveau.

Les pages de commentaires, les documents et le plan s'impriment sur du papier A4 ou Lettre, en position verticale, selon le format utilisé dans votre région. Les diapositives masquées ne sont pas imprimées, sauf si vous cochez **Imprimer les diapositives masquées**.

## Enregistrer au format PDF

Cliquez sur **Fichier > Exporter > Exporter au format PDF**. Une fenêtre vous demande ce qu'il faut inclure : mêmes choix **Imprimer**, **Ordre** et **Plage** que pour l'impression, et mêmes trois cases à cocher. Elle indique le nombre de pages du fichier. Cliquez sur **Exporter…**, puis choisissez un nom et un emplacement.

Le PDF contient du texte réel, avec les polices incorporées, ce qui lui donne le même aspect sur tous les ordinateurs. Son titre est celui de la présentation, ou le nom du fichier.

Cochez **PDF/A, pour une conservation à long terme** pour créer un fichier PDF/A-1b. Toutes les polices sont incluses dans le fichier, et les effets transparents sont aplatis, afin que les diapositives restent identiques des années plus tard. Le fichier n'est pas balisé pour les lecteurs d'écran. Un PDF protégé par un mot de passe ne peut pas être au format PDF/A : choisir un mot de passe désactive donc le PDF/A.

## Envoyer une présentation par e-mail

Cliquez sur **Fichier > Exporter > Créer un e-mail**. Choisissez la présentation sous forme de fichier ou de PDF. JeffDeck crée un fichier de message (.eml) qui la contient en pièce jointe, puis demande où l'enregistrer. Rien n'est envoyé. Ouvrez le fichier dans votre messagerie, indiquez le destinataire, puis envoyez-le. Une présentation protégée par un mot de passe est jointe avec ce mot de passe. Un PDF peut avoir son propre mot de passe.

## Protéger un PDF par un mot de passe

Dans la fenêtre **Exporter au format PDF**, cliquez sur **Chiffrer avec un mot de passe…**. Saisissez le mot de passe deux fois. Toute personne qui ouvre le PDF devra alors le saisir. Le mot de passe protège réellement le fichier : les pages sont brouillées par un chiffrement robuste (AES-256), et personne ne peut les lire sans lui.

Vous pouvez aussi cocher **Restreindre l'impression, la copie et la modification**, saisir un second mot de passe et choisir ce qui est autorisé. Ces restrictions ne sont qu'une demande. La plupart des lecteurs PDF les respectent, mais elles n'empêchent pas une personne déterminée de les contourner : ne comptez pas sur elles pour garder un fichier confidentiel.

Un mot de passe oublié ne peut pas être récupéré : conservez-le en lieu sûr. Il sert uniquement à ce PDF et n'est jamais enregistré.

Lorsque vous insérez comme image un PDF protégé par un mot de passe, JeffDeck demande ce mot de passe. S'il est erroné, il le redemande ; cliquez sur **Annuler** pour arrêter. La diapositive reçoit une copie de la page qui ne demande pas de mot de passe. Si le propriétaire du PDF demande qu'il ne soit pas copié, JeffDeck l'insère tout de même et affiche une note le signalant ; cette demande n'est pas un verrou.

## Enregistrer une diapositive sous forme d'image

Cliquez sur **Fichier > Exporter > Exporter la diapositive en tant qu'image**. JeffDeck enregistre la diapositive active sous forme d'image PNG ou JPEG, large de 1920 pixels, sur fond blanc. Il propose comme nom de fichier le nom de la présentation et le numéro de la diapositive.

## Voir aussi

- [L'écran Fichier](file-screen)
- [Commentaires du présentateur](notes)
- [Ajouter, modifier et supprimer des diapositives](slides)

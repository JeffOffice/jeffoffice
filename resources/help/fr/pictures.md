# Ajouter et ajuster des images
<!-- keywords: photo, image, illustration, insérer une image, rogner, recadrer, images clipart, page PDF, jpg, png, svg, recolorier, texte de remplacement, texte alternatif, légende, espace réservé, gestionnaire de graphiques, images en ligne, lier au fichier, image liée, insérer et lier, incorporer, image manquante, image introuvable, rétablir le lien -->

Une image se trouve dans un cadre. Vous pouvez rogner l'image, et déplacer ou redimensionner le cadre.

## Insérer une image à partir d'un fichier

1. Cliquez sur **Insertion > Illustrations > Images**.
2. Dans la fenêtre **Insérer une image**, choisissez un fichier. JeffPub ouvre les fichiers PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO et PDF.
3. Choisissez **Insérer**, **Lier au fichier** ou **Insérer et lier** dans la zone **Insérer en tant que**, située avec la liste des fichiers et accessible avec la touche Tab. **Insérer** est sélectionné à chaque ouverture de la fenêtre. Voir « Lier une image à son fichier », plus bas.
4. Cliquez sur **Ouvrir**.

L'image est placée au milieu de la page. Vous pouvez aussi faire glisser un fichier image sur la page, ce qui utilise toujours **Insérer**.

JeffPub lit les fichiers image jusqu'à 256 Mo. Un fichier plus volumineux est refusé, de même qu'un fichier vide ou tout ce qui n'est pas un fichier ordinaire sur le disque, comme un périphérique ou un canal. Le message indique que JeffPub ne peut pas le lire en tant qu'image.

Si vous choisissez plusieurs images à la fois, elles s'alignent sous forme de miniatures dans la zone de travail, l'espace gris à côté de la page. Faites glisser sur la page celles que vous voulez.

Si un PDF comporte plusieurs pages, la fenêtre **Insérer une page PDF** affiche chaque page. Choisissez-en une et cliquez sur **OK**.

## Lier une image à son fichier

**Insérer** place une copie de l'image dans la composition. Cette copie suit la composition partout, mais une composition contenant beaucoup de grandes photos devient elle aussi volumineuse. Un lien permet au contraire de conserver les photos dans leurs propres fichiers.

- **Insérer** copie l'image dans la composition. C'est le choix habituel.
- **Lier au fichier** ne conserve que l'emplacement du fichier et un petit aperçu, de 512 pixels au plus sur son côté le plus long. L'image sur la page, et dans une impression, un PDF ou un livre électronique, provient du fichier en pleine qualité. Si le fichier est déplacé ou supprimé, la page affiche le petit aperçu à la place.
- **Insérer et lier** fait les deux. La composition conserve l'image complète et le lien. Quand vous ouvrez la composition et que le fichier a changé, JeffPub le relit et remplace sa copie.

JeffPub mémorise l'emplacement complet du fichier, ainsi que sa position par rapport à la composition, par exemple « le dossier Photos à côté ». Si vous déplacez la composition et ses images ensemble, c'est cette seconde information qui l'emporte : les images sont donc retrouvées. Conservez la même organisation des dossiers.

JeffPub n'ouvre de lui-même le fichier d'une image liée que si ce fichier se trouve dans le dossier de la composition ou dans un sous-dossier de celui-ci. Une image liée à un fichier situé ailleurs affiche son aperçu enregistré, flou, et est marquée **Non mise à jour**, jusqu'à ce que vous la sélectionniez et cliquiez sur **Mettre à jour le lien** dans le Gestionnaire de graphiques. Cela empêche une composition qu'on vous envoie d'accéder au contenu de votre ordinateur. Pour que les images se chargent automatiquement, conservez-les dans le dossier de la composition.

Le **Gestionnaire de graphiques** indique si chaque image est **Incorporée**, **Liée**, **Manquante**, **Modification** ou **Non mise à jour**, et propose des boutons pour mettre à jour un lien, le faire pointer vers un autre fichier ou le transformer en copie. Voir [Gestionnaire de graphiques](graphics-manager).

Quand vous enregistrez une composition en tant que fichier `.pub`, créez un fichier Pack and Go ou l'envoyez par courrier électronique, JeffPub y place chaque image liée en entier, afin que le fichier ne dépende pas de fichiers restés sur place. Si un fichier lié est alors manquant, ou si l'image est **Non mise à jour**, c'est son petit aperçu qui est inclus. Un fichier `.pub` que vous ouvrez ne contient jamais de liens : ses images sont importées sous forme de copies.

## Ajouter un espace réservé pour l'image

Un espace réservé est un cadre vide qui réserve un emplacement pour une image. Cliquez sur **Insertion > Illustrations > Espace réservé pour l'image**, puis double-cliquez sur le cadre et choisissez un fichier. L'image remplit le cadre, et les bords qui dépassent sont masqués. Un espace réservé vide ne s'imprime pas.

## Trouver des images gratuites en ligne

Cliquez sur **Insertion > Illustrations > Images en ligne**. Le volet **Images en ligne** effectue la recherche dans deux bibliothèques gratuites d'images sous licence libre : Openverse, qui regroupe de nombreuses collections, et Wikimedia Commons, la bibliothèque des images de Wikipédia.

1. Choisissez une bibliothèque, tapez ce que vous cherchez, par exemple « grange rouge », et appuyez sur Entrée.
2. Cliquez sur une image pour voir son auteur et sa licence. Une licence est l'ensemble des règles d'utilisation de l'image. La plupart demandent de citer l'auteur.
3. Cliquez sur **Insérer**. JeffPub place l'image sur la page et utilise son titre comme texte de remplacement. Si **Ajouter un crédit sous l'image** est coché, une ligne comme « Grange rouge » par Pat Lee, CC BY 2.0 est ajoutée en dessous.

Vos termes de recherche sont envoyés à la bibliothèque choisie. Vérifiez que la licence de chaque image convient à l'usage que vous en ferez.

Une recherche affiche jusqu'à 24 images. JeffPub n'utilise que les adresses web sécurisées (https) d'une bibliothèque et écarte toute image dont l'adresse ne l'est pas. Depuis Wikimedia Commons, une image de plus de 1 920 pixels de large est fournie sous forme de copie de cette largeur, et non le fichier d'origine. Un téléchargement de plus de 50 Mo, ou trop long, est interrompu, et le volet indique que l'image n'a pas pu être téléchargée ou chargée.

## Rogner une image

Le rognage masque les bords d'une image.

1. Cliquez sur l'image, puis sur **Format de l'image > Rogner > Rogner**.
2. Faites glisser une poignée pour masquer une partie de l'image. Faites glisser l'image pour la décaler dans le cadre.
3. Cliquez de nouveau sur **Rogner**, ou appuyez sur **Échap**.

Également dans le groupe **Rogner** : **Ajuster** affiche l'image entière dans son cadre, **Remplissage** remplit le cadre, **Rogner à la forme** découpe selon une forme, par exemple un ovale, et **Effacer le rognage** affiche de nouveau l'image entière.

## Modifier l'apparence d'une image

Dans l'onglet **Format de l'image**, utilisez le groupe **Ajuster** :

- **Corrections** : choisissez une combinaison de luminosité et de contraste, de moins 40 à plus 40 pour cent. **Format de l'objet…**, en bas du menu, propose des curseurs.
- **Recolorier** : passez l'image en Nuances de gris, Sépia, Estompé, Noir et blanc, ou dans une nuance d'accent. **Définir la couleur transparente** rend transparente la couleur du coin supérieur gauche de l'image.
- **Transparence…** : estompez toute l'image.
- **Rétablir l'image** : annulez ces modifications.
- **Compresser les images…** : réduisez la taille du fichier en abaissant la définition à 300, 220 ou 96 ppp (pixels par pouce). Par défaut, cette commande modifie toutes les images et supprime les zones rognées.
- **Convertir en formes** : transforme une image SVG (un dessin composé de traits et de formes) en formes modifiables, découpées là où le dessin les masque. Le texte et les images qu'elle contient sont ignorés.

## Légende, texte de remplacement et permutation

- **Format de l'image > Styles d'image > Légende** ajoute une zone de texte sous l'image.
- Le texte de remplacement est une courte description destinée aux personnes qui ne peuvent pas voir l'image, comme les utilisateurs de lecteurs d'écran. Cliquez avec le bouton droit sur l'image, choisissez **Format de l'objet…**, ouvrez l'onglet **Texte de remplacement** et tapez le texte dans **Texte de remplacement :**.
- **Format de l'image > Insertion > Modifier l'image** place un autre fichier dans le même cadre.
- Pour échanger deux images, faites glisser l'une sur l'autre. Quand l'info-bulle indique **Relâchez pour échanger les images**, relâchez : les images changent de cadre, et le cadre que vous avez fait glisser revient à sa place. Vous pouvez aussi maintenir Maj enfoncée, cliquer sur les deux images, puis cliquer sur **Permuter**. Maintenez Ctrl enfoncée pendant le glissement pour copier une image. Une copie relâchée sur une autre image reste là où vous la déposez, et rien n'est permuté.

## Voir toutes les images

Cliquez sur **Affichage > Afficher > Gestionnaire de graphiques** pour afficher la liste de toutes les images dans le volet **Gestionnaire de graphiques**. Sélectionnez-en une, puis cliquez sur **Atteindre** pour y accéder, sur **Enregistrer sous…** pour en enregistrer une copie ou sur **Remplacer…** pour choisir un autre fichier. Pour les images liées, le volet propose aussi **Mettre à jour le lien**, **Modifier le lien…** et **Incorporer l'image**. Voir [Gestionnaire de graphiques](graphics-manager).

## Voir aussi

- [Gestionnaire de graphiques](graphics-manager)
- [Dessiner des formes et des traits](shapes)
- [Déplacer, aligner, grouper et superposer des objets](arrange)
- [L'onglet Format de l'image](tab-picture-format)

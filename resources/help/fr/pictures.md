# Ajouter et ajuster des images

<!-- keywords: effets d'image, ombre, reflet, lueur, bords adoucis, biseau, rotation 3D, rétablir l'image et la taille, photo, image, insérer une image, rogner, image clipart, page PDF, jpg, png, svg, recolorier, texte de remplacement, légende, espace réservé, gestionnaire de graphiques, images en ligne, lien vers un fichier, image liée, insérer et lier, incorporer, image manquante, relier, recharger, ajuster, recadrer, détourage -->

Une image se place dans un cadre. Vous pouvez rogner l'image, puis déplacer ou redimensionner le cadre.

## Insérer une image depuis un fichier

1. Cliquez sur **Insertion > Illustrations > Images**.
2. Dans la fenêtre **Insérer une image**, choisissez un fichier. JeffPub ouvre les fichiers PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO et PDF.
3. Choisissez **Insérer**, **Lier au fichier** ou **Insérer et lier** dans la zone **Insérer en tant que**, située avec la liste des fichiers et accessible avec la touche Tab. **Insérer** est sélectionné à chaque ouverture de la fenêtre. Voir « Lier une image à son fichier », plus bas.
4. Cliquez sur **Ouvrir**.

L'image apparaît au milieu de la page. Vous pouvez aussi faire glisser un fichier image sur la page ; l'option **Insérer** est alors utilisée.

JeffPub lit les fichiers image jusqu'à 256 Mo. Un fichier plus volumineux est refusé, de même qu'un fichier vide ou tout élément qui n'est pas un fichier ordinaire sur le disque, comme un périphérique ou un tube. Le message indique que JeffPub ne peut pas le lire comme une image.

Si vous choisissez plusieurs images à la fois, elles s'alignent sous forme de vignettes sur la zone de travail, l'espace gris à côté de la page. Faites glisser celles que vous voulez sur la page.

Si un PDF comporte plusieurs pages, la fenêtre **Insérer une page PDF** affiche chacune d'elles. Choisissez-en une, puis cliquez sur **OK**.

## Lier une image à son fichier

**Insérer** place une copie de l'image dans la composition. Cette copie suit la composition partout où elle va, mais une composition contenant de nombreuses grandes photos devient elle-même volumineuse. Un lien permet de conserver les photos dans leurs propres fichiers.

- **Insérer** copie l'image dans la composition. C'est le choix habituel.
- **Lier au fichier** conserve seulement l'emplacement du fichier et un petit aperçu, d'au plus 512 pixels sur son plus grand côté. L'image affichée sur la page, ainsi que dans une impression, un PDF ou un livre électronique, provient du fichier en pleine qualité. Si le fichier est déplacé ou supprimé, la page affiche l'aperçu réduit.
- **Insérer et lier** combine les deux. La composition conserve l'image complète et le lien. Lorsque vous ouvrez la composition et que le fichier a changé, JeffPub le relit et remplace sa copie.

JeffPub mémorise l'emplacement complet du fichier, ainsi que son emplacement par rapport à la composition, par exemple « le dossier Photos à côté d'elle ». Si vous déplacez la composition et ses images ensemble, c'est ce second emplacement qui prévaut, et les images sont retrouvées. Conservez les dossiers tels qu'ils étaient.

JeffPub ouvre automatiquement le fichier d'une image liée uniquement lorsque ce fichier se trouve dans le dossier de la composition ou dans un de ses sous-dossiers. Une image liée à un fichier situé ailleurs affiche son aperçu enregistré, flou, et est marquée **Non mise à jour**, jusqu'à ce que vous la sélectionniez et cliquiez sur **Mettre à jour le lien** dans le **Gestionnaire de graphiques**. Ce fonctionnement empêche une composition que l'on vous envoie d'accéder à votre ordinateur. Pour que les images se chargent seules, placez-les dans le dossier de la composition.

Le **Gestionnaire de graphiques** indique si chaque image est **Incorporée**, **Liée**, **Manquante**, **Modifiée** ou **Non mise à jour**, et propose des boutons pour mettre à jour un lien, le faire pointer vers un autre fichier ou le transformer en copie. Voir [Gestionnaire de graphiques](graphics-manager).

Lorsque vous enregistrez une composition au format `.pub`, créez un fichier Pack and Go ou l'envoyez par courriel, JeffPub intègre chaque image liée en entier, afin que le fichier ne dépende pas de fichiers restés ailleurs. Si un fichier lié est manquant à ce moment, ou si l'image est **Non mise à jour**, son petit aperçu est intégré à la place. Un fichier `.pub` ouvert ne contient jamais de liens : ses images sont importées sous forme de copies.

## Ajouter un espace réservé pour une image

Un espace réservé est un cadre vide qui réserve un emplacement pour une image. Cliquez sur **Insertion > Illustrations > Espace réservé pour l'image**, puis double-cliquez sur le cadre et choisissez un fichier. L'image remplit le cadre, et les bords qui dépassent sont masqués. Un espace réservé vide n'est pas imprimé.

## Trouver des images libres en ligne

Cliquez sur **Insertion > Illustrations > Images en ligne**. Le volet **Images en ligne** recherche dans deux bibliothèques gratuites d'images sous licence libre : Openverse, qui regroupe de nombreuses collections, et Wikimedia Commons, la bibliothèque des images de Wikipédia.

1. Choisissez une bibliothèque, saisissez ce que vous cherchez, par exemple « grange rouge », puis appuyez sur Entrée.
2. Cliquez sur une image pour voir son auteur et sa licence. Une licence est l'ensemble des règles d'utilisation de l'image. La plupart demandent de citer l'auteur.
3. Cliquez sur **Insérer**. JeffPub place l'image sur la page et utilise son titre comme texte de remplacement. Si la case **Ajouter un crédit sous l'image** est cochée, une ligne du type « Grange rouge » de Pat Lee, CC BY 2.0 apparaît sous l'image.

Les mots de votre recherche sont envoyés à la bibliothèque choisie. Vérifiez que la licence de chaque image convient à votre usage.

Une recherche affiche jusqu'à 24 images. JeffPub n'utilise que les adresses web sécurisées (https) des bibliothèques et écarte les images dont l'adresse ne l'est pas. Depuis Wikimedia Commons, une image de plus de 1 920 pixels de large est fournie en copie à cette largeur, et non dans sa version d'origine. Un téléchargement de plus de 50 Mo, ou trop long, est interrompu, et le volet indique que l'image n'a pas pu être téléchargée ni chargée.

## Rogner une image

Le rognage masque les bords d'une image.

1. Cliquez sur l'image, puis sur **Format de l'image > Rogner > Rogner**.
2. Faites glisser une poignée pour masquer une partie de l'image. Faites glisser l'image pour la déplacer à l'intérieur du cadre.
3. Cliquez de nouveau sur **Rogner**, ou appuyez sur **Échap**.

Dans le groupe **Rogner** également : **Proportions** découpe le cadre selon une forme, comme un carré, 4 de large sur 3 de haut ou 16 sur 9, autour de son centre (l'image reste à sa place sur la page) ; **Ajuster** affiche l'image entière dans son cadre ; **Remplir** remplit le cadre ; **Rogner selon la forme** coupe l'image selon une forme, par exemple un ovale ; et **Effacer le rognage** affiche de nouveau l'image entière.

## Modifier l'aspect d'une image

Dans l'onglet **Format de l'image**, utilisez le groupe **Ajuster** :

- **Corrections** : choisissez une paire luminosité et contraste, de moins 40 à plus 40 pour cent. **Mettre en forme l'objet…**, en bas du menu, propose des curseurs.
- **Recolorier** : rend l'image en Nuances de gris, Sépia, Délavé, Noir et blanc, ou dans une teinte d'accent. **Définir la couleur transparente** rend transparente la couleur située dans le coin supérieur gauche de l'image.
- **Transparence…** : estompe l'image entière.
- **Rétablir l'image** : annule toutes les modifications de cette liste, ainsi que la bordure, la forme et les effets de l'image. Le rognage est également annulé, de sorte que le cadre affiche l'image entière. Cliquez sur la flèche à côté pour **Rétablir l'image et la taille**, qui remet aussi l'image à la taille qu'elle a dans son fichier.
- **Compresser les images…** : réduit la taille du fichier en abaissant le niveau de détail à 300, 220 ou 96 ppp (pixels par pouce). Par défaut, toutes les images sont modifiées et les zones rognées supprimées.
- **Convertir en formes** : transforme une image SVG (un dessin composé de traits et de formes) en formes modifiables, découpées là où le dessin les rogne. Le texte et les images qu'il contient sont omis.

## Styles, bordures et effets d'image

Utilisez le groupe **Styles d'image** de l'onglet **Format de l'image** :

- La **galerie** présente une vingtaine d'aspects prêts à l'emploi : cadres, ombres, reflets, bords adoucis, formes arrondies ou ovales, rotation dans l'espace, lueur et biseaux. Un biseau est un bord incliné qui donne à l'image un aspect en relief. Chaque vignette affiche vos couleurs. Cliquez sur l'une d'elles pour appliquer cet aspect à l'image. Un style définit en même temps la forme, la bordure et les effets, et remplace ceux que l'image possédait. Le rognage, la taille et les couleurs sont conservés.
- **Bordure de l'image** définit la couleur, l'**Épaisseur** (l'épaisseur du trait) et les **Tirets** (trait plein, pointillé ou tireté). **Aucun contour** supprime la bordure.
- **Effets de l'image** ouvre **Ombre**, **Reflet**, **Lueur**, **Bords adoucis**, **Biseau** et **Rotation 3D**. Chacun propose des choix prêts à l'emploi. Choisissez-en un pour l'ajouter ; les autres effets restent en place. Choisissez la première entrée, comme **Aucune ombre**, pour en retirer un. **Plus d'options d'effets…** ouvre **Mettre en forme l'objet**.

Chaque action peut être annulée en une seule étape.

## Légende, texte de remplacement et permutation

- **Format de l'image > Styles d'image > Légende** ajoute une zone de texte sous l'image.
- Le texte de remplacement est une courte description destinée aux personnes qui ne peuvent pas voir l'image, comme les utilisateurs de lecteurs d'écran. Faites un clic droit sur l'image, choisissez **Mettre en forme l'objet…**, ouvrez l'onglet **Texte de remplacement**, puis saisissez le texte dans **Texte de remplacement**.
- **Format de l'image > Insérer > Modifier l'image** place un autre fichier dans le même cadre. Le cadre conserve sa taille, son emplacement et son aspect : la bordure, la forme, les effets et les modifications de couleur. Si la nouvelle image a la même forme que l'ancienne, le rognage est conservé. Si elle a une autre forme, elle remplit le cadre.
- Pour échanger deux images, faites glisser l'une sur l'autre. Lorsque l'info-bulle affiche **Relâchez pour échanger les images**, relâchez le bouton : les images changent de cadre, et le cadre que vous avez fait glisser retourne à sa place. Vous pouvez aussi maintenir Maj, cliquer sur les deux images, puis cliquer sur **Permuter**. Maintenez Ctrl pendant le glissement pour copier une image au lieu de la déplacer. Une copie déposée sur une autre image reste à l'endroit choisi, et aucune permutation n'a lieu.

## Voir toutes les images

Cliquez sur **Affichage > Afficher > Gestionnaire de graphiques** pour lister toutes les images dans le volet **Gestionnaire de graphiques**. Sélectionnez une image, puis cliquez sur **Atteindre** pour vous y rendre, sur **Enregistrer sous…** pour enregistrer une copie, ou sur **Remplacer…** pour choisir un autre fichier. Pour les images liées, il propose aussi **Mettre à jour le lien**, **Modifier le lien…** et **Incorporer l'image**. Voir [Gestionnaire de graphiques](graphics-manager).

## Voir aussi

- [Gestionnaire de graphiques](graphics-manager)
- [Dessiner des formes et des traits](shapes)
- [Déplacer, aligner, grouper et superposer des objets](arrange)
- [L'onglet Format de l'image](tab-picture-format)

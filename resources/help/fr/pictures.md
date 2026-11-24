# Ajouter et ajuster des images
<!-- keywords: effets d'image, ombre, reflet, lueur, bordures adoucies, biseau, rotation 3D, rétablir l'image et la taille, photo, image, insérer une image, rogner, image clipart, page PDF, jpg, png, svg, recolorier, texte de remplacement, légende, espace réservé, gestionnaire de graphiques, images en ligne, lier au fichier, image liée, insérer et lier, incorporer, image manquante, relier à nouveau -->

Une image se trouve dans un cadre. Vous pouvez rogner l'image, puis déplacer ou redimensionner le cadre.

## Insérer une image à partir d'un fichier

1. Cliquez sur **Insertion > Illustrations > Images**.
2. Dans la fenêtre **Insérer une image**, choisissez un fichier. JeffPub ouvre les fichiers PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO et PDF.
3. Choisissez **Insérer**, **Lier au fichier** ou **Insérer et lier** dans la zone **Insérer en tant que**, qui se trouve avec la liste des fichiers et qui est accessible avec la touche Tab. **Insérer** est sélectionné à chaque ouverture de la fenêtre. Voir « Lier une image à son fichier », plus bas.
4. Cliquez sur **Ouvrir**.

L'image apparaît au milieu de la page. Vous pouvez aussi glisser un fichier image sur la page ; l'option **Insérer** est alors utilisée.

JeffPub lit les fichiers image jusqu'à 256 Mo. Un fichier plus volumineux est refusé, de même qu'un fichier vide ou tout élément qui n'est pas un fichier ordinaire sur le disque, comme un périphérique ou un tube. Le message indique que JeffPub ne peut pas lire le fichier comme une image.

Si vous choisissez plusieurs images à la fois, elles apparaissent sous forme de miniatures dans la zone de travail, l'espace gris à côté de la page. Faites glisser celles que vous voulez sur la page.

Si un PDF comporte plusieurs pages, la fenêtre **Insérer une page PDF** affiche chaque page. Choisissez-en une et cliquez sur **OK**.

## Lier une image à son fichier

**Insérer** place une copie de l'image dans la composition. Cette copie suit la composition partout, mais une composition contenant beaucoup de grandes photos devient volumineuse. Un lien garde les photos dans leurs propres fichiers.

- **Insérer** copie l'image dans la composition. C'est le choix habituel.
- **Lier au fichier** ne conserve que l'emplacement du fichier et un petit aperçu, d'au plus 512 pixels sur son plus grand côté. L'image sur la page, ainsi que dans une impression, un PDF ou un livre électronique, provient du fichier à pleine qualité. Si le fichier est déplacé ou supprimé, la page affiche le petit aperçu.
- **Insérer et lier** fait les deux. La composition conserve l'image entière et le lien. Lorsque vous ouvrez la composition et que le fichier a changé, JeffPub le relit et remplace sa copie.

JeffPub mémorise l'emplacement complet du fichier, ainsi que son emplacement par rapport à la composition, par exemple « le dossier Photos à côté d'elle ». Si vous déplacez la composition et ses images ensemble, c'est ce second emplacement qui prime, et les images sont retrouvées. Conservez les dossiers tels qu'ils sont.

JeffPub ouvre de lui-même le fichier d'une image liée seulement lorsque ce fichier se trouve dans le dossier de la composition ou dans un dossier qu'il contient. Une image liée à un fichier situé ailleurs affiche son aperçu enregistré, flou, et est marquée **Non mise à jour**, jusqu'à ce que vous la sélectionniez et cliquiez sur **Actualiser le lien** dans le Gestionnaire de graphiques. Cela évite qu'une composition envoyée par quelqu'un accède à votre ordinateur. Pour que les images se chargent automatiquement, placez-les dans le dossier de la composition.

Le **Gestionnaire de graphiques** indique si chaque image est **Incorporée**, **Liée**, **Manquante**, **Modifiée** ou **Non mise à jour**, et propose des boutons pour actualiser un lien, le faire pointer vers un autre fichier ou le transformer en copie. Voir [Gestionnaire de graphiques](graphics-manager).

Lorsque vous enregistrez une composition au format `.pub`, créez un fichier Pack and Go ou envoyez-la par e-mail, JeffPub intègre chaque image liée en entier au fichier, afin qu'il ne dépende pas de fichiers restés ailleurs. Si un fichier lié est alors absent, ou si l'image est **Non mise à jour**, son petit aperçu est intégré à la place. Un fichier `.pub` que vous ouvrez ne contient jamais de liens : ses images sont importées sous forme de copies.

## Ajouter un espace réservé pour l'image

Un espace réservé est un cadre vide qui réserve un emplacement pour une image. Cliquez sur **Insertion > Illustrations > Espace réservé pour l'image**, puis double-cliquez sur le cadre et choisissez un fichier. L'image remplit le cadre, et les bords qui dépassent sont masqués. Un espace réservé vide ne s'imprime pas.

## Trouver des images libres en ligne

Cliquez sur **Insertion > Illustrations > Images en ligne**. Le volet **Images en ligne** recherche dans deux bibliothèques gratuites d'images sous licence libre : Openverse, qui regroupe de nombreuses collections, et Wikimedia Commons, la bibliothèque des images de Wikipédia.

1. Choisissez une bibliothèque, saisissez ce que vous cherchez, par exemple « grange rouge », puis appuyez sur Entrée.
2. Cliquez sur une image pour voir son auteur et sa licence. Une licence est l'ensemble des règles d'utilisation de l'image. La plupart demandent de citer l'auteur.
3. Cliquez sur **Insérer**. JeffPub place l'image sur la page et utilise son titre comme texte de remplacement. Si l'option **Ajouter un crédit sous l'image** est cochée, une ligne telle que « Grange rouge » de Pat Lee, CC BY 2.0 apparaît sous l'image.

Vos mots de recherche sont envoyés à la bibliothèque choisie. Vérifiez que la licence de chaque image convient à votre utilisation.

Une recherche affiche jusqu'à 24 images. JeffPub n'utilise que les adresses web sécurisées (https) des bibliothèques et écarte toute image dont l'adresse ne l'est pas. Depuis Wikimedia Commons, une image plus large que 1 920 pixels est récupérée à cette largeur, et non dans sa version originale. Un téléchargement de plus de 50 Mo, ou trop long, est interrompu, et le volet indique que l'image n'a pas pu être téléchargée ou chargée.

## Rogner une image

Le rognage masque les bords d'une image.

1. Cliquez sur l'image, puis sur **Format de l'image > Rogner > Rogner**.
2. Faites glisser une poignée pour masquer une partie de l'image. Faites glisser l'image pour la déplacer à l'intérieur du cadre.
3. Cliquez de nouveau sur **Rogner**, ou appuyez sur **Échap**.

Également dans le groupe **Rogner** : **Proportions** découpe le cadre selon une forme, comme un carré, 4 de large sur 3 de haut, ou 16 sur 9, autour de son centre (l'image reste à sa place sur la page) ; **Ajuster** affiche l'image entière dans son cadre ; **Remplir** remplit le cadre ; **Rogner selon la forme** découpe selon une forme, comme un ovale ; et **Effacer le rognage** affiche de nouveau l'image entière.

## Modifier l'aspect d'une image

Dans l'onglet **Format de l'image**, utilisez le groupe **Ajuster** :

- **Supprimer l'arrière-plan** : retire l'arrière-plan d'une image, par exemple le mur uni derrière une personne ou le blanc autour d'un logo. Cliquez sur l'image, puis sur **Supprimer l'arrière-plan**. JeffPub repère les couleurs le long des bords de l'image et teinte en rose ce qui ressemble à l'arrière-plan. Si la suppression est trop forte ou trop faible, cliquez sur **Marquer les zones à conserver** ou **Marquer les zones à supprimer** dans l'onglet **Suppression de l'arrière-plan**, puis faites glisser sur l'image ; là où deux marques se rejoignent, c'est la dernière tracée qui l'emporte. **Conserver les modifications** (ou Entrée) termine l'opération et garde le résultat, et **Abandonner toutes les modifications** (ou Échap) rétablit tout. Toute la session compte pour une seule étape d'annulation. L'image n'est jamais coupée : cliquez de nouveau sur **Supprimer l'arrière-plan** pour continuer à marquer, ou sur **Rétablir l'image** pour faire revenir l'image entière. Une très grande image affiche une boîte avec **Annuler** pendant que JeffPub travaille. Les dessins faits de lignes et de formes (SVG) et les images animées ne peuvent pas perdre leur arrière-plan. Un fichier .jpub conserve la suppression afin que vous puissiez la modifier ensuite. **Rétablir l'image** annule la suppression faite ici ; **Modifier l'image** n'est pas disponible pendant le marquage. Passer à une autre page, supprimer l'image ou enregistrer met fin à la suppression de l'arrière-plan et conserve ce que vous avez marqué. Un fichier .pub, un PDF, une page web ou un livre électronique ne contiennent que l'image finale, et Vérifier la compatibilité le signale pour un .pub. L'image finale est enregistrée à la taille à laquelle elle est affichée : elle peut paraître floue si un autre programme l'agrandit. JeffPub la reconstruit à partir de l'original chaque fois que vous enregistrez ici.
- **Corrections** : choisissez un couple de luminosité et de contraste, de moins 40 à plus 40 pour cent. **Mettre en forme l'objet…**, en bas du menu, propose des curseurs.
- **Recolorier** : rend l'image en **Nuances de gris**, **Sépia**, **Délavé**, **Noir et blanc** ou dans une teinte d'accentuation. **Définir la couleur transparente** rend transparente la couleur située dans le coin supérieur gauche de l'image.
- **Transparence…** : estompe l'image entière.
- **Rétablir l'image** : annule toutes les modifications de cette liste, ainsi que la bordure, la forme et les effets de l'image. Le rognage est aussi annulé, de sorte que le cadre affiche l'image entière. Cliquez sur la flèche à côté pour **Rétablir l'image et la taille**, qui remet aussi l'image à la taille qu'elle a dans son fichier.
- **Compresser les images…** : réduit la taille du fichier en ramenant la résolution à 300, 220 ou 96 ppp (pixels par pouce). Par défaut, cela modifie toutes les images et supprime les zones rognées.
- **Convertir en formes** : transforme une image SVG (un dessin fait de lignes et de formes) en formes modifiables, découpées là où le dessin les rogne. Le texte et les images qu'elle contient ne sont pas repris.

## Styles, bordures et effets d'image

Utilisez le groupe **Styles d'image** de l'onglet **Format de l'image** :

- La **galerie** présente une vingtaine d'aspects prêts à l'emploi : cadres, ombres, reflets, bordures adoucies, formes arrondies et ovales, rotation dans l'espace, lueur et biseaux. Un biseau est une bordure en pente qui donne à l'image un aspect en relief. Chaque vignette affiche vos couleurs. Cliquez sur l'une d'elles pour appliquer cet aspect à l'image. Un style définit en même temps la forme, la bordure et les effets, et remplace ceux que l'image avait. Le rognage, la taille et les couleurs sont conservés.
- **Bordure de l'image** définit la couleur, l'**Épaisseur** (l'épaisseur du trait) et les **Tirets** (trait plein, pointillé ou en tirets). **Aucun contour** supprime la bordure.
- **Effets d'image** ouvre **Ombre**, **Reflet**, **Lueur**, **Bordures adoucies**, **Biseau** et **Rotation 3D**. Chacun propose des choix prêts à l'emploi. Choisissez-en un pour l'ajouter. Les autres effets restent en place. Choisissez la première entrée, comme **Aucune ombre**, pour en retirer un. **Plus d'options d'effets…** ouvre **Mettre en forme l'objet**.

Chacune de ces opérations compte pour une étape d'annulation.

## Légende, texte de remplacement et permutation

- **Format de l'image > Styles d'image > Légende** ajoute une zone de texte sous l'image.
- Le texte de remplacement est une courte description destinée aux personnes qui ne peuvent pas voir l'image, comme les utilisateurs de lecteurs d'écran. Faites un clic droit sur l'image, choisissez **Mettre en forme l'objet…**, ouvrez l'onglet **Texte de remplacement**, puis saisissez dans **Texte de remplacement**.
- **Format de l'image > Insérer > Modifier l'image** place un autre fichier dans le même cadre. Le cadre conserve sa taille, sa place et son aspect : la bordure, la forme, les effets et les modifications de couleur. Si la nouvelle image a la même forme que l'ancienne, le rognage est conservé. Si elle a une autre forme, elle remplit le cadre.
- Pour échanger deux images, faites glisser l'une sur l'autre. Lorsque l'info-bulle indique **Relâchez pour échanger les images**, relâchez : les images changent de cadre, et le cadre que vous avez fait glisser retourne à sa place. Vous pouvez aussi maintenir Maj, cliquer sur les deux images, puis cliquer sur **Permuter**. Maintenez Ctrl pendant le glissement pour copier une image. Une copie déposée sur une autre image reste à l'endroit où vous l'avez placée, et rien n'est échangé.

## Voir toutes les images

Cliquez sur **Affichage > Afficher > Gestionnaire de graphiques** pour lister toutes les images dans le volet **Gestionnaire de graphiques**. Sélectionnez une image, puis cliquez sur **Atteindre** pour vous y rendre, sur **Enregistrer sous…** pour enregistrer une copie, ou sur **Remplacer…** pour choisir un autre fichier. Pour les images liées, il y a aussi **Actualiser le lien**, **Modifier le lien…** et **Incorporer l'image**. Voir [Gestionnaire de graphiques](graphics-manager).

## Voir aussi

- [Gestionnaire de graphiques](graphics-manager)
- [Dessiner des formes et des traits](shapes)
- [Déplacer, aligner, grouper et superposer des objets](arrange)
- [L'onglet Format de l'image](tab-picture-format)

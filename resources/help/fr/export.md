# Enregistrer en tant qu'image, page web ou livre électronique
<!-- keywords: exporter, exportation, enregistrer en tant qu'image, png, jpeg, gif, tiff, svg, image, page web, html, livre électronique, livre numérique, e-book, epub, mise en page fixe, page par page, livre illustré, pack and go, imprimante photo, courrier électronique, e-mail -->

Utilisez **Fichier > Exporter** pour convertir votre composition en un autre type de fichier. Votre composition elle-même n'est pas modifiée. Pour les PDF, consultez [Fichiers PDF et imprimeurs](pdf). Pour les fichiers .pub, consultez [Utiliser les fichiers .pub](pub-files).

## Enregistrer des pages en tant qu'images

1. Cliquez sur **Fichier > Exporter**, puis sur **Enregistrer en tant qu'image**.
2. Tapez un nom. Sous **Type**, choisissez **PNG**, **JPEG**, **GIF**, **TIFF**, **Bitmap** ou **Dessin vectoriel SVG**.
3. Cliquez sur **Enregistrer**.
4. Pour tous les types sauf SVG, choisissez une **Résolution (points par pouce)** : 96, 150, 300 ou 600. Plus il y a de points, plus l'image est nette et plus le fichier est volumineux. 300 est un bon choix pour l'impression.

Chaque page est enregistrée dans son propre fichier. Si votre composition comporte plusieurs pages, JeffPub ajoute -1, -2, etc. au nom.

Un fichier **SVG** est un dessin composé de traits et de courbes : il reste donc net quelle que soit sa taille. JeffPub convertit les lettres en contours, de sorte que le fichier s'affiche de la même façon sur un ordinateur où vos polices ne sont pas installées.

## Enregistrer en tant que page web

Cliquez sur **Fichier > Exporter**, puis sur **Enregistrer en tant que page web**. JeffPub crée un fichier .html unique qui s'ouvre dans n'importe quel navigateur web. Chaque page de votre composition apparaît sous forme d'image : les mots qu'elle contient ne peuvent donc être ni sélectionnés ni modifiés. Chaque image est accompagnée d'une version texte des mots de la page pour les lecteurs d'écran, et les liens que vous avez ajoutés fonctionnent toujours.

## Enregistrer en tant que livre électronique

Le format **EPUB** est le format standard des livres électroniques. Les liseuses, les tablettes et les téléphones peuvent l'ouvrir. Vous avez le choix entre deux types :

- **Texte qui s'adapte à l'écran** : les mots se réorganisent en fonction de l'écran et de la taille du texte de chaque lecteur, comme dans un roman. Les pages ne correspondent pas une à une à votre mise en page.
- **Chaque page exactement telle que conçue** : chaque page s'affiche comme dans JeffPub, comme dans un livre illustré, un magazine ou une bande dessinée. Les lecteurs effectuent un zoom pour lire les petits textes. Les mots restent présents sous chaque page : les lecteurs peuvent donc y faire des recherches, et les lecteurs d'écran peuvent les lire à voix haute.

1. Cliquez sur **Fichier > Exporter**, puis sur **Enregistrer en tant que livre numérique (EPUB)**.
2. Choisissez le type, puis vérifiez le **Titre** et l'**Auteur**.
3. Pour un texte adaptable, laissez l'option **Utiliser la première page comme couverture** cochée, ou décochez-la. (Un livre page par page utilise toujours sa première page comme couverture.)
4. Cliquez sur **OK**, tapez un nom de fichier, puis cliquez sur **Enregistrer**.

Dans un livre adaptable, un nouveau chapitre commence à chaque paragraphe de style Titre 1. Les paragraphes Titre 1, 2 et 3 forment la table des matières dans les deux types ; un livre page par page répertorie chaque titre à la page où il se trouve. Les images et les tableaux restent à leur place. Les notes de bas de page s'affichent dans une fenêtre contextuelle, et les notes de fin sont placées à la fin du livre. Consultez [Styles](styles) et [Table des matières, notes de bas de page et notes de fin](references).

## Pack and Go et impressions photo

- **Pack and Go : enregistrer pour un imprimeur professionnel** vous demande un dossier. Cette commande enregistre un PDF pour impression commerciale (images en pleine résolution, avec les traits de coupe et les autres repères d'impression hors de la page), la composition et un court fichier texte indiquant la taille de la page et le nombre de pages. Si l'un de ces trois éléments ne peut pas être enregistré, par exemple parce que le dossier est plein ou protégé, un message vous en avertit au lieu de signaler la réussite de l'opération.
- **Pack and Go : enregistrer pour un autre ordinateur** crée un fichier ZIP unique contenant la composition et les polices fournies avec JeffPub qu'elle utilise. Si le fichier ZIP ne peut pas être enregistré, un message vous en avertit. Les images liées à leurs fichiers sont stockées en intégralité dans la composition. Une image liée marquée **Non mise à jour** emporte le petit aperçu qu'elle affiche, car JeffPub n'a pas ouvert son fichier.
- **Enregistrer pour une imprimante photo** vous demande un dossier et enregistre chaque page sous forme de fichier JPEG à 300 ppp.

## Partager par courrier électronique

Cliquez sur **Fichier > Partager**. Choisissez **Envoyer la page active par courrier électronique**, **Envoyer en tant que pièce jointe** ou **Envoyer au format PDF**. JeffPub n'envoie rien lui-même. Il enregistre un fichier .eml, c'est-à-dire un message électronique que vous ouvrez dans votre logiciel de messagerie. Vous y ajoutez ensuite les destinataires.

## Enregistrer comme modèle

**Fichier > Exporter > Enregistrer comme modèle** ajoute votre composition à **Mes modèles**. Consultez [Démarrer une nouvelle composition](create).

## Voir aussi

- [Fichiers PDF et imprimeurs](pdf)
- [Imprimer une composition](printing)
- [Utiliser les fichiers .pub](pub-files)
- [Ouvrir et enregistrer des compositions](open-save)

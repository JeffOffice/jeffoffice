# La boîte de dialogue Mise en page

<!-- keywords: mise en page, boîte de dialogue, en-tête, pied de page, numéros de page, date, première page, pages paires impaires, noir et blanc, brouillon, notes, erreurs, ordre des pages, centrer sur la page, numéro de première page, codes d'en-tête et de pied de page, qualité d'impression, ppp, points par pouce -->

Ouvrez-la à partir de la petite flèche située dans le coin du groupe **Mise en page** de l'onglet **Mise en page**. Elle comporte quatre onglets. **OK** modifie la feuille active, en une seule étape que vous pouvez annuler.

## Page

- **Orientation du papier** : hauteur ou largeur.
- **Taille d'impression des cellules** : un pourcentage de la taille réelle, ou **Réduire pour ajuster**, avec le nombre de pages en largeur et en hauteur. « Nombre quelconque » laisse cette direction prendre autant de pages que nécessaire.
- **Papier** : le format du papier.
- **Qualité d'impression** : le nombre de points par pouce demandé à l'imprimante. Plus il y a de points, plus l'impression est fine, mais certaines imprimantes sont plus lentes. Si un classeur demande une valeur absente de la liste, JeffCrunch la conserve.
- **Commencer la numérotation des pages à** : le numéro attribué à la première page. Sans cette option, la première page porte le numéro 1, et un classeur imprimé en entier numérote ses pages de feuille en feuille.

## Marges

Saisissez l'espace autour des cellules, ainsi que la distance entre l'en-tête et le pied de page et les bords supérieur et inférieur du papier. Les zones utilisent des centimètres si votre ordinateur est réglé sur une région métrique, et des pouces sinon. **Horizontalement** et **Verticalement** centrent les cellules sur la page.

## En-tête et pied de page

Un **en-tête** est une ligne de texte en haut de chaque page, et un **pied de page** est une ligne en bas. Choisissez-en un dans la liste, ou cliquez sur **Modifier l'en-tête** ou **Modifier le pied de page** pour écrire le vôtre. Chacun comporte une partie gauche, une partie centrale et une partie droite, ainsi que des boutons qui insèrent le numéro de page, le nombre de pages, la date, l'heure, le nom du fichier, le dossier et le nom de la feuille. Ces éléments changent d'une page à l'autre : ils sont calculés au moment de l'impression de chaque page. Pour placer une image dans une partie, cliquez dans cette partie puis choisissez **Image…**, et sélectionnez un fichier PNG, JPEG, GIF ou BMP. L'image est limitée à 5,08 cm (2 pouces) sur son plus grand côté, et elle s'imprime à l'emplacement du code `&G`. **Supprimer l'image** la retire. Une partie ne contient qu'une seule image, et la première page ainsi que les pages paires peuvent chacune avoir leurs propres images.

Vous pouvez aussi définir un en-tête et un pied de page différents pour la première page, ainsi que pour les pages paires et impaires, choisir si les caractères s'agrandissent et se réduisent avec les cellules, et si l'en-tête et le pied de page s'alignent sur les marges.

Les boutons reposent sur de courts codes enregistrés dans le fichier, que vous pouvez aussi saisir vous-même : **&P** correspond au numéro de page, **&N** au nombre de pages, **&D** à la date, **&T** à l'heure, **&F** au nom du fichier, **&A** au nom de la feuille, et **&&** à une esperluette. **&B**, **&I** et **&U** activent ou désactivent le gras, l'italique et le soulignement, **&12** fixe la taille à 12 points, et **&"Arial,Gras"** choisit une police. **&L**, **&C** et **&R** commencent respectivement les parties gauche, centrale et droite.

## Feuille

- **Cellules à imprimer** : la zone d'impression, sous forme de plages telles que **A1:D20, F1:G9**. Laissez ce champ vide pour tout imprimer.
- **Lignes à répéter en haut** et **Colonnes à répéter à gauche** : les titres à imprimer, par exemple **1:3** et **A:B**.
- **Imprimer aussi** : les quadrillages entre les cellules, les numéros de ligne et les lettres de colonne, le noir et blanc uniquement (sans couleurs ni trames), et un brouillon rapide (sans quadrillage, sans images, graphiques ni formes).
- **Notes** et **Cellules contenant une erreur** : ce qu'il faut en faire. Le réglage des erreurs modifie ce qu'affiche la page : la valeur telle qu'affichée dans la cellule, rien, deux tirets ou #N/A. Pour les notes : **Ne pas les imprimer** n'imprime aucune note. **Les lister après la dernière page** imprime chaque note après la dernière page de la feuille, sur des pages distinctes portant les numéros de page suivants et l'en-tête et le pied de page de la feuille, sous la forme « Cellule A2 : l'auteur : le texte de la note », dans l'ordre des cellules. **Les afficher à leur emplacement** est conservé dans le fichier pour d'autres programmes, mais JeffCrunch n'imprime pas encore les notes sur les cellules.
- **Ordre des pages** : de haut en bas puis de gauche à droite, ou de gauche à droite puis de haut en bas.

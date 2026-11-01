# La boîte de dialogue Mise en page
<!-- keywords: mise en page, boîte de dialogue, en-tête, pied de page, numéros de page, numérotation, date, première page, pages paires et impaires, noir et blanc, brouillon, notes, commentaires, erreurs, ordre des pages, centrer sur la page, premier numéro de page, codes d'en-tête et de pied de page -->

Ouvrez-la à l'aide de la petite flèche située dans le coin du groupe **Mise en page** de l'onglet **Mise en page**. Elle comporte quatre onglets. **OK** modifie la feuille active, en une seule opération que vous pouvez annuler.

## Page

- **Orientation du papier** : portrait ou paysage.
- **Taille d'impression des cellules** : un pourcentage de la taille normale, ou **Réduire pour ajuster**, avec le nombre de pages en largeur et en hauteur. « Un nombre quelconque de » permet d'utiliser autant de pages que nécessaire dans ce sens.
- **Papier** : le format du papier.
- **Commencer la numérotation des pages à** : le numéro attribué à la première page. Sans ce réglage, la première page porte le numéro 1, et lorsqu'un classeur est imprimé en entier, la numérotation des pages se poursuit d'une feuille à la suivante.

## Marges

Tapez l'espace autour des cellules, ainsi que la distance entre l'en-tête et le pied de page et les bords supérieur et inférieur du papier. Les zones utilisent les centimètres si votre ordinateur est configuré pour une région métrique, et les pouces dans les autres cas. **Horizontalement** et **Verticalement** placent les cellules au milieu de la page.

## En-tête et pied de page

Un **en-tête** est une ligne de texte en haut de chaque page, et un **pied de page** une ligne en bas. Choisissez-en un dans la liste, ou choisissez **Modifier l'en-tête…** ou **Modifier le pied de page…** pour rédiger le vôtre. Chacun comporte une partie gauche, une partie centrale et une partie droite, ainsi que des boutons qui insèrent le numéro de page, le nombre de pages, la date, l'heure, le nom du fichier, le dossier et le nom de la feuille. Ces éléments changent d'une page à l'autre ; ils sont donc calculés au moment de l'impression. Pour insérer une image dans une partie, cliquez dans cette partie et choisissez **Image…**, puis sélectionnez un fichier PNG, JPEG, GIF ou BMP. L'image mesure au plus 5 cm sur son côté le plus long, et elle s'imprime à l'emplacement du code `&G`. **Supprimer l'image** la retire. Chaque partie peut contenir une image, et la première page et les pages paires peuvent chacune avoir leurs propres images.

Vous pouvez également définir un en-tête et un pied de page différents pour la première page, ainsi que pour les pages paires et impaires, et choisir si le texte suit la mise à l'échelle des cellules et si l'en-tête et le pied de page s'alignent sur les marges.

Derrière les boutons se trouvent des codes courts que le fichier conserve, et que vous pouvez taper vous-même : **&P** correspond au numéro de page, **&N** au nombre de pages, **&D** à la date, **&T** à l'heure, **&F** au nom du fichier, **&A** au nom de la feuille, et **&&** à une esperluette. **&B**, **&I** et **&U** activent et désactivent le gras, l'italique et le soulignement, **&12** définit la taille à 12 points, et **&"Arial,Bold"** choisit une police. **&L**, **&C** et **&R** marquent le début des parties gauche, centrale et droite.

## Feuille

- **Cellules à imprimer** : la zone d'impression, sous forme de plages comme **A1:D20, F1:G9**. Laissez cette zone vide pour tout imprimer.
- **Lignes à répéter en haut** et **Colonnes à répéter à gauche** : les titres à imprimer, par exemple **1:3** et **A:B**.
- **Imprimer aussi** : le quadrillage, les numéros de ligne et les lettres de colonne, en noir et blanc uniquement (sans couleurs ni remplissages), et en qualité brouillon (sans quadrillage, ni images, graphiques ou formes).
- **Notes** et **Cellules contenant une erreur** : le traitement à leur appliquer. Le réglage des erreurs modifie ce qui apparaît sur la page : tel qu'affiché dans la cellule, rien, deux tirets ou #N/A. Pour les notes : **Ne pas les imprimer** n'imprime aucune note. **Les lister après la dernière page** imprime chaque note après la dernière page de la feuille, sur des pages distinctes qui portent les numéros de page suivants ainsi que l'en-tête et le pied de page de la feuille, sous la forme « Cellule A2 : l'auteur : le texte de la note », dans l'ordre des cellules. **Les afficher à leur emplacement** est conservé dans le fichier pour d'autres programmes, mais JeffCrunch n'imprime pas encore les notes sur les cellules.
- **Ordre des pages** : vers le bas de la feuille puis en largeur, ou en largeur puis vers le bas.

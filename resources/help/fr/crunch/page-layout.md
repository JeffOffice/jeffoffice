# Mettre en place les pages
<!-- keywords: thème, thèmes, couleurs du thème, polices du thème, couleurs, polices, accent, police des titres, police du corps de texte, arrière-plan, image derrière les cellules, mosaïque, papier peint, mise en page, marges, orientation, portrait, paysage, papier, taille, zone d'impression, saut de page, sauts de page, titres à imprimer, titres, répéter les lignes, mise à l'échelle, ajuster à la page, quadrillage, en-têtes, options de feuille, de droite à gauche, miroir -->

L'onglet **Mise en page** détermine comment une feuille est découpée en pages lorsqu'elle est imprimée ou transformée en PDF. Chaque feuille conserve ses propres choix, enregistrés dans le classeur. Chaque modification correspond à une étape que vous pouvez annuler.

## Thèmes, couleurs et polices

Un classeur possède un **thème** : douze couleurs et deux polices (une pour les titres et une pour le corps du texte) que ses cellules, styles de cellule, styles de tableau, formes et graphiques peuvent utiliser sans en copier les valeurs. Lorsque vous modifiez le thème, tout ce qui s'y réfère change avec lui : une cellule remplie de « Accent 1, plus clair 40 % » prend le nouvel Accent 1, à la même variante plus claire.

- **Thèmes** affiche des thèmes prêts à l'emploi qui définissent couleurs et polices ensemble. **Rechercher des thèmes** prend le thème d'un fichier de thème, d'un classeur, d'un document ou d'une présentation. **Enregistrer le thème actuel** conserve le thème de ce classeur dans un fichier, et **Rétablir le thème du classeur** restitue le thème que le classeur avait à l'ouverture.
- **Couleurs** ne modifie que les douze couleurs. **Créer de nouvelles couleurs de thème** vous permet de les définir et de les nommer ; toutes les applications JeffOffice les proposent ensuite.
- **Polices** ne modifie que les polices des titres et du corps du texte. Les cellules qui utilisent les polices du thème suivent ce changement ; celles qui sont définies par un nom de police la conservent.

Chaque choix correspond à une étape que vous pouvez annuler. Les boutons de couleur (**Couleur de remplissage**, **Couleur de police**, **Bordures** et **Couleur d'onglet**) affichent les couleurs du thème en ligne, avec des versions plus claires et plus foncées sous chacune ; une couleur choisie là reste une couleur du thème et change avec lui. Une couleur tirée de **Couleurs standard** ou de **Autres couleurs** reste telle que vous l'avez choisie.

Un nouveau classeur démarre avec le thème propre à JeffOffice. Une forme ou un graphique créé ici conserve les couleurs qui lui ont été données ; une forme venue du fichier qui fait référence à une couleur du thème suit le thème. Un classeur ouvert depuis un ancien fichier binaire ou un fichier OpenDocument n'a pas de thème à modifier : ces boutons ne proposent alors rien. Lorsque vous enregistrez un classeur au format OpenDocument, qui ne connaît pas les thèmes, les couleurs sont écrites telles qu'elles apparaissent.

## Marges, orientation et taille

- **Marges** définit l'espace vide autour des cellules. **Standard**, **Larges** et **Étroites** sont des jeux prédéfinis. **Marges personnalisées** ouvre la boîte de dialogue Mise en page, où vous pouvez saisir chacune d'elles et indiquer à quelle distance du bord de la feuille se trouvent l'en-tête et le pied de page.
- **Orientation** est **Portrait** (le papier est plus haut que large) ou **Paysage** (plus large que haut).
- **Taille** correspond au format du papier : Lettre, Légal, A4 et autres. **Autres formats de papier** les présente tous.

## Zone d'impression

Par défaut, une feuille imprime tout ce qui contient quelque chose : valeurs, formules, cellules avec une bordure ou un remplissage visible, ainsi que tout graphique, image ou forme. Pour n'imprimer qu'une partie :

1. Sélectionnez les cellules voulues.
2. Choisissez **Zone d'impression > Définir la zone d'impression**.

**Ajouter la sélection à la zone d'impression** ajoute un second bloc de cellules à la zone d'impression. Chaque bloc s'imprime sur ses propres pages. **Effacer la zone d'impression** revient à l'impression de tout. Les lignes ou colonnes insérées ou supprimées à l'intérieur de la zone d'impression la modifient avec les cellules.

## Sauts de page

L'endroit où une page se termine et où la suivante commence est calculé à partir du papier, des marges et de la taille de vos lignes et colonnes : une ligne ou une colonne n'est jamais coupée en deux. Pour terminer une page plus tôt, sélectionnez la cellule située sous et à droite de l'endroit où le saut doit se trouver, puis choisissez **Sauts > Insérer un saut de page**. Si une ligne entière est sélectionnée, le saut est placé au-dessus d'elle ; si une colonne entière est sélectionnée, il est placé à sa gauche. **Supprimer le saut de page** retire le saut situé au-dessus ou à gauche de la cellule sélectionnée, et **Rétablir tous les sauts de page** retire tous les sauts que vous avez définis. Les sauts de page que vous avez définis sont ignorés lorsqu'une feuille est réduite pour tenir sur les pages.

## La présentation des sauts de page

**Affichage > Sauts de page** (ou le bouton en bas à droite de la fenêtre) montre la feuille découpée en pages d'impression. Les cellules qui s'impriment restent telles quelles, tout le reste est grisé, et chaque page porte son numéro au-dessus de ses cellules. Une ligne en pointillés correspond à un saut calculé par JeffCrunch, et une ligne pleine à un saut que vous avez défini.

- **Déplacer un saut.** Faites glisser une ligne vers une autre ligne ou colonne. Elle devient un saut que vous avez défini à cet endroit, et les pages qui suivent sont recalculées.
- **Supprimer un saut.** Faites glisser une ligne hors des cellules imprimées, sur les numéros de ligne ou les lettres de colonne.
- **Modifier la zone d'impression.** Faites glisser le bord bleu des cellules imprimées. Si aucune zone d'impression n'est définie, les cellules qui s'impriment naturellement forment une zone, dont ce bord est déplacé.

Chaque glissement correspond à une étape d'annulation. Pendant qu'une feuille est réduite pour tenir sur les pages, ses sauts sont ignorés, et les lignes ne peuvent donc pas être déplacées. **Normal** rétablit les cellules telles qu'elles étaient. L'affichage dans lequel chaque feuille a été laissée est enregistré avec le classeur.

## La présentation Mise en page

**Affichage > Mise en page** (ou le bouton du milieu en bas à droite de la fenêtre) montre la feuille sous forme de pages d'impression, les unes sous les autres, chacune représentée par une feuille de papier avec ses marges et un espace entre les pages. Des règles longent le haut et le côté gauche. L'en-tête et le pied de page sont dessinés à l'endroit où ils s'impriment, de même que les images qu'ils contiennent.

- **Travailler dans les cellules.** Cliquez sur une cellule pour la sélectionner, faites glisser pour en sélectionner plusieurs, et utilisez les touches fléchées, la saisie, F2 et le double-clic comme dans l'affichage Normal. Ce que vous tapez s'inscrit dans une bande placée au-dessus de la cellule, sur sa page.
- **Commencer depuis l'onglet Insertion.** **Insertion > En-tête et pied de page** affiche les pages (si elles ne le sont pas déjà) et ouvre la partie centrale de l'en-tête de la première page, prête à la saisie. Cliquez sur une autre partie de l'en-tête ou du pied de page pour y écrire.
- **Saisir un en-tête ou un pied de page.** Placez le pointeur sur la marge supérieure ou inférieure d'une page : ses parties gauche, centrale et droite s'éclairent. Cliquez sur l'une d'elles et saisissez le texte. L'onglet **Outils d'en-tête et de pied de page** apparaît, avec des boutons qui insèrent le numéro de page, le nombre de pages, la date, l'heure, le dossier et le nom du fichier, le nom de la feuille et une image ; ses options permettent de définir une première page, des pages paires et impaires ayant leur propre en-tête et pied de page, et de préciser si l'en-tête s'agrandit avec les cellules et s'aligne sur les marges. **En-tête** et **Pied de page** dans cet onglet sont des galeries de modèles prêts à l'emploi, comme **Page 1**, **Page 1 sur ?**, le nom de la feuille, le nom du fichier et la date. Choisir un modèle remplace tout l'en-tête ou le pied de page des pages que vous êtes en train de modifier (le texte que vous étiez encore en train de saisir n'est pas conservé). **(aucun)** le vide. **Atteindre l'en-tête** et **Atteindre le pied de page** ouvrent la partie centrale de cette marge pour y écrire. Appuyez sur **Ctrl+Entrée**, ou cliquez ailleurs, pour terminer ; **Échap** annule la saisie. Chaque en-tête ou pied de page terminé correspond à une étape d'annulation.
- **Zoomer** avec le curseur, ou maintenez **Ctrl** et faites tourner la molette de la souris.

Seules les pages visibles sont dessinées ; une feuille de plusieurs milliers de pages s'ouvre donc immédiatement. L'affichage dans lequel chaque feuille a été laissée est enregistré avec le classeur.

## Titres à imprimer

**Titres à imprimer** répète des lignes en haut, et des colonnes à gauche, sur chaque page qui suit celle où elles se trouvent, afin qu'une ligne d'en-tête figure sur toutes les pages. Saisissez les lignes (par exemple **1:3**) et les colonnes (par exemple **A:B**) dans l'onglet Feuille de la boîte de dialogue Mise en page.

## Mise à l'échelle pour tenir sur les pages

**Largeur** et **Hauteur** réduisent les cellules jusqu'à ce que la feuille ne dépasse pas, en largeur ou en hauteur, le nombre de pages indiqué. **Automatique** laisse cette direction occuper autant de pages que nécessaire. **Échelle** imprime à un pourcentage de la taille réelle, de 10 à 400 ; elle fonctionne lorsque Largeur et Hauteur sont toutes deux sur **Automatique**. Si la réduction pour tenir sur les pages rendrait les caractères très petits, la fenêtre d'impression l'indique.

## Options de feuille

**Afficher le quadrillage** et **Afficher les en-têtes** ne modifient que l'écran. **Imprimer le quadrillage** imprime les lignes entre les cellules sur le papier, et **Imprimer les en-têtes** imprime les numéros de ligne et les lettres de colonne.

**Feuille de droite à gauche** fait lire la feuille à partir de la droite. La colonne A passe au bord droit, et les colonnes se comptent vers la gauche. Les numéros de ligne passent eux aussi au bord droit. Utilisez cette option pour les langues qui s'écrivent de droite à gauche, comme l'arabe et l'hébreu. Elle modifie l'aspect de la feuille à l'écran ; ce choix est enregistré avec la feuille et reste en place à l'ouverture du classeur. Ce n'est pas une étape que vous pouvez annuler : cliquez de nouveau dessus pour la désactiver.

## Arrière-plan

**Arrière-plan** (dans le groupe **Mise en page**) place une image derrière les cellules de la feuille. Choisissez une image PNG, JPEG, GIF ou BMP. L'image est répétée en mosaïque pour remplir la feuille, et elle se déplace avec les cellules lorsque vous faites défiler. Une cellule dotée d'une couleur de remplissage masque l'image : donnez donc un remplissage aux cellules que vous voulez lire facilement. L'image n'apparaît qu'à l'écran : elle n'est pas imprimée et ne figure pas dans un PDF.

**Supprimer l'arrière-plan** retire l'image. Ces deux actions peuvent être annulées. L'image est enregistrée avec la feuille et revient à l'ouverture du classeur. Dans une copie .ods, elle est enregistrée comme image d'arrière-plan de la page, que d'autres logiciels peuvent imprimer. Un fichier image de plus de 8 Mo, ou comportant trop de points, est refusé, de même qu'une image qui porterait la somme des images du classeur (arrière-plans, en-têtes et pieds de page) au-delà de 64 Mo. Une image de plus de 2 400 points de côté est tout de même répétée en mosaïque à sa taille réelle.

## Autres choix

La petite flèche à l'angle du groupe **Mise en page** ouvre la boîte de dialogue Mise en page, qui regroupe toutes ces options et d'autres encore : [La boîte de dialogue Mise en page](page-setup).

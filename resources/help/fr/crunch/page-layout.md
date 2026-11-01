# Mettre en page les feuilles
<!-- keywords: mise en page, marges, orientation, portrait, paysage, papier, format, taille, zone d'impression, saut de page, sauts, imprimer les titres, titres, répéter les lignes, mise à l'échelle, ajuster à la page, quadrillage, en-têtes, options de la feuille -->

L'onglet **Mise en page** détermine comment une feuille est découpée en pages lorsqu'elle est imprimée ou convertie en PDF. Chaque feuille conserve ses propres choix, qui sont enregistrés dans le classeur. Chaque modification correspond à une étape que vous pouvez annuler.

## Marges, orientation et taille

- **Marges** définit l'espace vide autour des cellules. **Normales**, **Larges** et **Étroites** sont des jeux prédéfinis. **Marges personnalisées** ouvre la boîte de dialogue Mise en page, où vous pouvez taper chaque marge et indiquer à quelle distance du bord du papier se trouvent l'en-tête et le pied de page.
- **Orientation** est **Portrait** (le papier est plus haut que large) ou **Paysage** (plus large que haut).
- **Taille** correspond au papier : A4, A3, Lettre US et autres. **Autres tailles de papier** les répertorie tous.

## Zone d'impression

Par défaut, une feuille imprime tout ce qui contient quelque chose : valeurs, formules, cellules avec une bordure ou un remplissage visible, ainsi que les graphiques, images et formes. Pour n'en imprimer qu'une partie :

1. Sélectionnez les cellules voulues.
2. Choisissez **Zone d'impression > Définir la zone d'impression**.

**Ajouter la sélection à la zone d'impression** ajoute une deuxième plage de cellules à la zone d'impression. Chaque plage s'imprime sur ses propres pages. **Annuler la zone d'impression** rétablit l'impression de tout le contenu. Les lignes ou colonnes que vous insérez ou supprimez dans la zone d'impression la modifient en même temps que les cellules.

## Sauts de page

L'endroit où une page se termine et où la suivante commence est calculé à partir du papier, des marges et de la taille de vos lignes et colonnes : une ligne ou une colonne n'est jamais coupée en deux. Pour terminer une page plus tôt, sélectionnez la cellule située en dessous et à droite de l'emplacement du saut, puis choisissez **Sauts de page > Insérer un saut de page**. Si une ligne entière est sélectionnée, le saut se place au-dessus ; si une colonne entière est sélectionnée, il se place à sa gauche. **Supprimer le saut de page** supprime le saut situé au-dessus ou à gauche de la cellule sélectionnée, et **Rétablir tous les sauts de page** supprime tous les sauts que vous avez définis. Les sauts de page que vous définissez sont ignorés lorsqu'une feuille est réduite pour tenir sur la page.

## L'affichage Sauts de page

**Affichage > Sauts de page** (ou le bouton en bas à droite de la fenêtre) affiche la feuille découpée en pages telles qu'elles s'impriment. Les cellules imprimées restent inchangées, tout le reste est grisé, et chaque page porte son numéro sur ses cellules. Un trait en pointillés est un saut calculé par JeffCrunch, et un trait plein est un saut que vous avez défini.

- **Déplacer un saut.** Faites glisser un trait vers une autre ligne ou colonne. Il devient un saut défini par vous à cet endroit, et les pages suivantes sont redécoupées.
- **Supprimer un saut.** Faites glisser un trait hors des cellules imprimées, sur les numéros de ligne ou les lettres de colonne.
- **Modifier la zone d'impression.** Faites glisser le bord bleu des cellules imprimées. Si vous n'avez pas défini de zone d'impression, les cellules imprimées par défaut en deviennent une, avec ce bord déplacé.

Chaque glissement correspond à une étape d'annulation. Lorsqu'une feuille est réduite pour tenir sur la page, ses sauts sont ignorés et les traits ne peuvent donc pas être déplacés. **Normal** rétablit l'affichage habituel des cellules. L'affichage dans lequel chaque feuille a été laissée est enregistré avec le classeur.

## L'affichage Mise en page

**Affichage > Mise en page** (ou le bouton du milieu en bas à droite de la fenêtre) affiche la feuille sous forme des pages imprimées, les unes sous les autres, chacune comme une feuille de papier avec ses marges et un espace entre les pages. Des règles s'affichent en haut et à gauche. L'en-tête et le pied de page apparaissent à l'endroit où ils s'impriment, de même que les images qu'ils contiennent.

- **Travailler dans les cellules.** Cliquez sur une cellule pour la sélectionner, faites glisser pour en sélectionner plusieurs, et utilisez les touches de direction, la saisie, F2 et le double-clic comme en mode Normal. Votre saisie s'affiche dans une bande au-dessus de la cellule, sur sa page.
- **Partir de l'onglet Insertion.** **Insertion > En-tête et pied de page** affiche les pages (si elles ne le sont pas) et ouvre la partie centrale de l'en-tête de la première page, prête pour la saisie. Cliquez sur une autre partie de l'en-tête ou du pied de page pour y taper du texte.
- **Taper un en-tête ou un pied de page.** Placez le pointeur sur la marge du haut ou du bas d'une page : ses parties gauche, centrale et droite s'éclairent. Cliquez sur l'une d'elles et tapez. L'onglet **En-tête et pied de page** apparaît, avec des boutons qui insèrent le numéro de page, le nombre de pages, la date, l'heure, le dossier et le nom du fichier, le nom de la feuille et une image ; ses options permettent de donner à la première page, ainsi qu'aux pages paires et impaires, leurs propres en-têtes et pieds de page, et d'indiquer si l'en-tête suit la mise à l'échelle des cellules et s'aligne sur les marges. Appuyez sur **Ctrl+Entrée**, ou cliquez ailleurs, pour terminer ; **Échap** abandonne la saisie. Chaque en-tête ou pied de page terminé correspond à une étape d'annulation.
- **Zoomer** avec le curseur, ou maintenez la touche **Ctrl** enfoncée en tournant la molette de la souris.

Seules les pages visibles sont dessinées : une feuille de plusieurs milliers de pages s'ouvre donc immédiatement. L'affichage dans lequel chaque feuille a été laissée est enregistré avec le classeur.

## Imprimer les titres

**Imprimer les titres** répète des lignes en haut et des colonnes à gauche de chaque page suivant celle où elles se trouvent, afin qu'une ligne d'en-tête figure sur chaque page. Tapez les lignes (par exemple **1:3**) et les colonnes (par exemple **A:B**) dans l'onglet Feuille de la boîte de dialogue Mise en page.

## Mise à l'échelle

**Largeur** et **Hauteur** réduisent les cellules jusqu'à ce que la feuille ne dépasse pas ce nombre de pages en largeur ou en hauteur. **Automatique** permet à cette dimension d'occuper autant de pages que nécessaire. **Échelle** imprime à un pourcentage de la taille réelle, de 10 à 400, et ne fonctionne que si Largeur et Hauteur sont toutes deux sur **Automatique**. Si la réduction rend les caractères très petits, l'écran Imprimer vous en avertit.

## Options de la feuille

**Afficher le quadrillage** et **Afficher les en-têtes** ne modifient que l'écran. **Imprimer le quadrillage** imprime le quadrillage entre les cellules, et **Imprimer les en-têtes** imprime les numéros de ligne et les lettres de colonne.

## Autres options

La petite flèche dans le coin du groupe **Mise en page** ouvre la boîte de dialogue Mise en page, qui regroupe toutes ces options et d'autres encore : [La boîte de dialogue Mise en page](page-setup).

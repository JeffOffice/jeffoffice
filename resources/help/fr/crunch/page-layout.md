# Mettre en page les feuilles
<!-- keywords: mise en page, marges, orientation, portrait, paysage, papier, format, taille, zone d'impression, saut de page, sauts, imprimer les titres, titres, répéter les lignes, mise à l'échelle, ajuster à la page, quadrillage, en-têtes, options de la feuille -->

L'onglet **Mise en page** détermine comment une feuille est découpée en pages lorsqu'elle est imprimée ou convertie en PDF. Chaque feuille conserve ses propres choix, qui sont enregistrés dans le classeur. Chaque modification est une étape que vous pouvez annuler.

## Marges, orientation et taille

- **Marges** définit l'espace vide autour des cellules. **Normales**, **Larges** et **Étroites** sont des jeux prédéfinis. **Marges personnalisées** ouvre la boîte de dialogue Mise en page, dans laquelle vous pouvez saisir chaque marge et indiquer la distance entre l'en-tête ou le pied de page et le bord du papier.
- **Orientation** est **Portrait** (le papier est plus haut que large) ou **Paysage** (plus large que haut).
- **Taille** correspond au papier : Lettre US, Légal US, A4 et d'autres. **Autres tailles de papier** les répertorie tous.

## Zone d'impression

Par défaut, une feuille imprime tout ce qui contient quelque chose : valeurs, formules, cellules avec une bordure ou un remplissage visible, ainsi que les graphiques, images et formes. Pour n'en imprimer qu'une partie :

1. Sélectionnez les cellules voulues.
2. Choisissez **Zone d'impression > Définir la zone d'impression**.

**Ajouter la sélection à la zone d'impression** ajoute un deuxième bloc de cellules à la zone d'impression. Chaque bloc s'imprime sur ses propres pages. **Annuler la zone d'impression** rétablit l'impression de tout le contenu. Les lignes ou colonnes que vous insérez ou supprimez dans la zone d'impression la modifient en même temps que les cellules.

## Sauts de page

L'endroit où une page se termine et où la suivante commence est calculé à partir du papier, des marges et de la taille des lignes et des colonnes : une ligne ou une colonne n'est jamais coupée en deux. Pour terminer une page plus tôt, sélectionnez la cellule située en dessous et à droite de l'emplacement du saut, puis choisissez **Sauts > Insérer un saut de page**. Si une ligne entière est sélectionnée, le saut est placé au-dessus ; si une colonne entière est sélectionnée, il est placé à sa gauche. **Supprimer le saut de page** supprime le saut situé au-dessus ou à gauche de la cellule sélectionnée, et **Rétablir tous les sauts de page** supprime tous les sauts que vous avez définis. Les sauts de page que vous définissez sont ignorés lorsqu'une feuille est réduite pour tenir sur un nombre de pages donné.

## Le mode Sauts de page

**Affichage > Sauts de page** (ou le bouton en bas à droite de la fenêtre) affiche la feuille découpée en pages telles qu'elles s'impriment. Les cellules imprimées restent inchangées, tout le reste est grisé, et le numéro de chaque page s'affiche sur ses cellules. Une ligne en pointillés est un saut calculé par JeffCrunch, une ligne continue est un saut que vous avez défini.

- **Déplacer un saut.** Faites glisser une ligne vers une autre ligne ou colonne. Elle devient un saut défini par vous à cet endroit, et les pages suivantes sont redécoupées.
- **Supprimer un saut.** Faites glisser une ligne hors des cellules imprimées, sur les numéros de ligne ou les lettres de colonne.
- **Modifier la zone d'impression.** Faites glisser le bord bleu des cellules imprimées. Si vous n'avez pas défini de zone d'impression, les cellules imprimées par défaut en deviennent une, avec ce bord déplacé.

Chaque glissement est une étape d'annulation. Lorsqu'une feuille est réduite pour tenir sur un nombre de pages donné, ses sauts sont ignorés et les lignes ne peuvent donc pas être déplacées. **Normal** rétablit l'affichage habituel des cellules. Le mode d'affichage de chaque feuille est enregistré avec le classeur.

## Le mode Mise en page

**Affichage > Mise en page** (ou le bouton du milieu en bas à droite de la fenêtre) affiche la feuille sous forme de pages telles qu'elles s'impriment, les unes sous les autres, chacune comme une feuille de papier avec ses marges et un espace entre les pages. Des règles s'affichent en haut et à gauche. L'en-tête et le pied de page apparaissent à l'endroit où ils s'impriment, de même que les images qu'ils contiennent.

- **Travailler dans les cellules.** Cliquez sur une cellule pour la sélectionner, faites glisser pour en sélectionner plusieurs, et utilisez les touches de direction, la saisie, F2 et le double-clic comme en mode Normal. Ce que vous tapez s'affiche dans une bande au-dessus de la cellule, sur sa page.
- **Saisir un en-tête ou un pied de page.** Placez le pointeur sur la marge supérieure ou inférieure d'une page : ses parties gauche, centrale et droite s'éclairent. Cliquez sur l'une d'elles et tapez votre texte. L'onglet **En-tête et pied de page** apparaît, avec des boutons pour insérer le numéro de page, le nombre de pages, la date, l'heure, le dossier et le nom du fichier, le nom de la feuille et une image ; ses options permettent de donner à la première page, ainsi qu'aux pages paires et impaires, leurs propres en-têtes et pieds de page, et d'indiquer si l'en-tête suit la mise à l'échelle des cellules et s'aligne sur les marges. Appuyez sur **Ctrl+Entrée**, ou cliquez ailleurs, pour terminer ; **Échap** abandonne la saisie. Chaque en-tête ou pied de page terminé est une étape d'annulation.
- **Zoomer** avec le curseur, ou en maintenant **Ctrl** enfoncée tout en tournant la molette de la souris.

Seules les pages visibles sont dessinées : une feuille de plusieurs milliers de pages s'ouvre donc immédiatement. Le mode d'affichage de chaque feuille est enregistré avec le classeur.

## Imprimer les titres

**Imprimer les titres** répète des lignes en haut et des colonnes à gauche de chaque page suivant celle où elles se trouvent, de sorte qu'une ligne d'en-tête figure sur chaque page. Tapez les lignes (par exemple **1:3**) et les colonnes (par exemple **A:B**) dans l'onglet Feuille de la boîte de dialogue Mise en page.

## Mise à l'échelle

**Largeur** et **Hauteur** réduisent les cellules jusqu'à ce que la feuille ne dépasse pas ce nombre de pages en largeur ou en hauteur. **Automatique** laisse ce sens occuper autant de pages que nécessaire. **Échelle** imprime à un pourcentage de la taille réelle, de 10 à 400, et ne s'applique que lorsque Largeur et Hauteur sont toutes deux sur **Automatique**. Si la réduction pour ajuster la feuille rend le texte très petit, l'écran Imprimer vous le signale.

## Options de la feuille

**Afficher le quadrillage** et **Afficher les en-têtes** ne modifient que l'affichage à l'écran. **Imprimer le quadrillage** imprime le quadrillage entre les cellules, et **Imprimer les en-têtes** imprime les numéros de ligne et les lettres de colonne.

## Autres options

La petite flèche dans l'angle du groupe **Mise en page** ouvre la boîte de dialogue Mise en page, qui regroupe toutes ces options et bien d'autres : [La boîte de dialogue Mise en page](page-setup).

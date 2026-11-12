# Équations

<!-- keywords: équation, équations, math, mathématiques, formule, fraction, racine carrée, exposant, indice, puissance, somme, intégrale, grec, symbole, insérer une équation, accent, matrice, parenthèses, équations intégrées, galerie, modifier une équation, modifier l'équation, ods, image, pdf -->

Une **équation** est une formule mathématique, comme une fraction ou une racine carrée. Dans JeffCrunch, une équation est un objet placé au-dessus des cellules, comme une image ou une zone de texte. Ce n'est pas une cellule : elle ne modifie aucun nombre de la feuille.

## Insérer une équation dans la feuille

1. Cliquez sur **Insertion > Symboles > Équation**.
2. Saisissez l'équation sur une seule ligne. Sous la zone, vous voyez l'aperçu du résultat. Cliquez sur **OK**.

L'équation est placée dans la cellule active, et elle est sélectionnée. Pour partir d'un modèle courant, cliquez sur **Équations intégrées** et choisissez-en une, par exemple la formule du second degré ou l'aire d'un cercle.

## Saisir une équation

- **Fractions :** saisissez une barre oblique. `a/b` donne a sur b. `(x+1)/(x-1)` traite chaque groupe entre parenthèses comme un seul élément.
- **Puissances et indices :** `x^2` donne x au carré, et `x_i` donne x avec un petit i en indice.
- **Racines :** saisissez `sqrt(x+1)` pour une racine carrée.
- **Sommes, intégrales et produits :** saisissez `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`.
- **Symboles :** saisissez une barre oblique inverse suivie d'un nom. `\alpha` donne α, `\pi` donne π, `\infty` donne ∞ et `\leq` donne ≤.
- **Accents, parenthèses et matrices :** `\hat{x}`, `\left( ... \right)` et `\pmatrix{a & b \\ c & d}`.

## Modifier, déplacer et redimensionner

Double-cliquez sur l'équation, ou sélectionnez-la puis cliquez sur **Insertion > Symboles > Équation**, pour ouvrir la zone contenant sa formule. Modifiez-la, puis cliquez sur **OK**. Une seule étape suffit pour l'annuler. Faites glisser l'équation pour la déplacer, et faites glisser une poignée pour la redimensionner ; l'équation conserve ses proportions à l'intérieur du cadre.

## Enregistrement

- **.xlsx** conserve l'équation sous forme d'équation, dans le format standard des mathématiques, accompagnée d'une image pour les programmes qui ne peuvent pas lire les formules. À la réouverture du fichier, l'équation s'affiche de nouveau. Une équation créée dans un autre programme s'affiche de la même façon ; si ses formules ne peuvent pas être lues, vous voyez son image.
- **Le PDF et l'impression** dessinent l'équation sous forme de formes, si bien qu'elle reste nette à toutes les tailles.
- **Les copies en .ods** ne contiennent pas d'équations. JeffCrunch enregistre chacune d'elles sous forme d'image, à le même emplacement, et indique le nom des équations concernées lorsqu'il vous signale les éléments que la copie ne conserve pas.

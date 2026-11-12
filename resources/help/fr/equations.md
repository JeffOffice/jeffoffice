# Ajouter une équation
<!-- keywords: équation, équations, mathématiques, maths, formule, fraction, racine carrée, exposant, indice, puissance, somme, intégrale, grec, grecque, symbole, insérer une équation, alt+=, accent, matrice, parenthèses, équations intégrées, galerie, modifier une équation, éditer une équation, vecteur, pdf, jpub, fichier pub -->

Une **équation** est une expression mathématique, comme une fraction ou une racine carrée. Dans JeffPub, une équation est un objet de la page, comme une image : vous pouvez la déplacer, la redimensionner, la faire pivoter et lui donner une couleur.

## Placer une équation sur la page

1. Cliquez sur **Insertion > Texte > Équation**, ou appuyez sur **Alt+=**.
2. Saisissez l'équation sur une seule ligne. Sous la zone, vous voyez le rendu final. Cliquez sur **OK**.

L'équation est placée au centre de la page, sélectionnée. Pour partir d'une équation courante, cliquez sur la flèche située à côté de **Équation** et choisissez une équation intégrée, comme la formule quadratique ou l'aire d'un cercle.

## Saisir une équation

- **Fractions :** tapez une barre oblique. `a/b` donne a sur b. `(x+1)/(x-1)` traite chaque groupe entre parenthèses comme un seul élément.
- **Puissances et indices :** `x^2` donne x au carré, et `x_i` donne x avec un petit i en indice. Vous pouvez combiner les deux, comme dans `x_i^2`.
- **Racines :** tapez `sqrt(x+1)` pour une racine carrée, ou `sqrt[3]{x}` pour une racine cubique.
- **Sommes, intégrales et produits :** tapez `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`. La partie placée après le tiret bas s'écrit en dessous, et celle placée après l'accent circonflexe au-dessus.
- **Symboles :** tapez une barre oblique inversée suivie d'un nom. `\alpha` donne α, `\pi` donne π, `\infty` donne ∞, `\leq` donne ≤ et `\times` donne ×. Environ 70 noms sont disponibles.
- **Accents :** `\hat{x}`, `\vec{v}` et `\bar{x}` placent un signe au-dessus de la lettre.
- **Parenthèses extensibles :** tapez `\left( ... \right)`, et les parenthèses s'agrandissent à la hauteur de ce qu'elles entourent.
- **Matrices :** une **matrice** est un tableau de nombres. Tapez `\pmatrix{a & b \\ c & d}`. Placez une `&` entre les cellules d'une ligne, et deux barres obliques inversées entre les lignes.

## Modifier une équation

Sélectionnez l'équation et appuyez sur Entrée, ou double-cliquez dessus, ou cliquez sur **Insertion > Texte > Équation**. La zone s'ouvre avec l'équation écrite sur une seule ligne. Modifiez-la, puis cliquez sur **OK**. Une seule action suffit pour l'annuler.

Une équation provenant d'un fichier qui contient des éléments que la zone ne peut pas écrire est affichée telle quelle. Pour la modifier, saisissez une nouvelle équation.

## Déplacer, redimensionner et colorer

Faites glisser l'équation pour la déplacer. Faites glisser un coin pour l'agrandir ou la réduire ; l'équation conserve ses proportions. Utilisez **Remplissage de la forme** pour changer la couleur des caractères.

## Enregistrement

- **.jpub** conserve l'équation, afin que vous puissiez la modifier à nouveau.
- **PDF et impression** dessinent l'équation sous forme de formes, si bien qu'elle reste nette à toutes les tailles.
- **.pub** ne prend pas en charge les équations. JeffPub enregistre chacune d'elles sous forme d'image, à la même place et à la même taille, et vous en avertit lors de l'enregistrement.
- **EPUB** et **HTML** conservent l'équation sous la forme de la ligne que vous avez saisie.

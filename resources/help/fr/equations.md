# Ajouter une équation
<!-- keywords: onglet équation, structures, symboles, convertir, équation, équations, math, mathématiques, formule, fraction, racine carrée, exposant, indice, somme, intégrale, grec, symbole, insérer une équation, alt+=, accent, matrice, parenthèses, équations intégrées, galerie, modifier une équation, changer une équation, modifier l'équation, vecteur, pdf, jpub, fichier pub -->

Une **équation** est une expression mathématique, comme une fraction ou une racine carrée. Dans JeffPub, une équation est un objet placé sur la page, comme une image : vous pouvez la déplacer, la redimensionner, la faire pivoter et lui donner une couleur.

## Placer une équation sur la page

1. Cliquez sur **Insertion > Texte > Équation**, ou appuyez sur **Alt+=**.
2. Saisissez l'équation sur une seule ligne. Sous la zone de saisie, vous voyez son aspect final. Cliquez sur **OK**.

L'équation se place au milieu de la page, sélectionnée. Pour partir d'une équation connue, cliquez sur la flèche à côté de **Équation** et choisissez une équation intégrée, comme la formule du second degré ou l'aire d'un cercle.

## Comment la saisir

- **Fractions :** tapez une barre oblique. `a/b` donne a sur b. `(x+1)/(x-1)` traite chaque groupe entre parenthèses comme un seul élément.
- **Puissances et indices :** `x^2` donne x au carré, et `x_i` donne x avec un petit i en indice. Vous pouvez utiliser les deux, comme dans `x_i^2`.
- **Racines :** tapez `sqrt(x+1)` pour une racine carrée, ou `sqrt[3]{x}` pour une racine cubique.
- **Sommes, intégrales et produits :** tapez `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`. La partie après le tiret bas se place en dessous, et la partie après l'accent circonflexe au-dessus.
- **Symboles :** tapez une barre oblique inversée suivie d'un nom. `\alpha` donne α, `\pi` donne π, `\infty` donne ∞, `\leq` donne ≤ et `\times` donne ×. Environ 70 noms sont reconnus.
- **Accents :** `\hat{x}`, `\vec{v}` et `\bar{x}` placent un signe au-dessus de la lettre.
- **Parenthèses extensibles :** tapez `\left( ... \right)` : les parenthèses s'agrandissent à la hauteur de ce qu'elles contiennent.
- **Matrices :** une **matrice** est un tableau de nombres. Tapez `\pmatrix{a & b \\ c & d}`. Placez une `&` entre les cellules d'une ligne, et deux barres obliques inversées entre les lignes.

## Saisir en UnicodeMath ou en LaTeX

À côté de la zone de saisie se trouve la liste des formats de saisie : **Ligne simple** (celui utilisé ci-dessus), **UnicodeMath** et **LaTeX**. Il s'agit de deux manières d'écrire des mathématiques en texte brut, que beaucoup de personnes connaissent déjà. Lorsque vous choisissez un autre format, ce que vous avez saisi est réécrit dans ce format.

- **UnicodeMath :** `a/b` est une fraction, `x^2_i` combine une puissance et un indice, `√(x)` est une racine, et `∑_(i=1)^n i` est une somme. Placez des mots entre guillemets, comme `"si x"`, et ils restent du texte ordinaire au lieu d'être composés en caractères mathématiques. `(a|b)` crée des parenthèses à deux éléments, et `\matrix(a&b@c&d)` crée une matrice (`&` entre les cellules, `@` entre les lignes).
- **LaTeX :** `\frac{a}{b}` est une fraction, `\sqrt[3]{x}` est une racine cubique, `\sum_{i=1}^n i` est une somme, `\text{si x}` est du texte ordinaire, et `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` est une matrice.

Les deux boutons **Professionnel** et **Linéaire** modifient l'affichage de l'équation dans la zone de saisie. Professionnel la dessine en forme composée, telle qu'elle apparaîtra. Linéaire n'affiche que la ligne que vous avez saisie. Si une équation ne peut pas être écrite dans le format choisi, la zone de saisie vous le signale et conserve ce que vous avez tapé.

## Modifier une équation

Sélectionnez l'équation et appuyez sur Entrée, ou double-cliquez dessus, ou cliquez sur **Insertion > Texte > Équation**. La zone de saisie s'ouvre avec l'équation écrite sur une seule ligne. Modifiez-la, puis cliquez sur **OK**. Une seule étape suffit pour annuler la modification.

Une équation provenant d'un fichier s'ouvre en UnicodeMath, ou en LaTeX lorsque UnicodeMath ne peut pas l'écrire. Si aucun des deux ne convient, la zone de saisie affiche le texte de l'équation. Cliquez sur **OK** sans rien modifier, ou sur **Annuler**, pour laisser l'équation telle quelle, ou saisissez-en une nouvelle pour la remplacer.

## L'onglet Équation

Sélectionnez une équation : un onglet nommé **Équation** apparaît à la fin du ruban. Il disparaît lorsque vous sélectionnez autre chose.

- **Outils :** **Modifier l'équation** ouvre la zone de saisie. **Professionnel** et **Linéaire** modifient l'affichage de l'équation sélectionnée sur la page : Professionnel la dessine en forme composée, et Linéaire affiche la ligne dans laquelle elle a été saisie. Ce choix est conservé avec l'équation dans le fichier `.jpub`, il est donc le même lorsque vous rouvrez le fichier. Un fichier `.pub` reçoit l'image de l'équation telle qu'elle apparaît sur la page. **Texte normal** ouvre la zone de saisie avec une paire de guillemets ajoutée, pour les mots qui ne sont pas des mathématiques.
- **Convertir :** choisissez le format dans lequel s'ouvre la zone de saisie : **Ligne simple**, **UnicodeMath** ou **LaTeX**. La prochaine fois que vous modifierez une équation, elle sera écrite dans le format choisi. Une équation qu'aucune écriture en ligne simple ne peut représenter, comme une somme avec ses bornes au-dessus et en dessous, s'ouvre en UnicodeMath à la place.
- **Symboles :** lettres grecques, opérateurs, relations, flèches, ensembles et logique. Choisissez un symbole : la zone de saisie s'ouvre avec ce symbole ajouté à la ligne.
- **Structures :** fraction, indice et exposant, radical, intégrale, grand opérateur, parenthèses, fonction, accent, limite, opérateur et matrice. Cliquez sur un bouton pour obtenir sa forme habituelle, ou sur la flèche à côté pour afficher d'autres formes, comme une dérivée, une racine avec indice ou une matrice identité.

Une structure s'insère en fin de ligne, avec des lettres d'exemple, et la première est sélectionnée pour que vous puissiez taper par-dessus. La zone de saisie propose les mêmes structures sous forme d'une rangée de petits boutons, au-dessus de la ligne. Là, une structure s'insère au niveau du curseur, et si une partie de la ligne est sélectionnée, cette partie prend la première place de la structure.

## Déplacer, redimensionner et colorer

Faites glisser l'équation pour la déplacer. Faites glisser un coin pour l'agrandir ou la réduire ; l'équation conserve ses proportions. Utilisez **Remplissage de la forme** pour changer la couleur des caractères.

## Enregistrement

- **.jpub** conserve l'équation, ce qui permet de la modifier à nouveau.
- **PDF et impression** dessinent l'équation sous forme de formes, elle reste donc nette à toutes les tailles.
- **.pub** ne prend pas en charge les équations. JeffPub enregistre chacune d'elles sous forme d'image, à la même place et à la même taille, et vous le signale lors de l'enregistrement.
- **EPUB** et **HTML** conservent l'équation sous la forme de la ligne que vous avez saisie.

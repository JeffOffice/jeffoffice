# Équation

<!-- keywords: onglet équation, structures, symboles, convertir, équation, équations, mathématiques, math, formule, fraction, racine carrée, exposant, indice, somme, intégrale, grec, symbole, insérer une équation, accent, matrice, crochets, équations intégrées, galerie, modifier une équation, éditer une équation, ods, image, pdf -->

Une **équation** est une ligne de mathématiques, comme une fraction ou une racine carrée. Dans JeffCrunch, une équation est un objet placé au-dessus des cellules, comme une image ou une zone de texte. Ce n'est pas une cellule : elle ne modifie aucun nombre de la feuille.

## Insérer une équation dans la feuille

1. Cliquez sur **Insertion > Symboles > Équation**.
2. Saisissez l'équation sur une seule ligne. Sous la zone, vous voyez le résultat. Cliquez sur **OK**.

L'équation est placée dans la cellule active, sélectionnée. Pour partir d'une formule connue, cliquez sur **Équations intégrées** et choisissez-en une, comme la formule du second degré ou l'aire d'un cercle.

## Saisir une équation

- **Fractions :** saisissez une barre oblique. `a/b` donne a sur b. `(x+1)/(x-1)` traite chaque groupe entre parenthèses comme un seul élément.
- **Puissances et indices :** `x^2` donne x au carré, et `x_i` donne x avec un petit i en indice.
- **Racines :** saisissez `sqrt(x+1)` pour une racine carrée.
- **Sommes, intégrales et produits :** saisissez `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`.
- **Symboles :** saisissez une barre oblique inverse suivie d'un nom. `\alpha` donne α, `\pi` donne π, `\infty` donne ∞, et `\leq` donne ≤.
- **Accents, parenthèses et matrices :** `\hat{x}`, `\left( ... \right)` et `\pmatrix{a & b \\ c & d}`.

## Saisir l'équation en UnicodeMath ou en LaTeX

À côté de la zone se trouve la liste des formats de saisie : **Ligne simple** (celui présenté ci-dessus), **UnicodeMath** et **LaTeX**. Ce sont deux façons d'écrire les mathématiques en texte brut, que beaucoup d'utilisateurs connaissent déjà. Lorsque vous choisissez un autre format, ce que vous avez saisi est réécrit dans ce format.

- **UnicodeMath :** `a/b` est une fraction, `x^2_i` contient une puissance et un indice, `√(x)` est une racine, et `∑_(i=1)^n i` est une somme. Placez les mots entre guillemets, comme `"si x"`, pour qu'ils restent du texte et non des lettres mathématiques. `(a|b)` crée des parenthèses à deux parties, et `\matrix(a&b@c&d)` crée une matrice (`&` entre les cellules, `@` entre les lignes).
- **LaTeX :** `\frac{a}{b}` est une fraction, `\sqrt[3]{x}` est une racine cubique, `\sum_{i=1}^n i` est une somme, `\text{si x}` est du texte brut, et `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` est une matrice.

Les deux boutons **Professionnel** et **Linéaire** modifient l'affichage de votre équation dans la zone. Professionnel la dessine en forme développée, telle qu'elle apparaîtra. Linéaire n'affiche que la ligne que vous avez saisie. Si une équation ne peut pas être écrite dans le format choisi, la zone vous le signale et conserve ce que vous avez saisi.

## L'onglet Équation

Lorsque vous sélectionnez une équation, un onglet nommé **Équation** apparaît à la fin du ruban. Il disparaît lorsque vous sélectionnez autre chose.

- **Outils :** **Modifier l'équation** ouvre la zone. **Professionnel** et **Linéaire** choisissent comment la zone s'ouvre, en forme développée ou seulement sur la ligne. **Texte normal** ouvre la zone avec une paire de guillemets ajoutée, pour les mots qui ne sont pas des mathématiques.
- **Convertir :** choisissez le format dans lequel la zone s'ouvre : **Ligne simple**, **UnicodeMath** ou **LaTeX**. La prochaine fois que vous modifierez une équation, elle sera écrite dans le format choisi. Une équation qu'aucune écriture de type simple ne peut représenter, comme une somme avec ses bornes en haut et en bas, s'ouvre en UnicodeMath à la place.
- **Symboles :** lettres grecques, opérateurs, relations, flèches, ensembles et logique. Choisissez un symbole, et la zone s'ouvre avec ce symbole ajouté à la ligne.
- **Structures :** fraction, indice, radical, intégrale, grand opérateur, parenthèses, fonction, accent, limite, opérateur et matrice. Cliquez sur un bouton pour sa forme habituelle, ou sur la flèche à côté pour d'autres formes, comme une dérivée, une racine avec un indice ou une matrice identité.

Une structure est insérée à la fin de la ligne, avec des lettres d'exemple, et la première est sélectionnée pour que vous puissiez la remplacer en tapant. La zone propose les mêmes structures sous forme d'une rangée de petits boutons au-dessus de la ligne. Là, une structure est insérée au niveau du curseur, et si une partie de la ligne est sélectionnée, cette partie passe dans la première place de la structure.

## Modifier, déplacer et redimensionner

Double-cliquez sur l'équation, ou sélectionnez-la puis cliquez sur **Insertion > Symboles > Équation**, pour ouvrir la zone avec sa ligne. Modifiez-la puis cliquez sur **OK**. L'annulation ne demande qu'une étape. Faites glisser l'équation pour la déplacer, et faites glisser une poignée pour la redimensionner ; l'équation conserve ses proportions à l'intérieur de la zone.

## Enregistrement

- **.xlsx** conserve l'équation en tant qu'équation, dans le format standard des mathématiques, avec une image à côté pour les programmes qui ne peuvent pas lire les mathématiques. Rouvrir le fichier affiche à nouveau l'équation. Une équation créée dans un autre programme s'affiche de la même façon ; si ses mathématiques ne peuvent pas être lues, vous voyez son image.
- **PDF et impression** dessinent l'équation sous forme de formes, elle reste donc nette à toutes les tailles.
- **Les copies .ods** ne contiennent pas d'équations. JeffCrunch enregistre chacune d'elles sous forme d'image, au même emplacement, et nomme les équations lorsqu'il vous indique ce que la copie ne conserve pas.

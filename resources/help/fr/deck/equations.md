# Équations
<!-- keywords: onglet Équation, structures, symboles, convertir, équation, équations, math, mathématiques, formule, fraction, racine carrée, exposant, indice, somme, intégrale, grec, symbole, insérer une équation, accent, matrice, parenthèses, équations intégrées, galerie, modifier une équation, éditer une équation, pptx, odp, image, pdf, diaporama, impression -->

Une **équation** est une ligne de mathématiques, comme une fraction ou une racine carrée. Sur une diapositive, une équation est un objet, comme une image ou une zone de texte. Vous pouvez la déplacer, la redimensionner, la faire pivoter, la copier et la supprimer.

## Insérer une équation sur la diapositive

1. Cliquez sur **Insertion > Symboles > Équation** (ou appuyez sur **Alt+=**).
2. Saisissez l'équation sur une seule ligne. Sous la zone, vous voyez le rendu de l'équation. Cliquez sur **OK**.

L'équation s'insère au milieu de la diapositive, sélectionnée. Ses caractères ont à peu près la hauteur d'un titre. Une équation très large est réduite pour tenir sur la diapositive. Pour partir d'un modèle connu, cliquez sur **Équations intégrées** et choisissez-en une, par exemple la formule du second degré ou l'aire d'un cercle.

## Comment la saisir

- **Fractions :** saisissez une barre oblique. `a/b` donne a sur b. `(x+1)/(x-1)` traite chaque groupe entre parenthèses comme un seul élément.
- **Puissances et indices :** `x^2` donne x au carré, et `x_i` donne x avec un petit i en indice.
- **Racines :** saisissez `sqrt(x+1)` pour une racine carrée.
- **Sommes, intégrales et produits :** saisissez `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`.
- **Symboles :** saisissez une barre oblique inverse suivie d'un nom. `\alpha` donne α, `\pi` donne π, `\infty` donne ∞, et `\leq` donne ≤.
- **Accents, parenthèses et matrices :** `\hat{x}`, `\left( ... \right)` et `\pmatrix{a & b \\ c & d}`.

## Saisir en UnicodeMath ou en LaTeX

À côté de la zone se trouve la liste des formats de saisie : **Ligne simple** (celui utilisé ci-dessus), **UnicodeMath** et **LaTeX**. Ce sont deux façons d'écrire les mathématiques en texte brut, que beaucoup de personnes connaissent déjà. Quand vous choisissez un autre format, ce que vous avez saisi est réécrit dans ce format.

- **UnicodeMath :** `a/b` est une fraction, `x^2_i` contient une puissance et un indice, `√(x)` est une racine, et `∑_(i=1)^n i` est une somme. Placez les mots entre guillemets, par exemple `"si x"`, et ils restent du texte ordinaire au lieu d'être composés en caractères mathématiques. `(a|b)` crée des parenthèses à deux éléments, et `\matrix(a&b@c&d)` crée une matrice (`&` entre les cellules, `@` entre les lignes).
- **LaTeX :** `\frac{a}{b}` est une fraction, `\sqrt[3]{x}` est une racine cubique, `\sum_{i=1}^n i` est une somme, `\text{si x}` est du texte ordinaire, et `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` est une matrice.

Les deux boutons **Professionnel** et **Linéaire** modifient l'affichage de votre équation dans la zone. Professionnel la dessine en forme développée, telle qu'elle apparaîtra. Linéaire n'affiche que la ligne que vous avez saisie. Si une équation ne peut pas être écrite dans le format choisi, la zone vous le signale et conserve ce que vous avez saisi.

## Modifier, déplacer et redimensionner

Double-cliquez sur l'équation, ou sélectionnez-la et cliquez sur **Insertion > Symboles > Équation**, pour ouvrir la zone avec sa ligne. Modifiez-la, puis cliquez sur **OK**. Elle reste à sa place et conserve sa taille, et la modification peut être annulée en une seule étape. Faites glisser l'équation pour la déplacer. Faites glisser une poignée pour la redimensionner ; l'équation conserve ses proportions dans son cadre.

Une équation provenant d'un fichier peut elle aussi être modifiée. Tant que vous ne la modifiez pas, JeffDeck affiche l'image conservée dans le fichier et la réenregistre à l'identique. Double-cliquez dessus (ou sélectionnez-la et cliquez sur **Équation**). La zone commence par une ligne qui décrit l'équation. Si vous modifiez cette ligne, l'image devient une équation, au même endroit, et JeffDeck recompose les mathématiques à partir de ce que vous avez saisi ; les polices particulières de l'ancienne formule sont alors perdues. Si vous cliquez sur **OK** sans modifier la ligne, rien ne change.

## L'onglet Équation

Sélectionnez une équation, et un onglet nommé **Équation** apparaît à la fin du ruban. Il disparaît lorsque vous sélectionnez autre chose.

- **Outils :** **Modifier l'équation** ouvre la zone. **Professionnel** et **Linéaire** modifient l'affichage de l'équation sélectionnée sur la diapositive : Professionnel la dessine en forme développée, et Linéaire affiche la ligne dans laquelle elle est saisie. Ce choix ne reste attaché à l'équation que pendant que la présentation est ouverte : un fichier `.pptx` ou `.odp` enregistré contient toujours l'équation en forme développée, et il s'ouvre donc ainsi. **Texte normal** ouvre la zone avec une paire de guillemets ajoutée, pour les mots qui ne sont pas des mathématiques.
- **Convertir :** choisissez le format dans lequel la zone s'ouvre : **Ligne simple**, **UnicodeMath** ou **LaTeX**. La prochaine fois que vous modifierez une équation, elle sera écrite dans le format choisi. Une équation qu'aucune saisie en ligne simple ne peut écrire, comme une somme dont les bornes sont placées au-dessus et en dessous, s'ouvre en UnicodeMath à la place.
- **Symboles :** lettres grecques, opérateurs, relations, flèches, ensembles et logique. Choisissez un symbole, et la zone s'ouvre avec ce symbole ajouté à la ligne.
- **Structures :** fraction, indice et exposant, radical, intégrale, grand opérateur, parenthèses, fonction, accent, limite, opérateur et matrice. Cliquez sur un bouton pour obtenir sa forme habituelle, ou sur la flèche à côté pour afficher d'autres formes, comme une dérivée, une racine avec un indice ou une matrice identité.

Une structure s'insère à la fin de la ligne, avec des lettres d'exemple, et la première est sélectionnée pour que vous puissiez taper par-dessus. La zone propose les mêmes structures sous forme d'une rangée de petits boutons au-dessus de la ligne. Là, une structure s'insère au niveau du curseur, et si une partie de la ligne est sélectionnée, cette partie passe à la première place de la structure.

## Diaporama, impression et PDF

Le diaporama, l'impression et les fichiers PDF dessinent l'équation sous forme de formes, si bien qu'elle reste nette à toute taille.

## Enregistrement

- **.pptx** conserve l'équation en tant qu'équation, dans la forme standard des mathématiques, accompagnée d'une image de celle-ci pour les programmes qui ne peuvent pas lire les mathématiques. Lorsque vous rouvrez le fichier, JeffDeck affiche l'image et conserve les mathématiques, et vous pouvez toujours la modifier comme décrit ci-dessus. Une présentation contient au maximum 2 000 équations et 16 Mo de mathématiques au total. Au-delà, JeffDeck n'en ajoute plus.
- **.odp** ne contient pas d'équations dans la copie de JeffDeck. Chacune est enregistrée sous forme d'image, à la même place, et le message affiché après l'enregistrement l'indique.
- Une zone de texte contenant du texte autour de ses mathématiques n'est pas une équation ici. Elle est affichée sous forme de l'image conservée dans le fichier et réenregistrée dans son ensemble.

## Voir aussi

- [Formes, images, tableaux et texte artistique](objects)
- [Onglet Insertion](tab-insert)

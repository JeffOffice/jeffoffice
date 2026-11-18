# Équations
<!-- keywords: onglet équation, structures, symboles, convertir, équation, équations, math, mathématiques, formule, fraction, racine carrée, exposant, indice, somme, intégrale, grec, symbole, insérer une équation, alt+=, accent, chapeau, vecteur, barre, matrice, parenthèses, équations intégrées, galerie, modifier une équation, éditer une équation -->

Une **équation** est une ligne de mathématiques, par exemple une fraction ou une racine carrée. Cette rubrique explique comment la saisir et l'insérer dans un document.

## Insérer une équation

1. Placez le point d'insertion à l'endroit où l'équation doit apparaître.
2. Cliquez sur **Insertion > Équation**, ou appuyez sur **Alt+=**.
3. Saisissez l'équation sur une seule ligne. Sous la zone, vous voyez l'équation telle qu'elle apparaîtra, ainsi que la ligne de texte que la page affichera. Pour partir d'une équation connue, choisissez-en une dans la liste **Équations intégrées** de la zone. Cliquez sur **OK**.

L'équation est enregistrée dans le document selon la forme standard des mathématiques, afin que d'autres logiciels puissent l'ouvrir.

## Comment la saisir

- **Fractions :** saisissez une barre oblique. `a/b` correspond à a sur b. `(x+1)/(x-1)` traite chaque groupe entre parenthèses comme un seul élément. `\frac{1}{2}` fonctionne également.
- **Puissances :** saisissez un accent circonflexe. `x^2` correspond à x au carré. `e^(i*pi)` prend en compte le groupe entre parenthèses.
- **Indices :** saisissez un tiret bas. `x_i` correspond à x avec un petit i en bas. Vous pouvez utiliser les deux, comme dans `x_i^2`.
- **Racines :** saisissez `sqrt(x+1)` pour une racine carrée. Saisissez `sqrt[3]{x}` pour une racine cubique.
- **Sommes, intégrales et produits :** saisissez `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`. La partie placée après le tiret bas se place en dessous (ou en bas du signe d'intégrale), et la partie placée après l'accent circonflexe se place au-dessus.
- **Symboles :** saisissez une barre oblique inverse suivie d'un nom. `\alpha` donne α, `\pi` donne π, `\infty` donne ∞, `\leq` donne ≤, `\neq` donne ≠, `\times` donne ×, `\pm` donne ±, et `\to` donne →. Environ 70 noms sont reconnus.
- **Accents :** `\hat{x}`, `\check{x}`, `\tilde{x}`, `\acute{x}`, `\grave{x}`, `\breve{x}`, `\dot{x}`, `\ddot{x}`, `\bar{x}` et `\vec{v}` placent une marque au-dessus de la lettre. `\overline{ab}` trace un trait au-dessus du groupe, et `\underline{ab}` en trace un en dessous.
- **Parenthèses extensibles :** saisissez `\left( ... \right)`, et les parenthèses s'agrandissent à la hauteur de ce qui se trouve entre elles. Utilisez n'importe quel symbole de parenthèse après `\left` et `\right`, ou un point pour omettre un côté, comme dans `\left[ x \right.`.
- **Matrices :** une **matrice** est un tableau de nombres. Saisissez `\matrix{a & b \\ c & d}`. Placez une `&` entre les cellules d'une ligne, et deux barres obliques inverses entre les lignes. Utilisez `\pmatrix` pour des parenthèses rondes autour du tableau, `\bmatrix` pour des crochets, et `\vmatrix` pour des traits verticaux.
- **Fonctions :** `sin`, `cos`, `tan`, `log`, `ln`, `exp`, `lim`, `min` et `max` conservent leur nom entier.

Dans la page, JeffWrite affiche une équation sous forme d'une seule ligne de texte. L'équation enregistrée dans le fichier conserve ses fractions et ses racines, afin que d'autres logiciels la mettent en forme.

## Saisir en UnicodeMath ou en LaTeX

À côté de la zone se trouve une liste de formats de saisie : **Ligne simple** (celui décrit ci-dessus), **UnicodeMath** et **LaTeX**. Ce sont deux façons d'écrire les mathématiques en texte brut, que beaucoup de personnes connaissent déjà. Lorsque vous choisissez un autre format, ce que vous avez saisi est réécrit dans ce format.

- **UnicodeMath :** `a/b` est une fraction, `x^2_i` contient une puissance et un indice, `√(x)` est une racine, et `∑_(i=1)^n i` est une somme. Placez des mots entre guillemets, comme `"si x"`, et ils restent du texte ordinaire au lieu d'être composés en lettres mathématiques. `(a|b)` crée des parenthèses à deux parties, et `\matrix(a&b@c&d)` crée une matrice (`&` entre les cellules, `@` entre les lignes).
- **LaTeX :** `\frac{a}{b}` est une fraction, `\sqrt[3]{x}` est une racine cubique, `\sum_{i=1}^n i` est une somme, `\text{si x}` est du texte ordinaire, et `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` est une matrice.

Les deux boutons **Professionnel** et **Linéaire** modifient l'affichage de votre équation dans la zone. Professionnel la dessine mise en forme, telle qu'elle apparaîtra. Linéaire n'affiche que la ligne que vous avez saisie. Si une équation ne peut pas être écrite dans le format choisi, la zone vous l'indique et conserve ce que vous avez saisi.

## L'onglet Équation

Sélectionnez une équation (cliquez dessus, ou placez le point d'insertion sur elle avec Maj), et un onglet nommé **Équation** apparaît à la fin du ruban. Il disparaît lorsque vous sélectionnez autre chose.

- **Outils :** **Modifier l'équation** ouvre la zone. **Professionnel** et **Linéaire** déterminent l'affichage initial de la zone, mise en forme ou ligne seule. Le document lui-même affiche toujours une équation sous forme de ligne de texte. **Texte normal** ouvre la zone avec une paire de guillemets ajoutée, pour les mots qui ne sont pas des mathématiques.
- **Convertir :** choisissez le format dans lequel la zone s'ouvre : **Ligne simple**, **UnicodeMath** ou **LaTeX**. La prochaine fois que vous modifierez une équation, elle sera écrite dans le format choisi. Une équation qu'aucune ligne du type simple ne peut écrire, comme une somme avec ses bornes au-dessus et en dessous, s'ouvre en UnicodeMath à la place.
- **Symboles :** lettres grecques, opérateurs, relations, flèches, ensembles et logique. Choisissez un symbole, et la zone s'ouvre avec ce symbole ajouté à la ligne.
- **Structures :** fraction, indice, radical, intégrale, grand opérateur, parenthèses, fonction, accent, limite, opérateur et matrice. Cliquez sur un bouton pour sa forme habituelle, ou sur la flèche à côté pour davantage de formes, comme une dérivée, une racine n-ième ou une matrice identité.

Une structure s'insère à la fin de la ligne, avec des lettres d'exemple, et la première est sélectionnée afin que vous puissiez taper par-dessus. La zone propose les mêmes structures sous forme d'une rangée de petits boutons au-dessus de la ligne. Là, une structure s'insère au niveau du curseur, et si une partie de la ligne est sélectionnée, cette partie passe dans la première place de la structure.

## Équations intégrées

Cliquez sur **Insertion > Équations intégrées** pour afficher une liste d'équations connues, comme la formule quadratique ou l'aire d'un cercle. Cliquez sur l'une d'elles pour l'insérer au point d'insertion.

## Modifier une équation

Placez le point d'insertion à côté d'une équation, ou sélectionnez-la, puis cliquez sur **Insertion > Équation**. La zone s'ouvre avec l'équation écrite sur une seule ligne. Modifiez-la, puis cliquez sur **OK**. Une seule étape suffit pour annuler.

## Équations dans d'autres documents

JeffWrite affiche les équations qu'un document contient, et les conserve telles quelles. Une équation créée par un autre logiciel s'ouvre en UnicodeMath, ou en LaTeX lorsque UnicodeMath ne peut pas l'écrire, afin que vous puissiez la modifier. Si aucun des deux formats ne peut écrire toutes ses parties, comme les signes de regroupement, la zone s'ouvre avec le texte de l'équation. Cliquez sur **Annuler** pour conserver l'équation telle quelle, ou saisissez une nouvelle équation pour la remplacer.

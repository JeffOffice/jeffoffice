# Équations
<!-- keywords: équation, équations, math, mathématiques, formule, fraction, racine carrée, exposant, indice, somme, intégrale, grec, symbole, insérer une équation, alt+=, accent, chapeau, vecteur, barre, matrice, parenthèses, équations intégrées, galerie, modifier une équation, éditer une équation -->

Une **équation** est une expression mathématique, comme une fraction ou une racine carrée. Cette rubrique explique comment en saisir une et l'insérer dans un document.

## Insérer une équation

1. Placez le point d'insertion à l'endroit où doit figurer l'équation.
2. Cliquez sur **Insertion > Équation**, ou appuyez sur **Alt+=**.
3. Saisissez l'équation sur une seule ligne. Sous la zone, vous voyez l'équation telle qu'elle apparaîtra, ainsi que la ligne de texte qui sera affichée dans la page. Pour partir d'une équation connue, choisissez-en une dans la liste **Équations intégrées** de la zone. Cliquez sur **OK**.

L'équation est enregistrée dans le document selon la forme standard des mathématiques, afin que d'autres programmes puissent l'ouvrir.

## Comment la saisir

- **Fractions :** saisissez une barre oblique. `a/b` correspond à a sur b. `(x+1)/(x-1)` traite chaque groupe entre parenthèses comme un seul élément. `\frac{1}{2}` fonctionne aussi.
- **Puissances :** saisissez un accent circonflexe. `x^2` correspond à x au carré. `e^(i*pi)` traite le groupe entre parenthèses.
- **Indices :** saisissez un tiret bas. `x_i` correspond à x avec un petit i en dessous. Vous pouvez utiliser les deux, comme dans `x_i^2`.
- **Racines :** saisissez `sqrt(x+1)` pour une racine carrée. Saisissez `sqrt[3]{x}` pour une racine cubique.
- **Sommes, intégrales et produits :** saisissez `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`. La partie après le tiret bas se place en dessous (ou en bas du signe d'intégrale), et la partie après le circonflexe se place au-dessus.
- **Symboles :** saisissez une barre oblique inverse et un nom. `\alpha` donne α, `\pi` donne π, `\infty` donne ∞, `\leq` donne ≤, `\neq` donne ≠, `\times` donne ×, `\pm` donne ±, et `\to` donne →. Environ 70 noms sont disponibles.
- **Accents :** `\hat{x}`, `\check{x}`, `\tilde{x}`, `\acute{x}`, `\grave{x}`, `\breve{x}`, `\dot{x}`, `\ddot{x}`, `\bar{x}` et `\vec{v}` placent une marque au-dessus de la lettre. `\overline{ab}` trace un trait au-dessus du groupe, et `\underline{ab}` en trace un en dessous.
- **Parenthèses extensibles :** saisissez `\left( ... \right)`, et les parenthèses s'agrandissent à la hauteur de ce qui se trouve entre elles. Utilisez n'importe quel signe de parenthèse après `\left` et `\right`, ou un point pour omettre un côté, comme dans `\left[ x \right.`.
- **Matrices :** une **matrice** est une grille de nombres. Saisissez `\matrix{a & b \\ c & d}`. Placez un `&` entre les cellules d'une ligne, et deux barres obliques inverses entre les lignes. Utilisez `\pmatrix` pour des parenthèses rondes autour de la grille, `\bmatrix` pour des crochets, et `\vmatrix` pour des traits droits.
- **Fonctions :** `sin`, `cos`, `tan`, `log`, `ln`, `exp`, `lim`, `min` et `max` conservent leur nom entier.

Dans la page, JeffWrite affiche une équation sous la forme d'une ligne de texte. L'équation dans le fichier conserve ses fractions et ses racines, afin que d'autres programmes en assurent la mise en forme.

## Équations intégrées

Cliquez sur **Insertion > Équations intégrées** pour afficher une liste d'équations connues, comme la formule quadratique et l'aire d'un cercle. Cliquez sur l'une d'elles pour l'insérer au point d'insertion.

## Modifier une équation

Placez le point d'insertion à côté d'une équation, ou sélectionnez-la, puis cliquez sur **Insertion > Équation**. La zone s'ouvre avec l'équation écrite sur une seule ligne. Modifiez-la, puis cliquez sur **OK**. Une seule étape suffit pour l'annuler.

## Équations dans d'autres documents

JeffWrite affiche les équations qu'un document contient et les conserve telles quelles. Il peut modifier une équation composée des éléments indiqués ci-dessus. Une équation contenant d'autres éléments, comme des symboles de regroupement, est conservée telle quelle : dans ce cas, Insertion > Équation en insère une nouvelle au lieu de la modifier.

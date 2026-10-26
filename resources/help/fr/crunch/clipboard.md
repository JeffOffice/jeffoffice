# Couper, copier et coller
<!-- keywords: couper, copier, coller, presse-papiers, déplacer, dupliquer, collage spécial, valeurs, formules, formats, transposer, blancs non compris, addition, soustraction, multiplication, division, opération, cellules fusionnées, texte, tabulation, html, page web, autre programme, Échap, pointillés, bordure clignotante, bordure -->

**Copier** (**{key:Copy}**) place une copie des cellules sélectionnées dans le presse-papiers, un emplacement où l'ordinateur conserve un seul élément à la fois. **Coller** (**{key:Paste}**) insère le contenu du presse-papiers dans les cellules, à partir de la cellule sélectionnée. **Couper** (**{key:Cut}**) fonctionne comme Copier, mais lorsque vous collez, les cellules sont déplacées et leur emplacement d'origine est vidé.

Une bordure en pointillés animée entoure les cellules copiées ou coupées. Appuyez sur **Échap** pour la faire disparaître ; les cellules ne sont alors plus prêtes à être collées. Les cellules coupées sont également libérées si vous effectuez une autre modification avant de les coller (saisie, insertion, annulation) : coupez-les de nouveau si vous voulez toujours les déplacer.

## Coller dans JeffCrunch

Tout est conservé : les valeurs, les formules, les formats et les cellules fusionnées. Une formule continue de faire référence aux mêmes emplacements *par rapport à sa position*. Exemple : si vous copiez **=A1*2** depuis B1 et la collez en B5, elle devient **=A5*2**. Une référence comportant un signe dollar, comme **$A$1**, reste inchangée. Si vous sélectionnez une plage dont la taille est un multiple entier de celle de la copie, la copie est répétée pour la remplir : copiez une cellule, sélectionnez vingt cellules, collez, et les vingt cellules reçoivent le contenu.

Si un nombre collé est trop large pour une colonne qui a encore sa largeur d'origine, la colonne s'élargit.

## Couper déplace les cellules

Lorsque vous coupez puis collez des cellules, celles-ci sont réellement déplacées. Toutes les formules qui y faisaient référence pointent vers leur nouvel emplacement. Exemple : B1 contient **=A1+A2**. Coupez A1:A2 et collez en E4 : B1 devient **=E4+E5**. Le contenu des cellules remplacées par le collage est perdu (**Annuler** le rétablit), et une formule qui lisait l'une de ces cellules affiche **#REF!**, car la cellule qu'elle lisait n'existe plus. Si les cellules coupées ne représentent qu'une partie d'une cellule fusionnée, ou si vous collez sur une partie seulement d'une cellule fusionnée, rien n'est déplacé : sélectionnez toute la cellule fusionnée. Lorsque vous coupez des cellules vers une autre feuille, les formules continuent de lire les mêmes cellules qu'auparavant (elles indiquent le nom de la feuille). Le presse-papiers est vidé après un couper-coller.

## Collage spécial

**Ctrl+Alt+V**, ou la flèche sous **Coller** dans l'onglet Accueil, permet de choisir ce que vous collez :

- **Tout**, **Formules**, **Valeurs** ou **Formats**. *Valeurs* remplace une formule par le nombre ou le texte qu'elle affiche.
- Une **opération** : **Addition**, **Soustraction**, **Multiplication** ou **Division**. Les nombres copiés sont combinés avec ceux déjà présents dans les cellules. Exemple : les cellules contiennent 10 et 20, vous avez copié 5 et choisi *Multiplication* : elles deviennent 50 et 100.
- **Blancs non compris** : les cellules vides de la copie laissent inchangées les cellules sur lesquelles elles sont collées.
- **Transposer** : les lignes deviennent des colonnes et les colonnes des lignes.

## Entre programmes

Lorsque vous copiez, JeffCrunch place aussi les cellules dans le presse-papiers sous forme de texte (cellules séparées par des tabulations, une ligne par ligne de texte) et sous forme de tableau pour les pages web, afin que d'autres programmes puissent les récupérer. Lorsque vous collez un contenu qui ne provient pas de JeffCrunch (du texte avec des tabulations, ou un tableau issu d'une page web ou d'une autre feuille de calcul), chaque élément est placé dans sa propre cellule et interprété comme si vous l'aviez tapé : nombres, dates et formules commençant par un signe égal. Une copie depuis un autre classeur ouvert dans JeffCrunch apporte les valeurs, les formules et les formats de nombre, mais pas le reste de la mise en forme.

## Ce qui ne peut pas être collé

JeffCrunch vous prévient et ne modifie rien si le collage doit atteindre une partie d'une **formule matricielle**, la ligne d'en-tête d'un **tableau**, des cellules verrouillées d'une **feuille protégée**, dépasser la fin de la feuille ou porter sur plus de 100 000 cellules. Une **formule matricielle** entière peut être coupée et déplacée (sélectionnez toutes ses cellules) ; ce n'est pas possible pour une partie de formule matricielle, une table de données ou la ligne d'en-tête d'un tableau.

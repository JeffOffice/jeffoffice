# Couper, copier et coller

<!-- keywords: couper, copier, coller, presse-papiers, déplacer, dupliquer, collage spécial, valeurs, formules, formats, transposer, ignorer les blancs, additionner, soustraire, multiplier, diviser, opération, fusionnée, cellule fusionnée, texte, tabulation, html, page web, autre programme, Échap, bordure animée, bordures, largeurs de colonnes, notes, commentaires, formats de nombres, toutes sauf les bordures, image, bitmap, copier en tant qu'image, collage, cellules, copie -->

**Copier** (**{key:Ctrl+C}**) place une copie des cellules sélectionnées dans le presse-papiers, un emplacement où l'ordinateur conserve un élément à la fois. **Coller** (**{key:Ctrl+V}**) place le contenu du presse-papiers dans les cellules à partir de la cellule sélectionnée. **Couper** (**{key:Ctrl+X}**) fonctionne comme copier, mais lors du collage, les cellules sont déplacées et l'emplacement d'origine est vidé.

Une bordure en pointillés animée indique les cellules copiées ou coupées. Appuyez sur **Échap** pour faire disparaître cette bordure ; les cellules ne sont alors plus prêtes à être collées. Les cellules coupées sont également libérées si vous effectuez une autre modification avant de les coller (saisie, insertion, annulation) : coupez-les à nouveau si vous voulez toujours les déplacer.

## Coller dans JeffCrunch

Tout est repris : les valeurs, les formules, les formats et les cellules fusionnées. Une formule continue de pointer vers les mêmes emplacements *par rapport à sa position*. Exemple : copiez **=A1*2** depuis B1 et collez-la en B5, elle devient **=A5*2**. Une référence précédée d'un signe dollar, comme **$A$1**, reste inchangée. Si vous sélectionnez une plage dont la taille est un multiple exact de celle de la zone copiée, la copie est répétée pour la remplir : copiez une cellule, sélectionnez vingt cellules, collez, et les vingt reçoivent le contenu.

Un nombre collé trop large pour une colonne qui conserve sa largeur d'origine élargit la colonne.

## Couper déplace les cellules

Lorsque vous coupez des cellules puis les collez, elles sont réellement déplacées. Toute formule qui pointait vers elles pointe désormais vers leur nouvel emplacement. Exemple : B1 contient **=A1+A2**. Coupez A1:A2 et collez en E4 : B1 devient **=E4+E5**. Le contenu des cellules écrasées par le collage est perdu (**Annuler** le restaure), et une formule qui lisait l'une de ces cellules affiche **#RÉF!**, car la cellule qu'elle lisait n'existe plus. Si les cellules coupées ne représentent qu'une partie d'une cellule fusionnée, ou si vous collez sur une partie seulement d'une cellule fusionnée, rien n'est déplacé : sélectionnez la cellule fusionnée entière. Lorsque vous coupez des cellules pour les coller sur une autre feuille, les formules continuent de lire les cellules qu'elles lisaient auparavant (elles indiquent la feuille concernée). Le presse-papiers est vidé après un couper-coller.

## Collage spécial

**Ctrl+Alt+V**, ou la flèche située sous **Coller** dans l'onglet Accueil, permet de choisir ce qui est collé :

- **Tout**, **Formules**, **Valeurs** ou **Formats**. *Valeurs* remplace une formule par le nombre ou le texte qu'elle affiche.
- **Tout sauf les bordures** : tout est collé, mais les cellules conservent les bordures qu'elles avaient.
- **Formules et formats de nombres**, ou **Valeurs et formats de nombres** : les formules (ou les valeurs) sont collées avec la manière dont les nombres sont affichés, comme le pourcentage ou une date, mais sans les couleurs, les polices ni les bordures. Les cellules conservent l'aspect qu'elles avaient.
- **Largeurs de colonnes** : les colonnes dans lesquelles vous collez reçoivent les largeurs des colonnes copiées.
- **Commentaires et notes** : les notes des cellules copiées sont ajoutées aux cellules dans lesquelles vous collez. Aucune valeur de cellule ne change.
- Une **opération** : **Additionner**, **Soustraire**, **Multiplier** ou **Diviser**. Les nombres copiés sont combinés avec les nombres déjà présents dans les cellules. Exemple : les cellules contiennent 10 et 20, vous avez copié 5 et choisi *Multiplier* : elles deviennent 50 et 100.
- **Ignorer les blancs** : les cellules vides de la copie laissent inchangées les cellules sur lesquelles elles tombent.
- **Transposer** : les lignes deviennent des colonnes, et les colonnes deviennent des lignes.

## Le menu Coller et les images

La flèche située sous **Coller** propose les choix habituels, ainsi que **Sans bordures**, **Formules et mise en forme des nombres**, **Valeurs et mise en forme des nombres**, **Conserver les largeurs de colonnes source**, et **Commentaires et notes**. Ils correspondent aux choix du collage spécial, en un seul clic chacun.

La flèche située à côté de **Copier** propose **Copier en tant qu'image** et **Copier en tant que bitmap**. Chacune place sur le presse-papiers une image des cellules sélectionnées, dessinée telle qu'elles apparaissent à l'écran, avec le quadrillage. Un bitmap a la taille de l'écran. Une image contient deux fois plus de points, elle reste donc nette lorsqu'elle est agrandie. Collez-la dans JeffWrite ou JeffDeck en tant qu'image. Si les cellules ne tiennent pas dans une seule image, sélectionnez-en moins.

## Entre programmes

Lorsque vous copiez, JeffCrunch place également les cellules dans le presse-papiers sous forme de texte (cellules séparées par des tabulations, une ligne par rangée) et sous forme de tableau pour les pages web, afin que d'autres programmes puissent les récupérer. Lorsque vous collez un contenu qui ne provient pas de JeffCrunch, du texte avec des tabulations, ou un tableau issu d'une page web ou d'une autre feuille de calcul, chaque élément est placé dans sa propre cellule et interprété comme si vous l'aviez saisi : nombres, dates et formules commençant par un signe égal. Copier depuis un autre classeur ouvert dans JeffCrunch transfère les valeurs, les formules et les formats de nombres, mais pas les autres mises en forme.

## Ce qui ne peut pas être collé

JeffCrunch vous en informe et ne modifie rien lorsque le collage empièterait sur une partie d'une **formule matricielle**, sur la ligne des noms de colonnes d'un **tableau**, sur des cellules verrouillées d'une **feuille protégée**, au-delà de la fin de la feuille, ou sur plus de 100 000 cellules. Une **formule matricielle** entière peut être coupée et déplacée (sélectionnez toutes ses cellules) ; une partie de formule matricielle, une table de données ou la ligne des noms d'un tableau ne le peuvent pas.

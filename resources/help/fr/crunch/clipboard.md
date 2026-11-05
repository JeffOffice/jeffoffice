# Couper, copier et coller

<!-- keywords: couper, copier, coller, presse-papiers, déplacer, dupliquer, collage spécial, valeurs, formules, formats, transposer, ignorer les cellules vides, additionner, soustraire, multiplier, diviser, opération, fusionnée, texte, tabulation, html, page web, autre programme, Échap, bordure en pointillés, largeurs de colonnes, notes, commentaires, formats de nombre, toutes sauf les bordures, image, bitmap, copier en tant qu'image, image imprimée, comme imprimé, coller un lien, lien hypertexte, importation de texte, scinder le texte, séparateur -->

**Copier** (**{key:Copy}**) place une copie des cellules sélectionnées dans le presse-papiers, un emplacement où l'ordinateur conserve un seul élément à la fois. **Coller** (**{key:Paste}**) place le contenu du presse-papiers dans les cellules, à partir de la cellule sélectionnée. **Couper** (**{key:Cut}**) fonctionne comme copier, mais lors du collage, les cellules sont déplacées et l'emplacement d'origine est vidé.

Une bordure en pointillés animée indique les cellules copiées ou coupées. Appuyez sur **Échap** pour la faire disparaître ; les cellules ne sont alors plus prêtes à être collées. Les cellules coupées sont aussi libérées si vous effectuez une autre modification avant de les coller (saisie, insertion, annulation) : coupez-les à nouveau si vous voulez toujours les déplacer.

## Coller dans JeffCrunch

Tout est collé : les valeurs, les formules, les formats et les cellules fusionnées. Une formule continue de pointer vers les mêmes emplacements *par rapport à sa position*. Exemple : copiez **=A1*2** depuis B1 et collez-la en B5, elle devient **=A5*2**. Une référence précédée d'un signe dollar, comme **$A$1**, reste inchangée. Si vous sélectionnez une plage dont la taille est un multiple exact de celle de ce que vous avez copié, la copie est répétée pour la remplir : copiez une cellule, sélectionnez vingt cellules, collez, et les vingt reçoivent le contenu.

Un nombre collé trop large pour une colonne qui a encore sa largeur initiale élargit la colonne.

## Couper déplace les cellules

Lorsque vous coupez des cellules puis les collez, elles sont réellement déplacées. Chaque formule qui pointait vers elles pointe désormais vers leur nouvel emplacement. Exemple : B1 contient **=A1+A2**. Coupez A1:A2 et collez en E4 : B1 devient **=E4+E5**. Le contenu des cellules écrasées par le collage est perdu (**Annuler** le restaure), et une formule qui lisait l'une de ces cellules affiche **#REF!**, car la cellule qu'elle lisait n'existe plus. Si les cellules coupées ne sont qu'une partie d'une cellule fusionnée, ou si vous collez sur une partie seulement d'une cellule fusionnée, rien n'est déplacé : sélectionnez la cellule fusionnée entière. Lorsque vous coupez des cellules pour les coller sur une autre feuille, les formules continuent de lire les cellules qu'elles lisaient auparavant (elles indiquent la feuille). Le presse-papiers est vidé après un couper-coller.

## Collage spécial

**Ctrl+Alt+V**, ou la flèche sous **Coller** dans l'onglet Accueil, vous permet de choisir ce que vous collez :

- **Tout**, **Formules**, **Valeurs** ou **Formats**. *Valeurs* remplace une formule par le nombre ou le texte qu'elle affiche.
- **Tout sauf les bordures** : tout, mais les cellules conservent les bordures qu'elles avaient.
- **Formules et formats de nombre**, ou **Valeurs et formats de nombre** : les formules (ou les valeurs) sont accompagnées de la manière dont les nombres sont affichés, comme le pourcentage ou la date, mais pas des couleurs, des polices ni des bordures. Les cellules conservent l'aspect qu'elles avaient.
- **Largeurs de colonnes** : les colonnes dans lesquelles vous collez reçoivent les largeurs des colonnes copiées.
- **Commentaires et notes** : les notes des cellules copiées sont placées dans les cellules où vous collez. Aucune valeur de cellule ne change.
- Une **opération** : **Additionner**, **Soustraire**, **Multiplier** ou **Diviser**. Les nombres copiés sont combinés aux nombres déjà présents dans les cellules. Exemple : les cellules contiennent 10 et 20, vous avez copié 5, et vous avez choisi *Multiplier* : elles deviennent 50 et 100.
- **Ignorer les cellules vides** : les cellules vides de la copie laissent les cellules sur lesquelles elles tombent inchangées.
- **Transposer** : les lignes deviennent des colonnes, et les colonnes deviennent des lignes.

## Coller un lien, Coller en tant que lien hypertexte et Importation de texte

Trois autres options se trouvent dans la flèche sous **Coller**. Elles fonctionnent sur des cellules copiées dans le même classeur, sauf la dernière.

- **Coller un lien** place dans chaque cellule une formule qui pointe vers la cellule copiée. Exemple : copiez A1:B2, sélectionnez D4 et utilisez **Coller un lien** : D4 devient **=Feuil1!$A$1**. Lorsque A1 change, D4 change avec elle. Une cellule copiée vide affiche 0.
- **Coller en tant que lien hypertexte** place dans chaque cellule le même texte que celui qu'affiche la cellule copiée, et fait de chacun un lien vers la cellule copiée. Maintenez **Ctrl** enfoncée et cliquez dessus pour aller à cette cellule. Cette option fonctionne sur 2 000 cellules au maximum à la fois.
- **Importation de texte** sert pour du texte provenant d'un autre programme. Elle ouvre la même fenêtre que l'ouverture d'un fichier texte : vous indiquez ce qui sépare les éléments (une virgule, une tabulation, un espace, un point-virgule, une barre verticale ou un autre caractère), le caractère qui encadre un élément, la première ligne à utiliser, et la manière de lire chaque colonne (telle quelle, en texte, en date, ou ignorée). Choisissez *Texte* pour une colonne de codes comme 007, afin que les zéros soient conservés. Les éléments sont placés dans les cellules, à partir de la cellule sélectionnée, en une seule étape que vous pouvez annuler.

## Le menu Coller et les images

La flèche sous **Coller** affiche les choix habituels, ainsi que **Sans bordures**, **Formules et mise en forme des nombres**, **Valeurs et mise en forme des nombres**, **Conserver les largeurs de colonnes source** et **Commentaires et notes**. Ce sont les mêmes choix que dans Collage spécial, en un seul clic chacun.

La flèche à côté de **Copier** propose **Copier en tant qu'image** et **Copier en tant que bitmap**. Chacune place sur le presse-papiers une image des cellules sélectionnées, dessinée telle que vous la voyez, avec le quadrillage. Un bitmap a la taille de l'écran. Une image comporte deux fois plus de points, elle reste donc nette lorsqu'elle est agrandie. **Copier comme image imprimée** dessine les cellules telles que le papier les montrerait : le quadrillage n'apparaît que si la mise en page le prévoit à l'impression. Collez-la dans JeffWrite ou JeffDeck comme une image. Si les cellules ne tiennent pas dans une seule image, sélectionnez-en moins.

## Entre programmes

Lorsque vous copiez, JeffCrunch place aussi les cellules sur le presse-papiers sous forme de texte (cellules séparées par des tabulations, une ligne par rangée) et sous forme de tableau pour les pages web, afin que d'autres programmes puissent les récupérer. Lorsque vous collez quelque chose qui ne vient pas de JeffCrunch, du texte avec des tabulations ou un tableau provenant d'une page web ou d'une autre feuille de calcul, chaque élément va dans sa propre cellule et est interprété comme si vous l'aviez saisi : nombres, dates et formules commençant par un signe égal. Copier depuis un autre classeur ouvert dans JeffCrunch apporte les valeurs, les formules et les formats de nombre, mais pas les autres mises en forme.

## Ce qui ne peut pas être collé

JeffCrunch vous en informe et ne modifie rien lorsque le collage toucherait une partie d'une **formule matricielle**, la ligne des noms de colonnes d'un **tableau**, des cellules verrouillées d'une **feuille protégée**, au-delà de la fin de la feuille, ou plus de 100 000 cellules. Une **formule matricielle** entière peut être coupée et déplacée (sélectionnez toutes ses cellules) ; une partie de celle-ci, un tableau de données ou la ligne des noms d'un tableau ne le peuvent pas.

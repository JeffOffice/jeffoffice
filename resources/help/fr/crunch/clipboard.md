# Couper, copier et coller

<!-- keywords: couper, copier, coller, presse-papiers, déplacer, dupliquer, collage spécial, valeurs, formules, formats, transposer, ignorer les blancs, additionner, soustraire, multiplier, diviser, opération, fusionnée, texte, tabulation, html, page web, autre programme, Échap, fourmis, bordure, largeurs de colonnes, notes, commentaires, formats de nombre, toutes sauf les bordures, image, bitmap, copier comme image, image imprimée, comme imprimé, coller un lien, lien hypertexte, importation de texte, scinder le texte, séparateur, définir le collage par défaut, collage par défaut, largeur fixe, copie, coller un lien hypertexte, collage, coupe -->

**Copier** (**{key:Copy}**) place une copie des cellules sélectionnées dans le presse-papiers, un emplacement où l'ordinateur conserve un élément à la fois. **Coller** (**{key:Paste}**) place le contenu du presse-papiers dans les cellules, à partir de la cellule sélectionnée. **Couper** (**{key:Cut}**) fonctionne comme copier, mais lors du collage, les cellules sont déplacées et l'emplacement d'origine est vidé.

Une bordure en pointillés mobile indique les cellules copiées ou coupées. Appuyez sur **Échap** pour faire disparaître cette bordure ; les cellules ne sont alors plus prêtes à être collées. Les cellules coupées sont aussi libérées dès que vous effectuez une autre modification avant de les coller (saisie, insertion, annulation) : coupez-les de nouveau si vous voulez toujours les déplacer.

## Coller dans JeffCrunch

Tout est repris : les valeurs, les formules, les formats et les cellules fusionnées. Une formule continue de pointer vers les mêmes emplacements *par rapport à sa position*. Exemple : copiez **=A1*2** depuis B1 et collez-la en B5, elle devient **=A5*2**. Une partie précédée d'un signe dollar, comme **$A$1**, reste inchangée. Si vous sélectionnez une plage dont la taille est un multiple exact de celle de ce que vous avez copié, la copie est répétée pour la remplir : copiez une cellule, sélectionnez vingt cellules, collez, et les vingt reçoivent le contenu.

Un nombre collé trop large pour une colonne qui a encore sa largeur initiale élargit la colonne.

## Couper déplace les cellules

Lorsque vous coupez des cellules puis les collez, elles sont réellement déplacées. Chaque formule qui pointait vers elles pointe désormais vers leur nouvel emplacement. Exemple : B1 contient **=A1+A2**. Coupez A1:A2 puis collez en E4 : B1 devient **=E4+E5**. Le contenu des cellules écrasées par le collage disparaît (**Annuler** le restaure), et une formule qui lisait l'une de ces cellules affiche **#RÉF!**, car la cellule qu'elle lisait n'existe plus. Si les cellules coupées ne forment qu'une partie d'une cellule fusionnée, ou si vous collez sur une partie seulement d'une cellule fusionnée, rien n'est déplacé : sélectionnez la cellule fusionnée entière. Lorsque vous coupez des cellules pour les coller sur une autre feuille, les formules continuent de lire les cellules qu'elles lisaient auparavant (elles indiquent la feuille). Le presse-papiers est vidé après un couper-coller.

## Collage spécial

**Ctrl+Alt+V**, ou la flèche sous **Coller** dans l'onglet Accueil, permet de choisir ce que vous collez :

- **Tout**, **Formules**, **Valeurs** ou **Formats**. *Valeurs* transforme une formule en le nombre ou le texte qu'elle affiche.
- **Tout sauf les bordures** : tout, mais les cellules conservent les bordures qu'elles avaient.
- **Formules et formats de nombre**, ou **Valeurs et formats de nombre** : les formules (ou les valeurs) sont accompagnées de la manière dont les nombres sont affichés, comme le pourcentage ou la date, mais pas des couleurs, des polices ni des bordures. Les cellules gardent leur apparence d'origine.
- **Largeurs de colonnes** : les colonnes dans lesquelles vous collez reçoivent les largeurs des colonnes copiées.
- **Commentaires et notes** : les notes des cellules copiées sont placées dans les cellules où vous collez. Aucune valeur de cellule ne change.
- Une **opération** : **Additionner**, **Soustraire**, **Multiplier** ou **Diviser**. Les nombres copiés sont combinés avec les nombres déjà présents dans les cellules. Exemple : les cellules contiennent 10 et 20, vous avez copié 5 et choisi *Multiplier* : elles deviennent 50 et 100.
- **Ignorer les blancs** : les cellules vides de la copie laissent intactes les cellules de destination.
- **Transposer** : les lignes deviennent des colonnes, et les colonnes deviennent des lignes.

## Coller un lien, Coller comme lien hypertexte et Importation de texte

Trois autres choix se trouvent dans la flèche sous **Coller**. Ils s'appliquent à des cellules copiées dans le même classeur, sauf le dernier.

- **Coller un lien** place dans chaque cellule une formule qui pointe vers la cellule copiée. Exemple : copiez A1:B2, sélectionnez D4 et utilisez **Coller un lien** : D4 devient **=Feuil1!$A$1**. Lorsque A1 change, D4 change avec elle. Une cellule copiée vide affiche 0.
- **Coller comme lien hypertexte** place dans chaque cellule les mêmes mots que ceux qu'affiche la cellule copiée, et fait de chacun un lien vers la cellule copiée. Maintenez **Ctrl** et cliquez dessus pour accéder à cette cellule. Cela fonctionne sur 2 000 cellules au maximum à la fois.
- **Importation de texte** sert à coller du texte provenant d'un autre programme. Elle ouvre la même fenêtre que l'ouverture d'un fichier texte : vous indiquez ce qui sépare les éléments (une virgule, une tabulation, un espace, un point-virgule, une barre verticale ou un autre caractère), le caractère qui encadre un élément, la première ligne à utiliser et la manière de lire chaque colonne (telle que saisie, en texte, en date, ou ignorée). Certains textes alignent leurs éléments en colonnes de même largeur, séparées par des espaces. Cochez **Les colonnes ont une largeur fixe**, puis indiquez où commence chaque colonne : chaque ligne est coupée à ces emplacements, et les espaces autour de chaque élément sont supprimés. Choisissez *Texte* pour une colonne de codes comme 007, afin de conserver les zéros. Les éléments sont placés dans les cellules, à partir de la cellule sélectionnée, en une seule étape que vous pouvez annuler.

## Définir le collage par défaut

**Définir le collage par défaut** est le dernier choix de la flèche sous **Coller**. Il détermine ce qu'insèrent le bouton **Coller** et **{key:Paste}** lorsque vous avez copié des cellules : tout (par défaut), ou seulement les valeurs, seulement les formules, seulement la mise en forme, et ainsi de suite. Exemple : si vous choisissez *Valeurs*, une formule copiée est collée sous forme du nombre qu'elle affiche. Ce choix est conservé pour la prochaine fois. Il ne modifie pas le **Collage spécial** ni les choix du menu, et le texte provenant d'autres programmes est collé tel quel. Choisissez *Tout* pour revenir au comportement initial.

## Le menu Coller et les images

La flèche sous **Coller** affiche les choix habituels, ainsi que **Sans bordures**, **Formules et mise en forme des nombres**, **Valeurs et mise en forme des nombres**, **Conserver les largeurs de colonnes source** et **Commentaires et notes**. Ils sont identiques aux choix du **Collage spécial**, en un seul clic chacun.

La flèche à côté de **Copier** propose **Copier comme image** et **Copier en tant que bitmap**. Chacune place sur le presse-papiers une image des cellules sélectionnées, telle qu'elles s'affichent, avec le quadrillage. Un bitmap a la taille de l'écran. Une image comporte deux fois plus de points, elle reste donc nette lorsqu'on l'agrandit. **Copier comme image imprimée** dessine les cellules telles que le papier les montrerait : le quadrillage n'apparaît que si la mise en page le prévoit à l'impression. Collez-la dans JeffWrite ou JeffDeck comme une image. Si les cellules ne tiennent pas dans une seule image, sélectionnez-en moins.

## Entre les programmes

Lorsque vous copiez, JeffCrunch place aussi les cellules dans le presse-papiers sous forme de texte (cellules séparées par des tabulations, une ligne par rangée) et sous forme de tableau pour les pages web, afin que les autres programmes puissent les récupérer. Lorsque vous collez un contenu qui ne vient pas de JeffCrunch, du texte avec des tabulations, ou un tableau provenant d'une page web ou d'une autre feuille de calcul, chaque élément est placé dans sa propre cellule et interprété comme si vous l'aviez saisi : nombres, dates et formules commençant par un signe égal. Copier depuis un autre classeur ouvert dans JeffCrunch apporte les valeurs, les formules et les formats de nombre, mais pas les autres mises en forme.

## Ce qui ne peut pas être collé

JeffCrunch vous en informe et ne modifie rien lorsque le collage toucherait une partie d'une **formule matricielle**, la ligne des noms de colonnes d'un **tableau**, des cellules verrouillées d'une **feuille protégée**, qui dépasserait la fin de la feuille, ou qui concernerait plus de 100 000 cellules. Une **formule matricielle** entière peut être coupée et déplacée (sélectionnez toutes ses cellules) ; une partie de formule matricielle, une table de données ou la ligne des noms d'un tableau ne le peuvent pas.

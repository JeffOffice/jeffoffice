# Couper, copier et coller

<!-- keywords: couper, copier, coller, presse-papiers, déplacer, dupliquer, collage spécial, valeurs, formules, formats, transposer, ignorer les blancs, ajouter, soustraire, multiplier, diviser, opération, fusionnée, texte, tabulation, html, page web, autre programme, Échap, bordure animée, fourmis, bordure, largeurs de colonnes, notes, commentaires, formats de nombre, toutes sauf les bordures, image, bitmap, copier comme image, image imprimée, comme imprimé, coller un lien, lien hypertexte, importation de texte, texte délimité, déposer du texte, virgule, point-virgule, csv, séparateur, définir le collage par défaut, collage par défaut, largeur fixe, coller, copie, raccourci clavier, Ctrl+C, Ctrl+X, Ctrl+V -->

**Copier** (**{key:Copy}**) place une copie des cellules sélectionnées dans le presse-papiers, un emplacement où l'ordinateur conserve un élément à la fois. **Coller** (**{key:Paste}**) place le contenu du presse-papiers dans les cellules, à partir de la cellule sélectionnée. **Couper** (**{key:Cut}**) fonctionne comme copier, mais au moment du collage, les cellules sont déplacées et l'emplacement d'origine est vidé.

Une bordure en pointillés animée indique les cellules copiées ou coupées. Appuyez sur **Échap** pour la faire disparaître ; les cellules ne peuvent alors plus être collées. Les cellules coupées sont aussi libérées si vous effectuez une autre modification avant de les coller (saisie, insertion, annulation) : coupez-les de nouveau si vous voulez toujours les déplacer.

## Coller dans JeffCrunch

Tout est repris : les valeurs, les formules, les formats et les cellules fusionnées. Une formule continue de pointer vers les mêmes emplacements *par rapport à sa position*. Exemple : copiez **=A1*2** depuis B1 et collez-la en B5, elle devient **=A5*2**. Une partie précédée d'un signe dollar, comme **$A$1**, reste inchangée. Si vous sélectionnez une plage dont la taille est un multiple exact de celle de la copie, la copie est répétée pour la remplir : copiez une cellule, sélectionnez vingt cellules, collez, et les vingt reçoivent le contenu.

Un nombre collé trop large pour une colonne qui a encore sa largeur initiale élargit la colonne.

## Couper déplace les cellules

Lorsque vous coupez des cellules puis les collez, elles sont réellement déplacées. Chaque formule qui pointait vers elles pointe désormais vers leur nouvel emplacement. Exemple : B1 contient **=A1+A2**. Coupez A1:A2 et collez en E4 : B1 devient **=E4+E5**. Le contenu des cellules écrasées par le collage est perdu (**Annuler** le restaure), et une formule qui lisait l'une de ces cellules affiche **#REF!**, car la cellule qu'elle lisait n'existe plus. Si les cellules coupées ne forment qu'une partie d'une cellule fusionnée, ou si vous collez sur une partie seulement d'une cellule fusionnée, rien n'est déplacé : sélectionnez la cellule fusionnée entière. Lorsque vous coupez des cellules pour les coller sur une autre feuille, les formules continuent de lire les cellules qu'elles lisaient auparavant (elles indiquent la feuille). Le presse-papiers est vidé après un couper-coller.

## Collage spécial

**Ctrl+Alt+V**, ou la flèche sous **Coller** dans l'onglet Accueil, permet de choisir ce qui est collé :

- **Tout**, **Formules**, **Valeurs** ou **Formats**. *Valeurs* remplace une formule par le nombre ou le texte qu'elle affiche.
- **Tout sauf les bordures** : tout, mais les cellules conservent les bordures qu'elles avaient.
- **Formules et formats de nombre**, ou **Valeurs et formats de nombre** : les formules (ou les valeurs) sont accompagnées de la façon dont les nombres sont affichés, comme un pourcentage ou une date, mais sans les couleurs, les polices ni les bordures. Les cellules gardent l'aspect qu'elles avaient.
- **Largeurs de colonnes** : les colonnes dans lesquelles vous collez reçoivent les largeurs des colonnes copiées.
- **Commentaires et notes** : les notes des cellules copiées sont placées dans les cellules où vous collez. Aucune valeur de cellule ne change.
- Une **opération** : **Ajouter**, **Soustraire**, **Multiplier** ou **Diviser**. Les nombres copiés sont combinés aux nombres déjà présents dans les cellules. Exemple : les cellules contiennent 10 et 20, vous avez copié 5, et vous choisissez *Multiplier* : elles deviennent 50 et 100.
- **Ignorer les blancs** : les cellules vides de la copie laissent intactes les cellules sur lesquelles elles tombent.
- **Transposer** : les lignes deviennent des colonnes, et les colonnes des lignes.

## Le bouton Options de collage

Juste après avoir collé des cellules copiées dans JeffCrunch, un petit bouton **Options de collage** apparaît à côté de la dernière cellule collée. Cliquez dessus pour choisir une autre façon de placer les mêmes cellules : **Conserver la mise en forme source** (tout), **Valeurs**, **Formules**, **Mise en forme**, **Sans bordures**, **Formules et mise en forme des nombres**, **Valeurs et mise en forme des nombres**, **Conserver les largeurs de colonnes source**, ou **Transposer**. JeffCrunch annule le collage et le refait de cette façon. La modification ne compte que pour une seule étape : un seul **Annuler** annule tout le collage, quel que soit le nombre de fois où vous avez changé d'avis. Exemple : copiez A1:B2, où B1 contient **=A1*2**, et collez en F5. Choisissez **Valeurs** et G5 contient le nombre 2 au lieu de **=F5*2** ; choisissez **Conserver la mise en forme source** et la formule revient.

Au clavier, appuyez sur la touche **Ctrl** (la touche **Contrôle** sur un Mac) après un collage pour ouvrir le menu du bouton, puis utilisez les touches fléchées et **Entrée** pour choisir. Le bouton disparaît lorsque vous modifiez une cellule ou autre chose dans le classeur, copiez autre chose, appuyez sur **Échap**, ou faites défiler la dernière cellule collée hors de l'écran (il réapparaît lorsque vous revenez). Il n'est pas proposé après un couper-coller, ni pour du texte provenant d'un autre programme.

## Coller un lien, Coller en tant que lien hypertexte et Importation de texte

Trois autres choix se trouvent dans la flèche sous **Coller**. Ils fonctionnent sur les cellules copiées dans le même classeur, sauf le dernier.

- **Coller un lien** place dans chaque cellule une formule qui pointe vers la cellule copiée. Exemple : copiez A1:B2, sélectionnez D4 et utilisez **Coller un lien** : D4 devient **=Feuil1!$A$1**. Lorsque A1 change, D4 change avec lui. Une cellule copiée qui est vide affiche 0.
- **Coller en tant que lien hypertexte** place dans chaque cellule les mêmes mots que ceux qu'affiche la cellule copiée, et fait de chacun un lien vers la cellule copiée. Maintenez **Ctrl** et cliquez sur l'une d'elles pour aller à cette cellule. Cela fonctionne sur 2 000 cellules au maximum à la fois.
- **Importation de texte** sert pour du texte provenant d'un autre programme. Elle ouvre la même fenêtre que l'ouverture d'un fichier texte : vous indiquez ce qui sépare les morceaux (une virgule, une tabulation, un espace, un point-virgule, une barre verticale ou autre), ce qui entoure un morceau de guillemets, la première ligne à utiliser, et la façon de lire chaque colonne (telle quelle, comme texte, comme date, ou ignorée). Certains textes alignent leurs morceaux en colonnes de même largeur, séparées par des espaces. Cochez **Les colonnes ont une largeur fixe**, puis indiquez où commence chaque colonne : chaque ligne est coupée à ces endroits, et les espaces autour de chaque morceau sont supprimés. Choisissez *Texte* pour une colonne de codes comme 007, afin que les zéros soient conservés. Les morceaux sont placés dans les cellules, à partir de la cellule sélectionnée, en une seule étape que vous pouvez annuler.

## Texte avec virgules, points-virgules ou colonnes

Lorsque le texte que vous collez depuis un autre programme est nettement un tableau, JeffCrunch ouvre la même fenêtre que l'ouverture d'un fichier texte, afin que vous puissiez vérifier la façon dont il est découpé avant que quoi que ce soit n'entre dans les cellules. Cela ne se produit qu'avec un **Coller** simple (**{key:Paste}**), jamais avec **Collage spécial** ni l'un de ses choix. Cela se produit lorsque le texte comporte au moins deux lignes et aucune tabulation, et que soit chaque ligne compte le même nombre de morceaux, au moins trois, séparés par des virgules, des points-virgules ou des barres verticales, avec quelque chose dans chaque colonne (un morceau entre guillemets peut contenir une virgule) ; soit les morceaux sont alignés en colonnes de même largeur, avec les mêmes espaces entre eux sur chaque ligne (au moins trois lignes, et au moins deux espaces). Les nombres avec des virgules de milliers, comme 1,234, ainsi que les lignes de code ou les listes qui se terminent par une virgule ou un point-virgule ne sont pas considérés comme un tableau. La fenêtre commence avec la façon dont le texte a été reconnu. Si les nombres contiennent des virgules et que les morceaux sont séparés par des points-virgules, la virgule est le séparateur décimal. Cliquez sur **OK** pour placer les morceaux, ou sur **Annuler** pour ne rien placer.

Exemple : copiez ces lignes depuis un courriel.

    nom,quantité,prix
    Pomme,12,1.50
    Poire,3,0.25

Collez-les avec une troisième colonne, par exemple un prix, et la fenêtre affiche trois colonnes, avec **Virgule** coché. Une phrase sur une seule ligne, des lignes qui ne comptent pas le même nombre de virgules, ou des lignes de seulement deux morceaux sont collées comme du texte, sans fenêtre. Le texte comportant des tabulations va directement dans les cellules, comme auparavant. Pour ouvrir la fenêtre avec n'importe quel texte, choisissez **Importation de texte** (ci-dessus).

La même chose se produit lorsque vous faites glisser du texte depuis un autre programme et le déposez sur la grille : il est placé à partir de la cellule sélectionnée. Un dépôt ailleurs dans la fenêtre ne fait rien.

## Définir le collage par défaut

**Définir le collage par défaut** est le dernier choix dans la flèche sous **Coller**. Il détermine ce que le bouton **Coller** et **{key:Paste}** placent lorsque vous avez copié des cellules : tout (par défaut), ou seulement les valeurs, seulement les formules, seulement la mise en forme, etc. Exemple : si vous choisissez *Valeurs*, une formule copiée arrive sous forme du nombre qu'elle affiche. Ce choix est conservé pour la prochaine fois. Il ne modifie pas **Collage spécial** ni les choix du menu, et le texte provenant d'autres programmes est collé tel quel. Choisissez *Tout* pour revenir en arrière.

Ce même choix se trouve dans la page **Modification** de **Options**, dans deux zones : une pour les cellules copiées dans ce classeur, et une pour les cellules copiées dans un autre classeur. Tant que vous n'en choisissez pas un pour un autre classeur, il suit le premier.

## Le menu Coller et les images

La flèche sous **Coller** affiche les choix habituels, ainsi que **Sans bordures**, **Formules et mise en forme des nombres**, **Valeurs et mise en forme des nombres**, **Conserver les largeurs de colonnes source** et **Commentaires et notes**. Ce sont les mêmes choix que dans Collage spécial, en un seul clic chacun.

La flèche à côté de **Copier** propose **Copier comme image** et **Copier en tant que bitmap**. Chacun place sur le presse-papiers une image des cellules sélectionnées, dessinée telle que vous la voyez, avec le quadrillage. Un bitmap a la taille de l'écran. Une image comporte deux fois plus de points, elle reste donc nette lorsqu'on l'agrandit. **Copier comme image imprimée** dessine les cellules telles que le papier les montrerait : le quadrillage n'apparaît que si la mise en page prévoit de l'imprimer. Collez-la dans JeffWrite ou JeffDeck comme image. Si les cellules ne tiennent pas dans une seule image, sélectionnez moins de cellules.

## Entre programmes

Lorsque vous copiez, JeffCrunch place aussi les cellules dans le presse-papiers sous forme de texte (cellules séparées par des tabulations, une ligne par rangée) et sous forme de tableau pour les pages web, afin que les autres programmes puissent les utiliser. Lorsque vous collez quelque chose qui ne vient pas de JeffCrunch, du texte avec tabulations, ou un tableau provenant d'une page web ou d'un autre tableur, chaque morceau va dans sa propre cellule et est lu comme si vous l'aviez saisi : nombres, dates, et formules qui commencent par un signe égal. Copier depuis un autre classeur ouvert dans JeffCrunch reprend les valeurs, les formules et les formats de nombres, mais pas le reste de la mise en forme. Collées dans JeffDeck ou JeffPub, elles deviennent un tableau qui conserve les polices, les trames, les bordures et les nombres tels qu'ils s'affichent (voir **Coller les cellules sous forme de tableau** dans leur Aide).

## Ce qui ne peut pas être collé

JeffCrunch vous en informe et ne modifie rien lorsque le collage toucherait une partie d'une **formule matricielle**, la ligne des noms de colonnes d'un **tableau**, des cellules verrouillées d'une **feuille protégée**, au-delà de la fin de la feuille, ou plus de 100 000 cellules. Une **formule matricielle** entière peut être coupée et déplacée (sélectionnez toutes ses cellules) ; une partie de formule, un tableau de données ou la ligne des noms d'un tableau ne le peuvent pas.

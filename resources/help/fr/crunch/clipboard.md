# Couper, copier et coller

<!-- keywords: couper, copier, coller, presse-papiers, déplacer, dupliquer, collage spécial, valeurs, formules, formats, transposer, ignorer les cellules vides, additionner, soustraire, multiplier, diviser, opération, fusionnées, texte, tabulation, html, page web, autre programme, Échap, fourmis, bordure, largeurs de colonnes, notes, commentaires, formats de nombre, toutes sauf les bordures, image, bitmap, copier en tant qu'image, image imprimée, comme imprimé, coller le lien, lien hypertexte, importation de texte, texte délimité, déposer du texte, virgule, point-virgule, csv, séparateur, définir le collage par défaut, collage par défaut, largeur fixe -->

**Copier** (**{key:Copy}**) place une copie des cellules sélectionnées dans le presse-papiers, un espace où l'ordinateur conserve un élément à la fois. **Coller** (**{key:Paste}**) place le contenu du presse-papiers dans les cellules à partir de la cellule sélectionnée. **Couper** (**{key:Cut}**) fonctionne comme copier, mais au moment du collage, les cellules sont déplacées et leur ancien emplacement est vidé.

Un cadre en pointillés animé indique les cellules copiées ou coupées. Appuyez sur **Échap** pour faire disparaître ce cadre ; les cellules ne sont alors plus prêtes à être collées. Les cellules coupées sont aussi libérées si vous effectuez une autre modification avant de les coller (saisie, insertion, annulation) : coupez-les à nouveau si vous voulez toujours les déplacer.

## Coller dans JeffCrunch

Tout est repris : les valeurs, les formules, les formats et les cellules fusionnées. Une formule continue de pointer vers les mêmes emplacements *par rapport à sa position*. Exemple : copiez **=A1*2** depuis B1 et collez-la en B5, elle devient **=A5*2**. Une partie précédée d'un signe dollar, comme **$A$1**, reste inchangée. Si vous sélectionnez une plage dont le nombre de cellules est un multiple exact de celui de la copie, la copie est répétée pour la remplir : copiez une cellule, sélectionnez vingt cellules, collez, et les vingt reçoivent la valeur.

Un nombre collé trop large pour une colonne qui a encore sa largeur initiale élargit la colonne.

## Couper déplace les cellules

Lorsque vous coupez des cellules puis les collez, les cellules sont réellement déplacées. Chaque formule qui pointait vers elles pointe désormais vers leur nouvel emplacement. Exemple : B1 contient **=A1+A2**. Coupez A1:A2 et collez en E4 : B1 devient **=E4+E5**. Le contenu des cellules écrasées par le collage est perdu (**Annuler** le restaure), et une formule qui lisait l'une de ces cellules affiche **#REF!**, car la cellule qu'elle lisait n'existe plus. Si les cellules coupées ne forment qu'une partie d'une cellule fusionnée, ou si vous ne collez que sur une partie d'une cellule fusionnée, rien n'est déplacé : sélectionnez la cellule fusionnée entière. Lorsque vous coupez des cellules pour les coller sur une autre feuille, les formules continuent de lire les cellules qu'elles lisaient auparavant (en indiquant la feuille). Le presse-papiers est vidé après un couper-coller.

## Collage spécial

**Ctrl+Alt+V**, ou la flèche sous **Coller** dans l'onglet Accueil, vous permet de choisir ce que vous collez :

- **Tout**, **Formules**, **Valeurs** ou **Formats**. *Valeurs* remplace une formule par le nombre ou le texte qu'elle affiche.
- **Toutes sauf les bordures** : tout, mais les cellules conservent les bordures qu'elles avaient.
- **Formules et formats de nombre**, ou **Valeurs et formats de nombre** : les formules (ou les valeurs) sont accompagnées de la manière dont les nombres sont affichés, comme le pourcentage ou une date, mais pas des couleurs, des polices ni des bordures. Les cellules conservent l'aspect qu'elles avaient.
- **Largeurs de colonnes** : les colonnes dans lesquelles vous collez prennent les largeurs des colonnes copiées.
- **Commentaires et notes** : les notes des cellules copiées sont placées sur les cellules dans lesquelles vous collez. Aucune valeur de cellule ne change.
- Une **opération** : **Additionner**, **Soustraire**, **Multiplier** ou **Diviser**. Les nombres copiés sont combinés avec les nombres déjà présents dans les cellules. Exemple : les cellules contiennent 10 et 20, vous avez copié 5 et choisi *Multiplier* : elles deviennent 50 et 100.
- **Ignorer les cellules vides** : les cellules vides de la copie laissent les cellules sur lesquelles elles tombent inchangées.
- **Transposer** : les lignes deviennent des colonnes, et les colonnes deviennent des lignes.

## Coller le lien, Coller en tant que lien hypertexte et Coller l'importation de texte

Trois autres choix se trouvent dans la flèche sous **Coller**. Ils s'appliquent aux cellules copiées dans le même classeur, sauf le dernier.

- **Coller le lien** place dans chaque cellule une formule qui pointe vers la cellule copiée. Exemple : copiez A1:B2, sélectionnez D4 et utilisez **Coller le lien** : D4 devient **=Feuil1!$A$1**. Lorsque A1 change, D4 change avec elle. Une cellule copiée vide affiche 0.
- **Coller en tant que lien hypertexte** place dans chaque cellule les mêmes mots que ceux qu'affiche la cellule copiée, et fait de chacun un lien vers la cellule copiée. Maintenez **Ctrl** et cliquez dessus pour aller à cette cellule. Cela fonctionne sur 2 000 cellules au maximum à la fois.
- **Coller l'importation de texte** sert à coller du texte provenant d'un autre programme. Elle ouvre la même fenêtre que l'ouverture d'un fichier texte : vous indiquez ce qui sépare les éléments (une virgule, une tabulation, un espace, un point-virgule, une barre verticale ou autre chose), ce qui encadre un élément, la première ligne à utiliser et la manière de lire chaque colonne (telle quelle, en texte, en date, ou ignorée). Certains textes alignent leurs éléments en colonnes de même largeur, séparées par des espaces. Cochez **Les colonnes ont une largeur fixe**, puis indiquez où commence chaque colonne : chaque ligne est coupée à ces endroits, et les espaces autour de chaque élément sont supprimés. Choisissez *Texte* pour une colonne de codes comme 007, afin que les zéros soient conservés. Les éléments sont placés dans les cellules à partir de la cellule sélectionnée, en une seule étape que vous pouvez annuler.

## Texte avec virgules, points-virgules ou colonnes

Lorsque le texte que vous collez depuis un autre programme est clairement un tableau, JeffCrunch ouvre la même fenêtre que pour l'ouverture d'un fichier texte, afin que vous vérifiiez la manière dont il est découpé avant que quoi que ce soit n'entre dans les cellules. Cela ne se produit qu'avec un **Coller** simple (**{key:Paste}**), jamais avec **Collage spécial** ni l'un de ses choix. Cela se produit lorsque le texte compte au moins deux lignes et aucune tabulation, et que soit chaque ligne comporte le même nombre d'éléments, au moins trois, séparés par des virgules, des points-virgules ou des barres verticales, avec quelque chose dans chaque colonne (un élément entre guillemets peut contenir une virgule) ; soit les éléments sont alignés en colonnes de même largeur, avec les mêmes espaces entre eux sur chaque ligne (au moins trois lignes et au moins deux intervalles). Les nombres avec des virgules de milliers, comme 1,234, ainsi que les lignes de code ou les listes qui se terminent par une virgule ou un point-virgule ne sont pas pris pour un tableau. La fenêtre s'ouvre avec la manière dont le texte a été reconnu. Si les nombres comportent des virgules et que les éléments sont séparés par des points-virgules, la virgule est le séparateur décimal. Cliquez sur **OK** pour insérer les éléments, ou sur **Annuler** pour ne rien insérer.

Exemple : copiez ces lignes depuis un courriel.

    name,qty,price
    Apple,12,1.50
    Pear,3,0.25

Si vous les collez avec une troisième colonne, comme un prix, la fenêtre affiche trois colonnes, avec **Virgule** coché. Une phrase sur une seule ligne, des lignes qui ne comportent pas le même nombre de virgules, ou seulement deux éléments par ligne, sont collées comme du texte, sans fenêtre. Le texte contenant des tabulations va directement dans les cellules, comme auparavant. Pour ouvrir la fenêtre pour n'importe quel texte, choisissez **Coller l'importation de texte** (ci-dessus).

Il en va de même lorsque vous faites glisser du texte depuis un autre programme et le déposez sur la grille : il entre à partir de la cellule sélectionnée. Un dépôt ailleurs dans la fenêtre ne fait rien.

## Définir le collage par défaut

**Définir le collage par défaut** est le dernier choix de la flèche sous **Coller**. Il détermine ce que le bouton **Coller** et **{key:Paste}** insèrent lorsque vous avez copié des cellules : tout (par défaut), ou seulement les valeurs, seulement les formules, seulement la mise en forme, etc. Exemple : choisissez *Valeurs*, et une formule copiée apparaît sous la forme du nombre qu'elle affiche. Ce choix est conservé pour la prochaine fois. Il ne modifie ni **Collage spécial** ni les choix du menu, et le texte provenant d'autres programmes est collé tel quel. Choisissez *Tout* pour revenir au réglage initial.

## Le menu Coller et les images

La flèche sous **Coller** affiche les choix habituels, ainsi que **Sans bordures**, **Formules et mise en forme des nombres**, **Valeurs et mise en forme des nombres**, **Conserver les largeurs de colonnes source** et **Commentaires et notes**. Ils sont identiques aux choix du Collage spécial, en un seul clic chacun.

La flèche à côté de **Copier** propose **Copier en tant qu'image** et **Copier en tant que bitmap**. Chacune place sur le presse-papiers une image des cellules sélectionnées, telles que vous les voyez, avec le quadrillage. Un bitmap a la taille de l'écran. Une image comporte deux fois plus de points, elle reste donc nette lorsqu'on l'agrandit. **Copier en tant qu'image imprimée** dessine les cellules telles que la feuille imprimée les montrerait : le quadrillage n'apparaît que si la mise en page prévoit de l'imprimer. Collez-la dans JeffWrite ou JeffDeck comme image. Si les cellules ne tiennent pas dans une seule image, sélectionnez-en moins.

## Entre programmes

Lorsque vous copiez, JeffCrunch place aussi les cellules sur le presse-papiers sous forme de texte (cellules séparées par des tabulations, une ligne par rangée) et sous forme de tableau pour les pages web, afin que d'autres programmes puissent les utiliser. Lorsque vous collez un contenu qui ne vient pas de JeffCrunch, du texte avec des tabulations ou un tableau provenant d'une page web ou d'une autre feuille de calcul, chaque élément va dans sa propre cellule et est interprété comme si vous l'aviez saisi : nombres, dates et formules commençant par un signe égal. Copier depuis un autre classeur ouvert dans JeffCrunch reprend les valeurs, les formules et les formats de nombre, mais pas le reste de la mise en forme.

## Ce qui ne peut pas être collé

JeffCrunch vous en avertit et ne modifie rien lorsque le collage toucherait une partie d'une **formule matricielle**, la ligne des noms de colonnes d'un **tableau**, des cellules verrouillées d'une **feuille protégée**, au-delà de la fin de la feuille, ou plus de 100 000 cellules. Une **formule matricielle** entière peut être coupée et déplacée (sélectionnez toutes ses cellules) ; une partie de formule, un tableau de données ou la ligne des noms d'un tableau ne le peuvent pas.

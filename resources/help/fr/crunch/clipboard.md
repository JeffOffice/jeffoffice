# Couper, copier et coller

<!-- keywords: couper, copier, coller, presse-papiers, déplacer, dupliquer, collage spécial, valeurs, formules, formats, transposer, ignorer les vides, additionner, soustraire, multiplier, diviser, opération, fusionnée, texte, tabulation, html, page web, autre programme, Échap, lignes pointillées, bordure, largeurs de colonnes, notes, commentaires, formats de nombres, toutes sauf les bordures, image, bitmap, copier comme image, image imprimée, comme imprimé, coller un lien, lien hypertexte, importation de texte, texte délimité, glisser le texte, virgule, point-virgule, csv, séparateur, définir le collage par défaut, collage par défaut, largeur fixe -->

**Copier** (**{key:Ctrl+C}**) place une copie des cellules sélectionnées dans le presse-papiers, un emplacement où l'ordinateur conserve un élément à la fois. **Coller** (**{key:Ctrl+V}**) place le contenu du presse-papiers dans les cellules, à partir de la cellule sélectionnée. **Couper** (**{key:Ctrl+X}**) fonctionne comme copier, mais lors du collage, les cellules sont déplacées et l'ancien emplacement est vidé.

Une bordure en pointillés animée indique les cellules copiées ou coupées. Appuyez sur **Échap** pour la faire disparaître ; les cellules ne sont alors plus prêtes à être collées. Les cellules coupées sont également libérées si vous effectuez une autre modification avant de les coller (saisie, insertion, annulation) : coupez-les à nouveau si vous voulez toujours les déplacer.

## Coller dans JeffCrunch

Tout est repris : les valeurs, les formules, les formats et les cellules fusionnées. Une formule continue de pointer vers les mêmes emplacements *par rapport à sa position*. Exemple : copiez **=A1*2** depuis B1 et collez-la en B5, elle devient **=A5*2**. Une référence précédée d'un signe dollar, comme **$A$1**, reste inchangée. Si vous sélectionnez une plage dont la taille est un multiple exact de celle de la zone copiée, la copie est répétée pour la remplir : copiez une cellule, sélectionnez vingt cellules, collez, et les vingt reçoivent le contenu.

Un nombre collé qui est trop large pour une colonne ayant encore sa largeur d'origine élargit la colonne.

## Couper déplace les cellules

Lorsque vous coupez des cellules puis les collez, elles sont réellement déplacées. Toute formule qui pointait vers elles pointe désormais vers leur nouvel emplacement. Exemple : B1 contient **=A1+A2**. Coupez A1:A2 et collez en E4 : B1 devient **=E4+E5**. Le contenu des cellules sur lesquelles vous collez est perdu (**Annuler** le restaure), et une formule qui lisait l'une de ces cellules affiche **#RÉF!**, car la cellule lue n'existe plus. Si les cellules coupées ne forment qu'une partie d'une cellule fusionnée, ou si vous collez sur une partie seulement d'une cellule fusionnée, rien n'est déplacé : sélectionnez la cellule fusionnée entière. Lorsque vous coupez des cellules vers une autre feuille, les formules continuent de lire les cellules qu'elles lisaient auparavant (la feuille est indiquée). Le presse-papiers est vidé après un couper-coller.

## Collage spécial

**Ctrl+Alt+V**, ou la flèche située sous **Coller** dans l'onglet Accueil, permet de choisir ce qui est collé :

- **Tout**, **Formules**, **Valeurs** ou **Formats**. *Valeurs* remplace une formule par le nombre ou le texte qu'elle affiche.
- **Tout sauf les bordures** : tout, mais les cellules conservent les bordures qu'elles avaient.
- **Formules et formats de nombres**, ou **Valeurs et formats de nombres** : les formules (ou les valeurs) sont accompagnées de la manière dont les nombres sont affichés, par exemple en pourcentage ou en date, mais sans les couleurs, les polices ni les bordures. Les cellules gardent leur apparence d'origine.
- **Largeurs de colonnes** : les colonnes dans lesquelles vous collez reçoivent les largeurs des colonnes copiées.
- **Commentaires et notes** : les notes des cellules copiées sont placées dans les cellules où vous collez. Aucune valeur de cellule ne change.
- Une **opération** : **Additionner**, **Soustraire**, **Multiplier** ou **Diviser**. Les nombres copiés sont combinés avec les nombres déjà présents dans les cellules. Exemple : les cellules contiennent 10 et 20, vous avez copié 5 et choisi *Multiplier* : elles deviennent 50 et 100.
- **Ignorer les vides** : les cellules vides de la copie laissent les cellules de destination telles quelles.
- **Transposer** : les lignes deviennent des colonnes et les colonnes deviennent des lignes.

## Coller un lien, Coller comme lien hypertexte et Importation de texte

Trois autres choix se trouvent dans la flèche située sous **Coller**. Ils s'appliquent aux cellules copiées dans le même classeur, sauf le dernier.

- **Coller le lien** place dans chaque cellule une formule qui pointe vers la cellule copiée. Exemple : copiez A1:B2, sélectionnez D4 et utilisez **Coller le lien** : D4 devient **=Feuil1!$A$1**. Quand A1 change, D4 change avec elle. Une cellule copiée vide affiche 0.
- **Coller comme lien hypertexte** place dans chaque cellule le même texte que celui qu'affiche la cellule copiée, et fait de chacun un lien vers la cellule copiée. Maintenez **Ctrl** et cliquez dessus pour aller à cette cellule. Cette fonction s'applique à 2 000 cellules au maximum à la fois.
- **Importation de texte** sert à coller du texte provenant d'un autre programme. Elle ouvre la même fenêtre que l'ouverture d'un fichier texte : vous indiquez ce qui sépare les éléments (une virgule, une tabulation, une espace, un point-virgule, une barre verticale ou un autre caractère), ce qui encadre un élément, la première ligne à utiliser, et la manière de lire chaque colonne (telle quelle, en texte, en date, ou ignorée). Certains textes alignent leurs éléments en colonnes de même largeur, séparées par des espaces. Cochez **Les colonnes ont une largeur fixe**, puis indiquez où commence chaque colonne : chaque ligne est coupée à ces endroits, et les espaces autour de chaque élément sont supprimés. Choisissez *Texte* pour une colonne de codes comme 007, afin de conserver les zéros. Les éléments sont placés dans les cellules, à partir de la cellule sélectionnée, en une seule étape que vous pouvez annuler.

## Texte avec virgules, points-virgules ou colonnes

Lorsque le texte que vous collez depuis un autre programme est nettement un tableau, JeffCrunch ouvre la même fenêtre que pour l'ouverture d'un fichier texte, afin que vous puissiez vérifier le découpage avant que quoi que ce soit n'entre dans les cellules. Cela ne se produit qu'avec un **Coller** simple (**{key:Ctrl+V}**), jamais avec le **Collage spécial** ni l'un de ses choix. Cela se produit lorsque le texte compte au moins deux lignes et aucune tabulation, et que soit chaque ligne comporte le même nombre d'éléments, au moins trois, séparés par des virgules, des points-virgules ou des barres verticales, avec un contenu dans chaque colonne (un élément entre guillemets peut contenir une virgule) ; soit les éléments sont alignés en colonnes de même largeur, avec les mêmes espaces entre eux sur chaque ligne (au moins trois lignes et au moins deux espaces). Les nombres avec séparateur des milliers, comme 1,234, ainsi que les lignes de code ou les listes qui se terminent par une virgule ou un point-virgule, ne sont pas considérés comme un tableau. La fenêtre s'ouvre avec le découpage détecté pour le texte. Si les nombres contiennent des virgules et que les éléments sont séparés par des points-virgules, la virgule est le séparateur décimal. Cliquez sur **OK** pour insérer les éléments, ou sur **Annuler** pour ne rien insérer.

Exemple : copiez ces lignes depuis un courriel.

    nom,qté,prix
    Pomme,12,1.50
    Poire,3,0.25

Si vous les collez avec une troisième colonne, par exemple un prix, la fenêtre affiche trois colonnes, avec **Virgule** coché. Une seule phrase sur une ligne, des lignes qui ne comportent pas le même nombre de virgules, ou seulement deux éléments par ligne, sont collées comme du texte, sans fenêtre. Le texte avec tabulations est placé directement dans les cellules, comme auparavant. Pour ouvrir la fenêtre pour n'importe quel texte, choisissez **Importation de texte** (ci-dessus).

La même chose se produit lorsque vous faites glisser du texte depuis un autre programme et le déposez sur la grille : il est inséré à partir de la cellule sélectionnée. Un dépôt ailleurs dans la fenêtre ne produit aucun effet.

## Définir le collage par défaut

**Définir le collage par défaut** est le dernier choix de la flèche située sous **Coller**. Il détermine ce que le bouton **Coller** et **{key:Ctrl+V}** insèrent lorsque vous avez copié des cellules : tout (par défaut), ou seulement les valeurs, seulement les formules, seulement la mise en forme, etc. Exemple : si vous choisissez *Valeurs*, une formule copiée est collée sous forme du nombre qu'elle affiche. Ce choix est conservé pour la prochaine fois. Il ne modifie ni le **Collage spécial** ni les choix du menu, et le texte provenant d'autres programmes est collé tel quel. Choisissez *Tout* pour revenir à la situation initiale.

## Le menu Coller et les images

La flèche située sous **Coller** affiche les choix habituels, ainsi que **Sans bordures**, **Formules et mise en forme des nombres**, **Valeurs et mise en forme des nombres**, **Conserver les largeurs de colonnes source**, et **Commentaires et notes**. Ils correspondent aux choix du Collage spécial, en un clic chacun.

La flèche située à côté de **Copier** propose **Copier comme image** et **Copier comme bitmap**. Chacun place sur le presse-papiers une image des cellules sélectionnées, telle qu'elle apparaît à l'écran, avec le quadrillage. Un bitmap a la taille de l'écran. Une image comporte deux fois plus de points, elle reste donc nette lorsqu'elle est agrandie. **Copier comme image imprimée** dessine les cellules telles que le papier les affichera : le quadrillage n'apparaît que si la mise en page prévoit de l'imprimer. Collez-la dans JeffWrite ou JeffDeck en tant qu'image. Si les cellules ne tiennent pas dans une seule image, sélectionnez moins de cellules.

## Entre programmes

Lorsque vous copiez, JeffCrunch place aussi les cellules sur le presse-papiers sous forme de texte (cellules séparées par des tabulations, une ligne par rangée) et sous forme de tableau pour les pages web, afin que d'autres programmes puissent les utiliser. Lorsque vous collez un contenu qui ne provient pas de JeffCrunch, du texte avec tabulations ou un tableau issu d'une page web ou d'un autre tableur, chaque élément est placé dans sa propre cellule et interprété comme si vous l'aviez saisi : nombres, dates et formules commençant par un signe égal. La copie depuis un autre classeur ouvert dans JeffCrunch apporte les valeurs, les formules et les formats de nombres, mais pas le reste de la mise en forme. Collés dans JeffDeck ou JeffPub, ils deviennent un tableau qui conserve les polices, les trames, les bordures et les nombres tels qu'ils s'affichent (voir **Coller les cellules sous forme de tableau** dans leur aide).

## Ce qui ne peut pas être collé

JeffCrunch vous en avertit et ne modifie rien lorsque le collage toucherait une partie d'une **formule matricielle**, la ligne des noms de colonnes d'un **tableau**, des cellules verrouillées d'une **feuille protégée**, au-delà de la fin de la feuille, ou plus de 100 000 cellules. Une **formule matricielle** entière peut être coupée et déplacée (sélectionnez toutes ses cellules) ; une partie de celle-ci, un tableau de données ou la ligne des noms d'un tableau ne le peuvent pas.

# Couper, copier et coller

<!-- keywords: couper, copier, coller, presse-papiers, déplacer, dupliquer, collage spécial, valeurs, formules, formats, transposer, ignorer les blancs, additionner, soustraire, multiplier, diviser, opération, fusionnée, texte, tabulation, html, page web, autre programme, Échap, marche, bordure, largeurs de colonnes, notes, commentaires, formats de nombres, toutes sauf les bordures, image, bitmap, copier en tant qu'image, image imprimée, tel qu'imprimé, coller un lien, lien hypertexte, importation de texte, texte délimité, déposer du texte, virgule, point-virgule, csv, séparateur, définir le collage par défaut, collage par défaut, largeur fixe -->

**Copier** (**{key:Copy}**) place une copie des cellules sélectionnées dans le presse-papiers, un emplacement où l'ordinateur conserve un élément à la fois. **Coller** (**{key:Paste}**) place le contenu du presse-papiers dans les cellules, à partir de la cellule sélectionnée. **Couper** (**{key:Cut}**) fonctionne comme copier, mais au moment du collage, les cellules sont déplacées et leur emplacement d'origine est vidé.

Un cadre en pointillés animé indique les cellules copiées ou coupées. Appuyez sur **Échap** pour le faire disparaître ; les cellules ne sont alors plus prêtes à être collées. Les cellules coupées sont aussi libérées si vous effectuez une autre modification avant de les coller (saisie, insertion, annulation) : coupez-les à nouveau si vous voulez toujours les déplacer.

## Coller dans JeffCrunch

Tout est repris : les valeurs, les formules, les formats et les cellules fusionnées. Une formule continue de pointer vers les mêmes emplacements *par rapport à sa position*. Exemple : copiez **=A1*2** depuis B1 et collez-la en B5, elle devient **=A5*2**. Une référence précédée d'un signe dollar, comme **$A$1**, reste inchangée. Si vous sélectionnez une plage dont la taille est un multiple exact de celle de ce que vous avez copié, la copie est répétée pour la remplir : copiez une cellule, sélectionnez vingt cellules, collez, et les vingt reçoivent le contenu.

Un nombre collé trop large pour une colonne qui a encore sa largeur d'origine élargit la colonne.

## Couper déplace les cellules

Lorsque vous coupez des cellules puis les collez, elles sont réellement déplacées. Chaque formule qui pointait vers elles pointe désormais vers leur nouvel emplacement. Exemple : B1 contient **=A1+A2**. Coupez A1:A2 et collez en E4 : B1 devient **=E4+E5**. Le contenu des cellules recouvertes par le collage disparaît (**Annuler** le restaure), et une formule qui lisait l'une de ces cellules affiche **#REF!**, car la cellule qu'elle lisait n'existe plus. Si les cellules coupées ne forment qu'une partie d'une cellule fusionnée, ou si vous collez sur une partie seulement d'une cellule fusionnée, rien n'est déplacé : sélectionnez la cellule fusionnée entière. Lorsque vous coupez des cellules vers une autre feuille, les formules continuent de lire les cellules qu'elles lisaient auparavant (elles indiquent la feuille). Le presse-papiers est vidé après un couper-coller.

## Collage spécial

**Ctrl+Alt+V**, ou la flèche sous **Coller** dans l'onglet **Accueil**, permet de choisir ce qui est collé :

- **Tout**, **Formules**, **Valeurs** ou **Formats**. *Valeurs* transforme une formule en le nombre ou le texte qu'elle affiche.
- **Tout sauf les bordures** : tout, mais les cellules conservent les bordures qu'elles avaient.
- **Formules et formats de nombres**, ou **Valeurs et formats de nombres** : les formules (ou les valeurs) sont accompagnées de la manière dont les nombres sont affichés, comme le pourcentage ou une date, mais sans les couleurs, les polices ni les bordures. Les cellules conservent l'aspect qu'elles avaient.
- **Largeurs de colonnes** : les colonnes dans lesquelles vous collez reçoivent les largeurs des colonnes copiées.
- **Commentaires et notes** : les notes des cellules copiées sont placées dans les cellules où vous collez. Aucune valeur de cellule ne change.
- Une **opération** : **Additionner**, **Soustraire**, **Multiplier** ou **Diviser**. Les nombres copiés sont combinés aux nombres déjà présents dans les cellules. Exemple : les cellules contiennent 10 et 20, vous avez copié 5 et choisi *Multiplier* : elles deviennent 50 et 100.
- **Ignorer les blancs** : les cellules vides de la copie laissent intactes les cellules sur lesquelles elles tombent.
- **Transposer** : les lignes deviennent des colonnes et les colonnes deviennent des lignes.

## Le bouton Options de collage

Juste après avoir collé des cellules copiées dans JeffCrunch, un petit bouton **Options de collage** apparaît à côté de la dernière cellule collée. Cliquez dessus pour choisir une autre manière d'insérer les mêmes cellules : **Conserver la mise en forme source** (tout), **Valeurs**, **Formules**, **Mise en forme**, **Sans bordures**, **Formules et mise en forme des nombres**, **Valeurs et mise en forme des nombres**, **Conserver les largeurs de colonnes source**, ou **Transposer**. JeffCrunch annule le collage et le refait de cette manière. La modification ne compte que pour une étape : un seul **Annuler** annule tout le collage, quel que soit le nombre de fois où vous avez changé d'avis. Exemple : copiez A1:B2, où B1 contient **=A1*2**, et collez en F5. Choisissez **Valeurs** : G5 contient le nombre 2 au lieu de **=F5*2** ; choisissez **Conserver la mise en forme source** : la formule revient.

Au clavier, appuyez sur la touche **Ctrl** (la touche **Contrôle** sur un Mac) après un collage pour ouvrir le menu du bouton, puis utilisez les touches fléchées et **Entrée** pour choisir. Le bouton disparaît lorsque vous modifiez une cellule ou tout autre élément du classeur, lorsque vous copiez autre chose, lorsque vous appuyez sur **Échap**, ou lorsque la dernière cellule collée sort de l'écran (il réapparaît si vous revenez à elle). Il n'est pas proposé après un couper-coller, ni pour du texte provenant d'un autre programme.

## Coller un lien, Coller en tant qu'hyperlien et Importation de texte

Trois autres choix se trouvent dans la flèche sous **Coller**. Ils fonctionnent sur les cellules copiées dans le même classeur, sauf le dernier.

- **Coller le lien** place dans chaque cellule une formule qui pointe vers la cellule copiée. Exemple : copiez A1:B2, sélectionnez D4 et utilisez **Coller le lien** : D4 devient **=Feuil1!$A$1**. Lorsque A1 change, D4 change avec elle. Une cellule copiée vide affiche 0.
- **Coller en tant qu'hyperlien** place dans chaque cellule le même texte que celui qu'affiche la cellule copiée, et en fait un lien vers cette cellule. Maintenez **Ctrl** et cliquez sur l'une d'elles pour aller à cette cellule. Cela fonctionne sur 2 000 cellules au maximum à la fois.
- **Importation de texte** sert pour le texte provenant d'un autre programme. Elle ouvre la même fenêtre que l'ouverture d'un fichier texte : vous indiquez ce qui sépare les éléments (une virgule, une tabulation, un espace, un point-virgule, une barre verticale ou autre), le caractère qui encadre un élément, la première ligne à utiliser, et la manière de lire chaque colonne (telle quelle, en texte, en date, ou ignorée). Certains textes alignent leurs éléments en colonnes de même largeur, séparées par des espaces. Cochez **Les colonnes ont une largeur fixe**, puis indiquez où commence chaque colonne : chaque ligne est coupée à ces emplacements, et les espaces entourant chaque élément sont supprimés. Choisissez *Texte* pour une colonne de codes tels que 007, afin de conserver les zéros. Les éléments sont placés dans les cellules, à partir de la cellule sélectionnée, en une seule étape que vous pouvez annuler.

## Texte avec virgules, points-virgules ou colonnes

Lorsque le texte que vous collez depuis un autre programme est clairement un tableau, JeffCrunch ouvre la même fenêtre que l'ouverture d'un fichier texte, afin que vous puissiez vérifier la façon dont il est découpé avant que quoi que ce soit n'entre dans les cellules. Cela ne se produit qu'avec un **Coller** simple (**{key:Paste}**), jamais avec **Collage spécial** ni l'un de ses choix. Cela se produit lorsque le texte comporte au moins deux lignes et aucune tabulation, et que soit chaque ligne a le même nombre d'éléments, au moins trois, séparés par des virgules, des points-virgules ou des barres verticales, sans case vide dans une colonne (un élément entre guillemets peut contenir une virgule) ; soit les éléments sont alignés en colonnes de même largeur, avec les mêmes espaces entre eux sur chaque ligne (au moins trois lignes et au moins deux intervalles). Les nombres avec séparateurs de milliers, comme 1,234, ainsi que les lignes de code ou les listes qui se terminent par une virgule ou un point-virgule, ne sont pas considérés comme un tableau. La fenêtre s'ouvre avec la séparation détectée dans le texte. Si les nombres contiennent des virgules et que les éléments sont séparés par des points-virgules, la virgule est le séparateur décimal. Cliquez sur **OK** pour insérer les éléments, ou sur **Annuler** pour ne rien insérer.

Exemple : copiez ces lignes depuis un courriel.

    nom,qté,prix
    Pomme,12,1.50
    Poire,3,0.25

Si vous les collez avec une troisième colonne, par exemple un prix, la fenêtre affiche trois colonnes, avec **Virgule** coché. Une phrase sur une seule ligne, des lignes qui comportent un nombre différent de virgules, ou seulement deux éléments par ligne, sont collées comme du texte, sans fenêtre. Le texte avec tabulations passe directement dans les cellules, comme auparavant. Pour ouvrir la fenêtre avec n'importe quel texte, choisissez **Importation de texte** (voir ci-dessus).

La même chose se produit lorsque vous faites glisser du texte depuis un autre programme et le déposez sur la grille : il est inséré à partir de la cellule sélectionnée. Un dépôt ailleurs dans la fenêtre ne fait rien.

## Définir le collage par défaut

**Définir le collage par défaut** est le dernier choix de la flèche sous **Coller**. Il détermine ce que le bouton **Coller** et **{key:Paste}** insèrent lorsque vous avez copié des cellules : tout (par défaut), ou seulement les valeurs, seulement les formules, seulement la mise en forme, et ainsi de suite. Exemple : si vous choisissez *Valeurs*, une formule copiée arrive sous forme du nombre qu'elle affiche. Ce choix est conservé pour la prochaine fois. Il ne modifie ni **Collage spécial** ni les choix du menu, et le texte provenant d'autres programmes est collé tel quel. Choisissez *Tout* pour revenir en arrière.

Le même choix se trouve dans la page **Modification** des **Options**, dans deux listes : l'une pour les cellules copiées dans ce classeur, l'autre pour les cellules copiées dans un autre classeur. Tant que vous n'en choisissez pas un pour un autre classeur, il suit le premier.

## Le menu Coller et les images

La flèche sous **Coller** affiche les choix habituels, ainsi que **Sans bordures**, **Formules et mise en forme des nombres**, **Valeurs et mise en forme des nombres**, **Conserver les largeurs de colonnes source**, et **Commentaires et notes**. Ils sont identiques aux choix du collage spécial, en un seul clic chacun.

La flèche à côté de **Copier** propose **Copier en tant qu'image** et **Copier en tant que bitmap**. Chacune place sur le presse-papiers une image des cellules sélectionnées, telle que vous la voyez, avec le quadrillage. Un bitmap a la taille de l'écran. Une image contient deux fois plus de points, elle reste donc nette lorsqu'on l'agrandit. **Copier comme image imprimée** dessine les cellules telles que le papier les montrerait : le quadrillage n'apparaît que si la mise en page l'imprime. Collez-la dans JeffWrite ou JeffDeck comme une image. Si les cellules ne tiennent pas dans une seule image, sélectionnez moins de cellules.

## Entre programmes

Lorsque vous copiez, JeffCrunch place aussi les cellules dans le presse-papiers sous forme de texte (cellules séparées par des tabulations, une ligne par rangée) et sous forme de tableau pour les pages web, afin que les autres programmes puissent les récupérer. Lorsque vous collez quelque chose qui ne provient pas de JeffCrunch, du texte avec tabulations, ou un tableau issu d'une page web ou d'un autre tableur, chaque élément va dans sa propre cellule et est interprété comme si vous l'aviez saisi : nombres, dates et formules commençant par un signe égal. La copie depuis un autre classeur ouvert dans JeffCrunch apporte les valeurs, les formules et les formats de nombres, mais pas le reste de la mise en forme. Collez-les dans JeffDeck ou JeffPub : ils deviennent un tableau qui conserve les polices, les trames, les bordures et les nombres tels qu'ils s'affichent (voir **Coller les cellules en tant que tableau** dans leur aide).

## Ce qui ne peut pas être collé

JeffCrunch vous en informe et ne modifie rien lorsque le collage empiéterait sur une partie d'une **formule matricielle**, sur la ligne des noms de colonnes d'un **tableau**, sur des cellules verrouillées d'une **feuille protégée**, au-delà de la fin de la feuille, ou sur plus de 100 000 cellules. Une **formule matricielle** entière peut être coupée et déplacée (sélectionnez toutes ses cellules) ; une partie de celle-ci, un tableau de données ou la ligne des noms d'un tableau ne le peuvent pas.

## Le volet Presse-papiers

Le volet Presse-papiers conserve les 24 derniers éléments copiés ou coupés dans le programme de cette fenêtre, afin que vous puissiez coller un élément plus ancien sans le copier à nouveau. Cliquez sur la petite flèche dans l'angle du groupe **Presse-papiers** de l'onglet **Accueil** (ou appuyez sur **Alt**, puis **H**, puis **FO**) pour l'ouvrir. Un panneau apparaît à côté de la page.

Chaque élément affiche un petit aperçu et le type de contenu : texte, texte mis en forme, cellules, image, formes ou diapositives. Le plus récent est en haut. Cliquez sur un élément pour le coller à l'emplacement du curseur ou de la sélection. Il se colle comme tout autre collage : une étape d'**Annuler** le retire, et le bouton Options de collage apparaît comme d'habitude.

- **Tout coller** colle tous les éléments, du plus ancien au plus récent, afin qu'ils suivent l'ordre dans lequel vous les avez copiés.
- **Supprimer** retire l'élément choisi. **Tout effacer** les retire tous.
- Le volet ne collecte des éléments que lorsqu'il est ouvert. Pour continuer la collecte après l'avoir fermé, cochez **Continuer la collecte lorsque ce volet est fermé**. Ce choix est mémorisé, et la collecte commence au démarrage du programme, avant que vous n'ouvriez le volet.
- Pour collecter aussi ce que vous copiez dans d'autres programmes, cochez **Collecter aussi ce qui est copié dans d'autres programmes**. Seuls le texte brut et les images sont conservés, et cette option est désactivée à chaque ouverture du programme. Lorsqu'elle est active, une copie faite dans un autre programme est lue intégralement avant que ce programme décide de la conserver ou non ; une copie très volumineuse peut donc faire marquer une pause au programme et utiliser autant de mémoire que la copie est grande. Une seule forme de la copie est lue : son texte lorsqu'elle en contient, sinon une seule image. Lorsque l'option est désactivée, rien de ce qui est copié dans d'autres programmes n'est lu.

Les éléments ne sont conservés que dans la mémoire du programme. Rien n'est enregistré dans un fichier, et la liste est vidée lorsque vous fermez la dernière fenêtre du programme. Un élément reste dans la liste après la fermeture du document dont il provient. Un élément de plus de 8 Mo n'est pas conservé, et le volet vous en informe ; une image compte 4 octets par point (pixel), donc une image de plus d'environ 2 millions de pixels est trop grande. L'ensemble des éléments est limité à 64 Mo, et les plus anciens sont retirés en premier. Tout ce qu'un gestionnaire de mots de passe marque comme secret n'est jamais conservé.

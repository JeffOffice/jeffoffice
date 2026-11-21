# Utiliser JeffCrunch au clavier
<!-- keywords: clavier, raccourcis, touches, raccourcis clavier, touches de direction, ctrl, maj, début, fin, page précédente, page suivante, tab, tabulation, entrée, tout sélectionner, copier, F9, F1, zoom, touches d'accès, info-touches, personnaliser le clavier, modifier les raccourcis, attribuer des touches, réaffecter des touches, réinitialiser les touches -->

L'onglet **Aide** contient **Aide** (F1), **Raccourcis clavier**, qui ouvre cette page, **Nouveautés** et **Commentaires**, qui ouvrent leurs pages web.

Sur Mac, la touche **Commande** (⌘) remplace **Ctrl** dans les tableaux ci-dessous, sauf indication contraire dans un tableau.

## Déplacement et sélection

| Touches | Action |
| --- | --- |
| Touches de direction | Déplacent la cellule active d'une cellule, en ignorant les lignes et colonnes masquées |
| Maj+direction | Étend la sélection d'une cellule |
| Ctrl+direction | Va au bord des données (Échap arrête une longue recherche) |
| Ctrl+Maj+direction | Étend la sélection jusqu'au bord des données |
| Fin, puis une direction | Va au bord des données (la barre d'état affiche Fin) |
| Ctrl+A | Sélectionne la plage autour de la cellule active, puis la feuille entière (la feuille entière directement si la sélection est déjà plus grande que la plage) |
| Ctrl+Maj+* | Sélectionne la plage autour de la cellule active |
| Ctrl+clic ou Ctrl+glisser | Ajoute une plage à la sélection |
| Ctrl+Maj+Entrée | Saisit la formule comme formule matricielle sur les cellules sélectionnées |
| Début | Va à la première colonne de la ligne |
| Ctrl+Début | Va à la première cellule (A1, ou à la première cellule visible lorsque des lignes ou colonnes sont figées) |
| Ctrl+Fin | Va à la dernière cellule contenant quelque chose ou ayant une mise en forme |
| Page précédente, Page suivante | Monte ou descend d'un écran |
| Tab, Maj+Tab | Va à droite ou à gauche (dans les cellules sélectionnées, s'il y en a plusieurs) |
| Entrée, Maj+Entrée | Descend ou monte (dans les cellules sélectionnées, s'il y en a plusieurs) |
| Ctrl+Espace | Sélectionne la colonne entière |
| Maj+Espace | Sélectionne la ligne entière |
| {key:SelectAll} | Sélectionne tout |

## Modifier les cellules

| Touches | Action |
| --- | --- |
| Tout caractère | Commence une saisie dans la cellule active, à la place de son contenu |
| F2 | Modifie le contenu de la cellule active, dans la cellule ; pendant la saisie d'une formule, bascule entre la sélection de cellules et la modification du texte |
| F4 | Pendant la saisie d'une formule, place le signe $ devant les parties de l'adresse au curseur (A1, $A$1, A$1, $A1) |
| Entrée, Maj+Entrée | Valide la saisie, et descend ou monte |
| Tab, Maj+Tab | Valide la saisie, et va à droite ou à gauche |
| Échap | Annule la saisie |
| Alt+Entrée | Insère un saut de ligne dans la cellule |
| Suppr | Efface les cellules sélectionnées |
| {key:Undo}, {key:Redo} | Annule, rétablit |
| F4 | Répète la dernière commande sur les cellules sélectionnées (Ctrl+Y fait de même lorsqu'il n'y a rien à rétablir) |
| {key:Save} | Enregistre |
| {key:Cut}, {key:Copy}, {key:Paste} | Couper, copier, coller (Échap supprime le contour des cellules copiées) |
| Ctrl+Alt+V | Collage spécial |
| Ctrl et la touche plus, Ctrl+Maj et la touche plus, Ctrl et la touche moins | Insère ou supprime (des lignes ou des colonnes si des lignes ou colonnes entières sont sélectionnées ; les touches plus insèrent les cellules coupées, ou copiées sans modification depuis, et sinon des cellules vides) |
| Touches de direction, Maj+direction, Ctrl+direction (Option+direction sur Mac), Entrée, Échap | Après Déplacer ou redimensionner la note : déplace la zone de la note, la redimensionne, règle le pas à 1 pixel, valide ou annule |
| Ctrl+D, Ctrl+R | Recopie vers le bas, recopie vers la droite |
| Ctrl+E | Remplissage instantané |
| Alt et le signe égal | Somme |
| Ctrl+0, Ctrl+9 | Masque les colonnes, masque les lignes |
| Alt+Bas | Ouvre la liste d'une cellule qui en possède une |
| {key:Find} | Rechercher |
| Ctrl+H (sur Mac, Option+Commande+F) | Rechercher et remplacer |
| F5, Ctrl+G | Atteindre une cellule, une plage ou un nom |
| Ctrl+Maj+L | Active ou désactive le filtre |
| Maj+F3 | Choisir une fonction |
| Ctrl+F3 | La liste des noms |
| F9, Maj+F9 | Recalcule tout, recalcule cette feuille |
| Tab, Entrée | Pendant la saisie d'une formule : retient la fonction ou le nom sélectionné dans la liste |
| Échap | Arrête une recherche, un tri ou un filtre en cours |

## Choisir vos propres touches de raccourci

Vous pouvez modifier les touches de n'importe quelle commande, ou en attribuer à une commande qui n'en a pas. Cliquez sur **Fichier > Options**, puis sur l'onglet **Personnaliser**.

1. Saisissez dans la zone **Rechercher** pour trouver une commande par son nom ou par ses touches. La liste est regroupée par onglets du ruban, et chaque commande affiche ses touches. Les commandes absentes du ruban figurent sous **Autres**.
2. Cliquez sur la commande. Ses touches apparaissent sous **Touches de cette commande**.
3. Cliquez dans la zone **Appuyez sur les nouvelles touches**, puis appuyez sur les touches souhaitées, par exemple **Ctrl+Maj+B**. Cliquez ensuite sur **Attribuer**. Une commande peut avoir jusqu'à quatre touches.
4. Pour supprimer une touche, sélectionnez-la dans la liste, puis cliquez sur **Supprimer**.

Si une autre commande utilise déjà les touches saisies, JeffCrunch indique laquelle. Cliquez sur **Réaffecter** pour attribuer les touches à cette commande ; l'autre commande les perd. Rien ne change tant que vous n'avez pas cliqué sur **OK**. Ensuite, toutes les fenêtres ouvertes de JeffCrunch utilisent vos touches, ainsi que toutes les fenêtres que vous ouvrirez plus tard. Les info-bulles, les menus et le tableau des raccourcis ci-dessous affichent vos touches.

Un symbole peut également avoir des touches. Voir **Insérer un symbole** dans [Saisir du texte dans les cellules](edit-cells) : cliquez sur **Raccourci clavier** dans la boîte Symbole. Il apparaît alors dans cette liste comme une commande.

**Rétablir une touche** remet les touches habituelles de la commande choisie. **Tout rétablir** remet les touches habituelles de toutes les commandes. JeffCrunch ne conserve que les commandes dont vous avez modifié les touches : une commande que vous n'avez pas modifiée reçoit les meilleures touches qu'une version ultérieure pourrait lui attribuer.

Certaines touches ne peuvent pas être attribuées à une commande, car le système ou la saisie en a besoin. JeffCrunch affiche **réservée** lorsque vous en appuyez une. Les touches réservées sont **Ctrl+C**, **Ctrl+X**, **Ctrl+V**, **Ctrl+Z**, **Ctrl+A**, **Tab**, **Maj+Tab**, **Entrée**, **Échap**, **Retour arrière**, **Suppr**, **Alt+F4**, les touches fléchées, **Début**, **Fin**, **Page précédente**, **Page suivante** et **Espace**, **F1**, **F6**, **Maj+F6**, **Ctrl+Y**, **Ctrl+Maj+Z**, **Ctrl** avec une touche fléchée, **Début**, **Fin**, **Retour arrière** ou **Suppr** (avec ou sans **Maj**, lorsque cela a un sens dans une zone de texte), ainsi qu'une lettre, un chiffre ou un symbole pressé seul ou avec **Maj**. Une touche déjà utilisée par une autre partie de la fenêtre est également réservée. **Enregistrer**, **Annuler** et **Rétablir** conservent toujours au moins une touche. Sur Mac, **Ctrl** désigne ici la touche **Commande**.

## Commandes

Ce tableau affiche les touches que vous avez choisies.

<!-- shortcuts -->

## Touches d'accès

Appuyez sur **Alt** pour afficher une lettre sur chaque onglet et chaque commande du ruban, puis appuyez sur les lettres correspondantes. Appuyez sur **Alt**, puis sur **F**, pour accéder à l'écran Fichier. Appuyez sur **Échap** pour revenir en arrière.

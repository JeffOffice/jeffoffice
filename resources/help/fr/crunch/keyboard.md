# Utiliser JeffCrunch au clavier
<!-- keywords: clavier, raccourcis, touches, raccourcis clavier, flèche, ctrl, maj, début, fin, page précédente, page suivante, tabulation, entrée, tout sélectionner, copier, F9, F1, zoom, touches d'accès, info-touches, personnaliser le clavier, modifier les touches de raccourci, réaffecter les touches, rétablir les touches -->

L'onglet **Aide** contient **Aide** (F1), **Raccourcis clavier**, qui ouvre cette page, **Nouveautés** et **Commentaires**, qui ouvrent leurs pages web.

Sur un Mac, la touche **Commande** (⌘) remplit le rôle de **Ctrl** dans les tableaux ci-dessous, sauf indication contraire.

## Déplacement et sélection

| Touches | Effet |
| --- | --- |
| Touches fléchées | Déplacent la cellule active d'une cellule, en ignorant les lignes et colonnes masquées |
| Maj+flèche | Étend la sélection d'une cellule |
| Ctrl+flèche | Va au bord des données (Échap interrompt une longue recherche) |
| Ctrl+Maj+flèche | Étend la sélection jusqu'au bord des données |
| Début | Va à la première colonne de la ligne |
| Ctrl+Début | Va à la première cellule (A1, ou à la première cellule visible lorsque des lignes ou colonnes sont figées) |
| Ctrl+Fin | Va à la dernière cellule contenant une valeur ou une mise en forme |
| Page suivante, Page précédente | Descendent ou remontent d'un écran |
| Tab, Maj+Tab | Vont à droite ou à gauche (dans les cellules sélectionnées, s'il y en a plusieurs) |
| Entrée, Maj+Entrée | Descendent ou remontent (dans les cellules sélectionnées, s'il y en a plusieurs) |
| Ctrl+Espace | Sélectionne la colonne entière |
| Maj+Espace | Sélectionne la ligne entière |
| {key:SelectAll} | Sélectionne tout |

## Modification des cellules

| Touches | Effet |
| --- | --- |
| Tout caractère | Commence une saisie dans la cellule active, à la place de son contenu |
| F2 | Modifie le contenu de la cellule active, dans la cellule ; pendant la saisie d'une formule, bascule entre le pointage des cellules et la modification du texte |
| F4 | Pendant la saisie d'une formule, fixe des parties de l'adresse au niveau du curseur avec $ (A1, $A$1, A$1, $A1) |
| Entrée, Maj+Entrée | Valident la saisie, et descendent ou remontent |
| Tab, Maj+Tab | Valident la saisie, et vont à droite ou à gauche |
| Échap | Annule la saisie |
| Alt+Entrée | Insère un saut de ligne dans la cellule |
| Suppr | Efface les cellules sélectionnées |
| {key:Undo}, {key:Redo} | Annulent, rétablissent |
| {key:Save} | Enregistre |
| {key:Cut}, {key:Copy}, {key:Paste} | Couper, copier, coller (Échap supprime le contour des cellules copiées) |
| Ctrl+Alt+V | Collage spécial |
| Ctrl et le signe plus, Ctrl et le signe moins | Insèrent ou suppriment (des lignes ou des colonnes si des lignes ou colonnes entières sont sélectionnées) |
| Ctrl+D, Ctrl+R | Recopient vers le bas, vers la droite |
| Ctrl+E | Remplissage instantané |
| Alt et le signe égal | Somme |
| Ctrl+0, Ctrl+9 | Masquent les colonnes, masquent les lignes |
| Alt+Bas | Ouvre la liste d'une cellule qui en possède une |
| {key:Find} | Rechercher |
| Ctrl+H (sur un Mac, Option+Commande+F) | Rechercher et remplacer |
| F5, Ctrl+G | Atteindre une cellule, une plage ou un nom |
| Ctrl+Maj+L | Active ou désactive le filtre |
| Maj+F3 | Choisit une fonction |
| Ctrl+F3 | Affiche la liste des noms |
| F9, Maj+F9 | Recalculent tout, recalculent cette feuille |
| Tab, Entrée | Pendant la saisie d'une formule : prennent la fonction ou le nom sélectionné dans la liste |
| Échap | Interrompt une recherche, un tri ou un filtre en cours |

## Choisir vos propres touches de raccourci

Vous pouvez modifier les touches de n'importe quelle commande, ou en attribuer à une commande qui n'en a pas. Cliquez sur **Fichier > Options**, puis sur l'onglet **Personnaliser**.

1. Saisissez dans la zone **Rechercher** le nom ou les touches d'une commande pour la trouver. La liste est regroupée par onglets du ruban, et chaque commande affiche ses touches. Les commandes absentes du ruban figurent sous **Autres**.
2. Cliquez sur la commande. Ses touches sont listées sous **Touches de cette commande**.
3. Cliquez dans la zone **Appuyez sur les nouvelles touches**, puis appuyez sur les touches souhaitées, par exemple **Ctrl+Maj+B**. Cliquez ensuite sur **Attribuer**. Une commande peut avoir jusqu'à quatre touches.
4. Pour retirer une touche, sélectionnez-la dans la liste, puis cliquez sur **Supprimer**.

Si une autre commande utilise déjà les touches saisies, JeffCrunch indique laquelle. Cliquez sur **Réaffecter** pour attribuer ces touches à cette commande ; l'autre commande les perd. Rien ne change tant que vous n'avez pas cliqué sur **OK**. Ensuite, toutes les fenêtres JeffCrunch ouvertes utilisent vos touches, de même que toutes les fenêtres que vous ouvrirez plus tard. Les info-bulles, les menus et le tableau des raccourcis ci-dessous affichent vos touches.

Un symbole peut aussi avoir des touches. Voir **Insérer un symbole** dans [Saisir du texte dans les cellules](edit-cells) : cliquez sur **Raccourci clavier** dans la boîte Symbole. Il apparaît alors dans cette liste, comme une commande.

**Réinitialiser une** rétablit les touches habituelles de la commande choisie. **Tout réinitialiser** rétablit les touches habituelles de toutes les commandes. JeffCrunch ne conserve que les commandes dont vous avez modifié les touches ; une commande que vous n'avez jamais modifiée reçoit donc les meilleures touches qu'une version ultérieure lui attribuera.

Certaines touches ne peuvent pas être attribuées à une commande, car le système ou la saisie en ont besoin. JeffCrunch affiche **réservée** lorsque vous appuyez sur l'une d'elles. Les touches réservées sont **Ctrl+C**, **Ctrl+X**, **Ctrl+V**, **Ctrl+Z**, **Ctrl+A**, **Tab**, **Maj+Tab**, **Entrée**, **Échap**, **Retour arrière**, **Suppr**, **Alt+F4**, les touches fléchées, **Début**, **Fin**, **Page précédente**, **Page suivante** et **Espace**, **F1**, **F6**, **Maj+F6**, **Ctrl+Y**, **Ctrl+Maj+Z**, **Ctrl** avec une touche fléchée, **Début**, **Fin**, **Retour arrière** ou **Suppr** (avec ou sans **Maj**, lorsque cela a un sens dans une zone de texte), ainsi qu'une lettre, un chiffre ou un symbole pressé seul ou avec **Maj**. Une touche déjà utilisée par une autre partie de la fenêtre est également réservée. **Enregistrer**, **Annuler** et **Rétablir** conservent toujours au moins une touche. Sur un Mac, **Ctrl** désigne ici la touche **Commande**.

## Commandes

Ce tableau affiche les touches que vous avez choisies.

<!-- shortcuts -->

## Touches d'accès

Appuyez sur **Alt** pour afficher une lettre sur chaque onglet et chaque commande du ruban, puis appuyez sur ces lettres. Appuyez sur **Alt**, puis sur **F**, pour ouvrir l'écran Fichier. Appuyez sur **Échap** pour revenir en arrière.

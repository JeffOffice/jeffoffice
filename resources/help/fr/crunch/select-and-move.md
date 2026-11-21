# Sélectionner des cellules et se déplacer

<!-- keywords: sélectionner, sélection, cellule, plage, ligne, colonne, cliquer, glisser, maj, ctrl, flèche, début, fin, page précédente, page suivante, tab, entrée, ligne entière, colonne entière, zone de nom, atteindre, déplacer des cellules, bordure de glissement, défilement, barre d'état, somme, moyenne, nombre -->

**Sélectionner** des cellules, c'est choisir celles auxquelles la commande suivante s'appliquera. Les cellules sélectionnées sont teintées et entourées d'une bordure verte. La **cellule active** est celle où se trouve le curseur ; elle reste blanche à l'intérieur de la sélection.

## Avec la souris

- **Cliquez** sur une cellule pour la sélectionner. Cliquez à l'intérieur d'une cellule fusionnée pour sélectionner toute la cellule fusionnée.
- **Faites glisser** sur des cellules pour sélectionner une plage. Si vous faites glisser au-delà du bord de la fenêtre, la feuille défile.
- **Maj+clic** sur une autre cellule pour sélectionner la zone comprise entre la cellule active et cette cellule.
- Cliquez sur une **lettre de colonne** ou un **numéro de ligne** pour sélectionner la colonne ou la ligne entière. Faites glisser sur les lettres ou les numéros, ou utilisez Maj+clic, pour en sélectionner plusieurs.
- Cliquez sur le **coin** où se rejoignent les lettres et les numéros pour tout sélectionner.
- Maintenez **Ctrl** pendant que vous cliquez ou faites glisser pour **ajouter une plage** à la sélection. Les plages ajoutées sont teintées, et celle où se trouve le curseur a la bordure verte. **Supprimer** efface toutes les plages en une seule étape. Une commande qui ne traite qu'une plage à la fois, comme Copier, Recopier, Trier, Rechercher dans la sélection, Remplacer tout ou une mise en forme, vous le signale et ne modifie rien ; cliquez une fois pour revenir à une seule plage. Le passage à une autre feuille, ou l'insertion et la suppression de lignes ou de colonnes, ramène également à une seule plage.
- **Faites glisser la bordure** de la sélection pour déplacer les cellules à un autre endroit. Placez le pointeur sur le bord de la bordure verte jusqu'à ce qu'il prenne la forme de quatre flèches. Un contour en pointillés indique où les cellules seront placées. Tout ce qui pointe vers les cellules, comme une formule, les suit. Maintenez **Ctrl** pendant le glissement pour copier les cellules au lieu de les déplacer. Pour déplacer les cellules vers une autre feuille, commencez à faire glisser la bordure, puis maintenez **Alt** et relâchez le bouton au-dessus de l'onglet de cette feuille : les cellules sont placées dans les mêmes cellules de cette feuille (avec **Ctrl** également, elles y sont copiées). Maintenir **Alt** avant d'appuyer sur le bouton de la souris ne lance pas ce type de glissement. Les cellules déjà situées à leur destination sont remplacées, comme lors d'un déplacement sur une même feuille, et **Annuler** rétablit les deux feuilles en une seule étape. La feuille de destination doit l'autoriser : une feuille protégée, ainsi que des cellules qui couperaient des cellules fusionnées, un tableau ou un tableau croisé dynamique, sont refusées, et rien n'est modifié. Une feuille masquée n'a pas d'onglet, et le bouton **Nouvelle feuille** n'est pas un onglet : aucun des deux ne reçoit de cellules. Appuyer puis relâcher **Alt** pendant un glissement n'affiche pas les info-touches. Pour faire la même chose au clavier, coupez les cellules, allez sur l'autre feuille et collez-les. Maintenez **Maj** pendant le glissement pour insérer les cellules entre d'autres au lieu de les superposer. Pour faire de même sans souris, coupez ou copiez les cellules, puis utilisez **Insérer les cellules coupées ou copiées** (voir [Insérer et supprimer](insert-delete)). Si vous cliquez simplement sur la bordure sans la déplacer, la cellule est sélectionnée normalement.
- Faites tourner la **molette de la souris** pour faire défiler trois lignes. Maintenez **Maj** pour faire défiler horizontalement, ou maintenez **Ctrl** pour zoomer.

## Avec le clavier

Les touches fléchées déplacent la cellule active en ignorant les lignes et colonnes masquées. Maintenez **Maj** pour étendre la sélection. **Ctrl** avec une touche fléchée va jusqu'à la limite des données : jusqu'à la fin d'une suite de cellules contenant des données ou, depuis la fin d'une suite, jusqu'à la prochaine cellule contenant des données. Consultez [Utiliser JeffCrunch au clavier](keyboard) pour connaître toutes les touches.

**Ctrl+A** sélectionne le bloc de cellules autour de la cellule active, délimité par des lignes et des colonnes vides ; appuyez à nouveau pour sélectionner toute la feuille. Lorsque la sélection est déjà plus grande que ce bloc, Ctrl+A sélectionne directement toute la feuille. **Ctrl+Maj+\*** ne sélectionne que le bloc. Appuyez sur **Fin**, puis sur une touche fléchée, pour aller jusqu'à la limite des données comme avec Ctrl et la touche fléchée ; la barre d'état affiche **Fin** jusqu'à ce que vous appuyiez sur une touche fléchée, sur une autre touche ou que vous cliquiez.

## Aller à une cellule

Saisissez une adresse dans la **zone de nom**, par exemple **C5**, **A1:D10**, **B:D** ou **3:7**, puis appuyez sur **Entrée**. Appuyez sur **F5** (ou Ctrl+G) pour ouvrir la boîte de dialogue Atteindre, et saisissez-y l'adresse. Séparez les adresses par des virgules, par exemple **A1:B2,D4**, pour sélectionner plusieurs plages à la fois.

## Les nombres de la sélection

Lorsque vous sélectionnez deux cellules ou plus, la barre d'état affiche la **moyenne**, le **nombre** et la **somme** des cellules. Le nombre correspond au nombre de cellules contenant quelque chose ; la moyenne et la somme ne portent que sur les valeurs numériques. Les lignes et colonnes masquées sont exclues. Pour copier la sélection sous forme de texte, appuyez sur **Ctrl+C**.

## Ce qu'indique la barre d'état

À gauche de la barre d'état, **Prêt** indique que vous pouvez travailler ; **Entrée** s'affiche pendant la saisie dans une cellule ; **Modification** apparaît après avoir appuyé sur F2 ; **Pointage** apparaît lorsque vous pouvez cliquer ou utiliser les flèches pour placer l'adresse d'une cellule dans une formule ; et **Fin** apparaît après avoir appuyé sur la touche Fin. Lorsqu'un filtre est actif, la barre d'état indique aussi le nombre de lignes retenues par le filtre, par exemple **3 enregistrements sur 8 trouvés**.

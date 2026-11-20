# Sélectionner des cellules et se déplacer
<!-- keywords: sélectionner, sélection, cellule, plage, ligne, colonne, cliquer, glisser, maj, ctrl, flèche, début, fin, page précédente, page suivante, tab, entrée, ligne entière, colonne entière, zone de nom, atteindre, déplacer des cellules, bordure glisser, défiler, barre d'état, somme, moyenne, nombre, nb -->

**Sélectionner** des cellules signifie choisir celles auxquelles s'applique la commande suivante. Les cellules sélectionnées sont teintées, avec une bordure verte autour d'elles. La **cellule active** est celle où se trouve le curseur ; elle reste blanche à l'intérieur de la sélection.

## Avec la souris

- **Cliquez** sur une cellule pour la sélectionner. Cliquez dans une cellule fusionnée pour sélectionner la cellule fusionnée entière.
- **Faites glisser** sur des cellules pour sélectionner une plage. Faites glisser au-delà du bord de la fenêtre et la feuille défile.
- **Maj+clic** sur une autre cellule pour sélectionner de la cellule active jusqu'à cet endroit.
- Cliquez sur une **lettre de colonne** ou sur un **numéro de ligne** pour sélectionner la colonne ou la ligne entière. Faites glisser sur les lettres ou les numéros, ou utilisez Maj+clic, pour en sélectionner plusieurs.
- Cliquez sur le **coin** où se rejoignent les lettres et les numéros pour tout sélectionner.
- Maintenez **Ctrl** pendant que vous cliquez ou faites glisser pour **ajouter une plage** à la sélection. Les plages ajoutées sont teintées, et celle où se trouve le curseur a la bordure verte. **Supprimer** efface toutes les plages en une seule fois. Une commande qui ne travaille que sur une plage à la fois, comme Copier, Recopier, Trier, Rechercher dans la sélection, Remplacer tout ou une mise en forme, vous le signale et ne modifie rien ; cliquez une fois pour revenir à une seule plage. Passer à une autre feuille, ou insérer ou supprimer des lignes ou des colonnes, ramène également à une seule plage.
- **Faites glisser la bordure** de la sélection pour déplacer les cellules à un autre endroit. Placez le pointeur sur le bord de la bordure verte jusqu'à ce qu'il prenne la forme de quatre flèches. Un contour en pointillés indique où les cellules se poseront. Tout ce qui fait référence aux cellules, comme une formule, les suit. Maintenez **Ctrl** pendant le glissement pour copier les cellules au lieu de les déplacer. Maintenez **Maj** pendant le glissement pour insérer les cellules entre d'autres au lieu de les superposer. Pour faire la même chose sans souris, coupez ou copiez les cellules, puis utilisez **Insérer les cellules coupées ou copiées** (voir [Insérer et supprimer](insert-delete)). Si vous cliquez seulement sur la bordure sans la déplacer, la cellule est sélectionnée normalement.
- Tournez la **molette de la souris** pour faire défiler trois lignes. Maintenez **Maj** pour faire défiler horizontalement, ou maintenez **Ctrl** pour zoomer.

## Avec le clavier

Les touches fléchées déplacent la cellule active, en sautant les lignes et les colonnes masquées. Maintenez **Maj** pour étendre la sélection. **Ctrl** avec une touche fléchée va jusqu'au bord des données : jusqu'à la fin d'une suite de cellules contenant quelque chose, ou, depuis la fin d'une suite, jusqu'à la prochaine cellule contenant quelque chose. Consultez [Utiliser JeffCrunch avec le clavier](keyboard) pour connaître toutes les touches.

**Ctrl+A** sélectionne le bloc de cellules autour de la cellule active, délimité par des lignes et des colonnes vides ; appuyez de nouveau pour sélectionner la feuille entière. Lorsque la sélection est déjà plus grande que ce bloc, Ctrl+A sélectionne directement la feuille entière. **Ctrl+Maj+\*** sélectionne seulement le bloc. Appuyez sur **Fin**, puis sur une touche fléchée, pour aller au bord des données comme le fait **Ctrl** avec la touche fléchée ; la barre d'état affiche **Fin** jusqu'à ce que vous appuyiez sur la touche fléchée, sur une autre touche ou que vous cliquiez.

## Atteindre une cellule

Saisissez une adresse dans la **zone Nom**, par exemple **C5**, **A1:D10**, **B:D** ou **3:7**, puis appuyez sur **Entrée**. Appuyez sur **F5** (ou sur Ctrl+G) pour ouvrir Atteindre, et saisissez-y l'adresse. Séparez les adresses par des virgules, par exemple **A1:B2,D4**, pour sélectionner plusieurs plages à la fois.

## Les nombres de la sélection

Lorsque vous sélectionnez deux cellules ou plus, la barre d'état affiche la **moyenne**, le **nombre** et la **somme** des cellules. Le nombre correspond au nombre de cellules contenant quelque chose ; la moyenne et la somme ne portent que sur les nombres. Les lignes et colonnes masquées sont exclues. Pour copier la sélection sous forme de texte, appuyez sur **Ctrl+C**.

## Ce qu'indique la barre d'état

À gauche de la barre d'état, le mot **Prêt** indique que vous pouvez travailler ; **Entrée** apparaît pendant la saisie dans une cellule ; **Modifier** apparaît après avoir appuyé sur F2 ; **Pointer** apparaît lorsque vous pouvez cliquer ou utiliser les flèches pour placer l'adresse d'une cellule dans une formule ; **Fin** apparaît après avoir appuyé sur la touche Fin. Lorsqu'un filtre est actif, la barre d'état indique aussi le nombre de lignes restantes, par exemple **3 enregistrements sur 8 trouvés**.
